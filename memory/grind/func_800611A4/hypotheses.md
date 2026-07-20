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
