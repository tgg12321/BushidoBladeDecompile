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
GLABEL_BODY_LINES = 5

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
    bios_funcs, c_body_funcs).

    `splice_funcs` covers all regfix-injection cheats. `inline_funcs` covers
    file-scope __asm__(glabel) cheats. `c_body_funcs` covers multi-instruction
    __asm__ blocks INSIDE C function bodies that smuggle non-GTE/non-scheduling
    work via \\n-concatenation."""
    splice_funcs = set()
    inline_funcs = set()
    bios_funcs = set()
    c_body_funcs = set()

    regfix = Path("regfix.txt")
    if regfix.exists():
        content = regfix.read_text(encoding="utf-8")
        for func, k, n, line in scan_regfix_splices(content):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(content):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(content).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)

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

    return splice_funcs, inline_funcs, bios_funcs, c_body_funcs


def get_head_cheats(auth):
    """Same scan but against HEAD revision (for --check-new diff)."""
    splice_funcs = set()
    inline_funcs = set()
    c_body_funcs = set()

    try:
        head_regfix = subprocess.run(
            ["git", "show", "HEAD:regfix.txt"],
            capture_output=True, text=True, check=True,
        ).stdout
        for func, k, n, line in scan_regfix_splices(head_regfix):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(head_regfix):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(head_regfix).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)
    except subprocess.CalledProcessError:
        pass

    # All .c files tracked at HEAD
    src_list = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", "HEAD", "src/"],
        capture_output=True, text=True,
    ).stdout.split("\n")
    for src_path in src_list:
        if not src_path.endswith(".c"):
            continue
        try:
            head_content = subprocess.run(
                ["git", "show", f"HEAD:{src_path}"],
                capture_output=True, text=True, check=True,
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

    return splice_funcs, inline_funcs, c_body_funcs


def cmd_all():
    auth = load_authorized()
    regfix_content = Path("regfix.txt").read_text(encoding="utf-8") if Path("regfix.txt").exists() else ""
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
    # De-dup across regfix categories: a single func can hit splice + subst_multi
    # + cumulative all at once.
    regfix_unauth_funcs = (
        {s[0] for s in unauth_splices}
        | {s[0] for s in unauth_subst_multis}
        | {fn for fn, _ in over_cumulative}
    )
    c_body_unauth_funcs = {c[3] for c in c_body_unauth}
    total_unauth = len(regfix_unauth_funcs) + len(unauth) + len(c_body_unauth_funcs)
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
    regfix_content = Path("regfix.txt").read_text(encoding="utf-8") if Path("regfix.txt").exists() else ""
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
    return 0


def cmd_check_new():
    """Return 1 if working tree has cheats not present at HEAD."""
    auth = load_authorized()
    cur_splice, cur_inline, cur_bios, cur_c_body = get_current_cheats(auth)
    head_splice, head_inline, head_c_body = get_head_cheats(auth)

    new_splices = cur_splice - head_splice
    new_inline = cur_inline - head_inline
    new_c_body = cur_c_body - head_c_body

    if not new_splices and not new_inline and not new_c_body:
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
    print(file=sys.stderr)
    print("These patterns make the binary match without the C source", file=sys.stderr)
    print("actually corresponding to the emitted bytes (the bytes come from", file=sys.stderr)
    print("asm injected via splice/subst_multi/insert chains, file-scope __asm__,", file=sys.stderr)
    print("or multi-instruction asm blocks smuggling work).", file=sys.stderr)
    print(file=sys.stderr)
    print("If the original was hand-coded asm (see memory/", file=sys.stderr)
    print("feedback_hand_coded_asm_recognition.md), add the function name", file=sys.stderr)
    print("to inline_asm_canonical.txt with a justification comment, then", file=sys.stderr)
    print("re-commit.", file=sys.stderr)
    print(file=sys.stderr)
    print("If it's canonically C (~99% of the codebase), write a real C", file=sys.stderr)
    print("decomp instead of wrapping the asm.", file=sys.stderr)
    return 1


def cmd_summary():
    """One-line counts of unauthorized cheats. For session briefing."""
    auth = load_authorized()
    cur_splice, cur_inline, cur_bios, cur_c_body = get_current_cheats(auth)
    n_splice = len(cur_splice)
    n_inline = len(cur_inline)
    n_bios = len(cur_bios)
    n_c_body = len(cur_c_body)
    total = n_splice + n_inline + n_c_body
    if total == 0 and n_bios == 0:
        print("Cheats:   none — all inline asm and large splices are authorized")
    else:
        parts = []
        if n_splice:
            parts.append(f"{n_splice} splice")
        if n_inline:
            parts.append(f"{n_inline} inline-asm")
        if n_c_body:
            parts.append(f"{n_c_body} c-body-multi-insn")
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
    cur_splice, cur_inline, cur_bios, cur_c_body = get_current_cheats(auth)
    for f in sorted(cur_splice | cur_inline | cur_c_body):
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
