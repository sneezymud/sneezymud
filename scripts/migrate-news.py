#!/usr/bin/env python3
"""
Migrate lib/txt/news from a single monolithic file to individual
date-stamped files in lib/txt/news.d/.

Each entry in the old file starts with a line matching MM-DD-YY[:| :]
followed by content text (possibly with indented continuation lines).
Entries are separated by blank lines.

Output files are named YYYY-MM-DD-slug where slug is derived from the
first few words of content, sanitized to lowercase alphanumeric + hyphens.
Duplicate filenames get a numeric suffix (-2, -3, etc.).
"""

import os
import re
import sys
from pathlib import Path

# Date line pattern: MM-DD-YY followed by : or space-colon, then content
DATE_RE = re.compile(r'^(\d{2})-(\d{2})-(\d{2})\s*:\s*(.*)$')

def year_2to4(yy: int) -> int:
    """Convert 2-digit year to 4-digit. Assumes 00-99 maps to 2000-2099."""
    return 2000 + yy


def make_slug(text: str, max_words: int = 5) -> str:
    """Generate a filename slug from the first few words of text."""
    # Strip leading/trailing whitespace
    text = text.strip()
    if not text:
        return "entry"
    # Take first N words, lowercase, keep only alnum
    words = re.findall(r'[a-zA-Z0-9]+', text.lower())[:max_words]
    slug = '-'.join(words) if words else "entry"
    # Cap length
    return slug[:60]


def parse_news(filepath: str) -> list[dict]:
    """Parse the monolithic news file into a list of entries."""
    entries = []
    current = None

    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.rstrip('\n').rstrip('\r')

            m = DATE_RE.match(line)
            if m:
                # Save previous entry
                if current is not None:
                    entries.append(current)

                month, day, year = int(m.group(1)), int(m.group(2)), int(m.group(3))
                first_line = m.group(4).strip()
                current = {
                    'year': year_2to4(year),
                    'month': month,
                    'day': day,
                    'lines': [first_line] if first_line else [],
                }
            elif current is not None:
                # Blank line between entries or continuation
                stripped = line.strip()
                if stripped == '' and current['lines'] and current['lines'][-1] == '':
                    # Skip consecutive blank lines
                    continue
                # Strip common leading whitespace (indentation)
                current['lines'].append(stripped)
            # else: skip lines before first entry

    if current is not None:
        entries.append(current)

    return entries


def write_entries(entries: list[dict], outdir: str) -> int:
    """Write each entry to its own file. Returns count written."""
    os.makedirs(outdir, exist_ok=True)
    used_names: dict[str, int] = {}
    count = 0

    for entry in entries:
        # Build content: join lines, strip trailing blank lines
        lines = entry['lines']
        while lines and lines[-1] == '':
            lines.pop()
        content = '\n'.join(lines)
        if not content.strip():
            continue

        # Build filename
        date_prefix = f"{entry['year']:04d}-{entry['month']:02d}-{entry['day']:02d}"
        slug = make_slug(content)
        base = f"{date_prefix}-{slug}"

        if base in used_names:
            used_names[base] += 1
            filename = f"{base}-{used_names[base]}"
        else:
            used_names[base] = 1
            filename = base

        filepath = os.path.join(outdir, filename)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
            f.write('\n')

        count += 1

    return count


def main():
    # Resolve paths relative to repo root
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent
    news_file = repo_root / 'lib' / 'txt' / 'news'
    news_dir = repo_root / 'lib' / 'txt' / 'news.d'

    if not news_file.exists():
        print(f"Error: {news_file} not found", file=sys.stderr)
        sys.exit(1)

    if news_dir.exists() and any(news_dir.iterdir()):
        print(f"Warning: {news_dir} already has files. Proceeding anyway.")

    print(f"Parsing {news_file}...")
    entries = parse_news(str(news_file))
    print(f"Found {len(entries)} entries")

    print(f"Writing to {news_dir}/...")
    count = write_entries(entries, str(news_dir))
    print(f"Wrote {count} files")

    print("\nDone! You can verify with:")
    print(f"  ls {news_dir} | head -20")
    print(f"  ls {news_dir} | wc -l")


if __name__ == '__main__':
    main()

