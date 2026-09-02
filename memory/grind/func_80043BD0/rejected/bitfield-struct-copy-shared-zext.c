/* REJECTED s1: score 42/45 insns. Bitfield-struct copy with both extracts read before inserts: masks fold, but the extracts share one zero_extend:SI of the HImode struct reg -> a standalone 'andi 0xffff' the target lacks (target extracts/masks all read one SImode lhu result). */
typedef struct { u16 a : 6; u16 b : 9; u16 c : 1; } BitsE_43BD0;
typedef struct { u16 a : 4; u16 b : 1; u16 c : 11; } Bits16_43BD0;
typedef struct {
    u8 pad0[0xD];
    u8 f_D;
    BitsE_43BD0 e;
    u8 pad10[5];
    u8 f_15;
    Bits16_43BD0 s;
    u8 pad18[5];
    u8 f_1D;
    u8 pad1E[2];
} Prim_43BD0;
void func_80043BD0(Prim_43BD0 *p, s32 a1, s32 a2, s32 a3, s32 a4) {
    Bits16_43BD0 t;
    BitsE_43BD0 e;
    s32 na, nb;
    t = p->s;
    na = t.a + ((s16)a1 >> 6);
    nb = t.b + ((s16)a2 >> 8);
    t.a = na;
    t.b = nb;
    p->s = t;
    p->f_D += a2;
    p->f_1D += a2;
    p->f_15 += a2;
    e = p->e;
    na = e.a + ((s16)a3 >> 4);
    nb = e.b + a4;
    e.a = na;
    e.b = nb;
    p->e = e;
}
