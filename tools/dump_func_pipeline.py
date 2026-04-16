#!/usr/bin/env python3
"""Dump one function's asm across the compile pipeline."""

from __future__ import annotations

import argparse
from pathlib import Path

from func_tooling import (
    compile_stage_outputs,
    diff_text,
    extract_function_asm,
    find_split_asm_path,
    infer_source_file,
    write_stage_outputs,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target", help="C file path or function name")
    parser.add_argument("func_name", nargs="?", help="Function name when target is a C file")
    parser.add_argument(
        "--output-dir",
        help="Write per-stage asm files to this directory instead of stdout",
    )
    parser.add_argument(
        "--target-asm",
        action="store_true",
        help="Also print or write the split target asm when available",
    )
    parser.add_argument(
        "--diff",
        action="store_true",
        help="Print unified diffs from target asm to each pipeline stage",
    )
    return parser.parse_args()


def resolve_target(target: str, func_name: str | None) -> tuple[Path, str]:
    target_path = Path(target)
    if target_path.exists():
        if not func_name:
            raise SystemExit("Function name is required when dumping a C file")
        return target_path, func_name

    inferred = infer_source_file(target)
    if inferred is None:
        raise SystemExit(f"Could not infer source file for {target}")
    return inferred, target


def main() -> int:
    args = parse_args()
    c_file, func_name = resolve_target(args.target, args.func_name)
    stages = compile_stage_outputs(c_file)
    stage_texts = {
        "pre": extract_function_asm(stages["maspsx"].decode("utf-8"), func_name),
        "regfix1": extract_function_asm(stages["regfix1"].decode("utf-8"), func_name),
        "regfix2": extract_function_asm(stages["regfix2"].decode("utf-8"), func_name),
    }

    target_text = None
    target_path = find_split_asm_path(func_name)
    if args.target_asm and target_path is not None:
        target_text = target_path.read_text()

    if args.output_dir:
        written = write_stage_outputs(args.output_dir, func_name, stage_texts)
        for path in written:
            print(path)
        if target_text is not None:
            target_out = Path(args.output_dir) / f"{func_name}.target.s"
            target_out.write_text(target_text, encoding="utf-8")
            print(target_out)
        if args.diff and target_text is not None:
            for stage_name, text in stage_texts.items():
                diff_out = Path(args.output_dir) / f"{func_name}.{stage_name}.diff"
                diff_out.write_text(
                    diff_text(target_text, text, from_name=f"{func_name}.target", to_name=f"{func_name}.{stage_name}"),
                    encoding="utf-8",
                )
                print(diff_out)
        return 0

    for stage_name, text in stage_texts.items():
        print(f"=== {func_name} ({stage_name}) ===")
        print(text, end="" if text.endswith("\n") else "\n")
        print()

    if target_text is not None:
        print(f"=== {func_name} (target) ===")
        print(target_text, end="" if target_text.endswith("\n") else "\n")
        print()

    if args.diff and target_text is not None:
        for stage_name, text in stage_texts.items():
            print(f"=== {func_name} target -> {stage_name} diff ===")
            print(diff_text(target_text, text, from_name=f"{func_name}.target", to_name=f"{func_name}.{stage_name}"), end="")
            print()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
