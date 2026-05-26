#!/usr/bin/env python3
"""Orchestrator post-run review packet for a headless worker run.

Consolidates everything the orchestrator needs to confirm a run WITHOUT reading
the transcript, and recommends ACCEPT vs ESCALATE per the maximal-autonomy
boundary:

  ESCALATE (stop, wait for the user) when:
    - the oracle broke (worker committed a non-matching build), or
    - the worker errored, or
    - it made no progress (neither completed nor parked = stuck), or
    - it PARKED for a reason the orchestrator can't mechanically confirm (novel).
  ACCEPT (proceed autonomously, log it) otherwise:
    - a clean Tier-4 completion (oracle still matches), or
    - a park the orchestrator CAN confirm (e.g. jtbl-infra: rules verified as
      canonical jump-table infrastructure).

Pulls from metrics/headless_runs.jsonl (the run record), the session transcript
(via headless_audit), engine/queue.json (park status/reason), and git (the
commit the worker made). Output is compact; --json for machine use.

Usage:
  python3 tools/headless_review.py --latest
  python3 tools/headless_review.py --func replay_camera_rob_back_loose2
  python3 tools/headless_review.py --session <uuid> [--json]
"""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(REPO))
import headless_audit as ha           # noqa: E402
from engine import cheats, inlineasm  # noqa: E402

RUNLOG = REPO / "metrics" / "headless_runs.jsonl"
QUEUE = REPO / "engine" / "queue.json"


def _git(*args) -> str:
    try:
        return subprocess.run(["git", "-C", str(REPO), *args],
                              capture_output=True, text=True).stdout.strip()
    except Exception:
        return ""


def load_runs() -> list[dict]:
    if not RUNLOG.exists():
        return []
    out = []
    for line in RUNLOG.read_text(encoding="utf-8").splitlines():
        if line.strip():
            try:
                out.append(json.loads(line))
            except Exception:
                pass
    return out


def queue_item(func: str) -> dict | None:
    if not QUEUE.exists():
        return None
    for it in json.loads(QUEUE.read_text()).get("items", []):
        if it.get("func") == func:
            return it
    return None


def confirm_park(func: str, item: dict | None) -> tuple[str, str]:
    """(verdict, detail) — can the orchestrator mechanically confirm this park?"""
    if cheats.is_jtbl_infra(func):
        nfix = len(cheats.func_rule_lines(func, cheats.ASMFIX))
        return ("AUTO-CONFIRMED",
                f"jtbl-infra: all {nfix} rules are jump-table asmfix (rename/"
                f"replace_first/delete_between, references jtbl_*), zero regfix. "
                f"Canonical rodata-split infra; needs a global rodata reorder.")
    reason = (item or {}).get("park_reason", "")
    return ("NEEDS-REVIEW",
            f"park reason not mechanically confirmable — orchestrator judgment "
            f"required. reason: {reason!r}")


def classify_outcome(rec: dict, item: dict | None) -> str:
    if not rec.get("oracle_ok", True):
        return "ORACLE-BREAK"
    if rec.get("advanced"):
        # committed progress + oracle intact == success, even if the session
        # later errored or hit the budget cutoff (is_error). The oracle is the
        # ground truth: a byte-identical build means the committed work is right.
        return "TIER4-DONE"
    if rec.get("is_error"):
        return "ERROR"
    if (item or {}).get("status") == "parked" or rec.get("progressed"):
        return "PARKED"
    return "STUCK"


def uncommitted_leftover() -> list[str]:
    """Worker work left UNCOMMITTED (e.g. a budget cutoff mid-function). Excludes
    metrics/* (append-only logs the runner's own post-check writes). Non-empty =>
    the tree is dirty and the loop must NOT continue onto it."""
    out = []
    for ln in _git("status", "--porcelain").splitlines():
        if ln.strip() and "metrics/" not in ln:
            out.append(ln.strip())
    return out


def review(rec: dict) -> dict:
    func = rec.get("func")
    item = queue_item(func)
    outcome = classify_outcome(rec, item)

    # transcript-derived efficiency signals
    sid = rec.get("session_id", "")
    pdir = ha.find_projects_dir(None)
    tp = pdir / f"{sid}.jsonl" if (pdir and (pdir / f"{sid}.jsonl").exists()) else None
    audit = ha.digest_transcript(tp) if tp else {}

    # the worker's commit(s): use the exact head_before..head_after range the
    # runner recorded. If the heads are equal the worker committed NOTHING this
    # run -- show that, do NOT grep (grep surfaces unrelated OLD commits that
    # merely mention the func, which is misleading). Only fall back to grep for
    # legacy records that predate head tracking.
    hb, hbafter = rec.get("head_before"), rec.get("head_after")
    if hb and hbafter:
        if hb != hbafter:
            log = _git("log", f"{hb}..{hbafter}", "--format=%h %s")
            commits = [c for c in log.splitlines() if c.strip()]
            stat = _git("diff", "--stat", hb, hbafter)
        else:
            commits = []          # worker made no commit this run
            stat = "(no commit this run)"
    else:
        log = _git("log", "--grep", func, "-n", "2", "--format=%h %s")
        commits = [c for c in log.splitlines() if c.strip()]
        stat = _git("show", "--stat", "--format=", commits[0].split()[0]) if commits else ""

    park_verdict = park_detail = None
    if outcome == "PARKED":
        park_verdict, park_detail = confirm_park(func, item)

    leftover = uncommitted_leftover()

    # Tier-4 integrity: a COMMITTED match must not carry tier-3 inline asm
    # (register pins / hardcoded-$N asm / scheduling barriers) unless the function
    # is authorized canonical-asm. SHA1 can't catch this (tier-3 emits right bytes),
    # so audit the committed source directly.
    match_tier3 = 0
    if rec.get("advanced") and rec.get("file") and func not in cheats.canonical_asm_funcs():
        try:
            match_tier3 = max(0, inlineasm.file_func_tier3_count(rec["file"], func))
        except Exception:
            match_tier3 = 0

    # apply the maximal-autonomy escalation boundary
    if leftover:
        # a dirty tree (uncommitted worker work, e.g. budget cutoff mid-function)
        # must be resolved before the loop continues — never run onto it.
        decision, why = "ESCALATE", f"uncommitted leftover ({len(leftover)} path(s)) — orchestrator must commit/revert before continuing"
    elif match_tier3 > 0:
        decision, why = "ESCALATE", (f"CHEATED MATCH: {func} committed with {match_tier3} tier-3 "
                                     f"inline-asm block(s), non-canonical — NOT Tier-4 pure C")
    elif outcome in ("ORACLE-BREAK", "ERROR", "STUCK"):
        decision, why = "ESCALATE", f"outcome={outcome}"
    elif outcome == "PARKED" and park_verdict != "AUTO-CONFIRMED":
        decision, why = "ESCALATE", "park not mechanically confirmable (novel)"
    else:
        decision, why = "ACCEPT", f"outcome={outcome}" + (
            " (park auto-confirmed)" if outcome == "PARKED" else "")
        if audit and (audit.get("error_results") or audit.get("footgun_blocks")):
            why += "; note: tooling friction flagged (review audit)"

    return {
        "func": func, "file": rec.get("file"), "outcome": outcome,
        "decision": decision, "why": why, "uncommitted_leftover": leftover,
        "match_tier3": match_tier3,
        "oracle_ok": rec.get("oracle_ok"), "model": rec.get("model"),
        "cost_usd": rec.get("cost_usd"), "num_turns": rec.get("num_turns"),
        "park_confirmation": park_verdict, "park_detail": park_detail,
        "commits": commits, "commit_stat": stat,
        "audit": {k: audit.get(k) for k in
                  ("turns", "tool_calls_total", "error_results", "footgun_blocks",
                   "duplicate_cmd_count", "error_breakdown", "eng_subcmds")} if audit else None,
    }


def print_human(r: dict) -> None:
    print(f"=== POST-RUN REVIEW: {r['func']} ({r['file']}) ===")
    print(f"  model={r['model']}  cost=${r['cost_usd']}  turns={r['num_turns']}  oracle_ok={r['oracle_ok']}")
    print(f"  OUTCOME: {r['outcome']}")
    a = r.get("audit")
    if a:
        flag = "REVIEW" if (a.get("error_results") or a.get("footgun_blocks") or a.get("duplicate_cmd_count")) else "OK"
        print(f"  audit [{flag}]: tooling_errors={a.get('error_results')} footgun_blocks={a.get('footgun_blocks')} "
              f"dup_cmds={a.get('duplicate_cmd_count')}  engine_loop={a.get('eng_subcmds')}")
        if a.get("error_breakdown"):
            print(f"    error breakdown: {a['error_breakdown']}")
    if r.get("uncommitted_leftover"):
        print(f"  UNCOMMITTED LEFTOVER ({len(r['uncommitted_leftover'])}): {r['uncommitted_leftover'][:6]}")
    if r.get("match_tier3"):
        print(f"  ** CHEATED MATCH: {r['match_tier3']} tier-3 inline-asm block(s) in committed source (non-canonical)")
    if r.get("park_confirmation"):
        print(f"  PARK: [{r['park_confirmation']}] {r['park_detail']}")
    if r.get("commits"):
        print(f"  commit(s): {'; '.join(r['commits'])}")
        if r.get("commit_stat"):
            for ln in r["commit_stat"].splitlines():
                print(f"    {ln}")
    print(f"  >>> ORCHESTRATOR DECISION: {r['decision']} ({r['why']})")


def main() -> int:
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--latest", action="store_true")
    g.add_argument("--session")
    g.add_argument("--func")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()

    runs = load_runs()
    if not runs:
        print("no headless runs recorded (metrics/headless_runs.jsonl empty/absent).")
        return 0
    rec = None
    if a.session:
        rec = next((r for r in reversed(runs) if r.get("session_id") == a.session), None)
    elif a.func:
        rec = next((r for r in reversed(runs) if r.get("func") == a.func), None)
    else:  # --latest (default)
        rec = runs[-1]
    if not rec:
        print("no matching run record.")
        return 1

    r = review(rec)
    if a.json:
        print(json.dumps(r, indent=2, default=str))
    else:
        print_human(r)
    # exit code signals the orchestrator decision: 0 ACCEPT, 10 ESCALATE
    return 0 if r["decision"] == "ACCEPT" else 10


if __name__ == "__main__":
    raise SystemExit(main())
