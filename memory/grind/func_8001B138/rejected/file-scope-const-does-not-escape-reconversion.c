/* REJECTED — s3, 2026-07-12. Not a cheat (no semantic-purpose objection), simply
 * does not close the gap: measured sandbox --disable all score 1 (same as the
 * plain-literal clean floor). A file-scope static const does NOT escape the
 * u16-target-type reconversion the way a block-local variable does (see s2's
 * CONFIRMED-but-forbidden local-variable mechanism). Confirmed via objdump:
 * the store still assembles to `ori $v0,$zero,0xe400`, not target's
 * `addiu $v0,$zero,-0x1C00`. Banked as a dead frontier lead, not as a cheat.
 */

static const s16 D_VRAM_TIMER_MIN = -0x1C00;

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
                g_file_vram_timer = D_VRAM_TIMER_MIN;
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
