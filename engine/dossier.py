"""Per-function dossier: the FULL, live-verified picture of one queue item,
assembled mechanically (owner directive 2026-08-24; formalizes the
context-first-before-grinding directive of 2026-08-20).

Why this exists: a function's truth lives across ~10 surfaces — queue item,
src representation, rule files, grind ledger, decisions/journal/borderline,
canonical list, wip leftovers, name aliases — and hand-gathering them misses
different pieces every time (measured failure classes: parked-but-proven
2026-08-22; invisible directives, stale-HEAD ledger claims, alias-trapped
rulings, records-vs-tree contradictions — all 2026-08-24).

Surface entry points:
  dossier(func)  -> printable full picture
  audit(func)    -> list of consistency WARN strings (empty = coherent)
  audit_all()    -> {func: [warns]} across every queue item + ledger + rule key
  aliases(func)  -> all known names for the function (current + historical)

Read-only; every helper degrades to empty on failure rather than raising.
"""
from __future__ import annotations

import json
import os
import re
from pathlib import Path

QUEUE = "engine/queue.json"
BORDERLINE = "docs/grind/borderline.md"
DECISIONS = "docs/grind/decisions.md"
JOURNAL = "docs/grind/journal.md"
RULE_FILES = ("regfix.txt", "regfix_stage2.txt", "asmfix.txt")


def _read(p):
    try:
        return Path(p).read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def _queue_items():
    try:
        return json.loads(_read(QUEUE)).get("items", [])
    except ValueError:
        return []


def aliases(func: str) -> list[str]:
    """All known names: current + historical, from the borderline alias table
    (lines shaped `a = b`, possibly with parenthetical vram notes)."""
    names = {func}
    for m in re.finditer(r"(\w[\w$]*)\s*(?:\([^)]*\))?\s*=\s*(\w[\w$]*)",
                         _read(BORDERLINE)):
        a, b = m.group(1), m.group(2)
        if func in (a, b):
            names.update((a, b))
    return sorted(names)


def _src_representation(func: str, stem: str | None) -> tuple[str, str]:
    """(kind, where): kind in {include_asm, c_body, absent}. Strict code-line
    matching — comment text mentioning INCLUDE_ASM must not count (the
    func_800481E8 false-escape lesson, 2026-08-24)."""
    inc = re.compile(r'^INCLUDE_ASM\("asm/funcs", ' + re.escape(func) + r"\);", re.M)
    body = re.compile(r"^[A-Za-z_][\w \t\*]*\b" + re.escape(func) + r"\s*\(", re.M)
    paths = [f"src/{stem}.c"] if stem else sorted(str(p) for p in Path("src").glob("*.c"))
    for p in paths:
        t = _read(p)
        if inc.search(t):
            return ("include_asm", p)
        if body.search(t):
            return ("c_body", p)
    if stem:  # fall back to whole-src scan when the hinted stem missed
        return _src_representation(func, None)
    return ("absent", "")


def _rules_keyed(names: list[str]) -> dict[str, int]:
    out = {}
    pats = [re.compile(r"^" + re.escape(n) + r"\s*:") for n in names]
    for rf in RULE_FILES:
        n = sum(1 for ln in _read(rf).split("\n")
                if not ln.lstrip().startswith("#") and any(p.match(ln.strip()) for p in pats))
        if n:
            out[rf] = n
    return out


def _canonical_listed(names: list[str]) -> bool:
    t = _read("inline_asm_canonical.txt")
    return any(re.search(r"^" + re.escape(n) + r"\b", t, re.M) for n in names)


def _memberships(names: list[str]) -> list[str]:
    out = []
    for f in ("maspsx_prefill_label_funcs.txt",
              "expand_lb_funcs.txt", "expand_dest_funcs.txt", "multu_funcs.txt",
              "multu_pad_funcs.txt"):
        t = _read(f)
        if any(re.search(r"^" + re.escape(n) + r"\s*$", t, re.M) for n in names):
            out.append(f)
    try:
        cfg = json.loads(_read("tools/prologue_config.json") or "{}")
        if any(n in cfg for n in names):
            out.append("tools/prologue_config.json")
    except ValueError:
        pass
    return out


def _ledger(func: str) -> dict:
    d = Path("memory/grind") / func
    if not d.is_dir():
        return {}
    st = {}
    try:
        st = json.loads(_read(d / "state.json") or "{}")
    except ValueError:
        pass
    cand = _read(d / "candidate.c")
    return {
        "dir": str(d),
        "sessions": st.get("session_count"),
        "modality": st.get("current_modality"),
        "floors": [e.get("floor") for e in (st.get("floor_history") or [])][-4:],
        "frontier": len(st.get("frontier") or []),
        "banned": len(st.get("banned_constructs") or []),
        "constraints": len(st.get("judge_constraints") or []),
        "candidate": bool(cand),
        "candidate_bannered": "MIGRATION NOTE" in cand,
        "candidate_head_claims": bool(re.search(
            r"\b(HEAD body|at HEAD|on main|stays on main|applied to src)\b", cand)),
        "migration_pin": (d / "migration_pin.json").is_file(),
        "rejected": len(list((d / "rejected").glob("*"))) if (d / "rejected").is_dir() else 0,
    }


def _record_headings(names: list[str], path: str, limit: int = 12) -> list[str]:
    pats = [re.compile(r"\b" + re.escape(n) + r"\b") for n in names]
    out = []
    for i, ln in enumerate(_read(path).split("\n"), 1):
        if ln.startswith("## ") and any(p.search(ln) for p in pats):
            out.append(f"{path}:{i}: {ln[3:][:140]}")
            if len(out) >= limit:
                break
    return out


def _deferred() -> dict:
    try:
        return json.loads(_read("engine/deferred.json")).get("members", {})
    except ValueError:
        return {}


def audit(func: str, item: dict | None = None) -> list[str]:
    """Cross-surface consistency warnings. Empty list = coherent picture.
    Informational lines are prefixed 'INFO:' — audit_all counts only real
    warnings toward its exit status."""
    warns = []
    deferred = _deferred()
    items = _queue_items()
    if item is None:
        item = next((i for i in items if i["func"] == func), None)
    names = aliases(func)
    stem = item.get("file") if item else None
    kind, where = _src_representation(func, stem)
    rules = _rules_keyed(names)
    canon = _canonical_listed(names)
    led = _ledger(func)

    if item is None:
        if kind == "include_asm" and not canon:
            warns.append(f"QUEUE ESCAPE: {func} is INCLUDE_ASM at {where} but neither "
                         "queued nor canonical-listed (the ang_hosei class).")
        if rules:
            warns.append(f"ORPHAN RULES: {func} not in queue but keys {rules}.")
    else:
        if kind == "include_asm" and rules:
            warns.append(f"RULES ON INCLUDE_ASM: {func} is asm-supplied yet keys {rules} "
                         "(rules should have retired with the migration).")
        if kind == "c_body" and not canon:
            if func in deferred:
                warns.append(f"INFO: DEFERRED (proven 2026-08-24): {deferred[func][:110]}")
            elif not rules and item.get("rules", 0) == 0:
                warns.append(f"UNMIGRATED CLEAN BODY: {func} carries a C body with zero "
                             "rules while INCOMPLETE, and is NOT in the proven deferred set "
                             "(engine/deferred.json) — a missed migration or completion; "
                             "measure it.")
        if kind == "absent":
            warns.append(f"NO REPRESENTATION: {func} queued but neither INCLUDE_ASM nor a "
                         f"C body found in src/ (checked stem {stem!r}).")
        if item.get("unpark_reason") and led and not led.get("candidate_bannered"):
            ev = _read(Path("memory/grind") / func / "evidence.md")
            tag = item["unpark_reason"][:40]
            if tag not in ev:
                warns.append(f"INFO: DIRECTIVE NOT YET IN LEDGER: {func} carries an owner "
                             "directive no session has acknowledged/executed yet.")
    if canon and kind == "c_body":
        warns.append(f"CANONICAL-VS-BODY: {func} is canonical-listed but src has a C body.")
    if led and led.get("candidate_head_claims") and not led.get("candidate_bannered") \
            and kind == "include_asm":
        warns.append(f"STALE HEAD CLAIMS: {func} candidate.c asserts HEAD/main state without "
                     "a migration banner while the representation is INCLUDE_ASM.")
    if (Path("memory/wip") / func).is_dir() and led:
        warns.append(f"WIP LEFTOVER: memory/wip/{func}/ exists ALONGSIDE a grind ledger "
                     "(grind supersedes wip; decomp-loop mandates deletion).")
    if len(names) > 1 and not _record_headings([func], DECISIONS, 1) \
            and _record_headings([n for n in names if n != func], DECISIONS, 1):
        warns.append(f"INFO: ALIAS-TRAPPED RECORDS: decisions.md rulings for {func} exist only "
                     f"under {[n for n in names if n != func]} — grep those names too.")
    return warns


def dossier(func: str) -> str:
    items = _queue_items()
    item = next((i for i in items if i["func"] == func), None)
    names = aliases(func)
    stem = item.get("file") if item else None
    kind, where = _src_representation(func, stem)
    led = _ledger(func)
    lines = [f"=== DOSSIER {func} ===",
             f"aliases: {', '.join(names)}",
             f"queue: {json.dumps(item) if item else 'NOT IN QUEUE'}",
             f"src: {kind} @ {where or '-'}",
             f"rules keyed: {_rules_keyed(names) or 'none'}",
             f"canonical-listed: {_canonical_listed(names)}",
             f"gate/config memberships: {_memberships(names) or 'none'}",
             f"ledger: {json.dumps(led) if led else 'none'}"]
    heads = _record_headings(names, DECISIONS) + _record_headings(names, BORDERLINE)
    lines.append("record trail (headings):")
    lines += [f"  {h}" for h in heads] or ["  (none)"]
    jl = [ln for ln in _read(JOURNAL).split("\n")
          if any(n in ln for n in names)][-3:]
    lines.append("journal tail:")
    lines += [f"  {l[:160]}" for l in jl] or ["  (none)"]
    # Data model (2026-09-03, func_80033550 post-mortem): declared shape vs
    # census / sibling-addressing evidence for every global the target touches.
    try:
        from . import datamodel as _dm
        dm = _dm.render(func)
    except Exception:
        dm = ""
    if dm:
        lines.append(dm)
    warns = audit(func, item)
    lines.append("CONSISTENCY: " + ("OK" if not warns else ""))
    lines += [f"  WARN: {w}" for w in warns]
    return "\n".join(lines)


def audit_all() -> dict[str, list[str]]:
    """Standing drift detector: audit every queue item, every grind ledger dir,
    and every rule-file key. Returns only functions with warnings."""
    out = {}
    seen = set()
    for it in _queue_items():
        seen.add(it["func"])
        w = audit(it["func"], it)
        if w:
            out[it["func"]] = w
    for d in sorted(p.name for p in Path("memory/grind").iterdir() if p.is_dir()):
        if d not in seen:
            seen.add(d)
            w = audit(d, None)
            if w:
                out[d] = w
    keyed = set()
    for rf in RULE_FILES:
        for ln in _read(rf).split("\n"):
            m = re.match(r"^(\w[\w$]*)\s*:", ln.strip())
            if m and not ln.lstrip().startswith("#"):
                keyed.add(m.group(1))
    for f in sorted(keyed - seen):
        w = audit(f, None)
        if w:
            out[f] = w
    return out


def audit_all_severity(res: dict[str, list[str]] | None = None) -> tuple[int, int]:
    """(real_warnings, info_lines) for an audit_all result."""
    res = audit_all() if res is None else res
    real = sum(1 for ws in res.values() for w in ws if not w.startswith("INFO:"))
    info = sum(1 for ws in res.values() for w in ws if w.startswith("INFO:"))
    return real, info
