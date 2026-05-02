#!/usr/bin/env python3
"""
Triage all remaining INCLUDE_ASM stubs:
  1. Run m2c to get C starting point
  2. Compile with GCC 2.7.2 pipeline
  3. Score against target assembly using permuter scorer

Output: triage_results.csv
Usage: cd project_root && source .venv/bin/activate && python3 tools/triage_all.py
"""
import subprocess, os, re, sys, csv, tempfile, shutil
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed

ROOT = Path(__file__).resolve().parent.parent
M2C = ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = ROOT / "include" / "m2c_context.h"
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"
PERMUTER = ROOT / "tools" / "decomp-permuter" / "permuter.py"
COMPILE_SH = ROOT / "tools" / "permuter_compile.sh"

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

TYPEDEFS = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

typedef struct GameObj {
    /* 0x00 */ u8 field_00;
    /* 0x01 */ u8 field_01;
    /* 0x02 */ s16 field_02;
    /* 0x04 */ s16 field_04;
    /* 0x06 */ s16 field_06;
    /* 0x08 */ s16 field_08;
    /* 0x0A */ s16 field_0A;
    /* 0x0C */ s16 field_0C;
    /* 0x0E */ s16 field_0E;
    /* 0x10 */ s16 field_10;
    /* 0x12 */ s16 field_12;
    /* 0x14 */ s16 field_14;
    /* 0x16 */ s16 field_16;
    /* 0x18 */ s32 field_18;
    /* 0x1C */ s32 field_1C;
    /* 0x20 */ s32 field_20;
    /* 0x24 */ s32 field_24;
    /* 0x28 */ s32 field_28;
    /* 0x2C */ s32 field_2C;
    /* 0x30 */ s16 field_30;
    /* 0x32 */ s16 field_32;
    /* 0x34 */ s16 field_34;
    /* 0x36 */ s16 field_36;
    /* 0x38 */ s16 field_38;
    /* 0x3A */ s16 field_3A;
    /* 0x3C */ s16 field_3C;
    /* 0x3E */ s16 field_3E;
    /* 0x40 */ s16 field_40;
    /* 0x42 */ s16 field_42;
    /* 0x44 */ s32 field_44;
    /* 0x48 */ s32 field_48;
    /* 0x4C */ s32 field_4C;
    /* 0x50 */ s32 field_50;
    /* 0x54 */ s16 field_54;
    /* 0x56 */ s16 field_56;
    /* 0x58 */ s32 field_58;
    /* 0x5C */ s16 field_5C;
    /* 0x5E */ s16 field_5E;
    /* 0x60 */ s32 field_60;
    /* 0x64 */ s32 field_64;
    /* 0x68 */ s32 field_68;
    /* 0x6C */ s32 field_6C;
    /* 0x70 */ s32 field_70;
    /* 0x74 */ s32 field_74;
    /* 0x78 */ s32 field_78;
    /* 0x7C */ s32 field_7C;
    /* 0x80 */ s32 field_80;
    /* 0x84 */ s16 field_84;
    /* 0x86 */ s16 field_86;
    /* 0x88 */ s16 field_88;
    /* 0x8A */ s16 field_8A;
    /* 0x8C */ s32 field_8C;
    /* 0x90 */ s32 field_90;
    /* 0x94 */ s32 field_94;
    /* 0x98 */ s32 field_98;
    /* 0x9C */ s32 field_9C;
    /* 0xA0 */ s32 field_A0;
    /* 0xA4 */ s32 field_A4;
    /* 0xA8 */ s32 field_A8;
    /* 0xAC */ s32 field_AC;
    /* 0xB0 */ s32 field_B0;
    /* 0xB4 */ s32 field_B4;
    /* 0xB8 */ s32 field_B8;
    /* 0xBC */ s32 field_BC;
    /* 0xC0 */ s32 field_C0;
    /* 0xC4 */ s32 field_C4;
    /* 0xC8 */ s32 field_C8;
    /* 0xCC */ s32 field_CC;
    /* 0xD0 */ s32 field_D0;
    /* 0xD4 */ s32 field_D4;
    /* 0xD8 */ s32 field_D8;
    /* 0xDC */ s32 field_DC;
    /* 0xE0 */ s32 field_E0;
    /* 0xE4 */ s32 field_E4;
    /* 0xE8 */ s32 field_E8;
    /* 0xEC */ s32 field_EC;
    /* 0xF0 */ s32 field_F0;
    /* 0xF4 */ s32 field_F4;
    /* 0xF8 */ s16 field_F8;
    /* 0xFA */ s16 field_FA;
    /* 0xFC */ s32 field_FC;
} GameObj;

typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;

extern s16 D_800973FC[];
"""


def find_all_stubs():
    """Find all INCLUDE_ASM stubs across all source files."""
    stubs = []
    for src in sorted(ROOT.glob("src/*.c")):
        with open(src) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append((m.group(1), src.name))
    return stubs


def count_asm_lines(func):
    """Count instruction lines in assembly."""
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    if not asm_file.exists():
        return 0
    count = 0
    with open(asm_file) as f:
        for line in f:
            if "/*" in line and "*/" in line and ("addiu" in line or "lw" in line or
                "sw" in line or "jal" in line or "jr" in line or "lui" in line or
                "lh" in line or "sh" in line or "sb" in line or "lb" in line or
                "bnez" in line or "beqz" in line or "bne" in line or "beq" in line or
                "nop" in line or "sll" in line or "sra" in line or "addu" in line or
                "subu" in line or "mult" in line or "div" in line or "mfhi" in line or
                "mflo" in line or "andi" in line or "ori" in line or "slt" in line or
                "move" in line or "or " in line or "and " in line or "j " in line or
                "srl" in line or "xor" in line or "nor" in line or "lbu" in line or
                "break" in line or "bgez" in line or "blez" in line or "bgtz" in line or
                "bltz" in line or "srav" in line or "sllv" in line or "srlv" in line or
                "jalr" in line or "lhu" in line or "swl" in line or "swr" in line or
                "lwl" in line or "lwr" in line or "slti" in line or "sltiu" in line):
                count += 1
    return count


def run_m2c(func):
    """Run m2c on a function's assembly, return C code or None."""
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    if not asm_file.exists():
        return None
    try:
        result = subprocess.run(
            [sys.executable, str(M2C), "--valid-syntax", "--target", "mips-gcc-c",
             "--context", str(M2C_CONTEXT), str(asm_file)],
            capture_output=True, text=True, timeout=30
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout
        return None
    except Exception:
        return None


def make_compilable(m2c_output):
    """Wrap m2c output with typedefs and fix M2C_UNK/M2C_FIELD."""
    code = m2c_output
    # Replace M2C_UNK with s32
    code = re.sub(r'\bM2C_UNK\b', 's32', code)
    # Replace M2C_FIELD(ptr, type*, offset) with *(type*)((u8*)ptr + offset)
    code = re.sub(
        r'M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)',
        r'*(\2)(((u8*)\1)+\3)',
        code
    )
    return TYPEDEFS + code


def compile_to_obj(c_code, output_path):
    """Run the full GCC 2.7.2 pipeline, return True on success."""
    try:
        # CPP
        cpp_cmd = [
            "mipsel-linux-gnu-cpp",
            f"-I{ROOT}/include", "-undef", "-Wall", "-lang-c", "-fno-builtin",
            "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips",
            "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__",
            "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C", "-DPERMUTER",
            "-"
        ]
        p_cpp = subprocess.Popen(cpp_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, cwd=str(ROOT))

        # CC1
        cc1_cmd = [str(CC1)] + CC_FLAGS.split()
        p_cc1 = subprocess.Popen(cc1_cmd, stdin=p_cpp.stdout, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cpp.stdout.close()

        # MASPSX
        maspsx_cmd = [
            sys.executable, str(MASPSX),
            "--expand-div", "--aspsx-version=2.34",
            f"--sdata-syms={ROOT}/sdata_syms.txt",
            f"--sdata-funcs={ROOT}/sdata_funcs.txt",
            f"--sdata-exclude={ROOT}/sdata_exclude.txt",
        ]
        p_maspsx = subprocess.Popen(maspsx_cmd, stdin=p_cc1.stdout, stdout=subprocess.PIPE,
                                    stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cc1.stdout.close()

        # AS
        as_cmd = [
            "mipsel-linux-gnu-as",
            f"-I{ROOT}/include", "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0", "-o", str(output_path)
        ]
        p_as = subprocess.Popen(as_cmd, stdin=p_maspsx.stdout, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_maspsx.stdout.close()

        p_cpp.stdin.write(c_code.encode())
        p_cpp.stdin.close()

        p_as.wait(timeout=30)
        # Also check cc1 succeeded (otherwise as gets empty input and "succeeds")
        p_cc1.wait(timeout=5)
        return p_as.returncode == 0 and p_cc1.returncode == 0 and output_path.exists()
    except Exception:
        return False


def build_target_obj(func, output_path):
    """Assemble the target .s into a .o for scoring."""
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    # Convert splat format to GNU as format
    lines = [".set noat\n", ".set noreorder\n", ".section .text\n",
             f".global {func}\n", f"{func}:\n"]
    with open(asm_file) as f:
        for line in f:
            if "glabel" in line or "endlabel" in line:
                continue
            # Extract instruction from comment format
            m = re.search(r'/\*[^*]*\*/\s*(.*)', line)
            if m:
                instr = m.group(1).strip()
                if instr:
                    lines.append(f"  {instr}\n")
            elif line.strip().startswith(".L"):
                lines.append(line)

    target_s = output_path.with_suffix('.s')
    with open(target_s, 'w') as f:
        f.writelines(lines)

    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
             "-no-pad-sections", "-O1", "-G0", "-o", str(output_path), str(target_s)],
            capture_output=True, timeout=10, cwd=str(ROOT)
        )
        return result.returncode == 0
    except Exception:
        return False


def disasm_function(obj_path, func_name):
    """Disassemble a .o file and extract instruction mnemonics."""
    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-objdump", "-d", "-j", ".text", str(obj_path)],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode != 0:
            return []
        instrs = []
        for line in result.stdout.split('\n'):
            # Match disassembly lines: "   0:  27bdffe0    addiu   sp,sp,-32"
            m = re.match(r'\s+[0-9a-f]+:\s+[0-9a-f]+\s+(\w+)', line)
            if m:
                instrs.append(m.group(1))
        return instrs
    except Exception:
        return []


def score_function(func, c_code, tmpdir):
    """Score by comparing disassembly of compiled vs target. Lower = better."""
    work_dir = Path(tmpdir) / func
    work_dir.mkdir(parents=True, exist_ok=True)

    # Build target.o
    target_o = work_dir / "target.o"
    if not build_target_obj(func, target_o):
        return 99999

    # Compile base.c
    compiled_o = work_dir / "compiled.o"
    if not compile_to_obj(c_code, compiled_o):
        return 99999

    # Disassemble both and compare
    target_instrs = disasm_function(target_o, func)
    compiled_instrs = disasm_function(compiled_o, func)

    if not target_instrs or not compiled_instrs:
        return 99999

    # Simple diff score: count mismatched instructions
    # Uses LCS-like approach for instruction alignment
    n, m = len(target_instrs), len(compiled_instrs)
    size_diff = abs(n - m)

    # Count matching instructions (in order)
    matches = 0
    j = 0
    for i in range(n):
        while j < m and compiled_instrs[j] != target_instrs[i]:
            j += 1
        if j < m:
            matches += 1
            j += 1

    # Score: total instructions - 2*matches + size difference
    # (0 = perfect match, higher = worse)
    total = n + m
    score = total - 2 * matches + size_diff * 5
    return score


def process_one(args):
    """Process a single stub. Returns dict with results."""
    func, fname = args
    tmpdir = tempfile.mkdtemp(prefix=f"triage_{func}_")
    try:
        asm_lines = count_asm_lines(func)
        m2c_out = run_m2c(func)
        m2c_ok = 1 if m2c_out else 0
        compile_ok = 0
        score = 99999

        if m2c_out:
            c_code = make_compilable(m2c_out)

            # Quick compile check
            compiled_o = Path(tmpdir) / "quick_test.o"
            if compile_to_obj(c_code, compiled_o):
                compile_ok = 1
                score = score_function(func, c_code, tmpdir)

        return {
            "func": func, "file": fname, "asm_lines": asm_lines,
            "m2c_ok": m2c_ok, "compile_ok": compile_ok, "score": score
        }
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def main():
    stubs = find_all_stubs()
    print(f"Found {len(stubs)} stubs to triage")

    outcsv = ROOT / "triage_results.csv"
    results = []

    for i, (func, fname) in enumerate(stubs):
        print(f"[{i+1}/{len(stubs)}] {func} ({fname})...", end=" ", flush=True)
        r = process_one((func, fname))
        results.append(r)
        if r["score"] < 99999:
            print(f"score={r['score']}")
        else:
            print(f"m2c={'ok' if r['m2c_ok'] else 'FAIL'} compile={'ok' if r['compile_ok'] else 'FAIL'}")

    # Write CSV
    with open(outcsv, 'w', newline='') as f:
        w = csv.DictWriter(f, fieldnames=["func", "file", "asm_lines", "m2c_ok", "compile_ok", "score"])
        w.writeheader()
        w.writerows(results)

    # Summary
    print(f"\n{'='*60}")
    print(f"Results written to {outcsv}")
    print(f"{'='*60}")

    total = len(results)
    m2c_ok = sum(1 for r in results if r["m2c_ok"])
    compiled = sum(1 for r in results if r["compile_ok"])
    scored = sum(1 for r in results if r["score"] < 99999)
    under1000 = sum(1 for r in results if 0 < r["score"] < 1000)
    under500 = sum(1 for r in results if 0 < r["score"] < 500)
    under200 = sum(1 for r in results if 0 < r["score"] < 200)
    zeros = sum(1 for r in results if r["score"] == 0)

    print(f"Total stubs:   {total}")
    print(f"m2c parsed:    {m2c_ok}")
    print(f"Compiled:      {compiled}")
    print(f"Scored:        {scored}")
    print(f"Score = 0:     {zeros}")
    print(f"Score < 200:   {under200}")
    print(f"Score < 500:   {under500}")
    print(f"Score < 1000:  {under1000}")

    print(f"\n{'='*60}")
    print("Top 30 candidates (lowest scores):")
    print(f"{'='*60}")
    scored_results = sorted([r for r in results if r["score"] < 99999], key=lambda r: r["score"])
    for r in scored_results[:30]:
        print(f"  {r['func']:20s}  {r['file']:20s}  {r['asm_lines']:4d} insns  score={r['score']}")


if __name__ == "__main__":
    main()
