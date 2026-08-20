# Hypothesis ledger — func_800611A4

## [s1] Pin-free candidate scores floor=9 (43 insns == 43 target insns, pure register-name swap v0<->v1).
- mechanism: sandbox --disable all strips cheat-asm; residual is RA-only, no add/del/reorder.
- probe: Applied memory/grind/func_800611A4/candidate.c to src/text1b.c and ran `sandbox func_800611A4 --disable all`.
- result: score=9, target_insns=43, build_insns=43, rules_dropped=0, cheat_asm_stripped=393.
- verdict: CONFIRMED

## [s1] The RA pivot is the mask's 2-instruction lui+ori construction (0xFFFFEF/0xFF0060/0x10FFFF); when the mask fits in 1 insn (0xFF0000 in sibling func_800618B4), target flips RA and puts mask->$v0, 3rd load-temp->$v1.
- mechanism: GCC allocno priority orders lower-numbered pseudos to lower hard regs; the 2-insn mask has different ref/liveness weight than a 1-insn mask, so the mask-vs-load-temp tiebreak inverts between the two mask shapes.
- probe: Compared tail asm of func_800611A4 / 61250 / 61658 (2-insn masks) vs func_800618B4 (1-insn mask). All four share the identical `for i in 0..2: t=arg0[i]; STORE_i=t;` + `MASK_G=mask;` post-call cluster.
- result: Three 2-insn-mask siblings: mask lives in $v1, load-temp in $v0. One 1-insn-mask sibling: mask in $v0, load-temp in $v1 (on 3rd load). Mask-width is the RA pivot.
- verdict: CONFIRMED

## [s1] `restore-discarded-return-displaces-v0` does NOT apply here — func_80060A68 is declared `void` in-file AND its body has no return AND the sole caller (src/code6cac_b.c:3166) does not capture a return.
- mechanism: Rule requires an extern caller that writes `var = func(...)`; grep found none.
- probe: grep -rn 'func_800611A4\|func_80060A68' src/ include/ + read func_80060A68 body at text1b.c:13314.
- result: Both functions void-void; no extern return-capture caller. Lever ruled out for THIS function; may still apply to callers of the siblings — untested.
- verdict: KILLED

## [s2] Direct inlined stores (no `s32 t` local) — anonymous single-use pseudos would relax the tiebreak surface.
- mechanism: Named-local reuse produces one 6-ref web; anonymous inlined loads produce three 2-ref pseudos. If load-web dominance was the tiebreak, removing it should flip RA.
- probe: Rewrote post-call body as `D_800F1140 = arg0[0]; D_800F1144 = arg0[1]; D_800A3464 = 0xFFFFEF; D_800F1148 = arg0[2];`. Sandbox.
- result: score=11 (worse), build_insns=43. Pure RA shift; mask displacement got worse without the load-temp web.
- verdict: KILLED

## [s2] Dropping the `s32 *v1 = &D_800F116C` pointer alias frees an early-computed hard reg, propagating into post-call RA state.
- mechanism: The v1 pseudo lives from function entry across the call; removing it changes pre-call allocation which carries into post-call tiebreak.
- probe: Wrote `D_800F116C = 0x21001A; D_800A3468 = (s32)&D_800F116C;` directly (no alias). Sandbox.
- result: score=20, build_insns=44 (+1 insn). Without the alias GCC cannot share the lui/base between the two D_800F116C accesses — adds a full lui+addiu computation. Alias is load-bearing for address CSE, not tiebreak lever.
- verdict: KILLED

## [s2] Tightening `*v1 = 0x21001A;` liveness (immediately after `D_800A3468 = (s32) v1;`) shifts pre-call RA state.
- mechanism: Shorter alias-write liveness reduces pseudo lifetime across the pre-call cluster.
- probe: Moved `*v1 = 0x21001A;` to right after the D_800A3468 store, kept alias.
- result: score=20, build_insns=43. Reordering perturbs sp[]/arg1-halfword interleave badly.
- verdict: KILLED

## [s2] Declaration order of `t` and `mask` locals is the RA tiebreak axis (pseudo number tiebreak in GCC's allocno priority).
- mechanism: GCC breaks equal-priority allocnos by pseudo number, which is declaration order.
- probe: Swapped decl to `s32 mask; s32 t;` (mask first). Also tried mask-assigned-first post-call.
- result: Both cases score=9 unchanged. Pseudo declaration order does NOT determine the load-temp-vs-mask tiebreak here.
- verdict: KILLED

## [s2] Load-temp type width (u32 vs s32) affects RA priority via mode class.
- mechanism: Different mode class might have different reg preferences.
- probe: Changed `s32 t;` to `u32 t;` with `(u32)arg0[i]` casts.
- result: score=9, build_insns=43. Type width not the tiebreak.
- verdict: KILLED

## [s2] Walking-pointer form (`*arg0++`) reshapes the load-side pseudo class, potentially flipping RA of load-temp vs mask.
- mechanism: Serialized dep-chain loads produce different pseudo lifetimes than array-indexed loads.
- probe: Wrote `D_800F1140 = *arg0++; D_800F1144 = *arg0++; D_800A3464 = 0xFFFFEF; D_800F1148 = *arg0;` — also with named-temp form.
- result: score=7 (LOWERED from 9), build_insns=44. Load-temp lands in $v0 (matching target's direction), mask in $v1 — several register-name diffs disappear. But +1 insn (the addiu bump) is intrinsic and cannot reach sandbox 0.
- verdict: CONFIRMED

## [s3] Pre-call freelist ordering is the RA tiebreak lever (s2 frontier hypothesis)
- mechanism: Post-call RA state inherits the pre-call freelist; different pre-call orderings would leave $v0 vs $v1 differently ordered on the freelist.
- probe: Diffed our pre-call disasm against target's asm; also tried V2 (hoist D_800A3468) and V12 (hoist *v1=K).
- result: Pre-call is BYTE-IDENTICAL to target on the baseline pin-free candidate. Every reorder made it worse (V2=19, V12=20). Cannot alter the pre-call freelist because pre-call is already at its optimum.
- verdict: KILLED

## [s3] Mask atomically at very top of post-call collapses the mask liveness so mask+load-temp share $v0 → drops score below the 9-wall.
- mechanism: With `D_800A3464 = 0xFFFFEF;` (mask assign+store fused) as the FIRST post-call statement, mask pseudo dies before load 1 begins. GCC reuses $v0 for the load-temp instead of assigning them to conflicting registers.
- probe: V4 `mask=K; D_800A3464=mask;` at top; V7 identical `D_800A3464 = 0xFFFFEF;` (no local) at top; both scored 6/43.
- result: Score 6 (baseline 9). New floor. Structurally different from target (target keeps mask alive across loads in $v1 via interleaved lui/ori/sw); V7 has all-$v0 usage instead.
- verdict: CONFIRMED — floor lowered 9→6.

## [s3] Any mask position OTHER than atomic-first is equivalent to baseline 9 — GCC schedules them all to target's interleaved shape but with the mask→$v0 / load-temp→$v1 swap that plateaued s1/s2.
- mechanism: Once mask is alive across ≥1 load, GCC's global allocator picks $v0 for mask (higher priority allocno by its pseudo-priority formula) and displaces load-temp to $v1. This is the reverse of target's assignment.
- probe: V5 (mask defined first, stored between 2&3), V6 (mask between 1 and 2), V9 (mask between load 1 and store 1, stored after load 3), V16 (mask between 2&3, stored last), V21 (mask at end), V22 (mask inline between 1&2). All scored 9 with 43 insns.
- result: Six distinct interleave positions all hit the same 9-wall with the same $v0/$v1 swap.
- verdict: CONFIRMED — mask-position axis is measured dead outside V4/V7.

## [s3] u32 mask type (mode class) is the RA priority axis.
- mechanism: Different mode class might give mask pseudo different priority weight in global.c's `allocno_compare`.
- probe: V11 changed `s32 mask` to `u32 mask`, otherwise baseline shape.
- result: Score 9 unchanged.
- verdict: KILLED

## [s3] Pure m2c-reconstructed shape (no `t`, no `mask`, no v1 alias) matches original target C.
- mechanism: m2c's output for target says target had direct writes with no `t` load-temp and no `s32 *v1` alias. If that's the true original, compiling it should reach target.
- probe: V10 wrote the pure m2c shape verbatim.
- result: Score 22, build=44 (v1 alias drop adds an insn as s2 banked; combined with no-t-local also lifts RA cost).
- verdict: KILLED — m2c's reconstruction is not the C GCC 2.7.2 compiles back to target bytes. The v1 alias IS in the original C.

## [s3] Pre-call ordering is the RA tiebreak lever (s2 frontier).
- mechanism: Post-call RA state inherits the pre-call freelist; different pre-call orderings shift $v0 vs $v1 placement in the freelist.
- probe: Disassembled tmp/sandbox/func_800611A4/text1b.o and diffed pre-call against asm/funcs/func_800611A4.s; also measured V2 (hoist D_800A3468 = v1 before halfword reads) and V12 (move *v1 = 0x21001A early).
- result: Pre-call byte-identical to target on baseline pin-free candidate. Every reorder made it worse: V2=19, V12=20 (both build=43). Cannot alter freelist because sequence is already optimal.
- verdict: KILLED

## [s3] Mask assign+store atomically fused as the FIRST post-call statement collapses mask liveness so mask+load-temp share $v0 -> below the 9 wall.
- mechanism: With `D_800A3464 = 0xFFFFEF;` (or `mask=K; D_800A3464=mask;`) as the first post-call statement, mask pseudo dies before load 1 starts. GCC has no conflict and reuses $v0 for the three load-temps.
- probe: V4 (`mask = 0xFFFFEF; D_800A3464 = mask;` at top) and V7 (inline `D_800A3464 = 0xFFFFEF;` at top) with `t = arg0[i]; D_800F114{0,4,8} = t;` for the three loads. Sandbox --disable all.
- result: Both scored 6 with build_insns=43 (target_insns=43). New pin-free floor lowered from 9 to 6. Structurally different from target (target keeps mask alive across loads in $v1 via interleaved lui/ori/sw; V7 has mask atomic-first with all-$v0 usage), but produces strictly fewer differing insns than any interleaved variant.
- verdict: CONFIRMED

## [s3] Any mask position OTHER than atomic-first is equivalent to baseline 9 -- GCC schedules them all to target's interleaved shape but with mask->$v0/load-temp->$v1 swap.
- mechanism: Once mask is alive across >=1 load, GCC's global.c allocno_compare priority assigns mask to $v0 (higher priority) displacing load-temp to $v1 -- reversed from target.
- probe: V5 (mask defined first, stored between load 2 and 3), V6 (mask between 1 and 2), V9 (mask defined between load 1 and store 1, stored after load 3), V16 (mask between 2 and 3, store last), V21 (mask at end), V22 (mask inline between load 1 and 2). All build=43.
- result: All six interleave positions scored 9 -- the same wall s1/s2 hit. Mask-position axis is measured dead outside V4/V7's atomic-first arrangement.
- verdict: CONFIRMED

## [s3] u32 (unsigned) mode class for mask changes RA priority weight.
- mechanism: GCC's allocno priority uses `size` (mode class) as a multiplier; different mode class might tip the tiebreak.
- probe: V11: `u32 mask = 0xFFFFEF;` otherwise baseline shape.
- result: Score 9 unchanged.
- verdict: KILLED

## [s3] The pure m2c-reconstructed shape (no `t`, no `mask`, no v1 alias) is the original target C.
- mechanism: m2c's tmp/grind/func_800611A4/s1/m2c.txt reconstructs target with direct writes and no locals; if faithful, compiling it should reach 0.
- probe: V10 wrote the pure m2c shape verbatim; sandbox.
- result: Score 22, build_insns=44. The v1 alias drop adds one insn (already banked as rejected/drop_v1_alias.c in s2); combined with no-t-local, RA also degrades. m2c's shape is NOT what our fork compiles back to target bytes.
- verdict: KILLED

## [s4] Seeding the permuter DIRECTLY from the V0 interleaved target-shape (fresh basin, not the V7 seed the prior s4 attempts used) finds a cheat-FREE form below the sandbox-6 floor.
- mechanism: The interleaved shape's scheduling already matches target (mask lui/ori interleaved into load gaps); permuter's weighted metric ranks it ~50 (vs V7's 610). Seeding there gives random mutation a fresh 20-30 min clock at the shape closest to target, so a mutation that flips the mask/load-temp RA tiebreak within the interleaved shape could reach 0 without adding an insn.
- probe: New workspace tmp/grind/func_800611A4/s4/perm2 seeded from the interleaved V0 form (base_score=50). 69119 iters / 6 jobs / 30 min, harvest+stop. Measured every sub-50 find's honest sandbox --disable all.
- result: 3 novel finds. Cheat-free finds plateau at the same interleaved 9-wall (output-50-1). The only sub-6 finds require coercion locals: output-30-1 (sandbox 5) dual-purposes an invented variable (cheat-reviewer FAIL); output-40-1 (sandbox 7, above floor) uses a pointer-alias + staged intermediate. NO cheat-free form below 6.
- verdict: KILLED — the interleaved-V0 basin, like the V7 basin before it, contains no cheat-free form below the sandbox-6 floor. Every sub-9 form in this basin is coercion-gated.

## [s4] The mask/load-temp RA tiebreak refined: giving the load side $v0 displaces the mask to a THIRD register, never $v1.
- mechanism: In every form where the three post-call load-temps land in $v0 (matching target), the mask pseudo is pushed OUT of the $v0/$v1 pair entirely — to $a0 in the variable-reuse form (output-30-1). Target uniquely places load-temp=$v0 AND mask=$v1 interleaved. GCC's allocno priority never co-schedules the load web to $v0 while leaving the 2-insn mask pseudo in $v1; whichever pseudo the C structure hands $v0, the other is displaced to the next free caller-save, which is $a0 (freed by the pre-call homing) rather than $v1.
- probe: Disassembled output-30-1 (out30_disasm.txt): all 3 loads = $v0 (match), mask = $a0 (target $v1), pre-call halfword = $a0 (target $v0). 5 residual diffs.
- result: Confirms the wall is a THREE-way register interaction (load-temp web, mask pseudo, pre-call halfword pseudo competing for $v0/$v1/$a0), not a simple 2-pseudo swap. The pin ($3) forces mask into $v1 by fiat; no cheat-free lever measured this session reproduces that placement while keeping load-temp in $v0.
- verdict: CONFIRMED (mechanistic refinement of the s1-s3 tiebreak model).

## [s4] Seeding the permuter DIRECTLY from the V0 interleaved target-shape (a structurally-different chassis from the V7 atomic-first seed the two prior s4 campaigns used) finds a cheat-FREE form below the sandbox-6 floor.
- mechanism: The interleaved shape's scheduling already matches target (mask lui/ori interleaved into the load-delay gaps); permuter's weighted metric ranks it ~50 vs V7's 610. Seeding there gives random mutation a fresh 20-30 min clock at the shape closest to target, so a mutation flipping the mask/load-temp RA tiebreak WITHIN the interleaved shape could reach 0 without adding an insn.
- probe: New workspace tmp/grind/func_800611A4/s4/perm2 seeded from the interleaved V0 form (base_score=50). 69119 iters / 6 jobs / 30 min via tools/permuter_campaign.py, harvest --stop. Measured every sub-50 find's honest sandbox --disable all and disassembled the best.
- result: 3 novel finds. Cheat-free finds plateau at the same interleaved 9-wall (output-50-1). Only sub-6 finds require coercion locals: output-30-1 (sandbox 5) dual-purposes an INVENTED variable new_var2 across two unrelated statements (cheat-reviewer FAIL 2026-07-22); output-40-1 (sandbox 7, above floor) uses a fresh pointer-alias &arg0[1] + staged intermediate. NO cheat-free form below 6.
- verdict: KILLED

## [s4] The mask/load-temp RA tiebreak is a simple 2-pseudo v0<->v1 swap (the s1-s3 model).
- mechanism: s1-s3 characterized the 9-wall as mask->$v0 / load-temp->$v1 (reverse of target). If it were purely 2-pseudo, a lever giving the load side $v0 would hand the mask $v1 (target's placement).
- probe: output-30-1's variable-reuse is the first measured form to flip ALL THREE load-temps to $v0 (matching target's load side exactly). Disassembled tmp/sandbox/func_800611A4/text1b.o -> tmp/grind/func_800611A4/s4/out30_disasm.txt.
- result: When the load side gets $v0, the mask pseudo is displaced OUT of the $v0/$v1 pair entirely -- to $a0 (target wants $v1; 3 residual diffs), and the pre-call arg1+2 halfword also regresses to $a0 (target $v0; 2 diffs). The wall is a THREE-way register interaction (load web / mask pseudo / pre-call halfword competing for $v0/$v1/$a0), not a 2-pseudo swap. No cheat-free lever reproduces target's load=$v0 AND mask=$v1 simultaneously.
- verdict: CONFIRMED

## [s5] Directed PERM_LINESWAP + PERM_RANDOMIZE over the post-call cluster ONLY (pre-call frozen) finds a cheat-free form below the sandbox-6 floor.
- mechanism: s4's two random campaigns randomized the WHOLE function, wasting iterations perturbing the byte-identical pre-call region. Freezing pre-call (outside the PERM_RANDOMIZE region) + adding an explicit PERM_LINESWAP enumeration of the 4 post-call units concentrates the search on the only region with gradient — the mask/load statement geometry.
- probe: Workspace tmp/grind/func_800611A4/s5/permA, base_score=50, 6 jobs, ~34k iters / ~20 min, harvest --stop. Measured each novel find's honest sandbox.
- result: 5 novel finds, ALL coercion-gated (invented staging vars / pointer aliases / dead stores). No cheat-free form below the interleaved-50 plateau; a full 9-min window closed with zero novel finds. KILLED — the directed-lineswap-frozen-precall basin, like both s4 basins, has no cheat-free sub-6 form.
- verdict: KILLED

## [s5] The three-way RA wall (load->$v0 + mask->$v1 interleaved) is unreachable by ANY C structure, cheat or not.
- mechanism: s1-s4 model said giving the load side $v0 displaces the mask to a THIRD register ($a0), never $v1. If true even for cheat forms, no C reaches target's layout.
- probe: output-30-1 (invented-var staging the offset-0 load, reverse load order) honest sandbox=2; disassembled tmp/grind/func_800611A4/s5/out30_s5_disasm.txt.
- result: REFUTED for this form's LAYOUT — the invented-var staging DOES reproduce target's exact three-way layout (load=$v0 all three, mask=$v1 interleaved lui/ori/sw). The wall is NOT "mask can never reach $v1". The residual 2 diffs are purely load-offset ORDER (form emits 0x8,0x4,0x0; target 0x0,0x4,0x8). So s4's "mask displaced to $a0" was specific to s4's forms, not universal.
- verdict: CONFIRMED (refines the wall model: the RA layout IS reachable; the true residual is reconciling the flip with target's forward load order).

## [s5] The staging var is the RA-flip lever, not the load reordering; and the flip cannot be reconciled with target's forward load order.
- mechanism: isolate whether new_var2 (staging) or the reverse load order produces the flip; then test whether the flip survives forward order (which target uses).
- probe: (a) cheat-free reverse-order form, plain `t`, no new_var2; (b) new_var2 staging offset-0 load with forward order; (c) new_var2 staging offset-2 load with forward order.
- result: (a) = 9 (the wall — reordering alone does nothing; staging is the lever). (b) = 11. (c) = 8. Only reverse-order + stage-offset-0-last = 2. Every staged form matching target's forward ORDER fails to flip the RA; the only form that flips it has reverse order. Target's forward-order + flipped-RA is what the $3 pin forces by fiat and is not produced by our GCC from any staged C measured.
- verdict: CONFIRMED. And new_var2 is a cheat (invented staging local, prereq #2 violation); the fn's existing dead locals (`new_var` u16 truncates; `v1` s32* needs int/ptr pun) are unusable as sanctioned carriers. Floor stays 6.

## [s5] A directed PERM_LINESWAP+PERM_RANDOMIZE campaign over the post-call cluster only (pre-call frozen) finds a cheat-free form below the sandbox-6 floor.
- mechanism: s4's two random campaigns randomized the whole function, wasting iterations on the byte-identical pre-call. Freezing pre-call outside the PERM_RANDOMIZE region + an explicit PERM_LINESWAP over the 4 post-call units concentrates the search on the only region with gradient.
- probe: Workspace tmp/grind/func_800611A4/s5/permA, base_score=50, 6 jobs, ~34k iters/~20 min, harvest --stop. Honest sandbox of every novel find.
- result: 5 novel finds, ALL coercion-gated (invented staging vars, pointer aliases, dead self-assigns, dead branches). No cheat-free form below the interleaved-50 plateau; a full 9-min window closed with zero novel finds.
- verdict: KILLED

## [s5] The three-way RA layout target uses (load-temp->$v0 all three, mask 0xFFFFEF->$v1 interleaved) is unreachable by any C structure (s1-s4 said giving the loads $v0 always displaces the mask to a THIRD register, never $v1).
- mechanism: If the mask can never land on $v1 while the loads take $v0, no C reaches target's layout and only the $3 pin forces it.
- probe: output-30-1 (invented new_var2 staging the offset-0 load, reverse load order) honest sandbox=2; disassembled out30_s5_disasm.txt.
- result: REFUTED for the LAYOUT: the invented-var staging DOES reproduce target's exact layout (load=$v0 all three, mask=$v1 built interleaved lui/ori/sw). The residual 2 diffs are purely load-offset ORDER (form emits 0x8,0x4,0x0; target 0x0,0x4,0x8). The wall is not 'mask can never reach $v1'; it is 'the RA flip only occurs with reverse load order'.
- verdict: CONFIRMED

## [s5] The staging var (not the load reordering) is the RA-flip lever, and the flip cannot be reconciled with target's forward load order.
- mechanism: Isolate lever (staging vs reorder) then test whether the flip survives forward order (which target uses).
- probe: (a) cheat-free reverse-order, plain t, no new_var2; (b) new_var2 stage offset-0 forward; (c) new_var2 stage offset-2 forward.
- result: (a)=9 (wall; reorder alone does nothing). (b)=11. (c)=8. Only reverse-order + stage-offset-0-last = 2. Every staged form matching target's forward ORDER fails to flip the RA; the only form that flips it has reverse order. new_var2 is a cheat (invented staging local, prereq #2); the fn's existing dead locals (new_var u16 truncates; v1 s32* needs int/ptr pun) are unusable as sanctioned carriers.
- verdict: CONFIRMED

## [s6] The v0<->v1 wall is a LOCAL-allocator (local-alloc.c:472 death-count gate) decision, not global.c.
- mechanism: local-alloc.c:472 marks a pseudo LOCAL only if reg_n_deaths==1. The reused load-temp `t` (3 loads/3 stores = reg_n_deaths 3) is punted to global; the single-death interleaved mask is local. Local-alloc runs first, find_free_reg gives the mask the lowest free reg v0 (no MIPS REG_ALLOC_ORDER; mask is an unconstrained constant with no copy-suggestion). Global-alloc then gets the load-web and, v0 taken, assigns v1. => loads=$v1/mask=$v0 (reverse of target).
- probe: instrumented cc1 -da on formA (flip)/formC (wall). Read .greg dispositions + "N regs to allocate", .lreg reg_n_deaths ("dies in N places"), .combine pseudo structure. Confirmed target=formC structure via asm/funcs disasm.
- result: FORM C load-web(pseudo75,deaths3)->global->v1, mask(81)->local->v0 = sandbox 9. FORM A: new_var2 splits offset-0 load to pseudo75(deaths1)->local->v0, mask(82)->v1, 2-web(76)->global->v0 = sandbox 2 = target's exact layout. greg: "1 regs to allocate" (the load-web) in every form.
- verdict: CONFIRMED — the wall is local-alloc.c:472 + find_free_reg lowest-reg-first; the staging var's sole effect is converting one load into a single-death local pseudo that wins v0 and evicts the mask to v1.

## [s6] Reusing an EXISTING var (v1 s32*) as the staged carrier reproduces the flip cheat-free (frontier bullet 2).
- mechanism: if v1 (already used pre-call, dead post-call) can carry the staged last load, it is an existing var (pointer-alias-fake-exception candidate), not an invented one, so it could clear cheat-reviewer.
- probe: FORM D (forward) + FORM E (reverse) stage the last load through `v1 = (s32*)arg0[k]; t=(s32)v1;`. Standalone cc1 asm.
- result: BOTH fail to flip. v1 is live across the call, so reusing it post-call extends its range across the call boundary -> allocated as a call-crossing pseudo to $a2($6); load spreads 3-way; mask stays $v0. Worse than floor. Banked rejected/s6_v1carrier_{forward_spread,reverse_noflip}.c.
- verdict: KILLED — the flip needs a FRESH single-death local pseudo; no existing dead local qualifies (new_var u16 truncates; v1 s32* crosses the call -> wrong reg class). The frontier's "sanctioned existing-carrier" path is closed.

## [s6] The RA flip is reconcilable with target's FORWARD load order via a cheat-free structure (frontier bullet 1).
- mechanism: if some forward-order C makes the staged/last load a single-death local that grabs v0 without overlapping the interleaved mask, the flip would occur in forward order (= target).
- probe: FORM B (forward + new_var2 stage offset-2 last) and FORM D asm/greg vs FORM A (reverse). Compared liveness geometry of staged load vs interleaved mask.
- result: forward-staging spreads to a 3rd register (sandbox 8), never flips. Reason is geometric: target builds the mask AFTER load-8 (ori/sw follow the last load), so a forward staged-last (offset-8) load's live range OVERLAPS the still-live mask -> conflict -> spill to $a0/$a2. Only reverse order gives the staged last (offset-0) load a live range that starts after the mask's sw (non-overlap), letting both reuse v0. Target's forward-order mask->$v1 is exactly what the $3 pin forces by fiat.
- verdict: KILLED for cheat-free forward match — the flip is geometrically incompatible with target's forward-order-plus-interleaved-mask layout on this toolchain; the only forms that flip use reverse order (residual 2 = load offsets) AND an invented carrier (cheat).

## [s6] The v0<->v1 register wall is decided in LOCAL register allocation (local-alloc.c:472), not global.c.
- mechanism: local-alloc.c:472 marks a pseudo LOCAL only if reg_n_deaths==1. The reused load-temp t (3 loads + 3 stores = 3 deaths) is punted to global-alloc; the single-death interleaved mask (in-place lui/ori RMW = 1 death) is local. Local-alloc runs first; find_free_reg picks the lowest free reg (MIPS defines no REG_ALLOC_ORDER -> default v0=reg2 before v1=reg3), and the mask is an unconstrained li constant with no copy-suggestion, so it grabs v0. Global-alloc then allocates the punted load-web and, v0 taken, gives it v1 -> loads=$v1/mask=$v0, the reverse of target.
- probe: Instrumented cc1 -da (RTL/lreg/greg/combine dumps) on formA (flip, sandbox 2) vs formC (wall, sandbox 9). Read .greg 'Register dispositions' + 'N regs to allocate', .lreg reg_n_deaths ('dies in N places'), .combine pseudo structure. Cross-checked target = formC structure via asm/funcs/func_800611A4.s.
- result: FORM C: load-web pseudo 75 (deaths=3)->global->v1, mask 81->local->v0 = wall. FORM A: new_var2 splits the offset-0 load into pseudo 75 (deaths=1)->local->v0, mask 82->v1, 2-load-web 76->global->v0 = target's exact 3-way layout. greg shows '1 regs to allocate' (the load-web) in every form.
- verdict: CONFIRMED

## [s6] Reusing an EXISTING var (the v1 s32* pointer) as the staged carrier reproduces the flip cheat-free (frontier bullet 2 / pointer-alias-fake-exception).
- mechanism: v1 is used pre-call and dead post-call, so if it can carry the staged last load it is an existing var (sanctionable) rather than an invented one.
- probe: FORM D (forward) and FORM E (reverse) stage the last load via `v1 = (s32*)arg0[k]; t=(s32)v1;`. Standalone cc1 asm.
- result: Both FAIL to flip. v1 is live across the call, so reusing it post-call extends its range across the call boundary -> it is allocated as a call-crossing pseudo to $a2($6); the load spreads 3-way and the mask stays $v0. Worse than floor. The flip needs a FRESH single-death local pseudo; no existing dead local qualifies (new_var is u16 -> truncates s32; v1 is s32* and crosses the call -> wrong reg class).
- verdict: KILLED

## [s6] The RA flip can be reconciled with target's FORWARD load order (0x0,0x4,0x8) via a cheat-free structure (frontier bullet 1).
- mechanism: If some forward-order C makes the staged/last load a single-death local pseudo that grabs v0 without overlapping the interleaved mask, the flip occurs in forward order = target.
- probe: FORM B (forward + stage offset-2 last) and FORM D asm/greg vs FORM A (reverse). Compared liveness geometry of the staged load against the interleaved mask.
- result: Forward-staging spreads to a 3rd register (sandbox 8), never flips. Geometric: target builds the mask AFTER load-8 (ori/sw follow the last load), so a forward staged-last (offset-8) load's live range overlaps the still-live mask -> conflict -> spill to $a0/$a2. Only reverse order gives the staged last (offset-0) load a range starting after the mask's sw (non-overlap), letting both reuse v0. Target's forward-order mask->$v1 is what the $3 pin forces by fiat.
- verdict: KILLED

## [s7] FRONTIER-2: a cheat-free C shape can make the interleaved 0xFFFFEF mask a multi-death pseudo so it is punted to global-alloc, where the 6-ref load-web outranks the 2-ref mask for $v0 (leaving mask $v1 = target).
- mechanism: local-alloc.c:472 routes a pseudo to GLOBAL if reg_n_deaths!=1 OR reg_basic_block<0. A global mask + global web are ordered by qty priority floor_log2(refs)*refs*size/(death-birth); web (6 refs) beats mask (2 refs) for the lowest reg. But routing the mask to global requires >=2 deaths or >=2 blocks.
- probe: FORM F (target's exact forward-interleaved instruction shape) dumped via cc1 -da: .lreg shows the mask pseudo (reg76) 'used 2 times', single-death, and '1 basic blocks'. The mask has exactly ONE consumer (the single sw to D_800A3464) in one straight-line block.
- result: Mask reg_n_deaths==1 and reg_basic_block>=0 are structural invariants: a 2nd death needs a 2nd store (+1 insn), a 2nd block needs a branch (+1 insn) — both break target's 43-insn count. Mask can never be routed to global at 43 insns.
- verdict: KILLED

## [s7] FRONTIER-1: a cheat-free C shape can give the reused load-web reg_n_deaths==1 (LOCAL, higher priority than the mask so it grabs $v0) while preserving target's interleaved lw/sw scheduling.
- mechanism: A single-death load-web would be local and, ordered before the mask by qty priority, would take $v0, evicting the mask to $v1 (target). But single-death requires no pseudo reuse; the interleave is preserved only by the WAR anti-dependence that reuse creates.
- probe: FORM G (3 separate single-use temps t0/t1/t2) dumped via cc1 -da: .greg shows '0 regs to allocate' (all local); dispositions put the three loads in reg3/reg4/reg5 (three DISTINCT regs, no reused-$v0 web); the emitted .s HOISTS all three loads to the top (interleave destroyed). GCC disambiguates the fixed-symbol D_800F114x stores from the arg0[] pointer loads, so no memory anti-dep serializes them.
- result: Single-death (local) and interleave-preserving are mutually exclusive: the only interleave-preserving anti-dep is pseudo reuse, which is exactly what makes the web multi-death (global). No pure-C serialization exists (memory disambiguation defeats the alternative).
- verdict: KILLED

## [s7] NEW AXIS: expressing the 3 stores as an aggregate/struct copy (different pseudo topology) yields a single-death local load-web that wins $v0 and interleaves with the mask.
- mechanism: A block move might materialize a single reused temp routed differently than the named-t idiom.
- probe: FORM H (*(struct S3*)&D_800F1140 = *(struct S3*)arg0) dumped + emitted: the block move introduces an extra 'la $5,D_800F1140' base-address insn (+1 vs target's gp-relative %lo stores), hoists the loads into 3 distinct regs ($2/$3/$4), and does not interleave with the mask.
- result: Aggregate copy is +1 insn (build 44 > target 43) and structurally non-interleaved; strictly worse and not target's shape.
- verdict: KILLED

## [s8] The suggested-register path in find_free_reg can hand the single-death 0xFFFFEF mask quantity $v1 (freeing $v0 for the global load web = target's layout) at zero instruction cost.
- mechanism: local-alloc.c:2206-2212 -- when just_try_suggested is set, find_free_reg does IOR_COMPL_HARD_REG_SET (first_used, qty_phys_copy_sugg[qty]) (or qty_phys_sugg[qty] when there are no copy-suggestions), restricting the scan to the suggested registers BEFORE the default lowest-free-hard-reg fallback. This is the one documented mechanism that overrides the lowest-free rule s6/s7 identified as the wall, and it does not require any change to the death-count routing at local-alloc.c:472 -- so unlike every previously killed lever it costs no extra insn and preserves both the 43-insn count and the interleave.
- probe: re-ran FORM F (target's exact forward-interleaved shape) and FORM V7 (the banked floor-6 candidate) through the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1) with the current -mel CC_FLAGS and BB2_SUGG_DEBUG=1, capturing every SUGGDBG-QTY (per-quantity suggestion sets) and SUGGDBG-FFR (the two hard-reg sets find_free_reg actually scans, with the just_try_suggested flag) line. Artifacts tmp/grind/func_800611A4/s8/formF_forward_interleaved_target.suggdbg.txt and formV7_mask_atomic_first.suggdbg.txt, plus the full -da dump sets.
- result: The mask quantity carries NO suggestions of either kind in either form -- FORM F 'qty=9 reg1=76 birth=46 death=54 refs=2 ncopysugg=0 nsugg=0 copysugg= sugg=', FORM V7 'qty=9 reg1=81 ... ncopysugg=0 nsugg=0'. Its only SUGGDBG-FFR line is jts=0 (the unrestricted scan) with regs 2-25 free, so the lowest-free rule hands it reg 2 ($v0) exactly as s6/s7 predicted. Across the whole function the only suggestions that ever materialise are copysugg=4 and copysugg=5 (the $a0/$a1 argument-setup copies) and one sugg=30 -- hard reg 3 is never suggested for any quantity. Suggestions come only from combine_regs (local-alloc.c:1854-1899) on a SET copying a pseudo to/from a HARD register; under MIPS o32 the only ABI producers of $v1 are the high word of a DImode return and an explicit hard-reg copy. The mask is a li of the literal 0xFFFFEF with a single consumer, so a $v1 copy-suggestion would need a second 64-bit-returning call (>= +2 insns, breaks target's 43) or an explicit register copy (a register-asm pin = explicit cheat).
- verdict: KILLED -- the suggested-register path cannot be reached in pure C for this quantity, so it cannot override the lowest-free-reg scan. With this the local-alloc side of the v0<->v1 wall has no remaining lever at all: the death-count routing gate (line 472) was killed in s7 from both directions, and the suggestion override is killed here.

## [s8] The s6/s7 forensic pass attribution is chassis-stale because those dumps predate -mel in canonical CC_FLAGS (2026-08-04).
- mechanism: -mel flips cc1's runtime BYTES_BIG_ENDIAN (spill-slot layout, bitfield direction, lwl/lwr offsets), so an RA/scheduling conclusion drawn without it is not guaranteed to describe the shipping chassis.
- probe: re-ran FORM F with the current -mel flag set on the instrumented cc1 and compared the .greg allocation summary against the s7 record.
- result: identical -- ';; 1 regs to allocate: 75', ';; 75 conflicts: 75 2 16 29', dispositions 75 in 3 (web -> $v1) and 76 in 2 (mask -> $v0).
- verdict: KILLED (as a concern) -- -mel does not perturb this function's allocation; the s6/s7 local-alloc.c:472 + find_free_reg attribution is confirmed on the current chassis and needs no re-derivation by a future session.
## [s8b] find_free_reg's CONFLICT set (`used`) is a live lever: making a hard register occupy $v0 across the mask quantity's span evicts the mask to $v1 (= target's register for the mask) at no coercion cost.
- mechanism: local-alloc.c:2170 (`for (ins = born_index; ins < dead_index; ins++) IOR_HARD_REG_SET (used, regs_live_at[ins]);`) folds every hard register live inside the quantity's span into the set the lowest-free-reg scan skips. s6/s7/s8 attacked only the death-count routing gate (local-alloc.c:472) and the suggested-register override (local-alloc.c:2206-2212); the conflict set itself was never probed. If a hard reg occupies $v0 across [born,dead) of the mask, the scan must fall through to reg 3 = $v1.
- probe: FORM I -- FORM F's exact target-interleaved post-call shape with func_80060A68 declared to return s32 and its result consumed AFTER the mask's sw, so the o32 return register $v0 is semantically live across the mask's span. Instrumented cc1 (tools/gcc-2.7.2/cc1) with the current -mel CC_FLAGS and BB2_SUGG_DEBUG=1; read the mask quantity's SUGGDBG-FFR `used=` set and the emitted .s. Artifacts tmp/grind/func_800611A4/s8b/formI_callret_live_across_mask.{c,s,suggdbg.txt,i.lreg,i.greg}.
- result: CONFIRMED as a mechanism. The mask quantity (`qty=9 reg1=76 born=46 dead=54`) gets `used=0,1,2,26,...` -- hard reg 2 present -- and the emitted code builds the mask in $v1 (`li $3,0x00ff0000 / ori $3,$3,0xffef / sw $3,D_800A3464`). This is the first non-invented-local mechanism across eight sessions that moves the mask off $v0.
- verdict: CONFIRMED (mechanism), but see the next entry -- it cannot reach target.

## [s8b] The conflict-set lever can be aimed so that the mask is evicted from $v0 while the load web still receives $v0 (= target's exact layout).
- mechanism: target wants web -> $v0 and mask -> $v1. The conflict-set lever excludes $v0 from whatever quantity's span the hard register overlaps, so it would have to overlap the mask's span WITHOUT overlapping the web's.
- probe: FORM I's emitted allocation (which register the web actually got once the mask was evicted), cross-checked against the live ranges in target's own bytes (asm/funcs/func_800611A4.s, insns 0x8006120C-0x80061238).
- result: In FORM I the web does NOT take $v0 -- it takes $a0 (`lw $4,0($16)`), because $v0 is held by the return value whose liveness did the evicting. Structurally: in target the mask's live range (lui 0x8006121C .. sw 0x80061230) is a STRICT SUBINTERVAL of the web's (lw 0x8006120C .. sw 0x80061238). find_free_reg ORs regs_live_at over a span contained in the web's span, and global-alloc derives the web's hard-reg conflicts from the same liveness, so any hard-reg occupancy of $v0 that excludes the mask necessarily also excludes the web. The two cannot be separated by hard-reg liveness at target's instruction order, and re-ordering to break the containment is exactly the interleave-destroying change killed in s3/s7.
- verdict: KILLED.

## [s8b] A DImode (64-bit) return value -- named in s8 as the only o32 producer of $v1 -- can supply the mask quantity with a $v1 copy-suggestion or otherwise steer it to $v1.
- mechanism: the o32 ABI returns a 64-bit value in the $v0/$v1 pair, so a DImode call is the only construct that makes hard reg 3 an ABI value producer; s8 killed the suggestion path on the argument that such a call costs >= +2 insns but did not measure what it actually does.
- probe: FORM J (as FORM I but `extern long long func_80060A68_d(void);`, both halves consumed after the mask's store), same instrumented-cc1 harness. Artifacts tmp/grind/func_800611A4/s8b/formJ_dimode_callret.{c,s,suggdbg.txt}.
- result: The DImode result quantity carries `copysugg=2` (the LOW word), never 3 -- so no $v1 suggestion is produced for anything, confirming s8's conclusion by direct measurement rather than by ABI argument. Worse, hard regs 2 AND 3 both land in the mask quantity's conflict set (`qty=9 ... used=0,1,2,3,26,...`), so the mask is pushed past both target registers to $a0 (`li $4,0x00ff0000`) and the web to $a1. Strictly worse than the floor and structurally further from target than FORM I.
- verdict: KILLED.

## [s8b] Declaring the function to RETURN the last loaded value ties the multi-death load web to hard reg 2 (o32 return copy) and thereby wins it $v0, evicting the mask to $v1 = target -- at zero instruction cost, since a return of a value already in $v0 emits no move.
- mechanism: `(set (reg 2) (reg <web>))` at the epilogue is the ONE pure-C construct that creates a hard-reg copy for the web itself; combine_regs (local-alloc.c:1854-1899) turns such a SET into a copy-suggestion, and global.c's set_preference turns it into a hard-reg preference for a global allocno. If it also made hard reg 2 live inside the mask's span, the conflict-set lever above would fire in the right direction. Return-type widening is ordinary C (not a coercion family), so a win here would have been a legitimate signature question, not a cheat.
- probe: FORM K -- FORM F's exact target-interleaved shape, declared `s32 func_800611A4(...)` with `return t;` appended. Same instrumented-cc1 harness; read the mask quantity's `used=` set and the emitted .s. Artifacts tmp/grind/func_800611A4/s8b/formK_return_web_hardreg_pref.{c,s,suggdbg.txt}.
- result: The mask quantity's conflict set is UNCHANGED (`qty=9 reg1=76 born=46 dead=54 ... used=0,1,26,27,...` -- no hard reg 2), because GCC emits the return copy in the epilogue at an insn index after the mask's death index 54, so it never intersects [46,54). Local-alloc still hands the mask $v0 (`li $2,0x00ff0000`), global-alloc still forces the web to $v1 (`lw $3,0($16)`), and the return now costs an extra `move $2,$3` -- 44 insns against target's 43. The failure is one of TIMING, not preference strength: a hard-reg tie created at the function's exit is invisible to the mask's local-alloc scan, which has already finished by then.
- verdict: KILLED.

## [s8] find_free_reg's CONFLICT set (`used`) is a live lever: making a hard register occupy $v0 across the 0xFFFFEF mask quantity's span evicts the mask to $v1 (target's register for the mask) with no coercion construct at all.
- mechanism: local-alloc.c:2170 -- `for (ins = born_index; ins < dead_index; ins++) IOR_HARD_REG_SET (used, regs_live_at[ins]);` folds every hard register live anywhere inside the quantity's [born,dead) span into the set the lowest-free-hard-reg scan skips. s6/s7 attacked only the death-count routing gate (local-alloc.c:472) and s8 only the suggested-register override (local-alloc.c:2206-2212); the conflict set itself had never been probed in eight sessions.
- probe: FORM I (tmp/grind/func_800611A4/s8b/formI_callret_live_across_mask.c) = FORM F's exact target-interleaved post-call shape, with func_80060A68 declared to return s32 and its result consumed AFTER the mask's store so the o32 return register $v0 is live across the mask's span. Instrumented cc1 (tools/gcc-2.7.2/cc1) with the current -mel CC_FLAGS and BB2_SUGG_DEBUG=1; read the mask quantity's SUGGDBG-FFR `used=` set plus the emitted .s.
- result: Mask quantity `qty=9 reg1=76 born=46 dead=54 ... used=0,1,2,26,...` -- hard reg 2 IS in the conflict set -- and the emitted code builds the mask in $v1: `li $3,0x00ff0000 / ori $3,$3,0xffef / sw $3,D_800A3464`. First mechanism across nine sessions other than an invented staging local to move the mask off $v0.
- verdict: CONFIRMED

## [s8] That conflict-set lever can be aimed so the mask is evicted from $v0 while the load web still RECEIVES $v0 -- i.e. it reaches target's exact layout (web -> $v0, mask -> $v1).
- mechanism: The lever excludes $v0 from whatever quantity's span the occupying hard register overlaps, so it would have to overlap the mask's span WITHOUT overlapping the web's; global-alloc derives the web's hard-reg conflicts from the same liveness information.
- probe: Which register the web actually got in FORM I once the mask was evicted, cross-checked against the live ranges in target's own bytes (asm/funcs/func_800611A4.s, insns 0x8006120C-0x80061238).
- result: In FORM I the web does NOT inherit $v0 -- it takes $a0 (`lw $4,0($16)`), because $v0 is held by the very return value whose liveness did the evicting. Structurally decisive: in target the mask's live range (lui 0x8006121C .. sw 0x80061230) is a STRICT SUBINTERVAL of the web's (lw 0x8006120C .. sw 0x80061238), so any hard-reg occupancy of $v0 that excludes the mask necessarily also excludes the web. Breaking the containment means reordering away from target's interleave, which s3/s7 already killed.
- verdict: KILLED

## [s8] A DImode (64-bit) return value -- named by s8 as the only o32 producer of $v1 -- can supply the mask quantity with a $v1 copy-suggestion or otherwise steer it to $v1.
- mechanism: o32 returns a 64-bit value in the $v0/$v1 pair, making a DImode call the only construct in which hard reg 3 is an ABI value producer; combine_regs (local-alloc.c:1854-1899) records suggestions only from SETs copying between a pseudo and a hard register. s8 dismissed this on an insn-count argument without measuring it.
- probe: FORM J (tmp/grind/func_800611A4/s8b/formJ_dimode_callret.c) = FORM I with `extern long long func_80060A68_d(void);`, both halves consumed after the mask's store. Same instrumented-cc1 harness.
- result: The DImode result quantity carries `copysugg=2` (the LOW word), never 3 -- no $v1 suggestion is produced for anything, confirming s8's conclusion by direct measurement instead of ABI argument. Worse, hard regs 2 AND 3 both enter the mask quantity's conflict set (`qty=9 ... used=0,1,2,3,26,...`), pushing the mask past both target registers to $a0 (`li $4,0x00ff0000`) and the web to $a1.
- verdict: KILLED

## [s8] Declaring func_800611A4 to RETURN the last loaded value ties the multi-death load web itself to hard reg 2 via the o32 return copy, winning it $v0 and evicting the mask to $v1 -- at zero instruction cost, since returning a value already in $v0 emits no move. (Return-type widening is ordinary C, not a coercion family, so a win here would have been a signature question, not a cheat.)
- mechanism: `(set (reg 2) (reg <web>))` at the epilogue is the only pure-C construct creating a hard-reg copy for the web; combine_regs (local-alloc.c:1854-1899) turns such a SET into a copy-suggestion and global.c's set_preference into a hard-reg preference for a global allocno.
- probe: FORM K (tmp/grind/func_800611A4/s8b/formK_return_web_hardreg_pref.c) = FORM F's exact interleaved shape declared `s32` with `return t;` appended; read the mask quantity's `used=` set and the emitted .s under the same harness.
- result: Mask quantity's conflict set UNCHANGED (`qty=9 reg1=76 born=46 dead=54 ... used=0,1,26,27,...` -- no hard reg 2): GCC emits the return copy in the epilogue, at an insn index after the mask's death index 54, so it never intersects [46,54). Local-alloc still hands the mask $v0 (`li $2,0x00ff0000`), global-alloc still forces the web to $v1 (`lw $3,0($16)`), and the return adds a `move $2,$3` -- 44 insns vs target's 43. The failure is TIMING, not preference strength: a hard-reg tie created at function exit is invisible to the mask's local-alloc scan, which has already run.
- verdict: KILLED
