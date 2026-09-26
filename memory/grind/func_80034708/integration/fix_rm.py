"""Replace record_members.py's func_80037F40 block with the s9 form (whole function body swap)."""
p = 'tmp/func_80034708/record_members.py'
s = open(p).read()
a = s.index('# ---- code6cac_c_mid.c func_80037F40:')
b = s.index('# ---- code6cac_c_mid.c func_8003800C:')
new = r"""# ---- code6cac_c_mid.c func_80037F40: the checksum stays a byte sum over the record's object
# representation; each save slot receives the record by struct assignment into the save buffer's
# FileRecord slots (slot stride 0x24 == sizeof(FileRecord)); the slot's checksum word is indexed
# like the slot. Measured alternatives: integration/f37f40_try.py (ledger [s9]).
p = R + '/src/code6cac_c_mid.c'
s = rd(p)
i0 = s.index('void func_80037F40(u8 *a0) {')
i1 = s.index('\n}\n', i0) + 3
s = s[:i0] + '''void func_80037F40(u8 *a0) {
    s32 checksum;
    u8 *p;
    s32 i;

    checksum = 0;
    p = (u8 *)&D_80106A50;
    i = 0;
    do {
        checksum += *p++;
        i++;
    } while ((u32)i < 0x24);

    {
        u8 *base = a0;
        i = 0;
        do {
            ((FileRecord *)base)[i] = D_80106A50;
            *(s32 *)(base + i * 4 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            i++;
        } while (i < 3);
        *(s32 *)(base + 0xFC) = 0;
    }
}
''' + s[i1:]
if s.count('Quad') == 1:
    s = sub(s, 'typedef struct { s32 w[4]; } Quad;\n', '', p)
wr(p, s)

"""
s = s[:a] + new + s[b:]
open(p, 'w', newline='\n').write(s)
print('ok')
