import os

path = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/src/text1b.c'
content = open(path).read()

D = '$'  # dollar sign helper

R = {}

def asm_block(name, lines):
    s = '__asm__(\n'
    s += '    ".section .text\n"\n'
    s += '    ".set noat\n"\n'
    s += '    ".set noreorder\n"\n'
    s += f'    "glabel {name}\n"\n'
    for line in lines:
        s += f'    "{line}\n"\n'
    s += f'    "endlabel {name}\n"\n'
    s += '    ".set reorder\n"\n'
    s += '    ".set at\n"\n'
    s += ');\n'
    return s

R['func_800789B8'] = asm_block('func_800789B8', [
    f'addiu {D}a0, {D}zero, 0x1',
    'syscall 0',
    f'jr    {D}ra',
    'nop',
])

R['func_800789C8'] = asm_block('func_800789C8', [
    f'addiu {D}a0, {D}zero, 0x2',
    'syscall 0',
    f'jr    {D}ra',
    'nop',
])

R['func_800789D8'] = asm_block('func_800789D8', [
    f'addu  {D}v0, {D}sp, {D}zero',
    f'jr    {D}ra',
    f'addu  {D}sp, {D}a0, {D}zero',
])

R['func_800545F4'] = asm_block('func_800545F4', [
    f'lui   {D}t0, 0x4C80',
    '.globl D_800545F8',
    'D_800545F8:',
    f'ori   {D}t0, {D}t0, 0x8080',
    '.globl D_800545FC',
    'D_800545FC:',
    f'lui   {D}t0, 0x4880',
    '.globl D_80054600',
    'D_80054600:',
    f'ori   {D}t0, {D}t0, 0x8080',
])

R['func_80078F60'] = asm_block('func_80078F60', [
    f'lui   {D}t1, %hi(jtbl_800A3620)',
    f'lw    {D}t1, %lo(jtbl_800A3620)({D}t1)',
    'nop',
    f'jr    {D}t1',
    'nop',
])

R['func_80078F74'] = asm_block('func_80078F74', [
    f'lui   {D}t1, %hi(jtbl_800A3624)',
    f'lw    {D}t1, %lo(jtbl_800A3624)({D}t1)',
    'nop',
    f'jr    {D}t1',
    'nop',
])

R['func_80052CD4'] = asm_block('func_80052CD4', [
    f'mfc2  {D}t0, {D}9',
    f'mfc2  {D}t1, {D}10',
    f'sra   {D}t0, {D}t0, 2',
    f'sra   {D}t1, {D}t1, 2',
    f'sw    {D}t0, 0({D}a0)',
    f'sw    {D}t1, 0({D}a1)',
    f'jr    {D}ra',
    'nop',
])

R['func_80052C28'] = asm_block('func_80052C28', [
    f'addiu {D}t0, {D}zero, 3',
    f'sub   {D}a1, {D}t0, {D}a1',
    f'srlv  {D}a0, {D}a0, {D}a1',
    f'srl   {D}t0, {D}a0, 11',
    f'andi  {D}a0, {D}a0, 2047',
    f'addi  {D}a0, {D}a0, -4096',
    f'srav  {D}a0, {D}a0, {D}t0',
    f'jr    {D}ra',
    f'andi  {D}v0, {D}a0, 4095',
])

R['func_800790A4'] = asm_block('func_800790A4', [
    f'and   {D}v0, {D}v0, {D}s5',
])

R['func_80049E1C'] = asm_block('func_80049E1C', [
    f'addiu {D}a0, {D}zero, -1',
    f'addiu {D}v1, {D}zero, 57',
    f'lui   {D}v0, %hi(D_80099CC2)',
    f'addiu {D}v0, {D}v0, %lo(D_80099CC2)',
    '1:',
    f'sh    {D}a0, 0({D}v0)',
    f'addiu {D}v1, {D}v1, -1',
    f'bgez  {D}v1, 1b',
    f'addiu {D}v0, {D}v0, -2',
    f'addiu {D}v0, {D}zero, -1',
    f'sw    {D}v0, %gp_rel(D_800A324C)({D}gp)',
    f'jr    {D}ra',
    'nop',
])

R['func_80077860'] = asm_block('func_80077860', [
    f'addiu {D}sp, {D}sp, -24',
    f'sw    {D}ra, 16({D}sp)',
    'jal   func_80069250',
    'nop',
    f'addiu {D}v1, {D}zero, 1',
    f'bne   {D}v0, {D}v1, 1f',
    f'addu  {D}v0, {D}zero, {D}zero',
    f'sw    {D}zero, %gp_rel(D_800A35E4)({D}gp)',
    f'addiu {D}v0, {D}zero, 1',
    '1:',
    f'lw    {D}ra, 16({D}sp)',
    f'addiu {D}sp, {D}sp, 24',
    f'jr    {D}ra',
    'nop',
])

R['func_80078B04'] = (
    's32 func_80078B04(u16 a0) {\n'
    '    if (a0 < 3) {\n'
    '        return *(u16 *)((s32)D_8009BD6C + a0 * 16);\n'
    '    }\n'
    '    return 0;\n'
    '}\n'
)

R['func_80078B3C'] = (
    's32 func_80078B3C(u16 a0) {\n'
    '    s32 *p = (s32 *)D_8009BD68;\n'
    '    p[1] |= (&D_8009BD70)[a0];\n'
    '    return a0 < 3;\n'
    '}\n'
)

R['func_80078B70'] = (
    's32 func_80078B70(u16 a0) {\n'
    '    s32 *p = (s32 *)D_8009BD68;\n'
    '    p[1] &= ~(&D_8009BD70)[a0];\n'
    '    return 1;\n'
    '}\n'
)

R['func_80078BA8'] = (
    's32 func_80078BA8(u16 a0) {\n'
    '    if (a0 < 3) {\n'
    '        *(u16 *)((s32)D_8009BD6C + a0 * 16) = 0;\n'
    '        return 1;\n'
    '    }\n'
    '    return 0;\n'
    '}\n'
)

count = 0
for name, new in R.items():
    old = f'INCLUDE_ASM("asm/funcs", {name});\n'
    if old in content:
        content = content.replace(old, new, 1)
        count += 1
        print(f'OK: {name}')
    else:
        print(f'WARN: {name} not found')

with open(path, 'w', newline='\n') as f:
    f.write(content)
    f.flush()
    os.fsync(f.fileno())

print(f'\nDone: {count} replaced')
