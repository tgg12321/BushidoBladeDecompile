"""Consolidated Tier-4 work queue — ONE ordered list of every function still
carrying a cheat (a regfix/asmfix rule OR a load-bearing tier-3 pin/inline-asm),
so the agent never triages or hunts for "easy" wins: it works the TOP item to
Tier-4, marks it done, and moves to the next. The order is fixed at generate
time (easiest-first by honest pure-C distance), so cherry-picking is impossible.

Outstanding = (function has >=1 regfix/asmfix rule) OR (honest pure-C distance
> 0, i.e. it does NOT byte-match once its rules are off AND its tier-3
pins/inline-asm are stripped). A masked-0 function that still carries rules
(e.g. a branch-retarget asmfix) is included — it sorts to the top as a likely
free retire. A function with neither rules nor a pure-C gap is already Tier-4
and is omitted.

Routing (mirrors canonical._verdict's structural tiers):
  C            distance <= SUSPECT (50)            -> active, pure-C target
  ASM-SUSPECT  SUSPECT < distance <= NEAR_CERTAIN  -> active, bounded attempt then PARK
  ASM-PARTIAL  opcode: canonical region + C        -> active, work the C part
  ASM-STRUCTURAL distance > NEAR_CERTAIN (500)     -> authorize (user canonical-asm sign-off)
  ASM-WHOLE    opcode: >=80% canonical             -> authorize
  JTBL-INFRA   rules are all jump-table rodata-split infra (cheats.is_jtbl_infra)
                                                   -> authorize (needs a global rodata reorder)

Statuses: active (work it, top first) | authorize (needs user sign-off, not
pure-C) | parked (blocked; skipped by `next`) | done (verified Tier-4). done /
parked are sticky across `regen`.

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
from . import oracle as O
from . import pipeline as P
from . import sandbox
from . import score

QUEUE_PATH = "engine/queue.json"
_AUTHORIZE = {"ASM-WHOLE", "ASM-STRUCTURAL", "JTBL-INFRA"}
_STATUS_RANK = {"active": 0, "authorize": 1, "parked": 2, "done": 3}


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
    """Rebuild the queue from scratch. One tier-3-stripped build per file scores
    every function in it (rules off + pins/inline-asm stripped == honest pure-C
    distance). done/parked statuses are preserved from the existing queue."""
    prev = {}
    if preserve and Path(QUEUE_PATH).exists():
        for it in load().get("items", []):
            if it.get("status") in ("done", "parked"):
                prev[it["func"]] = it
    verdicts = {r["func"]: r["verdict"] for r in canonical.scan_all()}
    items, failures = [], []
    for stem in sorted(P.c_stems()):
        ref_o = f"build/src/{stem}.o"
        if not Path(ref_o).exists():
            continue
        tier4_o = f"{workdir}/{stem}/{stem}.tier4.o"
        try:
            sandbox.build_stripped_object(stem, tier4_o, f"{workdir}/{stem}/cfg",
                                          strip_tier3=True)
        except Exception as e:  # tier-3 strip broke the build -> flag for manual look
            failures.append({"file": stem, "error": str(e)[:200]})
            continue
        for func in score._o_func_table(ref_o):
            try:
                dist = score.score_func(tier4_o, ref_o, func)["score"]
            except KeyError:
                continue
            rules = _rule_count(func)
            if rules == 0 and dist == 0:
                continue  # already Tier-4 (no cheat, byte-clean) -> not outstanding
            if func in prev:  # sticky done/parked
                items.append({**prev[func], "file": stem, "distance": dist, "rules": rules})
                continue
            if cheats.is_jtbl_infra(func):
                # canonical jump-table rodata-split infra — needs a global rodata
                # reorder (user-authorized), not per-function pure-C work.
                verdict, status = "JTBL-INFRA", "authorize"
            else:
                verdict = _route(verdicts.get(func, "C"), dist)
                status = "authorize" if verdict in _AUTHORIZE else "active"
            items.append({"func": func, "file": stem, "distance": dist,
                          "verdict": verdict, "rules": rules, "status": status})
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
    """Tier-4 gate: the function must carry ZERO regfix/asmfix rules AND the
    current build/ must still equal the oracle (run `retire`/`verify-oracle`
    first — they leave build/ current). Refuses otherwise."""
    rules = _rule_count(func)
    if rules > 0:
        return {"ok": False, "func": func,
                "reason": f"{rules} regfix/asmfix rule(s) still keyed to {func} — not Tier-4"}
    v = O.verify(rebuild=False)
    if not v.get("build_matches"):
        return {"ok": False, "func": func,
                "reason": "current build/ SHA1 != oracle — run `verify-oracle`/`retire` first"}
    q = load()
    if not any(it["func"] == func for it in q.get("items", [])):
        return {"ok": False, "func": func, "reason": "not in queue"}
    for it in q["items"]:
        if it["func"] == func:
            it["status"] = "done"
            it["completed_at"] = time.strftime("%Y-%m-%dT%H:%M:%S")
    q["items"].sort(key=_sort_key)
    q["counts"] = _counts(q["items"])
    save(q)
    return {"ok": True, "func": func, "sha1": v.get("build_sha1")}


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
