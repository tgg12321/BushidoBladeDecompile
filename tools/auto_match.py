#!/usr/bin/env python3
"""
Auto-match: try to automatically match m2c output by:
1. Permuting variable declaration order
2. Adding register asm("reg") hints based on target assembly
3. Trying volatile/cast variations

Runs on all compilable stubs from triage_results.csv.
Usage: cd project_root && source .venv/bin/activate && python3 tools/auto_match.py
"""
import subprocess, os, re, sys, csv, tempfile, shutil, itertools
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

# Map MIPS register names to GCC register names
REG_NAMES = {
    "v0": "v0", "v1": "v1",
    "a0": "a0", "a1": "a1", "a2": "a2", "a3": "a3",
    "t0": "t0", "t1": "t1", "t2": "t2", "t3": "t3",
    "t4": "t4", "t5": "t5", "t6": "t6", "t7": "t7",
    "s0": "s0", "s1": "s1", "s2": "s2", "s3": "s3",
    "s4": "s4", "s5": "s5", "s6": "s6", "s7": "s7",
    "t8": "t8", "t9": "t9",
}


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
                                 stderr=subprocess.PIPE, cwd=str(ROOT))
        cc1_cmd = [str(CC1)] + CC_FLAGS.split()
        p_cc1 = subprocess.Popen(cc1_cmd, stdin=p_cpp.stdout, stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE, cwd=str(ROOT))
        p_cpp.stdout.close()

        prologue_fix = ROOT / "tools" / "prologue_fix.py"
        prologue_cmd = [sys.executable, str(prologue_fix)]
        p_prologue = subprocess.Popen(prologue_cmd, stdin=p_cc1.stdout, stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE, cwd=str(ROOT))
        p_cc1.stdout.close()

        maspsx_cmd = [
            sys.executable, str(MASPSX),
            "--expand-div", "--aspsx-version=2.34",
            f"--sdata-syms={ROOT}/sdata_syms.txt",
            f"--sdata-funcs={ROOT}/sdata_funcs.txt",
            f"--sdata-exclude={ROOT}/sdata_exclude.txt",
        ]
        p_maspsx = subprocess.Popen(maspsx_cmd, stdin=p_prologue.stdout, stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE, cwd=str(ROOT))
        p_prologue.stdout.close()
        as_cmd = [
            "mipsel-linux-gnu-as",
            f"-I{ROOT}/include", "-march=r3000", "-mtune=r3000",
            "-no-pad-sections", "-O1", "-G0", "-o", str(output_path)
        ]
        p_as = subprocess.Popen(as_cmd, stdin=p_maspsx.stdout, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, cwd=str(ROOT))
        p_maspsx.stdout.close()
        p_cpp.stdin.write(c_code.encode())
        p_cpp.stdin.close()

        # Wait for all stages and check return codes
        p_as.wait(timeout=30)
        p_maspsx.wait(timeout=5)
        p_prologue.wait(timeout=5)
        p_cc1.wait(timeout=5)
        p_cpp.wait(timeout=5)

        if p_cpp.returncode != 0 or p_cc1.returncode != 0 or p_prologue.returncode != 0:
            return False
        if p_maspsx.returncode != 0 or p_as.returncode != 0:
            return False
        if not output_path.exists() or output_path.stat().st_size == 0:
            return False
        return True
    except Exception:
        return False


def build_target_obj(func, output_path):
    asm_file = ROOT / "asm" / "funcs" / f"{func}.s"
    lines = [".set noat\n", ".set noreorder\n", ".section .text\n",
             f".global {func}\n", f"{func}:\n"]
    with open(asm_file) as f:
        for line in f:
            stripped = line.strip()
            if "glabel" in stripped or "endlabel" in stripped:
                continue
            m = re.search(r'/\*[^*]*\*/\s*(.*)', line)
            if m:
                instr = m.group(1).strip()
                if instr:
                    lines.append(f"  {instr}\n")
            elif stripped.startswith(".L"):
                lines.append(line)
            elif stripped.startswith("."):
                # Other directives (like .word for jump tables)
                lines.append(f"  {stripped}\n")

    target_s = output_path.with_suffix('.s')
    with open(target_s, 'w') as f:
        f.writelines(lines)

    # Fix GTE/COP2 instructions before assembly
    fix_gte = ROOT / "tools" / "fix_gte_asm.py"
    if fix_gte.exists():
        try:
            subprocess.run([sys.executable, str(fix_gte), str(target_s)],
                          capture_output=True, timeout=10, cwd=str(ROOT))
        except Exception:
            pass  # Best-effort; assembly may still work without it

    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
             "-no-pad-sections", "-O1", "-G0", "-o", str(output_path), str(target_s)],
            capture_output=True, timeout=10, cwd=str(ROOT)
        )
        if result.returncode != 0:
            return False
        # Validate .text section is non-empty
        r2 = subprocess.run(
            ["mipsel-linux-gnu-objdump", "-h", str(output_path)],
            capture_output=True, text=True, timeout=5)
        m = re.search(r'\.text\s+(\w+)', r2.stdout)
        if not m or m.group(1) == "00000000":
            return False
        return True
    except Exception:
        return False


def get_obj_bytes(obj_path):
    """Extract raw .text section bytes from object file."""
    try:
        result = subprocess.run(
            ["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text",
             str(obj_path), "/dev/stdout"],
            capture_output=True, timeout=10
        )
        if result.returncode == 0:
            return result.stdout
        return None
    except Exception:
        return None


def exact_match(compiled_o, target_o):
    """Check if two .o files produce identical .text sections."""
    compiled_bytes = get_obj_bytes(compiled_o)
    target_bytes = get_obj_bytes(target_o)
    if compiled_bytes is None or target_bytes is None:
        return False
    return compiled_bytes == target_bytes


def parse_declarations(m2c_code):
    """Extract variable declarations from m2c output."""
    # Find the function body
    m = re.search(r'\{(.*)\}', m2c_code, re.DOTALL)
    if not m:
        return [], "", ""

    body = m.group(1)
    # Split into declarations and statements
    lines = body.strip().split('\n')
    decls = []
    stmts = []
    in_decls = True
    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue
        # Declaration line: type varname; or type varname = expr;
        if in_decls and re.match(r'\s*(s8|u8|s16|u16|s32|u32|void|s8\s*\*|u8\s*\*|s16\s*\*|u16\s*\*|s32\s*\*|u32\s*\*|void\s*\*)\s+\w+', stripped):
            decls.append(line)
        else:
            in_decls = False
            stmts.append(line)

    # Get function signature
    sig_m = re.match(r'(.*?\{)', m2c_code, re.DOTALL)
    sig = sig_m.group(1) if sig_m else ""

    return decls, stmts, sig


def rebuild_function(sig, decls, stmts):
    """Rebuild function from signature, declarations, and statements."""
    body_lines = decls + [""] + stmts
    return sig + "\n" + "\n".join(body_lines) + "\n}\n"


def try_variant(func, c_code, tmpdir, target_o):
    """Compile a C variant and check for exact match."""
    compiled_o = Path(tmpdir) / "test.o"
    if compiled_o.exists():
        compiled_o.unlink()
    if not compile_to_obj(c_code, compiled_o):
        return False
    return exact_match(compiled_o, target_o)


def try_declaration_permutations(func, m2c_code, externs, tmpdir, target_o, max_perms=120):
    """Try all permutations of variable declaration order."""
    decls, stmts, sig = parse_declarations(m2c_code)
    if len(decls) <= 1 or len(decls) > 8:
        return None  # Too few or too many to permute

    n_perms = 1
    for i in range(1, len(decls) + 1):
        n_perms *= i
    if n_perms > max_perms:
        # Too many — try random subset
        import random
        indices = list(range(len(decls)))
        perms_to_try = set()
        perms_to_try.add(tuple(indices))  # Always try original
        while len(perms_to_try) < max_perms:
            p = list(indices)
            random.shuffle(p)
            perms_to_try.add(tuple(p))
        perm_list = [list(p) for p in perms_to_try]
    else:
        perm_list = list(itertools.permutations(range(len(decls))))

    for perm in perm_list:
        reordered_decls = [decls[i] for i in perm]
        func_code = rebuild_function(sig, reordered_decls, stmts)
        full_code = TYPEDEFS + externs + func_code
        if try_variant(func, full_code, tmpdir, target_o):
            return full_code

    return None


def extract_externs(m2c_code):
    """Extract extern declarations and function signatures from m2c output."""
    lines = m2c_code.strip().split('\n')
    externs = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("extern ") or (stripped.endswith(";") and "(" in stripped and not stripped.startswith("{")):
            externs.append(line)
    return "\n".join(externs) + "\n"


def extract_func_only(m2c_code):
    """Extract just the function definition (no externs/forward decls)."""
    # Find the function definition start
    m = re.search(r'^(\w[\w\s\*]+\s+func_\w+\s*\([^)]*\)\s*\{)', m2c_code, re.MULTILINE)
    if not m:
        return m2c_code
    start = m.start()
    # Find matching closing brace
    depth = 0
    for i in range(start, len(m2c_code)):
        if m2c_code[i] == '{':
            depth += 1
        elif m2c_code[i] == '}':
            depth -= 1
            if depth == 0:
                return m2c_code[start:i+1]
    return m2c_code[start:]


def process_function(func, tmpdir):
    """Try to auto-match a single function. Returns matching C code or None."""
    # Get m2c output
    m2c_out = run_m2c(func)
    if not m2c_out:
        return None

    c_code = make_compilable(m2c_out)
    externs = extract_externs(m2c_out)
    func_only = extract_func_only(c_code)

    # Build target .o
    target_o = Path(tmpdir) / "target.o"
    if not build_target_obj(func, target_o):
        return None

    # Try 1: Direct m2c output (already know func_8002EECC matched this way)
    if try_variant(func, c_code, tmpdir, target_o):
        return c_code

    # Try 2: Declaration order permutations
    result = try_declaration_permutations(func, func_only, externs, tmpdir, target_o)
    if result:
        return result

    return None


def find_candidates():
    """Get functions to try, sorted by score (lowest first)."""
    csvpath = ROOT / "triage_results.csv"
    candidates = []
    with open(csvpath) as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row["compile_ok"] == "1":
                candidates.append(row)
    # Sort by score, then by asm_lines (smaller first)
    candidates.sort(key=lambda r: (int(r["score"]), int(r["asm_lines"])))
    return candidates


def main():
    candidates = find_candidates()
    print(f"Auto-matching {len(candidates)} compilable stubs...")
    print(f"Strategy: declaration order permutation + direct m2c check")
    print()

    matches = []
    for i, row in enumerate(candidates):
        func = row["func"]
        score = int(row["score"])
        asm_lines = int(row["asm_lines"])

        # Skip very large functions (too many declarations to permute)
        if asm_lines > 200:
            continue

        print(f"[{i+1}/{len(candidates)}] {func} ({row['file']}, {asm_lines} insns, score={score})...", end=" ", flush=True)

        tmpdir = tempfile.mkdtemp(prefix=f"automatch_{func}_")
        try:
            result = process_function(func, tmpdir)
            if result:
                print("MATCH!")
                matches.append((func, row["file"], asm_lines, result))
                # Save the matching code
                match_dir = ROOT / "auto_matches"
                match_dir.mkdir(exist_ok=True)
                with open(match_dir / f"{func}.c", 'w') as f:
                    f.write(result)
            else:
                print("no match")
        except Exception as e:
            print(f"error: {e}")
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

    # Summary
    print(f"\n{'='*60}")
    print(f"RESULTS: {len(matches)} functions matched!")
    print(f"{'='*60}")
    for func, fname, insns, _ in matches:
        print(f"  {func:20s}  {fname:16s}  {insns:4d} insns")

    if matches:
        print(f"\nMatching code saved to {ROOT / 'auto_matches'}/")
        print("Review and paste into source files to decompile.")


if __name__ == "__main__":
    main()
