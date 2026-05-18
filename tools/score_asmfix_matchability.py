#!/usr/bin/env python3
"""Score asmfix queue functions by predicted matchability.

For each function in the asmfix retirement queue, extract signals from
the asm file (asm/funcs/<func>.s) and the existing C body (src/<file>.c)
and compute a matchability score. Higher score = more likely to decompile
cleanly to pure C without plateauing.

The score is heuristic, calibrated against this session's experience:
  - func_80075830 (matched, 104 insns, simple) scored highest
  - cpu_check_run_attack (got close, substantial body + pins) scored well
  - text1b.c motion family (plateau at 20-80 STRUCTURAL) scored low
  - GTE/needs_rodata_split functions scored lowest

Usage:
    python3 tools/score_asmfix_matchability.py            # top 20
    python3 tools/score_asmfix_matchability.py --limit 50 # top 50
    python3 tools/score_asmfix_matchability.py --all      # everything
    python3 tools/score_asmfix_matchability.py --verbose  # show signal breakdown
    python3 tools/score_asmfix_matchability.py --func F   # single function
"""
from __future__ import annotations

import argparse
import re
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WORK_QUEUE = ROOT / "WORK_QUEUE.md"
ASM_DIR = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"

STUB_LINE_RE = re.compile(
    r"^\s*(?:/\*[^*]*\*/|\(void\)\s*\w+\s*;|return\s+\d+\s*;)\s*$"
)

# GTE coprocessor-2 mnemonics + .word encodings
GTE_MNEMONICS = re.compile(
    r"\b(ctc2|mtc2|cfc2|mfc2|lwc2|swc2|mvmva|nclip|op|dpcs|intpl|mvmva|"
    r"ncds|cdp|nccs|cc|nclip|avsz3|avsz4|rtps|rtpt|gpf|gpl|sqr|nct|ncct|"
    r"nce|ncde|cdpcs)\b"
)
# COP2 .word encodings: top byte 0x48-0x4B (COP2), 0xC8-0xCB (LWC2), 0xE8-0xEB (SWC2)
GTE_WORD = re.compile(r"\.word\s+0x[4CE][89AB][0-9A-Fa-f]{6}")

SCRATCHPAD_RE = re.compile(r"0x1F800[0-9A-Fa-f]{3}")
LWL_SWL_RE = re.compile(r"\b(lwl|lwr|swl|swr)\b")
JAL_RE = re.compile(r"\bjal\s+(\w+)")
# Branches (excluding j and jr which are unconditional jumps)
BRANCH_RE = re.compile(r"\b(beq|bne|blez|bgez|bltz|bgtz|beqz|bnez|bne|bgezal|bltzal)\b")
JUMP_TABLE_RE = re.compile(r"\bjr\s+\$(v[01]|a[0-3]|t[0-9])\b")  # not jr $ra
FRAME_RE = re.compile(r"addiu\s+\$sp,\s*\$sp,\s*-0x([0-9A-Fa-f]+)")
CALLEE_SAVE_RE = re.compile(r"sw\s+\$(s[0-7]|fp|ra),")

# C body signals
REGISTER_PIN_RE = re.compile(r'register\s+\w[\w\s\*]*\s+\w+\s+asm\s*\(\s*"\$?([a-z]\w*)"')
ASM_VOLATILE_RE = re.compile(r'__asm__\s*(?:volatile)?\s*\(')
PACKET_BUF_RE = re.compile(r'u8\s+(\w+)\s*\[\s*(?:0x[0-9A-Fa-f]+|\d+)\s*\]')
M2C_ERROR_RE = re.compile(r'M2C_(?:ERROR|FIELD)')


@dataclass
class Signals:
    size: int = 0
    gte_ops: int = 0
    scratchpad_refs: int = 0
    lwl_swl: int = 0
    frame_bytes: int = 0
    callee_saves: int = 0
    jal_count: int = 0
    unique_callees: int = 0
    branch_count: int = 0
    has_jump_table: bool = False
    src_file: str = ""
    body_lines: int = 0       # meaningful (non-blank, non-stub-placeholder)
    has_pins: int = 0          # number of register asm() declarations
    has_packet_buffer: bool = False
    has_inline_asm: int = 0    # number of __asm__ blocks
    has_m2c_errors: bool = False
    is_stub: bool = False      # body has only `(void)argN;` placeholders


def parse_queue() -> list[tuple[str, int, str, str, str]]:
    """Returns (func, size, rec, src, tags) tuples from Asmfix Retirement Queue."""
    text = WORK_QUEUE.read_text(encoding="utf-8")
    rows = []
    in_section = False
    in_block = False
    for line in text.splitlines():
        if line.startswith("## "):
            in_section = "Asmfix Retirement Queue" in line
            continue
        if not in_section:
            continue
        if line.strip() == "```":
            in_block = not in_block
            continue
        if not in_block:
            continue
        m = re.match(
            r"^\s*\d+\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+)(?:\s+\[([^\]]+)\])?",
            line,
        )
        if m:
            rows.append((m.group(1), int(m.group(2)), m.group(3), m.group(4), m.group(5) or ""))
    return rows


def scan_asm(func: str) -> Signals:
    asm_path = ASM_DIR / f"{func}.s"
    s = Signals()
    if not asm_path.exists():
        return s
    text = asm_path.read_text(encoding="utf-8", errors="replace")
    s.gte_ops = len(GTE_MNEMONICS.findall(text)) + len(GTE_WORD.findall(text))
    s.scratchpad_refs = len(SCRATCHPAD_RE.findall(text))
    s.lwl_swl = len(LWL_SWL_RE.findall(text))
    s.branch_count = len(BRANCH_RE.findall(text))
    s.has_jump_table = bool(JUMP_TABLE_RE.search(text))

    jal_matches = JAL_RE.findall(text)
    s.jal_count = len(jal_matches)
    s.unique_callees = len(set(jal_matches))

    fm = FRAME_RE.search(text)
    if fm:
        s.frame_bytes = int(fm.group(1), 16)

    # Callee-saves in the first ~30 lines (prologue)
    prologue = "\n".join(text.splitlines()[:30])
    s.callee_saves = len(set(CALLEE_SAVE_RE.findall(prologue)))
    return s


def find_function_body(c_file: Path, func: str) -> tuple[int, str] | None:
    """Returns (meaningful_line_count, body_text) or None if not found."""
    if not c_file.exists():
        return None
    text = c_file.read_text(encoding="utf-8", errors="replace")
    sig_re = re.compile(
        rf"^(?:[\w\s\*]+\s+){re.escape(func)}\s*\([^;]*\)\s*\{{",
        re.MULTILINE,
    )
    m = sig_re.search(text)
    if not m:
        return None
    body_start = m.end()
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
    body = text[body_start:i - 1]
    meaningful = 0
    in_comment = False
    for line in body.splitlines():
        line = line.strip()
        if not line:
            continue
        if in_comment:
            if "*/" in line:
                in_comment = False
            continue
        if line.startswith("/*"):
            if "*/" not in line:
                in_comment = True
            continue
        if line.startswith("//"):
            continue
        if STUB_LINE_RE.match(line):
            continue
        meaningful += 1
    return meaningful, body


def scan_body(s: Signals, body: str) -> None:
    s.has_pins = len(REGISTER_PIN_RE.findall(body))
    s.has_inline_asm = len(ASM_VOLATILE_RE.findall(body))
    s.has_packet_buffer = bool(PACKET_BUF_RE.search(body))
    s.has_m2c_errors = bool(M2C_ERROR_RE.search(body))
    # is_stub: only placeholder content (we treat <=1 meaningful line as stub)
    s.is_stub = s.body_lines <= 1


def score(s: Signals, rec: str, tags: str) -> tuple[int, list[str]]:
    """Returns (score, explanation_lines). Higher = more likely to match."""
    pts = 0
    why = []

    # Recommendation tier penalty
    if rec == "needs_rodata_split":
        pts -= 200
        why.append("-200 needs_rodata_split (jtbl in another CU)")
    elif rec == "needs_function_split":
        pts -= 150
        why.append("-150 needs_function_split (project-level refactor)")
    elif rec == "needs_lwl_fix":
        pts -= 100
        why.append("-100 needs_lwl_fix (misaligned access)")
    elif rec == "needs_delay_slot_ra":
        pts -= 80
        why.append("-80 needs_delay_slot_ra (aspsx scheduling)")
    elif rec == "gte_function":
        pts -= 60
        why.append("-60 gte_function (needs inline asm + recipe)")
    # else "standard" — no penalty

    # Size: smaller = higher base score. Calibrated against func_80075830 (104 insns, matched).
    if s.size <= 50:
        size_pts = 80
    elif s.size <= 100:
        size_pts = 60
    elif s.size <= 150:
        size_pts = 40
    elif s.size <= 200:
        size_pts = 20
    elif s.size <= 300:
        size_pts = 0
    elif s.size <= 500:
        size_pts = -20
    elif s.size <= 800:
        size_pts = -50
    else:
        size_pts = -80
    pts += size_pts
    why.append(f"{size_pts:+d} size={s.size}insns")

    # Asm-level hard blockers. Existing inline-asm body absorbs GTE/scratchpad,
    # so reduce the penalty when the C body already has many __asm__ blocks.
    asm_offset = min(0.7, s.has_inline_asm / 30.0)  # 0..0.7 reduction factor
    if s.gte_ops > 0:
        gte_pen = int(-5 * min(s.gte_ops, 20) * (1 - asm_offset))
        pts += gte_pen
        why.append(f"{gte_pen:+d} {s.gte_ops} GTE ops (asm-offset={asm_offset:.0%})")
    if s.scratchpad_refs > 0:
        sp_pen = int(-5 * min(s.scratchpad_refs, 20) * (1 - asm_offset))
        pts += sp_pen
        why.append(f"{sp_pen:+d} {s.scratchpad_refs} scratchpad refs (asm-offset={asm_offset:.0%})")
    if s.lwl_swl > 0:
        lwl_pen = -15 * s.lwl_swl
        pts += lwl_pen
        why.append(f"{lwl_pen:+d} {s.lwl_swl} lwl/swl ops")
    if s.has_jump_table:
        pts -= 30
        why.append("-30 jump table (jr $vN)")

    # Frame size: large frames usually mean many locals → harder layout coercion
    if s.frame_bytes > 0:
        if s.frame_bytes <= 0x30:
            frame_pts = 10
        elif s.frame_bytes <= 0x60:
            frame_pts = 5
        elif s.frame_bytes <= 0xA0:
            frame_pts = 0
        elif s.frame_bytes <= 0x100:
            frame_pts = -10
        else:
            frame_pts = -20
        pts += frame_pts
        why.append(f"{frame_pts:+d} frame=0x{s.frame_bytes:X}")

    # Callee-save count beyond 2 (saves $ra + 1-2 are normal)
    cs_excess = max(0, s.callee_saves - 3)
    if cs_excess > 0:
        cs_pen = -5 * cs_excess
        pts += cs_pen
        why.append(f"{cs_pen:+d} {s.callee_saves} callee-saves")

    # jal count — chained calls = pre/post-call register coordination is harder.
    # Leaf or near-leaf functions are much easier; func_80075830 (matched) had 2 jals.
    if s.jal_count == 0:
        pts += 15
        why.append("+15 leaf function (no jal)")
    elif s.jal_count <= 2:
        pts += 5
        why.append(f"+5 near-leaf ({s.jal_count} jal)")
    elif s.jal_count <= 4:
        pass  # neutral
    else:
        jal_pen = -5 * (s.jal_count - 4)
        pts += jal_pen
        why.append(f"{jal_pen:+d} {s.jal_count} jal calls (chained-call coord)")

    # Many branches = complex control flow
    if s.branch_count > 12:
        br_pen = -1 * (s.branch_count - 12)
        pts += br_pen
        why.append(f"{br_pen:+d} {s.branch_count} branches")

    # Tag penalties (already partly captured by rec)
    if "aliasing_heavy" in tags:
        pts -= 15
        why.append("-15 aliasing_heavy")
    if "multi_jr_ra" in tags:
        pts -= 20
        why.append("-20 multi_jr_ra")

    # C body bonuses — substantial body is a head-start but stubs can still match cleanly
    # if the function's intrinsic complexity is low (func_80075830 was a stub and matched).
    if s.body_lines >= 30:
        # Substantial body, but only worth bonus if it's not a plateau-prone family.
        # Cap at +25 to not overwhelm the size/complexity signals.
        body_bonus = min(s.body_lines // 2, 25)
        pts += body_bonus
        why.append(f"+{body_bonus} existing body ({s.body_lines} lines)")
    if s.has_pins > 0:
        # Each pin = 5pts (someone already tried register coordination)
        pin_bonus = min(15, 5 * s.has_pins)
        pts += pin_bonus
        why.append(f"+{pin_bonus} {s.has_pins} register pins already")
    if s.has_packet_buffer:
        pts += 15
        why.append("+15 packet[] buffer already used (matches func_80075830 recipe)")
    if s.has_m2c_errors:
        pts -= 10
        why.append("-10 M2C_ERROR markers in body")

    return pts, why


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, default=20, help="Top N to show (default 20)")
    ap.add_argument("--all", action="store_true", help="Show all candidates")
    ap.add_argument("--verbose", action="store_true", help="Show signal breakdown per func")
    ap.add_argument("--func", help="Score one specific function and show breakdown")
    args = ap.parse_args()

    if args.func:
        # Single-function mode
        # Find it in the queue for size/rec/tags
        rows = parse_queue()
        match = next((r for r in rows if r[0] == args.func), None)
        if not match:
            print(f"NOTE: {args.func} not in active asmfix queue (already matched or not asmfix)")
            print("Computing signals from asm + src only; size derived from asm file.")
            # Count text instructions in asm file
            asm_p = ASM_DIR / f"{args.func}.s"
            size = 0
            if asm_p.exists():
                for line in asm_p.read_text(encoding="utf-8", errors="replace").splitlines():
                    # Count lines with /* HHHH ADDR HEX */ prefix (text instructions)
                    if re.match(r"\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s*\*/", line):
                        size += 1
            rec, src, tags = "standard", "", ""
            # Try to find src file
            for cf in SRC_DIR.glob("*.c"):
                if args.func in cf.read_text(encoding="utf-8", errors="replace"):
                    src = cf.name
                    break
        else:
            _, size, rec, src, tags = match
        sigs = scan_asm(args.func)
        sigs.size = size
        sigs.src_file = src
        body_info = find_function_body(SRC_DIR / src, args.func) if src else None
        if body_info:
            sigs.body_lines, body_text = body_info
            scan_body(sigs, body_text)
        pts, why = score(sigs, rec, tags)
        print(f"\n=== {args.func} ===")
        print(f"  size:      {sigs.size} insns")
        print(f"  src:       {sigs.src_file}")
        print(f"  rec:       {rec}  tags=[{tags}]")
        print(f"  asm: GTE={sigs.gte_ops} scratchpad={sigs.scratchpad_refs} "
              f"lwl/swl={sigs.lwl_swl} jal={sigs.jal_count} ({sigs.unique_callees} unique) "
              f"branches={sigs.branch_count} jtbl={sigs.has_jump_table}")
        print(f"  prologue:  frame=0x{sigs.frame_bytes:X} callee-saves={sigs.callee_saves}")
        print(f"  C body:    {sigs.body_lines} meaningful lines, pins={sigs.has_pins}, "
              f"asm={sigs.has_inline_asm}, packet={sigs.has_packet_buffer}, "
              f"m2c_err={sigs.has_m2c_errors}, stub={sigs.is_stub}")
        print(f"\n  SCORE: {pts}")
        for w in why:
            print(f"    {w}")
        return

    rows = parse_queue()
    results = []
    for func, size, rec, src, tags in rows:
        sigs = scan_asm(func)
        sigs.size = size
        sigs.src_file = src
        body_info = find_function_body(SRC_DIR / src, func)
        if body_info:
            sigs.body_lines, body_text = body_info
            scan_body(sigs, body_text)
        pts, why = score(sigs, rec, tags)
        results.append((pts, func, size, rec, src, tags, sigs, why))

    results.sort(key=lambda x: (-x[0], x[2]))  # score desc, size asc tiebreaker
    n = len(results) if args.all else args.limit

    print(f"# Matchability ranking (top {n} of {len(results)})\n")
    print(f"  Higher score = more likely to decompile cleanly to pure C.")
    print(f"  Scoring is heuristic; calibrated against session 2026-05-18 attempts.\n")
    print(f"{'score':>5}  {'size':>5}  {'func':<32}  {'rec':<22}  {'src':<24}  notes")
    print("-" * 120)
    for pts, func, size, rec, src, tags, sigs, why in results[:n]:
        notes = []
        if sigs.gte_ops > 0:
            notes.append(f"gte={sigs.gte_ops}")
        if sigs.scratchpad_refs > 0:
            notes.append(f"sp={sigs.scratchpad_refs}")
        if sigs.lwl_swl > 0:
            notes.append(f"lwl/swl={sigs.lwl_swl}")
        if sigs.has_jump_table:
            notes.append("jtbl")
        if sigs.frame_bytes > 0xA0:
            notes.append(f"frame=0x{sigs.frame_bytes:X}")
        if sigs.callee_saves > 4:
            notes.append(f"cs={sigs.callee_saves}")
        if not sigs.is_stub:
            notes.append(f"body={sigs.body_lines}L")
        if sigs.has_pins:
            notes.append(f"pins={sigs.has_pins}")
        if sigs.has_packet_buffer:
            notes.append("packet[]")
        notes_str = " ".join(notes) if notes else "—"
        print(f"{pts:>5}  {size:>5}  {func:<32}  {rec:<22}  {src:<24}  {notes_str}")
        if args.verbose:
            for w in why:
                print(f"         {w}")
            print()


if __name__ == "__main__":
    main()
