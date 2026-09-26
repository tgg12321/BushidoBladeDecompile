"""Stage (pre-split, applied by mk_integ3): every FileRecord consumer that reached a member through a
cast or a hard-coded offset now names the member. Also fixes the stale D_8010277C comment."""
import sys
R = 'tmp/func_80034708/integ'

def rd(p):
    return open(p, encoding='utf-8').read()

def wr(p, s):
    open(p, 'w', encoding='utf-8', newline='\n').write(s)

def sub(s, old, new, path):
    assert s.count(old) == 1, (path, old[:70])
    return s.replace(old, new)

# ---- code6cac_b.c func_80033D38: the three clock records by name (was a HitRec view of the record)
p = R + '/src/code6cac_b.c'
s = rd(p)
s = sub(s, '''void func_80033D38(void) {
    struct HitRec {
        u8 x;
        u8 y;
        s32 t;
    };
    struct HitRec *recs = (struct HitRec *)&D_80106A50;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        struct HitRec *p;
        j = n - 1;
        p = recs + j + 1;
        if (p->t < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        struct HitRec *ins;
        for (k = 2; k > n; k--) {
            recs[k + 1] = recs[k];
        }
        ins = recs + n + 1;
        ins->x = (u8)g_practice_menu_table[0].unk_0A;
        ins->y = (u8)g_practice_menu_table[0].unk_0E;
        ins->t = D_800A3858;
    }
}''', '''void func_80033D38(void) {
    FileRecord *rec = &D_80106A50;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (rec->times[j].unk_4 < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            rec->times[k] = rec->times[k - 1];
        }
        rec->times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        rec->times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        rec->times[n].unk_4 = D_800A3858;
    }
}''', p)
wr(p, s)

# ---- ings.c func_800167EC: record pointer + member names (was byte offsets off (u8 *)&D_80106A50)
p = R + '/src/ings.c'
s = rd(p)
s = sub(s, '''    s32 i = 0;
    u32 c = 0x1A5E0;
    u8 *p;

    p = (u8 *)&D_80106A50;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    *(s32 *)p = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        p[i * 8 + 8] = 0;
        p[i * 8 + 9] = 0;
        *(u32 *)(p + i * 8 + 0xC) = c;
    }''', '''    s32 i = 0;
    u32 c = 0x1A5E0;
    FileRecord *rec;

    rec = &D_80106A50;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    rec->unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = c;
    }''', p)
wr(p, s)

# ---- code6cac_c_mid.c func_80037F40: the checksum stays a byte sum over the record's object
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

# ---- code6cac_c_mid.c func_8003800C: restore the record from a save slot by struct assignment
# (was a CopyBlock view of D_80106A50); byte-identical.
p = R + '/src/code6cac_c_mid.c'
s = rd(p)
s = sub(s, '''        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            do {
                *dst = *sp2;
                sp2++;
                dst++;
            } while (sp2 != end);
            *(s32 *)dst = *(s32 *)sp2;
        }
''', '''        if (!(*(src + 0x23) & 0x80)) {
            D_80106A50 = *(FileRecord *)src;
        }
''', p)
if s.count('CopyBlock') == 1:
    s = sub(s, 'typedef struct { s32 w0, w1, w2, w3; } CopyBlock;\n', '', p)
wr(p, s)

# ---- code6cac_c2.c func_8003C714: the clock records through a FileTimeRec pointer
p = R + '/src/code6cac_c2.c'
s = rd(p)
body_old = '''    u8 *base;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = (u8 *)D_80106A50.times;
    do {
        src = base + i * 8;
        dst = (u8 *)s0 + i * 4;
        a = *(s32 *)(src + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(src + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(src + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *src;'''
body_new = '''    FileTimeRec *base;
    FileTimeRec *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = D_80106A50.times;
    do {
        src = &base[i];
        dst = (u8 *)s0 + i * 4;
        a = src->unk_4;
        a = a / 1800;
        dst[0x21] = a;
        b = src->unk_4;
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = src->unk_4;
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = src->unk_0;'''
s = sub(s, body_old, body_new, p)
wr(p, s)

# ---- code6cac.c: stale symbol name in a FAKE comment
p = R + '/src/code6cac.c'
s = rd(p)
s = sub(s, '        /* FAKE: block-local address cache for D_8010277C. Every &-free spelling',
        '        /* FAKE: block-local address cache for D_80102778.unk_4[0]. Every &-free spelling', p)
wr(p, s)

# ---- comments that still named the retired declarations (fixed in place, BEFORE the split)
p = R + '/src/code6cac_b.c'
s = rd(p)
s = sub(s, 'Target alternates v1/a0 for g_file_flags address', 'Target alternates v1/a0 for D_80106A50.flags address', p)
a = s.index(' * INTEGRATION HANDOFF (unchanged from s62-s65; s66 proved the bytes).')
b = s.index(" * `python3 tmp/grind/func_80034F88/s63/apply.py <body.c>`.\n") + len(" * `python3 tmp/grind/func_80034F88/s63/apply.py <body.c>`.\n")
s = s[:a] + ''' * INTEGRATION (s62-s66 history). The copy loop's indexed store
 * `lui $at,%hi(..); addu $at,$at,$v1; sb $v0,%lo(..)($at)` targets the three
 * colour bytes 0x80106A70..72; since 2026-09-26 they are D_80106A50.color[3]
 * and the flags byte is D_80106A50.flags, members of the 0x24-byte FileRecord
 * declared in include/system.h.
''' + s[b:]
s = sub(s, '(a second C handle on\n         * D_80106A73)', '(a second C handle on\n         * D_80106A50.flags)', p)
wr(p, s)
p = R + '/src/code6cac_c2.c'
s = rd(p)
s = sub(s, ''' * insn_count padding, with `base = (u8 *)&D_80106A58;` unchanged and the
 * `extern s32 D_80106A58;` declaration at src/code6cac_c2.c:156 untouched.''',
        ''' * insn_count padding, without retyping the clock records. (Since 2026-09-26
 * they are D_80106A50.times of the FileRecord in include/system.h, read through
 * a FileTimeRec pointer.)''', p)
wr(p, s)
print('record members ok')
