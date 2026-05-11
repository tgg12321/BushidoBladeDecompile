#!/usr/bin/env python3
"""apply_kengo_names.py — Promote confident kengo_matches.csv entries
into named_syms.txt so splat renames asm/funcs/func_XXXX.s -> <kengo>.s
on the next setup. Also rewrites src/*.c references in lockstep so the
build continues to link.

Steps:
  1. Read kengo_matches.csv, filter to BB2 functions that are still
     anonymous (starting with `func_`) and that have a high-confidence
     match (configurable; default excludes weak seq-similarity).
  2. Detect Kengo-name conflicts (multiple anonymous BB2 funcs claim the
     same Kengo name). Skip these in the default pass — they're either
     TU-static instances or spurious matches and need manual review.
  3. Detect collisions with already-claimed names in named_syms.txt /
     symbol_addrs.txt; skip if the name is already in use at a
     different address.
  4. Print the renamings as a plan, or `--apply` to:
       a. append entries to named_syms.txt
       b. substitute `func_XXXXXXXX` -> `<kengo_name>` across src/*.c
          using word-boundary regex (no substring corruption)
       c. report total renames + a follow-up reminder to run
          `make setup && make` to regenerate asm + verify SHA1.

Usage:
  python3 tools/apply_kengo_names.py                       # dry-run, plan only
  python3 tools/apply_kengo_names.py --apply               # do it
  python3 tools/apply_kengo_names.py --min-opseq-ratio 0.4 # stricter seq band
  python3 tools/apply_kengo_names.py --include-conflicts   # also handle conflict cases
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CSV_IN = ROOT / "kengo_matches.csv"
NAMED_SYMS = ROOT / "named_syms.txt"
SYMBOL_ADDRS = ROOT / "symbol_addrs.txt"
SRC_DIR = ROOT / "src"
ASM_FUNCS = ROOT / "asm" / "funcs"

DEFAULT_CONFIDENCES = {
    "name-unique", "name-callgraph", "callgraph",
    "caller-unique", "caller-callgraph",
    "affinity-unique", "affinity-callgraph",
    # seq-similarity gated by --min-opseq-ratio
}

SYM_LINE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
FUNC_NAME = re.compile(r"^func_([0-9A-Fa-f]{8})$")


def load_existing_names() -> dict[str, int]:
    """Existing name -> addr from both sym files. Used for collision check."""
    out: dict[str, int] = {}
    for f in (SYMBOL_ADDRS, NAMED_SYMS):
        if not f.exists():
            continue
        for line in f.read_text(errors="replace").splitlines():
            m = SYM_LINE.match(line)
            if m:
                out[m.group(1)] = int(m.group(2), 16)
    return out


def parse_csv(min_opseq_ratio: float) -> list[dict]:
    """Return [{bb2_func, kengo_name, addr, confidence, opseq_ratio}] for
    BB2 functions still anonymous and matched with adequate confidence."""
    rows: list[dict] = []
    for r in csv.DictReader(CSV_IN.open()):
        bb2 = r["bb2_func"]
        kn = r["kengo_name"]
        conf = r["confidence"]
        m = FUNC_NAME.match(bb2)
        if not m or not kn:
            continue
        addr = int(m.group(1), 16)
        opr = float(r.get("opseq_ratio") or 0)
        if conf in DEFAULT_CONFIDENCES:
            rows.append({"bb2": bb2, "kn": kn, "addr": addr,
                         "conf": conf, "opr": opr})
        elif conf == "seq-similarity" and opr >= min_opseq_ratio:
            rows.append({"bb2": bb2, "kn": kn, "addr": addr,
                         "conf": conf, "opr": opr})
    return rows


def classify(rows: list[dict], existing: dict[str, int],
             include_conflicts: bool) -> tuple[list[dict], list[dict]]:
    """Split rows into (to_apply, to_skip). A row is skipped when:
      - the Kengo name is claimed by multiple anonymous BB2 funcs
        (TU-static instances or spurious matches; needs manual review)
      - the Kengo name already exists in named_syms/symbol_addrs at a
        different address
      - asm/funcs/<kengo_name>.s already exists (BB2 has another
        function with that name not in the sym files — happens for
        symbols introduced by prior renames or extern declarations)."""
    by_kn: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        by_kn[r["kn"]].append(r)

    to_apply: list[dict] = []
    to_skip: list[dict] = []
    for kn, group in by_kn.items():
        existing_addr = existing.get(kn)
        existing_asm = (ASM_FUNCS / f"{kn}.s").exists()
        if len(group) == 1:
            row = group[0]
            if existing_addr is not None and existing_addr != row["addr"]:
                row["_skip_reason"] = (
                    f"name already used at 0x{existing_addr:08X}")
                to_skip.append(row)
            elif existing_asm:
                row["_skip_reason"] = (
                    f"asm/funcs/{kn}.s already exists for another func")
                to_skip.append(row)
            else:
                to_apply.append(row)
        else:
            if include_conflicts:
                for r in group:
                    r["_suffixed"] = f"{kn}_{r['addr']:08X}"
                    if (ASM_FUNCS / f"{r['_suffixed']}.s").exists():
                        r["_skip_reason"] = (
                            f"suffixed asm/funcs/{r['_suffixed']}.s exists")
                        to_skip.append(r)
                    else:
                        to_apply.append(r)
            else:
                for r in group:
                    r["_skip_reason"] = (
                        f"conflict: {len(group)} BB2 funcs claim {kn}")
                    to_skip.append(r)
    return to_apply, to_skip


def append_named_syms(to_apply: list[dict]) -> None:
    """Append `<name> = 0xADDR;` lines for each row. Preserves existing
    content; sorts new entries by address for readability."""
    to_apply_sorted = sorted(to_apply, key=lambda r: r["addr"])
    lines = NAMED_SYMS.read_text(errors="replace").splitlines()
    if lines and lines[-1] != "":
        lines.append("")
    lines.append("/* === Kengo-derived names (apply_kengo_names.py) === */")
    for r in to_apply_sorted:
        name = r.get("_suffixed") or r["kn"]
        lines.append(f"{name} = 0x{r['addr']:08X};")
    NAMED_SYMS.write_text("\n".join(lines) + "\n", encoding="utf-8",
                          newline="\n")


def substitute_in_sources(to_apply: list[dict]) -> dict[str, int]:
    """Word-boundary replace `func_XXXXXXXX` -> `<kengo_name>` across
    every file that names BB2 functions: src/*.c, include/*.h,
    regfix.txt, asmfix.txt, expand_lb_funcs.txt, sdata*.txt. Returns
    {file: count_replacements}.

    asm/funcs/*.s is excluded — splat regenerates it from the renamed
    symbol table on the next `make setup`.
    """
    plan = {r["bb2"]: r.get("_suffixed") or r["kn"] for r in to_apply}
    keys = "|".join(re.escape(k) for k in plan)
    pat = re.compile(rf"\b({keys})\b")

    candidates: list[Path] = []
    candidates.extend(sorted(SRC_DIR.glob("*.c")))
    candidates.extend(sorted((ROOT / "include").glob("*.h")))
    for name in ("regfix.txt", "asmfix.txt", "expand_lb_funcs.txt",
                 "sdata_funcs.txt", "sdata_exclude.txt"):
        p = ROOT / name
        if p.exists():
            candidates.append(p)

    edits: dict[str, int] = {}
    for p in candidates:
        text = p.read_text(encoding="utf-8", errors="replace")
        if "func_" not in text:
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
                    help="Actually edit files (default: dry-run plan only)")
    ap.add_argument("--min-opseq-ratio", type=float, default=0.30,
                    help="Min opseq ratio for seq-similarity rows (default 0.30)")
    ap.add_argument("--include-conflicts", action="store_true",
                    help="Apply N>1 BB2-to-same-Kengo cases with _<addr> suffix")
    args = ap.parse_args()

    if not CSV_IN.exists():
        print(f"ERROR: {CSV_IN} missing -- run tools/kengo_match.py first.",
              file=sys.stderr)
        return 1

    existing = load_existing_names()
    rows = parse_csv(args.min_opseq_ratio)
    to_apply, to_skip = classify(rows, existing, args.include_conflicts)

    by_conf = defaultdict(int)
    for r in to_apply:
        by_conf[r["conf"]] += 1

    print(f"Eligible BB2 anonymous funcs in CSV: {len(rows)}")
    print(f"  To apply: {len(to_apply)}")
    for conf in sorted(by_conf):
        print(f"    {conf:<25s} {by_conf[conf]}")
    print(f"  To skip:  {len(to_skip)}")
    skip_reasons: dict[str, int] = defaultdict(int)
    for r in to_skip:
        reason = r.get("_skip_reason", "?")
        # Bucket conflict reasons together.
        if reason.startswith("conflict:"):
            skip_reasons["conflict (>=2 BB2 funcs claim same name)"] += 1
        elif reason.startswith("name already used"):
            skip_reasons["name already in use at a different address"] += 1
        else:
            skip_reasons[reason] += 1
    for k, v in sorted(skip_reasons.items(), key=lambda x: -x[1]):
        print(f"    {k}: {v}")

    if not args.apply:
        print()
        print("(dry-run — pass --apply to write changes)")
        print()
        print("Sample of plan (first 20):")
        for r in sorted(to_apply, key=lambda x: x["addr"])[:20]:
            name = r.get("_suffixed") or r["kn"]
            print(f"  {r['bb2']:<22s} -> {name:<40s} "
                  f"({r['conf']}, opseq={r['opr']:.2f})")
        return 0

    append_named_syms(to_apply)
    edits = substitute_in_sources(to_apply)
    total_subst = sum(edits.values())

    # `make setup` (splat) is currently broken upstream (spimdisasm
    # comment-parser regression — unrelated to this work). Until that
    # is fixed we do the rename manually in three sweeps:
    #   1. rename asm/funcs/func_XXXX.s -> asm/funcs/<kengo>.s
    #      and update glabel/endlabel inside each
    #   2. rewrite `jal func_XXXX` callsites across all other asm files
    #      (asm/funcs/*.s, asm/6CAC.s, asm/data/*.s)
    asm_renames = 0
    asm_collisions = 0
    plan_map = {f"func_{r['addr']:08X}": (r.get('_suffixed') or r['kn'])
                for r in to_apply}
    for r in to_apply:
        addr = r["addr"]
        new_name = r.get("_suffixed") or r["kn"]
        old_path = ASM_FUNCS / f"func_{addr:08X}.s"
        new_path = ASM_FUNCS / f"{new_name}.s"
        if not old_path.exists():
            continue
        if new_path.exists():
            asm_collisions += 1
            continue
        body = old_path.read_text(encoding="utf-8", errors="replace")
        body = re.sub(rf"\bfunc_{addr:08X}\b", new_name, body)
        new_path.write_text(body, encoding="utf-8", newline="\n")
        old_path.unlink()
        asm_renames += 1

    # Sweep 2: scrub `jal func_XXXX` references across remaining asm.
    if plan_map:
        keys = "|".join(re.escape(k) for k in plan_map)
        pat = re.compile(rf"\b({keys})\b")
        asm_files: list[Path] = []
        asm_files.extend(sorted(ASM_FUNCS.glob("*.s")))
        asm_root = ASM_FUNCS.parent
        bulk = asm_root / "6CAC.s"
        if bulk.exists():
            asm_files.append(bulk)
        asm_files.extend(sorted((asm_root / "data").glob("*.s")))
        asm_callsite_refs = 0
        asm_callsite_files = 0
        for p in asm_files:
            body = p.read_text(encoding="utf-8", errors="replace")
            if "func_" not in body:
                continue
            new_body, n = pat.subn(lambda m: plan_map[m.group(1)], body)
            if n > 0:
                p.write_text(new_body, encoding="utf-8", newline="\n")
                asm_callsite_refs += n
                asm_callsite_files += 1

    print()
    print(f"Appended {len(to_apply)} names to {NAMED_SYMS.relative_to(ROOT)}")
    print(f"Substituted {total_subst} references across {len(edits)} files:")
    for name, n in sorted(edits.items()):
        print(f"  {name:<28s} {n}")
    print()
    print(f"Renamed {asm_renames} asm/funcs/*.s files in-place "
          f"(collisions: {asm_collisions})")
    if plan_map:
        print(f"Patched {asm_callsite_refs} `jal` callsites across "
              f"{asm_callsite_files} asm files")
    print()
    print("Verify: `wsl bash -c \"... && make clean && make\"` "
          "(no setup needed; we patched asm in lockstep)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
