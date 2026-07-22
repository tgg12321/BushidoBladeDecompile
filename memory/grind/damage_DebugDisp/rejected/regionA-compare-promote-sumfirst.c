/* REJECTED (s12, structural) — compare-depth-promotion WITH sum=0 moved to
 * source-position FIRST. Tests whether s11's claim ("sum=0 reschedules LAST
 * whenever sum reaches 11 refs, even when plain") is ref-count coupling or
 * merely an artifact of sum=0 sitting LAST in candidate source order.
 *
 * FORM (only delta vs rejected/regionA-compare-depth-promote-goto.c = sum=0
 * moved to the FIRST statement in the outer-loop body):
 *     sum = 0;                 // <-- FIRST (lowest LUID), plain, unbracketed
 *     j = 0;
 *     bp = base + offset;
 *     do { sum += *bp; bp++; j++; } while (j < 0x24U);
 *     chk = *(s32 *)((u8 *)chkptr + 0x6C);
 *     do { if (sum == chk) goto found; } while (0);   // compare-promotion
 *     chkptr++; i++; offset += 0x24;
 *   ...
 *   found: ...  (Region B index chassis unchanged)
 *
 * RESULT: sandbox --disable all = 7 (WORSE than s11's 5 AND floor 2).
 *   build_insns = 78 (one FEWER than target 79).
 *   objdump (tmp/grind/damage_DebugDisp/s12/probe_compare_sumfirst_objdump.txt):
 *     preheader = move a1,zero (114) ; move a0,zero (118) ; addu v1,t1,a3 (11c)
 *     inner: addiu a0,a0,1 ; addu a1,a1,v0 ; sltiu v0,a0,36 ; beq a1,v0,...
 *     => sum=$a1, j=$a0  (RA LOSS — the SWAP, same as plain).
 *
 * KILL + KEY FINDING: moving sum=0 first did NOT recover sum=0-first; it
 * DESTROYED the compare-promotion RA win. With sum=0 in candidate position
 * (s11) the compare bracket lifts sum to 11 refs -> sum=$a0. With sum=0 FIRST
 * the bracket no longer wins sum $a0 (sum=$a1). The compare-promotion RA win
 * is POSITION-DEPENDENT on sum=0's source LUID: it only holds when sum=0 sits
 * at its natural late position. So sum=$a0 (via ANY of sum's two promotable
 * refs) and sum=0-emitted-first are MUTUALLY EXCLUSIVE on this chassis — not
 * merely "A' not decoupled" (s11) but "the two goals actively conflict".
 * Additionally the goto-out control flow drops one insn (78) + reschedules,
 * so this branch of the space is strictly dominated. Reconfirms s6/s7/s10/s11
 * A'/A coupling from the last untested angle (sum-first + non-def promotion).
 *
 * ARITHMETIC COROLLARY (no test needed): plain-sum (sum=10, j=11) can NOT be
 * flipped to $a0 by any whole-function numbering/declaration perturbation that
 * leaves ref-counts unchanged, because allocno_compare priorities do NOT TIE
 * at 10 vs 11 (33333 < 36666); j wins outright regardless of pseudo order. A
 * flip requires sum n_refs>=11 (relocates def) OR sum live_length<=8 (needs a
 * bracket) OR j n_refs<=10 / j live_length>=10 (all byte-fixed by target). The
 * whole-function-numbering hope therefore only lives on the PERMUTER axis
 * (a structural mutation that produces a NEW byte-neutral sum ref), not manual.
 */
