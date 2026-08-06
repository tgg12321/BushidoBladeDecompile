#!/usr/bin/env python3
"""inverse_compose — cross-model composition: WHICH model owns the residual,
and what happens downstream if an upstream change is made.

The three backends each answer a question about ONE model, and each assumes its
model is the right place to ask.  That assumption is exactly what fails on a
residual like func_80072CD4, where the scheduler search returns a confident
negative because the real difference is upstream of the scheduler entirely.
This module supplies the missing triage and the missing downstream replay.

## 1. `classify` — which model does the residual FIRST appear in?

The pipeline is a funnel, and each stage can only permute what the stage above
built:

    front end + cse/combine/loop   builds the INSN MULTISET
        -> global/local alloc      assigns REGISTERS to a fixed multiset
            -> sched.c             ORDERS a fixed, already-allocated stream

So comparing our honest stream against target's answers the question directly:

  * register-blanked multisets differ  -> **PRE-RA**.  A different set of
    instructions. No RA or scheduler perturbation can reach it; those models
    permute and rename a fixed multiset. Searching them produces fiction.
  * multisets match, exact texts differ -> **RA**. Same instructions, different
    registers. inverse.py's territory.
  * texts match as a multiset, order differs -> **SCHED**. inverse_sched.py.

This is cheap, needs only the two asm streams, and is the check that should run
BEFORE any backend search.

## 2. `hypothesis` — replay the downstream models under an upstream change

An upstream (CSE-level) change cannot be spelled directly into the model files;
it comes from different C.  But its CONSEQUENCE on the downstream models is
expressible, and that is enough to test it:

  `merge-pseudos P,Q`  models "these two values become one" — exactly what CSE
  does when it unifies two materialisations of the same value.  The merged
  pseudo takes the union of the live range and conflicts and the sum of the
  references, which is what the RTL would carry.  The RA forward model is then
  replayed and its answer compared against target's assignment.

So the composed question — "if 252 were materialised once, would RA then put it
where target has it?" — becomes checkable without compiling anything.

**What this deliberately does NOT do:** search for the upstream change. The
hypothesis is hand-specified; this replays it. Automating the upstream search
would require generating candidate C and compiling it, which is a different
pathway (see the honest-limits section of the Phase 6 report).

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/inverse_compose.py classify <stem> <func>
        [--work tmp/inverse_work]
  python3 tools/ra_solver/inverse_compose.py hypothesis <model.json>
        --merge P,Q [--goal '{"pseudo": reg}']
"""
import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import levers as L                                       # noqa: E402
import goal_from_asm as G                                # noqa: E402
from simulate import Sim                                 # noqa: E402

sys.path.insert(0, str(ROOT / "tools" / "sched_solver"))
import goalmap as GM                                     # noqa: E402

PRE_RA, RA, SCHED, IDENTICAL = "PRE-RA", "RA", "SCHED", "IDENTICAL"


def rn(r):
    """Register name, None-safe: a merged-away pseudo has no assignment."""
    return "(gone)" if r is None else G.rname(r)

STAGE_TOOL = {
    PRE_RA: ("no backend — the residual is upstream of every model",
             L.RTL_SHAPE),
    RA: ("tools/ra_solver/inverse.py  (global / local)", None),
    SCHED: ("tools/ra_solver/inverse_sched.py", None),
}


def classify(hon, tgt):
    """-> (stage, evidence dict)."""
    hs = Counter(G.blank_regs(x) for x in hon)
    ts = Counter(G.blank_regs(x) for x in tgt)
    ev = {"honest_insns": len(hon), "target_insns": len(tgt)}
    if hs != ts:
        only_h = sorted((hs - ts).elements())
        only_t = sorted((ts - hs).elements())
        ev["only_in_honest"] = only_h
        ev["only_in_target"] = only_t
        # A `nop`-only multiset difference is NOT an RTL-shape difference.
        # maspsx inserts load-delay nops after the fact, so their count is a
        # consequence of the surrounding SCHEDULE (what sits in the delay
        # slot), i.e. downstream of RA, not upstream of it.  Labelling it
        # PRE-RA would send the reader to the wrong model.
        if all(t.strip() == "nop" for t in only_h + only_t):
            ev["nop_only"] = True
            ev["nop_delta"] = len(only_t) - len(only_h)
            return SCHED, ev
        return PRE_RA, ev
    ht = Counter(GM.key(x) for x in hon)
    tt = Counter(GM.key(x) for x in tgt)
    if ht != tt:
        ev["register_only_diff"] = True
        ev["only_in_honest"] = sorted((ht - tt).elements())
        ev["only_in_target"] = sorted((tt - ht).elements())
        return RA, ev
    if [GM.key(x) for x in hon] != [GM.key(x) for x in tgt]:
        moved = sum(1 for a, b in zip(hon, tgt) if GM.key(a) != GM.key(b))
        ev["slots_differing"] = moved
        return SCHED, ev
    return IDENTICAL, ev


def _replace_with_asmfile(func):
    """True if FUNC's emitted block is substituted wholesale from asm/funcs/ by
    an asmfix rule. Such a target is unreadable to this text-stream classifier
    (see the guard in cmd_classify)."""
    try:
        txt = (ROOT / "asmfix.txt").read_text(errors="replace")
    except OSError:
        return False
    return re.search(rf"^{re.escape(func)}:\s*replace_with_asmfile\b",
                     txt, re.M) is not None


def cmd_classify(a):
    # GUARD (2026-08-06): for a `replace_with_asmfile` function this classifier
    # returns a FICTITIOUS verdict rather than no verdict, which is worse — it
    # reports PRE-RA / rtl_shape, the one answer meaning "stop, no model can
    # reach this". Two compounding causes: <stem>.tgt.s carries the target as a
    # glabel/endlabel block of `/* off addr bytes */` disassembly that
    # goalmap.asm_body skips, and the two streams are assembler SOURCE vs
    # DISASSEMBLY, so identical instructions compare unequal (`subu $sp,$sp,144`
    # vs `addiu $sp, $sp, -0x90`). Measured on func_80089F3C: PRE-RA with a
    # 287-vs-318 gap when the real codegen residual was ZERO.
    wired = _replace_with_asmfile(a.func)
    if wired and a.force_text:
        print("PATH: text-stream classifier, GUARD OVERRIDDEN (--force-text) — "
              f"{a.func} IS wired `replace_with_asmfile`, so the verdict below "
              "is FICTION. Do not act on it.\n")
    elif wired:
        sys.exit(
            f"{a.func} is wired `replace_with_asmfile` in asmfix.txt, so its "
            f"target in {a.stem}.tgt.s is disassembly text this classifier "
            f"cannot read; it would report a FICTITIOUS PRE-RA verdict.\n"
            f"Use the object-level classifier instead:\n"
            f"  python3 tools/ra_solver/goal_from_tgt.py classify {a.stem} {a.func}\n"
            f"(--force-text bypasses this guard for debugging the guard itself.)\n"
            f"See docs/grind/inverse-compose-2026-08-06.md.")
    else:
        print(f"PATH: text-stream classifier ({a.stem}.hon.s vs {a.stem}.tgt.s); "
              f"{a.func} is not `replace_with_asmfile`-wired.\n")
    work = ROOT / a.work
    hon_p, tgt_p = work / f"{a.stem}.hon.s", work / f"{a.stem}.tgt.s"
    for p in (hon_p, tgt_p):
        if not p.exists():
            sys.exit(f"missing {p} — run: bash tools/ra_solver/mkasm_honest.sh "
                     f"{a.stem}")
    try:
        hon = [t for t, _ in G.asm_body(hon_p, a.func)]
        tgt = [t for t, _ in G.asm_body(tgt_p, a.func)]
    except KeyError as e:
        if wired:
            sys.exit(
                f"{e}\n\nThis is cause #1 of the guard's rationale, reproduced: "
                f"the target block is a glabel/endlabel section of "
                f"`/* off addr bytes */` disassembly, which asm_body skips.\n"
                f"(Cause #2, the source-vs-disassembly spelling mismatch, only "
                f"shows once a stream is parseable at all.)\n"
                f"Use: python3 tools/ra_solver/goal_from_tgt.py classify "
                f"{a.stem} {a.func}")
        raise
    stage, ev = classify(hon, tgt)

    print(f"{a.func} ({a.stem}): honest {ev['honest_insns']} insns, "
          f"target {ev['target_insns']} insns")
    print(f"\nFIRST DIVERGENCE: {stage}")
    if stage == IDENTICAL:
        print("  the honest stream already equals target for this function.")
        return 0
    tool, cls = STAGE_TOOL[stage]
    print(f"  next tool: {tool}")
    if stage == PRE_RA:
        print("\n  instruction shapes present in ONE stream only "
              "(registers blanked):")
        for t in ev["only_in_honest"][:12]:
            print(f"    ours only  : {t}")
        for t in ev["only_in_target"][:12]:
            print(f"    target only: {t}")
        print("\n  A different instruction MULTISET means the RA and scheduler")
        print("  models cannot express this residual: they permute and rename a")
        print("  FIXED set of insns. Searching them would produce fiction.")
        print()
        print(L.format_report([L.RTL_SHAPE, L.CSE_MERGE, L.CSE_SPLIT]))
        print()
        print(L.forbidden_block())
    elif stage == RA:
        print("  same instructions, different registers:")
        for t in ev["only_in_honest"][:8]:
            print(f"    ours  : {t}")
        for t in ev["only_in_target"][:8]:
            print(f"    target: {t}")
    elif ev.get("nop_only"):
        print(f"  the ONLY multiset difference is {abs(ev['nop_delta'])} "
              f"`nop`(s) ({'target has more' if ev['nop_delta'] > 0 else 'we have more'}).")
        print("  maspsx inserts load-delay nops after the fact, so their count "
              "follows from\n  what the SCHEDULE put in the delay slot — a "
              "scheduling residual, not an RTL\n  one. The instructions "
              "themselves are identical.")
    else:
        print(f"  same instructions and registers, {ev['slots_differing']} "
              f"slot(s) in a different order.")
    return 0


def merge_pseudos(model, p, q):
    """Model CSE unifying two values into one.

    The survivor P takes: the SUM of the references (both uses remain real
    uses), the UNION of the live range (it now spans both), and the UNION of
    the conflicts (it coexists with everything either did).  Q is removed.
    That is what the RTL carries after cse.c unifies two sets of the same
    value — it is not a free parameter, it is the consequence.
    """
    m = json.loads(json.dumps(model))          # deep copy
    fl = m["flow"]
    fp, fq = fl.get(str(p), {}), fl.get(str(q), {})

    def g(d, k, dflt=0):
        return d.get(k, dflt) or 0

    fp["nrefs_flow"] = g(fp, "nrefs_flow", 1) + g(fq, "nrefs_flow", 1)
    fp["livelen_flow"] = g(fp, "livelen_flow", 1) + g(fq, "livelen_flow", 1)
    fp["calls_crossed"] = max(g(fp, "calls_crossed"), g(fq, "calls_crossed"))
    fl[str(p)] = fp
    fl.pop(str(q), None)

    cf = m["conflicts"]
    merged = set(cf.get(str(p), [])) | set(cf.get(str(q), []))
    merged.discard(p)
    merged.discard(q)
    cf[str(p)] = sorted(merged)
    cf.pop(str(q), None)
    for k, v in cf.items():
        s = set(v)
        if q in s:
            s.discard(q)
            s.add(p)
        s.discard(int(k))
        cf[k] = sorted(s)

    hc = m["hard_conflicts"]
    hc[str(p)] = sorted(set(hc.get(str(p), [])) | set(hc.get(str(q), [])))
    hc.pop(str(q), None)

    for key in ("prefs", "full_prefs", "copy_prefs"):
        d = m.get(key) or {}
        if str(q) in d:
            d[str(p)] = sorted(set(d.get(str(p), [])) | set(d.pop(str(q))))
        m[key] = d

    m["order"] = [x for x in m["order"] if x != q]
    m["dispositions"].pop(str(q), None)
    m["allocdbg"] = [r for r in m.get("allocdbg", []) if r["pseudo"] != q]
    for name in ("use_only", "md_class"):
        m[name] = [x for x in m.get(name, []) if x != q]
    return m


def cmd_hypothesis(a):
    model = json.loads(Path(a.model).read_text())
    p, q = (int(t) for t in a.merge.split(","))
    base = Sim(model)
    _, before = base.simulate()
    print(f"hypothesis: CSE unifies pseudo {q} into pseudo {p} "
          f"(one materialisation instead of two)")
    print(f"  baseline allocation : "
          f"{ {k: rn(v) for k, v in sorted(before.items())} }")

    m2 = merge_pseudos(model, p, q)
    after_sim = Sim(m2)
    _, after = after_sim.simulate()
    print(f"  under the hypothesis: "
          f"{ {k: rn(v) for k, v in sorted(after.items())} }")
    fp = m2["flow"][str(p)]
    print(f"  merged pseudo {p}: refs={fp.get('nrefs_flow')} "
          f"livelen={fp.get('livelen_flow')} "
          f"conflicts={m2['conflicts'].get(str(p))}")

    if a.goal:
        goal = {int(k): v for k, v in json.loads(a.goal).items()}
        # FAITHFULNESS GUARD.  A merge DELETES pseudo Q, so any goal naming Q
        # is unsatisfiable by construction, and a goal that simply omits Q is
        # only a fragment of the real target.  Both cases make the RESULT line
        # below meaningless-but-encouraging, which is exactly the over-claim
        # this tool exists to prevent.  Say so loudly.
        if q in goal:
            print(f"\n  *** GOAL NAMES THE MERGED-AWAY PSEUDO {q}: "
                  f"unsatisfiable by construction.")
            print("      If target still needs TWO distinct values here, the "
                  "merge is the WRONG\n      hypothesis for this function — "
                  "it models one value where target has two.")
        elif len(goal) < len([x for x in model["order"]
                              if x in model["dispositions"] or True]) - 1:
            print(f"\n  NOTE: the goal constrains {len(goal)} pseudo(s) out of "
                  f"{len(model['order'])} allocated. A partial goal can be met "
                  f"by\n  a hypothesis that is not faithful to target overall — "
                  f"check that target really\n  has one value here, not two.")
        ok_b = all(before.get(k) == v for k, v in goal.items())
        ok_a = all(after.get(k) == v for k, v in goal.items())
        print(f"\n  goal {{{', '.join(f'{k}: {G.rname(v)}' for k, v in goal.items())}}}")
        print(f"    baseline reaches it   : {ok_b}")
        print(f"    hypothesis reaches it : {ok_a}")
        if ok_a and not ok_b:
            print("\n  RESULT: the upstream change CLOSES the allocation "
                  "residual.\n  The C-side lever family is below.")
        elif ok_a and ok_b:
            print("\n  RESULT: goal already met before the change — the "
                  "hypothesis is not load-bearing for it.")
        else:
            miss = {k: (rn(after.get(k)), rn(v))
                    for k, v in goal.items() if after.get(k) != v}
            print(f"\n  RESULT: the upstream change does NOT close it. "
                  f"Secondary residual: "
                  + ", ".join(f"pseudo {k}: got {g}, want {w}"
                              for k, (g, w) in miss.items()))
            print("  Feed that secondary residual back into "
                  "tools/ra_solver/inverse.py as its own goal.")
    changed = {k: (rn(before.get(k)), rn(after.get(k)))
               for k in set(before) | set(after)
               if before.get(k) != after.get(k)}
    print(f"\n  pseudos whose register CHANGED under the hypothesis: "
          + (", ".join(f"{k} {b}->{c}" for k, (b, c) in sorted(changed.items()))
             if changed else "(none)"))
    print()
    print(L.format_report([L.CSE_MERGE]))
    print()
    print(L.forbidden_block())
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("classify", help="which model owns the residual")
    c.add_argument("stem")
    c.add_argument("func")
    c.add_argument("--work", default="tmp/inverse_work")
    c.add_argument("--force-text", action="store_true",
                   help="bypass the replace_with_asmfile guard and classify "
                        "from the TEXT streams anyway. The verdict is known to "
                        "be fiction for that class (see cmd_classify); for "
                        "debugging the guard itself, not for deriving work.")
    c.set_defaults(fn=cmd_classify)
    h = sub.add_parser("hypothesis", help="replay RA under an upstream change")
    h.add_argument("model")
    h.add_argument("--merge", required=True, metavar="P,Q")
    h.add_argument("--goal")
    h.set_defaults(fn=cmd_hypothesis)
    a = ap.parse_args()
    return a.fn(a)


if __name__ == "__main__":
    sys.exit(main())
