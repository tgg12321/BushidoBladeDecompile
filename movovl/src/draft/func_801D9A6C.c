/*
 * MOVOVL.EXE game code — func_801D9A6C @ 0x801D9A6C (264B)
 * Probable role (hypothesis from callees/strings, NOT evidence-verified):
 *   ring-buffer frame fetch: StGetNext consumer
 *
 * m2c FIRST DRAFT (m2c --valid-syntax over movovl/asm/game.s).
 * NOT byte-verified: the overlay has no build integration yet, so no
 * compile/link/SHA1 oracle exists for this code. Treat every line as a
 * sketch. Requires m2c_macros.h (M2C_UNK / M2C_FIELD) to compile.
 */

s32 StGetNext(s32 *, void **);                      /* extern */
extern s32 D_801F65D0;
extern void *D_801F65E4;
extern u32 D_801F661C;
extern u32 D_801F6624;
extern M2C_UNK D_FFFFFFF;

s32 func_801D9A6C(void) {
    void *sp14;
    s32 sp10;
    M2C_UNK *var_s0;
    u16 temp_a0;
    u32 temp_v1;
    u32 temp_v1_2;
    u32 var_a0;

    var_s0 = &D_FFFFFFF;
loop_1:
    if (StGetNext(&sp10, &sp14) != 0) {
        if (var_s0 == NULL) {
            return 0;
        }
        goto loop_1;
    }
    temp_v1 = M2C_FIELD(sp14, u32 *, 8);
    if (temp_v1 >= (u32) D_801F6624) {
        M2C_FIELD(D_801F65E4, s8 *, 6) = 0;
    } else if (temp_v1 >= (u32) D_801F661C) {
        D_801F65D0 = 1;
    }
    M2C_FIELD(D_801F65E4, u16 *, 8) = (u16) M2C_FIELD(sp14, u16 *, 0x10);
    M2C_FIELD(D_801F65E4, u16 *, 0xA) = (u16) M2C_FIELD(sp14, u16 *, 0x12);
    if (M2C_FIELD(D_801F65E4, u8 *, 1) != 0) {
        temp_v1_2 = (s16) M2C_FIELD(D_801F65E4, u16 *, 8) * 3;
        var_a0 = (u32) (temp_v1_2 + (temp_v1_2 >> 0x1F)) >> 1;
    } else {
        var_a0 = (u32) M2C_FIELD(D_801F65E4, u16 *, 8);
    }
    M2C_FIELD(D_801F65E4, s16 *, 0x18) = (s16) var_a0;
    M2C_FIELD(D_801F65E4, s16 *, 0x10) = (s16) var_a0;
    temp_a0 = M2C_FIELD(D_801F65E4, u16 *, 0xA);
    M2C_FIELD(D_801F65E4, u16 *, 0x1A) = temp_a0;
    M2C_FIELD(D_801F65E4, u16 *, 0x12) = temp_a0;
    M2C_FIELD(D_801F65E4, u16 *, 0x22) = (u16) M2C_FIELD(D_801F65E4, u16 *, 0xA);
    return sp10;
}
