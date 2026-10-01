# Hypothesis ledger — func_80058580

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM. `candidate.c` scores 30 of 2991 insns (floor history 2989 -> 30). Jump-table blocker resolved (rodata-object-alignment ruling); maspsx `.L` hazard fix adopted (-2).
- CONSTRAINTS: no compiler/maspsx changes (no-compiler-divergence.md). work1..work4 each carry 7-12 roles -> lands only under Ruling 11 (`.claude/rules/reused-local-necessity.md:89-132`): dumps for both spellings, a mechanism, every reviewer-proposed split banked, manual-path layer-2. Constant-only values refused unless the Q20 per-branch-constant exception covers them.
- REMAINING DIFFS: $a1 local-temp cascade from the case-2 `w >> 27` temp; two `0x438 >> 4` sites (ours lhu/sll/sra, target lh/sra - combine forms the sign-extract; the original kept the lh separate somehow); the lim-chain `j`; three jtbl %lo sandbox artifacts that vanish at landing.
- PLAN:
  1. Re-measure the candidate.
  2. Fix the lone `(&D_8009A838)[...]` scalar-array spelling at candidate.c:242 (shape retired by judge-decl-cleanup 8e007927d).
  3. Bank a Ruling 11 (C)(3) value audit per work variable; flag `work1 = 100000` (:240), `work2 = -1` (:578), the `work3 = 0/1` writes against Q20.
  4. Diagnostics: `0x438 >> 4` - forms where the lh result has a second use in the same block, or read through a typed struct member; $a1 cascade - read .lreg for the `w >> 27` temp's local-alloc seat.
  5. Resolve the func_80057E84 prototype conflict (below).
- DEPENDS [F]: candidate has `extern void func_80057E84(u8 *, u8 *, s32, s32)` (:21, call :346) but func_80057E84's body takes `PathWalker *` and sits earlier in text1b.c (2959 vs 2997) -> land func_80057E84 first and adopt PathWalker here. Landing deletes the transcribed jtbl arrays just before line 2997. Shares the D_80099D8F alias with func_80055B60 and the record with func_80023F08.
- ODDS/LANE: manual, 3+ sessions (mostly Ruling 11 paperwork), ~35% [I].

## 2026-10-01 laneB (s4) — re-baseline 30, banked
Re-measured 30 (unchanged). Dumps of the case-2 temp: local-alloc lowest-free-reg (no MIPS REG_ALLOC_ORDER),
so the target's a1 needs v0..a0 busy in that block (evidence.md [s4]). Not attempted this session (context
budget): Ruling 11 value audit for work1..work4 (plan step 3), D_8009A838 array spelling (needs
func_80056FE8's raw `(s32)&D_8009A838 + …` read respelled in the same landing), func_80057E84 prototype
(still rotated; candidate's extern is the only declaration in text1b.c, no conflict today).

## 2026-10-01 FIX PLAN after layer-2 FAIL round 1 (evidence.md [s6]); ordered
Bytes are solved (rejected/l2-fail-2026-10-01-e5799063.c links byte-identical); everything left is policy.
1. Owner answers on docs/grind/borderline.md func_80058580 (a) work3 merged stale-read value, (b) Q34 with a
   constant init (work2 best), (c) the 0x438 `* 0x100 >> 12` rescale vs Q45. (a) decides whether work3 can
   land at all; (b)/(c) each need a ruling or a new spelling search (b: e.g. best kept as s16 local again
   with a different init form; c: rescale alternatives or a data-model reason for the >> 4 site).
2. PracticeMenuRec typing (the big one): retype `p` as `PracticeMenuRec *` and read members, adding members
   where this function proves them (0x6A u16 vs byte read at 0x80059018, 0x86, 0x26C, 0x6C, 0x3A4, 0x3A8..,
   0x364 waypoints, 0x425..0x44B bytes, 0x438/0x43A/0x43C halfwords). This overlaps
   memory/grind/func_80055B60/cluster-plan-2026-10-01.md and memory/grind/func_80021424/HANDOFF.md (L1 member
   adds): land those member additions first or jointly; func_80055138 must move to members in the same landing
   (its 0x3A4 is `u16 *`: settle one type). Re-measure the whole body after typing (register effects likely).
3. D_8009A830 table model: one `u8 [3][8]` (or a 3-record struct) at 0x8009A830 per named_syms 887/2233/2461;
   respell func_80056FE8 onto it (keep its duplicated-arm FAKE, fix the cited path to `ffd7fef75^:...`) and
   this function's read; retire the 838/840 scalar externs.
4. Cast cleanup (dm results): drop the byte-neutral casts now; for the byte-moving casts find typed spellings
   (st2 u8 local, best s16 local, besti s8, mask locals u32 within the Ruling 11 package, `work4 < 8`).
   `-(et < 5) & 100000` -> ternary.
5. `q = ep`: without it 58; either annotate as the pointer-alias / named-intermediate family with exhaustion
   or find a structure where q is not a second handle (q is read only for the 0x40 header); re-check the
   do-while(0) need afterwards.
6. work5 declared inside `while (off != 0)`; rename buf/buf2; retire named_syms.txt:2466 with the alias;
   annotate the contradictory OARR(0x404) compare (`!FAKE` or comment) and disclose the named intermediates
   a/b/c, rnd/row, ok4, ob.
7. Redo r11/ for the new body (only if (a) is admitted; otherwise work3 needs a different spelling) and
   rewrite the message to list every construct.
