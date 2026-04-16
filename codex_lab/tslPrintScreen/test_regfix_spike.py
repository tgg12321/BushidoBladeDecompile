#!/usr/bin/env python3
"""Compile the lab tslPrintScreen source with a lab-only regfix config."""

from __future__ import annotations

import argparse
import struct
import subprocess
import sys
import tempfile
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parents[1]

sys.path.append(str(ROOT / "tools"))
import regfix  # type: ignore  # noqa: E402


CPP = "mipsel-linux-gnu-cpp"
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
AS = "mipsel-linux-gnu-as"
OBJCOPY = "mipsel-linux-gnu-objcopy"
PROLOGUE_FIX = ROOT / "tools" / "prologue_fix.py"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"


def run(cmd, *, input_text=None, input_bytes=None):
    result = subprocess.run(
        cmd,
        input=input_text.encode() if input_text is not None else input_bytes,
        capture_output=True,
        cwd=ROOT,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed: {' '.join(str(x) for x in cmd)}\n"
            f"stdout:\n{result.stdout.decode(errors='ignore')}\n"
            f"stderr:\n{result.stderr.decode(errors='ignore')}"
        )
    return result


def compile_to_maspsx(source: Path) -> str:
    with tempfile.NamedTemporaryFile(suffix=".c", delete=False) as tmp_src:
        tmp_path = Path(tmp_src.name)
    try:
        tmp_path.write_text(source.read_text())
        cpp = run(
            [
                CPP,
                "-Iinclude",
                "-undef",
                "-Wall",
                "-lang-c",
                "-fno-builtin",
                "-Dmips",
                "-D__GNUC__=2",
                "-D__OPTIMIZE__",
                "-D__mips__",
                "-D__mips",
                "-Dpsx",
                "-D__psx__",
                "-D__psx",
                "-D_PSYQ",
                "-D__EXTENSIONS__",
                "-D_MIPSEL",
                "-D_LANGUAGE_C",
                "-DLANGUAGE_C",
                "-DPERMUTER",
                str(tmp_path),
            ]
        )
        cc1 = run(
            [
                str(CC1),
                "-O2",
                "-G0",
                "-funsigned-char",
                "-quiet",
                "-mcpu=3000",
                "-mips1",
                "-mno-abicalls",
                "-fno-builtin",
                "-w",
            ],
            input_bytes=cpp.stdout,
        )
        fixed = run(["python3", str(PROLOGUE_FIX)], input_bytes=cc1.stdout)
        maspsx = run(
            [
                "python3",
                str(MASPSX),
                "--expand-div",
                "--aspsx-version=2.34",
                f"--sdata-syms={ROOT / 'sdata_syms.txt'}",
                f"--sdata-funcs={ROOT / 'sdata_funcs.txt'}",
                f"--sdata-exclude={ROOT / 'sdata_exclude.txt'}",
            ],
            input_bytes=fixed.stdout,
        )
        return maspsx.stdout.decode()
    finally:
        if tmp_path.exists():
            tmp_path.unlink()


def apply_custom_regfix(asm_text: str, config_path: Path) -> str:
    config = regfix.load_config(config_path)
    current_func = None
    current_config = None
    func_lines = []
    insn_idx = 0
    buffering = False
    output = []

    for line in asm_text.splitlines(keepends=True):
        stripped = line.strip()
        label_match = regfix.re.match(r"^(\w+):$", stripped)
        if label_match and not buffering:
            func_name = label_match.group(1)
            if func_name in config:
                current_func = func_name
                current_config = config[func_name]
                func_lines = [(line, None)]
                insn_idx = 0
                buffering = True
                continue

        if buffering:
            end_match = regfix.re.match(r"^\s*\.end\s+(\w+)", stripped)
            if regfix.is_instruction(stripped):
                func_lines.append((line, insn_idx))
                insn_idx += 1
            else:
                func_lines.append((line, None))

            if end_match and end_match.group(1) == current_func:
                processed = regfix.process_function(func_lines, current_config)
                output.extend(text for text, _ in processed)
                buffering = False
                current_func = None
                current_config = None
                func_lines = []
        else:
            output.append(line)

    if buffering:
        output.extend(text for text, _ in func_lines)

    return "".join(output)


def assemble(asm_text: str, out_obj: Path):
    run(
        [
            AS,
            "-Iinclude",
            "-march=r3000",
            "-mtune=r3000",
            "-no-pad-sections",
            "-O1",
            "-G0",
            "-o",
            str(out_obj),
        ],
        input_text=asm_text,
    )


def text_bytes(obj_path: Path) -> bytes:
    with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as tmp:
        tmp_path = Path(tmp.name)
    try:
        run([OBJCOPY, "-O", "binary", "-j", ".text", str(obj_path), str(tmp_path)])
        return tmp_path.read_bytes()
    finally:
        if tmp_path.exists():
            tmp_path.unlink()


def compare_text(expected: bytes, actual: bytes):
    word_count = min(len(expected), len(actual)) // 4
    diffs = []
    for i in range(word_count):
        exp_word = struct.unpack_from("<I", expected, i * 4)[0]
        act_word = struct.unpack_from("<I", actual, i * 4)[0]
        if exp_word != act_word:
            diffs.append((i, exp_word, act_word))
    return diffs


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source",
        default=str(SCRIPT_DIR / "permuter" / "base.c"),
    )
    parser.add_argument(
        "--config",
        default=str(SCRIPT_DIR / "regfix_spike.txt"),
    )
    parser.add_argument(
        "--config2",
        default="",
    )
    parser.add_argument(
        "--target",
        default=str(SCRIPT_DIR / "permuter" / "target.o"),
    )
    parser.add_argument(
        "--save-asm",
        default=str(SCRIPT_DIR / "regfix_spike_output.s"),
    )
    args = parser.parse_args()

    source = Path(args.source)
    config = Path(args.config)
    config2 = Path(args.config2) if args.config2 else None
    target = Path(args.target)
    save_asm = Path(args.save_asm)

    maspsx_asm = compile_to_maspsx(source)
    fixed_asm = apply_custom_regfix(maspsx_asm, config)
    if config2 is not None and config2.exists():
        fixed_asm = apply_custom_regfix(fixed_asm, config2)
    save_asm.write_text(fixed_asm)

    with tempfile.NamedTemporaryFile(suffix=".o", delete=False) as tmp_obj:
        obj_path = Path(tmp_obj.name)
    try:
        assemble(fixed_asm, obj_path)
        expected = text_bytes(target)
        actual = text_bytes(obj_path)
        diffs = compare_text(expected, actual)
        print(f"target bytes:  {len(expected)}")
        print(f"actual bytes:  {len(actual)}")
        print(f"word diffs:    {len(diffs)}")
        if diffs:
            print("first diffs:")
            for idx, exp_word, act_word in diffs[:12]:
                print(
                    f"  [{idx:03d}] expected 0x{exp_word:08X}, got 0x{act_word:08X}"
                )
        else:
            print("match")
        print(f"saved asm:     {save_asm}")
    finally:
        if obj_path.exists():
            obj_path.unlink()


if __name__ == "__main__":
    main()
