#!/usr/bin/env python3
"""
CodeChecker analysis tooling for SneezyMUD.

Runs clangsa (path-sensitive cross-translation-unit static analysis) plus critical AST checks
that duplicate .clang-tidy WarningsAsErrors as a safety net.

Usage:
    python3 scripts/analyze.py run [--preset PRESET] [--jobs N]
    python3 scripts/analyze.py critical [--exit-code]
    python3 scripts/analyze.py export --format {html,text,sqlite} [--critical-only] [--output PATH]

Examples:
    # Run full analysis
    python3 scripts/analyze.py run

    # Check for critical violations (CI gating)
    python3 scripts/analyze.py critical --exit-code

    # Export results (all default to codechecker-report/)
    python3 scripts/analyze.py export --format html
    python3 scripts/analyze.py export --format text
    python3 scripts/analyze.py export --format text --critical-only
    python3 scripts/analyze.py export --format sqlite
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

# =============================================================================
# Configuration
# =============================================================================

# Default paths
DEFAULT_PRESET = "dev-clang"
DEFAULT_OUTPUT_DIR = Path(".codechecker-results")
DEFAULT_HTML_DIR = Path("codechecker-report")

# Path pattern for project source files (used for filtering)
# These must be kept in sync - one for Python string matching, one for CodeChecker glob
PROJECT_PATH_PATTERN = "/code/code/"
PROJECT_GLOB_PATTERN = "*/code/code/*"

# "Critical" checks are defined as those that should block PRs/CI if found, as
# they're likely to cause crashes, security vulnerabilities, or data/game state corruption.

# Critical AST checks (same as .clang-tidy WarningsAsErrors).
#
# These checks will be surfaced by clangd in the IDE, but are duplicated here as
# a safety net in case they're not addressed during development.
#
# Sync with .clang-tidy if changes are needed (should be rare).
CRITICAL_TIDY_CHECKS: list[str] = [
    "bugprone-assert-side-effect",
    "bugprone-bool-pointer-implicit-conversion",
    "bugprone-dangling-handle",
    "bugprone-exception-escape",
    "bugprone-inaccurate-erase",
    "bugprone-infinite-loop",
    "bugprone-misplaced-widening-cast",
    "bugprone-not-null-terminated-result",
    "bugprone-parent-virtual-call",
    "bugprone-signed-char-misuse",
    "bugprone-sizeof-expression",
    "bugprone-string-constructor",
    "bugprone-suspicious-memset-usage",
    "bugprone-suspicious-semicolon",
    "bugprone-swapped-arguments",
    "bugprone-throw-keyword-missing",
    "bugprone-undelegated-constructor",
    "bugprone-undefined-memory-manipulation",
    "bugprone-unhandled-self-assignment",
    "bugprone-unused-raii",
    "bugprone-use-after-move",
    "bugprone-virtual-near-miss",
]

# Additional clang-tidy checks to run during analysis.
#
# These are useful checks that benefit from whole-codebase analysis but aren't
# critical enough to block CI. They'll appear in CodeChecker results but won't
# be flagged by the 'critical' command.
ADDITIONAL_TIDY_CHECKS: list[str] = [
    "misc-header-include-cycle",
]

# Critical clang static analyzer (clangsa) checks
#
# All clangsa checks still run and appear in full output; this list only
# controls what the 'critical' command flags as PR-blocking.
CRITICAL_SA_CHECKS: list[str] = [
    # Core checks - null deref, UB, uninitialized memory
    "core.CallAndMessage",
    "core.DivideZero",
    "core.NonNullParamChecker",
    "core.NullDereference",
    "core.StackAddressEscape",
    "core.UndefinedBinaryOperatorResult",
    "core.uninitialized.Assign",
    "core.uninitialized.Branch",
    "core.uninitialized.UndefReturn",
    # C++ checks - memory safety
    "cplusplus.InnerPointer",
    "cplusplus.Move",
    "cplusplus.NewDelete",
    "cplusplus.NewDeleteLeaks",
    # Unix/POSIX - resource misuse that causes crashes
    "unix.Errno",
    "unix.Malloc",
    "unix.Stream",
    "unix.Vfork",
    "unix.cstring.NullArg",
    # Security - insecure APIs that cause corruption
    "security.insecureAPI.vfork",
    # Opt-in checks - real crashes/corruption
    "optin.cplusplus.UninitializedObject",
    # VA list - misuse causes crashes
    "valist.Uninitialized",
    "valist.Unterminated",
    # Alpha checks - experimental but detect real crashes/corruption
    "alpha.core.StackAddressAsyncEscape",
    "alpha.cplusplus.DeleteWithNonVirtualDtor",
    "alpha.cplusplus.InvalidatedIterator",
    "alpha.cplusplus.IteratorRange",
    "alpha.cplusplus.MismatchedIterator",
    "alpha.cplusplus.SmartPtr",
    "alpha.security.ArrayBoundV2",
    "alpha.security.ReturnPtrRange",
    "alpha.unix.PthreadLock",
    "alpha.unix.cstring.BufferOverlap",
    "alpha.unix.cstring.NotNullTerminated",
    "alpha.unix.cstring.OutOfBounds",
    "alpha.unix.cstring.UninitializedRead",
]

CRITICAL_CHECKERS: set[str] = set(CRITICAL_TIDY_CHECKS + CRITICAL_SA_CHECKS)

CODECHECKER_CONFIG: list[str] = [
    "--analyzer-config", "clangsa:mode=deep",
    "--analyzer-config", "clangsa:unroll-loops=true",
    "--analyzer-config", "clangsa:aggressive-binary-operation-simplification=true",
    "--checker-config", "clangsa:unix.Stream:Pedantic=true",
]


# =============================================================================
# Data Types
# =============================================================================

@dataclass
class BugPathEvent:
    """A single step in the bug path leading to an issue."""
    file_path: str
    line: int
    message: str


@dataclass
class Issue:
    """A single static analysis issue."""
    severity: str
    checker_name: str
    category: str
    file_path: str
    line: int
    column: int
    message: str
    report_hash: str | None
    bug_path: list[BugPathEvent]

    @classmethod
    def from_json(cls, data: dict) -> Issue:
        """Create an Issue from CodeChecker JSON report format."""
        bug_path = [
            BugPathEvent(
                file_path=event["file"]["path"],
                line=event["line"],
                message=event["message"],
            )
            for event in data.get("bug_path_events", [])
        ]
        return cls(
            severity=data["severity"],
            checker_name=data["checker_name"],
            category=data.get("category", ""),
            file_path=data["file"]["path"],
            line=data["line"],
            column=data["column"],
            message=data["message"],
            report_hash=data.get("report_hash"),
            bug_path=bug_path,
        )

    @property
    def is_critical(self) -> bool:
        """Check if this issue is from a critical checker."""
        return self.checker_name in CRITICAL_CHECKERS

    @property
    def sort_key(self) -> tuple:
        """Sort key for ordering issues by severity, then file, then line."""
        severity_order = {"HIGH": 0, "MEDIUM": 1, "LOW": 2, "STYLE": 3}
        return (severity_order.get(self.severity, 4), self.file_path, self.line)


# =============================================================================
# Utility Functions
# =============================================================================

def require_tool(name: str, install_hint: str) -> None:
    """Check that a tool is available, exit with helpful message if not."""
    if not shutil.which(name):
        print(f"Error: '{name}' not found.", file=sys.stderr)
        print(f"Install with: {install_hint}", file=sys.stderr)
        sys.exit(1)


def require_python_version(minimum: tuple[int, int]) -> None:
    """Check Python version meets minimum requirement."""
    if sys.version_info < minimum:
        print(
            f"Error: Python {minimum[0]}.{minimum[1]}+ required.", file=sys.stderr)
        print(
            f"Current version: {sys.version_info.major}.{sys.version_info.minor}", file=sys.stderr)
        sys.exit(1)


def require_results(results_dir: Path) -> None:
    """Check that analysis results exist."""
    if not results_dir.is_dir():
        print(
            f"Error: No analysis results found at {results_dir}", file=sys.stderr)
        print(
            "Run 'make analyze' or 'python3 scripts/analyze.py run' first.", file=sys.stderr)
        sys.exit(1)


def require_compile_commands(preset: str) -> Path:
    """Check that compile_commands.json exists and return its path."""
    build_dir = Path(f"build/{preset}")
    compile_commands = build_dir / "compile_commands.json"

    if not compile_commands.exists():
        print(f"Error: {compile_commands} not found.", file=sys.stderr)
        print(
            f"Run 'make build PRESET={preset}' or 'cmake --preset {preset}' first.", file=sys.stderr)
        sys.exit(1)

    return compile_commands


def check_codechecker_dependencies() -> None:
    """Check all dependencies needed to run CodeChecker analysis."""
    require_tool("CodeChecker", "pip install codechecker")
    require_tool("clang", "apt install clang (use update-alternatives to set version)")
    require_tool("clang-tidy", "apt install clang-tidy (use update-alternatives to set version)")


def parse_results(results_dir: Path) -> list[Issue]:
    """Parse CodeChecker results and return list of Issues."""
    result = subprocess.run(
        ["CodeChecker", "parse", str(results_dir), "-e", "json"],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0 and not result.stdout:
        print(f"Error parsing results: {result.stderr}", file=sys.stderr)
        sys.exit(1)

    try:
        data = json.loads(result.stdout)
    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}", file=sys.stderr)
        sys.exit(1)

    reports = data.get("reports", [])
    return [Issue.from_json(r) for r in reports]


def deduplicate_issues(issues: list[Issue]) -> list[Issue]:
    """Remove duplicate issues (same location + checker)."""
    seen: set[tuple] = set()
    unique: list[Issue] = []
    for issue in issues:
        key = (issue.file_path, issue.line, issue.column, issue.checker_name)
        if key not in seen:
            seen.add(key)
            unique.append(issue)
    return unique


def filter_critical(issues: list[Issue]) -> list[Issue]:
    """Filter to only critical issues."""
    return [i for i in issues if i.is_critical]


def filter_project_files(issues: list[Issue]) -> list[Issue]:
    """Filter to only issues in project source files (code/code/).

    Excludes system headers, vendored libraries, and other external code.
    Note: Bug path events may still reference external files to show full context.
    """
    return [i for i in issues if PROJECT_PATH_PATTERN in i.file_path]


def load_issues(critical_only: bool = False) -> list[Issue]:
    """Load, filter, and deduplicate issues from the results directory.

    Always filters to project files and deduplicates. Optionally filters to
    critical issues only.
    """
    issues = parse_results(DEFAULT_OUTPUT_DIR)
    issues = filter_project_files(issues)
    if critical_only:
        issues = filter_critical(issues)
    return deduplicate_issues(issues)


# =============================================================================
# Export Formatters
# =============================================================================

def format_text(issues: list[Issue]) -> str:
    """Format issues as LLM-friendly text."""
    issues = sorted(issues, key=lambda i: i.sort_key)

    # Count by severity
    counts = {"HIGH": 0, "MEDIUM": 0, "LOW": 0, "STYLE": 0}
    for issue in issues:
        if issue.severity in counts:
            counts[issue.severity] += 1

    lines = [
        "STATIC ANALYSIS ISSUES",
        "======================",
        f"Total: {len(issues)} unique issues",
        f"  HIGH:   {counts['HIGH']}",
        f"  MEDIUM: {counts['MEDIUM']}",
        f"  LOW:    {counts['LOW']}",
        f"  STYLE:  {counts['STYLE']}",
    ]

    for issue in issues:
        bug_path_lines = []
        for event in issue.bug_path:
            filename = Path(event.file_path).name
            bug_path_lines.append(
                f"  {filename}:{event.line} - {event.message}")

        lines.extend([
            "",
            "=" * 80,
            f"SEVERITY: {issue.severity}",
            f"CHECKER:  {issue.checker_name}",
            f"CATEGORY: {issue.category}",
            f"FILE:     {issue.file_path}:{issue.line}:{issue.column}",
            f"MESSAGE:  {issue.message}",
            "BUG PATH:",
            *bug_path_lines,
        ])

    return "\n".join(lines)


def export_sqlite(issues: list[Issue], db_path: Path) -> None:
    """Export issues to SQLite database."""
    issues = sorted(issues, key=lambda i: i.sort_key)

    conn = sqlite3.connect(str(db_path))
    conn.execute("PRAGMA foreign_keys = ON")

    # Create schema
    conn.executescript("""
        DROP TABLE IF EXISTS bug_path_events;
        DROP TABLE IF EXISTS issues;

        CREATE TABLE issues (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            severity TEXT NOT NULL,
            checker_name TEXT NOT NULL,
            category TEXT,
            file_path TEXT NOT NULL,
            line INTEGER NOT NULL,
            column INTEGER NOT NULL,
            message TEXT NOT NULL,
            report_hash TEXT,
            status TEXT DEFAULT 'open' CHECK(status IN ('open', 'fixed', 'false_positive', 'wontfix')),
            UNIQUE(file_path, line, column, checker_name)
        );

        CREATE TABLE bug_path_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            issue_id INTEGER NOT NULL REFERENCES issues(id) ON DELETE CASCADE,
            step_order INTEGER NOT NULL,
            file_path TEXT NOT NULL,
            line INTEGER NOT NULL,
            message TEXT NOT NULL
        );

        CREATE INDEX idx_issues_severity ON issues(severity);
        CREATE INDEX idx_issues_checker ON issues(checker_name);
        CREATE INDEX idx_issues_file ON issues(file_path);
        CREATE INDEX idx_issues_status ON issues(status);
        CREATE INDEX idx_bug_path_issue ON bug_path_events(issue_id);
    """)

    for issue in issues:
        cursor = conn.execute("""
            INSERT OR IGNORE INTO issues
            (severity, checker_name, category, file_path, line, column, message, report_hash)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            issue.severity,
            issue.checker_name,
            issue.category,
            issue.file_path,
            issue.line,
            issue.column,
            issue.message,
            issue.report_hash,
        ))

        issue_id = cursor.lastrowid
        if issue_id and cursor.rowcount > 0:
            for i, event in enumerate(issue.bug_path):
                conn.execute("""
                    INSERT INTO bug_path_events
                    (issue_id, step_order, file_path, line, message)
                    VALUES (?, ?, ?, ?, ?)
                """, (issue_id, i, event.file_path, event.line, event.message))

    conn.commit()

    # Print summary
    print("Issues exported successfully!")
    print()
    print("Summary:")
    cursor = conn.execute("""
        SELECT severity, COUNT(*) as count FROM issues
        GROUP BY severity
        ORDER BY CASE severity WHEN 'HIGH' THEN 1 WHEN 'MEDIUM' THEN 2 WHEN 'LOW' THEN 3 ELSE 4 END
    """)
    total = 0
    for row in cursor:
        print(f"  {row[0]}: {row[1]}")
        total += row[1]
    print(f"  Total: {total}")

    print()
    print("Top checkers:")
    cursor = conn.execute("""
        SELECT checker_name, COUNT(*) as count FROM issues
        GROUP BY checker_name ORDER BY count DESC LIMIT 10
    """)
    for row in cursor:
        print(f"  {row[0]}: {row[1]}")

    conn.close()
    print()
    print(f"Database written to: {db_path}")


# =============================================================================
# Commands
# =============================================================================

def cmd_run(args: argparse.Namespace) -> int:
    """Run CodeChecker analysis."""
    # Check all dependencies upfront
    check_codechecker_dependencies()

    preset = args.preset
    jobs = args.jobs or os.cpu_count() or 4
    compile_commands = require_compile_commands(preset)

    print("Running CodeChecker comprehensive analysis:")
    print(
        f"  - clang-tidy: {len(CRITICAL_TIDY_CHECKS)} critical + {len(ADDITIONAL_TIDY_CHECKS)} additional checks")
    print(f"  - clangsa: All checks including experimental (deep mode + CTU)")
    print(f"  - Parallelism: {jobs} jobs")
    print()

    # Build tidy checks string - must start with -* to disable all defaults
    all_tidy_checks = CRITICAL_TIDY_CHECKS + ADDITIONAL_TIDY_CHECKS
    tidy_checks = "-*," + ",".join(all_tidy_checks)

    # Clean previous results
    if DEFAULT_OUTPUT_DIR.exists():
        shutil.rmtree(DEFAULT_OUTPUT_DIR)

    # Write clang-tidy args to a temp file (--tidyargs expects a file path)
    # Note: We don't use --warnings-as-errors here because CodeChecker collects
    # all issues and we filter critical vs non-critical in the Python script.
    with tempfile.NamedTemporaryFile(
        mode="w", suffix=".txt", delete=False
    ) as tidy_args_file:
        tidy_args_file.write(f"--checks={tidy_checks}\n")
        tidy_args_path = tidy_args_file.name

    try:
        cmd = [
            "CodeChecker", "analyze", str(compile_commands),
            "-o", str(DEFAULT_OUTPUT_DIR),
            "-j", str(jobs),
            "--analyzers", "clang-tidy", "clangsa",
            "--tidyargs", tidy_args_path,
            "--ctu",
            "--ctu-ast-mode", "load-from-pch",
            "--file", "*/code/code/*",
            "--enable-all",
            "--enable", "alpha.",
            "--enable", "optin.",
            *CODECHECKER_CONFIG,
            "--clean",
            "--verbose", "info",
        ]

        result = subprocess.run(cmd)
        print()
    finally:
        # Clean up temp file
        Path(tidy_args_path).unlink(missing_ok=True)

    if result.returncode != 0:
        print("Analysis completed with errors.", file=sys.stderr)

    print("Analysis complete. Loading results...")
    all_issues = load_issues(critical_only=False)
    critical_issues = filter_critical(all_issues)
    print(f"Found {len(all_issues)} issues ({len(critical_issues)} critical)")
    print()
    print("Use `make analyze-export` to see results in various formats.")

    return 0


def cmd_critical(args: argparse.Namespace) -> int:
    """Check for production-critical violations."""
    require_tool("CodeChecker", "pip install codechecker")
    require_results(DEFAULT_OUTPUT_DIR)

    print("Checking for production-critical violations...")

    critical = load_issues(critical_only=True)

    if not critical:
        print("No production-critical violations found.")
        return 0

    print(f"Found {len(critical)} production-critical violation(s):")
    print()

    for issue in sorted(critical, key=lambda i: i.sort_key):
        print(
            f"  {issue.file_path}:{issue.line}: [{issue.checker_name}] {issue.message}")

    if args.exit_code:
        return 1

    return 0


def cmd_export(args: argparse.Namespace) -> int:
    """Export results in various formats."""
    require_tool("CodeChecker", "pip install codechecker")
    require_results(DEFAULT_OUTPUT_DIR)

    fmt: str = args.format
    critical_only: bool = args.critical_only
    output: str | None = args.output

    if fmt == "html":
        # Use CodeChecker's built-in HTML export
        output_dir = Path(output) if output else DEFAULT_HTML_DIR

        if critical_only:
            # CodeChecker HTML doesn't support critical-only filtering
            critical = load_issues(critical_only=True)
            print(
                f"Note: HTML export includes all issues. Found {len(critical)} critical issues.")
            print("Use text or sqlite export for critical-only results.")

        if output_dir.exists():
            shutil.rmtree(output_dir)

        # Filter to project files only (excludes system headers, vendored libs)
        # Note: HTML export uses CodeChecker's glob filtering, which doesn't deduplicate.
        # Issue counts may differ slightly from text/sqlite exports.
        result = subprocess.run([
            "CodeChecker", "parse", str(DEFAULT_OUTPUT_DIR),
            "-e", "html",
            "-o", str(output_dir),
            "--file", PROJECT_GLOB_PATTERN,
        ])

        if result.returncode == 0:
            print()
            print(f"HTML report generated in {output_dir}/")
            print(f"Open {output_dir}/index.html in a browser to view.")

        return result.returncode

    elif fmt == "text":
        issues = load_issues(critical_only=critical_only)

        if not issues:
            print("No issues to export.")
            return 0

        text = format_text(issues)

        text_path = Path(output) if output else DEFAULT_HTML_DIR / "issues.txt"
        text_path.parent.mkdir(parents=True, exist_ok=True)
        text_path.write_text(text)
        print(f"Text report written to: {text_path}")

        return 0

    elif fmt == "sqlite":
        issues = load_issues(critical_only=critical_only)

        if not issues:
            print("No issues to export.")
            return 0

        db_path = Path(output) if output else DEFAULT_HTML_DIR / "issues.db"
        db_path.parent.mkdir(parents=True, exist_ok=True)
        export_sqlite(issues, db_path)

        return 0

    else:
        print(f"Unknown format: {fmt}", file=sys.stderr)
        return 1


# =============================================================================
# CLI
# =============================================================================

def main() -> int:
    # Require Python 3.10+ for modern type hints (list[str] instead of List[str])
    require_python_version((3, 10))

    parser = argparse.ArgumentParser(
        description="CodeChecker analysis tooling for SneezyMUD",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # run command
    run_parser = subparsers.add_parser(
        "run",
        help="Run CodeChecker analysis",
        description="Run comprehensive static analysis using clang-tidy and clangsa.",
    )
    run_parser.add_argument(
        "--preset",
        default=DEFAULT_PRESET,
        help=f"CMake preset to use (default: {DEFAULT_PRESET})",
    )
    run_parser.add_argument(
        "--jobs", "-j",
        type=int,
        help="Number of parallel jobs (default: CPU count)",
    )

    # critical command
    critical_parser = subparsers.add_parser(
        "critical",
        help="Check for critical violations",
        description="Check for production-critical violations that should block CI/PRs.",
    )
    critical_parser.add_argument(
        "--exit-code",
        action="store_true",
        help="Exit with code 1 if critical violations found",
    )

    # export command
    export_parser = subparsers.add_parser(
        "export",
        help="Export results",
        description="Export analysis results in various formats.",
    )
    export_parser.add_argument(
        "--format", "-f",
        required=True,
        choices=["html", "text", "sqlite"],
        help="Output format",
    )
    export_parser.add_argument(
        "--critical-only",
        action="store_true",
        help="Export only critical issues",
    )
    export_parser.add_argument(
        "--output", "-o",
        help="Output path (file or directory depending on format)",
    )

    args = parser.parse_args()

    if args.command == "run":
        return cmd_run(args)
    elif args.command == "critical":
        return cmd_critical(args)
    elif args.command == "export":
        return cmd_export(args)
    else:
        parser.print_help()
        return 1


if __name__ == "__main__":
    sys.exit(main())
