#!/usr/bin/env python3
"""ra_solver.sweep — run the whole inverse pipeline over a set of functions.

For each function:

  1. honest/target asm pair          (mkasm_honest.sh, once per TU)
  2. register substitutions          (goal_from_asm.substitutions)
  3. honest models                   (honest_model.py / honest_local_model.py)
  4. attribution to pseudos or qtys  (see below)
  5. inverse search + lever mapping  (inverse.py)

ATTRIBUTION.  A substitution says "hard reg R should be T".  Global attribution
is by disposition holder (unique -> certain).  When no global allocno holds R
the exchange lives in local-alloc, and the candidate set is built structurally:
the qtys holding R, crossed with the qtys holding T whose live spans OVERLAP —
because two quantities can only exchange registers if they are simultaneously
live.  Every surviving candidate pair is searched and reported; none is
silently preferred.

Verdicts:
  LEVER        >=1 minimal perturbation vector found, mapped to legitimate C
  UNREACHABLE  no perturbation of any modelled input reaches target -> the
               mechanism is outside the model; next move is instrumentation
  GAP          the pipeline could not produce an input; the exact gap is recorded

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/sweep.py --json tmp/out.json --md docs/....md
"""
import argparse
import io
import json
import subprocess
import sys
from contextlib import redirect_stdout
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))

import goal_from_asm as G                                    # noqa: E402
import inverse as INV                                        # noqa: E402
import levers as L                                           # noqa: E402

WORK = ROOT / "tmp" / "inverse_work"
RA = WORK / "ra"

# The parked queue items whose park reason names a register exchange.
TARGETS = [
    ("func_800611A4", "text1b", "v0<->v1 RA swap (local-alloc.c:472 + find_free_reg)"),
    ("func_80037A20", "code6cac_c", "s0<->s1 allocno-priority swap + cse REG_WAS_0 fold"),
    ("ang_hosei_80056FE8", "text1b", "a1<->a2 find_reg swap, no copy-preference anchor"),
    ("func_80033550", "code6cac_b", "RA $a1-vs-$a3 (asm pin)"),
    ("special_camera_get_rot_dir", "code6cac_b2_post",
     "copy_end->s5 rotation reachable only via register pin"),
]


# Precisely-characterised pipeline gaps, appended to the report so the next
# session does not have to re-diagnose them.
#
# RESOLVED 2026-08-06 — func_80037A20 was here: extract.py's greg<->ent
# alignment matched segment signatures by pseudo-set SUBSET, which is ambiguous
# when sibling functions have nested signatures.  On code6cac_c (12 ents, 8
# segments) the greedy walk consumed segments at ents 0,3,4,5,8,9,10,11 —
# including two functions with NO allocnos — and skipped ent[7] = func_80037A20,
# so extraction died with "FATAL: no greg segment aligned".  extract.py now
# indexes by the func-tagged ALLOCDBG stream and verifies each pairing's allocno
# set, falling back to the subset heuristic only for hookless dumps.  The
# function now resolves to a LEVER verdict below.
GAP_NOTES = {}

# Owner/lead caveat block, carried in the GENERATOR so a sweep re-run cannot
# wipe it.  Originally committed as c9b5a6e8; the foreclosure paragraph was
# added 2026-08-06 (commit 61912561's model gap).
CAVEAT_BLOCK = """
> **BASELINE-ROUTING CAVEAT (2026-08-06, measured on func_80072CD4 — commit
> b9d92391).** The stripped-source baseline above is MAIN's body with cheat-asm
> stripped. For a parked function whose BANKED CANDIDATE beats main (WIP/grind
> ledger records a lower floor than the stripped-main distance), every model
> here is extracted from the WRONG baseline: the vectors may target a
> sub-problem the candidate already solved (func_80072CD4's 22 local vectors
> named exactly the exchange its distance-4 candidate had fixed; its stripped
> main is distance 12). Before acting on any verdict in this doc, check the
> function's ledger for a banked candidate and, if one exists, RE-DERIVE the
> model from that candidate body first. special_camera_get_rot_dir has this
> shape (banked 9 vs stripped-main 12). Separately: `write_stripped` is not a
> complete strip (leaves volatile array DECLARATIONS standing — commit
> d42977db), so frame-sensitive questions need per-construct verification.
> Also refuted by direct attempt: func_80037A20's pref_add vector was
> foreclosed as not-C-reachable, and the function's swap then fell to a plain
> named-intermediate restructure the solver never proposed (commit 8e1fd614) —
> the suite diagnoses residuals; it does not enumerate every closing spelling.

> **PREFERENCE FORECLOSURE (2026-08-06, commit 61912561 — now ENFORCED).**
> Every PREF_ADD lever this sweep emitted before today was mechanically
> impossible. `global.c set_preference` records a hard-reg copy preference ONLY
> from a SET between a pseudo and a HARD register, so a register that never
> appears as a hard reg in the function's pre-RA RTL can never be preferred —
> and callee-saved registers never appear there from ANY C (they enter at
> prologue/epilogue, after reload; only a forbidden `register asm("$N")` pin
> creates one). `prune_preferences` (global.c:897) independently strips every
> call-used register from a CALL-CROSSING allocno's preferences.
> `inverse.py` now carries the pre-RA hard-reg set in the model
> (`prera_hard`) and reports unreachable preference atoms as **FORECLOSED**
> with the mechanism instead of emitting them as levers. Ground truth
> reproduced exactly: func_80037A20 `v0 a0 a1 a2 a3 ra` (no $s1),
> func_80033550 `a0` (no $a3), special_camera_get_rot_dir `v0 v1 a0 a1 a2 ra`
> (no $s5). The refs-delta search was also widened from +3/-2 to +12/-6 with a
> cost gradient, and every report now prints its search bounds — the old cap is
> why pref_add looked like the UNIQUE 1-atom vector for func_80037A20 when
> wider refs deltas reach the goal too.
"""


def sh(cmd, timeout=3600):
    r = subprocess.run(cmd, shell=True, cwd=ROOT, capture_output=True,
                       text=True, timeout=timeout)
    return r.returncode, r.stdout + r.stderr


def ensure_asm(stem, log):
    if (WORK / f"{stem}.tgt.s").exists() and (WORK / f"{stem}.hon.s").exists():
        return True
    rc, out = sh(f"bash tools/ra_solver/mkasm_honest.sh {stem}")
    log.append(f"mkasm_honest {stem}: rc={rc}")
    return (WORK / f"{stem}.tgt.s").exists()


def ensure_global_model(func, stem, log):
    p = RA / f"{func}.model.json"
    if p.exists():
        return p
    rc, out = sh(f"python3 tools/ra_solver/honest_model.py {func} {stem}")
    log.append(f"honest_model {func}: rc={rc} {out.strip().splitlines()[-1:] }")
    return p if p.exists() else None


def ensure_local_model(stem, log):
    p = RA / f"{stem}.local.json"
    if p.exists():
        return p
    rc, out = sh(f"python3 tools/ra_solver/honest_local_model.py {stem}")
    log.append(f"honest_local_model {stem}: rc={rc}")
    return p if p.exists() else None


def overlap(a, b):
    return a["birth"] < b["death"] and b["birth"] < a["death"]


def local_candidates(rows, ours, want):
    """Structural candidate pairs for an R<->T exchange inside one block."""
    out = []
    by_blk = {}
    for r in rows:
        if r["pass"] == "main":
            by_blk.setdefault(r["blk"], []).append(r)
    for blk, group in sorted(by_blk.items()):
        hold_r = [r for r in group if r["got"] == ours]
        hold_t = [r for r in group if r["got"] == want]
        for x in hold_r:
            for y in hold_t:
                if overlap(x, y):
                    out.append((blk, x["qty"], y["qty"]))
    return out


def run_inverse(backend, goal, depth=2, top=5):
    buf = io.StringIO()
    try:
        with redirect_stdout(buf):
            hits = INV.search(backend, goal, depth, top)
            if hits:
                INV.report(hits, backend)
            else:
                INV.negative_report(backend, goal, depth)
    except Exception as e:                      # noqa: BLE001
        return None, f"{type(e).__name__}: {e}"
    return hits, buf.getvalue()


def do_func(func, stem, reason, results, md):
    entry = {"func": func, "stem": stem, "park_reason": reason,
             "log": [], "verdict": None, "detail": {}}
    results.append(entry)
    log = entry["log"]
    md.append(f"\n## {func}  (`src/{stem}.c`)\n")
    md.append(f"**Park reason:** {reason}\n")

    if not ensure_asm(stem, log):
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = f"could not build the asm pair for {stem}"
        md.append(f"**Verdict: GAP** — {entry['detail']['gap']}\n")
        return

    try:
        hon = [t for t, _ in G.asm_body(WORK / f"{stem}.hon.s", func)]
        tgt = [t for t, _ in G.asm_body(WORK / f"{stem}.tgt.s", func)]
    except KeyError as e:
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = f"function not found in the asm stream: {e}"
        md.append(f"**Verdict: GAP** — {entry['detail']['gap']}\n")
        return

    subs, detail = G.substitutions(hon, tgt)
    entry["detail"]["honest_insns"] = len(hon)
    entry["detail"]["target_insns"] = len(tgt)
    entry["detail"]["substitutions"] = {f"{G.rname(x)}->{G.rname(y)}": n
                                        for (x, y), n in subs.items()}
    md.append(f"Honest stream {len(hon)} insns, target {len(tgt)} insns.")
    if not subs:
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = ("no register substitution in the aligned "
                                  "same-skeleton pairs — the residual is not a "
                                  "rename (instruction-level difference)")
        md.append(f"\n**Verdict: GAP** — {entry['detail']['gap']}\n")
        return
    md.append(" Derived substitutions: "
              + ", ".join(f"`{G.rname(x)}->{G.rname(y)}` x{n}"
                          for (x, y), n in sorted(subs.items(),
                                                  key=lambda kv: -kv[1])) + "\n")

    # ---- global attribution ---------------------------------------------
    gm = ensure_global_model(func, stem, log)
    goal = {}
    if gm:
        model = json.loads(gm.read_text())
        disp = {int(k): v for k, v in model["dispositions"].items()}
        goal, notes = G.attribute(subs, disp)
        entry["detail"]["attribution_notes"] = notes
        md.append("Attribution:\n")
        for n in notes:
            md.append(f"- {n}\n")

    # Ambiguous global attribution: rather than give up, enumerate.  A
    # substitution R->T with several R-holders yields one candidate goal per
    # holder; each is searched and only the reachable ones are reported.  The
    # candidate set is bounded because it is restricted to holders that
    # CONFLICT with a T-holder — two allocnos can only exchange registers if
    # they are simultaneously live.
    if not goal and gm:
        model = json.loads(gm.read_text())
        disp = {int(k): v for k, v in model["dispositions"].items()}
        conf = {int(k): set(v) for k, v in model["conflicts"].items()}
        holders = {}
        for p, r in disp.items():
            if r is not None and r >= 0:
                holders.setdefault(r, []).append(p)
        gcands = []
        for (ours, want) in subs:
            for p in holders.get(ours, []):
                partners = [q for q in holders.get(want, [])
                            if q in conf.get(p, set()) or p in conf.get(q, set())]
                if partners:
                    for q in partners:
                        gcands.append({p: want, q: ours})
                else:
                    gcands.append({p: want})
        entry["detail"]["global_candidates"] = [
            {str(k): v for k, v in g.items()} for g in gcands]
        if gcands:
            md.append(f"\nAttribution is ambiguous, so all {len(gcands)} "
                      f"conflict-consistent candidate goal(s) were searched:\n")
            results_g, seen_g = [], set()
            for g in gcands:
                key = tuple(sorted(g.items()))
                if key in seen_g:
                    continue
                seen_g.add(key)
                backend = INV.GlobalBackend(str(gm))
                hits, text = run_inverse(backend, g)
                results_g.append({"goal": {str(k): v for k, v in g.items()},
                                  "reachable": bool(hits),
                                  "vectors": [{"atoms": [{"class": a.cls,
                                                          "desc": a.desc}
                                                         for a in c],
                                               "cost": cost}
                                              for n, cost, c in (hits or [])]})
                md.append(f"\n### candidate goal "
                          f"{ {str(k): INV.rname(v) for k, v in g.items()} }\n")
                md.append(("%d minimal vector(s).\n\n```\n%s\n```\n"
                           % (len(hits), text.strip())) if hits
                          else "UNREACHABLE.\n\n```\n%s\n```\n"
                               % text.strip()[:1200])
            entry["detail"]["global_results"] = results_g
            if any(r["reachable"] for r in results_g):
                entry["verdict"] = "LEVER"
                md.append(f"\n**Verdict: LEVER** "
                          f"({sum(r['reachable'] for r in results_g)}/"
                          f"{len(results_g)} candidate goals reachable).\n")
                return

    if goal:
        backend = INV.GlobalBackend(str(gm))
        hits, text = run_inverse(backend, goal)
        entry["detail"]["global_goal"] = {str(k): v for k, v in goal.items()}
        if hits:
            entry["verdict"] = "LEVER"
            entry["detail"]["vectors"] = [
                {"atoms": [{"class": a.cls, "desc": a.desc} for a in c],
                 "cost": cost} for n, cost, c in hits]
            md.append(f"\n**Verdict: LEVER** — {len(hits)} minimal vector(s), "
                      f"global backend.\n\n```\n{text.strip()}\n```\n")
        else:
            entry["verdict"] = "UNREACHABLE"
            md.append(f"\n**Verdict: UNREACHABLE** (global backend).\n\n"
                      f"```\n{text.strip()}\n```\n")
        return

    # ---- local attribution ----------------------------------------------
    lm = ensure_local_model(stem, log)
    if not lm:
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = "local model extraction failed"
        md.append(f"\n**Verdict: GAP** — {entry['detail']['gap']}\n")
        return
    rows = json.loads(lm.read_text()).get(func, [])
    if not rows:
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = f"no QTYDBG rows for {func} in {lm.name}"
        md.append(f"\n**Verdict: GAP** — {entry['detail']['gap']}\n")
        return

    cands = []
    for (ours, want) in subs:
        cands += [(ours, want) + c for c in local_candidates(rows, ours, want)]
    entry["detail"]["local_candidates"] = [
        {"ours": G.rname(o), "want": G.rname(w), "block": b, "qty_r": qr,
         "qty_t": qt} for o, w, b, qr, qt in cands]
    if not cands:
        entry["verdict"] = "GAP"
        entry["detail"]["gap"] = ("no overlapping qty pair holds the substituted "
                                  "registers — the exchange is neither a global "
                                  "allocno nor a block-local quantity pair "
                                  "(reload-retry or a hard operand)")
        md.append(f"\n**Verdict: GAP** — {entry['detail']['gap']}\n")
        return

    md.append(f"\nNo global allocno holds the substituted register, so the "
              f"exchange is block-local. {len(cands)} structural candidate "
              f"pair(s) (same block, overlapping live spans):\n")
    verdicts, texts = [], []
    seen = set()
    for ours, want, blk, qr, qt in cands:
        goal_l = {qr: want, qt: ours}
        # Both substitution directions of one exchange generate the same pair
        # with the roles swapped, which is the SAME goal — dedupe on the goal.
        key = (blk,) + tuple(sorted(goal_l.items()))
        if key in seen:
            continue
        seen.add(key)
        backend = INV.LocalBackend(str(lm), func, blk)
        hits, text = run_inverse(backend, goal_l)
        ok = bool(hits)
        verdicts.append(ok)
        md.append(f"\n### block {blk}: qty {qr} ({G.rname(ours)}) <-> "
                  f"qty {qt} ({G.rname(want)})\n")
        if hits:
            md.append(f"{len(hits)} minimal vector(s).\n\n```\n{text.strip()}\n```\n")
        else:
            md.append(f"UNREACHABLE.\n\n```\n{text.strip()[:1500]}\n```\n")
        texts.append({"block": blk, "qty_r": qr, "qty_t": qt,
                      "reachable": ok,
                      "vectors": [{"atoms": [{"class": a.cls, "desc": a.desc}
                                             for a in c], "cost": cost}
                                  for n, cost, c in (hits or [])]})
    entry["detail"]["local_results"] = texts
    entry["verdict"] = "LEVER" if any(verdicts) else "UNREACHABLE"
    md.append(f"\n**Verdict: {entry['verdict']}** "
              f"({sum(verdicts)}/{len(verdicts)} candidate pairs reachable).\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", default="tmp/inverse_sweep_2026-08-06.json")
    ap.add_argument("--md", default="docs/grind/inverse-sweep-2026-08-06.md")
    ap.add_argument("--only", help="comma-separated function subset")
    a = ap.parse_args()

    only = set(a.only.split(",")) if a.only else None
    results, md = [], []
    md.append("# Inverse-solver sweep — parked register-exchange functions\n")
    md.append("\n_Generated by `tools/ra_solver/sweep.py` (2026-08-06). "
              "Every verdict below is a DIAGNOSIS: minimal perturbations of the "
              "validated forward models, mapped to legitimate C techniques. "
              "No construct from a forbidden family is ever proposed._\n")
    md.append("\n_'Ours' is built from the CHEAT-STRIPPED source "
              "(`mkasm_honest.sh`): a parked function's on-main body carries asm "
              "pins that already force target's registers, so an on-main-vs-target "
              "diff is empty and there is no question to invert. Models come from "
              "the same stripped source via `honest_model.py` / "
              "`honest_local_model.py`._\n")
    md.append("\n_Re-run 2026-08-06 after the `extract.py` greg<->ent alignment "
              "fix (ALLOCDBG-indexed instead of pseudo-set-subset): func_80037A20 "
              "moved GAP -> LEVER; the other four verdicts are unchanged, and "
              "`validate.py` stayed 10/10 EXACT._\n")
    # The caveat block lives HERE, in the generator, not in the .md — the doc
    # is regenerated on every sweep run, so anything hand-edited into it is
    # silently destroyed (as happened to c9b5a6e8's block on the 2026-08-06
    # foreclosure re-run).  Edit this text, not the output.
    md.append(CAVEAT_BLOCK)

    for func, stem, reason in TARGETS:
        if only and func not in only:
            continue
        print(f"=== {func} ({stem})", flush=True)
        try:
            do_func(func, stem, reason, results, md)
        except Exception as e:                       # noqa: BLE001
            results.append({"func": func, "stem": stem, "verdict": "GAP",
                            "detail": {"gap": f"{type(e).__name__}: {e}"}})
            md.append(f"\n**Verdict: GAP** — driver exception: "
                      f"{type(e).__name__}: {e}\n")
        print(f"    -> {results[-1]['verdict']}", flush=True)

    for r in results:
        if r["verdict"] == "GAP" and r["func"] in GAP_NOTES:
            r["detail"]["gap_note"] = GAP_NOTES[r["func"]]
            md.append(f"\n**Characterised gap ({r['func']}):** "
                      f"{GAP_NOTES[r['func']]}\n")

    # Verdict table: a LEVER whose surviving vectors are all refs-deltas is a
    # materially different claim from one with a cheap spellable vector, and
    # 61912561 measured those particular deltas as byte-forced.  Say which.
    md.append("\n---\n\n## Verdict table (post-foreclosure)\n\n")
    md.append("| function | verdict | what the verdict now rests on |\n")
    md.append("|---|---|---|\n")
    for r in results:
        det = r.get("detail", {})
        kinds = set()
        for blkres in det.get("local_results", []) + det.get("global_results", []):
            for v in blkres.get("vectors", []):
                kinds |= {a["class"] for a in v["atoms"]}
        for v in det.get("vectors", []):
            kinds |= {a["class"] for a in v["atoms"]}
        rests = ", ".join(sorted(kinds)) if kinds else "—"
        if r["verdict"] == "GAP":
            rests = "no reachable vector (" + det.get("gap", "")[:60] + "…)"
        md.append(f"| `{r['func']}` | {r['verdict']} | {rests} |\n")
    md.append("\n_No `pref_add` / `pref_reroute` entry can appear in this "
              "table any more: those atoms are foreclosed before emission "
              "unless the register is provably appearable. Where a verdict "
              "rests only on `refs_up`/`refs_down`, note that 61912561 "
              "measured func_80037A20's refs vectors (pointer >=10, counter "
              "<=4) as byte-forced and already killed in s7/s8 — the model "
              "reaches the goal, the C cannot._\n")

    counts = {}
    for r in results:
        counts[r["verdict"]] = counts.get(r["verdict"], 0) + 1
    md.insert(2, f"\n**Summary:** " + ", ".join(f"{v} {k}"
                                                for k, v in sorted(counts.items()))
              + f" over {len(results)} function(s).\n")
    md.append("\n---\n\n" + L.forbidden_block(indent=""))

    Path(a.json).write_text(json.dumps(results, indent=1))
    Path(a.md).write_text("".join(md))
    print(f"\nwrote {a.json} and {a.md}: {counts}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
