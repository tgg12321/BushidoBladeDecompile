/* [s9] REJECTED — the session-9 single-local closing form with the base spelled
 * using the function's OWN mainline a0_ptr idiom instead of the scaled-index
 * integer cast. Measured 2026-08-25: score 1, target_insns 248, build_insns 248.
 * The lone residual is `addu v0,s0,v0` where target has `addu v0,v0,s0` — an
 * independent re-confirmation of [s8r.4] on the s9 chassis. The closing form
 * REQUIRES `(s32 *)((s3 << 2) + (s32)s0)`; `(s32 *)((u8 *)s0 + (s3 << 2))` and
 * `&s0[s3]` both cost exactly one diff on their own.
 */
    case 3: {
        s32 *hp = (s32 *)((u8 *)s0 + (s3 << 2)) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        g_stage_variant = 1;
        break;
    }
