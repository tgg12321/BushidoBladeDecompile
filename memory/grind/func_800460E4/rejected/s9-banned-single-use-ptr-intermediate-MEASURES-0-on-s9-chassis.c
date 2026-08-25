/* RESEARCH-ONLY, NOT A CANDIDATE - this is banned_constructs #4/#5 (the pm2/pm1
 * single-use pointer intermediate) re-measured on the SESSION-9 chassis to give
 * the 2026-08-25 escalation decision packet a current, non-inherited number.
 *
 * MEASURED THIS SESSION (s9, 2026-08-25):
 *   `sandbox func_800460E4 --disable all` -> score 0, target_insns 248,
 *   build_insns 248, rules_dropped 10, cheat_asm_stripped 0.
 * i.e. the honest, cheat-invisible pure-C distance is ZERO with this case-3
 * spelling and all ten regfix rules dropped. The same chassis measures 9 with
 * the non-banned candidate body and 35 with the committed rule-era HEAD body.
 *
 * The ONLY delta vs memory/grind/func_800460E4/candidate.c is the case-3 block
 * below. Everything else in the function is byte-identical to target already.
 *
 * It is banked REJECTED, not proposed: layer-1 has ruled this construct a cheat
 * twice for this function (banned_constructs #4 and #5) and a standing judge
 * constraint closes the MEM_IN_STRUCT_P axis in every direction. Do NOT submit
 * it as a candidate. It exists so the owner's decision packet can state the
 * exact consequence of a YES ruling on the case-3 lvalue-fidelity question.
 */
    case 3: {
        s32 *base = (s32 *)((s3 << 2) + (s32)s0);
        s32 *hp = base - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(base[-2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        g_stage_variant = 1;
        break;
    }
