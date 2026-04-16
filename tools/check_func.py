#!/usr/bin/env python3
"""Verify one or more decompiled functions against the original binary."""

from __future__ import annotations

import argparse
import os
import sys
import tempfile
from pathlib import Path

from func_tooling import (
    compare_word_sequences,
    compile_c_file_to_object,
    extract_function_asm,
    get_all_func_info,
    get_obj_functions,
    get_original_bytes,
    infer_source_file,
    link_object_at_vram_base,
    write_stage_outputs,
)


STAGE_CHOICES = {
    "pre": "maspsx",
    "regfix1": "regfix1",
    "regfix2": "regfix2",
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


def compare_object_slices(obj_path: str, obj_funcs, all_funcs) -> bool:
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp:
        obj_bin = tmp.name

    try:
        from func_tooling import run_command  # local import to keep top-level tidy

        run_command(["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text", obj_path, obj_bin])
        with open(obj_bin, "rb") as handle:
            obj_bytes = handle.read()
    finally:
        if os.path.exists(obj_bin):
            os.unlink(obj_bin)

    ok = True
    for func_off, _, func_name in obj_funcs:
        info = all_funcs.get(func_name)
        if info is None:
            print(f"  {func_name}: SKIP (not in asm metadata)")
            continue
        expected = get_original_bytes(info.addr, info.size)
        actual = obj_bytes[func_off:func_off + info.size]
        matched, lines, diff_words = compare_word_sequences(expected, actual, info.addr)
        if matched:
            print(f"  {func_name}: OK")
        else:
            print(f"  {func_name}: MISMATCH ({diff_words} differing words)")
            for line in lines:
                print(f"    {line}")
            ok = False
    return ok


def compare_linked_slices(obj_path: str, obj_funcs, all_funcs) -> bool:
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp:
        linked_bin = tmp.name

    base_func_off, _, base_func_name = obj_funcs[0]
    if base_func_name not in all_funcs:
        raise KeyError(f"{base_func_name} not found in asm metadata")
    base_addr = all_funcs[base_func_name].addr - base_func_off

    try:
        link_object_at_vram_base(obj_path, base_addr, linked_bin, all_funcs=all_funcs)
        with open(linked_bin, "rb") as handle:
            linked_bytes = handle.read()
    finally:
        if os.path.exists(linked_bin):
            os.unlink(linked_bin)
    ok = True
    for _, _, func_name in obj_funcs:
        info = all_funcs.get(func_name)
        if info is None:
            print(f"  {func_name}: SKIP (not in asm metadata)")
            continue
        expected = get_original_bytes(info.addr, info.size)
        start = info.addr - base_addr
        actual = linked_bytes[start:start + info.size]
        matched, lines, diff_words = compare_word_sequences(expected, actual, info.addr)
        if matched:
            print(f"  {func_name}: OK")
        else:
            print(f"  {func_name}: MISMATCH ({diff_words} differing words)")
            for line in lines:
                print(f"    {line}")
            ok = False
    return ok


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

        ok = compare_linked_slices(obj_path, obj_funcs, all_funcs) if compare_linked else compare_object_slices(obj_path, obj_funcs, all_funcs)
        return 0 if ok else 1
    finally:
        if os.path.exists(obj_path):
            os.unlink(obj_path)


if __name__ == "__main__":
    raise SystemExit(main())
