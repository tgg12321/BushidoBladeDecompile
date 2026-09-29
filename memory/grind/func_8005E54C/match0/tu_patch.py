FLAGS = '''/* 0x8009BD38: the match-settings flag word (the word the D_8009BD24 block
   carries at +0x14; func_8006E534 reads it as *(s32 *)(cfg + 0x14)). One
   field per bit range the code tests; names are bit offsets. */
typedef struct {
    u32 unk0 : 4;
    u32 unk4 : 6;
    u32 unk10 : 2;
    u32 unk12 : 2;
    u32 unk14 : 1;
    u32 unk15 : 2;
    u32 unk17 : 1;
    u32 unk18 : 14;
} Unk8009BD38Flags;
extern Unk8009BD38Flags D_8009BD38;'''

REC = '''/* 0x8009BD24: two players x five rounds of 2-byte records (byte 0 = the
   character the round was fought with). */
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;
extern Unk8009BD24Record D_8009BD24[2][5];'''


def patch(s):
    # MEASUREMENT: record-struct chr table + bitfield flag word; all text1b.c consumers respelled
    assert s.count('extern u8 D_8009BD24[];') == 1
    s = s.replace('extern u8 D_8009BD24[];', '')
    s = s.replace('D_8009BD24[0] < 0xC', 'D_8009BD24[0][0].chr < 0xC')
    s = s.replace('func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8)',
                  'func_8006E534(a0, D_800A35E0, (u8 *)D_8009BD24, D_800A35E8)')
    assert s.count('extern s32 D_8009BD38;') == 1
    s = s.replace('extern s32 D_8009BD38;', FLAGS + '\n' + REC)
    s = s.replace('(((u32)D_8009BD38 >> 14) & 1) + 1', 'D_8009BD38.unk14 + 1')
    s = s.replace('(D_8009BD38 & 0x3000) == 0x2000', 'D_8009BD38.unk12 == 2')
    s = s.replace('v = D_8009BD38 & 0xF;', 'v = D_8009BD38.unk0;')
    s = s.replace('(D_8009BD38 & 0xF) * 2', 'D_8009BD38.unk0 * 2')
    old = '''        s32 *p = &D_8009BD38;
        s32 cur;
        ret = 1;
        cur = *p;
        D_800A35E4 = 0;
        cur &= ~0xF;
        cur |= result & 0xF;
        *p = cur;'''
    assert old in s
    s = s.replace(old, '''        ret = 1;
        D_800A35E4 = 0;
        D_8009BD38.unk0 = result;''')
    return s
