#!/usr/bin/env python3
"""
asm_analysis.py — Analyze target assembly to generate decompilation hints.

Three analyses:
  1. Prologue register detection — identifies parameter moves and constant loads,
     generates `register asm` constraints for base.c
  2. Reload detection — finds same offset loaded from same base register multiple
     times, indicating the original C used separate loads (no shared variable)
  3. Loop structure hints — detects for-loop (offset folding, strength reduction)
     vs goto-loop patterns

Usage:
    python3 tools/asm_analysis.py asm/funcs/func_80032064.s
    python3 tools/asm_analysis.py asm/funcs/func_80032064.s --json
"""

import re
import sys
import json
import argparse
from collections import defaultdict

# MIPS register name → number mapping
REG_NUM = {
    '$zero': 0, '$at': 1,
    '$v0': 2, '$v1': 3,
    '$a0': 4, '$a1': 5, '$a2': 6, '$a3': 7,
    '$t0': 8, '$t1': 9, '$t2': 10, '$t3': 11,
    '$t4': 12, '$t5': 13, '$t6': 14, '$t7': 15,
    '$s0': 16, '$s1': 17, '$s2': 18, '$s3': 19,
    '$s4': 20, '$s5': 21, '$s6': 22, '$s7': 23,
    '$t8': 24, '$t9': 25,
    '$k0': 26, '$k1': 27,
    '$gp': 28, '$sp': 29, '$fp': 30, '$ra': 31,
}

# Reverse: number → name
REG_NAME = {v: k for k, v in REG_NUM.items()}

# C-friendly names (without $)
REG_C_NAME = {
    '$a0': 'a0', '$a1': 'a1', '$a2': 'a2', '$a3': 'a3',
    '$t0': 't0', '$t1': 't1', '$t2': 't2', '$t3': 't3',
    '$t4': 't4', '$t5': 't5', '$t6': 't6', '$t7': 't7',
    '$s0': 's0', '$s1': 's1', '$s2': 's2', '$s3': 's3',
    '$s4': 's4', '$s5': 's5', '$s6': 's6', '$s7': 's7',
    '$v0': 'v0', '$v1': 'v1',
    '$t8': 't8', '$t9': 't9',
}

PARAM_REGS = {'$a0', '$a1', '$a2', '$a3'}
ARG_ORDER = ['$a0', '$a1', '$a2', '$a3']


def parse_asm_file(path):
    """Parse a .s file into a list of (addr, hex, mnemonic, operands_str) tuples."""
    insns = []
    labels = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            # Label line
            if line.startswith('.L') or line.startswith('jlabel'):
                m = re.search(r'\.L([0-9A-Fa-f]+)', line)
                if m:
                    labels.append(int(m.group(1), 16))
                continue
            # Instruction line: /* offset addr hex */ mnemonic operands
            m = re.match(
                r'/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+([0-9A-Fa-f]+)\s+\*/\s+'
                r'(\S+)\s*(.*)',
                line
            )
            if m:
                addr = int(m.group(1), 16)
                hexval = int(m.group(2), 16)
                mnem = m.group(3)
                ops = m.group(4).strip()
                insns.append((addr, hexval, mnem, ops))
    return insns, labels


def parse_operands(ops_str):
    """Split operand string, handling offset(reg) notation."""
    # Remove comments
    ops_str = re.sub(r'#.*', '', ops_str).strip()
    # Split by comma, but respect parentheses
    parts = []
    depth = 0
    current = ''
    for ch in ops_str:
        if ch == ',' and depth == 0:
            parts.append(current.strip())
            current = ''
        else:
            if ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
            current += ch
    if current.strip():
        parts.append(current.strip())
    return parts


def parse_mem_operand(op):
    """Parse 'offset($reg)' → (offset, reg) or None."""
    m = re.match(r'(-?(?:0x)?[0-9A-Fa-f]+)\((\$\w+)\)', op)
    if m:
        off_str = m.group(1)
        if off_str.startswith('0x') or off_str.startswith('-0x'):
            off = int(off_str, 16) if not off_str.startswith('-') else -int(off_str[1:], 16)
        else:
            off = int(off_str)
        return (off, m.group(2))
    # %lo(SYM)($reg)
    m = re.match(r'%lo\((\w+)\)\((\$\w+)\)', op)
    if m:
        return (m.group(1), m.group(2))  # symbolic offset
    # %gp_rel(SYM)($gp)
    m = re.match(r'%gp_rel\((\w+)\)\((\$\w+)\)', op)
    if m:
        return (m.group(1), m.group(2))
    return None


def parse_immediate(op):
    """Parse an immediate value, return int or None."""
    op = op.strip()
    # Handle %hi(...) and %lo(...)
    if op.startswith('%hi(') or op.startswith('%lo('):
        return None  # symbolic
    # Hex
    m = re.match(r'^-?0x[0-9A-Fa-f]+$', op)
    if m:
        return int(op, 16) if not op.startswith('-') else -int(op[1:], 16)
    # Decimal
    m = re.match(r'^-?\d+$', op)
    if m:
        return int(op)
    return None


# ============================================================
# Analysis 1: Prologue Register Detection
# ============================================================

def analyze_prologue(insns, label_addrs=None):
    """
    Analyze function prologue to detect:
    - Parameter register moves (a0→aX, a1→aX, etc.)
    - Constant loads into registers
    - Global address loads
    Returns list of hints.
    """
    if label_addrs is None:
        label_addrs = set()
    results = {
        'frame_size': 0,
        'saved_regs': [],
        'param_moves': [],   # (src_param, dest_reg)
        'const_loads': [],   # (dest_reg, value)
        'global_loads': [],  # (dest_reg, symbol)
        'hints': [],
    }

    if not insns:
        return results

    # First instruction should be addiu sp,sp,-N
    mnem, ops = insns[0][2], insns[0][3]
    if mnem == 'addiu':
        parts = parse_operands(ops)
        if len(parts) == 3 and parts[0] == '$sp' and parts[1] == '$sp':
            val = parse_immediate(parts[2])
            if val is not None and val < 0:
                results['frame_size'] = -val

    # Scan prologue: stop at first label, branch, jal, or load/store to non-sp
    # The prologue is the setup before any real work begins
    prologue_end = min(len(insns), 20)

    for i, (addr, hexval, mnem, ops) in enumerate(insns[1:prologue_end], 1):
        # Stop at labels (loop starts, branch targets)
        if addr in label_addrs:
            break

        parts = parse_operands(ops)

        # Stop at loads/stores to non-sp (actual work, not prologue)
        if mnem in ('lw', 'lh', 'lhu', 'lb', 'lbu', 'sw', 'sh', 'sb'):
            if len(parts) == 2:
                mem = parse_mem_operand(parts[1])
                if mem and mem[1] == '$sp':
                    # sw to sp is prologue (saving regs)
                    results['saved_regs'].append(parts[0])
                    continue
                else:
                    break  # non-sp memory access = past prologue

        # Stop at first branch or jal
        if mnem in ('beq', 'beqz', 'bne', 'bnez', 'blez', 'bltz', 'bgez',
                     'bgtz', 'jal', 'j'):
            break

        # addu $dst, $src, $zero — move (param move or zero init)
        if mnem == 'addu' and len(parts) == 3 and parts[2] == '$zero':
            src, dst = parts[1], parts[0]
            if src in PARAM_REGS and dst != src:
                results['param_moves'].append((src, dst))
            elif src == '$zero':
                # zero init: addu $v1, $zero, $zero
                results['const_loads'].append((dst, 0))

        # addiu $dst, $zero, IMM — constant load
        if mnem == 'addiu' and len(parts) == 3 and parts[1] == '$zero':
            dst = parts[0]
            val = parse_immediate(parts[2])
            if val is not None and dst != '$sp':
                results['const_loads'].append((dst, val))

        # lui + addiu for global address
        if mnem == 'lui' and len(parts) == 2:
            dst = parts[0]
            if i + 1 < len(insns):
                next_mnem = insns[i + 1][2]
                next_ops = insns[i + 1][3]
                next_parts = parse_operands(next_ops)
                if next_mnem == 'addiu' and len(next_parts) == 3:
                    if next_parts[0] == dst and next_parts[1] == dst:
                        m2 = re.search(r'%hi\((\w+)\)', parts[1])
                        if m2:
                            results['global_loads'].append((dst, m2.group(1)))

    # Generate hints
    for src, dst in results['param_moves']:
        src_idx = ARG_ORDER.index(src)
        c_name = REG_C_NAME.get(dst, dst)
        param_name = f'arg{src_idx}'
        results['hints'].append(
            f'register ... {param_name} asm("{c_name}") = a{src_idx};  '
            f'/* {src} → {dst} */'
        )

    for reg, val in results['const_loads']:
        c_name = REG_C_NAME.get(reg, reg)
        if val == 0:
            results['hints'].append(
                f'/* {reg} = 0 (loop counter or accumulator, assigned in prologue) */'
            )
        else:
            if val < 0:
                results['hints'].append(
                    f'/* {reg} = {val} (0x{val & 0xFFFFFFFF:X}) — constant in named variable */'
                )
            else:
                results['hints'].append(
                    f'/* {reg} = 0x{val:X} ({val}) — constant in named variable */'
                )

    for reg, sym in results['global_loads']:
        c_name = REG_C_NAME.get(reg, reg)
        results['hints'].append(
            f'/* {reg} = &{sym} — global address loaded in prologue */'
        )

    return results


# ============================================================
# Analysis 2: Reload Detection
# ============================================================

def analyze_reloads(insns):
    """
    Find cases where the same offset(base_reg) is loaded multiple times.
    This indicates the original C did NOT cache the value in a local variable.
    """
    LOAD_MNEMS = {'lw', 'lh', 'lhu', 'lb', 'lbu'}
    results = []

    # Collect all loads: (mnemonic, offset, base_reg, insn_index)
    loads = []
    for i, (addr, hexval, mnem, ops) in enumerate(insns):
        if mnem in LOAD_MNEMS:
            parts = parse_operands(ops)
            if len(parts) == 2:
                mem = parse_mem_operand(parts[1])
                if mem and isinstance(mem[0], int):
                    loads.append((mnem, mem[0], mem[1], i, addr, parts[0]))

    # Group by (offset, base_reg) — find duplicates
    grouped = defaultdict(list)
    for mnem, off, base, idx, addr, dst in loads:
        grouped[(off, base)].append((mnem, idx, addr, dst))

    for (off, base), entries in sorted(grouped.items()):
        if len(entries) < 2:
            continue
        # Filter out loads that are clearly in a loop (same address)
        addrs = [e[2] for e in entries]
        if len(set(addrs)) < 2:
            continue
        # Check if different load widths (lhu vs lh) — extra important
        mnems = [e[0] for e in entries]
        width_note = ''
        if len(set(mnems)) > 1:
            width_note = f' [NOTE: different widths: {", ".join(mnems)} — use separate typed casts]'

        results.append({
            'offset': off,
            'base_reg': base,
            'count': len(entries),
            'loads': [(e[0], e[2], e[3]) for e in entries],
            'width_note': width_note,
        })

    return results


# ============================================================
# Analysis 3: Loop Structure Detection
# ============================================================

def analyze_loops(insns, labels):
    """
    Detect loop patterns and classify as:
    - 'for_loop': has strength reduction (addiu base,reg,OFFSET) or
      loop optimizer patterns (offset folded into base pointer)
    - 'goto_loop': raw offsets in load/store, manual counter
    - 'do_while': bottom-tested loop
    """
    results = []

    # Find backward branches (loop back-edges)
    branch_mnems = {'beq', 'beqz', 'bne', 'bnez', 'blez', 'bltz', 'bgez', 'bgtz'}
    func_start = insns[0][0] if insns else 0

    for i, (addr, hexval, mnem, ops) in enumerate(insns):
        if mnem not in branch_mnems:
            continue

        # Decode branch target
        parts = parse_operands(ops)
        target_str = parts[-1] if parts else ''
        m = re.search(r'\.L([0-9A-Fa-f]+)', target_str)
        if not m:
            continue
        target = int(m.group(1), 16)

        # Backward branch = loop
        if target >= addr:
            continue

        # Find the loop body (from target to this branch)
        loop_start_idx = None
        loop_end_idx = i
        for j, (a, _, _, _) in enumerate(insns):
            if a >= target:
                loop_start_idx = j
                break

        if loop_start_idx is None:
            continue

        loop_body = insns[loop_start_idx:loop_end_idx + 1]
        if len(loop_body) < 3:
            continue

        loop_info = {
            'start_addr': target,
            'end_addr': addr,
            'body_size': len(loop_body),
            'type': 'unknown',
            'details': [],
        }

        # Check for strength reduction: addiu $reg, $base, LARGE_OFFSET in loop
        # This indicates a for-loop where the compiler folded a base offset
        has_base_offset_init = False
        has_stride_increment = False
        has_counter = False
        has_slt_or_slti = False
        loop_loads = []
        loop_stores = []

        for j, (a, h, m2, o2) in enumerate(loop_body):
            p2 = parse_operands(o2)

            # Look for addiu $reg, $base, LARGE_OFFSET where dest != src
            # (offset folding: creating a new pointer from base + offset)
            if m2 == 'addiu' and len(p2) == 3:
                imm = parse_immediate(p2[2])
                if (imm is not None and abs(imm) > 0x10
                        and p2[0] != p2[1]  # dest != src (not an increment)
                        and p2[1] != '$sp' and p2[1] != '$zero'):
                    has_base_offset_init = True
                    loop_info['details'].append(
                        f'base offset fold: {p2[0]} = {p2[1]} + 0x{imm & 0xFFFF:X}'
                    )
                # Small stride increment (addiu reg,reg,2 or similar)
                if imm is not None and 0 < abs(imm) <= 0x100 and p2[0] == p2[1]:
                    has_stride_increment = True
                # Counter increment (addiu reg,reg,1)
                if imm == 1 and p2[0] == p2[1]:
                    has_counter = True

            # slt/slti in loop = counter-based loop
            if m2 in ('slt', 'slti'):
                has_slt_or_slti = True

            # Collect load/store offsets in loop
            if m2 in ('lw', 'lh', 'lhu', 'lb', 'lbu', 'sw', 'sh', 'sb'):
                if len(p2) == 2:
                    mem = parse_mem_operand(p2[1])
                    if mem and isinstance(mem[0], int):
                        if m2.startswith('l'):
                            loop_loads.append((mem[0], mem[1]))
                        else:
                            loop_stores.append((mem[0], mem[1]))

        # Classify loop type
        # Check if loads/stores use small offsets (0, 2, 4) vs large offsets (0x332, 0x334)
        all_offsets = [off for off, _ in loop_loads + loop_stores]
        small_offsets = [off for off in all_offsets if abs(off) <= 0x20]
        large_offsets = [off for off in all_offsets if abs(off) > 0x20]

        if has_base_offset_init and small_offsets and not large_offsets:
            loop_info['type'] = 'for_loop_folded'
            loop_info['details'].append(
                'Offsets folded into base pointer — use for-loop (GCC loop optimizer active)'
            )
        elif has_counter and has_slt_or_slti and large_offsets:
            loop_info['type'] = 'for_loop_raw'
            loop_info['details'].append(
                'Counter-based loop with raw offsets — for-loop, no offset folding'
            )
        elif has_counter and has_slt_or_slti:
            loop_info['type'] = 'for_loop'
            loop_info['details'].append(
                'Standard counter-based loop — use for-loop'
            )
        elif has_stride_increment and not has_counter:
            loop_info['type'] = 'pointer_loop'
            loop_info['details'].append(
                'Pointer-stride loop without counter — while(*ptr) style'
            )
        else:
            loop_info['type'] = 'goto_loop'
            loop_info['details'].append(
                'No clear loop optimizer pattern — try goto-based loop'
            )

        # Check if bottom-tested (branch is the loop test)
        if loop_body[-1][2] in branch_mnems:
            loop_info['details'].append('Bottom-tested (do-while or for with goto)')

        results.append(loop_info)

    return results


# ============================================================
# Main
# ============================================================

def format_text_report(path, prologue, reloads, loops):
    """Format a human-readable report."""
    lines = []
    func_name = path.rsplit('/', 1)[-1].rsplit('\\', 1)[-1].replace('.s', '')
    lines.append(f'=== Analysis: {func_name} ===')
    lines.append('')

    # Prologue
    lines.append(f'Frame size: 0x{prologue["frame_size"]:X} ({prologue["frame_size"]} bytes)')
    if prologue['saved_regs']:
        lines.append(f'Saved regs: {", ".join(prologue["saved_regs"])}')
    lines.append('')

    if prologue['param_moves'] or prologue['const_loads'] or prologue['global_loads']:
        lines.append('--- Prologue Register Map ---')
        for src, dst in prologue['param_moves']:
            lines.append(f'  {src} → {dst}  (parameter move)')
        for reg, val in prologue['const_loads']:
            if val == 0:
                lines.append(f'  {reg} = 0  (zero init)')
            elif val < 0:
                lines.append(f'  {reg} = {val} (0x{val & 0xFFFFFFFF:X})')
            else:
                lines.append(f'  {reg} = 0x{val:X} ({val})')
        for reg, sym in prologue['global_loads']:
            lines.append(f'  {reg} = &{sym}')
        lines.append('')

    if prologue['hints']:
        lines.append('--- Suggested register asm hints ---')
        for h in prologue['hints']:
            lines.append(f'  {h}')
        lines.append('')

    # Reloads
    if reloads:
        lines.append('--- Reload Detection (do NOT cache in a variable) ---')
        for r in reloads:
            off = r['offset']
            off_str = f'0x{off:X}' if off >= 0 else f'-0x{-off:X}'
            lines.append(f'  {off_str}({r["base_reg"]}) loaded {r["count"]}x{r["width_note"]}')
            for mnem, addr, dst in r['loads']:
                lines.append(f'    {mnem} {dst}, {off_str}({r["base_reg"]})  @ 0x{addr:08X}')
        lines.append('')

    # Loops
    if loops:
        lines.append('--- Loop Analysis ---')
        for l in loops:
            lines.append(f'  Loop 0x{l["start_addr"]:08X}..0x{l["end_addr"]:08X} '
                        f'({l["body_size"]} insns) — {l["type"]}')
            for d in l['details']:
                lines.append(f'    {d}')
        lines.append('')

    if not reloads and not loops:
        lines.append('No reloads or loops detected.')
        lines.append('')

    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(
        description='Analyze target assembly for decompilation hints'
    )
    parser.add_argument('asm_file', help='Path to target .s file')
    parser.add_argument('--json', action='store_true', help='Output as JSON')
    args = parser.parse_args()

    insns, labels = parse_asm_file(args.asm_file)
    if not insns:
        print(f'ERROR: No instructions found in {args.asm_file}', file=sys.stderr)
        sys.exit(1)

    # Collect label addresses for prologue boundary detection
    label_addrs = set()
    with open(args.asm_file) as f:
        for line in f:
            m = re.search(r'^\s*\.L([0-9A-Fa-f]+)', line.strip())
            if m:
                label_addrs.add(int(m.group(1), 16))
    prologue = analyze_prologue(insns, label_addrs)
    reloads = analyze_reloads(insns)
    loops = analyze_loops(insns, labels)

    if args.json:
        output = {
            'prologue': prologue,
            'reloads': reloads,
            'loops': loops,
        }
        print(json.dumps(output, indent=2, default=str))
    else:
        print(format_text_report(args.asm_file, prologue, reloads, loops))


if __name__ == '__main__':
    main()
