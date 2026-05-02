#!/usr/bin/env python3
"""
Score all compilable stubs by comparing m2c-compiled output against target assembly.
Uses objdump instruction comparison (not permuter) for fast, reliable scoring.

Output: updates triage_results.csv with actual scores
Usage: cd project_root && source .venv/bin/activate && python3 tools/score_stubs.py
"""
import subprocess, os, re, sys, csv, tempfile, shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
M2C = ROOT / "tools" / "m2c" / "m2c.py"
CC1 = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w"

TYPEDEFS = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
"""


def find_compilable_stubs():
    """Read triage_results.csv, return stubs where compile_ok=1."""
    csvpath = ROOT / "triage_results.csv"
    stubs = []
    with open(csvpath) as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row["compile_ok"] == "1":
                stubs.append(row)
    return stubs


def run_m2c(func):
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
    code = m2c_output
    code = re.sub(r'\bM2C_UNK\b', 's32', code)
    code = re.sub(
        r'M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)',
        r'*(\2)(((u8*)\1)+\3)',
        code
    )
    return TYPEDEFS + code


def compile_to_obj(c_code, output_path):
    try:
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
        cc1_cmd = [str(CC1)] + CC_FLAGS.split()
        p_cc1 = subprocess.Popen(cc1_cmd, stdin=p_cpp.stdout, stdout=subprocess.PIPE,
                                 stderr=subprocess.DEVNULL, cwd=str(ROOT))
        p_cpp.stdout.close()
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
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    lines = [".set noat\n", ".set noreorder\n", ".section .text\n",
             f".global {func}\n", f"{func}:\n"]
    with open(asm_file) as f:
        for line in f:
            if "glabel" in line or "endlabel" in line:
                continue
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


def get_instructions(obj_path, func_name=None):
    """Extract instruction list from .o file using objdump."""
    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-objdump", "-d", "-M", "no-aliases", str(obj_path)],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode != 0:
            return []

        instructions = []
        in_func = func_name is None  # if no func filter, take all
        for line in result.stdout.splitlines():
            # Function header like "<func_name>:"
            if func_name and f"<{func_name}>:" in line:
                in_func = True
                continue
            if not in_func:
                continue
            # Blank line after function ends it
            if in_func and func_name and line.strip() == "":
                if instructions:
                    break
                continue
            # Parse instruction line: "   0:    00851021    addu    v0,a0,a1"
            m = re.match(r'\s+[0-9a-f]+:\s+[0-9a-f]+\s+(.*)', line)
            if m:
                instr = m.group(1).strip()
                # Normalize: remove trailing comments, collapse whitespace
                instr = re.sub(r'\s+', ' ', instr)
                instructions.append(instr)
        return instructions
    except Exception:
        return []


def score_instructions(target_insns, compiled_insns):
    """Simple diff-based scoring. Lower = better, 0 = perfect match."""
    if not target_insns or not compiled_insns:
        return 99999

    # Exact match check
    if target_insns == compiled_insns:
        return 0

    # Count matching instructions at same positions
    max_len = max(len(target_insns), len(compiled_insns))
    min_len = min(len(target_insns), len(compiled_insns))

    # Size difference penalty
    size_diff = abs(len(target_insns) - len(compiled_insns))

    # Position-by-position comparison (for same-length or truncated)
    mismatches = 0
    for i in range(min_len):
        if i < len(target_insns) and i < len(compiled_insns):
            t = target_insns[i]
            c = compiled_insns[i]
            if t != c:
                # Check if just register difference
                t_op = t.split()[0] if t.split() else ""
                c_op = c.split()[0] if c.split() else ""
                if t_op == c_op:
                    mismatches += 5  # Same opcode, different operands (register diff)
                else:
                    mismatches += 60  # Different opcode (reordering/structural)

    # Extra/missing instructions
    mismatches += size_diff * 100

    return mismatches


def process_one(row, tmpdir):
    func = row["func"]
    d = Path(tmpdir) / func
    d.mkdir(parents=True, exist_ok=True)

    m2c_out = run_m2c(func)
    if not m2c_out:
        return 99999

    c_code = make_compilable(m2c_out)

    # Compile m2c output
    compiled_o = d / "compiled.o"
    if not compile_to_obj(c_code, compiled_o):
        return 99999

    # Build target
    target_o = d / "target.o"
    if not build_target_obj(func, target_o):
        return 99999

    # Get instructions from both
    target_insns = get_instructions(target_o, func)
    compiled_insns = get_instructions(compiled_o)

    return score_instructions(target_insns, compiled_insns)


def main():
    stubs = find_compilable_stubs()
    print(f"Scoring {len(stubs)} compilable stubs...")

    results = []
    for i, row in enumerate(stubs):
        func = row["func"]
        print(f"[{i+1}/{len(stubs)}] {func}...", end=" ", flush=True)

        tmpdir = tempfile.mkdtemp(prefix=f"score_{func}_")
        try:
            score = process_one(row, tmpdir)
            row["score"] = score
            results.append(row)
            if score == 0:
                print(f"PERFECT MATCH!")
            elif score < 99999:
                print(f"score={score}")
            else:
                print(f"FAIL")
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

    # Also include non-compilable stubs from original CSV
    csvpath = ROOT / "triage_results.csv"
    all_rows = []
    with open(csvpath) as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row["compile_ok"] == "0":
                all_rows.append(row)

    all_rows.extend(results)
    # Sort by func name for consistency
    all_rows.sort(key=lambda r: r["func"])

    # Write updated CSV
    with open(csvpath, 'w', newline='') as f:
        w = csv.DictWriter(f, fieldnames=["func", "file", "asm_lines", "m2c_ok", "compile_ok", "score"])
        w.writeheader()
        w.writerows(all_rows)

    # Summary
    print(f"\n{'='*60}")
    total = len(all_rows)
    scored = [r for r in all_rows if int(r["score"]) < 99999]
    zeros = [r for r in scored if int(r["score"]) == 0]
    under50 = [r for r in scored if 0 < int(r["score"]) <= 50]
    under200 = [r for r in scored if 0 < int(r["score"]) <= 200]
    under500 = [r for r in scored if 0 < int(r["score"]) <= 500]
    under1000 = [r for r in scored if 0 < int(r["score"]) <= 1000]

    print(f"Total stubs:     {total}")
    print(f"Scored:          {len(scored)}")
    print(f"Score = 0:       {len(zeros)}  (PERFECT - just paste m2c output!)")
    print(f"Score <= 50:     {len(under50)}  (very close, minor tweaks)")
    print(f"Score <= 200:    {len(under200)}  (close, register/order diffs)")
    print(f"Score <= 500:    {len(under500)}  (moderate work)")
    print(f"Score <= 1000:   {len(under1000)}  (significant work)")

    print(f"\n{'='*60}")
    print("Top 50 candidates (lowest non-zero scores):")
    print(f"{'='*60}")
    scored.sort(key=lambda r: int(r["score"]))
    for r in scored[:50]:
        print(f"  {r['func']:20s}  {r['file']:16s}  {r['asm_lines']:4s} insns  score={r['score']}")

    if zeros:
        print(f"\n{'='*60}")
        print(f"PERFECT MATCHES ({len(zeros)} functions - free wins!):")
        print(f"{'='*60}")
        for r in zeros:
            print(f"  {r['func']:20s}  {r['file']:16s}  {r['asm_lines']:4s} insns")


if __name__ == "__main__":
    main()
