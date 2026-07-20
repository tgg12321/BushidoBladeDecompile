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
