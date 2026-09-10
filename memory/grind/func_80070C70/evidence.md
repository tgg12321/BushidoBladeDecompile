# Evidence bank - func_80070C70

- [s1 2026-09-10] CHASSIS AT DISPATCH: HEAD carries `INCLUDE_ASM("asm/funcs", func_80070C70);` (the
  asm-until-matched migration retired the pre-migration body to
  `retired-chassis-2026-08/body.c`). `sandbox --disable all` on the untouched HEAD reports
  score 194 / target_insns 194 / build_insns 0 / no_c_body true. The migration_pin.json floor of
  118 is a PRE-MIGRATION number and was never re-measured until this session. Any session that
  reads "floor 118" from that pin is reading a stale chassis.

- [s1 2026-09-10] FLOOR 194 -> 108: restoring `retired-chassis-2026-08/body.c` with EVERY cheat-asm
  construct deleted (the `register s32 c60 asm("$20")` pin, the six
  `__asm__ __volatile__("addiu %0,$0,N")` constant materializers for 1/5/16/2/261, and the
  `__asm__("la %0,D_800A3590")`) and with the two declaration fixes below applied measures 108.
  The de-cheated body is therefore BETTER than the pinned 118, not worse - the cheats were never
  buying anything the sandbox could see, and two of them were papering over a wrong declaration.

- [s1 2026-09-10] OBJECT MODEL: the brief flagged two symbols. Both resolved, one measured.
  * D_800A3560 - INDEXED-ACCESS signal: **MISMATCH (measured; part of the 194 -> 108 drop)**.
    Header/TU declared `extern u8 D_800A3560;` (scalar). Target at 80070E28-80070E30 does
    `lui $at,%hi(D_800A3560); addu $at,$at,$s2; lbu $v1,%lo(D_800A3560)($at)` with $s2 stepping by
    3 per iteration - a u8 array of 3-byte records. Corrected to `extern u8 D_800A3560[];` at both
    occurrences in src/text1b.c (l.2124, l.6499) and indexed `D_800A3560[ctx_or_var_s2]`. NOTE
    src/text1b_b.c:266 carries the same scalar declaration and should be corrected in the same
    change when that TU is next touched (it currently has no uses there, so it is inert).
  * D_800A3590 - SPLIT-AGGREGATE signal (census: sub-symbol +6 of
    g_replay_motion_shared_state_d @800A358A): **MISMATCH as a scalar, and the aggregate-merge
    family is NOT the right fix here.** Target at 80070E5C-80070E64 does
    `lui $at,%hi(D_800A3590); addu $at,$at,$a0; lh $v0,%lo(D_800A3590)($at)` with $a0 = var_s0<<1.
    The relocation names **D_800A3590 itself**, not `D_800A358A+6`, so the shipped code's own
    addressing treats 800A3590 as an independent s16 ARRAY base. Prong (a) of the aggregate-merge
    family (base-register or stride evidence pointing at the 800A358A base) is NEGATIVE for this
    function: merging 800A3590 into g_replay_motion_shared_state_d would change the %hi/%lo symbol
    and cannot reproduce these bytes. The correct fix is the array declaration
    `extern s16 D_800A3590[];` (applied at l.2129, l.6500), indexed `D_800A3590[var_s0]`. Same
    text1b_b.c:271 note as above.
  * D_800A374C - no signal raised; used as `D_800A374C + 4` / `+ 0x28`, target emits
    `lui/lw %lo` then `addiu`, which the existing `extern s32 D_800A374C;` reproduces. MATCHES.

- [s1 2026-09-10] FRAME ARITHMETIC: IconC70 is 0x20 bytes, not 8. Target frame is 0x80: 0x00-0x18
  outgoing args (SetDrawMode takes 5 args, the 5th at 0x10), 0x18-0x68 locals, 0x68-0x80 six
  callee-saved slots (s0,s1,s2,s3,s4,ra). `prim` sits at 0x18 and `icon` at 0x48 in BOTH target and
  our build, so the locals region requires icon to span 0x48..0x68 = 0x20 bytes. Padding IconC70 to
  0x20 measured **108 -> 101** and moved our frame 0x70 -> 0x88. Independent check on the consumer:
  func_80069898 reads only offsets 0x0,0x2,0x4,0x6 of its `$a1` argument
  (asm/funcs/func_80069898.s lines 23-35), so the tail is genuinely untouched here - the 0x20 is
  the declared size of a shared game type, and the 12 trailing halfwords in candidate.c are an
  explicit PLACEHOLDER whose real field layout still has to be recovered. Siblings
  func_8006B120 / func_8006CFBC / func_800720FC / func_80074488 / func_8006F97C all call
  func_80069898 and are where to look.

- [s1 2026-09-10] THE REMAINING RESIDUAL IS A REGISTER-SEAT ROTATION PLUS ONE EXTRA REGISTER.
  At floor 101 we emit 193 insns against target's 194 - the shapes agree almost everywhere; what
  differs is WHICH register holds what, on essentially every insn that names one:

        value            target   ours
        arg0              $s1      $s0
        var_s0            $s0      $s1
        ctx (the 0x64 ptr) $s2     $s4
        var_s3 (mode)     $s3      $s2
        c60 (0x60)        $s4      $s5
        &D_800A3590[i]     -       $s3   <-- extra; target has no such register

  Target spends 5 callee-saved + ra; we spend 6 + ra. That single extra register is the entire
  0x88-vs-0x80 frame delta and is the obvious first domino.

- [s1 2026-09-10] PASS ATTRIBUTION (read from the dump, not guessed).
  `tmp/grind/func_80070C70/dumps/text1b.loop`, function section at line 44793,
  "Loop from 284 to 415: 50 real insns":

        Insn 312: regno 126 (life 2), move-insn savings 1  moved to 486   <-- &D_800A3590 hoisted
        Insn 315: giv reg 128 src reg 75 benefit 2 ... replaceable mult 2 add 0
        Insn 317: giv reg 129 src reg 75 benefit 4 ... replaceable mult 2 add (reg:SI 126)
        giv at 317 combined with giv at 319
        giv of insn 315 not worth while, 0 vs 50
        giv at 319 reduced to (reg:SI 159)
        giv at 317 reduced to (reg:SI 159)

  So: loop.c `move_movables` hoists the loop-invariant symbol address of D_800A3590 into pseudo 126
  (savings 1 - a marginal decision), which turns the array address into a giv carrying an
  `add (reg:SI 126)` term; that giv then COMBINES with the load's giv at 319, and the combination
  clears the reduction threshold that the bare `mult 2 add 0` giv at insn 315 fails
  ("not worth while, 0 vs 50"). The reduced giv becomes the induction pointer that eats our 7th
  callee-saved register. Target never hoists the address - it re-materializes lui %hi / addu /
  lh %lo inside the loop on every iteration. **The lever is the hoist at insn 312, not the spelling
  of the load.**

- [s1 2026-09-10] Why D_800A3560 escapes the same fate: its index `ctx_or_var_s2` is itself a biv
  (const 3) rather than a giv of the loop counter, so no address giv is formed and the fused
  `lui/addu/lbu %lo` form survives - exactly like target. That asymmetry is why "declare both as
  arrays" fixed one access shape and left the other divergent.

- [s1 2026-09-10] THE SCORE IS SATURATED BY THE ROTATION. Re-indexing the D_800A3590 read by
  `ctx_or_var_s2` instead of `var_s0` (a deliberately semantics-breaking diagnostic, reverted)
  measured 108 -> 108, i.e. exactly zero movement. A change that alters which biv feeds the array
  address moves no score at all, because nearly every scored insn already differs in its register
  field. Read that as: incremental spelling probes will keep reading 0 until the allocation is
  fixed. Attack the allocation, not the arithmetic.

- [s1 2026-09-10] SIBLING func_8006F97C (src/text1b.c, active, floor 513, 1 session, NO
  candidate.c) was checked as the brief requires: it has no banked spelling to transplant, so
  nothing could be measured from it. It does call func_80069898 with a stack struct, so it is the
  best place to recover IconC70's true tail layout, and its own floor should move once that type is
  corrected. The IconC70 = 0x20 finding is inheritance for it.

- [s1] CHASSIS: HEAD carries INCLUDE_ASM for func_80070C70; sandbox --disable all on untouched HEAD reports score 194 / target_insns 194 / build_insns 0 / no_c_body true. The migration_pin.json floor of 118 is a stale PRE-MIGRATION number and was never re-measured before this session.

- [s1] OBJECT MODEL: D_800A3560 -> MISMATCH (measured, part of the 194->108 drop): declared `extern s32/u8 D_800A3560;` but target indexes it with a computed register at 80070E28-80070E30 (lui %hi / addu $at,$at,$s2 / lbu %lo) where $s2 steps by 3 per iteration; corrected to `extern u8 D_800A3560[];` at src/text1b.c l.2124 and l.6499 and indexed D_800A3560[ctx_or_var_s2]. D_800A3590 -> MISMATCH as a scalar (measured, same drop), but the flagged aggregate-merge family is the WRONG fix: target's relocation at 80070E5C-80070E64 names D_800A3590 itself, not g_replay_motion_shared_state_d(800A358A)+6, so prong (a) of the aggregate-merge family is NEGATIVE for this function and a merge would change the %hi/%lo symbol away from what the bytes require; the correct fix is `extern s16 D_800A3590[];` (l.2129, l.6500) indexed D_800A3590[var_s0]. D_800A374C -> MATCHES: no signal was raised, it is used as D_800A374C + 4 / + 0x28 and the existing `extern s32 D_800A374C;` reproduces target's lui/lw %lo then addiu. Note src/text1b_b.c:266 and :271 still carry the old scalar declarations for the two corrected symbols; they have no uses in that TU so they are inert, but they should be corrected in the same change when text1b_b is next touched.

- [s1] The retired pre-migration chassis carried seven cheat-asm constructs (a register s32 c60 asm("$20") pin, six __asm__ __volatile__("addiu %0,$0,N") constant materializers for 1/5/16/2/261, and an __asm__("la %0,D_800A3590")). Deleting ALL of them and fixing the two declarations measures 108 - i.e. the de-cheated body is 10 points BETTER than the pinned 118, and two of the cheats were papering over the wrong declaration rather than buying anything.

- [s1] FRAME ARITHMETIC: target frame 0x80 = 0x18 outgoing args + 0x50 locals (0x18..0x68) + 0x18 six callee-saved slots. prim occupies 0x18..0x44 and icon starts at 0x48 in both builds, so icon must be 0x20 bytes. Padding IconC70 to 0x20 measured 108 -> 101 and moved our frame 0x70 -> 0x88 (0x88 rather than 0x80 only because we still burn a 7th callee-saved register).

- [s1] REGISTER-SEAT ROTATION at floor 101 (193 insns emitted vs target 194 - the shapes agree almost everywhere, the register fields do not): arg0 target $s1 / ours $s0; var_s0 target $s0 / ours $s1; ctx target $s2 / ours $s4; var_s3 (mode) target $s3 / ours $s2; c60 target $s4 / ours $s5; plus &D_800A3590[i] in our $s3 with no target counterpart. Target spends 5 callee-saved + ra, we spend 6 + ra.

- [s1] PASS ATTRIBUTION (read from tmp/grind/func_80070C70/dumps/text1b.loop, function section at line 44793, "Loop from 284 to 415: 50 real insns"): `Insn 312: regno 126 (life 2), move-insn savings 1  moved to 486` hoists the loop-invariant symbol address of D_800A3590; that gives the array-address giv an `add (reg:SI 126)` term (`Insn 317: giv reg 129 src reg 75 benefit 4 ... replaceable mult 2 add (reg:SI 126)`); `giv at 317 combined with giv at 319` then clears the reduction threshold that the bare `mult 2 add 0` giv at insn 315 fails (`giv of insn 315 not worth while, 0 vs 50`); and `giv at 319 reduced to (reg:SI 159)` creates the induction pointer that occupies our 7th callee-saved register. Target hoists nothing and re-materializes lui %hi / addu / lh %lo inside the loop every iteration. The lever is the hoist at insn 312 (savings 1 - a marginal decision), not the spelling of the load.

- [s1] D_800A3560's access escapes strength reduction in OUR build too, keeping the fused lui/addu/lbu %lo form exactly like target, because its index ctx_or_var_s2 is itself a biv (const 3) rather than a giv of the loop counter. That asymmetry explains why the array-declaration fix corrected one access shape and left the other divergent.

- [s1] SIBLING func_8006F97C (src/text1b.c, active, floor 513, 1 session) was checked as the brief requires: it has NO candidate.c, so there was no banked spelling to transplant and nothing could be measured from it. It does call func_80069898 with a stack struct, so it is both the best source for IconC70's real tail layout and a likely beneficiary of the IconC70 = 0x20 finding - its ledger should be told.
