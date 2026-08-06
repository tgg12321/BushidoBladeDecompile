#!/usr/bin/env python3
"""inverse_reload — the inverse solver's RELOAD backend.

The smallest of the three backends, because Campaign 7 proved the retry
outcome is CLOSED FORM.  Preferences are destructively consumed during
global_alloc, so they are empty in 0/194 observed retry calls, and the
pass-0/pass-1 split never changes the answer:

    got = min { r : r not in base | forbidden_regs | ~class | conflicts }
    base = fixed_reg_set (calls_crossed == 0 or acc) | call_used_reg_set

So the inverse is not a search — it is a set statement.  For a wanted register
W the requirement is exactly:

    W must not be excluded, and every register below W in the class must be.

`reload_sim.py --target` already prints that statement.  This module adds what
the other backends have and it did not: each requirement is TYPED by which
model input produces it, mapped to a C lever through levers.py with the
standing forbidden block, and the whole thing is falsifiable — `--selftest`
applies the computed requirements back to the exclusion sets and re-runs the
forward scan to confirm the wanted register actually comes out.

Scope note (2026-08-06): the twins (func_8007C7A0 / func_8007C86C) and
saTan4FireDisp reload routes are CLOSED — measured dead in Campaign 7 and not
to be reopened.  This backend exists for FUTURE residuals that reach reload.

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/inverse_reload.py --selftest [stem ...]
  python3 tools/ra_solver/inverse_reload.py --target <stem> <func> <pseudo> <reg>
  python3 tools/ra_solver/inverse_reload.py --list <stem>
"""
import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import levers as L                                             # noqa: E402
import reload_sim as R                                         # noqa: E402
from simulate import FIRST_PSEUDO, FIXED, CALL_USED, GR_REGS   # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "reload_work"

REGNAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
            "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
            "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]


def rname(r):
    return f"${REGNAMES[r]}" if 0 <= r < 32 else f"r{r}"


def last_find_reg(retry):
    fr = [x for x in retry["find_reg"] if x.get("best_reg", -1) >= 0]
    return fr[-1] if fr else retry["find_reg"][-1]


def requirements(retry, want):
    """-> (ok, must_free, must_block, why) for one retry call.

    must_free   registers that are excluded today and must stop being
    must_block  registers that are free today and must become excluded
    why         {reg: [(class, reason)]} attribution per register
    """
    fr = last_find_reg(retry)
    # BOTH exclusion sets matter.  find_reg scans pass 0 FIRST and only falls
    # through to pass 1 when pass 0 finds nothing, so a requirement derived
    # from pass1_used alone is wrong whenever the two sets differ: the wanted
    # register can be free in pass 1 yet excluded in pass 0, and pass 0 then
    # returns something else entirely.  (Measured: asking marionation_camera_Exec
    # pseudo 95 for $s2..$s7 came back $t8 until this was fixed.)  For the
    # wanted register to be the answer it must be free in whichever pass wins,
    # so require it free in both and every lower register blocked in both.
    used0 = set(fr["sets"]["pass0_used"])
    used1 = set(fr["sets"]["pass1_used"])
    cls = R.CLASS_CONTENTS.get(fr.get("class", 1), GR_REGS)
    base = set(FIXED) if (fr["acc"] or retry["calls"] == 0) else set(CALL_USED)
    forbidden = set(retry["forbidden"])
    conflicts = set(fr["sets"]["conflicts"])
    used_so_far = set(fr["sets"].get("used_so_far", []))
    someone = set(fr["sets"].get("someone_prefers", []))

    if want not in cls:
        return False, [], [], {want: [(L.CLASS_CHANGE,
                                       "outside the register class this "
                                       "find_reg call scans")]}
    # "Free in both passes" is SUFFICIENT but not NECESSARY: when pass 0 finds
    # nothing at all, pass 1 decides, and the wanted register may legitimately
    # be excluded in pass 0.  So ask the forward model first — if the current
    # sets already produce WANT, there is nothing to require.
    size = fr.get("size", 1)
    cur = R.scan(used0, size, size > 1)
    if cur < 0:
        cur = R.scan(used1, size, size > 1)
    if cur == want:
        return True, [], [], {}

    must_free = [want] if want in (used0 | used1) else []
    must_block = [r for r in sorted(cls)
                  if r < want and (r not in used0 or r not in used1)]

    why = {}
    for r in must_free:
        src = []
        if r in base:
            src.append((L.CALLS_CROSSED,
                        "base set: fixed/call-used, decided by calls-crossed"))
        if r in forbidden:
            src.append((L.RELOAD_FORBIDDEN,
                        "forbidden_regs — reload spilled it for this insn"))
        if r in conflicts:
            src.append((L.CONFLICT_DROP,
                        "hard_reg_conflicts — a live-range overlap"))
        # pass-0-only exclusions: find_reg's pass 0 additionally skips
        # registers never used so far in the function and those a
        # lower-priority conflicting allocno prefers.
        if r not in used1 and r in used0:
            if r not in used_so_far:
                src.append((L.RELOAD_PRESSURE,
                            "pass-0 only: not in regs_used_so_far — the "
                            "function has not touched this register yet"))
            if r in someone:
                src.append((L.PREF_REROUTE,
                            "pass-0 only: someone_prefers — a lower-priority "
                            "conflicting allocno prefers it"))
        why[r] = src or [(L.RELOAD_FORBIDDEN, "excluded, source unattributed")]
    for r in must_block:
        why[r] = [(L.CONFLICT_ADD,
                   "currently free and below the target — needs a genuine "
                   "live-range overlap"),
                  (L.RELOAD_FORBIDDEN,
                   "or reload must spill it instead, which is register "
                   "pressure at this insn")]
    return True, must_free, must_block, why


def apply_and_rescan(retry, must_free, must_block):
    """Falsifiability: apply the requirements to the dumped exclusion sets and
    re-run the forward scan.  Returns the register the model then produces."""
    fr = last_find_reg(retry)
    size = fr.get("size", 1)
    used0 = set(fr["sets"]["pass0_used"]) - set(must_free) | set(must_block)
    used1 = set(fr["sets"]["pass1_used"]) - set(must_free) | set(must_block)
    best = R.scan(used0, size, size > 1)
    if best < 0:
        best = R.scan(used1, size, size > 1)
    return best


def report(stem, func, pseudo, want):
    funcs = json.loads((WORK / f"{stem}.reload.json").read_text())
    if func not in funcs:
        sys.exit(f"{func} not in {stem}.reload.json")
    hits = [r for r in funcs[func]["retries"] if r["pseudo"] == pseudo]
    if not hits:
        print(f"{func}: pseudo {pseudo} has no retry_global_alloc call — it "
              f"was never evicted, so the PRE-reload model governs "
              f"(tools/ra_solver/inverse.py global).")
        return 1
    for retry in hits:
        print(f"{func} pseudo {pseudo}: got {rname(retry['got'])}, "
              f"want {rname(want)}  (calls_crossed={retry['calls']})")
        if retry["got"] == want:
            print("  already the answer — nothing to invert.")
            continue
        ok, free_, block_, why = requirements(retry, want)
        if not ok:
            print(f"  IMPOSSIBLE: {rname(want)} is outside this call's class.")
            print(L.format_report([L.CLASS_CHANGE]))
            continue
        got = apply_and_rescan(retry, free_, block_)
        print(f"  must become FREE   : {[rname(r) for r in free_]}")
        print(f"  must become BLOCKED: {[rname(r) for r in block_]}")
        print(f"  verification: applying exactly that to the exclusion sets "
              f"and re-running the scan yields {rname(got)} "
              f"({'OK' if got == want else 'MISMATCH'})")
        classes = []
        print()
        for r in free_ + block_:
            print(f"  {rname(r)}:")
            for cls, reason in why[r]:
                print(f"    [{cls}] {reason}")
                classes.append(cls)
        print()
        print(L.format_report(sorted(set(classes))))
        print()
        print(L.forbidden_block())
    return 0


def selftest(stems):
    """Validate the inverse against the forward model on every retry call.

    Two properties, both falsifiable:
      IDENTITY  asking for the register the call actually produced yields an
                empty requirement set
      REACHABLE for every other register in the class, applying the computed
                requirements and re-running the forward scan produces exactly
                that register
    """
    ident_ok = ident_bad = reach_ok = reach_bad = calls = 0
    bad = []
    for stem in stems:
        p = WORK / f"{stem}.reload.json"
        if not p.exists():
            print(f"  (no {p.name}; skipped)")
            continue
        funcs = json.loads(p.read_text())
        for func, d in funcs.items():
            for retry in d.get("retries", []):
                if not retry.get("find_reg"):
                    continue
                calls += 1
                got = retry["got"]
                fr = last_find_reg(retry)
                cls = R.CLASS_CONTENTS.get(fr.get("class", 1), GR_REGS)
                if got in cls:
                    ok, f_, b_, _ = requirements(retry, got)
                    if ok and not f_ and not b_:
                        ident_ok += 1
                    else:
                        ident_bad += 1
                        bad.append((stem, func, retry["pseudo"], got,
                                    "identity", f_, b_))
                for want in sorted(cls):
                    if want == got or want >= FIRST_PSEUDO:
                        continue
                    ok, f_, b_, _ = requirements(retry, want)
                    if not ok:
                        continue
                    res = apply_and_rescan(retry, f_, b_)
                    if res == want:
                        reach_ok += 1
                    else:
                        reach_bad += 1
                        if len(bad) < 12:
                            bad.append((stem, func, retry["pseudo"], want,
                                        f"rescan->{res}", f_, b_))
    print(f"retry calls examined : {calls}")
    print(f"IDENTITY  (want == got -> empty requirement set): "
          f"{ident_ok}/{ident_ok + ident_bad}")
    print(f"REACHABLE (apply requirements -> forward scan yields want): "
          f"{reach_ok}/{reach_ok + reach_bad}")
    for b in bad[:12]:
        print("  FAIL", b)
    return 0 if not ident_bad and not reach_bad else 1


def list_retries(stem):
    funcs = json.loads((WORK / f"{stem}.reload.json").read_text())
    n = 0
    for func, d in sorted(funcs.items()):
        for retry in d.get("retries", []):
            fr = last_find_reg(retry) if retry.get("find_reg") else {}
            print(f"{func:34s} pseudo {retry['pseudo']:4d} -> "
                  f"{rname(retry['got']):5s} calls={retry['calls']:2d} "
                  f"class={fr.get('class', '?')} "
                  f"forbidden={[rname(r) for r in retry['forbidden']]}")
            n += 1
    print(f"{n} retry call(s) in {stem}")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", nargs="*", metavar="STEM")
    ap.add_argument("--target", nargs=4,
                    metavar=("STEM", "FUNC", "PSEUDO", "REG"))
    ap.add_argument("--list", metavar="STEM")
    a = ap.parse_args()
    if a.selftest is not None:
        stems = a.selftest or sorted(
            p.name[:-len(".reload.json")] for p in WORK.glob("*.reload.json"))
        return selftest(stems)
    if a.target:
        stem, func, pseudo, reg = a.target
        return report(stem, func, int(pseudo), int(reg))
    if a.list:
        return list_retries(a.list)
    ap.error("one of --selftest / --target / --list")


if __name__ == "__main__":
    sys.exit(main())
