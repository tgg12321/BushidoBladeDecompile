#!/usr/bin/env python3
"""Preview regfix index effects for one function before editing config."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

import regfix
from func_tooling import PROJECT_ROOT, compile_stage_outputs, extract_function_asm, infer_source_file


STAGE_CHOICES = {
    "pre": "maspsx",
    "regfix1": "regfix1",
    "regfix2": "regfix2",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target", help="C file path or function name")
    parser.add_argument("func_name", nargs="?", help="Function name when target is a C file")
    parser.add_argument(
        "--config",
        default=str(PROJECT_ROOT / "regfix.txt"),
        help="Regfix config file to preview",
    )
    parser.add_argument(
        "--stage",
        choices=sorted(STAGE_CHOICES),
        default="pre",
        help="Pipeline stage to preview against",
    )
    parser.add_argument(
        "--range",
        dest="index_range",
        help="Optional slot range to print, e.g. 10-24",
    )
    return parser.parse_args()


def resolve_target(target: str, func_name: str | None) -> tuple[Path, str]:
    target_path = Path(target)
    if target_path.exists():
        if not func_name:
            raise SystemExit("Function name is required when target is a C file")
        return target_path, func_name

    inferred = infer_source_file(target)
    if inferred is None:
        raise SystemExit(f"Could not infer source file for {target}")
    return inferred, target


def parse_index_range(value: str | None) -> tuple[int, int] | None:
    if value is None:
        return None
    start_text, end_text = value.split("-", 1)
    start = int(start_text)
    end = int(end_text)
    if end < start:
        raise SystemExit("range end must be >= start")
    return start, end


def parse_instruction_entries(func_asm: str) -> list[dict[str, object]]:
    entries: list[dict[str, object]] = []
    insn_idx = 0
    for raw_line in func_asm.splitlines():
        stripped = raw_line.strip()
        if not regfix.is_instruction(stripped):
            continue
        entries.append(
            {
                "current_idx": insn_idx,
                "source_idx": insn_idx,
                "generated": False,
                "text": raw_line.strip(),
            }
        )
        insn_idx += 1
    return entries


def format_entry(entry: dict[str, object], slot: int) -> str:
    source = "-" if entry["source_idx"] is None else str(entry["source_idx"])
    kind = "gen" if entry["generated"] else "orig"
    return (
        f"slot {slot:>3} | cur {int(entry['current_idx']):>3} | "
        f"src {source:>3} | {kind:<4} | {entry['text']}"
    )


def dump_entries(title: str, entries: list[dict[str, object]], index_range: tuple[int, int] | None) -> None:
    print(title)
    start = 0
    end = len(entries) - 1
    if index_range is not None:
        start = max(start, index_range[0])
        end = min(end, index_range[1])
    for slot in range(start, end + 1):
        print(f"  {format_entry(entries[slot], slot)}")


def apply_preview(entries: list[dict[str, object]], func_config: dict[str, list[object]]) -> tuple[list[dict[str, object]], list[str]]:
    entries = [dict(entry) for entry in entries]
    notes: list[str] = []

    for reg_a, reg_b, start, end in func_config.get("swaps", []):
        changed = 0
        for entry in entries:
            idx = int(entry["current_idx"])
            if start is not None and (idx < start or idx > end):
                continue
            new_text = regfix.swap_registers_in_line(str(entry["text"]), [(reg_a, reg_b, start, end)], idx)
            if new_text != entry["text"]:
                entry["text"] = new_text
                changed += 1
        notes.append(f"swap {reg_a} <-> {reg_b} changed {changed} instruction(s)")

    for subst_idx, pattern, replacement in func_config.get("substs", []):
        changed = 0
        for entry in entries:
            if int(entry["current_idx"]) != subst_idx:
                continue
            new_text = re.sub(pattern, lambda _: replacement, str(entry["text"]), count=1)
            if new_text != entry["text"]:
                entry["text"] = new_text
                changed += 1
        notes.append(f"subst @{subst_idx} changed {changed} instruction(s)")

    for del_idx in sorted(func_config.get("deletes", []), reverse=True):
        target_pos = next((pos for pos, entry in enumerate(entries) if int(entry["current_idx"]) == del_idx), None)
        if target_pos is None:
            notes.append(f"delete @{del_idx} missing")
            continue
        removed = entries.pop(target_pos)
        for entry in entries:
            if int(entry["current_idx"]) > del_idx:
                entry["current_idx"] = int(entry["current_idx"]) - 1
        notes.append(f"delete @{del_idx} removed src {removed['source_idx']} `{removed['text']}`")

    for insert_idx, asm_text in sorted(func_config.get("inserts", []), key=lambda item: item[0], reverse=True):
        target_pos = next((pos for pos, entry in enumerate(entries) if int(entry["current_idx"]) == insert_idx), None)
        if target_pos is None:
            notes.append(f"insert @{insert_idx} missing")
            continue
        new_entry = {
            "current_idx": insert_idx,
            "source_idx": None,
            "generated": True,
            "text": asm_text,
        }
        entries.insert(target_pos, new_entry)
        for pos, entry in enumerate(entries):
            if pos == target_pos:
                continue
            if int(entry["current_idx"]) >= insert_idx:
                entry["current_idx"] = int(entry["current_idx"]) + 1
        notes.append(f"insert @{insert_idx} added `{asm_text}`")

    for insert_idx, asm_text in sorted(func_config.get("insert_afters", []), key=lambda item: item[0], reverse=True):
        target_pos = next((pos for pos, entry in enumerate(entries) if int(entry["current_idx"]) == insert_idx), None)
        if target_pos is None:
            notes.append(f"insert_after @{insert_idx} missing")
            continue
        new_entry = {
            "current_idx": insert_idx + 1,
            "source_idx": None,
            "generated": True,
            "text": asm_text,
        }
        entries.insert(target_pos + 1, new_entry)
        for pos, entry in enumerate(entries):
            if pos == target_pos + 1:
                continue
            if int(entry["current_idx"]) > insert_idx:
                entry["current_idx"] = int(entry["current_idx"]) + 1
        notes.append(f"insert_after @{insert_idx} added `{asm_text}`")

    for reorder_start, reorder_end, new_order in func_config.get("reorders", []):
        expected = set(range(reorder_start, reorder_end + 1))
        if set(new_order) != expected:
            notes.append(f"reorder {reorder_start}-{reorder_end} invalid indices {new_order}")
            continue
        range_entries = [entry for entry in entries if reorder_start <= int(entry["current_idx"]) <= reorder_end]
        if len(range_entries) != len(expected):
            notes.append(
                f"reorder {reorder_start}-{reorder_end} found {len(range_entries)} instruction(s), expected {len(expected)}"
            )
            continue
        idx_to_entry = {int(entry["current_idx"]): entry for entry in range_entries}
        ordered_entries = [idx_to_entry[idx] for idx in new_order]
        first_pos = next(pos for pos, entry in enumerate(entries) if reorder_start <= int(entry["current_idx"]) <= reorder_end)
        last_pos = max(pos for pos, entry in enumerate(entries) if reorder_start <= int(entry["current_idx"]) <= reorder_end)
        entries = entries[:first_pos] + ordered_entries + entries[last_pos + 1 :]
        notes.append(f"reorder {reorder_start}-{reorder_end} -> {','.join(str(idx) for idx in new_order)}")

    return entries, notes


def main() -> int:
    args = parse_args()
    c_file, func_name = resolve_target(args.target, args.func_name)
    index_range = parse_index_range(args.index_range)

    config_path = Path(args.config)
    config = regfix.load_config(config_path)
    func_config = config.get(func_name)
    if not func_config:
        raise SystemExit(f"{func_name} not found in {config_path}")

    stages = compile_stage_outputs(c_file)
    func_asm = extract_function_asm(stages[STAGE_CHOICES[args.stage]].decode("utf-8"), func_name)
    before_entries = parse_instruction_entries(func_asm)
    after_entries, notes = apply_preview(before_entries, func_config)

    print(f"Function: {func_name}")
    print(f"Source: {c_file}")
    print(f"Stage: {args.stage}")
    print(f"Config: {config_path}")
    print(f"Instructions: {len(before_entries)} -> {len(after_entries)}")
    print("Operations:")
    for note in notes:
        print(f"  {note}")
    print()
    dump_entries("Before:", before_entries, index_range)
    print()
    dump_entries("After:", after_entries, index_range)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
