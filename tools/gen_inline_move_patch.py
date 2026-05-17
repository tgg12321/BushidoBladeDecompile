#!/usr/bin/env python3
"""Generate inline_move_aliasing candidate C patch for a lost_codegen cheat.

Given a regfix.txt rule like:
    func: insert "addu $RD, $RS, $zero" @ N

Generates a C source snippet that emits the same instruction at the
target maspsx position via the legitimate inline_move_aliasing pattern:

    register T x asm("$RD");
    __asm__ volatile("move %0, %1" : "=r"(x) : "r"(source_var));

The source_var is determined by reading the function's dump-text around
position N to identify what register N currently holds.

Output is a suggested edit comment — NOT auto-applied (requires human
judgment about C source structure).

Usage: python3 tools/gen_inline_move_patch.py FUNC
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path


LOST_CODEGEN_RE = re.compile(
    r'^(\w+):\s+(insert|insert_after|insert_before)\s+"addu\s+\$(\w+),\s*\$(\w+),\s*\$(\w+)"\s+@\s+(\d+)\s*$',
    re.MULTILINE,
)

REG_TO_NAME = {
    "0": "zero", "1": "at", "2": "v0", "3": "v1",
    "4": "a0", "5": "a1", "6": "a2", "7": "a3",
    "8": "t0", "9": "t1", "10": "t2", "11": "t3",
    "12": "t4", "13": "t5", "14": "t6", "15": "t7",
    "16": "s0", "17": "s1", "18": "s2", "19": "s3",
    "20": "s4", "21": "s5", "22": "s6", "23": "s7",
    "24": "t8", "25": "t9", "26": "k0", "27": "k1",
    "28": "gp", "29": "sp", "30": "fp", "31": "ra",
}


def normalize_reg(reg: str) -> str:
    """Normalize $N or $name to canonical name."""
    if reg.isdigit():
        return REG_TO_NAME.get(reg, reg)
    return reg


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func", help="Function name")
    args = ap.parse_args()

    # Find lost_codegen rules for func
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        text = Path(fname).read_text(encoding="utf-8")
        for m in LOST_CODEGEN_RE.finditer(text):
            if m.group(1) != args.func:
                continue
            rd, rs, rt = m.group(3), m.group(4), m.group(5)
            idx = int(m.group(6))
            # Filter for lost_codegen pattern
            is_lost_codegen = (rs in ("0", "zero")) or (rt in ("0", "zero"))
            if not is_lost_codegen:
                continue
            rd_name = normalize_reg(rd)
            rs_name = normalize_reg(rs)
            rt_name = normalize_reg(rt)

            print(f"=== {fname} rule ===")
            print(f"  Pattern: addu ${rd_name}, ${rs_name}, ${rt_name} @ {idx}")
            print()

            # Determine source register (non-zero operand)
            if rs in ("0", "zero"):
                src_reg = rt_name
            else:
                src_reg = rs_name

            if src_reg == "zero":
                # Both zero: this is a value-zero init (e.g., bits = 0)
                print(f"  → Initializes ${rd_name} to 0")
                print(f"  Candidate C patch (place at corresponding source position):")
                print(f"  ```c")
                print(f"  register T x asm(\"${rd_name}\");")
                print(f"  __asm__ volatile(\"addu %0, $zero, $zero\" : \"=r\"(x));")
                print(f"  ```")
                print()
                print(f"  ALTERNATIVE: just write `x = 0;` if cc1 will emit it naturally.")
                print(f"  Use the asm form only if cc1 dead-store-eliminates the explicit assignment.")
            else:
                print(f"  → Copies ${src_reg} into ${rd_name}")
                print(f"  Candidate C patch (place at corresponding source position):")
                print(f"  ```c")
                print(f"  // Force materialization of source value (prevent collapsing):")
                print(f"  /* the source variable holding the value currently in ${src_reg} */")
                print(f"  T source_var = ...;  /* fill in from C source */")
                print(f"  register T renamed asm(\"${rd_name}\");")
                print(f"  __asm__ volatile(\"move %0, %1\" : \"=r\"(renamed) : \"r\"(source_var));")
                print(f"  /* then use `renamed` in subsequent C where target uses ${rd_name} */")
                print(f"  ```")

            print()
            print(f"  Note: REQUIRES corresponding C source restructure to USE the renamed")
            print(f"  variable everywhere target uses ${rd_name}, or related rules will")
            print(f"  cascade-fail.")
            print()

    return 0


if __name__ == "__main__":
    sys.exit(main())
