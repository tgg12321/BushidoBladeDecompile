/* candidate.c - func_80089A48 (s1, 2026-09-06) - sandbox 0/178 on the HEAD chassis.
 * TWO pieces: (1) the declaration block below REPLACES src/main.c's
 *   `extern volatile u16 D_800F7298[];` (main.c:76 at HEAD); (2) the body
 *   replaces INCLUDE_ASM("asm/funcs", func_80089A48). Apply scripts:
 *   tmp/grind/func_80089A48/s1/final_decl.py struct + apply_final.py. */

/* ---- declaration (goes at main.c:76 in place of the volatile u16[] extern) ---- */
/* SPU register block as a flat u16 image (SOTN libspu_internal.h SpuUnion.raw
 * shape). The live MMIO block is reached through D_800A2CDC (_spu_RXX base);
 * D_800F7298 is the RAM shadow that receives the write when D_800A2CD4 bit 0
 * (event queueing) is set. Sibling SpuSetKey addresses this same shadow's
 * KEY-ON/KEY-OFF words as D_800F7420 (= D_800F7298 + 0x188). */
typedef struct {
    volatile u16 raw[0x100];
} SpuRXXRaw;
extern SpuRXXRaw D_800F7298;

/* ---- body ---- */
/* PsyQ LIBSPU SpuSetAnyVoice — the 4.x-era build with the SPU_ENV_EVENT_QUEUEING
 * shadow (D_800A2CD4 bit 0): writes go to the RAM image D_800F7298 and the
 * dirty mask D_800A28A0 gets bit ((addr1 - 0xC6) >> 1); otherwise straight to
 * the MMIO block at D_800A2CDC. C ref for the non-shadow shape: sotn-decomp
 * src/main/psxsdk/libspu/s_sav.c (SpuSetAnyVoice). Callers: SpuSetNoiseVoice
 * (0xCA/0xCB) and SpuSetReverbVoice (0xCC/0xCD). */
s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        var_t0 = ((D_800F7298.raw[addr2] & 0xFF) << 16) | D_800F7298.raw[addr1];
    } else {
        var_t0 = ((((SpuRXXRaw *)D_800A2CDC)->raw[addr2] & 0xFF) << 16) |
                 ((SpuRXXRaw *)D_800A2CDC)->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] |= bits;
            D_800F7298.raw[addr2] |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr1] |= bits;
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] &= ~bits;
            D_800F7298.raw[addr2] &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr1] &= ~bits;
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] = bits;
            D_800F7298.raw[addr2] = (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr1] = bits;
            ((SpuRXXRaw *)D_800A2CDC)->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
