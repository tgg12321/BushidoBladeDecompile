# Evidence bank — func_800611A4

- WIP rejected_form: {'form': '3 separate per-load temps (s32 t0,t1,t2)', 'score': 11, 'why': 'loads hoist together, breaks the interleave'}

- WIP rejected_form: {'form': '3 block-local temps { s32 tN = arg0[N]; ... }', 'score': 11, 'why': 'same hoisting; live-range split made it worse'}

- WIP rejected_form: {'form': 'split constant mask = 0xFF0000; mask |= 0xFFEF', 'score': 9, 'why': 'GCC combine re-merges to single li in v0; swap persists'}

- WIP rejected_form: {'form': 'mask declared/assigned before the call (live across)', 'score': 9, 'why': 'no allocation change'}

- WIP rejected_form: {'form': 'mask assigned after 3rd load / mask block-scoped', 'score': 9, 'why': 'no allocation change'}

- WIP rejected_form: {'form': 'pointer-deref loads (*arg0, *(arg0+1)...)', 'score': 9, 'why': 'identical codegen to array index'}

- == imported from memory/wip notes.md ==
# func_800611A4 (text1b.c) — WIP / BLOCKED on RA wall

## TL;DR
Honest pure-C floor = **9**, and all 9 diffs are a **single v0<->v1 register-name
swap** on the post-call cluster. The build is otherwise byte-identical to target
(same instructions, same order, same load/store interleave). HEAD only "matches"
via two `register asm("$2")`/`asm("$3")` pins (cheat-asm). The swap is **NOT
reachable from correct pure C** — confirmed against BOTH our GCC 2.7.2 port AND
the original PsyQ cc1psx (both put load-temp in $v1, mask in $v0; target is the
reverse). BLOCKED pending canonical-asm authorization or a family-wide RA lever.

## The exact gap
Post-call body does, for i in {0,1,2}: `t = arg0[i]; D_800F114{0,4,8} = t;`
plus `D_800A3464 = 0xFFFFEF;` interleaved.

- **Target:** load-temp `t` -> `$v0`, mask `0xFFFFEF` -> `$v1`.
- **Ours (pin-free):** load-temp `t` -> `$v1`, mask -> `$v0`.

GCC's allocno priority hands the lower register (`$v0`) to the short-live-range,
CSE'd/REG_EQUIV'd constant mask, displacing the 3x-reused (fragmented, 3-web)
load temp to `$v1`. Target wants the opposite.

## Why it's a wall (not just "untried")
- ~12 pure-C levers tried (see meta.json rejected_forms + next_hypotheses):
  temp split, block-local split, const split, mask live-across-call, ptr deref,
  reorder. All score 9 or worse. None flip the swap.
- **cc1psx calibration (decisive):** the original PsyQ compiler produces the SAME
  allocation as our port (load-temp `$3`, mask `$2`). So target's swap is not what
  EITHER compiler emits from the semantically-correct C — the original source
  must have had a structurally different form, or this is a genuine RA wall.
- Only ways to force the swap are FORBIDDEN cheats: the `$2`/`$3` register-asm
  pins (current HEAD) or a regfix `subst $2 $3`.

## Resume guidance
1. Apply `candidate.c` (pin-free) and confirm `sandbox --disable all` == 9.
   Do NOT re-derive the rejected_forms.
2. Treat as a FAMILY: func_800618B4 (already PARKED, same "register-rotation pin
   cluster"), func_80061250, func_80061658 share the identical $2/$3 pin +
   post-call mask/load shape. A real lever here likely retires all of them.
3. Promising untried avenues (meta.json next_hypotheses): permuter from the
   floor-9 base; check if mask 0xFFFFEF is legitimately a non-constant in the
   original (would change allocno priority); instrumented-cc1 PRIO dump to find
   the minimal C change that inverts the two pseudos' priorities.

## Status
Source left at HEAD (oracle green). Card BLOCKED with reason pointing here.
This needs USER canonical-asm authorization (if the $2/$3 pins are accepted as
the canonical finished form for the family) OR a future family-wide RA lever.


- [s1] Baseline: sandbox --disable all == 9, target_insns=43, build_insns=43, rules_dropped=0 on pin-free candidate.c.

- [s1] Canonical verdict: C, distance=9 (`pure-C distance 9 <= 50 -- pure-C target`). Not asm-region / asm-structural.

- [s1] Diff shape: entirely v0<->v1 rename on the post-call load-temp/mask cluster — same 43 insns in same order, per WIP notes and reconfirmed here.

- [s1] Family: func_80061250 (mask=0xFF0060), func_80061658 (mask=0x10FFFF), func_800611A4 (mask=0xFFFFEF) all share the SAME post-call tail cadence with the SAME v0/v1 layout (load-temp->$v0, mask->$v1). All three carry the same $2/$3 pin cheat at HEAD.

- [s1] Sibling asymmetry: func_800618B4 has mask=0xFF0000 (1-insn `lui` only, ori-clean); target there emits `lw $v1, 0x8($s0); lui $v0, 0xFF; sw $v0, mask; sw $v1, ...` — mask in $v0, third load-temp in $v1. RA inversion is driven by mask insn-count.

- [s1] Caller code6cac_b.c:3166 `func_800611A4(arg2, arg3);` — return not captured; func_80060A68 body is genuinely void.

- [s1] m2c reconstruction of target (tmp/grind/func_800611A4/s1/m2c.txt): stores are DIRECT `D_800F1140 = M2C_FIELD(arg0, s32 *, 0);` with NO named `t` local. Also no `s32 *v1` pointer alias — writes `D_800F116C = 0x21001A;` directly. This structural shape has NOT been probed per the rejected-forms bank (the 3-separate-named-temps and pointer-deref forms were tried but not the no-temp inlined-expression form).

- [s1] Rejected-forms bank (do NOT re-probe): 3 separate per-load temps (11); 3 block-local temps (11); split constant mask 0xFF0000|0xFFEF (9, combine re-merges); mask live-across-call (9); mask block-scoped after 3rd load (9); pointer-deref loads (9).

- [s2] s2 baseline pin-free candidate applied to src/text1b.c scored 9 (target_insns=43, build_insns=43, cheat_asm_stripped=393, rules_dropped=0).

- [s2] 13 structural forms measured this session (see tmp/grind/func_800611A4/s2/sweep_log.txt).

- [s2] The RA tiebreak between the load-temp pseudo and the 2-insn (lui+ori 0xFFFFEF) mask pseudo IS movable — Variant I (`*arg0++` walking-pointer) inverts the RA direction so load-temp lands in $v0 (matches target).

- [s2] The walking-pointer form is not a valid match candidate: it produces build_insns=44 vs target 43. The extra addiu is semantically intrinsic to `*p++` and cannot be optimized away.

- [s2] Target disasm (asm/funcs/func_800611A4.s): post-call cluster loads at 0x0/0x4/0x8($s0) — no pointer bumps. Target-shape uses ARRAY-INDEXED loads (not walking) with $v0=load-temp, $v1=mask.

- [s2] Killed levers (ruled out this session): dropping v1 alias (+1 insn), tightening *v1 liveness (perturbs pre-call), decl order of t/mask (not tiebreak), mask assignment position (combine re-merges), load-temp type width (u32 vs s32), fully-inlined stores without walking (worse), partial walking (worse), mask position variants.

- [s3] Pre-call is BYTE-IDENTICAL to target on the pin-free baseline (verified via disassembly of tmp/sandbox/func_800611A4/text1b.o). The divergence is EXCLUSIVELY post-call — the s2 frontier hypothesis "pre-call freelist sets the post-call RA state" is architecturally impossible to close via pre-call reordering because the pre-call byte sequence already matches.

- [s3] Post-call structural sweep discovered a NEW floor: mask assign+store atomically hoisted to the FIRST post-call statement (V4/V7: `D_800A3464 = 0xFFFFEF; t = arg0[0]; ... t = arg0[2]; D_800F1148 = t;` — mask local optional, folded either way) scores 6 with 43 insns. Baseline pin-free = 9. Floor lowered 9 → 6.

- [s3] V4/V7 shape produces mask=$v0 AND load-temp=$v0 (both use $v0 since mask is dead before load 1). Target produces load-temp=$v0 AND mask=$v1 (interleaved). V4/V7 has fewer register-rename diffs than baseline but structurally-different scheduling (mask hoisted vs interleaved).

- [s3] Every mask-placement variant OTHER than "atomic-first" measured this session (mask-at-end, mask-between-loads-1-and-2, mask-between-loads-2-and-3, mask-alive-across-2-loads V9/V16, mask-alive-across-all-loads V5) scores 9 — GCC schedules them all to the target's interleaved shape but allocates mask→$v0 and load-temp→$v1 (opposite of target). This is the same 9-diff wall the s1/s2 sessions documented; s3 confirms it survives every mask-position and mask-liveness variant reachable without a pin.

- [s3] u32 vs s32 mask type (V11) makes no difference (score 9). Type width axis KILLED for mask (was already killed for `t` in s2).

- [s3] Pure m2c-suggested shape (V10: no `t`, no `mask`, no v1 alias, direct writes) scores 22 with build=44. Confirms the m2c reconstruction is not the C GCC would compile back to target — the v1 alias is load-bearing for address CSE (drop = +1 insn as banked in s2), and dropping the `t` load-temp further hurts RA.

- [s3] Hoisting `D_800A3468 = (s32)v1;` above the halfword reads (V2) or moving `*v1 = 0x21001A;` early (V12) both score 19-20 — pre-call byte sequence must not change. Reconfirms pre-call is at its optimal ordering.

- [s3] Pre-call disassembly of tmp/sandbox/func_800611A4/text1b.o (V0 baseline pin-free candidate) is BYTE-IDENTICAL to asm/funcs/func_800611A4.s through the JAL and its delay slot. Divergence is 100% post-call. The s2 pre-call-freelist frontier hypothesis is therefore not just under-explored -- it is architecturally impossible to move, because pre-call has no bytes left to change.

- [s3] V7 (`D_800A3464 = 0xFFFFEF;` as first post-call statement) drops the pin-free floor from 9 to 6 on 43 build_insns == 43 target_insns. Emitted post-call: `lui $v0,0xff; ori $v0,$v0,0xffef; sw $v0,gp(D_800A3464); lw $v0,0($s0); ...; sw $v0,%lo(D_800F1148)($at)` -- both mask and load-temp reuse $v0 because mask dies before load 1.

- [s3] In interleaved-mask forms (V5/V6/V9/V16/V21/V22) GCC produces target's exact interleave shape (mask lui slots between load 2 and store 2, mask ori and store slot between load 3 and store 3) but allocates mask -> $v0 and load-temp -> $v1 -- the exact reverse of target. This is a genuine RA priority inversion, not a scheduling difference.

- [s3] The v1 alias `s32 *v1 = &D_800F116C;` remains load-bearing: dropping it (V10, or s2's drop_v1_alias.c) adds one insn because GCC cannot share the %hi/%lo lui/addiu between D_800A3468=(s32)&D_800F116C and D_800F116C=0x21001A.

- [s3] The `s32 t` load-temp local is also load-bearing at the CURRENT floor: dropping it (V8 keep-mask, V23 inline-mask+no-t) scored 11 and 10 respectively -- the named load-temp helps GCC recognize the 6-ref web that keeps priority balanced.

- [s3] Type-width of mask (u32 vs s32, V11) has zero effect on this RA tiebreak. Type-width of `t` was also zero-effect per s2 rejected_forms.

- [s4] MODALITY=permuter. Prior s4 attempts ran two V7-seed (mask-atomic-first) random campaigns (~25k iters, zero cheat-free sub-6). This session ran a NEW, structurally-different chassis: fresh permuter campaign seeded DIRECTLY from the V0 INTERLEAVED target-shape (mask alive across loads, sandbox=9, permuter base_score=50) rather than V7 — the shape whose SCHEDULING already matches target, giving random mutation a fresh clock at the shape closest to target. Workspace tmp/grind/func_800611A4/s4/perm2/. 69119 iters, 6 jobs, 30 min, harvested+stopped (7 procs killed, no orphans).

- [s4] Campaign found 3 novel sub-50 finds: output-50-1 (perm 50 = interleaved-9 shape, cheat-free but the known 9-wall), output-40-1 (perm 40, sandbox 7), output-30-1 (perm 30, sandbox 5). NO cheat-free form below the sandbox-6 floor. Floor HELD at 6 (clean V7 candidate reconfirmed score=6, 43/43, 0 rules).

- [s4] output-30-1 (sandbox 5, 43 insns) is the LOWEST-scoring find but is a CHEAT-FORM: cheat-reviewer FAIL (2026-07-22). It dual-purposes a freshly-INVENTED local `new_var2` across the pre-call arg1+2 halfword (sp[1]) AND the post-call mask (D_800A3464). Fails staged-value-reused-variable prereq #2 (inventing a new var to borrow is the named excluded case; must reuse a var the fn ALREADY uses for a real job); defeat-licm-hoist-var-reuse is loop-scoped and this fn has no loop; no /* FAKE */; no exhaustion; and it does not close the match. Banked rejected/s4_perm2_new_var2_dualpurpose_reuse.c.

- [s4] MECHANISTIC FINDING (the session's real value): output-30-1's variable-reuse is the FIRST measured form that flips ALL THREE post-call load-temps to $v0 — matching target's load side EXACTLY. Disasm (tmp/grind/func_800611A4/s4/out30_disasm.txt): residual 5 diffs = mask 0xFFFFEF displaced to $a0 (target wants $v1; 3 insns: lui/ori/sw) + pre-call arg1+2 halfword regressed to $a0 (target $v0; 2 insns). So load-temp->$v0 IS reachable, but every lever that achieves it also displaces the mask OFF $v1 (to $a0 here, to $v0 in the atomic V7). Target uniquely needs load-temp=$v0 AND mask=$v1 SIMULTANEOUSLY; no cheat-free form measured reaches both. This refines the s1-s3 wall: it is not just "mask wins $v0", it is "any C that gives the load side $v0 pushes the mask to a THIRD register, never $v1".

- [s4] output-40-1 (sandbox 7, cheat-shaped: fresh pointer alias `&arg0[1]` + staged intermediate `new_var3`) flips ONLY load 1 to $v0; strictly worse than V7. Banked rejected/s4_perm2_pointer_alias_staged.c. Self-rejected (worse-than-floor + obvious coercion).

- [s4] Floor HELD at 6: clean pin-free V7 candidate reconfirmed sandbox --disable all = 6, target_insns=43 == build_insns=43, rules_dropped=0. src/text1b.c left at the clean V7 candidate.

- [s4] Fresh interleaved-V0-seed permuter campaign (tmp/grind/func_800611A4/s4/perm2): base_score=50, 69119 iters, 6 jobs, 30 min; harvested+stopped cleanly (7 procs killed, pid not orphaned).

- [s4] output-30-1 (perm score 30, sandbox 5, 43 insns) is the lowest find but a CHEAT-FORM: cheat-reviewer FAIL. Freshly-INVENTED local new_var2 dual-purposed across pre-call arg1+2 halfword (sp[1]) and post-call mask (D_800A3464). Fails staged-value-reused-variable prereq #2 (must reuse a var the fn ALREADY uses, not invent one); defeat-licm-hoist-var-reuse is loop-scoped and this fn has no loop; no /* FAKE */; no exhaustion; and does not close the match. Banked rejected/s4_perm2_new_var2_dualpurpose_reuse.c.

- [s4] Mechanistic gain: output-30-1's variable-reuse is the FIRST measured form to flip all three post-call load-temps to $v0 (target's load side exactly). Residual 5 diffs = mask on $a0 (target $v1) + pre-call halfword on $a0 (target $v0).

- [s4] output-40-1 (perm 40, sandbox 7, cheat-shaped: fresh pointer-alias &arg0[1] + staged new_var3) flips only load 1 to $v0; strictly worse than the V7 floor. Banked rejected/s4_perm2_pointer_alias_staged.c.

- [s4] Cumulative permuter coverage on func_800611A4 now spans two seed chassis (V7 atomic-first ~25k iters across prior s4 attempts + interleaved-V0 69k iters this session): zero cheat-free finds below sandbox 6 in either basin.

- [s5] MODALITY=permuter. DIRECTED chassis distinct from both s4 basins: PERM_RANDOMIZE + PERM_LINESWAP wrapping ONLY the post-call cluster (4 brace-blocks: 3 load/store pairs + mask store), pre-call region FROZEN outside the randomizer region (s4's whole-function random wasted iters mutating the byte-identical pre-call). Workspace tmp/grind/func_800611A4/s5/permA, base_score=50, 6 jobs, ~34k iters / ~20 min, harvest --stop (7 procs killed, no orphans).

- [s5] 5 novel finds, ALL coercion-gated (output-45-1, 30-1, 30-2, 45-2, 40-1): invented staging locals (new_var2/new_var3), invented pointer aliases (&arg0[1]), dead self-assigns (t=t;), dead branches (if(1){}). ZERO cheat-free form below the interleaved-50 plateau. A full ~9-min window (545s) ran with zero novel finds -> fresh-seed discipline satisfied. Directed-lineswap-frozen-precall basin KILLED for cheat-free sub-6, consistent with both s4 basins.

- [s5] MAJOR MECHANISTIC ADVANCE: output-30-1 honest sandbox = 2 (43/43) — the CLOSEST-EVER form (s4 best was 5; floor is 6). It reproduces target's EXACT three-way register layout: load-temp->$v0 (all three) AND mask 0xFFFFEF->$v1 built INTERLEAVED (lui after load2, ori after load3, sw) — the layout s1-s4 declared unreachable. Disasm tmp/grind/func_800611A4/s5/out30_s5_disasm.txt.

- [s5] The residual 2 diffs are PURELY load-offset ORDER: the sandbox-2 form emits loads 0x8,0x4,0x0($s0); target emits 0x0,0x4,0x8. Register assignment is otherwise byte-identical to target.

- [s5] The RA flip is caused by the invented `new_var2` staging local, NOT by the load reordering: the cheat-FREE reverse-order form (plain `t`, no new_var2, loads 2/1/0) scores 9 (the wall). Staging IS the lever; reordering alone does nothing.

- [s5] The flip is order-AND-position-specific and does NOT reconcile with target's forward order. Measured: reverse order + stage offset-0 load (last) = 2; forward order + stage offset-0 load (first) = 11; forward order + stage offset-2 load (last) = 8. Every staged form that matches target's FORWARD order fails to flip the RA; the only form that flips the RA (reverse) has the wrong order. Target's forward-order + flipped-RA is what the $3 pin forces by fiat; no staged C measured produces both simultaneously.

- [s5] new_var2 is a CHEAT (invented staging local; staged-value-reused-variable prereq #2 requires reusing an EXISTING var the fn already uses, not inventing one). func_800611A4's existing dead locals are unusable as sanctioned carriers: `new_var` is u16 (staging an s32 truncates -> real bug, not a match); `v1` is s32* (would need an int<->ptr type-pun). Banked rejected/s5_reverse_stage_lastload_sandbox2.c. Floor HELD at 6 (clean V7 candidate reconfirmed sandbox=6, 43/43, 0 rules; src left at clean candidate).

- [s5] s5 directed permuter chassis (PERM_LINESWAP+PERM_RANDOMIZE over post-call cluster only, pre-call frozen): base_score=50, ~34k iters/~20 min, 6 jobs, harvest --stop clean (7 procs killed, no orphans). 5 novel finds ALL coercion-gated; zero cheat-free sub-6.

- [s5] output-30-1 honest sandbox = 2 (43/43) is the CLOSEST-EVER form (s4 best 5, floor 6). It reproduces target's EXACT three-way register layout (load-temp->$v0 all three, mask->$v1 interleaved). The residual 2 diffs are purely load-offset ORDER (0x8,0x4,0x0 vs target 0x0,0x4,0x8). Disasm: tmp/grind/func_800611A4/s5/out30_s5_disasm.txt.

- [s5] The RA flip is caused by the invented new_var2 staging local, NOT the load reordering: cheat-free reverse-order form (plain t) scores 9 (the wall).

- [s5] The flip is order-and-position-specific and does not reconcile with target's forward order: reverse+stage-offset0-last=2, forward+stage-offset0-first=11, forward+stage-offset2-last=8. Target's forward-order + flipped-RA is what the $3 pin forces by fiat.

- [s5] new_var2 is a cheat (invented staging local; staged-value-reused-variable prereq #2 requires reusing an EXISTING var). Existing dead locals unusable as sanctioned carriers: new_var is u16 (truncates s32); v1 is s32* (int/ptr pun). Banked rejected/s5_reverse_stage_lastload_sandbox2.c.

- [s5] Floor HELD at 6: clean V7 candidate reconfirmed sandbox --disable all = 6, 43/43, 0 rules; src/text1b.c left at the clean pin-free candidate.
