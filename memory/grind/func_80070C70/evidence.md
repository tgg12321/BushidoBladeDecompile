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
