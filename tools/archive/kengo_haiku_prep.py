#!/usr/bin/env python3
"""
kengo_haiku_prep.py — Generate per-file agent context for Kengo-guided decompilation.

Outputs a manifest of which named stubs to attack per source file, along with
pre-fetched kengo_ref --bb2 output and m2c drafts for each function.

Usage:
  python3 tools/kengo_haiku_prep.py [src_file] [--max-insns N] [--output-dir DIR]

Examples:
  # Generate context for one file
  python3 tools/kengo_haiku_prep.py code6cac_b.c --max-insns 100

  # Generate for all files, stubs ≤100 insns
  python3 tools/kengo_haiku_prep.py --all --max-insns 100

  # Show summary only (no files written)
  python3 tools/kengo_haiku_prep.py --summary
"""

import sys
import os
import re
import csv
import subprocess
import json
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).parent.parent
SRC_DIR = ROOT / "src"
ASM_FUNCS = ROOT / "asm/funcs"
KENGO_MATCHES = ROOT / "kengo_matches.csv"
KENGO_FUNCS_FILE = ROOT / "Kengo/kengo_functions_full.txt"
M2C = ROOT / ".venv/bin/m2c"
OBJDUMP = "mipsel-linux-gnu-objdump"
KENGO_ELF = ROOT / "Kengo/disc/SLUS_200.21"


def load_kengo_matches():
    with open(KENGO_MATCHES) as f:
        return {r["bb2_func"]: r for r in csv.DictReader(f)}


def load_kengo_func_addrs():
    """name -> (addr, size)"""
    funcs = {}
    pat = re.compile(r"/\*\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\*/\s+(?:static\s+)?(\w+)\(\)")
    with open(KENGO_FUNCS_FILE) as f:
        for line in f:
            m = pat.search(line)
            if m:
                name = m.group(3)
                if name not in funcs:
                    funcs[name] = (int(m.group(1), 16), int(m.group(2), 16))
    return funcs


def get_named_stubs_for_file(src_file, kengo_matches, max_insns=None):
    """Return list of (func_addr, kengo_name, insns) stubs in src_file."""
    path = SRC_DIR / src_file
    if not path.exists():
        return []
    stubs = []
    pat = re.compile(r"INCLUDE_ASM\s*\(\s*\"[^\"]+\"\s*,\s*(\w+)\s*\)")
    with open(path) as f:
        for line in f:
            m = pat.search(line)
            if m:
                func = m.group(1)
                kr = kengo_matches.get(func)
                if kr and kr["kengo_name"]:
                    insns = int(kr.get("bb2_insns", "0") or 0)
                    if max_insns is None or insns <= max_insns:
                        stubs.append({
                            "func": func,
                            "kengo_name": kr["kengo_name"],
                            "bb2_insns": insns,
                            "kengo_insns": int(kr.get("kengo_insns", "0") or 0),
                            "kengo_src": kr.get("kengo_source_file", ""),
                        })
    return stubs


def get_kengo_asm(kengo_name, kengo_funcs):
    if kengo_name not in kengo_funcs:
        return None
    addr, size = kengo_funcs[kengo_name]
    result = subprocess.run(
        [OBJDUMP, "-d",
         f"--start-address=0x{addr:x}",
         f"--stop-address=0x{addr+size:x}",
         str(KENGO_ELF)],
        capture_output=True, text=True
    )
    lines = []
    in_body = False
    for line in result.stdout.splitlines():
        if re.match(r"^[0-9a-f]+ <\w", line):
            in_body = True
        if in_body:
            lines.append(line)
    return "\n".join(lines) if lines else None


def get_bb2_asm(func):
    """Find the BB2 asm file (by func addr or kengo name)."""
    for name in [func, func.replace("func_", "")]:
        p = ASM_FUNCS / f"{name}.s"
        if p.exists():
            return p.read_text()
    return None


def run_m2c(func):
    """Run m2c on the BB2 asm stub."""
    # Find asm file
    asm_path = ASM_FUNCS / f"{func}.s"
    if not asm_path.exists():
        return None, "asm file not found"
    cmd = [
        str(M2C),
        "--valid-syntax",
        "--target", "mips-ps1",
        "--context", str(ROOT / "include/m2c_context.h.m2c"),
        str(asm_path)
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=str(ROOT))
    if result.returncode != 0:
        return None, result.stderr[:500]
    return result.stdout, None


def build_context(src_file, stubs, kengo_funcs, output_dir):
    """Generate context JSON for all stubs in a source file."""
    contexts = []
    for stub in stubs:
        func = stub["func"]
        kname = stub["kengo_name"]
        print(f"  {func} ({stub['bb2_insns']}i) = {kname}", end=" ... ", flush=True)

        bb2_asm = get_bb2_asm(func)
        kengo_asm = get_kengo_asm(kname, kengo_funcs)
        m2c_draft, m2c_err = run_m2c(func)

        # Quick structural similarity score: count jal/call lines in each
        def count_jals(asm_text):
            if not asm_text:
                return 0
            return sum(1 for l in asm_text.splitlines() if re.search(r"\bjal\b", l))

        kengo_jals = count_jals(kengo_asm)
        bb2_jals = count_jals(bb2_asm)
        # Warn if Kengo has named jals but BB2 has very few
        likely_false = (kengo_jals >= 3 and bb2_jals == 0)
        similarity = "suspect" if likely_false else "ok"

        contexts.append({
            "func": func,
            "kengo_name": kname,
            "bb2_insns": stub["bb2_insns"],
            "kengo_insns": stub["kengo_insns"],
            "kengo_src": stub["kengo_src"],
            "kengo_jals": kengo_jals,
            "bb2_jals": bb2_jals,
            "similarity": similarity,
            "bb2_asm": bb2_asm or "",
            "kengo_asm": kengo_asm or "",
            "m2c_draft": m2c_draft or "",
            "m2c_error": m2c_err or "",
        })
        flag = " ⚠ suspect rename" if likely_false else ""
        print(f"kengo_jals={kengo_jals} bb2_jals={bb2_jals}{flag}")

    out_file = output_dir / f"{src_file.replace('.c', '')}_context.json"
    with open(out_file, "w") as f:
        json.dump({"src_file": src_file, "stubs": contexts}, f, indent=2)
    print(f"  → wrote {out_file}")
    return out_file


def print_summary(kengo_matches, max_insns=100):
    """Print a summary table of named stubs per file."""
    src_files = [f.name for f in SRC_DIR.glob("*.c")]

    print(f"Named stubs per file (≤{max_insns} insns):")
    print(f"{'File':<20} {'Total':>6} {'Named':>7} {'≤100i':>6}")
    print("-" * 45)
    for sf in sorted(src_files):
        all_stubs = get_named_stubs_for_file(sf, kengo_matches, max_insns=None)
        small_stubs = [s for s in all_stubs if s["bb2_insns"] <= max_insns]
        if all_stubs:
            # Count total stubs in file
            path = SRC_DIR / sf
            total = path.read_text().count("INCLUDE_ASM")
            print(f"{sf:<20} {total:>6} {len(all_stubs):>7} {len(small_stubs):>6}")
    print()


def main():
    args = sys.argv[1:]
    if not args or "--help" in args or "-h" in args:
        print(__doc__)
        sys.exit(0)

    max_insns = 100
    if "--max-insns" in args:
        idx = args.index("--max-insns")
        max_insns = int(args[idx + 1])

    output_dir = ROOT / "tmp" / "kengo_contexts"
    if "--output-dir" in args:
        idx = args.index("--output-dir")
        output_dir = Path(args[idx + 1])
    output_dir.mkdir(parents=True, exist_ok=True)

    kengo_matches = load_kengo_matches()
    kengo_funcs = load_kengo_func_addrs()

    if "--summary" in args:
        print_summary(kengo_matches, max_insns)
        return

    if "--all" in args:
        src_files = [f.name for f in SRC_DIR.glob("*.c")]
    else:
        # First non-flag arg is src file
        target = next((a for a in args if not a.startswith("--") and not a.lstrip("-").isdigit()), None)
        if not target:
            print("ERROR: specify a source file or --all")
            sys.exit(1)
        src_files = [target if target.endswith(".c") else target + ".c"]

    for sf in sorted(src_files):
        stubs = get_named_stubs_for_file(sf, kengo_matches, max_insns=max_insns)
        if not stubs:
            continue
        print(f"\n{sf}: {len(stubs)} named stubs ≤{max_insns} insns")
        build_context(sf, stubs, kengo_funcs, output_dir)


if __name__ == "__main__":
    main()
