"""Consolidated INCOMPLETE-work queue — ONE ordered list of every function still
carrying a cheat (a regfix/asmfix rule OR a load-bearing cheat-asm pin/inline
asm). Presence in the queue = INCOMPLETE. The agent never triages or hunts for
"easy" wins: it works the TOP item to completion, marks it done, and moves to
the next. The order is fixed at generate time (easiest-first by honest pure-C
distance), so cherry-picking is impossible.

Outstanding = (function has >=1 regfix/asmfix rule) OR (honest pure-C distance
> 0, i.e. it does NOT byte-match once its rules are off AND its cheat-asm pins/
inline asm are stripped). A masked-0 function that still carries rules (e.g. a
branch-retarget asmfix) is included — it sorts to the top as a likely free
retire. A function with neither rules nor a pure-C gap is already COMPLETED-C
(or COMPLETED-INLINE-ASM-CANONICAL if listed in inline_asm_canonical.txt) and
is omitted.

Routing (mirrors canonical._verdict's structural categories):
  C            distance <= SUSPECT (50)            -> active, pure-C target
  ASM-SUSPECT  SUSPECT < distance <= NEAR_CERTAIN  -> active, bounded attempt then PARK
  ASM-PARTIAL  opcode: canonical region + C        -> active, work the C part
  ASM-STRUCTURAL distance > NEAR_CERTAIN (500)     -> authorize (user canonical-asm sign-off)
  ASM-WHOLE    opcode: >=80% canonical             -> authorize
  JTBL-INFRA   rules are all jump-table rodata-split infra (cheats.is_jtbl_infra)
                                                   -> authorize (needs a global rodata reorder)

Sub-statuses (all mean INCOMPLETE — completed items DROP off the queue
entirely):
  active     work it (top first)
  authorize  needs user canonical-asm sign-off — not pure-C-closable
  parked     blocked; skipped by `next`. Sticky across `regen`.

Queue file: engine/queue.json (committed). Driven by `python3 -m engine.cli
queue {next,done,park,status,regen}`.
"""
from __future__ import annotations

import json
import time
from collections import Counter
from pathlib import Path

from . import buildconfig as cfg
from . import canonical
from . import cheats
from . import inlineasm
from . import oracle as O
from . import pipeline as P
from . import sandbox
from . import score

QUEUE_PATH = "engine/queue.json"
_AUTHORIZE = {"ASM-WHOLE", "ASM-STRUCTURAL", "JTBL-INFRA"}
_STATUS_RANK = {"active": 0, "authorize": 1, "parked": 2}


def load() -> dict:
    p = Path(QUEUE_PATH)
    return json.loads(p.read_text()) if p.exists() else {"items": []}


def save(q: dict) -> None:
    Path(QUEUE_PATH).write_text(json.dumps(q, indent=2) + "\n")


def _rule_count(func: str) -> int:
    return (len(cheats.func_rule_lines(func, cheats.REGFIX))
            + len(cheats.func_rule_lines(func, cheats.REGFIX2))
            + len(cheats.func_rule_lines(func, cheats.ASMFIX)))


def _route(opcode_verdict: str, distance: int) -> str:
    """(opcode verdict, honest pure-C distance) -> queue verdict."""
    if opcode_verdict in ("ASM-WHOLE", "ASM-PARTIAL"):
        return opcode_verdict
    if distance > canonical.NEAR_CERTAIN_DISTANCE:
        return "ASM-STRUCTURAL"
    if distance > canonical.SUSPECT_DISTANCE:
        return "ASM-SUSPECT"
    return "C"


def _sort_key(it: dict):
    return (_STATUS_RANK.get(it["status"], 0), it["distance"], it["file"], it["func"])


def _counts(items: list[dict]) -> dict:
    by_status = Counter(it["status"] for it in items)
    by_verdict = Counter(it["verdict"] for it in items
                         if it["status"] in ("active", "authorize"))
    return {"total": len(items), "active": by_status.get("active", 0),
            "by_status": dict(by_status), "by_verdict": dict(by_verdict)}


def generate(workdir: str = "tmp/queue", preserve: bool = True) -> dict:
    """Rebuild the queue from scratch. One cheat-stripped build per file scores
    every function in it (rules off + pins/inline-asm stripped == honest pure-C
    distance). `parked` statuses are preserved from the existing queue.
    `active`/`authorize` are recomputed. COMPLETED items are not in the queue."""
    prev = {}
    if preserve and Path(QUEUE_PATH).exists():
        for it in load().get("items", []):
            if it.get("status") == "parked":
                prev[it["func"]] = it
    verdicts = {r["func"]: r["verdict"] for r in canonical.scan_all()}
    canon_funcs = cheats.canonical_asm_funcs()
    items, failures = [], []
    for stem in sorted(P.c_stems()):
        ref_o = f"build/src/{stem}.o"
        if not Path(ref_o).exists():
            continue
        purec_o = f"{workdir}/{stem}/{stem}.purec.o"
        try:
            sandbox.build_stripped_object(stem, purec_o, f"{workdir}/{stem}/cfg",
                                          strip_cheat_asm=True)
        except Exception as e:  # cheat-asm strip broke the build -> flag for manual look
            failures.append({"file": stem, "error": str(e)[:200]})
            continue
        for func in score._o_func_table(ref_o):
            rules = _rule_count(func)
            try:
                dist = score.score_func(purec_o, ref_o, func)["score"]
                scorable = True
            except KeyError:
                # The stripped build is missing this function — typically because
                # a SIBLING function's index-anchored regfix-reorder rule crashed
                # the pipeline after cheat-asm/volatile strip shifted maspsx
                # indices (see [[jtbl-rodata-split-infrastructure]]). Do NOT
                # silently drop — if the function carries cheats (rules > 0 OR
                # detected cheat-asm/volatile cheats), it is STILL outstanding
                # and must stay in the queue, even though we can't measure
                # honest distance here. Distance is recorded as -1 to indicate
                # unscored.
                cheats_unscored = inlineasm.file_func_cheat_asm_count(stem, func)
                if rules == 0 and (cheats_unscored <= 0 or func in canon_funcs):
                    continue  # nothing to track
                dist = -1
                scorable = False
            cheat_count = inlineasm.file_func_cheat_asm_count(stem, func)
            if rules == 0:
                # COMPLETED-INLINE-ASM-CANONICAL: function is in inline_asm_canonical.txt
                # and carries 0 rules. The inline asm IS the accepted finished form,
                # so masked sandbox distance is meaningless here (the cheat-strip removes
                # the canonical body, producing a "distance" that just measures the
                # missing canonical insns). mark_done accepts this directly — regen
                # must drop them too, or they sit in the queue forever waiting to be
                # rediscovered (each ~$3 of agent time per false top).
                if func in canon_funcs:
                    continue
                # COMPLETED-C: 0 rules + 0 distance + 0 cheat-asm = pure-C byte-clean.
                if dist == 0 and cheat_count <= 0:
                    continue
            if func in prev:  # sticky parked
                pv = prev[func]
                # parked -> sticky regardless (until the user un-parks via regen
                # after fixing the underlying blocker).
                items.append({**pv, "file": stem, "distance": dist, "rules": rules})
                continue
            if cheats.is_jtbl_infra(func):
                # canonical jump-table rodata-split infra — needs a global rodata
                # reorder (user-authorized), not per-function pure-C work.
                verdict, status = "JTBL-INFRA", "authorize"
            else:
                verdict = _route(verdicts.get(func, "C"), dist)
                status = "authorize" if verdict in _AUTHORIZE else "active"
            entry = {"func": func, "file": stem, "distance": dist,
                     "verdict": verdict, "rules": rules, "status": status}
            if not scorable:
                entry["scorable"] = False
            items.append(entry)
    items.sort(key=_sort_key)
    q = {"generated_at": time.strftime("%Y-%m-%dT%H:%M:%S"),
         "oracle_sha1": cfg.ORACLE_SHA1, "build_failures": failures, "items": items}
    q["counts"] = _counts(items)
    save(q)
    return q


def next_item() -> dict | None:
    for it in load().get("items", []):
        if it["status"] == "active":
            return it
    return None


def mark_done(func: str) -> dict:
    """COMPLETED gate: a function is DONE only if (1) ZERO regfix/asmfix rules,
    (2) NO cheat-asm in source UNLESS it is authorized canonical-asm
    (inline_asm_canonical.txt), and (3) the current build/ still equals the
    oracle. (2) is what stops a cheated 'match' (register pins / `move $N,$N`
    injection / scheduling barriers) from being recorded as completed — SHA1
    alone can't catch it, since cheat asm produces the right bytes.

    On success, the function is REMOVED from the queue (queue presence =
    INCOMPLETE; completed items don't live there)."""
    q = load()
    item = next((it for it in q.get("items", []) if it["func"] == func), None)
    if item is None:
        return {"ok": False, "func": func, "reason": "not in queue"}
    rules = _rule_count(func)
    if rules > 0:
        return {"ok": False, "func": func,
                "reason": (f"{rules} regfix/asmfix rule(s) still keyed to {func} — "
                           f"not COMPLETED-C")}
    if func not in cheats.canonical_asm_funcs():
        cheat_count = inlineasm.file_func_cheat_asm_count(item["file"], func)
        if cheat_count > 0:
            return {"ok": False, "func": func,
                    "reason": (f"{cheat_count} cheat-asm block(s) in src/{item['file']}.c "
                               f"— NOT pure-C COMPLETED. Strip them (pure C), or, only "
                               f"if {func} is genuinely hand-written/canonical asm, "
                               f"authorize it in inline_asm_canonical.txt with evidence. "
                               f"(register pins, hardcoded-$N asm, scheduling barriers "
                               f"are cheats, not a match.)")}
    v = O.verify(rebuild=False)
    if not v.get("build_matches"):
        return {"ok": False, "func": func,
                "reason": "current build/ SHA1 != oracle — run `verify-oracle`/`retire` first"}
    # Drop the item — queue presence = INCOMPLETE; completion removes it.
    q["items"] = [it for it in q["items"] if it["func"] != func]
    q["items"].sort(key=_sort_key)
    q["counts"] = _counts(q["items"])
    save(q)
    completion_state = ("COMPLETED-INLINE-ASM-CANONICAL"
                        if func in cheats.canonical_asm_funcs()
                        else "COMPLETED-C")
    return {"ok": True, "func": func, "completion": completion_state,
            "sha1": v.get("build_sha1")}


def mark_parked(func: str, reason: str = "") -> dict:
    q = load()
    if not any(it["func"] == func for it in q.get("items", [])):
        return {"ok": False, "func": func, "reason": "not in queue"}
    for it in q["items"]:
        if it["func"] == func:
            it["status"] = "parked"
            it["park_reason"] = reason
    q["items"].sort(key=_sort_key)
    q["counts"] = _counts(q["items"])
    save(q)
    return {"ok": True, "func": func, "park_reason": reason}


def status() -> dict:
    q = load()
    return {"counts": q.get("counts", _counts(q.get("items", []))),
            "generated_at": q.get("generated_at"), "next": next_item()}
