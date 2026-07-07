"""engine CLI — Phase 0 surface.

  python3 -m engine.cli oracle-lock            capture oracle/manifest.json
  python3 -m engine.cli verify-oracle [--rebuild]
                                               confirm byte-identical build + report drift
  python3 -m engine.cli build                  full clean-driver build -> SHA1 check
  python3 -m engine.cli build-c <stem>         build one C object into build/src/
  python3 -m engine.cli parity [--out D --ref D]
                                               build all C objects with the driver and
                                               byte-compare vs a reference build dir

Run from the repo root, inside WSL.
"""
from __future__ import annotations

import argparse
import json
import sys

from . import buildconfig as cfg
from . import fixtures as F
from . import oracle as O
from . import canonical as CANON
from . import diagnose as DIAG
from . import integrate as INT
from . import metrics as MET
from . import orchestrator as ORCH
from . import pipeline as P
from . import queue as Q
from . import sandbox as SB


def _print_finish_nudge(func: str) -> None:
    """Reminder printed after a successful retire: register reusable findings
    where future agents will see them. The old `capture-recipe` tool + the
    `tools/recipes/` library were archived 2026-05-26; `.claude/rules/` is the
    live technique registry (auto-loads by `paths:` glob; the metrics layer
    fingerprints each as a technique slug). See CLAUDE.md "The loop" step 6."""
    print(
        f"\n─ {func} verified (SHA1 == oracle). Before you commit — REGISTER FINDINGS ─\n"
        "  Did this match teach a reusable codegen pattern or a non-obvious gotcha?\n"
        "    • reusable pattern   → add/update .claude/rules/<slug>.md (give it a `paths:`\n"
        "                           glob so it auto-loads on matching source reads; metrics\n"
        "                           fingerprints it as a technique slug)\n"
        "    • function-specific  → add a memory/ entry\n"
        "    • routine / no-op    → skip (don't manufacture a finding)\n"
        "  This human-written record is the durable one — there is no recipe-capture tool.\n"
    )


def main() -> int:
    ap = argparse.ArgumentParser(prog="engine")
    sub = ap.add_subparsers(dest="cmd", required=True)

    olp = sub.add_parser("oracle-lock", help="capture oracle/manifest.json")
    olp.add_argument("--commit", default=None,
                     help="record this git commit (WSL git can't read the worktree)")
    vp = sub.add_parser("verify-oracle", help="confirm byte-identical build + drift")
    vp.add_argument("--rebuild", action="store_true", help="force a fresh full build")
    vp.add_argument("--allow-dirty", action="store_true",
                    help="permit --rebuild with uncommitted build-input edits "
                         "(default: refused — rebuilding from dirty src corrupts "
                         "the canonical reference the sandbox scores against)")
    sub.add_parser("build", help="full clean-driver build -> SHA1 check")
    bp = sub.add_parser("build-c", help="build one C object")
    bp.add_argument("stem")
    pp = sub.add_parser("parity", help="byte-compare driver C objects vs a reference build")
    pp.add_argument("--out", default="tmp/parity")
    pp.add_argument("--ref", default="build")
    fap = sub.add_parser("fixtures-add", help="record golden fixtures by function name")
    fap.add_argument("names", nargs="+")
    sub.add_parser("fixtures-verify", help="check golden fixtures against the build")
    sbp = sub.add_parser("sandbox", help="cheat-invisible score for a function")
    sbp.add_argument("func")
    sbp.add_argument("--disable", choices=["all", "lost-codegen", "none"], default="lost-codegen")
    sbp.add_argument("--keep-cheat-asm", action="store_true",
                     help="do NOT strip cheat-asm pins/inline-asm (default: stripped, "
                          "so the score is the honest pure-C COMPLETED-C distance)")
    scp = sub.add_parser("scan-redundant", help="find functions whose rules are redundant (exact byte-identity)")
    g = scp.add_mutually_exclusive_group(required=True)
    g.add_argument("--file", help="scan one src file stem")
    g.add_argument("--all", action="store_true", help="scan every file with rules")
    scp.add_argument("--no-rebuild", action="store_true", help="reuse existing stripped .o (fast re-eval)")
    rtp = sub.add_parser("retire", help="retire a function's rules (delete + full-build SHA1 verify)")
    rtp.add_argument("func")
    rrp = sub.add_parser("retire-redundant", help="scan a file + auto-retire its distance-0 functions")
    rrp.add_argument("--file", required=True)
    dgp = sub.add_parser("diagnose", help="classify gap(s) vs immutable-plateau patterns (triage before permuting)")
    dgp.add_argument("funcs", nargs="+")
    dgp.add_argument("--detail", action="store_true", help="show the per-instruction diff")
    cnp = sub.add_parser("canonical", help="C-vs-asm classification for a function (canonical-asm gate)")
    cnp.add_argument("func")
    cnp.add_argument("--fast", action="store_true", help="opcode-only; skip the structural distance check (no build)")
    sub.add_parser("canonical-scan", help="classify every function as C / ASM-WHOLE / ASM-PARTIAL")
    sub.add_parser("test", help="run the engine regression suite (fast pure-logic + build-read tiers)")

    qp = sub.add_parser("queue", help="consolidated INCOMPLETE-work queue — work the TOP item to done")
    qp.add_argument("action", choices=["next", "done", "park", "status", "regen", "reopen"])
    qp.add_argument("func", nargs="?", help="function name (required for done/park/reopen)")
    qp.add_argument("--reason", default="", help="reason (for park/reopen)")
    qp.add_argument("--file", default="", help="src file stem (required for reopen)")

    a = ap.parse_args()

    if a.cmd == "oracle-lock":
        m = O.lock(git_commit=a.commit)
        print(f"locked oracle -> {O.MANIFEST}")
        print(f"  original sha1   {m['original_exe']['sha1']}  ({m['original_exe']['size']} bytes)")
        print(f"  expected build  {m['expected_build_sha1']}")
        print(f"  git commit      {m['git_commit']}")
        print(f"  corpus files    {len(m['corpus'])}")
        print(f"  config files    {len(m['config_files'])}")
        print(f"  golden fixtures {len(m['golden_fixtures'])}")
        return 0

    if a.cmd == "verify-oracle":
        if a.rebuild and not a.allow_dirty:
            dirty = O.dirty_build_inputs()
            if dirty:
                r = {
                    "ok": False,
                    "refused": "dirty-build-inputs",
                    "dirty": dirty,
                    "hint": "Uncommitted build-input edits present — a --rebuild now "
                            "would corrupt the canonical reference (build/) that the "
                            "sandbox scores against. During the edit loop use ONLY "
                            "`sandbox <func> --disable all` (it builds into tmp/ and "
                            "never touches build/). Commit or revert first, or pass "
                            "--allow-dirty if the dirty state IS the intended new "
                            "reference (e.g. mid-revert restoration).",
                }
                print(json.dumps(r, indent=2))
                MET.record_event("verify-oracle", None, r, exit_code=3)
                return 3
        r = O.verify(rebuild=a.rebuild)
        print(json.dumps(r, indent=2))
        MET.record_event("verify-oracle", None, r, exit_code=0 if r.get("ok") else 1)
        return 0 if r.get("ok") else 1

    if a.cmd == "build":
        exe = P.build_all()
        got = P.sha1(exe)
        ok = got == cfg.ORACLE_SHA1
        print(f"built {exe}\n  sha1 {got}\n  want {cfg.ORACLE_SHA1}\n  {'MATCH' if ok else 'MISMATCH'}")
        MET.record_event("build", None, {"sha1": got, "ok": ok}, exit_code=0 if ok else 1)
        return 0 if ok else 1

    if a.cmd == "build-c":
        out_o = f"build/src/{a.stem}.o"
        P.build_c_object(a.stem, out_o)
        print(f"built {out_o}  sha1 {P.sha1(out_o)}")
        return 0

    if a.cmd == "parity":
        res = P.parity_objects(a.out, a.ref)
        nmatch = sum(1 for _, s, _ in res if s == "MATCH")
        for stem, status, info in res:
            if status != "MATCH":
                print(f"  {status:6} {stem}  {info or ''}")
        print(f"parity: {nmatch}/{len(res)} C objects byte-identical to {a.ref}/")
        return 0 if nmatch == len(res) else 1

    if a.cmd == "fixtures-add":
        fx = F.add(a.names)
        print(f"golden fixtures ({len(fx)}):")
        for f in fx:
            print(f"  {f['vram']}  {f['size']:>5}B  {f['file']:<14} {f['name']}  {f['text_sha1'][:12]}")
        return 0

    if a.cmd == "fixtures-verify":
        res = F.verify()
        for name, status in res:
            print(f"  {status:8} {name}")
        ok = all(s == "OK" for _, s in res)
        print(f"fixtures: {sum(1 for _, s in res if s == 'OK')}/{len(res)} OK")
        return 0 if ok else 1

    if a.cmd == "sandbox":
        r = SB.sandbox_score(a.func, disable=a.disable,
                             strip_cheat_asm=not a.keep_cheat_asm)
        print(json.dumps(r, indent=2))
        MET.record_event("sandbox", a.func, r, extra={"disable": a.disable})
        return 0

    if a.cmd == "queue":
        if a.action == "regen":
            q = Q.generate()
            print(f"queue regenerated -> {Q.QUEUE_PATH}")
            print(json.dumps(q["counts"], indent=2))
            if q["build_failures"]:
                print("build_failures:", json.dumps(q["build_failures"]))
            return 0
        if a.action == "status":
            print(json.dumps(Q.status(), indent=2))
            return 0
        if a.action == "next":
            it = Q.next_item()
            if not it:
                print("queue: no active items (run `queue regen`, or all work is done/parked).")
                return 0
            # Attach WIP checkpoint summary if one exists for this function.
            # WIP entries (memory/wip/<func>/) preserve prior sessions' best
            # candidate bodies and measured floors so an agent can resume
            # mid-grind without re-deriving levers. See memory/wip/README.md.
            try:
                from . import wip as W  # noqa: WPS433 (local import: silent on failure)
                ws = W.summary(it["func"])
                if ws is not None:
                    it = {**it, "wip": ws}
            except Exception:
                pass
            print(json.dumps(it, indent=2))
            return 0
        if a.action in ("done", "park"):
            if not a.func:
                print(f"queue {a.action}: requires a function name")
                return 2
            r = (Q.mark_done(a.func) if a.action == "done"
                 else Q.mark_parked(a.func, a.reason))
            print(json.dumps(r, indent=2))
            MET.record_event(f"queue-{a.action}", a.func, r, exit_code=0 if r.get("ok") else 1)
            return 0 if r.get("ok") else 1
        if a.action == "reopen":
            if not a.func or not a.file:
                print("queue reopen: requires <func> --file <stem>")
                return 2
            r = Q.reopen(a.func, a.file, a.reason)
            print(json.dumps(r, indent=2))
            MET.record_event("queue-reopen", a.func, r, exit_code=0 if r.get("ok") else 1)
            return 0 if r.get("ok") else 1
        return 0

    if a.cmd == "scan-redundant":
        rebuild = not a.no_rebuild
        res = (ORCH.scan_all(rebuild=rebuild) if a.all
               else {a.file: ORCH.scan_file(a.file, rebuild=rebuild)})
        redundant = []
        for stem in sorted(res):
            per = res[stem]
            nr = sum(1 for v in per.values() if v["redundant"])
            print(f"{stem}: {len(per)} keyed funcs, {nr} redundant")
            for f, v in sorted(per.items(), key=lambda kv: (not kv[1]["redundant"], kv[1]["difficulty"])):
                mark = "  <- REDUNDANT" if v["redundant"] else ""
                print(f"    {v['difficulty']:>4}  {f}{mark}")
                if v["redundant"]:
                    redundant.append(f)
        print(f"\nredundant total: {len(redundant)}")
        if redundant:
            print("redundant:", " ".join(redundant))
        return 0

    if a.cmd == "retire":
        r = INT.retire_function(a.func)
        print(json.dumps(r, indent=2))
        MET.record_event("retire", a.func, r, exit_code=0 if r["ok"] else 1)
        if r["ok"]:
            _print_finish_nudge(a.func)
        return 0 if r["ok"] else 1

    if a.cmd == "diagnose":
        from collections import Counter
        verdicts = Counter()
        for f in a.funcs:
            try:
                d = DIAG.diagnose(f)
            except (KeyError, FileNotFoundError) as e:
                print(f"  SKIP {f}: {e}")
                continue
            verdicts[d["verdict"]] += 1
            MET.record_event("diagnose", f, d)
            print(f"  {d['verdict']:<13} {f:<30} (d{d['ndiff']}) {d['reason']}")
            if a.detail:
                for tag, t, b in d["pairs"]:
                    for x in t:
                        print(f"        TGT {x}")
                    for x in b:
                        print(f"        BLD {x}")
        print("\ntriage:", ", ".join(f"{v}={c}" for v, c in verdicts.most_common()))
        return 0

    if a.cmd == "canonical":
        r = CANON.classify(a.func) if a.fast else CANON.classify_full(a.func)
        print(json.dumps(r, indent=2))
        MET.record_event("canonical", a.func, r, extra={"fast": a.fast})
        return 0

    if a.cmd == "canonical-scan":
        from collections import Counter
        res = CANON.scan_all()
        c = Counter(r["verdict"] for r in res)
        print(f"canonical-asm classification of {len(res)} functions:")
        for v, n in c.most_common():
            print(f"  {v:<12} {n}")
        asm = [r for r in res if r["verdict"] in ("ASM-WHOLE", "ASM-PARTIAL")]
        partial = [r for r in asm if r["verdict"] == "ASM-PARTIAL"]
        print(f"\n{len(asm)} functions have canonical-asm regions -> auto-routed (no pure-C waste)")
        print(f"  {len(partial)} PARTIAL (pure C + inline-asm region), {len(asm) - len(partial)} WHOLE")
        for r in sorted(asm, key=lambda r: -r["asm_insns"])[:8]:
            print(f"    {r['verdict']:<12} {r['func']:<26} {r['asm_insns']}/{r['total']} asm  ({'; '.join(r['reasons'])})")
        return 0

    if a.cmd == "retire-redundant":
        res = ORCH.scan_file(a.file)
        redundant = [f for f, v in res.items() if v["redundant"]]
        print(f"{a.file}: {len(redundant)} redundant candidate(s): {redundant}")
        for f in redundant:
            r = INT.retire_function(f)
            print(f"  {'OK  ' if r['ok'] else 'FAIL'} {f}  dropped={r.get('total_dropped')}")
        return 0

    if a.cmd == "test":
        from . import test_engine
        return test_engine.main()

    return 2


if __name__ == "__main__":
    sys.exit(main())
