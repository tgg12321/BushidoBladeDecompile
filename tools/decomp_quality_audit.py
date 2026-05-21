#!/usr/bin/env python3
"""Audit decompilation QUALITY: partition every function by HOW it matches.

Computed from the BUILT objects + config files (not source-text heuristics),
so it reflects exactly what links into the SHA1-matching binary.

KEY FACT this tool encodes: bb2.ld links .text ONLY from build/src/*.o; the
asm/*.s text files (6CAC/text1b/text2/...) and asm/funcs/*.s are reference-only
and never linked. Every shipped function is therefore C-defined; the axis of
progress is the QUALITY of each C definition:

  bridged    - replace_with_asmfile: original hand-disassembled bytes substituted
  inline_asm - body contains __asm__ (incl. file-scope `__asm__("glabel ...")`)
  assisted   - regfix / asmfix-surgical / register-asm pins (no inline asm)
  pure       - none of the above (clean C through the maspsx pipeline)

Requires: a completed `make` (build/bb2.elf + build/src/*.o) and
mipsel-linux-gnu-readelf on PATH (run under WSL). Override the readelf binary
with $READELF.

Usage:
  python3 tools/decomp_quality_audit.py            # full report
  python3 tools/decomp_quality_audit.py --summary  # one-line headline
  python3 tools/decomp_quality_audit.py --json     # machine-readable
"""
from __future__ import annotations
import argparse
import glob
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
READELF = os.environ.get("READELF", "mipsel-linux-gnu-readelf")


def _syms(obj: str):
    """Yield (name, type, bind, ndx) per symtab entry of an ELF object."""
    try:
        out = subprocess.run([READELF, "-sW", obj], capture_output=True,
                             text=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError) as exc:
        sys.exit(f"decomp_quality_audit: readelf failed on {obj}: {exc}\n"
                 f"  (need the WSL mipsel-linux-gnu toolchain; set $READELF to override)")
    for line in out.splitlines():
        p = line.split()
        if len(p) >= 8 and p[0].endswith(":") and p[0][:-1].isdigit():
            yield (p[7], p[3], p[4], p[6])      # name, type, bind, ndx


def _kind(name: str) -> str:
    if re.match(r"(D_|jtbl_|jpt_|jtab_|_D_|L_)", name):
        return "data"                            # jump table / data typed FUNC
    if re.search(r"_8[0-9A-Fa-f]{7}$", name) and not name.startswith("func_"):
        return "alt"                             # named func + address suffix
    return "real"


def _keyed(path: Path, only=None, exclude=None) -> set[str]:
    """Function names from `name: <verb> ...` rule files (active rules only)."""
    s: set[str] = set()
    if not path.exists():
        return s
    for ln in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        t = ln.strip()
        if not t or t.startswith("#"):
            continue
        m = re.match(r"([A-Za-z_]\w*)\s*:\s*(\S+)", t)
        if not m:
            continue
        if only and m.group(2) != only:
            continue
        if exclude and m.group(2) in exclude:
            continue
        s.add(m.group(1))
    return s


def _names(path: Path) -> set[str]:
    """Bare-function-name-per-line files (one name per line, # comments)."""
    s: set[str] = set()
    if not path.exists():
        return s
    for ln in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        t = ln.split("#")[0].strip()
        if t and re.fullmatch(r"[A-Za-z_]\w*", t):
            s.add(t)
    return s


def collect() -> dict:
    elf = ROOT / "build" / "bb2.elf"
    if not elf.exists():
        sys.exit("decomp_quality_audit: build/bb2.elf not found — run `make` first.")

    # Function universe = FUNC symbols defined across the linked src objects.
    func_obj: dict[str, str] = {}
    src_objs = sorted(glob.glob(str(ROOT / "build" / "src" / "*.o")))
    if not src_objs:
        sys.exit("decomp_quality_audit: no build/src/*.o — run `make` first.")
    for o in src_objs:
        for name, typ, bind, ndx in _syms(o):
            if typ == "FUNC" and ndx not in ("UND", "ABS") and not name.endswith(".NON_MATCHING"):
                func_obj.setdefault(name, o)
    allfunc = set(func_obj)
    kinds = defaultdict(set)
    for n in allfunc:
        kinds[_kind(n)].add(n)
    real = kinds["real"]

    # Config / cheat sets.
    regfix = _keyed(ROOT / "regfix.txt") | _keyed(ROOT / "regfix_stage2.txt")
    bridge = _keyed(ROOT / "asmfix.txt", only="replace_with_asmfile")
    surgical = _keyed(ROOT / "asmfix.txt", exclude={"replace_with_asmfile"})
    canon = _names(ROOT / "inline_asm_canonical.txt")

    # Inline __asm__ / register pins / file-scope glabel bodies.
    import classify_inline_asm as cia
    records = []
    for f in sorted((ROOT / "src").glob("*.c")):
        records.extend(cia.scan_file(f))
    asm_block, pins = set(), set()
    tierf = defaultdict(lambda: {"tier2": 0, "tier3": 0})
    for r in records:
        fn = r.get("func")
        if not fn:
            continue
        if r["kind"] == "asm_block":
            asm_block.add(fn); tierf[fn][r["tier"]] += 1
        elif r["kind"] == "register_pin":
            pins.add(fn)
    glabel = set()
    for f in sorted((ROOT / "src").glob("*.c")):
        for m in re.finditer(r"glabel\s+([A-Za-z_]\w*)", f.read_text(errors="ignore")):
            glabel.add(m.group(1))
    glabel &= allfunc
    inline = (asm_block | glabel) & allfunc
    pins &= allfunc

    # Mutually-exclusive partition (worst-quality wins).
    cat = {}
    for f in real:
        if f in bridge:
            cat[f] = "bridged"
        elif f in inline:
            cat[f] = "inline_asm"
        elif f in regfix or f in surgical or f in pins:
            cat[f] = "assisted"
        else:
            cat[f] = "pure"
    b = defaultdict(set)
    for f, c in cat.items():
        b[c].add(f)

    ia = b["inline_asm"]
    return {
        "allfunc": allfunc, "real": real, "kinds": kinds,
        "regfix": regfix, "bridge": bridge, "surgical": surgical,
        "canon": canon, "pins": pins, "inline": inline,
        "asm_block": asm_block, "glabel": glabel, "tierf": tierf,
        "buckets": b,
        "ia_t2": {f for f in ia if tierf[f]["tier2"] and not tierf[f]["tier3"]},
        "ia_t3": {f for f in ia if tierf[f]["tier3"] and not tierf[f]["tier2"]},
        "ia_mix": {f for f in ia if tierf[f]["tier2"] and tierf[f]["tier3"]},
        "ia_glabel": {f for f in ia if f in glabel and f not in asm_block},
        "ia_auth": ia & canon,
        "src_objs": src_objs,
    }


def summary_dict(d: dict) -> dict:
    b, real = d["buckets"], d["real"]
    return {
        "total_real_functions": len(real),
        "total_incl_alt_entries": len(real) + len(d["kinds"]["alt"]),
        "pure_c": len(b["pure"]),
        "assisted_c": len(b["assisted"]),
        "inline_asm": len(b["inline_asm"]),
        "bridged": len(b["bridged"]),
        "inline_asm_canonical_bodies": len(d["ia_glabel"]),
        "inline_asm_in_function_blocks": len(b["inline_asm"]) - len(d["ia_glabel"]),
        "feature_regfix": len(d["regfix"] & real),
        "feature_pins": len(d["pins"] & real),
        "feature_asmfix_surgical": len(d["surgical"] & real),
        "feature_bridged": len(d["bridge"] & real),
        "feature_any_inline_asm": len(d["inline"] & real),
    }


def render(d: dict) -> str:
    b, real = d["buckets"], d["real"]
    T = len(real)
    pc = lambda n: f"{100.0 * n / T:4.1f}%" if T else " n/a"
    o = []
    o.append("=" * 64)
    o.append("BB2 DECOMP QUALITY AUDIT  (asm/*.s text = reference-only; all C-defined)")
    o.append("=" * 64)
    o.append(f"distinct primary functions: {T}   (+{len(d['kinds']['alt'])} alt entries, "
             f"{len(d['kinds']['data'])} data-labels excluded)")
    o.append("")
    o.append("PARTITION (mutually exclusive):")
    for k, lab in [("pure", "Pure C (no cheats)"),
                   ("assisted", "Assisted C (regfix/asmfix/pins)"),
                   ("inline_asm", "Inline __asm__ in body"),
                   ("bridged", "Bridged (replace_with_asmfile)")]:
        o.append(f"   {lab:<36s} {len(b[k]):4d}  {pc(len(b[k]))}")
    o.append("")
    o.append(f"Inline-asm detail ({len(b['inline_asm'])}): "
             f"{len(d['ia_glabel'])} whole-fn glabel bodies, "
             f"{len(d['ia_t2'])} t2-only, {len(d['ia_mix'])} mixed, {len(d['ia_t3'])} t3-only; "
             f"{len(d['ia_auth'])} authorized")
    o.append("Feature usage (overlapping): "
             f"regfix={len(d['regfix']&real)}, pins={len(d['pins']&real)}, "
             f"asmfix-surgical={len(d['surgical']&real)}, bridged={len(d['bridge']&real)}, "
             f"any-inline-asm={len(d['inline']&real)}")
    return "\n".join(o)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--summary", action="store_true", help="one-line headline")
    ap.add_argument("--json", action="store_true", help="machine-readable JSON")
    args = ap.parse_args()
    d = collect()
    if args.json:
        print(json.dumps(summary_dict(d), indent=2))
        return 0
    if args.summary:
        s = summary_dict(d)
        print(f"Decomp quality: {s['pure_c']} pure C / {s['assisted_c']} assisted "
              f"/ {s['inline_asm']} inline-asm / {s['bridged']} bridged "
              f"(of {s['total_real_functions']} functions).")
        return 0
    print(render(d))
    return 0


if __name__ == "__main__":
    sys.exit(main())
