/* func_80047A90 — BEST FORM, session 2 (structural), honest sandbox floor 8 (down from 14).
 *
 * THIS FORM IS APPLIED IN src/sound.c (pure C, zero pins, zero rules).
 *
 * Session-2 change vs the s1 14-form: the inner-loop tail (pa1++; a3 += 4; pa2++;)
 * is duplicated into BOTH arms of the if (k == 8) — the sanctioned
 * duplicated-statement-into-arms family (.claude/rules/duplicated-statement-into-arms.md,
 * owner rulings 2026-07-01 + 2026-08-06 control-transfer-tail clarification;
 * FAKE-annotated in src). Effect (measured, final8.lreg):
 *   pa1 7->9 refs (prio .794), pa2 4->6 (.375), a3off 4->6 (.364), k live 36->40 (.25)
 *   => allocation order pa1(a1) pa2(a2) a3off(a3) ... k(t0): loop 2 registers ALL
 *   land on target, including the delay-slot addiu a2,a2,4. Cross-jump re-merges the
 *   duplicate byte-neutrally: 84/84 insns, loop-2 body byte-identical to target.
 *
 * Remaining 8 = (a) loop-1 2-cycle i<->judge, 6 slots: i(a2 vs t0), judge-base(a3 vs a2);
 *               (b) loop-2 init order, 2 slots: target emits k=1 BEFORE the pt2 lui pair
 *                   (insns 30-33), we emit it after (source order pt2;pt1;k=1 is
 *                   register-load-bearing: k-first=18, k-middle=15 measured s2).
 * See evidence.md s2 for why (a) is closed to every priority-family spelling and what
 * the s3 frontier is (forensics on find_reg order / permuter sweep / k=1 scheduling).
 */
void func_80047A90(void) {
    s32 i;
    s32 k;
    s32 a3;
    s32 v1;
    s32 a0;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    for (i = 0; i < 0x11; i++) {
        D_800EF59C[i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }

    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
    k = 1;
  outer_loop:
    pa1 = pt1;
    a3 = 0;
    pa2 = pt2;
    pt3 = pt1 + 0x11;
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (k == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges,
         * byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3, k->$t0 */
        pa1++;
        a3 += 4;
        pa2++;
    }
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    k++;
    pt1 += 0x11;
    if (k < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}
