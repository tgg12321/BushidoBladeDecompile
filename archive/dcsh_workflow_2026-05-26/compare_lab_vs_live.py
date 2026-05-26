#!/usr/bin/env python3
"""Compare isolated lab asm against the live TU for one function."""

from __future__ import annotations

import argparse
from pathlib import Path

from func_tooling import compile_stage_outputs, diff_text, extract_function_asm


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("live_c", help="Live C file")
    parser.add_argument("lab_c", help="Lab C file")
    parser.add_argument("func_name", help="Function name to compare")
    parser.add_argument(
        "--stage",
        choices=("pre", "regfix1", "regfix2"),
        default="pre",
        help="Pipeline stage to compare",
    )
    parser.add_argument(
        "--output",
        help="Optional file to write the unified diff into",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    stage_key = {"pre": "maspsx", "regfix1": "regfix1", "regfix2": "regfix2"}[args.stage]
    live_stage = compile_stage_outputs(args.live_c)[stage_key].decode("utf-8")
    lab_stage = compile_stage_outputs(args.lab_c)[stage_key].decode("utf-8")

    live_func = extract_function_asm(live_stage, args.func_name)
    lab_func = extract_function_asm(lab_stage, args.func_name)
    diff = diff_text(lab_func, live_func, from_name=f"{args.func_name}.lab", to_name=f"{args.func_name}.live")

    if args.output:
        out_path = Path(args.output)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(diff, encoding="utf-8")
        print(args.output)
    else:
        print(diff, end="")

    return 0 if not diff else 1


if __name__ == "__main__":
    raise SystemExit(main())
