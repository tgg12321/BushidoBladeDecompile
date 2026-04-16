#!/usr/bin/env python3
"""Per-function asm text post-pass for label/symbol/linkage fixes."""

from __future__ import annotations

import re
import sys
from pathlib import Path


CONFIG_PATH = Path(__file__).resolve().parent.parent / "asmfix.txt"


def unescape_config_text(text: str) -> str:
    return text.replace("\\\\", "\\")


def parse_config(path: Path) -> dict[str, list[tuple[str, ...]]]:
    config: dict[str, list[tuple[str, ...]]] = {}
    if not path.exists():
        return config

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue

        m = re.match(r'(\w+)\s*:\s*rename\s+"([^"]+)"\s+"([^"]+)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("rename", m.group(2), m.group(3)))
            continue

        m = re.match(r'(\w+)\s*:\s*replace_first\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("replace_first", unescape_config_text(m.group(2)), unescape_config_text(m.group(3))))
            continue

        m = re.match(r'(\w+)\s*:\s*insert_before\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("insert_before", unescape_config_text(m.group(2)), unescape_config_text(m.group(3)).replace("\\n", "\n")))
            continue

        m = re.match(r'(\w+)\s*:\s*insert_after\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("insert_after", unescape_config_text(m.group(2)), unescape_config_text(m.group(3)).replace("\\n", "\n")))
            continue

        m = re.match(r'(\w+)\s*:\s*delete_between\s+"([^"]+)"\s+"([^"]+)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("delete_between", unescape_config_text(m.group(2)), unescape_config_text(m.group(3))))
            continue

        print(f"asmfix: WARNING: ignoring malformed line: {raw_line}", file=sys.stderr)

    return config


def apply_ops(func_name: str, text: str, ops: list[tuple[str, ...]]) -> str:
    for op in ops:
        kind = op[0]
        if kind == "rename":
            text = text.replace(op[1], op[2])
            continue

        if kind == "replace_first":
            text, count = re.subn(op[1], op[2], text, count=1, flags=re.MULTILINE)
            if count == 0:
                print(f'asmfix: WARNING: replace_first did not match in {func_name}: {op[1]}', file=sys.stderr)
            continue

        if kind == "insert_before":
            pattern, insert_text = op[1], op[2]
            m = re.search(pattern, text, flags=re.MULTILINE)
            if not m:
                print(f'asmfix: WARNING: insert_before did not match in {func_name}: {pattern}', file=sys.stderr)
                continue
            text = text[:m.start()] + insert_text + ("\n" if insert_text and not insert_text.endswith("\n") else "") + text[m.start():]
            continue

        if kind == "insert_after":
            pattern, insert_text = op[1], op[2]
            m = re.search(pattern, text, flags=re.MULTILINE)
            if not m:
                print(f'asmfix: WARNING: insert_after did not match in {func_name}: {pattern}', file=sys.stderr)
                continue
            end = m.end()
            text = text[:end] + ("\n" if not text[:end].endswith("\n") else "") + insert_text + ("\n" if insert_text and not insert_text.endswith("\n") else "") + text[end:]
            continue

        if kind == "delete_between":
            start_pat, end_pat = op[1], op[2]
            start = re.search(start_pat, text, flags=re.MULTILINE)
            if not start:
                print(f'asmfix: WARNING: delete_between start did not match in {func_name}: {start_pat}', file=sys.stderr)
                continue
            end = re.search(end_pat, text[start.end():], flags=re.MULTILINE)
            if not end:
                print(f'asmfix: WARNING: delete_between end did not match in {func_name}: {end_pat}', file=sys.stderr)
                continue
            end_start = start.end() + end.start()
            text = text[:start.start()] + text[end_start:]
            continue

    return text


def process_asm_text(asm_text: str, config: dict[str, list[tuple[str, ...]]]) -> str:
    if not config:
        return asm_text

    lines = asm_text.splitlines(keepends=True)
    out: list[str] = []
    idx = 0
    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()
        if stripped.endswith(":"):
            func_name = stripped[:-1]
            if func_name in config:
                block = [line]
                idx += 1
                while idx < len(lines):
                    block.append(lines[idx])
                    if lines[idx].strip() == f".end\t{func_name}":
                        idx += 1
                        break
                    idx += 1
                out.append(apply_ops(func_name, "".join(block), config[func_name]))
                continue
        out.append(line)
        idx += 1
    return "".join(out)


def main() -> int:
    config = parse_config(CONFIG_PATH)
    asm_text = sys.stdin.read()
    sys.stdout.write(process_asm_text(asm_text, config))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
