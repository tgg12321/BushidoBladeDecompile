# Hypothesis ledger — func_8003DE14

## KILLED (s1, 2026-09-10)

### K1 — the 119 migration-pin floor is the real floor
**Statement:** the honest floor of the banked pre-migration body is 119 on the current
chassis.
**Probe:** deleted the five `register ... asm("sN")` pins and the
`__asm__ volatile("" : "=r"(count) : "0"(count))` barrier from
`retired-chassis-2026-08/body.c`, changed nothing else, ran
`sandbox func_8003DE14 --disable all`.
**Result:** **66**, with `target_insns == build_insns == 173`. KILLED (instance).
`migration_pin.json` is stale and must not be quoted as the floor again.

### K2 — the target's inner-loop `mult r*factor` / `g*factor` / `b*factor` need an
anti-LICM construct
**Statement:** GCC hoists the three invariant `channel * factor` multiplies out of the
per-pixel loop, so a lever is needed to keep them inline like the target does.
**Probe:** `.loop` dump of the s1 best form.
**Result:** loop.c already reports all three (insn 226 / 235 / 244) as
`life 1, savings 1 not desirable` and leaves them inline, matching the target.
KILLED (instance) — there is nothing to fix here; do not spend a lever on it.

### K3 — the three 2-nop mult/mflo pads in the target are a missing pipeline pass
**Statement:** `func_8003DE14` needs an entry in `multu_pad_funcs.txt` because the
target has 179 instructions (with pads) and the build has 173.
**Probe:** `objdump -dz` (elision disabled) on both the sandbox `.o` and the reference
`build/src/code6cac_c2.o`; inspected `multu_pad_funcs.txt`.
**Result:** the pads are present in BOTH objects (GNU `as` reorder-mode inserts the
mult/mflo hazard nops); the 179-vs-173 gap was `objdump`'s `...` elision of identical
zero words. `multu_pad_funcs.txt` has zero active entries project-wide. KILLED
(instance) — no pipeline scope grant is needed for this function.

### K4 — `src` / `dst` cursor register assignment follows C declaration order
**Statement:** declaring `dst` before `src` will flip the build's `a2`=src / `a3`=dst
to the target's `a3`=src / `a2`=dst.
**Probe:** v5 = v2 with the two pointer declarations swapped.
**Result:** 57, byte-identical residual to v2. KILLED (instance) — declaration order
is not the discriminator for this pair on this chassis.

### K5 — a fresh multi-set scratch (`tmp` = div numerator, then `count - 1`) is the
better carrier for the un-hoisted comparand
**Statement:** routing `(i+1)<<12` and then `count - 1` through one fresh local `tmp`
(the register the target reuses is `$v0`, which holds the div numerator first) beats
borrowing the existing `total`.
**Probe:** v3 vs v2, both measured with `sandbox --disable all`.
**Result:** v3 = 58, v2 = 57. KILLED (instance) — the fresh-scratch spelling is
strictly worse AND would need the harder family justification, so v2's borrow of the
existing `total` is the form to carry forward.

### K6 — naming the `((g & 0xF8) << 2) | (s32)-0x8000` sub-expression restores the
target's OR association
**Statement:** introducing `s32 gc = ((g & 0xF8) << 2) | (s32)-0x8000;` and then
`target_color = ((u32)r >> 3) | gc;` reproduces the target's
`or v0,v0,v1` / `or a0,a0,v0` grouping.
**Probe:** v4 (= v2 + the named intermediate).
**Result:** 62, i.e. FIVE worse than v2 (57). KILLED (instance) — the named
intermediate makes the allocation worse, not better; the association problem needs a
different spelling.

## OPEN FRONTIER (for s2+)

### F1 — the `target_color` OR association is the one extra instruction
Build emits `v1 = (r>>3) | (s32)-0x8000` FIRST (constant folded leftwards by
`fold`/`combine`) and then needs a trailing `move s6,a0`; that `move` is exactly the
`build_insns 174` vs `target_insns 173` delta. The target instead ORs the constant
into the GREEN term (`v0 = (g&0xF8)<<2; v0 |= -0x8000; a0 = (r>>3) | v0;
s6 = a0 | ((b&0xF8)<<7)`).
**Mechanism:** GCC 2.7.2 `fold-const.c` reassociation of a constant operand through
associative `|` chains, then `combine`/`jump2` failing to coalesce the final pseudo
into `s6`.
**Next probe:** spell the whole thing as one expression with the constant physically
inside the green term and with NO `|=` accumulation, e.g.
`target_color = ((u32)r >> 3) | (((g & 0xF8) << 2) | (s32)-0x8000) | ((b & 0xF8) << 7);`
and also the variant that computes blue first. Read `.combine` for func_8003DE14 to
see which OR pseudo survives and whether the trailing copy is a combine or a greg
artifact. Cheap: pure ordinary C, no family question.

### F2 — the un-hoisted `count - 1` comparand needs the RIGHT carrier register
v2 (borrow `total`) lands `count - 1` in `t6` and keeps it live across the inner loop;
the target recomputes it into `$v0` every iteration (preheader copy at 8003DF34 +
delay-slot copy at 8003E048, the reorg.c peel). So v2 fixed the *outer*-loop hoist
(66 -> 57) but has NOT reproduced the per-iteration recomputation.
**Mechanism:** `loop.c:1631`. The dump shows `threshold <= 56` here with
`insn_count = 57`, so a `life 1, savings 1` invariant inside this inner loop is
already left inline. That means the correct C form probably does NOT need any
borrow at all — it needs the comparand's pseudo to have life 1 and savings 1 and to
sit at the loop TOP, and v1's 66 must have hoisted for a different reason
(`already_moved` / `m->forces`, the other clause of the same `if`).
**Next probe:** re-dump with v1 applied (`if (i == count - 1)` inline) and read the
`.loop` diagnostics for the pseudo holding `count - 1` — is it reported at all, and
if so under which clause? That one read decides whether this function needs ANY
sanctioned-family construct or is plain ordinary C. Do this BEFORE proposing any
`/* FAKE */` form.

### F3 — the r/g/b callee-save rotation
Target `s5`=r, `s4`=g, `s3`=b, `s6`=target_color; build is rotated by one
(`s6`/`s5`/`s4` + `s3`). Likely a downstream cascade of F1/F2 rather than an
independent lever — re-measure only after F1 and F2 land.
**Next probe:** `tools/ra_solver` / `inverse_compose.py classify` on the `.greg`
allocation for the four pseudos, to get a REACHABLE / FORECLOSED verdict instead of
guessing at C spellings.

## Standing constraint on the current best form

`candidate.c` (score 57) contains `total = count - 1;` — a BORROW of an existing local
that is dead after its guard test. That is the variable-reuse family
(`.claude/rules/defeat-licm-hoist-var-reuse.md`, gated by
`.claude/rules/staged-value-reused-variable.md`) and needs a `/* FAKE */` annotation,
a quoted scope sentence, a precedent citation and a demonstrated lever-exhaustion
ladder before it can ever ship. F2 exists precisely to find out whether it is needed
at all. Do not submit `candidate.c` as-is.

## [s1] The 119 floor recorded in migration_pin.json is the honest floor of the banked pre-migration body on the current chassis.
- mechanism: The banked body carries five `register ... asm("sN")` pins and one `__asm__ volatile("" : "=r"(count) : "0"(count))` barrier; the sandbox strips cheat-asm, so the recorded floor was inflated by constructs that never reached the scored object (see the unannotated-fake-inflates-honest-floor memory).
- probe: Deleted the five register pins and the asm barrier from retired-chassis-2026-08/body.c, changed nothing else, applied to src/code6cac_c2.c, ran `sandbox func_8003DE14 --disable all`.
- result: Score 66, with target_insns == build_insns == 173. The frame layout (0x858; src_buf +0x10, dst_buf +0x410, color_info +0x810), the control-flow skeleton and the call sequence are already exactly right. Further pure-C work this session reached 57.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), src/code6cac_c2.c, no FAKE constructs present in the measured body

## [s1] GCC hoists the three loop-invariant `channel * factor` multiplies out of the per-pixel loop, so an anti-LICM lever is needed to keep them inline the way the target does.
- mechanism: loop.c move_movables would treat `r*factor` / `g*factor` / `b*factor` as single-set non-trapping loop invariants and hoist them to the preheader.
- probe: pwsh tools/grinder/dump.ps1 func_8003DE14; read tmp/grind/func_8003DE14/dumps/code6cac_c2.loop diagnostics for `;; Function func_8003DE14`, and identified insns 226/235/244 in the .cse dump.
- result: The .loop dump reports `Loop from 145 to 303: 57 real insns` and `Insn 226/235/244: regno 131/135/139 (life 1), savings 1 not desirable` — all three already stay inline, matching the target's `mult $s5,$t3` / `mult $s4,$t3` / `mult $s3,$t3` at 8003DFA0/DFC0/DFE0. Calibration: since a life-1 savings-1 movable is 'not desirable' at insn_count 57, `threshold <= 56` on this chassis for this call-free loop (loop.c:532 formula under -msoft-float).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, cc1 -da dumps of src/code6cac_c2.c with the s1 best form applied; no FAKE constructs in the dumped body

## [s1] func_8003DE14 needs a multu_pad_funcs.txt entry because asm/funcs/func_8003DE14.s shows 179 instructions (three 2-nop mult/mflo pads) while the build shows 173.
- mechanism: tools/multu_pad.py restores the ASPSX 2-instruction gap between mult/multu and its consuming mflo/mfhi, but only for functions listed in multu_pad_funcs.txt.
- probe: objdump -dz (zero-run elision disabled) on both tmp/sandbox/func_8003DE14/code6cac_c2.o and the reference build/src/code6cac_c2.o; inspected multu_pad_funcs.txt.
- result: The pads are present in BOTH objects — GNU as in reorder mode already inserts the mult/mflo hazard nops. The 179-vs-173 discrepancy was objdump collapsing identical zero words to `...` in the default listing. multu_pad_funcs.txt contains zero active entries project-wide. No pipeline scope grant is needed for this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, sandbox object vs build/src/code6cac_c2.o, no FAKE constructs

## [s1] Declaring `dst` before `src` flips the build's a2=src / a3=dst cursor registers to the target's a3=src / a2=dst.
- mechanism: GCC 2.7.2 local-alloc assigns hard registers to pseudos in order of declaration/first-set within a block, so declaration order can decide which of two same-class pseudos gets the lower hard register.
- probe: v5 = best form with the two `u16 *` cursor declarations swapped; `sandbox func_8003DE14 --disable all`.
- result: Score 57 — identical to the unswapped form, and the same a2/a3 assignment in the diff. Declaration order is not the discriminator for this pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, form v5 (tmp/grind/func_8003DE14/s1/body_v5.c), no FAKE constructs

## [s1] A fresh multi-set scratch local carrying the div numerator `(i+1)<<12` and then `count - 1` is a better carrier than borrowing the existing `total` local.
- mechanism: Per defeat-licm-hoist-var-reuse, a pseudo with n_times_set > 1 fails the loop.c:705 movable test; the target's `$v0` holds the div numerator before it holds `count - 1`, so the fresh scratch was the register-faithful spelling.
- probe: v3 (fresh `tmp`, numerator then count-1) vs v2 (borrow existing `total`); both measured with `sandbox func_8003DE14 --disable all`.
- result: v3 = 58, v2 = 57. The fresh scratch is worse and would also need the harder family justification (inventing a local to borrow is explicitly outside the variable-reuse family), so v2 is the form carried forward.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, forms v2/v3 in tmp/grind/func_8003DE14/s1/, both un-annotated (no /* FAKE */ present at measurement time)

## [s1] Introducing a named intermediate `s32 gc = ((g & 0xF8) << 2) | (s32)-0x8000;` restores the target's OR association for target_color.
- mechanism: Naming the sub-expression was expected to block fold-const.c from reassociating the -0x8000 constant leftwards into the red term, reproducing the target's `or v0,v0,v1` / `or a0,a0,v0` grouping.
- probe: v4 = best form + the named intermediate; `sandbox func_8003DE14 --disable all`.
- result: Score 62, five WORSE than the 57 baseline. The named intermediate perturbs the callee-save allocation of r/g/b without fixing the association. The extra `move s6,a0` remains.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, form v4 (tmp/grind/func_8003DE14/s1/body_v4.c), no FAKE constructs


## KILLED / CONFIRMED (s2, 2026-09-10, structural)

### C1 (CONFIRMED) - the `count - 1` comparand needs NO sanctioned-family construct; it stays inline as ordinary C once another invariant spends the first move
**Statement:** s1's F2 asked whether the function needs any FAKE construct for the
comparand. It does not. Declaring `s32 complement = blend_base - factor;` inside the
inner do-body (instead of in the `if (total > 0)` prologue) makes it the first movable
in the inner loop's movables list; move_movables moves it (it is still loop-invariant
and lands in the inner preheader, exactly where the target has `subu $t5,$fp,$t3` at
8003DF30), then executes `threshold -= 3` at `loop.c:1904`. The next movable - insn
147, the `count - 1` comparand, life 1 savings 1 - then fails
`threshold * savings * lifetime >= insn_count` (55 >= 59 is false) and is reported
"not desirable", so it stays inline at the top of the loop body and is no longer
hoisted out of the outer loop either.
**Probe:** `pwsh tools/grinder/dump.ps1 func_8003DE14` with v1 applied and again with
x1 applied; read the `;; Function func_8003DE14` section of
`tmp/grind/func_8003DE14/dumps/code6cac_c2.loop` both times.
**Result:** v1 - `Loop from 142 to 302: 58 real insns` /
`Insn 147: regno 115 (life 1), savings 1  moved to 382` then
`Insn 382: regno 115 (life 78), savings 1 halved since already moved  moved to 383`.
x1 - `Loop from 139 to 303: 59 real insns` /
`Insn 144: regno 114 (life 50), savings 1  moved to 382` /
`Insn 147: regno 115 (life 1), savings 1 not desirable`. Score 59 and
`build_insns == target_insns == 173` (the borrow form was 174). CONFIRMED.

### C2 (CONFIRMED) - the r/g/b callee-save rotation is a downstream cascade (s1 F3)
**Statement:** s1 predicted the `s6/s5/s4` vs `s5/s4/s3` rotation would fall out once
the OR chain and the comparand were fixed, and was not an independent lever.
**Probe:** side-by-side of the y1 object against `build/src/code6cac_c2.o`.
**Result:** r/g/b now sit in the target's `s5/s4/s3` and `target_color` in `s6`; the
four pseudos no longer appear in the diff at all. No spelling was aimed at them.
CONFIRMED.

### K7 - re-associating the target_color OR chain at the C level reproduces the target's `or v0,v0,v1` / `or a0,a0,v0` grouping
**Statement:** writing the `-0x8000` constant physically inside the green term, with
or without `|=` accumulation, makes GCC OR the constant into the green term the way the
target does (s1 frontier F1's next probe, verbatim).
**Probe:** seven spellings measured - w1/w2/w3/w4/w5 on the s1 borrow chassis and
y1/y3/y4/y7/z4/z5/q1/q2 on the s2 structural chassis (full table in evidence.md).
**Result:** in every one the build still emits `li v0,-32768` followed by
`or <red>,<red>,v0` as the FIRST or of the chain - the parentheses around the green
group are not honoured, fold-const.c reassociates the constant operand outwards.
Scores: 59/66/63/64/59 on the borrow chassis (all worse than its 57) and
52/58/58/60/53/72/55/68 on the structural chassis (the flat single expression, 52, is
the best of them but still has the wrong grouping). KILLED (instance): C-level
regrouping of this OR chain is not the discriminator. The next probe must be a
`.combine`/`.cse` read to find where the constant is absorbed, not another spelling.

### K8 - declaration order fixes the `src`/`dst` and `j`/`complement` register pairs
**Statement:** swapping the declaration order of the two cursor pointers, or moving
`j` to the head of the outer-loop block, flips the build's `a2`=src/`a3`=dst and
`t5`=j/`t4`=complement to the target's `a3`=src/`a2`=dst and `t4`=j/`t5`=complement.
**Probe:** z1 (`dst` before `src`) and z2 (`j` declared first), both on the y1 form.
**Result:** both score 52, byte-identical residual to y1. KILLED (instance) - third
independent confirmation of K4 on a third chassis. Stop trying declaration order on
this function; route the pair to `tools/ra_solver`.

## OPEN FRONTIER (for s3+)

### G1 - the OR-chain constant absorption is a fold/combine question, not a spelling
fold-const.c pulls `(s32)-0x8000` out of the parenthesised green group and merges it
with whichever term the chain starts with. Seven spellings are banked as dead (K7).
**Next probe:** with y1 applied, read
`tmp/grind/func_8003DE14/dumps/code6cac_c2.combine` and `.cse` for the
`;; Function func_8003DE14` section and find the insn that first materialises
`(ior (reg) (const_int -32768))` - is the grouping already wrong in `.rtl` (the tree
came out of fold reassociated, so a fix must change the TREE shape - e.g. make the red
term not be the chain's first operand at tree level) or does `.combine` re-sink it?
That decides whether any tree-level spelling is left at all.

### G2 - one allocation decision, two symptoms (`a2`/`a3` and `t4`/`t5`)
In both pairs the build hands the lower-numbered caller-save to the wrong member.
Declaration order is measured inert (K8), so this is an allocno-ordering question.
**Next probe:** `tools/ra_solver/inverse_compose.py classify` on the `.greg`/`.lreg`
allocation for the src/dst and j/complement pseudos, for a typed REACHABLE /
FORECLOSED verdict with ranked C-lever vectors.

### G3 - blend-arm ordering residual
The `andi 0x7C00` sits after the final `or` chain in the build and before it in the
target, and the blue-channel `mflo`/`srl` interleave differs by one slot. Same
instruction count. **Next probe:** `tools/sched_solver` on the blend arm's basic block,
or a statement re-association of
`*dst = (pixel & 0x8000) | r_ch | g_ch | (b_shift & 0x7C00);` that computes the masked
blue term into its own named local before the final OR.

## Standing constraint on the current best form - WITHDRAWN (s2)

s1 recorded that `candidate.c` could not ship because of the `total = count - 1;`
borrow. That borrow is GONE (see C1) and the replacement is strictly better (52 vs 57,
and 173 vs 174 instructions). `candidate.c` is now ordinary C end to end: no borrowed
local, no dead store, no named intermediate, no FAKE annotation, no sanctioned-family
claim. There is no open family question on this function.

## [s2] The `count - 1` comparand needs no sanctioned-family construct at all: declaring the loop-invariant `complement` INSIDE the inner do-body makes it the first movable, move_movables spends the first move on it and applies `threshold -= 3` (loop.c:1904), and the comparand then fails the arithmetic clause and stays inline the way the target has it.
- mechanism: loop.c:532 `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` gives 58 for this call-free inner loop (bounded 58 <= threshold <= 59 by the dumps themselves). loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count` is TRUE for the FIRST life-1/savings-1 movable in a 58-insn loop and FALSE for every later one because each successful move runs `threshold -= 3` at loop.c:1904. Moving `complement`'s declaration into the loop inserts a movable ahead of the comparand in the movables list (and raises insn_count to 59), so the comparand is reported 'not desirable' and is left inline; it is consequently no longer a movable for the outer loop either. `complement` itself is hoisted straight back to the inner-loop preheader, which is exactly where the target keeps it (`subu $t5,$fp,$t3` at 8003DF30).
- probe: `pwsh tools/grinder/dump.ps1 func_8003DE14` with v1 applied and again with x1 applied; read the `;; Function func_8003DE14` section of tmp/grind/func_8003DE14/dumps/code6cac_c2.loop both times; then `sandbox func_8003DE14 --disable all` on x1 and on y1.
- result: v1 dump: `Loop from 142 to 302: 58 real insns` / `Insn 147: regno 115 (life 1), savings 1  moved to 382`, then in the outer loop `Insn 382: regno 115 (life 78), savings 1 halved since already moved  moved to 383` (reg 115 identified from the .jump dump as `(set (reg:SI 115) (plus:SI (reg/v:SI 73) (const_int -1)))`, i.e. count - 1). x1 dump: `Loop from 139 to 303: 59 real insns` / `Insn 144: regno 114 (life 50), savings 1  moved to 382` / `Insn 147: regno 115 (life 1), savings 1 not desirable`. x1 scores 59 with build_insns == target_insns == 173; s1's borrow form scored 57 but with build_insns 174. Adding the flat single-expression target_color (y1) takes it to 52, still 173/173. candidate.c is now ordinary C end to end - no borrowed local, no dead store, no named intermediate, no FAKE annotation, no family claim.
- verdict: CONFIRMED

## [s2] Re-associating the target_color OR chain at the C level - writing the `(s32)-0x8000` constant physically inside the green term, with or without `|=` accumulation - reproduces the target's `or v0,v0,v1` / `or a0,a0,v0` grouping.
- mechanism: fold-const.c reassociates a constant operand outwards through an associative `|` chain, so the parentheses around `(((g & 0xF8) << 2) | (s32)-0x8000)` are not honoured and the constant is merged with whichever term heads the chain.
- probe: Seven distinct spellings measured with `sandbox func_8003DE14 --disable all`: w1/w2/w3/w4/w5 on the s1 borrow chassis (base 57) and y1/y3/y4/y7/z4/z5/q1/q2 on the s2 structural chassis. Objects side-by-side-diffed against build/src/code6cac_c2.o with tmp/grind/func_8003DE14/s1/sd2.py.
- result: Every spelling still emits `li v0,-32768` followed by `or <red>,<red>,v0` as the first OR of the chain; none reproduced the target's green-term grouping. Borrow chassis: 59/66/63/64/59 (all worse than its 57). Structural chassis: y1 52, z4 53, q1 55, y3 58, y4 58, y7 60, q2 68, z5 72. The flat single expression (y1, 52) is the best of them and is the new candidate, but its grouping is still wrong, so the +0 instruction-count win came from elsewhere. This is now a fold/combine question for a dump read, not a spelling question.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), src/code6cac_c2.c, bodies in tmp/grind/func_8003DE14/s2/body_{w1,w2,w3,w4,w5,y1,y3,y4,y7,z4,z5,q1,q2}.c, no FAKE constructs present in any measured body

## [s2] Declaration order fixes the two swapped register pairs: swapping the `src`/`dst` cursor declarations, or moving `j` to the head of the outer-loop block, flips the build's a2=src/a3=dst and t5=j/t4=complement to the target's a3=src/a2=dst and t4=j/t5=complement.
- mechanism: GCC 2.7.2 local-alloc walks pseudos in first-set order within a block, so declaration order can decide which of two same-class pseudos receives the lower hard register.
- probe: z1 (`dst` declared before `src`) and z2 (`j` declared first among the outer-body locals), both built on the y1 form; `sandbox func_8003DE14 --disable all` plus the side-by-side diff.
- result: Both score 52, the same as y1, with a byte-identical residual - a2/a3 and t4/t5 are still swapped. This is the third independent confirmation of s1's K4 on a third chassis. Declaration order is not the discriminator for these pairs; the next attack is tools/ra_solver/inverse_compose.py classify on the .greg allocation, not more spellings.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, forms tmp/grind/func_8003DE14/s2/body_z1.c and body_z2.c on the y1 chassis, no FAKE constructs

## [s2] The r/g/b callee-save rotation (target s5/s4/s3 + s6=target_color, build s6/s5/s4 + s3) is an independent lever needing its own spelling.
- mechanism: s1's F3 predicted instead that it was a downstream cascade of the OR-chain shape and the comparand's live range, via global.c allocno ordering.
- probe: Side-by-side diff of the y1 object against build/src/code6cac_c2.o, with no spelling aimed at r/g/b at all.
- result: r/g/b now sit in the target's s5/s4/s3 and target_color in s6; all four pseudos dropped out of the diff entirely once the comparand stayed inline and target_color became a single flat expression. s1's F3 is CONFIRMED as a cascade and this hypothesis is dead - do not spend a session on the rotation.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, form tmp/grind/func_8003DE14/s2/body_y1.c (score 52), no FAKE constructs

## s3 (2026-09-10, structural)

### H-s3-1 (CONFIRMED) — the OR-chain constant is aimed by writing it next to the OTHER term

- statement: In `target_color = red | green | blue | (s32)-0x8000`, the term the
  constant is written next to in C is the term it is NOT emitted on; writing
  `((red) | (s32)-0x8000) | green | blue` therefore produces the target's
  `red | (green | -0x8000)` tree and its exact instruction sequence.
- mechanism: `fold` reaches `associate:` (fold-const.c:3685) for BIT_IOR_EXPR
  and calls `split_tree` (fold-const.c:882); when arg0 is `(VAR|CON)` it
  rebuilds `VAR | fold(ARG1|CON)` (fold-const.c:3729), and when arg1 is
  `(VAR|CON)` it rebuilds `fold(ARG0|CON) | VAR` (fold-const.c:3778). Either
  way the constant crosses to the other operand, in the TREE, before RTL
  expansion — which is why the s2 `.rtl` dump already showed the constant on
  the red term and why parentheses appeared to be ignored.
- probe: `.rtl` dump on the s2 y1 chassis (insns 84-93, constant on red), then
  the a1 spelling + `.rtl` re-dump (insns 84-93, constant on green) + the
  emitted `.s` block vs 8003DEA0-8003DED0.
- result: CONFIRMED. The emitted color block is byte-exact with a1, including
  `li $3,-32768` before the three `lbu`s. Note the whole-function score got
  WORSE (52 -> 60) because the same edit flipped the i/count allocation; the
  item is nonetheless closed and every later form keeps this spelling.
- verdict: CONFIRMED

### H-s3-2 (CONFIRMED) — `i = 0;` placement decides the $s1/$s2 assignment

- statement: Moving the `i = 0;` statement later in the prologue shortens `i`'s
  live range enough to raise its allocno priority above `count`'s, giving `i`
  $s1 and `count` $s2 as in the target.
- mechanism: global.c allocno priority is
  `floor_log2(refs)*refs*10000/live_length`; measured on the a1 chassis
  `count` = 3277 (13 refs / 119) and `i` = 3243 (12 refs / 111), a 1% margin.
  `tools/ra_solver/inverse.py global` returned REACHABLE at one atom with
  `live_shrink` on `i` of only 2 LUIDs sufficient.
- probe: four placements of `i = 0;` (before `if (count > 0)`, before
  `r = color_info[0];`, before `saved_y = rect[1];`, before
  `func_80052BE4(color_info)`), each measured with `sandbox --disable all`.
- result: all four score 43 (from 52/60); the $s1/$s2 pair and the prologue
  `move sN,zero` slot both leave the diff. Bodies b1-b4.
- verdict: CONFIRMED

### H-s3-3 (CONFIRMED) — one arm-local `*dst++` swaps the src/dst cursor registers

- statement: Writing the `dst` increment inside the `i == count - 1`
  zero-pixel arm (instead of routing that arm through the shared
  `advance_dst: dst++;` tail) lifts `dst`'s reg_n_refs enough to overtake
  `src` in allocno priority, so `src` lands in $a3 and `dst` in $a2 as in the
  target.
- mechanism: same global.c priority ordering; on the 43 chassis `src` had 32
  refs / priority 27118 and `dst` 26 refs / 17931, and `inverse.py --goal
  '{"108": 7, "109": 6}'` reported one-atom REACHABLE via `refs_up` on `dst`
  by 6 (or `refs_down` on `src` by 6). Each arm that carries its own
  `*dst++` adds references to that pseudo.
- probe: five doses measured — one arm (d4, d3, d6), two arms (d5), three arms
  (d2) — plus a re-extract of the ra_solver model on the winning chassis.
- result: d4 = 31 and the model now reports 108 -> $a3, 109 -> $a2 (dispositions
  25/25). Larger doses give jump2 a second identical tail and cost 1-2
  instructions (171-172 vs 173).
- verdict: CONFIRMED

### H-s3-4 (KILLED, instance) — masking the blue channel in its own statement

- statement: Computing `b_shift = ((b_src * complement + b * factor) >> 5) &
  0x7C00;` in the assignment statement and then OR-ing the already-masked local
  reproduces the target's `andi 0x7C00`-before-the-final-`or` order.
- mechanism: the mask would become part of the `b_shift` pseudo's defining insn
  rather than an operand of the final IOR, which should move it ahead of the
  last `or` in sched.c's ready list.
- probe: measured on two chassis — a3 (on the s2 y1 base) and d1 (on the b1
  base).
- result: a3 = 53 vs base 52; d1 = 44 vs b1 43. Both one point WORSE, and the
  blend-arm interleave stayed in the diff. Not the lever for that item on
  either chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, forms tmp/grind/func_8003DE14/s3/body_a3.c and body_d1.c, no FAKE constructs

### H-s3-5 (KILLED, instance) — block-scoping `j` to shrink its live range

- statement: Declaring `s32 j = 0;` inside the `if (total > 0)` block shrinks
  `j`'s live range enough to swap the `j`/`complement` pair into the target's
  $t4/$t5.
- mechanism: `inverse.py` lists `live_shrink` on pseudo 115 (`j`) by 8 LUIDs as
  a one-atom vector for that goal, and block-scoping the declaration is the
  ordinary-C way to shorten a range.
- probe: c1 (on the b1 chassis), c2 (`src`/`dst`/`j` all scoped in), d7 (on the
  d4 chassis).
- result: c1 = 44 (vs 43), c2 = 46, d7 = 33 (vs 31). Every scoped form is worse
  and the $t4/$t5 pair stayed swapped, so the achieved shrink is smaller than
  the 8 LUIDs the model needs. The remaining one-atom vectors for that goal
  (`refs_down` on `complement` by 1, `live_extend` on `complement` by 8,
  `refs_up` on `j` by 2) are untested.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, forms tmp/grind/func_8003DE14/s3/body_c1.c, body_c2.c, body_d7.c, no FAKE constructs

### H-s3-6 (KILLED, instance) — s2's "no regrouping can move the constant" frontier

- statement: s2's frontier claim that no remaining C-level regrouping of the
  same three terms can move `-0x8000` onto the green term, so the fix would
  have to change which term heads the chain at tree level or change what the
  green term IS.
- mechanism: it rested on seven measured spellings all producing
  `li v0,-32768 / or <red>,<red>,v0` first.
- probe: read fold-const.c's `associate:` block and `split_tree`, then measured
  the one spelling family the seven had not covered (constant attached to RED).
- result: DISPROVEN — a1 moves the constant onto green with no change to what
  any term is. The seven earlier spellings all attached the constant to green
  or to blue, which is exactly the set fold maps onto red.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, form tmp/grind/func_8003DE14/s3/body_a1.c + .rtl dump, no FAKE constructs

## [s3] In target_color = red | green | blue | (s32)-0x8000, the term the constant is written next to in C is the term it is NOT emitted on, so spelling it `((red) | (s32)-0x8000) | green | blue` produces the target's `red | (green | -0x8000)` tree and its exact instruction sequence.
- mechanism: fold reaches `associate:` (fold-const.c:3685) for BIT_IOR_EXPR and calls split_tree (fold-const.c:882): when arg0 is (VAR|CON) it rebuilds VAR | fold(ARG1|CON) (fold-const.c:3729); when arg1 is (VAR|CON) it rebuilds fold(ARG0|CON) | VAR (fold-const.c:3778). Either way the constant crosses to the other operand, in the tree, before RTL expansion.
- probe: Read the .rtl dump on the s2 y1 chassis (insns 84-93: constant already on the red term), read fold-const.c's associate:/split_tree, then measured the untried spelling family (constant attached to RED) as body_a1.c and re-dumped .rtl + .s.
- result: CONFIRMED. a1's .rtl carries (ior red (ior green -32768)) and the emitted block `li $3,-32768 / lbu $21,$20,$19 / srl $4,$21,3 / andi $2,$20,0xf8 / sll $2,$2,2 / or $2,$2,$3 / or $4,$4,$2` is byte-exact vs 8003DEA0-8003DEC0. Whole-function score went 52 -> 60 because the same edit flipped an unrelated 1%-margin allocno priority; the OR item itself is closed and every later form keeps this spelling.
- verdict: CONFIRMED

## [s3] Moving the `i = 0;` statement later in the prologue shortens i's live range enough to raise its global.c allocno priority above count's, giving i $s1 and count $s2 as in the target.
- mechanism: global.c allocno priority = floor_log2(refs)*refs*10000/live_length; measured on the a1 chassis count = 3277 (13 refs / 119 LUIDs) vs i = 3243 (12 refs / 111), a 1% margin. tools/ra_solver/inverse.py global --goal '{"73": 18, "74": 17}' returned REACHABLE with 24 one-atom vectors, the cheapest a live_shrink on i of 2 LUIDs.
- probe: Four late placements of `i = 0;` (before `if (count > 0)`, before `r = color_info[0];`, before `saved_y = rect[1];`, before `func_80052BE4(color_info)`) measured with sandbox --disable all (bodies b1-b4).
- result: CONFIRMED — all four score 43 (from 52/60); the $s1/$s2 pair and the prologue `move sN,zero` slot both leave the side-by-side diff.
- verdict: CONFIRMED

## [s3] Writing the dst increment inside the `i == count - 1` zero-pixel arm, instead of routing that arm through the shared `advance_dst: dst++;` tail, lifts dst's reg_n_refs enough to overtake src in allocno priority so src lands in $a3 and dst in $a2 as in the target.
- mechanism: Same global.c priority ordering: on the 43 chassis src had 32 refs / priority 27118 and dst 26 refs / 17931, and inverse.py --goal '{"108": 7, "109": 6}' reported one-atom REACHABLE via refs_up on dst by 6 (or refs_down on src by 6). Each arm carrying its own *dst++ adds references to that pseudo.
- probe: Five doses measured — one arm (d4 = 31, d3 = 37, d6 = 33), two arms (d5 = 35), three arms (d2 = 35) — plus a re-extract of the ra_solver model on the d4 chassis.
- result: CONFIRMED — d4 scores 31 and the re-extracted model reports 108 -> $a3, 109 -> $a2 with dispositions 25/25 matching the dump. Larger doses hand jump2 a second identical tail and cost 1-2 instructions (171-172 vs target 173); d4 is 172, one short.
- verdict: CONFIRMED

## [s3] Computing the blue channel's 0x7C00 mask inside the b_shift assignment and OR-ing the already-masked local reproduces the target's andi-0x7C00-before-the-final-or emission order.
- mechanism: The mask would become part of the b_shift pseudo's defining insn rather than an operand of the final IOR, which should move it ahead of the last or in sched.c's ready list.
- probe: Measured on two chassis: a3 (on the s2 y1 base) and d1 (on the b1 base).
- result: a3 = 53 vs base 52; d1 = 44 vs b1 43. Both one point worse and the blend-arm interleave stayed in the diff, so on these two chassis this is not the lever for that item.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), forms tmp/grind/func_8003DE14/s3/body_a3.c and body_d1.c, no FAKE constructs

## [s3] Declaring `s32 j = 0;` inside the `if (total > 0)` block shrinks j's live range enough to swap the j/complement pair into the target's $t4/$t5.
- mechanism: inverse.py lists live_shrink on pseudo 115 (j) by 8 LUIDs as a one-atom vector for that goal, and block-scoping the declaration is the ordinary-C way to shorten a range.
- probe: c1 (j scoped in, on the b1 chassis), c2 (src/dst/j all scoped in), d7 (j scoped in, on the d4 chassis).
- result: c1 = 44 (vs 43), c2 = 46, d7 = 33 (vs 31); every scoped form is worse and $t4/$t5 stayed swapped, so the achieved shrink is smaller than the 8 LUIDs the model needs. The other one-atom vectors for that goal (refs_down on complement by 1, live_extend on complement by 8, refs_up on j by 2) remain untested.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, forms tmp/grind/func_8003DE14/s3/body_c1.c, body_c2.c, body_d7.c, no FAKE constructs

## [s3] s2's frontier claim that no remaining C-level regrouping of the same three terms can move -0x8000 onto the green term, so the fix would have to change which term heads the chain at tree level or change what the green term IS.
- mechanism: It rested on seven measured spellings that all produced `li v0,-32768` + `or <red>,<red>,v0` first, read as evidence that fold ignores the source grouping entirely.
- probe: Read fold-const.c's associate: block and split_tree, then measured the one spelling family the seven had not covered — the constant attached to the RED term (body_a1.c).
- result: DISPROVEN. a1 moves the constant onto green with no change to what any term is; the seven earlier spellings all attached the constant to green or to blue, which is exactly the set fold maps back onto red.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, form tmp/grind/func_8003DE14/s3/body_a1.c plus its .rtl dump, no FAKE constructs


## [s4] The 173rd instruction is lost to a jump2 cross-jump because s3's d4 form inlined `dst++` on the ZERO-PIXEL arm; the target inlines it on the TARGET-COLOUR arm and routes the zero-pixel arm through the shared `advance_dst` tail.
- mechanism: jump2's cross-jumping merges identical tails. With `*dst++ = pixel; src++; goto loop_check;` in the zero-pixel arm AND `*dst++ = target_color; src++; goto loop_check;` in the colour arm, both arms end in the identical four-insn tail `sh / addiu a2,a2,2 / j loop_check / addiu a3,a3,2`, so jump2 keeps one copy and the build emits 172 instructions. The target's own disassembly shows the asymmetric shape: 0x13c zero-pixel arm = `sh v0,0(a2) / j 0x210 (advance_dst) / addiu a3,a3,2` with NO dst increment, and 0x148 colour arm = `sh s6,0(a2) / addiu a2,a2,2 / j 0x214 (loop_check) / addiu a3,a3,2`.
- probe: read the assembled target.o at 0x12c-0x214 and identified the two tail labels, then measured form f1 = candidate.c with the zero-pixel arm changed to `*dst = pixel; src++; goto advance_dst;`.
- result: CONFIRMED. f1 measures score 43 / **build_insns 173** (target 173) — the first form in four sessions with the correct instruction count. The side-by-side over the arm block shows structural identity; every remaining difference there is which of $a2/$a3 is used. Whole-function score rose 31 -> 43 because the same edit removes the dst references that had been winning the src/dst seat. Banked as memory/grind/func_8003DE14/chassis_f1_structure_exact_43.c.
- verdict: CONFIRMED

## [s4] On the f1 chassis the src/dst register swap is a pure reference-count problem with a floor_log2 threshold at 32, and no live-length or birth-order lever reaches it.
- mechanism: global.c allocno priority = floor_log2(refs) * refs * 10000 / live_length. Re-extracted model (dispositions 25/25, sort order MATCH): pseudo 108 (src) refs 32 / livelen ~59 / priority 27118 -> $a2; pseudo 109 (dst) refs 26 / livelen ~58 / priority 17931 -> $a3. The live lengths are within one LUID; the whole gap comes from floor_log2(32)=5 vs floor_log2(26)=4.
- probe: tools/ra_solver/extract.py + simulate.py on the f1 chassis, then inverse.py global --goal '{"108": 7, "109": 6}'.
- result: CONFIRMED. Minimal solution size 1 atom, 8 distinct vectors, ALL of them reference-count moves (`refs_down 108: 32 -> <=26` or `refs_up 109: 26 -> >=32`). No live_shrink/live_extend/birth-order vector is emitted at size 1, and 50 preference atoms are reported mechanically unreachable from C.
- verdict: CONFIRMED

## [s4] Adding C-level references to `dst` in the zero-pixel arm (store-then-increment, indexed store, or the same split in the colour arm) raises reg_n_refs enough to win the $a2 seat.
- mechanism: the ra_solver refs_up vector on pseudo 109; each extra textual reference to dst was expected to be counted by flow.c.
- probe: four semantics-preserving respellings measured on the d4 chassis — e1 `*dst = pixel; dst = dst + 1;`, e2 `dst[0] = pixel; dst++;`, e4 (e1 plus the colour arm split the same way), e6 (colour arm split only) — plus e3 `dst = dst + 1; dst[-1] = pixel;` as a stronger dose. Bodies tmp/grind/func_8003DE14/s4/body_e1.c, body_e2.c, body_e3.c, body_e4.c, body_e6.c.
- result: KILLED. e1/e2/e4/e6 all measure 31 / 172 — byte-identical to candidate.c. REG_N_REFS is counted by flow.c over RTL, and the tree has already canonicalised `*p = v; p = p + 1;` back into `*p++ = v;`, so a C-level split adds no reference at all. e3 does change the address form and measures 31 / **174** — one instruction too many, allocation unmoved.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), d4 chassis, forms tmp/grind/func_8003DE14/s4/body_e1.c, body_e2.c, body_e3.c, body_e4.c, body_e6.c, no FAKE constructs

## [s4] Reducing `src`'s reference count by hoisting the read and/or the increment out of the arms (the refs_down-on-108 half of the inverse.py solution) reaches the src/dst goal on the f1 chassis.
- mechanism: inverse.py vector #1 is `refs_down pseudo 108: 32 -> 26`; collapsing the four arm-local `src++` (and/or the two duplicated `*src` reads) into one shared statement is the only ordinary-C spelling that removes that many references.
- probe: g1 (`u16 pixel = *src++;` hoisted to the top of the inner do-body, all four `src++` deleted) and g2 (only the READ hoisted, the four `src++` kept), both on the f1 chassis. Bodies tmp/grind/func_8003DE14/s4/body_g1.c and body_g2.c.
- result: KILLED. g1 = 61 / 162 and g2 = 48 / 166 against a target of 173. The four `addiu a3,a3,2` the target carries in its arm delay slots exist only because there are four separate `src++` statements for reorg.c to fill with; hoisting removes eleven (g1) or seven (g2) instructions. On this chassis the refs_down direction is paid for with exactly the structure f1 was built to recover.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (score 43 / 173 insns), forms tmp/grind/func_8003DE14/s4/body_g1.c and body_g2.c, no FAKE constructs

## [s4] decomp-permuter, seeded on the d4 chassis and again on the f1 chassis, finds a semantics-preserving form below the banked floor.
- mechanism: randomised C-level perturbation of the function body against the honest per-function object diff, run through tools/permuter_campaign.py with --stack-diffs.
- probe: two campaigns. tmp/perm_3DE14_s4a on the d4 chassis (permuter base 360), 27,747 iterations / 985 s. tmp/perm_3DE14_s4b on the f1 chassis (permuter base 300), 7,009 iterations / 447 s. Both harvested with --stop.
- result: KILLED for both seeds. s4a's best find (300) is `dst = dst++; *dst = pixel;` — undefined behaviour that under this cc1 leaves dst unchanged, so the zero-pixel arm stops advancing the destination cursor; measured 30 / 174 and rejected on semantics (rejected/e5-...). s4b's best find (290) changes nothing inside the function: it widens `extern void DrawSync(s32);` to `extern long long DrawSync(s32);` in the TU declarations. No campaign produced a semantics-preserving in-function form below the banked floor of 31.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, d4 chassis (score 31) and f1 chassis (score 43), campaigns tmp/perm_3DE14_s4a (27,747 it) and tmp/perm_3DE14_s4b (7,009 it), no FAKE constructs

## [s4] The 173rd instruction is lost to a jump2 cross-jump because s3's d4 form inlined dst++ on the ZERO-PIXEL arm; the target inlines it on the TARGET-COLOUR arm and routes the zero-pixel arm through the shared advance_dst tail.
- mechanism: jump2's cross-jumping merges identical tails. With `*dst++ = pixel; src++; goto loop_check;` in the zero-pixel arm AND `*dst++ = target_color; src++; goto loop_check;` in the colour arm, both arms end in the identical four-insn tail `sh / addiu a2,a2,2 / j loop_check / addiu a3,a3,2`, so jump2 keeps one copy and the build emits 172. The target's disassembly is asymmetric: 0x13c zero-pixel arm = `sh v0,0(a2) / j 0x210 (advance_dst) / addiu a3,a3,2` with NO dst increment; 0x148 colour arm = `sh s6,0(a2) / addiu a2,a2,2 / j 0x214 (loop_check) / addiu a3,a3,2`.
- probe: Read the assembled target.o at 0x12c-0x214 and identified the two tail labels (0x210 = advance_dst, 0x214 = loop_check), then measured form f1 = candidate.c with the zero-pixel arm changed to `*dst = pixel; src++; goto advance_dst;`.
- result: CONFIRMED. f1 measures score 43 / build_insns 173 (target 173) — the first form in four sessions with the correct instruction count, and the side-by-side over the arm block is structurally identical (every remaining line there differs only in $a2 vs $a3). The whole-function score rose 31 -> 43 because the same edit removes the dst references that had been winning the src/dst seat. Banked as memory/grind/func_8003DE14/chassis_f1_structure_exact_43.c.
- verdict: CONFIRMED

## [s4] On the f1 chassis the src/dst register swap is a pure reference-count problem with a floor_log2 threshold at 32 refs, and no live-length or birth-order lever reaches it at solution size 1.
- mechanism: global.c allocno priority = floor_log2(refs) * refs * 10000 / live_length. Re-extracted model (sort order MATCH, dispositions 25/25): pseudo 108 (src) refs 32 / livelen ~59 / priority 27118 -> $a2; pseudo 109 (dst) refs 26 / livelen ~58 / priority 17931 -> $a3. The live lengths are within one LUID; the whole gap comes from floor_log2(32)=5 vs floor_log2(26)=4.
- probe: tools/ra_solver/extract.py func_8003DE14 code6cac_c2 + simulate.py on the f1 chassis, then inverse.py global --goal '{"108": 7, "109": 6}' and --goal '{"115": 12, "116": 13}'.
- result: CONFIRMED. Minimal solution size 1 atom, 8 distinct vectors, ALL reference-count moves (refs_down 108: 32 -> <=26, or refs_up 109: 26 -> >=32). No live_shrink/live_extend/birth-order vector is emitted at size 1; 50 preference atoms are reported mechanically unreachable from C. The j/complement goal is unchanged from s3: 9 vectors, live_shrink 115 by 8 (already measured dead), refs_down 116 by 1..5, live_extend 116 by 8, refs_up 115 by 2.
- verdict: CONFIRMED

## [s4] Adding C-level references to dst in the zero-pixel arm (store-then-increment, indexed store, or the same split in the colour arm) raises reg_n_refs on the d4 chassis.
- mechanism: The ra_solver refs_up vector on pseudo 109; each extra textual reference to dst was expected to be counted by flow.c.
- probe: Four semantics-preserving respellings measured on the d4 chassis — e1 `*dst = pixel; dst = dst + 1;`, e2 `dst[0] = pixel; dst++;`, e4 (e1 plus the colour arm split the same way), e6 (colour arm split only) — plus e3 `dst = dst + 1; dst[-1] = pixel;` as a stronger dose.
- result: KILLED. e1/e2/e4/e6 all measure 31 / 172, byte-identical to candidate.c: REG_N_REFS is counted by flow.c over RTL, and the tree has already canonicalised `*p = v; p = p + 1;` back into `*p++ = v;`, so a C-level split adds no reference at all. e3 does force a distinct address form and measures 31 / 174 — one instruction too many with the allocation unmoved. This is a generally useful fact for the RA-solver workflow: a refs_up vector cannot be bought with a spelling the tree folds.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), d4 chassis (score 31/172), forms tmp/grind/func_8003DE14/s4/body_e1.c, body_e2.c, body_e3.c, body_e4.c, body_e6.c, no FAKE constructs

## [s4] Reducing src's reference count by hoisting the read and/or the increment out of the arms — the refs_down-on-108 half of the inverse.py solution — reaches the src/dst goal on the f1 chassis.
- mechanism: inverse.py vector #1 is refs_down pseudo 108: 32 -> 26; collapsing the four arm-local `src++` (and/or the two duplicated `*src` reads) into one shared statement is the only ordinary-C spelling that removes that many references.
- probe: g1 (`u16 pixel = *src++;` hoisted to the top of the inner do-body, all four arm-local `src++` deleted) and g2 (only the `*src` READ hoisted, the four `src++` kept), both measured on the f1 chassis with sandbox --disable all.
- result: KILLED. g1 = 61 / 162 and g2 = 48 / 166 against a target of 173. The four `addiu a3,a3,2` the target carries in its arm delay slots exist only because there are four separate `src++` statements for reorg.c to fill them with; hoisting removes eleven (g1) or seven (g2) instructions. On this chassis the refs_down direction is paid for with exactly the structure f1 was built to recover, so the remaining reachable half of the goal is refs_up on dst.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (score 43 / 173 insns), forms tmp/grind/func_8003DE14/s4/body_g1.c and body_g2.c, no FAKE constructs

## [s4] decomp-permuter, seeded on the d4 chassis and again on the f1 chassis, finds a semantics-preserving form below the banked floor of 31.
- mechanism: Randomised C-level perturbation of the function body scored against the honest per-function object diff, run through tools/permuter_campaign.py with --stack-diffs on a full-TU workspace that reproduces the Makefile pipeline verbatim.
- probe: Two campaigns, both harvested with --stop before the session ended. tmp/perm_3DE14_s4a on the d4 chassis (permuter base 360): 27,747 iterations / 985 s. tmp/perm_3DE14_s4b on the f1 chassis (permuter base 300): 7,009 iterations / 447 s.
- result: KILLED for both seeds. s4a's best find (permuter 300) is `dst = dst++; *dst = pixel;` — an unsequenced self-assignment that under this cc1 leaves dst unchanged, so the zero-pixel arm stops advancing the destination cursor and the function computes a different image; measured 30 / 174 and rejected on SEMANTICS, not policy. Its one useful datum (the extra dst reference is what buys the point) drove the e-series above. s4b's best find (permuter 290) changes nothing inside the function: it widens `extern void DrawSync(s32);` to `extern long long DrawSync(s32);` in the TU declarations — outside this session's surface and a prototype contradiction on a Sony library function; banked as a lead only. Neither campaign produced a semantics-preserving in-function form below 31.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, d4 chassis (score 31) and f1 chassis (score 43), campaigns tmp/perm_3DE14_s4a (27,747 it) and tmp/perm_3DE14_s4b (7,009 it), no FAKE constructs


## [s5] The remaining residual on the structurally-exact (173-insn) chassis lives in the blend block's spelling space - which sub-expressions are named locals, in what order those declarations sit, and which way round each commutative product is written.
- mechanism: The `enumerate` modality's premise (owner ruling 2026-09-08): for a residual confined to one block, the useful search space is naming x declaration order x operand order, and it is small enough to enumerate exhaustively with tools/spelling_enum.py + tools/sweep_variants.py instead of hand-probing a dozen forms per session.
- probe: Four exhaustive sweeps, every variant scored with `sandbox func_8003DE14 --disable all`. (1) enum1: the blend block on the f1 chassis, written fully named as r_src/g_src/b_src/r_ch/g_ch/b_shift/b_ch with the store as the anchor, 1,800 spellings (all inline subsets x all def-before-use orders), no swaps. (2) enum2: the outer-loop declaration block `total / src / dst / factor / j` on the enum1 winner (h1) chassis, all 120 orderings, every name forced to stay named because all five are read outside the region. (3) enum3: the same 1,800-spelling blend enumeration on the d4 chassis (the score-31 floor form). (4) enum4: the blend block on the h1 chassis WITH the commutative-operand-swap axis enabled, 768 spellings. 4,488 spellings in total.
- result: ZERO HIT in all four. Histograms: enum1 {42:46, 43:37, 44:74, 46:118, 48:104, 49:434, 50:283, 51:498, 52:162, 53:44} - best 42 / 173, one point better than the f1 base of 43, banked as chassis_h1_structure_exact_42.c (it keeps r_src/g_src/b_src/r_ch named and writes the green and blue channels inline in the store). enum2 {42:40, 43:80} - best 42 IS the baseline, no improvement; declaration and birth order in the very block where src and dst are born does not move the 108/109 seat at all. enum3 {31:37, 32:74, 34:118, 36:37, 37:111, 38:168, 39:323, 40:44, 41:44, 42:175, 43:118, 47:46, 53:67, 54:323, 55:115} - best 31 IS the d4 baseline, no improvement. enum4 {42:96, 43:288, 44:288, 45:96} - best 42 IS the baseline and the entire swap axis spans only 4 points, so commutative operand order in the blend products is very nearly inert here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), three chassis - f1/f1_srcup (43 / 173), h1 (42 / 173), d4 (31 / 172) - no FAKE constructs. Variant bodies and per-variant scores in tmp/grind/func_8003DE14/s5/enum{1,2,3,4}/ and enum{1,2,3,4}_results.json.

## [s5] Moving `src++` within the blend arm (before the innermost brace, between the declarations and the channel arithmetic, or after the store) changes the emitted bytes on the f1 chassis.
- mechanism: reorg.c fills the arm delay slots from the `src++` insns, and s4 showed the NUMBER of `src++` statements is load-bearing for the instruction count; the POSITION of one of them was untested.
- probe: f1 (src++ between the declarations and the channel assignments, the s4 form), f1_srcup (src++ hoisted out of the innermost brace, before it), f1_srcdown (src++ after the store), all measured with sandbox --disable all. Bodies tmp/grind/func_8003DE14/s5/base/f1.c, f1_srcup.c, f1_srcdown.c.
- result: KILLED. All three measure 43 / 173, identical. Within this arm the increment's position is byte-neutral, so it is a free axis for building enumeration chassis - s5 used f1_srcup as the enum1 base precisely because hoisting the increment turns the blend block into an all-declaration region the enumerator can permute freely.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173), no FAKE constructs

## [s5] The remaining residual on the structurally-exact (173-insn) chassis lives in the blend block's spelling space: which sub-expressions are named locals, in what order those declarations sit, and which way round each commutative product is written.
- mechanism: The enumerate modality's premise (owner ruling 2026-09-08): for a residual confined to one block, the useful search space is naming x declaration order x operand order, and it is small enough to enumerate exhaustively with tools/spelling_enum.py + tools/sweep_variants.py instead of hand-probing a dozen forms per session. Each variant is a complete function body spliced into src/code6cac_c2.c and scored with the honest cheat-stripped sandbox distance.
- probe: Four exhaustive sweeps. enum1: the inner-loop blend block on the f1_srcup chassis, written fully named as r_src/g_src/b_src/r_ch/g_ch/b_shift/b_ch with the *dst store as the anchor — 1,800 spellings (all inline subsets x all def-before-use orders), no swaps. enum2: the outer-loop declaration block 'total / src / dst / factor / j' on the enum1 winner (h1) chassis, all 120 orderings, every name forced to stay named because all five are read outside the region. enum3: the same 1,800-spelling blend enumeration on the d4 chassis (the score-31 floor form). enum4: the blend block on the h1 chassis with the commutative-operand-swap axis enabled, 768 spellings. 4,488 spellings in total, each scored with 'sandbox func_8003DE14 --disable all'.
- result: ZERO HIT in all four sweeps; no spelling in any region reached distance 0 and none beat its own chassis baseline except enum1. enum1 histogram {42:46, 43:37, 44:74, 46:118, 48:104, 49:434, 50:283, 51:498, 52:162, 53:44} — best 42 / 173, one point better than the f1 base of 43; that winner keeps r_src/g_src/b_src/r_ch named and writes the green and blue channels inline in the final store, and is banked as memory/grind/func_8003DE14/chassis_h1_structure_exact_42.c. enum2 histogram {42:40, 43:80} — best 42 IS the baseline; the src/dst seat never moved, so declaration and birth order in the very block where src and dst are born is worth exactly one point and is not the lever. enum3 histogram {31:37, 32:74, 34:118, 36:37, 37:111, 38:168, 39:323, 40:44, 41:44, 42:175, 43:118, 47:46, 53:67, 54:323, 55:115} — best 31 IS the d4 baseline, so the floor did not move. enum4 histogram {42:96, 43:288, 44:288, 45:96} — best 42 IS the baseline and the entire swap axis spans only 4 points, i.e. commutative operand order in these blend products is very nearly inert.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), three chassis measured: f1/f1_srcup (43 / 173 insns), h1 (42 / 173), d4 (31 / 172). No FAKE constructs in any variant; every spelling is ordinary C emitted by tools/spelling_enum.py.

## [s5] Moving the 'src++' statement within the blend arm — before the innermost brace, between the declarations and the channel arithmetic, or after the store — changes the emitted bytes on the f1 chassis.
- mechanism: reorg.c fills this arm's delay slots from the src++ insns, and s4 proved the NUMBER of src++ statements is load-bearing for the 173-instruction count (hoisting them cost 7-11 instructions). The POSITION of one of them inside the arm had never been measured.
- probe: Three bodies measured with sandbox --disable all: f1 (src++ between the declarations and the channel assignments, the s4 form), f1_srcup (src++ hoisted out of the innermost brace, placed before it), f1_srcdown (src++ moved after the *dst store). Bodies tmp/grind/func_8003DE14/s5/base/f1.c, f1_srcup.c, f1_srcdown.c.
- result: All three measure 43 / 173 — identical. The increment's position inside this arm is byte-neutral, which makes it a free axis for constructing enumeration chassis: hoisting it before the brace turns the blend block into an all-declaration region the enumerator can permute freely, and that is exactly how the enum1 base was built.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173 insns), no FAKE constructs

## [s6] synthesis — merged attack, frontier reset

- **CONFIRMED (H-s6-1).** `reg_n_refs` is loop-depth weighted (`flow.c:2081`,
  `2329`, `2515`, `2725`; depth seeded at 1, `flow.c:456`), so for this
  function an inner-loop reference is worth 3 and an outer-loop one 2. The
  C-level accounting is exact: src = (4 `addiu` x2 + 2 `lhu` x1) x3 + 2 = 32,
  dst = (4 `sh` x1 + 2 `addiu` x2) x3 + 2 = 26 — the same numbers ra_solver
  extracts. Reusable project-wide: reference counts on a structurally-fixed
  chassis are countable from the emitted arms, not a free spelling variable.

- **CONFIRMED (H-s6-2).** The TARGET's own weighted reference counts are also
  32 (src, `$a3`) and 26 (dst, `$a2`), counted directly off
  `asm/funcs/func_8003DE14.s`. The original therefore allocated src to `$a3`
  WITHOUT any reference-count advantage for dst. Everything in s4/s5 that
  treated "the goal is a pure reference-count move on pseudo 109" as a
  description of the target is a description of our chassis only.

- **KILLED, instance (H-s6-3).** "The h1 (42/173) chassis' residual is the
  src/dst seat transposed, so a lever that swaps the pair closes it."
  Measured: on h1, dst is ALREADY at `$a2` (correct) and src sits at `$a1`;
  the find_reg trace is `108 pri 27118 hard_conf=[2,3,4,29] someone=[3] ->
  best 5`. A transposing lever applied to h1 moves dst OFF its correct seat.

- **CONFIRMED (H-s6-4).** Naming one more blend intermediate (the target's own
  shape: `r_ch`/`g_ch` masked-named, `b_shift` named unmasked with its
  `& 0x7C00` inline in the store) creates one more short-lived high-priority
  allocno, which takes `$a1` and moves the cursor pair to the classic swap
  (src `$a2`, dst `$a3`). Chassis k1, 43 / 173, banked. Score got WORSE (42 ->
  43) while the allocation got closer — the s3 non-monotonicity lesson holds.

- **KILLED, instance (H-s6-5).** "Some blend-block naming/shape on the h1
  chassis reaches the target's `$a3`/`$a2` seat directly." Six shapes measured
  (k1 43, k2 44, k3 48, k4 52, k5 43, k6 42, all 173 insns); none seats src at
  `$a3`. k1/k5 reach the swap, k6 stays where h1 is.

- **CONFIRMED (H-s6-6).** The target's seat IS reachable in ordinary C and its
  price is exactly the 173rd instruction. k8 (= k1 + the s3 d4 dose on the
  last-frame zero arm) lifts dst 26 -> 32 weighted refs, crossing the
  `floor_log2` step in `allocno_compare` (`global.c:643`); simulate --trace on
  `tmp/grind/func_8003DE14/s6/k8.model.json` gives dst `$a2` / src `$a3` — the
  target's seat — at 31 / **172**, because the arm's tail becomes identical to
  the colour arm's and jump2 cross-jumps it.

- **KILLED, class (H-s6-7).** "A hard-register PREFERENCE can deflect src off
  `$a2` in find_reg's pass 0 (`regs_someone_prefers`)." `prune_preferences`
  builds that set from the `hard_reg_full_preferences` of lower-priority
  conflicting allocnos, and `set_preference` can only record a preference for
  a hard reg that appears in the pre-RA RTL. `$a2` is argument register 3;
  this function's four callees take at most two arguments and no call lies
  inside either cursor's live range, so `$a2` never appears as a hard reg
  before allocation. `inverse.py global` on the k1 model reports this
  mechanically ("FORECLOSED — 50 preference atom(s) NOT emitted"). Predicate:
  `tools/gcc-2.7.2/global.c:1000` (`IOR_HARD_REG_SET (used, regs_someone_prefers[allocno])`)
  over a set that `prune_preferences` can only ever fill from hard regs the
  RTL mentions.

- **KILLED, instance (H-s6-8).** "Declaration/birth order in the outer-loop
  block moves the seat on a chassis that actually shows the swap." All 24
  `src`-first orderings of `total / src / dst / factor / j` measured on the k1
  chassis: every one scores 43, identical to k1. (s5 swept 120 orderings on
  h1, but h1's seat was not the swap, so that sweep never tested this.)

### Frontier reset (strongest three, in order)

1. **`duplicated-statement-into-arms` that jump2 duplicates BACK.** The only
   size-1 atoms on the k1 model are `refs_up 109: 26->32` and
   `refs_down 108: 32->26`, and inverse.py names
   `duplicated-statement-into-arms` (SANCTIONED, FAKE-annotated,
   `.claude/rules/duplicated-statement-into-arms.md`) as the byte-free way to
   do the first. Every dose measured so far (s3 d2/d3/d5/d6, s6 k8) hands
   jump2 a SECOND IDENTICAL TAIL and loses an instruction. Next probe: find a
   duplication whose arms' tails are NOT byte-identical — e.g. duplicate the
   `dst` store into arms that jump to DIFFERENT labels, or duplicate a
   statement that sits BEFORE the store rather than after it — and verify
   `build_insns == 173` before claiming anything. Do the byte-neutrality check
   first; only if a 173-insn form with dst refs >= 32 exists is the FAKE
   paperwork (scope quote + precedent + exhaustion ledger) worth writing.

2. **`refs_down` on src via `hoist-shared-arm-computation-defeats-copy-pref`.**
   The mirror atom: get src from 32 to 26 weighted refs, i.e. remove exactly
   one `addiu a3,a3,2` from the inner loop while keeping 173 instructions. s4
   killed the two natural hoists (g1 = 61/162, g2 = 48/166) because they
   remove the delay-slot fillers reorg.c needs for the arms' `j`s. Untried:
   move ONE `src++` to a shared tail that only two of the four arms reach
   (e.g. `advance_dst: dst++; src++;` with the colour arm keeping its own),
   which removes one increment from the count while leaving three `j` delay
   slots fillable. Measure `build_insns` first; the seat is only worth
   checking on a 173-insn result.

3. **The blend arm's emission order (sched.c), now the largest diff block.**
   On h1 ~25 of the 42 differing instructions are the blend arm: the target
   emits all six `mult`/`mflo` pairs and both channel adds before any `or`,
   while our builds interleave an `or` after each channel. k1/k5/k6 move it
   but do not close it. This is a first-pass scheduler question, not a naming
   one (s5 exhausted naming), so attack it with `tools/sched_solver` on the k1
   chassis — order- and clock-exact — rather than with more spellings.

## [s6] reg_n_refs in GCC 2.7.2 is loop-depth weighted, so this function's src/dst reference counts are exactly computable from the emitted arm structure rather than being a free spelling variable.
- mechanism: flow.c:2081/2329/2515/2725 all do `reg_n_refs[regno] += loop_depth`, with basic_block_loop_depth seeded at 1 for function scope (flow.c:456, and loop_depth==0 aborts at flow.c:1453). For func_8003DE14 the outer do-loop is depth 2 and the per-pixel loop depth 3. Counting the emitted insns: src = (4 x `addiu a3,a3,2` [set+use = 2 refs] + 2 x `lhu ..,0(a3)` [1 ref]) x 3 + `addiu a3,sp,0x10` [2 refs] x 2 = 32; dst = (4 x `sh ..,0(a2)` [1 ref] + 2 x `addiu a2,a2,2` [2 refs]) x 3 + `addiu a2,sp,0x410` [2 refs] x 2 = 26.
- probe: Read flow.c's four reg_n_refs sites and the basic_block_loop_depth seeding, then hand-count the arms in the h1 build and compare against tools/ra_solver/extract.py's model for pseudos 108/109.
- result: The hand count reproduces ra_solver's extracted numbers exactly (108 refs 32, 109 refs 26) on the h1 chassis. CONFIRMED.
- verdict: CONFIRMED

## [s6] The target binary's own weighted reference counts for the src and dst cursors are 32 and 26 - identical to ours - so the original allocated src to $a3 with no reference-count advantage for dst.
- mechanism: asm/funcs/func_8003DE14.s references $a3 with 2 `lhu` (8003DF40, 8003DF6C) and 4 `addiu $a3,$a3,2` (DF58, DF68, DF88, DFE4) inside the depth-3 inner loop plus the `addiu $a3,$sp,0x10` init at DEE8 (depth 2) = 32; it references $a2 with 4 `sh` (DF50, DF5C, DF80, E020) and 2 `addiu $a2,$a2,2` (DF60, E024) plus the `addiu $a2,$sp,0x410` init at DEEC = 26. Applying the H-s6-1 weighting to the target's own instruction stream is a direct measurement of the original compilation's allocno_compare inputs.
- probe: grep every $a2/$a3 reference out of asm/funcs/func_8003DE14.s, classify each as set / use, and apply the loop-depth weights.
- result: 32 and 26. Consequence recorded in the ledger: s4's and s5's treatment of `refs_up on 109 / refs_down on 108` as a DESCRIPTION of the target is wrong - those vectors force OUR chassis, they are not what the original did.
- verdict: CONFIRMED

## [s6] The residual on the h1 chassis (42 / 173) is the src/dst cursor pair transposed, so a lever that swaps the pair closes it.
- mechanism: Every session since s3 has described the residual as 'src/dst swapped' and aimed levers at moving dst ahead of src. A side-by-side against the reference object plus the ra_solver find_reg trace shows what the allocation actually is on this chassis.
- probe: Applied chassis_h1_structure_exact_42.c, ran tmp/grind/func_8003DE14/s4/sbs.sh against the reference object, and ran tools/ra_solver/extract.py + simulate.py --trace.
- result: On h1 dst is ALREADY correctly seated at $a2; src sits at $a1, two registers below the target's $a3. The find_reg trace reads `a=108 pri=27118 hard_conf=[2,3,4,29] someone=[3] best=5` - only $v0/$v1/$a0 are excluded, so the ascending scan stops at $a1. A transposing lever applied to h1 moves dst OFF its correct seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), h1 chassis (42 / 173 insns), memory/grind/func_8003DE14/chassis_h1_structure_exact_42.c, no FAKE constructs

## [s6] Naming one more blend intermediate creates one more short-lived high-priority allocno, which takes $a1 and moves the src/dst pair from h1's (src $a1, dst $a2) to the classic swap (src $a2, dst $a3).
- mechanism: global.c's allocno_compare orders by floor_log2(refs)*refs/live_length; a short-lived named intermediate has few refs but a tiny live length, so it outranks the cursors and is allocated first, adding a hard-reg conflict to the cursors' exclusion sets. The target's own blend arm (8003DF8C-8003E020) shows exactly one more such intermediate than h1: r_ch and g_ch are masked-named, the blue channel is named with only the shift and its 0x7C00 mask is applied inline in the store.
- probe: Six blend spellings measured on h1 with sandbox --disable all (bodies tmp/grind/func_8003DE14/s6/body_k{1..6}.c), then ra_solver extract + simulate --trace on the k1 result.
- result: k1 (the target's own shape) = 43 / 173 with `108 pri 27118 hard_conf=[2,3,4,5,29] -> $a2` and `109 pri 17931 hard_conf=[2,3,4,5,6,29] -> $a3`. Scores: k6 42, k1 43, k5 43, k2 44, k3 48, k4 52 - all 173 insns. k1 scores one WORSE than h1 while being strictly closer to the target's allocation (the s3 score-is-not-monotone lesson). Banked as memory/grind/func_8003DE14/chassis_k1_target_blend_naming_43.c.
- verdict: CONFIRMED

## [s6] One of the six blend-block naming shapes measured on the h1 chassis, including the target's own r_ch / g_ch / b_shift shape, seats src at $a3.
- mechanism: s5 swept naming x declaration order x operand order inside this block and never checked the resulting register seat, only the score. Since the seat is what the residual is, the six shapes were re-measured with the seat as the read-out.
- probe: sandbox --disable all on body_k1..k6 (tmp/grind/func_8003DE14/s6/), plus ra_solver find_reg traces on k1.
- result: None of the six seats src at $a3. k1 and k5 reach the classic swap (src $a2 / dst $a3), k6 stays where h1 is (src $a1 / dst $a2), k2/k3/k4 are worse. Blend naming alone moves the pair by one register, not two.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, h1 chassis (42 / 173 insns), forms tmp/grind/func_8003DE14/s6/body_k1.c .. body_k6.c, no FAKE constructs

## [s6] The target's seat (src $a3 / dst $a2) is reachable in ordinary C, and on the k1 chassis its price is exactly the 173rd instruction.
- mechanism: Adding one dst-modifying instruction inside the depth-3 inner loop adds 2 RTL references x depth 3 = +6 weighted refs, taking dst from 26 to 32. That crosses the floor_log2 step in allocno_compare (global.c:643): floor_log2(26)=4 gives dst 17931 while floor_log2(32)=5 gives 27118, which overtakes src. The instruction it costs is jump2's cross-jump of the arm's now-identical tail.
- probe: k8 = k1 with the last-frame zero-pixel arm respelled `*dst++ = pixel; src++; goto loop_check;`. Measured with sandbox --disable all and re-extracted with tools/ra_solver.
- result: k8 = 31 / 172. simulate --trace on tmp/grind/func_8003DE14/s6/k8.model.json: `109 pri 27118 -> $a2`, `108 pri 26666 -> $a3` - the target's seat, dispositions matching the dump. Banked as memory/grind/func_8003DE14/chassis_k8_target_seat_172insn_31.c. The seat and the 173rd instruction have been mutually exclusive on every dose measured so far (s3 d2/d3/d5/d6, s6 k8).
- verdict: CONFIRMED

## [s6] A hard-register preference held by another allocno can deflect src off $a2 in find_reg's pass 0 (the regs_someone_prefers exclusion), which would seat src at $a3 without any reference-count change.
- mechanism: find_reg's pass 0 excludes regs_someone_prefers[allocno]; prune_preferences builds that set as the union of hard_reg_full_preferences over the LOWER-priority allocnos that conflict with this one. regs_used_so_far cannot be the discriminator because global.c:367 seeds it with every call_used_reg before any allocation. So the only pass-0 route to $a3 for src is some conflicting allocno preferring $a2.
- probe: Read find_reg (global.c:952) and prune_preferences; then ran tools/ra_solver/inverse.py global on the k1 model with --goal '{"108": 7, "109": 6}'.
- result: inverse.py reports the whole preference axis dead in so many words: 'FORECLOSED - 50 preference atom(s) NOT emitted (mechanically unreachable from C): $a2 never appears as a hard reg in this function's pre-RA RTL, so global.c set_preference can never record a preference for it.' $a2 is argument register 3; this function's four callees (DrawSync, StoreImage, LoadImage, func_80052BE4) take at most two arguments and no call lies inside either cursor's live range, so no C spelling of this function's body can put $a2 into the pre-RA RTL as a hard reg. inverse.py's minimal solution on k1 is 1 atom, 8 vectors, every one of them refs_down 108: 32->26 or refs_up 109: 26->32 - no conflict, live-length, birth-order or preference atom exists at size 1.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), model tmp/grind/func_8003DE14/s6/k1.model.json, no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/global.c:1000

## [s6] Declaration/birth order in the outer-loop block moves the src/dst seat on the k1 chassis, which is the first chassis in this grind that actually shows the classic swap.
- mechanism: s5 swept all 120 orderings of `total / src / dst / factor / j` on the h1 chassis, but h1's seat is not the swap (H-s6-3), so that sweep never tested the swap. Birth order changes which short-lived temps are live across each cursor's birth and therefore which hard-reg conflicts each cursor carries into find_reg.
- probe: Generated the 24 src-first permutations of the five declarations off the k1 body (tmp/grind/func_8003DE14/s6/mkdecl.py) and scored every one with sandbox --disable all (tmp/grind/func_8003DE14/s6/declperm_results.json).
- result: Every one of the 24 scores 43 - identical to k1 itself, and to each other. Both cursors are born one instruction apart and die at the same inner-loop exit, so no reordering of this block produces a temp that conflicts with src but not with dst.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), 24 bodies tmp/grind/func_8003DE14/s6/body_m_*.c, no FAKE constructs

## [s7] The residual on the k1 chassis is FOUR pseudos, not two: goal_from_tgt's object-level attribution gives {108 (src) -> $a3, 109 (dst) -> $a2, 115 (j) -> $t4, 116 (complement) -> $t5}, and against that FULL goal inverse.py's minimal solution is 2 atoms, one from each pair.
- mechanism: solver rule (3) — a subset goal voids the vectors. s6 ran `inverse.py global --goal '{"108": 7, "109": 6}'` (the cursor pair only) and concluded "1 atom, 8 vectors, all refs". Running the same model against the full four-pseudo disposition shows the two pairs are INDEPENDENT sub-problems: every one of the 72 minimal vectors is (one cursor atom) x (one j/complement atom). `goal_from_tgt.py classify code6cac_c2 func_8003DE14` also types the whole residual as FIRST DIVERGENCE: RA with ours 173 / target 173 insns, so nothing pre-RA or scheduler-side is in the way of either pair.
- probe: `goal_from_tgt.py classify`, then `goal_from_tgt.py goal --model tmp/grind/func_8003DE14/s6/k1.model.json`, then `inverse.py global tmp/grind/func_8003DE14/s6/k1.model.json --goal '{"108": 7, "115": 12, "109": 6, "116": 13}' --depth 2 --top 12` (tmp/grind/func_8003DE14/s7/inverse_fullgoal_d2.txt).
- result: minimal solution size 2 atoms, 72 distinct vectors; the cursor half is always `refs_down 108: 32->26` or `refs_up 109: 26->32`, the j/complement half is always one of `live_shrink 115: 57->49`, `refs_down 116: 11->10`, `live_extend 116: 52->60`, `refs_up 115: 11->13`. Re-running the cursor pair alone at depth 2 (inverse_cursor_d2.txt) returns 8 vectors, 7 of them refs_up 109 and 1 refs_down 108 — no live-length, conflict, birth-order or preference atom reaches the cursor seat inside the searched bounds (refs +12/-6, live +/-2,4,8).
- verdict: CONFIRMED

## [s7] The j/complement ($t4/$t5) pair is decided by live_length ALONE — both allocnos carry refs 11 — and an EXACT TIE is enough, because j is the lower allocno and allocno_compare falls through to `return *v1 - *v2`.
- mechanism: global.c:643 allocno_compare computes pri = floor_log2(refs)*refs/live_length*10000*size and, when the two priorities are equal, returns `*v1 - *v2` (global.c:653) — the lower allocno number is allocated first. On the k1 chassis j is pseudo 115 and complement is 116, so the tie goes to j, which is exactly what the target wants ($t4 = the first of the pair). k1 has j live 57 / complement live 52, so complement wins outright.
- probe: perturbed pseudo 115's livelen in the q_j_late model and re-ran the validated global.c simulator (tmp/grind/func_8003DE14/s7/tie_test.py; simulate.Sim.simulate(overrides=...)) at livelen 53, 52, 51, 49.
- result: 53 -> 115=$t5 116=$t4 (ours); 52, 51 and 49 all -> 115=$t4 116=$t5 (the target's). The threshold is <= 52, i.e. j's live length only has to REACH complement's, not undercut it. inverse.py's grid only samples live +/-2,4,8, which is why s6 never saw that the requirement is 1 insn away rather than 8.
- verdict: CONFIRMED

## [s7] j's live_length can be driven from 57 to 53 by moving `s32 j = 0;` inside the `if (total > 0)` guard, and no further by moving other declarations of the outer block in with it.
- mechanism: both j and complement are live across the whole inner loop (complement is loop-invariant and live over the back edge, j is the induction variable), so their live lengths differ only by the distance between their two SETS in the inner-loop preheader. `j = 0` is emitted from the source ahead of loop.c's hoisted `complement = blend_base - factor` (loop.c emits movables immediately before the loop start note, i.e. after whatever the preheader already holds), so j is structurally born first and is live exactly one insn longer. Moving j's birth later should close that insn.
- probe: q_j_late (j born as the first statement of the guarded block) plus six r-forms that additionally move src / dst / factor into the guarded block AHEAD of j (tmp/grind/func_8003DE14/s7/r{1..6}*.c). Scored with sandbox --disable all and modelled with tmp/grind/func_8003DE14/s7/models.py (extract.py + ALLOCDBG).
- result: q_j_late gives j livelen 53 (pri 6226) vs complement 52 (pri 6346) — still one short — and scores 44/173. ALL SIX r-forms also land on j livelen 53 exactly, whatever else is moved into the block (scores 45,45,46,46,46,46, all 173 insns). 53 is the floor this lever reaches: once j's set is adjacent to the preheader, the remaining insn is loop.c's hoisted subu itself, which is always emitted after it.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), k1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s7/q_j_late.c and r1_sd_in.c .. r6_s_in.c, no FAKE constructs

## [s7] Writing `complement` in the OUTER body ahead of `j = 0` makes complement the longer-lived of the pair (the live_extend 116 atom), but it is not a route to the target's $t4/$t5 because it costs the `count - 1` inline property.
- mechanism: if complement's set precedes j's, complement is live one insn longer, its priority drops below j's and j is allocated first. The known cost is s2's: `complement` declared INSIDE the inner loop is that loop's first movable, and spending loop.c's first move applies `threshold -= 3` (loop.c:1904) so the `count - 1` comparand fails the movable test and stays inline the way the target has it.
- probe: four u-forms placing `s32 complement = blend_base - factor;` at different points of the outer declaration block, always before `s32 j = 0;`, with the inner-loop declaration deleted (tmp/grind/func_8003DE14/s7/u{1..4}*.c); scored, modelled, and one of them disassembled side-by-side against the target object (tmp/grind/func_8003DE14/s7/sbs.py).
- result: all four score 65 / 173 (vs k1's 43). The side-by-side shows the cause directly: at stream position 48 the target has `li s8,4096` while u1 has `addiu s6,s2,-1` — `count - 1` is hoisted into the outer preheader, and the whole preheader block shifts with it. The pseudo NUMBERING also swaps (complement becomes 115 and j 116), so the models.py read-out "115 -> $t4" is complement, not j: the emitted `subu t4 / move t5` pair is byte-identical to k1's, i.e. the $t4/$t5 residual is NOT fixed. Any future reading of these models must key on role, not on the pseudo number.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s7/u1_c_before_j.c .. u4_fc_then_j.c, no FAKE constructs

## [s7] The `refs_up 115` half of the j/complement atom is reachable in ordinary C — `if (j < total)` as the inner-loop guard buys j refs 11 -> 13 and DOES seat j at $t4 — but the guard's own codegen costs more than the pair is worth.
- mechanism: j is 0 at the guard, so `if (j < total)` and `if (total > 0)` are the same predicate; the comparison is one extra depth-2 reference to j, i.e. +2 weighted refs, which lifts pri from 5789 to 6842 past complement's 6346.
- probe: q_j_guard (guard rewritten as `if (j < total)`) and q_while_total (guard on the recomputed `rect[2] * rect[3]`), scored and modelled.
- result: q_j_guard = 61 / 173 with j refs 13, livelen 57, pri 6842 -> $t4 — the target's seat for j, reached. q_while_total = 61 / 173 as well. Both pay ~18 points elsewhere: materialising j before the compare turns the single `blez` guard into an slt/branch pair and shifts the whole preheader, and the cursors are untouched ($a2/$a3 still swapped). Banked as memory/grind/func_8003DE14/rejected/s7-j-guard-refs-up-seats-t4-but-61.c.
- verdict: CONFIRMED

## [s7] The cursor pair's live-length routes require the cursors to stop being live across the inner loop, which they are by construction, so the cursor seat is a reference-count question and nothing else.
- mechanism: with src refs 32 / live 59 and dst refs 26 / live 58, allocno_compare needs pri(dst) > pri(src). Holding refs fixed, that means 104/L_dst > 160/59 or 104/58 > 160/L_src. Exact thresholds were measured rather than inferred.
- probe: tmp/grind/func_8003DE14/s7/cursor_thresh.py sweeps the k1 model through the validated simulator, one input at a time.
- result: dst livelen must fall from 58 to <= 38 (-20), or src livelen must rise from 59 to >= 90 (+31), or dst nrefs must rise from 26 to 32. Both cursors are live over the entire inner loop (each is incremented and re-read every iteration, so each is live across the back edge), and the inner loop's own block span is ~50 insns — a 20-insn shrink or a 31-insn extension is not a property any spelling of this loop can have. That leaves `dst refs 26 -> 32` (the k8 route, which costs the 173rd instruction to jump2) and `src refs 32 -> 26` as the only two live doors.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), model tmp/grind/func_8003DE14/s6/k1.model.json, no FAKE constructs

## [s7] In the TARGET's own instruction stream three of the four `addiu $a3,$a3,2` sit in `j` delay slots, which is where a reg_n_refs / instruction-count decoupling could come from that costs nothing.
- mechanism: reg_n_refs is counted by flow.c on the pre-RA RTL; reorg.c runs AFTER allocation. jump2's cross-jump REMOVES an insn after counting (that is k8: dst refs 32 with 172 insns), and reorg.c's delay-slot fill can DUPLICATE the branch target's first insn after counting — the opposite sign, an insn gained for free. If one of the target's four `addiu $a3` is a reorg copy rather than a moved source insn, the original C had three `src++` statements, src's pre-RA refs were 26 or lower, and dst is allocated first with no duplicated-statement construct anywhere.
- probe: read asm/funcs/func_8003DE14.s 8003DF38-8003E02C and classified every $a3 reference. Then checked the shape of the fills.
- result: the four increments are at DF58 (delay slot of `j .L8003E024`), DF68 (delay slot of `j .L8003E028`), DF88 (delay slot of `j .L8003E024`) and DFE4 (inline, mid-blend-arm). All three delay-slot fills target labels whose first insn is `addiu $a2,$a2,2` or the loop latch, NOT `addiu $a3,$a3,2`, so none of them is a "steal the target's first insn" duplication on the face of it — they are consistent with reorg sinking each arm's own preceding insn. That makes the straightforward reading src refs = 32, identical to ours, which is exactly the contradiction s6 recorded: same refs, same live lengths, different seat. The decoupling therefore has to come from a C shape where fewer `src++` statements exist pre-RA than `addiu $a3` insns exist post-reorg, and the three-way shared advance tail is the untested shape.
- verdict: CONFIRMED

## [s7] The k1 residual is four pseudos, not two: goal_from_tgt's object-level attribution gives {108 src -> $a3, 109 dst -> $a2, 115 j -> $t4, 116 complement -> $t5}, and against that FULL goal inverse.py's minimal solution is 2 atoms, always one cursor atom crossed with one j/complement atom.
- mechanism: Solver rule (3): a subset goal voids the vectors. s6 ran the cursor pair alone and read '1 atom, 8 vectors, all refs'. goal_from_tgt.py classify types the whole gap as FIRST DIVERGENCE: RA at ours 173 / target 173 insns, so nothing pre-RA or scheduler-side blocks either pair; inverse.py at depth 2 then shows the two pairs factor.
- probe: goal_from_tgt.py classify code6cac_c2 func_8003DE14 (the object-level path; the text classifier refuses on a zero-rule function), then goal_from_tgt.py goal --model tmp/grind/func_8003DE14/s6/k1.model.json, then inverse.py global --goal '{"108": 7, "115": 12, "109": 6, "116": 13}' --depth 2 --top 12.
- result: Minimal solution size 2 atoms, 72 distinct vectors. Cursor half is always refs_down 108: 32->26 or refs_up 109: 26->32. j/complement half is always live_shrink 115: 57->49, refs_down 116: 11->10, live_extend 116: 52->60, or refs_up 115: 11->13. Re-running the cursor pair alone at depth 2 returns 8 vectors, 7 refs_up 109 and 1 refs_down 108 - no live-length, conflict, birth-order or preference atom inside the searched bounds (refs +12/-6, live +/-2,4,8).
- verdict: CONFIRMED

## [s7] The j/complement ($t4/$t5) pair is decided by live_length alone - both allocnos carry refs 11 - and an EXACT TIE at 52 is sufficient, because j is the lower allocno and allocno_compare falls through to `return *v1 - *v2`.
- mechanism: global.c:643 allocno_compare computes pri = floor_log2(refs)*refs/live_length*10000*size and, on equal priorities, returns *v1 - *v2 (global.c:653) - the lower allocno number is allocated first and takes the lower register. j is pseudo 115, complement 116, so the tie goes to j, which is exactly what the target wants ($t4). k1 has j live 57 / complement live 52, so complement wins outright and takes $t4.
- probe: tmp/grind/func_8003DE14/s7/tie_test.py: perturb pseudo 115's livelen in the q_j_late model and re-run the validated global.c simulator via simulate.Sim.simulate(overrides=...) at 53, 52, 51, 49.
- result: 53 -> 115=$t5 116=$t4 (ours). 52, 51 and 49 all -> 115=$t4 116=$t5 (the target's). The threshold is <= 52: j's live length only has to REACH complement's, not undercut it. inverse.py's live grid samples +/-2,4,8 only, which is why s6 saw a demand for -8 rather than the real -1.
- verdict: CONFIRMED

## [s7] Moving j's birth later drives its live_length from 57 to 53 and stops there: all six forms that additionally move src, dst and/or factor into the `if (total > 0)` guard ahead of j measure j livelen 53 exactly.
- mechanism: Both j and complement are live across the whole inner loop (complement is loop-invariant and live over the back edge; j is the induction variable), so their live lengths differ only by the distance between their two SETS in the preheader. loop.c emits the hoisted complement immediately before the loop start note, i.e. after whatever the preheader already holds, so j's init precedes it by one insn.
- probe: q_j_late plus r1_sd_in .. r6_s_in (tmp/grind/func_8003DE14/s7/), scored with sandbox --disable all and modelled with tmp/grind/func_8003DE14/s7/models.py (extract.py + ALLOCDBG nrefs/livelen/pri/hardreg/order).
- result: q_j_late: j livelen 53 pri 6226 vs complement 52 pri 6346, score 44/173. All six r-forms: j livelen 53 regardless of what else moves into the block; scores 45,45,46,46,46,46, every one 173 insns. Banked as memory/grind/func_8003DE14/rejected/s7-j-late-livelen-53-floor-at-52-44.c and s7-decls-into-guard-j-livelen-stuck-53-46.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), k1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s7/q_j_late.c and r1_sd_in.c .. r6_s_in.c, no FAKE constructs

## [s7] Writing `complement` in the outer body ahead of `j = 0` (the live_extend 116 atom) leaves the emitted $t4/$t5 pair byte-identical to k1's and costs the `count - 1` inline property.
- mechanism: If complement's set precedes j's, complement is live one insn longer and its priority drops below j's. The cost is s2's: complement declared INSIDE the inner loop is that loop's first movable, and spending loop.c's first move applies threshold -= 3 (loop.c:1904) so the `count - 1` comparand fails the movable test and stays inline the way the target has it.
- probe: Four u-forms placing `s32 complement = blend_base - factor;` at different points of the outer declaration block, always before `s32 j = 0;`, inner declaration deleted; scored, modelled, and u1 disassembled positionally against build/src/code6cac_c2.o with tmp/grind/func_8003DE14/s7/sbs.py.
- result: All four score 65 / 173 (k1 is 43). The side-by-side names the cause at stream position 48: target `li s8,4096` vs ours `addiu s6,s2,-1` - count-1 hoisted into the outer preheader, dragging the whole block. The move also RENUMBERS the pseudos (complement becomes 115, j becomes 116), so models.py's '115 -> $t4' here is complement: the emitted `subu $t4 / move $t5` is unchanged. Future model read-outs on declaration-moving forms must key on role, not pseudo number. Banked as memory/grind/func_8003DE14/rejected/s7-complement-outer-loses-count-minus-1-inline-65.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s7/u1_c_before_j.c .. u4_fc_then_j.c, no FAKE constructs

## [s7] The refs_up half of the j/complement atom is reachable in ordinary C - `if (j < total)` as the inner-loop guard buys j refs 11 -> 13 and seats j at $t4 - at a cost of 18 points elsewhere.
- mechanism: j is 0 at the guard, so `if (j < total)` and `if (total > 0)` are the same predicate; the comparison is one extra depth-2 reference to j, i.e. +2 weighted refs, lifting pri from 5789 to 6842 past complement's 6346.
- probe: q_j_guard and q_while_total, scored with sandbox --disable all and modelled with models.py.
- result: q_j_guard = 61 / 173, j refs 13 livelen 57 pri 6842 -> $t4 (the target's seat for j, reached). q_while_total = 61 / 173. Both pay ~18: materialising j for the compare replaces the single `blez` guard with an slt/branch pair and shifts the preheader; the cursors are untouched. Banked as memory/grind/func_8003DE14/rejected/s7-j-guard-refs-up-seats-t4-but-61.c.
- verdict: CONFIRMED

## [s7] Changing only the cursors' live lengths did not reach the target seat on the k1 chassis: the measured thresholds are dst 58 -> 38 or src 59 -> 90, while both cursors are live across the whole inner loop whose block span is about 50 insns.
- mechanism: With src refs 32 / live 59 and dst refs 26 / live 58, allocno_compare needs pri(dst) > pri(src): holding refs fixed that is 104/L_dst > 160/59 or 104/58 > 160/L_src. Each cursor is incremented and re-read every iteration, so each is live over the back edge and its live range covers the loop.
- probe: tmp/grind/func_8003DE14/s7/cursor_thresh.py sweeps the k1 model one input at a time through the validated simulator and reports the first hit.
- result: dst livelen 38 -> 108=$a3 109=$a2 (first hit descending from 58); src livelen 90 -> 108=$a3 109=$a2 (first hit ascending from 59); dst nrefs 32 -> 108=$a3 109=$a2 (first hit ascending from 26). A 20-insn shrink or a 31-insn extension is far outside the loop's span, leaving the two reference-count doors: dst 26 -> 32 (the k8 route, which costs the 173rd instruction to jump2) and src 32 -> 26.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, k1 chassis (43 / 173 insns), model tmp/grind/func_8003DE14/s6/k1.model.json, no FAKE constructs

## [s7] Three of the target's four `addiu $a3,$a3,2` sit in `j` delay slots, but none of the three copies its branch target's first insn, so the straightforward reading of the target's src reference count stays 32 and the s6 same-refs-different-seat contradiction stands.
- mechanism: reg_n_refs is counted by flow.c on pre-RA RTL while reorg.c runs after allocation, so reorg's delay-slot fill can add an insn that was never counted - the opposite sign to jump2's cross-jump, which removes one after counting (that is k8: dst refs 32 with 172 insns). A 'steal the branch target's first insn' fill would mean the original C had fewer `src++` statements than the asm has `addiu $a3` insns.
- probe: Read asm/funcs/func_8003DE14.s 8003DF38-8003E02C and classified every $a3 reference and every delay-slot fill against its branch target's first insn.
- result: DF58 fills `j .L8003E024`, DF68 fills `j .L8003E028`, DF88 fills `j .L8003E024`, DFE4 is inline mid-blend-arm. .L8003E024 begins `addiu $a2,$a2,2` and .L8003E028 begins the loop latch, so none of the three fills is a target-first-insn duplication; each is consistent with reorg sinking its own arm's preceding insn. The decoupling that would resolve the contradiction therefore has to be a C shape with fewer `src++` statements pre-RA than `addiu $a3` insns post-reorg - the three-way shared advance tail, which is untested.
- verdict: CONFIRMED

## [s8] Re-audit: the banked floor forms still measure what the ledger says on HEAD 2026-09-10 — candidate.c and k8 both 31 / 172, k1 43 / 173.
- mechanism: Kill re-audit mandated after three flat sessions. Every banked form on this function is ordinary C with zero FAKE constructs, so the ablation half of the re-audit (`tools/fake_ablate.py`) has nothing to strip; the chassis half is a straight re-measure.
- probe: `tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2` over memory/grind/func_8003DE14/{candidate.c, chassis_k8_target_seat_172insn_31.c, chassis_k1_target_blend_naming_43.c}.
- result: 31/172, 31/172, 43/173 — identical to the ledger. No chassis drift; every s6/s7 conclusion measured on k1 remains chassis-valid.
- verdict: CONFIRMED

## [s8] The src/dst seat is decided by the NUMBER OF `src++` STATEMENTS in the inner loop: exactly three gives src refs 26 and the target's $a3/$a2 seat, in ordinary C, with no extra dst reference.
- mechanism: flow.c:2081 counts `reg_n_refs[regno] += loop_depth`, and the per-pixel loop is at depth 3, so each `src++` statement (2 RTL refs: one use, one set) is worth 6 weighted refs. k1: src = (2 reads + 4 incs x 2) x 3 + 2 = 32, dst = (4 stores + 2 incs x 2) x 3 + 2 = 26. allocno_compare (global.c:643) then gives src floor_log2(32)*32/59 = 27118 vs dst floor_log2(26)*26/58 = 17931, so src is allocated first and takes $a2. Dropping to three `src++` statements takes src to 26, i.e. pri 17931 vs dst 18245 — dst is allocated first and takes $a2, src takes $a3.
- probe: q1 = k1 with both zero-pixel arms routed through a shared `advance_src: src++;` tail placed just above `advance_dst:`, and the blend arm given an explicit `goto advance_dst;` so every path still advances src exactly once (three statements remain: blend arm, colour arm, shared tail). Scored with sandbox --disable all and modelled with tmp/grind/func_8003DE14/s8/models.py. Also p1..p4 (partial/over-shared variants) and q2/q3.
- result: q1 = 37 / 172 with `src refs=26 live=58 pri=17931 -> $a3` and `dst refs=26 live=57 pri=18245 -> $a2` — THE TARGET'S SEAT, reached in ordinary C without k8's duplicated `*dst++` statement. q2 (shared block after the latch) and q3 (blend `src++` written early) reach the same seat at 174 and 172 insns. p2/p3 (only ONE arm shares) leave four statements and measure refs 32 with the seat unmoved, confirming that the count is over STATEMENTS, not arms. Banked as memory/grind/func_8003DE14/chassis_q1_target_seat_refs26_172insn_37.c.
- verdict: CONFIRMED

## [s8] k8 loses its 173rd instruction to jump2's find_cross_jump merging the two last-frame arms' two-insn tails, and the target's arms survive the same pass because their tails match on only ONE insn.
- mechanism: jump.c:2020 calls `find_cross_jump (insn, target, 2, ...)` for each other jump to the same label — minimum 2 matching insns. k8's .greg RTL has arm1 = [sh pixel],[a2+=2],[a3+=2],[j 277] and arm2 = [sh s6],[a2+=2],[a3+=2],[j 277]: two matching insns, so do_cross_jump deletes them (174 pre-jump2 -> 172 emitted). The target's arms are [sh $v0],[addiu $a3],[j] and [sh $s6],[addiu $a2],[addiu $a3],[j]: the `addiu $a3` matches, the next insn back does not, 1 < 2, no merge.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_XJUMP_DEBUG=1 over the k8 body (tmp/grind/func_8003DE14/s8/d_k8/stderr.log), plus the .greg dump read with tmp/grind/func_8003DE14/s8/rtl.py, plus the aligned side-by-side (sbs2.py) that localises the missing instruction to exactly that arm.
- result: k8's emitted last-frame zero arm is `j L / sh $v0,0($a2)` where the target has `sh $v0,0($a2) / j .L8003E024 / addiu $a3,$a3,2`. The one missing instruction is that arm's own `addiu $a3`, deleted by the cross-jump.
- verdict: CONFIRMED

## [s8] reorg.c's delay-slot fill cannot manufacture the target's fourth `addiu $a3` by copying a shared `advance_src:` block's first insn into an unconditional jump's slot.
- mechanism: The copy path exists — reorg.c:3169-3202 ("If there are slots left to fill and our search was stopped by an unconditional branch, try the insn at the branch target") does `add_to_delay_list (copy_rtx (next_trial), delay_list)` and then `reorg_redirect_jump (trial, new_label)`, which is a genuine instruction CREATED after flow.c counted reg_n_refs. But the enclosing `if` at reorg.c:3057-3061 admits only `GET_CODE (insn) != JUMP_INSN` or a CONDITIONAL jump (`condjump_p && ! simplejump_p`). An unconditional `j` can fill its slot only from the backward scan over its own block (reorg.c:2954 onward), which always finds the arm's own store first.
- probe: read reorg.c:3040-3210 and jump.c:1996-2032, then measured the three shared-tail forms (p1 170, q1 172, q2 174) and read their emitted arms with tmp/grind/func_8003DE14/s8/sbs2.py.
- result: In every shared-tail form the arm's jump carries that arm's own `sh` in its delay slot and the shared `addiu $a3` stays where it is, single-instanced: `j L / sh $v0,0($a2)` (2 insns) against the target's `sh $v0,0($a2) / j .L8003E024 / addiu $a3,$a3,2` (3 insns). No form gained an instruction from the steal path. Consequence: the target's four `addiu $a3` were four pre-RA instructions, so the TARGET's own src reference count is 32 — the same as k1's — and s6's "same refs, different seat" contradiction stands.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), q1/q2/p1 chassis (172/174/170 insns), no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/reorg.c:3059

## [s8] Both measured doors to the target cursor seat land exactly one instruction short of 173, from opposite sides.
- mechanism: Door A (k8): keep four `src++` statements and add a fourth dst-touching statement, taking dst to refs 32; that arm's tail then matches its sibling's on two insns and jump2 deletes both (174 -> 172). Door B (q1): drop to three `src++` statements, taking src to refs 26; the emitted stream then has three `addiu $a3` where the target has four (173 -> 172). The two doors are the only ones inside the searched atom bounds (s7's inverse.py: refs_down 108 or refs_up 109).
- probe: k8 and q1 both measured this session with sandbox --disable all, modelled with extract.py/ALLOCDBG, and diffed against the target object with sbs2.py.
- result: k8 = 31 / 172 (seat correct, missing its own arm's `addiu $a3`); q1 = 37 / 172 (seat correct, missing the fourth `addiu $a3`). Neither is a scoring regression to be explained away — they are two structurally different chassis with the SAME 172-vs-173 gap at the SAME instruction.
- verdict: CONFIRMED

## [s8] Re-audit: the banked floor forms still measure what the ledger says on HEAD 2026-09-10 - candidate.c and k8 both 31 / 172, k1 43 / 173.
- mechanism: Kill re-audit mandated after three flat sessions. Every banked form on this function is ordinary C with zero FAKE constructs, so tools/fake_ablate.py has nothing to strip; the chassis half is a straight re-measure with sweep_variants.py.
- probe: tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2 over memory/grind/func_8003DE14/{candidate.c, chassis_k8_target_seat_172insn_31.c, chassis_k1_target_blend_naming_43.c}.
- result: 31/172, 31/172, 43/173 - identical to the ledger. No chassis drift, so every s6/s7 conclusion measured on k1 remains chassis-valid and no banked kill is void.
- verdict: CONFIRMED

## [s8] The src/dst seat is decided by the number of src++ STATEMENTS in the inner loop: exactly three gives src refs 26 and the target's $a3/$a2 seat, in ordinary C, with no extra dst reference.
- mechanism: flow.c:2081 counts reg_n_refs[regno] += loop_depth and the per-pixel loop sits at depth 3, so one src++ statement (2 RTL refs) is worth 6 weighted refs. k1: src = (2 reads + 4 incs x 2) x 3 + 2 = 32, dst = (4 stores + 2 incs x 2) x 3 + 2 = 26; allocno_compare (global.c:643) gives src 27118 vs dst 17931 so src is allocated first and takes $a2. Three statements take src to 26 (pri 17931) against dst 18245, so dst goes first and takes $a2.
- probe: q1 = k1 with both zero-pixel arms routed through a shared 'advance_src: src++;' tail above 'advance_dst:', and the blend arm given an explicit 'goto advance_dst;' so every path advances src exactly once. Scored with sandbox --disable all, modelled with tmp/grind/func_8003DE14/s8/models.py (extract.py + ALLOCDBG). Plus p1..p4, q2, q3.
- result: q1 = 37 / 172 with src refs=26 live=58 pri=17931 -> $a3 and dst refs=26 live=57 pri=18245 -> $a2: the target's seat, reached in ordinary C without k8's duplicated *dst++ statement. p2/p3 (only ONE arm shares) keep four statements, measure refs 32 and do not move the seat - the count is over statements, not arms. Banked as memory/grind/func_8003DE14/chassis_q1_target_seat_refs26_172insn_37.c.
- verdict: CONFIRMED

## [s8] k8 loses its 173rd instruction to jump2's find_cross_jump merging the two last-frame arms' two-insn tails, and the target's arms survive the same pass because their tails match on only one insn.
- mechanism: jump.c:2020 calls find_cross_jump(insn, target, 2, ...) for each other jump to the same label - minimum 2 matching insns. k8's .greg RTL: arm1 = [sh pixel],[a2+=2],[a3+=2],[j 277] and arm2 = [sh s6],[a2+=2],[a3+=2],[j 277] -> two matching insns -> do_cross_jump deletes them (174 pre-jump2 -> 172 emitted). The target's arms are [sh $v0],[addiu $a3],[j] vs [sh $s6],[addiu $a2],[addiu $a3],[j]: one match, below the minimum.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_XJUMP_DEBUG=1 on the k8 body (tmp/grind/func_8003DE14/s8/d_k8/stderr.log), the .greg RTL read with s8/rtl.py, and the aligned side-by-side s8/sbs2.py.
- result: k8's emitted last-frame zero arm is 'j L / sh $v0,0($a2)' where the target has 'sh $v0,0($a2) / j .L8003E024 / addiu $a3,$a3,2'. The single missing instruction is that arm's own addiu $a3, deleted by the cross-jump.
- verdict: CONFIRMED

## [s8] reorg.c's delay-slot fill cannot manufacture the target's fourth addiu $a3 by copying a shared advance_src: block's first insn into an unconditional jump's delay slot.
- mechanism: The copy path exists at reorg.c:3169-3202 (add_to_delay_list (copy_rtx (next_trial)) + reorg_redirect_jump) and would be a genuine instruction created after flow.c counted reg_n_refs, but the enclosing if at reorg.c:3057-3061 admits only non-jump insns and CONDITIONAL jumps (condjump_p && ! simplejump_p). An unconditional j can only fill its slot from the backward scan over its own block (reorg.c:2954 onward), which always finds the arm's own store first.
- probe: Read reorg.c:3040-3210 and jump.c:1996-2032, then measured the three shared-tail forms (p1 170, q1 172, q2 174) and read their emitted arms with s8/sbs2.py against the target object.
- result: Every shared-tail form emits the arm's own sh in the jump's delay slot and leaves the shared addiu $a3 single-instanced: ours 'j L / sh $v0,0($a2)' (2 insns) vs the target's 'sh $v0,0($a2) / j .L8003E024 / addiu $a3,$a3,2' (3 insns). No form gained an instruction from the steal path, so the target's four addiu $a3 were four pre-RA instructions and the target's own src reference count is 32, the same as k1's.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), q1 (172 insns), q2 (174), p1 (170), no FAKE constructs
- predicate_cite: tools/gcc-2.7.2/reorg.c:3059

## [s8] Both measured doors to the target cursor seat land exactly one instruction short of 173, at the same instruction, from opposite sides.
- mechanism: Door A (k8): four src++ statements plus a fourth dst-touching statement takes dst to refs 32; that arm's tail then matches its sibling's on two insns and jump2 deletes both (174 -> 172). Door B (q1): three src++ statements takes src to refs 26; the emitted stream then carries three addiu $a3 where the target has four (173 -> 172). s7's inverse.py bounds these as the only cursor atoms (refs_down 108 / refs_up 109).
- probe: k8 and q1 both measured this session with sandbox --disable all, modelled with extract.py/ALLOCDBG, and diffed against build/src/code6cac_c2.o with s8/sbs2.py.
- result: k8 = 31 / 172 (seat correct, missing its own arm's addiu $a3); q1 = 37 / 172 (seat correct, missing the fourth addiu $a3). Two structurally different chassis with the same one-instruction gap at the same place.
- verdict: CONFIRMED

## [s9] Re-audit: the banked floor is unchanged on the current chassis — candidate.c and k8 both 31 / 172, q1 37 / 172, f1/k1 43 / 173.
- mechanism: Mandated kill re-audit (floor flat since s3). Every banked form on this function is ordinary C with zero FAKE constructs, so `tools/fake_ablate.py` has nothing to strip and the audit reduces to a straight chassis re-measure.
- probe: `tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2 --variants memory/grind/func_8003DE14/{candidate.c, chassis_k8_target_seat_172insn_31.c, chassis_q1_target_seat_refs26_172insn_37.c}` plus the f1 baseline.
- result: 31/172, 31/172, 37/172, baseline f1 43/173 — identical to the s8 ledger. No chassis drift; every s6/s7/s8 conclusion remains chassis-valid and no banked kill is void.
- verdict: CONFIRMED

## [s9] A fresh m2c decompile of asm/funcs/func_8003DE14.s reproduces the f1 chassis' control flow statement for statement, so f1 IS the target's emitted shape and the residual is not a control-flow rederivation problem.
- mechanism: rederive modality — m2c reconstructs the arm/label graph directly from the branch structure, independent of every assumption this ledger has accumulated. If the ledger's chassis had the wrong arm topology, m2c would show a different one.
- probe: `python3 tools/m2c/m2c.py --target mipsel-ido-c -f func_8003DE14 asm/funcs/func_8003DE14.s` -> tmp/grind/func_8003DE14/s9/m2c.c.
- result: m2c emits exactly f1's topology: `if (var_s1 == var_v0)` first with the zero arm doing `*var_a2 = temp_v0; var_a3 += 2; goto block_11;` and the colour arm doing `*var_a2 = COLOUR; var_a2 += 2; var_a3 += 2;` (dst++ INLINED on the colour arm only), the blend arm falling into the shared `block_11: var_a2 += 2;`. The only m2c-visible artefact not in f1 is the second `var_v0 = temp_s2 - 1;` at the bottom of the loop body, which is reorg's delay-slot copy of the loop-top insn (the emitted stream has `addiu $v0,$s2,-1` at both 8003DF34 and 8003E048, with the loop label .L8003DF38 between them), not a source statement.
- verdict: CONFIRMED

## [s9] The s6 "same refs, different seat" contradiction is NOT a divergence between our cc1 port and PsyQ's original cc1psx: the original compiler seats the f1 chassis' cursors exactly the way ours does.
- mechanism: cc1psx calibration/self-disproof (.claude/rules/cc1psx-calibration-only). If SN's GCC 2.7.2.SN.1 had a different `allocno_compare`, `floor_log2` or `find_reg` than decompals/mips-gcc-2.7.2, the identical C would seat src/dst differently under the two compilers, and three sessions of allocator modelling would be measuring the wrong allocator.
- probe: `tmp/grind/func_8003DE14/s9/psx.sh` — preprocess src/code6cac_c2.c carrying the f1 body with the canonical CPP flags, pipe into `tools/cc1psx_wrapper.sh -quiet -O2 -mcpu=3000 -mips1 -msoft-float -funsigned-char -w -G0`, read the cursor initialisations out of the emitted `func_8003DE14`.
- result: cc1psx emits `addu $6,$sp,16` (src_buf -> $a2) and `addu $7,$sp,1040` (dst_buf -> $a3) — the SAME transposed seat our build produces, against the target's `addiu $a3,$sp,0x10` / `addiu $a2,$sp,0x410`. Both compilers agree, so the contradiction is a property of the C we are feeding them, not of the toolchain.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), f1 chassis (43 / 173 insns), no FAKE constructs, artifact tmp/grind/func_8003DE14/s9/f1.psx.s

## [s9] Moving `dst`'s birth as late as C allows shrinks its live_length by only 5 insns (58 -> 53) against the 20 the seat needs, measured in C rather than in the model.
- mechanism: s7 killed the live-length door by sweeping the k1 MODEL one input at a time (dst 58 -> 38 or src 59 -> 90 needed). That is an inference about C, not a measurement of it, so the re-audit re-ran it as source edits. Both cursors are set in the outer-loop body and re-read across the inner loop's back edge, so each one's live range is pinned to the whole inner-loop block span; only the preheader window between the two births is free.
- probe: three f1 variants measured with tmp/grind/func_8003DE14/s8/models.py (ALLOCDBG via tools/ra_solver/extract.py) and scored with sweep_variants — c1 (`dst` last in the outer declaration block), c2 (`dst` first / `src` last), c3 (`dst` declared inside the `if (total > 0)` block, the latest scope that still dominates the loop).
- result: c1 src 32/59 -> $a2, dst 26/57 -> $a3, score 43. c2 dst 26/59 -> $a3, src 32/57 -> $a2, score 43. c3 src 32/59 -> $a2, dst 26/53 -> $a3, score 44 — the biggest C-reachable shrink is 5 insns, giving pri(dst) 19622 against pri(src) 27118. The threshold is live 38 (pri 27368). Declaration placement also never moves the ORDER, only the two lengths by +/-1..5.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s9/c1.c c2.c c3.c, no FAKE constructs

## [s9] Declaring `dst` in an inner scope DOES create the first allocno that conflicts with `src` but not with `dst` — the precondition for a $a2 blocker — but the only such allocno has priority 11428 against the 27118 it needs.
- mechanism: find_reg (global.c:952) allocates in priority order and, in pass 1, takes the LOWEST non-conflicting hard reg; src and dst carry byte-identical conflict sets on every chassis measured so far (hard [2,3,4,5,29,64,65,66], identical pseudo-conflict lists), so whichever is allocated first necessarily takes $a2. A third route to the target seat therefore exists that does not need the priority order to flip at all: an allocno X with pri > pri(src) that conflicts with src but NOT with dst, and that cannot reach $v0/$v1/$a0/$a1, would take $a2 itself; src would then fall to $a3 and dst could re-use $a2 in pass 0 (`IOR_COMPL_HARD_REG_SET (used, regs_used_so_far)` admits an already-used reg when there is no conflict).
- probe: diffed the `conflicts` and `hard_conflicts` sets of the src and dst pseudos in the f1 / c1 / c3 extraction models.
- result: f1 and c1 are perfectly symmetric (src-only conflicts: none; dst-only: none). c3 breaks the symmetry for the first time: pseudo 101 conflicts with src and not dst, and dst additionally drops hard reg 65. But 101 sits at pri 11428 / ord 10, far below src's 27118, so it is allocated long after src and takes no useful seat. For X to take $a2 it would additionally have to be locked out of $v0/$v1/$a0/$a1, i.e. there would have to be five simultaneously-live high-priority short-lived pseudos in the outer-loop preheader window, which the target's lean preheader (two `lh`, `mult`, `mflo`, `addiu`, `sll`, `div`, `mflo`) does not contain.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 / c1 / c3 chassis (43/43/44, 173 insns), models tmp/grind/func_8003DE14/s8/{f1,c1,c3}.model.json, no FAKE constructs

## [s9] Index-addressed cursors (`src_buf[j]` / `dst_buf[j]`, no pointer variables) are the wrong shape class: loop.c strength-reduces both into single-update givs and the emitted stream loses eight instructions.
- mechanism: rederive modality — the one inner-loop shape class this ledger had never spelled. With no explicit cursors, loop.c's biv/giv machinery owns the addressing and emits ONE update per giv at the latch, so the four `addiu $a3` / two `addiu $a2` the target spreads across the arms cannot appear, and the reference counts that decide the seat are generated by loop.c rather than by source statements.
- probe: tmp/grind/func_8003DE14/s9/ix1.c (f1 with every `*src` / `*dst` rewritten as `src_buf[j]` / `dst_buf[j]` and all the cursor increments deleted), scored with sweep_variants.
- result: 56 / 165 instructions — eight short of the target's 173 and the worst score of any structurally-complete form on this function. The arm topology is unchanged; the whole delta is the cursor addressing.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1-derived ix1 chassis (56 / 165 insns), form tmp/grind/func_8003DE14/s9/ix1.c, no FAKE constructs

## [s9] Re-audit: the banked floor is unchanged on the current chassis - candidate.c and k8 both 31 / 172, q1 37 / 172, f1 43 / 173.
- mechanism: Mandated kill re-audit (floor flat since s3). Every banked form on this function is ordinary C with zero FAKE constructs, so tools/fake_ablate.py has nothing to strip and the audit reduces to a straight chassis re-measure.
- probe: tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2 --variants memory/grind/func_8003DE14/{candidate.c,chassis_k8_target_seat_172insn_31.c,chassis_q1_target_seat_refs26_172insn_37.c} plus the f1 baseline.
- result: 31/172, 31/172, 37/172, f1 43/173 - identical to the s8 ledger. No chassis drift; no banked kill is void.
- verdict: CONFIRMED

## [s9] A fresh m2c decompile of asm/funcs/func_8003DE14.s reproduces the f1 chassis' control flow statement for statement, so f1 is the target's emitted shape and the residual is not a control-flow rederivation problem.
- mechanism: rederive modality - m2c reconstructs the arm/label graph from the branch structure alone, independent of every assumption this ledger has accumulated. A wrong arm topology would show up here.
- probe: python3 tools/m2c/m2c.py --target mipsel-ido-c -f func_8003DE14 asm/funcs/func_8003DE14.s -> tmp/grind/func_8003DE14/s9/m2c.c
- result: m2c emits exactly f1's topology: the i == count-1 test first, the last-frame zero arm doing *dst = pixel / src++ / goto advance_dst, the last-frame colour arm INLINING dst++ and jumping past the shared tail, the blend-zero arm sharing the tail, the blend arm falling into it. The only extra m2c shows is a second 'count - 1' at the bottom of the loop body, which is reorg's delay-slot copy of the loop-top insn (emitted addiu $v0,$s2,-1 at both 8003DF34 and 8003E048 with the loop label between them), not a source statement.
- verdict: CONFIRMED

## [s9] The s6 same-refs-different-seat contradiction is not a divergence between our cc1 port and PsyQ's original cc1psx: cc1psx seats the f1 chassis' cursors exactly the way our build does.
- mechanism: cc1psx calibration/self-disproof (.claude/rules/cc1psx-calibration-only). If GCC 2.7.2.SN.1 had a different allocno_compare / floor_log2 / find_reg than decompals/mips-gcc-2.7.2, the identical C would seat src/dst differently under the two compilers and three sessions of allocator modelling would be modelling the wrong allocator.
- probe: tmp/grind/func_8003DE14/s9/psx.sh - canonical CPP of src/code6cac_c2.c carrying the f1 body, piped into tools/cc1psx_wrapper.sh -quiet -O2 -mcpu=3000 -mips1 -msoft-float -funsigned-char -w -G0; cursor initialisations read out of the emitted func_8003DE14.
- result: cc1psx emits addu $6,$sp,16 (src_buf -> $a2) and addu $7,$sp,1040 (dst_buf -> $a3) - the same transposed seat our build produces, against the target's addiu $a3,$sp,0x10 / addiu $a2,$sp,0x410. Both compilers agree; the contradiction is a property of the C being fed to them.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), f1 chassis (43 / 173 insns), no FAKE constructs, artifact tmp/grind/func_8003DE14/s9/f1.psx.s

## [s9] Moving dst's birth as late as C allows shrinks its live_length by only 5 insns (58 -> 53) against the 20 the seat needs, measured in C rather than in the model.
- mechanism: s7 killed the live-length door by sweeping the k1 MODEL one input at a time (dst 58 -> 38 or src 59 -> 90 needed) - an inference about C, not a measurement of it. Both cursors are set in the outer-loop body and re-read across the inner loop's back edge, so each live range is pinned to the whole inner-loop block span; only the preheader window between the two births is free.
- probe: Three f1 variants modelled with tmp/grind/func_8003DE14/s8/models.py (ALLOCDBG via tools/ra_solver/extract.py) and scored with sweep_variants: c1 (dst last in the outer declaration block), c2 (dst first / src last), c3 (dst declared inside the if (total > 0) block).
- result: c1 src 32/59 -> $a2, dst 26/57 -> $a3, 43. c2 dst 26/59 -> $a3, src 32/57 -> $a2, 43. c3 src 32/59 -> $a2, dst 26/53 -> $a3, 44 - the biggest C-reachable shrink is 5 insns, pri(dst) 19622 against pri(src) 27118, threshold live 38. Declaration placement never moves the ORDER, only the lengths by 1..5.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173 insns), forms tmp/grind/func_8003DE14/s9/c1.c c2.c c3.c, no FAKE constructs

## [s9] Declaring dst in an inner scope creates the first allocno that conflicts with src but not with dst - the precondition for an $a2 blocker - but the only such allocno sits at priority 11428 against the 27118 it needs.
- mechanism: find_reg (global.c:952) allocates in priority order and in pass 1 takes the LOWEST non-conflicting hard reg; src and dst carry byte-identical conflict sets on every chassis measured (hard [2,3,4,5,29,64,65,66]), so whichever is allocated first necessarily takes $a2. A third route exists that does not need the priority order to flip: an allocno X with pri > pri(src) conflicting with src but NOT dst and locked out of $v0/$v1/$a0/$a1 would take $a2 itself, src would fall to $a3, and dst could re-use $a2 in pass 0 (IOR_COMPL_HARD_REG_SET (used, regs_used_so_far) admits an already-used reg when there is no conflict).
- probe: Diffed the conflicts and hard_conflicts sets of the src and dst pseudos across the f1 / c1 / c3 extraction models (tmp/grind/func_8003DE14/s8/f1.model.json, c1.model.json, c3.model.json).
- result: f1 and c1 are perfectly symmetric (no src-only and no dst-only conflicts). c3 breaks the symmetry for the first time: pseudo 101 conflicts with src and not dst, and dst additionally drops hard reg 65 - but 101 is pri 11428 / ord 10, allocated long after src. For X to be pushed down to $a2 there would have to be five simultaneously-live high-priority short-lived pseudos in the outer-loop preheader, which the target's seven-insn preheader does not contain.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 / c1 / c3 chassis (43 / 43 / 44, 173 insns), no FAKE constructs

## [s9] Index-addressed cursors (src_buf[j] / dst_buf[j], no pointer variables) are the wrong shape class: loop.c strength-reduces both into single-update givs and the emitted stream loses eight instructions.
- mechanism: rederive modality - the one inner-loop shape class this ledger had never spelled. With no explicit cursors loop.c's biv/giv machinery owns the addressing and emits ONE update per giv at the latch, so the four addiu $a3 / two addiu $a2 the target spreads across its arms cannot appear, and the reference counts that decide the seat are generated by loop.c rather than by source statements.
- probe: tmp/grind/func_8003DE14/s9/ix1.c (f1 with every *src / *dst rewritten as src_buf[j] / dst_buf[j] and all cursor increments deleted), scored with sweep_variants.
- result: 56 / 165 instructions - eight short of the target's 173 and the worst score of any structurally complete form on this function. The arm topology is unchanged; the whole delta is the cursor addressing.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1-derived ix1 chassis (56 / 165 insns), form tmp/grind/func_8003DE14/s9/ix1.c, no FAKE constructs

## s10 hypotheses (forensics, 2026-09-10)

**H-s10-1 (CONFIRMED).** Because flow_analysis precedes combine_instructions,
two extra depth-3 occurrences of the dst pseudo can be paid for in reg_n_refs
and then reclaimed by combine, so dst can outrank src in allocno_compare
without changing the emitted instruction count. Probe: the f1 chassis with a
two-statement dst round-trip appended to the shared inner-loop tail. Result:
dst nrefs 26 -> 38, pri 32758 > src 27118, dst -> $a2 and src -> $a3,
score 43 -> 29 at build_insns 173. Banked as candidate.c.

**H-s10-2 (KILLED, instance).** The same lift spelled as a self-assign leaves
reg_n_refs at 26. Measured on the f1 chassis, HEAD 2026-09-10, no other change:
43 / 173, allocation bit-identical to f1. The insn is gone before
flow_analysis sees it.

**H-s10-3 (KILLED, class).** Diverting src off $a2 inside find_reg's pass 0 by
seeding regs_someone_prefers[108] with hard reg 6 cannot be reached from C in
this function: those bits come only from set_preference on a reg-to-reg copy
with a hard reg on one side (tools/gcc-2.7.2/global.c:1717), and $a2 is never a
copy endpoint here (two parameters, every call takes at most two arguments).
Measured on f1 with BB2_FINDREG_DEBUG=108: someone_prefers empty, $a2 selected
in pass 0.

**H-s10-4 (KILLED, instance).** The pure live-length route (door (c)) does not
flip the seat on this chassis. With refs pinned at 32/26 the flip needs
L_src > 1.538 x L_dst; measured pre-scheduler lives on f1 are src 70 / dst 67
(sched then compresses them to 59/58, sched.c:5106), and both cursors are live
across the whole inner-loop body because both are set and used on its back
edge, so their separation is bounded by the outer-loop preheader (about a dozen
pre-combine insns). Extending src past the inner loop would make it cross the
outer loop's LoadImage/DrawSync calls, which pushes find_reg onto the
callee-saved set entirely (global.c:970-975).

## [s10] Because flow_analysis computes reg_n_refs BEFORE combine_instructions runs, extra depth-3 occurrences of the dst pseudo can be paid for in the allocator's reference count and then reclaimed by combine, letting dst outrank src in allocno_compare without changing the emitted instruction count.
- mechanism: toplev.c:2983 calls flow_analysis (flow.c:2081 reg_n_refs[regno] += loop_depth) and only afterwards, at toplev.c:3004, calls combine_instructions; measured on the f1 chassis combine deletes 19 of the 140 post-flow insns in this function. A two-statement dst round-trip in the shared inner-loop tail adds 4 occurrences of pseudo 109 at pixel-loop depth 3 (+12 weighted, 26 -> 38); combine folds (dst+2)-2 back to dst so nothing is emitted. allocno_compare (global.c:635) then ranks dst 38/58 = 32758 above src 32/59 = 27118, dst is allocated first, and find_reg's plain ascending scan (mips.h defines no REG_ALLOC_ORDER) hands the first-allocated cursor $a2.
- probe: f1 chassis + the round-trip = tmp/grind/func_8003DE14/s10/r2.c, banked as memory/grind/func_8003DE14/candidate.c. sandbox --disable all: score 29, build_insns 173, target_insns 173. ALLOCDBG (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1, tmp/grind/func_8003DE14/s10/d_r2/stderr.log): ord=2 pseudo=109 hardreg=6 nrefs=38 livelen=58 pri=32758; ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=59 pri=27118.
- result: CONFIRMED. Score 43 (f1) -> 29, a new floor below the previous 31 and, unlike that 31 form, at the target's own instruction count. The sbs2.py aligned diff shows target insns 0-69 and 134-172 now byte-exact: both cursor halves are correct and the whole 29 sits in the blend block. NOTE FOR THE NEXT SESSION: the round-trip is a net-zero pair of dead stores to a LOCAL, i.e. a FAKE-family construct that is NOT submittable as written - see the frontier.
- verdict: CONFIRMED

## [s10] Spelling the same reference lift as a self-assign of dst at the same inner-loop site leaves dst's reg_n_refs at 26 and the allocation unchanged.
- mechanism: A no-op register move is removed by cse's delete_trivially_dead_insns, which runs before flow_analysis (toplev.c orders cse2 ahead of flow at 2983), so flow never counts the occurrences.
- probe: tmp/grind/func_8003DE14/s10/r1.c (f1 with the self-assign, nothing else changed), sandbox --disable all.
- result: KILLED. Score 43 / build_insns 173 - bit-identical to the unmodified f1 chassis. Banked as memory/grind/func_8003DE14/rejected/s10-dst-self-assign-deleted-before-flow-43.c. Only a spelling that survives to flow AND is folded later by combine moves the count.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), f1 chassis (43 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s10/r1.c

## [s10] Diverting src off $a2 inside find_reg's pass 0, by getting hard reg 6 into regs_someone_prefers[108], is not reachable from C in func_8003DE14 because hard reg $a2 is never a register-copy endpoint in this function.
- mechanism: find_reg ORs regs_someone_prefers into the pass-0 exclusion set (global.c:1001), and prune_preferences fills it only from the hard_reg_full_preferences of lower-priority conflicting allocnos (global.c:920-928). Those preference bits exist only where set_preference sees a register-to-register copy with a hard reg on one side (global.c:1717). func_8003DE14 has two parameters ($a0, $a1) and every call it makes takes at most two arguments (DrawSync 1, StoreImage 2, LoadImage 2, func_80052BE4 1), so no RTL copy ever names hard reg 6.
- probe: Instrumented cc1 with BB2_FINDREG_DEBUG=108 on the f1 chassis; tmp/grind/func_8003DE14/s10/d_fr108/stderr.log.
- result: KILLED. The trace prints conflicts {2,3,4,5,29}, someone_prefers EMPTY, own_copy_prefs EMPTY, own_full_prefs {30}, and pass0_used excluding 6 - src takes $a2 in pass 0 as the lowest hard reg outside its conflict set. Recorded alongside a new asymmetry worth knowing: src carries a $fp preference (src_buf is at virtual-frame offset 0, so its address expands as a plain reg copy) while dst carries none.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173 insns), no FAKE constructs present, BB2_FINDREG_DEBUG=108 trace
- predicate_cite: tools/gcc-2.7.2/global.c:1717

## [s10] Leaving the cursors' reference counts at 32/26 and moving only their live lengths does not flip the seat on the f1 chassis: the required ratio L_src > 1.538 x L_dst exceeds what the two cursors' shared inner-loop residency allows.
- mechanism: allocno_compare (global.c:635) divides floor_log2(refs)*refs by live_length, so with refs pinned the flip needs 160/L_src < 104/L_dst. reg_live_length is not a flow output - schedule_insns pass 1 overwrites it (sched.c:5106) - and the f1 .sched dump prints the actual values: register 108 life shortened from 70 to 59, register 109 from 67 to 58. Both cursors are set and used on the inner loop's back edge, so both are live over its whole body and their live lengths can differ only by the outer-loop preheader span; pushing src's range past the inner loop would make it cross the outer loop's LoadImage/DrawSync calls, at which point find_reg starts from call_used_reg_set (global.c:970-975) and neither $a2 nor $a3 is reachable at all.
- probe: f1 -da dump set, tmp/grind/func_8003DE14/s10/d_f1/code6cac_c2.sched (life shortened/extended lines) and .flow / .combine insn counts.
- result: KILLED for this chassis. Measured pre-scheduler lives are src 70 / dst 67 against a requirement of L_src >= 1.538 x L_dst = 103 at dst 67. This closes the s9 door (c) with the right units (scheduler-recomputed lives over the post-combine stream) rather than s9's emitted-stream estimate - and it is superseded in practice by the reference-count route, which flipped the seat this session.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, f1 chassis (43 / 173 insns), no FAKE constructs present, dumps tmp/grind/func_8003DE14/s10/d_f1/

## [s11] The +12 weighted-reference lift on the dst pseudo that seats the cursors like the target has an ordinary-C spelling: `dst++` duplicated into each of the three inner-loop arms instead of a shared `advance_dst:` label reached by goto.
- mechanism: flow.c:2081 does `reg_n_refs[regno] += loop_depth`, and the pixel loop is at depth 3, so one `dst++` (a read and a write of pseudo 109) is 6 weighted references. Replacing the single shared copy with three arm-local copies takes that 6 to 18, i.e. dst's weighted reg_n_refs from 26 to 38 - exactly the count the s10 `dst++; dst--;` round-trip produced. allocno_compare (global.c:635) then ranks dst 38/60 above src 32/61 and find_reg's ascending scan hands the first-allocated cursor $a2. The lift is free of emitted cost because jump2's cross-jump tail-merges the three copies back: the shared tail is `addiu dst,1 / addiu j,1 / slt / bne`, four matching insns against the two-insn minimum at jump.c:2020.
- probe: tmp/grind/func_8003DE14/s11/a1.c (the s10 chassis with the label and both gotos deleted and `dst++` written into each arm), `sandbox func_8003DE14 --disable all`, plus BB2_ALLOC_DEBUG=1 on the instrumented cc1 (tmp/grind/func_8003DE14/s11/d_a1/stderr.log).
- result: CONFIRMED. Score 29 / build_insns 173 / target_insns 173 - the same floor and the same instruction count as the s10 form. ALLOCDBG: ord=3 pseudo=109 hardreg=6 nrefs=38 livelen=60 pri=31666 (dst -> $a2) and ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=61 pri=26229 (src -> $a3). No net-zero statement, no dead store, and nothing FAKE-family remains anywhere in the body; the construct is the sanctioned duplicated-statement-into-arms shape (.claude/rules/duplicated-statement-into-arms.md). candidate.c is now this form; the s10 round-trip is retired.
- verdict: CONFIRMED

## [s11] Compressing the zero-pixel arms to `*dst++ = pixel;` is not a neutral rewrite of `*dst = pixel; src++; dst++;`.
- mechanism: the post-increment form lets combine fold the store's address and the increment into one addressing computation per arm, which removes the two insns cross-jump would otherwise have merged.
- probe: tmp/grind/func_8003DE14/s11/a2.c, sandbox --disable all.
- result: KILLED. 171 build_insns against the target's 173, score 35 (worse than a1's 29). The split store/increment is load-bearing for the instruction budget. Banked as rejected/s11-star-dst-plusplus-in-zero-arms-folds-two-insns-35.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/a2.c

## [s11] Hoisting `complement` out of the inner loop in the C source lowers neither its reference count nor its allocation rank, because loop.c had already hoisted the set into the preheader.
- mechanism: `blend_base - factor` is loop-invariant in the pixel loop, so LICM moves its set to the preheader before flow_analysis runs; the weighted reference count that reaches allocno_compare is therefore already the hoisted one (set at depth 2 = 2, three multiplies at depth 3 = 9, total 11) whether or not the C writes it outside.
- probe: tmp/grind/func_8003DE14/s11/b1.c (complement declared after `s32 j = 0;`) and b2.c (declared before it); sandbox plus BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s11/d_b1/stderr.log).
- result: KILLED. Both score 45 (against a1's 29) at 173 insns. ALLOCDBG on b1 shows complement (pseudo 116) still at nrefs 11, with the live lengths moving to j 56 / complement 55 - the gap narrows to one insn but the order does not flip, and the preheader arithmetic moves enough to cost 16 score. Banked as rejected/s11-complement-hoisted-out-of-inner-loop-45.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1-derived b1/b2 chassis (45 / 173 insns), no FAKE constructs present, dumps tmp/grind/func_8003DE14/s11/d_b1/

## [s11] Shortening j's live range by moving `j = 0;` inside `if (total > 0)` closes the j/complement priority gap to one insn but does not flip the seat, and costs two instructions of score elsewhere.
- mechanism: with j and complement both pinned at nrefs 11, allocno_compare (global.c:635) reduces to a pure live-length comparison, and the tie-break at global.c:652-653 is ascending allocno (j is pseudo 115, complement 116), so j needs only to TIE. Moving the initialisation past the `total > 0` test removes the preheader insns from j's range.
- probe: tmp/grind/func_8003DE14/s11/jc/C1.c (plus C4 j-declared-first, C6 both, D2 C1 with `while (++j < ...)`); sandbox and BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s11/d_C1/stderr.log).
- result: KILLED for these spellings. C1 moves j's live_length 59 -> 55 against complement's 54, i.e. pri 6000 vs 6111 - one insn short of the tie that would seat j first - and the score regresses 29 -> 31 because the preheader `move t4,zero` shifts. C4 and C6 also score 31, D2 scores 31. Banked as rejected/s11-j-init-inside-if-livelen-55-still-one-short-31.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, dumps tmp/grind/func_8003DE14/s11/d_C1/

## [s11] Duplicating `j++` into the three arms the way `dst++` now is does not reproduce the dst result: cross-jump cannot re-merge the copies.
- mechanism: the dst duplication merges because everything below `dst++` is identical on all three paths (`addiu j,1 / slt / bne`), which clears the two-matching-insn minimum at jump.c:2020. Once `j++` is itself duplicated, the arms' tails diverge above the increment and the merge window collapses.
- probe: tmp/grind/func_8003DE14/s11/jd/D3.c, sandbox --disable all.
- result: KILLED. 175 build_insns against the target's 173 and score 47. Also confirms the lift would have overshot: j at 23 weighted refs prices to 15593, far above complement's 6111 and above several unrelated allocnos, so even a free spelling would have moved j out of the $t4/$t5 band. Banked as rejected/s11-j-increment-duplicated-into-arms-175-insns-47.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/jd/D3.c

## [s11] Reusing j as the outer tail's `new_y` carrier lengthens j's live range past the inner loop but breaks the outer-loop codegen.
- mechanism: j is dead after the inner loop, so borrowing it for the `((u16*)rect)[1] + ((u16*)rect)[3]` value is a real, sanctioned variable reuse; the intent was to add depth-2 references and extend j's live_length past complement's.
- probe: tmp/grind/func_8003DE14/s11/jd/D1.c, sandbox --disable all.
- result: KILLED. Score 41 at 172 build_insns - one instruction short of the target and 12 score worse than a1. Banked as rejected/s11-j-reused-as-new-y-carrier-outer-tail-41.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/jd/D1.c

## [s11] Reshaping the blend arm at source level does not move the residual on the corrected-cursor-seat chassis.
- mechanism: the blend residual (target insns 88-123) is a register-naming and emission-order difference - the target holds `px` in $a0, the red product in $a1 and the blue product in $t7 where we use $v1/$a1/$v1, and it runs the blue channel's srl/andi/mult before the mflo we run first. If that were an expand-order consequence of the C, permuting the C's channel order, temp naming, staging and operand order would move it.
- probe: twelve structurally distinct blend bodies swept in one sweep_variants call (tmp/grind/func_8003DE14/s11/bl/B1..B12.c): extractions inlined into the multiplies, products staged into named temps then shifted in the OR, b_src extracted first, b channel computed first, g channel computed first, an OR accumulator (`out |= ...`), `src++` moved to the end of the arm, b's extraction inlined only, `px` instead of `pixel` in the red extraction, a pre-masked `b_ch`, a re-associated OR, and `ch * factor + x_src * complement` operand order.
- result: KILLED for this spelling space. Best = 29, i.e. no variant beat the incumbent; B1/B3/B7/B8 tie at 29, B9/B10/B11 are 30, and the rest run 34-53. This re-confirms s5's negative blend sweep, now on the chassis where the cursors are correctly seated, and points the blend residual at the scheduler / allocation order rather than at the source shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s11/bl/B1..B12.c

## [s11] The +12 weighted-reference lift on the dst pseudo that seats the cursors like the target has an ordinary-C spelling: dst++ duplicated into each of the three inner-loop arms instead of a shared advance_dst: label reached by goto.
- mechanism: flow.c:2081 does reg_n_refs[regno] += loop_depth and the pixel loop is at depth 3, so one dst++ (a read and a write of pseudo 109) is 6 weighted references. Replacing the single shared copy with three arm-local copies takes that 6 to 18, i.e. dst's weighted reg_n_refs from 26 to 38 - exactly the count the s10 dst++/dst-- round-trip produced. allocno_compare (global.c:635) then ranks dst 38/60 above src 32/61 and find_reg's ascending scan hands the first-allocated cursor $a2. The lift is free of emitted cost because jump2's cross-jump tail-merges the three copies back: the shared tail is addiu dst,1 / addiu j,1 / slt / bne, four matching insns against the two-insn minimum at jump.c:2020.
- probe: tmp/grind/func_8003DE14/s11/a1.c (the s10 chassis with the label and both gotos deleted and dst++ written into each arm), sandbox func_8003DE14 --disable all, plus BB2_ALLOC_DEBUG=1 on the instrumented cc1 (tmp/grind/func_8003DE14/s11/d_a1/stderr.log).
- result: CONFIRMED. Score 29 / build_insns 173 / target_insns 173 - the same floor and the same instruction count as the s10 form. ALLOCDBG: ord=3 pseudo=109 hardreg=6 nrefs=38 livelen=60 pri=31666 (dst -> $a2) and ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=61 pri=26229 (src -> $a3). No net-zero statement, no dead store and nothing FAKE-family remains anywhere in the body; the construct is the sanctioned duplicated-statement-into-arms shape (.claude/rules/duplicated-statement-into-arms.md). memory/grind/func_8003DE14/candidate.c is now this form and the s10 round-trip is retired.
- verdict: CONFIRMED

## [s11] Compressing the zero-pixel arms to *dst++ = pixel; is not a neutral rewrite of *dst = pixel; src++; dst++; on the a1 chassis.
- mechanism: The post-increment form lets combine fold the store's address and the increment into one addressing computation per arm, which removes the two insns cross-jump would otherwise have merged, so the emitted stream falls two instructions below the target's 173.
- probe: tmp/grind/func_8003DE14/s11/a2.c, sandbox --disable all.
- result: KILLED. 171 build_insns against the target's 173, score 35 (a1 is 29). The split store/increment is load-bearing for the instruction budget. Banked as memory/grind/func_8003DE14/rejected/s11-star-dst-plusplus-in-zero-arms-folds-two-insns-35.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/a2.c

## [s11] Hoisting complement out of the inner loop in the C source lowers neither its reference count nor its allocation rank on the a1 chassis, because loop.c had already hoisted the set into the preheader.
- mechanism: blend_base - factor is loop-invariant in the pixel loop, so LICM moves its set to the preheader before flow_analysis runs; the weighted reference count that reaches allocno_compare is therefore already the hoisted one (set at depth 2 = 2, three multiplies at depth 3 = 9, total 11) whether or not the C writes it outside.
- probe: tmp/grind/func_8003DE14/s11/b1.c (complement declared after s32 j = 0;) and b2.c (declared before it); sandbox plus BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s11/d_b1/stderr.log).
- result: KILLED. Both score 45 (a1 is 29) at 173 insns. ALLOCDBG on b1 shows complement (pseudo 116) still at nrefs 11, with the live lengths moving to j 56 / complement 55 - the gap narrows to one insn but the order does not flip, and the preheader arithmetic moves enough to cost 16 score. Banked as rejected/s11-complement-hoisted-out-of-inner-loop-45.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1-derived b1/b2 chassis (45 / 173 insns), no FAKE constructs present, dumps tmp/grind/func_8003DE14/s11/d_b1/

## [s11] Shortening j's live range by moving j = 0; inside if (total > 0) closes the j/complement priority gap to one insn but does not flip the seat, and costs two instructions of score elsewhere.
- mechanism: With j and complement both pinned at nrefs 11, allocno_compare (global.c:635) reduces to a pure live-length comparison, and the tie-break at global.c:652-653 is ascending allocno (j is pseudo 115, complement 116), so j needs only to TIE. Moving the initialisation past the total > 0 test removes the preheader insns from j's range.
- probe: tmp/grind/func_8003DE14/s11/jc/C1.c, plus C4 (j declared first), C6 (both) and D2 (C1 with while (++j < ...)); sandbox and BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s11/d_C1/stderr.log).
- result: KILLED for these spellings. C1 moves j's live_length 59 -> 55 against complement's 54, i.e. pri 6000 vs 6111 - one insn short of the tie that would seat j first - and the score regresses 29 -> 31 because the preheader move t4,zero shifts. C4, C6 and D2 all score 31. Banked as rejected/s11-j-init-inside-if-livelen-55-still-one-short-31.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, dumps tmp/grind/func_8003DE14/s11/d_C1/

## [s11] Duplicating j++ into the three arms the way dst++ now is does not reproduce the dst result on the a1 chassis: cross-jump cannot re-merge the copies.
- mechanism: The dst duplication merges because everything below dst++ is identical on all three paths (addiu j,1 / slt / bne), which clears the two-matching-insn minimum at jump.c:2020. Once j++ is itself duplicated the arms' tails diverge above the increment and the merge window collapses.
- probe: tmp/grind/func_8003DE14/s11/jd/D3.c, sandbox --disable all.
- result: KILLED. 175 build_insns against the target's 173 and score 47. It also confirms the lift would have overshot: j at 23 weighted refs prices to 15593, far above complement's 6111 and above several unrelated allocnos, so even a free spelling would have moved j out of the $t4/$t5 band. Banked as rejected/s11-j-increment-duplicated-into-arms-175-insns-47.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/jd/D3.c

## [s11] Reusing j as the outer tail's new_y carrier lengthens j's live range past the inner loop but breaks the outer-loop codegen on the a1 chassis.
- mechanism: j is dead after the inner loop, so borrowing it for the ((u16*)rect)[1] + ((u16*)rect)[3] value is a real, sanctioned variable reuse; the intent was to add depth-2 references and push j's live_length past complement's so the pair flips.
- probe: tmp/grind/func_8003DE14/s11/jd/D1.c, sandbox --disable all.
- result: KILLED. Score 41 at 172 build_insns - one instruction short of the target and 12 score worse than a1. Banked as rejected/s11-j-reused-as-new-y-carrier-outer-tail-41.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s11/jd/D1.c

## [s11] Reshaping the blend arm at source level does not move the residual on the corrected-cursor-seat 29 chassis: twelve structurally distinct spellings all score 29 or worse.
- mechanism: The blend residual (target insns 88-123) is a register-naming and emission-order difference - the target holds px in $a0, the red product in $a1 and the blue product in $t7 where we use $v1/$a1/$v1, and it runs the blue channel's srl/andi/mult before the mflo we run first. If that were an expand-order consequence of the C, permuting the C's channel order, temp naming, staging and operand order would move it.
- probe: Twelve blend bodies swept in one sweep_variants call (tmp/grind/func_8003DE14/s11/bl/B1..B12.c): extractions inlined into the multiplies, products staged into named temps then shifted in the OR, b_src extracted first, b channel computed first, g channel computed first, an OR accumulator, src++ moved to the end of the arm, b's extraction inlined only, px instead of pixel in the red extraction, a pre-masked b_ch, a re-associated OR, and ch * factor + x_src * complement operand order.
- result: KILLED for this spelling space. Best = 29, i.e. no variant beat the incumbent; B1/B3/B7/B8 tie at 29, B9/B10/B11 are 30 and the rest run 34-53. This re-confirms s5's negative blend sweep, now on the chassis where the cursors are correctly seated, and points the blend residual at the scheduler / allocation order rather than at the source shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, a1 chassis (29 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s11/bl/B1..B12.c

## [s12] The j/complement seat is decided ENTIRELY by the distance between the two set insns in the inner loop's preheader, and LICM guarantees complement loses that race whenever the subtraction is written inside the loop body.
- mechanism: j (pseudo 115) and complement (pseudo 116) both carry nrefs 11, so allocno_compare (tools/gcc-2.7.2/global.c:635-648) reduces to pri = 33/live_length and global.c:652-653 breaks an exact tie on ascending allocno (j wins a tie). Both pseudos are set only in the preheader and read on every iteration, so each is live over the ENTIRE pixel-loop body; their live lengths therefore differ by exactly the number of insns between their two set insns - nothing about where complement is USED inside the loop can change either number (which is why s11's twelve blend reshapings never moved this pair). loop.c's scan_loop appends a hoisted invariant at the END of the preheader, immediately before loop_start, so a LICM-hoisted `complement` set is always BELOW `j = 0` and always shorter-lived.
- probe: six placements measured with sandbox --disable all plus BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1 (tmp/grind/func_8003DE14/s12/d_*/stderr.log): base/s11 (complement inside the do-body, j = 0 in the outer body), W2 (complement written inside `if (total > 0)`, then `j = 0;`), W5 (both declared inside the if, complement first), W6 (complement inside the if, j = 0 still outside), W7 (j declared at the top of the do-body), H2 (j declared before complement inside the if).
- result: CONFIRMED, and it drops the floor 29 -> 28. Measured live lengths (j/complement): base 59/54, W6 59/54, H2 55/54, W2 54/55, W5 55/54 with the declaration order swapping the pseudo numbers, W7 54/55. Only the placements that put complement's set ABOVE `j = 0` inside the guard flip the seat: W2/W5/W7 all print `ord=15 ... hardreg=12 nrefs=11 livelen=54 pri=6111` for j and `ord=16 ... hardreg=13 livelen=55 pri=6000` for complement, i.e. j -> $t4 and complement -> $t5, the TARGET's seat. Aligned diff: target insns 70/71, 131 and 133 are now byte-exact. Score 28 at 173 build_insns / 173 target_insns. candidate.c is now the W5 spelling.
- verdict: CONFIRMED

## [s12] Writing `complement` in the outer do-body (above `j = 0`) also flips the seat, but costs 17 score because it moves the subtraction above the `blez` guard and stretches `factor`'s live range.
- mechanism: the flip works for the same live-length reason (complement's set is above j's), but the target emits `subu t5,s8,t3` at insn 71, i.e. INSIDE the guarded block after the `blez`. Writing it in the outer body puts it before the branch and lengthens factor (pseudo 110) from 57 to 62.
- probe: tmp/grind/func_8003DE14/s12/W9.c (complement declared immediately before `s32 j = 0;` in the outer do-body) and W10.c (complement in the outer body, `j = 0` inside the if); sandbox plus BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s12/d_W9/stderr.log).
- result: KILLED. W9 scores 45 and W10 42, both at 173 insns, against W5's 28. ALLOCDBG for W9: j 115 livelen 55 pri 6000 at ord=15 hardreg=12 (the seat DOES flip) and complement 116 livelen 56 pri 5892, with factor livelen 62. This re-confirms s11's b1/b2 result with the seat instrumentation attached: the flip is not the expensive part, the subtraction's position relative to the guard branch is. Banked as rejected/s12-complement-in-outer-body-before-j-flips-seat-but-45.c and rejected/s12-complement-outer-j-inside-if-no-flip-42.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/W9.c and W10.c

## [s12] The remaining head residual and the correct cursor seat are directly coupled on this chassis: restoring the target's `blez` delay slot costs the seat.
- mechanism: reorg.c fills the guard branch's delay slot with the closest movable insn preceding the branch. In the s11 body that insn was `move t4,zero` (`j = 0` was the last statement before the `if`), which is what the target emits at insn 70. Moving `j = 0` below the branch - the very move that flips the seat - leaves `addiu a2,sp,1040` (the `dst = dst_buf` init the target emits at insn 54) as the closest candidate, so reorg sinks it into the slot instead. The four-insn head difference (target 54 deleted, our 70/71 inserted, target 71 deleted) is exactly that exchange.
- probe: tmp/grind/func_8003DE14/s12/H2.c (`s32 j = 0;` declared BEFORE `s32 complement` inside the guard, so the delay slot is available again), plus H1.c (dst declared before src) and H3.c (total computed last in the outer body) as placement controls; sandbox --disable all; aligned diff via tmp/grind/func_8003DE14/s12/sbs.sh.
- result: KILLED for these spellings. H2 restores the target's delay slot and loses the seat, scoring 31; H1 and H3 both score 28, i.e. neither denying reorg the `addiu a2` candidate (H1) nor moving the guard's operand computation (H3) recovers the slot while the seat is held. Banked as rejected/s12-j-declared-before-complement-keeps-delay-slot-loses-seat-31.c and rejected/s12-complement-inside-if-j-outside-no-flip-29.c (W6, the no-flip control at 29).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/H1.c H2.c H3.c W6.c

## [s12] Re-running s11's twelve blend-arm reshapings on the corrected-seat 28 chassis does not move the blend residual.
- mechanism: if the blend arm's register naming and its blue-channel srl/andi/mult position were an expand-order consequence of the C, the s11 sweep's negative result might have been an artefact of the wrong cursor/counter seat underneath it. Rebasing all twelve bodies onto the W5 placement tests that directly.
- probe: tmp/grind/func_8003DE14/s12/B1w.c .. B12w.c (s11's bl/B1..B12 with the W5 placement applied), one sandbox --disable all each.
- result: KILLED. B1w/B3w/B7w/B8w tie the incumbent at 28, B9w/B10w/B11w are 29, and B2w/B4w/B5w/B6w/B12w run 33-54. No variant beats 28, so the blend residual is independent of the seat fix and remains a scheduler/allocation-order question rather than a source-shape one - the same conclusion s11 reached, now verified on the chassis where j, complement, src and dst are all seated like the target.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/B1w..B12w.c

## [s12] The trip-test residual (target 127-130) does not respond to the C spelling of `rect[2] * rect[3]`: the $v0/$v1 naming on the two loads is invariant under operand order, comparison direction and explicit widening.
- mechanism: the loop-bottom recompute is `lh <a>,4(s0) / lh <b>,6(s0) / mult <a>,<b>` in both streams - same loads, same order, same mult operand order - with only the two destination registers swapped (target v0 then v1, ours v1 then v0). Both pseudos are born and die inside the loop-bottom block, so they are local-alloc quantities rather than global allocnos; if their hard registers followed the source at all, changing which operand is written first would move them.
- probe: tmp/grind/func_8003DE14/s12/T1.c (`while (j < rect[3] * rect[2])`), T2.c (`while (rect[2] * rect[3] > j)`), T3.c (`while (j < (s32)rect[2] * (s32)rect[3])`); sandbox --disable all on each plus the aligned diff from s12/sbs.sh.
- result: KILLED for these spellings. All three score 28, i.e. none beats the incumbent. The aligned diff shows the naming is untouched in every case: T2 and T3 reproduce the incumbent's `lh v1,4(s0) / lh v0,6(s0) / mult v1,v0` exactly, and T1 merely swaps which address is loaded first (our stream becomes `lh v1,6(s0)` then `lh v0,4(s0)`, moving AWAY from the target's load order) while still naming the 4(s0) destination v1. The rows immediately above (target 122/123, `or v0,v0,v1` / `andi v1,a0,K` against our `or v0,v0,a0` / `andi v1,v1,K`) are unchanged across all three, which points at this cluster being a downstream consequence of the blend arm's $v0/$v1 pressure rather than an independent sub-problem. Banked as rejected/s12-trip-test-operands-swapped-loads-reorder-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/T1.c T2.c T3.c

## [s12] The j/complement seat is decided entirely by the distance between the two pseudos' set insns in the inner loop's preheader, and a LICM-hoisted `complement` set is always placed below `j = 0`, so writing the subtraction inside the `if (total > 0)` guard above `j = 0` flips the seat to the target's.
- mechanism: j (pseudo 115) and complement (pseudo 116) both carry weighted nrefs 11, so allocno_compare (tools/gcc-2.7.2/global.c:635-648) collapses to pri = 33/live_length with global.c:652-653 breaking an exact tie on ascending allocno. Both are set only in the preheader and read every iteration, so both are live across the entire loop body: that constant cancels and only the preheader distance between their set insns matters. loop.c's scan_loop appends a hoisted invariant at the END of the preheader (immediately before loop_start), so a hoisted complement set always sits below `j = 0` and always has the shorter live range. Writing the subtraction inside the guard leaves LICM nothing to hoist and lets statement order put `j = 0` below it.
- probe: Six placements measured with `sandbox func_8003DE14 --disable all` plus BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1: base/s11, W2, W5, W6, W7 and H2 (tmp/grind/func_8003DE14/s12/*.c, dumps in s12/d_*/stderr.log), with the aligned object diff from tmp/grind/func_8003DE14/s12/sbs.sh.
- result: CONFIRMED and the floor drops 29 -> 28. Live lengths j/complement: base 59/54, W6 59/54, H2 55/54 (all complement-first, wrong seat), W2 54/55, W5 55/54 by pseudo with the numbering swapped, W7 54/55 (all j-first, right seat). W2/W5/W7 print ord=15 hardreg=12 pri=6111 for j and ord=16 hardreg=13 pri=6000 for complement, i.e. j -> $t4 and complement -> $t5. Target insns 70/71, 131 and 133 are now byte-exact; score 28 at 173 build_insns / 173 target_insns. candidate.c is the W5 spelling and is ordinary C - no annotation-bearing construct anywhere in the body.
- verdict: CONFIRMED

## [s12] Writing `complement` in the outer do-body above `j = 0` also flips the seat but scores 45, because it puts the subtraction above the `blez` guard where the target emits it below, and stretches factor's live range from 57 to 62.
- mechanism: The flip happens for the same live-length reason, but the target emits `subu t5,s8,t3` at insn 71, inside the guarded block. Hoisting it into the outer body moves it across the branch and lengthens factor (pseudo 110), which re-prices several neighbouring allocnos.
- probe: tmp/grind/func_8003DE14/s12/W9.c (complement immediately before `s32 j = 0;` in the outer do-body) and W10.c (complement in the outer body, `j = 0` inside the guard); sandbox --disable all plus BB2_ALLOC_DEBUG (s12/d_W9/stderr.log, s12/d_W10/stderr.log).
- result: KILLED. W9 scores 45 and W10 42, both at 173 insns, against W5's 28. W9's ALLOCDBG confirms the seat DOES flip (j 115 at ord=15 hardreg=12, livelen 55 pri 6000; complement 116 livelen 56 pri 5892) with factor at livelen 62 - so the cost is the subtraction's position relative to the guard branch, not the flip. This re-confirms s11's b1/b2 kill with the seat instrumentation attached. Banked as rejected/s12-complement-in-outer-body-before-j-flips-seat-but-45.c and rejected/s12-complement-outer-j-inside-if-no-flip-42.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10 (post -mel, post -msoft-float), W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/W9.c and W10.c

## [s12] On this chassis the remaining head residual and the correct counter seat are directly coupled: the spellings that give reorg back the target's `blez` delay slot are the spellings that lose the seat.
- mechanism: reorg.c fills the guard branch's delay slot with the closest movable insn preceding the branch. In the s11 body that insn was `move t4,zero`, which is what the target emits at insn 70. Moving `j = 0` below the branch - the move that flips the seat - leaves `addiu a2,sp,1040` (the `dst = dst_buf` init the target emits at insn 54) as the closest candidate, so reorg sinks that into the slot instead; the four-insn head difference is exactly that exchange.
- probe: tmp/grind/func_8003DE14/s12/H2.c (`s32 j = 0;` declared before `s32 complement` inside the guard), H1.c (dst declared before src, denying reorg its preferred candidate), H3.c (total computed last in the outer body) and W11.c (dst declared inside the guard); sandbox --disable all on each plus the aligned diff.
- result: KILLED for these spellings. H2 restores the target's delay slot and loses the seat, scoring 31. H1 and H3 both hold the seat and stay at 28; W11 scores 30 - neither changing which cursor init precedes the branch nor moving the guard's operand computation recovers the slot. Banked as rejected/s12-j-declared-before-complement-keeps-delay-slot-loses-seat-31.c, rejected/s12-complement-inside-if-j-outside-no-flip-29.c (the W6 no-flip control) and rejected/s12-dst-declared-inside-if-30.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/H1.c H2.c H3.c W6.c W11.c

## [s12] Re-running s11's twelve blend-arm reshapings on top of the corrected-seat 28 chassis does not move the blend residual, so the blend difference is independent of the cursor and counter seats.
- mechanism: If the blend arm's register naming and its blue-channel srl/andi/mult position were an expand-order consequence of the C, s11's negative sweep could have been an artefact of the wrong seat underneath it. Rebasing all twelve bodies onto the W5 placement tests that directly.
- probe: tmp/grind/func_8003DE14/s12/B1w.c .. B12w.c (s11's bl/B1..B12 with the W5 placement applied), one `sandbox --disable all` each.
- result: KILLED. B1w/B3w/B7w/B8w tie the incumbent at 28, B9w/B10w/B11w are 29, B2w/B4w/B5w/B6w/B12w run 33-54. No variant beats 28. The blend residual survives the seat fix unchanged, which confirms s11's attribution: it is a scheduler / allocation-order question, not a source-shape one.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/B1w..B12w.c

## [s12] The trip-test residual (target 127-130) does not respond to the C spelling of `rect[2] * rect[3]`: the $v0/$v1 naming on the two lh destinations is invariant under operand order, comparison direction and explicit widening.
- mechanism: The loop-bottom recompute is `lh <a>,4(s0) / lh <b>,6(s0) / mult <a>,<b>` in both streams - same loads, same order, same mult operand order - with only the two destination registers swapped (target v0 then v1, ours v1 then v0). Both pseudos are born and die inside the loop-bottom block, so they are local-alloc quantities rather than global allocnos; if their hard registers followed the source at all, changing which operand is written first would move them.
- probe: tmp/grind/func_8003DE14/s12/T1.c (`while (j < rect[3] * rect[2])`), T2.c (`while (rect[2] * rect[3] > j)`) and T3.c (`while (j < (s32)rect[2] * (s32)rect[3])`); sandbox --disable all on each plus the aligned diff from s12/sbs.sh.
- result: KILLED for these spellings. All three score 28 and none beats the incumbent. T2 and T3 reproduce the incumbent's `lh v1,4(s0) / lh v0,6(s0) / mult v1,v0` exactly; T1 merely swaps which address loads first (our stream becomes `lh v1,6(s0)` then `lh v0,4(s0)`, moving AWAY from the target's load order) while still naming the 4(s0) destination v1. The two rows immediately above (target 122 `or v0,v0,v1` / 123 `andi v1,a0,K` against our `or v0,v0,a0` / `andi v1,v1,K`) are unchanged across all three, which points at this cluster being a downstream consequence of the blend arm's $v0/$v1 pressure rather than an independent sub-problem. Banked as rejected/s12-trip-test-operands-swapped-loads-reorder-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-10, W5 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s12/T1.c T2.c T3.c

## s13 (structural) — floor 28 -> 26

**CONFIRMED — the blend arm has no scheduling residual.** Raw index-by-index
comparison (tmp/grind/func_8003DE14/s13/sxs.py) of target vs build shows
identical opcodes in identical slots across 84..131; only register names differ.
s12's frontier claim of a mis-ordered blue channel was a difflib alignment
artifact. Do NOT spend a session on tools/sched_solver for this block.

**CONFIRMED — one C variable = one hard register (GCC 2.7.2, no SSA).**
Reusing each `X_src` local to hold that channel's `X * factor` product forces the
shifted source component and the product into one register, which is what the
target does ($a1 for the r channel at insns 94/95/98). Spelled with a named
per-channel `rp/gp/bp` for the complement product, this measures 26 (form C2) —
a 2-point drop and the first movement on the blend arm in three sessions.

**KILLED (instance) — `px` as a user variable vs a compiler temp.** Deleting the
`s32 px = pixel & 0xFFFF;` local entirely (D1: `if (pixel == 0)`,
`(pixel >> 2) & 0xF8`, `(pixel >> 7) & 0xF8`) produces a byte-identical object to
C2 at 26. D5 (explicit `(u32)` casts) likewise 26. Measured on HEAD 2026-09-10,
C2 chassis (26 / 173 insns), no FAKE constructs present.

**KILLED (instance) — changing px's reference count from the source.** Using
`px` for the 0x1F extraction (D3) scores 27; using `px` for the 0x8000 alpha
mask (D2) scores 40 and inflates the build to 174 insns. Same chassis, no FAKE.

**KILLED (instance) — eleven neighbouring spellings of the channel-variable
reuse.** C1/C3/C4/C5/C6/C7/F1/F2/F3/F5 score 43/44/40/38/44/54/40/27/43/59; only
C2's exact shape (named complement product + src-var reused for the factor
product, per channel, sequential) reaches 26. F4 (lazy g_src/b_src declarations)
ties at 26. Same chassis, no FAKE.

**KILLED (instance) — six statement-order spellings for the head delay slot.**
E1 (j=0 before complement inside the guard) 30, E2 (dst initialised inside the
guard) 28, E3 32, E4 (total computed last) 26, E5 (dst declared before src) 26,
E6 (`if (rect[2] * rect[3] > 0)`) 26 — E4/E5/E6 byte-identical to C2. Measured on
HEAD 2026-09-10, C2 chassis, no FAKE constructs present.

## [s13] The blend arm's instruction order already matches the target exactly; the entire blend residual is register naming, not scheduling.
- mechanism: A raw index-by-index side-by-side of the target object stream against the sandbox object stream (tmp/grind/func_8003DE14/s13/sxs.py, no difflib) shows target[84..131] and ours[84..131] carrying identical opcodes in identical slots. s12's frontier claim that the blue channel's srl/andi/mult runs at target 104-107 where we run it at 109-112 came from sbs2.py's SequenceMatcher inserting a del/ins block around a pure rename.
- probe: sxs.py 84 132 on the s12 (W5) chassis and again on the s13 (C2) chassis.
- result: Opcode-for-opcode identical across the whole blend arm on both chassis. Confirms there is nothing for tools/sched_solver to solve in this block and retires s12 frontier item 3.
- verdict: CONFIRMED

## [s13] Writing a channel's shifted source component and that channel's X*factor product into ONE C local forces them into one hard register, matching the target's per-channel register reuse.
- mechanism: GCC 2.7.2 has no SSA: a non-address-taken C local is exactly one pseudo for its whole scope, so a second assignment to r_src makes the product share r_src's hard register. Target insns 94/95/98 are sll a1,v0,0x3 / mult a1,t5 / mflo a1 - one register ($a1) carrying both r-channel values, where the s12 body produced two pseudos in $v0 and $a1.
- probe: Form C2 (tmp/grind/func_8003DE14/s13/C2.c): per-channel 'rp = r_src * complement; r_src = r * factor; r_ch = ((rp + r_src) >> 15) & 0x1F;' for r, g and b.
- result: score 26 (was 28), build_insns 173, target_insns 173. Target rows 94 and 95 are now byte-exact. First movement on the blend arm in three sessions. Saved as memory/grind/func_8003DE14/candidate.c.
- verdict: CONFIRMED

## [s13] Deleting the px user variable so the zero-extended pixel exists only as a compiler temp changes px's register seat.
- mechanism: px is pseudo 122, a global allocno (live across the bnez at 89) with nrefs 12 / livelen 11 / pri 32727, allocated 3rd and taking hardreg 3 ($v1) where the target has $a0. The hypothesis was that a user-variable pseudo (reg/v) versus a CSE temp changes its birth order and therefore its priority.
- probe: Form D1: 's32 px = pixel & 0xFFFF;' deleted, 'if (pixel == 0)', '(pixel >> 2) & 0xF8', '(pixel >> 7) & 0xF8' written on the u16 directly. Form D5: D1 with explicit (u32) casts.
- result: D1 scores 26 with an aligned diff row-for-row IDENTICAL to C2's; D5 also 26. The pseudo's user-variable status is byte-neutral here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2 chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s13/D1.c and D5.c

## [s13] Changing px's source-level reference count moves its seat toward the target's $a0.
- mechanism: px's global allocno priority is nrefs*K/livelen; adding or removing a use changes nrefs and could sink px below an allocno that would then take $v1.
- probe: D3 uses px for the 0x1F red extraction (one more ref); D2 uses px for the 0x8000 alpha mask (one more ref, longer live range).
- result: D3 = 27, D2 = 40 AND inflates the build to 174 insns. Both worse; neither moves px off $v1.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2 chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s13/D2.c and D3.c

## [s13] A neighbouring spelling of the channel-variable reuse beats C2's exact shape.
- mechanism: The reuse can be spelled several ways: self-multiply into the src var, full accumulate, naming the factor product instead of the complement product, folding the sums into the final or-chain, or mixing per channel to imitate the target's r=$a1 / g=$v1 / b=$a0 assignment.
- probe: Eleven forms measured on the C2 chassis: C1 (sums inline in the or chain), C3 (r_src *= complement), C4 (r,g reuse + b self-multiply), C5 (full accumulate), C6 (C3 with src++ late), C7 (factor products named first), F1 (b self-multiply only), F2 (g self-multiply only), F3 (C2 sums inlined), F4 (lazy g_src/b_src declarations), F5 (factor first then self-multiply).
- result: 43 / 44 / 40 / 38 / 44 / 54 / 40 / 27 / 43 / 26 / 59 respectively, all at 173 insns. Only F4 ties C2 at 26; every other spelling is worse. C2's shape (named complement product, src var reused for the factor product, per channel, sequential) is the local optimum.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2 chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s13/{C1,C3,C4,C5,C6,C7,F1,F2,F3,F4,F5}.c

## [s13] A statement-order change in the outer do-body restores the target's blez delay slot without losing the j/complement seat.
- mechanism: Target keeps 'addiu a2,sp,1040' (dst = dst_buf) at insn 54 and fills the blez delay slot at 70 with 'move t4,zero' (j = 0) taken from the fall-through thread, with 'subu t5,s8,t3' (complement) at 71. Our build hands reorg 'addiu a2,sp,1040' instead. For reorg to take j = 0 from the fall-through it must be the FIRST insn after the branch, i.e. above complement's set; but for j to keep $t4 its set must be BELOW complement's (equal nrefs, so the shorter live length wins the global.c priority race).
- probe: Six forms: E1 (j = 0 first, complement second, both inside the guard), E2 (C2 + dst initialised inside the guard), E3 (E1 + dst inside the guard), E4 (total computed after src/dst/factor), E5 (dst declared before src), E6 (total deleted, guard written 'if (rect[2] * rect[3] > 0)').
- result: E1 = 30, E2 = 28, E3 = 32, E4 = 26, E5 = 26, E6 = 26 - all at 173 insns. E4/E5/E6 are byte-identical to C2, so moving or deleting 'total' and swapping the src/dst declaration order does not change reorg's pick. E2, which denies reorg the addiu a2 candidate by sinking dst into the guard, costs 2 rather than gaining.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2 chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s13/E1.c..E6.c

## s14 (structural) — floor 26, unchanged; the px seat is now mechanically located

**The px/g_src swap is the WHOLE blend residual, and it is a global.c ordering
question with an exact arithmetic.** ALLOCDBG on the C2 (candidate) chassis,
tmp/grind/func_8003DE14/s14/d_aac/stderr.log:

    ord=0 pseudo=118 hardreg=2  ($v0) nrefs=9  livelen=3  pri=90000   <- count-1
    ord=1 pseudo=160 hardreg=65 (LO)
    ord=2 pseudo=122 hardreg=3  ($v1) nrefs=12 livelen=11 pri=32727   <- px
    ord=3 pseudo=109 hardreg=6  ($a2)                                  <- dst
    ord=4 pseudo=126 hardreg=4  ($a0) nrefs=12 livelen=12 pri=30000   <- g_src
    ord=5 pseudo=123 hardreg=5  ($a1) nrefs=12 livelen=13 pri=27692   <- r_src

pri = nrefs*30000/livelen (global.c:635-653); nrefs is the depth-3-weighted
reg_n_refs, so ONE source reference is worth 3. px is allocated 3rd, finds $v0
already taken by a CONFLICTING allocno (p118, `count - 1`), and takes $v1.
r_src already sits on the target's $a1. The target's seats are px=$a0,
g_src=$v1, r_src=$a1 — i.e. exactly "g_src allocated before px".

The arithmetic for that flip is now exact: g_src needs pri > 32727, i.e.
livelen <= 10 at nrefs 12 (36000) or nrefs >= 14 at livelen 12 (35000); or px
needs pri < 30000, i.e. livelen >= 13 at nrefs 12 (27692, and the resulting tie
with r_src is won by px because allocno order breaks ties on ascending pseudo
number, 122 < 123 — which is the target's order g_src, px, r_src).

**KILLED (instance) — the full 3x3 per-channel reuse lattice.** Each channel
independently spelled A (named complement product + src var reused for the
factor product = the C2 shape), B (src var self-multiplied by complement +
named factor product) or C (both products named, no reuse); all 27 combinations
measured. Minimum is 26 at AAA (the incumbent) and AAC; everything else is
27-44. AAC is byte-DIFFERENT from AAA but scores the same, and it reproduces the
target's b-channel REGISTER pattern (bp lands in b_src's register, the factor
product lands elsewhere) — so the b channel's remaining rows are pure naming
too. Forms tmp/grind/func_8003DE14/s14/G_*.c.

**KILLED (instance) — splitting a channel's source variable does NOT make it a
competing global allocno.** ACA (g spelled C) scores 38 and its ALLOCDBG
(s14/d_aca/stderr.log) shows NO short-lived g_src allocno at all: born and dead
inside one basic block, it becomes a local-alloc QUANTITY, and local-alloc seats
it on $v0 (coalesced with the dying `srl` temp that feeds it) rather than on
$v1. px is still ord=2 on $v1. The surviving global (p128, the g*factor product)
takes $v0. So "make the g source short-lived" does not put a conflicting holder
on $v1.

**KILLED (instance) — all six channel-block orderings.** r,g,b (incumbent) = 26;
r,b,g = 34; g,r,b = 33; g,b,r = 40; b,r,g = 34; b,g,r = 40. Forms s14/O_*.c.

**KILLED (instance) — every placement of `src++` inside the blend arm.** Six
placements (before the products, between each channel block, after all three,
and inside the r block between its two mults) all score 26 with identical
bytes: sched1 normalises the cursor bump, so it is not a LUID/live-length lever.
Forms s14/P_s0.c..P_s5.c.

**KILLED (instance) — one shared `sum` local for the three channel totals.**
The target writes all three channel sums into $v0 (rows 111/114/118), which
looks like one reused C variable; spelled that way it scores 53 (N1), 45 (N3,
with the b channel split) and 46 (N4, r+g only). A single long-lived `sum`
pseudo conflicts with the channel variables and re-prices the arm. The target's
shared $v0 is therefore local-alloc reusing a dead register, NOT a shared C
variable.

**KILLED (instance) — deriving the b source from the g source's shift to drop a
px reference.** `shifted = (u32)px >> 2; g_src = shifted & 0xF8;
b_src = ((u32)shifted >> 5) & 0xF8;` (Q1) was meant to cut px from 4 source
references (weighted 12) to 3 (weighted 9, pri 24545, which would sort px below
g_src). combine refolds the shift chain back to `px >> 7`, the object is
byte-identical to the incumbent at 26, and ALLOCDBG still prints px at
nrefs=12 livelen=11 pri=32727 ord=2. Reference count cannot be lowered this way.

## [s14] The remaining 19-insn blend residual plus the 3-insn trip-test residual are a single global.c allocation-order fact: px (pseudo 122, pri 32727) is allocated before g_src (pseudo 126, pri 30000), so px takes $v1 and g_src takes $a0, where the target has them the other way round.
- mechanism: global.c:635-653 sorts allocnos by pri = weighted reg_n_refs * 30000 / live_length and hands each the lowest free hard register that does not conflict. $v0 is blocked for px by p118 (`count - 1`, pri 90000, allocated first), so px falls to $v1. Every other differing register in the blend arm and the trip test is downstream of that one seat: r_src already sits on the target's $a1, and a v1<->a0 rename of our stream makes rows 88, 89, 99, 100, 101, 104, 105, 116 and 122 byte-exact.
- probe: BB2_ALLOC_DEBUG=1 dumps on the C2/candidate chassis (s14/d_aac), on the g-split chassis (s14/d_aca) and on the shift-chain chassis (s14/d_q1), read against the raw index-by-index object comparison (s14/sxs.py).
- result: CONFIRMED. The three dumps agree on px = pseudo 122, nrefs 12, livelen 11, pri 32727, ord 2, hardreg 3 ($v1) across every spelling measured this session, including the ones that score 26, 38 and 45. The seat is invariant under all 27 per-channel reuse spellings, all 6 channel orderings, all 6 src++ placements and the shift-chain rewrite.
- verdict: CONFIRMED

## [s14] The remaining 19-insn blend residual and the 3-insn trip-test residual are a single global.c allocation-order fact: px (pseudo 122, pri 32727) is ordered before g_src (pseudo 126, pri 30000), so px takes $v1 and g_src takes $a0 where the target has them the other way round.
- mechanism: global.c:635-653 sorts allocnos by pri = depth-weighted reg_n_refs * 30000 / live_length and hands each the lowest free non-conflicting hard register. $v0 is blocked for px by p118 (the `count - 1` value, pri 90000, allocated first and conflicting), so px falls to $v1. r_src (pseudo 123, pri 27692) already sits on the target's $a1, and a v1<->a0 rename of our stream makes target rows 88, 89, 99, 100, 101, 104, 105, 116 and 122 byte-exact.
- probe: BB2_ALLOC_DEBUG=1 dumps of the instrumented cc1 (tools/gcc-2.7.2/cc1) on three different chassis - the candidate/C2 body, the g-split ACA form and the shift-chain Q1 form - read against a raw index-by-index object comparison (tmp/grind/func_8003DE14/s14/sxs.py).
- result: All three dumps print px identically: pseudo 122, nrefs 12, livelen 11, pri 32727, ord 2, hardreg 3 ($v1). The seat is invariant across all 27 per-channel reuse spellings, all 6 channel-block orderings, all 6 src++ placements and the shift-chain rewrite, at scores ranging from 26 to 53. The flip arithmetic is now exact: g_src needs pri above 32727, i.e. live_length <= 10 at nrefs 12 (36000) or nrefs >= 14 at live_length 12 (35000); or px needs pri below 30000, i.e. live_length >= 13 at nrefs 12 (27692) - that value ties r_src and is won by px on ascending pseudo number (122 < 123, global.c:652-653), producing exactly the target order g_src, px, r_src -> $v1, $a0, $a1.
- verdict: CONFIRMED

## [s14] Some combination of the three per-channel product/reuse spellings beats the incumbent shape.
- mechanism: Each channel can be spelled A (named complement product, src var reused for the factor product), B (src var self-multiplied by complement, named factor product) or C (both products named, no reuse). The target's b channel shows a different register pattern from its r and g channels, which suggested a mixed spelling.
- probe: All 27 combinations generated by tmp/grind/func_8003DE14/s14/gen_all.py and measured one at a time with sandbox func_8003DE14 --disable all (forms s14/G_AAA.c .. G_CCC.c).
- result: Minimum is 26, reached only by AAA (the incumbent) and AAC; the other 25 combinations score 27 to 44 at 173 insns. AAC is byte-different from AAA at the same score and reproduces the target's b-channel register pattern (the complement product lands in b_src's register, the factor product elsewhere), which confirms the b-channel rows are register naming rather than source shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s14/G_AAA.c .. G_CCC.c

## [s14] Splitting a channel's source variable in two makes the short-lived source a competing global allocno that can take $v1 ahead of px.
- mechanism: A source value that dies at its own multiply has a very short live length and therefore a very high global.c priority, so it would be allocated before px and would block $v1, pushing px to $a0.
- probe: Form ACA (g channel spelled C, r and b left as A), scored with sandbox --disable all and dumped with BB2_ALLOC_DEBUG=1 (tmp/grind/func_8003DE14/s14/d_aca/stderr.log).
- result: ACA scores 38 and its allocation table contains no short-lived g_src allocno at all: born and dead inside one basic block, it becomes a local-alloc quantity, and local-alloc coalesces it with the dying srl temp that feeds it, seating it on $v0. px is still ord=2 on $v1 with nrefs 12 / livelen 11 / pri 32727. The surviving global (the g*factor product, pseudo 128) takes $v0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s14/G_ACA.c

## [s14] Reordering the three channel blocks in the source shortens g_src's live range enough to reorder it against px.
- mechanism: g_src's live range runs from its andi to the g sum; moving the g block relative to the r and b blocks changes where the scheduler places its birth and death and therefore its live_length, the denominator of the global.c priority.
- probe: All six permutations of the r/g/b channel blocks on the incumbent shape (tmp/grind/func_8003DE14/s14/O_rgb.c .. O_bgr.c), one sandbox --disable all each.
- result: r,g,b (incumbent) 26; g,r,b 33; r,b,g 34; b,r,g 34; g,b,r 40; b,g,r 40. All at 173 insns. The incumbent ordering is uniquely optimal among the six.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s14/O_rgb.c .. O_bgr.c

## [s14] Moving the src++ cursor bump inside the blend arm shifts LUIDs enough to change a channel pseudo's live length by one or two.
- mechanism: live_length is counted over the scheduled insn chain, so putting the cursor bump at a different point in the arm would shift the births and deaths of the channel pseudos relative to each other and re-price them in global.c.
- probe: Six placements measured (before the products, between each pair of channel blocks, after all three, and inside the r block between its two multiplies): tmp/grind/func_8003DE14/s14/P_s0.c .. P_s5.c.
- result: All six score 26 with identical bytes. sched1 normalises the cursor bump to the same slot (row 110) regardless of where the source puts it, so it is not a live-length lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s14/P_s0.c .. P_s5.c

## [s14] The target's three channel sums all landing in $v0 (rows 111, 114, 118) come from one reused C variable carrying the three sums in turn.
- mechanism: GCC 2.7.2 has no SSA, so one non-address-taken local is one pseudo and therefore one hard register; a single sum local written three times would reproduce the target's single $v0, exactly as s13's per-channel variable reuse reproduced the target's $a1.
- probe: N1 (shared sum for all three channels), N3 (shared sum with the b channel's products both named) and N4 (shared sum for r and g only): tmp/grind/func_8003DE14/s14/N1.c, N3.c, N4.c.
- result: N1 = 53, N3 = 45, N4 = 46, all at 173 insns, against the incumbent 26. A single long-lived sum pseudo conflicts with the channel variables and re-prices the whole arm. The target's shared $v0 is local-alloc reusing a dead scratch register, not a shared C variable.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s14/N1.c N3.c N4.c

## [s14] Deriving the b channel source from the g channel's shifted intermediate removes one px reference and lowers px's priority below g_src's.
- mechanism: px's priority numerator is its depth-3-weighted reg_n_refs (four source references = 12). Cutting the px >> 7 to a shift of the already-computed px >> 2 would leave three references (weighted 9, pri 24545), sorting px below g_src (30000) and r_src (27692) and handing px $a0.
- probe: Form Q1 (shifted = (u32)px >> 2; g_src = shifted & 0xF8; b_src = ((u32)shifted >> 5) & 0xF8), measured with sandbox --disable all and dumped with BB2_ALLOC_DEBUG=1 (tmp/grind/func_8003DE14/s14/d_q1/stderr.log).
- result: combine refolds the shift chain back to a single srl 7 from px, the object is byte-identical to the incumbent at 26, and ALLOCDBG still prints px at nrefs 12 / livelen 11 / pri 32727 ord 2 hardreg 3. A new pseudo (128) appears for the named shift at pri 32727 and takes $v0 without conflicting with px. px's reference count cannot be reduced this way.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, form tmp/grind/func_8003DE14/s14/Q1.c

## [s15] The blend arm's residual is reachable by re-ordering the nine channel assignment statements — in particular by writing them in the order the TARGET's own instruction stream lays them out (all three products, then all three sums).
- mechanism: GCC 2.7.2 assigns pseudo live ranges over the pre-scheduling insn chain, so the source order of the nine statements sets the LUIDs that global.c:635-653 divides into `nrefs * 30000 / live_length`. s14 measured only the six whole-block permutations; the full interleaving space (three chains of three, per-channel order fixed by data dependence) is 9!/(3!^3) = 1680 spellings and had never been swept. The target's own emitted order (asm/funcs/func_8003DE14.s rows 103-131: sll/mult/mflo/mult/mflo for r, then g, then b, then the three addu/sra/andi sums) corresponds to the source order `r,r,g,g,b,b,r,g,b`.
- probe: tmp/grind/func_8003DE14/s15/gen_interleave.py emitted all 1680 bodies; all were scored in one tools/sweep_variants.py call (tmp/grind/func_8003DE14/s15/enum_il.json).
- result: ZERO HIT. The 1680 spellings collapse onto eight scores {26:196, 30:84, 31:84, 33:280, 34:476, 40:392, 41:84, 43:84}; the best is 26, which is the incumbent's own score, and 196 spellings tie it byte-identically — including the target-shaped order `rrggbbrgb`. Writing the arm in exactly the statement order the target's instruction stream exhibits does not change a single byte. sched1 normalises the whole axis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_il/I_*.c

## [s15] The 4-insn head residual (the blez delay slot) is reachable by hoisting `j = 0` and/or `complement` out of the `if (total > 0)` guard, crossed with the order of the four per-iteration statements.
- mechanism: reorg.c fills the guard branch's delay slot from the closest movable preceding insn; our build hands it `addiu a2,sp,1040` (dst) where the target has nothing movable and falls back to filling from the fall-through thread with `move t4,zero` (j = 0). s12/s13 measured six hand-built orderings INSIDE the guard (E1..E6) and found the j/complement seat and the delay slot in tension; hoisting either declaration ABOVE the guard had never been measured, and neither had the full 24-order space of `total / src / dst / factor`.
- probe: tmp/grind/func_8003DE14/s15/gen_head.py emitted all 588 spellings (hoist subsets of {complement, j} x every def-before-use order x the inner order of whatever stays inside); one sweep_variants call (s15/enum_head.json).
- result: ZERO HIT. Histogram {26:24, 28:20, 29:40, 30:84, 40:60, 44:360}. Every spelling that hoists `j = 0` above the guard lands in the 40 or 44 band (420 of 588); hoisting `complement` alone lands at 29/30. The 24 ties at 26 are precisely the spellings that keep complement-then-j inside the guard, with the four per-iteration statements in ANY of their 24 orders — which generalises s13's E4/E5/E6 byte-identity from 3 samples to the whole order space.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_head/H_*.c

## [s15] Declaring the `src` / `dst` cursors at function scope and assigning them inside the outer loop changes their birth LUIDs enough to deny reorg the `addiu a2,sp,1040` delay-slot candidate.
- mechanism: a function-scope pointer declaration makes the cursor pseudo live from the top of the function, which moves its birth insn out of the loop body and changes both its live_length and where its initialising `addiu` can be scheduled.
- probe: 96 bodies (4 declaration-site combinations x 24 statement orders) generated by tmp/grind/func_8003DE14/s15/gen_head2.py and swept in one call (s15/enum_head2.json).
- result: ZERO HIT. Only the all-declared-in-loop combination ties the baseline (40 spellings at 26); hoisting either cursor's declaration costs 33 to 97 points (bands 59, 108, 109, 116, 123). The cursors must be born inside the outer loop body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_head2/H2_*.c

## [s15] Respelling the last-pass arm test `i == count - 1` removes or re-prices pseudo 118, the pri-90000 allocno that blocks $v0 for px.
- mechanism: s14 established that px cannot take $v0 because p118 (`count - 1`) conflicts with px and is allocated first at pri 90000. An algebraically equivalent comparison that does not materialise `count - 1` (e.g. `i + 1 == count`) would remove that allocno from the conflict set.
- probe: seven respellings measured with sandbox --disable all (tmp/grind/func_8003DE14/s15/enum_test/T1..T7).
- result: ZERO HIT. `count - 1 == i` 27, `count - i == 1` 32 (174 insns), `i - count == -1` 32 (174), `i >= count - 1` 34 (174), `!(i < count - 1)` 34 (174), `i + 1 == count` 51, `count == i + 1` 51. The forms that avoid materialising `count - 1` all ADD an instruction or re-price the whole loop; none reaches the incumbent's 26.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_test/T*.c

## [s15] The 3-insn trip-test residual (target `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1` vs ours with the two pseudos named the other way round) is fixable from the latch's own spelling.
- mechanism: the two loaded values are short-lived local-alloc quantities whose registers follow the order in which the operands are read, so swapping the multiply's operands or hoisting the product into a local would swap the two names.
- probe: eight latch spellings (operand swap, parenthesised product, `>` with operands reversed, `!=`, the hoisted `total`, and the `(u16 *)` sub-word reads) measured in one sweep (s15/enum_latch.json).
- result: ZERO HIT. The four operand-order/comparison-direction forms are byte-identical to the incumbent at 26 — the latch's operand order is inert. `j != rect[2] * rect[3]` changes the instruction COUNT to 172 (28), `j < total` is 40, and the two `(u16 *)` forms are 36 (they emit `lhu` where the target emits `lh`). The residual is downstream register naming, not latch spelling.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_latch/L*.c

## [s15] Some spelling of the blend arm WITHOUT the s13 per-channel variable reuse — i.e. somewhere in the full inline-subset x declaration-order lattice of the no-reuse (SSA) form — reaches or beats the reuse form's 26.
- mechanism: the enumerate modality's premise. s5 swept this lattice on the f1/h1/d4 chassis (best 42/31), but the chassis has changed twice since (s11's arm-local dst++ and s12's complement-inside-the-guard), so the lattice had to be re-swept on the current chassis. tools/spelling_enum.py enumerates all 2^6 inline subsets of the six named channel locals crossed with every def-before-use declaration order.
- probe: 550 spellings from tools/spelling_enum.py on tmp/grind/func_8003DE14/s15/enumc_base2.c, scored in one sweep_variants call (s15/enum_ssa.json). NOTE: the first attempt (s15/enumc_base.c) was invalid — it placed `src++;` immediately before the region, making every declaration-bearing spelling a C89 declaration-after-statement parse error (549 of 550 measured a degenerate 100 / 116 insns). The corrected chassis moves `src++;` after the region, which s14 measured byte-neutral.
- result: ZERO HIT. Histogram {28:37, 33:37, 34:37, 35:37, 41:74, 44:37, 45:83, 54:67, 55:104, 56:37}; best 28, reached by 37 spellings and equal to the s12 shape the reuse form replaced. The no-reuse lattice cannot reach 26, so the per-channel variable reuse is worth 2 points that naming and ordering alone do not buy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_ssa/v*.c

## [s15] The blend arm's residual is reachable by re-ordering the nine channel assignment statements, in particular by writing them in the order the target's own instruction stream lays them out (all three products, then all three sums).
- mechanism: GCC 2.7.2 prices global allocnos as nrefs*30000/live_length over the pre-scheduling insn chain (global.c:635-653), so the source order of the nine statements sets the LUIDs that fix px's and g_src's live lengths. s14 measured only the six whole-block permutations; the full interleaving space of three dependence-ordered chains of three is 9!/(3!^3) = 1680 spellings and had never been swept.
- probe: tmp/grind/func_8003DE14/s15/gen_interleave.py emitted all 1680 bodies; all scored in one tools/sweep_variants.py call (s15/enum_il.json).
- result: ZERO HIT. The 1680 spellings collapse onto eight scores {26:196, 30:84, 31:84, 33:280, 34:476, 40:392, 41:84, 43:84}; the best is 26 = the incumbent, with 196 byte-identical ties. The target-shaped order rrggbbrgb (read off asm/funcs/func_8003DE14.s rows 103-131) is one of the ties, not an improvement: sched1 normalises the entire axis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_il/I_*.c

## [s15] The 4-insn head residual (the blez delay slot) is reachable by hoisting j = 0 and/or complement out of the if (total > 0) guard, crossed with the order of the four per-iteration statements.
- mechanism: reorg.c fills the guard branch's delay slot from the closest movable preceding insn; our build hands it addiu a2,sp,1040 (dst) where the target has no movable predecessor and falls back to filling from the fall-through thread with move t4,zero (j = 0). s12/s13 measured six orderings INSIDE the guard; hoisting either declaration ABOVE the guard was never measured.
- probe: tmp/grind/func_8003DE14/s15/gen_head.py emitted all 588 spellings (hoist subsets of {complement, j} x every def-before-use order x the inner order of whatever stays inside); one sweep_variants call (s15/enum_head.json).
- result: ZERO HIT. Histogram {26:24, 28:20, 29:40, 30:84, 40:60, 44:360}. Every spelling that hoists j = 0 above the guard scores 40 or 44 (420 of 588); hoisting complement alone scores 29/30. The 24 ties at 26 are exactly the spellings keeping complement-then-j inside the guard, with the four per-iteration statements in ANY of their 24 orders - which generalises s13's E4/E5/E6 byte-identity from three samples to the whole order space.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_head/H_*.c

## [s15] Declaring the src / dst cursors at function scope and assigning them inside the outer loop denies reorg the addiu a2,sp,1040 delay-slot candidate.
- mechanism: A function-scope pointer declaration makes the cursor pseudo live from the top of the function, moving its birth insn out of the loop body and changing both its live_length and where its initialising addiu can be scheduled.
- probe: 96 bodies (4 declaration-site combinations x 24 statement orders) from tmp/grind/func_8003DE14/s15/gen_head2.py, swept in one call (s15/enum_head2.json).
- result: ZERO HIT. Only the all-declared-in-loop combination ties the baseline (40 spellings at 26); hoisting either cursor's declaration costs 33 to 97 points (bands 59, 108, 109, 116, 123). The cursors must be born inside the outer loop body.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_head2/H2_*.c

## [s15] Respelling the last-pass arm test i == count - 1 removes or re-prices pseudo 118, the pri-90000 allocno that blocks $v0 for px.
- mechanism: s14 established px cannot take $v0 because p118 (count - 1) conflicts with px and is allocated first at pri 90000; an equivalent comparison that never materialises count - 1 would remove that allocno from the conflict set.
- probe: Seven respellings measured with sandbox --disable all (tmp/grind/func_8003DE14/s15/enum_test/T1..T7).
- result: ZERO HIT. count - 1 == i 27; count - i == 1 32 (174 insns); i - count == -1 32 (174); i >= count - 1 34 (174); !(i < count - 1) 34 (174); i + 1 == count 51; count == i + 1 51. Every form that avoids materialising count - 1 adds an instruction or re-prices the loop; none reaches 26.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_test/T*.c

## [s15] The 3-insn trip-test residual is fixable from the inner-loop latch's own spelling (operand order, comparison form, or a hoisted product).
- mechanism: The two loaded values are short-lived local-alloc quantities whose hard registers follow the order the operands are read, so swapping the multiply's operands would swap the two names.
- probe: Eight latch spellings measured in one sweep (s15/enum_latch.json).
- result: ZERO HIT. The four operand-order / comparison-direction forms are byte-identical at 26 (the axis is inert); j != rect[2] * rect[3] changes the instruction count to 172 (28); j < total is 40; the two (u16 *) sub-word forms are 36 because they emit lhu where the target emits lh. The residual is downstream register naming.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_latch/L*.c

## [s15] Some spelling of the blend arm WITHOUT the s13 per-channel variable reuse reaches or beats 26 somewhere in the full inline-subset x declaration-order lattice of the no-reuse form.
- mechanism: The enumerate premise: tools/spelling_enum.py enumerates all 2^6 inline subsets of the six named channel locals crossed with every def-before-use declaration order. s5 swept this lattice on the f1/h1/d4 chassis (best 42/31), but the chassis changed twice since (s11 arm-local dst++, s12 complement inside the guard), so every s5 conclusion about it was chassis-void.
- probe: 550 spellings from tools/spelling_enum.py on s15/enumc_base2.c, one sweep_variants call (s15/enum_ssa.json). The first chassis attempt was invalid (src++ before the region made every declaration-bearing spelling a C89 declaration-after-statement parse error: 549 of 550 measured a degenerate 100 / 116); the corrected chassis moves src++ after the region, which s14 measured byte-neutral.
- result: ZERO HIT. Histogram {28:37, 33:37, 34:37, 35:37, 41:74, 44:37, 45:83, 54:67, 55:104, 56:37}; best 28 (37 spellings), equal to the s12 shape the reuse form replaced. The no-reuse lattice cannot reach 26, so the per-channel variable reuse is worth 2 points that naming and ordering alone never buy.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s15/enum_ssa/v*.c
