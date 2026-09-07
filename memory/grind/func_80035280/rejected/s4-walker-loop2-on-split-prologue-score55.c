/* REJECTED -- s4 (2026-09-07, permuter modality).
 * The s1 walker loop-2 spelling (three per-field locals a/b/c, `dst`/`base`
 * pointer walkers) grafted onto the s3/s4 flag-split prologue, i.e. the highest
 * loop-2 RTL insn_count ever measured here (62) combined with the best-known
 * prologue. MEASURED: 55 normalized instruction diffs versus 29 for the
 * accepted s4 body. insn_count 62 is still far below the loop.c:1631
 * requirement of >= 123, so the extra RTL buys nothing and the walker IVs cost
 * a great deal in the emitted loop. Used this session ONLY as the structurally
 * different seed for permuter campaign #2 (tmp/perm_80035280_w).
 */
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;

extern u8 D_80106A73;
extern s32 D_80106A58;
extern s32 *func_80077D00(void);

void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    u8 *dst;
    s32 i;
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s32 flags;
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
        ((u8 *)p + i)[0x17] = *src;
        ((u8 *)p + i)[0x1D] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    dst = (u8 *)p;
    for (i = 0; i < 3; i++) {
        a = *(s32 *)(base + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(base + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(base + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        d = *base;
        dst[0x24] = d;
        base += 8;
        dst += 4;
    }
}
