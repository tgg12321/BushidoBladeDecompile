#!/usr/bin/env python3
"""Generate WORK_QUEUE.md at the project root — the canonical, ordered
list of remaining functions for future agents to pull from.

Reads tmp/batch_attempt.csv, live-scans asmfix.txt for `replace_with_asmfile`
entries, scans already-committed C bodies for suspect inline-asm debt, filters
to in-scope categories, and writes a Markdown queue grouped by complexity tier.

Tier ordering (top of queue = pull these first) — effort-based, mixing
recommendation + instruction count + aliasing_heavy into a single score:

  1. Trivial / Easy   (effort < 1.6)
  2. Moderate         (effort < 3.5)
  3. Hard             (effort < 7.0)
  4. Slog             (effort >= 7.0)

Effort = base(rec) * size_factor(insns) * (1.8 if aliasing_heavy else 1).
Base: standard 1.0, delay_slot_ra 1.2, gte 1.3, lwl 1.4, rodata_split
1.6, function_split 2.0. Size factor: <=40 -> 0.5, <=100 -> 1.0,
<=200 -> 1.8, <=400 -> 3.0, <=700 -> 4.5, <=1200 -> 6.5, else 9.0.

Within each tier, entries are sorted by effort score asc, then size,
then src file (siblings cluster), then name. The result is a smooth
difficulty ramp: a 4-insn gte helper outranks a 753-insn standard
function, which outranks a 1112-insn aliasing_heavy beast.

Out-of-scope categories (permanently_blocked, bios_or_syscall,
not_code_symbol, not_found) are NOT in the queue. Structural split work
(needs_function_split) is listed separately because it may become C after
function-boundary work, but it is not normal one-function queue work yet.
Recognized library shortcuts (psyq_stdlib_*) are also listed separately if
present; they are mechanical C/library replacements, not permanent blockers.

Functions with `kind=asmfix` (already binary-matched via `replace_with_asmfile`
in asmfix.txt) are excluded from the default active queue but listed in their
own retirement queue. Their bytes come from `asm/funcs/<func>.s` regardless
of the C body, so re-decompiling them in pure C is a separate "remove the
asmfix entry" project.

Functions with `kind=inline_asm_debt` are C-shaped functions whose remaining
inline asm is not a canonical permanent-asm exception. They stay in the active
queue until the suspect asm is removed or narrowed to accepted GTE/BIOS/data
usage.
"""
from __future__ import annotations

import csv
import re
from collections import defaultdict
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / "tmp" / "batch_attempt.csv"
OUT = ROOT / "WORK_QUEUE.md"
SRC_DIR = ROOT / "src"
ASMFIX = ROOT / "asmfix.txt"
ASM_FUNCS = ROOT / "asm" / "funcs"

IN_SCOPE = {
    "easy_attempt",
    "standard",
    "gte_function",
    "needs_rodata_split",
    "needs_lwl_fix",
    "needs_delay_slot_ra",
}
DEFERRED_STRUCTURAL = {"needs_function_split"}

# Same policy as tools/audit_asm_cheats.py: GTE/BIOS/data asm may be
# legitimate, but ordinary MIPS scheduling/register scaffolding inside a C
# body is still decompilation debt and must remain visible in WORK_QUEUE.md.
#
# GTE encoded as `.word 0xXX......`: coprocessor-2 instructions
# (op=0b010010 → top byte 0x48-0x4B for COP2 family, op=0b110010 →
# 0xC8-0xCB for lwc2, op=0b111010 → 0xE8-0xEB for swc2). These are
# the canonical form when maspsx can't parse the symbolic GTE
# mnemonic (most BB2 GTE wrappers use this).
ACCEPTABLE_INLINE_ASM_OPS = re.compile(
    r"(\b("
    r"mfc2|cfc2|mtc2|ctc2|lwc2|swc2|"
    r"syscall|break|"
    r"move|"  # pseudo-mnemonic for `addu rd, rs, $zero` placement-control move
    r"\.incbin|\.section\s+\.data|\.pushsection|\.popsection"
    r")\b)"
    r"|"
    # GTE/lwc2/swc2 encoded as `.word 0x{48-4B,C8-CB,E8-EB}........`
    r"\.word\s+0x(4[89A-Ba-b]|[Cc][89A-Ba-b]|[Ee][89A-Ba-b])[0-9A-Fa-f]{6}\b"
)
SUSPECT_INLINE_ASM_OPS = re.compile(
    r"\b("
    r"multu|mult|mflo|mfhi|"
    r"addu|subu|sll|sra|srl|"
    r"lh|lhu|lb|lbu|lw|sh|sb|sw|"
    r"bgez|blez|bne|beq|bltz|bgtz|j\s|jal\s|jr"
    r")\b"
)

# Functions where file-scope `__asm__()` retirement is canonical per
# project policy (hand-written-asm with non-C-expressible patterns).
# Loaded once at import; checked by the inline-asm-debt scanner.
INLINE_ASM_CANONICAL_FILE = ROOT / "inline_asm_canonical.txt"


def load_canonical_inline_asm() -> set[str]:
    if not INLINE_ASM_CANONICAL_FILE.exists():
        return set()
    names: set[str] = set()
    for line in INLINE_ASM_CANONICAL_FILE.read_text(
        encoding="utf-8", errors="ignore"
    ).splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        # Strip trailing `# comment`
        name = line.split("#", 1)[0].strip()
        if name:
            names.add(name)
    return names


_CANONICAL_INLINE_ASM_NAMES = load_canonical_inline_asm()
INLINE_ASM_RE = re.compile(r"\b__asm__\s*(?:volatile)?\s*\(")
FUNC_DEF_RE = re.compile(
    r"(?m)^[A-Za-z_][\w *]*?\s\**(\w+)\s*\([^)]*\)\s*\{",
)
ASMFIX_RE = re.compile(r"^(\w+):\s+replace_with_asmfile\b", re.MULTILINE)


def classify_info(func: str) -> dict:
    tools_path = str(ROOT / "tools")
    if tools_path not in sys.path:
        sys.path.insert(0, tools_path)
    from classify_func import classify

    return classify(func)


def find_src_for_func(func: str) -> str:
    pat = re.compile(rf"\b{re.escape(func)}\b")
    for src in sorted(SRC_DIR.glob("*.c")):
        try:
            text = src.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        if pat.search(text):
            return str(src.relative_to(ROOT)).replace("\\", "/")
    return ""


def asm_insn_count(func: str) -> int:
    asm = ASM_FUNCS / f"{func}.s"
    if not asm.exists():
        return 0
    return sum(
        1
        for line in asm.read_text(encoding="utf-8", errors="ignore").splitlines()
        if "/* " in line and " */" in line
    )


def find_function_bounds(text: str, name_match) -> tuple[int, int] | None:
    if name_match.end() == 0 or text[name_match.end() - 1] != "{":
        return None
    body_start = name_match.end()
    depth = 1
    i = body_start
    while i < len(text) and depth > 0:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    if depth != 0:
        return None
    return body_start, i - 1


def iter_asm_blocks(body: str):
    for asm_match in INLINE_ASM_RE.finditer(body):
        depth = 0
        i = asm_match.end() - 1
        while i < len(body):
            if body[i] == "(":
                depth += 1
            elif body[i] == ")":
                depth -= 1
                if depth == 0:
                    yield body[asm_match.end():i]
                    break
            i += 1


def _block_insn_count(block: str) -> int:
    """Count distinct asm instructions in a single __asm__ template block.

    Sum of SUSPECT + ACCEPTABLE mnemonic occurrences. Single-instruction
    blocks are codegen hints (allowed per skill §6.1); multi-instruction
    blocks smuggle function work in concatenated form.

    The block is the C-string content of `__asm__ ( "<here>" )` — strings
    typically contain a single mnemonic or `\\n`-separated instructions."""
    return (len(SUSPECT_INLINE_ASM_OPS.findall(block))
            + len(ACCEPTABLE_INLINE_ASM_OPS.findall(block)))


def is_inline_asm_debt(body: str) -> bool:
    asm_blocks = list(iter_asm_blocks(body))
    if not asm_blocks:
        return False
    asm_chars = sum(len(block) for block in asm_blocks)
    asm_ratio = asm_chars / len(body) if body else 0
    suspect_blocks = [b for b in asm_blocks if SUSPECT_INLINE_ASM_OPS.search(b)]
    if not suspect_blocks:
        return False
    has_acceptable = any(ACCEPTABLE_INLINE_ASM_OPS.search(b) for b in asm_blocks)
    # Single-instruction suspect blocks are §6.1-allowed codegen hints
    # (e.g. `__asm__ ("sll %0,%1,16" : "=r"(t) : "r"(x))` for (s16)x cast).
    # Only flag when at least one suspect block carries multiple
    # instructions — that's the smuggle-work-via-concatenation pattern.
    multi_insn_suspect = any(_block_insn_count(b) > 1 for b in suspect_blocks)
    if not multi_insn_suspect:
        # ALL suspect blocks are single-instruction. Still flag if asm
        # dominates the function body (cumulative many-narrow-blocks
        # smuggle = same effect as a file-scope cheat).
        return asm_ratio > 0.5
    return not has_acceptable or asm_ratio > 0.5


def classify_inline_asm_debt(func: str, src: Path) -> dict:
    info = classify_info(func)
    rec = info.get("recommendation", "standard")
    tags = list(info.get("blocker_tags", []))
    if "inline_asm_debt" not in tags:
        tags.append("inline_asm_debt")
    return {
        "func": func,
        "src": str(src.relative_to(ROOT)).replace("\\", "/"),
        "kind": "inline_asm_debt",
        "result": "CLASSIFY",
        "score": "",
        "stage": "",
        "reason": "",
        "elapsed": "",
        "attempts": "",
        "recommendation": rec,
        "size_insns": str(info.get("size", {}).get("insns", 0)),
        "blocker_tags": ",".join(tags),
    }


def scan_inline_asm_debt() -> list[dict]:
    debt: list[dict] = []
    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        for func_match in FUNC_DEF_RE.finditer(text):
            name = func_match.group(1)
            if name in {"if", "while", "for", "switch", "do"}:
                continue
            # Canonical inline-asm retirement: original was hand-written
            # assembly with non-C-expressible patterns. The function is
            # accepted in its file-scope `__asm__()` form per project
            # policy (see inline_asm_canonical.txt).
            if name in _CANONICAL_INLINE_ASM_NAMES:
                continue
            bounds = find_function_bounds(text, func_match)
            if not bounds:
                continue
            body_start, body_end = bounds
            if is_inline_asm_debt(text[body_start:body_end]):
                debt.append(classify_inline_asm_debt(name, src))
    return debt


def scan_regfix_overwrite_debt() -> list[dict]:
    """Surface functions with wildcard `subst ".*"` rule clusters in regfix.txt
    or regfix_stage2.txt — these force-overwrite the C-source's compiled
    output line-by-line with the rule's literal text. Functionally equivalent
    to replace_with_asmfile (binary content from rules, not C codegen).

    Tag: `regfix_overwrite_debt`. Fix path: remove the wildcard substs from
    regfix*.txt and write real C that produces the target bytes naturally
    (or authorize via inline_asm_canonical.txt if proven canonical asm)."""
    tools_path = str(ROOT / "tools")
    if tools_path not in sys.path:
        sys.path.insert(0, tools_path)
    from audit_asm_cheats import scan_regfix_wildcard_substs, WILDCARD_SUBST_LIMIT

    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    if not regfix_content:
        return []

    debt: list[dict] = []
    counts = scan_regfix_wildcard_substs(regfix_content)
    for fname, wcount in counts.items():
        if wcount < WILDCARD_SUBST_LIMIT:
            continue
        if fname in _CANONICAL_INLINE_ASM_NAMES:
            continue
        info = classify_info(fname)
        rec = info.get("recommendation", "standard")
        tags = list(info.get("blocker_tags", []))
        if "regfix_overwrite_debt" not in tags:
            tags.append("regfix_overwrite_debt")
        tags.append(f"wildcard_substs={wcount}")
        if rec.startswith("permanently_blocked:") or rec.startswith("bios_or_syscall:"):
            tags.append(f"classifier_said:{rec}")
            rec = "standard"
        src_path = find_src_for_func(fname)
        debt.append({
            "func": fname,
            "src": src_path,
            "kind": "regfix_overwrite_debt",
            "result": "CLASSIFY",
            "score": "",
            "stage": "",
            "reason": "",
            "elapsed": "",
            "attempts": "",
            "recommendation": rec,
            "size_insns": str(info.get("size", {}).get("insns", 0)),
            "blocker_tags": ",".join(tags),
        })
    return debt


def scan_c_body_asm_debt() -> list[dict]:
    """Surface multi-instruction __asm__ blocks inside C function bodies that
    smuggle non-§6.1-whitelisted instructions (lw/sw/addu/etc.) via
    \\n-concatenation. These were the previous-agent shortcut that bypassed
    file-scope+glabel detection — the asm is inside a C function body but
    still violates §6.1's "single instruction per asm block" rule.

    Tag: `c_body_asm_debt`. Refactor path: split the multi-insn block into
    per-instruction __asm__ blocks, or move the non-GTE/non-scheduling work
    into pure C. Reference template: src/text1b.c GTE wrappers
    (game_2d_CheckLifeGaugeNoDisp et al.)."""
    tools_path = str(ROOT / "tools")
    if tools_path not in sys.path:
        sys.path.insert(0, tools_path)
    from audit_asm_cheats import scan_c_body_smuggled_work

    debt: list[dict] = []
    seen: set[str] = set()
    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        for _f, _l, _n, fname, _insns in scan_c_body_smuggled_work(text, src.name):
            if not fname:
                continue
            if fname in _CANONICAL_INLINE_ASM_NAMES:
                continue
            if fname in seen:
                continue
            seen.add(fname)
            info = classify_info(fname)
            rec = info.get("recommendation", "standard")
            tags = list(info.get("blocker_tags", []))
            if "c_body_asm_debt" not in tags:
                tags.append("c_body_asm_debt")
            # Same override as scan_file_scope_asm_debt: classifier may have
            # said permanently_blocked because it read the smuggled asm; we
            # force into active queue for user audit.
            if rec.startswith("permanently_blocked:") or rec.startswith("bios_or_syscall:"):
                tags.append(f"classifier_said:{rec}")
                rec = "standard"
            debt.append({
                "func": fname,
                "src": str(src.relative_to(ROOT)).replace("\\", "/"),
                "kind": "c_body_asm_debt",
                "result": "CLASSIFY",
                "score": "",
                "stage": "",
                "reason": "",
                "elapsed": "",
                "attempts": "",
                "recommendation": rec,
                "size_insns": str(info.get("size", {}).get("insns", 0)),
                "blocker_tags": ",".join(tags),
            })
    return debt


def scan_file_scope_asm_debt() -> list[dict]:
    """Surface file-scope `__asm__(... glabel funcname ...)` cheats as
    inline_asm_debt work items.

    The scan_inline_asm_debt() function above only finds inline asm inside
    C-shaped function bodies. Agents have historically also dumped verbatim
    asm at FILE SCOPE (no C wrapper, `glabel` inside the asm block defining
    the symbol). The audit_asm_cheats.py auditor catches those; this scanner
    surfaces them as queue work so they actually get decompiled.

    Note: classify_func reads asm/funcs/<func>.s and may return
    `permanently_blocked:<reason>` because the original asm contains
    cop0/break/overflow ops. For known cheats (not in inline_asm_canonical.txt)
    we override to `standard` so they land in the active queue. The user
    decides per-function at decomp time: authorize as canonical (add to
    inline_asm_canonical.txt with justification) or write real C."""
    tools_path = str(ROOT / "tools")
    if tools_path not in sys.path:
        sys.path.insert(0, tools_path)
    from audit_asm_cheats import scan_inline_asm_bodies, GLABEL_BODY_LINES

    debt: list[dict] = []
    seen: set[str] = set()
    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        for _f, _l, n, fname, is_bios in scan_inline_asm_bodies(text, src.name):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in _CANONICAL_INLINE_ASM_NAMES:
                continue
            if is_bios:
                continue  # 3-insn BIOS trampolines belong in canonical list, not work queue
            if fname in seen:
                continue
            seen.add(fname)
            row = classify_inline_asm_debt(fname, src)
            rec = row.get("recommendation", "")
            if rec.startswith("permanently_blocked:") or rec.startswith("bios_or_syscall:"):
                # Classifier read the cheated asm and concluded canonical, but
                # the function is NOT in inline_asm_canonical.txt. Force into
                # active queue for user audit; tag the original verdict so the
                # decomp session can see what classify_func found.
                row["blocker_tags"] = ",".join(
                    [t for t in row["blocker_tags"].split(",") if t]
                    + [f"classifier_said:{rec}"]
                )
                row["recommendation"] = "standard"
            debt.append(row)
    return debt


def scan_orphan_cheats(existing_funcs: set[str]) -> list[dict]:
    """Functions with audit-flagged cheats but NOT in any other queue.
    These currently SHA1-match (via the cheat); the cleanup work is to
    remove the cheat rules + coerce the C source to produce equivalent
    codegen naturally.

    Excludes funcs already in:
      - active decomp queue (kind != asmfix entries)
      - asmfix retirement queue (kind == asmfix entries)
      - any of the *_debt scans (inline_asm / c_body / regfix_overwrite)
    so we only surface cheats that aren't picked up by an existing queue.

    Sort: ascending by cheat rule count (smaller = easier to retire),
    then by size, then by name.
    """
    try:
        import audit_asm_cheats as audit
    except Exception:
        return []

    auth = audit.load_authorized()
    splice_funcs, inline_funcs, bios_funcs, c_body_funcs, instruction_insert_funcs = (
        audit.get_current_cheats(auth)
    )

    all_cheat_funcs = (
        splice_funcs | inline_funcs | bios_funcs | c_body_funcs
        | set(instruction_insert_funcs.keys())
    )
    orphan_funcs = all_cheat_funcs - existing_funcs

    # Count TOTAL regfix rules per function (cheat + non-cheat). Cheats wedged
    # inside many auxiliary rules are structurally harder to retire than
    # isolated ones — see feedback_cheat_cleanup_techniques.md. Total-rule
    # count is a much better difficulty proxy than cheat-rule count alone.
    import re as _re_total
    total_rules: dict[str, int] = {}
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if not p.exists():
            continue
        for line in p.read_text(encoding="utf-8", errors="ignore").splitlines():
            m = _re_total.match(r'^\s*([A-Za-z_][\w]*)\s*:', line)
            if m:
                total_rules[m.group(1)] = total_rules.get(m.group(1), 0) + 1

    entries: list[dict] = []
    for func in sorted(orphan_funcs):
        size = asm_insn_count(func)
        src = find_src_for_func(func)
        types: list[str] = []
        rule_count = 0
        if func in splice_funcs:
            types.append("splice")
            rule_count += 1
        if func in inline_funcs:
            types.append("inline_asm")
            rule_count += 1
        if func in bios_funcs:
            types.append("bios_inline")
            rule_count += 1
        if func in c_body_funcs:
            types.append("c_body_multi_insn")
            rule_count += 1
        if func in instruction_insert_funcs:
            n = instruction_insert_funcs[func]
            types.append(f"lost_codegen({n})")
            rule_count += n
        entries.append({
            "func": func,
            "size": size,
            "rec": "cheat_cleanup",
            "src": src.replace("src/", "") if src else "",
            "kind": "cheat_cleanup",
            "tags": ",".join(types),
            "rule_count": rule_count,
            "total_rules": total_rules.get(func, 0),
        })

    # Primary sort: total regfix rule count (lower = simpler to retire).
    # Secondary: cheat-rule count, then size, then name. Empirical: success
    # rate ~75% for total_rules <= 9, 0% for total_rules >= 12 in single-
    # session quick retirements.
    entries.sort(key=lambda e: (e["total_rules"], e["rule_count"], e["size"], e["func"]))
    return entries


def classify_asmfix_row(func: str) -> dict:
    info = classify_info(func)
    size = asm_insn_count(func) or int(info.get("size", {}).get("insns", 0) or 0)
    return {
        "func": func,
        "src": find_src_for_func(func),
        "kind": "asmfix",
        "result": "CLASSIFY",
        "score": "",
        "stage": "",
        "reason": "",
        "elapsed": "",
        "attempts": "",
        "recommendation": info.get("recommendation", "standard"),
        "size_insns": str(size),
        "blocker_tags": ",".join(info.get("blocker_tags", []) or []),
    }


def scan_asmfix_rows() -> list[dict]:
    if not ASMFIX.exists():
        return []
    seen: set[str] = set()
    rows: list[dict] = []
    for func in ASMFIX_RE.findall(ASMFIX.read_text(encoding="utf-8", errors="ignore")):
        if func in seen:
            continue
        seen.add(func)
        rows.append(classify_asmfix_row(func))
    return rows


def is_library_shortcut(rec: str) -> bool:
    return rec.startswith("psyq_stdlib_")


def row_entry(r: dict) -> dict:
    return {
        "func": r["func"],
        "size": int(r.get("size_insns", "0") or 0),
        "rec": r["recommendation"],
        "src": r.get("src", "").replace("src/", ""),
        "kind": r.get("kind", ""),
        "tags": r.get("blocker_tags", ""),
    }


_REC_BASE = {
    "standard": 1.0,
    "easy_attempt": 1.0,
    "needs_delay_slot_ra": 1.2,
    "gte_function": 1.3,
    "needs_lwl_fix": 1.4,
    "needs_rodata_split": 1.6,
    "needs_function_split": 2.0,
}


def effort_score(rec: str, size: int, tags: str) -> float:
    """Single difficulty estimate for the queue ordering.

    Combines per-recommendation base effort, instruction count (in
    coarse buckets so siblings cluster), and the aliasing_heavy tag.
    See module docstring for the threshold definitions.
    """
    base = _REC_BASE.get(rec or "", 1.0)
    if "aliasing_heavy" in (tags or ""):
        base *= 1.8
    if size <= 40:
        size_factor = 0.5
    elif size <= 100:
        size_factor = 1.0
    elif size <= 200:
        size_factor = 1.8
    elif size <= 400:
        size_factor = 3.0
    elif size <= 700:
        size_factor = 4.5
    elif size <= 1200:
        size_factor = 6.5
    else:
        size_factor = 9.0
    return base * size_factor


def tier_for(rec: str, size: int, tags: str) -> tuple[int, str]:
    """Return (tier_index, tier_label). Lower tier_index = higher priority.

    Out-of-scope categories map to indices 90+ so they never appear in
    any pullable queue. In-scope work is bucketed by `effort_score()`
    into 4 effort tiers: Trivial/Easy, Moderate, Hard, Slog. This mixes
    recommendation categories so a tiny gte/lwl/rodata-split function
    surfaces above a 700-insn standard one — the queue rewards small
    wins first regardless of category.
    """
    rec_l = rec or ""
    if rec_l.startswith("permanently_blocked:"):
        return (90, rec_l)
    if rec_l.startswith("bios_or_syscall:"):
        return (91, rec_l)
    if rec_l == "not_code_symbol":
        return (92, rec_l)
    if rec_l not in _REC_BASE:
        return (99, f"other ({rec_l})")

    score = effort_score(rec_l, size, tags or "")
    if score < 1.6:
        return (1, "Tier 1 -- Trivial / Easy")
    if score < 3.5:
        return (2, "Tier 2 -- Moderate")
    if score < 7.0:
        return (3, "Tier 3 -- Hard")
    return (4, "Tier 4 -- Slog")


def sort_entries(entries: list[dict]) -> None:
    entries.sort(
        key=lambda x: (
            tier_for(x["rec"], x["size"], x["tags"])[0],
            effort_score(x["rec"], x["size"], x["tags"]),
            x["size"],
            x["src"],
            x["func"],
        )
    )


def append_numbered_queue(
    lines: list[str], entries: list[dict], group_by_tier: bool = False
) -> None:
    """Append a single numbered code block, or one block per tier if
    `group_by_tier` is set — tier headers help readers see where the
    easy/moderate/hard/slog cutoffs are without counting items.
    """
    if not group_by_tier:
        lines.append("```")
        for pos, entry in enumerate(entries, 1):
            tags = f"  [{entry['tags']}]" if entry["tags"] else ""
            lines.append(
                f"{pos:>4}  {entry['func']:<32s}  "
                f"{entry['size']:>4d}  "
                f"{entry['rec']:<30s}  "
                f"{entry['src']:<24s}{tags}"
            )
        lines.append("```")
        lines.append("")
        return

    cur_label: str | None = None
    pos = 0
    for entry in entries:
        _, label = tier_for(entry["rec"], entry["size"], entry["tags"])
        if label != cur_label:
            if cur_label is not None:
                lines.append("```")
                lines.append("")
            lines.append(f"### {label}")
            lines.append("")
            lines.append("```")
            cur_label = label
        pos += 1
        tags = f"  [{entry['tags']}]" if entry["tags"] else ""
        lines.append(
            f"{pos:>4}  {entry['func']:<32s}  "
            f"{entry['size']:>4d}  "
            f"{entry['rec']:<30s}  "
            f"{entry['src']:<24s}{tags}"
        )
    if cur_label is not None:
        lines.append("```")
        lines.append("")


def main() -> int:
    if not CSV_PATH.exists():
        print(
            f"ERROR: {CSV_PATH} missing -- run "
            f"`python3 tools/batch_attempt.py --classify-only` first.",
            file=sys.stderr,
        )
        return 1

    rows = list(csv.DictReader(CSV_PATH.open()))
    live_asmfix_rows = scan_asmfix_rows()
    live_asmfix_funcs = {r["func"] for r in live_asmfix_rows}
    # File-scope `__asm__("glabel ...")` cheats. The CSV classifies these
    # as permanently_blocked because classify_func reads the cheated asm
    # and sees cop0/break/etc — but they're known cheats not on the
    # canonical list. We OVERRIDE the stale CSV row so they land in the
    # active queue, not out-of-scope. asmfix bridging takes precedence
    # if both are present (kind=asmfix → retirement queue).
    file_scope_debt_rows = [
        r for r in scan_file_scope_asm_debt()
        if r["func"] not in live_asmfix_funcs
    ]
    file_scope_debt_funcs = {r["func"] for r in file_scope_debt_rows}
    c_body_debt_rows = [
        r for r in scan_c_body_asm_debt()
        if r["func"] not in live_asmfix_funcs
        and r["func"] not in file_scope_debt_funcs
    ]
    c_body_debt_funcs = {r["func"] for r in c_body_debt_rows}
    regfix_overwrite_debt_rows = [
        r for r in scan_regfix_overwrite_debt()
        if r["func"] not in live_asmfix_funcs
        and r["func"] not in file_scope_debt_funcs
        and r["func"] not in c_body_debt_funcs
    ]
    regfix_overwrite_debt_funcs = {r["func"] for r in regfix_overwrite_debt_rows}
    # asmfix.txt is the source of truth for replacement-backed functions.
    # Drop stale CSV rows for asmfix names AND for any asm-debt variant
    # (their fresh classification supersedes the CSV).
    rows = [
        r for r in rows
        if r["func"] not in live_asmfix_funcs
        and r["func"] not in file_scope_debt_funcs
        and r["func"] not in c_body_debt_funcs
        and r["func"] not in regfix_overwrite_debt_funcs
    ]
    existing_funcs = (
        {r["func"] for r in rows}
        | live_asmfix_funcs
        | file_scope_debt_funcs
        | c_body_debt_funcs
        | regfix_overwrite_debt_funcs
    )
    debt_rows = [
        r for r in scan_inline_asm_debt()
        if r["func"] not in existing_funcs
    ]
    rows.extend(live_asmfix_rows)
    rows.extend(debt_rows)
    rows.extend(file_scope_debt_rows)
    rows.extend(c_body_debt_rows)
    rows.extend(regfix_overwrite_debt_rows)
    # Functions with replace_with_asmfile in asmfix.txt are tagged kind=asmfix.
    # Their bytes come from the .s file regardless of C — they are not in
    # the pure-C work queue. Surface them in their own section instead.
    # Permanently-blocked / bios_or_syscall asmfix entries can never be
    # retired to pure C — they belong in the permanent out-of-scope group,
    # not the retirement queue.
    def _is_permanent_asmfix(r):
        rec = r.get("recommendation", "")
        return rec.startswith("permanently_blocked:") or rec.startswith("bios_or_syscall:")

    asmfix_rows = [r for r in rows if r.get("kind") == "asmfix" and not _is_permanent_asmfix(r)]
    asmfix_permanent_rows = [r for r in rows if r.get("kind") == "asmfix" and _is_permanent_asmfix(r)]
    pure_c_rows = [r for r in rows if r.get("kind") != "asmfix"]
    # Permanent-bridged entries are surfaced under the existing out-of-scope
    # categorization (so they're visible but clearly not work).
    pure_c_rows = pure_c_rows + asmfix_permanent_rows
    in_scope = [r for r in pure_c_rows if r["recommendation"] in IN_SCOPE]
    deferred_rows = [
        r for r in pure_c_rows
        if r["recommendation"] in DEFERRED_STRUCTURAL
    ]
    library_rows = [
        r for r in pure_c_rows
        if is_library_shortcut(r["recommendation"])
    ]
    out_scope = [
        r for r in pure_c_rows
        if r["recommendation"] not in IN_SCOPE
        and r["recommendation"] not in DEFERRED_STRUCTURAL
        and not is_library_shortcut(r["recommendation"])
    ]

    # Group by tier, sort within tier
    tiers: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for r in in_scope:
        size = int(r.get("size_insns", "0") or 0)
        tier = tier_for(r["recommendation"], size, r.get("blocker_tags", ""))
        tiers[tier].append(row_entry(r))

    # Sort within each tier by effort score (so a 4-insn gte beats a
    # 100-insn standard), then size, then src (siblings cluster), then name.
    for tier in tiers:
        tiers[tier].sort(
            key=lambda x: (
                effort_score(x["rec"], x["size"], x["tags"]),
                x["size"],
                x["src"],
                x["func"],
            )
        )

    # Build flat ordered queue
    ordered: list[dict] = []
    for tier in sorted(tiers.keys()):
        for entry in tiers[tier]:
            entry["tier_label"] = tier[1]
            ordered.append(entry)

    deferred_entries = [row_entry(r) for r in deferred_rows]
    sort_entries(deferred_entries)

    asmfix_entries = [row_entry(r) for r in asmfix_rows]
    sort_entries(asmfix_entries)

    # Cheat cleanup queue: audit-flagged cheats not in any other queue.
    # Computed AFTER the active/asmfix/deferred queues so we know what's
    # already scheduled and only surface the leftovers.
    queued_funcs = (
        {e["func"] for e in ordered}
        | {e["func"] for e in deferred_entries}
        | {e["func"] for e in asmfix_entries}
        | {r["func"] for r in library_rows}
    )
    cheat_orphan_entries = scan_orphan_cheats(queued_funcs)

    # Out-of-scope grouped by recommendation, alphabetized
    out_groups: dict[str, list[str]] = defaultdict(list)
    for r in out_scope:
        out_groups[r["recommendation"]].append(r["func"])
    for rec in out_groups:
        out_groups[rec].sort()

    # Write
    lines: list[str] = []
    lines.append("# BB2 Work Queue")
    lines.append("")
    lines.append(
        "**Canonical ordered list of remaining functions.** Pull from the top. "
        "Do not search for easier targets; do not skip ahead. Per "
        "`feedback_workflow_rules.md` THE HARD RULE, every function in this "
        "queue gets worked end-to-end to pure C match."
    )
    lines.append("")
    lines.append(
        f"Generated by `tools/gen_work_queue.py` from `tmp/batch_attempt.csv` "
        f"plus live scans for asmfix replacements and suspect inline-asm "
        f"debt. To refresh: run `bash tools/dc.sh refresh-queue`."
    )
    lines.append("")
    lines.append("## Backlog Summary")
    lines.append("")
    lines.append("| Queue | Count | Pull command |")
    lines.append("|-------|------:|--------------|")
    lines.append(f"| Active Decomp Queue | {len(ordered)} | `bash tools/dc.sh next` |")
    lines.append(
        f"| Structural Split Queue | {len(deferred_entries)} | "
        "`bash tools/dc.sh next-structural` |"
    )
    lines.append(
        f"| Asmfix Retirement Queue | {len(asmfix_entries)} | "
        "`bash tools/dc.sh next-asmfix` |"
    )
    lines.append(
        f"| Cheat Cleanup Queue | {len(cheat_orphan_entries)} | "
        "`bash tools/dc.sh next-cheat-cleanup` |"
    )
    lines.append(f"| Permanent Out-of-scope | {len(out_scope)} | none |")
    lines.append("")
    lines.append(
        f"**Total active/deferred queue work: "
        f"{len(ordered) + len(deferred_entries)} functions.**"
    )
    lines.append(
        f"**Total pure-C/asmfix-retirement backlog: "
        f"{len(ordered) + len(deferred_entries) + len(asmfix_entries)} "
        "functions.**"
    )
    if cheat_orphan_entries:
        lines.append(
            f"**Total cheat cleanup backlog (orphan cheats outside other queues): "
            f"{len(cheat_orphan_entries)} functions.**"
        )
    lines.append("")
    lines.append("## How to use")
    lines.append("")
    lines.append("```")
    lines.append("# Get the next ordinary decomp function")
    lines.append("bash tools/dc.sh next")
    lines.append("")
    lines.append("# Get the next function-boundary split task")
    lines.append("bash tools/dc.sh next-structural")
    lines.append("")
    lines.append("# Get the next asmfix retirement task")
    lines.append("bash tools/dc.sh next-asmfix")
    lines.append("```")
    lines.append("")
    lines.append(
        "After matching: commit the result, then `dc.sh refresh-queue` "
        "to regenerate this file (the matched function falls off the queue)."
    )
    if debt_rows:
        lines.append("")
        lines.append(
            "`inline_asm_debt` entries are previously matched C functions "
            "that still contain suspect non-canonical inline asm. They are "
            "not permanent blockers; retire the asm or reduce it to accepted "
            "GTE/BIOS/data-only usage before considering them done."
        )
    lines.append("")
    lines.append("## Tier summary")
    lines.append("")
    lines.append("| Tier | Count | Description |")
    lines.append("|------|-------|-------------|")
    tier_counts: dict[tuple[int, str], int] = defaultdict(int)
    for tier, entries in tiers.items():
        tier_counts[tier] = len(entries)
    for tier in sorted(tier_counts.keys()):
        idx, label = tier
        lines.append(f"| {idx} | {tier_counts[tier]:>3} | {label} |")
    lines.append("")
    lines.append("---")
    lines.append("")
    lines.append("## Queue (top = next)")
    lines.append("")
    lines.append("Format: `<#>  <func>  <size insns>  <rec>  <src>  [tags]`")
    lines.append("")

    # Group by tier with headers in the queue body too
    cur_tier_label: str | None = None
    pos = 0
    for entry in ordered:
        if entry["tier_label"] != cur_tier_label:
            if cur_tier_label is not None:
                lines.append("```")
                lines.append("")
            lines.append(f"### {entry['tier_label']}")
            lines.append("")
            lines.append("```")
            cur_tier_label = entry["tier_label"]
        pos += 1
        tags = f"  [{entry['tags']}]" if entry["tags"] else ""
        lines.append(
            f"{pos:>4}  {entry['func']:<32s}  "
            f"{entry['size']:>4d}  "
            f"{entry['rec']:<20s}  "
            f"{entry['src']:<24s}{tags}"
        )
    if cur_tier_label is not None:
        lines.append("```")
        lines.append("")

    lines.append("---")
    lines.append("")
    lines.append("## Structural Split Queue (top = next-structural)")
    lines.append("")
    lines.append(
        "These entries are not permanent assembly. They contain multiple "
        "`jr $ra` bodies or merged entrypoints and need function-boundary "
        "splitting before they make sense as normal pure-C queue items."
    )
    lines.append("")
    lines.append(
        "Use `bash tools/dc.sh next-structural` to claim one of these."
    )
    lines.append("")
    lines.append(f"**Total structural split queue: {len(deferred_entries)} functions.**")
    lines.append("")
    lines.append("Format: `<#>  <func>  <size insns>  <rec>  <src>  [tags]`")
    lines.append("")
    append_numbered_queue(lines, deferred_entries)

    lines.append("---")
    lines.append("")
    lines.append("## Asmfix Retirement Queue (top = next-asmfix)")
    lines.append("")
    lines.append(
        "These functions have `replace_with_asmfile` entries in `asmfix.txt`. "
        "The build replaces their bytes with `asm/funcs/<func>.s` regardless "
        "of the C body, so they binary-match without being decompiled in pure C. "
        "Use this queue when deliberately retiring asmfix entries."
    )
    lines.append("")
    lines.append(
        "Use `bash tools/dc.sh next-asmfix` to claim one of these. Entries "
        "classified as canonical BIOS/syscall or permanent asm are kept at "
        "the bottom for audit/confirmation rather than ordinary C work."
    )
    lines.append("")
    lines.append(f"**Total asmfix retirement queue: {len(asmfix_entries)} functions.**")
    lines.append("")
    lines.append("Format: `<#>  <func>  <size insns>  <rec>  <src>  [tags]`")
    lines.append("")
    append_numbered_queue(lines, asmfix_entries, group_by_tier=True)

    if cheat_orphan_entries:
        lines.append("---")
        lines.append("")
        lines.append("## Cheat Cleanup Queue (top = next-cheat-cleanup)")
        lines.append("")
        lines.append(
            "These functions are currently SHA1-matching, but only because a "
            "non-canonical asm cheat in `regfix.txt` / `asmfix.txt` / src/*.c "
            "fills a gap that the C source cannot produce naturally. They are "
            "NOT in any other queue (already match, so the active queue "
            "doesn't surface them). The work is to remove the cheat rule(s) "
            "and coerce the C source to emit equivalent codegen — same effort "
            "level as ordinary decomp, just starting from a matching-via-cheat "
            "state instead of an unmatched stub."
        )
        lines.append("")
        lines.append(
            "Cheat types: `splice` (regfix splice rule), `inline_asm` "
            "(file-scope `__asm__(glabel)` cheat in src/*.c), `bios_inline` "
            "(BIOS-style file-scope cheat), `c_body_multi_insn` (multi-instruction "
            "`__asm__` block inside a C function body), `lost_codegen(N)` "
            "(N single-instruction inserts of `addu/or RD,RS,$zero` and "
            "similar GCC-collapsed copies)."
        )
        lines.append("")
        lines.append(
            "Sorted by cheat rule count (ascending — easiest to retire first), "
            "then size. Use `bash tools/dc.sh next-cheat-cleanup` to claim one."
        )
        lines.append("")
        lines.append(
            f"**Total cheat cleanup queue: {len(cheat_orphan_entries)} functions.**"
        )
        lines.append("")
        lines.append("Format: `<#>  <func>  <size insns>  total=<N>  cheat=<N>  <src>  [cheat_types]`")
        lines.append(
            "(Sorted by `total` ascending. Empirically `total ≤ 9` is "
            "single-session retirable; `total ≥ 12` usually needs a deeper "
            "structural rework. See `feedback_cheat_cleanup_techniques.md`.)"
        )
        lines.append("")
        lines.append("```")
        for pos, e in enumerate(cheat_orphan_entries, 1):
            tags = f"  [{e['tags']}]" if e["tags"] else ""
            lines.append(
                f"{pos:>4}  {e['func']:<40s}  "
                f"{e['size']:>4d}  "
                f"total={e['total_rules']:<3d}  "
                f"cheat={e['rule_count']:<3d}  "
                f"{e['src']:<24s}{tags}"
            )
        lines.append("```")
        lines.append("")

    if library_rows:
        lines.append("---")
        lines.append("")
        lines.append("## Mechanical Library Replacements")
        lines.append("")
        lines.append(
            "These entries are not permanent assembly. They match known "
            "PsyQ/libc-style leaf routines and should be replaced through "
            "the library/C-idiom path rather than treated as blocked."
        )
        lines.append("")
        lines.append(f"**Total library shortcuts: {len(library_rows)} functions.**")
        lines.append("")
        library_groups: dict[str, list[str]] = defaultdict(list)
        for r in library_rows:
            library_groups[r["recommendation"]].append(r["func"])
        for rec in library_groups:
            library_groups[rec].sort()
        for rec in sorted(library_groups.keys()):
            funcs = library_groups[rec]
            lines.append(f"### {rec} ({len(funcs)})")
            lines.append("")
            lines.append("```")
            for f in funcs:
                lines.append(f"  {f}")
            lines.append("```")
            lines.append("")

    lines.append("---")
    lines.append("")
    lines.append("## Out of scope (auto-gated by `dc.sh classify`)")
    lines.append("")
    lines.append(
        "These are NOT in the queue. They are either true toolchain/canonical "
        "assembly blockers or explicit BIOS/syscall / non-code-symbol "
        "exceptions. Listed for auditability only — do not attempt them as "
        "ordinary pure-C functions."
    )
    lines.append("")
    lines.append(f"**Total out-of-scope: {len(out_scope)} functions.**")
    lines.append("")
    for rec in sorted(out_groups.keys()):
        funcs = out_groups[rec]
        lines.append(f"### {rec} ({len(funcs)})")
        lines.append("")
        lines.append("```")
        for f in funcs:
            lines.append(f"  {f}")
        lines.append("```")
        lines.append("")

    OUT.write_text(
        "\n".join(line.rstrip() for line in lines) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    print(
        f"Wrote {OUT.relative_to(ROOT)} ({len(ordered)} in-scope, "
        f"{len(asmfix_rows)} asmfix-replaced, {len(deferred_rows)} deferred, "
        f"{len(cheat_orphan_entries)} cheat-cleanup, "
        f"{len(library_rows)} library, {len(out_scope)} out-of-scope)"
    )

    # Print top 10 for convenience
    print()
    print("Top 10 (next to work):")
    for i, e in enumerate(ordered[:10], 1):
        print(f"  {i:>3}. {e['func']:<32s} {e['size']:>4d} insns  "
              f"{e['rec']:<20s}  {e['src']}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
