/*
 * REJECTED - func_8003C714 - s13 (2026-09-05), structural modality.
 * DIAGNOSTIC form, never a submission candidate.
 *
 * Purpose: the s12 frontier asked whether loop.cs may_not_move admission gate
 * (loop.c:3037, n_times_set[regno] > 0 && last_set[regno] == 0, with last_set
 * cleared only at a CODE_LABEL or JUMP_INSN, loop.c:3089) could be made to fire
 * on the 0x91A2B3C5 magic, which would keep it in the loop at ZERO insn_count
 * cost. That needs ONE pseudo SET twice in two basic blocks of the loop.
 *
 * MEASURED: score 27, build_insns 118 (+13 emitted instructions vs the 105 of
 * candidate.c). The .loop movable table shows may_not_move did NOT fire - each
 * division site expands its magic into its own FRESH pseudo (84 and 92, each
 * n_times_set 1) and combine_movables (loop.c:1245) MATCHES them instead,
 * raising the surviving movables savings 1 -> 2 and lifetime 1 -> 2, i.e.
 * moving the desirability product 119 -> 476, the wrong way.
 *
 * DEAD because: a C author cannot name the pseudo that holds a divisions magic
 * constant, and cse1 only replaces USES, never adds a second SET to an existing
 * pseudo. Separately, any interior basic-block split needs an emitted branch or
 * an extra referenced label, and the target has exactly one CODE_LABEL
 * (.L8003C754) and one branch (bnez at 8003C83C).
 * See hypotheses.md K39 and evidence.md s13.2.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        if (i != 0) {
            dst[0x21] = *(s32 *)(src + 4) / 1800;
        } else {
            dst[0x21] = (*(s32 *)(src + 4) + 1) / 1800;
        }
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
