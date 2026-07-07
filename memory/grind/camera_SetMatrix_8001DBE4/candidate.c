/*
 * Recon-session s1 PROPOSAL (UNMEASURED — apply and sandbox in probes s2).
 *
 * HEAD form (measured floor 0) uses the forbidden empty-body if:
 *     if (D_800A38F8 > D_800A37A0) { } else { do... while(!(cond)); i=0; do... while(i<15); }
 * Judge-flagged as 'empty-body if(cond){} dead-read' family. Must land WITHOUT
 * that construct.
 *
 * Hypothesis 1 (primary): invert to `if (!(cond)) { loop }`. GCC 2.7.2 jump.c
 * routinely inverts if-then-goto branch sense: `if (!c) body;` -> `if (c) goto
 * skip; body; skip:` which emits `bnez` matching target's sltu/bnez pair. Delay
 * slots (nops on initial skip + loop2 tail; addu s0,$0,$0 as i=0 init on loop1
 * exit) should schedule identically because control-flow shape is unchanged.
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
