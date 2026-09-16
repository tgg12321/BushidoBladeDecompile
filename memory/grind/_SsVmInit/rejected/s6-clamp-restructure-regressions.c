/* REJECTED s6 — structural respellings of the a0-clamp if/else that
 * REGRESSED the score (baseline candidate.c if/else form is 3; both below
 * are worse). Kept for the record so a future session doesn't re-try them.
 *
 * 1) Unconditional store then conditional clamp override:
 *    _SsVmMaxVoice = a0;
 *    if ((u8)a0 >= 0x18) { _SsVmMaxVoice = 0x18; }
 *    -> score 3 -> 9; build_insns 199 (lost the 200==200 parity the if/else
 *    form has). Rejected.
 *
 * 2) Ternary expression:
 *    _SsVmMaxVoice = ((u8)a0 >= 0x18) ? 0x18 : a0;
 *    -> score 3 -> 5 (target_insns==build_insns==200, parity kept, but more
 *    scored diffs than the if/else form). Rejected.
 *
 * Also flat (score unchanged at 3, byte-identical hunks — not regressions,
 * but not closes either):
 * 3) _SsVmMaxVoice = (u8)a0; (explicit duplicate cast in the else arm,
 *    instead of bare `a0`) — see hypotheses.md s6 for the RTL-level
 *    explanation (the compare's masked pseudo already dies before the else
 *    arm; duplicating the cast source-side doesn't change which pseudo the
 *    store's RTL references).
 * 4) void _SsVmInit(u32 a0) (parameter type s32 -> u32) — flat, no benefit,
 *    no evidence basis, reverted.
 */
