/* s42 PROBE H2 (rederive): post-call address formation (target's sll/sra-14/addu shape)
 * combined with arg0 DYING BEFORE the first call -- `scale = arg0[2]*40` hoisted above the
 * prev-if so arg0's last reference is the wrap test.  The carried set then has EIGHT
 * members (cys, cxs, raw cx, raw cy, table, next-index, arg2, arg3), matching the target's
 * eight callee-saves, instead of the nine that e1/g1's carried-arg0 forms need. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;
    s32 off;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);
    scale = arg0[2] * 40;

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        off = tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    next_vert = (s16 *)(off + (s32)table);
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
