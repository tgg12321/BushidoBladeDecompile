/* REJECTED — s12: hoisted bases re-typed `s32 (*)[]` so the element access is a real
ARRAY_REF (expand sets MEM_IN_STRUCT_P) instead of an INDIRECT_REF = 7/91,
byte-identical to the candidate. m1/m2/m4/m5/m6 (each base separately, the
index as `u8 (*)[2]`, all three together, arg4-only) are all 7 as well, so
MEM_IN_STRUCT_P / the alias class of the table MEMs is a DEAD axis here.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 (*tbl_11dc)[];
    u8 *idx_1494;
    s32 (*tbl_125c)[];
    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = &D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = &D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    do {
        s32 arg4;
        tslTm2LoadImage_2(&D_800161B8);
        arg4 = (*tbl_125c)[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, (*tbl_11dc)[D_800A11D5], arg4,
                     (*tbl_125c)[idx_1494[1]]);
        cdrom_ClearIrq();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
