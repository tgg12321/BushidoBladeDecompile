/* REJECTED s4 — variants tried for the a0-clamp register-choice tie (score 3
 * residual: target keeps the masked a0 in $a0 for the sb store; ours keeps
 * it in $v0). All measured FLAT or WORSE than the baseline candidate.c on
 * the s4 chassis (target_insns=200):
 *
 * 1) a0 = (u8)a0; if (a0 >= 0x18) {...} else { _SsVmMaxVoice = a0; }
 *    -> score 3 (same numeric score) but shape REGRESSED: introduced a
 *    NEW source-level hunk (signed `slti` instead of target's unsigned
 *    `sltiu`) that wasn't present in the accepted candidate. Self-truncating
 *    the parameter changed the compiler's inferred sign/width for the
 *    later compare. Rejected — worse structurally at equal score.
 *
 * 2) if ((u8)a0 < 0x18) { _SsVmMaxVoice = a0; } else { _SsVmMaxVoice = 0x18; }
 *    (reverting to the pre-s4 if/else branch order)
 *    -> score regressed 3 -> 5. Rejected.
 *
 * 3) { u8 vv = a0; if (vv >= 0x18) {...} else { _SsVmMaxVoice = vv; } }
 *    (explicit named u8 temp instead of inline (u8)a0 cast)
 *    -> build_insns went 200 -> 201 (regressed instruction count; score
 *    stayed 3 only because the score metric didn't count the extra insn
 *    the same way, but this is a worse chassis than the accepted form).
 *    Rejected.
 */
