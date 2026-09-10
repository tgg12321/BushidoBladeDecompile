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

- [s2 2026-09-10] CHASSIS RE-MEASURED. The s1 candidate.c body plus its three companion edits
  (`extern u8 D_800A3560[];`, `extern s16 D_800A3590[];` at both occurrences each, and IconC70
  sized 0x20) applied to a clean HEAD measures `sandbox --disable all` score 101 / target_insns 194
  / build_insns 193 — identical to s1. The floor is reproducible and the ledger's 101 is current.

- [s2 2026-09-10] THE LICM GATE IS QUANTIFIED. loop.c:1631 is
  `(threshold * savings * m->lifetime) >= insn_count`, with threshold set at loop.c:532 to
  `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`; this loop calls func_8007352C so the
  multiplier is 1 and threshold is a per-build constant no C can move. For insn 312
  (`(set (reg:SI 126) (symbol_ref "D_800A3590"))`) savings = 1 and m->lifetime = 2. Five measured
  points on the SAME chassis, obtained by adding throwaway statements to the inner loop and reading
  the .loop dump (full table + method in tmp/grind/func_80070C70/s2/licm_threshold_bracket.md):
  insn_count 50 -> moved; 52 -> moved; 60 -> not desirable; 64 -> not desirable; 73 -> not
  desirable. The life-1 movables in the same loop (insns 295/299) are "not desirable" at
  insn_count 50. Those bracket **26 <= threshold <= 29** for this build.
  Two exact consequences: (a) the insn_count route needs the inner loop to carry 53-59+ RTL insns
  and is therefore UNREACHABLE by byte-faithful C, because the target's loop is SMALLER in RTL
  terms than ours (it has neither the symbol move nor the separate address plus); (b) the
  **lifetime route is live** — at m->lifetime == 1 the product is 26..29, comfortably below
  insn_count 50, so the movable would be rejected outright.

- [s2 2026-09-10] KILLING THE HOIST IS PROVEN SUFFICIENT (this is the causal step s1's frontier
  was missing). In the 60-insn diagnostic build, where loop.c rejected insn 312, the emitted
  assembly for the D_800A3590 read is `lh $2,D_800A3590($2)`
  (tmp/grind/func_80070C70/dumps/text1b.s:16616) — the ASPSX/gas macro that expands to
  `lui $at,%hi; addu $at,$at,$2; lh %lo($at)`, byte-for-byte the target's shape at
  80070E5C-80070E64 — and the preheader carries no `la`/`lui+addiu` of D_800A3590 at all. So the
  entire residual chain (hoist -> the giv's invariant `add (reg 126)` term -> the 317/319 combine
  -> `giv at 319 reduced to (reg:SI 159)` -> a 7th callee-saved induction pointer -> frame 0x88 vs
  target 0x80 -> the 5-seat rotation on top of it) hangs off this ONE decision. Fix insn 312 and
  the rest is expected to follow.

- [s2 2026-09-10] WHY m->lifetime IS 2, AND WHERE THE FUSION IS ACTUALLY LOST. Read from
  tmp/grind/func_80070C70/dumps/text1b.cse (the pre-loop RTL), the two array reads in the same
  loop expand differently:
      insn 291: (set (reg/v:QI 118) (mem/s:QI (plus:SI (reg/v:SI 74) (symbol_ref "D_800A3560"))))
      insn 312: (set (reg:SI 126) (symbol_ref "D_800A3590"))
      insn 315: (set (reg:SI 128) (ashift:SI (reg/v:SI 75) (const_int 1)))
      insn 317: (set (reg:SI 129) (plus:SI (reg:SI 128) (reg:SI 126)))
      insn 319: (set (reg:HI 130) (mem/s:HI (reg:SI 129)))
  D_800A3560 has element size 1, so its ARRAY_REF offset is a bare pseudo, the PLUS canonicalises
  reg-first, `(plus reg symbol_ref)` is a legitimate MIPS address, and NO pseudo is ever created
  for the symbol — which is exactly why s1 observed that access matching the target while the
  other diverged. D_800A3590 has element size 2, so its offset is a `mult`, the PLUS comes out
  symbol-first, memory_address (explow.c) cannot accept it, and force_reg'ing the whole address
  materialises the symbol into reg 126. reg 126's lifetime is 2 for the precise reason that the
  index scale (insn 315) is emitted BETWEEN the symbol move (312) and the address plus (317): make
  the scaled index already available when the address is expanded and 312/317 become adjacent,
  lifetime falls to 1, and the movable is rejected. That is the concrete, measured next lever.

- [s2 2026-09-10] SEVEN ORDINARY-C SPELLINGS MEASURED, ALL NEUTRAL (details in hypotheses.md
  K1/K2/K3). Four re-associations / pointer forms of the read
  (`(D_800A3590[var_s0] << 4) + t`, `*(D_800A3590 + var_s0)`, the split-init
  `prim.p_static = prim.p_geom + 0xC; prim.p_static += D_800A3590[var_s0] << 4;`, and
  `D_800A3590[var_s0 + 0]`), plus the array-dimension declarations `extern s16 D_800A3590[64];`
  and `[1]`, all measure 101 with the preheader `addiu s3,s3,%lo(D_800A3590)` (the hoisted `la`)
  still present. Note that the split-init form is an equally natural source for the target's two
  stores to prim.p_static at 80070E58 and 80070E74; it is byte-neutral against the `s32 t` form
  candidate.c carries, so it is a free alternative spelling if a reviewer ever objects to the
  two consecutive assignments, but it was NOT adopted this session because the `s32 t` form keeps
  the second store's addend in a register instead of re-reading the escaped struct member.

- [s2 2026-09-10] D_800A3558's TWO DISTINCT LOADS ARE NOT A CAST PROBLEM. The target reads
  %gp_rel(D_800A3558) twice with different signedness in the same region — `lhu $a2` at 80070DF4
  and 80070ECC (feeding `sll 16 / sra 16 / addu $a1` in the body) and `lh $v0` at 80070E08 and
  80070ED0 (feeding the loop bound). Declaring `extern s16 D_800A3558;` and spelling the bound as
  `(D_800A3558 + 1)` and the body as `(s16)(u16)D_800A3558` measured 101 and still emitted a
  single `lh`: GCC 2.7.2's convert_to_integer folds `(short)(unsigned short)x` to `(short)x` for
  both an s32-declared and an s16-declared x. So no cast spelling on a single integer declaration
  of that symbol produces the pair. Whatever produces target's `lhu` is either a differently-typed
  declaration (a u16 object) or an intermediate object of type u16 — worth one probe next session,
  but it is a 2-3 insn shape issue, not the register residual.

- [s2] Chassis reproducible: restoring s1's candidate.c body plus its three companion edits onto a clean HEAD measures score 101 / target_insns 194 / build_insns 193, identical to s1. src/text1b.c was reverted to HEAD at end of session; the tree carries only ledger changes.

- [s2] loop.c:1631 is the gate: `(threshold * savings * m->lifetime) >= insn_count`. loop.c:532 sets threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); this loop calls func_8007352C so the multiplier is 1. Measured bracket for this build: 26 <= threshold <= 29.

- [s2] Measured insn_count sweep on one chassis (diagnostic statements, each reverted): 50 -> insn 312 moved to 486; 52 -> moved to 491; 60 -> not desirable; 64 -> not desirable; 73 -> not desirable. Life-1 movables (insns 295, 299) are already 'not desirable' at insn_count 50.

- [s2] The insn_count route is unreachable by byte-faithful C: the loop would need 53-59+ RTL insns and the target's own loop is smaller in RTL terms than ours, having neither the `(set reg (symbol_ref))` move nor a separate address plus.

- [s2] The lifetime route is open by a wide margin: at m->lifetime == 1 the product is 26..29 against insn_count 50, so the movable is rejected. reg 126's lifetime is 2 solely because insn 315, the `(ashift (reg 75) (const_int 1))` index scale, sits between the symbol move at 312 and the address plus at 317.

- [s2] Killing the hoist is sufficient, not merely necessary: in the 60-insn diagnostic build the read emits `lh $2,D_800A3590($2)` (tmp/grind/func_80070C70/dumps/text1b.s:16616), the assembler macro for target's exact lui %hi / addu / lh %lo at 80070E5C-80070E64, with no `la` of D_800A3590 in the preheader. The scale-1 control in the same loop is `lbu $3,D_800A3560($19)` (line 16595).

- [s2] RTL root cause read from tmp/grind/func_80070C70/dumps/text1b.cse: D_800A3560 (element size 1) expands to the fused `(mem (plus (reg 74) (symbol_ref)))` at insn 291 and never creates a symbol pseudo; D_800A3590 (element size 2) expands to insn 312 symbol move / insn 315 ashift / insn 317 plus / insn 319 load, because a `mult` offset makes the PLUS come out symbol-first and memory_address (explow.c) has to force_reg the whole address. This is the mechanism behind s1's observed asymmetry between the two accesses.

- [s2] Seven ordinary-C spellings measured byte-neutral this session: four re-associations / pointer forms of the D_800A3590 read, two completed array-bound declarations of D_800A3590, and an s16 re-declaration of D_800A3558 with recast use sites. All banked under memory/grind/func_80070C70/rejected/ with their reasons.

- [s2] The split-init spelling `prim.p_static = prim.p_geom + 0xC; prim.p_static += D_800A3590[var_s0] << 4;` is byte-neutral against the `s32 t` form candidate.c carries, and is an equally natural source for the target's two stores to prim.p_static at 80070E58 and 80070E74. Recorded as a free alternative, not adopted.

- [s2] IconC70's tail (`s16 sp50[12]`) is still the s1 placeholder and still blocks submission. Not touched this session; F3 in hypotheses.md carries it forward with the five func_80069898 callers to read.

- [s2] Sibling check: func_800720FC (src/text1b.c, active, floor 688, 1 session) names this function but has no candidate.c, so there was again no banked spelling to transplant. It is one of the five func_80069898 callers named in F3, so it is the natural place to recover IconC70's tail alongside func_8006F97C.
