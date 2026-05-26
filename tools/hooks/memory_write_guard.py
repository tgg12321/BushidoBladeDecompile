#!/usr/bin/env python3
"""PreToolUse hook on Edit|Write|MultiEdit. Validates writes to memory/
and .claude/rules/ against project conventions.

Blocks writes that would break the indexer or create badly-formatted
files. Pass-through for any other path.

Exit codes:
  0 — allow (path not guarded, or all checks pass)
  2 — block (validation failure; reason on stderr per Claude Code hook convention)
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n", re.DOTALL)
KEBAB_RE = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*$")

# Valid memory/ subdirectories and their expected metadata.type values
MEMORY_DIRS = {
    "rules": "rule",
    "workflow": "workflow",
    "recipes": "recipe",
    "reference": "reference",
    "audit": "audit",
    "naming": "naming",
    "project": "project",
    "user": "user",
    "history": "history",
}

MAX_DESC_LEN = 500  # description max chars. MEMORY.md stays compact regardless:
                    # regen_memory_index.py shows only the first sentence (<=130 chars).


def parse_frontmatter(text: str) -> tuple[dict, str] | tuple[None, str]:
    m = FRONTMATTER_RE.match(text)
    if not m:
        return None, text
    fm_text = m.group(1)
    body = text[m.end():]
    fm: dict = {}
    in_metadata = False
    metadata: dict = {}
    for raw in fm_text.splitlines():
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        if raw.startswith("  ") and in_metadata:
            stripped = raw.strip()
            if ":" in stripped:
                key, _, val = stripped.partition(":")
                metadata[key.strip()] = val.strip()
            continue
        in_metadata = False
        stripped = raw.strip()
        if not stripped or stripped.startswith("- "):
            continue
        if ":" in stripped:
            key, _, val = stripped.partition(":")
            key = key.strip()
            val = val.strip()
            if val.startswith('"') and val.endswith('"'):
                val = val[1:-1]
            if key == "metadata":
                in_metadata = True
                continue
            fm[key] = val
    if metadata:
        fm["metadata"] = metadata
    return fm, body


def validate_content(file_path: str, content: str) -> list[str]:
    """Return list of validation errors. Empty list = pass."""
    errors: list[str] = []
    p = Path(file_path)
    stem = p.stem
    name = p.name

    # Filename must be kebab-case + .md
    if not name.endswith(".md"):
        errors.append(f"file must end in .md (got {name})")
        return errors
    if not KEBAB_RE.match(stem):
        errors.append(f"filename must be kebab-case (got {stem!r}; use letters/digits/hyphens only, no underscores or feedback_/project_ prefixes)")

    # Determine scope and dir-type
    path_str = str(p).replace("\\", "/")
    is_rules = "/.claude/rules/" in path_str
    memory_dir = None
    for d in MEMORY_DIRS:
        if f"/memory/{d}/" in path_str:
            memory_dir = d
            break

    if not is_rules and memory_dir is None:
        # Shouldn't be guarded; bail
        return []

    # Frontmatter required
    fm, body = parse_frontmatter(content)
    if fm is None:
        errors.append("missing frontmatter (required: opening `---` line as line 1)")
        return errors

    # Common required fields
    if not fm.get("name"):
        errors.append("frontmatter missing required field: `name:` (kebab-case slug)")
    elif fm["name"] != stem:
        errors.append(f"frontmatter `name: {fm['name']}` must match filename stem `{stem}`")

    description = fm.get("description", "")
    if not description:
        errors.append("frontmatter missing required field: `description:` (one-line summary used in MEMORY.md index)")
    elif len(description) > MAX_DESC_LEN:
        errors.append(f"description too long ({len(description)} chars > {MAX_DESC_LEN}) — would bloat MEMORY.md")

    # Scope-specific validation
    if memory_dir:
        # memory/ files need metadata.type matching the directory
        md = fm.get("metadata", {})
        if not isinstance(md, dict):
            errors.append("frontmatter `metadata:` must be a nested block")
        else:
            type_ = md.get("type", "")
            expected = MEMORY_DIRS[memory_dir]
            if not type_:
                errors.append(f"frontmatter `metadata.type:` is required; expected `{expected}` for files in memory/{memory_dir}/")
            elif type_ != expected:
                errors.append(f"frontmatter `metadata.type: {type_}` does not match directory memory/{memory_dir}/ (expected `{expected}`)")

    if is_rules:
        # .claude/rules/ files need paths: frontmatter
        paths_field = fm.get("paths", "")
        if not paths_field:
            errors.append("frontmatter missing required field: `paths:` (JSON-style list of file globs that trigger auto-load)")
        elif isinstance(paths_field, str):
            # Should look like ["..."]
            stripped = paths_field.strip()
            if not (stripped.startswith("[") and stripped.endswith("]")):
                errors.append(f"`paths:` must be a JSON-style list like [\"regfix.txt\", \"src/*.c\"] (got {paths_field!r})")
            elif "[]" == stripped:
                errors.append("`paths:` is empty — rule would never auto-load")

    return errors


def simulate_edit(file_path: str, old_string: str, new_string: str) -> str | None:
    """Read current file, apply the edit, return resulting content."""
    p = Path(file_path)
    if not p.exists():
        return None
    current = p.read_text(encoding="utf-8", errors="replace")
    if old_string not in current:
        return None
    return current.replace(old_string, new_string, 1)


def simulate_multiedit(file_path: str, edits: list[dict]) -> str | None:
    p = Path(file_path)
    if not p.exists():
        return None
    content = p.read_text(encoding="utf-8", errors="replace")
    for edit in edits:
        old = edit.get("old_string", "")
        new = edit.get("new_string", "")
        if old not in content:
            return None
        content = content.replace(old, new, 1)
    return content


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        # Can't parse — pass through (don't break unrelated tool calls)
        return 0

    tool = payload.get("tool_name", "")
    tool_input = payload.get("tool_input", {})
    file_path = tool_input.get("file_path", "")

    if not file_path:
        return 0

    # Only guard memory/ and .claude/rules/ paths
    path_str = file_path.replace("\\", "/")
    in_memory_dir = any(f"/memory/{d}/" in path_str for d in MEMORY_DIRS)
    in_rules_dir = "/.claude/rules/" in path_str
    if not (in_memory_dir or in_rules_dir):
        return 0

    # Simulate the edit to get the resulting content
    if tool == "Write":
        content = tool_input.get("content", "")
    elif tool == "Edit":
        content = simulate_edit(
            file_path,
            tool_input.get("old_string", ""),
            tool_input.get("new_string", ""),
        )
        if content is None:
            # Can't simulate — pass through
            return 0
    elif tool == "MultiEdit":
        content = simulate_multiedit(file_path, tool_input.get("edits", []))
        if content is None:
            return 0
    else:
        # NotebookEdit or unknown — pass through
        return 0

    errors = validate_content(file_path, content)
    if errors:
        print(f"BLOCKED by memory_write_guard.py: {file_path}", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        print(
            "\nThese checks exist because the indexer fails (and MEMORY.md\n"
            "becomes invalid) when frontmatter is missing/malformed, the\n"
            "filename doesn't match conventions, or metadata.type doesn't\n"
            "match the directory. Fix the content and retry the write.",
            file=sys.stderr,
        )
        return 2

    return 0


if __name__ == "__main__":
    sys.exit(main())
