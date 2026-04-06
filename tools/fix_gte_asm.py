#!/usr/bin/env python3
"""
fix_gte_asm.py - Fix GTE/COP2 instructions in assembly files for GNU as.

The GNU assembler (mipsel-linux-gnu-as) doesn't recognize PS1 GTE instruction
mnemonics (ctc2, mtc2, mfc2, cfc2, lwc2, swc2, mvmva, etc.). This script
replaces them with raw .word encodings extracted from the instruction comment.

Usage:
    python3 tools/fix_gte_asm.py <input.s> [output.s]
    python3 tools/fix_gte_asm.py --fix-targets   # fix all permuter target.s files

Each asm line has the format:
    /* offset addr HEXBYTES */  mnemonic  operands
Where HEXBYTES is the raw little-endian encoding. We convert to .word 0xXXXXXXXX.
"""
import os
import re
import struct
import sys

# GTE/COP2 mnemonics that GNU as doesn't understand
GTE_MNEMONICS = {
    'ctc2', 'mtc2', 'mfc2', 'cfc2',
    'lwc2', 'swc2',
    'mvmva', 'rtps', 'rtpt', 'nclip', 'avsz3', 'avsz4',
    'ncds', 'ncdt', 'nccs', 'ncct', 'dpcs', 'dpct',
    'intpl', 'sqr', 'op', 'gpf', 'gpl', 'ncs', 'nct',
    'cdp', 'cc', 'dcpl',
    'cop2',  # generic COP2
}


def hex_to_word(hex_bytes):
    """Convert hex bytes string (little-endian file order) to .word value.
    e.g., '4AD8C044' -> 0x44C0D84A"""
    raw = bytes.fromhex(hex_bytes)
    if len(raw) != 4:
        return None
    return struct.unpack('<I', raw)[0]


def fix_line(line):
    """Fix a single asm line, replacing GTE mnemonics with .word."""
    # Match the standard splat asm format:
    #    /* offset addr HEXBYTES */  mnemonic  operands
    m = re.match(
        r'^(\s*)(/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s*\*/\s+)'
        r'(\w+)(.*)',
        line
    )
    if not m:
        return line, False

    indent = m.group(1)
    comment = m.group(2)
    hex_bytes = m.group(3)
    mnemonic = m.group(4).lower()
    rest = m.group(5)

    if mnemonic not in GTE_MNEMONICS:
        return line, False

    word = hex_to_word(hex_bytes)
    if word is None:
        return line, False

    # Replace with .word, preserving the comment for reference
    new_line = f"{indent}.word 0x{word:08X}  /* {mnemonic}{rest} */"
    return new_line, True


def fix_target_line(line):
    """Fix a line in permuter target.s format (comments already stripped)."""
    # target.s format after sed processing:
    #    mnemonic  operands
    # We need the raw encoding, but it's been stripped from comments.
    # We can't fix these without the original encoding.
    # Instead, we should fix the permuter_setup.sh to preserve encodings.
    return line, False


def fix_file(input_path, output_path=None):
    """Fix GTE instructions in an asm file."""
    with open(input_path) as f:
        lines = f.readlines()

    fixed_count = 0
    new_lines = []
    for line in lines:
        new_line, was_fixed = fix_line(line.rstrip('\n'))
        new_lines.append(new_line)
        if was_fixed:
            fixed_count += 1

    if output_path is None:
        output_path = input_path

    with open(output_path, 'w') as f:
        for line in new_lines:
            f.write(line + '\n')

    return fixed_count


def fix_all_targets():
    """Regenerate ALL permuter target.s files from asm/funcs/, fixing:
    - GTE/COP2 instructions → .word encodings
    - jlabel directives → .word encodings (jump table entries)
    - alabel directives → regular labels
    """
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root)

    fixed_funcs = 0
    fixed_insns = 0

    for d in sorted(os.listdir("permuter")):
        target_s = os.path.join("permuter", d, "target.s")
        func_asm = os.path.join("asm", "funcs", f"{d}.s")

        if not os.path.isfile(target_s) or not os.path.isfile(func_asm):
            continue

        with open(func_asm) as f:
            asm_lines = f.readlines()

        # Check if this file needs fixing (GTE, jlabel, alabel, or broken target)
        asm_text = ''.join(asm_lines)
        # Also check existing target.s for issues
        with open(target_s) as f:
            target_text = f.read()
        needs_fix = (
            any(re.search(r'\b' + mn + r'\b', asm_text, re.IGNORECASE) for mn in GTE_MNEMONICS)
            or 'jlabel' in asm_text
            or 'alabel' in asm_text
            or 'jlabel' in target_text
            or 'alabel' in target_text
        )
        if not needs_fix:
            continue

        new_lines = [".set noat\n", ".set noreorder\n", "\n", ".text\n", "\n"]
        count = 0

        for line in asm_lines:
            line = line.rstrip('\n')

            # Convert glabel to regular label
            if line.startswith('glabel '):
                label = line[7:].strip()
                new_lines.append(f"{label}:\n")
                continue

            # Convert alabel to regular label
            m_alabel = re.match(r'\s*alabel\s+(\w+)', line)
            if m_alabel:
                new_lines.append(f"{m_alabel.group(1)}:\n")
                count += 1
                continue

            # Skip endlabel
            if line.strip().startswith('endlabel'):
                continue

            # Handle jlabel (jump table entries) — convert to .word label reference
            # Format 1: /* offset addr HEXBYTES */  jlabel .Lxxxx
            # Format 2:   jlabel .Lxxxx  (bare, no comment)
            m_jlabel = re.match(r'\s*(?:/\*.*?\*/\s+)?jlabel\s+(\S+)', line)
            if m_jlabel:
                label = m_jlabel.group(1).strip()
                new_lines.append(f"    .word {label}\n")
                count += 1
                continue

            # Try to fix GTE instruction
            fixed_line, was_fixed = fix_line(line)
            if was_fixed:
                m_word = re.match(r'\s*\.word\s+(0x[0-9A-Fa-f]+)\s+/\*\s*(.*?)\s*\*/', fixed_line)
                if m_word:
                    new_lines.append(f"    .word {m_word.group(1)}  /* {m_word.group(2)} */\n")
                else:
                    new_lines.append(fixed_line + "\n")
                count += 1
                continue

            # Regular instruction: strip address comment, keep instruction
            m_insn = re.match(r'\s*/\*.*?\*/\s+(.*)', line)
            if m_insn:
                insn = m_insn.group(1).rstrip()
                if insn:
                    new_lines.append(f"    {insn}\n")
                continue

            # Labels (.L*:), directives, etc.
            stripped = line.strip()
            if stripped:
                if re.match(r'\.\w+:', stripped):
                    new_lines.append(f"  {stripped}\n")
                else:
                    new_lines.append(line + "\n")

        if count > 0:
            with open(target_s, 'w') as f:
                f.writelines(new_lines)
            fixed_funcs += 1
            fixed_insns += count
            print(f"  {d}: fixed {count} instruction(s)")

    return fixed_funcs, fixed_insns


def reassemble_targets():
    """Re-assemble all fixed target.s into target.o."""
    import subprocess
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root)

    success = 0
    fail = 0

    for d in sorted(os.listdir("permuter")):
        target_s = os.path.join("permuter", d, "target.s")
        target_o = os.path.join("permuter", d, "target.o")

        if not os.path.isfile(target_s):
            continue

        try:
            r = subprocess.run(
                ["mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
                 "-no-pad-sections", "-O1", "-G0",
                 target_s, "-o", target_o],
                capture_output=True, text=True, timeout=10)
            if r.returncode == 0:
                # Verify .text is non-empty
                r2 = subprocess.run(
                    ["mipsel-linux-gnu-objdump", "-h", target_o],
                    capture_output=True, text=True, timeout=5)
                m = re.search(r'\.text\s+(\w+)', r2.stdout)
                text_size = m.group(1) if m else "?"
                if text_size != "00000000":
                    success += 1
                else:
                    print(f"  WARNING: {d} .text still empty after fix")
                    fail += 1
            else:
                print(f"  FAIL: {d}: {r.stderr.strip()[:100]}")
                fail += 1
        except Exception as e:
            print(f"  ERROR: {d}: {e}")
            fail += 1

    return success, fail


if __name__ == "__main__":
    if "--fix-targets" in sys.argv:
        print("Fixing GTE instructions in all permuter target.s files...")
        funcs, insns = fix_all_targets()
        print(f"\nFixed {insns} GTE instructions across {funcs} functions")

        if "--reassemble" in sys.argv:
            print("\nReassembling target.o files...")
            ok, fail = reassemble_targets()
            print(f"Reassembled: {ok} success, {fail} failures")
    elif len(sys.argv) >= 2:
        input_file = sys.argv[1]
        output_file = sys.argv[2] if len(sys.argv) > 2 else None
        count = fix_file(input_file, output_file)
        print(f"Fixed {count} GTE instruction(s)")
    else:
        print("Usage:")
        print("  python3 tools/fix_gte_asm.py <input.s> [output.s]")
        print("  python3 tools/fix_gte_asm.py --fix-targets [--reassemble]")
