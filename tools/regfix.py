#!/usr/bin/env python3
"""
Per-function register fixup pass for the BB2 decompilation build pipeline.

Sits between maspsx and as in the build pipeline:
  cpp | cc1 | prologue_fix | maspsx | regfix | as

Reads assembly from stdin, and for functions listed in a config file,
replaces specific register names. This handles cases where GCC 2.7.2's
register allocator picks a different temp register than the original
compiler, but the code is otherwise identical.

Config file format (regfix.txt):
  # Comments and blank lines are ignored

  # Function-wide swap:
  func_800806A4: $10 <-> $11

  # Range-based swap (instruction indices, 0-based, inclusive):
  PutShadowRmd: $2 <-> $3 @ 1-7
  PutShadowRmd: $2 <-> $3 @ 18-27

Instruction indices count actual instructions (not directives, labels,
or comments) from the function entry point, 0-based.
"""
import re
import sys
from pathlib import Path


def load_config(config_path):
    """Load register swap config.

    Returns {func_name: [(reg_a, reg_b, start, end), ...]}
    where start/end are instruction indices (None = whole function).
    """
    swaps = {}
    if not config_path.exists():
        return swaps
    for line in config_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        # Parse: func_name: $X <-> $Y [@ start-end]
        m = re.match(r'(\w+)\s*:\s*(\$\w+)\s*<->\s*(\$\w+)(?:\s*@\s*(\d+)\s*-\s*(\d+))?', line)
        if m:
            func = m.group(1)
            reg_a = m.group(2)
            reg_b = m.group(3)
            start = int(m.group(4)) if m.group(4) is not None else None
            end = int(m.group(5)) if m.group(5) is not None else None
            swaps.setdefault(func, []).append((reg_a, reg_b, start, end))
        else:
            print(f"regfix: WARNING: ignoring malformed line: {line}", file=sys.stderr)
    return swaps


def is_instruction(line):
    """Check if a line is an actual instruction (not directive/label/comment/blank)."""
    s = line.strip()
    if not s:
        return False
    if s.startswith('.') or s.startswith('#') or s.endswith(':'):
        return False
    # Assembler directives we should skip
    if any(s.startswith(d) for d in ('gcc2_compiled', '.set', '.frame', '.mask', '.fmask',
                                      '.file', '.version', '.ident', '.size', '.type',
                                      '.globl', '.ent', '.end', '.align', '.text',
                                      '.data', '.rdata', '.section')):
        return False
    return True


def swap_registers(line, swap_pairs, insn_idx):
    """Apply register swaps to an assembly line.

    Only applies swaps whose range includes the current instruction index.
    Uses a placeholder to avoid A->B then B->A double-swap.
    """
    for reg_a, reg_b, start, end in swap_pairs:
        # Check range
        if start is not None and (insn_idx < start or insn_idx > end):
            continue
        placeholder = f"__REGFIX_{id(swap_pairs)}_{reg_a}__"
        ra = re.escape(reg_a)
        rb = re.escape(reg_b)
        line = re.sub(r'(?<!\w)' + ra + r'(?!\d)', placeholder, line)
        line = re.sub(r'(?<!\w)' + rb + r'(?!\d)', reg_a, line)
        line = line.replace(placeholder, reg_b)
    return line


def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    config_path = project_root / 'regfix.txt'

    swaps = load_config(config_path)
    if not swaps:
        for line in sys.stdin:
            sys.stdout.write(line)
        return

    current_func = None
    current_swaps = None
    insn_idx = 0

    for line in sys.stdin:
        stripped = line.strip()

        # Match function entry label
        label_match = re.match(r'^(\w+):$', stripped)
        if label_match:
            func_name = label_match.group(1)
            if func_name in swaps:
                current_func = func_name
                current_swaps = swaps[func_name]
                insn_idx = 0
            elif current_func and func_name.startswith('$'):
                pass  # local label — still in function
            # Don't clear on other labels (could be branch targets like .L2)

        # Detect end of function
        end_match = re.match(r'^\s*\.end\s+(\w+)', stripped)
        if end_match:
            if current_func and current_swaps and is_instruction(stripped):
                line = swap_registers(line, current_swaps, insn_idx)
            sys.stdout.write(line)
            if end_match.group(1) == current_func:
                current_func = None
                current_swaps = None
            continue

        # Apply swaps if inside a target function
        if current_func and current_swaps:
            if is_instruction(stripped):
                line = swap_registers(line, current_swaps, insn_idx)
                insn_idx += 1

        sys.stdout.write(line)


if __name__ == '__main__':
    main()
