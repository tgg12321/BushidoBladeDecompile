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

Order of operations: register swaps first (on original indices),
then substitutions (on original indices), then inserts (shifts indices),
then reorders (on post-insert indices).
Instruction indices count actual instructions (not directives, labels,
or comments) from the function entry point, 0-based.
"""
import re
import sys
from pathlib import Path


def load_config(config_path):
    """Load config. Returns {func_name: {'swaps': [...], 'reorders': [], 'inserts': [], 'substs': []}}"""
    config = {}
    if not config_path.exists():
        return config
    for line in config_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue

        # Parse reorder: func_name: reorder i,j,k,... @ start-end
        m = re.match(r'(\w+)\s*:\s*reorder\s+([\d,]+)\s*@\s*(\d+)\s*-\s*(\d+)', line)
        if m:
            func = m.group(1)
            order = [int(x) for x in m.group(2).split(',')]
            start = int(m.group(3))
            end = int(m.group(4))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'substs': []})
            config[func]['reorders'].append((start, end, order))
            continue

        # Parse insert: func_name: insert "asm_text" @ index
        m = re.match(r'(\w+)\s*:\s*insert\s+"([^"]+)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            asm_text = m.group(2)
            idx = int(m.group(3))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'substs': []})
            config[func]['inserts'].append((idx, asm_text))
            continue

        # Parse subst: func_name: subst "pattern" "replacement" @ index
        m = re.match(r'(\w+)\s*:\s*subst\s+"([^"]+)"\s+"([^"]*)"\s*@\s*(\d+)', line)
        if m:
            func = m.group(1)
            pattern = m.group(2)
            replacement = m.group(3)
            idx = int(m.group(4))
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'substs': []})
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
            config.setdefault(func, {'swaps': [], 'reorders': [], 'inserts': [], 'substs': []})
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
                        text = re.sub(pattern, replacement, text, count=1)
            new_lines.append((text, idx))
        lines = new_lines

    # Phase 2: Apply instruction inserts (sorted by index descending to preserve positions)
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

    # Phase 3: Apply instruction reorders (on post-insert indices)
    for reorder_start, reorder_end, new_order in reorder_list:
        # Build map of insn_idx -> line index in our list
        idx_to_pos = {}
        for pos, (text, idx) in enumerate(lines):
            if idx is not None and reorder_start <= idx <= reorder_end:
                idx_to_pos[idx] = pos

        # Validate
        expected = set(range(reorder_start, reorder_end + 1))
        if set(new_order) != expected:
            print(f"regfix: WARNING: reorder indices {new_order} don't match range {reorder_start}-{reorder_end}", file=sys.stderr)
            continue

        # Collect the instruction lines in their current order
        positions = sorted(idx_to_pos.values())
        insn_lines = [lines[p] for p in positions]

        # Map new_order indices to the instruction lines
        idx_to_line = {idx: line_tuple for idx, line_tuple in zip(range(reorder_start, reorder_end + 1), insn_lines)}
        reordered = [idx_to_line[i] for i in new_order]

        # Put them back into the positions
        for pos, new_line in zip(positions, reordered):
            lines[pos] = new_line

    return lines


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    config_path = project_root / 'regfix.txt'

    config = load_config(config_path)
    if not config:
        for line in sys.stdin:
            sys.stdout.write(line)
        return

    current_func = None
    current_config = None
    func_lines = []  # collected lines for current function
    insn_idx = 0
    buffering = False

    all_input = sys.stdin.readlines()

    i = 0
    while i < len(all_input):
        line = all_input[i]
        stripped = line.strip()

        # Match function entry label
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
            # Detect end of function
            end_match = re.match(r'^\s*\.end\s+(\w+)', stripped)

            if is_instruction(stripped):
                func_lines.append((line, insn_idx))
                insn_idx += 1
            else:
                func_lines.append((line, None))

            if end_match and end_match.group(1) == current_func:
                # Process and emit the collected function
                processed = process_function(func_lines, current_config)
                for text, _ in processed:
                    sys.stdout.write(text)
                buffering = False
                current_func = None
                current_config = None
                func_lines = []
        else:
            sys.stdout.write(line)

        i += 1

    # Flush any remaining buffered lines (shouldn't happen in well-formed input)
    if buffering:
        for text, _ in func_lines:
            sys.stdout.write(text)


if __name__ == '__main__':
    main()
