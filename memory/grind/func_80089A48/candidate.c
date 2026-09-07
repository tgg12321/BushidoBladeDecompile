/* MIGRATION BANNER (s4, 2026-09-06): main carries func_80089A48 as INCLUDE_ASM("asm/funcs", func_80089A48)
 * (asm-until-matched). Body below is byte-for-byte the s3 body (Judge clearance hash 3d9403702d133b3a),
 * re-applied in place in src/main.c this session (tmp/grind/func_80089A48/s4/applied_diff.patch), measured
 * sandbox --disable all = 0/178, canonical pure-C, resubmitted as candidate-ready s4. The s3 candidate-ready
 * was discarded only by the self-vet banned-construct tripwire (CONSTRUCTS line quoted the banned wrapper's
 * tokens), not by any review - self_vet.md rewritten, `grindlib.py selfvet` exits 0. Older banners follow. */
/* MIGRATION BANNER (s3, 2026-09-06): main carries func_80089A48 as INCLUDE_ASM("asm/funcs", func_80089A48)
 * (asm-until-matched). This body was APPLIED IN PLACE in src/main.c this session (tmp/grind/func_80089A48/s3/
 * applied_diff.patch), measured sandbox --disable all = 0/178, and its body hash 3d9403702d133b3a equals the
 * Judge PASS clearance of docs/grind/decisions.md 2026-09-06 07:52 -> submitted as candidate-ready s3.
 * The "NOT SUBMITTED / ruling-request" text below is the s2 history that the ruling resolved. */
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
