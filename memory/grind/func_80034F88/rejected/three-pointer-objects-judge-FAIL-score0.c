/* JUDGE FAIL 2026-08-13 16:36 — THREE pointer objects (qm/q1/q2) aliasing
 * &D_80106A73.  Measures honest score 0 at 49/49 insns, but ruled the banned
 * four-handle construct respelled (checklist T1/T2/T3/T5): q1/q2 are
 * value-redundant with qm and exist only to raise the allocno count so
 * global.c allocno_compare/find_reg land the target $v1/$a0 base split.
 * DO NOT RE-PROPOSE, in this or any other handle count > 1.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 va;
    s32 vb;
    s32 vc;
    s32 ca;
    s32 cb;
    s32 cc;
    u8 *qm;
    u8 *q1;
    u8 *q2;

    p = func_80077D00();
    qm = &D_80106A73;
    *qm &= 0xF8;

    {
        q1 = &D_80106A73;
        ca = p[8] & 1;
        va = *q1;
        if (ca) {
            ca = va | 1;
        } else {
            ca = va;
        }
        *q1 = ca;
    }

    {
        q2 = &D_80106A73;
        cb = p[8] & 2;
        vb = *q2;
        if (cb) {
            cb = vb | 2;
        } else {
            cb = vb;
        }
        *q2 = cb;
    }

    {
        q2 = &D_80106A73;
        cc = p[8] & 4;
        vc = *q2;
        if (cc) {
            cc = vc | 4;
        } else {
            cc = vc;
        }
        *q2 = cc;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
