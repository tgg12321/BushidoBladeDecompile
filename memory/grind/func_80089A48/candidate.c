/* candidate.c - func_80089A48 (s2 recon, 2026-09-06) - sandbox 0/178 on the HEAD chassis
 * (INCLUDE_ASM representation on main; measured this session with the body applied in place of
 * the INCLUDE_ASM line - tmp/grind/func_80089A48/s2/apply.py vB_union reproduces it exactly).
 *
 * NOT SUBMITTED: the s1 single-member struct wrap was BANNED (layer-1 FAIL 2026-09-06 07:25). This
 * form is the SOTN/psyz header-canonical libspu object model instead and is filed as a
 * ruling-request, not a candidate-ready. TWO pieces:
 *   (1) DECLARATION (replaces main.c's `extern volatile u16 D_800F7298[];` at line 76; the existing
 *       SpuRXX typedef from main.c ~2449 moves up unchanged so the union can name it):
 *
 *       typedef struct {            // main.c's existing SpuRXX (unchanged)
 *           u16 pad[196];
 *           volatile u16 key_on[2];
 *           volatile u16 key_off[2];
 *       } SpuRXX;
 *       typedef union {             // sotn-decomp src/main/psxsdk/libspu/libspu_internal.h:165-170
 *           SpuRXX rxx;             //   (SpuUnion: record view + raw[0x100] register-index view)
 *           volatile u16 raw[0x100];
 *       } SpuUnion;
 *       extern SpuUnion D_800F7298; // _spu_RQ shadow image (4.x SPU_ENV_EVENT_QUEUEING)
 *
 *   (2) BODY below (replaces INCLUDE_ASM("asm/funcs", func_80089A48)). D_800F7420 stays a separate
 *       extern: merging it as D_800F7298.rxx.key_on[k] measured SpuSetKey 14 (see evidence.md s2).
 */
s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        var_t0 = ((D_800F7298.raw[addr2] & 0xFF) << 16) | D_800F7298.raw[addr1];
    } else {
        var_t0 = ((((SpuUnion *)D_800A2CDC)->raw[addr2] & 0xFF) << 16) | ((SpuUnion *)D_800A2CDC)->raw[addr1];
    }
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] |= bits;
            D_800F7298.raw[addr2] |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] |= bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] &= ~bits;
            D_800F7298.raw[addr2] &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] &= ~bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            D_800F7298.raw[addr1] = bits;
            D_800F7298.raw[addr2] = (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((SpuUnion *)D_800A2CDC)->raw[addr1] = bits;
            ((SpuUnion *)D_800A2CDC)->raw[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
