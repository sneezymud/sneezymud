# SneezyMUD Build System
#
# Usage:
#   make              - Build with default preset (dev-clang)
#   make run          - Run with sanitizer options
#   make PRESET=dev-gcc build  - Build with specific preset
#
# Available presets: dev-gcc, dev-clang, release-gcc, release-clang

PRESET ?= dev-clang
BUILD_DIR := build/$(PRESET)
BINARY := code/sneezy

# Sanitizer options for development runs
export ASAN_OPTIONS := strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
export UBSAN_OPTIONS := print_stacktrace=1:halt_on_error=1

.PHONY: all build configure clean clean-all rebuild run debug format help
.PHONY: build-profile ninja-trace iwyu-check iwyu-fix
.PHONY: analyze analyze-export analyze-ci
.PHONY: check-bun check-python3
.PHONY: test test-func test-all
.PHONY: restore-backup

all: build

# Configure the build directory using CMake preset
configure:
	cmake --preset $(PRESET)

# Build the project (configures if needed)
build:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

# Clean the build directory for current preset
clean:
	rm -rf $(BUILD_DIR)

# Clean all build directories and analysis results
clean-all:
	rm -rf build .codechecker-results codechecker-report

# Clean and rebuild
rebuild: clean build

# Run sneezy with strict sanitizer options
run: build
	./$(BINARY)

# Run sneezy under gdb
debug: build
	gdb -ex run ./$(BINARY)


# Run C++ unit tests via CTest
test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# Run functional tests via bun (optionally: make test-func TEST=smoke)
test-func: check-bun
	cd tests/functional && bun test $(if $(TEST),tests/$(TEST).test.ts)

# Run all tests (C++ unit + functional)
test-all: test test-func

# Restore a nightly backup to local dev environment
# Usage: make restore-backup [DOCKER=1] [DATE=YYYY-MM-DD]
restore-backup:
	./scripts/restore-backup-dev.sh $(if $(DOCKER),--docker) $(if $(DATE),--date $(DATE))

# Format C++ source files (all files, or specific file if FILE is set)
# Edits in-place using the rules in the .clang-format file in the root folder
format:
ifdef FILE
	clang-format -i --verbose $(FILE)
else
	find code/code/ -type f \( -iname "*.h" -o -iname "*.cc" \) -exec clang-format -i --verbose {} +
endif

# Run IWYU analysis (dry run, no changes)
iwyu-check: configure
	cmake --build --preset $(PRESET) --target iwyu-check

# Run IWYU analysis and apply fixes
iwyu-fix: configure
	cmake --build --preset $(PRESET) --target iwyu-fix

# Profile the build using ClangBuildAnalyzer (requires Clang preset)
build-profile:
	./scripts/build-profile.sh $(PRESET)

# Generate Ninja build trace for visualization
ninja-trace:
	./scripts/ninja-trace.sh $(PRESET)

# Static analysis via CodeChecker (Python script)
# All analysis logic is in scripts/analyze.py for clarity and maintainability.
#
# Workflow:
#   1. make analyze                  - Run full analysis
#   2. make analyze-export FMT=X     - Export results of previous run in desired format
#   3. make analyze-ci               - CI gating (exits non-zero if critical issues)
#
# The Python script handles:
#   - clang-tidy: 22 production-critical AST checks (same as .clang-tidy WarningsAsErrors)
#   - clangsa: All checks including experimental alpha (deep mode + CTU)
#   - Filtering to critical-only issues
#   - Multiple output formats (html, text, sqlite) - all default to codechecker-report/

PYTHON := python3
ANALYZE := $(PYTHON) scripts/analyze.py

check-bun:
	@command -v bun >/dev/null 2>&1 || { \
		echo "Error: 'bun' not found."; \
		echo "Install: https://bun.sh/docs/installation"; \
		exit 1; \
	}

check-python3:
	@command -v $(PYTHON) >/dev/null 2>&1 || { \
		echo "Error: 'python3' not found."; \
		echo "Install with: apt install python3"; \
		exit 1; \
	}

# Run comprehensive static analysis
analyze: build check-python3
	$(ANALYZE) run --preset $(PRESET)

# Unified export target with parameters
# Usage: make analyze-export [FMT=html|text|sqlite] [CRITICAL=1] [OUTPUT=path]
# Examples:
#   make analyze-export                     # HTML report (codechecker-report/)
#   make analyze-export FMT=text            # Text (codechecker-report/issues.txt)
#   make analyze-export FMT=sqlite          # SQLite (codechecker-report/issues.db)
#   make analyze-export FMT=text CRITICAL=1 # Critical-only text
FMT ?= html
analyze-export: check-python3
	$(ANALYZE) export --format $(FMT) $(if $(CRITICAL),--critical-only) $(if $(OUTPUT),--output $(OUTPUT))

# CI gating - exits non-zero if critical violations found
analyze-ci: check-python3
	$(ANALYZE) critical --exit-code

help:
	@echo "SneezyMUD Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build           - Configure and build (default)"
	@echo "  configure       - Configure only (no build)"
	@echo "  clean           - Remove build directory for current preset"
	@echo "  clean-all       - Remove all build directories"
	@echo "  rebuild         - Clean and rebuild current preset"
	@echo "  run             - Build and run with sanitizer options"
	@echo "  debug           - Build and run under gdb"
	@echo "  test            - Run C++ unit tests"
	@echo "  test-func       - Run functional tests (requires running server, TEST=name)"
	@echo "  test-all        - Run all tests"
	@echo "  restore-backup  - Restore a nightly backup (DOCKER=1, DATE=YYYY-MM-DD)"
	@echo "  format          - Run clang-format (all files, or FILE=path)"
	@echo "  iwyu-check      - Run include-what-you-use analysis (dry run)"
	@echo "  iwyu-fix        - Run include-what-you-use and apply fixes"
	@echo "  analyze         - Run comprehensive static analysis"
	@echo "  analyze-export  - Export results (FMT=html|text|sqlite)"
	@echo "  analyze-ci      - CI gating (exits non-zero if critical issues)"
	@echo "  build-profile   - Profile build with ClangBuildAnalyzer"
	@echo "  ninja-trace     - Generate Ninja build trace"
	@echo ""
	@echo "Variables:"
	@echo "  PRESET          - CMake preset (default: dev-clang)"
	@echo "  TEST            - Specific functional test name (e.g., smoke, gmcp)"
	@echo "  FILE            - Specific file for format target"
	@echo "  FMT             - Export format: html, text, or sqlite (default: html)"
	@echo "  CRITICAL        - Set to 1 to filter to critical issues only"
	@echo "  OUTPUT          - Output path for export"
	@echo ""
	@echo "Available presets:"
	@echo "  dev-gcc         - Debug build with GCC, ASan + UBSan"
	@echo "  dev-clang       - Debug build with Clang, ASan + UBSan"
	@echo "  release-gcc     - Release build with GCC, ASan + UBSan + LTO"
	@echo "  release-clang   - Release build with Clang, ASan + UBSan + ThinLTO"
	@echo ""
	@echo "Examples:"
	@echo "  make                                      # Build with dev-clang"
	@echo "  make PRESET=dev-gcc                       # Build with dev-gcc"
	@echo "  make analyze                              # Run static analysis"
	@echo "  make analyze-export                       # Full HTML report"
	@echo "  make analyze-export FMT=text CRITICAL=1   # Critical issues as text"
	@echo "  make analyze-ci                           # CI check (fails if issues)"
