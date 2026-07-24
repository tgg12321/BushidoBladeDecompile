/* REJECTED (s2) — the same-lvalue dual-spelling, but NOW measured at DISTANCE 0.
 *
 *   ofs = i*12;   (reused ofs -> index in v1, s2 register-flip lever)
 *   p = (s32 *)((u8 *)&D_800F1198 + ofs);
 *   p[2] = 0;                                // col c -> 8(v0)
 *   p[1] = 0;                                // col b -> 4(v0)
 *   *(s32 *)((u8 *)&D_800F1198 + ofs) = 0;   // col a re-spelled as symbol+index
 *
 * s1 measured this dual-spelling at score 12 (index landed in v0, not v1). The s2
 * `ofs`-reuse lever fixes the register allocation (index -> v1), and with that fix
 * this SAME dual-spelling now closes to sandbox distance 0 (build_insns 38 ==
 * target 38). It is the ONLY distance-0 form found.
 *
 * STILL A CHEAT — do NOT propose as candidate-ready. The third store writes the
 * identical lvalue as p[0] (&D_800F1198 + ofs), spelled as the full symbol
 * expression ONLY to force GCC to emit a separate %hi/%lo(1198)+index address
 * (vs reusing base pointer v0). Same row, two spellings, no semantic purpose ->
 * codegen steer, [[inline-asm-injection]]-adjacent (fails 6-test #1/#2/#3/#4).
 * Already in the rejected bank (epilogue-dual-spelling-col0.c, audit-orch3a FAIL).
 *
 * IMPLICATION: a legitimate byte-match is proven to EXIST (distance 0 reachable),
 * but its only known spelling is this cheat. Every consistent pure-C spelling
 * gives full-CSE (col a folds to 0(v0), score 4) or full-recompute (all three
 * separate, score 6). Target's PARTIAL CSE (base pointer for b,c + separate
 * recompute for a) has no non-steer pure-C form. scan_hand_coded = LOW 0/8, so
 * canonical-asm is refused (ordinary GCC addressing artifact, not hand-coded).
 */
