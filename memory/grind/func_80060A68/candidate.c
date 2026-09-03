/* [s18 2026-09-03 - rederive modality.  BODY UNCHANGED (still the E2 body, re-measured
 * 2 / build 66 / target 66 on today's HEAD).  The s18 header is prepended; every earlier
 * header below is intact.]
 *
 * WHAT s18 ADDS.  74 ordinary-C bodies, no FAKE construct anywhere.
 *
 * (1) R4 IS CORRECTED, AND IT IS THE SESSION'S MAIN FINDING.  s17 recorded R4 as a purely
 *     positional law - "the p10 read must not follow a gp-based store".  The Z family
 *     (nine bodies, spine S1, P, S6, S8, S2, S5, S3, ... which uses the 0x1C store as the
 *     second cse separator) puts ALL THREE 0x10 reads ahead of BOTH gp stores and STILL
 *     emits the p10 load at slot 25 in $v0, at 9-11 / 66.  The slot-11 load is a
 *     consequence of the $a1 SEAT (R3: a span-2 $v0 donor strictly inside [p10 read, +4
 *     read]); R4 is a second necessary condition on top of it, never a sufficient one.
 *
 * (2) THE SEPARATOR LAW.  Chaining R3 (the donor is a gp addiu, so a gp store sits after
 *     the p10 read), corrected R4 (every 0x10 read must precede that gp store) and R1 (a
 *     store between consecutive 0x10 reads) leaves exactly one candidate for the second
 *     separator: a copy store.  Its price was measured on 47 bodies (M1-M8, W1-WB, and the
 *     30-body T sweep of which copy moves x where the 0x18/0x1A pair sits x 5 tail orders).
 *     The only prefix that keeps the slot-11 $a1 load, C1,C2,S1,S2,C3,P, is 67 instructions
 *     in all five tails (best 10): local-alloc gives the moved copy $v0 for both its
 *     pointer and its value, its two loads land adjacent, and a load-delay nop appears.
 *     Prefixes that move C1 or C2 instead are 66 but drop the p10 load to slot 24-25.
 *     Splitting a copy so only its `sw` moves (V1-V9) is byte-neutral when the halves stay
 *     adjacent (control VC = 5 / 67 == unsplit H3) but measures 12-14 as a separator,
 *     because the detached value pseudo takes a seat and pushes p10 to $a2.
 *
 * (3) U2 - TARGET'S EXACT LOAD GEOMETRY WITH NO `temp2` LOCAL AT ALL.  C1,C2,C3, S1, S3i,
 *     P, S4, S5, S6, S7, S8 - where S3i is the +2 read written inline into its own 0x1A
 *     store - measures 5 / 66 with loads at 11 ($a1), 19 ($a0), 22 ($a0).  A fused
 *     read+store carries its own cse separator for free.  This refines s16's "the temp2
 *     named intermediate is load-bearing", which was measured on bodies that also deleted
 *     p10.  The catch: a fused statement has no source-level interior, so R2's donor site
 *     does not exist on that spine; U2's residual is K7's single seat, reached with one
 *     fewer local.  See rejected/s18-U2-*.
 *
 * (4) Y1 - A NEW SCORE-5 CLASS THAT HOLDS THE SLOT-11 LOAD WITH NO COPY MOVED.
 *     C1,C2,C3, S1, P, S4, S5, S6, S2, S7, S3, S8 is 5 / 66 with SLOTS 0-20 BYTE-IDENTICAL
 *     to target; the p10 read uses the 0x18 store as its separator and the gp store inside
 *     [P, S6] buys the $a1 seat.  Its whole residual is that the +2 read sits downstream of
 *     that gp store, pinning its load at slot 26 in $v0 - the mirror image of H3's
 *     residual.  See rejected/s18-Y1-*.
 *
 * NEXT.  Three 66-instruction bodies now each hold a different two-thirds of the residual
 * (E2 both seats + target's 25/26/27 group but two loads; K7/U2 all three loads in target's
 * slots and registers but the +2 value in $v0; Y1 slots 0-20 exact with the +2 load pinned
 * at 26).  What none has is a FREE store between the +2 read and the p10 read that leaves
 * the +2 read upstream of the gp store.  The named next probes are in hypotheses.md
 * H-s18-1..5 and in the session frontier.
 */
/* [s17 2026-09-03 - rederive modality.  BODY UNCHANGED (still the E2 body, re-measured
 * 2 / build 66 / target 66 on today's HEAD).  The s17 header is prepended; every earlier
 * header below is intact.]
 *
 * WHAT s17 ADDS.
 *
 * (1) THE READ DECOMPOSITION WAS REDERIVED, NOT PERMUTED.  Fifteen bodies delete or reassign
 *     the `p10` pointer local - the +4 read written inline with no pointer local at all
 *     (A family), p10 feeding the +0 read instead (B), p10 feeding the +2 read instead (C).
 *     All measure 3-8 at 66-68 and NONE puts a `lw ?,0x10($v1)` at slot 11.  The p10 local is
 *     load-bearing: target's slot-11 load heads a pointer pseudo that lives from slot 11 to
 *     the `lhu $a1,0x4($a1)` at slot 28, and only a source-level pointer local read well
 *     before its halfword use produces a pseudo of that shape.
 *
 * (2) THE FOUR-CONDITION LAW (QTYDBG ground truth, local-alloc.c:1649-1685 priority and
 *     1563-1580 first-fit).  R1 three loads = a store between each consecutive pair of 0x10
 *     reads.  R2 the +2 value seats $a0 = its live range must be LENGTHENED (adjacent read and
 *     store give it refs 2 / span 2 = priority 10000, the maximum, which first-fits to $v0), so
 *     that a span-2 $v0 quantity inside the range is allocated first.  R3 p10 seats $a1 = a
 *     span-2 $v0 quantity inside [p10 read, +4 read] plus `idx` live across it.  R4 the load
 *     is emitted at slot 11 = the p10 read must not follow a gp-based store, because sched2
 *     disambiguates mem(v1+0x10) from the v1-based 0x18/0x1A/0x20.. stores but not from
 *     `sw $v0,%gp_rel(D_800A3478)($gp)`.  THE BIND: the only two span-2 $v0 quantities this
 *     function owns are the addiu halves of the two gp stores, and R2 wants one of them
 *     between the +2 read and the 0x1A store while R4 wants the p10 read ahead of both.
 *
 * (3) H3 IS THE NEW CLOSEST-BY-STRUCTURE BODY.  C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8 measures
 *     5 / build 67 / target 66 and is TARGET'S ENTIRE STREAM with one instruction relocated:
 *     `lw $a1,0x10($v1)` at slot 26 (right behind the gp store that blocks it) instead of
 *     slot 11, plus the resulting nop at slot 32.  Both target seats, all three loads, no FAKE
 *     construct.  See rejected/s17-H3-*.
 *
 * (4) s16's NAMED NEXT PROBE IS CLOSED.  The D_800A347C pair (S7) as the window donor on the
 *     slot-11 spine: seven placements, 7-9 every time (rejected/s17-G3-*).  Both gp stores
 *     behave identically under R2 and R4.
 *
 * NEXT.  The question is now single-valued: find a span-2, refs-2 quantity that can occupy $v0
 * strictly inside the +2 value's live range and that is NOT one of the two gp stores, so the
 * p10 read can stay ahead of both.  A copy statement does it (M2/M6/M7 reach slot 11 with both
 * seats) but costs an instruction and 10-13 points.  See hypotheses.md H-s17-1..6.
 */
/* [s16 2026-09-03 - forensics modality.  BODY UNCHANGED (still the E2 body, 2 / build 66 /
 * target 66); the s16 header below is prepended, the s15 header that follows is intact.
 *
 * WHAT s16 ADDS.  Three findings, none of which moved the floor:
 *
 * (1) The two halves of the residual are now each exhibited SEPARATELY at 66 instructions
 *     in ordinary C.  E2 (this body) has target's register seats and target's
 *     25/26/27 addiu / sw %gp(D_800A3478) / sh 0x1A group but only two 0x10 loads.  K7
 *     (rejected/s16-K7-...) has TARGET'S EXACT THREE-LOAD GEOMETRY - lw $a1,0x10($v1) at
 *     slot 11, lw $a0,0x10($v1) at 19 and 22 - and its whole residual is that the +2
 *     halfword value seats in $v0 (QTYDBG reg74 ord 5 got=2).
 *
 * (2) s15's mechanism claim is CORRECTED.  The $a0 seat does NOT require
 *     `D_800A3478 = outer + 0x18;` inside the +2 read's range; it requires ANY short-lived
 *     high-priority $v0 pseudo there.  rejected/s16-L1-... moves the third COPY statement
 *     into that window instead and carries BOTH target seats at once (reg74 got=4,
 *     reg75 got=5) with three loads at 66 instructions - the first body in the campaign to
 *     do so.  It costs 9 points elsewhere because the moved copy's own three instructions
 *     schedule at 21/23/26 rather than target's 16/18/20.
 *
 * (3) The E2 spine was swept EXHAUSTIVELY: 228 bodies, all ordinary C
 *     (tmp/grind/func_80060A68/s16/{xsweep,ysweep}.log).  Not one produced target's load
 *     geometry [11,19,22].  On the spine that has the gp store inside the +2 read's range,
 *     the third load lands at slot 4 (65 insns), at slot 26/27 (66 insns), or the body
 *     costs 67 - never slot 11.
 *
 * NEXT.  The lead is K7's spine (gp store AFTER the 0x1A store), where the load geometry
 * is already target's: find a $v0 donor for the +2 read's window whose own instructions
 * belong where target puts them.  See hypotheses.md H-s16-1..4.
 */
/* [s15 2026-09-03 - forensics modality.  BODY REPLACED: candidate.c is now the E2 body.
 * Still 2 / build 66 / target 66, but a DIFFERENT and much closer score-2 class; the
 * s7..s14 body is preserved at rejected/s14-candidate-2load-a1-plus0-read-score2.c.]
 *
 * THE ONE FACT THIS SESSION ADDS.  The +2 halfword value's $a0 seat - which every record
 * from 2026-08-19 onward treated as unreachable without a multiply-set carrier - is bought
 * by ONE ordinary statement move: put `D_800A3478 = outer + 0x18;` BETWEEN the +2 read and
 * the 0x1A store.  Mechanism (local-alloc.c:1649-1685 priority, 1563-1580 first-fit,
 * QTYDBG ground truth in tmp/grind/func_80060A68/s15/qtydbg_W5.txt): with the store after
 * the 0x1A store, the +2 value pseudo has refs 2 / span 2 => priority 10000 (the maximum a
 * refs=2 quantity can have), it is allocated 6th of 25, nothing holds $v0 across its range,
 * and first-fit gives it $v0.  Moving the gp store inside its range lengthens it to span 6
 * (priority 3333, allocated 24th) AND puts the addiu quantity (refs 2, span 2, priority
 * 10000, allocated 7th) in $v0 across it, so first-fit is forced past {$v0,$v1} to $a0.
 *
 * THIS BODY (E2) - two 0x10 loads, 66 insns, score 2.  Its ONLY differing slots are
 *     22  ours nop              vs  target lw $a0,0x10($v1)
 *     24  ours lhu $a0,2($a1)   vs  target lhu $a0,2($a0)
 * Everything else matches, including target's 25/26/27 `addiu v0,v1,0x18 /
 * sw v0,%gp(D_800A3478) / sh a0,0x1A(v1)`.  The residual is entirely the MISSING THIRD
 * 0x10 load: cse folds p10 and the +2 read's pointer into one $a1 load.
 *
 * THE cse RULE THAT GOVERNS THE THIRD LOAD (new, measured s15).  A second
 * `*(s32 *)(outer + 0x10)` is folded onto the first unless a store separates them, and
 * BOTH kinds of store separate: the register-based stores (`*(u16 *)(outer + 0x18) = ...`,
 * the copy stores) AND the gp symbol stores (`D_800A3478 = ...`).  E2 vs E3 differ only in
 * whether the p10 statement sits before or after `D_800A3478 = outer + 0x18;` and that
 * alone is 2 loads vs 3.
 *
 * THE TENSION THAT IS LEFT (the whole frontier).  Three loads WITH the early slot-11 $a1
 * one requires the p10 statement before a copy store; three loads with the gp store between
 * the +2 read and the 0x1A store then measures 67 in every ordering tried (30 bodies:
 * A1-A8, B00-B23, C1-C18).  Putting the p10 statement after the gp store keeps 66 and 3
 * loads but the third load lands at slot 26 in $v0 (E3, score 3 - the best 3-load body in
 * the campaign, rejected/s15-E3-*).  Moving `D_800A347C = outer + 0x20;` early to act as
 * the cse separator is 68/69 (H1-H12).
 */
/* [s14 2026-09-03 - solver modality.  BODY UNCHANGED, re-measured 2 / 66 / 66 on
 * today HEAD.  READ THIS BEFORE SPENDING ANOTHER SESSION ON THIS BODY.]
 *
 * THIS 2-LOAD BODY IS NO LONGER THE FRONTIER.  goal_from_tgt.py classify types its
 * residual PRE-RA (ours a nop, target a lw #,16(#)) - unmodellable.  The 3-load family
 * types RA, and s14 reached a 3-load body, W5, whose SLOTS 0-23 ARE BYTE-IDENTICAL TO
 * TARGET including the slot-11 early-a1 0x10 pointer load that the 2026-08-19/08-30/09-01
 * records all call unreachable without a multiply-set carrier.  W5 is ordinary C (two
 * statement moves, no invented local, no second write to anything) and measures 5/66;
 * its whole residual is slots 24-29.  See
 * rejected/s14-W5-three-loads-early-a1-slot11-slots0-23-target-identical-score5.c and
 * hypotheses.md H-s14-1..6.  candidate.c is retained only because 2 < 5 by the metric.
 */
/* [s13 2026-09-01 - escalation/disposition, owner Ruling A reopen.  BODY UNCHANGED,
 * re-measured 2 / 66 / 66 on today's HEAD.  s13 finished the local-alloc order search
 * that s12 could not run (inverse.py atoms() hangs on itertools.permutations over 23
 * quantities), enumerated the complete seat-escape set from the QTYDBG/FFR ground truth
 * (pri(qty8) >= 5000: refs3/span<=6, refs4/span<=16, refs5/span<=20, refs6/span<=24) and
 * measured every member dead - see evidence.md s13 and decisions.md 2026-09-01.] */
/* [s12 2026-08-30 - escalation/disposition modality.  BODY UNCHANGED.  Re-measured
 * 2 / 66 / 66 on today's HEAD (HEAD carries INCLUDE_ASM since commit 0bef2aa3).]
 *
 * THE RESIDUAL, RE-CONFIRMED BY FRESH DISASSEMBLY (tmp/grind/func_80060A68/s12/base.dis):
 * exactly two adjacent slots.  Our line 22 `lhu v0,0(a1)` vs target `lhu v0,0(a0)`, and our
 * line 23 `nop` vs target `lw a0,0x10(v1)`.  Nothing else differs.
 *
 * WHAT s12 CLOSED.
 *  1. THE PARTITION AXIS IS COMPLETE.  The three halfword reads of *(s32 *)(outer + 0x10)
 *     can be grouped three ways onto two loads and all three are now measured:
 *     {+0,+4} shared = this body, 2 / 66;  {+2,+4} shared = s10 w1/w2, 2 / 66;
 *     {+0,+2} shared = s12 body c1, 4 / 66 (rejected/s12-partition-0-and-2-share-a-load-*).
 *  2. THE FREE-SEPARATOR AXIS IS DEAD.  Breaking the cse fold of the 0x18 read onto p10
 *     needs an aliasing store between them, and every store this function already performs
 *     costs instructions in that position: D_800A3478 -> 11 / 68, D_800A347C -> 8 / 68,
 *     both -> 14 / 69, copy 3's store -> 5 / 67.
 *  3. THE local-alloc SEAT QUESTION IS ANSWERED ANALYTICALLY, not by more search.  The
 *     priority formula at tools/gcc-2.7.2/local-alloc.c:1649-1685 is
 *     floor_log2(refs)*refs*size/(death-birth)*10000, allocated first-fit at
 *     local-alloc.c:1563-1580.  On the measured qty table pri(p10)=1666 < pri(the +2
 *     pointer)=3333, so the +2 pointer takes $a0 first and p10 takes $a1 - exactly what is
 *     observed.  Raising p10 above 3333 requires either a 4th reference to the +0x10
 *     pointer (the fabricated-consumer axis, closed by s10 and a banned family) or a live
 *     range under 9 insns (= move the +4 read early, which is d8 and c1, both 4 / 66 with
 *     the load in $v0).  No lever lives here.
 *
 * THE LAW THIS BODY SITS AGAINST.  A `lw ?,0x10($v1)` is emitted at slot 12 and allocated
 * $a1 IFF it has an early consumer; a load with no early consumer is emitted late and
 * allocated $v0.  Four independent bodies exhibit both halves (base/y6, w1/w2, c1, d8).
 * Target needs a load that is BOTH unshared (three loads) AND early-in-$a1 - the one
 * combination the law excludes at 66 instructions.
 *
 * DISPOSITION s12.  Endgame gate 1 re-run on today's chassis: scan_hand_coded --single =
 * tier LOW 1/8 (S4 only) - FAILS.  Gate 2 is not applicable: this body carries no coercion
 * construct at all, so no SOTN-master precedent is even citable.  Both gates fail, which is
 * the owner's pre-decided case, so the 2026-07-27 standing auto-ruling applies and the
 * entry filed this session in docs/grind/decisions.md records REFUSED / OWNER-ACCEPTED
 * INCOMPLETE with nothing pending on the owner.  src/text1b.c was restored to HEAD; no
 * rules touched, no commits.
 */
/* [s11 2026-08-25 - escalation modality.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's
 * HEAD (HEAD carries INCLUDE_ASM since commit 0bef2aa3).]
 *
 * THE s9/s10 MECHANISM IS WRONG - DO NOT SPEND ANYTHING ON "OUTSIDE PRESSURE AT T-30".
 * The sched2 dump with the three-load body d8 applied (tmp/grind/func_80060A68/s11/sched2.d8,
 * function region at line 30625) shows the +4 address load, insn 39
 *     (set (reg/v:SI 2 v0) (mem:SI (plus (reg/v:SI 3 v1) (const_int 16))))
 * carrying REG_DEP_OUTPUT on insn 51, the +0 halfword read
 *     (set (reg:HI 2 v0) (mem:HI (reg:SI 4 a0))).
 * Both are in $v0.  sched2 is POST-RELOAD, so that output dependence is created by reload's
 * register assignment, and it is what gives insn 39 INSN_PRIORITY 8 and releases it only at
 * T-30.  The residual is an ALLOCATION fact, not a readiness fact.  Independently, the
 * displacement idea was arithmetically impossible: from T-30 down, insn 53 is the only other
 * pending priority-8 insn and every insn scheduled at T-33..T-45 has priority <= 7, so extra
 * ready insns can displace a priority-8 insn by at most ONE cycle where target needs 13.
 * FRONTIER ITEM 2 IS CLOSED.
 *
 * WHAT s11 OPENED INSTEAD.  Putting a named local on copy 2's loaded VALUE and seating p10
 * between that load and its store (body x2, banked at
 * rejected/s11-copy2-value-local-3loads-p10-reaches-a1-but-slot24-and-67insns-score5.c,
 * disassembly at tmp/grind/func_80060A68/s11/x2.dis) is the FIRST body in the campaign with
 * THREE lw ?,0x10($v1) loads whose +4 address load is in $a1 - target's register - and whose
 * slots 0-11 are byte-identical to target, including target's adjacent
 * `lw $v0,0xC($v1); lw $a0,0xC($v1)` pair at slots 10/11.  It costs one instruction (67) and
 * puts the $a1 load at slot 24 instead of 12, so it measures 5.  All seven value/base-local
 * shapes measured (x1 x2 x4 x6 x7 x8 x9) land at exactly 5/67; every p10 seat inside the copy
 * triple costs that instruction.  The open question is now sharply stated: hold p10 off $v0 in
 * a 66-instruction three-load body.
 *
 * ALSO KILLED: the c2 named intermediate is codegen-INERT in every after-the-copy-triple seat
 * (y6/y7/y8 = 2/66, candidate.c's byte class - banked at
 * rejected/s11-c2-named-intermediate-after-copy-triple-codegen-inert-score2.c).
 *
 * DISPOSITION s11.  Endgame gates re-checked: scan_hand_coded --single = tier LOW 1/8 (S4 only)
 * so gate 1 FAILS; gate 2 is not applicable because this body carries no coercion construct at
 * all.  No decision packet was filed: the 2026-08-25 representation packet was already ruled (b)
 * and executed, and what remains is a grind question with a corrected mechanism and a live axis,
 * not an owner-decidable one.  src/text1b.c was restored to HEAD; no rules, no commits.
 */
/* MIGRATION NOTE (2026-08-25, commit 0bef2aa3): main now carries
 * INCLUDE_ASM("asm/funcs", func_80060A68) — the rule-era C body and the last
 * 2 asmfix rules are retired (owner ruling (b), decisions.md 2026-08-25;
 * chassis banked in retired-chassis-2026-08/). HEAD-state claims below
 * predate the migration: "HEAD's own committed body measures 39 / 64" refers
 * to the RETIRED chassis, not current main. This candidate (2 / 66) remains
 * the working frontier. */
/* [s10 2026-08-25 - escalation/disposition.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's
 * HEAD (HEAD's own committed body measures 39 / 64).]
 *
 * THE TWO MISSING INSTRUCTIONS, NAMED.  Diffed slot-by-slot this session
 * (tmp/grind/func_80060A68/s10/cmp.py cand): line 22 emits `lhu v0,0(a1)` where target has
 * `lhu v0,0(a0)`, and line 23 emits a `nop` where target has the third `lw a0,16(v1)`.
 * Nothing else differs - all 15 relocation sites and all 64 other instructions match.
 *
 * s10's CONSERVATION RESULT (closes frontier item 1).  cse folds a repeated
 * `*(s32 *)(outer + 0x10)` onto the most recent live equivalent, so "three separate
 * `lw ?,0x10($v1)` loads" is the same condition as "each of the three halfword reads is
 * separated from the previous by an aliasing store", which is the same condition as "each load
 * has exactly one consumer" - and s9's ready-list-starvation law then pins the +4 load adjacent
 * to its consumer.  Whichever read is chosen as p10's second consumer, that read loses BOTH its
 * own load and its hard register, so the residual is invariant at 2 and only MOVES between
 * reads: s10's w1/w2 (banked at rejected/s10-mirrored-partition-*.c) put p10 on the +2 read
 * instead of the +0 read, make line 22 target-exact, and break line 26 instead.  A second
 * consumer that is not one of the three reads does not exist in this function's semantics and
 * would have to be fabricated (dead read / address-of / discard = banned family).
 * FRONTIER AFTER s10: only an OUTSIDE pressure source - an insn ready at sched2's T-30 whose own
 * emission slot is not target's slot-23 load-delay slot.  s9 measured seven, all dead.
 *
 * DISPOSITION s10: both endgame gates re-checked and both FAIL (scan_hand_coded --single =
 * tier LOW 1/8, S4 only; no construct in play so no SOTN precedent is even applicable).  The
 * function was ESCALATED WITH A DECISION PACKET (docs/grind/decisions.md, 2026-08-25) on the
 * REPRESENTATION question raised by the RULES-TO-ZERO campaign, not on a request to relax any
 * standard.  src/text1b.c was reverted to HEAD; no rules touched, no commits.
 */
/* [s9 2026-08-19 - escalation/disposition.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's
 * HEAD (HEAD's own body measures 39 / 64).  Read this note before spending anything the s6/s7/s8
 * notes below say about WHY the last two instructions are missing.]
 *
 * 1. THE RESIDUAL IS READY-LIST STARVATION IN sched2, NOT reg_n_sets AND NOT REGISTER OCCUPANCY.
 *    s9 built a body (d8, banked at
 *    rejected/s9-three-loads-66insns-plus4-address-load-adjacent-to-consumer-slot24-v0-score4.c)
 *    with all THREE `lw ?,0x10($v1)` loads at 66 instructions whose slots 30-63 are byte-identical
 *    to target.  Its ENTIRE residual is one placement: target emits the +4 read's address load at
 *    slot 12 as `lw $a1,0x10($v1)`, SEVENTEEN slots above its consumer (`lhu $a1,0x4($a1)` at slot
 *    29); every body that reads `*(s32 *)(outer + 0x10)` into a local used ONLY for the +4 read
 *    emits that load at slot 24-25, immediately above its consumer.  Everything between 12 and 24
 *    is target's stream shifted by exactly one, which is why the count comes out at 66 either way
 *    (target's slot-23 load-delay slot is a nop in the two-load bodies).
 *
 * 2. DUMP-ATTRIBUTED, NOT INFERRED.  `pwsh tools/grinder/dump.ps1 func_80060A68` with b1 applied,
 *    tmp/grind/func_80060A68/dumps/text1b.sched2:36617 (function) and its trace:
 *        ;; ready list at T-30: 39 (3), now 39
 *        ;; launching 56 before 39 with no stalls at T-31
 *    insn 39 is the +4 address load, priority 3.  sched.c schedules BACKWARD and never idles a
 *    cycle while anything is ready; insn 39 becomes ready only when its single consumer is
 *    scheduled at T-29, and it is then the ONLY ready insn, so it is forced into T-30 (= forward
 *    slot ~25).  THE LAW: a 0x10 load with exactly ONE consumer cannot be hoisted - it is pinned
 *    adjacent to that consumer by ready-list starvation.  Every body in the campaign that DOES
 *    reach `lw $a1,0x10($v1)` at slot 12 (this candidate, s8's f3, s5's q5, s9's g1) gives that
 *    load a SECOND consumer (the +0 or the +2 read) - and paying that second consumer is exactly
 *    what costs the third load.  This supersedes the s6/s7 `reg_n_sets` framing AND refines s8's:
 *    $v0/$a0 occupancy decides the REGISTER the load gets, ready-list starvation decides its SLOT.
 *
 * 3. WHAT WAS MEASURED AGAINST IT (28 bodies, all single-write locals, all in
 *    tmp/grind/func_80060A68/s9/).  Pressure sources on the three-load orders: the `idx` read
 *    (g1 8/66 - reaches $a1 at slot 12 but its own load then takes target's slot-23 load-delay
 *    slot; c1/c2/d5 over-pressure the pseudo to $a2 at 67 insns); a named `gp18 = outer + 0x18`
 *    (a2 6/66, c3 5/67, g3 4/66 - inert on the slot); a named `gp20 = outer + 0x20` (d4 5/67);
 *    copy 2 split into a `c2` local (a3 11/65, c5 5/67); copy 3 split into a `c3` local (c4 8/66 -
 *    reaches $a1 at slot 12, but the split lets cse keep copy 2's base in $a0, so the copy triple
 *    loses target's `lw $a0,0xC($v1)` reload); the gp-347C store hoisted above the halfword group
 *    (d1 8/68 - also reaches $a1 at slot 12, at the price of two extra instructions); the +2
 *    read's own read hoisted (a1 11/64, a5 10/64, a6 13/64).  The +4 read spelled as a fresh
 *    in-line read seated early is not better (e1 10/67, e2 7/65, e3 11/67), nor is pushing the +4
 *    read as late as the source allows (e5 7/67, e6 8/68).  The v2 shape itself re-measured at
 *    b1 = b2 = b3 = 5/67 (source position of the p10 read is inert, as banked since s1).
 *
 * 4. FRONTIER IF THIS FUNCTION IS EVER RE-ACTIVATED.  Find a C form in which the +4 address load
 *    has a SECOND, EARLY consumer that does not consume one of the other two 0x10 reads - or one
 *    in which some other insn is ready at sched2's T-30 whose own emission slot is not 23.  Every
 *    pressure source tried so far is either dependence-free (and therefore the best filler for
 *    slot 23, which target gives to the third 0x10 load) or inert on the slot.
 *
 * 5. DISPOSITION.  s9 was dispatched in `escalation` modality; the floor did not move (2), both
 *    endgame gates fail again (scan_hand_coded --single func_80060A68 = tier LOW 1/8, S4 only; no
 *    SOTN-master precedent is even in play, since no coercion construct is proposed - the
 *    remaining gap is an ordinary scheduling placement), so the 2026-07-27 standing auto-ruling
 *    applies and the docs/grind/decisions.md entry was updated with the corrected mechanism.
 *    src/text1b.c was reverted to HEAD; no rules touched, no commits.
 */
/* [s8 2026-08-19 - structural.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's HEAD.]
 *
 * s8 did not change this body, but it overturns the causal story the s6/s7 notes below tell,
 * so read this note before spending anything they say.
 *
 * 1. THE RESIDUAL IS NOT GATED ON birthing_insn_p.  s6 and s7 concluded that the only
 *    C-visible input left was sched.c:2536 `return (reg_n_sets[i] == 1);` - i.e. the
 *    multiply-written carrier the Judge banned.  That reduction is wrong.  The bump does fire,
 *    and it does pin p10's load next to its consumer IN SCHED1's ORDER - but the byte that is
 *    actually missing is decided AFTER that, by which hard register local-alloc gives p10's
 *    pseudo, and sched2 (which never bumps, reload_completed == 1) is free to undo sched1's
 *    adjacency whenever the register is dead across the gap.  Two separable gates; only the
 *    second one has to move, and it moves under ordinary statement order.
 *
 * 2. DUMP-VERIFIED MECHANISM.  text1b.greg for this function says `;; 1 regs to allocate: 83`,
 *    so p10's pseudo (reg 75, set by insn 39) is assigned by LOCAL-ALLOC, and the dispositions
 *    line reads `75 in 2` = $v0.  In sched1's order (... 51, 53, 39, 59, 56, 61, 64 ...) $v0 is
 *    killed at insn 53 (`sh $v0,0x18($v1)`) and re-born at insn 64, so it is free across reg
 *    75's entire range and local-alloc's lowest-free-hard-reg scan takes it.  Keep any value
 *    live in $v0 across insns 39..56 and reg 75 lands in $a1 - dead from slot 12 to slot 28 -
 *    and sched2 hoists the load to slot 12 as `lw $a1,0x10($v1)`, exactly as in target.
 *
 * 3. TWENTY BODIES PROVE IT, WITH p10 SINGLE-SET THROUGHOUT.  Three independent honest
 *    pressure sources reach $a1 at slot 12: hoisting the `idx` read above the p10 consumer
 *    (f3 / f4 / i3 / i4 / r1, 8 / 66); sinking copy 3's store below the +2 read (n2 10 / 67,
 *    m1 9 / 66, o4 / o5 10 / 66); and splitting copy 3 into a named `c3` local whose store
 *    becomes the p10/0x18 cse separator (u5 8 / 66, ua 8 / 66).  s10's "conservation law"
 *    (three 0x10 loads XOR an un-pinned p10 load) is falsified: f3 has BOTH, at 66
 *    instructions, with a prefix byte-identical to target THROUGH SLOT 22 - eleven slots
 *    deeper than cb and covering the whole copy triple.
 *
 * 4. WHAT IS LEFT.  f3 and u5 bracket the answer from opposite ends and their correct regions
 *    are complementary: f3 is exact in slots 1-22, u5 is exact in slots 26-34 (including
 *    `lhu $a1,0x4($a1)` at 29 and the late `lhu $a0,0x0($v1)` idx read at 30, both campaign
 *    firsts).  f3's whole 8-slot cost is that the hoisted `idx` load, which depends on nothing
 *    but $v1, is schedulable into the load-delay slot at 23 - the slot target gives to the
 *    third `lw $a0,0x10($v1)`.  The next session's job is a pressure source whose OWN emission
 *    slot is not 23.  Both banked bodies and the full 59-body sweep are in
 *    memory/grind/func_80060A68/rejected/ and tmp/grind/func_80060A68/s8/.
 *
 * The INTEGRATION HAZARD in the s10 note below (asmfix.txt:109-110 must be retired in the same
 * change as any body swap) is unchanged.  src/text1b.c was reverted to HEAD before finishing.
 */
/* [s7 2026-08-19 - synthesis.  BODY UNCHANGED.  Re-measured 2 / 66 / 66 on today's HEAD.]
 *
 * s7's job was to merge six sessions into one attack and reset the frontier.  It did not
 * change this body (still the floor at 2), but it moved the campaign forward in three ways.
 *
 * 1. A NEW AXIS, AND A BETTER SEAT THAN v2.  Every prior session moved the `p10` statement.
 *    None moved its CONSUMER (`temp_a1 = *(u16 *)(p10 + 4)`), which is what actually decides
 *    when sched1 releases p10's load.  Sweeping the seven consumer seats gives
 *    ca 8/66, cb 4/66, cc 7/66, cd 8/67, v2 5/67, ce 7/67, y1 8/68.  `cb` (consumer placed
 *    immediately after the 0x18 store, above the +2 read) is the best non-candidate body the
 *    campaign has produced: 66 instructions, ALL THREE `lw ?,0x10($v1)` loads present, and a
 *    single defect - p10's load emitted at slot 23 as `lw $v0,0x10($v1)` instead of slot 11
 *    as `lw $a1,0x10($v1)`, shifting everything from slot 11 on by one.  It is banked at
 *    rejected/s7-consumer-after-0x18-store-3-loads-p10-load-pinned-adjacent-to-consumer-score4-66insns.c
 *    and is the seat the next session should read dumps against, NOT v2.
 *
 * 2. A GEOMETRIC PROOF THAT THE TARGET'S COMPILE DID NOT BUMP INSN 39.  Under the
 *    birthing_insn_p LAUNCH_PRIORITY bump, a single-set pointer load is ALWAYS emitted
 *    adjacent to its consumer: the bump makes it outrank everything, and it becomes ready
 *    exactly when its consumer is scheduled (cb trace: `launching 39 before 59 ... at T-32`
 *    then `ready list at T-32: 53 (8) 39 (7f000001), now 39 53`).  The target puts p10's load
 *    at slot 11 and its consumer at slot 28 - SEVENTEEN slots apart.  sched1 cannot do that.
 *    Only sched2 can (reload_completed == 1, so no bump), and only if the hard register is
 *    free across the gap: target's p10 is `$a1`; cb's is `$v0`, written at slot 20 and read
 *    at slot 22, so the hoist is blocked.  This is independent of any particular body and
 *    strengthens s6's conclusion rather than replacing it.
 *
 * 3. THE LAST UNPROBED GATE IS CLOSED.  s6 left birthing_insn_p's `bb_live_regs` test
 *    (sched.c:2524-2531) unmeasured.  s7 measured it with the instrumented cc1 and
 *    BB2_SCHED_DEBUG=1: `SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3`.
 *    birth=1.  It cannot be otherwise: backward scheduling releases insn 39 only after an
 *    insn that READS its destination is scheduled, and that read is what sets the bit.  So
 *    `reg_n_sets[i] == 1` is now the SOLE C-visible input to the bump, and that is the
 *    multiply-written carrier the Judge banned for this function on 2026-08-19.
 *
 * Also killed on the cb body: declaration order (four permutations, all 4/66 byte-inert),
 * local types (`s32 temp_a1`, `u16 *p10` with `p10[2]`, `s32 temp2`, all 4/66), and using a
 * gp store as the cse separator instead of a copy store (g1 8/68, g2 11/68 - a
 * `(mem (symbol_ref))` store is an absolute scheduling barrier).
 *
 * DISPOSITION.  The function is ACTIVE.  s7 is a `progress` outcome; src/text1b.c was
 * reverted to HEAD before finishing.  The INTEGRATION HAZARD in the s10 note below
 * (asmfix.txt:109-110 must be retired in the same change as any body swap) is unchanged.
 */
/* [s6 2026-08-19 - rederive.  BODY UNCHANGED from s5/s10.  Read this note first.]
 *
 * s6 re-measured this body on today's HEAD: score 2 / build 66 / target 66.  The floor is
 * confirmed again.  s6 did NOT change the body; it closed the shape question and named the
 * exact GCC gate that the whole remaining residual hangs on.
 *
 * 1. THE SHAPE IS FORCED.  s6 re-derived the target's C independently of s5 by enumerating
 *    every store that can act as a cse separator (cse.c will not disambiguate two
 *    `(plus (reg) (const_int))` addresses; sched.c will, off a common base, but never across
 *    a `(mem (symbol_ref))` gp store).  Target needs three `lw ?,0x10($v1)` loads and the +2
 *    read must sit ABOVE the gp-3478 store, which leaves exactly one legal separator
 *    assignment: the +4 pointer read first into a local, separated from the +0 read by a copy
 *    store, and the +0/+2 pair separated by the 0x18 store.  That is v2 (5 / 67).  The one
 *    partition s5 never tried -- hoisting the +0 VALUE into its own local above the copy-3
 *    store -- was measured from five seats (x1/x2/x6/x7/x8) and scores 9..13.
 *
 * 2. THE v6 FAMILY IS DEAD.  v6 (3 fresh in-line reads, 3 / 66) differs from target by
 *    exactly ONE instruction position: the third 0x10 load sits at slot 26 instead of 11.
 *    It cannot be hoisted at any priority -- `sw $v0,%gp_rel(D_800A3478)($gp)` sits at slot 25
 *    and sched.c cannot disambiguate a symbol_ref store from a (reg + const) load.  s5's
 *    frontier item 2 ("raise its INSN_PRIORITY") is therefore impossible, not merely hard.
 *
 * 3. THE RESIDUAL IS ONE GATE.  With v2 applied, tools/grinder/dump.ps1 gives sched1's trace:
 *        ;; ready list at T-31: 39 (7f000001) 53 (8), now 39 53
 *    insn 39 is p10's load, insn 53 the 0x18 store, insn 56 the +2 read's address load.
 *    0x7f000001 is LAUNCH_PRIORITY (sched.c:187): sched.c:4049 puts it on the insn being
 *    scheduled, and adjust_priority (sched.c:2541-2590) copies that max_priority onto every
 *    released insn for which birthing_insn_p is true -- and birthing_insn_p (sched.c:2505)
 *    is true for any live single-set pseudo birth, because its only real gate is
 *    `reg_n_sets[i] == 1` (the n_deaths switch above it is dead code; GCC says so itself at
 *    sched.c:2551).  So insn 39 takes T-31, insn 53 is pushed to T-33 and insn 56 to T-32;
 *    since sched.c schedules BACKWARD that emits the 0x18 store BEFORE the +2 address load,
 *    so reg91 (the +0 value, $v0) is already dead when reg92 is born and local-alloc hands
 *    reg92 $v0 -- where target, having 56 before 53, is forced onto $a0.  The "missing load"
 *    and the "wrong register" are ONE defect with ONE cause: insn 39's bump.
 *
 * 4. THE ONLY LEVER IS BANNED.  The sole C-visible input to that gate is reg_n_sets[p10] != 1,
 *    i.e. writing p10 twice with both writes live -- the multiply-assigned pointer-staging
 *    carrier the Judge banned for this function on 2026-08-19.  A dead first write does not
 *    work (flow.c deletes it; measured inert at 5 / 67).  Next session should expect the
 *    driver to move this function to `escalation`; if it does not, the remaining unmeasured
 *    gate is birthing_insn_p's `bb_live_regs` test, which no one has probed.
 *
 * The s5 and s10 notes below are unchanged and still accurate, INCLUDING the integration
 * hazard about asmfix.txt:109-110.
 */
/* [s5 2026-08-19 - rederive.  BODY UNCHANGED from s10; read this note first.]
 *
 * s5 re-measured this body on today's HEAD: score 2 / build 66 / target 66.  The floor is
 * confirmed, not stale.  s5 kept this body as the candidate but banked TWO new same-or-near
 * floor attractors that a future session should prefer as forensic seats:
 *
 *   rejected/s5-p10-early-feeds-plus2-and-plus4-plus0-fresh-score2-66insns.c   (q5)
 *      Same floor (2 / 66) but a STRICTLY CLEANER residual: it gets target's slot 22
 *      `lhu $v0,0($a0)` right, where THIS body emits `lhu v0,0(a1)`.  Its only two wrong
 *      slots are 23 (nop vs `lw $a0,0x10($v1)`) and 25 (register).  If you are going to read
 *      .lreg/.greg for anything, read it for q5 and for v2 side by side.
 *
 *   rejected/s5-three-fresh-inline-reads-no-p10-load-lands-slot27-v0-score3-66insns.c   (v6)
 *      Score 3 / 66 with NO p10 local at all - the plainest body the campaign has produced.
 *      It emits all three `lw ?,0x10($v1)` loads at 66 instructions; it is short one HOIST,
 *      not one load.  q1 and q3 (p10 early with an early consumer) compile byte-identically
 *      to it, so "give p10 an early consumer" is not a distinct body.
 *
 * s5's main result is a DERIVATION, not a search: applying the store-separator law (cse.c does
 * not disambiguate two (plus pseudo const) addresses, so an intervening store forces a fresh
 * load; sched.c does disambiguate them, so loads may hoist across those same stores) to the
 * target's own instruction stream proves that the target's +4 pointer cannot be a third in-line
 * read - there is no store available to separate it from the +2 read, and GCC never reorders
 * two non-disambiguable stores.  The target's C is therefore the v2 body (p10 read before the
 * copy-3 store; +0 and +2 read fresh in-line; +4 read through p10), which measures 5 / 67.
 * The entire remaining gap is v2's REGISTER ALLOCATION - reload gives the +2 read's address
 * pseudo $v0 where target gives it $a0.  s5 killed the declaration-level levers on that pseudo
 * from four more seats (named address locals p0 / p2 / both, and a u16*-typed p10, all 5 / 67
 * and byte-identical to v2), on top of s10's three type-level attempts.  See evidence.md and
 * hypotheses.md, both appended this session.
 *
 * Also killed this session: the m2c-literal "no `outer` local" family (26-29 / 68-70 insns) and
 * struct-typed member access via MEM_IN_STRUCT_P (regresses 3 -> 6, 2 -> 8).
 *
 * The INTEGRATION HAZARD below is unchanged and still load-bearing.
 */
/* [s10 2026-08-19 — forensics.  READ THIS FIRST: THE BODY IN THIS FILE CHANGED.]
 *
 * WHAT CHANGED.  Until s9 this file held the "staged temp_a1" body (temp_a1 written
 * twice: once with the 0x10 pointer, once with the 0x1A/0x1C halfword — the sanctioned
 * staged-value-reused-variable construct).  That body is PRESERVED VERBATIM at
 * memory/grind/func_80060A68/prior-floor2-staged-temp_a1.c together with its full s1-s9
 * header; it is NOT rejected and still measures score 2 / build 66 / target 66.  This
 * file now holds the s10 body, which measures the SAME floor (score 2, build 66 /
 * target 66, re-measured this session) but is strictly better on two axes:
 *
 *   1. It contains NO multiply-assigned local at all.  Every local here is written
 *      exactly once and read for a real job: `p10` holds the 0x10 pointer for the +4
 *      halfword read, `temp2` holds the 0x1A halfword across the D_800A3478 store,
 *      `temp_a1` holds the 0x1C halfword and the call's second argument.  There is no
 *      staged carrier, so there is no FAKE annotation and no sanctioned-family claim to
 *      defend.  (The prior body needed the staged-temp_a1 claim.)
 *   2. Its residual is in a DIFFERENT PLACE.  For the first time on this function the
 *      whole prefix through slot 20 is byte-identical to target, INCLUDING the three
 *      contested loads that every session since s1 has been fighting:
 *          slot 10  lw v0,12(v1)      slot 11  lw a0,12(v1)      slot 12  lw a1,16(v1)
 *      The sched2 rank_for_schedule LUID tie at T-45 (sched.c:2464) that s2-s9 attributed
 *      the residual to is GONE from this body.  The remaining 2-instruction gap is:
 *          target  ... lhu v0,0(a0) / lw a0,16(v1) / sh v0,24(v1) / lhu a0,2(a0) ...
 *          ours    ... lhu v0,0(a1) / nop          / sh v0,24(v1) / lhu a0,2(a0) ...
 *      i.e. the 0x18 halfword read is CSE-folded onto p10's register (a1) instead of
 *      re-loading 16(v1), and the load that target puts in the slot-23 load-delay slot is
 *      therefore missing and shows up as a nop.
 *
 * THE MECHANISM, AND WHY IT IS A CONSERVATION LAW (this is the s10 finding).
 * `p10` is single-set, so its load (insn 39) takes sched1's birthing_insn_p
 * LAUNCH_PRIORITY bump (sched.c:2504-2535) and is chained LATE, which is what finally
 * gives sched2 the LUID order it needs to emit copy 2's address load at slot 11 and the
 * staged load at slot 12.  But a bumped load is chained next to its FIRST consumer, and
 * with three independent reads of *(s32 *)(outer + 0x10) p10's only consumer is the +4
 * read at target slot 29 — sched1 then chains insn 39 between insns 56 and 58, sched2
 * reaches T-30 with insn 39 as the sole ready insn, and the load overshoots to slot 25
 * (measured: rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c and the
 * whole r2/r3/r4/t1/u1/u2/u3 family, all score 5 / 67 insns).  Moving the p10 statement
 * BELOW the copy-3 store removes the memory fence between p10's load and the 0x18 read,
 * so cse folds the 0x18 read onto p10 — which gives insn 39 an EARLY consumer (slot 22),
 * unpins it, and lands it exactly on target's slot 12.  That is the trade this body makes:
 *
 *     3 independent 16(v1) loads  <=>  p10 has no early consumer  <=>  sched2 T-30 pin
 *     p10 has an early consumer   <=>  one of the 3 loads is CSE-folded away  <=>  a nop
 *
 * Nine source positions for the p10 statement were measured this session (r1..r9) and
 * every one of them sits on one side or the other of that equivalence; so does every
 * attempt to re-separate the loads with a store (t1: split copy 3 so its store falls
 * between p10 and the 0x18 read -> 3 loads and the pin returns, score 5; t2: hoist the
 * D_800A3478 gp store above the 0x1A read -> score 8).
 *
 * WHERE THE NEXT SESSION SHOULD PUSH.  The r4-shape pin has a named and now dump-verified
 * proximate cause that is NOT scheduling at all: in the 3-load bodies reload gives insn 56
 * (the 0x1A read's address load) hard register v0, and insn 53 is `sh v0,24(v1)`, so 53
 * carries a write-after-read anti-dependence on 56.  sched2 therefore cannot release 53
 * until 56 is scheduled, T-30 comes up with only insn 39 ready, and 39 is forced into it
 * (n9b/trace_sched2.txt T-30 `;; ready list at T-30: 39 (3)`).  In the target — and in the
 * prior staged body — insn 56 gets a0 instead, 53 IS ready at T-30 (n9/trace_sched2.txt
 * `;; ready list at T-30: 39 (3) 53 (8), now 53`), 53 wins on priority 8 vs 3, and insn 39
 * survives to a much later cycle.  So the 3-load body closes the function IF insn 56's
 * destination can be steered from v0 to a0 by honest C.  Three type-level attempts failed
 * this session (s32 temp2, s32 temp_a1, a fresh single-set `q` local for the 0x1A address
 * — all score 5); the next lever is a register-allocation one, read out of .lreg/.greg
 * rather than guessed.
 *
 * INTEGRATION HAZARD (unchanged, still live and load-bearing).  This body's first body
 * instruction is `lhu $2,0($3)`, while asmfix.txt:109 (a delete_between anchored on
 * `^lhu\t\$4,0\(\$3\)$`) and asmfix.txt:110 (a 43-instruction insert_before splicing the
 * whole target body) are written against the HEAD body.  Applying this body to
 * src/text1b.c WITHOUT retiring both asmfix rules in the same change silently duplicates
 * the function in a full build.  src/text1b.c was reverted to HEAD before this session
 * finished.
 *
 * DISPOSITION.  The function is ACTIVE, not parked.  s8's terminal-disposition claim was
 * voided by the driver (wrong modality) and s9 reverted the decisions.md entry; nothing
 * this session changes that.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    p10 = *(s32 *)(outer + 0x10);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
