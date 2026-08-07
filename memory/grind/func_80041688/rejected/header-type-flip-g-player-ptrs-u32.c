/* REJECTED — s14 header-type-correction u32 sign flip: MEASUREMENT INERT + FOUR-PRONG-TEST FAILS.
 *
 * Form: `extern u32 g_player_ptrs[];` (both site: text1a.c:163, text1a.c:898).
 *
 * Sandbox measurement: score=2, target_insns=82, build_insns=82, rules_dropped=3,
 * cheat_asm_stripped=23 — BYTE-IDENTICAL to baseline.  s32 -> u32 is inert for the
 * FALSE-arm color-lbu emission-order divergence.
 *
 * Four-prong test per [[header-type-correction-from-use-sites]]:
 *
 *   (a) FAIL — codebase-wide use-site grep (12 hits in src/text1a.c):
 *       - g_player_ptrs[i] = 0;          type-agnostic
 *       - g_player_ptrs[a0] = (s32)ptr;  type-agnostic (both s32 and u32 hold ptr bits)
 *       - (s32*)((u8*)g_player_ptrs + a0*4)   pointer arith, type-agnostic
 *       - return g_player_ptrs[a0];      type-agnostic (implicit conv into s32 return)
 *       - (s16*)g_player_ptrs[a0]        int->ptr cast, type-agnostic
 *       - if (g_player_ptrs[i] == 0)     type-agnostic
 *       - (s32*)g_player_ptrs[arg0]      int->ptr cast, type-agnostic (this fn's site)
 *       ZERO signed-specific-semantics sites: no `< 0`, no arithmetic shift, no
 *       signed clamp, no round-toward-zero idiom.  "Absence of contradiction is
 *       not evidence of correctness" (rule line 45).  Prong (a) fails.
 *
 *   (b) FAIL — the `(s32)ptr` cast at line 182 is not functionally necessary
 *       (assigning ptr to s32/u32 lvalue produces identical bytes on this ABI;
 *       cast only silences a warning).  The `(s32*)` / `(s16*)` reads are
 *       int->ptr conversions that are needed under EITHER type.  No cast is
 *       functionally necessary under s32 vs. u32.  Prong (b) fails.
 *
 *   (d) FAIL — flipping to u32 eliminates ZERO casts; every current cast
 *       remains needed under u32 (int->ptr casts on read, s32 vs. u32 assign
 *       is a warning under either direction).  The "correction" is not a
 *       whole-tree simplification.  Prong (d) fails.
 *
 * Mechanistic disconnection: s6 CONFIRMED the divergence mechanism as sched.c
 * insn_priority hazard-tag on pseudo-78 (source-level `b`) in BB18.  Changing
 * g_player_ptrs' element type from s32 to u32 does not alter (i) pseudo 78's
 * live range, (ii) sched1's chain-length priority on the three color-lbu's,
 * or (iii) combine/CSE behavior on the FALSE-arm `(u8*)player + 0x18/19/1A`
 * accesses.  `player` is loaded once at entry and thereafter aliased via u8*;
 * the source type of the array-element cast has no downstream RTL effect on
 * BB18.  Measurement result byte-identical to baseline confirms the axis is
 * mechanistically disconnected from the s6-CONFIRMED lever.
 */
