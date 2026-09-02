/* s8: NINETEEN sanctioned-family / ordinary-C respellings measured frame-INERT.
 * Chassis pure-c-floor2-body.c (sandbox --disable all = 2), zero FAKE constructs.
 * vars stayed at 8 for every one of them; none reached the target's 24.
 *   duplicated-statement-into-arms: armdup50 vars=8 bd=9; armdup58 vars=8 bd=4
 *   do-while(0) wrap:               dw0arms  vars=8 bd=0; dw0judge vars=8 bd=10
 *   mixed exit (goto endK+return):  gotoend  vars=8 bd=0
 *   same-value re-store (memory):   dupstore2c vars=8 bd=2
 *   split-read hoist of tbl[4]:     sr_tbl4  vars=8 bd=22
 *   variable-reuse (borrow i):      reuse_i  vars=8 bd=60
 *   named-intermediate:             ni_scale vars=8 bd=0; ni_ang vars=8 bd=165
 *   pointer alias to src vector:    ptr_src  vars=8 bd=0
 *   loop-guard respellings:         loop_amp 49 / loop_cont 0 / loop_neg 3 / loop_swap 10, all vars=8
 *   arm 1+3 merge:                  arm13    vars=8 bd=54
 *   dead scalar local fed by loads: jdeadloc vars=8 bd=0  (loads die before combine -> no orphan)
 *   Judge read multiplied by zero:  jmulzero vars=8 bd=0  (folded in the front end)
 * The file body below is the inert dw0arms shape, kept as a representative.
 */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    do {
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    } while (0);
    *(s32 *)(obj + 0x50) = 1;
    *(u8 *)(obj + 5) = 0;
    *(s16 *)obj = 0;
    return (s32 *)obj;
}
