/* REJECTED s1: score 39/45 insns. HImode bitfield-struct copy; second field extract reads the post-insert value so the two insert masks never fold (no andi 0xFFE0 / 0x8000). */
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
    t = p->s;
    t.a += (s16)a1 >> 6;
    t.b += (s16)a2 >> 8;
    p->s = t;
    p->f_D += a2;
    p->f_1D += a2;
    p->f_15 += a2;
    e = p->e;
    e.a += (s16)a3 >> 4;
    e.b += a4;
    p->e = e;
}
