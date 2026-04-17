#!/usr/bin/env python3
"""Verify one or more decompiled functions against the original binary."""

from __future__ import annotations

import argparse
import os
import sys
import tempfile
from pathlib import Path

from func_tooling import (
    compare_functions_in_object,
    compile_c_file_to_object,
    extract_function_asm,
    get_all_func_info,
    get_obj_functions,
    infer_source_file,
    write_stage_outputs,
)


STAGE_CHOICES = {
    "pre": "maspsx",
    "regfix1": "regfix1",
    "regfix2": "regfix2",
    "asmfix": "asmfix",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Compile a decompiled function through the BB2 pipeline and compare it "
            "against the original binary."
        )
    )
    parser.add_argument(
        "target",
        help="C file path or function name to infer from src/",
    )
    parser.add_argument(
        "func_names",
        nargs="*",
        help="Optional function names when the first argument is a C file",
    )
    parser.add_argument(
        "--stage",
        choices=sorted(STAGE_CHOICES),
        help="Print or write the extracted function asm at the requested pipeline stage",
    )
    parser.add_argument(
        "--stage-only",
        action="store_true",
        help="Only dump the requested stage instead of also comparing bytes",
    )
    parser.add_argument(
        "--write-stage-dir",
        help="Directory to write extracted stage asm files into",
    )
    parser.add_argument(
        "--obj-slice",
        action="store_true",
        help="Compare raw object .text slices instead of linked slices",
    )
    parser.add_argument(
        "--linked-slice",
        action="store_true",
        help="Force linked-slice comparison (default)",
    )
    return parser.parse_args()


def resolve_target(target: str, func_names: list[str]) -> tuple[Path, list[str]]:
    target_path = Path(target)
    if target_path.exists():
        return target_path, func_names

    source_path = infer_source_file(target)
    if source_path is None:
        raise SystemExit(f"Could not infer source file for function {target}")

    requested = [target, *func_names]
    return source_path, requested
def main() -> int:
    args = parse_args()
    c_file, requested_funcs = resolve_target(args.target, args.func_names)
    all_funcs = get_all_func_info()

    with tempfile.NamedTemporaryFile(suffix=".o", delete=False) as tmp:
        obj_path = tmp.name

    try:
        stage_outputs = compile_c_file_to_object(c_file, obj_path)
        obj_funcs = get_obj_functions(obj_path)
        if requested_funcs:
            obj_funcs = [entry for entry in obj_funcs if entry[2] in requested_funcs]

        if not obj_funcs:
            print("No matching functions found")
            return 1

        if args.stage:
            stage_name = STAGE_CHOICES[args.stage]
            asm_text = stage_outputs[stage_name].decode("utf-8")
            extracted = {}
            for _, _, func_name in obj_funcs:
                extracted[func_name] = extract_function_asm(asm_text, func_name)

            if args.write_stage_dir:
                written = []
                for func_name, text in extracted.items():
                    written.extend(write_stage_outputs(args.write_stage_dir, func_name, {stage_name: text}))
                for path in written:
                    print(path)
            else:
                for idx, (_, _, func_name) in enumerate(obj_funcs):
                    if idx:
                        print()
                    print(f"=== {func_name} ({stage_name}) ===")
                    sys.stdout.write(extracted[func_name])

            if args.stage_only:
                return 0

        compare_linked = True
        if args.obj_slice:
            compare_linked = False
        if args.linked_slice:
            compare_linked = True

        if obj_funcs[0][2] not in all_funcs:
            print(f"  {obj_funcs[0][2]}: not found in asm metadata")
            return 1

        results = compare_functions_in_object(
            obj_path,
            requested_funcs=[func_name for _, _, func_name in obj_funcs],
            linked=compare_linked,
            all_funcs=all_funcs,
        )
        ok = True
        for result in results:
            if result.matched:
                print(f"  {result.name}: OK")
            else:
                label = "not found in asm metadata" if result.lines == ("not found in asm metadata",) else f"MISMATCH ({result.diff_words} differing words)"
                print(f"  {result.name}: {label}")
                for line in result.lines:
                    if line == "not found in asm metadata":
                        continue
                    print(f"    {line}")
                ok = False
        return 0 if ok else 1
    finally:
        if os.path.exists(obj_path):
            os.unlink(obj_path)


if __name__ == "__main__":
    raise SystemExit(main())
