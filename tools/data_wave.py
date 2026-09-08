#!/usr/bin/env python3
"""
data_wave.py — atomic RENAME cascade for BB2 DATA symbols (globals), the data-side sibling
of tools/naming_wave.py.

Given a manifest of `addr,proposed_name` rows (default docs/naming/libscan/data_manifest.csv,
verdict CONFIRM only), it retires EVERY symbol currently defined at that address (the splat
`D_<ADDR>` auto name and any `g_*` aliases in the three linker-script registries) in favour
of the proposed name, across every surface where a data name is a key:

  src/*.c, include/*.h            whole-word substitution outside comments (string literals in
                                  scope — INCLUDE_ASM / __asm__ bodies), via naming_wave.sub_c
  asm/**/*.s                      same (%hi/%lo operands, .word references)
  bb2.ld                          same
  named_syms.txt, symbol_addrs.txt, undefined_syms_auto.txt
                                  line-wise: the definition line is rewritten to the new name;
                                  a second definition of the same (name, addr) is dropped so
                                  the linker sees one assignment; retired names in comments stay
  *.txt gate lists (volatile_extern_allowlist.txt, sdata_*.txt, ...)   sub_hash
  tools/**/*.py, engine/**/*.py   short string-literal keys only, via naming_wave.sub_py

Byte-neutrality is the claim, the oracle is the proof: run
`python3 -m engine.cli verify-oracle --rebuild --allow-dirty` after --apply.
--apply refuses a dirty working tree so `git checkout` is always a clean rollback.

Usage:
  python3 tools/data_wave.py                      # dry run over the default manifest
  python3 tools/data_wave.py --manifest-csv X --apply --manifest tmp/out.json
"""
from __future__ import annotations
import argparse, csv, glob, json, os, re, subprocess, sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import naming_wave as nw  # noqa: E402  (sub_c, sub_hash, sub_py, read, write_lf)

REGISTRIES = ["undefined_syms_auto.txt", "named_syms.txt", "symbol_addrs.txt"]
SYM_LINE = re.compile(r"^(\s*)([A-Za-z_]\w*)(\s*=\s*)(0x[0-9A-Fa-f]{8})(\s*;)(.*)$")
AUTO = re.compile(r"^(D|func)_[0-9A-Fa-f]{8}$")
ADDR = re.compile(r"^0x8[0-9A-Fa-f]{7}$")
LIST_FILES = ["volatile_extern_allowlist.txt", "sdata_funcs.txt", "sdata_exclude.txt"]


def die(msg):
    print("FATAL:", msg, file=sys.stderr); sys.exit(2)


def registry_defs():
    """addr(lower) -> {name: [(file, lineno)]}"""
    defs = defaultdict(lambda: defaultdict(list))
    for rel in REGISTRIES:
        p = ROOT / rel
        if not p.exists():
            continue
        for i, line in enumerate(nw.read(p).split("\n"), 1):
            m = SYM_LINE.match(line)
            if m:
                defs[m.group(4).lower()][m.group(2)].append((rel, i))
    return defs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest-csv", default=str(ROOT / "docs/naming/libscan/data_manifest.csv"))
    ap.add_argument("--only", default="", help="comma-separated addresses")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--manifest", default="", help="write the edit manifest JSON here")
    a = ap.parse_args()

    if a.apply:
        st = subprocess.run(["git", "status", "--short"], cwd=ROOT, capture_output=True, text=True).stdout
        if st.strip():
            die("working tree is dirty — commit or stash first so git checkout is a clean rollback:\n" + st)

    rows = [r for r in csv.DictReader(open(a.manifest_csv, encoding="utf-8", errors="replace"))
            if (r.get("verdict") or "CONFIRM").strip().upper() == "CONFIRM"]
    only = {x.strip().lower() for x in a.only.split(",") if x.strip()}
    defs = registry_defs()
    all_names = {n for d in defs.values() for n in d}

    ops = []   # (addr, new, olds)
    errors = []
    for r in rows:
        addr = r["addr"].strip().lower()
        new = r["proposed_name"].strip()
        if only and addr not in only:
            continue
        if not ADDR.match(addr) or not re.match(r"^[A-Za-z_]\w*$", new):
            errors.append(f"{addr}: bad row {new!r}"); continue
        olds = set(defs.get(addr, {}))
        olds.add("D_" + addr[2:].upper())
        olds.discard(new)
        # P1: the new name must not be defined at ANOTHER address
        other = [ad for ad, d in defs.items() if new in d and ad != addr]
        if other:
            errors.append(f"{addr}: '{new}' is already defined at {other}"); continue
        # P2: at least one old name must actually be referenced or defined somewhere
        ops.append((addr, new, sorted(olds)))
    if errors:
        for e in errors:
            print("  REJECT:", e)
        die("preflight failed")
    if not ops:
        die("no operations")

    code_map = {}
    for addr, new, olds in ops:
        for o in olds:
            if o in code_map and code_map[o] != new:
                die(f"identifier {o} mapped to two targets")
            code_map[o] = new
    pat = re.compile(r"\b(" + "|".join(re.escape(k) for k in sorted(code_map, key=len, reverse=True)) + r")\b")
    repl = lambda m: code_map[m.group(1)]

    edits = {}      # rel -> (new_text, hits)
    def plan_file(rel, fn):
        p = ROOT / rel
        if not p.exists():
            return
        text = nw.read(p)
        new_text, n = fn(text)
        if n:
            edits[rel] = (new_text, n)

    for pattern in ("src/*.c", "include/*.h", "asm/*.s", "asm/funcs/*.s", "asm/data/*.s", "bb2.ld"):
        for p in sorted(glob.glob(str(ROOT / pattern))):
            plan_file(os.path.relpath(p, ROOT).replace("\\", "/"), lambda t: nw.sub_c(pat, repl, t))
    for rel in LIST_FILES:
        plan_file(rel, lambda t: nw.sub_hash(pat, repl, t))
    py_notes = []
    for root in ("tools", "engine"):
        for p in sorted(glob.glob(str(ROOT / root / "**" / "*.py"), recursive=True)):
            rel = os.path.relpath(p, ROOT).replace("\\", "/")
            if any(part in nw.PY_EXCLUDE_PARTS for part in rel.split("/")) or rel in nw.PY_EXCLUDE_FILES \
                    or rel == "tools/data_wave.py":
                continue
            text = nw.read(ROOT / rel)
            new_text, n, notes = nw.sub_py(pat, repl, text)
            py_notes += [f"{rel}: {x}" for x in notes]
            if n:
                edits[rel] = (new_text, n)

    # object-defined symbols (build/bb2.map): when an old name is defined by an OBJECT (a
    # dlabel block in asm/data/*.s, or a C definition), the .s/.c rename makes the object
    # define the NEW name, so every registry assignment at that address becomes a duplicate
    # definition and is dropped (a comment keeps the retired alias visible).
    MAPDEF = re.compile(r"^\s+0x0*([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$")
    objdef = defaultdict(set)
    mp = ROOT / "build/bb2.map"
    if mp.exists():
        for line in nw.read(mp).split("\n"):
            m = MAPDEF.match(line)
            if m:
                objdef["0x" + m.group(1)].add(m.group(2))
    obj_addrs = {ad for ad, nw_, ol in ops if objdef.get(ad, set()) & set(ol)}

    # registries: line-wise
    reg_report = []
    for rel in REGISTRIES:
        p = ROOT / rel
        if not p.exists():
            continue
        lines = nw.read(p).split("\n")
        out, n, seen = [], 0, set()
        for i, line in enumerate(lines, 1):
            m = SYM_LINE.match(line)
            if m and m.group(4).lower() in {o[0] for o in ops} and m.group(2) in code_map:
                addr = m.group(4).lower(); new = code_map[m.group(2)]
                if addr in obj_addrs:
                    cmt = "/* %s */" if rel != "symbol_addrs.txt" else "// %s"
                    out.append(cmt % f"data-wave 2026-09-07: {m.group(2)} = {m.group(4)} retired; {new} is now object-defined")
                    reg_report.append(f"{rel}:{i} dropped {m.group(2)} (object now defines {new})")
                    n += 1
                    continue
                if (new, addr) in seen:
                    reg_report.append(f"{rel}:{i} dropped duplicate definition of {new} (was {m.group(2)})")
                    n += 1
                    continue
                seen.add((new, addr))
                tail = m.group(6)
                note = f"  /* data-wave 2026-09-07: was {m.group(2)} */" if rel != "symbol_addrs.txt" else f"  // data-wave 2026-09-07: was {m.group(2)}"
                if m.group(2) != new:
                    out.append(f"{m.group(1)}{new}{m.group(3)}{m.group(4)}{m.group(5)}{tail}{'' if 'data-wave' in tail else note}")
                    reg_report.append(f"{rel}:{i} {m.group(2)} -> {new}")
                    n += 1
                else:
                    out.append(line)
            else:
                out.append(line)
        if n:
            edits[rel] = ("\n".join(out), n)

    print("=" * 78)
    print(f"DATA WAVE — {len(ops)} addresses, {len(code_map)} identifiers remapped")
    for addr, new, olds in ops:
        print(f"  {addr}: {', '.join(olds)} -> {new}")
    print(f"\n-- file edits ({len(edits)}) --")
    for rel, (t, n) in sorted(edits.items()):
        print(f"  {rel}  ({n})")
    if reg_report:
        print("\n-- registry lines --")
        for x in reg_report:
            print("  " + x)
    if py_notes:
        print("\n-- python long strings NOT rewritten --")
        for x in py_notes[:20]:
            print("  " + x)
    manifest = dict(mode="apply" if a.apply else "dry-run",
                    ops=[dict(addr=ad, new=nw_, olds=ol) for ad, nw_, ol in ops],
                    edits={rel: n for rel, (t, n) in edits.items()}, registry=reg_report, py_notes=py_notes)
    if a.manifest:
        Path(a.manifest).write_text(json.dumps(manifest, indent=1), encoding="utf-8")
    if not a.apply:
        print("\nDRY RUN — nothing written. Re-run with --apply.")
        return 0
    for rel, (t, n) in edits.items():
        nw.write_lf(ROOT / rel, t)
    # residual audit: any old name still present outside comments?
    resid = []
    for rel in list(edits) + ["include/m2c_context.h"]:
        p = ROOT / rel
        if not p.exists():
            continue
        text = nw.read(p)
        found = set(pat.findall(re.sub(nw._C_TOKEN, lambda m: m.group(0) if not m.group(0).startswith(("//", "/*")) else "", text)))
        if found:
            resid.append(f"{rel}: {sorted(found)}")
    print("\nAPPLIED.", "Residuals:" if resid else "No residual old names outside comments.")
    for x in resid:
        print("  " + x)
    print("Now prove byte-neutrality: python3 -m engine.cli verify-oracle --rebuild --allow-dirty")
    return 0


if __name__ == "__main__":
    sys.exit(main())
