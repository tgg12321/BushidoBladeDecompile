/* REJECTED -- s4 (2026-09-07, permuter modality).
 * Loop 2 respelled through a 4-byte `struct { u8 m, s, c, t; }` record pointer
 * (`q = (Clk *)((u8 *)p + 0x21); q[i].m = ... / q[i].s = ... / q[i].t = ...`)
 * instead of the inline `((u8 *)p)[i * 4 + 0x21]` byte subscripts. Motivation:
 * the aggregate shape is the sanctioned spelling when a target addresses a
 * record through a base register, and a different address computation might
 * carry different pre-combine RTL into loop 2 (the loop.c:1631 insn_count
 * lever). MEASURED on the s4 mini-TU harness (tmp/perm_80035280/compile.sh,
 * byte-validated against the full-TU sandbox object this session): 39 normalized
 * instruction diffs against the target, versus 29 for the accepted s4 body.
 * The struct form makes GCC keep a separate record pointer AND the `base`
 * walker, so loop 2 gains an extra induction variable and loses the target's
 * `addiu $a1, $a1, 4` shape. Strictly worse; not a route to insn_count 123.
 */
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;

typedef struct { u8 m; u8 s; u8 c; u8 t; } Clk;

extern u8 D_80106A73;
extern s32 D_80106A58;
extern s32 *func_80077D00(void);

void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    Clk *q;
    s32 i;
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
    q = (Clk *)((u8 *)p + 0x21);
    for (i = 0; i < 3; i++) {
        q[i].m = *(s32 *)(base + i * 8 + 4) / 1800;
        q[i].s = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        q[i].c = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        q[i].t = base[i * 8];
    }
}
