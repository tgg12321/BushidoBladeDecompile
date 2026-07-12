/* REJECTED s7 — const-qualified local holder does NOT escape the
 * target-type reconversion (scores 1, same as bare literal), unlike
 * the Judge-forbidden plain (non-const) s16 tmp holder (scores 0).
 * Mechanism: c-typeck.c convert_for_assignment() line 3987 calls
 * decl_constant_value() on any VAR_DECL RHS under -O2; that function
 * (c-typeck.c:976) substitutes DECL_INITIAL back in only when
 * TREE_READONLY(decl) is true. const collapses this holder back to
 * a bare literal -- no weaker/differently-qualified variant of the
 * local-holder idea is available; the only working spelling is
 * exactly the Judge-forbidden non-const mutable local.
 */
void func_8001B138(s32 *arg0) {
    D_800FF5C8 = 0;
    D_800FF5CC = 0;
    D_800FF5D0 = 0;
    D_800FF5D8 = 0;
    D_800FF5DA = 0;
    D_800FF5DC = 0;
    D_800FF5E0 = 0;
    if (D_800A38BA != 0 && D_800A3834 == 1) {
        if (*arg0 & 1) {
            D_800A37E0 = 1;
            if (*arg0 & 8) {
                g_file_vram_timer = (u16)(g_file_vram_timer + 0x4CC);
            }
            if (*arg0 & 2) {
                g_file_vram_timer = (u16)(g_file_vram_timer - 0x4CC);
            }
            if ((s16)g_file_vram_timer < -0x1C00) {
                { const s16 tmp = -0x1C00; g_file_vram_timer = tmp; }
            }
            if ((s16)g_file_vram_timer >= 0x7401) {
                g_file_vram_timer = 0x7400;
            }
            *arg0 = *arg0 & ~0xB;
        }
        {
            s32 v;
            v = (s16)g_file_vram_timer;
            if (v < 0) {
                v = v + 0xF;
            }
            D_800FF5E0 = v >> 4;
        }
    }
    *arg0 = *arg0 & ~0x10001;
}
