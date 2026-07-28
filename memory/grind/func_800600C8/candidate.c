/* func_800600C8 — MATCH FORM s2: sandbox --disable all = 0, 121/121 insns.
 * 100% cheat-free plain C. APPLIED IN src/text1b.c as of s2 end.
 *
 * The three ordering levers that closed 6 -> 0 (all statement order, no
 * constructs; mechanism = GCC 2.7.2 sched.c REVERSE list scheduler +
 * flow.c single-slot last_mem_set backward DSE scan):
 *  1. s.p0 = &D_8009B6FC BEFORE the d1/d0 init pair (p0-store gets the
 *     LOWEST LUID of the tied leaf-store group; reverse scheduler picks
 *     highest-LUID first = emits lowest-LUID first -> lui/addiu/sw18
 *     emit before the pair).
 *  2. Init order s.d1 = hi; s.d0 = hi; (chain order == emit order among
 *     priority-1 class-3 ties -> sh 0x4A then sh 0x48, target order).
 *  3. Overwrite order s.d1 = hi % 10; BEFORE s.d0 = arg0 % 10; — this is
 *     what makes (2) flow-safe: backward DSE scan visits 109(fp+72-ow)
 *     first, then 141(fp+74-ow) resets last_mem_set to fp+74, so when the
 *     scan reaches d0-init(fp+72) the tracker holds fp+74 -> not dead.
 *     With the old ow order (d0-ow last in scan direction) d0-init died
 *     whenever it was the last pre-ow store to fp+72.
 *     The divmod interleave is dataflow/priority-driven and survived the
 *     statement reorder byte-identically.
 */
typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 zero1C;
    s32 pad20;
    s32 pad24;
    s8 byte28;
    s8 padpad[7];
    s16 d0;
    s16 d1;
} S60C8;
s32 func_800600C8(s32 arg0, s32 arg1, s32 arg2)
{
    S60C8 s;
    s32 dist_off = arg1 + 0xB4;
    s32 end_off = arg1 + 0xC0;
    s32 cur_tex = arg1;
    s32 i;
    s16 hi;

    s.p0 = &D_8009B6F0;
    s.byte28 = 0;
    s.zero10 = 0;
    s.zero1C = 0;
    s.arg2 = arg2;
    if (arg0 < 0xA) {
        s.width = 0x93;
    } else {
        s.width = 0xA3;
    }
    s.p1 = &D_8009B758;
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    hi = arg0;
    s.p0 = &D_8009B6FC;
    s.d1 = hi;
    s.d0 = hi;
    hi = ((s16)arg0) / 10;
    s.d1 = hi % 10;
    s.d0 = ((s16)arg0) % 10;
    i = 0;
loop_60C8:
    s.p1 = (s32 *)((s32)&D_8009B708 + ((&s.d0)[i] * 8));
    if (arg0 < 0xA) {
        s.width = 0x64;
    } else {
        s.width = (((1 - i) << 2) << 3) + 0x54;
    }
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    if (s.d1 != 0) {
        i += 1;
        if (i < 2) goto loop_60C8;
    }
    initTexPage(dist_off, 1, 0, saMotionSet((s32 *)&D_8009B6F0, 0), 0);
    ot_Link(D_800A374C + (arg2 * 4), dist_off);
    return end_off - arg1;
}
