#!/usr/bin/env python3
"""
Extract target prologue instruction sequences from asm files.

Reads all INCLUDE_ASM stubs and their corresponding assembly,
extracts the prologue region (from function entry to the end of
the save/setup block), and outputs a JSON config mapping each
function to its target prologue instruction list.

The prologue region is defined as: from the first instruction after
glabel through either:
  - The first branch/jump instruction (inclusive of its delay slot)
  - Or a maximum of 30 instructions
whichever comes first.

Only functions with 2+ callee-saved register saves (sw $s0-$s7/$ra/$fp)
are included, since those are the ones with scheduling differences.
"""
import json
import re
import sys
from pathlib import Path


def extract_insn(line):
    """Extract the mnemonic and operands from a disassembly line.

    Input format: '    /* ADDR ADDR HEX */  mnemonic   operands'
    Returns: (mnemonic, operands_string, full_normalized) or None
    """
    m = re.search(r'\*/\s+(\S+)\s*(.*)', line)
    if not m:
        return None
    mnemonic = m.group(1)
    operands = m.group(2).strip()
    # Normalize whitespace
    normalized = f"{mnemonic}\t{operands}" if operands else mnemonic
    return (mnemonic, operands, normalized)


def normalize_operand(op):
    """Normalize register names and formatting for matching.

    GCC uses $16-$23 for s0-s7, $31 for ra, $30 for fp, $29 for sp.
    Target asm uses named registers. We normalize to named form.
    """
    # Map numbered to named registers
    reg_map = {
        '$0': '$zero', '$1': '$at', '$2': '$v0', '$3': '$v1',
        '$4': '$a0', '$5': '$a1', '$6': '$a2', '$7': '$a3',
        '$8': '$t0', '$9': '$t1', '$10': '$t2', '$11': '$t3',
        '$12': '$t4', '$13': '$t5', '$14': '$t6', '$15': '$t7',
        '$16': '$s0', '$17': '$s1', '$18': '$s2', '$19': '$s3',
        '$20': '$s4', '$21': '$s5', '$22': '$s6', '$23': '$s7',
        '$24': '$t8', '$25': '$t9',
        '$28': '$gp', '$29': '$sp', '$30': '$fp', '$31': '$ra',
    }
    result = op
    # Replace numbered registers with names (longest first to avoid partial matches)
    for num, name in sorted(reg_map.items(), key=lambda x: -len(x[0])):
        result = result.replace(num, name)
    return result


def is_branch_or_jump(mnemonic):
    return mnemonic in {
        'beq', 'bne', 'bgez', 'bgtz', 'blez', 'bltz',
        'beqz', 'bnez', 'bgezal', 'bltzal',
        'j', 'jal', 'jr', 'jalr',
    }


def is_callee_save(line):
    """Check if this is a sw to a callee-saved register."""
    return bool(re.search(r'sw\s+\$(s[0-7]|ra|fp)\s*,', line))


def get_prologue_key(mnemonic, operands):
    """Generate a matching key for a prologue instruction.

    This key is used to match GCC's version of an instruction with
    the target's version. We normalize registers and strip comments.
    """
    # Normalize operands
    norm_ops = normalize_operand(operands)
    # Strip inline comments
    if '/*' in norm_ops:
        norm_ops = norm_ops[:norm_ops.index('/*')].strip()
    # Remove trailing whitespace
    return f"{mnemonic}\t{norm_ops}"


def extract_target_prologue(func_name, asm_content):
    """Extract the prologue instruction sequence from target assembly."""
    pattern = "glabel " + func_name + "\n"
    idx = asm_content.find(pattern)
    if idx < 0:
        return None

    lines = asm_content[idx:idx+3000].split("\n")

    prologue = []
    save_count = 0
    insn_count = 0
    found_branch = False

    for line in lines[1:]:  # skip glabel line
        parsed = extract_insn(line)
        if parsed is None:
            continue

        mnemonic, operands, normalized = parsed
        insn_count += 1

        # Track callee saves
        if is_callee_save(line):
            save_count += 1

        prologue.append(normalized)

        # Stop conditions
        if is_branch_or_jump(mnemonic):
            # Include the delay slot instruction too
            # (it's the next instruction line)
            for next_line in lines[lines.index(line)+1:]:
                next_parsed = extract_insn(next_line)
                if next_parsed:
                    prologue.append(next_parsed[2])
                    break
            break

        if insn_count >= 30:
            break

    if save_count < 2:
        return None

    return prologue


def main():
    # Read all asm content
    asm_content = ""
    for af in sorted(Path("asm").rglob("*.s")):
        if "data" in af.parts or "rodata" in af.parts:
            continue
        with open(af) as f:
            asm_content += f.read() + "\n"

    # Get all stub function names
    stubs = []
    for f in sorted(Path("src").glob("*.c")):
        with open(f) as fh:
            for line in fh:
                m = re.search(r'INCLUDE_ASM\([^,]+,\s*([^)]+)\)', line)
                if m and not line.strip().startswith('//') and not line.strip().startswith('#define'):
                    stubs.append(m.group(1).strip())

    # Also get already-matched functions (for regression testing)
    # We'll extract their prologues too
    matched = []
    for f in sorted(Path("src").glob("*.c")):
        with open(f) as fh:
            content = fh.read()
        # Find function definitions (not INCLUDE_ASM)
        for m in re.finditer(r'(?:void|s32|u32|s16|u16|u8|int)\s+(func_[0-9A-Fa-f]+|[a-zA-Z_]\w+)\s*\([^)]*\)\s*\{', content):
            name = m.group(1)
            if name not in stubs:
                matched.append(name)

    # Extract prologues
    config = {}
    for func in stubs + matched:
        prologue = extract_target_prologue(func, asm_content)
        if prologue is not None:
            config[func] = prologue

    # Output
    if '--json' in sys.argv:
        json.dump(config, sys.stdout, indent=2)
    else:
        # Human-readable format
        print(f"# Extracted {len(config)} function prologues")
        print(f"# Stubs: {len([f for f in stubs if f in config])}")
        print(f"# Matched: {len([f for f in matched if f in config])}")
        print()
        for func, insns in sorted(config.items()):
            print(f"### {func}")
            for insn in insns:
                print(f"  {insn}")
            print()


if __name__ == "__main__":
    main()
