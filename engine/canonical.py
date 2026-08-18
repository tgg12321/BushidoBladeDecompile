"""Canonical-asm INTAKE gate — deterministic C-vs-asm classification BEFORE any
pure-C effort, so agents never (a) grind an impossible pure-C match or (b) reach
for asm injection to brute-force bytes. The decision is the gate's, not the
agent's.

The target disasm comes from `objdump -d build/bb2.elf` (resolved via the ELF
symbol table / disasm headers). This is name-independent and exact-per-function
— it does NOT depend on asm/funcs/<name>.s, which is fragile: asm files aren't
1:1 with functions (e.g. motion_Close @0x80083804 lives inside motion_Open.s)
and their names lag behind renamed symbols.

Definitive signals in that disasm:
  * cop2/GTE ops — no C analog. objdump renders the GTE transfers as mnemonics
    (mtc2/mfc2/ctc2/cfc2/lwc2/swc2) and the GTE command words as the generic
    coprocessor mnemonic `c2  0x......` (e.g. 4aa00428 -> `c2 0xa00428`).
  * syscall (BIOS trampoline)
  * raw .word (undisassembled instruction / data-in-text)

A function/region with these is AUTO-ROUTED to canonical-asm (no pure-C attempt).
Region-granular, so a partially-asm function gets pure C for the C parts and
inline-asm only for the genuinely-asm span. A function with NO definitive signal
is 'C' (the pure-C ladder); if that ladder plateaus, the controller PARKs with
evidence and escalates to a user canonical-asm call — that's the gate for the
heuristic/uncertain cases (auth model: auto-route definitive, gate the rest).
"""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

from . import buildconfig as cfg

# Distance thresholds for STRUCTURAL (non-opcode) hand-asm detection. The
# masked cheat-invisible distance = how far GCC's natural pure-C output sits
# from the target. Genuinely-C functions sit close (regfix does minor reg-alloc
# fix-ups); hand-asm sits far (GCC would never emit that shape). Calibration
# knobs.
#
# 2026-06-09 calibration update: the prior `distance > 500` → ASM-STRUCTURAL
# auto-escalation was too aggressive — a 1400-insn pure-C function with
# accumulated RA/scheduling drift trivially exceeds 500. The 2026-06-09 audit
# of the 26 ASM-STRUCTURAL items in the authorize bucket found 0/26 had any
# hand-coded signals (scan_hand_coded LOW for all); they were just LARGE
# functions auto-misrouted. Fix: ASM-STRUCTURAL now requires the distance
# threshold AND an independent hand-coded signal (scan_hand_coded tier ≥
# POSSIBLE). Without that corroboration, large distance demotes to
# ASM-SUSPECT (bounded pure-C attempt, then PARK) — keep pushing pure C, the
# size alone isn't evidence the function was hand-written.
# See `.claude/rules/canonical-gate-distance-not-evidence.md`.
#
# 2026-08-18 follow-up: a four-agent triage of all 47 ASM-SUSPECT queue items
# found ZERO hand-coded candidates (every one tier LOW, none of the decisive
# S1/S2/S6 signals) — the band is dominated by functions with NO C body written
# yet, whose "distance" is just the full instruction count. A scanner-confirmed
# LOW tier is therefore treated as affirmative counter-evidence and the item is
# re-verdicted an ordinary C target; the tier is recorded on the verdict so the
# evidence travels with the routing.
SUSPECT_DISTANCE = 50       # > this: consult scan_hand_coded (SUSPECT unless tier=LOW)
NEAR_CERTAIN_DISTANCE = 500  # > this AND hand-coded tier ≥ POSSIBLE: ASM-STRUCTURAL.
                             # > this WITHOUT signal: ASM-SUSPECT (or C if tier=LOW).

# Unambiguous cop2/GTE mnemonics objdump knows by name. 'break' is EXCLUDED
# (GCC emits it for div-by-zero / overflow traps — not an asm signal).
_GTE = re.compile(
    r"^(mtc2|mfc2|ctc2|cfc2|cop2|lwc2|swc2|"
    r"rtps|rtpt|nclip|avsz3|avsz4|mvmva|gpf|gpl|sqr|dcpl|dpcs|dpct|intpl|"
    r"ncs|nct|ncds|ncdt|cdp|nccs|ncct|ncc|op|cc)$"
)

# ELF symbol-table line:  <vram> <flags...> F <section> <size> <name>
_TSYM = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+\S+\s+([0-9a-fA-F]+)\s+(\S+)\s*$")
# objdump -d instruction line:  "   8002ec84:\t<hex>\t<mnem> <operands>"
_DLINE = re.compile(r"^\s*[0-9a-f]+:\s+[0-9a-f]+\s+(\S+)(.*)$")

_table_cache: dict | None = None


def _func_table() -> dict:
    """name -> (vram, size) from build/bb2.elf F-symbols (cached). A 0-size
    symbol (objdump occasionally emits one) gets a gap-fallback to the next
    function start so its range is still bounded."""
    global _table_cache
    if _table_cache is None:
        out = subprocess.run([cfg.OBJDUMP, "-t", "build/bb2.elf"],
                             capture_output=True, text=True).stdout
        syms = {}
        for line in out.splitlines():
            m = _TSYM.match(line)
            if m:
                syms[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
        starts = sorted({v for v, _ in syms.values()})
        nxt = {starts[i]: starts[i + 1] for i in range(len(starts) - 1)}
        _table_cache = {}
        for name, (vram, size) in syms.items():
            if size == 0:
                size = nxt.get(vram, vram + 4) - vram
            _table_cache[name] = (vram, size)
    return _table_cache


_STRUCTURAL = {"nop", "jr"}  # pure padding/return — not "C work" (excluded from
                             # the ASM-WHOLE denominator so a tiny GTE leaf wrapper
                             # isn't dragged below the threshold by its nops + jr).


def _detect(dlines) -> tuple[list, int, int]:
    """Scan objdump -d instruction lines -> (hits, total_insns, structural). hits
    is a list of (idx, mnemonic, reason) for definitive-asm instructions;
    structural counts nop/jr (return + delay-slot padding)."""
    hits, idx, structural = [], 0, 0
    for line in dlines:
        m = _DLINE.match(line)
        if not m:
            continue  # symbol header / blank
        mn, ops = m.group(1), m.group(2)
        if mn in _STRUCTURAL:
            structural += 1
        reason = None
        if mn == ".word":
            reason = "raw .word (undisassembled)"
        elif mn == "syscall":
            reason = "syscall (BIOS trampoline)"
        # GTE transfers decode as mtc2/.../swc2; GTE *command* words decode as
        # the generic coprocessor mnemonic "c2  0x......" (e.g. 4aa00428 = c2).
        elif _GTE.match(mn) or mn in ("c2", "cop2"):
            reason = f"GTE/cop2 op ({mn})"
        # Trapping-overflow arithmetic: GCC 2.7.2 emits only the unsigned
        # forms (addu/addiu/subu) from C. Verified empirically 2026-08-05:
        # every function in build/bb2.elf containing add/addi/sub is either
        # canonical-listed hand-asm (70) or unresolved asm debt (3) — zero
        # compiled-C functions fire this signal.
        elif mn in ("add", "addi", "sub"):
            reason = f"trapping-overflow op ({mn}) — no GCC 2.7.2 C form"
        if reason:
            hits.append((idx, mn, reason))
        idx += 1
    return hits, idx, structural


def definitive_insns(func: str):
    """(hits, total_insns) for one function from a range disasm. None if the
    function isn't in build/bb2.elf (rebuild first)."""
    tbl = _func_table()
    if func not in tbl:
        return None
    vram, size = tbl[func]
    cmd = [cfg.OBJDUMP, "-d", f"--start-address=0x{vram:08x}",
           f"--stop-address=0x{vram + size:08x}", "build/bb2.elf"]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    return _detect(out.splitlines())


def _regions(indices: list[int]) -> list[tuple[int, int]]:
    """Collapse instruction indices into contiguous [start,end] spans."""
    if not indices:
        return []
    spans, s, p = [], indices[0], indices[0]
    for i in indices[1:]:
        if i == p + 1:
            p = i
        else:
            spans.append((s, p))
            s = p = i
    spans.append((s, p))
    return spans


def _verdict(func: str, hits: list, total: int, structural: int = 0,
             distance: int | None = None) -> dict:
    """Build the verdict dict from a detected (hits, total[, structural]).
    Definitive opcode signals win outright; otherwise the optional pure-C
    `distance` applies the structural tier."""
    if hits:
        spans = _regions([i for i, _, _ in hits])
        frac = len(hits) / total if total else 0
        nonstruct = total - structural
        # ASM-WHOLE if the function is dense in canonical ops (>=0.8) OR every
        # NON-structural instruction is canonical — a pure GTE/asm leaf wrapper
        # (mtc2/avsz3/mfc2 + nop/jr) whose only non-canonical insns are padding.
        whole = frac >= 0.8 or (nonstruct > 0 and len(hits) == nonstruct)
        verdict = "ASM-WHOLE" if whole else "ASM-PARTIAL"
        reasons = sorted({r for _, _, r in hits})
        return {"func": func, "verdict": verdict, "asm_insns": len(hits), "total": total,
                "regions": spans, "reasons": reasons,
                "reason": f"{len(hits)}/{total} insns canonical-asm ({'; '.join(reasons)})"}
    out = {"func": func, "verdict": "C", "asm_insns": 0, "total": total}
    if distance is None:
        out["reason"] = "no definitive asm signal (structural distance not checked)"
        return out
    out["distance"] = distance
    if distance > SUSPECT_DISTANCE:
        # Distance alone is not strong evidence — large pure-C functions
        # accumulate enough RA/scheduling drift to exceed the threshold without
        # being hand-written. Require corroboration from scan_hand_coded, and
        # RECORD the tier on the verdict either way: it is the evidence the
        # routing rests on, and downstream consumers (engine/queue.py) carry it
        # into the worklist so a reader can see WHY an item was labelled.
        tier = _hand_coded_tier(func)
        out["hand_coded_tier"] = tier
        if distance > NEAR_CERTAIN_DISTANCE and tier in ("STRONG", "POSSIBLE"):
            out["verdict"] = "ASM-STRUCTURAL"
            out["reason"] = (f"pure-C distance {distance} > {NEAR_CERTAIN_DISTANCE} "
                             f"AND scan_hand_coded tier={tier} — corroborated "
                             f"hand-asm (auto-escalate, no pure-C grind)")
        elif tier == "LOW":
            # Affirmative counter-evidence: the scanner ran and found NO
            # hand-coded indicator (0-2/8, none of the decisive S1/S2/S6).
            # A high distance then measures function SIZE / un-attempted body,
            # not asm-ness — the 2026-08-18 triage of all 47 ASM-SUSPECT queue
            # items found 0 hand-coded candidates, most with no C body written
            # at all (distance == full instruction count). Route as an ordinary
            # pure-C target rather than leaving a suspicion label that reads as
            # evidence. UNAVAILABLE/TIGHT_C are NOT this case — unknown is not
            # counter-evidence, so those keep the conservative SUSPECT label.
            out["reason"] = (f"pure-C distance {distance} > {SUSPECT_DISTANCE} but "
                             f"scan_hand_coded tier=LOW (no S1/S2/S6 signal) — "
                             f"distance is size, not hand-asm evidence. Ordinary "
                             f"pure-C target. See canonical-gate-distance-not-evidence.md.")
        else:
            out["verdict"] = "ASM-SUSPECT"
            out["reason"] = (f"pure-C distance {distance} > {SUSPECT_DISTANCE} and "
                             f"scan_hand_coded tier={tier} — structural-asm suspect; "
                             f"distance alone is NOT hand-asm evidence, keep grinding "
                             f"pure C. See canonical-gate-distance-not-evidence.md.")
    else:
        out["reason"] = f"pure-C distance {distance} <= {SUSPECT_DISTANCE} — pure-C target"
    return out


_hand_coded_cache: dict[str, str] = {}
_hand_coded_bulk: dict[str, str] | None = None


def _hand_coded_bulk_tiers() -> dict[str, str] | None:
    """{func: tier} for every NON-LOW function, from ONE whole-tree scan.

    `scan_hand_coded.py --single` re-runs the full asm/funcs scan internally
    (it needs cluster context), so it costs ~5s PER CALL. `queue regen` now
    consults the tier for every item above SUSPECT_DISTANCE (~160 items), which
    would be ~13 minutes of the same scan repeated. `--all --json` emits exactly
    the STRONG/POSSIBLE/TIGHT_C candidates in one pass, so anything with an asm
    file and no entry here is LOW by construction (those are the only four
    tiers). Returns None if the scan is unavailable — callers fall back to the
    per-function path."""
    global _hand_coded_bulk
    if _hand_coded_bulk is None:
        import json
        import os
        try:
            env = {**os.environ, "PYTHONIOENCODING": "utf-8"}
            r = subprocess.run(
                ["python3", "tools/scan_hand_coded.py", "--all", "--json"],
                capture_output=True, text=True, env=env, timeout=600,
            )
            if r.returncode != 0 or not r.stdout.strip():
                return None
            _hand_coded_bulk = {e["func"]: e["tier"] for e in json.loads(r.stdout)}
        except (subprocess.TimeoutExpired, json.JSONDecodeError, OSError, KeyError,
                TypeError):
            return None
    return _hand_coded_bulk


def _hand_coded_tier(func: str) -> str:
    """Return scan_hand_coded's tier for `func` (STRONG/POSSIBLE/TIGHT_C/LOW),
    or 'UNAVAILABLE' if the asm file is missing or the scan errors. Cached
    per-process so the corroboration check is cheap on repeat. Subprocess
    invocation (tools/scan_hand_coded.py) — the canonical gate isn't hot.
    """
    if func in _hand_coded_cache:
        return _hand_coded_cache[func]
    bulk = _hand_coded_bulk_tiers()
    if bulk is not None and Path(f"asm/funcs/{func}.s").exists():
        # Present in the candidate dump -> its tier; absent WITH an asm file ->
        # LOW (the dump carries every non-LOW function). A missing asm file is
        # NOT LOW — it falls through to the single-function path, which reports
        # UNAVAILABLE.
        tier = bulk.get(func, "LOW")
        _hand_coded_cache[func] = tier
        return tier
    import json
    import os
    try:
        env = {**os.environ, "PYTHONIOENCODING": "utf-8"}
        r = subprocess.run(
            ["python3", "tools/scan_hand_coded.py", "--single", func, "--json"],
            capture_output=True, text=True, env=env, timeout=30,
        )
        if r.returncode != 0 or not r.stdout.strip():
            tier = "UNAVAILABLE"
        else:
            tier = json.loads(r.stdout).get("tier", "UNAVAILABLE")
    except (subprocess.TimeoutExpired, json.JSONDecodeError, OSError):
        tier = "UNAVAILABLE"
    _hand_coded_cache[func] = tier
    return tier


def classify(func: str, distance: int | None = None) -> dict:
    """C-vs-asm verdict for one function. distance=None skips the structural
    tier (fast opcode-only)."""
    res = definitive_insns(func)
    if res is None:
        return {"func": func, "verdict": "NO-TARGET",
                "reason": f"{func} not in build/bb2.elf symbol table (rebuild?)"}
    return _verdict(func, res[0], res[1], distance=distance, structural=res[2])


def classify_full(func: str) -> dict:
    """Complete verdict: opcode classification PLUS the structural distance tier.
    Computes the masked cheat-invisible distance (one cheat-disabled build of the
    function's file) when there's no definitive opcode signal."""
    quick = classify(func, distance=None)
    if quick["verdict"] != "C":
        return quick  # definitive opcode or no-target — distance irrelevant
    from . import sandbox  # local import: keeps the opcode path build-free
    try:
        # true pure-C distance: regfix/asmfix off AND cheat-asm stripped
        r = sandbox.sandbox_score(func, disable="all", strip_cheat_asm=True)
    except (KeyError, FileNotFoundError, RuntimeError) as e:
        quick["reason"] += f" (distance unavailable: {e})"
        return quick
    if r.get("score") is None:  # sandbox returned a clean unscorable result
        quick["reason"] += f" (distance unavailable: {r.get('error', 'unscorable')})"
        return quick
    return classify(func, distance=r["score"])


def scan_all() -> list[dict]:
    """Opcode-only classification of every real function, from ONE full disasm of
    build/bb2.elf. Bounds each function by its symbol-table size (NOT the next
    objdump header) so internal `.L` labels can't fragment a function."""
    import bisect
    tbl = _func_table()
    out = subprocess.run([cfg.OBJDUMP, "-d", "build/bb2.elf"],
                         capture_output=True, text=True).stdout
    insns = []  # (addr, line) for every disassembled instruction
    for line in out.splitlines():
        if _DLINE.match(line):
            insns.append((int(line.split(":", 1)[0], 16), line))
    insns.sort()
    addrs = [a for a, _ in insns]
    results = []
    for name, (vram, size) in tbl.items():
        lo = bisect.bisect_left(addrs, vram)
        hi = bisect.bisect_left(addrs, vram + size)
        results.append(_verdict(name, *_detect(line for _, line in insns[lo:hi])))
    return sorted(results, key=lambda r: r["func"])
