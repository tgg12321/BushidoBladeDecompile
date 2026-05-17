#!/usr/bin/env python3
"""Audit asm-cheat patterns that bypass the pure-C decomp requirement.

Two patterns make the binary match without the C source actually
corresponding to the emitted bytes:

  1. LARGE SPLICE in regfix.txt
     `func: splice K..N "line1" "line2" ...` with N replacement lines.
     `splice` was designed for small structural transforms (3-line
     example in regfix.py docs). Replacing 100+ lines is essentially
     `replace_with_asmfile` reimplemented in regfix — the C source
     emits nothing useful, the bytes come from the splice content
     verbatim from asm/funcs/<func>.s.

  2. INLINE __asm__ FUNCTION BODY in src/*.c
     `__asm__(".section .text\\n... glabel funcname\\n ... endlabel ...")`
     A multi-instruction `__asm__` block containing `glabel funcname`
     is a file-scope asm function definition. GCC never sees the
     function body as C; the asm text goes straight to gas.

Both patterns are LEGITIMATE for functions whose ORIGINAL implementation
was hand-coded asm (see memory/feedback_hand_coded_asm_recognition.md).
Such functions must be listed in inline_asm_canonical.txt.

Anything not on that list is an UNAUTHORIZED cheat.

Modes:
  --all          : full report, exit 0
  --summary      : one-line counts ("S splices, I inline-asm, B BIOS trampolines"),
                   exit 0. Used by `dc.sh start` for session briefing.
  --func NAME    : check one function; exit 1 if unauthorized cheat found
  --check-new    : compare current state vs HEAD; exit 1 if new cheats added.
                   (Used by the active_func_guard hook before commit.)
  --list-funcs   : print one unauthorized function name per line, exit 0.
                   Used by queue generators to surface cheat-fix work.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

# Thresholds: tuned to distinguish "small structural transform"
# (legitimate splice usage) from "full-body asm replacement".
LARGE_SPLICE_LINES = 30           # single splice rule with >= this many replacement lines
LARGE_SUBST_MULTI_LINES = 30      # single subst_multi rule with >= this many replacement lines
CUMULATIVE_RULE_LINES = 60        # sum across all multi-line ops per function — catches the
                                  # split-evasion attack (e.g., 4x20-line splices to dodge
                                  # the per-rule 30 threshold). 60 leaves headroom for
                                  # legit complex recipes (func_8007CE0C currently 46).
WILDCARD_SUBST_LIMIT = 10         # max `subst ".*"` / `subst ".+"` / `subst ""` rules per
                                  # function. Wildcard substs force-overwrite whatever the
                                  # C source compiled to with the rule's literal text —
                                  # functionally equivalent to replace_with_asmfile (the
                                  # bytes come from the rules, not C codegen). >=10 in
                                  # regfix.txt + regfix_stage2.txt combined = cheat.
GLABEL_BODY_LINES = 5

# Single-instruction `insert`/`insert_after`/`insert_before` rules inject one
# MIPS instruction into the maspsx stream — bytes that didn't come from C
# compilation. The common pattern is restoring something GCC's constant-prop
# or dead-store-eliminator removed (e.g., `addu $sN, $0, $zero` for s2=0).
# These are a smaller-scale cousin of splice: each rule only restores 1 insn
# but they're still asm-injection. Any NEW one in a commit must be flagged
# via --check-new; the EXISTING ones are tracked for cleanup. There is no
# authorized-list equivalent for these — if you need them, the function
# should be in inline_asm_canonical.txt or have a documented recipe note.
INSTRUCTION_INSERT_RE = re.compile(
    r'^(\w+):\s+(insert|insert_after|insert_before)\s+"([^"]*)"\s+@\s+\d+\s*$',
    re.MULTILINE,
)
# Patterns we KNOW are "lost-codegen restoration" (most severe):
LOST_CODEGEN_PATTERNS = (
    re.compile(r"^addu\s+\$\w+,\s*\$0,\s*\$zero"),       # zero-move (s2=0 etc.)
    re.compile(r"^addu\s+\$\w+,\s*\$\w+,\s*\$zero"),     # reg-move (move rd, rs)
    re.compile(r"^addu\s+\$\w+,\s*\$zero,\s*\$\w+"),     # reg-move (move rd, rs, alt)
)

# 3-instruction BIOS trampoline: addiu $tN, $zero, 0xXX; jr $tN; addiu $tM,...
# Common in text1b_b.c func_80078XXX series. Legitimate but should be
# listed in inline_asm_canonical.txt — recommend adding rather than
# silently allowing.
BIOS_TRAMPOLINE = re.compile(
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+'
    r'.{0,80}?'
    r'jr\s+\$t\d+'
    r'.{0,80}?'
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+',
    re.DOTALL,
)


def load_authorized(path="inline_asm_canonical.txt"):
    auth = set()
    p = Path(path)
    if not p.exists():
        return auth
    for line in p.read_text(encoding="utf-8").split("\n"):
        name = line.split("#")[0].strip()
        if name:
            auth.add(name)
    return auth


def scan_regfix_splices(content):
    """Return list of (func, K_orig, N_repl, line_no) for splice rules."""
    cheats = []
    for i, line in enumerate(content.split("\n"), 1):
        m = re.match(r"^(\w+):\s+splice\s+(\d+)\.\.(\d+)\s+(.+)$", line)
        if m:
            func = m.group(1)
            k_orig = int(m.group(3)) - int(m.group(2)) + 1
            n_repl = len(re.findall(r'"[^"]*"', m.group(4)))
            cheats.append((func, k_orig, n_repl, i))
    return cheats


def scan_regfix_subst_multi(content):
    """Return list of (func, N_repl, line_no) for subst_multi rules.

    Syntax: `func: subst_multi "pattern" "line1" "line2" ... [@ idx]`.
    Replacement-line count = (quoted strings) - 1 (pattern is the first arg)."""
    cheats = []
    for i, line in enumerate(content.split("\n"), 1):
        m = re.match(r"^(\w+):\s+subst_multi\s+(.+)$", line)
        if m:
            quoted = re.findall(r'"[^"]*"', m.group(2))
            n_repl = max(0, len(quoted) - 1)
            cheats.append((m.group(1), n_repl, i))
    return cheats


def scan_regfix_wildcard_substs(content):
    """Return {func: wildcard_count} for `subst` rules whose pattern matches
    ANY input (`.*`, `.+`, empty string). These force-overwrite the C-source
    output at index N with the rule's literal text — functionally identical
    to replace_with_asmfile (binary content from the rule, not from C codegen)
    but applied via N small rules per function instead of one big bridge.

    A function with WILDCARD_SUBST_LIMIT+ of these is having its entire
    instruction stream rewritten by regfix — the C source is decorative."""
    from collections import defaultdict
    counts = defaultdict(int)
    for line in content.split("\n"):
        m = re.match(r'^(\w+):\s+subst\s+"([^"]*)"\s+"', line)
        if not m:
            continue
        func, pat = m.group(1), m.group(2)
        if pat in (".*", ".+", ""):
            counts[func] += 1
    return dict(counts)


def scan_regfix_instruction_inserts(content):
    """Return {func: [(op, body, line_no), ...]} for single-instruction
    insert/insert_after/insert_before rules. These inject MIPS instructions
    that didn't come from C compilation. The most common variant (and the
    one that motivated this scanner) is restoring `addu $sN, $0, $zero` —
    a register-zero move that GCC's constant-prop ate.

    Skips:
      - Multi-instruction inserts (contain `\\n` in the quoted body) — those
        are scheduling recipes (delay_slot_fill, etc.) that are checked by
        scan_regfix_cumulative against the 60-line threshold.
      - Label inserts (body ends with `:`) — those are anchor labels, not
        instruction injection.
    """
    from collections import defaultdict
    out = defaultdict(list)
    for m in INSTRUCTION_INSERT_RE.finditer(content):
        func, op, body = m.group(1), m.group(2), m.group(3)
        # Skip multi-insn (encoded via \n inside the quoted body)
        if "\\n" in body:
            continue
        # Skip label-only inserts
        if body.endswith(":"):
            continue
        line_no = content[:m.start()].count("\n") + 1
        out[func].append((op, body, line_no))
    return dict(out)


def _classify_insert_body(body):
    """Categorize a single-instruction insert body. Returns one of:
    'lost_codegen' (zero-move / reg-move — almost always restored GCC output),
    'nop' (scheduling barrier — usually legitimate),
    'other' (everything else).

    The body is the literal quoted string from regfix.txt — `\\t` is two
    characters (escaped tab marker), not an actual tab. Normalize before
    matching so the patterns see real whitespace."""
    norm = body.replace("\\t", " ").replace("\\n", "\n")
    for pat in LOST_CODEGEN_PATTERNS:
        if pat.match(norm):
            return "lost_codegen"
    if norm == "nop":
        return "nop"
    return "other"


def scan_regfix_cumulative(content):
    """Return {func: total_injected_lines} summed across all multi-line ops
    (splice + subst_multi + insert_after/before/label). Used to catch the
    split-evasion attack — splitting one giant splice into N medium-sized
    ones under the per-rule threshold."""
    from collections import defaultdict
    totals = defaultdict(int)
    for func, _k, n_repl, _line in scan_regfix_splices(content):
        totals[func] += n_repl
    for func, n_repl, _line in scan_regfix_subst_multi(content):
        totals[func] += n_repl
    for line in content.split("\n"):
        m = re.match(r"^(\w+):\s+(insert_after|insert_before|insert_label)\s+", line)
        if m:
            totals[m.group(1)] += 1
    return dict(totals)


def _extract_balanced_asm_blocks(text):
    """Yield (start_pos, end_pos, body) for each __asm__(...) block in `text`.
    Properly handles nested parens (offsets like `0($sp)` contain `)`).

    The original `__asm__\\s*\\(...\\([^)]+\\)` regex stops at the first `)`,
    truncating long asm bodies. This generator tracks paren depth."""
    i = 0
    while i < len(text):
        m = re.search(r"__asm__\s*(?:volatile\s*)?\(", text[i:])
        if not m:
            return
        start = i + m.start()
        body_start = i + m.end()
        depth = 1
        j = body_start
        while j < len(text) and depth > 0:
            if text[j] == "(":
                depth += 1
            elif text[j] == ")":
                depth -= 1
            j += 1
        yield start, j, text[body_start:j - 1]
        i = j


def _count_real_insns(block):
    """Count actual asm instructions in a __asm__ block body.

    Strips: directives (.set/.section), labels (glabel/endlabel/alabel/.L1:),
    quote scaffolding. Handles smuggled multi-insn-per-line (\\n separator
    inside a single quoted string)."""
    n = 0
    for src_line in block.split("\n"):
        s = src_line.strip()
        qm = re.match(r'^"(.+)\\n"$', s) or re.match(r'^"(.+)"$', s)
        if not qm:
            continue
        asm = qm.group(1).rstrip("\\n").strip()
        if not asm:
            continue
        if asm.startswith(".") or asm.startswith("glabel") or asm.startswith("endlabel") \
                or asm.startswith("alabel") or asm.endswith(":"):
            continue
        # Each \n inside the quoted string is another insn (smuggling pattern)
        n += asm.count("\\n") + 1
    return n


def _is_whitelisted_insn(insn):
    """Per §6.1: instructions allowed in any inline-asm block (single or multi).

    GTE coprocessor primitives, scheduling barriers (nop), and GTE ops encoded
    as raw words. Anything else is "function logic" and must be split into
    separate single-insn blocks (or expressed in pure C)."""
    insn = insn.strip()
    if insn == "nop":
        return True
    if re.match(r"(cop2|ctc2|mtc2|mfc2|cfc2|lwc2|swc2)\b", insn):
        return True
    # GTE/COP2 .word encodings: 0x4[89A-F]xxxxxx (cop2/ctc2/mtc2/...),
    # 0xC[89A-B]xxxxxx (lwc2), 0xE[89A-B]xxxxxx (swc2)
    if re.match(r"\.word\s+0x[4Cc][89A-Fa-f][0-9A-Fa-f]{6}\b", insn):
        return True
    if re.match(r"\.word\s+0x[Ee][89A-Bb][0-9A-Fa-f]{6}\b", insn):
        return True
    return False


def _enclosing_func_name(text, asm_start):
    """Find the C function definition that contains the asm block at asm_start.
    Returns the function name, or None if at file scope / not in a function."""
    depth = 0
    last_func = None
    pos = 0
    # Walk through the source character by character, tracking brace depth.
    # When depth transitions 0→1, the immediately preceding `name(args)` is
    # the function we're entering.
    func_def_re = re.compile(r"([A-Za-z_]\w*)\s*\([^)]*\)\s*$")
    while pos < asm_start:
        c = text[pos]
        if c == "{":
            if depth == 0:
                # Look back for the function name
                # Scan back through whitespace/newlines to find `name(args)`
                back = pos
                while back > 0 and text[back - 1] in " \t\n":
                    back -= 1
                # Find the matching `(`/`)` pair ending at `back`
                if back > 0 and text[back - 1] == ")":
                    # Find matching `(`
                    d = 1
                    k = back - 2
                    while k > 0 and d > 0:
                        if text[k] == ")":
                            d += 1
                        elif text[k] == "(":
                            d -= 1
                        k -= 1
                    # k+1 is at the `(`, scan back for the identifier
                    name_end = k + 1
                    name_start = name_end
                    while name_start > 0 and (text[name_start - 1].isalnum() or text[name_start - 1] == "_"):
                        name_start -= 1
                    if name_end > name_start:
                        last_func = text[name_start:name_end]
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                last_func = None
        pos += 1
    return last_func if depth > 0 else None


def _has_hardcoded_gpr(insn):
    """True if the instruction contains explicit MIPS GPR references.

    Matches: $0-$31, $zero, $at, $v0/$v1, $a0-$a3, $t0-$t9, $s0-$s7/$fp,
    $k0/$k1, $gp, $sp, $ra. Caller is responsible for first filtering out
    coprocessor instructions (ctc2/cfc2/mtc2/mfc2/lwc2/swc2/.word-encoded
    GTE) — those `$N` operands refer to COP2 registers, not GPRs."""
    return bool(re.search(
        r"\$(?:[0-9]+|zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|fp|k[01]|gp|sp|ra)\b",
        insn,
    ))


def scan_inline_asm_lost_codegen_injection(content, source_name):
    """Return list of (file, line_no, func_name, insn) for single-instruction
    `__asm__` blocks INSIDE C function bodies whose template:
      1. Contains zero `%N` / `%[name]` operand placeholders, AND
      2. Contains hardcoded GPR register references (`$8`, `$v1`, etc.), AND
      3. Matches a `_classify_insert_body` lost_codegen pattern
         (`addu $rD, $rS, $zero`, `addu $rD, $0, $zero`, `addu $rD, $zero, $rS`).

    These are the lost-codegen cheat migrated from regfix.txt into the C
    source. The bytes still don't come from compilation — they come from the
    asm template verbatim with no GCC tracking. Functionally identical to
    `insert_after "addu $t0,$v1,$zero" @ N` in regfix; we must catch both
    forms.

    Excluded (legitimate single-insn asm — these all pass):
      - Templates with `%N` placeholders (proper codegen hints; GCC picks regs)
      - Coprocessor ops (ctc2/cfc2/mtc2/mfc2/lwc2/swc2/COP2 .word) — `$N` there
        is a COP2 register, not GPR
      - Non-lost-codegen single insns (sll/lui/nop/etc.) — those are scheduling
        / barrier / constant-materialization hints, not the cheat pattern."""
    cheats = []
    for asm_start, _end, body in _extract_balanced_asm_blocks(content):
        if _count_real_insns(body) != 1:
            continue
        func_name = _enclosing_func_name(content, asm_start)
        if func_name is None:
            continue
        if re.search(r"glabel\s+\w+", body):
            continue
        # Extract the single instruction text
        insns = []
        for src_line in body.split("\n"):
            s = src_line.strip()
            qm = re.match(r'^"(.+)\\n"$', s) or re.match(r'^"(.+)"$', s)
            if not qm:
                continue
            asm = qm.group(1).rstrip("\\n").strip()
            if not asm or (asm.startswith(".") and not asm.startswith(".word")):
                continue
            if asm.endswith(":") or asm.startswith("glabel") or asm.startswith("endlabel") \
                    or asm.startswith("alabel"):
                continue
            for sub in asm.split("\\n"):
                sub = sub.strip()
                if sub:
                    insns.append(sub)
        if len(insns) != 1:
            continue
        insn = insns[0]
        if re.search(r"%[0-9]+|%\[", insn):
            continue
        if _is_whitelisted_insn(insn):
            continue
        if not _has_hardcoded_gpr(insn):
            continue
        if _classify_insert_body(insn) != "lost_codegen":
            continue
        line_no = content[:asm_start].count("\n") + 1
        cheats.append((source_name, line_no, func_name, insn))
    return cheats


def scan_c_body_smuggled_work(content, source_name):
    """Return list of (file, line_no, insn_count, enclosing_func, insns) for
    multi-instruction __asm__ blocks INSIDE C function bodies that contain at
    least one instruction not on the §6.1 whitelist.

    §6.1's "single instruction per asm block" rule is violated by ANY
    multi-insn block, but the high-severity case is multi-insn blocks
    smuggling non-GTE/non-scheduling work via \\n-concatenation. Those are
    what this scanner flags as cheats."""
    cheats = []
    for asm_start, _, body in _extract_balanced_asm_blocks(content):
        insn_count = _count_real_insns(body)
        if insn_count <= 1:
            continue
        # Skip file-scope blocks (those go through scan_inline_asm_bodies)
        if _enclosing_func_name(content, asm_start) is None:
            continue
        # Skip blocks with glabel (file-scope cheats, in case is_file_scope missed)
        if re.search(r"glabel\s+\w+", body):
            continue
        # Extract instruction lines and check whitelist
        insns = []
        for src_line in body.split("\n"):
            s = src_line.strip()
            qm = re.match(r'^"(.+)\\n"$', s) or re.match(r'^"(.+)"$', s)
            if not qm:
                continue
            asm = qm.group(1).rstrip("\\n").strip()
            if not asm or asm.startswith(".") and not asm.startswith(".word"):
                continue
            if asm.endswith(":") or asm.startswith("glabel") or asm.startswith("endlabel") or asm.startswith("alabel"):
                continue
            # Handle smuggled multi-insn-per-line via \n
            for sub in asm.split("\\n"):
                sub = sub.strip()
                if sub:
                    insns.append(sub)
        has_smuggled_work = any(not _is_whitelisted_insn(i) for i in insns)
        if not has_smuggled_work:
            continue
        line_no = content[:asm_start].count("\n") + 1
        func = _enclosing_func_name(content, asm_start)
        cheats.append((source_name, line_no, insn_count, func, insns))
    return cheats


def _trampoline_at_start(block):
    """Return True if the trampoline pattern (addiu $tN, $zero, 0xVEC;
    jr $tN; addiu $tM, $zero, FUNC#) appears at the START of the function
    body — within the first ~5 instruction lines after glabel.

    This catches BIOS-trampoline-style functions regardless of trailing
    content (padding nops, embedded data tables, jumptable installation
    code, etc.). The original `insn_count <= 12 and BIOS_TRAMPOLINE.search()`
    check missed func_800831D8 (trampoline + 3 padding nops, 13 lines) and
    func_8008D060 (trampoline + 50-line module table)."""
    # Find glabel position; only look in the first ~400 chars after it
    gm = re.search(r"glabel\s+\w+", block)
    if not gm:
        return False
    # The first 400 chars after glabel covers: optional .set directives,
    # the 3 trampoline instructions, and small amounts of whitespace. The
    # original 80-char-gap regex requires the 3 pattern parts within tight
    # proximity; we just additionally constrain that they're NEAR the start.
    head = block[gm.end():gm.end() + 400]
    return bool(BIOS_TRAMPOLINE.search(head))


def scan_inline_asm_bodies(content, source_name):
    """Return list of (file, line_no, asm_lines, func_name, is_bios) for
    `__asm__` blocks containing `glabel funcname`.

    `is_bios=True` covers any block whose body STARTS with the 3-instruction
    BIOS trampoline pattern, regardless of trailing content. Padding nops,
    embedded module tables, jumptable installation code — all valid trailing
    content for a BIOS-related function. The trampoline-at-start is the
    discriminator."""
    cheats = []
    for m in re.finditer(r"__asm__\s*(?:volatile\s*)?\(([^)]+)\)", content):
        block = m.group(1)
        glabel_match = re.search(r"glabel\s+(\w+)", block)
        if not glabel_match:
            continue
        fname = glabel_match.group(1)
        insn_count = block.count("\\n")
        line_no = content[:m.start()].count("\n") + 1
        is_bios = _trampoline_at_start(block)
        cheats.append((source_name, line_no, insn_count, fname, is_bios))
    return cheats


def get_current_cheats(auth):
    """Scan current working tree. Return (splice_funcs, inline_funcs,
    bios_funcs, c_body_funcs, instruction_insert_funcs, asm_injection_funcs).

    `splice_funcs` covers all regfix-injection cheats (large splice / large
    subst_multi / cumulative-volume / wildcard subst). `inline_funcs` covers
    file-scope __asm__(glabel) cheats. `c_body_funcs` covers multi-instruction
    __asm__ blocks INSIDE C function bodies that smuggle non-GTE/non-scheduling
    work via \\n-concatenation. `instruction_insert_funcs` covers per-function
    counts of single-instruction lost-codegen insert rules in regfix.txt (zero-
    move / reg-move — these inject what GCC's optimizer ate, sub-threshold for
    the cumulative line check but still asm injection). `asm_injection_funcs`
    covers single-instruction `__asm__` blocks in C bodies whose template is
    a hardcoded lost-codegen-pattern instruction with no `%N` placeholders
    (the same cheat moved from regfix to C source)."""
    splice_funcs = set()
    inline_funcs = set()
    bios_funcs = set()
    c_body_funcs = set()
    instruction_insert_funcs = {}
    asm_injection_funcs = {}

    # Combine stage1 + stage2 regfix content for unified per-function checks.
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    if regfix_content:
        for func, k, n, line in scan_regfix_splices(regfix_content):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(regfix_content):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(regfix_content).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, wcount in scan_regfix_wildcard_substs(regfix_content).items():
            if wcount >= WILDCARD_SUBST_LIMIT and func not in auth:
                splice_funcs.add(func)
        # Track lost-codegen single-instruction inserts per function.
        # Authorized functions (inline_asm_canonical.txt) skip the count.
        for func, entries in scan_regfix_instruction_inserts(regfix_content).items():
            if func in auth:
                continue
            lost = sum(1 for _op, body, _ln in entries
                       if _classify_insert_body(body) == "lost_codegen")
            if lost > 0:
                instruction_insert_funcs[func] = lost

    for src in sorted(Path("src").glob("*.c")):
        text = src.read_text(encoding="utf-8")
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(text, src.name):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth:
                continue
            if is_bios:
                bios_funcs.add(fname)
            else:
                inline_funcs.add(fname)
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(text, src.name):
            if fname and fname not in auth:
                c_body_funcs.add(fname)
        for f, l, fname, _insn in scan_inline_asm_lost_codegen_injection(text, src.name):
            if fname and fname not in auth:
                asm_injection_funcs[fname] = asm_injection_funcs.get(fname, 0) + 1

    return (splice_funcs, inline_funcs, bios_funcs, c_body_funcs,
            instruction_insert_funcs, asm_injection_funcs)


def get_head_cheats(auth):
    """Same scan but against HEAD revision (for --check-new diff)."""
    splice_funcs = set()
    inline_funcs = set()
    c_body_funcs = set()
    instruction_insert_funcs = {}
    asm_injection_funcs = {}

    head_regfix = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        try:
            head_regfix += subprocess.run(
                ["git", "show", f"HEAD:{fname}"],
                capture_output=True, text=True, check=True, encoding="utf-8", errors="replace",
            ).stdout + "\n"
        except subprocess.CalledProcessError:
            pass
    if head_regfix:
        for func, k, n, line in scan_regfix_splices(head_regfix):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(head_regfix):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(head_regfix).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, wcount in scan_regfix_wildcard_substs(head_regfix).items():
            if wcount >= WILDCARD_SUBST_LIMIT and func not in auth:
                splice_funcs.add(func)
        for func, entries in scan_regfix_instruction_inserts(head_regfix).items():
            if func in auth:
                continue
            lost = sum(1 for _op, body, _ln in entries
                       if _classify_insert_body(body) == "lost_codegen")
            if lost > 0:
                instruction_insert_funcs[func] = lost

    # All .c files tracked at HEAD
    src_list = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", "HEAD", "src/"],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    ).stdout.split("\n")
    for src_path in src_list:
        if not src_path.endswith(".c"):
            continue
        try:
            head_content = subprocess.run(
                ["git", "show", f"HEAD:{src_path}"],
                capture_output=True, text=True, check=True, encoding="utf-8", errors="replace",
            ).stdout
        except subprocess.CalledProcessError:
            continue
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(head_content, src_path):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth or is_bios:
                continue
            inline_funcs.add(fname)
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(head_content, src_path):
            if fname and fname not in auth:
                c_body_funcs.add(fname)
        for f, l, fname, _insn in scan_inline_asm_lost_codegen_injection(head_content, src_path):
            if fname and fname not in auth:
                asm_injection_funcs[fname] = asm_injection_funcs.get(fname, 0) + 1

    return (splice_funcs, inline_funcs, c_body_funcs, instruction_insert_funcs,
            asm_injection_funcs)


def cmd_all():
    auth = load_authorized()
    # Combine stage1 + stage2 regfix files for unified per-function checks.
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    splices = scan_regfix_splices(regfix_content)
    full_splices = [s for s in splices if s[2] >= LARGE_SPLICE_LINES]
    unauth_splices = [s for s in full_splices if s[0] not in auth]

    subst_multis = scan_regfix_subst_multi(regfix_content)
    large_subst_multis = [s for s in subst_multis if s[1] >= LARGE_SUBST_MULTI_LINES]
    unauth_subst_multis = [s for s in large_subst_multis if s[0] not in auth]

    cumulative = scan_regfix_cumulative(regfix_content)
    over_cumulative = sorted(
        ((fn, total) for fn, total in cumulative.items()
         if total >= CUMULATIVE_RULE_LINES and fn not in auth),
        key=lambda x: -x[1],
    )

    wildcard = scan_regfix_wildcard_substs(regfix_content)
    over_wildcard = sorted(
        ((fn, n) for fn, n in wildcard.items()
         if n >= WILDCARD_SUBST_LIMIT and fn not in auth),
        key=lambda x: -x[1],
    )

    inserts = scan_regfix_instruction_inserts(regfix_content)
    insert_lost_per_func = {}
    for fn, entries in inserts.items():
        if fn in auth:
            continue
        lost = [(op, body, ln) for op, body, ln in entries
                if _classify_insert_body(body) == "lost_codegen"]
        if lost:
            insert_lost_per_func[fn] = lost
    insert_lost_sorted = sorted(insert_lost_per_func.items(), key=lambda x: -len(x[1]))

    print(f"=== regfix.txt splice rules ===")
    print(f"  {len(splices)} total, {len(full_splices)} full-body (>= {LARGE_SPLICE_LINES} replacement lines)")
    print(f"  {len(unauth_splices)} UNAUTHORIZED:")
    for func, k, n, line in sorted(unauth_splices, key=lambda x: -x[2]):
        print(f"    line {line:5d}  {func:32s}  orig_K={k:4d}  replace_N={n:4d}")

    print()
    print(f"=== regfix.txt subst_multi rules ===")
    print(f"  {len(subst_multis)} total, {len(large_subst_multis)} large (>= {LARGE_SUBST_MULTI_LINES} replacement lines)")
    print(f"  {len(unauth_subst_multis)} UNAUTHORIZED:")
    for func, n, line in sorted(unauth_subst_multis, key=lambda x: -x[1]):
        print(f"    line {line:5d}  {func:32s}  replace_N={n:4d}")

    print()
    print(f"=== regfix.txt cumulative rule volume per function ===")
    print(f"  {sum(1 for v in cumulative.values() if v > 0)} functions have multi-line rules")
    print(f"  threshold: {CUMULATIVE_RULE_LINES} cumulative lines (splice + subst_multi + insert_*)")
    print(f"  {len(over_cumulative)} UNAUTHORIZED over threshold:")
    for fn, total in over_cumulative:
        print(f"    {fn:32s}  total_lines={total}")

    print()
    print(f"=== regfix wildcard `subst \".*\"` rules per function ===")
    print(f"  {sum(1 for v in wildcard.values() if v > 0)} functions have wildcard substs")
    print(f"  threshold: {WILDCARD_SUBST_LIMIT}+ wildcard substs = force-overwrite cheat")
    print(f"  {len(over_wildcard)} UNAUTHORIZED over threshold:")
    for fn, n in over_wildcard:
        print(f"    {fn:32s}  wildcard_count={n}")

    print()
    print(f"=== regfix single-instruction lost-codegen inserts per function ===")
    print(f"  `insert_after \"addu $sN,$0,$zero\" @ idx` and similar — restore an")
    print(f"  instruction GCC's optimizer ate (constant-prop / dead-store).")
    print(f"  These are asm injection (1 insn each), below the cumulative-{CUMULATIVE_RULE_LINES}")
    print(f"  threshold but still bytes not from C compilation.")
    print(f"  {len(insert_lost_per_func)} functions have lost-codegen inserts:")
    for fn, entries in insert_lost_sorted:
        print(f"    {fn:32s}  count={len(entries)}")
        for op, body, ln in entries[:3]:
            print(f"      line {ln:5d}  {op:14s}  {body!r}")
        if len(entries) > 3:
            print(f"      ... and {len(entries) - 3} more")

    print()
    print(f"=== Inline __asm__ glabel function bodies in src/*.c ===")
    inline_all = []
    for src in sorted(Path("src").glob("*.c")):
        inline_all.extend(
            scan_inline_asm_bodies(src.read_text(encoding="utf-8"), src.name)
        )
    significant = [b for b in inline_all if b[2] >= GLABEL_BODY_LINES]
    auth_blocks = [b for b in significant if b[3] in auth]
    bios = [b for b in significant if b[4] and b[3] not in auth]
    unauth = [b for b in significant if not b[4] and b[3] not in auth]
    print(f"  {len(significant)} with >= {GLABEL_BODY_LINES} lines:")
    print(f"    {len(auth_blocks):4d} authorized (in inline_asm_canonical.txt)")
    print(f"    {len(bios):4d} BIOS trampolines (3-insn pattern; should be added to authorized list)")
    print(f"    {len(unauth):4d} UNAUTHORIZED")
    for f, l, n, fname, _ in sorted(unauth, key=lambda x: -x[2])[:25]:
        print(f"    {f:25s}:{l:6d}  {fname:25s}  ~{n} lines")
    if len(unauth) > 25:
        print(f"    ... and {len(unauth) - 25} more")

    print()
    print(f"=== Multi-instruction __asm__ inside C function bodies ===")
    c_body_all = []
    for src in sorted(Path("src").glob("*.c")):
        c_body_all.extend(
            scan_c_body_smuggled_work(src.read_text(encoding="utf-8"), src.name)
        )
    c_body_unauth = [c for c in c_body_all if c[3] and c[3] not in auth]
    print(f"  {len(c_body_all)} multi-insn blocks containing non-§6.1-whitelisted instructions")
    print(f"  {len(c_body_unauth)} UNAUTHORIZED (host function not in inline_asm_canonical.txt):")
    for f, l, n, fname, insns in sorted(c_body_unauth, key=lambda x: -x[2])[:15]:
        print(f"    {f:25s}:{l:6d}  in {fname:30s}  ({n} insns)")
    if len(c_body_unauth) > 15:
        print(f"    ... and {len(c_body_unauth) - 15} more")

    print()
    print(f"=== Inline-asm lost-codegen INJECTIONS inside C function bodies ===")
    print(f"  Single-instruction __asm__ blocks with hardcoded `$N` registers (no `%N`")
    print(f"  placeholders) emitting `addu RD,RS,$zero`-style moves — the same lost-")
    print(f"  codegen cheat migrated from regfix.txt into the C source. Bytes still")
    print(f"  don't come from compilation; GCC has no view into the operation.")
    asm_inj_all = []
    for src in sorted(Path("src").glob("*.c")):
        asm_inj_all.extend(
            scan_inline_asm_lost_codegen_injection(src.read_text(encoding="utf-8"), src.name)
        )
    asm_inj_unauth = [c for c in asm_inj_all if c[2] and c[2] not in auth]
    asm_inj_by_func = {}
    for f, l, fname, insn in asm_inj_unauth:
        asm_inj_by_func.setdefault(fname, []).append((f, l, insn))
    print(f"  {len(asm_inj_all)} injection blocks across all src/*.c")
    print(f"  {len(asm_inj_by_func)} UNAUTHORIZED functions (not in inline_asm_canonical.txt):")
    for fname, entries in sorted(asm_inj_by_func.items(), key=lambda x: -len(x[1])):
        print(f"    {fname:32s}  count={len(entries)}")
        for f, l, insn in entries[:3]:
            print(f"      {f}:{l}  __asm__(\"{insn}\")")
        if len(entries) > 3:
            print(f"      ... and {len(entries) - 3} more")

    print()
    # De-dup across regfix categories: a single func can hit splice + subst_multi
    # + cumulative + wildcard + lost-codegen inserts + inline-asm-injection at once.
    regfix_unauth_funcs = (
        {s[0] for s in unauth_splices}
        | {s[0] for s in unauth_subst_multis}
        | {fn for fn, _ in over_cumulative}
        | {fn for fn, _ in over_wildcard}
        | set(insert_lost_per_func.keys())
    )
    c_body_unauth_funcs = {c[3] for c in c_body_unauth}
    asm_inj_unauth_funcs = set(asm_inj_by_func.keys())
    total_unauth = (len(regfix_unauth_funcs) + len(unauth)
                    + len(c_body_unauth_funcs) + len(asm_inj_unauth_funcs))
    if total_unauth > 0:
        print(f"TOTAL UNAUTHORIZED ASM CHEATS: {total_unauth}")
        print(f"  ({len(bios)} BIOS trampolines should also be added to inline_asm_canonical.txt)")
    else:
        print("No unauthorized asm cheats. All inline-asm bodies and regfix injections are accounted for.")
    return 0


def cmd_func(name):
    auth = load_authorized()
    if name in auth:
        return 0
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    for func, k, n, line in scan_regfix_splices(regfix_content):
        if func == name and n >= LARGE_SPLICE_LINES:
            print(f"UNAUTHORIZED: {name} has full-body splice in regfix.txt:{line} "
                  f"({n}-line replacement). Add to inline_asm_canonical.txt or do real C decomp.",
                  file=sys.stderr)
            return 1
    for func, n, line in scan_regfix_subst_multi(regfix_content):
        if func == name and n >= LARGE_SUBST_MULTI_LINES:
            print(f"UNAUTHORIZED: {name} has large subst_multi in regfix.txt:{line} "
                  f"({n}-line replacement). Add to inline_asm_canonical.txt or do real C decomp.",
                  file=sys.stderr)
            return 1
    total = scan_regfix_cumulative(regfix_content).get(name, 0)
    if total >= CUMULATIVE_RULE_LINES:
        print(f"UNAUTHORIZED: {name} has {total} cumulative regfix-injected lines "
              f"(threshold {CUMULATIVE_RULE_LINES}). Looks like split-evasion of the "
              f"per-rule splice threshold. Add to inline_asm_canonical.txt or do real C decomp.",
              file=sys.stderr)
        return 1
    wcount = scan_regfix_wildcard_substs(regfix_content).get(name, 0)
    if wcount >= WILDCARD_SUBST_LIMIT:
        print(f"UNAUTHORIZED: {name} has {wcount} wildcard `subst \".*\"` rules in regfix.txt/"
              f"regfix_stage2.txt (threshold {WILDCARD_SUBST_LIMIT}). These force-overwrite "
              f"the C source's compiled output line-by-line — functionally equivalent to "
              f"replace_with_asmfile. Remove the substs and write real C, or add to "
              f"inline_asm_canonical.txt with authorization.",
              file=sys.stderr)
        return 1
    inserts = scan_regfix_instruction_inserts(regfix_content).get(name, [])
    lost = [(op, body, ln) for op, body, ln in inserts
            if _classify_insert_body(body) == "lost_codegen"]
    if lost:
        print(f"UNAUTHORIZED: {name} has {len(lost)} lost-codegen single-instruction "
              f"insert rule(s) in regfix.txt:", file=sys.stderr)
        for op, body, ln in lost:
            print(f"  line {ln:5d}  {op:14s}  {body!r}", file=sys.stderr)
        print(f"These inject MIPS instructions that didn't come from C compilation "
              f"(usually restoring what GCC's optimizer ate). Add to "
              f"inline_asm_canonical.txt or rewrite the C to defeat the optimizer "
              f"(register pinning, structure change, etc.).", file=sys.stderr)
        return 1
    for src in sorted(Path("src").glob("*.c")):
        text = src.read_text(encoding="utf-8")
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(text, src.name):
            if fname == name and n >= GLABEL_BODY_LINES and not is_bios:
                print(f"UNAUTHORIZED: {name} has inline __asm__ body in {f}:{l} "
                      f"(~{n} lines). Add to inline_asm_canonical.txt or do real C decomp.",
                      file=sys.stderr)
                return 1
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(text, src.name):
            if fname == name:
                print(f"UNAUTHORIZED: {name} has multi-insn __asm__ block in {f}:{l} "
                      f"({n} insns) containing non-§6.1-whitelisted instructions. "
                      f"Split into per-instruction __asm__ blocks or do real C decomp.",
                      file=sys.stderr)
                return 1
        injections = [(f, l, insn) for f, l, fn, insn
                      in scan_inline_asm_lost_codegen_injection(text, src.name)
                      if fn == name]
        if injections:
            print(f"UNAUTHORIZED: {name} has {len(injections)} inline-asm "
                  f"lost-codegen INJECTION(s) (hardcoded `$N` registers, no `%N` "
                  f"placeholders, body matches lost_codegen pattern):", file=sys.stderr)
            for f, l, insn in injections:
                print(f"  {f}:{l}  __asm__(\"{insn}\")", file=sys.stderr)
            print(f"These emit the same MIPS bytes as `insert_after \"addu RD,RS,$zero\" @ N` "
                  f"would — just sourced from inline asm with no GCC tracking. The cheat "
                  f"under a different name (per skill §7 anti-pattern). Use proper operand "
                  f"constraints (`%0`/`%1` with `=r`/`r` binding to `register T x asm(\"$N\")` "
                  f"variables) so GCC tracks the operation, or restructure the C to defeat "
                  f"the optimizer.", file=sys.stderr)
            return 1
    return 0


def cmd_check_new():
    """Return 1 if working tree has cheats not present at HEAD."""
    auth = load_authorized()
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(auth)
    (head_splice, head_inline, head_c_body, head_inserts,
     head_asm_inj) = get_head_cheats(auth)

    new_splices = cur_splice - head_splice
    new_inline = cur_inline - head_inline
    new_c_body = cur_c_body - head_c_body
    # New insert cheats: any function whose lost-codegen insert count went up,
    # OR any function that gained lost-codegen inserts.
    new_inserts = {}
    for fn, n in cur_inserts.items():
        head_n = head_inserts.get(fn, 0)
        if n > head_n:
            new_inserts[fn] = (head_n, n)
    # New inline-asm lost-codegen INJECTIONS in src/*.c (same cheat moved
    # from regfix to C source — the §7 "anti-pattern under a different name").
    new_asm_inj = {}
    for fn, n in cur_asm_inj.items():
        head_n = head_asm_inj.get(fn, 0)
        if n > head_n:
            new_asm_inj[fn] = (head_n, n)

    if (not new_splices and not new_inline and not new_c_body
            and not new_inserts and not new_asm_inj):
        return 0

    print("ASM-CHEAT GUARD: new unauthorized asm-cheat patterns since HEAD:", file=sys.stderr)
    for func in sorted(new_splices):
        print(f"  Regfix injection for {func} (large splice / large subst_multi / "
              f"cumulative >= {CUMULATIVE_RULE_LINES} lines) in regfix.txt", file=sys.stderr)
    for fname in sorted(new_inline):
        print(f"  Inline __asm__ glabel body for {fname} in src/*.c", file=sys.stderr)
    for fname in sorted(new_c_body):
        print(f"  Multi-insn __asm__ block inside C body of {fname} "
              f"(non-§6.1-whitelisted insns smuggled via \\n-concatenation)", file=sys.stderr)
    for fn, (head_n, cur_n) in sorted(new_inserts.items()):
        delta = cur_n - head_n
        print(f"  Lost-codegen insert(s) for {fn}: HEAD had {head_n}, now {cur_n} "
              f"(+{delta}). Single-instruction `insert_after \"addu $sN,$0,$zero\"` rules "
              f"inject what GCC's optimizer ate — those bytes don't come from C", file=sys.stderr)
    for fn, (head_n, cur_n) in sorted(new_asm_inj.items()):
        delta = cur_n - head_n
        print(f"  Inline-asm lost-codegen INJECTION for {fn}: HEAD had {head_n}, "
              f"now {cur_n} (+{delta}). Single-instruction `__asm__` block with "
              f"hardcoded `$N` registers (no `%N` placeholders) emitting `addu RD,RS,$zero` "
              f"-style move — the same lost-codegen cheat migrated from regfix.txt into "
              f"the C source. Bytes still don't come from compilation; GCC has no view "
              f"into the operation. Use proper operand constraints (`%0`/`%1` with `=r`/`r` "
              f"binding to `register T x asm(\"$N\")` variables) so GCC tracks it, or "
              f"restructure the C to defeat the optimizer.", file=sys.stderr)
    print(file=sys.stderr)
    print("These patterns make the binary match without the C source", file=sys.stderr)
    print("actually corresponding to the emitted bytes (the bytes come from", file=sys.stderr)
    print("asm injected via splice/subst_multi/insert chains, file-scope __asm__,", file=sys.stderr)
    print("multi-instruction asm blocks smuggling work, or single-instruction", file=sys.stderr)
    print("lost-codegen inserts).", file=sys.stderr)
    print(file=sys.stderr)
    print("If the original was hand-coded asm (see memory/", file=sys.stderr)
    print("feedback_hand_coded_asm_recognition.md), add the function name", file=sys.stderr)
    print("to inline_asm_canonical.txt with a justification comment, then", file=sys.stderr)
    print("re-commit.", file=sys.stderr)
    print(file=sys.stderr)
    print("If it's canonically C (~99% of the codebase), write a real C", file=sys.stderr)
    print("decomp instead of wrapping the asm. For lost-codegen inserts,", file=sys.stderr)
    print("try register pinning (`register T x asm(\"$N\")`) or a different C", file=sys.stderr)
    print("structure that GCC won't constant-fold.", file=sys.stderr)
    return 1


def cmd_summary():
    """One-line counts of unauthorized cheats. For session briefing."""
    auth = load_authorized()
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(auth)
    n_splice = len(cur_splice)
    n_inline = len(cur_inline)
    n_bios = len(cur_bios)
    n_c_body = len(cur_c_body)
    n_inserts = len(cur_inserts)
    n_asm_inj = len(cur_asm_inj)
    total = n_splice + n_inline + n_c_body + n_inserts + n_asm_inj
    if total == 0 and n_bios == 0:
        print("Cheats:   none — all inline asm and regfix injections are authorized")
    else:
        parts = []
        if n_splice:
            parts.append(f"{n_splice} splice")
        if n_inline:
            parts.append(f"{n_inline} inline-asm")
        if n_c_body:
            parts.append(f"{n_c_body} c-body-multi-insn")
        if n_inserts:
            total_lost = sum(cur_inserts.values())
            parts.append(f"{n_inserts} func(s) w/ lost-codegen inserts ({total_lost} insn(s))")
        if n_asm_inj:
            total_inj = sum(cur_asm_inj.values())
            parts.append(f"{n_asm_inj} func(s) w/ inline-asm injection ({total_inj} insn(s))")
        if n_bios:
            parts.append(f"{n_bios} BIOS-trampoline (recommend adding to authorized list)")
        msg = ", ".join(parts)
        print(f"Cheats:   {total} unauthorized ({msg})")
        print(f"          run 'python3 tools/audit_asm_cheats.py --all' to list, or")
        print(f"          'dc.sh next-cheat' to pull a cheat-fix work item")
    return 0


def cmd_list_funcs():
    """Print one unauthorized function name per line."""
    auth = load_authorized()
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(auth)
    for f in sorted(cur_splice | cur_inline | cur_c_body
                    | set(cur_inserts.keys()) | set(cur_asm_inj.keys())):
        print(f)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--all", action="store_true", help="full report of all cheats (default)")
    g.add_argument("--summary", action="store_true", help="one-line cheat counts")
    g.add_argument("--func", help="check one function; non-zero exit if unauthorized")
    g.add_argument("--check-new", action="store_true",
                   help="compare current state vs HEAD; non-zero exit if new cheats added")
    g.add_argument("--list-funcs", action="store_true",
                   help="print unauthorized function names, one per line")
    args = ap.parse_args()
    if args.summary:
        return cmd_summary()
    if args.list_funcs:
        return cmd_list_funcs()
    if args.check_new:
        return cmd_check_new()
    if args.func:
        return cmd_func(args.func)
    return cmd_all()


if __name__ == "__main__":
    sys.exit(main())
