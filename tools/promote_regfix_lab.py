#!/usr/bin/env python3
"""Translate lab regfix configs onto the live TU for one function."""

from __future__ import annotations

import argparse
from pathlib import Path

import regfix
from func_tooling import (
    build_index_to_label_map,
    build_label_to_index_map,
    compile_stage_outputs,
    extract_function_asm,
    remap_label_tokens,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--func", required=True, help="Function name")
    parser.add_argument("--live", required=True, help="Live C file")
    parser.add_argument("--lab", required=True, help="Lab C file")
    parser.add_argument("--stage1", required=True, help="Lab stage-1 regfix config")
    parser.add_argument("--stage2", help="Optional lab stage-2 regfix config")
    parser.add_argument("--out-stage1", required=True, help="Output path for translated stage-1 config")
    parser.add_argument("--out-stage2", help="Output path for translated stage-2 config")
    return parser.parse_args()


def translate_config_lines(lines, src_label_to_idx, dst_idx_to_label):
    translated = []
    for line in lines:
        translated.append(remap_label_tokens(line, src_label_to_idx, dst_idx_to_label))
    return translated


def main() -> int:
    args = parse_args()
    func_name = args.func

    live_stages = compile_stage_outputs(args.live)
    lab_stages = compile_stage_outputs(args.lab)
    live_pre = extract_function_asm(live_stages["maspsx"].decode("utf-8"), func_name)
    lab_pre = extract_function_asm(lab_stages["maspsx"].decode("utf-8"), func_name)

    stage1_config = regfix.load_config(Path(args.stage1))
    stage1_lines = [
        line for line in Path(args.stage1).read_text().splitlines()
        if line.strip() and not line.strip().startswith("#") and line.split(":", 1)[0].strip() == func_name
    ]
    stage1_translated = translate_config_lines(
        stage1_lines,
        build_label_to_index_map(lab_pre),
        build_index_to_label_map(live_pre),
    )
    out_stage1 = Path(args.out_stage1)
    out_stage1.parent.mkdir(parents=True, exist_ok=True)
    out_stage1.write_text("\n".join(stage1_translated) + ("\n" if stage1_translated else ""), encoding="utf-8")

    if args.stage2:
        if not args.out_stage2:
            raise SystemExit("--out-stage2 is required when --stage2 is provided")

        lab_stage1_text = regfix.process_asm_text(lab_stages["maspsx"].decode("utf-8"), stage1_config)
        live_stage1_config = regfix.load_config(Path(args.out_stage1))
        live_stage1_text = regfix.process_asm_text(live_stages["maspsx"].decode("utf-8"), live_stage1_config)

        lab_stage1_func = extract_function_asm(lab_stage1_text, func_name)
        live_stage1_func = extract_function_asm(live_stage1_text, func_name)
        stage2_lines = [
            line for line in Path(args.stage2).read_text().splitlines()
            if line.strip() and not line.strip().startswith("#") and line.split(":", 1)[0].strip() == func_name
        ]
        stage2_translated = translate_config_lines(
            stage2_lines,
            build_label_to_index_map(lab_stage1_func),
            build_index_to_label_map(live_stage1_func),
        )
        out_stage2 = Path(args.out_stage2)
        out_stage2.parent.mkdir(parents=True, exist_ok=True)
        out_stage2.write_text("\n".join(stage2_translated) + ("\n" if stage2_translated else ""), encoding="utf-8")

    print(args.out_stage1)
    if args.out_stage2:
        print(args.out_stage2)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
