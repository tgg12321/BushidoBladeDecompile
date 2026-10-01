extern u8 *D_800A36E0;
extern u8 *D_800A36E4;
void func_8006BD28(s32 arg0, s32 arg1, S_6A880 *arg2, s32 arg3) {
    s32 *sheets;
    s32 cells;
    s32 i, j, n;

    sheets = *(s32 **)(*(s32 *)(D_800A34FC + 0x24) + 0x20);

    arg2->col_b = 0x30;
    arg2->col_g = 0x30;
    arg2->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        arg2->header = sheets[arg0 * 2 + i];
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
