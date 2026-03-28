#!/usr/bin/env python3
"""
Batch permuter pipeline for Bushido Blade 2.

For each remaining INCLUDE_ASM stub:
1. Runs m2c to generate base C
2. Post-processes M2C_FIELD/M2C_UNK into GCC 2.7.2-compatible types
3. Sets up the permuter directory (target.s, target.o, compile.sh)
4. Runs the permuter with a time limit
5. Reports results and optionally applies matches to source files

Multithreaded: runs multiple permuter instances in parallel.

Usage:
  python3 tools/batch_permuter.py                           # setup all, run permuter
  python3 tools/batch_permuter.py --file src/text1a.c       # single file
  python3 tools/batch_permuter.py --func func_80041430      # single function
  python3 tools/batch_permuter.py --setup-only               # just create permuter dirs
  python3 tools/batch_permuter.py --timeout 120              # 120s per function (default 90)
  python3 tools/batch_permuter.py --threads 4                # 4 parallel permuter runs
  python3 tools/batch_permuter.py --min-score 50             # only permute if m2c score >= 50%
  python3 tools/batch_permuter.py --apply                    # apply score-0 matches to source
"""

import argparse
import csv
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS_DIR = PROJECT_ROOT / "asm" / "funcs"
SRC_DIR = PROJECT_ROOT / "src"
INCLUDE_DIR = PROJECT_ROOT / "include"
PERMUTER_DIR = PROJECT_ROOT / "permuter"
M2C_SCRIPT = PROJECT_ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = PROJECT_ROOT / "include" / "m2c_context.h"
PERMUTER_PY = PROJECT_ROOT / "tools" / "decomp-permuter" / "permuter.py"
PERMUTER_COMPILE = PROJECT_ROOT / "tools" / "permuter_compile.sh"

CC1 = str(PROJECT_ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1")
MASPSX = str(PROJECT_ROOT / "tools" / "maspsx" / "maspsx.py")

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w".split()
CPP_FLAGS = "-Iinclude -undef -Wall -lang-c -fno-builtin".split()
CPP_DEFS = ("-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx "
            "-D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL "
            "-D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER").split()
AS_FLAGS = "-Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0".split()
MASPSX_FLAGS = ("--expand-div --aspsx-version=2.34 "
                "--sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt "
                "--sdata-exclude=sdata_exclude.txt").split()

LOAD_ADDR = 0x80010000
EXE_HEADER = 0x800
ORIGINAL_EXE = PROJECT_ROOT / "disc" / "SLUS_006.63"

# Lock for printing
print_lock = threading.Lock()


def log(msg):
    with print_lock:
        print(msg, flush=True)


def find_remaining_stubs(src_file=None, func_name=None):
    """Find all INCLUDE_ASM stubs, optionally filtered."""
    stubs = []
    if src_file:
        files = [Path(src_file)]
    else:
        files = sorted(SRC_DIR.glob("*.c"))
    for path in files:
        with open(path) as f:
            for line_no, line in enumerate(f, 1):
                m = re.match(r'\s*INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)', line)
                if m:
                    fname = m.group(1)
                    if func_name and fname != func_name:
                        continue
                    stubs.append({
                        "src": str(path),
                        "src_base": os.path.basename(str(path)),
                        "func": fname,
                        "line": line_no,
                    })
    return stubs


def get_func_info_from_asm(asm_path):
    """Extract function address and size from assembly file."""
    with open(asm_path) as f:
        lines = f.readlines()
    addrs = []
    for line in lines:
        m = re.search(r'/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+', line)
        if m:
            addrs.append(int(m.group(1), 16))
    if addrs:
        return addrs[0], addrs[-1] + 4 - addrs[0], len(lines)
    return None, None, 0


def is_handwritten_asm(asm_path):
    """Check if assembly contains handwritten markers."""
    with open(asm_path) as f:
        content = f.read()
    # Check for instructions GCC cannot emit
    markers = [
        r'\badd\b\s+\$',       # add (not addu) - but careful with addiu
        r'\baddi\b\s+\$',      # addi (not addiu)
        r'\bsub\b\s+\$',       # sub (not subu)
        r'\bsyscall\b',        # syscall
        r'\bjalr\s+\$t[0-9]',  # BIOS trampoline
    ]
    for marker in markers:
        # Filter out false positives from comments
        for line in content.split('\n'):
            # Strip comments
            code = re.sub(r'/\*.*?\*/', '', line).strip()
            if re.search(marker, code):
                return True
    return False


def get_original_bytes(addr, size):
    """Read original binary bytes from the PS-X EXE."""
    offset = EXE_HEADER + (addr - LOAD_ADDR)
    with open(ORIGINAL_EXE, "rb") as f:
        f.seek(offset)
        return f.read(size)


def ensure_m2c_context():
    """Generate m2c context header if needed."""
    if not M2C_CONTEXT.exists():
        cmd = ["mipsel-linux-gnu-cpp", "-Iinclude", "-P",
               str(INCLUDE_DIR / "common.h")]
        result = subprocess.run(cmd, capture_output=True, cwd=str(PROJECT_ROOT))
        with open(M2C_CONTEXT, "wb") as f:
            f.write(result.stdout)


def run_m2c(func_name, asm_path):
    """Run m2c decompiler on an assembly file."""
    ensure_m2c_context()
    cmd = [sys.executable, str(M2C_SCRIPT), "-t", "mipsel-gcc-c",
           "--context", str(M2C_CONTEXT), "--valid-syntax", str(asm_path)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                cwd=str(PROJECT_ROOT), timeout=60)
    except subprocess.TimeoutExpired:
        return None, "m2c timeout"
    if result.returncode != 0:
        return None, result.stderr.strip()[:200]
    return result.stdout.strip(), None


def postprocess_m2c(c_code, func_name, src_file):
    """
    Enhanced post-processing of m2c output for permuter compatibility.
    Goes beyond auto_decompile.py's basic cleanup.
    """
    lines = c_code.split("\n")
    out = []

    # Step 1: Replace M2C_UNK with s32 (most common type in this codebase)
    c_code = re.sub(r'\bM2C_UNK\b', 's32', c_code)

    # Step 2: Replace M2C_FIELD(expr, type, offset) with *(type)((s32)(expr) + offset)
    def fix_m2c_field(m):
        expr = m.group(1)
        typ = m.group(2).strip()
        offset = m.group(3).strip()
        return "*(%s)((s32)(%s) + %s)" % (typ, expr, offset)
    c_code = re.sub(r'M2C_FIELD\(([^,]+),\s*([^,]+),\s*([^)]+)\)', fix_m2c_field, c_code)

    # Step 3: Replace M2C_MEMSET with a loop or leave as comment
    c_code = re.sub(r'M2C_MEMSET\([^)]*\)', '/* M2C_MEMSET - TODO */', c_code)

    # Step 4: Fix undeclared sp/saved_reg/subroutine_arg variables
    lines = c_code.split("\n")
    out = []
    declared = set()
    func_body_started = False

    # Pre-scan existing declarations to avoid redeclarations
    for line in lines:
        dm = re.match(r'\s+(?:s32|u32|s16|u16|s8|u8|void\s*\*)\s+(sp[0-9A-Fa-f]+|saved_reg_\w+|subroutine_arg\d+)\b', line)
        if dm:
            declared.add(dm.group(1))

    for line in lines:
        if re.match(r'.*\)\s*\{', line) and not func_body_started:
            func_body_started = True
            out.append(line)
            all_code = "\n".join(lines)
            for vm in re.finditer(r'\b(sp[0-9A-Fa-f]+|saved_reg_\w+|subroutine_arg\d+)\b', all_code):
                var = vm.group(1)
                if var not in declared:
                    declared.add(var)
                    out.append("    s32 %s;" % var)
            continue
        out.append(line)

    result = "\n".join(out)

    # Step 5: Fix undeclared D_XXXX externs
    used_globals = set(re.findall(r'\bD_[0-9A-Fa-f]{8}\b', result))
    declared_globals = set(re.findall(r'extern\s+.*?(D_[0-9A-Fa-f]{8})', result))
    for g in sorted(used_globals - declared_globals):
        result = ("extern s32 %s;\n" % g) + result

    # Step 6: Fix undeclared func_XXXX externs
    used_funcs = set(re.findall(r'\bfunc_[0-9A-Fa-f]{8}\b', result))
    declared_funcs = set(re.findall(r'(?:extern\s+\w+\s+|(?:void|s32|u32|s16|u16)\s+)(func_[0-9A-Fa-f]{8})\s*\(', result))
    for fn in sorted(used_funcs - declared_funcs):
        if fn != func_name:
            result = ("extern s32 %s();\n" % fn) + result

    # Step 7: Fix pointer dereference type errors
    def fix_deref(m):
        inner = m.group(1)
        if re.match(r'\s*\([^)]*\*\)', inner):
            return m.group(0)
        if re.match(r'\s*\w+\s*$', inner):
            return m.group(0)
        return "*(s32 *)(%s)" % inner
    result = re.sub(
        r'\*\(([^;{}]+?(?:[+\-*/<>|&^]|<<|>>)[^;{}]*?)\)',
        fix_deref, result
    )

    # Step 8: Strip function prototypes to K&R style (avoids arg count mismatches)
    # m2c may infer wrong arg counts; K&R func() accepts any args in C89
    result = re.sub(
        r'^((?:extern\s+)?(?:s32|u32|void|s16|u16|s8|u8)\s+\*?\s*func_[0-9A-Fa-f]{8})\s*\([^)]+\)\s*;(.*)',
        r'\1();\2', result, flags=re.MULTILINE)

    # Step 9: Inline type definitions (permuter can't find common.h)
    # Remove any #include "common.h" since permuter preprocesses base.c standalone
    result = re.sub(r'#include\s+"common\.h"\s*\n?', '', result)
    result = re.sub(r'#include\s+"include_asm\.h"\s*\n?', '', result)
    result = re.sub(r'#include\s+"m2c_macros\.h"\s*\n?', '', result)

    type_defs = """typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
#define NULL ((void *)0)
"""
    result = type_defs + result

    return result


def compile_test(c_code, func_name):
    """Quick compile test to see if the C code compiles at all."""
    tmpdir = tempfile.mkdtemp(prefix="bb2_perm_")
    try:
        c_path = os.path.join(tmpdir, func_name + ".c")
        obj_path = os.path.join(tmpdir, func_name + ".o")

        with open(c_path, "w") as f:
            f.write(c_code)

        # cpp -> cc1 -> maspsx -> as
        cpp_r = subprocess.run(
            ["mipsel-linux-gnu-cpp"] + CPP_FLAGS + CPP_DEFS + [c_path],
            capture_output=True, cwd=str(PROJECT_ROOT))
        if cpp_r.returncode != 0:
            return False, "CPP fail"

        cc1_r = subprocess.run(
            [CC1] + CC_FLAGS, input=cpp_r.stdout,
            capture_output=True, cwd=str(PROJECT_ROOT))
        if cc1_r.returncode != 0:
            return False, "CC1 fail"

        mas_r = subprocess.run(
            ["python3", MASPSX] + MASPSX_FLAGS, input=cc1_r.stdout,
            capture_output=True, cwd=str(PROJECT_ROOT))
        if mas_r.returncode != 0:
            return False, "MASPSX fail"

        as_r = subprocess.run(
            ["mipsel-linux-gnu-as"] + AS_FLAGS + ["-o", obj_path],
            input=mas_r.stdout, capture_output=True, cwd=str(PROJECT_ROOT))
        if as_r.returncode != 0:
            return False, "AS fail"

        return True, None
    except Exception as e:
        return False, str(e)[:100]
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def setup_permuter_dir(func_name, src_file, m2c_code):
    """Create permuter directory with target.s, target.o, base.c, compile.sh."""
    work_dir = PERMUTER_DIR / func_name
    work_dir.mkdir(parents=True, exist_ok=True)

    asm_path = ASM_FUNCS_DIR / (func_name + ".s")

    # Create target.s
    with open(asm_path) as f:
        asm_lines = f.readlines()

    target_s = work_dir / "target.s"
    with open(target_s, "w") as f:
        f.write(".set noat\n")
        f.write(".set noreorder\n\n")
        f.write(".text\n\n")
        for line in asm_lines:
            # Convert glabel to regular label, strip endlabel and comments
            line = re.sub(r'^glabel\s+(\w+)', r'\1:', line)
            line = re.sub(r'^endlabel\b.*', '', line)
            line = re.sub(r'/\*.*?\*/\s*', '', line)
            f.write(line)

    # Assemble target.o
    target_o = work_dir / "target.o"
    subprocess.run(
        ["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
         "-no-pad-sections", "-O1", "-G0",
         str(target_s), "-o", str(target_o)],
        capture_output=True, cwd=str(PROJECT_ROOT))

    # Write base.c
    base_c = work_dir / "base.c"
    with open(base_c, "w") as f:
        f.write(m2c_code)

    # Copy compile.sh
    compile_sh = work_dir / "compile.sh"
    shutil.copy2(str(PERMUTER_COMPILE), str(compile_sh))

    # Create settings.toml
    settings = work_dir / "settings.toml"
    with open(settings, "w") as f:
        f.write('compiler = "gcc"\n')

    return work_dir


def run_permuter(func_name, timeout_secs):
    """Run the permuter on a function with a time limit. Returns best score."""
    work_dir = PERMUTER_DIR / func_name
    if not (work_dir / "base.c").exists():
        return None, "no base.c"
    if not (work_dir / "target.o").exists():
        return None, "no target.o"

    cmd = [sys.executable, str(PERMUTER_PY),
           str(work_dir), "-j1", "--stop-on-zero", "--better-only"]

    best_score = None
    try:
        # Use subprocess.run with timeout — simpler and more reliable
        # The permuter writes scores to stderr with \r (carriage returns)
        try:
            result = subprocess.run(
                cmd, capture_output=True, text=True,
                cwd=str(PROJECT_ROOT), timeout=timeout_secs)
            output = result.stdout + "\n" + result.stderr
        except subprocess.TimeoutExpired as e:
            # Collect whatever output we got
            output = ""
            if e.stdout:
                output += e.stdout.decode("utf-8", errors="replace")
            if e.stderr:
                output += e.stderr.decode("utf-8", errors="replace")

        # Parse ALL score mentions (including \r-separated ones)
        # The permuter output uses \r to overwrite lines, so split on both \r and \n
        for chunk in re.split(r'[\r\n]+', output):
            m = re.search(r'score\s*=\s*(\d+)', chunk)
            if m:
                score = int(m.group(1))
                if best_score is None or score < best_score:
                    best_score = score

        # Also parse "base score = N" line
        m = re.search(r'base score\s*=\s*(\d+)', output)
        if m:
            base_score = int(m.group(1))
            if best_score is None:
                best_score = base_score

        if best_score == 0:
            return 0, "MATCH"
        return best_score, None

    except Exception as e:
        return None, str(e)[:200]


def process_one_stub(stub, args):
    """Process a single stub: m2c -> postprocess -> setup -> permuter."""
    func_name = stub["func"]
    src_file = stub["src"]
    src_base = stub["src_base"]
    asm_path = ASM_FUNCS_DIR / (func_name + ".s")

    result = {
        "func": func_name,
        "src": src_base,
        "status": "pending",
        "score": None,
        "handwritten": False,
    }

    if not asm_path.exists():
        result["status"] = "no_asm"
        return result

    # Get function info
    addr, size, num_lines = get_func_info_from_asm(asm_path)
    if addr is None:
        result["status"] = "bad_asm"
        return result
    result["addr"] = addr
    result["size"] = size
    result["instr"] = size // 4

    # Check for handwritten asm
    if is_handwritten_asm(asm_path):
        result["status"] = "handwritten"
        result["handwritten"] = True
        return result

    # Run m2c
    m2c_code, m2c_err = run_m2c(func_name, asm_path)
    if m2c_code is None:
        result["status"] = "m2c_fail"
        result["error"] = m2c_err
        return result

    # Post-process
    processed = postprocess_m2c(m2c_code, func_name, src_file)

    # Test compile
    compiles, compile_err = compile_test(processed, func_name)
    if not compiles:
        # Try raw m2c with just common.h
        raw_with_include = '#include "common.h"\n' + m2c_code
        compiles2, _ = compile_test(raw_with_include, func_name)
        if compiles2:
            processed = raw_with_include
            compiles = True

    if not compiles:
        result["status"] = "compile_fail"
        result["error"] = compile_err
        return result

    # Check m2c score threshold
    if args.min_score > 0:
        # Quick score check without full permuter
        pass  # We'll set up regardless and let the permuter handle it

    # Setup permuter directory
    try:
        setup_permuter_dir(func_name, src_file, processed)
        result["status"] = "setup_ok"
    except Exception as e:
        result["status"] = "setup_fail"
        result["error"] = str(e)[:200]
        return result

    if args.setup_only:
        return result

    # Run permuter
    log("  [PERMUTER] %-30s (%s, %d instr) ..." % (func_name, src_base, result["instr"]))
    best_score, perm_err = run_permuter(func_name, args.timeout)

    if best_score == 0:
        result["status"] = "MATCH"
        result["score"] = 0
        log("  [MATCH!]   %-30s (%s) -- PERFECT MATCH!" % (func_name, src_base))
    elif best_score is not None:
        result["status"] = "permuted"
        result["score"] = best_score
        log("  [DONE]     %-30s (%s) best score = %d" % (func_name, src_base, best_score))
    else:
        result["status"] = "permuter_fail"
        result["error"] = perm_err
        log("  [FAIL]     %-30s (%s) permuter error: %s" % (func_name, src_base, perm_err))

    return result


def apply_match_to_source(func_name, src_file, line_no):
    """Apply a score-0 permuter match to the source file."""
    best_c = PERMUTER_DIR / func_name / "best.c"
    if not best_c.exists():
        return False, "no best.c file"

    with open(best_c) as f:
        best_code = f.read()

    # Extract just the function body from best.c
    # The permuter's best.c contains the full preprocessed file;
    # we need to find the function definition
    func_match = re.search(
        r'(?:^|\n)(\w[\w\s*]*\b' + func_name + r'\s*\([^)]*\)\s*\{)',
        best_code, re.MULTILINE)

    if not func_match:
        return False, "could not find function in best.c"

    # Find the matching closing brace
    start = func_match.start(1)
    depth = 0
    end = start
    for i in range(start, len(best_code)):
        if best_code[i] == '{':
            depth += 1
        elif best_code[i] == '}':
            depth -= 1
            if depth == 0:
                end = i + 1
                break

    func_body = best_code[start:end]

    # Replace the INCLUDE_ASM line in the source file
    with open(src_file) as f:
        content = f.read()

    old = 'INCLUDE_ASM("asm/funcs", %s);' % func_name
    if old not in content:
        return False, "INCLUDE_ASM line not found in source"

    content = content.replace(old, func_body)

    with open(src_file, "w") as f:
        f.write(content)

    return True, None


def main():
    parser = argparse.ArgumentParser(
        description="Batch m2c + permuter pipeline for BB2 decompilation")
    parser.add_argument("--file", help="Only process stubs from this source file")
    parser.add_argument("--func", help="Process a single function")
    parser.add_argument("--setup-only", action="store_true",
                        help="Only set up permuter directories, don't run")
    parser.add_argument("--timeout", type=int, default=90,
                        help="Permuter timeout per function in seconds (default: 90)")
    parser.add_argument("--threads", "-j", type=int, default=4,
                        help="Number of parallel permuter runs (default: 4)")
    parser.add_argument("--min-score", type=int, default=0,
                        help="Only permute functions with m2c score >= this %%")
    parser.add_argument("--apply", action="store_true",
                        help="Apply score-0 matches to source files")
    parser.add_argument("--csv", help="Export results to CSV")
    parser.add_argument("--skip-handwritten", action="store_true",
                        help="Skip handwritten asm functions")
    args = parser.parse_args()

    print("=" * 70)
    print("BB2 Batch Permuter Pipeline")
    print("=" * 70)
    print()

    # Find stubs
    stubs = find_remaining_stubs(args.file, args.func)
    print("Found %d INCLUDE_ASM stubs" % len(stubs))

    if not stubs:
        return

    # Phase 1: Setup (sequential m2c + compile test)
    print("\n--- Phase 1: m2c + compile test + permuter setup ---")
    to_permute = []
    results = []
    handwritten = []

    for i, stub in enumerate(stubs, 1):
        func_name = stub["func"]
        asm_path = ASM_FUNCS_DIR / (func_name + ".s")

        if not asm_path.exists():
            log("  [%d/%d] %-30s no asm file" % (i, len(stubs), func_name))
            results.append({"func": func_name, "src": stub["src_base"], "status": "no_asm"})
            continue

        addr, size, num_lines = get_func_info_from_asm(asm_path)
        if addr is None:
            results.append({"func": func_name, "src": stub["src_base"], "status": "bad_asm"})
            continue

        # Check handwritten
        if is_handwritten_asm(asm_path):
            handwritten.append(stub)
            log("  [%d/%d] %-30s %-15s HANDWRITTEN (%d instr)" % (
                i, len(stubs), func_name, stub["src_base"], size // 4))
            results.append({
                "func": func_name, "src": stub["src_base"],
                "status": "handwritten", "instr": size // 4
            })
            continue

        # Run m2c
        m2c_code, m2c_err = run_m2c(func_name, asm_path)
        if m2c_code is None:
            log("  [%d/%d] %-30s %-15s m2c FAIL: %s" % (
                i, len(stubs), func_name, stub["src_base"], (m2c_err or "")[:40]))
            results.append({
                "func": func_name, "src": stub["src_base"],
                "status": "m2c_fail", "instr": size // 4
            })
            continue

        # Post-process
        processed = postprocess_m2c(m2c_code, func_name, stub["src"])

        # Compile test
        compiles, compile_err = compile_test(processed, func_name)
        if not compiles:
            raw = '#include "common.h"\n' + m2c_code
            compiles2, _ = compile_test(raw, func_name)
            if compiles2:
                processed = raw
                compiles = True

        if not compiles:
            log("  [%d/%d] %-30s %-15s compile FAIL: %s" % (
                i, len(stubs), func_name, stub["src_base"], (compile_err or "")[:30]))
            results.append({
                "func": func_name, "src": stub["src_base"],
                "status": "compile_fail", "instr": size // 4
            })
            continue

        # Setup permuter directory
        try:
            setup_permuter_dir(func_name, stub["src"], processed)
        except Exception as e:
            log("  [%d/%d] %-30s %-15s setup FAIL: %s" % (
                i, len(stubs), func_name, stub["src_base"], str(e)[:30]))
            results.append({
                "func": func_name, "src": stub["src_base"],
                "status": "setup_fail", "instr": size // 4
            })
            continue

        log("  [%d/%d] %-30s %-15s READY (%d instr)" % (
            i, len(stubs), func_name, stub["src_base"], size // 4))

        stub["instr"] = size // 4
        stub["addr"] = addr
        stub["size"] = size
        to_permute.append(stub)

    print("\n--- Phase 1 Summary ---")
    print("  Ready for permuter: %d" % len(to_permute))
    print("  Handwritten asm:    %d" % len(handwritten))
    print("  m2c/compile fail:   %d" % (len(stubs) - len(to_permute) - len(handwritten)))

    if args.setup_only:
        print("\n--setup-only: stopping here. Permuter dirs in: %s" % PERMUTER_DIR)
        return

    if not to_permute:
        print("\nNo functions ready for permuter.")
        return

    # Phase 2: Run permuter in parallel
    print("\n--- Phase 2: Running permuter (%d threads, %ds timeout) ---" % (
        args.threads, args.timeout))

    matches = []

    def permute_one(stub):
        func_name = stub["func"]
        best_score, err = run_permuter(func_name, args.timeout)
        return stub, best_score, err

    with ThreadPoolExecutor(max_workers=args.threads) as pool:
        futures = {pool.submit(permute_one, s): s for s in to_permute}

        for future in as_completed(futures):
            stub, best_score, err = future.result()
            func_name = stub["func"]

            r = {
                "func": func_name,
                "src": stub["src_base"],
                "instr": stub.get("instr", 0),
            }

            if best_score == 0:
                r["status"] = "MATCH"
                r["score"] = 0
                matches.append(stub)
                log("  *** MATCH: %-30s (%s) ***" % (func_name, stub["src_base"]))
            elif best_score is not None:
                r["status"] = "permuted"
                r["score"] = best_score
                log("  %-34s (%s) best = %d" % (func_name, stub["src_base"], best_score))
            else:
                r["status"] = "permuter_fail"
                r["error"] = err

            results.append(r)

    # Phase 3: Summary
    print("\n" + "=" * 70)
    print("RESULTS")
    print("=" * 70)

    match_count = sum(1 for r in results if r.get("status") == "MATCH")
    permuted = [r for r in results if r.get("status") == "permuted"]
    handwritten_count = sum(1 for r in results if r.get("status") == "handwritten")

    print("Total stubs:      %d" % len(stubs))
    print("MATCHES (score 0): %d" % match_count)
    print("Handwritten asm:  %d" % handwritten_count)
    print("Permuted (no match): %d" % len(permuted))

    if permuted:
        by_score = sorted(permuted, key=lambda x: x.get("score", 99999))
        print("\nClosest misses (top 20):")
        for r in by_score[:20]:
            print("  %-30s %-15s score = %s" % (
                r["func"], r["src"], r.get("score", "?")))

    if matches:
        print("\n*** MATCHED FUNCTIONS ***")
        for stub in matches:
            print("  %s (%s)" % (stub["func"], stub["src_base"]))

        if args.apply:
            print("\nApplying matches to source files...")
            for stub in matches:
                ok, err = apply_match_to_source(
                    stub["func"], stub["src"], stub.get("line", 0))
                if ok:
                    print("  Applied %s to %s" % (stub["func"], stub["src_base"]))
                else:
                    print("  FAILED %s: %s" % (stub["func"], err))

    # Export CSV
    if args.csv:
        fields = ["func", "src", "status", "instr", "score", "error"]
        with open(args.csv, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
            w.writeheader()
            for r in sorted(results, key=lambda x: x.get("score") or 99999):
                w.writerow(r)
        print("\nExported to %s" % args.csv)


if __name__ == "__main__":
    main()
