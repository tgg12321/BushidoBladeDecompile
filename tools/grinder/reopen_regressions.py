#!/usr/bin/env python3
"""One-time fold-in of the semantic-audit regressions (owner ruling 2026-07-06):
parse docs/fleet/regressions.md, re-open each function as an ordinary queue item,
and seed its grind ledger with the audit diagnosis. Idempotent: skips items
already in the queue or already ledgered. Run under WSL from the repo root:
    python3 tools/grinder/reopen_regressions.py [--dry-run]
"""
import argparse, glob, os, re, sys
sys.path.insert(0, os.getcwd())
from engine import queue as Q                      # noqa: E402
sys.path.insert(0, os.path.join(os.getcwd(), "tools", "grinder"))
import grindlib as G                               # noqa: E402

LINE = re.compile(r"^- (?P<ts>\S+)\s+\*\*(?P<func>[^*]+)\*\*\s*[—–-]\s*(?P<diag>.+)$")

# The 2026-06-24 sweep's "Cleaned (6)" table (docs/fleet/regressions.md, Resolution
# log section) landed actual cheat-cleanup commits for these functions and none of
# them recur in a later (post-sweep) finding — so they are genuinely resolved, not
# just deferred. Everything else that was touched by the sweep (the 3 WIP-stubs,
# the 10 deferred-hard functions) is still cheat-carrying and must be reopened.
RESOLVED = {
    "func_80065680", "func_80044170", "func_80044DE4",
    "func_8006C168", "func_80075830", "saTan0Main",
}

# func_80037F08_ret (regressions.md line ~103) names a __asm__(".globl .../ X = Y")
# alias symbol, not a real C function — there is no "func_80037F08_ret(...) {" body
# to find_file_stem(). The alias points at motion_LoadPreCalcData_80037F08
# (src/code6cac_c_mid.c:184, asm/funcs/motion_LoadPreCalcData_80037F08.s), which is
# the actual queue/asm function name and the one the fix touches. Remap so the
# finding lands on the real function instead of being skipped as unresolvable.
FUNC_REMAP = {
    "func_80037F08_ret": "motion_LoadPreCalcData_80037F08",
}


DEF_PAT_TEMPLATE = r"^\w[\w\s\*]*\b{func}\s*\([^;{{}}]*\)\s*(\{{|;)"


def find_file_stem(func):
    """Locate the src file that DEFINES func (body, i.e. the signature is
    terminated by '{' — not just a forward-declared/extern prototype, which
    is terminated by ';' and is NOT the defining file)."""
    pat = re.compile(DEF_PAT_TEMPLATE.format(func=re.escape(func)), re.M)
    hits = []
    for path in glob.glob("src/*.c"):
        text = open(path, encoding="utf-8", errors="replace").read()
        for m in pat.finditer(text):
            if m.group(1) == "{":
                hits.append(os.path.splitext(os.path.basename(path))[0])
                break
    if len(hits) == 1:
        return hits[0]
    return hits[0] if hits else None   # multiple hits: first; verify manually


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    latest = {}
    for line in open("docs/fleet/regressions.md", encoding="utf-8"):
        m = LINE.match(line.strip())
        if m:
            latest[m.group("func").strip()] = m.group("diag").strip()

    excluded = sorted(f for f in RESOLVED if f in latest)
    for f in excluded:
        del latest[f]
    if excluded:
        print(f"excluded {len(excluded)} resolved-in-sweep function(s): {', '.join(excluded)}")

    remapped = sorted(f for f in FUNC_REMAP if f in latest)
    for f in remapped:
        latest[FUNC_REMAP[f]] = latest.pop(f)
    if remapped:
        print(f"remapped {len(remapped)} alias finding(s) to their real function: "
              + ", ".join(f"{f} -> {FUNC_REMAP[f]}" for f in remapped))

    print(f"parsed {len(latest)} unique outstanding regression functions")
    done = skipped = failed = 0
    for func, diag in sorted(latest.items()):
        stem = find_file_stem(func)
        if not stem:
            print(f"  SKIP {func}: no src definition found (resolve manually)")
            failed += 1
            continue
        if a.dry_run:
            print(f"  DRY  {func} -> {stem}")
            continue
        r = Q.reopen(func, stem, reason=f"regression: {diag[:200]}")
        if not r.get("ok"):
            print(f"  SKIP {func}: {r.get('reason')}")
            skipped += 1
            continue
        G.init_ledger(".", func, stem, origin="regression")
        G.add_judge_constraint(".", func,
                               f"REGRESSION DIAGNOSIS (completion bar: match must land "
                               f"WITHOUT this construct family): {diag}")
        G.append_evidence(".", func, f"Audit diagnosis (regressions.md): {diag}")
        done += 1
        print(f"  OK   {func} -> {stem}")
    print(f"\nreopened {done}, skipped {skipped}, unresolved {failed}")


if __name__ == "__main__":
    main()
