/* BEGIN func_8006BD28 */
extern u8 *D_800A36E0;
extern u8 *D_800A36E4;
void func_8006BD28(s32 arg0, s32 arg1, S_6A880 *arg2, s32 arg3) {
    /* sprite-sheet header pointers, two per arg0: the table that word +0x20 of
       the block at *(D_800A34FC + 0x24) points to */
    s32 *sheets;
    /* FAKE: named intermediate (no-new-park-categories entry 6): the sheet's
       8-byte cells start at header + 0xC. Spelled inline, fold
       (tools/gcc-2.7.2/fold-const.c:3685-3737) reassociates header + 0xC + j * 8
       into header + (j * 8 + 0xC) and loop.c strength-reduces that giv into its
       own callee-saved register; the target adds 0xC first and recomputes
       j << 3 each iteration.
       Receipts: memory/grind/func_8006BD28/evidence.md */
    s32 cells;
    s32 i, j, n;

    sheets = *(s32 **)(*(s32 *)(D_800A34FC + 0x24) + 0x20);

    arg2->col_b = 0x30;
    arg2->col_g = 0x30;
    arg2->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        /* FAKE: operand grouping (or-tree-shape-shift carve-out): with
           (sheets + i) + arg0 * 2, loop.c hoists arg0 * 8 alone (the target's
           prologue sll + spill at sp+0x20) and keeps i * 4 + sheets per
           iteration; sheets[arg0 * 2 + i] and (sheets + arg0 * 2)[i] measured
           33 and 51.
           Receipts: memory/grind/func_8006BD28/evidence.md */
        arg2->header = *(sheets + i + arg0 * 2);
        if (arg2->header == -1) return;

        n = (arg0 == 0x12) ? 3 : 1;

        for (j = 0; j < n; j++) {
            arg2->x = 0;
            arg2->y = arg1;
            arg2->ot_idx = 8;
            if (arg0 != 0x12 || j == arg3 || j == 2) {
                arg2->has_color = 0;
            } else {
                arg2->has_color = 1;
            }
            arg2->semi = 0;
            arg2->sprt_out = (s32)D_800A36E4;
            cells = arg2->header + 0xC;
            arg2->table = cells + j * 8;
            D_800A36E4 = (u8 *)func_8007352C((s32)arg2);
        }

        SetDrawMode((s32)D_800A36E0, 1, 0, func_8006E480(arg2->header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 0x20, (s32)D_800A36E0);
        D_800A36E0 += 12;
    }
}
/* END func_8006BD28 */
