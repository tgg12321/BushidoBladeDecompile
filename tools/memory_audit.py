#!/usr/bin/env python3
"""Non-destructive audit of memory/ contents.

Walks the Claude Code memory directory, reads each file's frontmatter and body,
extracts cross-links, computes per-file metadata (size, age, line count, type,
mentioned functions/tools), identifies clusters by shared topic, flags orphans
(files not referenced from MEMORY.md), and detects broken [[link]] references.

Writes a markdown triage report to tmp/memory_audit.md for user review.
Read-only — never modifies any memory file.
"""
from __future__ import annotations

import json
import re
import sys
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

MEMORY_DIR = Path.home() / ".claude" / "projects" / "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile" / "memory"
REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_PATH = REPO_ROOT / "tmp" / "memory_audit.md"
PATH_RULES_DIR = REPO_ROOT / ".claude" / "rules"

LINK_RE = re.compile(r"\[\[([a-zA-Z0-9_\-]+)\]\]")
FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n", re.DOTALL)
FUNC_RE = re.compile(r"\bfunc_[0-9A-Fa-f]{8}\b")
HEADING_RE = re.compile(r"^#+\s+(.*?)$", re.MULTILINE)


def parse_frontmatter(text: str) -> tuple[dict[str, Any], str]:
    m = FRONTMATTER_RE.match(text)
    if not m:
        return {}, text
    fm_text = m.group(1)
    body = text[m.end():]
    fm: dict[str, Any] = {}
    current_key = None
    indent_stack: list[tuple[int, dict]] = []
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
            current_key = key
        elif stripped.startswith("- "):
            current_key = None
    return fm, body


def extract_links(body: str) -> set[str]:
    return set(LINK_RE.findall(body))


def topic_signature(name: str, body: str) -> set[str]:
    """Rough topic tags for clustering."""
    sig: set[str] = set()
    name_lower = name.lower()
    body_lower = body.lower()
    keywords = {
        "voice-control": ["voice_contorol", "single_game_voicecontorol"],
        "exec-game": ["exec_game"],
        "cc1psx": ["cc1psx"],
        "regfix": ["regfix"],
        "asmfix": ["asmfix", "bridge"],
        "naming": ["named_syms", "kengo", "rename"],
        "canonical-asm": ["canonical-asm", "canonical_asm", "inline_asm_canonical"],
        "audit": ["audit", "cheat-audit"],
        "parallel-worktree": ["worktree", "parallel"],
        "minimize-asm": ["minimize-asm", "minimize_asm", "minimize-regfix"],
        "register-pin": ["register pin", "register_asm", 'asm("'],
        "stop-rule": ["voluntary stop", "no-stopping", "wrap-up"],
        "matching-technique": ["matching playbook", "smart_match", "permuter"],
        "code6cac": ["code6cac"],
        "text1b": ["text1b"],
        "cu-split": ["cu-split", "cu_split"],
        "cheat-cleanup": ["cheat-cleanup", "cheat_cleanup", "lost_codegen"],
    }
    for tag, needles in keywords.items():
        if any(n in name_lower or n in body_lower for n in needles):
            sig.add(tag)
    return sig


def derive_type(fm: dict[str, Any], name: str) -> str:
    if "metadata" in fm and isinstance(fm["metadata"], dict):
        t = fm["metadata"].get("type", "")
        if t:
            return t
    if name.startswith("project_"):
        return "project"
    if name.startswith("user_"):
        return "user"
    if name.startswith("feedback_"):
        return "feedback"
    return "unknown"


def age_days(path: Path) -> int:
    mtime = datetime.fromtimestamp(path.stat().st_mtime, tz=timezone.utc)
    now = datetime.now(tz=timezone.utc)
    return (now - mtime).days


def looks_session_dated(name: str) -> bool:
    return bool(re.search(r"_(20\d{2}_\d{2}_\d{2}|20\d{2}-\d{2}-\d{2})", name))


def main() -> int:
    if not MEMORY_DIR.exists():
        print(f"memory dir not found: {MEMORY_DIR}", file=sys.stderr)
        return 1

    files: list[dict[str, Any]] = []
    all_names: set[str] = set()
    fm_name_to_filename: dict[str, str] = {}

    # Also know about path-scoped rules in .claude/rules/ for link validation
    # (their stems and frontmatter names are valid [[link]] targets)
    if PATH_RULES_DIR.exists():
        for path in sorted(PATH_RULES_DIR.glob("*.md")):
            all_names.add(path.stem)
            text = path.read_text(encoding="utf-8", errors="replace")
            fm, _ = parse_frontmatter(text)
            fm_name = (fm.get("name") or path.stem).strip()
            if fm_name:
                fm_name_to_filename[fm_name] = path.stem

    for path in sorted(MEMORY_DIR.rglob("*.md")):
        text = path.read_text(encoding="utf-8", errors="replace")
        fm, body = parse_frontmatter(text)
        fm_name = (fm.get("name") or path.stem).strip()
        all_names.add(path.stem)
        if fm_name:
            fm_name_to_filename[fm_name] = path.stem
        links_out = extract_links(body)
        topics = topic_signature(path.stem, body)
        funcs = set(FUNC_RE.findall(body))
        size = path.stat().st_size
        lines = text.count("\n") + 1
        headings = [m.group(1).strip() for m in HEADING_RE.finditer(body)]
        h1 = next((h for h in headings if not h.startswith("##")), "")
        files.append({
            "path": path,
            "name": path.stem,
            "fm_name": fm_name,
            "fm_type": derive_type(fm, path.stem),
            "description": fm.get("description", "")[:200],
            "size": size,
            "lines": lines,
            "age_days": age_days(path),
            "links_out": sorted(links_out),
            "topics": sorted(topics),
            "funcs_mentioned": sorted(funcs),
            "first_heading": h1,
            "session_dated": looks_session_dated(path.stem),
        })

    # Resolve [[link]] targets: links can target frontmatter `name` slugs OR filenames
    def resolve_link(slug: str) -> str | None:
        # Try direct filename match
        if slug in all_names:
            return slug
        # Try frontmatter name lookup
        if slug in fm_name_to_filename:
            return fm_name_to_filename[slug]
        return None

    # MEMORY.md special handling
    memory_index = MEMORY_DIR / "MEMORY.md"
    indexed_files: set[str] = set()
    memory_size = 0
    memory_lines = 0
    if memory_index.exists():
        idx_text = memory_index.read_text(encoding="utf-8", errors="replace")
        memory_size = memory_index.stat().st_size
        memory_lines = idx_text.count("\n") + 1
        # Find filename mentions
        for f in files:
            if f["name"] == "MEMORY":
                continue
            if f["name"] + ".md" in idx_text or f"({f['name']}.md)" in idx_text:
                indexed_files.add(f["name"])

    # Cluster by topic signature
    clusters: dict[str, list[str]] = defaultdict(list)
    for f in files:
        if f["name"] == "MEMORY":
            continue
        for topic in f["topics"]:
            clusters[topic].append(f["name"])

    # Broken links
    broken_links: list[tuple[str, str]] = []
    for f in files:
        for link in f["links_out"]:
            if resolve_link(link) is None:
                broken_links.append((f["name"], link))

    # Orphans (not in MEMORY.md, excluding MEMORY itself)
    orphans = sorted(
        f["name"] for f in files
        if f["name"] != "MEMORY" and f["name"] not in indexed_files
    )

    # Write report
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    with OUTPUT_PATH.open("w", encoding="utf-8") as out:
        out.write("# Memory audit\n\n")
        out.write(f"Generated: {datetime.now(tz=timezone.utc).strftime('%Y-%m-%d %H:%M UTC')}\n")
        out.write(f"Source: `{MEMORY_DIR}`\n\n")

        # Summary
        non_index = [f for f in files if f["name"] != "MEMORY"]
        total_size = sum(f["size"] for f in non_index)
        out.write("## Summary\n\n")
        out.write(f"- **Total files:** {len(non_index)} (+ MEMORY.md index)\n")
        out.write(f"- **Total size:** {total_size:,} bytes ({total_size/1024:.1f} KB)\n")
        out.write(f"- **MEMORY.md:** {memory_size:,} bytes, {memory_lines} lines (Claude Code limit: 25,600 bytes / 200 lines)\n")
        out.write(f"- **Indexed:** {len(indexed_files)}\n")
        out.write(f"- **Orphan (not in MEMORY.md):** {len(orphans)}\n")
        out.write(f"- **Broken [[links]]:** {len(broken_links)}\n")
        out.write(f"- **Session-dated files:** {sum(1 for f in non_index if f['session_dated'])}\n")
        out.write("\n")

        # Type distribution
        type_counts: dict[str, int] = defaultdict(int)
        for f in non_index:
            type_counts[f["fm_type"]] += 1
        out.write("### Type distribution (from frontmatter `metadata.type`)\n\n")
        for t, c in sorted(type_counts.items(), key=lambda x: -x[1]):
            out.write(f"- `{t}`: {c}\n")
        out.write("\n")

        # MEMORY.md status
        out.write("## MEMORY.md status\n\n")
        if memory_size > 25600:
            out.write(f"⚠️  **MEMORY.md is {memory_size:,} bytes — OVER the 25,600-byte (25KB) auto-load limit.** Tail content beyond line 200 is truncated at session start.\n\n")
        elif memory_lines > 200:
            out.write(f"⚠️  **MEMORY.md is {memory_lines} lines — OVER the 200-line auto-load limit.**\n\n")
        else:
            out.write(f"✓ MEMORY.md is within auto-load limit.\n\n")

        # Topic clusters (multi-file)
        out.write("## Topic clusters (multiple files share a topic)\n\n")
        multi_clusters = {k: sorted(v) for k, v in clusters.items() if len(v) > 1}
        for topic in sorted(multi_clusters, key=lambda t: -len(multi_clusters[t])):
            names = multi_clusters[topic]
            out.write(f"### `{topic}` — {len(names)} files\n\n")
            for n in names:
                f = next(x for x in files if x["name"] == n)
                age = f["age_days"]
                size_kb = f["size"] / 1024
                desc = f["description"][:120]
                out.write(f"- **{n}** ({size_kb:.1f}KB, {f['lines']}L, {age}d old) — {desc}\n")
            out.write("\n")

        # Orphans
        out.write("## Orphan files (not referenced from MEMORY.md)\n\n")
        if orphans:
            for n in orphans:
                f = next(x for x in files if x["name"] == n)
                out.write(f"- **{n}** ({f['size']/1024:.1f}KB, {f['age_days']}d old)\n")
        else:
            out.write("(none)\n")
        out.write("\n")

        # Broken links
        out.write("## Broken [[link]] references\n\n")
        if broken_links:
            for src, tgt in sorted(set(broken_links)):
                out.write(f"- `{src}` → `[[{tgt}]]` (target not found)\n")
        else:
            out.write("(none)\n")
        out.write("\n")

        # Session-dated files (candidates for archive)
        out.write("## Session-dated files (candidates for memory/history/)\n\n")
        session_files = [f for f in non_index if f["session_dated"]]
        if session_files:
            for f in sorted(session_files, key=lambda x: -x["age_days"]):
                out.write(f"- **{f['name']}** ({f['age_days']}d old, {f['size']/1024:.1f}KB)\n")
        else:
            out.write("(none)\n")
        out.write("\n")

        # Per-file inventory (full)
        out.write("## Full inventory\n\n")
        out.write("| File | Type | Size | Lines | Age | FM-name | First heading |\n")
        out.write("|------|------|-----:|------:|----:|---------|---------------|\n")
        for f in non_index:
            heading = f["first_heading"][:50].replace("|", "\\|") if f["first_heading"] else ""
            fm_name = f["fm_name"][:40] if f["fm_name"] else ""
            out.write(f"| `{f['name']}` | {f['fm_type']} | {f['size']/1024:.1f}KB | {f['lines']} | {f['age_days']}d | {fm_name} | {heading} |\n")
        out.write("\n")

        # Function mentions (per-function memory map)
        out.write("## Per-function memory (files mentioning specific func_XXXXXXXX)\n\n")
        func_to_files: dict[str, list[str]] = defaultdict(list)
        for f in non_index:
            for fn in f["funcs_mentioned"]:
                func_to_files[fn].append(f["name"])
        for fn in sorted(func_to_files, key=lambda x: -len(func_to_files[x]))[:25]:
            files_for_fn = func_to_files[fn]
            if len(files_for_fn) > 1:
                out.write(f"- **{fn}** — {len(files_for_fn)} files: {', '.join('`'+x+'`' for x in files_for_fn)}\n")
        out.write("\n")

        # JSON data dump for downstream tooling
        out.write("## Raw data (for downstream tools)\n\n")
        out.write("<details><summary>JSON dump</summary>\n\n```json\n")
        json_data = {
            "files": [{k: v if not isinstance(v, Path) else str(v) for k, v in f.items()} for f in non_index],
            "memory_md_size": memory_size,
            "memory_md_lines": memory_lines,
            "orphans": orphans,
            "broken_links": broken_links,
            "multi_clusters": multi_clusters,
        }
        out.write(json.dumps(json_data, indent=2, default=str))
        out.write("\n```\n\n</details>\n")

    print(f"Wrote audit report: {OUTPUT_PATH}")
    print(f"Files: {len(non_index)}, Total: {total_size/1024:.1f}KB")
    print(f"MEMORY.md: {memory_size:,} bytes ({memory_lines} lines)")
    print(f"Orphans: {len(orphans)}, Broken links: {len(broken_links)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
