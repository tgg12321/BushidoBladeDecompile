/*
 * s2 (structural modality): H1 CONFIRMED byte-equivalent via out-of-tree cc1
 * dump — src edit blocked by main_reintegration_lock held by another session.
 *
 * HEAD form (empty-if, judge-rejected 'empty-body if(cond){} dead-read' family):
 *     if (D_800A38F8 > D_800A37A0) { } else { LOOPS }
 *
 * H1 (inverted branch sense, non-empty body): `if (!(D_800A38F8 > D_800A37A0)) { LOOPS }`
 * Mechanism: GCC 2.7.2 jump.c inverts if-then-goto branch sense; sltu operand
 * order preserved because comparison expression is identical (only enclosing
 * sense negated); i=0 remains sole scheduling candidate for loop1 exit delay slot.
 *
 * Measurement (s2, this session):
 *  - Sandbox on HEAD (empty-if form): score=0, 51/51 insns, scorable.
 *  - Attempted src/code6cac.c edit → blocked by main_reintegration_lock
 *    (holder session_id=c56ff61a-f40f-49f8-ad83-348536c53c90). Never touched
 *    the lock (role rule).
 *  - Compared cc1-emitted .s for HEAD vs H1 out-of-tree
 *    (tmp/grind/camera_SetMatrix_8001DBE4/s2/HEAD.func.s vs H1.func.s):
 *    only diff is jump-label numbering (.L272↔.L271, .L273↔.L272, .L277↔.L276).
 *    Every instruction, register, and operand order matches; PC offsets identical
 *    after assembly. Sandbox=0 on H1 is proven by construction.
 *
 * Next session must acquire the reintegration lock (or it's released), apply
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
