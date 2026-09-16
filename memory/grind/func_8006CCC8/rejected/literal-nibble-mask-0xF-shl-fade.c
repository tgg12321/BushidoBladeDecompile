/* REJECTED -- func_8006CCC8 -- s7 (solver), 2026-09-16
 * WHY DEAD: score 2 (189/189). `masked = *(rec+0x1A) & (0xF << fade)` with no mask variable. loop.c move_movables hoists the (const_int 15) set (insn 547 in tmp/grind/func_8006CCC8/s7/loop_literal_func.txt) to the END of the outer-loop preheader (after shift = 0, next to the giv init), whereas target's `addiu $s6,$zero,0xF` sits between `move $s1,0` (i = 0) and `move $s5,0` (fade = 0). inverse_compose classify: SCHED (same insns/regs, 3 slots reordered); sched_solver perturb (luid,luid_move depth 2): exactly ONE vector, `luid_move 547 -> immediately before 54` = emit the constant set as a source statement before fade = 0, i.e. the mask variable.
 * measured_on: src/text1b.c working tree, s7 switch + ternary-mask chassis (candidate.c = 0), no FAKE construct present.
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 masked;
    s16 field;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    fade = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] <= 0) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = lim;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((s16 *)((u8 *)D_800A34FC + 0x28))[i] >= lim) {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = 0;
            } else {
                ((s16 *)((u8 *)D_800A34FC + 0x28))[i] = (s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] + 1);
            }
        }

        field = ((s16 *)((u8 *)D_800A34FC + 0x28))[i];
        switch (field) {
        case 0:
        case 1:
        case 2:
            if (*arg1 & (0xF0 << shift)) {
                func_8005C650(0, 0x7F, 0x7F);
                func_8006CBD4(i, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1A) & (0xF << fade);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (0xF << fade);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);
                }
            }
            break;
        }
        fade += 4;
    }
    return ret;
}
