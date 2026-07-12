/* REJECTED s8 (rederive modality, 2026-07-12) -- score REGRESSES to 2 (worse than the
   clean floor's 1). Hypothesis was: a named bound variable used in BOTH the
   comparison AND the store has genuine dual semantic purpose (unlike the
   Judge-forbidden store-only holder), so it might legitimately reach addiu
   without failing the human-programmer test.

   Measured mechanism: this is a DIFFERENT failure mode than the forbidden
   family. Making the comparison operand `min` (a variable) instead of the
   literal `-0x1C00` defeats the `slti $v0, $v0, -7168` immediate-compare
   encoding the target uses -- the compiler must now load `min` into a
   register for the comparison, changing the branch's instruction shape
   entirely (not just the store's constant-materialization). Net effect:
   worse, not better. Confirms the target's comparison genuinely needs a
   literal RHS, which forecloses "named threshold constant used for both
   compare and store" as a viable non-cheat lever for this residual. */
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
            {
                s16 min = -0x1C00;
                if ((s16)g_file_vram_timer < min) {
                    g_file_vram_timer = min;
                }
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
