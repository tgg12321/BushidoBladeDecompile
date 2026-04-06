#!/usr/bin/env python3
"""
Prologue reorder pass for the BB2 decompilation build pipeline.

Sits between cc1 and maspsx in the build pipeline:
  cpp | cc1 | prologue_fix.py | maspsx | as

Reads GCC's assembly output from stdin, identifies function prologues,
and reorders the save instructions + interleaved computation to match
the target binary's prologue order.

Strategy: We don't try to match exact instruction text (GCC pseudo-insns
differ from target machine code). Instead, we classify each prologue
instruction by its *action* — "save $s1", "move $a0 to $s3", "load
address of X into $s2" — and match those actions between GCC and target.
Then we reorder GCC's instructions to match the target's action sequence.
"""
import json
import re
import sys
from pathlib import Path


# Register normalization
REG_MAP = {
    '$0': '$zero', '$1': '$at', '$2': '$v0', '$3': '$v1',
    '$4': '$a0', '$5': '$a1', '$6': '$a2', '$7': '$a3',
    '$8': '$t0', '$9': '$t1', '$10': '$t2', '$11': '$t3',
    '$12': '$t4', '$13': '$t5', '$14': '$t6', '$15': '$t7',
    '$16': '$s0', '$17': '$s1', '$18': '$s2', '$19': '$s3',
    '$20': '$s4', '$21': '$s5', '$22': '$s6', '$23': '$s7',
    '$24': '$t8', '$25': '$t9',
    '$28': '$gp', '$29': '$sp', '$30': '$fp', '$31': '$ra',
}

BRANCH_MNEMONICS = {
    'beq', 'bne', 'bgez', 'bgtz', 'blez', 'bltz',
    'beqz', 'bnez', 'bgezal', 'bltzal',
}
JUMP_MNEMONICS = {'j', 'jal', 'jr', 'jalr'}
ALU_MNEMONICS = {
    'andi', 'ori', 'xori', 'sll', 'srl', 'sra', 'subu',
    'and', 'or', 'nor', 'xor', 'sllv', 'srlv', 'srav', 'negu',
}
CALLEE_SAVE_REGS = {
    '$s0', '$s1', '$s2', '$s3', '$s4', '$s5', '$s6', '$s7', '$fp',
}
PROLOGUE_TEMP_REGS = {'$v0', '$v1', '$at'}


def normalize_regs(text):
    """Replace numbered registers with named ones."""
    result = text
    for num, name in sorted(REG_MAP.items(), key=lambda x: -len(x[0])):
        result = result.replace(num, name)
    return result


def classify_gcc_insn(line):
    """Classify a GCC assembly instruction by its semantic action.

    Returns a tuple (action_type, key_details) or None for non-instructions.

    Action types:
      ('stack_alloc', size)           - addiu/subu $sp,$sp,-N
      ('save', reg)                   - sw $reg, offset($sp)
      ('move', dst_reg, src_reg)      - move $dst,$src / addu $dst,$src,$zero
      ('load_addr', dst_reg, symbol)  - la $dst, SYMBOL
      ('load', dst_reg, details)      - lw/lbu/lh/lhu $dst, ...
      ('other', normalized_text)      - anything else
    """
    stripped = line.strip()
    if not stripped or stripped.startswith('.') or stripped.startswith('#') or stripped.endswith(':'):
        return None

    parts = stripped.split(None, 1)
    if not parts:
        return None
    mnemonic = parts[0]
    operands = normalize_regs(parts[1]) if len(parts) > 1 else ''

    # Stack allocation
    if mnemonic == 'subu' and operands.startswith('$sp,$sp,'):
        size = operands.split(',')[2]
        return ('stack_alloc', size)
    if mnemonic == 'addiu' and '$sp,$sp,-' in operands:
        return ('stack_alloc', operands.split(',')[2])

    # Save to stack
    if mnemonic == 'sw':
        m = re.match(r'(\$\w+)\s*,\s*\d+\(\$sp\)', operands)
        if m:
            return ('save', m.group(1))

    # Move (various forms)
    if mnemonic == 'move':
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)', operands)
        if m:
            return ('move', m.group(1), m.group(2))
    if mnemonic == 'addu':
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)\s*,\s*\$zero', operands)
        if m:
            return ('move', m.group(1), m.group(2))

    # Load address
    if mnemonic == 'la':
        m = re.match(r'(\$\w+)\s*,\s*(\S+)', operands)
        if m:
            return ('load_addr', m.group(1), m.group(2))
    # lui + addiu pair for address load - just classify lui part
    if mnemonic == 'lui':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('lui', m.group(1), m.group(2))
    if mnemonic == 'addiu' and '%lo(' in operands:
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)\s*,\s*%lo\((\S+)\)', operands)
        if m:
            return ('addiu_lo', m.group(1), m.group(3))

    # Load from memory
    if mnemonic in ('lw', 'lbu', 'lh', 'lhu', 'lb'):
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('load', m.group(1), m.group(2))

    # Store (non-stack)
    if mnemonic in ('sw', 'sh', 'sb'):
        return ('store', operands)

    # Load immediate (GCC pseudo-op)
    if mnemonic == 'li':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('li', m.group(1), m.group(2))

    # Non-move addu (e.g., addu $s5, $v0, $v1)
    if mnemonic == 'addu':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', 'addu', m.group(1), m.group(2))

    # Non-stack, non-%lo addiu (e.g., addiu $s0, $a0, 4)
    if mnemonic == 'addiu':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', 'addiu', m.group(1), m.group(2))

    # ALU/shift operations
    if mnemonic in ALU_MNEMONICS:
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', mnemonic, m.group(1), m.group(2))

    # Unclassified
    return ('other', f"{mnemonic}\t{operands}")


def classify_target_insn(insn):
    """Classify a target prologue instruction.

    Target instructions are already in named-register form from
    extract_prologues.py. Same classification logic as GCC.
    """
    parts = insn.split('\t', 1)
    if len(parts) < 2:
        return ('other', insn)
    mnemonic = parts[0].strip()
    operands = parts[1].strip()

    # Stack allocation
    if mnemonic == 'addiu' and '$sp,$sp,-' in operands.replace(' ', ''):
        return ('stack_alloc', operands.replace(' ', '').split(',')[2])

    # Save to stack
    if mnemonic == 'sw':
        m = re.match(r'(\$\w+)\s*,\s*0x[\dA-Fa-f]+\(\$sp\)', operands)
        if m:
            return ('save', m.group(1))
        m = re.match(r'(\$\w+)\s*,\s*\d+\(\$sp\)', operands)
        if m:
            return ('save', m.group(1))

    # Move
    if mnemonic == 'addu':
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)\s*,\s*\$zero', operands)
        if m:
            return ('move', m.group(1), m.group(2))

    # Load immediate (addiu $reg, $zero, IMM — same as li pseudo-op)
    if mnemonic == 'addiu' and '$zero' in operands:
        m = re.match(r'(\$\w+)\s*,\s*\$zero\s*,\s*(.*)', operands)
        if m:
            return ('li', m.group(1), m.group(2))

    # Load address (lui part)
    if mnemonic == 'lui':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('lui', m.group(1), m.group(2))
    if mnemonic == 'addiu' and '%lo(' in operands:
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)\s*,\s*%lo\((\S+)\)', operands)
        if m:
            return ('addiu_lo', m.group(1), m.group(3))
    if mnemonic == 'ori' and '%lo(' in operands:
        # ori used for address low half sometimes
        m = re.match(r'(\$\w+)\s*,\s*(\$\w+)\s*,.*', operands)
        if m:
            return ('ori_lo', m.group(1))

    # Load from memory
    if mnemonic in ('lw', 'lbu', 'lh', 'lhu', 'lb'):
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('load', m.group(1), m.group(2))

    # Non-move addu
    if mnemonic == 'addu':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', 'addu', m.group(1), m.group(2))

    # Non-stack, non-%lo addiu
    if mnemonic == 'addiu':
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', 'addiu', m.group(1), m.group(2))

    # ALU/shift operations
    if mnemonic in ALU_MNEMONICS:
        m = re.match(r'(\$\w+)\s*,\s*(.*)', operands)
        if m:
            return ('alu', mnemonic, m.group(1), m.group(2))

    # Other
    return ('other', f"{mnemonic}\t{operands}")


def action_matches(gcc_action, target_action):
    """Check if a GCC action semantically matches a target action."""
    if gcc_action is None or target_action is None:
        return False

    g_type = gcc_action[0]
    t_type = target_action[0]

    # Exact type match for saves and moves
    if g_type == t_type:
        if g_type == 'stack_alloc':
            return True  # both allocate stack, sizes should match
        if g_type == 'save':
            return gcc_action[1] == target_action[1]  # same register
        if g_type == 'move':
            return (gcc_action[1] == target_action[1] and
                    gcc_action[2] == target_action[2])
        if g_type == 'lui':
            return gcc_action[1] == target_action[1]  # same dest register
        if g_type == 'load':
            return gcc_action[1] == target_action[1]  # same dest register
        if g_type == 'load_addr':
            return gcc_action[1] == target_action[1]  # same dest register
        if g_type == 'addiu_lo':
            return gcc_action[1] == target_action[1]  # same dest register

    # GCC 'la' expands to target 'lui' + 'addiu_lo' in the assembler.
    # Match 'load_addr' against 'lui' (collapsed target pair) or raw 'lui'
    if g_type == 'load_addr' and t_type == 'lui':
        return gcc_action[1] == target_action[1]  # same dest register

    # Load immediate (GCC 'li' matches target 'addiu $reg, $zero, IMM')
    if g_type == 'li' and t_type == 'li':
        return gcc_action[1] == target_action[1]  # same dest register

    # ALU/shift operations
    if g_type == 'alu' and t_type == 'alu':
        return (gcc_action[1] == target_action[1] and   # same mnemonic
                gcc_action[2] == target_action[2])       # same dest register

    return False


def find_prologue_region(lines, start_idx):
    """Find prologue instructions in GCC output starting from start_idx.

    The prologue consists of:
    - The stack allocation (subu/addiu $sp)
    - Callee-save register stores (sw $sN/$ra/$fp to stack)
    - Parameter moves into saved registers (move $sN, $aN)
    - Load-address instructions for saved registers (la $sN, SYMBOL)

    We stop as soon as we see an instruction that doesn't fit this pattern
    (e.g., lbu, li, sb, arithmetic), or at a branch/jump/.set noreorder.

    Returns list of (line_index, original_line, action) tuples.
    """
    result = []
    found_stack = False
    i = start_idx

    # Track which callee-save registers have been saved
    saved_regs = set()

    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Skip directives
        if not stripped or stripped.startswith('.') or stripped.endswith(':'):
            if '.set' in stripped and 'noreorder' in stripped:
                break
            i += 1
            continue

        # Skip #nop comments
        if stripped == '#nop':
            i += 1
            continue

        action = classify_gcc_insn(line)
        if action is None:
            i += 1
            continue

        # First instruction should be stack alloc
        if not found_stack:
            if action[0] == 'stack_alloc':
                found_stack = True
                result.append((i, line, action))
                i += 1
                continue
            else:
                break

        # Stop at branches/jumps
        parts = stripped.split(None, 1)
        mnemonic = parts[0] if parts else ''
        if mnemonic in BRANCH_MNEMONICS or mnemonic in JUMP_MNEMONICS:
            break

        # Only allow prologue-appropriate instructions:
        a_type = action[0]
        if a_type == 'save':
            # sw $reg, offset($sp) — always part of prologue
            saved_regs.add(action[1])
            result.append((i, line, action))
        elif a_type == 'move':
            # move $sN, $aN — parameter save, part of prologue
            dst = action[1]
            if dst in CALLEE_SAVE_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'load_addr':
            # la $reg, SYMBOL — address load (may be into temp for later use)
            dst = action[1]
            if dst in CALLEE_SAVE_REGS or dst in PROLOGUE_TEMP_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'lui':
            # lui $reg, ... — upper address load
            dst = action[1]
            if dst in CALLEE_SAVE_REGS or dst in PROLOGUE_TEMP_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'addiu_lo':
            # addiu $reg, $reg, %lo(...) — lower address part
            dst = action[1]
            if dst in CALLEE_SAVE_REGS or dst in PROLOGUE_TEMP_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'load':
            # lw $sN, GLOBAL — load global value into saved register
            dst = action[1]
            if dst in CALLEE_SAVE_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'li':
            # li $sN, IMM — load immediate into saved register
            dst = action[1]
            if dst in CALLEE_SAVE_REGS:
                result.append((i, line, action))
            else:
                break
        elif a_type == 'alu':
            # ALU/shift into saved register or temp feeding saved register
            dst = action[2]  # ('alu', mnemonic, dst, rest)
            if dst in CALLEE_SAVE_REGS or dst in PROLOGUE_TEMP_REGS:
                result.append((i, line, action))
            else:
                break
        else:
            # Any other instruction type = end of prologue
            break

        i += 1

        if len(result) >= 30:
            break

    return result


def build_target_actions(target_insns):
    """Build action list from target prologue, collapsing lui+addiu/ori pairs
    into single 'load_addr' actions since GCC emits these as 'la'."""
    actions = []
    skip_next = False

    for idx, insn in enumerate(target_insns):
        if skip_next:
            skip_next = False
            continue

        action = classify_target_insn(insn)

        # Check if this lui is followed by addiu_lo/ori_lo or lw for the same reg
        # If so, collapse into a single action since GCC emits these as pseudo-ops
        if action[0] == 'lui' and idx + 1 < len(target_insns):
            next_action = classify_target_insn(target_insns[idx + 1])
            if next_action[0] in ('addiu_lo', 'ori_lo') and next_action[1] == action[1]:
                actions.append(('load_addr', action[1], 'collapsed'))
                skip_next = True
                continue
            if next_action[0] == 'load' and next_action[1] == action[1]:
                # lui $reg + lw $reg, ... → collapsed global variable load
                actions.append(('load', next_action[1], 'collapsed'))
                skip_next = True
                continue

        actions.append(action)

    return actions


def reorder_prologue(lines, func_name, target_insns):
    """Reorder prologue of func_name in lines to match target ordering."""
    # Find function label
    label_idx = None
    for i, line in enumerate(lines):
        if line.strip() == f'{func_name}:':
            label_idx = i
            break
    if label_idx is None:
        return False

    # Extract GCC prologue
    gcc_pro = find_prologue_region(lines, label_idx + 1)
    if len(gcc_pro) < 2:
        return False

    # Build target action sequence (collapsing lui+addiu pairs)
    target_actions = build_target_actions(target_insns)

    # Match: for each target action, find the corresponding GCC instruction
    used = [False] * len(gcc_pro)
    order = []  # indices into gcc_pro, in target order

    for t_action in target_actions:
        matched = False
        for g_idx in range(len(gcc_pro)):
            if used[g_idx]:
                continue
            _, _, g_action = gcc_pro[g_idx]
            if action_matches(g_action, t_action):
                order.append(g_idx)
                used[g_idx] = True
                matched = True
                break
        if not matched:
            # Target has an instruction GCC doesn't — that's fine,
            # it might be from assembler expansion. Just skip it.
            pass

    # Append any remaining GCC prologue instructions not matched
    for g_idx in range(len(gcc_pro)):
        if not used[g_idx]:
            order.append(g_idx)

    # Check if reordering actually changes anything
    if order == list(range(len(gcc_pro))):
        return False  # Already in target order

    # Apply the reorder: write GCC lines in the new order
    reordered_lines = [gcc_pro[order[i]][1] for i in range(len(order))]

    # Replace in the original lines array
    for pos, g_idx in enumerate(range(len(gcc_pro))):
        line_idx = gcc_pro[g_idx][0]
        if pos < len(reordered_lines):
            lines[line_idx] = reordered_lines[pos]

    return True


def main():
    script_dir = Path(__file__).parent
    config_path = script_dir / "prologue_config.json"

    if not config_path.exists():
        sys.stdout.write(sys.stdin.read())
        return

    with open(config_path) as f:
        config = json.load(f)

    lines = sys.stdin.readlines()

    applied = 0
    skipped = 0

    for i, line in enumerate(lines):
        m = re.match(r'\s*\.ent\s+(\S+)', line)
        if m:
            func_name = m.group(1)
            if func_name in config:
                if reorder_prologue(lines, func_name, config[func_name]):
                    applied += 1
                else:
                    skipped += 1

    if applied > 0 or skipped > 0:
        sys.stderr.write(
            f"PROLOGUE_FIX: {applied} reordered, {skipped} unchanged\n"
        )

    sys.stdout.write(''.join(lines))


if __name__ == "__main__":
    main()
