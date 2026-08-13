/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 24 at 51 build insns; the floor is 18 at 51.
 *
 * True if/else diamond (`if (c) val2 = val|K; else val2 = val;`).
 * Tested the s6 hypothesis that moving BOTH assignments after the branch
 * would free $v0 for the selected value.  It does remove the val2/c allocno
 * conflict (.greg: `75 conflicts: 72 74 75 2 29` — no 76) but val2 STILL
 * lands in $v1, because the conflict that actually binds is with HARD REG 2,
 * not with the condition's pseudo.  Score 24 vs the floor's 18.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/ifelse/
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    if (c) {
        val2 = val | 1;
    } else {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    if (c) {
        val2 = val | 2;
    } else {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    if (c) {
        val2 = val | 4;
    } else {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
