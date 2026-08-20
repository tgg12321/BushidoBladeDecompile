/* REJECTED s29 2026-08-20 — score 10, target_insns 111 == build_insns 111.
 * Same body as the MATCHING candidate.c except the struct is reached through a
 * LOCAL cast (`VertRing_57CC8 *poly = (VertRing_57CC8 *)arg0;`) with the
 * parameter still declared `u8 *arg0`. The extra copy reorders the prologue
 * param->callee-save moves: ours emits `move s6,a2` before `move s2,a0`, so
 * prev_idx lands in $a2 and the base load in $a0, while target has them
 * swapped ($a0 / $a2). All 10 diffs are that one register swap. Fix: type the
 * PARAMETER, do not cast into a local. See candidate.c.
 */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 radius;
    u8 count;
    s16 *verts;
} Poly_57CC8;

void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    Poly_57CC8 *poly = (Poly_57CC8 *)arg0;
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    u16 cy;

    prev_idx = arg1 - 1;
    cx = poly->verts[arg1 * 2];
    cy = poly->verts[arg1 * 2 + 1];

    if ((s16) prev_idx < 0) {
        prev_idx = poly->count - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)poly->count) {
            next_idx = 0;
        }
    }

    ang_prev = ratan2(poly->verts[(s16) prev_idx * 2] - (s16) cx, poly->verts[(s16) prev_idx * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(poly->verts[(s16) next_idx * 2] - (s16) cx, poly->verts[(s16) next_idx * 2 + 1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = poly->radius * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(*(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
