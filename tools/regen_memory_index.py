#!/usr/bin/env python3
"""Regenerate memory/MEMORY.md from frontmatter of all memory/*/*.md files.

Reads each file's frontmatter `name`, `description`, and `metadata.type`.
Groups entries by directory, renders a structured markdown index, validates
the result against Claude Code's auto-load limits (200 lines OR 25,600 bytes).

Fails (non-zero exit) if:
- Any file has missing/malformed frontmatter
- Any [[link]] reference doesn't resolve to a file stem
- The generated MEMORY.md exceeds 200 lines or 24KB

Intended to be run via pre-commit hook so MEMORY.md is
never hand-edited and always reflects current memory/ state.
"""
from __future__ import annotations

import re
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any

def _find_memory_dir() -> Path:
    """Find the memory dir whether running from Windows or WSL.

    On Windows Git Bash, ~ is C:/Users/Trenton. On WSL, ~ is /home/trenton
    — but the actual memory dir lives on the Windows side at /mnt/c/Users/...
    Tries known candidates in priority order.
    """
    candidates = [
        Path.home() / ".claude" / "projects" / "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile" / "memory",
        Path("/mnt/c/Users/Trenton/.claude/projects/C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile/memory"),
        Path("/c/Users/Trenton/.claude/projects/C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile/memory"),
        Path("C:/Users/Trenton/.claude/projects/C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile/memory"),
    ]
    for c in candidates:
        if c.exists():
            return c
    # Fall back to first candidate so the error message is clear
    return candidates[0]


MEMORY_DIR = _find_memory_dir()
INDEX_PATH = MEMORY_DIR / "MEMORY.md"

# Path-scoped rules live in the repo at .claude/rules/. They auto-load
# when Claude reads matching files (per `paths:` frontmatter), so they
# do NOT appear in MEMORY.md. But [[link]] cross-references from memory/
# may target them, so the indexer must know about them for link validation.
REPO_PATH_RULES_DIR = (Path(__file__).resolve().parent.parent / ".claude" / "rules")

# Auto-load limits per Claude Code docs.
MAX_LINES = 200
MAX_BYTES = 24_576   # docs say 25,600 (25KB), leave 1KB headroom

# Directory descriptions and ordering for the index header.
DIR_ORDER = [
    ("rules",     "Durable standards + principles — COMPLETED-C bar, anti-cheat, canonical-asm, programmatic-first."),
    ("workflow",  "Harness/environment mechanics — WSL build invocation, LF line endings."),
    ("recipes",   "Named technique playbooks. Load on demand."),
    ("reference", "Durable technical reference — matching playbook, toolchain facts (cc1/maspsx/m2c), scoring. Load on demand."),
    ("audit",     "Cheat detection + authorization patterns."),
    ("naming",    "Naming-specific lessons."),
    ("project",   "Current state — the greenfield engine (the workflow) + durable function facts."),
    ("user",      "User profile + standing directives. Loaded first by the agent harness."),
    ("history",   "Archived dated session notes. Not auto-loaded; browse for traceability of past sessions."),
]

FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n", re.DOTALL)
LINK_RE = re.compile(r"\[\[([a-zA-Z0-9_\-]+)\]\]")
SENTENCE_END_RE = re.compile(r"[.!?]\s+|[.!?]$")


def parse_frontmatter(text: str) -> tuple[dict[str, Any], str]:
    m = FRONTMATTER_RE.match(text)
    if not m:
        return {}, text
    fm_text = m.group(1)
    body = text[m.end():]
    fm: dict[str, Any] = {}
    for raw in fm_text.splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        stripped = raw.strip()
        if ":" in stripped and not stripped.startswith("- "):
            key, _, val = stripped.partition(":")
            key = key.strip()
            val = val.strip()
            if val.startswith('"') and val.endswith('"'):
                val = val[1:-1]
            fm[key] = val if val else {}
    return fm, body


def first_sentence(text: str, max_chars: int = 140) -> str:
    """Extract first sentence (or up to max_chars), strip whitespace."""
    text = text.strip()
    # Unescape backslash-quote artifacts from YAML frontmatter parsing
    text = text.replace('\\"', '"').replace("\\'", "'")
    if not text:
        return ""
    m = SENTENCE_END_RE.search(text)
    if m and m.end() <= max_chars + 30:  # accept overshoot if sentence ends close
        sent = text[:m.end()].rstrip()
    else:
        sent = text[:max_chars].rstrip()
        if len(text) > max_chars:
            # Truncate at last whitespace for clean cut
            cut = sent.rfind(" ")
            if cut > max_chars // 2:
                sent = sent[:cut] + "…"
            else:
                sent = sent + "…"
    return sent.rstrip(".")


def main() -> int:
    if not MEMORY_DIR.exists():
        print(f"memory dir not found: {MEMORY_DIR}", file=sys.stderr)
        return 1

    # Phase 1: collect entries per directory
    entries_by_dir: dict[str, list[dict[str, Any]]] = defaultdict(list)
    all_stems: set[str] = set()
    fm_name_to_stem: dict[str, str] = {}
    errors: list[str] = []

    for path in sorted(MEMORY_DIR.rglob("*.md")):
        if path.name == "MEMORY.md":
            continue
        rel = path.relative_to(MEMORY_DIR)
        dir_name = rel.parts[0] if len(rel.parts) > 1 else "root"
        stem = path.stem
        all_stems.add(stem)
        text = path.read_text(encoding="utf-8", errors="replace")
        fm, body = parse_frontmatter(text)
        if not fm.get("name"):
            errors.append(f"missing frontmatter name: {rel}")
            continue
        fm_name = fm["name"].strip()
        fm_name_to_stem[fm_name] = stem
        description = fm.get("description", "").strip()
        if not description:
            errors.append(f"missing description: {rel}")
        entries_by_dir[dir_name].append({
            "stem": stem,
            "name": fm_name,
            "description": description,
            "rel_path": str(rel).replace("\\", "/"),
            "size": path.stat().st_size,
        })

    # Also collect path-scoped rules from .claude/rules/ — these are auto-loaded
    # by path trigger, not via MEMORY.md, but cross-references must resolve.
    path_scoped_stems: list[tuple[str, list[str]]] = []
    if REPO_PATH_RULES_DIR.exists():
        for path in sorted(REPO_PATH_RULES_DIR.glob("*.md")):
            stem = path.stem
            all_stems.add(stem)
            text = path.read_text(encoding="utf-8", errors="replace")
            fm, _ = parse_frontmatter(text)
            fm_name = (fm.get("name") or stem).strip()
            fm_name_to_stem[fm_name] = stem
            # Parse paths: from frontmatter (simple list of strings)
            paths_field = fm.get("paths", "[]")
            paths_list: list[str] = []
            if isinstance(paths_field, str):
                inner = paths_field.strip().lstrip("[").rstrip("]")
                paths_list = [p.strip().strip('"').strip("'") for p in inner.split(",") if p.strip()]
            path_scoped_stems.append((stem, paths_list))

    # Phase 2: validate all [[links]] resolve (across memory/ + .claude/rules/)
    broken: list[tuple[str, str]] = []
    for path in sorted(MEMORY_DIR.rglob("*.md")):
        if path.name == "MEMORY.md":
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for m in LINK_RE.finditer(text):
            target = m.group(1)
            if target not in all_stems and target not in fm_name_to_stem:
                broken.append((str(path.relative_to(MEMORY_DIR)), target))
    # Also validate links within path-scoped rule files
    if REPO_PATH_RULES_DIR.exists():
        for path in sorted(REPO_PATH_RULES_DIR.glob("*.md")):
            text = path.read_text(encoding="utf-8", errors="replace")
            for m in LINK_RE.finditer(text):
                target = m.group(1)
                if target not in all_stems and target not in fm_name_to_stem:
                    broken.append((f".claude/rules/{path.name}", target))

    if errors:
        print("Frontmatter errors:", file=sys.stderr)
        for e in errors:
            print(f"  {e}", file=sys.stderr)
        return 1

    if broken:
        print(f"WARN: {len(broken)} broken [[link]] references:", file=sys.stderr)
        for src, tgt in sorted(set(broken))[:15]:
            print(f"  {src} -> [[{tgt}]]", file=sys.stderr)
        if len(broken) > 15:
            print(f"  ... and {len(broken) - 15} more", file=sys.stderr)
        # Don't fail; warn and continue.

    # Phase 3: render MEMORY.md
    out_lines: list[str] = []
    out_lines.append("# Memory index")
    out_lines.append("")
    out_lines.append("**New agent? Read [user/role.md](user/role.md) and [project/greenfield-engine-v2.md](project/greenfield-engine-v2.md) first — the engine IS the workflow.**")
    out_lines.append("**Build/verify:** run `python3 -m engine.cli verify-oracle` in WSL. Full CLI + design in greenfield-engine-v2.")
    out_lines.append("")
    out_lines.append("Auto-generated by `tools/regen_memory_index.py` from per-file frontmatter. Do not hand-edit.")
    out_lines.append("")

    for dir_name, dir_desc in DIR_ORDER:
        items = entries_by_dir.get(dir_name, [])
        if not items:
            continue
        out_lines.append(f"## {dir_name}/")
        out_lines.append(f"_{dir_desc}_")
        out_lines.append("")
        items_sorted = sorted(items, key=lambda x: x["stem"])
        for entry in items_sorted:
            summary = first_sentence(entry["description"], max_chars=130)
            out_lines.append(f"- [{entry['stem']}]({entry['rel_path']}) — {summary}")
        out_lines.append("")

    # Any uncategorized (shouldn't happen, but guard)
    known_dirs = {d for d, _ in DIR_ORDER}
    other_dirs = [d for d in entries_by_dir if d not in known_dirs]
    for d in sorted(other_dirs):
        out_lines.append(f"## {d}/  (uncategorized)")
        out_lines.append("")
        for entry in sorted(entries_by_dir[d], key=lambda x: x["stem"]):
            summary = first_sentence(entry["description"], max_chars=130)
            out_lines.append(f"- [{entry['stem']}]({entry['rel_path']}) — {summary}")
        out_lines.append("")

    # Path-scoped rules — these don't appear in browse navigation because
    # they auto-load when Claude reads matching files. Listed here as a
    # short summary so the existence + trigger map is discoverable.
    if path_scoped_stems:
        out_lines.append("## .claude/rules/ — path-scoped rules (auto-load on matching file reads)")
        out_lines.append("_They load only when Claude reads a file matching their `paths:` glob. Technique rules are ON-DEMAND: the **codegen-technique-index** rule (auto-loads on `src/*.c`) maps symptoms to slugs; Read the rule file when a symptom matches._")
        out_lines.append("")
        on_demand: list[str] = []
        for stem, paths in sorted(path_scoped_stems):
            if paths == [f".claude/rules/{stem}.md"]:
                on_demand.append(stem)
                continue
            paths_str = ", ".join(f"`{p}`" for p in paths) if paths else "(no paths)"
            out_lines.append(f"- **{stem}** — {paths_str}")
        if on_demand:
            out_lines.append(f"- _on-demand (via codegen-technique-index):_ {', '.join(on_demand)}")
        out_lines.append("")

    content = "\n".join(out_lines).rstrip() + "\n"

    # Phase 4: validate size
    n_lines = content.count("\n")
    n_bytes = len(content.encode("utf-8"))
    print(f"Generated: {n_lines} lines, {n_bytes:,} bytes")
    print(f"Limits:    {MAX_LINES} lines, {MAX_BYTES:,} bytes")

    if n_lines > MAX_LINES:
        print(f"FAIL: {n_lines} lines exceeds {MAX_LINES} limit", file=sys.stderr)
        return 1
    if n_bytes > MAX_BYTES:
        print(f"FAIL: {n_bytes:,} bytes exceeds {MAX_BYTES:,} limit", file=sys.stderr)
        return 1

    # Write
    INDEX_PATH.write_text(content, encoding="utf-8")
    print(f"Wrote: {INDEX_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
