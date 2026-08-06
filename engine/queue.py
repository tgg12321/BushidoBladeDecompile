"""Consolidated INCOMPLETE-work queue — ONE ordered list of every function still
carrying a cheat (a regfix/asmfix rule, a load-bearing cheat-asm pin/inline
asm, or a plain `register` allocation hint). Presence in the queue =
INCOMPLETE. The agent never triages or hunts for
"easy" wins: it works the TOP item to completion, marks it done, and moves to
the next. The order is fixed at generate time (easiest-first by honest pure-C
distance), so cherry-picking is impossible.

Outstanding = (function has >=1 regfix/asmfix rule) OR (honest pure-C distance
> 0, i.e. it does NOT byte-match once its rules are off AND its cheat-asm pins,
plain register hints, and inline asm are stripped). A masked-0 function that
still carries rules (e.g. a branch-retarget asmfix) is included — it sorts to
the top as a likely free retire. A function with neither rules nor a pure-C gap
is already COMPLETED-C (or COMPLETED-INLINE-ASM-CANONICAL if listed in
inline_asm_canonical.txt) and is omitted.

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
import re
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


def _no_c_body(stem: str, func: str) -> bool:
    """True when src/<stem>.c supplies `func` wholly from asm (INCLUDE_ASM or a
    `glabel` block) and carries no C definition for it."""
    try:
        text = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    except OSError:
        return False
    return (func in inlineasm.whole_body_asm_funcs(text)
            and inlineasm._func_body_span(text, func) is None)


_AENT_RE = re.compile(r'\.aent\s+([A-Za-z_]\w*)')


def _not_a_c_function(stem: str, func: str) -> bool:
    """True when `func` is a symbol in the object's function table that is not a
    C-level function of src/<stem>.c at all, and therefore is not decomp work.

    This is what lets an UNKNOWN cheat count be dropped SAFELY. Without the
    distinction, "unknown" has two very different causes and treating both as
    not-clean floods the queue: measured 2026-08-06, 11 of the 12 remaining
    unknowns are non-C symbols, and they arrive with distance 0, which _sort_key
    puts at the very top of the active lane.

    Three mechanical signals, all conservative:
      * the name never appears in the .c text at all — it comes from an
        `.include`d asm body (8 symbols: D_80088BA0, the text1b D_8005xxxx set);
      * it is declared `.aent <name>` — an alternate ENTRY into another function,
        not a function of its own (3 symbols: g_data_start, g_module_func_tbl,
        g_module_type_tbl);
      * it is placed by an instruction-less `glabel <name>` marker block — an
        address marker, not a body (1 symbol: D_80081F1C, declared `extern u8`
        and taken by address).

    A body-span parse failure matches NONE of them, so it is retained as
    outstanding rather than silently dropped.
    """
    try:
        text = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    except OSError:
        return False
    if re.search(r'\b' + re.escape(func) + r'\b', text) is None:
        return True
    return (func in set(_AENT_RE.findall(text))
            or func in inlineasm.symbol_marker_funcs(text))


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
    every function in it (rules off + pins/plain-register-hints/inline-asm
    stripped == honest pure-C distance). `parked` statuses are preserved from
    the existing queue.
    `active`/`authorize` are recomputed. COMPLETED items are not in the queue."""
    prev = {}
    if preserve and Path(QUEUE_PATH).exists():
        for it in load().get("items", []):
            # `owner_override` is carried across too: routing here is mechanical
            # (distance > NEAR_CERTAIN -> ASM-STRUCTURAL -> authorize) and an owner
            # ruling can overturn it for a specific function. Without this, regen
            # would silently re-route the item and revert the ruling. First use:
            # the 26 functions the 2026-06-09 canonical-asm audit REJECTED (they
            # were auto-routed by the distance>500 heuristic, which that audit
            # found is not evidence of hand-coded asm) and the owner returned to
            # active on 2026-08-01.
            if (it.get("status") == "parked" or it.get("origin") == "regression"
                    or it.get("owner_override")):
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
            prologue = cheats.func_prologue_count(func)
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
                if rules == 0 and prologue == 0 and (
                        cheats_unscored == 0 or func in canon_funcs
                        or (cheats_unscored < 0 and _not_a_c_function(stem, func))):
                    continue  # nothing to track
                dist = -1
                scorable = False
                if cheats_unscored > 0 and _no_c_body(stem, func):
                    # Asm-supplied with no C at all: the honest pure-C distance
                    # is the whole function, not "unknown". Keeps easiest-first
                    # ordering meaningful instead of parking it at -1.
                    try:
                        dist = len(score.normalized_insns(ref_o, func))
                        scorable = True
                    except KeyError:
                        pass
            cheat_count = inlineasm.file_func_cheat_asm_count(stem, func)
            if rules == 0 and prologue == 0:
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
                #
                # ONLY a measured-zero count may drop. A NEGATIVE count is
                # UNKNOWN and must never read as clean — that is precisely how an
                # undecompiled function silently left the queue (ang_hosei,
                # 2026-08-06). The one safe exception is a symbol that is not a
                # C-level function at all (`_not_a_c_function`), which is not
                # decomp work in the first place.
                if dist == 0 and (cheat_count == 0
                                  or (cheat_count < 0 and _not_a_c_function(stem, func))):
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
            elif cheats.is_canonical_extraction_only(func):
                # Authorized canonical body whose ONLY rule is the
                # [infra-rule: canonical-asm-extraction] replace_with_asmfile
                # wiring. Tracked debt (NOT complete — zero-rules bar), but
                # not per-function grind work: the wiring is retired by the
                # INCLUDE_ASM/TU-resplit campaign. Keep it visible outside
                # the active lane, like jtbl-infra.
                verdict, status = "CANON-EXTRACT", "authorize"
            elif _no_c_body(stem, func):
                # Asm-supplied, no C at all. `dist` here is definitionally "the
                # whole function is missing", so it carries ZERO information
                # about whether the original was hand-written asm — feeding it to
                # _route would send every large conversion to ASM-STRUCTURAL and
                # out of the active lane on exactly the distance heuristic
                # .claude/rules/canonical-gate-distance-not-evidence.md
                # disclaims (and that the owner overturned for 26 functions on
                # 2026-08-01). Keep the distance for ORDERING, route on the
                # opcode verdict alone.
                verdict = _route(verdicts.get(func, "C"), 0)
                status = "authorize" if verdict in _AUTHORIZE else "active"
            else:
                verdict = _route(verdicts.get(func, "C"), dist)
                status = "authorize" if verdict in _AUTHORIZE else "active"
            entry = {"func": func, "file": stem, "distance": dist,
                     "verdict": verdict, "rules": rules, "status": status}
            if prologue:
                entry["prologue_fix"] = prologue
            if not scorable:
                entry["scorable"] = False
            items.append(entry)
    # re-add ONLY regression-origin items the scan can't derive (their cheat
    # is semantic); a parked item the scan no longer produces is mechanically
    # complete and must drop, as before.
    have = {it["func"] for it in items}
    for fn, it in prev.items():
        if fn not in have and it.get("origin") == "regression":
            items.append(it)
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
    (2) NO cheat construct in source UNLESS it is authorized canonical-asm
    (inline_asm_canonical.txt), and (3) the current build/ still equals the
    oracle. (2) is what stops a cheated 'match' (register pins, plain
    `register` allocator hints, `move $N,$N` injection, scheduling barriers)
    from being recorded as completed — SHA1 alone can't catch it, since cheat
    constructs can produce the right bytes.

    On success, the function is REMOVED from the queue (queue presence =
    INCOMPLETE; completed items don't live there)."""
    q = load()
    item = next((it for it in q.get("items", []) if it["func"] == func), None)
    if item is None:
        return {"ok": False, "func": func, "reason": "not in queue"}
    rules = _rule_count(func)
    if rules > 0:
        if cheats.is_canonical_extraction_only(func):
            return {"ok": False, "func": func,
                    "reason": (f"{func} is canonical-authorized but its "
                               f"replace_with_asmfile wiring is still in asmfix.txt "
                               f"— transitional infrastructure debt (2026-08-06 "
                               f"ruling; zero-rules bar). It is retired by the "
                               f"INCLUDE_ASM/TU-resplit campaign "
                               f"(docs/superpowers/specs/2026-08-06-tu-resplit-"
                               f"campaign.md), not by queue done. The function "
                               f"routes to CANON-EXTRACT/authorize on regen.")}
        return {"ok": False, "func": func,
                "reason": (f"{rules} regfix/asmfix rule(s) still keyed to {func} — "
                           f"not COMPLETED-C")}
    pcount = cheats.func_prologue_count(func)
    if pcount > 0:
        return {"ok": False, "func": func,
                "reason": (f"{pcount} prologue_fix entry(ies) keyed to {func} "
                           f"(prologue_config/frame_fix/delay_slot_ra) — prologue_fix "
                           f"reorders cc1's OWN prologue into a target order it did NOT "
                           f"compile (a cheat; audit 2026-06-15). Delete the entry: if "
                           f"cc1 then byte-matches it is COMPLETED-C, else find the C "
                           f"lever or authorize canonical-asm.")}
    gates = cheats.maspsx_gate_entries(func)
    if func not in cheats.canonical_asm_funcs():
        bad_gates = [p for p, cls in gates if cls == "cheat-pathway"]
        if bad_gates:
            return {"ok": False, "func": func,
                    "reason": (f"{func} is gated in {', '.join(bad_gates)} — a "
                               f"cheat-pathway maspsx gate (a pure-C spelling exists "
                               f"for that effect; e.g. unsigned operands for multu). "
                               f"Fix the C and delete the gate entry; see "
                               f".claude/rules/maspsx-gate-lists.md.")}
        cheat_count = inlineasm.file_func_cheat_asm_count(item["file"], func)
        if cheat_count < 0:
            # UNKNOWN, not clean. No C body could be located for a function that
            # is not canonical-authorized, so there is nothing to certify as
            # pure C. Refusing is the safe direction: treating unknown as zero is
            # exactly how an undecompiled function gets recorded COMPLETED-C.
            return {"ok": False, "func": func,
                    "reason": (f"cheat-construct count for {func} is UNKNOWN — no C "
                               f"definition found in src/{item['file']}.c. A function "
                               f"with no C body cannot be COMPLETED-C. If its body is "
                               f"supplied from asm (INCLUDE_ASM or a glabel block), it "
                               f"is undecompiled and stays INCOMPLETE; if it is "
                               f"genuinely hand-written asm, authorize it in "
                               f"inline_asm_canonical.txt with evidence.")}
        if cheat_count > 0:
            return {"ok": False, "func": func,
                    "reason": (f"{cheat_count} cheat construct(s) in src/{item['file']}.c "
                               f"— NOT pure-C COMPLETED. Strip them (pure C), or, only "
                               f"if {func} is genuinely hand-written/canonical asm, "
                               f"authorize it in inline_asm_canonical.txt with evidence. "
                               f"(register pins, plain register hints, hardcoded-$N asm, "
                               f"scheduling barriers are cheats, not a match.)")}
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
    result = {"ok": True, "func": func, "completion": completion_state,
              "sha1": v.get("build_sha1")}
    if gates:
        # Transparency, not refusal: fidelity-class gates model the original
        # assembler (no C spelling exists) — record the dependency so the
        # completion is auditable (.claude/rules/maspsx-gate-lists.md).
        result["maspsx_gates"] = [p for p, _cls in gates]
    return result


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


def reopen(func: str, file: str, reason: str = "", origin: str = "regression") -> dict:
    """Re-open a previously-COMPLETED function as an ordinary INCOMPLETE queue
    item. For semantic-audit regressions (owner ruling 2026-07-06): the
    committed code byte-matches (0 rules, 0 honest distance, 0 cheat-asm), so
    `generate()`'s mechanical scan treats it as COMPLETED-C and can never
    re-derive it as outstanding on its own. `origin: "regression"` items are
    carried across regen the same way `parked` items are (see generate())."""
    q = load()
    items = q.setdefault("items", [])
    if any(it["func"] == func for it in items):
        return {"ok": False, "func": func, "reason": f"{func} already in queue"}
    item = {"func": func, "file": file, "distance": 0, "verdict": "C",
            "rules": 0, "status": "active", "origin": origin,
            "reopen_reason": (reason or "")[:400]}
    items.append(item)
    items.sort(key=_sort_key)
    q["items"] = items
    q["counts"] = _counts(items)
    save(q)
    return {"ok": True, "func": func, "item": item}


def status() -> dict:
    q = load()
    return {"counts": q.get("counts", _counts(q.get("items", []))),
            "generated_at": q.get("generated_at"), "next": next_item()}
