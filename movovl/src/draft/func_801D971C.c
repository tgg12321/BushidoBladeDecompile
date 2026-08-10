/*
 * MOVOVL.EXE game code — func_801D971C @ 0x801D971C (448B, no jal callers)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   CD-stream interrupt callback (installed by pointer): StCdInterrupt + DecDCTout pump
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

M2C_UNK DecDCTout(s32, s32);                        /* extern */
M2C_UNK StCdInterrupt();                            /* extern */
extern void *D_801F65E4;
extern s32 D_801F6628;
extern s32 D_801F67E8;

void func_801D971C(void) {
    s16 temp_a1;
    s16 temp_v0_2;
    s32 temp_hi;
    u32 temp_lo;
    void *temp_v0;

    if (M2C_ERROR(/* Read from unset register $v0 */) != 0) {
        StCdInterrupt();
        D_801F67E8 = 0;
    }
    if ((M2C_FIELD(D_801F65E4, u8 *, 5) != 0) && (temp_hi = (s16) M2C_FIELD((D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8)), s16 *, 0x10) % (s16) M2C_FIELD(D_801F65E4, s16 *, 0x20), (temp_hi != 0))) {
        M2C_FIELD(D_801F65E4, u8 *, 5) = 0U;
        M2C_FIELD(D_801F65E4, u16 *, 0x1C) = (u16) (M2C_FIELD(D_801F65E4, u16 *, 0x1C) + temp_hi);
    } else {
        M2C_FIELD(D_801F65E4, u16 *, 0x1C) = (u16) (M2C_FIELD(D_801F65E4, u16 *, 0x1C) + (u16) M2C_FIELD(D_801F65E4, s16 *, 0x20));
    }
    temp_v0 = D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8);
    temp_a1 = (s16) M2C_FIELD(D_801F65E4, u16 *, 0x1C);
    if (temp_a1 < (M2C_FIELD(temp_v0, s16 *, 0xC) + M2C_FIELD(temp_v0, s16 *, 0x10))) {
        temp_v0_2 = M2C_FIELD(D_801F65E4, s16 *, 0x22);
        temp_lo = M2C_FIELD(D_801F65E4, s16 *, 0x20) * temp_v0_2;
        DecDCTout(D_801F6628 + ((M2C_FIELD(D_801F65E4, u8 *, 3) * 0x38400) + 0x50000) + (temp_a1 * temp_v0_2 * 2), (s32) (temp_lo + (temp_lo >> 0x1F)) >> 1);
        return;
    }
    M2C_FIELD(D_801F65E4, s8 *, 0) = 1;
    M2C_FIELD(D_801F65E4, u8 *, 5) = 1U;
    M2C_FIELD(D_801F65E4, u8 *, 4) = (u8) (M2C_FIELD(D_801F65E4, u8 *, 4) == 0);
    M2C_FIELD(D_801F65E4, u16 *, 0x1C) = (u16) M2C_FIELD((D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8)), u16 *, 0xC);
    M2C_FIELD(D_801F65E4, u16 *, 0x1E) = (u16) M2C_FIELD((D_801F65E4 + (M2C_FIELD(D_801F65E4, u8 *, 4) * 8)), u16 *, 0xE);
}
