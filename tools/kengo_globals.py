#!/usr/bin/env python3
"""kengo_globals.py — propose BB2 data-symbol names from Kengo's globals.

Builds per-function data-reference sets on both sides, then scores
candidate (BB2_D, Kengo_K) name pairs by co-occurrence across the
function pairs in kengo_matches.csv.

Mechanism:
  1. Parse Kengo/kengo_globals.txt for named OBJECT symbols
     (~666 with clean names). Build address-range -> name index.
  2. Walk tmp/kengo_disasm.txt; for each Kengo function, resolve
     data references:
       - %hi/%lo pairs from `lui $r, 0xHH` + `<op> ..., -off($r)`
       - gp-relative loads/stores (gp = 0x00360e70)
     Map effective address into the nearest Kengo named global.
  3. Walk asm/funcs/*.s for BB2 D_8XXXXXXX references per function.
  4. Use kengo_matches.csv to project each Kengo function's name onto
     its BB2 counterpart (only `name-unique`/`callgraph`/`caller-*`/
     `affinity-*` confidences are trusted).
  5. For each (BB2_D, Kengo_K) candidate pair, compute Jaccard of:
       set of BB2 funcs that reference BB2_D
       set of BB2 funcs that "should" reference K (mapped from
         Kengo funcs that reference K)
  6. Output the strongest pairs as a name proposal list.

Default: dry-run report. `--apply` writes accepted entries into
named_syms.txt and substitutes `D_8XXXXXXX` -> `<kengo_name>` across
src/, include/, asmfix/regfix/sdata texts.

Usage:
  python3 tools/kengo_globals.py                  # dry-run report
  python3 tools/kengo_globals.py --apply
  python3 tools/kengo_globals.py --min-jaccard 0.5 --min-dominance 1.5
  python3 tools/kengo_globals.py --min-co-funcs 3
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
KENGO_GLOBALS = ROOT / "Kengo" / "kengo_globals.txt"
KENGO_DISASM = ROOT / "tmp" / "kengo_disasm.txt"
KENGO_MATCHES = ROOT / "kengo_matches.csv"
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
NAMED_SYMS = ROOT / "named_syms.txt"
SYMBOL_ADDRS = ROOT / "symbol_addrs.txt"

KENGO_GP = 0x00360E70  # _gp = 0x00360e70 from readelf

# Confidence tiers that are trusted to seed Kengo->BB2 function mapping.
TRUSTED_CONFIDENCES = {
    "name-unique", "name-callgraph", "callgraph",
    "caller-unique", "caller-callgraph",
    "affinity-unique", "affinity-callgraph",
}

# Kengo globals table parsing: ELF readelf -s output preserved as text:
#    47: 001a6868     0 OBJECT  LOCAL  DEFAULT    4 BOOLEAN.78
GLOBAL_RE = re.compile(
    r"^\s*\d+:\s+([0-9a-fA-F]+)\s+(\d+)\s+OBJECT\s+\S+\s+\S+\s+\S+\s+(\S+)"
)


def parse_kengo_globals() -> list[tuple[int, int, str]]:
    """Return [(addr, size, name)] for named, sized OBJECT entries. Skips
    libc internals (gcc2_compiled, .NN suffixes, leading underscore)."""
    out: list[tuple[int, int, str]] = []
    for line in KENGO_GLOBALS.read_text(errors="replace").splitlines():
        m = GLOBAL_RE.match(line)
        if not m:
            continue
        addr, size, name = int(m.group(1), 16), int(m.group(2)), m.group(3)
        if size == 0:
            continue
        if "." in name or name.startswith("_") or name in {"gcc2_compiled"}:
            continue
        out.append((addr, size, name))
    out.sort()
    return out


def build_addr_index(globals_list: list[tuple[int, int, str]]):
    """Sorted list of (addr_start, addr_end, name) for binary search."""
    return [(a, a + s, n) for a, s, n in globals_list]


def resolve_addr(addr: int, idx) -> str | None:
    """Return the name whose range covers `addr`, or None."""
    # Linear scan suffices for our scale; <1k entries.
    for start, end, name in idx:
        if start <= addr < end:
            return name
    return None


# Kengo disassembly extractor: a function header looks like
#   0011d370 <coli_check_circle_hit_line>:
# Each instruction line:
#   11d370:\t<hex>\t<mnem>\t<operands>
FUNC_HDR = re.compile(r"^([0-9a-fA-F]+) <(\w+)>:")
INSN_LINE = re.compile(r"^\s+([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+(\S+)\s*(.*)")
LUI_OP = re.compile(r"^([a-z][a-z0-9]*)$")
LUI_PAT = re.compile(r"\$?(\w+),(?:0x)?([0-9a-fA-F]+)")
MEM_PAT = re.compile(r"-?\d+\(\$?(\w+)\)")
HILO_PAT = re.compile(r"%(hi|lo)\((\w+)\)")
GP_LOAD_STORE = re.compile(r"^(lw|sw|lh|sh|lb|sb|lhu|lbu|lwc1|swc1)$")
HILO_LOAD_STORE_ADDIU = re.compile(r"^(lw|sw|lh|sh|lb|sb|lhu|lbu|lwc1|swc1|addiu)$")


def extract_kengo_refs(idx) -> dict[str, set[str]]:
    """Return {kengo_func_name: {kengo_data_name, ...}}.

    Detects two reference styles:
      - high/low pairs: lui $r, 0xHH later paired with <op> ..., -off($r)
        Effective address = (HH << 16) + sign_extend_16(off)
      - gp-relative: <load/store> $r, -off(gp)
        Effective address = KENGO_GP + sign_extend_16(off)
    """
    out: dict[str, set[str]] = {}
    cur_func: str | None = None
    last_lui: dict[str, int] = {}  # reg -> upper16<<16
    text = KENGO_DISASM.read_text(errors="replace")
    for line in text.splitlines():
        m = FUNC_HDR.match(line)
        if m:
            cur_func = m.group(2)
            last_lui.clear()
            out.setdefault(cur_func, set())
            continue
        if cur_func is None:
            continue
        m = INSN_LINE.match(line)
        if not m:
            continue
        mnem, operands = m.group(3), m.group(4)
        # Track lui's
        if mnem == "lui":
            lm = LUI_PAT.search(operands)
            if lm:
                reg = lm.group(1)
                imm = int(lm.group(2), 16)
                last_lui[reg] = (imm & 0xFFFF) << 16
            continue
        # Pair with subsequent load/store/addiu
        if HILO_LOAD_STORE_ADDIU.match(mnem):
            mm = re.search(r"(-?\d+)\(\$?(\w+)\)", operands)
            if mm:
                off = int(mm.group(1))
                reg = mm.group(2)
                if reg == "gp":
                    addr = (KENGO_GP + off) & 0xFFFFFFFF
                elif reg in last_lui:
                    addr = (last_lui[reg] + off) & 0xFFFFFFFF
                    last_lui.pop(reg, None)
                else:
                    continue
                name = resolve_addr(addr, idx)
                if name:
                    out[cur_func].add(name)
            elif mnem == "addiu":
                # `addiu rd, rs, imm` where rs had the lui — pseudo for taking
                # the address of the symbol.
                am = re.match(r"\$?(\w+),\$?(\w+),(-?\d+)", operands)
                if am:
                    rs = am.group(2)
                    imm = int(am.group(3))
                    if rs in last_lui:
                        addr = (last_lui[rs] + imm) & 0xFFFFFFFF
                        last_lui.pop(rs, None)
                        name = resolve_addr(addr, idx)
                        if name:
                            out[cur_func].add(name)
    return out


BB2_DATA_RE = re.compile(r"\bD_8[0-9A-Fa-f]{7}\b")


def extract_bb2_refs() -> dict[str, set[str]]:
    """{bb2_func_name: {D_8XXXXXXX, ...}} over asm/funcs/*.s."""
    out: dict[str, set[str]] = {}
    for p in sorted(ASM_FUNCS.glob("*.s")):
        name = p.stem
        if name.startswith("D_"):
            continue
        text = p.read_text(encoding="utf-8", errors="replace")
        refs = set(BB2_DATA_RE.findall(text))
        if refs:
            out[name] = refs
    return out


def load_function_matches() -> dict[str, str]:
    """{bb2_func_name: kengo_func_name} for trusted confidence rows.

    Suffix-stripped variants (`<kn>_<ADDR>`) map to bare `<kn>` so renamed
    TU-static instances participate in the propagation.
    """
    out: dict[str, str] = {}
    if not KENGO_MATCHES.exists():
        return out
    for r in csv.DictReader(KENGO_MATCHES.open()):
        bb2 = r["bb2_func"]
        kn = r["kengo_name"]
        conf = r["confidence"]
        if not kn or conf not in TRUSTED_CONFIDENCES:
            continue
        out[bb2] = kn
    return out


def score_candidates(bb2_refs: dict[str, set[str]],
                      kengo_refs: dict[str, set[str]],
                      fn_match: dict[str, str]) -> dict[str, dict[str, dict]]:
    """For each BB2 D symbol, score every Kengo K symbol by co-occurrence
    across matched function pairs."""
    # Build inverse: D -> set(bb2 funcs that reference D)
    inv_bb2: dict[str, set[str]] = defaultdict(set)
    for f, refs in bb2_refs.items():
        for d in refs:
            inv_bb2[d].add(f)

    # K -> set(bb2 funcs whose Kengo equivalent references K)
    inv_k_via_bb2: dict[str, set[str]] = defaultdict(set)
    for bb2_func, kengo_func in fn_match.items():
        for k in kengo_refs.get(kengo_func, set()):
            inv_k_via_bb2[k].add(bb2_func)

    scored: dict[str, dict[str, dict]] = defaultdict(dict)
    for d, bb2_fns in inv_bb2.items():
        if len(bb2_fns) < 2:
            # Single-callsite syms have weak statistical evidence — skip.
            continue
        for k, mapped_fns in inv_k_via_bb2.items():
            inter = bb2_fns & mapped_fns
            if not inter:
                continue
            union = bb2_fns | mapped_fns
            jacc = len(inter) / len(union)
            scored[d][k] = {
                "jaccard": jacc,
                "co": len(inter),
                "bb2_count": len(bb2_fns),
                "kengo_count": len(mapped_fns),
            }
    return scored


def pick_proposals(scored: dict[str, dict[str, dict]],
                   min_jaccard: float, min_dominance: float,
                   min_co: int) -> list[dict]:
    """For each BB2_D, pick the dominant Kengo_K if it satisfies all
    thresholds: jaccard >= min_jaccard, co-occurrence >= min_co, and
    top/second >= min_dominance."""
    proposals: list[dict] = []
    for d, candidates in scored.items():
        ranked = sorted(candidates.items(), key=lambda x: -x[1]["jaccard"])
        if not ranked:
            continue
        top_k, top_s = ranked[0]
        second_s = ranked[1][1]["jaccard"] if len(ranked) > 1 else 0.0
        if top_s["jaccard"] < min_jaccard:
            continue
        if top_s["co"] < min_co:
            continue
        if top_s["jaccard"] / max(second_s, 0.01) < min_dominance:
            continue
        proposals.append({
            "bb2": d,
            "kn": top_k,
            "jaccard": top_s["jaccard"],
            "co": top_s["co"],
            "bb2_count": top_s["bb2_count"],
            "kengo_count": top_s["kengo_count"],
            "second_jaccard": second_s,
            "second_kn": ranked[1][0] if len(ranked) > 1 else "",
        })
    proposals.sort(key=lambda p: -p["jaccard"])
    return proposals


def load_existing_names() -> dict[str, int]:
    """Existing symbol names from named_syms.txt and symbol_addrs.txt."""
    out: dict[str, int] = {}
    pat = re.compile(r"^\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
    for f in (SYMBOL_ADDRS, NAMED_SYMS):
        if not f.exists():
            continue
        for line in f.read_text(errors="replace").splitlines():
            m = pat.match(line)
            if m:
                out[m.group(1)] = int(m.group(2), 16)
    return out


def filter_proposals(proposals: list[dict], existing: dict[str, int]) -> tuple[list[dict], list[dict]]:
    """Drop proposals that collide with already-used names or addresses.
    Returns (accepted, skipped_with_reason)."""
    accepted: list[dict] = []
    skipped: list[dict] = []
    used_names: dict[str, int] = dict(existing)
    used_addrs: dict[int, str] = {a: n for n, a in existing.items()}
    for p in proposals:
        d_addr = int(p["bb2"][2:], 16)  # D_8XXXXXXX -> 0x8XXXXXXX
        kn = p["kn"]
        if kn in used_names:
            p["_skip"] = f"name {kn} already used at 0x{used_names[kn]:08X}"
            skipped.append(p)
            continue
        if d_addr in used_addrs:
            p["_skip"] = f"addr 0x{d_addr:08X} already named as {used_addrs[d_addr]}"
            skipped.append(p)
            continue
        used_names[kn] = d_addr
        used_addrs[d_addr] = kn
        accepted.append(p)
    return accepted, skipped


def append_names(accepted: list[dict]) -> None:
    """Append `<kn> = 0xADDR;` entries to named_syms.txt."""
    lines = NAMED_SYMS.read_text(errors="replace").splitlines()
    if lines and lines[-1] != "":
        lines.append("")
    lines.append("/* === Kengo-derived globals (kengo_globals.py) === */")
    for p in sorted(accepted, key=lambda x: int(x["bb2"][2:], 16)):
        addr = int(p["bb2"][2:], 16)
        lines.append(f"{p['kn']} = 0x{addr:08X};")
    NAMED_SYMS.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


def substitute_in_files(accepted: list[dict]) -> dict[str, int]:
    """Word-boundary substitute D_8XXXXXXX -> <kn> across src/, include/,
    asmfix/regfix/sdata text files, plus asm/funcs/*.s."""
    plan = {p["bb2"]: p["kn"] for p in accepted}
    if not plan:
        return {}
    keys = "|".join(re.escape(k) for k in plan)
    pat = re.compile(rf"\b({keys})\b")

    files: list[Path] = []
    files.extend(sorted(SRC_DIR.glob("*.c")))
    files.extend(sorted((ROOT / "include").glob("*.h")))
    for name in ("regfix.txt", "asmfix.txt", "sdata_funcs.txt",
                 "sdata_exclude.txt", "expand_lb_funcs.txt"):
        p = ROOT / name
        if p.exists():
            files.append(p)
    files.extend(sorted(ASM_FUNCS.glob("*.s")))
    files.extend(sorted((ROOT / "asm").glob("*.s")))
    files.extend(sorted((ROOT / "asm" / "data").glob("*.s")))

    edits: dict[str, int] = {}
    for p in files:
        text = p.read_text(encoding="utf-8", errors="replace")
        if "D_8" not in text:
            continue
        new_text, n = pat.subn(lambda m: plan[m.group(1)], text)
        if n > 0:
            p.write_text(new_text, encoding="utf-8", newline="\n")
            edits[str(p.relative_to(ROOT))] = n
    return edits


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--apply", action="store_true",
                    help="Append names + rewrite refs (default: dry-run)")
    ap.add_argument("--min-jaccard", type=float, default=0.5,
                    help="Min Jaccard for a proposal (default 0.5)")
    ap.add_argument("--min-dominance", type=float, default=1.5,
                    help="Min top/second Jaccard dominance (default 1.5)")
    ap.add_argument("--min-co", type=int, default=2,
                    help="Min co-occurrence count (default 2)")
    ap.add_argument("--top", type=int, default=20,
                    help="Sample size for dry-run output (default 20)")
    args = ap.parse_args()

    print("Parsing Kengo globals...", file=sys.stderr)
    glob_list = parse_kengo_globals()
    print(f"  {len(glob_list)} named OBJECT entries", file=sys.stderr)

    print("Indexing Kengo data references (this scans 6MB of disasm)...", file=sys.stderr)
    idx = build_addr_index(glob_list)
    kengo_refs = extract_kengo_refs(idx)
    nonzero = sum(1 for v in kengo_refs.values() if v)
    print(f"  {nonzero}/{len(kengo_refs)} Kengo functions reference >=1 named global",
          file=sys.stderr)

    print("Extracting BB2 data references...", file=sys.stderr)
    bb2_refs = extract_bb2_refs()
    print(f"  {len(bb2_refs)} BB2 functions with >=1 D_8XXX ref", file=sys.stderr)

    print("Loading BB2->Kengo function matches (trusted tiers)...", file=sys.stderr)
    fn_match = load_function_matches()
    print(f"  {len(fn_match)} trusted function-pair mappings", file=sys.stderr)

    print("Scoring candidate pairs...", file=sys.stderr)
    scored = score_candidates(bb2_refs, kengo_refs, fn_match)
    print(f"  {len(scored)} BB2 D-symbols have at least one Kengo candidate",
          file=sys.stderr)

    proposals = pick_proposals(scored,
                                 args.min_jaccard, args.min_dominance, args.min_co)
    print(f"  {len(proposals)} pass thresholds "
          f"(jacc>={args.min_jaccard}, dom>={args.min_dominance}, co>={args.min_co})",
          file=sys.stderr)

    existing = load_existing_names()
    accepted, skipped = filter_proposals(proposals, existing)

    print()
    print(f"=== Proposals ===")
    print(f"  accepted: {len(accepted)}")
    print(f"  skipped:  {len(skipped)} (name/addr collisions with existing syms)")
    print()
    print(f"  {'jacc':>5s} {'co':>3s} {'BB2/Kg':>7s}  {'bb2_data':<18s} -> {'kengo_name':<32s} 2nd")
    for p in accepted[:args.top]:
        bk = f"{p['bb2_count']}/{p['kengo_count']}"
        print(f"  {p['jaccard']:.2f}  {p['co']:>3d}  {bk:<7s}  "
              f"{p['bb2']:<18s} -> {p['kn']:<32s} "
              f"({p['second_kn']} {p['second_jaccard']:.2f})")
    if len(accepted) > args.top:
        print(f"  ... +{len(accepted) - args.top} more")

    if skipped:
        print()
        print(f"Sample skips:")
        for p in skipped[:5]:
            print(f"  {p['bb2']:<18s} -> {p['kn']:<32s}  [{p['_skip']}]")

    if not args.apply:
        print()
        print("(dry-run — pass --apply to write changes)")
        return 0

    append_names(accepted)
    edits = substitute_in_files(accepted)
    total = sum(edits.values())
    print()
    print(f"Appended {len(accepted)} names to {NAMED_SYMS.relative_to(ROOT)}")
    print(f"Substituted {total} D_8XXX references across {len(edits)} files")
    print()
    print("Verify: `wsl bash -c \"... && make\"`  (no setup needed)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
