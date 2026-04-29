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
then substitutions (on original indices), then fill_delay (reads
post-subst source, replaces nop, queues source deletion), then
deletes (on original indices), then inserts (shifts indices), then
insert_afters (shifts indices), then reorders (on post-insert
indices).
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
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['deletes'].append(idx)
            continue

        # Parse fill_delay: func_name: fill_delay @ jal_idx <- source_idx
        m = re.match(r'(\w+)\s*:\s*fill_delay\s*@\s*(\d+)\s*<-\s*(\d+)', line)
        if m:
            func = m.group(1)
            jal_idx = int(m.group(2))
            source_idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['fill_delays'].append((jal_idx, source_idx))
            continue

        # Parse drain_delay: func_name: drain_delay @ jal_idx
        m = re.match(r'(\w+)\s*:\s*drain_delay\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            jal_idx = int(m.group(2))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
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
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['reorders'].append((start, end, order))
            continue

        # Parse insert: func_name: insert "asm_text" @ index
        m = re.match(r'(\w+)\s*:\s*insert\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            asm_text = m.group(2).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['inserts'].append((idx, asm_text))
            continue

        # Parse insert_after: func_name: insert_after "asm_text" @ index
        m = re.match(r'(\w+)\s*:\s*insert_after\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            asm_text = m.group(2).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['insert_afters'].append((idx, asm_text))
            continue

        # Parse subst: func_name: subst "pattern" "replacement" @ index
        m = re.match(r'(\w+)\s*:\s*subst\s+"([^"]+)"\s+"([^"]*)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            pattern = m.group(2)
            replacement = m.group(3).replace('\\n', '\n').replace('\\t', '\t')
            idx = int(m.group(4))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
            config[func]['substs'].append((idx, pattern, replacement))
            continue

        # Parse register swap: func_name: $X <-> $Y [@ start-end]
        m = re.match(r'(\w+)\s*:\s*(\$\w+)\s*<->\s*(\$\w+)(?:\s*@\s*(\d+)\s*-\s*(\d+))?', line)
        if m:
            func = m.group(1)
            reg_a = m.group(2)
            reg_b = m.group(3)
            start = int(m.group(4)) if m.group(4) is not None else None
            end = int(m.group(5)) if m.group(5) is not None else None
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'insert_afters': [], 'substs': [], 'deletes': [], 'fill_delays': [], 'drain_delays': []})
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


def process_function(lines, func_config):
    """Process a collected function's lines: apply swaps, inserts, then reorders.

    lines is a list of (original_line_text, insn_idx_or_None) tuples.
    """
    swap_list = func_config.get('swaps', [])
    subst_list = func_config.get('substs', [])
    insert_list = func_config.get('inserts', [])
    reorder_list = func_config.get('reorders', [])

    # Phase 1: Apply register swaps (on original indices)
    if swap_list:
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                text = swap_registers_in_line(text, swap_list, idx)
            new_lines.append((text, idx))
        lines = new_lines

    # Phase 1.5: Apply text substitutions (on original indices, after swaps)
    if subst_list:
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                for subst_idx, pattern, replacement in subst_list:
                    if idx == subst_idx:
                        # Use lambda to avoid backreference interpretation of $N in replacement
                        text = re.sub(pattern, lambda m: replacement, text, count=1)
            new_lines.append((text, idx))
        lines = new_lines

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

    # Phase 4: Apply instruction reorders (on post-insert indices)
    # Each instruction carries any preceding non-instruction lines (labels, .set, comments)
    for reorder_start, reorder_end, new_order in reorder_list:
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
            else:
                group_start = positions[i - 1] + 1

            insn_idx_val = lines[insn_pos][1]
            groups[insn_idx_val] = [lines[p] for p in range(group_start, insn_pos + 1)]

        # Determine the full span of lines being replaced
        first_group = groups[lines[positions[0]][1]]
        span_start = None
        for pos in range(len(lines)):
            if lines[pos] is first_group[0]:
                span_start = pos
                break
        if span_start is None:
            span_start = positions[0]
        span_end = positions[-1] + 1  # exclusive

        # Build reordered section
        reordered_section = []
        for new_idx in new_order:
            reordered_section.extend(groups[new_idx])

        # Replace the span
        lines = lines[:span_start] + reordered_section + lines[span_end:]

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
                processed = process_function(func_lines, current_config)
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
