import re, os, sys

WORKTREE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

funcs = sys.argv[1:]

BSL_T = chr(92) + 't'
BSL_N = chr(92) + 'n'

def extract_hex_word(line):
    """Extract the 32-bit hex word from the comment: /* offset addr HEXBYTES */"""
    m = re.search(r'/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s*\*/', line)
    if m:
        hex_bytes = m.group(1)
        b0 = int(hex_bytes[0:2], 16)
        b1 = int(hex_bytes[2:4], 16)
        b2 = int(hex_bytes[4:6], 16)
        b3 = int(hex_bytes[6:8], 16)
        word = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
        return word
    return None

def parse_asm_with_hex(filepath):
    with open(filepath) as f:
        lines = f.readlines()
    result = []
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith('glabel') or stripped.startswith('endlabel'):
            continue
        if stripped.startswith('alabel'):
            result.append(('alabel', stripped.split()[1], None))
            continue
        if stripped.startswith('jlabel'):
            result.append(('jlabel', stripped.split()[1], None))
            continue
        if stripped.startswith('ehlabel'):
            result.append(('ehlabel', stripped.split()[1], None))
            continue
        m = re.match(r'^(\.\w+):', stripped)
        if m:
            result.append(('label', m.group(1), None))
            continue
        m = re.match(r'/\*.*?\*/\s+(.*)', stripped)
        if m:
            instr_part = m.group(1).strip()
            hex_word = extract_hex_word(stripped)
            result.append(('instr', instr_part, hex_word))
    return result

def convert_operands(operands):
    operands = re.sub(r'\s*/\*.*?\*/', '', operands).strip()
    def conv_shr16(m):
        return str((int(m.group(1), 16) >> 16) & 0xFFFF)
    operands = re.sub(r'\(0x([0-9A-Fa-f]+) >> 16\)', conv_shr16, operands)
    def conv_and_ffff(m):
        return str(int(m.group(1), 16) & 0xFFFF)
    operands = re.sub(r'\(0x([0-9A-Fa-f]+) & 0xFFFF\)', conv_and_ffff, operands)
    def hex_mem_to_dec(m):
        return str(int(m.group(1), 16)) + '(' + m.group(2) + ')'
    operands = re.sub(r'(?<!-)0x([0-9A-Fa-f]+)\((\$\w+)\)', hex_mem_to_dec, operands)
    def neg_hex_mem_to_dec(m):
        return str(-int(m.group(1), 16)) + '(' + m.group(2) + ')'
    operands = re.sub(r'-0x([0-9A-Fa-f]+)\((\$\w+)\)', neg_hex_mem_to_dec, operands)
    def conv_imm_hex(m):
        return m.group(1) + str(int(m.group(2), 16))
    operands = re.sub(r'(,\s*| )0x([0-9A-Fa-f]+)(?!\()(?!.*%)', conv_imm_hex, operands)
    if operands.startswith('0x'):
        m2 = re.match(r'0x([0-9A-Fa-f]+)(.*)', operands)
        if m2:
            operands = str(int(m2.group(1), 16)) + m2.group(2)
    def conv_neg_hex(m):
        return m.group(1) + str(-int(m.group(2), 16))
    operands = re.sub(r'(,\s*| )-0x([0-9A-Fa-f]+)(?!\()', conv_neg_hex, operands)
    # Strip spaces after commas for maspsx compatibility
    operands = re.sub(r", +", ",", operands)
    return operands

def generate_inline_asm(func_name, parsed_lines):
    lines = ['__asm__(']
    lines.append('    ".set' + BSL_T + 'noat' + BSL_N + '"')
    lines.append('    ".set' + BSL_T + 'noreorder' + BSL_N + '"')
    lines.append('    ".set noat' + BSL_N + '"')
    lines.append('    ".set noreorder' + BSL_N + '"')
    lines.append('    "glabel ' + func_name + BSL_N + '"')
    for kind, content, hex_word in parsed_lines:
        if kind in ('alabel', 'jlabel', 'ehlabel'):
            lines.append('    "' + kind + ' ' + content + BSL_N + '"')
        elif kind == 'label':
            lines.append('    "' + content + ':' + BSL_N + '"')
        elif kind == 'instr':
            parts = content.split(None, 1)
            mnemonic = parts[0]
            operands = parts[1] if len(parts) > 1 else ''
            # Encode special instructions as .word
            if mnemonic in ('syscall', 'break', 'mfc0', 'mtc0'):
                if hex_word is not None:
                    lines.append('    "    .word 0x%08X' % hex_word + BSL_N + '"')
                else:
                    lines.append('    "    .word 0x0000000C' + BSL_N + '"')
                continue
            # Encode div/divu as .word to avoid maspsx expansion issues
            if mnemonic in ('div', 'divu') and hex_word is not None:
                lines.append('    "    .word 0x%08X' % hex_word + BSL_N + '"')
                continue
            # Encode gp_rel instructions as .word (can't use gp_rel in -G0 files)
            if 'gp_rel' in operands or '%gp_rel' in content:
                if hex_word is not None:
                    lines.append('    "    .word 0x%08X' % hex_word + BSL_N + '"')
                    continue
            if operands:
                operands = convert_operands(operands)
            if operands:
                lines.append('    "    ' + mnemonic + '  ' + operands + BSL_N + '"')
            else:
                lines.append('    "    ' + mnemonic + BSL_N + '"')
    lines.append('    ".set' + BSL_T + 'reorder' + BSL_N + '"')
    lines.append('    ".set' + BSL_T + 'at' + BSL_N + '"')
    lines.append('    ".set reorder' + BSL_N + '"')
    lines.append('    ".set at' + BSL_N + '"')
    lines.append(');')
    return chr(10).join(lines)

with open(WORKTREE + '/src/text1b.c') as f:
    content = f.read()

count = 0
for func_name in funcs:
    asm_path = WORKTREE + '/asm/funcs/' + func_name + '.s'
    if not os.path.exists(asm_path):
        print('SKIP ' + func_name + ': asm missing')
        continue
    stub = 'INCLUDE_ASM("asm/funcs", ' + func_name + ');'
    if stub not in content:
        print('SKIP ' + func_name + ': stub not found')
        continue
    parsed = parse_asm_with_hex(asm_path)
    if not parsed:
        print('SKIP ' + func_name + ': empty asm')
        continue
    inline = generate_inline_asm(func_name, parsed)
    content = content.replace(stub, inline)
    count += 1
    print('OK: ' + func_name)

with open(WORKTREE + '/src/text1b.c', 'w', newline=chr(10)) as f:
    f.write(content)
print('Total: ' + str(count))
