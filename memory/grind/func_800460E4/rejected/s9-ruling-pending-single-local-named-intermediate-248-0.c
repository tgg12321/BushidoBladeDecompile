/* [s9] RULING-PENDING, NOT A SUBMITTED CANDIDATE — the case-3 closing form that
 * measures ZERO on the session-9 chassis with a SINGLE fresh local.
 *
 * MEASURED THIS SESSION (2026-08-25, `sandbox func_800460E4 --disable all`):
 *   score 0, target_insns 248, build_insns 248, rules_dropped 10,
 *   cheat_asm_stripped 0.
 * Same chassis: committed rule-era HEAD body = 35; non-banned candidate.c
 * body = 9; this form = 0. BYTE-NEUTRAL (248 == 248) — no cross-jump, no
 * missing instruction, all seats target-exact.
 *
 * WHY THIS IS NOT banned_constructs #4/#5 (pm2/pm1), verbatim differences:
 *   - #4/#5 respelled BOTH stage-header words through TWO pointer intermediates
 *     (`pm2`/`pm1`, or `base`+`hp`). This form has ONE local and respells ONE
 *     word; the -8 word keeps the function's own `s0[s3 - 2]` array-index idiom,
 *     identical to case 13's byte-exact spelling.
 *   - The shared-base variant (`base` written once but READ TWICE) fails prong
 *     (1) of the named-intermediate entry. Here the base expression is inlined
 *     into hp's initializer, so `hp` is the ONLY local and it is once-written,
 *     once-read.
 *   - s8 measured the closest previously-tried shape — one single-use pointer,
 *     -8 word left as the array idiom — at 249/8, but spelled `&s0[s3 - 1]`.
 *     The scaled-index cast base is what recovers the 248. That combination
 *     (cast base + inlined `- 1` + kept array form for the -8 word) had never
 *     been measured before this session.
 *
 * PRONG STATUS against no-new-park-categories.md:193-214 (named intermediate,
 * owner clarification 2026-08-17) — all six measured/satisfiable:
 *   (1) once-written, once-read           — YES (sole write is the init; sole read is *hp)
 *   (2) real value in the target's bytes  — YES (target: sll v0,s3,2 / addu v0,v0,s0 /
 *       lw a0,-4(v0); the address is target's own, the loaded word is s4's stage pointer)
 *   (3) byte-neutral                      — YES, build_insns 248 == target_insns 248
 *   (4) fresh local, not a borrow         — YES
 *   (5) dest not live-pre-initialized     — YES (fresh block scope)
 *   (6) dump-proven mechanism + exhaustion + FAKE + layer-1/2 — mechanism is
 *       dump-proven in evidence.md [s6]/[s7]; exhaustion is 9 sessions /
 *       53+ rejected forms / the source-level enumeration [s8r.2]; the FAKE
 *       line below is the annotation; reviews are the open item.
 *
 * WHY IT IS BANKED REJECTED ANYWAY: state.json judge_constraints carries a
 * LATER standing constraint (2026-08-25 09:06 ruling) reading "No spelling of
 * any lvalue in this function - global declaration type, second handle, pointer
 * intermediate, or cast - may be chosen to change MEM_IN_STRUCT_P / the sched.c
 * anti_dependence exemption; the /s axis is closed for func_800460E4 in every
 * direction." That names pointer intermediates explicitly. The EARLIER 06:39
 * ruling, by contrast, expressly left this route open: "NOT closed: a
 * byte-neutral (248-insn) fresh named pointer local holding a real consumed
 * address stays available under the named-intermediate entry's prongs -- but
 * the measured nv routes are 249 insns, so they fail byte-neutrality and close
 * nothing." The 248-insn form that ruling supposed did not exist now exists and
 * measures 0. Session 9 therefore did NOT submit this; it filed the conflict as
 * a decision packet (docs/grind/decisions.md, 2026-08-25 escalation entry).
 * DO NOT submit this form as a candidate until that packet is ruled on.
 *
 * The base spelling is load-bearing and re-confirmed this session: replacing
 * `(s32 *)((s3 << 2) + (s32)s0)` with the mainline a0_ptr idiom
 * `(s32 *)((u8 *)s0 + (s3 << 2))` measures 248/1 — the lone diff is
 * `addu v0,s0,v0` vs target's `addu v0,v0,s0` ([s8r.4], re-measured at s9).
 *
 * Apply on top of memory/grind/func_800460E4/candidate.c, replacing its case 3.
 */
    case 3: {
        /* FAKE: fresh once-written/once-read pointer intermediate naming the
         * address of the stage header's last word, mechanism: expand-time
         * MEM_IN_STRUCT_P (expr.c:4567-4577) -> sched.c anti_dependence
         * exemption -> sched1 load/store order, lever-exhaustion:
         * memory/grind/func_800460E4/hypotheses.md + evidence.md [s1]-[s8r] */
        s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        g_stage_variant = 1;
        break;
    }
