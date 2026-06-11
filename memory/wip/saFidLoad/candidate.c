/* saFidLoad candidate — PASS-vetted levers 1+2 ONLY (tail lever REJECTED,
 * see rejected/). Floor NOT yet measured from this base — measure first.
 * Lever 1: volatile coercion cast removed (vv = *p plain re-read).
 * Lever 2: idx/base named locals (SOTN named-intermediate family) — retires
 *          the reorder 13,12,14 prologue rule.
 * Tail: ORIGINAL form (the 5-6 tail rules still cover the shared-sext tail).
 */
s32 saFidLoad(s32 arg0, s16 arg1) {
    s32 idx;
    u8 *base;
    s32 **p;
    s32 *v;
    s32 *vv;
    s16 ret;
    title_mv_exec2(0);
    idx = arg1;
    base = (u8 *)&D_800EFC38;
    p = (s32 **)(base + idx * 4);
    v = *p;
    if (v != 0) {
        v = (s32 *)((u8 *)v + arg0);
        *p = v;
        *v = *v + arg0;
        vv = *p;
        *(s32 *)((u8 *)vv + 4) = *(s32 *)((u8 *)vv + 4) + arg0;
        func_80087F64(idx);
        ret = coli_CheckBukiPreHit_800880B8(*(s32 *)((u8 *)*p + 4), idx, *(s32 *)((u8 *)&D_800EFB38 + idx * 4));
        if (ret == idx) {
            return (s16)tslCDFileRead(ret);
        }
        return ret;
    }
    return (s16)-1;
}
