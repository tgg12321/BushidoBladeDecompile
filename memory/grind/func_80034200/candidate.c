/* candidate for func_80034200 (session s1 recon)
 * honest pure-C floor: 21 (target 40 insns, build 37 insns; 3 missing + register renames).
 * this is the CLEAN starting point; the as-inherited src was a register-asm-pin
 * + volatile-pad-array cheat scaffold that scored 16 by coercing GCC's allocator
 * (see rejected/register-asm-pins-plus-volatile-pad.c).
 */
void func_80034200(void) {
    s32 shift = 0;
    s32 i = 0;
    s32 acc = 0;
    s32 count;
    s32 innerBound;
    u8 *base;
    u8 *p;
    u8 *end_p;

    count = D_800A389B;
    g_disp_enable = DISP_LOADING;
    if (count <= 0) {
        goto end;
    }
    innerBound = D_800A3874;
    base = &D_800F65F8;
    do {
        s32 useReal = (i < innerBound);
        p = base;
        end_p = base + 2;
        do {
            s32 v0;
            if (useReal) {
                v0 = ((s32)*p) << shift;
            } else {
                v0 = 3 << shift;
            }
            acc |= v0;
            p++;
            shift += 2;
        } while ((s32)p < (s32)end_p);
        i++;
        base += 2;
    } while (i < D_800A389B);

end:
    D_800A3784 = acc;
}
