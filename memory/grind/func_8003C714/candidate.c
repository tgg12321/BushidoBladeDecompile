/*
 * s18 (2026-09-05, forensics) -- this body is UNCHANGED and is still the best
 * ADMISSIBLE form (sandbox score 15, target_insns 104, build_insns 105,
 * re-measured this session on the current chassis). Two things about it are now
 * superseded as CHASSIS, not as bytes:
 *
 *   (1) `src = (u8 *)&D_80106A58 + i * 8;` should become
 *       `base = (u8 *)&D_80106A58;` before the loop plus `src = base + i * 8;`
 *       inside it. That is byte-neutral (measured score 15 / build 105,
 *       identical to this body) and it REMOVES the &D_80106A58 movable from
 *       loop.c's list entirely, which deletes s11 H17's admissible insn_count
 *       window [120,122]: with the base movable gone the 0x91A2B3C5 magic is
 *       the FIRST movable and the requirement collapses to a one-sided
 *       `insn_count > threshold`. See hypotheses.md H30.
 *   (2) `threshold` is NOT chassis-fixed at 122. loop.c:532 halves it to 61
 *       whenever the loop contains a CALL_INSN at loop_optimize time
 *       (loop.c:2202), and a CALL_INSN inside a DEAD libcall block is deleted
 *       whole by flow.c before combine and register allocation, so it costs no
 *       emitted bytes. s8's rejected/threshold-term-only-movable-by-cheat-or-
 *       by-a-call-in-the-loop.c dismissed this on the premise that a call in the
 *       loop necessarily emits a jal; that premise is false for a dead libcall.
 *       See hypotheses.md H29.
 *
 * Together those two facts take the distance-0 carrier requirement from s17's
 * THIRTEEN dead statements to ONE, frame-exact:
 *     v = (long long)((float)*(s32 *)(src + 4) / 30.0f);
 * measures sandbox score 0 / target 104 == build 104 / frame 0x20 on the
 * shipped chassis (hypotheses.md H31, banked at rejected/loop-has-call-halves-
 * threshold-one-dead-fixsfdi-carrier-d0-but-rhs-unnatural.c). It is NOT in this
 * body because its admissibility is an open ruling question: the store itself is
 * the sanctioned store-level-dead defensive-init shape (v is unconditionally
 * re-assigned `v = *src;` before any read), but the float/long long cast chain
 * in the RHS exists only to make GCC emit a __fixsfdi CALL_INSN. s18 returned a
 * ruling-request on exactly that.
 *
 * Also unchanged: ANY distance-0 body here still carries the two declaration
 * puns (D_80106A58 splat sub-symbol, *(u16 *)&D_80101ED2) and needs the
 * include/*.h aggregate merge routed as an integration handoff before it can be
 * submitted at all.
 */
