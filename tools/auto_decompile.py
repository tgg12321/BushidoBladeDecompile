#!/usr/bin/env python3
"""
Automated decompilation pipeline for Bushido Blade 2.

Scans remaining INCLUDE_ASM stubs, runs m2c to generate initial C,
optionally compiles and diffs against the original binary.

Usage:
  python3 tools/auto_decompile.py                     # scan all stubs, run m2c
  python3 tools/auto_decompile.py --compile            # also compile and diff
  python3 tools/auto_decompile.py --file src/gpu.c     # only stubs in one file
  python3 tools/auto_decompile.py --func func_800167AC # single function
  python3 tools/auto_decompile.py --output-dir m2c_out # save m2c output
  python3 tools/auto_decompile.py --top 20             # top 20 closest matches
  python3 tools/auto_decompile.py --csv report.csv     # export CSV
"""

import argparse
import csv
import os
import re
import struct
import subprocess
import sys
import tempfile
import shutil
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS_DIR = PROJECT_ROOT / "asm" / "funcs"
SRC_DIR = PROJECT_ROOT / "src"
INCLUDE_DIR = PROJECT_ROOT / "include"
M2C_SCRIPT = PROJECT_ROOT / "tools" / "m2c" / "m2c.py"
ORIGINAL_EXE = PROJECT_ROOT / "disc" / "SLUS_006.63"
M2C_CONTEXT = PROJECT_ROOT / "include" / "m2c_context.h"

CC1 = str(PROJECT_ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1")
MASPSX = str(PROJECT_ROOT / "tools" / "maspsx" / "maspsx.py")
UNDEF_FUNCS = str(PROJECT_ROOT / "undefined_funcs_auto.txt")
UNDEF_SYMS = str(PROJECT_ROOT / "undefined_syms_auto.txt")

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w".split()
CPP_FLAGS = "-Iinclude -undef -Wall -lang-c -fno-builtin".split()
CPP_DEFS = ("-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx "
            "-D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL "
            "-D_LANGUAGE_C -DLANGUAGE_C").split()
AS_FLAGS = "-Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0".split()
MASPSX_FLAGS = "--expand-div --aspsx-version=2.34".split()

LOAD_ADDR = 0x80010000
EXE_HEADER = 0x800


def find_remaining_stubs(src_file=None):
    stubs = []
    if src_file:
        files = [Path(src_file)]
    else:
        files = sorted(SRC_DIR.glob("*.c"))
    for path in files:
        with open(path) as f:
            for line in f:
                m = re.match(r'\s*INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)', line)
                if m:
                    stubs.append((str(path), m.group(1)))
    return stubs


def get_func_info_from_asm(asm_path):
    with open(asm_path) as f:
        lines = f.readlines()
    addrs = []
    for line in lines:
        m = re.search(r'/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+', line)
        if m:
            addrs.append(int(m.group(1), 16))
    if addrs:
        return addrs[0], addrs[-1] + 4 - addrs[0]
    return None, None


def get_original_bytes(addr, size):
    offset = EXE_HEADER + (addr - LOAD_ADDR)
    with open(ORIGINAL_EXE, "rb") as f:
        f.seek(offset)
        return f.read(size)


def ensure_m2c_context():
    if not M2C_CONTEXT.exists():
        cmd = ["mipsel-linux-gnu-cpp", "-Iinclude", "-P",
               str(INCLUDE_DIR / "common.h")]
        result = subprocess.run(cmd, capture_output=True, cwd=str(PROJECT_ROOT))
        with open(M2C_CONTEXT, "wb") as f:
            f.write(result.stdout)


def run_m2c(func_name, asm_path):
    ensure_m2c_context()
    cmd = [sys.executable, str(M2C_SCRIPT), "-t", "mips-gcc-c",
           "--context", str(M2C_CONTEXT), str(asm_path)]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                cwd=str(PROJECT_ROOT), timeout=60)
    except subprocess.TimeoutExpired:
        return None, "timeout"
    if result.returncode != 0:
        return None, result.stderr.strip()
    return result.stdout.strip(), None


def compile_and_diff(c_code, func_name, func_addr, func_size):
    tmpdir = tempfile.mkdtemp(prefix="bb2_m2c_")
    try:
        c_path = os.path.join(tmpdir, func_name + ".c")
        obj_path = os.path.join(tmpdir, func_name + ".o")
        bin_path = os.path.join(tmpdir, func_name + ".bin")
        ld_path = os.path.join(tmpdir, "link.ld")
        elf_path = os.path.join(tmpdir, func_name + ".elf")

        with open(c_path, "w") as f:
            f.write('#include "common.h"\n\n')
            f.write(c_code)

        # cpp -> cc1 -> maspsx -> as
        cpp_r = subprocess.run(
            ["mipsel-linux-gnu-cpp"] + CPP_FLAGS + CPP_DEFS + [c_path],
            capture_output=True, cwd=str(PROJECT_ROOT))
        if cpp_r.returncode != 0:
            return None, None, None, "CPP fail"

        cc1_r = subprocess.run(
            [CC1] + CC_FLAGS, input=cpp_r.stdout,
            capture_output=True, cwd=str(PROJECT_ROOT))
        if cc1_r.returncode != 0:
            return None, None, None, "CC1 fail"

        mas_r = subprocess.run(
            ["python3", MASPSX] + MASPSX_FLAGS, input=cc1_r.stdout,
            capture_output=True, cwd=str(PROJECT_ROOT))
        if mas_r.returncode != 0:
            return None, None, None, "MASPSX fail"

        as_r = subprocess.run(
            ["mipsel-linux-gnu-as"] + AS_FLAGS + ["-o", obj_path],
            input=mas_r.stdout, capture_output=True, cwd=str(PROJECT_ROOT))
        if as_r.returncode != 0:
            return None, None, None, "AS fail"

        # Link at correct address
        with open(ld_path, "w") as f:
            f.write("SECTIONS {\n")
            f.write("    .text 0x%08X : SUBALIGN(4) { *(.text) }\n" % func_addr)
            f.write("    /DISCARD/ : { *(*) }\n")
            f.write("}\n")

        subprocess.run(
            ["mipsel-linux-gnu-ld", "-nostdlib", "--no-check-sections",
             "--unresolved-symbols=ignore-all", "--noinhibit-exec",
             "-T", ld_path, "-T", UNDEF_FUNCS, "-T", UNDEF_SYMS,
             obj_path, "-o", elf_path],
            capture_output=True, cwd=str(PROJECT_ROOT))

        subprocess.run(
            ["mipsel-linux-gnu-objcopy", "-O", "binary", "-j", ".text",
             elf_path, bin_path],
            capture_output=True, cwd=str(PROJECT_ROOT))

        if not os.path.exists(bin_path):
            return None, None, None, "Link fail"

        with open(bin_path, "rb") as f:
            compiled = f.read()

        original = get_original_bytes(func_addr, func_size)
        min_len = min(len(original), len(compiled))
        diff_count = 0
        for i in range(0, min_len, 4):
            if original[i:i+4] != compiled[i:i+4]:
                diff_count += 1
        diff_count += abs(len(original) - len(compiled)) // 4
        total = func_size // 4

        return total - diff_count, total, diff_count, None
    except Exception as e:
        return None, None, None, str(e)[:100]
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def main():
    parser = argparse.ArgumentParser(description="BB2 auto-decompile pipeline")
    parser.add_argument("--file", help="Only process stubs from this source file")
    parser.add_argument("--func", help="Process a single function")
    parser.add_argument("--compile", action="store_true", help="Compile and diff")
    parser.add_argument("--output-dir", help="Save m2c C output to directory")
    parser.add_argument("--top", type=int, default=0, help="Show top N matches")
    parser.add_argument("--csv", help="Export to CSV")
    parser.add_argument("-q", "--quiet", action="store_true")
    args = parser.parse_args()

    if args.output_dir:
        os.makedirs(args.output_dir, exist_ok=True)

    if args.func:
        all_stubs = find_remaining_stubs()
        stubs = [(s, f) for s, f in all_stubs if f == args.func]
        if not stubs:
            asm_path = ASM_FUNCS_DIR / (args.func + ".s")
            if asm_path.exists():
                stubs = [("standalone", args.func)]
            else:
                print("Function %s not found" % args.func)
                sys.exit(1)
    else:
        stubs = find_remaining_stubs(args.file)

    if not stubs:
        print("No INCLUDE_ASM stubs found")
        sys.exit(0)

    print("Found %d remaining stubs" % len(stubs))
    if args.compile:
        print("Will compile and diff each against original")
    print()

    results = []
    for i, (src, func) in enumerate(stubs, 1):
        asm_path = ASM_FUNCS_DIR / (func + ".s")
        if not asm_path.exists():
            r = {"func": func, "src": os.path.basename(src),
                 "status": "no_asm", "instr": 0}
            results.append(r)
            if not args.quiet:
                print("  [%d/%d] %-30s %-20s no_asm" % (i, len(stubs), func, r["src"]))
            continue

        addr, size = get_func_info_from_asm(asm_path)
        if addr is None:
            r = {"func": func, "src": os.path.basename(src),
                 "status": "bad_asm", "instr": 0}
            results.append(r)
            continue

        num_instr = size // 4

        m2c_output, m2c_err = run_m2c(func, asm_path)
        if m2c_output is None:
            r = {"func": func, "src": os.path.basename(src),
                 "status": "m2c_fail", "instr": num_instr, "error": m2c_err}
            results.append(r)
            if not args.quiet:
                print("  [%d/%d] %-30s %-20s m2c FAIL" % (i, len(stubs), func, r["src"]))
            continue

        if args.output_dir:
            out_path = Path(args.output_dir) / (func + ".c")
            with open(out_path, "w") as f:
                f.write(m2c_output)

        r = {"func": func, "src": os.path.basename(src),
             "status": "m2c_ok", "instr": num_instr}

        if args.compile:
            match, total, diff, err = compile_and_diff(m2c_output, func, addr, size)
            if err:
                r["status"] = "compile_fail"
                r["error"] = err
            else:
                r["status"] = "compiled"
                r["match"] = match
                r["total"] = total
                r["diff"] = diff
                r["score"] = round(100.0 * match / total, 1) if total else 0

        results.append(r)

        if not args.quiet:
            if r["status"] == "compiled":
                diff = r.get("diff", "?")
                if diff == 0:
                    tag = "MATCH!"
                else:
                    tag = "%.1f%% (%s diff)" % (r.get("score", 0), diff)
                print("  [%d/%d] %-30s %-20s %s" % (i, len(stubs), func, r["src"], tag))
            elif r["status"] == "compile_fail":
                err = r.get("error", "")[:40]
                print("  [%d/%d] %-30s %-20s compile_fail: %s" % (i, len(stubs), func, r["src"], err))
            else:
                print("  [%d/%d] %-30s %-20s %s" % (i, len(stubs), func, r["src"], r["status"]))

    # Summary
    print()
    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)

    m2c_ok = sum(1 for r in results if r["status"] in ("m2c_ok", "compiled", "compile_fail"))
    m2c_fail = sum(1 for r in results if r["status"] == "m2c_fail")
    compile_ok = sum(1 for r in results if r["status"] == "compiled")

    print("Total stubs:  %d" % len(results))
    print("m2c success:  %d" % m2c_ok)
    print("m2c failed:   %d" % m2c_fail)

    if args.compile:
        print("Compiled OK:  %d" % compile_ok)
        compiled = [r for r in results if r["status"] == "compiled"]
        if compiled:
            perfect = [r for r in compiled if r.get("diff") == 0]
            close = [r for r in compiled if 0 < r.get("diff", 999) <= 5]
            print()
            print("PERFECT matches: %d" % len(perfect))
            print("Close (<=5 diff): %d" % len(close))

            if perfect:
                print("\n  PERFECT:")
                for r in sorted(perfect, key=lambda x: x["func"]):
                    print("    %s (%s, %d instr)" % (r["func"], r["src"], r["instr"]))

            if args.top > 0:
                by_score = sorted(compiled, key=lambda x: x.get("score", 0), reverse=True)
                print("\n  TOP %d:" % min(args.top, len(by_score)))
                for r in by_score[:args.top]:
                    print("    %-30s %-20s score=%5.1f%% (%d/%d, %d diff)" % (
                        r["func"], r["src"], r.get("score", 0),
                        r.get("match", 0), r.get("total", 0), r.get("diff", 0)))

    # Per-file breakdown
    by_file = {}
    for r in results:
        f = r.get("src", "?")
        d = by_file.setdefault(f, {"total": 0, "m2c_ok": 0, "compiled": 0, "perfect": 0})
        d["total"] += 1
        if r["status"] in ("m2c_ok", "compiled", "compile_fail"):
            d["m2c_ok"] += 1
        if r["status"] == "compiled":
            d["compiled"] += 1
        if r.get("diff") == 0:
            d["perfect"] += 1

    print("\nPer-file:")
    for f in sorted(by_file):
        d = by_file[f]
        line = "  %-25s stubs=%3d  m2c_ok=%3d" % (f, d["total"], d["m2c_ok"])
        if args.compile:
            line += "  compiled=%3d  perfect=%3d" % (d["compiled"], d["perfect"])
        print(line)

    if args.csv:
        fields = ["func", "src", "status", "instr"]
        if args.compile:
            fields += ["match", "total", "diff", "score"]
        fields.append("error")
        with open(args.csv, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
            w.writeheader()
            for r in sorted(results, key=lambda x: x.get("score", 0), reverse=True):
                w.writerow(r)
        print("\nExported to %s" % args.csv)


if __name__ == "__main__":
    main()
