#!/usr/bin/env python3
"""Migrate path-scopable memory files from memory/ to .claude/rules/.

Each entry in MIGRATIONS specifies:
- src_rel: source path relative to memory/
- paths:   list of glob patterns that should trigger this rule's auto-load

The migration:
1. Reads the source file from memory/<src_rel>
2. Parses frontmatter; injects `paths: [...]` into frontmatter
3. Writes to .claude/rules/<stem>.md
4. Deletes the source file
5. Indexer (regen_memory_index.py) is aware of .claude/rules/ for link
   validation but does NOT list these in MEMORY.md (they auto-load via
   path trigger).

Run once; safe to re-run (skips files already migrated).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
RULES_DIR = REPO_ROOT / ".claude" / "rules"

# Find the memory dir same way the indexer does
def _find_memory_dir() -> Path:
    candidates = [
        Path.home() / ".claude" / "projects" / "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile" / "memory",
        Path("/mnt/c/Users/Trenton/.claude/projects/C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile/memory"),
        Path("C:/Users/Trenton/.claude/projects/C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile/memory"),
    ]
    for c in candidates:
        if c.exists():
            return c
    return candidates[0]

MEMORY_DIR = _find_memory_dir()

FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n", re.DOTALL)

# Migration mapping: src_rel_to_memory -> paths spec
MIGRATIONS = [
    {
        "src_rel": "reference/regfix-reference.md",
        "paths": ["regfix.txt", "asmfix.txt", "regfix_stage2.txt", "tools/regfix*.py"],
    },
    {
        "src_rel": "reference/regfix-subst-multi-splice.md",
        "paths": ["regfix.txt", "asmfix.txt", "regfix_stage2.txt", "tools/regfix*.py"],
    },
    {
        "src_rel": "reference/register-asm-pins.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "reference/inline-move-aliasing.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "reference/maspsx-noreorder-stripping.md",
        "paths": ["src/*.c", "tools/maspsx/**"],
    },
    {
        "src_rel": "naming/kengo-cross-reference.md",
        "paths": ["named_syms.txt", "docs/naming/**", "kengo_matches.csv"],
    },
    {
        "src_rel": "naming/kengo-name-unreliable.md",
        "paths": ["named_syms.txt", "docs/naming/**", "kengo_matches.csv"],
    },
    {
        "src_rel": "naming/raw-dx-unsafe.md",
        "paths": ["src/*.c", "named_syms.txt"],
    },
    {
        "src_rel": "audit/bridge-signature.md",
        "paths": ["asmfix.txt"],
    },
    {
        "src_rel": "recipes/voice-control-playbook.md",
        "paths": ["src/code6cac.c"],
    },
    {
        "src_rel": "recipes/exec-game-plateau.md",
        "paths": ["src/main.c"],
    },
    {
        "src_rel": "recipes/satan2-kabuto-pin.md",
        "paths": ["src/code6cac.c", "src/code6cac_b.c"],
    },
    {
        "src_rel": "recipes/gte-3x3.md",
        "paths": ["src/display.c", "src/text1b.c"],
    },
    {
        "src_rel": "recipes/scratchpad-gte.md",
        "paths": ["src/text1b.c", "src/display.c"],
    },
    {
        "src_rel": "recipes/shared-end-label.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "reference/dead-vars-local-array.md",
        "paths": ["src/text1b.c", "src/*.c"],
    },
    {
        "src_rel": "reference/store-before-jal.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "reference/strength-reduce-defeat.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "recipes/dead-branch-scheduling.md",
        "paths": ["regfix.txt", "src/*.c"],
    },
    {
        "src_rel": "audit/lost-codegen-insert-cheat.md",
        "paths": ["regfix.txt", "src/*.c"],
    },
    {
        "src_rel": "audit/inline-asm-injection.md",
        "paths": ["src/*.c"],
    },
    {
        "src_rel": "recipes/cu-split-asmfix-drift.md",
        "paths": ["asmfix.txt", "src/text1b.c"],
    },
    {
        "src_rel": "recipes/retirement-recipes.md",
        "paths": ["asmfix.txt"],
    },
    {
        "src_rel": "recipes/cheat-cleanup-techniques.md",
        "paths": ["regfix.txt", "asmfix.txt", "regfix_stage2.txt"],
    },
]


def main() -> int:
    if not MEMORY_DIR.exists():
        print(f"memory dir not found: {MEMORY_DIR}", file=sys.stderr)
        return 1
    RULES_DIR.mkdir(parents=True, exist_ok=True)

    migrated: list[str] = []
    skipped_missing: list[str] = []
    skipped_already: list[str] = []
    errors: list[str] = []

    for m in MIGRATIONS:
        src = MEMORY_DIR / m["src_rel"]
        stem = src.stem
        dest = RULES_DIR / f"{stem}.md"

        if dest.exists():
            skipped_already.append(m["src_rel"])
            continue
        if not src.exists():
            skipped_missing.append(m["src_rel"])
            continue

        text = src.read_text(encoding="utf-8", errors="replace")
        fm_match = FRONTMATTER_RE.match(text)
        if not fm_match:
            errors.append(f"{m['src_rel']}: no frontmatter")
            continue

        # Inject paths: line into frontmatter
        fm_text = fm_match.group(1)
        body = text[fm_match.end():]

        # Build the paths value as a JSON-style list (Claude Code reads as YAML)
        paths_value = "[" + ", ".join(f'"{p}"' for p in m["paths"]) + "]"
        # Insert paths: line right after `name:` (or at end of frontmatter)
        new_fm_lines: list[str] = []
        inserted = False
        for line in fm_text.splitlines():
            new_fm_lines.append(line)
            if not inserted and line.lstrip().startswith("name:"):
                new_fm_lines.append(f"paths: {paths_value}")
                inserted = True
        if not inserted:
            new_fm_lines.append(f"paths: {paths_value}")
        new_fm = "\n".join(new_fm_lines)

        new_content = f"---\n{new_fm}\n---\n{body}"
        dest.write_text(new_content, encoding="utf-8")
        src.unlink()
        migrated.append(f"{m['src_rel']} -> .claude/rules/{stem}.md (paths: {paths_value})")

    print(f"Migrated:        {len(migrated)}")
    for line in migrated:
        print(f"  {line}")
    print()
    print(f"Already at dest: {len(skipped_already)}")
    for line in skipped_already:
        print(f"  {line}")
    print()
    print(f"Source missing:  {len(skipped_missing)}")
    for line in skipped_missing:
        print(f"  {line}")
    if errors:
        print()
        print(f"ERRORS:          {len(errors)}")
        for line in errors:
            print(f"  {line}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
