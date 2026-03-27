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
            [sys.executable, str(M2C), "--valid-syntax", "--target", "mips-gcc-c", str(asm_file)],
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
        return p_as.returncode == 0 and output_path.exists()
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


def score_function(func, c_code, tmpdir):
    """Set up permuter dir and score. Returns score or 99999."""
    perm_dir = Path(tmpdir) / func
    perm_dir.mkdir(parents=True, exist_ok=True)

    # Write base.c
    base_c = perm_dir / "base.c"
    with open(base_c, 'w') as f:
        f.write(c_code)

    # Build target.o
    target_o = perm_dir / "target.o"
    if not build_target_obj(func, target_o):
        return 99999

    # Compile base.c
    compiled_o = perm_dir / "compiled.o"
    if not compile_to_obj(c_code, compiled_o):
        return 99999

    # Copy compile.sh
    compile_sh = perm_dir / "compile.sh"
    shutil.copy2(COMPILE_SH, compile_sh)
    compile_sh.chmod(0o755)

    # Run permuter in debug mode
    try:
        result = subprocess.run(
            [sys.executable, str(PERMUTER), str(perm_dir), "--debug"],
            capture_output=True, text=True, timeout=60, cwd=str(ROOT)
        )
        m = re.search(r'base score = (\d+)', result.stdout + result.stderr)
        if m:
            return int(m.group(1))
    except Exception:
        pass
    return 99999


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
