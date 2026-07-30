/* REJECTED (s2) — write the primitive through a real POLY_G4 struct pointer
 * (`poly->x1 = 640;` …) instead of `*(s16 *)(p + 0x10) = 640;`.
 *
 * The idea was mechanically motivated, not cosmetic: tools/gcc-2.7.2/sched.c
 * lines 834-881 carry the classic MEM_IN_STRUCT_P disambiguation — a MEM that
 * is in a struct AND has a varying address is treated as NOT conflicting with a
 * MEM that is neither in a struct nor address-varying. The lw of the scalar
 * global D_800A374C is exactly the second kind, so struct-typed member stores
 * should have dissolved the pre-load / post-load barrier that currently pins
 * our `sh $v1,0x10` inside the pre-load group (residual R1).
 *
 * Measured, with the score-2 tail in place (base 2):
 *   struct + leading x1=640 store (this file)          12
 *   struct + ascending coordinates after the ot load   10   (scalar equiv: 8)
 *   struct + setXY4 order (RGB block first)            10
 * Every struct spelling is strictly WORSE than its scalar-pointer counterpart,
 * so whatever the flag does here it does not free the store, and it costs
 * additional diffs elsewhere. Killed: the byte-offset scalar-cast spelling is
 * the better base, and the load barrier is not attackable via MEM_IN_STRUCT_P.
 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} PolyG4;

void func_8003553C(void) {
    PolyG4 *poly;
    PolyG4 *next;
    u32 *ot;

    poly = (PolyG4 *)D_800A38B4;
    initPolyG4((u8 *)poly);
    poly->x1 = 640;
    poly->y2 = 240;
    poly->y3 = 240;
    poly->r0 = 0;
    poly->g0 = 0;
    poly->b0 = 0x80;
    poly->r1 = 0;
    poly->g1 = 0;
    poly->b1 = 0x80;
    poly->r2 = 0;
    poly->g2 = 0;
    poly->b2 = 0;
    poly->r3 = 0;
    poly->g3 = 0;
    poly->b3 = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    poly->x0 = 0;
    poly->y0 = 0;
    poly->y1 = 0;
    poly->x2 = 0;
    poly->x3 = 640;
    next = poly + 1;
    ot_Link(ot, (u32 *)poly);
    D_800A38B4 = next;
}
