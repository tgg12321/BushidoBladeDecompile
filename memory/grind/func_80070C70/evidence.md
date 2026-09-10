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

- [s3 2026-09-10] FLOOR 101 -> 99, AND THE INSN COUNT NOW MATCHES TARGET EXACTLY (194 vs 194).
  The single change is a DECLARATION: `extern s16 D_800A3590[];` -> an array of a 2-byte record
  (`typedef struct RecC70 { s16 v; } RecC70; extern RecC70 D_800A3590[];`, both occurrences,
  src/text1b.c l.2129 and l.6500) with the read spelled `D_800A3590[var_s0].v`. Measured
  `sandbox --disable all` score 99 / target_insns 194 / build_insns 194 (was 101 / 194 / 193).
  `extern s16 D_800A3590[][1];` with the read `D_800A3590[var_s0][0]` measures IDENTICALLY 99, so
  the win belongs to the ACCESS PATH, not to the particular record spelling.

- [s3 2026-09-10] THE MECHANISM, READ OUT OF THE GCC 2.7.2 SOURCES (this is s2's F1 answered, and
  the answer was not "shorten the pseudo's lifetime" - it was "never create the pseudo").
  * `D_800A3590[var_s0]` on a bare `extern s16 D_800A3590[]` builds a real ARRAY_REF
    (c-typeck.c build_array_ref takes the ARRAY_TYPE branch), and expand_expr's ARRAY_REF case
    (expr.c:4589) rewrites a NONCONSTANT index into `*(&array + index*size)`. The address rtx is
    therefore `(plus (mult (reg) (const_int 2)) (symbol_ref))`.
  * MIPS GO_IF_LEGITIMATE_ADDRESS (tools/gcc-2.7.2/config/mips/mips.h) accepts a PLUS only when one
    operand is a REG and the other a CONST_INT or a CONSTANT_ADDRESS_P term, and it swaps the two
    operands ONLY under `code0 != REG && code1 == REG`. With a MULT on one side and a SYMBOL_REF on
    the other, neither operand is a REG, no swap fires, and the address is rejected.
  * memory_address (explow.c:385) then runs break_out_memory_refs, which force_reg's the SYMBOL_REF
    into a fresh pseudo (s2's insn 312), after which force_operand (expr.c) expands operand 0 first
    and emits the scale (insn 315) and then the add (insn 317). That emission ORDER is precisely
    why the symbol pseudo's LOOP_REG_LIFETIME was 2, which is what let loop.c:1631 hoist it.
  * `D_800A3590[var_s0].v` is a COMPONENT_REF, so expand_expr takes the get_inner_reference path
    instead: get_inner_reference (expr.c:3608) folds the ARRAY_REF into a byte `offset` tree, and
    the COMPONENT_REF case expands that offset with `expand_expr (offset, NULL_RTX, VOIDmode, 0)`
    - a REAL insn, emitted BEFORE anything else - then force_reg's it and builds
    `(plus (symbol_ref) (reg))`. THAT form is legitimate on MIPS (the macro's swap branch fires),
    so no symbol pseudo is needed at all, there is no loop-invariant movable, and the load comes
    out as target's fused `lui %hi / addu / lh %lo`.
  * Verified in tmp/grind/func_80070C70/dumps/text1b.loop: the `Insn 312: regno 126 (life 2),
    move-insn savings 1  moved to 486` line is GONE from the "Loop from 284 to 415" section, and
    `mipsel-linux-gnu-objdump -dr` on the sandbox object now shows exactly ONE R_MIPS_HI16 /
    R_MIPS_LO16 pair for D_800A3590 (was a preheader `la` plus the in-loop pair).
  * GENERAL LESSON worth carrying to sibling ledgers: on this compiler, a scale-1 array access
    (element size 1) reaches the fused `SYM($reg)` addressing for free, but a scale-2-or-more array
    access CANNOT - unless the reference is a COMPONENT_REF (record element, or a nested ARRAY_REF
    with a constant inner index), which routes the offset through get_inner_reference and emits it
    as a plain register before the address is formed. s1 observed this asymmetry between
    D_800A3560 and D_800A3590 and could not explain it; this is the explanation.

- [s3 2026-09-10] THE RESIDUAL AT 99 MOVED ONE PASS LATER: it is now loop.c STRENGTH_REDUCE, not
  move_movables. The new text1b.loop section reads:
      Insn 291: dest address src reg 74 benefit 2 ... mult 1 add (symbol_ref "D_800A3560")
      Insn 313: giv reg 127 src reg 75 benefit 2 ... mult 2 add 0
      Insn 319: dest address src reg 75 benefit 4 ... mult 2 add (symbol_ref "D_800A3590")
      giv of insn 291 not worth while, 0 vs 48.
      giv at 319 combined with giv at 313
      giv at 319 reduced to (plus:SI (reg:SI 159) (symbol_ref:SI ("D_800A3590")))
      giv at 313 reduced to (reg:SI 159)
  So the scale-1 D_800A3560 address giv is REJECTED exactly as in target, while the scale-2
  D_800A3590 pair is combined and reduced into a fourth induction register (emitted as
  `move s3,zero` + `addiu s3,s3,2`). Target spends no such register: it recomputes
  `sll $a0, $s0, 1` inside the loop (80070E3C, in the delay slot of the first beq). That one
  register is the entire 0x88-vs-0x80 frame delta and the seat rotation rides on top of it.

- [s3 2026-09-10] THE STRENGTH-REDUCE GATE, QUANTIFIED FROM SOURCE (loop.c:3824):
  `v->lifetime * threshold * benefit < insn_count` marks a giv "not worth while", where
  `threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` (loop.c:3241 - note this is a
  DIFFERENT threshold from move_movables' `(1 + n_non_fixed_regs)` at loop.c:532 that s2
  bracketed), and `benefit` is the recorded benefit minus `add_cost * bl->biv_count`
  (add_cost = rtx_cost of a reg+reg PLUS = 2, loop.c:307). This loop calls func_8007352C so the
  loop_has_call multiplier is 1. s2's move_movables bracket of 26..29 pins
  n_non_fixed_regs to 25..28, hence strength_reduce's threshold to 28..31.
  Arithmetic for the two givs, with insn_count 48 and lifetime 1:
    insn 291: benefit 2 - 2*1 = 0  ->  product 0  < 48  -> rejected (dump prints exactly "0 vs 48")
    insn 313+319 combined: benefit 6 - 2*1 = 4  ->  product 112..124  >= 48  -> reduced
    insn 319 alone (if the combine were prevented): 4 - 2 = 2 -> product 56..62 >= 48 -> STILL
      reduced. Rejection would need insn_count > 62, i.e. a loop 15+ RTL insns larger than ours,
      and target's loop is SMALLER than ours, so the insn_count route is closed here too.
  The only terms left in the inequality that C can touch are `benefit` (which is fixed at 4 for a
  mult-2 DEST_ADDR giv because general_induction_var charges 2 per rtx operation) and
  `bl->biv_count` (benefit would fall to 0 if biv 75's class carried TWO increment insns).

- [s3 2026-09-10] SIX RESTATEMENTS AND THREE D_800A3558 RE-TYPINGS MEASURED, ALL 99 -> 99.
  Banked in rejected/loop-body-restatements-at-99.c and rejected/u16-decl-D_800A3558-lhu-lh-pair.c.
  The three D_800A3558 probes close s2's F2 "u16 object" route on this chassis: neither
  `extern u16 D_800A3558;` (with the bound cast or uncast) nor `extern s16 D_800A3558;`
  reproduces the target's `lhu` + `sll 16` + `sra 16` / `lh` pair. The six restatements re-confirm
  at floor 99 what s1 found at 108: statement-level spelling inside the loop moves the score by
  exactly 0 while the allocation residual saturates it.

- [s3 2026-09-10] PERMUTER CAMPAIGNS (telemetry in metrics/events.jsonl; artifacts under
  tmp/perm_70c70). Campaign A on the floor-101 chassis: base permuter score 5083, 12,658
  iterations, 562 outputs, best 4198. Its single best find was a `new_var = prim.p_geom;`
  temp lifted ACROSS a reassignment of prim.p_geom - semantically WRONG (it captures
  *(ctx+4) and then uses it where the source needs *(ctx)), so it is a scorer artifact, not a
  proposal; recorded so no later session re-chases it. Campaign B was re-seeded on the floor-99
  chassis after the declaration win (base permuter score 4778, i.e. the chassis improvement shows
  in the permuter metric too).

- [s3 2026-09-10] TOOL NOTE: `tools/ra_solver/inverse_compose.py classify text1b func_80070C70
  --target-object tmp/perm_70c70/target.o --ours-object tmp/sandbox/func_80070C70/text1b.o`
  fails with `func_80070C70 not found in tmp/perm_70c70/target.o` even though
  `mipsel-linux-gnu-nm` shows `00000000 T func_80070C70` in that object and `objdump -d` renders
  194 instructions for it. The single-function .o that permuter workspaces build from
  asm/funcs/<f>.s is therefore NOT usable as the classifier's --target-object. A session that
  wants H4's typed RA verdict must build the target object some other way (or fix the extractor);
  do not burn turns re-trying this exact invocation.

- [s3] FLOOR 101 -> 99 with build_insns 193 -> 194, equal to the target's 194 for the first time. The only change is the declaration of D_800A3590: `extern s16 D_800A3590[];` becomes `typedef struct RecC70 { s16 v; } RecC70; extern RecC70 D_800A3590[];` at both occurrences (src/text1b.c l.2129 and l.6500) with the read spelled `D_800A3590[var_s0].v`.

- [s3] MECHANISM, read out of the GCC 2.7.2 sources rather than guessed: a nonconstant-index ARRAY_REF on a scale-2 array is rewritten by expr.c:4589 into `*(&array + index*2)`, giving the address `(plus (mult (reg) 2) (symbol_ref))`; MIPS GO_IF_LEGITIMATE_ADDRESS (config/mips/mips.h) rejects it because neither operand is a REG (its operand swap fires only when `code0 != REG && code1 == REG`); memory_address (explow.c:385) then break_out_memory_refs's the SYMBOL_REF into a pseudo and force_operand emits the scale after it, which is exactly why the symbol pseudo had lifetime 2 and loop.c:1631 hoisted it. A COMPONENT_REF instead takes expand_expr's get_inner_reference path, which expands the byte offset FIRST as a real insn and force_reg's it, forming `(plus (symbol_ref) (reg))` -- legitimate on MIPS via the swap branch -- so no symbol pseudo and no movable ever exist.

- [s3] GENERAL LESSON for sibling ledgers in this TU and beyond: on GCC 2.7.2 / MIPS, an element-size-1 array access reaches the fused `SYM($reg)` addressing for free, but an element-size-2-or-more array access cannot unless the reference is a COMPONENT_REF (record member, or a nested ARRAY_REF with a constant inner index). This is the explanation for the asymmetry s1 observed between D_800A3560 (matched) and D_800A3590 (diverged) and could not account for.

- [s3] THE RESIDUAL MOVED ONE PASS LATER, from loop.c move_movables to loop.c strength_reduce. The regenerated dump reads `giv of insn 291 not worth while, 0 vs 48` for the scale-1 D_800A3560 address giv (matching target), and `giv at 319 combined with giv at 313` / `giv at 313 reduced to (reg:SI 159)` for the scale-2 D_800A3590 pair, producing a fourth induction register emitted as `move s3,zero` plus `addiu s3,s3,2`. Target spends no such register: it recomputes `sll $a0, $s0, 1` in-loop at 80070E3C.

- [s3] THE STRENGTH-REDUCE GATE IS loop.c:3824 `v->lifetime * threshold * benefit < insn_count`, with `threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` at loop.c:3241 (NOT the `(1 + n_non_fixed_regs)` of loop.c:532 that s2 bracketed) and benefit reduced by `add_cost * bl->biv_count` with add_cost = 2 (loop.c:307). s2's 26..29 bracket pins n_non_fixed_regs to 25..28, so this threshold is 28..31. With insn_count 48 and lifetime 1: insn 291 gives benefit 0 and product 0 (rejected, dump prints `0 vs 48`); the combined 313+319 pair gives benefit 4 and product 112..124 (reduced); insn 319 alone would give benefit 2 and product 56..62, still >= 48. Only `benefit` and `bl->biv_count` remain as terms C can touch -- the insn_count route needs a loop 15+ RTL insns larger than ours and the target's loop is smaller.

- [s3] NINE MEASUREMENTS ALL READ 99 on the new chassis: three D_800A3558 re-typings (u16 with cast bound, u16 with uncast bound, s16) and six loop-body / declaration restatements. This re-confirms s1's saturation finding at the new floor -- statement-level spelling inside the loop moves the score by exactly 0 while the seat rotation plus the extra reduced-giv register dominate it.

- [s3] THE RECORD DECLARATION IS THE OPEN SUBMISSION BLOCKER, NOT THE FLOOR. The 2-byte stride is proven by the target's bytes, but `struct RecC70 { s16 v; }` is a codegen-motivated spelling of that stride and `extern s16 D_800A3590[][1];` is equally contrived (it measures the same 99). The real record type has to be recovered from D_800A3590's other consumers -- func_8006F100, func_80070188, func_80070F78 (all still INCLUDE_ASM) and src/text1b_b.c:271 -- and per the brief's DATA MODEL block the census row keeps its `alias of g_replay_motion_shared_state_d+6` suffix. If those consumers show a wider record with the halfword at offset 0 the declaration becomes evidence-backed; if they show a bare halfword array this needs a ruling-request before any submission.

- [s3] PERMUTER TELEMETRY: campaign A (floor-101 chassis) base 5083, 12,658 iterations, 562 outputs, best 4198; campaign B (floor-99 chassis, re-seeded after the declaration win per the 2026-09-01 chassis rule) base 4778, 31,512 iterations, 1,241 new outputs, best 3498. Both harvested with --stop, no campaign outlives the session. Neither best find is usable: A's lifts a temp across a reassignment (semantically wrong) and B's is built entirely out of the permuter's synthetic `inline_fn` helpers. Set `perm_inline = 0.0` in settings.toml for any future campaign here.

- [s3] TOOL NOTE (saves a later session real turns): `tools/ra_solver/inverse_compose.py classify text1b func_80070C70 --target-object tmp/perm_70c70/target.o --ours-object tmp/sandbox/func_80070C70/text1b.o` fails with `func_80070C70 not found in tmp/perm_70c70/target.o` even though nm shows `00000000 T func_80070C70` there and objdump renders its 194 instructions. The single-function .o that a permuter workspace assembles from asm/funcs/<f>.s is not usable as the classifier's --target-object; H4's typed RA verdict needs the target object built some other way.

- [s3] SIBLING CHECK as the brief requires: func_8006F97C (floor 513, 1 session) and func_800720FC (floor 688, 1 session) still carry no candidate.c, so there was again no banked spelling to transplant. Both are inheritors of this session's findings rather than sources for it: they share the TU, they are both callers of func_80069898 (so they bear on IconC70's tail, frontier F3), and func_8006F97C is one of D_800A3590's other consumers (so it bears on frontier F2'). The scale-2 COMPONENT_REF addressing lesson above applies to any array access they carry.

- [s3] src/text1b.c was reverted to HEAD at end of session; the tree carries only memory/grind/func_80070C70 ledger changes (candidate.c, evidence.md, hypotheses.md, two new rejected/ files).

## [s4] The extra induction register that s3 (H7) pinned as the residual is removed by giving var_s0 THREE source increment sites: `bl->biv_count` 3 drives the combined D_800A3590 address giv's benefit to 0 and loop.c:3824 rejects the reduction.
- mechanism: tools/gcc-2.7.2/loop.c:3824 marks a giv "not worth while" when `v->lifetime * threshold * benefit < insn_count`, where the benefit used is the recorded benefit minus `add_cost * bl->biv_count` (add_cost = rtx_cost of a reg+reg PLUS = 2, loop.c:307) and threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) (loop.c:3241), bracketed to 28..31 by s2/s3. `bl->biv_count` counts the induction INCREMENT insns recorded for the biv register, so the C source controls it directly by how many times it writes `var_s0 += 1`. biv_count 1 -> benefit 6-2 = 4 -> product 112..124 >= insn_count -> reduced (s3's chassis, and its 4th induction register `move sN,zero` / `addiu sN,sN,2`); biv_count 2 -> benefit 2 -> product 56..62 >= 48 -> still reduced; biv_count 3 -> benefit 0 -> product 0 < insn_count -> REJECTED, which is what the target requires (it recomputes `sll $a0, $s0, 1` in-loop at 80070E3C, in the delay slot of the first beq).
- probe: Rewrote the loop so `var_s0 += 1;` appears in all three arms - both arms of the inner mode `if`, and a plain `else` on the `code != 5 && code != 16` test - while `var_s3 += 0x16C;` and `ctx_or_var_s2 += 3;` remain in a SINGLE shared fall-through block after the outer `if/else` (the target's .L80070EC4 shape). Measured `& tools/wteng.ps1 main sandbox func_80070C70 --disable all`; re-ran `pwsh tools/grinder/dump.ps1 func_80070C70` and re-read the `Loop from 284 to 460` section of tmp/grind/func_80070C70/dumps/text1b.loop; confirmed the emitted loop against `mipsel-linux-gnu-objdump -dr tmp/sandbox/func_80070C70/text1b.o`.
- result: 99 -> 61, build_insns 194 -> 192. The dump prints `giv at 337 combined with giv at 331` then `giv of insn 331 not worth while, 0 vs 63`; s3's `giv at 313 reduced to (reg:SI 159)` is GONE, and insn 291's scale-1 D_800A3560 giv is rejected harder (`-124 vs 63`). WHICH statement is duplicated matters as much as the count: duplicating the whole call tail into the mode arms also reaches biv_count 3 but measures 88 at 200 insns, because `prim.code = 1` then appears twice in the loop and move_movables hoists the constant 1 into a SEVENTH callee-saved register (`li s4,1` in the preheader). Nine spellings were measured (88 / 88 / 93 / 93 / 97 / 77 / 98 / 98 / 65) before the 61 form; all banked in memory/grind/func_80070C70/rejected/over-duplicated-arms-at-biv3.c.
- verdict: CONFIRMED

## [s4] A permuter campaign on the biv_count-3 chassis, run with perm_inline = 0.0, produced two usable spelling proposals that take the floor 61 -> 56 - the first usable campaign output this function has had.
- mechanism: Mandated modality. s3 banked two campaigns with no usable find (floor-101 and floor-99 chassis, 44k iterations) and recommended `perm_inline = 0.0` in settings.toml's `[weight_overrides]`, because the permuter's synthetic `inline_fn` helpers otherwise dominate the output stream with forms that are not decomp C. The 2026-09-01 chassis rule also forbids re-seeding a chassis that already banks a 0-find campaign, so this campaign was seeded on the structurally new biv_count-3 loop.
- probe: `tools/permuter_campaign.py launch --func func_80070C70 --dir tmp/perm_70c70_s4 --label s4-biv3-chassis-floor61 -j 8`; waited in-turn via `permuter_campaign.py wait`; harvested with `--stop`. Base permuter score 4228 (vs 4778 at floor 99 and 5083 at floor 101), 25,626 iterations in 869 s, 288 new outputs, best 3325. The best find was read by hand and its two real changes re-spelled and measured individually (tmp/grind/func_80070C70/s4/probe6.py).
- result: (i) `prim.link = *(s32 *)(arg0 + 0x10);` written BEFORE `prim.code = 1;` at the FIRST func_8007352C call site - plain statement reordering, ordinary C - measures 61 -> 58. (ii) reading `*(s32 *)(ctx_or_var_s2 + 8)` into a named intermediate `new_var` before the `*(arg0 + 0x18) += 0xC` store and assigning it to prim.p_geom after measures 59 alone (190 insns); both together measure 56 at 190 insns. The find's third change, `var_s0 = (unsigned long long) 0`, is a redundant width cast (forbidden family F2) and was discarded. Both adopted; candidate.c now carries them.
- verdict: CONFIRMED

## [s4] A u16 or s16 declaration of D_800A3558 reproduces the target's tail-block `lhu` plus in-body `sll 16` / `sra 16` pair on the biv_count-3 chassis (re-measurement of s3's K4, which was chassis-relative).
- mechanism: s2's K1 showed convert_to_integer folds `(short)(unsigned short)x` to `(short)x`; s3's K4 measured the u16/s16 declaration route byte-neutral on the floor-99 chassis. Because kills are chassis-relative and this session changed the chassis, the route was re-measured.
- probe: Four spellings on the floor-61 chassis via tmp/grind/func_80070C70/s4/probe5.py: `extern u16 D_800A3558;` with body `(D_800A35B0 + (s16)D_800A3558)`; the same with the bound spelled `(s32)(D_800A35B0 + (s16)(D_800A3558 + 1))`; `extern s16 D_800A3558;` with body `(D_800A35B0 + (s16)(u16)D_800A3558)`; and the u16 declaration with the loop-entry test unchanged.
- result: 61 / 67 / 61 / 61 - every byte-neutral spelling still emits a single `lh` in the body block and none reproduces the target's pair. The structural fact the target shows is that its zero-extending load sits in a DIFFERENT basic block (the loop tail, 80070ECC) from its sign-extension (80070E78/E7C in the next iteration's body), which is what prevents combine from folding the sign_extend into the load; no declaration spelling moves a load across a basic-block boundary.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-61 chassis (s4 candidate.c loop + `extern u8 D_800A3560[];` + the RecC70 record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present, `sandbox --disable all`, 2026-09-10

## [s4] SIBLING CHECK as the brief requires: func_8006F97C (floor 513), func_80070188 (696), func_80070F78 (808) and func_800720FC (688) still carry no candidate.c, so there was again no banked spelling to transplant onto this chassis. The transplant runs the OTHER way this session: the biv_count lever above is a general loop.c result that applies to any of them whose residual is a reduced address giv, and the RecC70 / IconC70 type questions (frontiers F6) are answered from their asm.
- mechanism: Brief's SIBLING LEDGERS block; all four are active queue siblings in the same TU (src/text1b.c) and none has been through a grind session beyond its s1 recon.
- probe: Re-read the brief's sibling table; no candidate.c exists for any of them.
- result: Nothing to transplant in. Recorded for the next session: the s4 finding to propagate is "count the source increment sites of the loop counter before blaming the allocator - loop.c:3824's benefit term is `recorded_benefit - 2 * bl->biv_count`".
- verdict: CONFIRMED

## [s4] src/text1b.c was reverted to HEAD at end of session; the tree carries only memory/grind/func_80070C70 ledger changes (candidate.c, evidence.md, hypotheses.md, two new rejected/ files).

- [s4] Honest floor moved 99 -> 61 -> 58 -> 56 this session, all measured with `& tools/wteng.ps1 main sandbox func_80070C70 --disable all`; build_insns 194 -> 192 -> 190 against target 194.

- [s4] loop.c's giv gate has a term the C source controls directly: the benefit used at loop.c:3824 is the recorded benefit MINUS add_cost * bl->biv_count (add_cost = 2, loop.c:307), and bl->biv_count is the number of source-level increment sites of the biv. Three `var_s0 += 1;` sites take the combined D_800A3590 address giv from benefit 4 (reduced) to benefit 0 (rejected).

- [s4] The dump line that proves it: text1b.loop's `Loop from 284 to 460` section now reads `giv at 337 combined with giv at 331` then `giv of insn 331 not worth while, 0 vs 63`, where s3's dump at the same site read `giv at 313 reduced to (reg:SI 159)`.

- [s4] biv_count 2 is not enough and is actively worse (101/101/102 vs the 99 one-site chassis) - the gate needs benefit <= 1.

- [s4] Duplicating more than `var_s0 += 1` costs: the call tail duplicated into the mode arms puts `prim.code = 1` in the loop twice, and move_movables then hoists the constant 1 into a seventh callee-saved register (`li s4,1` in the preheader).

- [s4] The skip arm must be a plain `else`, not a `continue` carrying its own increments: with `else`, `var_s3 += 0x16C;` and `ctx_or_var_s2 += 3;` stay a single shared fall-through block, exactly the target's .L80070EC4 (65 at 195 insns vs 61 at 192 insns).

- [s4] The permuter is useful on this function once the chassis is right and perm_inline is zeroed: base permuter score fell 5083 (floor 101) -> 4778 (99) -> 4228 (61), and this campaign's best find (3325) contained two adoptable changes, where s3's two campaigns on the pre-biv_count chassis contained none.

- [s4] Remaining 2-insn shortfall at 190 vs 194: the target's `lhu $a2, %gp_rel(D_800A3558)` in the loop TAIL block (alongside the condition's own `lh $v0` of the same address) plus `sll $v0,$a2,16` / `sra $v0,$v0,16` in the NEXT iteration's body. Declaration retyping is spent across s2 K1, s3 K4 and s4 K7; the discriminator is basic-block placement, not type.

- [s4] Callee-saved seats are still rotated at floor 56 (target: arg0=$s1, var_s0=$s0, var_s3=$s3, ctx_or_var_s2=$s2, c60=$s4, six saved slots incl. $ra, frame 0x80), but the spurious allocno is gone, so this is now a clean 5-seat permutation - ra_solver territory.

- [s4] The RecC70 and IconC70 declarations are still codegen-motivated placeholders and still block submission independently of the floor (frontier F6, carried unspent from s1).

- [s4] src/text1b.c was reverted to HEAD at end of session; the tree carries only memory/grind/func_80070C70 ledger changes.

- [s5] [enumerate 2026-09-10] FLOOR 56 -> 53, and the chassis is now ORDINARY C with zero FAKE
  constructs. The second loop is spelled `for (var_s0 = 0; var_s0 < D_800A35B0 + (s16)D_800A3558 + 1;
  var_s0++)` with the two secondary counters expressed as functions of var_s0
  (`D_800A3560[var_s0 * 3]`, `prim.mode = 0x50 + var_s0 * 0x16C`). s4's three-arm `var_s0 += 1`
  biv_count construct, the `new_var` named intermediate and the RecC70 record are all retired.
- [s5] MECHANISM CHAIN (read out of tools/gcc-2.7.2/, not inferred): a top-test loop makes
  jump.c:2163 `duplicate_loop_exit_test` copy the exit test in front of the loop (= the target's
  `blez` guard at 80070E18) and mark the original test's registers REG_LOOP_TEST_P (jump.c:2253).
  cse.c:8581 then calls `cse_around_loop` (cse.c:7741) whose `cse_set_around_loop` (cse.c:7909)
  rewrites a loop-HEAD SET_SRC to a REG_LOOP_TEST_P register the TAIL test already loaded. This is
  the ONLY mechanism in GCC 2.7.2 that puts a load in the loop tail and consumes it at the top of
  the next iteration -- i.e. the target's `lhu $a2, %gp_rel(D_800A3558)` at 80070ECC feeding
  `sll $v0,$a2,16 / sra $v0,$v0,16` at 80070E78/E7C, which four sessions of declaration retyping
  (s2 K1, s3 K4, s4 K7) could not reproduce. combine cannot fold the sign_extend into the load
  because they are in different basic blocks, which is why the target shows BOTH `lhu` and `lh` of
  the same address in the tail block.
- [s5] `duplicate_loop_exit_test` bails on an exit test containing a CALL_INSN or CODE_LABEL, on a
  nested-loop NOTE_INSN_LOOP_BEG, and on any exit test longer than 20 insns (jump.c:2175-2220).
  This loop's bound (three loads, two adds, the compare) is well inside that.
- [s5] The target's `addiu $s3,$zero,0x50` / `addu $s2,$zero,$zero` at 80070E20/E24 sit BETWEEN the
  guard branch and the loop label. Nothing in an `if (...) { init; do { } while (); }` chassis can
  put them there: source statements before a loop precede the NOTE_INSN_LOOP_BEG, and both
  duplicate_loop_exit_test and move_movables insert before that note. They are reduced-GIV
  initialisations emitted by loop.c's strength_reduce, which is what proves var_s3 and ctx are
  expressions of the loop counter in the original source, not independent accumulators.
- [s5] The callee-saved seat rotation that s1/s2/s4 tracked as frontier item F5 (target arg0=$s1,
  var_s0=$s0, var_s3=$s3, ctx=$s2, c60=$s4) comes out CORRECT on the for-loop chassis with no RA
  work at all. It was a symptom of the wrong loop structure, not an allocation problem, so
  tools/ra_solver is no longer the right next tool for this function.
- [s5] D_800A3590 is a plain `extern s16 D_800A3590[]` indexed by the loop counter. The RecC70
  record measures 90 vs the array's 53 on this chassis. The s1/s2/s3/s4 frontier item "recover
  RecC70's real field list from its other consumers" is CLOSED -- there is no record. IconC70's
  `s16 sp50[12]` tail is the only placeholder declaration still blocking submission.
- [s5] D_800A3558's declared type is byte-neutral at 53: `extern s32` with an `(s16)` cast, `u16`,
  and `s16` all measure 53. Keep the ordinary `extern s32 D_800A3558;`.
- [s5] RESIDUAL AT 53 (193 insns vs 194). (i) frame 0x98 vs 0x80: three pseudos (118, 168, 171)
  exist only as `(use (reg))` insns created between .flow and .lreg, have no conflicts, get no hard
  register in .greg, and take 8-byte stack slots at sp+104/112/120 -- leftovers of
  duplicate_loop_exit_test's `reg_map` copies. Every sp-relative insn differs by the offset, so this
  is the dominant scoring item. (ii) D_800A3560's giv is reduced to a full ADDRESS
  (`lui s2 / addiu s2 / lbu 0(s2)`, `addiu s2,s2,3`) where the target reduces it to a byte OFFSET
  and re-adds `%hi/%lo` every iteration. (iii) `t = prim.p_geom + 0xC` lands in p_geom's own
  register in three places where the target keeps two live registers (`addiu $v1,$v0,0xC`).
- [s5] TOOLING: tmp/grind/func_80070C70/s5/sweep.py is a guard-clean variant sweeper (splices a body
  into src/text1b.c, scores via `tools/wteng.ps1 main sandbox`, always restores) that also applies
  header-declaration substitutions via `//@sub <anchor>|||<replacement>` lines at the top of a
  variant file -- tools/sweep_variants.py cannot vary declarations and is blocked by the
  worktree-contamination guard unless pinned through wteng. s5/cmp2.py is a normalising
  target-vs-built instruction differ (objdump aliases, register names and hex offsets folded).
- [s5] src/text1b.c was reverted to HEAD at end of session; the tree carries only
  memory/grind/func_80070C70 ledger changes.

- [s5] Floor 56 -> 53 with a body that is ordinary C: no duplicated increments across arms, no named intermediate, no record spelling, no dead store, no pad, no volatile, no asm. Every FAKE construct the ledger accumulated across s3/s4 is retired by the structural change.

- [s5] The matching source shape for the second loop is `for (var_s0 = 0; var_s0 < D_800A35B0 + (s16)D_800A3558 + 1; var_s0++)` with `D_800A3560[var_s0 * 3]` and `prim.mode = 0x50 + var_s0 * 0x16C` -- the secondary counters are expressions of the loop variable, and GCC's strength reduction turns them back into the target's $s3 and $s2 increments.

- [s5] jump.c:2163 duplicate_loop_exit_test fires only on a top-test loop; it is the sole source of REG_LOOP_TEST_P (jump.c:2253), which is the sole enabler of cse.c:7741 cse_around_loop. That chain is the mechanism behind the target's tail-block lhu of D_800A3558 at 80070ECC feeding the next iteration's sll 16 / sra 16 at 80070E78/E7C -- the frontier item four sessions attacked with declaration retyping.

- [s5] The target's addiu $s3,$zero,0x50 / addu $s2,$zero,$zero at 80070E20/E24 sit between the guard branch and the loop label, which is where loop.c's strength_reduce emits reduced-giv initialisations. No if-guarded do/while chassis can place them there, because source statements before a loop precede NOTE_INSN_LOOP_BEG and both duplicate_loop_exit_test and move_movables insert before that note.

- [s5] The callee-saved seat rotation tracked as frontier F5 since s1 (target arg0=$s1, var_s0=$s0, var_s3=$s3, ctx=$s2, c60=$s4) comes out correct with no RA work: it was a symptom of the wrong loop structure, not an allocation problem. tools/ra_solver is no longer the right next tool for this function.

- [s5] D_800A3590 is a plain `extern s16 D_800A3590[]` halfword array; the RecC70 record measures 90 versus 53. The long-standing frontier item asking for RecC70's real field list is closed -- there is no record. IconC70's `s16 sp50[12]` tail is now the ONLY placeholder declaration blocking submission.

- [s5] D_800A3558's declared type is byte-neutral at 53 (extern s32 with an (s16) cast, u16, and s16 all score 53), so the ordinary extern s32 declaration is kept.

- [s5] Residual at 53 (193 insns vs 194): (i) frame 0x98 vs 0x80, caused by three no-conflict pseudos (118, 168, 171) that exist only as (use (reg)) insns and take 8-byte stack slots at sp+104/112/120 -- leftovers of duplicate_loop_exit_test's reg_map copies; every sp-relative insn differs by the offset, making this the dominant scoring item. (ii) D_800A3560's giv is reduced to a full address where the target reduces it to a byte offset and re-adds %hi/%lo each iteration. (iii) `t = prim.p_geom + 0xC` lands in p_geom's own register in three places where the target keeps two live registers.

- [s5] tmp/grind/func_80070C70/s5/sweep.py is a guard-clean variant sweeper that also varies header DECLARATIONS via `//@sub <anchor>|||<replacement>` lines; tools/sweep_variants.py cannot vary declarations and is blocked by the worktree-contamination guard unless pinned through wteng.

## s6 (enumerate, 2026-09-10) — floor 53 -> 39, build_insns 194 == target, FRAME EXACT

- [s6] The floor-53 s5 chassis re-measured 53 at dispatch (sandbox --disable all, 193 insns
  vs target 194), so every s5 conclusion below is on the same chassis it was banked on.
- [s6] NEW INSTRUMENT: `vars=` from cc1's own `.frame` comment is a 1-second-per-variant
  gradient on the frame, independent of the sandbox score
  (tmp/grind/func_80070C70/s6/varsweep.sh; see [[phantom-frame-slots-gcc272]]). The target
  is `.frame $sp,128,$31  # vars= 80, regs= 6/0, args= 24, extra= 0`.
- [s6] The 0x18 frame overshoot is THREE RELOAD SPILL SLOTS, measured directly, not inferred.
  The instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_FRAME_DEBUG=1 prints the whole
  frame-slot census for the function (tmp/grind/func_80070C70/s6/framedbg.sh):
      ctx=stack_temp     size=48 frame_offset=48    <- prim  (PrimC70, 44 bytes -> 48)
      ctx=stack_temp     size=32 frame_offset=80    <- icon  (IconC70, 0x20)
      ctx=spill_new_p116 size=8  frame_offset=88
      ctx=spill_new_p165 size=8  frame_offset=96
      ctx=spill_new_p170 size=8  frame_offset=104
  The last three are pseudos that carry a reference but NO set: they appear in the dumps
  only as `(insn N (use (reg:SI 165)))` / `(use (reg:SI 170))` (grep the .lreg dump), get
  no hard register in .greg, and reload's alter_reg gives each an 8-byte slot. This ALSO
  independently PROVES IconC70's 0x20 size: prim 48 + icon 32 = the target's vars= 80 with
  nothing left over.
- [s6] PASS ATTRIBUTION, read out of the dumps rather than guessed. Regs 165/170 do not
  exist in .rtl and first appear in .jump — i.e. they are created by jump.c:2246
  `reg_map[REGNO (reg)] = gen_reg_rtx (GET_MODE (reg))` inside duplicate_loop_exit_test,
  which hands a fresh pseudo to every exit-test insn whose destination's first AND last uid
  both lie inside the exit code. The `(use (reg))` insns themselves first appear in
  .combine: combine.c:10839 emits `emit_insn_after (gen_rtx (USE, VOIDmode, XEXP (note, 0)), tem)`
  when it deletes an insn and cannot find a home for the orphaned REG_DEAD note, parking it
  on a USE at the following CODE_LABEL. So the chain is
  jump.c duplicate_loop_exit_test -> dead copies -> combine.c USE-at-label -> reload spill slot.
- [s6] THE TWO TARGET FEATURES ARE COUPLED THROUGH THE SAME TRANSFORM, and this is the
  central fact the next session inherits. cse.c:7909 `cse_set_around_loop` is the only
  mechanism that can move the loop-head reads of D_800A3558 / D_800A35B0 into the previous
  iteration's TAIL (the target's `lhu $a2` / `lw $a1` at 80070ECC-ED4 feeding
  `sll $v0,$a2,16 / sra / addu $v0,$a1,$v0` at 80070E78-E80). Its gate is literally
  `REG_LOOP_TEST_P (src_elt->exp)` at cse.c:7936, and REG_LOOP_TEST_P is set in exactly one
  place in the compiler: jump.c:2253, inside duplicate_loop_exit_test. duplicate_loop_exit_test
  only fires on a TOP-TEST loop (`for`/`while`), never on an `if`-guarded do/while. So the
  top-test chassis buys the tail-load CSE and pays 24 frame bytes; the do/while chassis buys
  the exact frame and pays the tail-load CSE. Measured both ways this session:
      top-test `for`  + named ctx : score 49, build_insns 194, vars 104 (frame 152)
      if-guarded do/while + ctx   : score 43, build_insns 190, vars  80 (frame 128)  <- adopted
- [s6] `s32 ctx = var_s0 * 3;` (a fresh once-written/once-read named intermediate for the
  D_800A3560 index) is worth 4 points and one instruction on BOTH chassis. It is what makes
  loop.c's strength_reduce reduce D_800A3560's giv to a byte OFFSET (`addiu $s2,$s2,3`, with
  `lui $at,%hi / addu $at,$at,$s2 / lbu %lo($at)` at 80070E28-E30 = the target) instead of a
  full ADDRESS (`lui $s2 / addiu $s2 / lbu 0($s2)`). This CLOSES the s5 frontier item (ii).
  A `u8 *p = D_800A3560 + var_s0 * 3;` pointer local measures identically (49 on the for
  chassis); inlining the index (`D_800A3560[var_s0 * 3]` or `*(D_800A3560 + var_s0 * 3)`)
  costs the 4 points back; a comma-initialised second biv (`for (i = 0, ctx = 0; ...; i++, ctx += 3)`)
  is catastrophic (88). NOTE FOR THE NEXT SESSION: `ctx` is a NAMED INTERMEDIATE and must be
  vetted against that family's 6 prongs before any candidate-ready.
- [s6] The `||` operand order in the mode test is NOT byte-neutral: writing
  `(D_800A35BC == 2) || (((s16)D_800A3558 + D_800A35B0) != 0)` scores 39 at 194 insns where
  the other order scores 40 at 190 insns. (The target evaluates the D_800A3558 term first in
  the emitted code, which is the ordering GCC produces from the swapped source.)
- [s6] D_800A3558's declared type is byte-neutral on BOTH chassis: `extern s32` + an `(s16)`
  cast, `extern u16`, and `extern s16` all give identical scores in all 9 bound x condition
  pairs on the do/while chassis and identical vars= on the for chassis. The ordinary
  `extern s32 D_800A3558;` is kept.

- [s6] Floor 53 -> 39 this session; build_insns is now 194, exactly the target's 194, and the frame is EXACT (.frame $sp,128,$31 # vars= 80, regs= 6/0, args= 24 -- every prologue and epilogue instruction matches, which was the single biggest scoring item at 14 instructions).

- [s6] NEW INSTRUMENT for this function and for the project: cc1's own `vars=` field in the .frame comment is a 1-second-per-variant gradient on the frame, independent of the sandbox score (tmp/grind/func_80070C70/s6/varsweep.sh; see the phantom-frame-slots-gcc272 memory). The BB2_FRAME_DEBUG=1 census in the instrumented cc1 at tools/gcc-2.7.2/cc1 (function.c:735) then names each slot -- stack_temp vs spill_new_pNNN -- which is what turned three sessions of frame guessing into one measurement.

- [s6] The frame overshoot and the tail-load CSE are COUPLED THROUGH ONE PASS. cse.c:7936 gates cse_set_around_loop on REG_LOOP_TEST_P, and jump.c:2253 (duplicate_loop_exit_test) is the only place in GCC 2.7.2 that sets it; the same routine's gen_reg_rtx at jump.c:2246 is what leaves the dead pseudos that become 24 bytes of spill slots. Top-test loop = tail-load CSE + 24 wasted frame bytes; if-guarded do/while = exact frame, no tail-load CSE. Measured: 49 vs 39.

- [s6] s5's structural conclusion is SUPERSEDED, not wrong: the top-test for chassis really does produce the target's tail lhu + next-iteration sll/sra pair, but it pays 14 prologue/epilogue instructions for 4 body instructions. The s5 frontier item that framed the overshoot as 'a source shape whose exit test has fewer replaceable intermediate registers' is now measured dead as a spelling axis (all 67 bound/condition/declaration spellings stay at vars= 104 or 96).

- [s6] `s32 ctx = var_s0 * 3;` closes the s4/s5 frontier item on D_800A3560's giv: naming the index makes strength_reduce reduce a byte OFFSET (addiu $s2,$s2,3, with the symbol re-added per reference) instead of a full ADDRESS. Worth 4 points and one instruction on both chassis. It is a fresh once-written/once-read NAMED INTERMEDIATE and needs that family's 6-prong vet before any candidate-ready; a `u8 *p = D_800A3560 + var_s0 * 3;` pointer local measures identically and is the alternative spelling to vet alongside it.

- [s6] IconC70's 0x20 size is now proven by direct measurement rather than by frame arithmetic: the BB2_FRAME_DEBUG census reports stack_temp 48 (prim) + stack_temp 32 (icon) = the target's vars= 80 with no residue. The `s16 sp50[12]` spelling of the trailing 24 bytes is still a placeholder for the real member list.

- [s6] The `||` operand order in the mode test is not byte-neutral: (D_800A35BC == 2) first is 39 at 194 insns, the D_800A3558 term first is 40 at 190.

- [s6] Everything measured this session is ordinary C except the one named intermediate: no dead store, no pad, no volatile, no asm, no duplicated arms, no record wrapper.

- [s7] The chassis was re-measured at dispatch and matched the ledger exactly: candidate.c + `extern u8 D_800A3560[];` + `extern s16 D_800A3590[];` + IconC70 sized 0x20 scores 39 at 194 insns with `sandbox --disable all`. No banked conclusion was stale on arrival.

- [s7] THE TARGET'S SECOND LOOP IS A TOP-TEST LOOP. Its guard block at 80070DF4-80070E18 (lhu $a2 / lw $a1 / lh $v0 / addiu $v0,$v0,1 / addu $v0,$a1,$v0 / blez) is an insn-for-insn copy of its loop tail test at 80070ECC-80070EE4 (lhu $a2 / lh $v0 / lw $a1 / addiu +1 / addu / slt / bnez) with the counter constant-folded, which is precisely what jump.c's duplicate_loop_exit_test emits. s6's structural conclusion that the exact 0x80 frame and the loop-carried $a1/$a2 registers are mutually exclusive is therefore FALSE OF THE TARGET; it was only true of our builds.

- [s7] The `lhu $a2` that appears in both the target's guard block and its loop tail is used by NEITHER test. It is the loop body's read of D_800A3558, consumed at 80070E78-80070E80 by `sll $v0,$a2,16 / sra / addu $v0,$a1,$v0`, and it is there because cse.c:7909 cse_set_around_loop moved the body's read into the exit-test register. That transform's only gate is REG_LOOP_TEST_P (cse.c:7936), set only at jump.c:2253 inside duplicate_loop_exit_test.

- [s7] Rewriting the s6 body's if-guarded do/while as a top-test `for` MAKES CSE_SET_AROUND_LOOP FIRE ON OUR BUILD. The emitted code carries the same structure as the target: guard `lw $5,D_800A3558 / lw $6,D_800A35B0 / lh $2,D_800A3558 / addu / addu 1 / blez`, a body with no reload of either symbol that consumes `sll $2,$5,16 / sra / addu $2,$2,$6`, and a tail `lw $6 / lh $2 / lw $5 / addu / addu 1 / slt / bnez`. Best variant of that family: 49 at 194 insns, banked as memory/grind/func_80070C70/chassis-toptest-cse-49.c.

- [s7] The whole remaining cost of the top-test chassis is the frame. Of its 49, fourteen diffs are the six callee-saved `sw`, the six `lw` and the two `addiu $sp` insns, all pure vars=104-vs-80 offset differences; the rest is about four register-seat diffs plus the two known scheduling ties. The do/while chassis at 39 has the frame exact but is missing the entire carried-register structure. Killing 24 frame bytes on the top-test chassis is therefore worth far more than any body spelling on the do/while chassis.

- [s7] The three spill slots on the top-test chassis are pseudos 116, 165 and 170, present in .combine as literal `(use (reg:SI 170))`, `(use (reg:SI 165))` and `(use (reg/s:SI 116))` insns. In .jump the copied guard is insns 478-485: 164=(mem:SI D_800A3558), 165=(ashift 164 16), 166=(ashiftrt 165 16), 167=(mem:SI D_800A35B0), 168=(plus 167 166), 169=(plus 168 1), 170=(lt reg75 169). combine folds 164/165/166 into one `lh` (orphaning 165) and folds the `lt` into `blez` (orphaning 170); 116 is a `reg/s` pointer pseudo from the pre-loop region.

- [s7] The declared type of D_800A3558 is byte-neutral on the cse-firing top-test chassis too - all four declarations (s32+cast, u16+cast, s16 bare, s16 with an (s16)(u16) double cast) tie at every (bound, condition) point, and the s16 BB2_FRAME_DEBUG census is the identical p116/p165/p170 triple at vars=104. That is the fourth distinct chassis on which this axis has measured dead.

- [s7] The winning `||` operand order in the mode test is CHASSIS-DEPENDENT. On the do/while chassis `(D_800A35BC == 2)` first is best (39); on the top-test chassis the target's own order (the D_800A3558 sum first) is best everywhere (49 vs 51-52) and is the half that reaches the target's 194 insns. Any body-spelling histogram must be re-run when the chassis moves.

- [s7] Parenthesising the loop bound as `D_800A35B0 + (<read> + 1)` to reproduce the target's `addiu $v0,$v0,1`-before-`addu` costs 5-8 points and an extra instruction on the top-test chassis (57/54 vs 51/49).

- [s7] Everything measured this session is ordinary C. The only non-ordinary construct anywhere in the ledger's live forms remains s6's `s32 ctx = var_s0 * 3;` named intermediate, which is unchanged and still un-vetted.

- [s7] Chassis re-measured at dispatch and matched the ledger exactly: candidate.c + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20 = 39 at 194 insns with sandbox --disable all. No banked conclusion was stale on arrival, and the session ends with src/text1b.c restored to that form and re-measured at 39.

- [s7] The target's guard block at 80070DF4-80070E18 and its loop tail test at 80070ECC-80070EE4 are the same six insns (lhu $a2 / lw $a1 / lh $v0 / addiu +1 / addu / branch), which is the signature of jump.c's duplicate_loop_exit_test. The target's second loop is therefore a TOP-TEST loop, and it has vars=80, so the exact frame and the loop-carried registers are NOT mutually exclusive.

- [s7] The lhu $a2 in both of those blocks is used by neither test; it is the loop body's read of D_800A3558, consumed at 80070E78-80070E80 by sll $v0,$a2,16 / sra / addu $v0,$a1,$v0. Only cse.c:7909 cse_set_around_loop places a loop-head read there, and its gate REG_LOOP_TEST_P (cse.c:7936) is set only at jump.c:2253.

- [s7] Rewriting the s6 body's if-guarded do/while as a top-test for makes cse_set_around_loop fire on our build: the emitted body carries no reload of D_800A3558 or D_800A35B0 and consumes sll $2,$5,16 / sra / addu $2,$2,$6, with both symbols loaded in the guard and re-loaded in the loop tail, exactly as the target does.

- [s7] Of the top-test chassis' 49 points, 14 diffs are the six callee-saved sw, the six lw and the two addiu $sp insns, i.e. pure vars=104-vs-80 offsets; the rest is about four register-seat diffs and the two known scheduling ties. The do/while 39 chassis has the frame exact and the entire carried-register structure missing.

- [s7] The three spill slots are pseudos 116, 165 and 170: .combine holds (insn 541 (use (reg:SI 170))), (insn 540 (use (reg:SI 165))) and (insn 542 (use (reg/s:SI 116))). The copied guard in .jump is insns 478-485 with 164=(mem:SI D_800A3558), 165=(ashift 164 16), 166=(ashiftrt 165 16), 167=(mem:SI D_800A35B0), 168=(plus 167 166), 169=(plus 168 1), 170=(lt reg75 169).

- [s7] The declared type of D_800A3558 is byte-neutral on this chassis too (four declarations tie at every bound x condition point, and the s16 frame census is the identical p116/p165/p170 triple at vars=104) - the fourth chassis on which that axis has measured dead.

- [s7] The winning || operand order is chassis-dependent: D_800A35BC-first wins on the do/while chassis (39), the target's sum-first order wins on the top-test chassis (49 vs 51-52) and is the half that reaches 194 insns.

- [s7] Everything measured this session is ordinary C; the only non-ordinary construct in any live form remains s6's `s32 ctx = var_s0 * 3;` named intermediate, unchanged and still un-vetted.

- [s8] [structural 2026-09-10] CHASSIS RE-MEASURED at dispatch: the if-guarded do/while candidate.c body is still 39 at 194 insns and the s7 top-test `for` body (s7/w/d32_b1_cy.c) is still 49 at 194 insns. Both baselines reproduce exactly, so every s6/s7 chassis-relative conclusion is still valid on HEAD.
- [s8] [structural 2026-09-10] PSEUDO 116 IDENTIFIED (s7 guessed "a reg/s pointer from the pre-loop region" and was wrong). tmp/grind/func_80070C70/dumps/text1b.jump insn 267 is `(set (reg/s:SI 116) (ashift:SI (reg/s:SI 114) (const_int 16)))` and insn 268 is `(set (reg/s:SI 115) (ashiftrt:SI (reg/s:SI 116) (const_int 16)))` carrying a REG_EQUAL sign_extend. 116 is therefore the sign-extension intermediate of `(s16)D_800A3558` inside the loop's OWN tail exit test, and 165 is the identical insn inside the guard copy that duplicate_loop_exit_test created. All three orphans (116, 165, 170) are the same phenomenon.
- [s8] [structural 2026-09-10] THE ORPHAN MECHANISM IS NAMED AND READ IN THE SOURCE. combine.c:10834-10840: when distribute_notes cannot find an insn after the combined insn that references a dying register and reaches a CODE_LABEL first, it emits `(use (reg))` after that label and parks the REG_DEAD note on it. reload1.c:2382 alter_reg then allocates a stack slot whenever `reg_renumber[i] < 0 && reg_n_refs[i] > 0 && reg_equiv_constant[i] == 0 && reg_equiv_memory_loc[i] == 0` - and a pseudo with references but no set is live from function entry in flow's view, so it conflicts with everything and global-alloc can never seat it. The 24 bytes are not an allocator preference; they follow mechanically once the `(use)` insns exist.
- [s8] [structural 2026-09-10] THE TARGET READS D_800A3558 TWICE PER TAIL, WITH TWO DIFFERENT WIDTHS: `lhu $a2,%gp_rel(D_800A3558)($gp)` at 80070ECC and `lh $v0,%gp_rel(D_800A3558)($gp)` at 80070ED0, from the same gp offset; the body then sign-extends the lhu value by hand (`sll $v0,$a2,16 / sra $v0,$v0,16 / addu $v0,$a1,$v0` at 80070E78-E80). The guard block at 80070DF4/80070E08 has the same lhu+lh pair. This is direct target evidence that the loop bound and the loop body read the object through DIFFERENT C spellings - the sweep space s7 covered (one spelling applied to both sites) never contained the target's form.
- [s8] [structural 2026-09-10] ...AND THE ASYMMETRIC SPELLING IS STILL BYTE-NEUTRAL. 12 asymmetric variants (declaration in {s16, u16} x bound in {bare, (s16), (s16)(u16)} x body in the other two) all score 49 at 194 insns on the top-test chassis, and the BB2_FRAME_DEBUG census on the s16 asymmetric form is the identical spill triple at vars=104. Combined with s2/K1, s4/K7, s6 and s7/K8 this axis is now dead across 5 chassis and 29 spellings; do not re-open it without a NEW mechanism.
- [s8] [structural 2026-09-10] THE EXIT-TEST COMPLEXITY TRADE IS MEASURED IN BOTH DIRECTIONS. Simplifying the exit test does remove the orphans - the fully hoisted bound falls to ONE orphan and vars 104 -> 88 - but the loop then needs a 7th callee-saved register (regs= 7/0 vs the target's 6/0) and scores 60 at 187 insns. `<=` (no +1) and `!=` are 79 at 192 insns: the whole cse_set_around_loop structure collapses with them. Four partial hoists are 54-62. There is no C-level midpoint on this chassis that keeps the cse structure and drops the frame.
- [s8] [structural 2026-09-10] SPELLING THE CARRIED VALUES AS C LOCALS DOES NOT REPRODUCE THE TARGET'S RELOADS. Six variants (locals read at the top of the body / also used as the bound / read before the loop and re-read at the bottom, on both chassis) score 42, 59, 42 (do/while) and 52, 60, 63 (top-test), with instruction counts falling to 191-192 on the do/while forms. A C local is exactly what loop.c and cse.c are free to hoist; the target's per-iteration reloads survive because they are a MEM the compiler chose to re-materialise, not a source-level variable.
- [s8] [structural 2026-09-10] F8 IS BACKWARDS, NOT MERELY UNPRODUCTIVE. Computing `prim.p_static = g + 0xC` before the `prim.p_geom = g` store - the source order the target's `lw $v0,4($s2) / addiu $v1,$v0,12 / sw $v0 / sw $v1` implies - costs a point on both chassis (39->40, 49->50), and doing it at both pre-loop sites costs two (41 / 51), with the instruction count unchanged at 194. The two-live-register form is not reachable by re-ordering these source statements.
- [s8] [structural 2026-09-10] NEW BEST TOP-TEST FORM: 48, not 49. Inlining the named local `t` in the second loop's body (writing `prim.p_static = prim.p_geom + 0xC;` twice) is worth a point on the top-test chassis and costs two on the do/while chassis (39 -> 41 at 195 insns). Banked as memory/grind/func_80070C70/chassis-toptest-cse-48.c. Its frame census is unchanged, so the point is a body seat, not the frame.
- [s8] [structural 2026-09-10] TOOLING: tmp/grind/func_80070C70/s8/sweep.py (score+insns, ~2s/variant) and tmp/grind/func_80070C70/s8/fdrun.py (BB2_FRAME_DEBUG census + the `.frame` line, run from PowerShell as `python tmp/grind/func_80070C70/s8/fdrun.py <variant.c> ...`) both work against a pristine snapshot at s8/text1b.pristine.c and restore src/text1b.c on exit. fdrun.py must be invoked from PowerShell, not the Bash tool - `wsl` is not on the Git-Bash PATH.

- [s8] Both chassis re-measured at dispatch and unchanged on HEAD: the if-guarded do/while candidate.c body is 39 at 194 insns, the s7 top-test `for` body is 49 at 194 insns. Every s6/s7 chassis-relative conclusion is therefore still valid.

- [s8] Pseudo 116 is the (s16) sign-extension ashift inside the loop's OWN tail exit test (text1b.jump insn 267), and 165 is the same insn in the guard copy - correcting s7's guess that 116 was a pre-loop pointer pseudo.

- [s8] The 24 extra frame bytes follow mechanically from combine.c:10834-10840 emitting `(use (reg))` at a CODE_LABEL plus reload1.c:2382 alter_reg's `reg_renumber[i] < 0 && reg_n_refs[i] > 0` slot rule; a refs-but-never-set pseudo is live from function entry in flow's view and can never be seated.

- [s8] The target reads D_800A3558 twice per loop tail from the same gp offset with two different widths - `lhu $a2` at 80070ECC and `lh $v0` at 80070ED0 - and sign-extends the lhu value by hand in the body at 80070E78. The loop bound and the loop body read the object through different C spellings.

- [s8] That asymmetric spelling is nevertheless byte-neutral for us: 12 declaration x per-site-cast combinations all score 49 at 194 insns with the identical spill triple at vars=104. The axis is dead across 5 chassis and 29 spellings.

- [s8] The exit-test complexity trade is measured in both directions and has no C-level midpoint on this chassis: the fully hoisted bound drops to one orphan and vars 104 -> 88 but buys a seventh callee-saved register (60 at 187 insns), while `<=`/`!=` collapse the cse_set_around_loop structure entirely (79 at 192 insns).

- [s8] Spelling the carried values as C locals does not reproduce the target's per-iteration reloads (42/59/42 do-while, 52/60/63 top-test, with instruction counts falling to 191-192): a C local is hoistable by loop.c and cse.c, whereas the target's reloads are a MEM the compiler chose to re-materialise.

- [s8] F8 is backwards, not merely unproductive: computing the p_static value before the p_geom store costs a point per site on both chassis with the instruction count unchanged at 194.

- [s8] New best top-test form is 48 (inline the named local `t`), banked as memory/grind/func_80070C70/chassis-toptest-cse-48.c; the same edit costs two points on the do/while chassis.

- [s8] Tooling for the next session: tmp/grind/func_80070C70/s8/sweep.py (score + build_insns, ~2s per variant) and tmp/grind/func_80070C70/s8/fdrun.py (BB2_FRAME_DEBUG census + the .frame line). fdrun.py must be run from PowerShell - `wsl` is not on the Git-Bash PATH.

## [s9 solver 2026-09-10] Chassis re-measured, solver layer typed, and the 16-bit-read axis closed by a compiler predicate

- [s9] CHASSIS CONFIRMED. `memory/grind/func_80070C70/candidate.c`'s body re-measured at exactly **39 / 194 insns** before any probe (tmp/grind/func_80070C70/s9/w/base39.c, sandbox --disable all). The ledger floor of 39 is live; nothing on HEAD moved between s8 and s9.
- [s9] The target object for object-level comparison is `build/src/text1b.o`: on main func_80070C70 is `INCLUDE_ASM("asm/funcs", func_80070C70)`, so the committed build carries the ORIGINAL 194 instructions in a normal ELF object. `mipsel-linux-gnu-objdump -d` of that object against `tmp/sandbox/func_80070C70/text1b.o` gives a same-dialect side-by-side diff that the raw `asm/funcs/*.s` text cannot (that file is disassembly with `/* offset addr bytes */` prefixes and a different mnemonic/immediate style). Recipe banked as tmp/grind/func_80070C70/s9/dis.sh.
- [s9] SOLVER VERDICT: **PRE-RA on every chassis tried**. `tools/ra_solver/inverse_compose.py classify` refuses this function by design (it is zero-rule, so the text-stream target is absent and it would print a fictitious PRE-RA verdict); the object-based `tools/ra_solver/goal_from_tgt.py classify text1b func_80070C70` is the correct entry point and prints `FIRST DIVERGENCE: PRE-RA / next tool: none - the residual is upstream of every model` for the floor-39 do/while body, for chassis-toptest-cse-48.c, and for both s16/u16 top-test bodies. tools/ra_solver's global/local/reload models and tools/sched_solver therefore have no purchase at this distance; the remaining work is C-level structure.
- [s9] THE TARGET'S SECOND-LOOP REGISTER STRUCTURE, read off the objects (guard block 80070DF4-80070E18, tail 80070ECC-80070EE4, body 80070E78): `$a2 = lhu %gp_rel(D_800A3558)`, `$a1 = lw %gp_rel(D_800A35B0)` are loaded in BOTH the guard and the tail; the exit test uses a THIRD, separate `lh %gp_rel(D_800A3558)` plus `addiu $v0,$v0,1` plus `addu $v0,$a1,$v0`; the loop body sign-extends the live `$a2` by hand with `sll $v0,$a2,0x10 / sra $v0,$v0,0x10 / addu $v0,$a1,$v0 / bnez`. So the SAME symbol is read with `lhu` and with `lh` at the same two program points - the `lhu` is the body's read that cse_set_around_loop hoisted out, the `lh` is the exit test's read that combine re-folded.
- [s9] **COMPILER PREDICATE (class-closing): `tools/gcc-2.7.2/config/mips/mips.md:2346`.** The `extendhisi2` define_expand begins `if (optimize && GET_CODE (operands[1]) == MEM) operands[1] = force_not_mem (operands[1]);` and then, because the operand is no longer a MEM, emits `ashlsi3` + `ashrsi3` by 16 and `DONE`s. Under -O the one-insn `extendhisi2_internal` memory pattern (which is what emits a bare `lh`) is UNREACHABLE from expand. Verified in the .rtl dump of a body that reads `extern s16 D_800A3558;` completely bare with no cast: insns 266/267/268 are `(reg:HI 114) <- (mem:HI (symbol_ref "D_800A3558"))`, `(reg:SI 116) <- (ashift (subreg:SI (reg:HI 114)) 16)`, `(reg:SI 115) <- (ashiftrt (reg:SI 116) 16)`. The same triple appears for the second D_800A3558 read (333-335) and for `D_800A3590[var_s0]` (319-321). CONSEQUENCE: the declared type and the per-site cast of a 16-bit read are invisible to the expander, which is why 30+ spellings across s2/s4/s6/s7/s8 and s9's own six-variant matrix all measure byte-neutral. Every `lh` in the final assembly - ours AND the target's - is combine re-folding the pair; the target's build paid the same fold.
- [s9] SCORE IS THE WRONG GRADIENT ON THE TOP-TEST CHASSIS. `chassis-toptest-cse-48.c` (score 48) still differs from the target by `lw x2` vs `lhu x2` on top of the frame. The bodies at score **49** whose D_800A3558 is `extern s16` read bare in the bound with an `(s16)` cast in the body - or `extern u16` with an `(s16)` cast at both sites - have NO load-shape difference at all: their entire object-level residual is the 13 prologue/epilogue insns that differ only by the 24-byte frame overshoot, plus one `addiu #,#,12` against one `nop`. Banked as `memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c`; it supersedes chassis-toptest-cse-48.c as the structural best even though it scores one point higher.
- [s9] THE ORPHANS ARE UNCHANGED BY THE DECLARATION AND BY THE BODY'S LABEL TOPOLOGY. BB2_FRAME_DEBUG censuses on six declaration variants and on three re-spellings of the body's `if ((code != 5) && (code != 16))` guard (early `continue`, two sequential `continue`s, two nested `if`s) all return the identical `spill_new_p116 / spill_new_p168 / spill_new_p173` at frame_offset 88/96/104 and `.frame $sp,152 # vars= 104, regs= 6/0, args= 24`. In the .jump dump p116 is the ashift of the loop's own tail exit test, p168 the ashift of the guard copy, p173 the `lt` of the guard copy; in .combine all three appear as literal `(use (reg))` insns. The CODE_LABEL that combine.c:10834 reaches is therefore NOT the body `if`'s join label.

- [s9] CHASSIS: memory/grind/func_80070C70/candidate.c's body re-measures at exactly 39 / 194 insns on current HEAD (sandbox --disable all), so the ledger floor of 39 is live and unchanged.

- [s9] The correct target object for object-level comparison is build/src/text1b.o: func_80070C70 is INCLUDE_ASM on main, so the committed build carries the original 194 instructions in an ordinary ELF object that objdumps in the same dialect as our sandbox object. Recipe banked at tmp/grind/func_80070C70/s9/dis.sh; the raw asm/funcs/*.s text is NOT usable for this (different mnemonic and immediate style).

- [s9] tools/ra_solver/inverse_compose.py classify refuses zero-rule functions by design and points at tools/ra_solver/goal_from_tgt.py classify, which is the correct solver entry point for func_80070C70.

- [s9] TARGET SECOND-LOOP STRUCTURE, read off the objects: $a2 = lhu %gp_rel(D_800A3558) and $a1 = lw %gp_rel(D_800A35B0) are loaded in BOTH the guard block (80070DF4/80070DFC) and the loop tail (80070ECC/80070ED4); the exit test uses a separate third read, lh %gp_rel(D_800A3558) at 80070E08 and 80070ED0, plus addiu $v0,$v0,1 and addu $v0,$a1,$v0; the body sign-extends the live $a2 by hand at 80070E78 with sll/sra/addu/bnez. The same symbol is read with lhu AND with lh at the same two program points.

- [s9] COMPILER PREDICATE tools/gcc-2.7.2/config/mips/mips.md:2346 - extendhisi2's expander calls force_not_mem on a MEM operand whenever optimize is on, then emits ashlsi3+ashrsi3 by 16 and DONEs, so the one-insn extendhisi2_internal memory pattern (the bare `lh`) is unreachable from expand. Confirmed in the .rtl dump on a fully bare `extern s16 D_800A3558;` read: insns 266/267/268 are the mem:HI load plus the ashift/ashiftrt pair, and 319-321 and 333-335 are the same triple for D_800A3590[var_s0] and for the second D_800A3558 read.

- [s9] CONSEQUENCE OF THAT PREDICATE: the target's own build also went through force_not_mem, the shift pair, and combine's re-fold to `lh`. So the target had the same three combine fold candidates we do and still produced vars=80 - the 24-byte overshoot is NOT an inevitable consequence of the fold, and s8's F10 framing (remove the sign-extension from the exit test) is answered: it cannot be removed from any 16-bit read.

- [s9] SCORE VS SHAPE: chassis-toptest-cse-48.c scores 48 but differs from the target by `lw x2` vs `lhu x2` on top of the frame; the score-49 bodies (extern s16 bare in the bound + (s16) cast in the body, or extern u16 with an (s16) cast at both sites) have zero load-shape difference and a residual of exactly 13 frame insns plus one `addiu #,#,12` against one `nop`. Banked as memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c.

- [s9] ORPHAN IDENTITY under every spelling tried: .jump shows p116 = the ashift of the loop's own tail exit test, p168 = the ashift of the guard copy, p173 = the `lt` of the guard copy; .combine shows all three as literal `(use (reg))` insns; BB2_FRAME_DEBUG shows spill_new_p116/p168/p173 at frame_offset 88/96/104 giving vars=104 against the target's 80. Six declaration variants and three body-guard re-spellings leave that triple bit-identical.

- [s9] combine.c:10830-10846 is the emission site: the `(use (reg))` is created only when the scan reaches a CODE_LABEL with place still 0 (`if (REG_NOTE_KIND (note) == REG_DEAD && place == 0 && tem != 0)`); the alternative at combine.c:10826 is finding an insn that references the register. s9 proved the label reached is not the body `if`'s join label.

- [s10 forensics 2026-09-10] LEDGER HYGIENE BUG (fixed this session; it cost s10 two turns and would
  have cost every future session the same). `memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c`
  as banked by s9 carried its 18-line header comment ABOVE its `//@sub` directive. The installer
  (`tmp/grind/func_80070C70/s9/install.py`, copied forward as s10/install.py) consumes `//@sub` lines
  only with `while b.startswith('//@sub ')`, i.e. only at the very start of the file, and then slices
  the body from `b.index(SIG)` - so the directive was silently DROPPED, the body installed against the
  pristine `extern s32 D_800A3558;`, and the banked "score 49" file actually measures 56. The file has
  been rewritten with the `//@sub` on line 1 and re-measured at 49. RULE for every future banked
  candidate: `//@sub` directives must be the FIRST lines of the file, above any comment.

- [s10 forensics 2026-09-10] THE FULL MECHANISM OF THE 24 FRAME BYTES, END TO END, EVERY STEP READ OUT
  OF A DUMP OR A COMPILER SOURCE LINE. s8/s9 had named combine.c:10834 as the emission site; s10 closes
  the chain from there to the frame bytes and names the enabling predicate.
  1. `combine.c:2089` computes `elim_i2 = (newi2pat || i2dest_in_i2src || i2dest_in_i1src ? 0 : i2dest)`.
     When try_combine SPLITS a 3-insn combination back into two insns it sets `newi2pat`, and elim_i2
     becomes 0 - so i2's dest is NOT on the eliminate list.
  2. `combine.c:10741` (`if (XEXP (note, 0) == elim_i2 || ...) break;`) is therefore not taken for that
     REG_DEAD note, and the note falls into the backward scan at combine.c:10756-10758, which runs
     `tem = prev_nonnote_insn (tem)` while `GET_CODE (tem) == INSN || GET_CODE (tem) == CALL_INSN`. The
     insn that SET the dying register has already been turned into a NOTE by the combination, so
     `prev_nonnote_insn` skips straight past it and nothing else references the register.
  3. `combine.c:10830-10836`: `place == 0 && tem != 0` -> `emit_insn_after (gen_rtx (USE, VOIDmode, reg), tem)`.
  4. THE NEW STEP (s10): a pseudo whose ONLY reference is that `(use (reg))` gets no register-class
     preference from any constraint, and regclass hands it ST_REGS. Read verbatim out of
     `tmp/grind/func_80070C70/dumps/text1b.lreg` (s10 func-section extract w/sec.lreg, lines 69/121/129):
        Register 116 used 4 times across 1 insns in block 11; ST_REGS or none.
        Register 168 used 2 times across 2 insns in block 2;  ST_REGS or none.
        Register 173 used 2 times across 1 insns in block 2;  ST_REGS or none.
     ST_REGS is the MIPS fp-status class (`tools/gcc-2.7.2/config/mips/mips.h:1386`), disjoint from
     GR_REGS, so global_alloc can never satisfy it and `reg_renumber[i]` stays < 0.
  5. `reload1.c:2382-2404` `alter_reg`: `reg_renumber[i] < 0 && reg_n_refs[i] > 0 && no equiv` with
     `from_reg == -1` ("No known place to spill from => no slot to reuse") -> a brand-new
     `assign_stack_local`. The instrumentation at reload1.c:2403 prints it as `spill_new_p<N>`. Each one
     costs 8 bytes (mode=4/SImode, alignment 8).
  CONSEQUENCE: an orphaned `(use (reg))` is ALWAYS 8 frame bytes and can never be "allocated away" by
  any RA-side lever. The only lever is to stop combine from emitting it.

- [s10 forensics 2026-09-10] THE `tem` IDENTITY - s9's open frontier question F12 is ANSWERED. Read
  directly out of `tmp/grind/func_80070C70/s10/w/sec.combine` (func-section extract of the .combine dump
  for the top-test chassis):
  * `(insn 545 405 500 (use (reg/s:SI 116)))` sits immediately after `(code_label 405 404 545 924 "")`,
    which `.jump` shows is preceded by `(note 404 402 405 "" NOTE_INSN_LOOP_CONT)` - i.e. `tem` is the
    SECOND LOOP'S OWN CONTINUE LABEL, and p116 is the ashift of the loop's tail exit test (`.jump` insns
    266/267/268: `reg:HI 114 <- mem:HI D_800A3558`, `reg 116 <- ashift(subreg 114,16)`,
    `reg 115 <- ashiftrt(reg 116,16)`).
  * `(insn 544 203 543 (use (reg:SI 173)))` and `(insn 543 544 209 (use (reg:SI 168)))` sit immediately
    after `(jump_insn 203 ...)`, the FIRST loop's bottom `bnez` back-edge - NOT a CODE_LABEL at all. The
    comment at combine.c:10828 says "we have hit a CODE_LABEL", but the scan loop only tests
    `INSN || CALL_INSN`, so a JUMP_INSN terminates it too and is a legal `tem`. p168/p173 belong to the
    SECOND loop's duplicate_loop_exit_test GUARD COPY (`.jump` insns 481-488), which sits far downstream;
    the scan walked back over ~15 insns and two CALL_INSNs before stopping at insn 203.
  * The guard copy's post-combine form (sec.combine lines 626-661) is `lhu; lh; lw; addu; addiu; li 0; blez`
    - the `slt` (insn 487, setting p173) was folded into the branch as `le (reg 172) (const_int 0)` while
    `(set (reg/v:SI 75) (const_int 0))` was RE-EMITTED as the newi2pat at i2's slot. That re-emitted
    `var_s0 = 0` sitting between the `addiu` and the `blez` is the visible fingerprint of the newi2pat
    split that orphans p173, and it is present in the target's assembly too.

- [s10 forensics 2026-09-10] THE 24-BYTE OVERSHOOT IS A PROPERTY OF THE TOP-TEST CHASSIS ALONE, AND THE
  do/while CHASSIS ALREADY HAS THE TARGET'S EXACT FRAME. 26-body frame census
  (`tmp/grind/func_80070C70/s10/frame_census.txt`, harness `s10/probe.py` + `s10/probe.sh`: one
  instrumented-cc1 run per body with BB2_FRAME_DEBUG=1, no assembler and no sandbox, ~8s each - this is
  the cheap gradient for any frame question on this function):
  * ALL SIX do/while declaration variants (s9/d/dw_*.c) and the floor-39 candidate.c measure 0 spills,
    `.frame $sp,128 # vars= 80, regs= 6/0, args= 24` - byte-identical to the target's frame.
  * ALL FIVE sign-extending top-test variants measure 3 orphans / vars=104.
  This DEMOTES s9's frontier claim that "the top-test chassis is now the shorter road". On the frame axis
  the do/while chassis is already AT the target and the top-test chassis has to buy back 24 bytes it
  spends; the two chassis are each missing a different thing, and the do/while's deficit (the
  cse_set_around_loop hoist) is the one that has never been attacked with a dump in hand.

- [s10 forensics 2026-09-10] LOOP TOPOLOGY IS INERT ON THE ORPHAN COUNT; THE BOUND'S SIGN-EXTENSION IS
  THE ONLY INPUT THAT MOVES IT. Nine top-test spellings measured (frame_census.txt): fresh loop variable,
  init hoisted out of the for-header, increment moved to the bottom of the body with an empty third
  clause, while-form, inverted body guard with `continue`, bound hoisted into a local, two operand
  reorderings of the bound expression, and `!(i >= bound)`. Eight of the nine leave p116/p168/p173
  bit-identical at vars=104. Only two inputs move the count, and both are about the 16-bit READ, not the
  loop's shape:
  * `extern u16 D_800A3558` read BARE in the bound with an `(s16)` cast in the body (s9/d/tt_u16_b_c.c):
    1 orphan (p171), vars=88 - the bound becomes a zero-extend, there is no ashift/ashiftrt pair to fold,
    and two of the three orphans never exist. Sandbox score 54 (the bound's `lhu` costs more than the 16
    frame bytes it saves). Banked rejected/toptest-u16-bare-bound-1-orphan-54.c.
  * `var_s0 != bound` instead of `<`: 2 orphans (p116,p167), vars=96 - no `slt` to fold into the branch,
    so p173 never exists. Banked rejected/toptest-ne-bound-2-orphans.c.
  * bound hoisted into a local: 1 orphan but `regs= 7/0` - it buys the frame back and immediately spends a
    SEVENTH callee-saved register. Banked rejected/toptest-bound-in-local-7th-callee-saved.c.
  So s9's F12 next-probe ("attack THAT label from C: what statement ends the loop body, whether the
  increment is written in the for-header or at the bottom, and what immediately follows the loop") is
  measured dead as an instance on this chassis: `tem` is not reachable from loop topology, because the
  orphan is created by the newi2pat split of the 16-bit compare wherever that compare happens to sit.

- [s10 forensics 2026-09-10] THE TOP-TEST RESIDUAL IS NOT ONLY THE FRAME. s9 recorded the score-49 body's
  object-level residual as "13 prologue/epilogue insns plus one addiu against one nop". The s10 objdump
  diff (`tmp/grind/func_80070C70/s9/dis.sh` regenerated; ours.txt vs tgt.txt) shows in addition:
  (a) a genuine $a1/$a2 SEAT SWAP - the target holds the hoisted `lhu` of D_800A3558 in $a2 and the `lw`
  of D_800A35B0 in $a1 (`lhu a2,0(gp)` / `lw a1,0(gp)` / `sll v0,a2,0x10` / `addu v0,a1,v0`), while we
  hold them the other way round at BOTH the guard and the tail; (b) several $v0-vs-$v1 destination
  differences on the `addiu #,#,12` / `sw #,28(sp)` pairs; (c) delay-slot and load-ordering differences
  (the target keeps a `nop` after `lw v1,0(gp)` where we fill that slot with `sw v1,28(sp)`). Any future
  session quoting "the top-test chassis is 2 shapes from the target" should quote this instead.

- [s10] Chassis re-measured at dispatch: memory/grind/func_80070C70/candidate.c = score 39 / 194 build insns / 194 target insns under `sandbox func_80070C70 --disable all`. The ledger floor of 39 is live on current HEAD; tmp/grind/func_80070C70/s8/text1b.pristine.c is byte-identical to HEAD's src/text1b.c.

- [s10] The 24-byte frame overshoot decomposes exactly: BB2_FRAME_DEBUG prints stack_temp size=48 (prim) at frame_offset 48, stack_temp size=32 (icon) at 80, then spill_new_p116 at 88, spill_new_p168 at 96, spill_new_p173 at 104, then round_frame at 104. The target's vars=80 is precisely prim(48) + icon(32) with zero spills.

- [s10] tools/gcc-2.7.2/combine.c:2089 is the enabling predicate for the whole orphan family: elim_i2 is zeroed whenever try_combine produces a newi2pat, which is what leaves i2dest's REG_DEAD note to be placed by the backward scan instead of being dropped at combine.c:10741.

- [s10] tools/gcc-2.7.2/config/mips/mips.h:1386 defines ST_REGS as the fp-status class. A USE-only pseudo is assigned that class by regclass, which is why reg_renumber stays negative and reload1.c:2403 always allocates a fresh slot. This is the step s8 and s9 were missing.

- [s10] The .combine dump shows the post-combine guard copy of the second loop as lhu / lh / lw / addu / addiu / li 0 / blez, with the `li 0` re-emitted between the addiu and the blez as the newi2pat - so the guard's `var_s0 = 0` sitting in that position is a compiler artifact of the split, not a source-order effect.

- [s10] 26-body frame census (tmp/grind/func_80070C70/s10/frame_census.txt): 0 spills / vars=80 for candidate.c and all six do/while declaration variants; 3 spills / vars=104 for all five sign-extending top-test variants and for eight of the twelve top-test topology spellings; 2 spills / vars=96 for the `!=` bound; 1 spill / vars=88 for a bare u16 bound (score 54) and for the bound hoisted into a local (but at regs=7/0).

- [s10] New cheap gradient banked for this function: tmp/grind/func_80070C70/s10/probe.py + probe.sh runs only cpp|cc1 with the instrumented compiler and prints `spills=<n> <pseudos> <.frame line>` per body in about 8 seconds, against roughly 40 seconds for a sandbox run. Frame and orphan questions should be answered with it before anything is scored. tmp/grind/func_80070C70/s10/scan.py <pass> extracts the func_80070C70 section from any -da dump into s10/w/sec.<pass>.

- [s10] Ledger-hygiene rule established: a banked candidate's //@sub directives must be the first lines of the file. The s9 top-test chassis violated this and measured 56 instead of 49 for every session that would have installed it; the file has been fixed.

## [s11 rederive 2026-09-10] FLOOR 39 -> 22 on the UNCHANGED if-guarded do/while chassis

s11's mandate was a structurally different C shape. Two genuinely different shapes were
built and both are DEAD (banked in rejected/), but the object-level diff produced while
re-deriving them exposed four ordinary-C statement-level spellings that were worth 17
points together. All numbers are `sandbox func_80070C70 --disable all`, 194/194 insns,
frame `$sp,128 # vars= 80, regs= 6/0, args= 24` with 0 spill slots throughout.

- BASELINE RE-MEASURED FIRST (chassis check): memory/grind/func_80070C70/candidate.c as
  banked by s10 scores 39 on today's HEAD. The brief's "measurement unavailable" is
  resolved: the chassis had NOT moved.
- L1 TAIL-BOUND PARENTHESISATION, 39 -> 38. `var_s0 < D_800A35B0 + ((s16)D_800A3558 + 1)`.
  The target's tail is `lh $v0; addiu $v0,$v0,1; addu $v0,$a1,$v0` - the 1 is added to the
  sign-extended halfword, not to the sum. Guard copy must stay unparenthesised (both = 41
  at 196 insns; guard-only = 41).
- L2 NAMED geom/static TEMP PAIR AT THE TWO PRE-LOOP SITES, worth 7 points.
  `g = *(s32*)(ctx+N); t = g + K; prim.p_geom = g; prim.p_static = t;` reproduces the
  target's `addiu $v1,$v0,12 / sw $v0,0x18($sp) / sw $v1,0x1C($sp)` two-live-register
  form. Writing the add as `prim.p_static = prim.p_geom + 0xC;` (reading the member back)
  lets the scheduler put the add AFTER the store, the pseudo dies at the add, and it
  reuses the same hard register. Sequencing the add as its own statement before both
  stores is what keeps two pseudos live across it.
- L3 `prim.link` READ BEFORE `prim.code` STORE at both loop call sites, worth 7 points
  (36 -> 31 second loop, 31 -> 29 first loop). The target emits `lw $v1,0x10($s1)` before
  `sw $v0,0x2C($sp)` at both.
- L4 `g = prim.p_geom; t = g + 0xC;` inside the second loop body, 38 -> 36. Same
  two-live-register effect at the in-loop site.

### s11 negative results (structural rederivations that are DEAD on this chassis)
- WALKING POINTER over D_800A3560 (`u8 *p = D_800A3560; code = *p; ... p += 3;`) = 91 at
  195 insns; with a mode accumulator = 93; with a second walking pointer over D_800A3590
  = 93. A pointer that is incremented is a real ADDRESS biv, so the %hi(D_800A3560) base
  must be materialised in a register before the loop; the target re-does
  `lui $at,%hi / addu $at,$at,$s2 / lbu %lo($at)` every iteration, i.e. it uses an OFFSET
  giv. This is the same effect s4 recorded from the other direction.
- EXPLICIT MODE ACCUMULATOR (`m = 0x50; ... prim.mode = m; ... m += 0x16C;`) is BYTE-
  NEUTRAL against `prim.mode = 0x50 + var_s0 * 0x16C` (39/39, 194/194 insns). loop.c's
  strength reduction already produces the target's `addiu $s3,$s3,0x16C`.
- CARRYING THE TWO LOOP GLOBALS IN LOCALS re-assigned at the bottom of the body - the
  literal C spelling of the target's loop-carried $a1/$a2 - is 55 (s32 locals) to 59
  (u16/s16 locals with the sign-extend in the body) at 197 insns. Confirms s8's kill on
  the new, much lower chassis.
- `<=` BOUND without the explicit +1 = 43 at 192 insns; `0 <= ...` guard = 39; `>= 0`
  guard = 39; guard `!(... <= 0)` = 38 (ties, byte-identical to L1).
- DUPLICATE READ of `*(s32*)(ctx+N)` at the two pre-loop sites instead of a local = 44.
- `prim.p_static` STORED BEFORE `prim.p_geom` = 40 (s8's result reproduced).
- `||` OPERANDS IN THE TARGET'S ORDER (sum first) = 30 at 190 insns. The target really
  does test the sum first; spelling it that way loses four insns elsewhere. Do not "fix"
  this by inspection - it is measured worse at every chassis tried (39->40 at s6,
  29->34 at s11-r1, 22->30 at s11-u1).

### s11 tooling
`tmp/grind/func_80070C70/s11/score.py` + `score.sh` - installs a body via the s9 installer
and calls `engine.sandbox.sandbox_score(..., disable="all", strip_cheat_asm=True)`
in-process, one line of `score= insns=/` per body, no PowerShell round-trip. Invoke as
`bash tools/wsl.sh 'bash tmp/grind/func_80070C70/s11/score.sh <body.c> ...'` from the repo
root (the Bash tool here is Git Bash, not WSL - `bash tools/wsl.sh` is the bridge).

- [s11] CHASSIS CHECK: the brief reported 'measurement unavailable'. Re-measured first thing this session - the s10 candidate body scores exactly 39 at 194/194 insns on today's HEAD, so the chassis had not moved and every banked s10 conclusion was spendable.

- [s11] NEW FLOOR 22 at 194/194 insns, frame still exact: '.frame $sp,128 # vars= 80, regs= 6/0, args= 24' with 0 spill slots (tmp/grind/func_80070C70/s10/probe.sh on the banked candidate). The chassis is UNCHANGED - still the if-guarded do/while; all four levers are statement-level spellings inside it.

- [s11] The 39 -> 22 path is fully reproducible from the banked probe bodies: base 39 -> w7_paren_tailonly 38 -> z3_g_both_body 36 -> q3_linkfirst 31 -> r1_loop1_linkfirst 29 -> u1_t_both 22.

- [s11] KILL RE-AUDIT PAID OFF, AND IT IS THE LESSON OF THIS SESSION: s7's kill of the parenthesised bound and s8's kill of the geom/static temp pair were both real, but the first was measured on the TOP-TEST chassis and the second in a reversed store order. Re-testing them on the current chassis in the target's own store order turned two 'dead' axes into 8 of this session's 17 points. Instance kills on this function have repeatedly been chassis- or spelling-relative.

- [s11] THE RESIDUAL AT 22 IS NOW ESSENTIALLY ONE CLUSTER: about 18 of the 22 points are the loop-carried $a1/$a2 ('lhu $a2,%gp_rel(D_800A3558)' and 'lw $a1,%gp_rel(D_800A35B0)' loaded in the second loop's guard block and again in its tail, consumed by the body as sll/sra/addu instead of being re-loaded). This is the unchanged cse_set_around_loop frontier from s7-s10. The remaining ~4 are the prologue scheduler tie and the || operand order, both killed above.

- [s11] The direct C spelling of that cluster is now measured dead on the LOW chassis too (55-59 at 197 insns), so the hoist has to come from cse, not from the source carrying the values.

- [s11] s11 tooling for reuse: tmp/grind/func_80070C70/s11/score.py + score.sh installs a body via the s9 installer and calls engine.sandbox.sandbox_score(disable='all', strip_cheat_asm=True) in-process, printing one 'score= insns=/' line per body with no PowerShell round-trip. Note the Bash tool in this harness is Git Bash, not WSL - invoke as: bash tools/wsl.sh 'bash tmp/grind/func_80070C70/s11/score.sh <body.c> ...'.

- [s11] src/text1b.c was restored to pristine (INCLUDE_ASM) at the end of the session; only memory/grind/func_80070C70/ and tmp/ carry changes.

- [s12] [rederive 2026-09-10] CHASSIS QUESTION CLOSED WITH A NUMBER. The shape-exact top-test for chassis, given s11's three body levers, scores 31/194 (49 -> 42 with L3 -> 36 with L2 -> 31 with L4). The if-guarded do/while with the same levers is 22/194. The top-test chassis' 24-byte orphan-spill frame penalty (H-s10-1) is therefore still not affordable, and this is now a measured comparison of two bodies carrying the SAME levers rather than a strategic argument. Bodies tmp/grind/func_80070C70/s12/a/.
- [s12] [rederive 2026-09-10] All three of s10's frame-buying levers re-measured on the improved (31) top-test chassis are still net-negative: bound-in-a-local 51/187, bare-u16 bound 34/194, != bound 62/192, both together 51/187.
- [s12] [rederive 2026-09-10] The carried-locals family, re-measured on the 22 chassis: six two-local spellings are byte-identical at 42/195; D_800A35B0-only is 27/194 (four sub-spellings all 27); D_800A3558-only is 40/196; no bottom re-assignment is 40/197. The gap to plain re-reads has closed from 16-20 points (s11, on the 39 chassis) to 5 points, so the family is chassis-sensitive rather than flatly wrong.
- [s12] [rederive 2026-09-10] s32 var_s3 = 0xA; is REMOVABLE at zero cost (literal prim.code = 0xA; is 22/194, identical) and s12's candidate.c drops it. s32 c60 = 0x60; is NOT removable: the literal spelling is 29/191, i.e. the constant-holder is worth three instructions. One constant-holder construct remains to be vetted at submission instead of two.
- [s12] [rederive 2026-09-10] ICON RECORD SIZE DISPROVEN. func_80069898 (asm/funcs/func_80069898.s) dereferences its $a1 argument at exactly four offsets - lhu +0x0/+0x2/+0x4/+0x6 - feeding a TILE prim three times; there is no other $s1-relative access. Sibling caller func_8006B120 has frame 0x68 with $s0 at 0x50 and args 0x0..0x17, i.e. vars = 56 = prim(48 at 0x18) + icon(8 at 0x48). The icon record is EIGHT bytes, not 0x20, so the 24 bytes at sp+0x50..0x67 in func_80070C70 are a separate local - and a REFERENCED one, because an unreferenced s32 sp50[6] is dropped outright (frame 128 -> 104, score 36). func_800720FC shows the same idiom with 32 spare bytes (frame 0x98, $s0 at 0x70, icon at 0x48). This is also inheritance for the active queue sibling func_8006F97C, which calls the same func_80069898.
- [s12] [rederive 2026-09-10] cse.c:7909 cse_set_around_loop reads in full: for a SET inside the loop whose SET_SRC is in the hash table with an equivalent REG carrying REG_LOOP_TEST_P and a lower COST, it scans backwards from loop_start for an insn setting a same-mode reg to the same SET_SRC and emits a copy after it (cse.c:7970 emit_insn_after (gen_move_insn (src_elt->exp, SET_DEST (set)), p)). The target's lhu $a2 and lw $a1 in the second loop's guard block are exactly those emitted copies. REG_LOOP_TEST_P has exactly one setter outside integrate.c - jump.c:2253 inside duplicate_loop_exit_test - which is why the do/while chassis cannot reach this transform by any body spelling.

- [s12] Floor is 22 at 194/194 instructions with the target's exact frame; the s12 candidate.c reaches it with ONE FEWER construct than s11's (var_s3 retired, measured byte-neutral).

- [s12] The top-test-vs-do/while chassis argument that s7 through s11 kept reopening now has a number on both sides carrying the SAME body levers: top-test 31, do/while 22. The top-test chassis genuinely has the cse_set_around_loop structure (objdump-confirmed: lhu a2 / lw a1 in the guard and tail, sll/sra/addu in the body) and its entire residual is the 152-vs-128 frame plus an a0/a1/a2 seat permutation - it just costs 9 more than not having the structure at all.

- [s12] cse.c:7909 cse_set_around_loop, read in full: for a SET inside the loop whose SET_SRC is in the hash table with an equivalent REG carrying REG_LOOP_TEST_P and a lower COST, it scans backwards from loop_start for an insn setting a same-mode reg to the same SET_SRC and emits a copy after it at cse.c:7970. The target's lhu $a2 and lw $a1 in the guard block ARE those emitted copies. REG_LOOP_TEST_P has exactly one setter outside integrate.c - jump.c:2253, inside duplicate_loop_exit_test - which is why no do/while body spelling can reach the transform.

- [s12] IconC70 is 8 bytes, not 0x20: func_80069898 reads only +0x0/+0x2/+0x4/+0x6 of its $a1 argument, and sibling caller func_8006B120 allocates exactly 8 bytes for it (frame 0x68, $s0 at 0x50, args 0x18, prim 48 at 0x18, icon 8 at 0x48). The 24 spare bytes in func_80070C70 are a separate REFERENCED local: an unreferenced s32 sp50[6] is dropped and the frame falls 128 -> 104.

- [s12] GCC 2.7.2 here DROPS a wholly unreferenced local aggregate rather than reserving frame bytes for it - so the phantom-frame-slot pattern in the Claude memory project/phantom-frame-slots-gcc272 does not extend to unreferenced arrays, and any C form that reproduces the target's 24 extra bytes must reference the local.

- [s12] The carried-locals family is chassis-sensitive, not flatly wrong: 55-59 against a floor of 39 in s11, 27-42 against a floor of 22 in s12. Its best member (D_800A35B0 alone in a local) is only 5 points behind.

- [s12] s12 left src/text1b.c restored to pristine INCLUDE_ASM (tmp/grind/func_80070C70/s10/install.py --restore); no tracked build file is modified by this session.

- [s13] The object at sp+0x48 in func_80070C70 is a `u16 rect[]` ARRAY, not the invented `IconC70` struct: func_80069898 is already decompiled at src/text1b.c:5413 with prototype `void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2)` reading arg1[0..3], and both callers already on main (func_8006BB68 at src/text1b.c:5822, func_8006DD94 at src/text1b.c:6065) pass a local `u16 rect[4]` written in the order rect[2], rect[0], rect[1], rect[3] - the exact store order and offsets func_80070C70's target asm uses at sp+0x4C/0x48/0x4A/0x4E.
- [s13] The 24 bytes at sp+0x50..0x67 that s12 called a still-unidentified local are the UNWRITTEN TAIL of that array: `u16 rect[16]` spans 0x48..0x67 and measures 22/194 BYTE-IDENTICAL to the s12 IconC70+`s16 sp50[12]` body; `u16 rect[4]` (written size only) measures 36/194, so the tail is load-bearing. The declared length is a range - rect[15] and rect[16] are byte-identical because stmt.c:3419 8-aligns a BLKmode automatic - exactly the range caveat recorded for func_8006DD94 (src/text1b.c:6032).
- [s13] This makes func_80070C70's oversized local the same accepted OVERSIZED-LOCALS shape as the two in-tree exemplars (src/text1b.c:6032 func_8006DD94; src/text1a_post.c:404 func_80041BF4 `s16 rect[8]`) - extending the LIVE address-taken object rather than adding a dead pad - and removes the last invented type from the candidate.
- [s13] The scalar declaration-order axis is completely inert on the 22-point if-guarded do/while chassis: all 24 permutations of `s32 var_s0; s32 t; u8 code; s32 g;` are byte-identical at 22/194.
- [s13] The first loop body's statement-order axis is exhaustively swept: all 6 orders x 2 increment orders; the candidate's order is the unique optimum (22 vs 24/24/29/30/30) and the two trailing increments are byte-neutral.
- [s13] The pre-guard position of `prim.p_geom = *(s32 *)(ctx + 8)` has exactly three legal slots and the candidate holds the best (22 vs 25 and 26).
- [s13] $a1/$a2 are caller-saved and the second loop body contains a `jal`, so the target's loop-carried $a1/$a2 are RELOADED in the tail every iteration (28ffc `lhu a2,0(gp)` / 29000 `lh v0,0(gp)` / 29004 `lw a1,0(gp)`); the guard block only supplies the entry copies. The target's tail therefore carries three gp loads where our re-read spelling carries two.
- [s13] The target's mode test branches on the SUM first (28fb4 `bnez` on `$a1 + (s16)$a2`, then 28fc4 `bne` against 2), i.e. the source is `if (sum != 0 || D_800A35BC == 2)`; our 22-point incumbent emits the D_800A35BC-first order. The sum-first spelling is structurally correct and still scores worse (30, matching s11's banked kill) because it ripples $v1 -> $a1 onto the pre-loop `addiu #,#,72` / `sw #,28(sp)` pair.

- [s13] func_80069898 is ALREADY DECOMPILED in the same TU (src/text1b.c:5413) with the prototype void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2) and reads arg1[0], arg1[1], arg1[2], arg1[3] - its second parameter is a u16 POINTER, so the caller's object is an array, and the extern s32 func_80069898(s32 a0, s32 *p, s32 mode); re-declaration the grind body carried was shadowing the real prototype with a wrong pointer type.

- [s13] Both on-main callers of func_80069898 declare a local u16 rect[4] and write it in the order rect[2], rect[0], rect[1], rect[3] (func_8006BB68 src/text1b.c:5822 writes 0xAF, 0xE8, 0x25, 1; func_8006DD94 src/text1b.c:6065). func_80070C70's target asm stores 0xE7, 0xCC, 0x25, 1 to sp+0x4C, sp+0x48, sp+0x4A, sp+0x4E - the same idiom, the same order.

- [s13] u16 rect[16]; (spanning sp+0x48..0x67) measures 22/194 and is BYTE-IDENTICAL to the s12 IconC70 + s16 sp50[12] body over all 194 instructions; s16 rect[16] is 22; u16 rect[15] is 22 (declared length is a range because stmt.c:3419 8-aligns a BLKmode automatic); u16 rect[4] is 36/194.

- [s13] That construct is the same OVERSIZED-LOCALS shape already accepted on main at src/text1b.c:6032 (func_8006DD94, EnvB s + u16 rect[4]) and src/text1a_post.c:404 (func_80041BF4, s16 rect[8]): the extended object is the LIVE address-taken one, not a dead pad. The sibling func_800720FC has the same idiom with 32 spare bytes.

- [s13] All 24 permutations of the four body scalar declarations are byte-identical at 22/194 - the scalar declaration-order axis is inert on this chassis.

- [s13] The first loop body's statement-order axis is exhaustively swept: 6 orders x 2 increment orders, the candidate's order uniquely optimal at 22 (others 24, 24, 29, 30, 30), increments byte-neutral.

- [s13] The pre-guard position of prim.p_geom = *(s32 *)(ctx + 8) has exactly three legal slots; the candidate holds the best (22 vs 25 and 26).

- [s13] $a1/$a2 are caller-saved and the second loop body contains a jal, so the target's loop-carried $a1/$a2 do NOT survive the call: the tail at 28ffc-29004 reloads all three values (lhu a2,0(gp), lh v0,0(gp), lw a1,0(gp)) every iteration and the guard block supplies only the entry copies. The target's tail carries three gp loads where our re-read spelling carries two - which is why the literal carried-locals transcription is 195 insns and 42 points (s12).

- [s13] The target's mode test branches on the sum FIRST (28fb4 bnez on $a1 + (s16)$a2, then 28fc4 bne against 2), so the original source order is if (sum != 0 || D_800A35BC == 2); our 22-point incumbent emits the D_800A35BC-first order. s13 re-derived and re-measured this at 30, matching s11's banked kill: the structurally-correct spelling loses 2 points in the pre-loop block ($v1 -> $a1 on the addiu #,#,72 / sw #,28(sp) pair) plus loop-body scheduling.

- [s13] Residual composition at 22, re-confirmed by objdump: 2 points are the prologue tie (addiu $a0,$sp,24 before vs after move $s0,$zero), the other ~20 are the single cse_set_around_loop cluster (cse.c:7933 REG_LOOP_TEST_P, set only by jump.c:2253 duplicate_loop_exit_test, which needs a top-test loop; the top-test chassis prices out at 31).

- [s14] [enumerate] The second loop's LOCAL SPELLING SPACE IS EXHAUSTED on the 22-point
  if-guarded do/while chassis: 220 spellings measured across six exhaustive axes (128-cell
  bound x mode-test expression cross product; 20-cell invariant-hoist cross product; 21-cell
  declaration-signedness re-audit; 15-cell HImode-carrier sweep; 12 mode-test branch structures;
  all 24 loop-2 statement orders). The incumbent body is the UNIQUE optimum on every axis; the
  only ties are byte-identical. JSONs: tmp/grind/func_80070C70/s14/enum{1,2,4,5,6}.json and
  enum3.{s32,s16,u16}.json.
- [s14] The three expression axes are ADDITIVE and separable, which is why hand-probing them one
  at a time (s6, s11, s13) kept finding "+1 here, +8 there": guard-bound spelling contributes
  0/+1/+3/+4, tail-bound spelling 0/0/+1/+1, mode-test order 0 (35BC-first) or +8 (sum-first),
  and `!= 0` vs bare truthiness is byte-neutral. A cell's score is the incumbent 22 plus the sum
  of its three axis penalties, exactly, in all 128 cells.
- [s14] The target's sum-first mode-test topology costs +8 in EVERY structural dress (plain
  if/else, ternary, duplicated-arm else-if chain, goto/label, De Morgan inversion) - so the
  divergence is NOT a branch-shape choice the C can make. It follows from the target holding
  D_800A3558 in $a2 and D_800A35B0 in $a1 across the loop back edge.
- [s14] The lhu/lh question is now fully characterised. The DECLARATION selects the signedness
  of ALL reads together (u16 decl + bare reads => all three of our reads become `lhu`, still 22;
  s16/u16 decl makes all seven cast spellings byte-identical), confirming s2's convert_to_integer
  folding argument on the current chassis. The ONE construct that does emit the target's
  `sll 16 / sra 16` sign-extension is a HImode carrier LOCAL (`u16 h; h = D_800A3558;` then
  `(s16)h`) - and it measures 25/194 with 48 raw differing instructions vs the incumbent's 44,
  so the shape is reachable and is not what is costing the points.
- [s14] Raw instruction-diff count for the incumbent 22-point body against the target is 44
  differing lines of 194 (tmp/grind/func_80070C70/s14/ours.txt vs s13/tgt.txt) - a useful
  second metric alongside the weighted score: the HImode-carrier form scores +3 AND diffs +4,
  so the two metrics agree on this chassis.

- [s14] Chassis re-verified this session: the s13 candidate body installed into src/text1b.c with its two companion extern edits measures score 22, target_insns 194, build_insns 194 under sandbox --disable all. The ledger floor of 22 is current, not stale.

- [s14] 220 spellings measured in one session across six exhaustive axes (128 + 20 + 21 + 15 + 12 + 24). The incumbent body is the UNIQUE optimum on every axis; the only cells that tie it are byte-identical to it. The second loop's local spelling space is exhausted on this chassis.

- [s14] The three expression axes are ADDITIVE and separable, and this explains the confusing hand-probe history (s6 'the || order is worth 1 point', s13 'the || swap is worth 8'): a cell's score is 22 plus the sum of guard-spelling (0/+1/+3/+4), tail-spelling (0/0/+1/+1) and mode-test-order (0/+8) penalties, exactly, in all 128 cells. Per-axis penalties are chassis-dependent, which is why s6's number differed.

- [s14] The target's sum-first mode-test topology costs +8 in EVERY structural dress (plain if/else, ternary, duplicated-arm else-if, goto/label, De Morgan inversion, nested if), so it is not a branch-shape the C chooses.

- [s14] The lhu/lh question is now fully characterised. The DECLARATION selects the signedness of all reads together (a u16 extern with bare reads turns all three of our loads into lhu, still 22; under an s16 or u16 extern all seven cast spellings are byte-identical), re-confirming s2's convert_to_integer folding argument on the current chassis. The ONE construct that does emit the target's sll 16 / sra 16 sign-extension is a HImode carrier LOCAL, and it measures 25/194 - so the shape is reachable and is not what costs the points.

- [s14] Raw instruction-diff count for the incumbent against the target is 44 differing lines of 194 (tmp/grind/func_80070C70/s14/ours.txt vs s13/tgt.txt). This is a useful second metric: the HImode-carrier form is +3 on score AND +4 on raw diff, so the two metrics agree here rather than masking shape progress.

- [s14] Both remaining signature divergences reduce to ONE fact: the target keeps D_800A3558 (lhu) in $a2 and D_800A35B0 in $a1 live across the second loop's back edge, reloading both in the tail after the jal, and supplies the entry copies from a guard block that is an insn-for-insn copy of the tail test. No local C spelling on the if-guarded do/while chassis reaches that, because a C local is hoisted (measured: every hoist 31-41) and a re-read is CSE'd against the guard's own reads.

- [s14] src/text1b.c was restored byte-exact to its pristine INCLUDE_ASM state at the end of the session; git status shows only metrics/events.jsonl modified.

## [s15 synthesis 2026-09-10] CHASSIS FLIP: the do/while chassis is retired; floor 22 -> 7/194.

- [s15] The four-session foreclosure of the TOP-TEST chassis rested on ONE unexamined
  assumption: that its 24-byte frame overshoot (three combine-orphaned `(use (reg))` spill
  pseudos, vars=104 vs the target's 80) was a cost to be removed. It is not. The do/while
  chassis reaches vars=80 only by declaring the sp+0x48 array OVERSIZED (`u16 rect[16]`,
  0x20 bytes, an oversized-locals FAKE); on the top-test chassis the orphan slots supply 24 of
  those 32 bytes for free, so the array supplies only 8 - `u16 rect[4]`, the honest
  declaration that func_80069898 (src/text1b.c:5413, `u16 *` parameter, reads arg1[0..3]) and
  its two on-main callers (src/text1b.c:5822, 6065) already document. MEASURED: top-test +
  rect[4] = 17/194; top-test + the s12 0x20-byte icon = 31/194 (the s12 number reproduced
  exactly); every rect[N] with N >= 5 = 31. The 14-point gap s12 attributed to the chassis was
  entirely the array's declared size.
- [s15] The remaining 17 -> 7 is the loop bound's ADDITION ASSOCIATION, a lever that only
  exists once the exit test is duplicated into a guard block: the target computes
  ((1 + D_800A35B0) + D_800A3558), and `1 + D_800A35B0 + D_800A3558` (or `D_800A35B0 + 1 +
  D_800A3558`) = 7/194, while every association measured before s15 -
  ((D_800A35B0 + D_800A3558) + 1) - is 17, and the parenthesised (ext + 1) forms are 21 at 196
  insns.
- [s15] AT 7 THE REGISTER ALLOCATION IS EXACT. The target's $a1 = D_800A35B0, $a2 = the `lhu`
  of D_800A3558 and $a0 = the `sll` index are all reproduced, as are the lhu/lh/lw opcode
  triple and the sum-first mode-test topology. s14 spent 220 spellings proving those three
  things unreachable on the do/while chassis and concluded they were "downstream of the
  register allocation of D_800A3558/D_800A35B0 across the loop's back edge" - which was
  correct, and the chassis flip delivers all of them at once.
- [s15] The whole residual at 7 is two pure INSTRUCTION-ORDER ties, and neither moves with any
  source statement position measured so far (100 spellings this session):
    (1) 2 insns - the prologue tie, `addiu a0,sp,24` before (target) vs after (ours)
        `move s0,zero`; now 40+ spellings deep across s11/s13/s15.
    (2) 5 insns - the guard block. Target: move s0,zero / lhu a2,0(gp) / lw v0,24(s1) /
        lw a1,0(gp) / addiu v0,v0,12 / sw v0,24(s1) / lh v0,0(gp). Ours: lw v0,24(s1) /
        move s0,zero / addiu v0,v0,12 / sw v0,24(s1) / lhu a2,0(gp) / lw a1,0(gp) /
        lh v0,0(gp). Identical multiset and registers; the target interleaves the three gp
        loads into the `*(arg0+0x18) += 0xC` chain's latency slots. sched.c
        rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) is the decision point: INSN_PRIORITY
        first, then a three-way dependence classification against last_scheduled_insn. The
        instrumented cc1 already carries a BB2_RANK_DEBUG hook that prints exactly those
        class-tie decisions (sched.c ~2440) - that is the next probe, together with the
        .sched/.sched2 dumps regenerated for this body this session.
- [s15] FAKE ACCOUNTING IMPROVED. The candidate now carries ONE FAKE construct instead of two:
  `u16 rect[16]` is gone (rect[4] is both honest and optimal here); `s32 c60 = 0x60;` remains
  load-bearing on this chassis too (literal = 14/191 vs the local's 7/194) and still needs
  .claude/rules/named-local-fake-exception.md plus a /* FAKE */ annotation at submission.
  Also dropped as byte-neutral: the `s32 var_s3 = 0xA;` carrier and every `(s16)` cast on
  D_800A3558 (the `extern s16 D_800A3558;` declaration selects the read signedness on its own).
- [s15] METHODOLOGICAL NOTE FOR FUTURE SESSIONS: this floor drop came from re-reading a
  FORECLOSED chassis' verdict against a finding made one session later on the OTHER chassis.
  s12 wrote "the top-test chassis' frame penalty remains unaffordable" and s13/s14 then spent
  two sessions and 220 spellings inside the chassis that verdict left standing. The contradiction
  rule in the brief ("if the ledger marks every chassis dead, one verdict is wrong") is what this
  session executed, and the weakest verdict was the one whose cost term had never been
  re-priced after a later discovery changed one of its inputs.

- [s15] FLOOR 22 -> 7 / 194 this session, on a chassis the ledger had treated as foreclosed since s12.

- [s15] The s12 verdict 'the top-test chassis' frame penalty remains unaffordable' was measured with an IconC70 icon declared 0x20 bytes wide. Swapping that for u16 rect[4] reproduces s12's 31 exactly at every rect[N>=5] and gives 17 at rect[4] - the 14-point 'chassis penalty' was the array's declared size, not the chassis.

- [s15] u16 rect[4] is the honest declaration: func_80069898 at src/text1b.c:5413 takes a u16 * and reads arg1[0..3], and both on-main callers (src/text1b.c:5822 func_8006BB68, src/text1b.c:6065 func_8006DD94) declare exactly u16 rect[4]. The do/while chassis' oversized rect[16] FAKE is retired.

- [s15] 17 -> 7 is the loop bound's addition ASSOCIATION, a lever that only exists once jump.c:2253 duplicate_loop_exit_test has copied the test into a guard block: the target computes ((1 + D_800A35B0) + D_800A3558); 1 + D_800A35B0 + D_800A3558 and D_800A35B0 + 1 + D_800A3558 = 7, every ((base+ext)+1) association = 17, and the parenthesised (ext+1) forms = 21 at 196 insns.

- [s15] At 7 the register allocation is EXACT: target $a1 = D_800A35B0, $a2 = the lhu of D_800A3558, $a0 = the sll index, plus the lhu/lh/lw opcode triple and the sum-first mode-test topology, are all reproduced (tmp/grind/func_80070C70/s15/ours.txt vs s13/tgt.txt). s14's conclusion that those three divergences were downstream of the register seats was right, and the chassis flip delivers all three at once.

- [s15] The residual at 7 is exactly two instruction-ORDER ties with identical multisets and registers: 2 insns of the prologue tie (addiu a0,sp,24 before vs after move s0,zero) and 5 insns of the guard block, where the target interleaves the three gp loads into the *(arg0+0x18) += 0xC chain's latency slots (target: move s0,zero / lhu a2,0(gp) / lw v0,24(s1) / lw a1,0(gp) / addiu v0,v0,12 / sw v0,24(s1) / lh v0,0(gp); ours emits the 24(s1) chain first).

- [s15] s32 c60 = 0x60; is still load-bearing on the new chassis (literal = 14/191 vs the local's 7/194) - it remains the candidate's only FAKE construct and still needs .claude/rules/named-local-fake-exception.md plus a /* FAKE */ annotation at submission.

- [s15] Byte-neutral and therefore dropped from the candidate: the s32 var_s3 = 0xA; carrier for the first loop's prim.code, and every (s16) cast on D_800A3558 (the extern s16 D_800A3558 declaration selects the read signedness on its own).

- [s15] METHOD: the drop came from executing the brief's contradiction rule - re-pricing the WEAKEST foreclosure (the one whose cost term had never been re-checked after a later session changed one of its inputs) instead of grinding further inside the chassis that verdict left standing.

- [s16] ENUMERATION (mandated modality), 317 spellings swept in five sweeps, ALL >= 7: the
  guard-region local spelling space, the mode-test/bound operand-order space, the entry-block
  initialiser-position space, the DECLARATION-ORDER space and the invariant-carrier space are
  each exhausted on the 7-point top-test/rect[4] chassis. Histograms:
    v1 (70) guard region = {5 spellings of *(arg0+0x18) += 0xC} x {p_geom inline vs via g} x
      {bound inline vs hoisted local} x {mode-test sum inline vs hoisted local} x {hoist before
      vs after the store}: best 7 (the incumbent form and its `+= 0xC` / `0xC + x` synonyms - all
      three byte-identical), a named temp for the increment = 8, p_geom via g = 25, ANY hoisted
      bound or mode-test local >= 42.
    v2 (48) = {mode-test operand order} x {`!= 0` vs bare} x {clause order} x {6 bound
      associations}: 7 only for associations 1+B0+3558 and B0+1+3558 with sum-first and
      B0-first; operand swap = 8, clause-order swap (BC==2 first) = 15, the four other
      associations >= 15. `!= 0` vs bare is byte-neutral.
    v4 (20) = all 13 source positions of the entry-block `var_s0 = 0;` PLUS rewriting the first
      loop as a for-loop (init in the header) or a while-loop: ALL 7, identical insn counts.
      The emission point of `move s0,zero` is insensitive to the source.
    v5 (147) = all 120 permutations of the five scalar declarations x 9 positions of
      `u16 rect[4]` x 9 of `s32 c60` x 9 of `PrimC70 prim`: ALL 120 scalar permutations = 7;
      c60 position inert (9/9 = 7); rect position inert except the 2 slots that break the frame
      (45); prim position 2 of 9 legal (the rest 45). Declaration order is a dead axis here.
    v6 (30) = carrying D_800A3558 and/or D_800A35B0 in an explicit local ({s16,u16,s32} x
      {with,without an s32 b} x 3 insertion points x 2 orders): best 23, i.e. 16 WORSE.

- [s16] THE MECHANISM OF THE 5-INSN GUARD TIE IS NOT A PRIORITY TIE - IT IS AN ALIAS DEPENDENCE.
  Read out of tmp/grind/func_80070C70/dumps/text1b.sched2 (function slice saved as
  tmp/grind/func_80070C70/s16/f.sched2): all three gp loads in the guard block carry a DATA
  dependence on insn 253, the `sw v0,24(s1)` store of `*(arg0 + 0x18) += 0xC`:
    (insn 479 ... (set (reg/s:HI 6 a2) (mem:HI (symbol_ref "D_800A3558")))  ... (insn_list 253 ...
    (insn 482 ... (set (reg/s:SI 5 a1) (mem:SI (symbol_ref "D_800A35B0")))  ... (insn_list 253 ...
    (insn 480 ... (set (reg:SI 2 v0) (sign_extend (mem:HI (symbol_ref "D_800A3558")))) ... 253 ...
  GCC 2.7.2's sched_analyze cannot prove a store through (plus (reg s1) 24) does not alias a
  symbol_ref MEM, so the loads are PINNED after the store. That is why every spelling this
  session emits the += chain first: the scheduler is not choosing, it is obeying a dependence.
  The target's a1/a2 loads sit BEFORE that store, so in the target's RTL they were emitted
  before it - i.e. the divergence is WHERE the loop invariants are placed (loop.c preheader
  insertion relative to the pre-loop store), not how the block is scheduled.

- [s16] The C-level attempt to move those loads earlier - reading the globals into named locals
  before the store - is measured DEAD (v6, best 23 vs 7): an explicit carrier keeps the value
  live in a callee-saved seat across the loop and changes the loop body's codegen, so it never
  reproduces "hoisted load, per-iteration register use" that loop.c produces for free.

- [s16] Entry-block tie UIDs for the next session: insn 49 = `(set (reg/v:SI 16 s0) (const_int 0))`
  and insn 71 = `(set (reg:SI 4 a0) (plus (reg 29 sp) (const_int 24)))`. In the .sched2 order ours
  is 37, 49, 40, 43, 46, 71, 52; the target's is 37, 71, 40, 43, 46, 49, 52 - a pure swap of 49
  and 71. Insn 71's only dependences are (insn_list 550) and REG_DEP_ANTI 4; insn 49's is
  REG_DEP_ANTI 562. Both are ready in the same cycle, so this one IS the rank_for_schedule
  question (sched.c:2408), and the instrumented cc1's BB2_PRIO_DEBUG (sched.c:1504) will print
  the two INSN_PRIORITY values directly.

- [s16] Chassis re-verified this session: the s15 candidate body reinstalled on HEAD's src/text1b.c scores 7/194 under `sandbox func_80070C70 --disable all` (measured twice, before and after the candidate.c header edit). src/text1b.c was restored to HEAD at the end of the session.

- [s16] The 7-point residual is exactly the two blocks s15 described: (a) entry block, `addiu a0,sp,24` and `move s0,zero` swapped; (b) guard block, target = move s0,zero / lhu a2,0(gp) / lw v0,24(s1) / lw a1,0(gp) / addiu v0,v0,12 / sw v0,24(s1) / lh v0,0(gp), ours = the 24(s1) chain first then the three gp loads. Identical multisets, identical registers.

- [s16] RE-ATTRIBUTION (from tmp/grind/func_80070C70/dumps/text1b.sched2, function slice saved as tmp/grind/func_80070C70/s16/f.sched2): insn 479 (lhu a2 = D_800A3558), insn 482 (lw a1 = D_800A35B0) and insn 480 (lh v0 = D_800A3558) EACH carry a data dependence `(insn_list 253 ...)` on insn 253, the `sw v0,24(s1)` store of *(arg0 + 0x18) += 0xC. GCC 2.7.2's sched_analyze cannot prove a store through (plus (reg s1) 24) does not alias a symbol_ref MEM, so the three loads are PINNED after the store. The guard block is not a scheduler tie at all - the scheduler is obeying a dependence. The target's a1/a2 loads sit BEFORE that store, so in the target's RTL they were emitted before it: the divergence is WHERE loop.c places the invariant hoists relative to the pre-loop store, not how sched.c orders the block.

- [s16] The entry-block divergence IS a genuine rank_for_schedule question: insn 49 = (set (reg/v:SI 16 s0) (const_int 0)) and insn 71 = (set (reg:SI 4 a0) (plus (reg 29 sp) (const_int 24))). Our .sched2 order is 37, 49, 40, 43, 46, 71, 52; the target's is 37, 71, 40, 43, 46, 49, 52 - a pure swap. Insn 71's only dependences are (insn_list 550) and REG_DEP_ANTI 4; insn 49's is REG_DEP_ANTI 562; both are ready in the same cycle.

- [s16] ENUMERATION: 317 spellings across five sweeps, best 7, 293 at the floor or above with 0 below. v1 (70) guard-region local spelling; v2 (48) mode-test/bound operand-order space; v4 (20) entry-block initialiser positions + first-loop chassis rewrites; v5 (147) declaration-order space; v6 (30) explicit invariant carriers. Per-sweep histograms are in tmp/grind/func_80070C70/s16/v{1,2,4,5,6}.json and in evidence.md.

- [s16] Byte-neutral synonyms confirmed on this chassis (all 7): `*(arg0+0x18) = *(arg0+0x18) + 0xC`, `*(arg0+0x18) += 0xC`, `*(arg0+0x18) = 0xC + *(arg0+0x18)`; and `(X != 0)` vs a bare `X` in the mode test. A named temp for the increment is NOT neutral (8).

- [s16] Sanity note for a later session: a generator that reuses s15/gen1.py's default FOR macro inherits the LOSING bound association (D_800A35B0 + D_800A3558 + 1) and the (s16)-cast mode test, which alone costs 10 points (17 vs 7). tmp/grind/func_80070C70/s16/gen4.py shows the two .replace() calls that fix it; v3 is the mis-based sweep kept only as the control that proved the axis inert at 17.

- [s16] s16 rejected forms banked: rejected/s16-hoist-D_800A35B0-local-preheader-23.c, s16-himode-carrier-D_800A3558-local-32.c, s16-named-temp-for-0x18-increment-8.c, s16-modetest-operand-swap-8.c, s16-modetest-clause-order-swap-15.c.

## s17 (structural, 2026-09-10) — 7 -> 0 / 194, BYTE MATCH, full-build SHA1 == oracle

- [s17] **THE MECHANISM.** `tools/gcc-2.7.2/sched.c:817 true_dependence` returns 0 when
  `MEM_IN_STRUCT_P(store) && rtx_addr_varies_p(store) && GET_MODE(store) != QImode &&
  !MEM_IN_STRUCT_P(load) && !rtx_addr_varies_p(load)`. s16 closed the 5-insn guard-block
  residual as "the three gp loads carry a data dependence on the `*(arg0 + 0x18) += 0xC`
  store", which was correct but INCOMPLETE: the dependence exists only because a pointer-cast
  deref (`*(s32 *)(arg0 + 0x18)`) never sets MEM_IN_STRUCT_P. Spelling the access as a
  COMPONENT_REF (`((GameObj *)arg0)->field_18`) sets it, the loads (symbol_ref, fixed address,
  not in a struct) become independent of the store, and sched.c produces the target's
  interleave. **7 -> 2 in one edit.** (tmp/grind/func_80070C70/s17/v1.json)
- [s17] Granularity measured: all 12 `arg0 + 0x18` sites as struct members = 2; only the four
  sites of the pre-third-loop block = 2; only the store's LHS = 2; only the store's RHS (the
  read) = 7. It is the STORE's MEM that must be in-struct - which is exactly what the predicate
  says (the store is `mem`, the loads are `x`).
- [s17] A `GameObj *o = (GameObj *)arg0;` alias local instead of inline casts costs 18 (the
  alias pseudo takes a register seat). Inline casts at each use are both cheaper and honest.
- [s17] **2 -> 0.** On the struct-store chassis the second loop's induction-variable initialiser
  is NO LONGER INERT (s16's kill was explicitly chassis-relative). Writing that loop as
  `for (var_s0 = 0; var_s0 < 6; var_s0++)` - initialiser in the loop header instead of the entry
  block - flips the `addiu a0,sp,24` / `move s0,zero` prologue tie to the target's order. All 13
  entry-block positions from the first `func_8007352C` call onward also give 0; the 4 positions
  before it stay at 2. (s17/v2.json, 31 spellings)
- [s17] The `prim.p_static = t; prim.p_static = t + (D_800A3590[i] << 4);` dead store is NOT
  required: the compound `prim.p_static += D_800A3590[i] << 4;` (ordinary split-init
  accumulation) is also 0. Dropping the first store entirely costs 22. (s17/v4.json)
- [s17] The duplicated `prim.zero1C = 0; prim.mode = 0;` pair is ORIGINAL CODE, not a coercion:
  asm/funcs/func_80070C70.s emits `sw zero,48(sp)` / `sw zero,52(sp)` before the `lw s2,100(v1)`
  context fetch and again after it. Single occurrence = 2, half of it = 1, both = 0. 20
  single-occurrence positions/orders swept (s17/v4.json).
- [s17] `s32 c60 = 0x60;` remains load-bearing on this chassis too: inline literal = 7/191,
  `const`-qualified = 7, split init (`s32 c60; c60 = 0x60;`) = 0, position after the other decls
  = 0. The target holds the constant in a callee-saved register (`li s4,96` + three
  `move a1,s4`), which is what a live-across-call C local produces. FAKE-annotated under
  .claude/rules/named-local-fake-exception.md. (s17/v5.json)
- [s17] The s15 `IconC70 { ...; s16 sp50[12]; }` frame-coercion field is NOT needed on this
  chassis and has been removed; so has the wrong `extern s32 func_80069898(s32, s32 *, s32);`
  m2c prototype. Kept declaration edits: `extern s16 D_800A3558;`, `extern u8 D_800A3560[];`,
  `extern s16 D_800A3590[];` (the last two applied at the ORIGINAL declaration site, lines
  2124/2129, not as a second conflicting declaration).
- [s17] `& tools/wteng.ps1 main verify-oracle` -> ok:true, build_sha1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa. The TU-wide declaration changes break nothing.
- [s17] **Transferable lesson for every sibling in this repo:** when a scheduler residual is
  "loads pinned after a base-register store", the question is not the schedule, it is whether
  the store is a COMPONENT_REF. Pointer-cast derefs (`*(s32 *)(p + N)`) suppress GCC 2.7.2's
  only memory-disambiguation heuristic; typed struct access restores it.
