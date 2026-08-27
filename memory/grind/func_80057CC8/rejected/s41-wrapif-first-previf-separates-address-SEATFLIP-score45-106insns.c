/* s41 PROBE D1: wrap-if FIRST (single address def in the wrap-merge block), the PREV-IF then supplies
 * the branch that makes the address cross-block, and the prev-if reuses `cnt` so arg0[3]
 * is read once.  Address pseudo target: 3 refs cross-block -> priority 1*3/L_88 vs arg0
 * 2*4/L_72; `scale` between the calls keeps L_72 short. */
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
    s32 cnt;
    s32 off;
    s16 *table;
    s16 *next_vert;

    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);
    cnt = arg0[3];

    {
        s32 tmp = arg1 + 1;
        off = tmp * 4;
        if ((s16) tmp >= cnt) {
            off = 0;
        }
    }
    next_vert = (s16 *)((s32)table + off);

    prev_idx = arg1 - 1;
    if ((s16) prev_idx < 0) {
        prev_idx = cnt - 1;
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    scale = arg0[2] * 40;
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
