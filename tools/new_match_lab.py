#!/usr/bin/env python3
"""Scaffold a new Codex lab for a target function."""

from __future__ import annotations

import argparse
from pathlib import Path

from func_tooling import (
    CODEX_LAB_DIR,
    extract_function_text_from_source,
    find_split_asm_path,
    get_all_func_info,
    infer_source_file,
)


README_TEMPLATE = """# `{func_name}` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `{source_file}`
- Split asm: `{split_asm}`
- Address: `{addr}`
- Size: `{size:#x}`
- Source state: `{source_state}`

## Lane

- Classification: `unknown`
- Why:
  - fill in whether this is source-shape, compiler-behavior, or post-compile

## Hot Regions

1. Fill in the known stubborn block(s)

## Workflow

1. Confirm the cleanest semantic baseline.
2. Compare isolated and live pre-regfix asm before deep iteration.
3. Escalate to `regfix` only after the source shape is stable.

## Notes

- Add score snapshots, failed experiments, and promotion notes here.
"""


STUB_BASE_TEMPLATE = """/* Stub scaffold for `{func_name}`.
 *
 * This function is still `INCLUDE_ASM` in `{source_file}`.
 * Fill in typedefs, externs, and the function signature before compiling.
 */

/* TODO: recover prototype for {func_name}. */
/* TODO: paste or write a C baseline here. */
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("func_name", help="Function name to scaffold")
    parser.add_argument(
        "--output-root",
        default=str(CODEX_LAB_DIR),
        help="Root directory for the lab scaffold",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    func_name = args.func_name
    source_path = infer_source_file(func_name)
    if source_path is None:
        raise SystemExit(f"Could not infer source file for {func_name}")

    split_path = find_split_asm_path(func_name)
    if split_path is None:
        raise SystemExit(f"Could not find split asm for {func_name}")

    all_funcs = get_all_func_info()
    if func_name not in all_funcs:
        raise SystemExit(f"No asm metadata found for {func_name}")

    lab_root = Path(args.output_root) / func_name
    permuter_dir = lab_root / "permuter"
    permuter_dir.mkdir(parents=True, exist_ok=True)

    source_text = source_path.read_text()
    try:
        func_text = extract_function_text_from_source(source_text, func_name)
        source_state = "decompiled function body found"
    except ValueError:
        func_text = STUB_BASE_TEMPLATE.format(func_name=func_name, source_file=source_path.as_posix())
        source_state = "stub only (no live C body yet)"

    (permuter_dir / "base.c").write_text(func_text, encoding="utf-8")
    (permuter_dir / "compile.sh").write_text(
        "#!/bin/bash\n"
        "set -e\n"
        "echo \"Add a per-function compile command here.\"\n",
        encoding="utf-8",
    )
    (permuter_dir / "settings.toml").write_text("# Fill in permuter settings here.\n", encoding="utf-8")
    (permuter_dir / "hints.txt").write_text("# Add score notes and hot regions here.\n", encoding="utf-8")
    (permuter_dir / "target.s").write_text(split_path.read_text(), encoding="utf-8")

    info = all_funcs[func_name]
    (lab_root / "README.md").write_text(
        README_TEMPLATE.format(
            func_name=func_name,
            source_file=source_path.as_posix(),
            split_asm=split_path.as_posix(),
            addr=f"0x{info.addr:08X}",
            size=info.size,
            source_state=source_state,
        ),
        encoding="utf-8",
    )

    print(lab_root)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
