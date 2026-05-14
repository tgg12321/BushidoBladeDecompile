#!/usr/bin/env python3
"""
Per-function register fixup and instruction reorder pass for the BB2
decompilation build pipeline.

Sits between maspsx and as in the build pipeline:
  cpp | cc1 | prologue_fix | maspsx | regfix | as

Handles cases where GCC 2.7.2's register allocator or instruction
scheduler differs from the original compiler, but code is otherwise
structurally identical.

Config file format (regfix.txt):
  # Function-wide register swap:
  func_800806A4: $10 <-> $11

  # Range-based register swap (instruction indices, 0-based, inclusive):
  PutShadowRmd: $2 <-> $3 @ 1-7

  # Instruction reorder (move instruction at index A to index B):
  cpu_get_dist: reorder 8,10,6,7,9,11,12 @ 8-14

  The reorder directive specifies the new order of instruction indices
  within the given range. The list must contain exactly the indices
  in the range, rearranged.

  # Instruction insert (add a new instruction before index N):
  func_8001EEB4: insert "andi $3,$5,0xFFFF" @ 18

  Inserts the given assembly text as a new instruction before the
  instruction at the specified index. All subsequent indices shift +1.

  # Text substitution at a specific instruction index:
  func_80086BFC: subst "addiu \\$5," "addiu \\$7," @ 21

  Applies a regex substitution on the instruction at the specified index.
  Only the first match is replaced. Useful for field-specific register
  changes that bidirectional swaps cannot handle (e.g., changing only
  the destination register when source uses the same register number).

  # 1-to-N regex substitution (replace 1 maspsx line with K output lines):
  func_8002D320: subst_multi "blez\\s+\\$2,\\.L273" "slt\\t$2,$6,$3" "beq\\t$2,$0,.L273" @ 94

  Like `subst` but expands a single matched line into MULTIPLE output
  lines. Takes a regex pattern plus 2+ replacement strings; each string
  becomes its own line in the maspsx output. Indentation from the
  original line is preserved on every output line.

  The matched line's instruction-idx stays attached to the FIRST output
  line; subsequent output lines get idx=None (synthesized, can't be
  targeted by other rules). Other rules at @ idx > N still work — they
  use ORIGINAL indices, which subst_multi doesn't perturb. insert_after
  @ N puts new lines after the WHOLE multi-line block.

  Use case: GCC emits `blez $rN, .L` (one instruction) where target
  emits `slt $rD, $rA, $rN; beq $rD, $zero, .L` (two instructions) —
  the standard MIPS expansion of `if (a < b) goto L` when `a` is not
  $zero. `subst` alone can't fix this (it's 1-to-1). Without
  `subst_multi`, every workaround leaks: subst+insert_after races with
  maspsx debug nops, source-level barriers misaligned all the carefully
  indexed surrounding rules.

  Limit: the replacement strings cannot themselves contain embedded
  newlines or unescaped double-quotes. If you need that, use the
  `splice` op below.

  # Replace a contiguous range of K maspsx lines with N output lines:
  func_8002D320: splice 92..94 "slt\\t$2,$6,$3" "beq\\t$2,$0,.L273" "nop"

  K-to-N block replacement (no regex pattern; positional). Removes
  every instruction line in the [start, end] inclusive range and emits
  the listed replacement strings in their place. Indentation taken
  from the line at `start`.

  K is determined by counting maspsx-output lines whose idx falls in
  [start, end]; N is the number of quoted replacement strings. K and
  N may differ. Subsequent rules' @ indices reference ORIGINAL pre-
  splice positions; splice doesn't shift other rules.

  Use case: a recognizable multi-line GCC idiom (e.g. mine's
  `li $tN, 1; addu $vM, $tN, $0; addu $vM, $tN, $0`) needs to be
  collapsed into a different shape (`addiu $vM, $0, 1`). 3-to-1
  reductions are awkward with delete+subst chains because of how
  delete renumbers — splice handles the whole block atomically.

  # Instruction delete (remove the instruction at index N):
  func_80017FA0: delete @ 8

  Removes the instruction at the specified index. All subsequent
  instruction indices shift -1. Use for extra instructions GCC generates
  that are not in the target (e.g., ori in a lui+ori+sw(0) pattern
  when target uses lui+sw(offset)).

  # Insert AFTER a specific instruction index:
  func_80019310: insert_after "addu $12,$2,$0" @ 22

  Like insert, but places the new instruction immediately AFTER the
  instruction at the specified index (before any subsequent .word
  directives or non-instruction lines). Useful when .word directives
  follow the target instruction and a regular insert at the next
  index would land after those .word lines.

  # Insert a LABEL after an instruction (no instruction-index shift):
  func_8007C97C: insert_label ".LC97C:" @ 3

  Like insert_after, but for labels. The label is inserted right after
  the instruction at the specified index, but does NOT take an
  instruction index — subsequent rules using the same indices continue
  to refer to the same instructions. Use this for synthesizing branch
  targets (e.g., the `.LC97C` label in the early-exit alias-breaker
  recipe). The label name should end with `:`.

  # Fill a jal/branch delay slot with an existing instruction:
  func_80088740: fill_delay @ 22 <- 19

  Reads the instruction text at <source_idx>, replaces the nop at
  <jal_idx>+1 with that text, and removes the original instruction
  at <source_idx>. The jal_idx must be a branch/jal whose immediate
  next instruction is currently a nop. Source_idx must be a single
  instruction. Runs after swaps/substs (so register renames flow
  through), and feeds the source removal into the delete phase
  (which handles renumbering). Used when GCC emits store-then-jal-
  then-nop but target schedules store into the jal's delay slot.

  # Drain a jal/branch delay slot, moving its instruction before the jal:
  func_80088740: drain_delay @ 16

  Inverse of fill_delay. Reads the instruction at <jal_idx>+1
  (which must NOT be a nop), inserts a copy immediately before
  the jal at <jal_idx>, and replaces the original delay slot with
  a nop. Used when target keeps a nop in the delay slot but GCC
  scheduled an unrelated instruction there. After draining, the
  total instruction count grows by 1 (insert) and the delay slot
  becomes a no-op.

Order of operations: register swaps first (on original indices),
then substitutions (on original indices), then subst_multi (on
original indices; produces multi-line output but doesn't reshape
the tuple list), then splice (on original indices; replaces a
range with K replacement lines in one block), then fill_delay
(reads post-subst source, replaces nop, queues source deletion),
then deletes (on original indices), then inserts (shifts indices),
then insert_afters (shifts indices), then insert_labels (no index
shift), then reorders (on post-insert indices).
Instruction indices count actual instructions (not directives, labels,
or comments) from the function entry point, 0-based.
"""
import os
import re
import sys
from pathlib import Path


def load_config(config_path):
    """Load config. Returns {func_name: {'swaps': [...], 'reorders': [], 'inserts': [], 'substs': []}}"""
    config = {}
    if not config_path.exists():
        return config
    for line in config_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue

        # Parse delete: func_name: delete @ index
        m = re.match(r'(\w+)\s*:\s*delete\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            idx = int(m.group(2))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['deletes'].append(idx)
            continue

        # Parse fill_delay: func_name: fill_delay @ jal_idx <- source_idx
        m = re.match(r'(\w+)\s*:\s*fill_delay\s*@\s*(\d+)\s*<-\s*(\d+)', line)
        if m:
            func = m.group(1)
            jal_idx = int(m.group(2))
            source_idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['fill_delays'].append((jal_idx, source_idx))
            continue

        # Parse drain_delay: func_name: drain_delay @ jal_idx
        m = re.match(r'(\w+)\s*:\s*drain_delay\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            jal_idx = int(m.group(2))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['drain_delays'].append(jal_idx)
            continue

        # Parse reorder: func_name: reorder i,j,k,... @ start-end
        m = re.match(r'(\w+)\s*:\s*reorder\s+([\d,]+)\s*@\s*(\d+)\s*-\s*(\d+)', line)
        if m:
            func = m.group(1)
            order = [int(x) for x in m.group(2).split(',')]
            start = int(m.group(3))
            end = int(m.group(4))
            # Warn when reorder spans idx 0: the function symbol label, .ent,
            # .frame, .mask, .fmask are all attached as preceding non-instruction
            # lines to the first instruction in the function. Reordering moves
            # them with the group, which can place the function symbol mid-body.
            # See memory/feedback_regfix_label_attachment.md.
            if start == 0 and order and order[0] != 0:
                print(
                    f"regfix: WARNING: {func}: reorder spans idx 0 and idx 0 is "
                    f"NOT first in the new order (new order: {order}). The function "
                    f"symbol label will move with idx 0. Use subst-swap to swap "
                    f"instruction CONTENT in place instead. See memory/"
                    f"feedback_regfix_label_attachment.md.",
                    file=sys.stderr
                )
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['reorders'].append((start, end, order))
            continue

        # Parse insert: func_name: insert "asm_text" @ index
        m = re.match(r'(\w+)\s*:\s*insert\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            asm_text = m.group(2).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(3))
            # Warn on hardcoded `.L<digits>` references — GCC's auto-labels
            # are file-wide and shift when other functions are added or
            # removed. Use insert_label to synthesize a stable label and
            # reference it from the insert. See memory/
            # feedback_label_renumber_breaks_regfix.md.
            if re.search(r'\.L\d+\b', asm_text):
                lbl = re.search(r'\.L\d+\b', asm_text).group(0)
                print(
                    f"regfix: WARNING: {func}: insert references GCC auto-label "
                    f"{lbl!r}: {asm_text!r}. This label is file-wide and will "
                    f"shift if other functions are added/removed. Synthesize a "
                    f"stable label with insert_label and use that name "
                    f"instead. See memory/feedback_label_renumber_breaks_regfix.md.",
                    file=sys.stderr
                )
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['inserts'].append((idx, asm_text))
            continue

        # Parse insert_after: func_name: insert_after "asm_text" @ index
        m = re.match(r'(\w+)\s*:\s*insert_after\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            asm_text = m.group(2).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['insert_afters'].append((idx, asm_text))
            continue

        # Parse insert_label: func_name: insert_label "label_text" @ index
        # Like insert_after but for labels — does NOT take an instruction
        # index, so subsequent rules' indices remain unaffected.
        m = re.match(r'(\w+)\s*:\s*insert_label\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            label_text = m.group(2).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['insert_labels'].append((idx, label_text))
            continue

        # Parse subst: func_name: subst "pattern" "replacement" @ index
        m = re.match(r'(\w+)\s*:\s*subst\s+"([^"]+)"\s+"([^"]*)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            pattern = m.group(2)
            replacement = m.group(3).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(4))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['substs'].append((idx, pattern, replacement))
            continue

        # Parse subst_multi: func_name: subst_multi "pattern" "line1" "line2" [...] @ index
        # 1-to-N substitution: replace the matched line with multiple output
        # lines. The first quoted arg is the regex pattern; the remaining
        # quoted args are the output lines (in order). At least one output
        # line is required (with one, this is equivalent to `subst`).
        m = re.match(r'(\w+)\s*:\s*subst_multi\s+(.+?)\s*@\s*(\d+)\s*$', line)
        if m:
            func = m.group(1)
            args_part = m.group(2)
            idx = int(m.group(3))
            # Extract all "..." quoted strings from args_part. Allow escaped
            # quotes via \" though they're rare in regfix patterns.
            quoted = re.findall(r'"((?:[^"\\]|\\.)*)"', args_part)
            if len(quoted) < 2:
                print(
                    f"regfix: ERROR: {func}: subst_multi @ {idx} needs at least "
                    f"a pattern and ONE replacement string; got {len(quoted)} "
                    f"quoted args. Skipping.",
                    file=sys.stderr,
                )
                continue
            pattern = quoted[0]
            replacements = [
                s.replace('\\n', '\n').replace('\\t', '\t')
                for s in quoted[1:]
            ]
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['subst_multis'].append((idx, pattern, replacements))
            continue

        # Parse splice: func_name: splice start..end "line1" "line2" [...]
        # K-to-N range replacement (no regex). Removes all maspsx lines in
        # [start, end] inclusive and emits the listed replacement strings
        # in their place.
        m = re.match(r'(\w+)\s*:\s*splice\s+(\d+)\.\.(\d+)\s+(.+?)\s*$', line)
        if m:
            func = m.group(1)
            start = int(m.group(2))
            end = int(m.group(3))
            args_part = m.group(4)
            if start > end:
                print(
                    f"regfix: ERROR: {func}: splice {start}..{end} has start > end. "
                    f"Skipping.",
                    file=sys.stderr,
                )
                continue
            quoted = re.findall(r'"((?:[^"\\]|\\.)*)"', args_part)
            if not quoted:
                print(
                    f"regfix: ERROR: {func}: splice {start}..{end} needs at least "
                    f"one replacement string. Skipping.",
                    file=sys.stderr,
                )
                continue
            replacements = [
                s.replace('\\n', '\n').replace('\\t', '\t')
                for s in quoted
            ]
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['splices'].append((start, end, replacements))
            continue

        # Parse register swap: func_name: $X <-> $Y [@ start-end]
        m = re.match(r'(\w+)\s*:\s*(\$\w+)\s*<->\s*(\$\w+)(?:\s*@\s*(\d+)\s*-\s*(\d+))?', line)
        if m:
            func = m.group(1)
            reg_a = m.group(2)
            reg_b = m.group(3)
            start = int(m.group(4)) if m.group(4) is not None else None
            end = int(m.group(5)) if m.group(5) is not None else None
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'insert_labels': [], 'substs': [], 'subst_multis': [], 'splices': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['swaps'].append((reg_a, reg_b, start, end))
            continue

        print(f"regfix: WARNING: ignoring malformed line: {line}", file=sys.stderr)
    return config


def is_instruction(line):
    """Check if a line is an actual instruction (not directive/label/comment/blank)."""
    s = line.strip()
    if not s:
        return False
    if s.startswith('.') or s.startswith('#') or s.endswith(':'):
        return False
    if any(s.startswith(d) for d in ('gcc2_compiled',)):
        return False
    return True


def swap_registers_in_line(line, swap_list, insn_idx):
    """Apply register swaps to a single assembly line."""
    for reg_a, reg_b, start, end in swap_list:
        if start is not None and (insn_idx < start or insn_idx > end):
            continue
        placeholder = f"__REGFIX_{id(swap_list)}_{reg_a}__"
        ra = re.escape(reg_a)
        rb = re.escape(reg_b)
        line = re.sub(r'(?<!\w)' + ra + r'(?!\d)', placeholder, line)
        line = re.sub(r'(?<!\w)' + rb + r'(?!\d)', reg_a, line)
        line = line.replace(placeholder, reg_b)
    return line


def _dump_lines(lines, label, func_name=None):
    """Debug: dump (position, idx, text) when REGFIX_DEBUG_DUMP env var matches func name."""
    target = os.environ.get('REGFIX_DEBUG_DUMP', '')
    if not target or (func_name and target != func_name):
        return
    range_env = os.environ.get('REGFIX_DEBUG_RANGE', '')
    rng = None
    if range_env:
        m = re.match(r'(\d+)-(\d+)', range_env)
        if m:
            rng = (int(m.group(1)), int(m.group(2)))
    print(f"\n# === REGFIX DEBUG: {label} ===", file=sys.stderr)
    pos = 0
    for text, idx in lines:
        text_strip = text.rstrip()
        if idx is not None:
            if rng is None or rng[0] <= pos <= rng[1]:
                print(f"#   pos {pos:4d} | idx {idx:4d} | {text_strip}", file=sys.stderr)
            pos += 1
        else:
            if rng is None or (pos > 0 and rng[0] <= pos - 1 <= rng[1]):
                print(f"#   pos  --- | idx  --- | {text_strip}", file=sys.stderr)


def process_function(lines, func_config):
    """Process a collected function's lines: apply swaps, inserts, then reorders.

    lines is a list of (original_line_text, insn_idx_or_None) tuples.
    """
    swap_list = func_config.get('swaps', [])
    subst_list = func_config.get('substs', [])
    insert_list = func_config.get('inserts', [])
    reorder_list = func_config.get('reorders', [])
    fname = func_config.get('__name__')
    _dump_lines(lines, "Phase 0: initial (post-maspsx)", fname)

    # Phase 1: Apply register swaps (on original indices)
    if swap_list:
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                text = swap_registers_in_line(text, swap_list, idx)
            new_lines.append((text, idx))
        lines = new_lines

    # Phase 1.5: Apply text substitutions (on original indices, after swaps).
    # Track which subst rules fired so we can warn on silently-dropped patterns
    # (e.g. literal `.L2` written when build has `.L40` — see
    # memory/feedback_regfix_gcc_label_regex.md).
    if subst_list:
        # Map (subst_idx, pattern, replacement) -> "did it fire?"
        # We key on (idx, pattern) and track:
        #   - "visited": True if some line at this idx existed
        #   - "matched": True if re.sub actually changed the text
        subst_status = {(s_idx, pat): {'visited': False, 'matched': False, 'replacement': repl}
                        for s_idx, pat, repl in subst_list}
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                for subst_idx, pattern, replacement in subst_list:
                    if idx == subst_idx:
                        subst_status[(subst_idx, pattern)]['visited'] = True
                        new_text = re.sub(pattern, lambda m: replacement, text, count=1)
                        if new_text != text:
                            subst_status[(subst_idx, pattern)]['matched'] = True
                            text = new_text
            new_lines.append((text, idx))
        lines = new_lines

        # Warn on rules that visited their idx but didn't match. A rule that
        # didn't visit (i.e., the idx doesn't exist in this function's pipeline
        # output) is reported separately below.
        func_name = func_config.get('__name__', '<unknown>')
        for (s_idx, pat), status in subst_status.items():
            if status['visited'] and not status['matched']:
                print(
                    f"regfix: WARNING: {func_name}: subst pattern did not match "
                    f"at idx {s_idx}: pattern={pat!r}. Rule was silently ignored. "
                    f"Common cause: hardcoded `.L<N>` label that differs between "
                    f"isolated permuter and full-file build (use `\\.L\\d+` regex). "
                    f"See memory/feedback_regfix_gcc_label_regex.md.",
                    file=sys.stderr
                )
            elif not status['visited']:
                print(
                    f"regfix: WARNING: {func_name}: subst @ idx {s_idx} not "
                    f"found in function (max valid idx may be lower). "
                    f"pattern={pat!r}. Rule was silently ignored.",
                    file=sys.stderr
                )

    # Phase 1.55: Apply subst_multi (1-to-N substitution).
    #
    # For each matching tuple at idx N, splits the replacement strings into
    # K separate (text, idx) entries. The first entry keeps idx=N so other
    # rules can still address it; subsequent entries get idx=None (they're
    # synthesized, can't be a regfix target). insert_after @ N continues to
    # find idx N at the FIRST entry of the block — to insert AFTER the
    # whole block, use insert_after @ (next original idx) instead.
    #
    # Indentation is preserved from the original line. The replacement
    # strings themselves can include leading whitespace; whitespace is
    # left alone (use \t in the rule for explicit indents).
    subst_multi_list = func_config.get('subst_multis', [])
    if subst_multi_list:
        sm_status = {
            (sm_idx, pat): {'visited': False, 'matched': False}
            for sm_idx, pat, _repls in subst_multi_list
        }
        new_lines = []
        for text, idx in lines:
            consumed = False
            if idx is not None:
                for sm_idx, pattern, replacements in subst_multi_list:
                    if idx != sm_idx:
                        continue
                    sm_status[(sm_idx, pattern)]['visited'] = True
                    try:
                        if not re.search(pattern, text):
                            continue
                    except re.error as e:
                        print(
                            f"regfix: WARNING: {fname}: subst_multi @ {sm_idx} "
                            f"invalid regex {pattern!r}: {e}",
                            file=sys.stderr,
                        )
                        continue
                    sm_status[(sm_idx, pattern)]['matched'] = True
                    # Capture indentation from original line. Trailing
                    # newline (if any) is preserved on the LAST emitted
                    # line only.
                    m_indent = re.match(r'^(\s*)', text)
                    indent = m_indent.group(1) if m_indent else '\t'
                    trailing_newline = '\n' if text.endswith('\n') else ''
                    n_repls = len(replacements)
                    for i, repl in enumerate(replacements):
                        # Strip user-provided leading whitespace; we own it.
                        body = repl.lstrip()
                        # The first entry inherits the matched line's idx;
                        # subsequent are synthesized (idx None).
                        out_idx = sm_idx if i == 0 else None
                        suffix = trailing_newline if i == n_repls - 1 else '\n'
                        new_lines.append((f"{indent}{body}{suffix}", out_idx))
                    consumed = True
                    break
            if not consumed:
                new_lines.append((text, idx))
        lines = new_lines

        # Warn on rules that visited their idx but didn't match
        for (s_idx, pat), status in sm_status.items():
            if status['visited'] and not status['matched']:
                print(
                    f"regfix: WARNING: {fname}: subst_multi pattern did not match "
                    f"at idx {s_idx}: pattern={pat!r}. Rule was silently ignored.",
                    file=sys.stderr,
                )
            elif not status['visited']:
                print(
                    f"regfix: WARNING: {fname}: subst_multi @ idx {s_idx} not "
                    f"found in function output. pattern={pat!r}.",
                    file=sys.stderr,
                )

    _dump_lines(lines, "Phase 1.55: after subst_multi", fname)

    # Phase 1.65: Apply splice (K-to-N range replacement).
    #
    # Removes every instruction-bearing line whose idx falls in [start, end]
    # and emits the listed replacement strings in their place. Non-
    # instruction lines (labels, .set directives, comments) within the
    # range are PRESERVED in their original positions — splice only
    # touches lines with a non-None idx.
    #
    # The first replacement line takes the start idx. Subsequent
    # replacement lines get idx=None (synthesized). All other lines in
    # (start, end] are removed entirely (no idx left behind).
    splice_list = func_config.get('splices', [])
    if splice_list:
        for sp_start, sp_end, replacements in splice_list:
            # Collect the positions of instruction lines in [start, end]
            in_range_positions = []
            for pos, (_text, idx) in enumerate(lines):
                if idx is not None and sp_start <= idx <= sp_end:
                    in_range_positions.append((pos, idx))

            if not in_range_positions:
                print(
                    f"regfix: WARNING: {fname}: splice {sp_start}..{sp_end} "
                    f"found no instructions in range. Skipping.",
                    file=sys.stderr,
                )
                continue

            # Indent inherited from the line at start (or first matching line).
            first_pos = in_range_positions[0][0]
            first_text = lines[first_pos][0]
            m_indent = re.match(r'^(\s*)', first_text)
            indent = m_indent.group(1) if m_indent else '\t'
            trailing_newline = '\n' if first_text.endswith('\n') else '\n'

            # Build replacement tuples.
            n_repls = len(replacements)
            replacement_tuples = []
            for i, repl in enumerate(replacements):
                body = repl.lstrip()
                out_idx = sp_start if i == 0 else None
                replacement_tuples.append((f"{indent}{body}{trailing_newline}", out_idx))

            # Replace lines in-range: first matching pos gets the replacement
            # block; the rest are dropped.
            positions_to_drop = {p for p, _ in in_range_positions[1:]}
            new_lines = []
            inserted_block = False
            for pos, entry in enumerate(lines):
                if pos == first_pos:
                    new_lines.extend(replacement_tuples)
                    inserted_block = True
                elif pos in positions_to_drop:
                    continue
                else:
                    new_lines.append(entry)
            if not inserted_block:
                # Should be unreachable given the guard above, but defend.
                new_lines = replacement_tuples + new_lines
            lines = new_lines

    _dump_lines(lines, "Phase 1.65: after splice", fname)

    # Phase 1.7: Apply fill_delay (read source post-swap/subst, replace nop, queue source for deletion)
    fill_delay_list = func_config.get('fill_delays', [])
    extra_deletes = []
    for jal_idx, source_idx in fill_delay_list:
        # Find source instruction (post-swap/subst text)
        source_text = None
        for text, idx in lines:
            if idx == source_idx:
                source_text = text
                break
        if source_text is None:
            print(f"regfix: WARNING: fill_delay source index {source_idx} not found", file=sys.stderr)
            continue

        # Find delay slot (the next instruction after jal_idx — should be jal_idx+1)
        nop_pos = None
        nop_text = None
        for pos, (text, idx) in enumerate(lines):
            if idx == jal_idx + 1:
                nop_pos = pos
                nop_text = text
                break
        if nop_pos is None:
            print(f"regfix: WARNING: fill_delay delay-slot index {jal_idx + 1} not found", file=sys.stderr)
            continue
        # Strip inline comments (maspsx emits "nop # DEBUG: branch/jump" for branch/jump delay slots)
        nop_body = re.sub(r'\s*#.*$', '', nop_text).strip()
        if nop_body != 'nop':
            print(f"regfix: WARNING: fill_delay expected nop at index {jal_idx + 1}, got: {nop_text.strip()!r}", file=sys.stderr)
            continue

        # Preserve indentation from the original nop line; copy body from source text
        # Strip the source line down to just the instruction body (no surrounding whitespace/newline)
        source_body = source_text.strip()
        # Match indentation of the nop line (everything before 'nop')
        m_indent = re.match(r'^(\s*)', nop_text)
        indent = m_indent.group(1) if m_indent else '\t'
        # Preserve trailing newline from nop_text
        trailing = '\n' if nop_text.endswith('\n') else ''
        new_text = f"{indent}{source_body}{trailing}"
        lines[nop_pos] = (new_text, jal_idx + 1)

        # Queue the source for deletion (handled in Phase 2)
        extra_deletes.append(source_idx)

    # Phase 1.8: Apply drain_delay (move delay-slot insn before jal, replace slot with nop)
    # We do this by mutating the delay-slot line text in place AND queuing an insert.
    drain_delay_list = func_config.get('drain_delays', [])
    extra_inserts = []
    for jal_idx in drain_delay_list:
        # Find delay slot (jal_idx + 1)
        ds_pos = None
        ds_text = None
        for pos, (text, idx) in enumerate(lines):
            if idx == jal_idx + 1:
                ds_pos = pos
                ds_text = text
                break
        if ds_pos is None:
            print(f"regfix: WARNING: drain_delay delay-slot index {jal_idx + 1} not found", file=sys.stderr)
            continue
        if ds_text.strip() == 'nop':
            print(f"regfix: WARNING: drain_delay at jal {jal_idx}: delay slot is already nop", file=sys.stderr)
            continue

        # Capture body, build a copy to insert before the jal
        ds_body = ds_text.strip()
        # Replace the delay slot with nop, preserving indentation
        m_indent = re.match(r'^(\s*)', ds_text)
        indent = m_indent.group(1) if m_indent else '\t'
        trailing = '\n' if ds_text.endswith('\n') else ''
        lines[ds_pos] = (f"{indent}nop{trailing}", jal_idx + 1)

        # Queue an insert of the original delay-slot body before jal_idx
        extra_inserts.append((jal_idx, ds_body))

    # Phase 2: Apply instruction deletes (on original indices, sorted descending)
    delete_list = func_config.get('deletes', []) + extra_deletes
    if delete_list:
        for del_idx in sorted(delete_list, reverse=True):
            # Find and remove the line with this instruction index
            del_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == del_idx:
                    del_pos = pos
                    break
            if del_pos is None:
                print(f"regfix: WARNING: delete index {del_idx} not found", file=sys.stderr)
                continue
            lines.pop(del_pos)
            # Renumber: shift all instruction indices > del_idx by -1
            renumbered = []
            for text, idx in lines:
                if idx is not None and idx > del_idx:
                    renumbered.append((text, idx - 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered

    _dump_lines(lines, "Phase 2: after deletes", fname)

    # Merge in any inserts queued by drain_delay (before jal_idx, with jal_idx as the insert point)
    insert_list = insert_list + extra_inserts

    # Phase 3: Apply instruction inserts (sorted by index descending to preserve positions)
    if insert_list:
        for insert_idx, asm_text in sorted(insert_list, key=lambda x: x[0], reverse=True):
            # Find the position in lines where insn_idx == insert_idx
            insert_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    insert_pos = pos
                    break
            if insert_pos is None:
                print(f"regfix: WARNING: insert index {insert_idx} not found", file=sys.stderr)
                continue
            # Insert the new instruction line before this position
            # Wrap in .set noat/.set at if the instruction uses $at ($1)
            if re.search(r'\$1(?!\d)', asm_text) or '$at' in asm_text:
                new_line = f".set\tnoat\n\t{asm_text}\n.set\tat\n"
            else:
                new_line = f"\t{asm_text}\n"
            lines.insert(insert_pos, (new_line, insert_idx))
            # Renumber: shift all instruction indices >= insert_idx by +1
            renumbered = []
            for text, idx in lines:
                if idx is not None and idx >= insert_idx and (text, idx) != lines[insert_pos]:
                    renumbered.append((text, idx + 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered

    _dump_lines(lines, "Phase 3: after inserts", fname)

    # Phase 3.5: Apply insert_after (sorted by index descending to preserve positions)
    insert_after_list = func_config.get('insert_afters', [])
    if insert_after_list:
        for insert_idx, asm_text in sorted(insert_after_list, key=lambda x: x[0], reverse=True):
            # Find the position in lines where insn_idx == insert_idx
            target_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    target_pos = pos
                    break
            if target_pos is None:
                print(f"regfix: WARNING: insert_after index {insert_idx} not found", file=sys.stderr)
                continue
            # Insert right after the target instruction (pos + 1)
            insert_pos = target_pos + 1
            if re.search(r'\$1(?!\d)', asm_text) or '$at' in asm_text:
                new_line = f".set\tnoat\n\t{asm_text}\n.set\tat\n"
            else:
                new_line = f"\t{asm_text}\n"
            new_idx = insert_idx + 1
            lines.insert(insert_pos, (new_line, new_idx))
            # Renumber: shift all instruction indices > insert_idx by +1
            renumbered = []
            for text, idx in lines:
                if idx is not None and idx > insert_idx and (text, idx) != lines[insert_pos]:
                    renumbered.append((text, idx + 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered

    _dump_lines(lines, "Phase 3.5: after insert_afters", fname)

    # Phase 3.6: Apply insert_label (sorted by index descending; no index shift)
    insert_label_list = func_config.get('insert_labels', [])
    if insert_label_list:
        for insert_idx, label_text in sorted(insert_label_list, key=lambda x: x[0], reverse=True):
            # Find the position in lines where insn_idx == insert_idx
            target_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    target_pos = pos
                    break
            if target_pos is None:
                print(f"regfix: WARNING: insert_label index {insert_idx} not found", file=sys.stderr)
                continue
            # Insert right after the target instruction. Label gets idx=None
            # so it doesn't participate in instruction-index renumbering.
            insert_pos = target_pos + 1
            new_line = f"{label_text}\n"
            lines.insert(insert_pos, (new_line, None))

    _dump_lines(lines, "Phase 3.6: after insert_labels (PRE-REORDER state — use these IDXs)", fname)

    # Phase 4: Apply instruction reorders (on post-insert indices)
    # Each instruction carries any preceding non-instruction lines (labels, .set, comments)
    for ri, (reorder_start, reorder_end, new_order) in enumerate(reorder_list):
        # Build map of insn_idx -> line position
        idx_to_pos = {}
        for pos, (text, idx) in enumerate(lines):
            if idx is not None and reorder_start <= idx <= reorder_end:
                idx_to_pos[idx] = pos

        # Validate
        expected = set(range(reorder_start, reorder_end + 1))
        if set(new_order) != expected:
            print(f"regfix: WARNING: reorder indices {new_order} don't match range {reorder_start}-{reorder_end}", file=sys.stderr)
            continue

        positions = sorted(idx_to_pos.values())
        if not positions:
            continue

        # Build groups: each instruction + preceding non-insn lines
        # Group boundaries: from (prev_insn_pos + 1) to (this_insn_pos), inclusive
        #
        # SPECIAL CASE: when reorder_start == 0, the preceding non-insn lines
        # of idx 0 contain the function-label preamble (`func_NAME:`, `.ent`,
        # `.frame`, `.mask`, `.fmask`, `.set noreorder`). If we attach these
        # to idx 0's group, they move with idx 0 in the reorder, which places
        # the function symbol mid-body and breaks link addresses.
        # See memory/feedback_regfix_label_attachment.md.
        #
        # Fix: detect reorder_start == 0, extract the preamble separately,
        # and emit it FIRST in the reordered section regardless of where
        # idx 0 lands. Idx 0's group then contains only the instruction line.
        preamble_lines = []
        groups = {}
        for i, insn_pos in enumerate(positions):
            if i == 0:
                group_start = insn_pos
                # Also grab preceding non-insn lines up to previous instruction
                while group_start > 0 and lines[group_start - 1][1] is None:
                    group_start -= 1
                    # Don't go past the position before the first instruction in range
                    if group_start < positions[0]:
                        # Only include non-insn lines that are "close" (within the conceptual range)
                        # Check if this non-insn line is after the previous instruction outside the range
                        found_prev_insn = False
                        for check in range(group_start - 1, -1, -1):
                            if lines[check][1] is not None:
                                found_prev_insn = True
                                break
                        if found_prev_insn:
                            break

                if reorder_start == 0:
                    # Preamble = non-instruction lines preceding idx 0.
                    # Idx 0's group should contain ONLY the instruction line
                    # so the preamble doesn't follow idx 0 around the reorder.
                    preamble_lines = [lines[p] for p in range(group_start, insn_pos)]
                    group_start = insn_pos
            else:
                group_start = positions[i - 1] + 1

            insn_idx_val = lines[insn_pos][1]
            groups[insn_idx_val] = [lines[p] for p in range(group_start, insn_pos + 1)]

        # Determine the full span of lines being replaced
        # When reorder_start == 0, the preamble lives between the first
        # in-range instruction's group_start (computed above) and the
        # actual instruction position. We need span_start to cover those
        # preamble lines too so the output replaces them once and the
        # preamble emitted explicitly in `reordered_section` takes their place.
        if reorder_start == 0 and preamble_lines:
            # First preamble line position is the original group_start for idx 0.
            # Walk back from positions[0] over None-idx lines to find it.
            span_start = positions[0]
            while span_start > 0 and lines[span_start - 1][1] is None:
                span_start -= 1
                if span_start < positions[0]:
                    found_prev_insn = False
                    for check in range(span_start - 1, -1, -1):
                        if lines[check][1] is not None:
                            found_prev_insn = True
                            break
                    if found_prev_insn:
                        break
        else:
            first_group = groups[lines[positions[0]][1]]
            span_start = None
            for pos in range(len(lines)):
                if lines[pos] is first_group[0]:
                    span_start = pos
                    break
            if span_start is None:
                span_start = positions[0]
        span_end = positions[-1] + 1  # exclusive

        # Build reordered section. Preamble (if any) comes first to keep
        # the function label attached to whatever instruction now sits at
        # the original idx 0 position.
        reordered_section = list(preamble_lines)
        for new_idx in new_order:
            reordered_section.extend(groups[new_idx])

        # Replace the span
        lines = lines[:span_start] + reordered_section + lines[span_end:]

        _dump_lines(lines, f"Phase 4.{ri}: after reorder #{ri} (range {reorder_start}-{reorder_end} -> {new_order})", fname)

    _dump_lines(lines, "Phase FINAL", fname)

    return lines


def process_asm_text(asm_text, config):
    """Apply regfix config to a full asm text blob and return the rewritten text."""
    current_func = None
    current_config = None
    func_lines = []
    insn_idx = 0
    buffering = False
    output = []

    all_input = asm_text.splitlines(keepends=True)
    i = 0
    while i < len(all_input):
        line = all_input[i]
        stripped = line.strip()

        label_match = re.match(r'^(\w+):$', stripped)
        if label_match and not buffering:
            func_name = label_match.group(1)
            if func_name in config:
                current_func = func_name
                current_config = config[func_name]
                func_lines = [(line, None)]
                insn_idx = 0
                buffering = True
                i += 1
                continue

        if buffering:
            end_match = re.match(r'^\s*\.end\s+(\w+)', stripped)

            if is_instruction(stripped):
                func_lines.append((line, insn_idx))
                insn_idx += 1
            else:
                func_lines.append((line, None))

            if end_match and end_match.group(1) == current_func:
                # Pass function name into config so warnings can identify it
                config_with_name = dict(current_config)
                config_with_name['__name__'] = current_func
                processed = process_function(func_lines, config_with_name)
                output.extend(text for text, _ in processed)
                buffering = False
                current_func = None
                current_config = None
                func_lines = []
        else:
            output.append(line)

        i += 1

    if buffering:
        output.extend(text for text, _ in func_lines)

    return ''.join(output)


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    config_path = Path(os.environ.get('REGFIX_CONFIG', project_root / 'regfix.txt'))

    config = load_config(config_path)
    if not config:
        for line in sys.stdin:
            sys.stdout.write(line)
        return

    sys.stdout.write(process_asm_text(''.join(sys.stdin.readlines()), config))


if __name__ == '__main__':
    main()
