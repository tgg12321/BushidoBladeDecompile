import sys, os
# apply.py <tree> <body.c> [--tail]: apply the D_800A37D2[2] merge + the func_8001C8DC body to a scratch tree.
T, body = sys.argv[1], sys.argv[2]
tail = '--tail' in sys.argv

def edit(path, pairs):
    p = os.path.join(T, path)
    s = open(p, newline='').read()
    for old, new in pairs:
        assert s.count(old) == 1, (path, old)
        s = s.replace(old, new)
    open(p, 'w', newline='\n').write(s)

HDR_NEW = open('tmp/c8dc/hdr_block.txt').read()
edit('include/code6cac.h', [("extern u8 D_800A37D2;\nextern u8 D_800A37D3;\n", HDR_NEW)])
bodytxt = open(body).read().rstrip('\n') + '\n'
if tail:
    bodytxt += ("\n/* Tail word after func_8001C8DC's seven-entry compiler-generated switch table. */\n"
                "const u32 D_800100E0[1] = { 0x00000000 };\n")
edit('src/code6cac_tu2.c', [
    ('INCLUDE_RODATA("asm/rodata", jtbl_800100C4);\n', ''),
    ('INCLUDE_ASM("asm/funcs", func_8001C8DC);\n', bodytxt),
    ('func_8005E098(D_800A37D2, D_800A37D3, D_800A38B4, 1)', 'func_8005E098(D_800A37D2[0], D_800A37D2[1], D_800A38B4, 1)'),
])
edit('src/code6cac_b_tu2.c', [("    D_800A37D3 = 0;\n    D_800A37D2 = 0;\n", "    D_800A37D2[1] = 0;\n    D_800A37D2[0] = 0;\n")])
edit('src/code6cac_c2.c', [("(&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;",
                            "D_800A37D2[D_800A3748] = D_800A37D2[D_800A3748] + 1;")])
p = os.path.join(T, 'src/text1b.c')
s = open(p, newline='').read()
n = s.count('D_800A37D2 ')
s2 = s.replace('temp = D_800A37D2 / 5;', 'temp = D_800A37D2[0] / 5;') \
      .replace('(u8)(D_800A37D2 % 5)', '(u8)(D_800A37D2[0] % 5)') \
      .replace('temp = D_800A37D2 / 3;', 'temp = D_800A37D2[0] / 3;') \
      .replace('                D_800A37D2 = 0;\n', '                D_800A37D2[0] = 0;\n')
s2 = s2.replace("case 2's level D_800A37D2 / 5; case 2's practice level D_800A37D2 / 3",
                "case 2's level D_800A37D2[0] / 5; case 2's practice level D_800A37D2[0] / 3")
assert s2.count('D_800A37D2[0]') == 6, s2.count('D_800A37D2[0]')
open(p, 'w', newline='\n').write(s2)
edit('undefined_syms_auto.txt', [("D_800A37D3 = 0x800A37D3;\n", "")])
print('applied', body, 'tail' if tail else 'no-tail')
