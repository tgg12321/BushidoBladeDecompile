/* REJECTED (s11, structural) — compare-depth-promotion decoupler.
 *
 * GOAL: give sum its 11th weighted reg_n_refs (to win $a0) by PROMOTING the
 * post-loop compare from outer-loop depth 1 (weight 2) to depth 2 (weight 3)
 * via a do-while(0) bracket, keeping `sum = 0;` PLAIN at low LUID (so it would
 * schedule FIRST = target order, unlike the def-bracket do-while(0)).
 *
 * WEIGHT MODEL (confirmed s11): reg_n_refs += (1 + loop_depth) per ref.
 *   sum(plain) = sum0(d1:2) + accum[2 refs](d2:3+3) + compare(d1:2) = 10.
 *   Promote compare d1->d2: +1 => sum = 11 (ties j=11, wins $a0 by pseudo 77<79).
 *
 * FORM (chk pre-loaded OUTSIDE the bracket so chkptr is NOT pulled to depth 2):
 *     ... inner accumulate (sum=0 PLAIN) ...
 *     chk = *(s32 *)((u8 *)chkptr + 0x6C);
 *     do { if (sum == chk) goto found; } while (0);
 *     chkptr++; i++; offset += 0x24;
 *   ...
 *   return 0;      // i==3 no-match fall-through
 *   found: ...     // matched path (copy + Region B)
 *
 * RESULT: sandbox --disable all = 5 (WORSE than candidate floor 2).
 *   - Region A SOLVED: objdump idx8/9/10 = addiu a1,a1,1 / addu a0,a0,v0 /
 *     sltiu v0,a1,0x24  => sum=$a0, j=$a1 == TARGET inner alloc. The compare
 *     promotion DID lift sum to 11 refs and win $a0 WITHOUT a do-while(0) on
 *     the sum=0 def. NEW confirmed RA mechanism (frontier #3 RA-win half).
 *   - A' NOT decoupled: preheader still emits j=0(pos4), bp(pos5), sum=0(pos6)
 *     — the SAME sum,bp,j -> j,bp,sum swap. Even with sum=0 PLAIN, whenever sum
 *     reaches 11 refs the preheader reschedules sum=0 LAST. sum=0-first is NOT
 *     recoverable by promoting the compare.
 *   - goto-out-of-do-while(0) control flow adds ~3 positional diffs (the extra
 *     `j` + reorganized i==3 test) on top of A' => 5.
 *
 * INLINE-CHK VARIANT (chk inline in the bracketed compare): sandbox = 16 —
 *   the chkptr load is pulled into the depth-2 bracket, cascading the outer
 *   allocation (offset->t0, chkptr->a2, bp=base+offset recomputed). Confirms
 *   the s8 warning: any outer-IV memory ref inside the bracket cascades.
 *
 * KILL: compare-depth-promotion reaches sum=$a0 but (a) still emits sum=0 last
 * (A' persists — sum=0-first is unreachable once sum wins $a0 on this chassis),
 * and (b) requires goto-out-of-bracket control flow costing >= +3 diffs. Both
 * of sum's ONLY two references (def, compare) yield sum=0-last when promoted to
 * reach 11 refs; there is no third sum ref to promote. Target reaches 11 with
 * sum=0 first => its 11th ref is a whole-function loop_depth/LUID context effect,
 * not a manual structural promotion — directed-permuter axis (frontier #1).
 */
