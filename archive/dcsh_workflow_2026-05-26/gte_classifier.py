#!/usr/bin/env python3
"""Identify which `gte_*()` C macros from include/gte.h a function uses.

Given a function with inline `__asm__()` (or an asm/funcs/<func>.s file),
scan the cop2 / lwc2 / swc2 / mfc2 / mtc2 / NCLIP / RTPx / etc. mnemonics
and report a histogram. Prints a candidate `#include "gte.h"` line plus
any direct macro suggestions for atomic ops (gte_rtps, gte_avsz3, etc).

This is the seed for Phase-4 GTE migration: it tells you which macros a
function would need so the surrounding C body can be expressed without
inline asm.
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"

# Map atomic GTE mnemonics -> C macro suggestion
GTE_OPS_TO_MACRO = {
    "rtps":   "gte_rtps()",
    "rtpt":   "gte_rtpt()",
    "nclip":  "gte_nclip()",
    "avsz3":  "gte_avsz3()",
    "avsz4":  "gte_avsz4()",
    "dpcs":   "gte_dpcs()",
    "dpct":   "gte_dpct()",
    "intpl":  "gte_intpl()",
    "sqr":    "gte_sqr()",
    "ncs":    "gte_ncs()",
    "nct":    "gte_nct()",
    "ncds":   "gte_ncds()",
    "ncdt":   "gte_ncdt()",
    "nccs":   "gte_nccs()",
    "ncct":   "gte_ncct()",
    "cdp":    "gte_cdp()",
    "cc":     "gte_cc()",
    "dpcl":   "gte_dpcl()",
}

# Known cop2 .word encodings from gte.h
WORD_TO_MACRO = {
    "0x4A180001": "gte_rtps()",
    "0x4A280030": "gte_rtpt()",
    "0x4A400006": "gte_nclip()",
    "0x4A48002D": "gte_avsz3()",
    "0x4A58002E": "gte_avsz4()",
    "0x4A780010": "gte_dpcs()",
    "0x4AF8002A": "gte_dpct()",
    "0x4A980011": "gte_intpl()",
    "0x4AA00428": "gte_sqr()",
    "0x4AC8041E": "gte_ncs()",
    "0x4AD80420": "gte_nct()",
    "0x4AE80413": "gte_ncds()",
    "0x4AF80416": "gte_ncdt()",
    "0x4B08041B": "gte_nccs()",
    "0x4B18043F": "gte_ncct()",
    "0x4B280414": "gte_cdp()",
    "0x4B38041C": "gte_cc()",
    "0x4A680029": "gte_dpcl()",
}

# COP2 data-register stores/loads -- map register-numbered swc2/lwc2 to a hint
SWC2_TO_HINT = {
    "$12": "gte_stsxy0(*) -- screen XY for vertex 0",
    "$13": "gte_stsxy1(*) -- screen XY for vertex 1",
    "$14": "gte_stsxy(*)  -- screen XY single (or vertex 2)",
    "$19": "gte_stsz(*)   -- screen Z",
    "$24": "gte_stmac0(out)",
    "$25": "gte_stmac1(out)",
    "$26": "gte_stmac2(out)",
    "$27": "gte_stmac3(out)",
}

LWC2_TO_HINT = {
    "$0": "gte_ldv0(p)  -- vector 0 low (vx,vy)",
    "$1": "gte_ldv0(p)  -- vector 0 high (vz)",
    "$2": "gte_ldv1(p)  -- vector 1 low",
    "$3": "gte_ldv1(p)  -- vector 1 high",
    "$4": "gte_ldv2(p)  -- vector 2 low",
    "$5": "gte_ldv2(p)  -- vector 2 high",
    "$6": "gte_ldrgb(p) -- packed RGB",
}


INLINE_ASM_LINE = re.compile(r'^\s*"(.*?)\\n"\s*$')


def collect_asm_lines(func: str) -> list[str]:
    """Collect raw asm lines for a function from either asm/funcs/<func>.s or
    a src/*.c inline `__asm__()` block."""
    p = ASM_FUNCS / f"{func}.s"
    if p.exists():
        out = []
        for line in p.read_text(encoding="utf-8").splitlines():
            stripped = re.sub(r"/\*[^*]*\*/", "", line).strip()
            if not stripped:
                continue
            if stripped.startswith("glabel") or stripped.startswith("endlabel"):
                continue
            out.append(stripped)
        return out
    # fallback: scan src/
    for src in sorted((ROOT / "src").glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        for blk in re.finditer(r"__asm__\s*\(([^;]*?)\)\s*;", text, re.DOTALL):
            body = blk.group(1)
            if re.search(rf"glabel\s+{re.escape(func)}\b", body):
                lines = []
                for raw in body.splitlines():
                    m = INLINE_ASM_LINE.match(raw.rstrip())
                    if m:
                        s = m.group(1).strip()
                        if not s.startswith(".set"):
                            lines.append(s)
                return lines
    return []


def classify(func: str) -> dict:
    raw_lines = collect_asm_lines(func)
    if not raw_lines:
        return {"func": func, "found": False}

    op_counter: Counter[str] = Counter()
    macro_hits: Counter[str] = Counter()
    swc2_hits: Counter[str] = Counter()
    lwc2_hits: Counter[str] = Counter()
    mtc2_hits: Counter[str] = Counter()
    mfc2_hits: Counter[str] = Counter()
    word_encoded = []

    for line in raw_lines:
        # Strip leading 'glabel' or '.LXXXX:' label lines
        if line.endswith(":") or line.startswith(".L"):
            continue
        toks = line.split(None, 1)
        if not toks:
            continue
        mnem = toks[0].lstrip(".")
        ops = toks[1] if len(toks) > 1 else ""

        if mnem in GTE_OPS_TO_MACRO:
            op_counter[mnem] += 1
            macro_hits[GTE_OPS_TO_MACRO[mnem]] += 1
            continue
        if mnem == "word":
            # Possibly a cop2 op encoded as raw word
            ops_n = ops.split(",")[0].strip().lower()
            for w, macro in WORD_TO_MACRO.items():
                if w.lower() == ops_n:
                    word_encoded.append((line, macro))
                    macro_hits[macro] += 1
                    break
            continue
        if mnem == "lwc2":
            m = re.match(r"\$(\d+)", ops)
            if m:
                key = f"${m.group(1)}"
                lwc2_hits[key] += 1
        elif mnem == "swc2":
            m = re.match(r"\$(\d+)", ops)
            if m:
                key = f"${m.group(1)}"
                swc2_hits[key] += 1
        elif mnem == "mtc2":
            mm = re.search(r",\s*\$(\d+)", ops)
            if mm:
                mtc2_hits[f"${mm.group(1)}"] += 1
        elif mnem == "mfc2":
            mm = re.search(r",\s*\$(\d+)", ops)
            if mm:
                mfc2_hits[f"${mm.group(1)}"] += 1

    # Convert lwc2/swc2 register hits to macro hints
    suggested_macros = Counter()
    for mac in macro_hits:
        suggested_macros[mac] += macro_hits[mac]
    for reg, n in lwc2_hits.items():
        h = LWC2_TO_HINT.get(reg)
        if h:
            mac = h.split(" --")[0].strip()
            suggested_macros[mac] += n
    for reg, n in swc2_hits.items():
        h = SWC2_TO_HINT.get(reg)
        if h:
            mac = h.split(" --")[0].strip()
            suggested_macros[mac] += n

    return {
        "func": func, "found": True,
        "asm_lines": len(raw_lines),
        "atomic_ops": dict(op_counter),
        "word_encoded": word_encoded,
        "lwc2_data_regs": dict(lwc2_hits),
        "swc2_data_regs": dict(swc2_hits),
        "mtc2_data_regs": dict(mtc2_hits),
        "mfc2_data_regs": dict(mfc2_hits),
        "suggested_macros": dict(suggested_macros.most_common()),
    }


def print_report(d: dict):
    f = d["func"]
    if not d["found"]:
        print(f"{f}: no inline asm or asm/funcs/.s found")
        return
    print(f"=== {f} -- {d['asm_lines']} asm lines ===")
    if d["atomic_ops"]:
        print("  atomic GTE ops:")
        for mnem, n in d["atomic_ops"].items():
            macro = GTE_OPS_TO_MACRO.get(mnem, "?")
            print(f"    {mnem:8s} x{n:<3d}  -> {macro}")
    if d["word_encoded"]:
        print("  cop2 ops via .word:")
        for line, macro in d["word_encoded"]:
            print(f"    {line}  -> {macro}")
    if d["lwc2_data_regs"]:
        print("  lwc2 vector loads:")
        for reg, n in d["lwc2_data_regs"].items():
            print(f"    {reg:5s} x{n:<3d}  ({LWC2_TO_HINT.get(reg, '?')})")
    if d["swc2_data_regs"]:
        print("  swc2 result stores:")
        for reg, n in d["swc2_data_regs"].items():
            print(f"    {reg:5s} x{n:<3d}  ({SWC2_TO_HINT.get(reg, '?')})")
    if d["mtc2_data_regs"]:
        print("  mtc2 register transfers in: ",
              ", ".join(f"{k} x{v}" for k, v in d["mtc2_data_regs"].items()))
    if d["mfc2_data_regs"]:
        print("  mfc2 register transfers out:",
              ", ".join(f"{k} x{v}" for k, v in d["mfc2_data_regs"].items()))
    if d["suggested_macros"]:
        print("  suggested includes/macros:")
        print('    #include "gte.h"')
        for mac in d["suggested_macros"]:
            print(f"    {mac}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    d = classify(args.func)
    if args.json:
        import json
        print(json.dumps(d, indent=2))
    else:
        print_report(d)
    return 0 if d["found"] else 1


if __name__ == "__main__":
    sys.exit(main())
