/*
 * s3 (structural): H1 CONFIRMED byte-equivalent to HEAD via cc1 dump (s2 first,
 * re-confirmed s3). s3 additionally confirms H2 (`<=` operator form) and H4
 * (`i++` postincrement) both byte-identical to H1 (diff -> 0 lines) and to
 * HEAD (only jump-label renumbering .L272<->.L271, .L273<->.L272,
 * .L277<->.L276 — all post-assembly bytes match). src edit blocked AGAIN
 * by main_reintegration_lock held by session c56ff61a-f40f-49f8-ad83-
 * 348536c53c90 (same holder as s2, held 11+ min at s3 attempt).
 *
 * HEAD form (empty-if, judge-rejected 'empty-body if(cond){} dead-read' family):
 *     if (D_800A38F8 > D_800A37A0) { } else { LOOPS }
 *
 * Chosen replacement (H1): `if (!(D_800A38F8 > D_800A37A0)) { LOOPS }`
 * Mechanism: GCC 2.7.2 jump.c inverts if-then-goto branch sense; sltu operand
 * order preserved because comparison expression is identical (only enclosing
 * sense negated); i=0 remains sole scheduling candidate for loop1 exit delay slot.
 *
 * Equivalence class (proven byte-identical to H1 via cc1 dump, all safe drop-ins):
 *   H1: !(D_800A38F8 > D_800A37A0) with i += 1;
 *   H2: (D_800A38F8 <= D_800A37A0) with i += 1;
 *   H4: !(D_800A38F8 > D_800A37A0) with i++;
 *
 * Next session must acquire the reintegration lock (or find it released), apply
 * this body to src/code6cac.c:1618-1632, run sandbox to re-verify=0, and
 * report candidate-ready.
 */
void camera_SetMatrix_8001DBE4(void) {
    s32 i;

    if (g_disp_enable != DISP_ACTIVE) {
        return;
    }
    func_8003AA78();
    if (!(D_800A38F8 > D_800A37A0)) {
        do {
            func_8003AA48();
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            func_8003AA48();
            i += 1;
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (i < 15);
    }
    func_8003AAB0();
    gpu_InitDisplay();
    gpu_DisableDisplay();
}
