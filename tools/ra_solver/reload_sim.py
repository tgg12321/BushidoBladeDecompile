#!/usr/bin/env python3
"""ra_solver.reload_sim — replicate global.c find_reg on RETRYING calls.

Campaign 7 (2026-08-06).  This is the reload-side companion to `simulate.py`:
`simulate.py` reproduces the initial `global_alloc` allocation (retry=0);
this reproduces what `retry_global_alloc` does to a pseudo after reload's
spill loop evicted it from its hard register.

Two validation levels, both run by `--check`:

  SCAN   Replay find_reg's register scan from the *dumped* exclusion sets
         (pass0_used / pass1_used / own_copy_prefs / own_full_prefs) and
         check the resulting best_reg against the dump.  This validates the
         scan + the two preference-upgrade stages, and in particular the
         subtlety that the upgrade filters against `used` — which is the
         PASS-0 set when pass 0 succeeded and the PASS-1 set otherwise —
         not against used1 unconditionally.

  SETS   Reconstruct pass0_used / pass1_used from find_reg's own recipe
         (global.c:958-990) out of the primitives the RETRYDBG entry gives us
         — conflicts, forbidden (= the `losers` argument), used_so_far,
         someone_prefers, calls-crossed, class — and check them bit-for-bit
         against the dumped sets.  This validates the set CONSTRUCTION, i.e.
         that `forbidden_regs` and the grown conflict set are the only
         reload-side inputs.

Usage:
  python3 tools/ra_solver/reload_sim.py --check [stem ...]
  python3 tools/ra_solver/reload_sim.py --show <stem> <func>
"""
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from simulate import (FIRST_PSEUDO, FIXED, NO_GLOBAL, CALL_USED, GR_REGS,
                      mode_ok)                                   # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "reload_work"

# config/mips/mips.h REG_CLASS_CONTENTS, restricted to the 0..67 registers the
# dumps cover.  Class indices follow `enum reg_class` in mips.h:
#   0 NO_REGS  1 GR_REGS  2 FP_REGS  3 HI_REG  4 LO_REG
#   5 HILO_REG 6 MD_REGS  7 ST_REGS  8 ALL_REGS
CLASS_CONTENTS = {
    0: set(),
    1: set(range(0, 32)),
    2: set(range(32, 64)),
    3: {64},
    4: {65},
    5: {66},
    6: {64, 65},
    7: {67},
    8: set(range(0, 68)),
}

# CALLER_SAVE_PROFITABLE(REFS, CALLS) — mips.h / defaults.h:
#   4 * CALLS < REFS
def caller_save_profitable(refs, calls):
    return 4 * calls < refs


def scan(used, size, mode_two_word):
    """find_reg's ascending register scan (no REG_ALLOC_ORDER on MIPS)."""
    i = 0
    while i < FIRST_PSEUDO:
        regno = i
        if regno not in used and (regno in GR_REGS) and \
                (not mode_two_word or regno % 2 == 0):
            lim = regno + size
            j = regno + 1
            while j < lim and j not in used:
                j += 1
            if j == lim:
                return regno
            i = j                 # skip starting points we know will lose
        else:
            i += 1
    return -1


def upgrade(prefs, used, size, mode_two_word, best_reg):
    """One preference-upgrade stage.  Candidates are PREFS minus USED; the
    first that fits (ascending) wins.  Class-compat tests are vacuous on MIPS
    (every GPR is GR_REGS)."""
    if best_reg < 0:
        return -1
    cands = set(prefs) - set(used)
    if not cands:
        return -1
    for r in sorted(cands):
        if r not in GR_REGS:
            continue
        if mode_two_word and r % 2:
            continue
        lim = r + size
        j = r + 1
        while j < lim and j not in used:
            j += 1
        if j == lim:
            return r
    return -1


def replay_scan(fr):
    """SCAN level: reproduce best_reg from the dumped exclusion sets."""
    sets = fr["sets"]
    size = fr.get("size", 1)
    two = size > 1
    used0 = set(sets["pass0_used"])
    used1 = set(sets["pass1_used"])

    best = scan(used0, size, two)
    used = used0
    if best < 0:
        best = scan(used1, size, two)
        used = used1

    # copy preferences first; a hit does `goto no_prefs` and SKIPS the
    # plain-preference stage entirely.
    r = upgrade(sets.get("own_copy_prefs", []), used, size, two, best)
    if r < 0:
        r = upgrade(sets.get("own_full_prefs", []), used, size, two, best)
    if r >= 0:
        best = r
    return best


def build_sets(retry, fr):
    """SETS level: rebuild used1/used0 from find_reg's recipe.

        acc ? call_fixed_reg_set : (calls_crossed == 0 ? fixed : call_used)
        | no_global_alloc_regs | losers | ~reg_class_contents[class]
        -> used2 (the pre-conflict set)
        | hard_reg_conflicts -> used1
        used0 = used1 | ~regs_used_so_far | regs_someone_prefers
    """
    sets = fr["sets"]
    if fr["acc"]:
        # call_fixed_reg_set = fixed_regs | call_used that are also fixed;
        # on MIPS call_fixed_reg_set == fixed_reg_set (no CONDITIONAL_REGISTER
        # _USAGE additions), so accepting call-clobbered regs excludes only
        # the fixed ones.
        base = set(FIXED)
    elif retry["calls"] == 0:
        base = set(FIXED)
    else:
        base = set(CALL_USED)
    used2 = base | set(NO_GLOBAL) | set(retry["forbidden"])
    used2 |= (set(range(FIRST_PSEUDO))
              - CLASS_CONTENTS.get(fr.get("class", 1), GR_REGS))
    used1 = used2 | set(sets["conflicts"])
    used0 = used1 | (set(range(FIRST_PSEUDO)) - set(sets["used_so_far"])) \
        | set(sets["someone_prefers"])
    return used2, used1, used0


def check(stems):
    scan_ok = scan_bad = 0
    set_ok = set_bad = 0
    failures = []
    for stem in stems:
        p = WORK / f"{stem}.reload.json"
        if not p.exists():
            continue
        funcs = json.loads(p.read_text())
        for fname, f in funcs.items():
            for retry in f["retries"]:
                for fr in retry["find_reg"]:
                    if "best_reg" not in fr or "pass0_used" not in fr["sets"]:
                        continue
                    got = replay_scan(fr)
                    if got == fr["best_reg"]:
                        scan_ok += 1
                    else:
                        scan_bad += 1
                        failures.append(
                            f"SCAN {stem}:{fname} p{retry['pseudo']} "
                            f"alt={fr['alt']} acc={fr['acc']} "
                            f"sim={got} dump={fr['best_reg']}")
                    u2, u1, u0 = build_sets(retry, fr)
                    d1 = set(fr["sets"]["pass1_used"])
                    d0 = set(fr["sets"]["pass0_used"])
                    d2 = set(fr["sets"].get("used2_noconflict", []))
                    # the dumps only cover regs 0..31
                    m = set(range(32))
                    if (u1 & m) == (d1 & m) and (u0 & m) == (d0 & m) \
                            and (u2 & m) == (d2 & m):
                        set_ok += 1
                    else:
                        set_bad += 1
                        failures.append(
                            f"SETS {stem}:{fname} p{retry['pseudo']} "
                            f"used1 sim={sorted(u1 & m)} dump={sorted(d1 & m)} "
                            f"used0 sim={sorted(u0 & m)} dump={sorted(d0 & m)}")
    print(f"SCAN level: {scan_ok}/{scan_ok + scan_bad} find_reg retry calls "
          f"reproduce best_reg exactly")
    print(f"SETS level: {set_ok}/{set_ok + set_bad} exclusion-set "
          f"reconstructions are bit-identical")
    for line in failures[:40]:
        print("  " + line)
    return 0 if not (scan_bad or set_bad) else 1


def show(stem, func):
    funcs = json.loads((WORK / f"{stem}.reload.json").read_text())
    f = funcs[func]
    print(f"{func}: prr={f['prr']}  bad_spill={f['bad_spill_regs']}")
    for n in f["needs"]:
        print(f"  needs pass={n['pass']} {n['classes']} "
              f"new_bb={n.get('new_bb_needs')} changed={n.get('changed')}")
    for s in f["spills"]:
        print(f"  new_spill_reg pass={s['pass']} idx={s['idx']} "
              f"regno={s['regno']} class={s['class']} need={s['need']}")
    for k in f["kickouts"]:
        print(f"  kickout pass={k['pass']} spillreg={k['spillreg']} "
              f"pseudo={k['pseudo']} had={k['had']} nrefs={k['nrefs']}")
    for r in f["retries"]:
        print(f"  RETRY pseudo={r['pseudo']} had={r.get('had')} "
              f"spillreg={r.get('spillreg')} nrefs={r['nrefs']} "
              f"livelen={r['livelen']} calls={r['calls']} -> got={r['got']}")
        print(f"    forbidden={r.get('forbidden')}")
        print(f"    conflicts={r.get('conflicts')}")
        for fr in r["find_reg"]:
            s = fr["sets"]
            print(f"    find_reg alt={fr['alt']} acc={fr['acc']} "
                  f"best={fr.get('best_reg')} sim={replay_scan(fr)}")
            print(f"      pass0_used={s.get('pass0_used')}")
            print(f"      pass1_used={s.get('pass1_used')}")
            print(f"      copy_prefs={s.get('own_copy_prefs')} "
                  f"full_prefs={s.get('own_full_prefs')} "
                  f"someone={s.get('someone_prefers')}")


REGNAME = (["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3"]
           + [f"t{i}" for i in range(8)] + [f"s{i}" for i in range(8)]
           + ["t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"])


def target(stem, func, pseudo, want):
    """Inverse solver: what has to change for PSEUDO's retry to yield WANT.

    The retry outcome is closed-form (see the module docstring and the
    validation numbers): preferences are always empty by retry time, so

        got = min { r : r not in base | forbidden | ~class | conflicts }

    which makes the inverse a set statement, not a search:
      * every register below WANT must be excluded, and
      * WANT itself must not be.
    """
    funcs = json.loads((WORK / f"{stem}.reload.json").read_text())
    hits = [r for r in funcs[func]["retries"] if r["pseudo"] == pseudo]
    if not hits:
        print(f"{func}: pseudo {pseudo} has no retry_global_alloc call")
        return 1
    for r in hits:
        fr = [x for x in r["find_reg"] if x.get("best_reg", -1) >= 0]
        fr = fr[-1] if fr else r["find_reg"][-1]
        used = set(fr["sets"]["pass1_used"])
        base = set(FIXED) if (fr["acc"] or r["calls"] == 0) else set(CALL_USED)
        cls = CLASS_CONTENTS.get(fr.get("class", 1), GR_REGS)
        print(f"{func} pseudo {pseudo}: got={r['got']} "
              f"({REGNAME[r['got']] if 0 <= r['got'] < 32 else r['got']}), "
              f"want={want} ({REGNAME[want] if 0 <= want < 32 else want})")
        if want not in cls:
            print(f"  IMPOSSIBLE: ${REGNAME[want]} is outside the class this "
                  f"find_reg call scans")
            continue
        must_free = [want] if want in used else []
        must_block = [r_ for r_ in sorted(cls) if r_ < want and r_ not in used]
        why_free = []
        for r_ in must_free:
            src = []
            if r_ in base:
                src.append("base (fixed/call-used — set by calls-crossed)")
            if r_ in set(r["forbidden"]):
                src.append("forbidden_regs (reload spilled it)")
            if r_ in set(fr["sets"]["conflicts"]):
                src.append("hard_reg_conflicts (a live-range overlap)")
            why_free.append(f"    ${REGNAME[r_]}: excluded by " +
                            "; ".join(src or ["?"]))
        print(f"  must become FREE : {[REGNAME[x] for x in must_free]}")
        for line in why_free:
            print(line)
        print(f"  must become BLOCKED: {[REGNAME[x] for x in must_block]}")
        if not must_free and not must_block:
            print("  already the answer")
    return 0


def main(argv):
    if "--target" in argv:
        i = argv.index("--target")
        return target(argv[i + 1], argv[i + 2], int(argv[i + 3]),
                      int(argv[i + 4]))
    if "--show" in argv:
        i = argv.index("--show")
        show(argv[i + 1], argv[i + 2])
        return 0
    stems = [a for a in argv[1:] if not a.startswith("--")]
    if not stems:
        stems = sorted(p.name[:-len(".reload.json")]
                       for p in WORK.glob("*.reload.json"))
    return check(stems)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
