/* func_800692C0 — pin-free / no-reorder pure-C form. sandbox --disable all = 9.
 * The 9 diffs are exactly TWO coupled effects (see notes.md / meta.json):
 *   (1) `li t6,1` (hoisted loop-invariant const 1) scheduled position:
 *       target = right after `sll a1,a1,0x4`; ours = after `lui/addiu t4` (p).
 *   (2) sum/bitpos register swap: target sum=$t2($10), bitpos=$t1($9); ours reversed.
 * HEAD "matches" only via register asm("$10")/asm("$9") pins + a regfix
 * `reorder 6,3,4,5 @ 3-6` (scheduling cheat). Both forbidden. This is the
 * cheat-free floor. */
s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 sum;
    s32 i;
    s32 a3_off;
    s32 bitpos;
    u32 *p;
    u32 maskA;
    u32 maskB;
    u32 v;
    s32 c;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    a3_off = 0;
    bitpos = 0;
    p = &D_800A32D0;

    do {
        s32 idx4;
        arg3 = (s16 *)((s32)arg3 + a3_off);
        v = i * 4;
        maskB = *p << arg1;
        idx4 = v;
        maskA = *(u32 *)((s32)&D_800A32C8 + idx4) << arg1;
        if (*arg2 == 0) {
            v = *arg0;
            if (v & maskA) {
                *arg3 = 1;
            } else if (v & maskB) {
                c = -1;
                *arg3 = c;
            }
        } else {
            v = *arg0;
            if (v & maskA) {
                c = 6;
                *arg2 = c;
            } else if (v & maskB) {
                c = -6;
                *arg3 = c;
            }
            sval = *arg2;
            if (sval >= 6) {
                sum += 1 << bitpos;
                *arg3 = 0;
                *arg2 = 0;
            } else if (sval < -5) {
                c = 2;
                sum += c << bitpos;
                *arg3 = 0;
                *arg2 = 0;
            }
        }
        a3_off += 2;
        bitpos += 0x10;
        p++;
        i++;
        *arg2 = (u16)*arg2 + (u16)*arg3;
        arg2++;
    } while (i < 2);

    return sum;
}
