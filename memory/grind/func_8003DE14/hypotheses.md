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

## [s16] KILL RE-AUDIT: the two closest-to-target banked kills (s14 AAC, s15 target-shaped interleave rrggbbrgb) still measure 26 on today's chassis.
- mechanism: an instance kill is only valid on the chassis and FAKE state it was measured under. The incumbent body carries NO annotation-bearing construct anywhere (ordinary C throughout), so tools/fake_ablate.py has nothing to ablate; the re-audit is therefore a pure chassis re-measurement.
- probe: tmp/grind/func_8003DE14/s16/audit/{A0_incumbent,A1_aac,A2_targetorder}.c swept in one tools/sweep_variants.py call (s16/audit.json).
- result: all three score 26 at 173 insns on HEAD 2026-09-10, identical to their s14/s15 measurements. The chassis has not moved and both kills stand. (The driver's dispatch line said "measurement unavailable"; the real HEAD honest floor for this body is 26.)
- verdict: CONFIRMED

## [s16] The pseudo map behind the seat residual, CORRECTED: p121 = pixel, p122 = px, p123 = r_src, p126 = g_src, p128 = b_src - and b_src is a GLOBAL allocno at pri 32727 sitting on $v0, not a local-alloc quantity.
- mechanism: read directly out of the .lreg RTL of the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1): `(set (reg/v:SI 122) (zero_extend:SI (reg/v:HI 121)))` names px and pixel; the two `(and:SI ... (const_int 248))` defs that are also mult destinations name g_src and b_src; the `(ashift:SI ... (const_int 3))` def names r_src.
- probe: tmp/grind/func_8003DE14/s16/d_base/ (dump of the incumbent) plus tmp/grind/func_8003DE14/s16/batch_alloc.py, which recovers the same mapping automatically for a whole directory of forms.
- result: the incumbent's global table is px p122 (nrefs 12 / livelen 11 / pri 32727) -> $v1, b_src p128 (12 / 11 / 32727) -> $v0, g_src p126 (12 / 12 / 30000) -> $a0, r_src p123 (12 / 13 / 27692) -> $a1. The target's seats, read off asm/funcs/func_8003DE14.s rows 94-124, are g_src -> $v1, px -> $a0, b_src -> $a0 (reusing px's register after px dies at row 118) and r_src -> $a1. So the residual is a THREE-way seat difference, not the two-way px/g_src swap s13/s14 recorded, and s14's reading that b_src is a local quantity was wrong: b_src is the second allocno in the 32727 tie and is what takes $v0 away from the target's scratch.
- verdict: CONFIRMED

## [s16] Moving the channel locals' DECLARATIONS (not their assignments) renumbers the pseudos and flips the global.c:652-653 tie-break, which is enough to reseat px and g_src.
- mechanism: global.c:635-653 sorts allocnos by pri and breaks EQUAL priorities on ascending pseudo number; pseudo numbers follow declaration order. A declaration-only hoist changes no statement and no live range, so it is a free renumbering knob - the one axis s5/s15's declaration sweeps never isolated (they moved initialisers, i.e. statements).
- probe: 384 bodies (every subset/order of {r_src, g_src, b_src} declared at 4 sites: above `u16 pixel`, between pixel and px, after px, or in the inner block) from tmp/grind/func_8003DE14/s16/gen_renum2.py, swept in one call (s16/renum3.json); ALLOCDBG dumps of the baseline and of R_gbr_000 (s16/d_base, s16/d_gbr000).
- result: ALL 384 score 26 at 173 insns. The renumbering is real and visible - in R_gbr_000 g_src becomes p121 (was p126) while px stays p122 - and it DOES move hard registers inside the 32727 tie (px $v1 -> $v0, b_src $v0 -> $v1). It never reaches the target seat because px (32727) and g_src (30000) are not tied, so the tie-break never arbitrates between them. Renumbering is a live, byte-neutral lever that only pays off once some other change equalises the two priorities.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/renum3/R_*.c

## [s16] The target's three key seats (r_src $a1, g_src $v1, px $a0) are reachable in ordinary C - the question is not whether the allocator can be steered there, but what the steering costs elsewhere.
- mechanism: which C local carries each of the six blend products decides each channel pseudo's reg_n_refs and live_length, i.e. both inputs to the global.c priority. s14 swept only the 27 WITHIN-channel spellings; this session swept the CROSS-channel lattice (a product may be carried by any local that is provably dead at that point, liveness simulated in the generator), 1,483 legal spellings.
- probe: tmp/grind/func_8003DE14/s16/gen_carrier.py emitted the lattice; tmp/grind/func_8003DE14/s16/batch_alloc.py compiled every one of them with the instrumented cc1 and recorded the global-allocno table plus the pseudo->variable map (s16/carrier_alloc.json, 3.0 MB); s16/analyze.py ranked them by how many of the four target seats they hit.
- result: 4 forms hit three of the four target seats exactly (r_src $a1, g_src $v1, px $a0): C_FFrSgSgSF, C_FFrSgSgSbS, C_FFrSgSgSrG, C_FFrSgSgSrP - all of them make g_src carry TWO products (nrefs 18) and drop px to livelen 10 / pri 36000. 89 further forms hit two seats. The s14 prediction is independently confirmed too: C_FFFgSFrG has px at exactly nrefs 12 / livelen 13 / pri 27692 and does seat g_src on $v1 and px on $a0.
- verdict: CONFIRMED

## [s16] Some cross-channel carrier spelling that reaches the target's seats also reaches or beats the incumbent's 26.
- mechanism: the seat residual is 19 of the 26 points, so a spelling that fixes the seats should be able to pay for a fair amount of collateral damage elsewhere.
- probe: the 25 best seat-matching forms scored with tools/sweep_variants.py (s16/top.json), and the best one (C_FFrSgSgSbS, three seats correct) disassembled side-by-side against the target (tmp/grind/func_8003DE14/s13/sxs.py, rows 94-131).
- result: ZERO HIT. Best of the seat-correct forms is 38 at 173 insns; the 25-form best is 34. The side-by-side shows why: making g_src carry a second product re-prices the whole arm for sched1, so the products and sums re-interleave (our rows 97-118 no longer line up opcode-for-opcode with the target's), and the loop's own registers move too (complement $t5 -> $t4, factor $t3 -> $t2, j $t4 -> $t3). The seat win is real but is swamped by a wholesale re-schedule. The full 1,483-form lattice's minimum is 26, reached only by incumbent-shaped spellings.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/carrier/*.c (alloc tables in s16/carrier_alloc.json, scores for the top 25 in s16/top.json)

## [s16] A px/g_src PRIORITY TIE plus the renumbering lever hands g_src the seat by pseudo number and closes the blend residual.
- mechanism: the conjunction no previous session tried. 108 of the 1,483 carrier spellings tie px and g_src at one priority (27692, 30000, 32727 or higher) with g_src numbered ABOVE px; hoisting g_src's declaration above `u16 pixel` makes it the lower-numbered allocno, so global.c:652-653 allocates g_src first and it should take px's seat.
- probe: tmp/grind/func_8003DE14/s16/gen_tie.py applied the (byte-neutral, phase-1-proven) declaration hoist to all 108 tie forms; swept in one call (s16/tie.json).
- result: ZERO HIT. Histogram runs 34..56; best is 34 (T_FFrGrSgSbS), then six at 36. Every tie form pays the same re-schedule tax as the rest of the carrier lattice, so the renumbering lever - which is genuinely free on the incumbent - has nothing cheap to arbitrate. A priority tie and the incumbent instruction sequence have not been made to coexist.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/tie/T_*.c

## [s16] KILL RE-AUDIT: the two closest-to-target banked kills (s14's AAC form, s15's target-shaped interleave rrggbbrgb) still measure 26 at 173 insns on the current chassis, and the incumbent body still measures 26.
- mechanism: An instance kill is only valid on the chassis and FAKE state it was measured under. The incumbent carries no annotation-bearing construct at all (ordinary C throughout), so tools/fake_ablate.py has nothing to ablate and the re-audit reduces to a chassis re-measurement of the two closest forms.
- probe: tmp/grind/func_8003DE14/s16/audit/{A0_incumbent,A1_aac,A2_targetorder}.c scored in one tools/sweep_variants.py call (s16/audit.json).
- result: All three score 26 / 173 on HEAD 2026-09-10, identical to their s14/s15 measurements. Both kills stand and the chassis has not moved (the dispatch brief's 'measurement unavailable' is resolved: the HEAD honest floor for this body is 26).
- verdict: CONFIRMED

## [s16] The blend residual's pseudo map is p121 = pixel, p122 = px, p123 = r_src, p126 = g_src, p128 = b_src, and b_src is a GLOBAL allocno at pri 32727 holding $v0 - so the residual is a three-way seat difference, not the two-way px/g_src swap s13/s14 recorded.
- mechanism: The .lreg RTL of the instrumented cc1 names each allocno: (set (reg/v:SI 122) (zero_extend:SI (reg/v:HI 121))) is px from pixel; the two (and:SI ... (const_int 248)) defs that are also mult destinations are g_src and b_src; the (ashift:SI ... (const_int 3)) def is r_src. BB2_ALLOC_DEBUG=1 then gives each one's nrefs/livelen/pri/hardreg.
- probe: tmp/grind/func_8003DE14/s16/d_base (incumbent dump) and tmp/grind/func_8003DE14/s16/batch_alloc.py, which recovers the mapping automatically for a whole directory of forms.
- result: Ours: px p122 (12/11/32727) $v1, b_src p128 (12/11/32727) $v0, g_src p126 (12/12/30000) $a0, r_src p123 (12/13/27692) $a1. Target (asm/funcs/func_8003DE14.s rows 94-124): g_src $v1, px $a0, b_src $a0 (reusing px's register after px dies at row 118), r_src $a1, with NOTHING global in $v0 - the target leaves $v0 to local-alloc for the srl temps and the three channel sums.
- verdict: CONFIRMED

## [s16] Moving the channel locals' DECLARATIONS (not their assignments) renumbers the pseudos and flips the global.c:652-653 tie-break, which is enough to reseat px and g_src.
- mechanism: global.c:635-653 sorts allocnos by pri = weighted reg_n_refs * 30000 / live_length and breaks EQUAL priorities on ascending pseudo number; pseudo numbers follow declaration order. Splitting a local into (declaration, assignment) and moving only the declaration renumbers without moving a statement or changing a live range - the axis s5/s15's declaration sweeps never isolated, because those moved initialisers, i.e. statements.
- probe: 384 bodies (every subset and order of {r_src, g_src, b_src} declared at four sites: above u16 pixel, between pixel and px, after px, or in the inner block) from tmp/grind/func_8003DE14/s16/gen_renum2.py, swept in one call (s16/renum3.json); ALLOCDBG dumps of the baseline and of R_gbr_000 (s16/d_base, s16/d_gbr000).
- result: All 384 score 26 at 173 insns. The renumbering is real - in R_gbr_000 g_src becomes p121 (was p126) while px stays p122, and the two tied 32727 allocnos swap seats (px $v1 -> $v0, b_src $v0 -> $v1) - but it never arbitrates px against g_src, because those two are not tied (32727 vs 30000). Renumbering is a live, byte-neutral lever with nothing to decide until some other change equalises the two priorities.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/renum3/R_*.c

## [s16] The target's three key blend seats (r_src $a1, g_src $v1, px $a0) are reachable in ordinary C by changing which dead local carries each of the six blend products.
- mechanism: The carrier assignment decides each channel pseudo's reg_n_refs and live_length, i.e. both inputs to the global.c priority that orders the allocnos. s14 swept only the 27 within-channel spellings; the cross-channel lattice (any local that is provably dead at that point may carry a product) had never been enumerated.
- probe: tmp/grind/func_8003DE14/s16/gen_carrier.py emitted 1,483 liveness-checked spellings; tmp/grind/func_8003DE14/s16/batch_alloc.py compiled every one with the instrumented cc1 and recorded its global-allocno table plus pseudo->variable map (s16/carrier_alloc.json, 3.0 MB); s16/analyze.py ranked by target-seat hits.
- result: Four forms hit three of the four target seats exactly (C_FFrSgSgSF, C_FFrSgSgSbS, C_FFrSgSgSrG, C_FFrSgSgSrP), all by making g_src carry two products (nrefs 18, pri 45000) and dropping px to livelen 10; 89 forms hit two seats; 108 forms tie px and g_src. s14's predicted route is confirmed too: C_FFFgSFrG puts px at exactly nrefs 12 / livelen 13 / pri 27692 and seats g_src on $v1 and px on $a0.
- verdict: CONFIRMED

## [s16] Some cross-channel carrier spelling that reaches the target's seats also reaches or beats the incumbent's 26.
- mechanism: The seat difference accounts for 19 of the 26 residual points, so a spelling that fixes the seats has a large budget for collateral damage elsewhere.
- probe: The 25 best seat-matching forms scored with tools/sweep_variants.py (tmp/grind/func_8003DE14/s16/top.json); the best seat-correct form (C_FFrSgSgSbS) disassembled index-by-index against the target with tmp/grind/func_8003DE14/s13/sxs.py (rows 94-131).
- result: ZERO HIT. The seat-correct forms score 38 at 173 insns; the best of the top 25 is 34; the whole 1,483-form lattice bottoms out at 26, reached only by incumbent-shaped spellings. The side-by-side shows the cost: giving g_src a second product re-prices the arm for sched1, so products and sums re-interleave (our rows 97-118 stop lining up opcode-for-opcode with the target's) and the loop's own registers move too (complement $t5 -> $t4, factor $t3 -> $t2, j $t4 -> $t3).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/carrier/*.c with alloc tables in s16/carrier_alloc.json and scores in s16/top.json

## [s16] A px/g_src priority TIE combined with the byte-neutral renumbering lever hands g_src the seat by pseudo number and closes the blend residual.
- mechanism: The conjunction no previous session tried: 108 carrier spellings tie px and g_src at one priority with g_src numbered above px, and a declaration-only hoist makes g_src the lower-numbered allocno, so global.c:652-653 allocates g_src first and it takes px's seat.
- probe: tmp/grind/func_8003DE14/s16/gen_tie.py applied the phase-1-proven byte-neutral declaration hoist to all 108 tie forms; swept in one call (s16/tie.json).
- result: ZERO HIT. Scores run 34 to 56; best is 34 (T_FFrGrSgSbS), then six at 36. Creating the tie costs the schedule exactly as the rest of the lattice does, so the renumbering lever - free on the incumbent - has nothing cheap to arbitrate. A priority tie and the incumbent instruction sequence have not been made to coexist.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s16/tie/T_*.c

## s17 (solver)

### H17.1 - KILLED (instance)
**Statement.** On the incumbent chassis, an emission order for the outer-loop
head basic block in which both cursor `addiu`s precede the `mflo` is obtainable
from sched1/sched2 by an INSN_PRIORITY delta (s16 frontier item 3's next probe).

**Mechanism tested.** `tools/sched_solver` models both scheduling passes
order- and clock-exactly (6978/6978 blocks). If our head block's pick order
differed from the target's, perturb would print that block with a goal and
search `luid` / `luid_move` atoms for the priority perturbation that reaches it.

**Probe.** `extract.py code6cac_c2` (parity=True, 750 blocks / 3989 picks);
`s17/perturb2.py ... --func func_8003DE14 --pass {1,2} --goal-from-target
code6cac_c2 --target-object build/src/code6cac_c2.o --ours-object
tmp/sandbox/func_8003DE14/code6cac_c2.o --atoms luid,luid_move --depth 2`.

**Result.** Neither pass flags the head block at all: its goal equals ours. The
only block flagged in either pass is block 10 (the blend arm), and it is
reported as a non-topological goal - an alignment artifact. So there is no
priority disagreement to perturb: our head block already emits the target's
order, and the rows-54..71 rotation is created after sched2, in reorg.c's
delay-slot fill (documented as out of the model's scope). Measured on HEAD
2026-09-10, C2/candidate chassis (26 / 173 insns), no FAKE constructs present.
Logs `s17/perturb_pass1.log`, `s17/perturb_pass2.log`.

### H17.2 - KILLED (instance)
**Statement.** The insns sched1 interleaves into the blend arm (the `src++` bump
at row 110 and the trip-test `mflo` at row 117) can be moved out of g_src's
second live segment by a scheduler lever, shortening g_src's live length from 12
to <= 10 and lifting its allocno priority to >= 36000 (s16 frontier item 1's
next probe).

**Mechanism tested.** Same solver run; block 10 IS the blend arm (its node table
carries the six unit-1 `mult` / partner pairs 227/397 ... 252/412).

**Result.** The solver reports block 10's target-derived goal as NOT a
topological order (8 violations in pass 2, 3 in pass 1) - the aligner mis-paired
duplicate instruction text, which is the same difflib artifact s13 identified by
raw index-by-index comparison. Our blend-arm order IS the target's, so there is
no alternative legal schedule to steer toward and no INSN_PRIORITY delta to
find. g_src's live length must be attacked through its live RANGE (references
and their positions), not through the pick order. Measured on HEAD 2026-09-10,
C2/candidate chassis (26 / 173 insns), no FAKE constructs present.

### H17.3 - CONFIRMED
**Statement.** The entire 26-point residual is downstream of the RTL multiset:
our 173-insn stream and the target's 173-insn stream contain the same
instructions modulo register names.

**Probe.** `inverse_compose.py classify code6cac_c2 func_8003DE14
--target-object build/src/code6cac_c2.o --ours-object
tmp/sandbox/func_8003DE14/code6cac_c2.o` -> `FIRST DIVERGENCE: RA`, honest 173 /
target 173 (`s17/classify.log`).

**Result.** The register-blanked multisets are identical, so the PRE-RA test
passes. No front-end / cse / combine / loop hypothesis can be the explanation
for any part of this residual, and any future session proposing one is
proposing something already measured false on this chassis. The residual is
(a) register assignment - global.c's seat for px / g_src / b_src / r_src and the
two short-lived trip-test pseudos - plus (b) one reorg.c delay-slot choice.

## [s17] On the incumbent chassis, an emission order for the outer-loop head basic block in which both cursor addiu's precede the mflo is obtainable from sched1/sched2 by an INSN_PRIORITY delta (the s16 frontier's head-region next probe).
- mechanism: tools/sched_solver replicates both scheduling passes order- and clock-exactly (6978/6978 blocks project-wide). If our head block's pick order differed from the target's, perturb would print that block with a derived goal and search the luid / luid_move atoms for the priority perturbation that reaches it.
- probe: extract.py code6cac_c2 (parity=True, 82 funcs, 750 blocks, 3989 picks) on the candidate body spliced into src; then tmp/grind/func_8003DE14/s17/perturb2.py tmp/sched_solver_work/code6cac_c2.sched.json --func func_8003DE14 --pass 1 and --pass 2 --goal-from-target code6cac_c2 --target-object build/src/code6cac_c2.o --ours-object tmp/sandbox/func_8003DE14/code6cac_c2.o --atoms luid,luid_move --depth 2.
- result: Neither pass flags the head block: its target-derived goal equals ours. The only block flagged in either pass is block 10, the blend arm, and it is reported SKIPPED because its goal is not a topological order (8 violations in pass 2, 3 in pass 1 - the aligner mis-paired register-renamed duplicate instruction text). There is therefore no priority disagreement to perturb in the head block: our sched2 output is already the target's order, and the rows-54..71 rotation (addiu a2,sp,1040 at target row 54 vs our blez delay slot at row 69) is created downstream of the model, in reorg.c's fill_simple_delay_slots, which the sched_solver README explicitly excludes from its scope. The next probe for that residual is reorg ELIGIBILITY - making the last pre-branch insn one reorg cannot hoist (the target has an mflo there) - not a scheduler priority delta.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (score 26 / 173 insns, re-measured this session), no FAKE constructs present (fake_ablate: nothing to ablate); logs tmp/grind/func_8003DE14/s17/perturb_pass1.log and perturb_pass2.log

## [s17] The insns sched1 interleaves into the blend arm - the src++ cursor bump at row 110 and the trip-test mflo at row 117 - can be moved out of g_src's second live segment by a scheduler lever, shortening g_src's live length from 12 to 10 or less and lifting its allocno priority to 36000 or more (the s16 frontier's blend-arm next probe).
- mechanism: live_length is counted over the post-sched1 chain, so the claim requires an alternative legal emission order for the blend basic block. Block 10 of the extracted model IS the blend arm: its node table carries the twelve unit-1 insns, six mult at icost 12 with their six paired icost-1 partners (uids 227/397, 230/400, 238/403, 241/406, 249/409, 252/412).
- probe: Same two perturb runs (pass 1 and pass 2), goal derived from the target object.
- result: The solver reports block 10's target-derived goal as NOT a topological order and skips it - the difflib aligner mis-paired the blend arm's duplicate instruction texts, which differ only in register names. That is the same artifact s13 identified by raw index-by-index comparison, and it means our blend-arm emission order IS the target's. There is no alternative schedule to steer toward and no INSN_PRIORITY delta to find, so g_src's live length cannot be shortened from the pick order. If g_src's priority is to be lifted, it must come from its live RANGE - the number and position of its references - which is an RA/front-end question, not a scheduler one.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis (score 26 / 173 insns, re-measured this session), no FAKE constructs present; logs tmp/grind/func_8003DE14/s17/perturb_pass1.log and perturb_pass2.log

## [s17] The entire 26-point residual is downstream of the RTL instruction multiset: our 173-insn stream and the target's 173-insn stream contain exactly the same instructions modulo register names.
- mechanism: inverse_compose.py's classify funnel tests PRE-RA first by comparing register-BLANKED instruction multisets; only if those match does it fall through to the RA (same instructions, different registers) verdict. Object mode is the only legal mode for an INCLUDE_ASM-routed function - both sides are objdump renderings in the same language.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_c2 func_8003DE14 --target-object build/src/code6cac_c2.o --ours-object tmp/sandbox/func_8003DE14/code6cac_c2.o (log tmp/grind/func_8003DE14/s17/classify.log).
- result: FIRST DIVERGENCE: RA; honest 173 insns, target 173 insns; next tool tools/ra_solver/inverse.py (global / local). The register-blanked multisets are identical, so no front-end / cse / combine / loop divergence exists anywhere in this function. Every one of the 26 points is a register name or a placement, never a different or missing instruction. The residual decomposes into (a) global.c's seats for px / g_src / b_src / r_src and the two short-lived trip-test pseudos, and (b) one reorg.c delay-slot choice in the outer-loop head.
- verdict: CONFIRMED


## s18 (forensics, 2026-09-10)

- CONFIRMED - The head block's 4-insn residual is produced by reorg.c's
  `fill_simple_delay_slots` backward scan, not by sched1/sched2 and not by
  allocation.  BB2_DBR_DEBUG prints the scan rejecting insns 128/125/124/394 and
  accepting insn 121 (`addiu a2,sp,1040`, the `dst = dst_buf` init) for the
  blez's delay slot, while .sched2 already emits 121 adjacent to 118 exactly as
  the target does.  (tmp/grind/func_8003DE14/s18/dbr/stderr.log:2185-2191;
  reorg.c:2963-3020)

- CONFIRMED - Making `move j,zero` the last insn before the blez gives reorg the
  target's delay slot and restores the target's rows 56/57 and 73-77 exactly.
  The C spelling is `s32 j = 0;` hoisted out of the `if (total > 0)` guard into
  the per-outer-iteration declaration list (form h1, score 28; head region
  byte-identical to the target modulo one register-name swap).

- KILLED (instance) - On the h1 chassis, moving the DECLARATIONS of `j` and
  `complement` (declaration split from assignment, either order, h5/h6) flips the
  $t4/$t5 seats of the pair.  Both score 28 and the alloc table shows the same
  pri 6111 / 5593 pair in the same order.  Measured on HEAD 2026-09-10, h1
  chassis (28 / 173), no FAKE constructs.

- KILLED (instance) - Spelling the guard as a comparison that reads `j`
  (`if (j < total)`, `if (j < rect[2] * rect[3])`) buys j the extra depth-3
  reg_n_refs site that would price it above complement.  Both score 45: the
  comparison no longer folds to `blez` and GCC emits slt + branch.  Measured on
  HEAD 2026-09-10, h1 chassis, no FAKE constructs.

- KILLED (instance) - Hoisting `complement` out of the guard alongside `j`
  (h7/h8, either declaration order) prices the pair the target's way.  Both score
  44: complement's dependence on `factor` drags the whole factor division ahead
  of the total multiply and `total` lands in $t6 instead of $v1.  Measured on
  HEAD 2026-09-10, no FAKE constructs.

- KILLED (instance) - Rewriting the inner do-while as
  `while (j < rect[2]*rect[3])` with `complement` inside the body for LICM to
  place in the preheader reproduces the target's preheader shape.  Score 59.
  Measured on HEAD 2026-09-10, no FAKE constructs.

## [s18] The head block's 4-insn residual is produced by reorg.c's fill_simple_delay_slots backward scan choosing insn 121 (`addiu a2,sp,1040`, the dst = dst_buf init) for the blez's delay slot, not by sched1/sched2 emission order.
- mechanism: fill_simple_delay_slots (reorg.c:2963-3020) scans backwards from the branch accumulating set/needed resources and takes the first insn that is resource-clean AND eligible_for_delay. The instrumented cc1's DBRDBG trace prints the whole scan for jump_insn 131: trial 128 (the divmod parallel) is resource-clean but elig=0 (multi-insn template); 125, 124 and 394 all collide with set/needed; 121 is clean and elig=1, so reorg deletes it from the block and puts it in the slot. The .sched2 dump already emits 121 immediately after 118 (`addiu a3,sp,16`), i.e. exactly the target's rows 56/57, so the scheduler produces the target's order and reorg is what breaks it.
- probe: pwsh tools/grinder/dump.ps1 func_8003DE14 (read the .sched2 function region, lines 18900-20509) plus a re-run of cpp|cc1 through tools/gcc-2.7.2/cc1 with BB2_DBR_DEBUG=1; trace at tmp/grind/func_8003DE14/s18/dbr/stderr.log:2185-2191.
- result: Confirmed by the pass's own instrumentation. This closes the s17 frontier item 1 question 'is it sched or reorg' with a direct measurement rather than an inference, and it identifies the single source-side input that matters: which insn is LAST in the head block before the blez.
- verdict: CONFIRMED

## [s18] Hoisting `s32 j = 0;` out of the `if (total > 0)` guard into the per-outer-iteration declaration list makes `move j,zero` the last pre-branch insn, so reorg fills the blez delay slot with it and the head region becomes byte-identical to the target apart from the j/complement register-name swap.
- mechanism: reorg takes the CLOSEST eligible insn before the branch. With j=0 emitted at the end of the head block it sits closer than insn 121, so the backward scan takes j=0 and leaves `addiu a2,sp,1040` at the target's row 57. The guarded block then begins with the complement subu, matching the target's preheader.
- probe: Form h1 (tmp/grind/func_8003DE14/s18/h1.c, banked as memory/grind/func_8003DE14/chassis_s18_h1_head_exact_28.c); sandbox --disable all = 28 / 173; objdump stream at tmp/grind/func_8003DE14/s18/h1.txt compared index-by-index with asm/funcs/func_8003DE14.s rows 55-57 and 73-77.
- result: target 55-57 `mult v1,v0 / addiu a3,sp,0x10 / addiu a2,sp,0x410` == h1 2514-251c; target 73-77 `mflo t3 / blez v1 / [slot] addu t4,zero,zero / subu t5,fp,t3 / addiu v0,s2,-1` == h1 2554-2564 with t4 and t5 exchanged. The 4-insn head residual that has stood since s13 is gone; h1's 28 decomposes head 0 + blend 19 + trip 3 + the 6-row t4/t5 swap, so a form carrying h1's head and the incumbent's seats scores 22.
- verdict: CONFIRMED

## [s18] On the h1 chassis, moving the declarations of `j` and `complement` (declaration split from assignment, either order) exchanges their $t4/$t5 hard registers.
- mechanism: s16 established declaration-site renumbering as a real byte-neutral lever for the 32727-priority TIE pair. This pair is not tied: global.c prices complement at 6111 (nrefs 11 / livelen 54) and j at 5593 (nrefs 11 / livelen 59), allocates in descending priority order, and hands each allocno the lowest free hard register, so pseudo numbering never enters the decision.
- probe: Forms h5 (complement declared first) and h6 (j declared first), both with bare declarations above the guard and plain assignments in place; sandbox --disable all on each; BB2_ALLOC_DEBUG=1 alloc table at tmp/grind/func_8003DE14/s18/alloc_h6/stderr.log.
- result: h5 = 28, h6 = 28; the alloc table shows ord=15 pseudo=116 hardreg=12 pri=6111 and ord=16 pseudo=115 hardreg=13 pri=5593 in both. The renumbering lever does not reach this pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s18/h5.c and h6.c

## [s18] Spelling the guard as a comparison that reads j (`if (j < total)` or `if (j < rect[2] * rect[3])`, both equivalent since j == 0 there) buys j the extra loop-depth-3 reg_n_refs site that would price it above complement.
- mechanism: pri = floor_log2(nrefs) * nrefs * 10000 / livelen and reg_n_refs is loop-depth weighted in flow.c, so one extra depth-3 read of j is worth +3 refs: 3*14*10000/59 = 7118 > complement's 6111, which would hand j the $t4 seat.
- probe: Forms h9 and h10 (h1 with the guard rewritten); sandbox --disable all on each.
- result: Both score 45. The comparison no longer folds to `blez`: GCC emits slt + branch, which costs far more than the seat is worth. The reference-count route to the flip has no byte-neutral spelling at the guard.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis (28 / 173 insns), no FAKE constructs present, forms tmp/grind/func_8003DE14/s18/h9.c and h10.c

## [s18] Hoisting `complement` out of the `if (total > 0)` guard alongside `j` gives complement the longer live range and so restores the target's j=$t4 / complement=$t5 seats on top of h1's head structure.
- mechanism: complement's def would precede j's, making livelen(complement) > livelen(j) and inverting the global.c priority compare that h1 loses.
- probe: Forms h7 (complement declared first) and h8 (j first); sandbox --disable all; objdump stream at tmp/grind/func_8003DE14/s18/h7.txt.
- result: Both score 44. Making complement live before the guard drags its dependence chain with it: the head block emits `addiu v0,s1,1 / sll / div / mflo t3` BEFORE `lh / lh / mult`, and `total` lands in $t6 instead of $v1 (h7.txt:2508-255c). This reproduces s15's 'every hoisted-j spelling is 40+' result on the s18 chassis and localises the cost to complement, not to j.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis re-measured this session at 26 / 173, no FAKE constructs present, forms tmp/grind/func_8003DE14/s18/h7.c and h8.c

## [s18] Rewriting the inner do-while as `while (j < rect[2] * rect[3])` with `complement` moved inside the loop body, so loop.c hoists it into the preheader, reproduces the target's head + preheader shape.
- mechanism: GCC 2.7.2 rotates a while loop into `init; if (!cond) goto end; preheader; do { body } while (cond);`, which is the target's exact shape (entry blez at row 74, preheader subu at 76 and count-1 at 77), and LICM would place the loop-invariant complement in that preheader.
- probe: Form h4 (tmp/grind/func_8003DE14/s18/h4.c); sandbox --disable all.
- result: Score 59. loop.c does not produce the target's preheader from this source and the whole blend arm is re-scheduled around the moved complement. Banked as memory/grind/func_8003DE14/rejected/s18-while-loop-form-59.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), C2/candidate chassis re-measured this session at 26 / 173, no FAKE constructs present, form tmp/grind/func_8003DE14/s18/h4.c

## [s19] Duplicating the inner loop's `j++` latch into the exit arms lifts reg_n_refs enough to re-seat j, and cross-jumping re-merges the copies to identical bytes (the s11 `dst++` precedent).
- mechanism: .claude/rules/duplicated-statement-into-arms.md — a real statement duplicated into 2+ arms is counted by flow.c once per copy (loop-depth weighted, +6 per extra copy at depth 3), lifting allocno_pri, and jump2's cross-jump merges the copies back to one insn so the bytes are unchanged. s11 used exactly this for the `dst++` cursor bump on this function.
- probe: On the h1 chassis (28 / 173), three routings: b2 (all four exit paths increment j themselves, shared label empty), b3 (blend-else path increments, the three early exits share one copy), b4 (blend-else + both last-frame arms separate). sandbox --disable all on each plus BB2_ALLOC_DEBUG on b3 (tmp/grind/func_8003DE14/s19/alloc_b3/stderr.log).
- result: KILLED. b3 44 at **174** insns, b2 45 at **175**, b4 47 at 174 — cross-jumping does NOT re-merge the duplicated `j++` here (unlike the `dst++` case, whose copies end blocks that already share a tail), so every copy costs a real insn. The alloc table also shows the lift OVERSHOOTS the needed window: j (pseudo 115) goes to nrefs 17 / livelen 60 / pri 11333, is allocated at ord=10 and takes $t1 (hardreg 9) instead of $t4, while complement correctly lands on $t5. Banked as rejected/s19-j-inc-duplicated-into-arms-not-crossjump-merged-44.c and rejected/s19-j-inc-duplicated-four-arms-45.c.

## [s19] A DEPTH-2 reference to `j` added in the head or the inner-loop preheader buys the +2 reg_n_refs that prices j above complement (nrefs 11 -> 13, pri 6610 > 6111).
- mechanism: pri = floor_log2(nrefs)*nrefs*10000/livelen; flow.c weights reg_n_refs by loop depth, so one extra reference at depth 2 (outer-do-while body) is worth +2. nrefs 13 at livelen 59 gives 6610, which sits inside the (6111, 6964) window between complement and factor.
- probe: Two spellings on the h1 chassis: c2 = a redundant `j = 0;` as the first statement inside the `if (total > 0)` guard (depth-2 WRITE); c4 = `s32 complement = (blend_base - factor) + j;` (depth-2 READ, value-neutral because j == 0 there). sandbox --disable all plus BB2_ALLOC_DEBUG tables at tmp/grind/func_8003DE14/s19/alloc_c2 and alloc_c4.
- result: KILLED for both spellings. Each scores 28 / 173 — byte-identical to h1 — and the alloc tables are IDENTICAL to h1's line for line (j still nrefs=11 livelen=59 pri=5593 -> $t5). cse2 runs before flow.c's life_analysis and knows `j == 0` at every pre-loop site, so it folds the read and deletes the redundant store before any reference is counted. Banked as rejected/s19-redundant-depth2-j-store-cse2-deletes-28.c and rejected/s19-complement-plus-j-depth2-read-cse2-folds-28.c.

## [s19] livelen(j) is a controllable quantity on the h1 chassis: moving the `s32 j = 0;` statement earlier among the five head statements lengthens j's live range, which re-prices it.
- mechanism: reg_live_length is computed by flow.c on the pre-scheduling RTL, so j's live range starts where expand emits its initialiser. complement's def is pinned to the first insn after the guard branch and factor's to the div/mflo pair, so neither moves.
- probe: All five positions of `s32 j = 0;` in the head statement list (f1 = first, f2 = 2nd, f3 = 3rd, f4 = 4th/just before factor, h1 = last); sandbox --disable all on each + BB2_ALLOC_DEBUG (s19/alloc_f1 .. alloc_f4).
- result: CONFIRMED. livelen(j) = 59 (h1 and f4, both score 28), 60-61 (f3, 29), 61 (f1 and f2, 30); all five at 173 insns. complement holds livelen 54 / pri 6111 and factor livelen 56 / pri 6964 in every spelling. This makes the f1 chassis (livelen 61, pri 5409, banked as chassis_s19_f1_jfirst_livelen61_30.c) the one that pairs with a +3 depth-3 reference: nrefs 14 at livelen 61 is pri 6885, inside the (6111, 6964) window, whereas nrefs 14 at livelen 59 is 7118 and would steal factor's $t3.

## [s19] global.c's register-preference machinery (`regs_someone_prefers`) can be used to make `complement` skip $t4 and leave it for the later-allocated `j`.
- mechanism: prune_preferences (global.c:911-929) records, for each allocno, the registers preferred by every LOWER-priority allocno that conflicts with it, and find_reg excludes that set.
- probe: read find_reg's two-pass structure (tools/gcc-2.7.2/global.c:994-1080) against the h1 alloc table.
- result: KILLED (class). The exclusion applies only in pass 0, and pass 0 additionally ORs in `~regs_used_so_far` with the comment "we never allocate a register for the first time in pass 0" (global.c:998-1001). $t4 and $t5 are both first-time assignments at ord 15/16, so pass 0 cannot assign either; pass 1 copies `used1`, which does not contain regs_someone_prefers. The complementary conflict route is empty too: j's live range strictly contains complement's, so no allocno can conflict with complement without also conflicting with j. `allocno_pri` is therefore the sole lever on this seat.

## [s19] Duplicating the inner loop's `j++` latch into the exit arms lifts reg_n_refs enough to re-seat j on $t4, and cross-jumping re-merges the copies to identical bytes (the s11 `dst++` precedent on this same function).
- mechanism: duplicated-statement-into-arms: flow.c counts each copy separately with loop-depth weighting (+6 per extra copy at depth 3), lifting global.c's allocno_pri; jump2's cross-jump was expected to merge the copies back so the bytes are unchanged.
- probe: h1 chassis (chassis_s18_h1_head_exact_28.c, 28/173). Three routings: b2 all four exit paths increment j themselves, b3 only the blend-else path, b4 blend-else plus both last-frame arms. sandbox --disable all on each; BB2_ALLOC_DEBUG table for b3.
- result: b3 44 at 174 insns, b2 45 at 175, b4 47 at 174. Cross-jumping does NOT re-merge these copies (unlike the dst++ case), so every copy costs a real instruction. The alloc table also shows the lift overshoots: j goes to nrefs 17 / livelen 60 / pri 11333, is allocated at ord=10 and takes $t1 instead of $t4 (complement does land on $t5). Banked as rejected/s19-j-inc-duplicated-into-arms-not-crossjump-merged-44.c and rejected/s19-j-inc-duplicated-four-arms-45.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis re-measured this session at 28/173, no FAKE constructs present in any of the three forms

## [s19] A depth-2 reference to `j` added in the head block or the inner-loop preheader buys the +2 reg_n_refs that prices j above complement (nrefs 11 -> 13, pri 6610 > 6111), restoring the target's $t4 seat on the h1 chassis.
- mechanism: pri = floor_log2(nrefs)*nrefs*10000/livelen and flow.c weights reg_n_refs by loop depth, so one extra reference in the outer-do-while body is worth +2 refs; nrefs 13 at livelen 59 is pri 6610, inside the (6111, 6964) window between complement and factor.
- probe: Two spellings on the h1 chassis: c2 = a redundant `j = 0;` as the first statement inside the `if (total > 0)` guard (depth-2 write); c4 = `s32 complement = (blend_base - factor) + j;` (depth-2 read, value-neutral since j == 0 there). sandbox --disable all plus BB2_ALLOC_DEBUG alloc tables (tmp/grind/func_8003DE14/s19/alloc_c2, alloc_c4).
- result: Both score 28 / 173, byte-identical to h1, and both alloc tables are identical to h1's line for line (j stays nrefs=11 livelen=59 pri=5593 -> $t5). cse2 runs before flow.c's life_analysis and knows j == 0 at every pre-loop site, so it folds the read and deletes the redundant store before any reference is counted. Banked as rejected/s19-redundant-depth2-j-store-cse2-deletes-28.c and rejected/s19-complement-plus-j-depth2-read-cse2-folds-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis at 28/173, no FAKE constructs present

## [s19] global.c's register-preference machinery (regs_someone_prefers) can be used to make `complement` skip $t4 and leave it for the later-allocated `j`.
- mechanism: prune_preferences (global.c:911-929) records for each allocno the registers preferred by every lower-priority conflicting allocno, and find_reg excludes that set when choosing a hard register.
- probe: Read find_reg's two-pass structure (tools/gcc-2.7.2/global.c:994-1080) against the measured h1 alloc table (complement ord=15 -> hardreg 12, j ord=16 -> hardreg 13).
- result: The exclusion applies only in pass 0, and pass 0 additionally ORs in the complement of regs_used_so_far with the comment 'we never allocate a register for the first time in pass 0' (global.c:997-1001); pass 1 copies used1, which does not contain regs_someone_prefers. $t4 and $t5 are both first-time assignments at ord 15/16, so no preference construct can reach this decision. The complementary conflict route is empty as well: j's live range strictly contains complement's, so every allocno conflicting with complement also conflicts with j. allocno_pri is the sole lever on this seat.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis alloc table at 28/173, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/global.c:1000

## [s19] livelen(j) is a controllable quantity on the h1 chassis: moving the `s32 j = 0;` statement earlier among the five head statements lengthens j's live range and re-prices it, while complement's and factor's live lengths stay pinned.
- mechanism: reg_live_length is computed by flow.c on the pre-scheduling RTL, so j's range starts where expand emits its initialiser; complement's def is pinned to the first insn after the guard branch and factor's to the div/mflo pair.
- probe: All five positions of `s32 j = 0;` in the head list (f1 first, f2 2nd, f3 3rd, f4 4th i.e. just before factor, h1 last); sandbox --disable all on each plus BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s19/alloc_f1 .. alloc_f4).
- result: CONFIRMED. livelen(j) = 59 for h1 and f4 (both 28), 60-61 for f3 (29), 61 for f1 and f2 (30); all five at 173 insns. complement holds livelen 54 / pri 6111 and factor livelen 56 / pri 6964 in every spelling. The f1 chassis (livelen 61, pri 5409) is therefore the one that pairs with a +3 depth-3 reference: nrefs 14 at livelen 61 is pri 6885 (inside the window), whereas nrefs 14 at livelen 59 is 7118 and would steal factor's $t3. Banked as chassis_s19_f1_jfirst_livelen61_30.c.
- verdict: CONFIRMED


## [s20] The incumbent and h1 differ only in WHICH of the j / complement pair is assigned first; `reg_live_length` follows ASSIGNMENT order while the pseudo number follows DECLARATION order, so the two are independent levers.
- mechanism: flow.c computes reg_live_length on the pre-scheduling RTL, where each local's def sits where `expand` emitted its assignment; `global_alloc` numbers allocnos by pseudo number, which `expand` assigns in declaration order. global.c allocates in descending allocno_pri and hands each allocno the lowest free hard register, so the later-assigned member of an equal-nrefs pair has the SHORTER live range, the HIGHER priority, and takes $t4.
- probe: four spellings of the guard block on the incumbent chassis, all 173 insns. e2 = decl j,complement / assign j,complement. e3 = bare decls complement,j / assign j,complement. e4 = bare decls j,complement / assign complement,j. base26 = the incumbent (decl+assign complement,j). sandbox --disable all + BB2_ALLOC_DEBUG on each (tmp/grind/func_8003DE14/s20/alloc_e2, alloc_e3, alloc_e4, alloc_base26).
- result: CONFIRMED. e2 30, e3 30, e4 26, base26 26. In all four the member assigned SECOND carries livelen 54 / pri 6111 and takes hardreg 12 ($t4); the member assigned FIRST carries livelen 55 / pri 6000 and takes hardreg 13. The pseudo numbers move with declaration order and never enter the decision (no tie). This also corrects s19's ledger: the incumbent's p115 is complement and p116 is j, the reverse of h1's mapping, so the incumbent's seats were ALREADY the target's and only h1's are inverted. e4 banked as memory/grind/func_8003DE14/chassis_s20_e4_decl_assign_split_26.c.
- verdict: CONFIRMED

## [s20] The head-exact (h1) chassis and the target's j=$t4 seat are mutually exclusive at nrefs 11, because `factor`'s division splits the head into four basic blocks and leaves the `blez` in a two-insn block whose only delay-slot candidate is j's own def.
- mechanism: reorg.c's fill_simple_delay_slots scans backward from the jump within its own basic block. asm/funcs/func_8003DE14.s:50-80 shows `factor`'s `div $zero,$v0,$s2` followed by PsyQ's trap checks (`bnez $s2`+`break 7`, `bne $s2,$at`, `bne $v0,$at`+`break 6`), four branches that split the head; the block ending in `blez $v1` is just `mflo $t3` + the blez. So the only way to put `move j,zero` in the slot is to have j's def inside that block — which places it BEFORE complement's def (the first insn of the fall-through block), which makes LL(j) > LL(complement), which inverts the priority compare.
- probe: asm/funcs/func_8003DE14.s:50-80 read directly (target rows 70-77); h1 / e2 / e3 / e4 / base26 alloc tables and scores; s18's BB2_DBR_DEBUG trace re-read against the block structure.
- result: KILLED. The minimum live-range gap between the pair is 1 insn (e2/e3: j 55 vs complement 54), which still prices j at 6000 < 6111, and the head-exact h1 spelling pays 5 (59 vs 54 -> 5593). The fall-through fill is not an escape: e2 and e3 put `move j,zero` first in the fall-through block and still score 30, so reorg does not take it from there. Every head-exact spelling measured across s18/s19/s20 therefore loses the seat, and every seat-correct spelling pays the 4-insn head residual. Scores: base26 26, e4 26, h1 28, e2 30, e3 30, f1 30.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), incumbent (26/173) and h1 (28/173) chassis re-measured this session, no FAKE constructs present in e2/e3/e4/g1-g4

## [s20] The `(S + P) - P` chain extender is byte-neutral on this function and lifts the PASSENGER's reg_n_refs, not the subject's, by +2 per loop-depth level.
- mechanism: toplev.c:2984 runs flow_analysis (which fills reg_n_refs and reg_live_length) once, BEFORE combine_instructions at toplev.c:3004, and life analysis is never re-run; combine then folds `(S + P) - P` back to S and deletes both insns, so the references flow.c counted survive into global.c's allocno_pri with zero byte cost. Because combine removes a def by forwarding it into its single use, the SUBJECT's reference migrates rather than multiplies; the passenger, which appears once in each of the two chain insns, gains 2 references at that site's loop depth.
- probe: three spellings on the h1 / f1 chassis, each scored with sandbox --disable all and dumped with BB2_ALLOC_DEBUG. d3 = `b_shift = (((bp + j) - j) + b_src) >> 5;` (j as passenger, depth 3). d2 = `jt = j + factor; jt = jt - factor;` feeding the latch (j as SUBJECT, factor as passenger, depth 3). d5 = d3 on the f1 chassis.
- result: CONFIRMED, and the direction matters. All three build 173 insns (byte-neutral). d3: nrefs(j) 11 -> 17, livelen 59 unchanged, pri 11525, j jumps to ord=11 and takes hardreg 9 ($t1); score 33. d2: nrefs(j) UNCHANGED at 11 while nrefs(factor) goes 13 -> 19 (pri 13333); score 34. d5: nrefs(j) 17 at livelen 61, pri 11147; score 35. A staged copy (d1, `jt = j;` feeding the latch) is removed before flow.c counts anything — its alloc table is byte-identical to h1's — and `(j - j)` (d4) is folded by fold-const at tree level. Banked as rejected/s20-depth3-passenger-byte-neutral-nrefs17-overshoots-33.c, rejected/s20-latch-chain-extender-subject-ref-conserving-34.c, rejected/s20-staged-copy-to-latch-invisible-to-flow-28.c.
- verdict: CONFIRMED

## [s20] A +2 (depth-1) or +4 (depth-2) passenger lift on `j` lands pri(j) inside the (6111, 6964) window and restores the target's seats on the head-exact chassis.
- mechanism: pri = floor_log2(nrefs)*nrefs*10000/livelen. nrefs 13 at livelen 59 is 6610 and nrefs 13 at livelen 61 is 6393, both strictly inside the window between complement (6111) and factor (6964); nrefs 15 needs livelen 65-73. The `(S + P) - P` extender was proved byte-neutral by d2/d3, so the only question was a site for it outside the inner loop.
- probe: three passenger sites, all on the h1 chassis, all 173 insns, each with a BB2_ALLOC_DEBUG table. p1 = `s32 complement = ((blend_base - factor) + j) - j;` (depth 2, first insn of the guard). p5 = `if (((total + j) - j) > 0) {` (depth 2, the guard test). p8 = `s32 blend_base = (0x1000 + j) - j;` with `s32 j = 0;` hoisted to the `if (count > 0)` block and `j = 0;` left in the head (depth 1). Plus p2 = p1 on the f1 chassis, and p6/p7 = byte-neutral neutral-passenger padding of j's live range (`dst = (dst + total) - total;`, and the same on `src`).
- result: KILLED for every out-of-inner-loop site. p1 28, p5 28, p8 28, p2 30 — and all four alloc tables are byte-identical to their chassis baseline (nrefs(j) 11, livelen 59 / 61, pri 5593 / 5409). cse2 runs before flow.c and knows `j == 0` at every program point outside the inner loop, so it constant-folds the whole chain away before any reference is counted; the same mechanism that killed s19's c2/c4. The live-range padding route also fails: p6 and p7 do lengthen j to 62 / 63 and complement to 55 / 56 but lift the padded variables' own counts and re-price the blend arm (both 50 / 173). Inside the inner loop the passenger survives (d3) but the quantum is +6, which prices j at 11525 and re-seats it on $t1. Banked as rejected/s20-depth2-passenger-on-complement-cse2-folds-j-is-0-28.c, rejected/s20-depth2-passenger-on-guard-test-cse2-folds-28.c, rejected/s20-depth1-passenger-on-blend-base-cse2-folds-j-is-0-28.c, rejected/s20-neutral-passenger-livelen-padding-50.c, rejected/s20-two-neutral-passengers-livelen-padding-50.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis re-measured this session at 28/173 and f1 at 30/173, no FAKE constructs present (the passenger chains are un-annotated probes, never proposed as candidates)

## [s20] livelen(factor) = 56 can be shortened below 54 by moving the `factor` statement through the head or by naming the numerator, which would raise the window's upper bound and let a +3 lift on j pay at score 28 instead of 30 (s19 frontier item 2).
- mechanism: pri(factor) = 3*13*10000/livelen, so livelen 54 gives 7222 > 7118 = pri(j) at nrefs 14 / livelen 59. factor's def is the div/mflo pair and s19 had only ever moved `j = 0` relative to it, never the division itself.
- probe: four head spellings on the h1 chassis, each with a BB2_ALLOC_DEBUG table. g1 = factor first in the head (before total/src/dst). g2 = factor second. g3 = factor third. g4 = numerator split into `s32 num = (i + 1) << 12;` with `s32 factor = num / count;`. sandbox --disable all on each.
- result: KILLED. All four score 28 / 173 and all four report factor at nrefs 13 / livelen 56 / pri 6964, identical to h1. factor's live range is pinned by the PsyQ division trap checks: the `div` can issue anywhere in the head but the `mflo` that defines factor's pseudo is forced into the two-insn block immediately before the `blez` (asm/funcs/func_8003DE14.s:72-74), so the distance from factor's def to the end of the inner loop is invariant under head statement order. Banked as rejected/s20-factor-first-in-head-livelen-pinned-56-28.c and rejected/s20-numerator-split-factor-livelen-pinned-56-28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 chassis re-measured this session at 28/173, no FAKE constructs present

## [s20] A dead store to `j` is a byte-free way to add references, because flow.c counts reg_n_refs on the RTL it sees.
- mechanism: reg_n_refs is incremented in mark_set_1 / mark_used_regs (flow.c:2081, 2329, 2515, 2725); a store that no later insn reads is still an insn at that point, so one might expect it to be counted before being deleted.
- probe: read propagate_block's dead-insn path (tools/gcc-2.7.2/flow.c:1476-1517) and cross-check against s19's measured c2 form (a redundant `j = 0;` inside the guard, alloc table identical to h1's).
- result: KILLED (class). `int insn_is_dead = insn_dead_p (PATTERN (insn), old, 0) && ! INSN_VOLATILE (insn);` is evaluated BEFORE any reference bookkeeping, and on the final pass a dead insn is turned into a NOTE_INSN_DELETED and the loop does `goto flushed`, skipping mark_set_regs and mark_used_regs entirely. So no dead store anywhere in this function can lift reg_n_refs, which also rules out the whole dead-store / self-assign family as a priority lever here — independently of the fact that it would require a FAKE annotation.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float), h1 and incumbent chassis, no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/flow.c:1490

## [s20] The incumbent and h1 differ only in which of the j / complement pair is ASSIGNED first: reg_live_length follows assignment order while the pseudo number follows declaration order, so the two are independent levers and the later-assigned member of an equal-nrefs pair takes $t4.
- mechanism: flow.c computes reg_live_length on the pre-scheduling RTL, where each local's def sits where expand emitted its ASSIGNMENT; global_alloc numbers allocnos by pseudo number, which expand assigns in DECLARATION order. global.c allocates in descending allocno_pri and hands each allocno the lowest free hard register, so the shorter live range wins $t4.
- probe: Four guard-block spellings on the incumbent chassis (e2 decl j,complement / assign j,complement; e3 decl complement,j / assign j,complement; e4 decl j,complement / assign complement,j; base26 the incumbent), each scored with sandbox --disable all and dumped with BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s20/alloc_e2, alloc_e3, alloc_e4, alloc_base26).
- result: CONFIRMED. e2 30, e3 30, e4 26, base26 26, all 173 / 173 insns. In all four the member assigned SECOND carries livelen 54 / pri 6111 and hardreg 12 ($t4); the member assigned FIRST carries livelen 55 / pri 6000 and hardreg 13 ($t5). This corrects s19's ledger: the incumbent's p115 is complement and p116 is j (the reverse of h1's mapping), so the incumbent's seats were already the target's and only h1's are inverted. e4 banked as memory/grind/func_8003DE14/chassis_s20_e4_decl_assign_split_26.c.
- verdict: CONFIRMED

## [s20] On this chassis the head-exact (h1) delay-slot fill and the target's j=$t4 seat cannot both be had at nrefs 11, because factor's division splits the head into four basic blocks and leaves the blez in a two-insn block whose only eligible delay-slot candidate is j's own def.
- mechanism: reorg.c's fill_simple_delay_slots scans backward from the jump inside its own basic block. asm/funcs/func_8003DE14.s:50-80 shows factor's div followed by PsyQ's trap checks (bnez $s2 + break 7, bne $s2,$at, bne $v0,$at + break 6) - four branches - so the block ending in blez $v1 is just mflo $t3 plus the blez. Putting move j,zero in the slot requires j's def inside that block, which places it before complement's def (the first fall-through insn), which makes livelen(j) > livelen(complement) and inverts the priority compare.
- probe: Read asm/funcs/func_8003DE14.s:50-80 directly; measured e2, e3, e4, base26, h1 and f1 with sandbox --disable all plus BB2_ALLOC_DEBUG alloc tables; re-read s18's BB2_DBR_DEBUG trace against the recovered block structure.
- result: KILLED. The smallest live-range gap the pair can be spelled with is 1 insn (e2/e3: j 55 vs complement 54), which still prices j at 6000 < complement's 6111; the head-exact h1 spelling pays 5 (59 vs 54 -> 5593). The fall-through fill is not an escape either: e2 and e3 put move j,zero first in the fall-through block and still score 30, so reorg does not take it from there. Scores: base26 26, e4 26, h1 28, e2 30, e3 30, f1 30, all 173 insns.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float); incumbent chassis re-measured this session at 26/173 and h1 at 28/173; no FAKE constructs present in e2/e3/e4

## [s20] The (S + P) - P chain extender is byte-neutral on this function and lifts the PASSENGER's reg_n_refs rather than the subject's, by +2 per loop-depth level.
- mechanism: toplev.c:2984 runs flow_analysis (which fills reg_n_refs and reg_live_length) once, before combine_instructions at toplev.c:3004, and life analysis is never re-run, so references on insns combine later folds away are counted permanently. combine removes a def by forwarding it into its single use, so the SUBJECT's reference migrates rather than multiplies, while the passenger - named once in each of the two chain insns - gains 2 references at that site's loop depth.
- probe: d3 = `b_shift = (((bp + j) - j) + b_src) >> 5;` (j as passenger, depth 3, h1 chassis). d2 = `jt = j + factor; jt = jt - factor;` feeding the latch (j as subject, factor as passenger, depth 3). d5 = d3 on the f1 chassis. d1 = a plain staged copy `jt = j;` feeding the latch. d4 = `(j - j)`. sandbox --disable all plus BB2_ALLOC_DEBUG on each.
- result: CONFIRMED, with the direction as the key finding. All three chain forms build 173 insns (byte-neutral). d3: nrefs(j) 11 -> 17, livelen 59 unchanged, pri 11525, j lifted to ord=11 and seated on hardreg 9 ($t1); score 33. d2: nrefs(j) UNCHANGED at 11 while nrefs(factor) went 13 -> 19 (pri 13333); score 34. d5: nrefs(j) 17 at livelen 61, pri 11147; score 35. The staged copy (d1) is removed before flow.c counts anything (alloc table byte-identical to h1's, 28) and (j - j) is folded by fold-const at tree level (d4, 28).
- verdict: CONFIRMED

## [s20] A +2 (depth-1) or +4 (depth-2) passenger lift on j lands pri(j) inside the (6111, 6964) window and restores the target's seats on the head-exact chassis.
- mechanism: pri = floor_log2(nrefs)*nrefs*10000/livelen, so nrefs 13 at livelen 59 is 6610 and at livelen 61 is 6393, both strictly between complement's 6111 and factor's 6964; nrefs 15 would need livelen 65-73. The chain extender was proved byte-neutral by d2/d3, so the only open question was a site for it outside the inner loop.
- probe: p1 = `s32 complement = ((blend_base - factor) + j) - j;` (depth 2, first guard insn). p5 = `if (((total + j) - j) > 0) {` (depth 2, the guard test). p8 = `s32 blend_base = (0x1000 + j) - j;` with `s32 j = 0;` hoisted to the if (count > 0) block and `j = 0;` left in the head (depth 1). p2 = p1 on the f1 chassis. p6 / p7 = byte-neutral neutral-passenger live-range padding (`dst = (dst + total) - total;`, plus the same on src). sandbox --disable all plus BB2_ALLOC_DEBUG on each.
- result: KILLED for every site outside the inner loop. p1 28, p5 28, p8 28, p2 30, all 173 insns, and every alloc table is byte-identical to its chassis baseline (nrefs(j) 11, livelen 59 / 61, pri 5593 / 5409). cse2 runs before flow.c and knows j == 0 at every program point outside the inner loop, so it constant-folds the whole chain away before any reference is counted - the same mechanism that killed s19's c2 and c4. The live-range padding route also fails: p6 / p7 do lengthen j to 62 / 63 and complement to 55 / 56 but lift the padded variables' own counts and re-price the blend arm (both 50 / 173). Inside the inner loop the passenger survives (d3) but the quantum is +6.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float); h1 chassis re-measured this session at 28/173 and f1 at 30/173; the passenger chains are un-annotated probes, never proposed as candidates, and no other FAKE construct was present

## [s20] livelen(factor) = 56 can be shortened below 54 by moving the factor statement through the head or by naming the numerator, raising the window's upper bound so a bigger lift on j still pays (s19 frontier item 2).
- mechanism: pri(factor) = 3*13*10000/livelen, so livelen 54 would give 7222 > 7118 = pri(j) at nrefs 14 / livelen 59. s19 had only ever moved `j = 0` relative to the division, never the division itself.
- probe: Four head spellings on the h1 chassis: g1 factor first, g2 factor second, g3 factor third, g4 numerator split into `s32 num = (i + 1) << 12;` with `s32 factor = num / count;`. sandbox --disable all plus BB2_ALLOC_DEBUG on each.
- result: KILLED. All four score 28 / 173 and all four report factor at nrefs 13 / livelen 56 / pri 6964, identical to h1. The div may issue anywhere in the head, but the mflo that defines factor's pseudo is pinned into the two-insn block immediately before the blez by the trap-check blocks (asm/funcs/func_8003DE14.s:72-74), so the distance from factor's def to the end of the inner loop is invariant under head statement order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float); h1 chassis re-measured this session at 28/173; no FAKE constructs present

## [s20] A dead store to j is a byte-free way to add references, because flow.c counts reg_n_refs on the RTL it is handed.
- mechanism: reg_n_refs is incremented in mark_set_1 / mark_used_regs (flow.c:2081, 2329, 2515, 2725), so an insn that is present when propagate_block walks the block might be counted before a later pass removes it.
- probe: Read propagate_block's dead-insn path (tools/gcc-2.7.2/flow.c:1476-1517) and cross-check against s19's measured c2 form (a redundant `j = 0;` inside the guard whose alloc table came back identical to h1's).
- result: KILLED as a class. insn_is_dead is computed before any reference bookkeeping, and on the final pass a dead insn is turned into a NOTE_INSN_DELETED and the loop does `goto flushed`, skipping mark_set_regs and mark_used_regs entirely. No dead store anywhere in this function can lift reg_n_refs, which removes the dead-store / self-assign family as a priority lever here independently of its FAKE-annotation requirement.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-10 (post -mel, post -msoft-float); incumbent and h1 chassis; no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/flow.c:1490


## s21 (forensics) - hypotheses

### CONFIRMED
- H21-1  A `(S + j) - j` chain extender placed AFTER the inner loop (where j is
  the loop-exit value, not a cse2-known constant) survives cse2, is counted by
  flow.c, and is folded away by combine - lifting nrefs(j) 11 -> 15 and
  livelen(j) 59 -> 73, pricing j at 6164, inside the s19 window (6111, 6964).
  On the head-exact h1 chassis this restores the target's j=$t4 /
  complement=$t5 seats and takes the score 28 -> 23 at 173 insns, with rows
  0..87 byte-identical to asm/funcs.  Best site measured: the LoadImage `dst`
  argument (q3).  MEASURED ON: HEAD 2026-09-11, h1 chassis re-measured 28/173.
- H21-2  A depth-3 `(S + g_src) - g_src` chain extender on the blue sum lifts
  nrefs(g_src) 12 -> 18 / pri 30000 -> 60000, which makes global.c allocate
  g_src before px: g_src takes $v1 and px takes $a0 (the target's pair) while
  r_src keeps $a1.  Worth 23 -> 16 on the q3 chassis and 26 -> 19 on the s13
  incumbent (y2).  MEASURED ON: HEAD 2026-09-11, both chassis re-measured.
- H21-3  The two lifts are independent and compose: h1 28, +j 23, +g_src 22,
  +both 16.

### KILLED (all instance kills, HEAD 2026-09-11, chassis as stated)
- H21-4  Moving the b_src computation later in the blend arm changes px's
  livelen and re-orders the blend allocnos.  KILLED: four placements (t1-t4),
  declaration pinned so pseudo numbers do not move, all produce alloc tables
  identical to q3's row for row (livelen(px) = 11 in every one) and all score
  23.  Statement order inside the blend arm does not move a live range.
- H21-5  b_src's $v0 seat is a priority question that a passenger lift can
  move.  KILLED: v1 lifts b_src to pri 65454 / ord=1 and it STILL takes
  hardreg 2.  The seat is decided by find_reg's lowest-free rule over the
  conflict set, not by ordinal position.
- H21-6  A named local for the channel sum becomes the $v0-holding allocno the
  target has and pushes b_src to $a0.  KILLED in both spellings: one shared
  `sum` (w1/w2) does take $v0 but conflicts with all three channels and scores
  52; three per-channel sums (x1/x2) are once-written/once-read, so combine
  folds them back and they never reach the allocno table (score unchanged).
- H21-7  Hoisting the g_src / b_src declarations above `px` wins the
  global.c:654 lower-pseudo tie-break.  KILLED: u1/u2 restructure the arm into
  116 build insns and score 100.
- H21-8  `j` reused (sanctioned variable-reuse family) as the carrier for the
  post-loop lift lands the same window.  KILLED: r1 (j carries new_y) prices j
  at 10967 / ord=11 and loses an insn (172); r3 (j stages the LoadImage
  argument) prices it at 7627 / ord=14.  Both are outside (6111, 6964).
- H21-9  Any post-loop carrier will do.  KILLED: of six carriers only the
  LoadImage dst argument lands the window - q1/q4 (new_y addends) price j at
  7258, q2 (the i latch) lands 6164 but re-prices the outer loop to 60.

## [s21] A combine-foldable (S + j) - j chain extender placed AFTER the inner loop - where j is the loop-exit value rather than the cse2-known constant 0 - survives cse2, is counted by flow.c, and lifts nrefs(j) 11 -> 15 with livelen(j) 59 -> 73, pricing j at 6164 inside the s19 window (6111, 6964) and seating j on $t4 / complement on $t5 on the head-exact h1 chassis.
- mechanism: flow_analysis (toplev.c:2984) fills reg_n_refs / reg_live_length ONCE and is never re-run before combine_instructions (toplev.c:3004) deletes the chain, so the references are priced by global.c allocno_compare but cost zero bytes. s20 killed the depth-1/depth-2 lift on j, but every site it measured (p1 on complement, p5 on the guard test, p8 on blend_base) sits BEFORE the inner loop where cse2 proves j == 0 and folds the expression away.
- probe: q3 = h1 + LoadImage((s32)rect, ((s32)dst_buf + j) - j); built with BB2_ALLOC_DEBUG and scored with the honest sandbox; alloc table read row by row (ord 15 = pseudo 115 j, hardreg 12; ord 16 = pseudo 116 complement, hardreg 13).
- result: score 23, build_insns 173, target_insns 173 (byte-neutral). Rows 0..87 of the function are byte-identical to asm/funcs/func_8003DE14.s - the 4-insn head residual that has been on the frontier since s13 is closed. Carrier sweep: q1/q4 (new_y addends) price j at 7258 -> ord 14; q2 (the i latch) lands 6164 but re-prices the outer loop, score 60; r1 (j reused for new_y) 10967 / 172 insns / 39; r3 (j staging the LoadImage arg) 7627 / 31.
- verdict: CONFIRMED

## [s21] A depth-3 chain extender on the blue sum lifts nrefs(g_src) 12 -> 18 and pri 30000 -> 60000, which makes global.c allocate g_src before px so g_src takes $v1 and px takes $a0 - the target's pair - while r_src keeps $a1.
- mechanism: global.c:650-654 orders allocnos by descending pri = floor_log2(nrefs)*nrefs*10000/livelen and find_reg hands each the lowest-numbered non-conflicting hard register; px (pseudo 122, pri 32727) previously preceded g_src (pseudo 126, pri 30000). The lift is priced by flow.c and deleted by combine, so it is byte-neutral.
- probe: u3 = q3 + b_shift = (((bp + b_src) + g_src) - g_src) >> 5; plus the independent ablations y1 (h1 + g_src lift only) and y2 (s13 incumbent + g_src lift only).
- result: u3 score 16 / 173 insns; y1 22; y2 19. Alloc table: ord=1 p126 g_src hardreg 3 ($v1), ord=3 p122 px hardreg 4 ($a0), ord=6 p123 r_src hardreg 5 ($a1). The j lever and the g_src lever are independent and compose (28 -> 23 -> 16).
- verdict: CONFIRMED

## [s21] Moving the b_src computation later inside the blend arm lengthens px's live range and re-orders the blend allocnos.
- mechanism: px's last use is the >> 7 shift that feeds b_src; if that shift moved two insns later, livelen(px) would go 11 -> 13, pricing px at 27692 and letting g_src be allocated first without any chain extender.
- probe: t1-t4: the b_src assignment placed at four sites between r_src = r * factor and bp = b_src * complement, with s32 b_src; pinned at the original declaration site so pseudo numbers do not move. Alloc tables compared row by row against q3.
- result: All four alloc tables are identical to q3's, including livelen(px) = 11, and all four score 23 at 173 insns. Statement order inside the blend arm did not move any live range in these four placements.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); q3 chassis re-measured this session at 23/173; the q3 j-lift FAKE construct was present in all four forms

## [s21] b_src's $v0 seat is a priority question, so a passenger lift on b_src (or on px, r_src or bp) moves it onto the target's $a0.
- mechanism: if b_src's ordinal position changed, find_reg would be choosing from a different free set and could land hardreg 4.
- probe: v1 (depth-3 passenger on b_src, pri 65454 -> ord=1), v2 (on px), v3 (on r_src), v4 (on bp), each built with BB2_ALLOC_DEBUG on the u3 chassis.
- result: v1 puts b_src at ord=1 and it STILL takes hardreg 2 ($v0); score stays 16. v2 and v3 push px or r_src around and regress to 23; v4 spills bp to local alloc and stays 16. b_src's seat is decided by find_reg's lowest-free rule over its conflict set, not by ordinal position - $v0 is simply free and non-conflicting at that point, while the target has a conflicting allocno holding $v0 across b_src's range.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; u3 chassis re-measured this session at 16/173, with both s21 FAKE chain extenders present

## [s21] Naming the channel sums as C locals creates the $v0-holding allocno the target has and pushes b_src off $v0 onto $a0.
- mechanism: the target uses $v0 for all three channel sums (rows 111, 114, 118); a named local would become a global allocno that conflicts with b_src and would take $v0 before it.
- probe: w1/w2 - one shared sum local reused for all three channel sums; x1/x2 - three per-channel locals r_sum / g_sum / b_sum, each once-written and once-read.
- result: The shared sum DOES become pseudo 136 (nrefs 18, pri 90000, hardreg 2) and DOES push b_src off $v0, but it conflicts with all three channel values and scatters g_src to $a2 and b_src to $a3: score 52 on both chassis. The three per-channel locals are folded back into the shift by combine, never appear in the allocno table, and leave both the alloc table and the score unchanged (16 / 23).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; u3 (16/173) and q3 (23/173) chassis, both s21 FAKE chain extenders present in the u3-derived forms

## [s21] Hoisting the g_src and b_src declarations above px wins the global.c:654 lower-pseudo tie-break for the blend pair without a chain extender.
- mechanism: pseudo numbers follow declaration order and allocno_compare breaks an exact priority tie by lower allocno number, so a renumbered g_src would precede px if their priorities were equalised.
- probe: u1 (g_src declared uninitialised at the px block level, assigned in place), u2 (same for g_src and b_src).
- result: Both restructure the blend arm - 116 build insns against 173 target insns - and score 100. The declaration could not be hoisted out of the inner block in these two spellings without changing what the arm compiles to.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; q3 chassis (23/173) with the q3 j-lift FAKE construct present


## s22 (rederive) - hypotheses

### CONFIRMED
- H22-1  The s21 incumbent's blue-channel variable reuse (`bp = b_src *
  complement; b_src = b * factor;`) is worth -1 insn when dropped: writing the
  blue channel as a single expression with no `bp` local (a2) scores 15 and makes
  rows 106-108 structurally identical to the target, with only the register
  differing.  MEASURED ON: HEAD 2026-09-11, u3 chassis re-measured 16/173, both
  s21 FAKE extenders present.
- H22-2  A shared `sum` local for the GREEN and BLUE channel sums (d2) is worth a
  further -1: 14/173.  The split matters - red+green (f1) and all three (d1) both
  score 30 because they add a global allocno and shift every lower-priority
  allocno one hard register; red only (f2) is single-use and combine folds it
  back (15, alloc table unchanged).  MEASURED ON: HEAD 2026-09-11, a2 chassis
  15/173, both s21 FAKE extenders present.
- H22-3  The remaining 11 blend insns are downstream of exactly one find_reg
  decision.  BB2_FINDREG_DEBUG=139 prints conflicts {3,5,6,7,8,16,29}, empty
  someone_prefers and a pass0_used that does not contain regno 2, so
  global.c's first-fit loop returns $v0 where the target uses $a0.

### KILLED (all instance kills, HEAD 2026-09-11, chassis as stated)
- H22-4  Inverting the blue channel's reuse so the PRODUCT lands back in b_src
  and b*factor goes to a fresh local reproduces the target's a0<-a0 pair.
  KILLED: a1 scores 38; applying the same inversion to all three channels (a3)
  scores 50.
- H22-5  A fully inline blend arm with no named channel locals reproduces the
  target's three-$v0-sums shape.  KILLED: b1 scores 43 - without the named
  carriers px/r_src/g_src lose their $a0/$a1/$v1 seats entirely.
- H22-6  Reusing `px` as the blue channel's carrier seats the blue temp on px's
  vacated $a0.  KILLED: c1 scores 23.
- H22-7  Accumulating the output word into a named `out` opened before the blue
  channel makes the $v0 accumulator conflict with the blue temp and pushes it to
  $a0.  KILLED in three spellings: e1 48, e2 41, e3 36 - the new allocno takes
  $v1 or $a1 and scatters g_src / px / r_src off the target's blend triple.
- H22-8  A multi-write shared sum that takes $v0 as a global allocno is a net
  win.  KILLED: it does fix rows 111-112 exactly (f1/d1 emit the target's
  `addu $v0,$t3,$a1` / `sra $a1,$v0,0xf`) but the extra allocno shifts
  complement $t5->$t6, factor $t3->$t4 and rp $t2->$t3; both score 30.
- H22-9  Chaining the whole blue channel through ONE variable (source, product
  and shifted result) extends its live range past row 120 so it conflicts with
  the or-chain accumulator on $v0.  KILLED: g1 31, g2 55, g4 31.
- H22-10  The 3-insn trip-test residual is decidable by the latch's expression
  shape.  KILLED: the $v1/$v0 pair is positional - `rect[3] * rect[2]` (h1)
  swaps the two offsets but not the registers; `rect[2]*rect[3] > j` (h3) and
  `(s32)rect[2] * rect[3]` (h2) are byte-identical to the incumbent latch;
  `total` re-read in the latch (h4) costs 2.
- H22-11  Operand order inside the three channel adds moves the sum's register.
  KILLED: red (i1) and green (i2) flips are byte-identical at 14; the blue flip
  (i3/i4) costs 16.

## [s22] The s21 incumbent's blue-channel variable reuse (bp = b_src * complement; b_src = b * factor;) costs one insn: writing the blue channel as a single expression with no bp local scores 15 instead of 16 and makes rows 106-108 structurally identical to the target, with only the register differing.
- mechanism: The reuse makes b_src a two-segment pseudo (written at the andi, re-written at the mflo of b*factor) whose second write forces the complement product into a separate register, so the mflo lands on a fresh temp instead of on b_src's own register. Without the reuse, b_src and the complement product share one register exactly as the target does (target andi $a0 / mult $a0 / mflo $a0; ours andi $v0 / mult $v0 / mflo $v0).
- probe: a2 = u3 with `b_shift = (((b_src * complement + b * factor) + g_src) - g_src) >> 5;` and the `bp` declaration deleted; honest sandbox score plus BB2_ALLOC_DEBUG table, side-by-side objdump rows 88-140.
- result: a2 = 15 / 173 build insns / 173 target insns. u3 re-measured this session at 16/173. Counter-ablations: a1 (invert the reuse so the product lands back in b_src) 38; a3 (invert all three channels) 50.
- verdict: CONFIRMED

## [s22] A shared `sum` local staging the GREEN and BLUE channel sums takes the a2 body from 15 to 14 at 173 insns, and the choice of which channels share the name is decisive: red+green and all-three both score 30, red-only is folded back by combine and scores 15.
- mechanism: A multi-write name survives combine and becomes a real carrier; a single-write name is once-written/once-read and combine folds it into the following shift, leaving the allocno table unchanged. A carrier that spans red as well becomes a global allocno high in the priority order, which adds one entry to allocno_order so every lower-priority allocno shifts one hard register (complement $t5 -> $t6, factor $t3 -> $t4, rp $t2 -> $t3). Sharing only green and blue keeps the carrier out of the global table.
- probe: d2 (green+blue), d1 (all three), f1 (red+green), f2 (red only), f3 (red+blue), d4 (d1 minus the g_src extender); each installed, scored with the honest sandbox and dumped with BB2_ALLOC_DEBUG.
- result: d2 = 14/173 (new floor); d1 = 30; f1 = 30; f2 = 15; f3 = 17; d4 = 43. f1/d1 DO emit the target's `addu $v0,$t3,$a1` / `sra $a1,$v0,0xf` pair on rows 111-112, so the mechanism is right and only the allocno-shift tax makes them lose.
- verdict: CONFIRMED

## [s22] On the d2 body the remaining 11 blend-arm insns (rows 106,107,108,111,112,114,115,118,119,123,124) are downstream of exactly one global.c find_reg decision: the blue temp's seat, $v0 in our build and $a0 in the target.
- mechanism: BB2_FINDREG_DEBUG=139 prints conflicts {3,5,6,7,8,16,29} (neither 2 nor 4), an empty regs_someone_prefers, and a pass0_used that omits regno 2, so find_reg's first-fit loop returns 2. With the blue temp on $a0 instead, $v0 becomes the lowest free register for all three channel sums - the target's rows 111/114/118 - and b_shift keeps $a0. Only two sets can put regno 2 into pass0_used: a conflicting allocno already holding $v0, or a later-allocated allocno that prefers $v0 (global.c set_preference fires only on a reg-reg copy where one side is already hard-numbered).
- probe: tmp/grind/func_8003DE14/s22/findreg.py d2 139 (BB2_FINDREG_DEBUG on the instrumented cc1 tools/gcc-2.7.2/cc1), plus row-by-row side-by-side of d2 against build/src/code6cac_c2.o.
- result: The three exclusion sets are banked in tmp/grind/func_8003DE14/s22/findreg_d2_139/stderr.log. Both escape routes are now named and one of them (a $v0-holding global allocno) is measured to cost more than it buys.
- verdict: CONFIRMED

## [s22] Accumulating the output word into a named `out` local opened before the blue channel makes the $v0 or-chain accumulator conflict with the blue temp and moves it to $a0.
- mechanism: Pseudo 118 (the or-chain accumulator) is allocated first at pri 90000 and holds $v0, but its live range is rows 120-125 and the blue temp's is 106-118, so they do not conflict. Naming the accumulator and opening it earlier should extend it backwards across the blue temp.
- probe: e1 (`out = (pixel & 0x8000) | r_ch | g_ch;` before the blue sum), e2 (`out` opened after red, compound-or per channel), e3 (`out = pixel & 0x8000;` at the top of the arm); each scored and dumped with BB2_ALLOC_DEBUG.
- result: e1 = 48, e2 = 41, e3 = 36. The new allocno outranks the blend triple: in e1 g_src moves off $v1 to $a1 and in e3 it moves to $a0, so px / g_src / r_src all lose the target's seats. The accumulator's live range was not extended onto the blue temp in any of the three.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); d2 chassis re-measured this session at 14/173, with both s21 FAKE chain extenders present in all three forms

## [s22] Chaining the whole blue channel through one variable - source, complement product and shifted result - extends its live range past row 120 so it conflicts with the or-chain accumulator on $v0 and is pushed to $a0.
- mechanism: In the target the blue value and b_shift share $a0 and b_shift is still live at row 123, which is after the accumulator's first def at 120; making them one C variable should make that overlap a single allocno's conflict.
- probe: g1 (a2 base, b_src carries source, product and shifted result, b_shift declaration deleted), g2 (same on the d2 base with the shared sum), g4 (g1 minus the g_src chain extender).
- result: g1 = 31, g2 = 55, g4 = 31. The merged variable lengthens the arm's critical live range and re-orders the blend allocnos instead of only adding the $v0 conflict.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; a2 chassis 15/173 and d2 chassis 14/173, both s21 FAKE chain extenders present

## [s22] A fully inline blend arm with no named channel locals, or `px` reused as the blue carrier, reproduces the target's three-$v0-sums shape.
- mechanism: Without named carriers every channel value is a short-lived temp, which is what the target's sums look like; reusing px for blue would seat the blue value on the register px vacates at row 105.
- probe: b1 (single `*dst = ...` expression, no channel locals at all) and c1 (`px = (((u32)px >> 7) & 0xF8) * complement;` with the b_src declaration deleted).
- result: b1 = 43: without the named carriers px / r_src / g_src lose the $a0 / $a1 / $v1 seats the s21 levers bought. c1 = 23: px's own allocno is re-priced and the blend pair inverts again.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; u3 chassis 16/173 (b1) and a2 chassis 15/173 (c1), both s21 FAKE chain extenders present

## [s22] The 3-insn trip-test residual (rows 127/128/130) is decidable by the latch's expression shape, as the s21 frontier proposed.
- mechanism: The two loads of rect[2] and rect[3] are short-lived locally-allocated temps, so local-alloc should decide them from the order in which the quantities are born within the latch block, and an operand swap or a staged read should flip them.
- probe: h1 `while (j < rect[3] * rect[2])`, h2 `while (j < (s32)rect[2] * rect[3])`, h3 `while (rect[2] * rect[3] > j)`, h4 `total = rect[2] * rect[3]; } while (j < total);` - each scored and the rows 127-131 read out side by side, not just the score.
- result: h1 = 14, h2 = 14, h3 = 14, h4 = 16. h1 swaps which OFFSET each load carries (lh $v1,6 then lh $v0,4) but leaves $v1 on the first load and $v0 on the second, so the register pair is attached to the emission slot rather than to the operand; h2 and h3 are byte-identical to the incumbent latch; h4 costs 2. Operand flips inside the channel adds are inert for red and green (i1/i2 = 14) and cost 16 on blue (i3/i4 = 30).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; d2 chassis 14/173 with both s21 FAKE chain extenders present

## s23 (2026-09-11) — REDERIVE

### CONFIRMED

**H-s23-1 — Reusing `px` as the blue channel's carrier (source byte and
complement product) lands the target's `$a0` blue seat and drops the floor.**
Mechanism: the target keeps the blue source and its `mult` result in px's own
register (`andi $a0,$v0,0xF8` / `mult $a0,$t5` / `mflo $a0`, rows t106-t108); a
separate `b_src` local is a distinct pseudo that local-alloc/global-alloc has no
reason to co-locate with px. Probe: `p1`/`p2` on the d2 chassis.
Result: d2 14 -> p1 12 -> p2 12 with t106-t108 byte-exact. VERDICT CONFIRMED.
NOTE: this VOIDS s22's instance kill of the same construct (forms b1 43 / c1 15,
measured on the u3 and a2 chassis before the shared `sum` local existed).

**H-s23-2 — All 12 remaining rows, including the 3 latch rows, are downstream of
ONE register seat: the `b * factor` global allocno holding `$v0` across the whole
inner-loop block.** Mechanism: `reg_preferred_class` of a mulsi3 result is
`LO_REG`, which is `CLASS_LIKELY_SPILLED_P`, so `local-alloc.c:472` refuses it a
quantity (`reg_qty = -1`) and it becomes a global allocno; `global.c` `find_reg`
first-fit gives it regno 2 (conflicts = {3,5,6,7,8,16,29}, someone_prefers = {}).
Probe: the s23 `.lreg` dump plus the v2/v3 latch respellings, which leave the 12
rows byte-identical. VERDICT CONFIRMED.

### KILLED (all instance scope, measured on HEAD 2026-09-11, p2 chassis at 12/173
unless stated, both s21 F1 chain extenders present)

**H-s23-3 — Staging `b * factor` into a named C local re-prices the allocno and
seats it off `$v0`.** Probes `s1` (top of arm) 47 / 171 insns, `s2` 41, `s3` 41,
`s4` (`bf + px` operand order) 41, `s5` (all three `*factor` products staged up
front) 49 / 167 insns. Naming the product makes it an ordinary local that dies
once and loses the `LO_REG` preference, so local-alloc seats it and the whole arm
re-seats around it. KILLED, instance.

**H-s23-4 — Re-partitioning the shared `sum` local across the channels reaches
the `$v0` seat on the px chassis.** Probes `r1` (all three) 44, `r2` (red+blue)
28, `r3` (blue only) 23, `r4` (distinct `sum`/`sum2`) 23, `r5` 12 (tie).
The s22 partition (`sum` for green+blue) remains the best. KILLED, instance.

**H-s23-5 — `b_src`'s declaration site or a late initialiser moves the blue
seat.** Probes `q4` (declared last among the source locals) 14, `q5` (declared
uninitialised, assigned immediately before its use) 14, both on the d2 chassis
with a byte-identical residual to d2. KILLED, instance.

**H-s23-6 — The 3-row latch residual (`lh $v0,4($s0)` / `lh $v1,6($s0)` /
`mult $v0,$v1`) is decidable from the latch expression.** Probes on the p2
chassis: `v1` `j != rect[2]*rect[3]` 14 / 172 insns, `v2`
`while (++j < rect[2]*rect[3])` 12, `v3` dropping the `total` local entirely and
testing `rect[2]*rect[3] > 0` inline 12, `v4` `(s32)`-cast product 12, `v5`
`rect[2]*rect[3] - j > 0` 14. v2 and v3 print exactly the same 12 rows as p2 —
the latch seat moves only with the block-wide `$v0` reservation. KILLED,
instance. (This extends s22's h1-h4 latch kill to the px chassis and to the
`!=`, `++j`, and no-`total` shapes.)

**H-s23-7 — An output-word accumulator or an or-chain re-association reaches the
seat on the new chassis.** Probes `u1` (`out` opened before the red channel) 70,
`u2` (named blue mask) 13, `u3` (or-chain re-associated) 42, `u4` (r_ch masked at
use) 12 (tie), `u5` (blue sum inlined, `sum` dropped) 23. KILLED, instance.

### LIVE FRONTIER (for s24)

1. **Get regno 2 out of the `b*factor` allocno's first-fit reach.** The cheapest
   measured-plausible route is a LOCAL quantity that local-alloc seats on `$v0`
   and whose range overlaps RTL insns 253-260 (the b*factor def-to-use window).
   The channel sums ARE local quantities in the p2 body but land on `$a1`/`$v1`/
   `$a0`; find the C shape that makes one of them the first local quantity
   local-alloc seats in block 10. Accept only forms with build_insns 173 whose
   BB2_ALLOC_DEBUG row for the b*factor allocno prints hardreg != 2.
2. **Drop the b*factor allocno's priority below the allocnos that would then take
   `$v0`.** `pri = floor_log2(nrefs)*nrefs*10000/livelen`; it is currently
   ord=10, nrefs=6, livelen=10, pri=12000, ahead of `gp` (10909) and `rp`
   (10000). A shape that lengthens its livelen without adding a reference (the
   `mult` emitted earlier, the `mflo` consumed later) pushes it down the order.
   Note s1-s5 show that NAMING the product is the wrong way to do this.
3. **Re-measure the two s21 F1 chain extenders on the p2 chassis.** The ablation
   numbers in s22's header (d2 minus g_src extender = 23, minus j extender = 21,
   neither = 28) are d2-relative; the px reuse changed the arm's allocation, so
   the g_src price may now be reachable in ordinary C. `tools/fake_ablate.py` on
   p2 is the first probe of s24 before any new spelling work.

## [s23] Reusing the existing `px` local as the blue channel's carrier - for both the source byte and the complement product - reproduces the target's blue seat ($a0) and lowers the honest floor below s22's d2 body.
- mechanism: The target keeps the blue source AND its mult result in px's own register: t106 `andi $a0,$v0,0xF8`, t107 `mult $a0,$t5`, t108 `mflo $a0`. A separate `b_src` local is a distinct pseudo with no reason to co-locate with px, and in our build it forced the blue product onto $v0. C-level variable reuse (the sanctioned variable-reuse-for-codegen-control family) is what merges the two live ranges before allocation sees them.
- probe: Built p1 (px carries the source only) and p2 (px carries the source and the product) on s22's d2 chassis, plus counter-ablations p3/p4/p5; scored each with `sandbox func_8003DE14 --disable all` and diffed the normalized opcode streams against build/src/code6cac_c2.o.
- result: d2 14 -> p1 12 -> p2 12 (all 173 build / 173 target). p2 makes t106-t108 byte-exact where p1 still emits `mflo $v0`; p3 (source+product folded into one statement) 14, p4 (px also carries the blue sum) 23, p5 46. p2 is the new banked candidate. This also VOIDS s22's instance kill of 'px reused as the blue carrier' (forms b1 43, c1 15), which was measured on the u3 and a2 chassis before d2's shared `sum` local existed.
- verdict: CONFIRMED

## [s23] All 12 remaining divergent rows - the three channel sums, the blue mask, and the three latch rows - are downstream of one register seat: the `b * factor` mulsi3 result is a GLOBAL allocno that takes $v0 and holds it across the whole inner-loop basic block.
- mechanism: A mulsi3 result has reg_preferred_class LO_REG; CLASS_LIKELY_SPILLED_P(LO_REG) is true and its reg_alternate_class is GR_REGS (not NO_REGS), so tools/gcc-2.7.2/local-alloc.c:472 sets reg_qty = -1 and local-alloc never seats it. It becomes a global allocno (BB2_ALLOC_DEBUG ord=10 nrefs=6 livelen=10 pri=12000) and global.c's find_reg first-fit hands it regno 2, because neither 2 nor 4 is in its conflict set and someone_prefers is empty. In the target $v0 is instead free scratch: each channel sum is born in $v0 and dies one insn later, and the latch's first load takes $v0 too.
- probe: pwsh tools/grinder/dump.ps1 func_8003DE14 on the p2 body; read the register table and the RTL of block 10 in tmp/grind/func_8003DE14/dumps/code6cac_c2.lreg (function region from line 15386) - reg 138 is the b*factor product ('used 6 times across 9 insns in block 10; pref LO_REG, else GR_REGS'), reg 134/128/140 are the three sums. Cross-checked with five latch respellings (v1-v5) whose full opcode diffs leave the 12 rows byte-identical.
- result: Confirmed by the dump plus the latch probes: v2 (`while (++j < rect[2]*rect[3])`) and v3 (drop the `total` local, test the product inline) both score 12 and print EXACTLY the same 12 rows as p2, so the latch's $v0/$v1 swap is not decidable from the latch expression - it moves only with the block-wide $v0 reservation. The dump also explains why the sums are tied to their sources: r_src, g_src, px and pixel all print 'dies in 2 places' (they are the reused C variables), failing the same local-alloc.c:472 reg_n_deaths==1 test, so combine_regs (local-alloc.c:1784) bails on every operand and the sums are fresh LOCAL quantities that cannot reach $v0.
- verdict: CONFIRMED

## [s23] Staging `b * factor` into a named C local re-prices its allocno and seats it off $v0.
- mechanism: Giving the product a C name was expected to change its reference count and live length and so its find_reg order; instead it converts the pseudo from a LO_REG-preferring global allocno into an ordinary local that dies once, which local-alloc seats directly and the whole arm re-seats around it.
- probe: Five placements of a named `bf = b * factor` on the p2 chassis: s1 at the top of the arm, s2 after the red channel, s3 after the green channel, s4 with `bf + px` operand order, s5 with all three `*factor` products staged up front. Scored with sandbox --disable all.
- result: s1 47 (and only 171 build insns), s2 41, s3 41, s4 41, s5 49 (167 build insns). Every form is far worse than the 12 baseline and several change the instruction count, so none is a partial win.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); p2 chassis at 12/173, with both s21 F1 chain extenders present

## [s23] Re-partitioning the shared `sum` local across the three channels reaches the $v0 seat on the px-reuse chassis.
- mechanism: The target's three sums all occupy $v0, which suggested one reused C carrier; the s22 partition (green+blue) was chosen before the px reuse existed, so the optimum partition could have moved with the chassis.
- probe: r1 `sum` for all three channels, r2 red+blue, r3 blue only, r4 two distinct locals `sum`/`sum2`, r5 `b*factor` named and added through `sum` - all on the p2 chassis; plus q1/q2/q3/q6 on the pre-px d2 chassis.
- result: r1 44, r2 28, r3 23, r4 23, r5 12 (tie, no new rows); q1 15, q2 17, q3 15, q6 30. The s22 green+blue partition remains optimal on both chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173 (r-series) and d2 chassis 14/173 (q-series), both s21 F1 chain extenders present

## [s23] The 3-row latch residual (lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1) is decidable from the shape of the loop-latch expression or from whether the trip count is held in a `total` local.
- mechanism: The two loads are short local quantities; the order in which local-alloc seats them would ordinarily follow the expression's operand order, so a different latch spelling should swap them.
- probe: v1 `j != rect[2]*rect[3]`, v2 `while (++j < rect[2]*rect[3])` with the increment moved into the latch, v3 dropping the `total` local entirely and testing `rect[2]*rect[3] > 0` inline in the guard, v4 `(s32)`-cast product, v5 `rect[2]*rect[3] - j > 0`. Scored, and the full normalized opcode diff read for v2 and v3.
- result: v1 14 (172 build insns - drops an instruction), v2 12, v3 12, v4 12, v5 14. v2 and v3 print exactly the same 12 divergent rows as p2, including the latch rows, so the latch seat does not move with the latch expression. This extends s22's h1-h4 latch kill to the px chassis and to the `!=`, `++j` and no-`total` shapes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s23] An output-word accumulator, an or-chain re-association, a named blue mask, or `b_src`'s declaration site reaches the blue/sum seats on the px-reuse chassis.
- mechanism: Each of these changes which pseudo is live across the b*factor product's def-to-use window, which is the window that would have to contain a $v0-seated local quantity for the product to be excluded from regno 2.
- probe: u1 `out` accumulator opened before the red channel, u2 named blue mask local, u3 or-chain re-associated as ((c|r)|(g|b)), u4 r_ch masked at its use instead of at its definition, u5 blue sum inlined into b_shift with `sum` dropped; q4 `b_src` declared last among the source locals and q5 `b_src` declared uninitialised and assigned immediately before its use (both on the d2 chassis).
- result: u1 70, u2 13, u3 42, u4 12 (tie, same rows), u5 23; q4 14 and q5 14 with a residual byte-identical to d2. None of these live ranges lands inside the b*factor window in a way that changes the seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173 (u-series) and d2 chassis 14/173 (q-series), both s21 F1 chain extenders present

## [s24] The engine's honest score is computed on objdump output WITHOUT `-z`, so runs of two-or-more consecutive zero words are elided on BOTH sides; the real streams are 179 instructions each and, aligned 1:1, the residual is EXACTLY 12 rows.
- mechanism: engine/score.py's normalized_insns() calls `objdump -dr --start-address/--stop-address`. GNU objdump's default (no `-z`) prints `...` in place of a run of identical zero words, so the three `nop; nop` pairs the target carries after each `mflo` (asm/funcs/func_8003DE14.s rows t97/98, t105/106, t113/114) never enter the instruction list - and neither do ours. Both objects are 0x2cc = 716 bytes = 179 words (objdump -t on build/src/code6cac_c2.o and on tmp/sandbox/func_8003DE14/code6cac_c2.o), so the nop padding is ALREADY identical and is not part of the residual. A naive side-by-side that reads asm/funcs/*.s on one side (nops present) and plain `objdump -d` on the other (nops elided) mis-aligns by 6 rows from t97 onward and invents a phantom scheduling / multu-pad divergence; s24 chased exactly that for four turns before `objdump -dz` resolved it. func_8003DE14 is NOT in multu_pad_funcs.txt and does not need to be.
- probe: objdump -t / objdump -dz row counts on both objects; tmp/grind/func_8003DE14/s24/ed2.py (a `-dz`, branch-target-masked, 1:1 difflib alignment) on the p2 body.
- result: 179 vs 179, 29 raw edits of which 17 are objdump spellings (`move` vs `addu x,y,zero`, `li` vs `addiu x,zero,N`, `fp` vs `s8`, hex vs decimal) and EXACTLY 12 are real: t117 t118 t120 t121 t123 t124 t125 t129 t130 t133 t134 t136. This is the same 12 the engine scores.
- verdict: CONFIRMED

## [s24] All 12 residual rows are one register-allocation pattern: the target routes every channel sum through a scratch register and puts the `b*factor` mflo in $t7, while our build ties each sum's destination to one of its own source operands' registers and puts that mflo in $v0.
- mechanism: Target rows: `addu $v0,$t2,$a1` / `sra $a1,$v0,15` (red), `addu $v0,$t1,$v1` / `sra $v1,$v0,10` (green), `mflo $t7` / `addu $v0,$a0,$t7` / `sra $a0,$v0,5` (blue), `andi $v1,$a0,0x7C00` / `or $v0,$v0,$v1` (blue mask), `lh $v0,4($s0)` / `lh $v1,6($s0)` / `mult $v0,$v1` (latch). Ours: `addu $a1,$t2,$a1` / `sra $a1,$a1,15`, `addu $v1,$t1,$v1` / `sra $v1,$v1,10`, `mflo $v0` / `addu $a0,$a0,$v0` / `sra $a0,$a0,5`, `andi $a0,$a0,0x7C00` / `or $v0,$v0,$a0`, `lh $v1,4($s0)` / `lh $v0,6($s0)` / `mult $v1,$v0`. In every case the target consumes a fresh register for the intermediate and writes the final channel value back into the operand's register one insn later; ours collapses the two steps onto the operand's register.
- probe: the s24 ed2.py alignment above, on the p2 body.
- result: The 12 rows are 4 independent seats - the red sum, the green sum, the blue mflo + sum, and the latch's two loads - all with the same shape. No row is a scheduling or instruction-count difference, and the whole residual is confined to the last 20 instructions of the blend arm plus the latch.
- verdict: CONFIRMED

## [s24] The s23 attribution "global.c's find_reg first-fit hands the b*factor allocno regno 2" is imprecise: global.c's FIRST pass gives it LO (hardreg 65); it reaches $v0 only through reload's retry_global_alloc. And local quantities in the blend block DO already reach $v0.
- mechanism: BB2_ALLOC_DEBUG on the p2 body prints `ord=9 pseudo=138 hardreg=65 nrefs=6 livelen=9 pri=13333`; hardreg 65 is LO_REG on this MIPS config (64=HI, 65=LO). The .greg dump then prints `Register 138 now in 2`, which is the retry path (tools/gcc-2.7.2/global.c:1317, retry_global_alloc, reached from reload1 when the LO seat is not satisfiable for every use). Separately BB2_QTY_DEBUG for block 10 prints `ord=0 qty=18 ... got=2`, `ord=2 qty=0 ... got=2`, `ord=3 qty=5 ... got=2`, `ord=4 qty=10 ... got=2`: four local quantities are seated on $v0 by local-alloc in the very block the residual lives in, and three of them (the `srl $v0,$a0,2` / `srl $v0,$a0,7` / `andi $v0,$t0,0x1F` source extractions) already match the target byte-for-byte.
- probe: tmp/grind/func_8003DE14/s24/alloc.sh (instrumented cc1 = tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1, then again with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1; output in tmp/grind/func_8003DE14/s24/alloc.txt and alloc_fn.txt.
- result: The full global ord table is banked in evidence.md. The practical consequence: s23's frontier item 1 ("find a C shape that makes a local quantity reach $v0") is already satisfied and is NOT the lever - the sums are losing $v0 to a tie with their own operand, not to the b*factor allocno's first-fit.
- verdict: CONFIRMED

## [s24] Both s21 F1 chain extenders are still load-bearing on the p2 (px-reuse) chassis; the g_src extender's price did NOT collapse when the blue channel lost its `b_src` pseudo.
- mechanism: s22's ablation numbers (d2 minus the g_src extender = 23, minus the j extender = 21, neither = 28) were d2-relative, and s23's frontier item 3 predicted the px reuse might have made the g_src price reachable in ordinary C.
- probe: a1 (`b_shift = sum >> 5;`), a2 (`LoadImage((s32)rect, (s32)dst_buf);`), a3 (both), each applied to memory/grind/func_8003DE14/candidate.c and scored with `sandbox func_8003DE14 --disable all`.
- result: a1 40, a2 19, a3 38, against the 12 baseline - so on this chassis the g_src extender is worth 28 and the j extender 7. Frontier item 3 of s23 is closed negative: the ordinary-C sweep for g_src's price does not reopen on the px chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, ablating each s21 F1 chain extender in turn

## [s24] Sharing ONE C carrier across channels for the complement products or for the `*factor` products unties the sum-to-operand register tie.
- mechanism: local-alloc.c:1784 combine_regs refuses to merge a destination with a source whose `reg_qty` is negative - which is what a pseudo that dies in more than one place gets - so making `rp` (or the `*factor` holder) carry two or three channels' values should have made the red/green adds allocate a fresh destination.
- probe: c1 `rp` carries the red AND green complement products (gp dropped); c2 `r_src` carries the red AND green `*factor` products; c3 both; c4 `rp` carries all three complement products; c5 `rp` also carries the green source byte. Scored, and the c1 body's full -dz alignment read.
- result: c1 33, c2 53 (175 build insns), c3 51, c4 53, c5 87 (166 build insns). The c1 diff shows the red add is STILL tied (`addu $a1,$t5,$a1`) and the green add likewise (`addu $v1,$t5,$v1`), so the tie survives a multi-death carrier; the extra 21 points come from the collateral re-seating of $t1/$t3/$t5 in the outer loop. Cross-channel carrier sharing is dead on this chassis for the second time (s16 measured the same family across 1,483 spellings).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s24] The sum-to-operand tie is decidable by the sum expression's operand order, by the declaration scope of the product locals, or by splitting the shift and the mask into separate statements.
- mechanism: the add's operand order determines which operand combine_regs is offered first, and moving `rp`/`gp` out of the innermost block changes their birth index in local-alloc's quantity ordering.
- probe: d1 `(r_src + rp)`, d2 `(g_src + gp)`, d3 all three sums' operands swapped (including `b * factor + px`); e1 `rp`/`gp` declared at the per-row outer scope; e2 the red channel routed through `sum` then shifted; e3 the green mask split off into its own statement.
- result: d1 12, d2 12, d3 12, e1 12 - all four print a byte-identical 12-row residual to p2 (the tree is canonicalised before local-alloc sees it, and an arm-local vs per-row declaration of a pseudo only ever live inside the arm does not move its birth index). e2 44, e3 25.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s24] Re-ordering the blend arm at statement granularity - hoisting the `*factor` products to the per-row block, moving the blue source above the green channel, swapping the blue and green channel blocks, or reversing the arm's declaration order - reaches the $v0 seats.
- mechanism: each of these changes the RTL emission order of the blend arm, which is what fixes both the local-alloc quantity ordering and global.c's allocno priority order.
- probe: b1 `sum = b * factor + px;`; b2 the two blue-source statements moved above the green channel; b3 `s32 bf = b * factor;` hoisted into the per-row block next to `complement`; b4 all three products hoisted there; b5 the whole blue channel block moved above the green one; b6 the arm's six `s32` declarations reversed.
- result: b1 12 (tie), b6 12 (tie), b2 33 (174 build insns), b3 41, b4 50 (174 build insns), b5 43 (174 build insns). Every form that actually moves a multiply out of the arm also changes the instruction count, so none is a partial win; the two ties leave the same 12 rows.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

### LIVE FRONTIER (for s25)

1. **Decide whether the sum-to-operand tie is combine_regs or merely find_free_reg
   order.** The c1 result (a carrier dying in two places did NOT untie the red add)
   argues it is NOT combine_regs, which would make s23's and s24's working model wrong.
   NEXT PROBE, before any new spelling: re-run tmp/grind/func_8003DE14/s24/alloc.sh with
   BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 on the p2 body and map the block-10 qty numbers
   (qty18/reg152, qty16/reg132, qty17/reg149, qty15/reg131) onto the red sum, the green
   sum, `rp`, `gp` and `px` by cross-reading the .lreg RTL. If a sum shares a qty with its
   operand it is combine_regs; if they merely got the same hard reg it is find_free_reg's
   scan order and the lever is the quantity ORDER (qty_compare), not the death counts.
2. **`mflo $t7` (t123).** The b*factor product reaches $v0 only via reload's
   retry_global_alloc; the target's $t7 is a register global.c never hands out in our
   build. Probe shapes that change reg 138's nrefs/livelen WITHOUT naming it as a C local
   (s23 s1-s5 killed naming) - e.g. consuming the blue sum one statement later, or giving
   the blue channel an extra ordinary use of an already-live value - and read the
   BB2_ALLOC_DEBUG ord/pri row for pseudo 138 each time.
3. **The LICM axis is un-probed on this chassis.** `r * factor`, `g * factor` and
   `b * factor` are all loop-invariant with respect to the INNER loop, yet the target
   recomputes all three inside it (t95-t115). Read tmp/grind/func_8003DE14/dumps/*.loop for
   whether GCC 2.7.2 hoists any of them out of the inner loop in our build; if it does and
   the target does not, the defeat-licm-hoist-var-reuse family (which is exactly what this
   function's ledger is cited for at .claude/rules/defeat-licm-hoist-var-reuse.md:43) may
   apply to the `*factor` products and change the whole arm's allocno set.

## [s24] The engine's honest score is computed on objdump output without -z, so runs of two or more consecutive zero words are elided on BOTH sides; the real instruction streams are 179 each and, aligned 1:1, the residual is exactly 12 rows.
- mechanism: engine/score.py:183 normalized_insns() runs `objdump -dr --start-address/--stop-address`; GNU objdump's default collapses a run of identical zero words into a single `...` line, so the three `nop; nop` pairs the target carries after each mflo (asm/funcs/func_8003DE14.s t97/98, t105/106, t113/114) never enter the list, and neither do ours. objdump -t reports 0x2cc = 716 bytes = 179 words for func_8003DE14 in BOTH build/src/code6cac_c2.o and tmp/sandbox/func_8003DE14/code6cac_c2.o.
- probe: objdump -t and objdump -dz row counts on both objects; then tmp/grind/func_8003DE14/s24/ed2.py, a -dz + branch-target-masked + spelling-canonicalised 1:1 difflib alignment, on the p2 body.
- result: 179 vs 179. 29 raw edits, of which 17 are objdump spellings (move vs addu x,y,zero; li vs addiu x,zero,N; fp vs s8; hex vs decimal) and exactly 12 are real: t117 t118 t120 t121 t123 t124 t125 t129 t130 t133 t134 t136 - the same 12 the engine scores. The nop padding already matches; func_8003DE14 does not belong in multu_pad_funcs.txt. s23's sxs.py mis-aligns by 6 rows from t97 onward and fabricates a scheduling divergence.
- verdict: CONFIRMED

## [s24] All 12 residual rows are one register-allocation pattern: the target routes every channel sum, the blue mask and the latch's first load through a fresh scratch register and puts the b*factor mflo in $t7, while our build ties each of those destinations to one of its own source operands' registers and puts the mflo in $v0.
- mechanism: Target: addu $v0,$t2,$a1 / sra $a1,$v0,15 (red); addu $v0,$t1,$v1 / sra $v1,$v0,10 (green); mflo $t7 / addu $v0,$a0,$t7 / sra $a0,$v0,5 (blue); andi $v1,$a0,0x7C00 / or $v0,$v0,$v1; lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1. Ours writes each of those into the operand's own register instead (a1, v1, a0), and swaps the two latch loads.
- probe: The corrected -dz alignment (ed2.py) on the p2 body, read row by row.
- result: Four independent seats, one shape; no row is a scheduling or instruction-count difference, and the whole residual is confined to the last 20 instructions of the blend arm plus the latch. Rows t0-t116, t119, t122, t126-t128, t131, t132, t135, t137-t140 all match.
- verdict: CONFIRMED

## [s24] s23's attribution 'global.c find_reg first-fit hands the b*factor allocno regno 2' names the wrong pass: global.c's first pass seats pseudo 138 in LO (hardreg 65) at ord=9 pri=13333, and it reaches $v0 only through reload's retry_global_alloc; separately, four block-10 LOCAL quantities already reach $v0.
- mechanism: BB2_ALLOC_DEBUG prints the full allocno order/seat/priority table; hardreg 64=HI and 65=LO on this MIPS config. The .greg dump then prints 'Register 138 now in 2', which is the tools/gcc-2.7.2/global.c:1317 retry path reached from reload1. BB2_QTY_DEBUG for block 10 prints got=2 for qty18/reg152, qty0/reg125, qty5/reg127 and qty10/reg137.
- probe: tmp/grind/func_8003DE14/s24/alloc.sh against the instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1, then with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1; full tables banked in evidence.md E-s24-3.
- result: Three of those four $v0 local quantities (srl $v0,$a0,2 / srl $v0,$a0,7 / andi $v0,$t0,0x1F) already match the target byte-for-byte. s23's frontier item 1 - 'find a C shape that makes a local quantity reach $v0' - is therefore already satisfied and is not the lever; the sums lose $v0 to a tie with their own operand, not to the b*factor allocno.
- verdict: CONFIRMED

## [s24] Both s21 F1 chain extenders are still load-bearing on the p2 px-reuse chassis: the g_src extender's price did not collapse when the blue channel lost its b_src pseudo.
- mechanism: s22's ablation numbers (d2 minus g_src extender 23, minus j extender 21, neither 28) were d2-relative; s23's frontier item 3 predicted the px reuse might make the g_src price reachable in ordinary C.
- probe: a1 (b_shift = sum >> 5), a2 (LoadImage((s32)rect, (s32)dst_buf)), a3 (both), each applied to memory/grind/func_8003DE14/candidate.c and scored with sandbox --disable all.
- result: a1 40, a2 19, a3 38 against the 12 baseline - the g_src extender is worth 28 on this chassis and the j extender 7. s23 frontier item 3 closes negative.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); p2 chassis 12/173, ablating each s21 F1 chain extender in turn

## [s24] Sharing one C carrier across channels for the complement products or for the *factor products unties the sum-to-operand register tie.
- mechanism: tools/gcc-2.7.2/local-alloc.c:1784 combine_regs returns 0 when the used (source) pseudo has reg_qty < 0, which is what a pseudo dying in more than one place gets - so a carrier used by two or three channels should force the red/green adds to take a fresh destination.
- probe: c1 rp carries the red and green complement products (gp dropped); c2 r_src carries the red and green *factor products; c3 both; c4 rp carries all three complement products; c5 rp also carries the green source byte. Scored, and the c1 body's full -dz alignment read row by row.
- result: c1 33, c2 53 (175 build insns), c3 51, c4 53, c5 87 (166 build insns). Crucially the c1 diff shows the red add STILL tied (addu $a1,$t5,$a1) and the green add likewise (addu $v1,$t5,$v1), so the tie survives the multi-death carrier - which means the combine_regs model inherited from s23 is not confirmed and may be the wrong mechanism. The extra 21 points are collateral re-seating of $t1/$t3/$t5 in the outer loop.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s24] The sum-to-operand tie is decidable by the sum expression's operand order, by the declaration scope of the product locals, or by splitting the shift and the mask into separate statements.
- mechanism: The add's operand order determines which operand local-alloc is offered first, and moving rp/gp out of the innermost block changes their birth index in the quantity ordering.
- probe: d1 (r_src + rp); d2 (g_src + gp); d3 all three sums' operands swapped including b * factor + px; e1 rp/gp declared at the per-row outer scope; e2 the red channel routed through the shared `sum` then shifted; e3 the green mask split into its own statement.
- result: d1 12, d2 12, d3 12, e1 12 - all four print a byte-identical 12-row residual to p2. e2 44, e3 25. Operand order is canonicalised before local-alloc sees it, and an arm-local vs per-row declaration of a pseudo only ever live inside the arm does not move its birth index.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s24] Re-ordering the blend arm at statement granularity - hoisting the *factor products to the per-row block, moving the blue source above the green channel, swapping the blue and green channel blocks, or reversing the arm's declaration order - reaches the $v0 seats.
- mechanism: Each of these changes the RTL emission order of the blend arm, which fixes both the local-alloc quantity ordering and global.c's allocno priority order.
- probe: b1 sum = b * factor + px; b2 the two blue-source statements moved above the green channel; b3 s32 bf = b * factor hoisted into the per-row block next to complement; b4 all three products hoisted there; b5 the whole blue channel block moved above the green one; b6 the arm's six s32 declarations reversed.
- result: b1 12 (tie, same rows), b6 12 (tie, same rows), b2 33 (174 build insns), b3 41, b4 50 (174 build insns), b5 43 (174 build insns). Every form that actually moves a multiply out of the arm also changes the instruction count, so none is a partial win.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s25] The blend arm's residual is reachable by some spelling in the carrier / naming / statement-order space of the arm itself (which value is a named local, which local carries it, which channel block comes first, where `src++` sits).
- mechanism: Every earlier session probed this space by hand, a few forms at a time (s24's b/c/d/e series is 20 forms). The enumerate modality searches it exhaustively: a generator (tmp/grind/func_8003DE14/s25/gen.py) emits a complete function body for every point of a 9-axis product space, each variant semantics-preserving by construction (each channel chain keeps its internal order; the green `*factor` carrier always precedes the blue F1 extender that reads it). Axes: (1) `r * factor` reuses `r_src` or takes a fresh `rf`; (2) the red sum is inlined into its shift / named fresh / carried by the shared `sum`; (3)+(4) the same two axes for green; (5) the blue source reuses `px` or takes a fresh `b_src`; (6) the blue complement product reuses its own source carrier or takes a fresh `bp`; (7) the blue sum inlined / shared / fresh; (8) channel block order RGB / GRB / GBR; (9) `src++` at the top of the arm, at the bottom, or after the second block's source read.
- probe: 3,888 distinct bodies generated; 1,496 scored with `tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2` (the full 1,296-point product of axes 1-8 at `src++`=aftersrc, plus 300 bodies covering all three `src++` positions for 100 axis points). Scores in tmp/grind/func_8003DE14/s25/scores_phase1.txt + scores.txt, axis roll-up by tmp/grind/func_8003DE14/s25/analyze.py.
- result: MINIMUM 12, reached by exactly 8 of the 1,496 bodies; 32 distinct scores, worst 60; every 12 keeps the same 12 residual rows. Per-axis minima: rf_reuse True 12 / False 28; gf_reuse True 12 / False 31; channel order RGB 12 / GRB 28 / GBR 30; blue source px-reuse 12 / fresh local 14; green sum shared 12 / inline 15 / fresh 15; blue sum shared 12 / inline 15 / fresh 15. TWO axes turn out to be FREE at the floor (new): the blue complement product may take a fresh `bp` local instead of reusing `px` (12), and the red sum may be a FRESH named local instead of being inlined into its shift (v1415, 12, banked as rejected/s25-enum-red-sum-fresh-local-ties-12.c) - so the incumbent p2 spelling is one of an 8-member plateau, not a unique optimum. `src++`'s position is byte-neutral in all 100 triples measured (all three positions score identically every time).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present, all 1,496 bodies 173 build insns at the floor

## [s25] Three of the twelve residual rows (the latch's `lh $v0,4($s0)` / `lh $v1,6($s0)` / `mult $v0,$v1`) are reachable from the spelling of the loop bound - the operand order of `rect[2] * rect[3]`, whether the bound re-reads `rect` or uses the named `total`, whether the guard tests `total` or the expression, or whether the two halves are named.
- mechanism: our build emits the two `lh`s into the opposite registers from the target's, and the latch's multiply is the only residual row outside the blend arm; the loop bound is re-read from memory at the bottom of the inner loop, so its C spelling is what feeds cse2/local-alloc there.
- probe: 20 bodies on the v0119 (= p2) chassis crossing {`total = rect[2]*rect[3]`, `total = rect[3]*rect[2]`, `w = rect[2]; h = rect[3]; total = w*h`} x {`if (total > 0)`, `if (rect[2]*rect[3] > 0)`} x {`while (j < rect[2]*rect[3])`, `... rect[3]*rect[2]`, `... total`, `... w*h`}; scored in one sweep (tmp/grind/func_8003DE14/s25/latch/raw.json, index.txt).
- result: minimum 12 (8 bodies), and the two `lh` rows are present in every one of them. The while-condition's own operand order is byte-neutral (c_ab and c_ba tie at 12 for every `total` spelling) - GCC canonicalises it before cse2. Using the named `total` as the bound removes the loop's re-read entirely (32-34 at 169-170 build insns: three instructions SHORT of the target, so the target genuinely re-reads `rect[2]` and `rect[3]` at the bottom of the loop). Swapping the operands of the EARLIER `total` computation costs +2 (L06 14), and naming `w`/`h` is neutral (L12 12). The latch rows are not spelled from the bound.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173 (v0119 body), both s21 F1 chain extenders present

## [s25] The LICM axis (s24 frontier item 3): GCC 2.7.2 hoists one or more of `r * factor`, `g * factor`, `b * factor` out of the INNER loop in our build while the target recomputes all three inside it, which would put the whole arm's allocno set out of reach of any register-seat lever.
- mechanism: .claude/rules/defeat-licm-hoist-var-reuse.md (whose line 43 cites this very function) names the ordinary-C counter-shape for a hoist the target does not perform; loop.c's move_movables decides it.
- probe: `pwsh tools/grinder/dump.ps1 func_8003DE14` with the v0119 body applied to src/code6cac_c2.c, then read the func_8003DE14 section of tmp/grind/func_8003DE14/dumps/code6cac_c2.loop (lines 11426-12360).
- result: the inner loop is "Loop from 140 to 317: 62 real insns" and it contains NO `moved to` line at all. The three multiply candidates are named and REJECTED by loop.c's own savings test: `Insn 145: regno 117 (life 1), savings 1 not desirable`, `Insn 174: regno 120 ... not desirable`, `Insn 254: regno 138 ... not desirable` (the print is tools/gcc-2.7.2/loop.c:1977). Our build recomputes all three products inside the inner loop exactly as the target does (asm/funcs/func_8003DE14.s t95-t115). There is no LICM divergence in this function, so the defeat-licm family has nothing to bite on here and s24's frontier item 3 closes negative.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-11; v0119 (= p2) body applied to src, canonical cc1 -da dump
- predicate_cite: tools/gcc-2.7.2/loop.c:1977

### LIVE FRONTIER (for s26)

1. **The residual is not in the blend arm's SPELLING - it is in the arm's block
   STRUCTURE or in the surrounding chassis.** 1,496 exhaustive spellings of the
   arm and 20 of the latch all bottom at 12 with the same 12 rows. What the sweep
   did NOT vary: the arm's block structure (the `if (i == count - 1)` split, the
   `px == 0` early-out, the `goto loop_check` exits), the declaration SCOPE of the
   arm's locals relative to the two enclosing blocks, and the outer per-row
   block's contents. That is the next enumeration axis, and it is cheap: move the
   head/tail split in tmp/grind/func_8003DE14/s25/gen.py outward one block and
   re-generate.
2. **Two axes are FREE at the floor - a lever the next session gets for free.**
   The blue complement product may live in a fresh `bp` local and the red sum may
   be a fresh named local, both at 12 with 173 build insns. A free axis is a place
   where an extra pseudo can be introduced WITHOUT paying, i.e. the cheapest place
   to change reg 138's nrefs/livelen (s24 frontier item 2) without the collateral
   re-seating that killed every earlier attempt.
3. **Frontier item 1 of s24 is still un-run** (is the sum-to-operand tie
   combine_regs or find_free_reg scan order?): re-run
   tmp/grind/func_8003DE14/s24/alloc.sh with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 on
   the p2 body and map block-10 qty18/reg152, qty16/reg132, qty17/reg149,
   qty15/reg131 onto the red sum, the green sum, `rp`, `gp` and `px`. Same qty =>
   combine_regs; different qty, same hard reg => find_free_reg scan order, and the
   lever is the quantity ORDER (qty_compare), not any death count.

## [s25] The blend arm's residual is reachable by some spelling in the carrier / naming / statement-order space of the arm itself (which value is a named local, which local carries it, which channel block comes first, where src++ sits).
- mechanism: Earlier sessions probed this space by hand a few forms at a time (s24's b/c/d/e series is 20 forms). The enumerate modality searches it exhaustively with a generator (tmp/grind/func_8003DE14/s25/gen.py) that emits a complete function body for every point of a 9-axis product space, each variant semantics-preserving by construction: each channel chain keeps its internal order and the green *factor carrier always precedes the blue F1 extender that reads it. Axes: r*factor carrier (reuse r_src / fresh rf), red sum (inline / fresh / shared `sum`), the same two for green, blue source carrier (reuse px / fresh b_src), blue product carrier (same / fresh bp), blue sum (inline / fresh / shared), channel block order (RGB / GRB / GBR), src++ position (top / bottom / after the second block's source read).
- probe: 3,888 distinct bodies generated; 1,496 scored with tools/sweep_variants.py --func func_8003DE14 --file code6cac_c2 in 100-variant chunks (the full 1,296-point product of axes 1-8 at src++=aftersrc, plus 300 bodies covering all three src++ positions for 100 axis points). Scores in tmp/grind/func_8003DE14/s25/scores_phase1.txt and scores.txt; axis roll-up by analyze.py.
- result: MINIMUM 12, reached by exactly 8 of the 1,496 bodies; 32 distinct scores, worst 60; every floor body builds 173 insns and prints the SAME 12 residual rows. Per-axis minima: r*factor must reuse r_src (fresh 28); g*factor must reuse g_src (fresh 31); order must be RGB (GRB 28, GBR 30); the blue source must reuse px (fresh 14); the green and blue sums must share one `sum` local (inline 15, fresh 15). NEW: two axes are FREE at the floor - the blue complement product may take a fresh `bp` local (12) and the red sum may be a fresh named local instead of being inlined into its shift (v1415, 12) - so the incumbent p2 spelling is one of an 8-member plateau, not a unique optimum. src++'s position is byte-neutral in all 100 triples measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present; all floor bodies 173 build insns

## [s25] Three of the twelve residual rows (the latch's lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1) are reachable from the spelling of the loop bound - the operand order of rect[2] * rect[3], whether the bound re-reads rect or uses the named total, whether the guard tests total or the expression, or whether the two halves are named.
- mechanism: Our build emits the two lh's into the opposite registers from the target's, and the latch's multiply is the only residual row outside the blend arm; the loop bound is re-read from memory at the bottom of the inner loop, so its C spelling is what feeds cse2/local-alloc there.
- probe: 20 bodies on the v0119 (= p2) chassis crossing three `total` spellings x two guard spellings x four while-condition spellings, scored in one sweep (tmp/grind/func_8003DE14/s25/latch/raw.json, index.txt).
- result: Minimum 12 (8 bodies), with the two lh rows present in every one. The while-condition's own operand order is canonicalised away (c_ab ties c_ba at every point). Using the named `total` as the bound removes the re-read entirely: 169-170 build insns, score 32-34 - three instructions SHORT of the target, so the target genuinely re-reads rect[2] and rect[3] at the latch. Swapping the operands of the EARLIER `total = rect[2]*rect[3]` costs +2 (14); naming w/h for the two halves is neutral (12).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173 (v0119 body), both s21 F1 chain extenders present

## [s25] s24 frontier item 3: GCC 2.7.2 hoists one or more of r*factor, g*factor, b*factor out of the INNER loop in our build while the target recomputes all three inside it, which would put the arm's allocno set out of reach of any register-seat lever.
- mechanism: loop.c's move_movables decides the hoist; .claude/rules/defeat-licm-hoist-var-reuse.md (line 43 cites this very function) names the ordinary-C counter-shape for a hoist the target does not perform.
- probe: pwsh tools/grinder/dump.ps1 func_8003DE14 with the v0119 body applied to src/code6cac_c2.c, then the func_8003DE14 section of tmp/grind/func_8003DE14/dumps/code6cac_c2.loop (lines 11426-12360).
- result: The inner loop is 'Loop from 140 to 317: 62 real insns' and contains NO 'moved to' line. loop.c names and rejects exactly the three multiply candidates by its own savings test: 'Insn 145: regno 117 (life 1), savings 1 not desirable', 'Insn 174: regno 120 ... not desirable', 'Insn 254: regno 138 ... not desirable'. Our build recomputes all three products inside the inner loop exactly as the target does (asm/funcs/func_8003DE14.s t95-t115), so there is no LICM divergence for the defeat-licm family to act on.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-11; v0119 (= p2) body applied to src, canonical cc1 -da dump
- predicate_cite: tools/gcc-2.7.2/loop.c:1977

## [s26] KILL RE-AUDIT: the two s21 F1 chain extenders are still load-bearing on the p2 chassis as measured TODAY, and their prices are not the ones s22/s24 recorded.
- mechanism: both extenders are F1-family combine-foldable chain extenders that lift `reg_n_refs` on `g_src` and on `j`; s24 measured them on the p2 chassis and recorded 23/21/28, but the chassis has been re-dumped since and an extender's price is a function of the whole arm's allocation, so it must be re-measured before any conclusion rests on it.
- probe: four complete bodies (both extenders / j removed / g_src removed / neither) in tmp/grind/func_8003DE14/s26/ab/, scored in one `tools/sweep_variants.py` call on HEAD 2026-09-11.
- result: 12 (both) / 19 (no j) / 40 (no g_src) / 38 (neither); all four build 173 insns. Both extenders are confirmed load-bearing and the ordinary-C floor of this chassis is 38. NEW and important: the two interact non-additively and with opposite sign - removing only the g_src extender (40) is worse than removing both (38), so the j extender is worth -7 with the g_src extender present and +2 without it. Single-lever ablation deltas on this body are meaningless; use the 2x2.
- verdict: CONFIRMED

## [s26] The sum-to-operand register tie in the blend arm is local-alloc's combine_regs merging several pseudos into ONE quantity, not find_free_reg handing two separate quantities the same hard register by scan order (s24 frontier item 1).
- mechanism: block_alloc (tools/gcc-2.7.2/local-alloc.c:1295) scans an insn's operands and calls combine_regs(operand, dest) for the first one that succeeds, breaking on the first win; combine_regs (:1784) merges the two pseudos' quantities so they are guaranteed the same hard register. The alternative explanation was that each value keeps its own quantity and merely gets the same register from find_free_reg's first fit.
- probe: tmp/grind/func_8003DE14/s26/alloc.sh (the s24 allocator script with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 BB2_ALLOC_DEBUG=1) on the p2 body -> tmp/grind/func_8003DE14/s26/alloc.txt; cross-read against a freshly regenerated tmp/grind/func_8003DE14/dumps/code6cac_c2.lreg for the SAME body.
- result: block 10 has 21 pseudos of which 16 are local-alloc-eligible, but block_alloc ends with only 7 real quantities (12 of the 19 printed are reg1=-1 HI/LO ranges). The merge is explicit in the counts: qty18 prints reg1=152 refs=24 while "Register 152 used 6 times" - one quantity carrying four 6-ref pseudos; qty16 refs=24, qty17 refs=18, qty15 refs=18 are the same shape. Same quantity, therefore combine_regs. s24's frontier item 1 is closed.
- verdict: CONFIRMED

## [s26] None of the blend arm's six products is a local-alloc quantity: the three complement products are excluded by CLASS_LIKELY_SPILLED_P(LO_REG) and the three *factor products by reg_n_deaths != 1, so every local quantity in block 10 is a DERIVED value (sum / shift / mask / or-chain) and the wrong seats are multi-pseudo combine chains, not mispriced allocnos.
- mechanism: tools/gcc-2.7.2/local-alloc.c:470-476 sets reg_qty[i] = -2 (eligible) only if reg_basic_block[i] >= 0 AND reg_n_deaths[i] == 1 AND (reg_alternate_class == NO_REGS OR !CLASS_LIKELY_SPILLED_P(reg_preferred_class)); otherwise -1. combine_regs:1825 additionally rejects any usedreg whose reg_qty is < 0.
- probe: the .lreg header lines for this exact body, cross-read with the QTYDBG table.
- result: registers 129/130/138 print "pref LO_REG, else GR_REGS" (the three complement products) and registers 123/126 print "dies in 2 places" (the reused C variables carrying the *factor products); px is not block-local at all. All six fail the gate. The seven real quantities and their priorities (pri = floor_log2(refs)*refs*size/(death-birth)*10000) are ord0 qty18 refs24 life10 pri96000 got $v0; ord1 qty16 refs24 life14 pri68571 got $v1; ord2/3/4 qty0/5/10 refs6 life2 pri60000 all got $v0; ord5 qty17 refs18 life12 pri60000 got $a0; ord6 qty15 refs18 life18 pri40000 got $a1. Short single-pseudo quantities ALREADY reach $v0; the three wrong seats are precisely the three MULTI-pseudo merged quantities.
- verdict: CONFIRMED

## [s26] The output-word or-chain's shape - associativity, operand order, a named blue mask or high bit, an accumulator local, an accumulator reusing sum or px, or folding the 0x7C00 mask into the shift statement - reaches the $v0 seats on the p2 chassis.
- mechanism: qty18 (the tail chain, pri 96000) is the quantity that takes $v0 and holds it for the block's last five insns; the or-chain is what that quantity is made of, so its C shape is the direct handle on qty18's refs and life, which is what qty_compare_1 sorts on. Earlier sessions' w1/y3/z5 or-chain rejects were measured on long-superseded chassis and were therefore void.
- probe: 11 complete bodies in tmp/grind/func_8003DE14/s26/orv/, scored in one tools/sweep_variants.py call.
- result: minimum 12, reached only by the incumbent and by o9 (a named high bit, s32 hi = pixel & 0x8000, a tie at 12/173). Everything else is strictly worse, with a very wide spread: o1 named blue mask 13, o11 mask folded into the shift 13, o3 reversed operand order 25, o8 high bit last 31, o4 accumulator local 36, o6 sub-parenthesised (r_ch | g_ch) 36, o5 accumulator reusing sum 40, o7 blue mask into b_shift 41 (174 insns), o2 pairwise association 42, o10 accumulator reusing px 45 (174 insns). The incumbent's flat left-association is already optimal and the or-chain axis is exhausted on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); p2 chassis 12/173, both s21 F1 chain extenders present

## [s26] Giving a channel RESULT (r_ch, g_ch, b_shift) a reused carrier instead of a fresh local breaks the sum-shift-mask combine chain, because a carrier that dies twice fails the local-alloc.c:470-476 eligibility gate and cannot be a combine_regs usedreg.
- mechanism: this is the direct C-level attempt at the reg_n_deaths != 1 route identified this session. s25's 1,496-body sweep enumerated carriers for the channel SOURCES, the complement PRODUCTS and the SUMS but always gave the three channel results fresh locals, so this axis was untouched.
- probe: 8 complete bodies in tmp/grind/func_8003DE14/s26/cv/ (r_ch reusing r_src or rp; g_ch reusing gp or g_src with the g_src extender then collapsed; b_shift reusing px, gp or rp; and r_ch+b_shift together), scored in one tools/sweep_variants.py call.
- result: minimum 12 = the incumbent; no variant improves. r_ch reusing r_src costs 4 (16); g_ch reusing g_src (which forces the g_src extender to collapse) 27; r_ch+b_shift together 29 at 174 insns; r_ch reusing rp 32; b_shift reusing gp 33; b_shift reusing px 35 at 174; b_shift reusing rp 36 at 174; g_ch reusing gp 37. Making a channel result die twice does break chains, but it also re-seats the arm's globals and pays more than it earns on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

### LIVE FRONTIER (for s27)

1. **Break the sum -> shift -> mask combine CHAINS via block structure, not via
   spelling.** The three wrong seats are qty16/qty17/qty15, each a 3-4 pseudo
   merged quantity; in the target each of those chains is split (the sum is born
   in $v0 and dies one insn later). Per local-alloc.c:470-476 a C form breaks the
   chain only if the SUM pseudo fails the eligibility gate where block_alloc
   reaches the shift - i.e. the sum is live across a basic-block boundary
   (reg_basic_block < 0) or dies more than once. Spelling cannot produce either
   (s25: 1,496 bodies; s26: 11 or-chain + 8 channel-result bodies). Block
   structure can. Next probe: extend tmp/grind/func_8003DE14/s25/gen.py's
   head/tail split outward one block so the generator can vary the
   `if (i == count - 1)` split (early-out vs if/else vs hoisting the test out of
   the inner loop), the `px == 0` early-out shape (goto vs if/else vs continue),
   the `goto loop_check` exits, and the declaration scope of the arm's locals
   relative to the two enclosing blocks; ~1.8 s per body, so a 1,000-body space
   is ~30 minutes. Accept only forms at 173 build insns whose QTYDBG block-10
   table prints MORE than 7 real quantities - that is the mechanical signature of
   a broken chain, and it is checkable without reading the residual.

2. **The 2x2 extender interaction is a lever in its own right.** a1 (g_src
   extender removed, j extender kept) scores 40 while a3 (neither) scores 38: the
   j extender is worth -7 in one context and +2 in the other. The two extenders
   are competing for the same seats, which makes the PAIR - not either one alone
   - the thing to re-derive. Since the final body must justify every F1 construct
   under lever-exhaustion, a form that needs only ONE extender is worth strictly
   more than the incumbent even at an equal score. Next probe: on the a2 chassis
   (j extender removed, 19) re-run the s25 arm sweep's eight floor bodies - the
   arm's optimum was derived with BOTH extenders present and may not be the
   optimum with one.

3. **qty18 is the quantity that owns $v0 (pri 96000, life 10, refs 24) and the
   or-chain axis cannot move it.** What has never been tried is lengthening its
   LIFE without adding refs, which is the only other term in qty_compare_1. A
   form that separates the or-chain's first and last insn by one unrelated
   statement drops pri below qty16's 68571 and hands $v0 to a different quantity.
   Next probe: with the o9 tie body (named high bit hoisted to the top of the
   arm, 12/173) as the chassis, walk the declaration of `hi` and the `*dst =`
   store apart one statement at a time and read the QTYDBG pri column for qty18
   on each - an ordinary-C statement-order move, and o9's existence proves the
   hoist itself is byte-neutral.

## [s26] The two s21 F1 chain extenders are still load-bearing on the p2 chassis as measured today, and their individual prices differ from the ones s22/s24 recorded: the pair interacts non-additively and with opposite sign.
- mechanism: Both are F1-family combine-foldable chain extenders lifting reg_n_refs on g_src and on j. An extender's price is a function of the whole arm's allocation, so a price recorded on an earlier chassis is not transferable; the mandated kill re-audit re-measures the full 2x2 rather than a single-lever delta.
- probe: Four complete bodies (both extenders / j removed / g_src removed / neither) generated into tmp/grind/func_8003DE14/s26/ab/ and scored in one tools/sweep_variants.py call on HEAD 2026-09-11.
- result: 12 (both) / 19 (no j) / 40 (no g_src) / 38 (neither), all four at 173 build insns. Both extenders remain load-bearing. The honest ordinary-C floor of this chassis is 38, not the 28 the ledger carried. Removing ONLY the g_src extender (40) is worse than removing both (38): the j extender is worth -7 with the g_src extender present and +2 without it, so the two compete for the same seats and single-lever ablation deltas on this body are meaningless.
- verdict: CONFIRMED

## [s26] The sum-to-operand register tie in the blend arm is local-alloc's combine_regs merging several pseudos into one quantity, not find_free_reg handing two separate quantities the same hard register by scan order.
- mechanism: block_alloc (tools/gcc-2.7.2/local-alloc.c:1295) scans an insn's operands and calls combine_regs(operand, dest) for the first one that succeeds, breaking on the first win; combine_regs (:1784) merges the two pseudos' quantities so they are guaranteed the same hard register. The competing explanation (s24 frontier item 1) was that each value keeps its own quantity and merely gets the same register by first fit.
- probe: tmp/grind/func_8003DE14/s26/alloc.sh - the s24 allocator script re-run with BB2_QTY_DEBUG=1 BB2_SUGG_DEBUG=1 BB2_ALLOC_DEBUG=1 on the p2 body (1,773 lines to tmp/grind/func_8003DE14/s26/alloc.txt) - cross-read against a freshly regenerated tmp/grind/func_8003DE14/dumps/code6cac_c2.lreg for the SAME body.
- result: Block 10 holds 21 pseudos, 16 of them local-alloc-eligible, yet block_alloc ends with only 7 real quantities (the other 12 of the 19 printed rows are reg1=-1 HI/LO hard-reg ranges). The merge is explicit in the reference counts: qty18 prints reg1=152 refs=24 while the .lreg line for register 152 reads 'used 6 times' - one quantity carrying four 6-ref pseudos. qty16 refs=24, qty17 refs=18 and qty15 refs=18 have the same shape. Same quantity, therefore combine_regs; s24's frontier item 1 is closed.
- verdict: CONFIRMED

## [s26] None of the blend arm's six products is a local-alloc quantity: the three complement products are excluded by CLASS_LIKELY_SPILLED_P(LO_REG) and the three *factor products by reg_n_deaths != 1, so every local quantity in block 10 is a derived value and the three wrong seats are exactly the three multi-pseudo merged quantities.
- mechanism: tools/gcc-2.7.2/local-alloc.c:470-476 sets reg_qty[i] = -2 (eligible) only when reg_basic_block[i] >= 0 AND reg_n_deaths[i] == 1 AND (reg_alternate_class == NO_REGS OR !CLASS_LIKELY_SPILLED_P(reg_preferred_class)); otherwise -1. combine_regs:1825 additionally rejects any usedreg whose reg_qty is below zero.
- probe: The .lreg per-register header lines for this exact body, cross-read with the QTYDBG block-10 quantity table and qty_compare_1's priority formula (local-alloc.c:1660).
- result: Registers 129/130/138 print 'pref LO_REG, else GR_REGS' (the three complement products) and registers 123/126 print 'dies in 2 places' (the reused C variables carrying the *factor products); px is not block-local at all. All six fail the gate. The seven real quantities, in allocation order: qty18 refs24 life10 pri96000 got $v0; qty16 refs24 life14 pri68571 got $v1; qty0/qty5/qty10 refs6 life2 pri60000 all got $v0; qty17 refs18 life12 pri60000 got $a0; qty15 refs18 life18 pri40000 got $a1. Short single-pseudo quantities already reach $v0 - so the residual is not a priority problem - and the three wrong seats are precisely the three multi-pseudo merged quantities.
- verdict: CONFIRMED

## [s26] The output-word or-chain's shape - associativity, operand order, a named blue mask or high bit, an accumulator local, an accumulator reusing sum or px, or folding the 0x7C00 mask into the shift statement - lowers the score below 12 on the p2 chassis.
- mechanism: qty18 (the tail chain, pri 96000) is the quantity that takes $v0 and holds it across the block's last five insns, and the or-chain is what that quantity is made of, so its C shape is the direct handle on the refs and life terms qty_compare_1 sorts on. Earlier sessions' w1/y3/z5 or-chain rejects were measured on long-superseded chassis and were void.
- probe: 11 complete bodies in tmp/grind/func_8003DE14/s26/orv/, scored in one tools/sweep_variants.py call against the 12/173 baseline.
- result: Minimum 12, reached only by the incumbent and by o9 (a named high bit, s32 hi = pixel & 0x8000, tying at 12/173). Everything else is strictly worse over a wide spread: named blue mask 13, mask folded into the shift 13, reversed operand order 25, high bit last 31, accumulator local 36, sub-parenthesised (r_ch | g_ch) 36, accumulator reusing sum 40, blue mask into b_shift 41 at 174 insns, pairwise association 42, accumulator reusing px 45 at 174 insns. The incumbent's flat left-association is already optimal on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); p2 chassis 12/173, both s21 F1 chain extenders present

## [s26] Giving a channel result (r_ch, g_ch, b_shift) a reused carrier instead of a fresh local breaks the sum-shift-mask combine chain and lowers the score, because a carrier that dies twice fails the local-alloc.c:470-476 eligibility gate and cannot be a combine_regs usedreg.
- mechanism: This is the direct C-level attempt at the reg_n_deaths != 1 route identified this session. s25's 1,496-body sweep enumerated carriers for the channel sources, the complement products and the sums, but always gave the three channel results fresh locals, so this axis was untouched.
- probe: 8 complete bodies in tmp/grind/func_8003DE14/s26/cv/ - r_ch reusing r_src or rp; g_ch reusing gp or g_src (with the g_src extender then collapsed); b_shift reusing px, gp or rp; and r_ch plus b_shift together - scored in one tools/sweep_variants.py call.
- result: Minimum 12 = the incumbent; nothing improves. r_ch reusing r_src costs 4 (16); g_ch reusing g_src 27; r_ch and b_shift together 29 at 174 insns; r_ch reusing rp 32; b_shift reusing gp 33; b_shift reusing px 35 at 174; b_shift reusing rp 36 at 174; g_ch reusing gp 37. Making a channel result die twice does break chains, but it re-seats the arm's globals and pays more than it earns here.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; p2 chassis 12/173, both s21 F1 chain extenders present

## [s27] The 12 residual rows are one register-allocation fact - the target's $v0 is owned across the blend arm by a pseudo our build does not have - and the `mflo $t7` is a consequence of that ownership rather than an independent seat.
- mechanism: global.c seats pseudo 138 (`b * factor`) in LO (its preferred class is LO_REG); insn 260 needs a GR operand, so reload spills 65 and `retry_global_alloc` re-seats 138 by the Phase-6 closed form `got = min{r not in base|forbidden|~class|conflicts}`. reload_sim.py prints the scan leaving {2, 15, 24, 25} free, so ours takes $v0 and the target's $t7 is simply the next free register once $v0 is taken.
- probe: goal_from_tgt.py classify/goal (object mode) for the stage verdict and the 12 renames; extract.py + simulate.py (0 diffs vs ALLOCDBG) for the forward model; inverse.py global with the FULL 24-allocno goal at depth 2; reload_harvest.sh + reload_extract.py + reload_sim.py --show for the retry; hand counterfactuals in tmp/grind/func_8003DE14/s27/cf.py.
- result: stage = RA (no multiset or ordering difference). inverse.py returns a validated NEGATIVE for 138->15 at depth 2, and names reload spill-retry as the owner. Every counterfactual that denies 138 the LO seat (no LO preference / conflict with 159 / longer live length) gives it $v0 anyway and rotates 129/130/110/115/116 down a seat. So the seat cannot be moved by pricing 138; it moves only if $v0 is already occupied.
- verdict: CONFIRMED

## [s27] Carrying ALL THREE channel sums in ONE local makes that pseudo a three-death (local-alloc-ineligible) global allocno, which takes $v0 across the arm and thereby forces the `b * factor` reload retry into $t7 - both of the target's disputed seats at once.
- mechanism: local-alloc.c:470-476 sets reg_qty = -1 when reg_n_deaths != 1, so a variable written and read three times in the block is never a local quantity and never a combine_regs usedreg; it becomes a global allocno holding one hard register over the whole arm. With $v0 held, reload's ascending retry scan (which had {2,15,24,25} free) returns 15 = $t7.
- probe: tmp/grind/func_8003DE14/s27/v/s3_ext_none.c (banked as memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c) applied to src, then extract.py + the .lreg register header lines + the model's dispositions.
- result: `.lreg` prints "Register 128 used 18 times across 8 insns in block 10; dies in 3 places"; dispositions give 128 -> hardreg 2 ($v0) at ord 1 (nrefs 18, livelen 8, pri 90000) and 137 (the b*factor mflo) -> hardreg 15 ($t7) through the same reload retry. The three shift results split into their own pseudos, matching the target's `sra $a1,$v0,15 / sra $v1,$v0,10 / sra $a0,$v0,5` shape. Both target seats are produced by ordinary C.
- verdict: CONFIRMED

## [s27] The g_src reference-lift that the incumbent gets from `((sum + g_src) - g_src) >> 5` can be relocated onto another value so that it survives on the three-way-shared-sum chassis.
- mechanism: the blue-path extender is what splits the blue sum into its own pseudo (140), which is exactly what prevents the shared `sum` from reaching three deaths; so the three-way share and that extender are mutually exclusive by construction. The extender's only job is lifting pseudo 126's reg_n_refs from 12 to 18 (priority 32727 -> 60000, ord 4 -> ord 1), so any other placement that flow.c still counts would do.
- probe: seven complete bodies in tmp/grind/func_8003DE14/s27/v2/ placing `(X + g_src) - g_src` on b_shift, on g_ch's mask, on either operand inside the sum expression, on the or-chain's g_ch term, on the whole or-chain word, and on b_shift inside the or-chain; scored in one tools/sweep_variants.py call.
- result: p1/p3/p8/p9/p10 all score exactly 42 - the same as removing the extender entirely - and p4 45, p11 47. Every placement folds before flow.c counts references, so none restores 126 to 18 refs. The three-way chassis therefore sits at 42, with its seat bank rotated one position from the target (122 $a0->$a2, 126 $v1->$a3, 123 $a1->$v1, 109 $a2->$t0, 108 $a3->$t1, 121 $t0->$t2, 130 $t1->$a0, 129 $t2->$a1).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; three-way-shared-sum chassis (tmp/grind/func_8003DE14/s27/v/s3_ext_none.c), j extender present, g_src blue-path extender removed by construction

## [s27] KILL RE-AUDIT: the s26 2x2 extender prices still hold on today's chassis.
- mechanism: mandated re-audit of the closest banked instance kill before new probing; an extender's price is a function of the whole arm's allocation and the chassis has been re-dumped since s26.
- probe: s2_ext_blue (= the incumbent p2 body) and s2_ext_none (g_src extender removed, j extender kept) in the same tools/sweep_variants.py call as the s27 sweep.
- result: 12 and 40 respectively, both at 173 build insns - identical to s26's 2x2 entries. The banked prices are current, not stale.
- verdict: CONFIRMED

### LIVE FRONTIER (for s28)

1. **Lift pseudo 126 (`g_src`) back to 18 refs on the three-way-shared-sum
   chassis WITHOUT re-splitting the blue sum.** That chassis already produces
   BOTH target seats (128 -> $v0, 137 -> $t7); the entire 42-point gap is the
   one-position seat rotation caused by 126 falling from ord 1 to ord 4. Seven
   `(X + g_src) - g_src` placements are dead (all fold pre-flow). Untried and
   ordinary: give the green channel a genuinely extra READ of `g_src` that
   survives to flow.c - e.g. compute the green source from `g_src` twice
   (duplicate-read into branch arms, .claude/rules/split-read-defeats-hoist.md),
   or re-order so `g_src` is also the carrier of a second real value in the arm
   (variable reuse), or lift 126's priority the other way: SHORTEN its live
   length (pri = floor_log2(refs)*refs/livelen*10000, so livelen 11 -> 6 gives
   60000 without touching refs). The live-length route has never been tried and
   needs no extra construct at all.

2. **Ask the solver the inverse question on the s27 model instead of guessing.**
   Re-run `python3 tools/ra_solver/extract.py func_8003DE14 code6cac_c2` with
   `chassis_s27_threeway_sum_42.c` applied, then
   `inverse.py global <model> --goal` with the FULL target disposition
   {118:2, 128:2, 126:3, 122:4, 123:5, 109:6, 108:7, 121:8, 130:9, 129:10,
   110:11, 115:12, 116:13, 137:15, 72:16, 74:17, 73:18, 78:19, 77:20, 76:21,
   79:22, 75:23, 100:30, 156:65} at depth 2-3. On the p2 chassis that goal was
   unreachable because it needed reload to move 138; on THIS chassis the reload
   outcome is already correct, so the question reduces to pure allocno pricing -
   exactly what the model is validated for. A vector there is a directly
   spellable C lever.

3. **The blue mask and the latch load pair may follow for free.** Rows
   t123/t129/t133/t134 (149 -> $v1, 154/157 swapped) were never separately
   attacked; they are downstream of the same $v0 ownership. Re-read the residual
   with tmp/grind/func_8003DE14/s24/ed2.py on the s27 chassis before treating
   them as independent - if they already match there, the remaining work is
   only frontier 1.

## [s27] The 12 residual rows are one register-allocation fact - in the target a pseudo owns $v0 across the blend arm, and the `mflo $t7` is what reload's ascending retry scan returns once $v0 is occupied, not an independently steerable seat.
- mechanism: global.c seats pseudo 138 (`b * factor`) in LO because reg_preferred_class is LO_REG; insn 260 needs a GR operand, so reload spills 65 and retry_global_alloc re-seats it by the Phase-6 closed form got = min{r not in base|forbidden|~class|conflicts}. reload_sim.py prints that scan with exactly {2,15,24,25} free, so ours takes $v0 (2) and the target's $t7 (15) is simply the next free seat.
- probe: goal_from_tgt.py classify/goal in object mode (build/src/code6cac_c2.o vs tmp/sandbox/func_8003DE14/code6cac_c2.o); extract.py + simulate.py forward check; inverse.py global with the FULL 24-allocno goal at depth 2; reload_harvest.sh + reload_extract.py + reload_sim.py --show; hand counterfactuals in tmp/grind/func_8003DE14/s27/cf.py.
- result: Stage verdict RA (173/173, 12 renames, no multiset or ordering difference). simulate.py reproduces the p2 allocation with ZERO diffs across 24 allocnos. inverse.py returns a validated NEGATIVE for 138->15 at depth 2 and names reload spill-retry as the owner. Counterfactuals removing 138's LO preference, adding a 138<->159 conflict, or stretching its live length all give 138 $v0 anyway and rotate 129/130/110/115/116 down one seat each.
- verdict: CONFIRMED

## [s27] Carrying all three channel sums in ONE local makes that pseudo a three-death, local-alloc-ineligible global allocno which takes $v0 across the arm and thereby forces the b*factor reload retry into $t7, producing both of the target's disputed seats with ordinary C.
- mechanism: local-alloc.c:470-476 sets reg_qty = -1 when reg_n_deaths != 1, so a local written and read three times in the block is never a local quantity and never a combine_regs usedreg; it becomes a global allocno holding one hard register over the whole arm. With $v0 held, reload's ascending retry scan returns 15 = $t7.
- probe: tmp/grind/func_8003DE14/s27/v/s3_ext_none.c (banked as memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c) applied to src, then tools/ra_solver/extract.py + the .lreg register header lines + the model dispositions, and tmp/grind/func_8003DE14/s24/ed2.py on the sandbox object.
- result: The .lreg prints 'Register 128 used 18 times across 8 insns in block 10; dies in 3 places'; dispositions give 128 -> hardreg 2 ($v0) at ord 1 (nrefs 18, livelen 8, pri 90000) and 137 (the b*factor mflo) -> hardreg 15 ($t7) through the same reload retry. The residual diff on that body shows t117/t118/t123/t124 (addu $v0 / sra $a1 / mflo $t7) are target-exact, and the three shift results split into their own pseudos exactly as the target has them.
- verdict: CONFIRMED

## [s27] On the three-way-shared-sum chassis the g_src reference lift can be relocated onto another value - b_shift, g_ch's mask, either operand inside the sum expression, the or-chain's g_ch term, the whole or-chain word, or b_shift inside the or-chain - so that flow.c still counts it and pseudo 126 keeps 18 references.
- mechanism: The blue-path extender `((sum + g_src) - g_src) >> 5` is what splits the blue sum into its own pseudo, which is exactly what prevents the shared `sum` from reaching three deaths, so the share and that extender are mutually exclusive. The extender's only job is lifting pseudo 126's reg_n_refs from 12 to 18 (priority 32727 -> 60000, ord 4 -> ord 1), so another placement that survives to flow.c would serve.
- probe: Seven complete bodies in tmp/grind/func_8003DE14/s27/v2/ scored in one tools/sweep_variants.py call against the 12/173 baseline.
- result: p1_bshift / p3_gch / p8_orall / p9_sumpx / p10_sumbf all score exactly 42 - identical to removing the extender entirely - and p4_orgch 45, p11_orbsh 47, all at 173 build insns. Every placement folds before flow.c counts references, so none restores 126 to 18 refs. The three-way chassis therefore stays at 42 with its seat bank rotated one position (122 $a0->$a2, 126 $v1->$a3, 123 $a1->$v1, 109 $a2->$t0, 108 $a3->$t1, 121 $t0->$t2, 130 $t1->$a0, 129 $t2->$a1).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; three-way-shared-sum chassis (tmp/grind/func_8003DE14/s27/v/s3_ext_none.c), j extender present, blue-path g_src extender absent by construction

## [s27] KILL RE-AUDIT (mandated): the s26 2x2 extender prices reproduce on today's chassis.
- mechanism: An extender's price is a function of the whole arm's allocation, so a price recorded on an earlier dump is not transferable; the re-audit re-measures the incumbent and the g_src-removed body in the same sweep as this session's new bodies.
- probe: s2_ext_blue (= the incumbent p2 body) and s2_ext_none (g_src extender removed, j extender kept) inside the s27 tools/sweep_variants.py call, plus the driver-style dispatch measurement of the incumbent.
- result: 12 and 40 respectively, both at 173 build insns - identical to s26's 2x2 entries for 'both present' and 'g_src removed'. The banked prices are current; the incumbent chassis is independently confirmed at 12/173 by the dispatch measurement.
- verdict: CONFIRMED

## [s28] Pseudo 126 (the green carrier) can be lifted above pseudo 122 on the three-way-shared-sum chassis by SHORTENING its live length - moving `g_src = ((u32)px >> 2) & 0xF8` down to its use, so priority floor_log2(12)*12/livelen*10000 rises from 32727 to ~60000.
- mechanism: qty/allocno priority is floor_log2(refs)*refs*size/(death-birth)*10000 (local-alloc.c:1660 and the global analogue); with refs held at 12 a livelen of 6 instead of 11 gives 60000, which is above 122's 48000, and moving a definition closer to its single use is ordinary C needing no construct at all.
- probe: six complete bodies in tmp/grind/func_8003DE14/s28/v1/ (green source late, both sources late, red source late, source inlined into the product, source scoped into a block, unchanged control) swept in one tools/sweep_variants.py call; then the `.lreg` register header lines and the emitted `.s` captured for base / v01 / v02 via tmp/grind/func_8003DE14/s28/regs.sh and asmcap.sh.
- result: all three reordered bodies score exactly 42 at 173 build insns, identical to the control. The `.lreg` line for the green carrier is character-identical in all three ("used 12 times across 11 insns in block 10; dies in 2 places") and the emitted assembly is byte-identical (3429 bytes, empty diff). The two bodies that inline or re-scope the source drop a real insn (167) and score 60.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s27 three-way-shared-sum chassis (memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c), j extender present, blue-path g_src extender absent by construction

## [s28] The reason source statement position cannot move a pseudo's live length here is the FIRST scheduling pass: sched.c re-emits the blend arm in one canonical order before local-alloc ever measures a span.
- mechanism: In GCC 2.7.2 the first `schedule_insns` runs between combine and local-alloc. `priority()` (tools/gcc-2.7.2/sched.c:1434) computes INSN_PRIORITY by walking LOG_LINKS - the dependence graph - and nothing else, so two bodies whose arms have the same dependence graph get identical priorities and `schedule_block` emits them in the same order whatever order they arrived in. `.lreg`'s "used N times across M insns" is therefore a property of the scheduled order, not of the C.
- probe: per-pass RTL for the function region captured for the control and for `v02_both_late` (tmp/grind/func_8003DE14/s28/d_base/, d_v02/) and compared with a UID-stripping normalizer (s28/norm.py) pass by pass: rtl, jump, cse, loop, cse2, flow, combine, sched, lreg, greg.
- result: through `combine` the two bodies differ in real insn ORDER (records 3-12 of the 30-insn arm window are a different sequence); at `sched` they are position-for-position identical, with only pseudo numbers and insn_list UIDs differing. The scheduled arm order front-loads all six multiplies ahead of every sum, which fixes the green carrier's span at 11 insns. The boundary was measured too: relocating `src++` DOES change the dependence graph and yields a different 169-insn body at 58.
- verdict: CONFIRMED

## [s28] On the three-way-shared-sum chassis the surrounding bank rotation can be bought back by raising the green carrier's reference count with a REAL second job instead of the dead `(X + g_src) - g_src` extender - carry the blue channel in `g_src` too.
- mechanism: The s27 rotation is priced: 126 fell to 12 refs / livelen 11 (32727) and lost ord 1 to 122 (px, 24 refs / livelen 20, 48000). Making `g_src` also the blue source and blue complement product gives it three real defs and three real uses in the arm, which raises reg_n_refs honestly (the references survive to flow.c because the values are consumed) instead of relying on a fold-prone identity.
- probe: nine complete bodies in tmp/grind/func_8003DE14/s28/v2/ (blue on g_src both ways, blue source only, split green carrier two ways, GRB order, blue on r_src, one carrier for all three sources, control) swept in one call; then `.lreg` register lines for the winner via s28/regs.sh and the residual via tmp/grind/func_8003DE14/s24/ed2.py.
- result: `w01_gsrc_blue_both` scores 36 / 173 - a 6-point drop from the chassis's 42 - and `w08_gsrc_blue_bp` ties it. The `.lreg` confirms the pricing: the green carrier goes 12 refs / 11 insns / 2 deaths -> 24 refs / 12 insns / 3 deaths (priority 80000), the shared `sum` prints 18 / 13 / 3, and `px` drops to 12 refs and out of block 10. Alternatives: blue source only on g_src 54, blue on r_src 45, split green carrier 45 (174 insns), GRB order 47, one carrier for all three sources 52. Banked as memory/grind/func_8003DE14/chassis_s28_threeway_w01_36.c.
- verdict: CONFIRMED

## [s28] The 36 body can be improved further by cheap spelling refinements around the output word and the source reads.
- mechanism: With the carrier pricing fixed, the remaining rows are the surrounding bank; the untried cheap axes were reading the channel sources from `pixel` instead of the `px` copy, giving `px` a real job in the output word, naming the high bit, and relocating `src++`.
- probe: seven bodies in tmp/grind/func_8003DE14/s28/v3/ swept against the 36 control.
- result: reading either source from `pixel` ties at 36; routing the high bit through `px` is 43 at 174 insns; a named `hi` is 56; both `src++` relocations are 58 at 169 insns. No refinement improves on 36.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s28 three-way + blue-on-g_src chassis (memory/grind/func_8003DE14/chassis_s28_threeway_w01_36.c), j extender present

### LIVE FRONTIER (for s29)

1. **Close the last 36 on the w01 three-way chassis by dependence-graph edits, not
   statement order** (s28 proved order is normalized by sched.c). The residual on
   w01 now contains genuine ORDER rows (ed2.py shows `addu v1,a1,v1` / `sra a1`
   hoisted ahead of the third multiply, i.e. two target rows deleted and two
   inserted), which is a scheduler-visible difference and therefore attackable by
   changing LOG_LINKS: split the blue channel's dependence chain (e.g. compute
   `b * factor` into its own named local so the mult is no longer a direct
   operand of the shared sum), or re-associate `sum = g_src + b * factor` so the
   sum depends on the mult through one more insn. Use tools/sched_solver on the
   w01 body first - the residual is now part RA, part order, and the solver is
   order- and clock-exact.

2. **Decide which basin to spend on.** p2 is 12 with two F1 extenders (not
   submittable); the three-way basin is now 36 but is ORDINARY C end to end except
   for the j extender, and it owns both disputed seats. Before more grinding, run
   `goal_from_tgt.py classify` on the w01 body to get its stage verdict - if it is
   RA-only, re-run extract.py + inverse.py global on the w01 model with the full
   disposition goal (s27 frontier item 2 was never run on a 36-point body) and
   spell the top vectors.

3. **Price the j extender on the w01 chassis.** Its price has only ever been
   measured on the p2 chassis (-7 with the g_src extender present, +2 without).
   The w01 body carries it; a 2x2 on w01 (j present/absent x blue-on-g_src
   present/absent) would say whether the 36 is an ordinary-C floor or still
   extender-propped, which decides whether this basin can ever be submitted.

## [s28] Pseudo 126 (the green carrier) can be lifted above pseudo 122 on the three-way-shared-sum chassis by shortening its live length - moving `g_src = ((u32)px >> 2) & 0xF8` down to its use, so priority floor_log2(12)*12/livelen*10000 rises from 32727 to about 60000.
- mechanism: Allocno/quantity priority is floor_log2(refs)*refs*size/(death-birth)*10000 (local-alloc.c:1660 and the global analogue). With refs held at 12, a livelen of 6 instead of 11 gives 60000, above pseudo 122's 48000, and moving a definition closer to its single use is ordinary C requiring no construct.
- probe: Six complete bodies in tmp/grind/func_8003DE14/s28/v1/ (green source late, both sources late, red source late, source inlined into the product, source scoped into a block, unchanged control) swept in one tools/sweep_variants.py call; then .lreg register header lines and the emitted .s captured for base / v01 / v02 via tmp/grind/func_8003DE14/s28/regs.sh and asmcap.sh.
- result: All three reordered bodies score exactly 42 at 173 build insns, identical to the control. The .lreg line for the green carrier is character-identical in all three ('used 12 times across 11 insns in block 10; dies in 2 places'), and the emitted assembly is byte-identical (3429 bytes, empty diff). The two bodies that inline or re-scope the source delete a real insn (167 build insns) and score 60.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s27 three-way-shared-sum chassis (memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c), j extender present, blue-path g_src extender absent by construction

## [s28] The reason source statement position does not move a pseudo's live length on this chassis is the first scheduling pass: sched.c re-emits the blend arm in one canonical order before local-alloc measures any span.
- mechanism: In GCC 2.7.2 the first schedule_insns runs between combine and local-alloc. priority() (tools/gcc-2.7.2/sched.c:1434) computes INSN_PRIORITY by walking LOG_LINKS - the dependence graph - and nothing else, so two bodies whose arms have the same dependence graph get identical priorities and schedule_block emits them in the same order whatever order they arrived in. The .lreg 'used N times across M insns' line is therefore a property of the scheduled order, not of the C.
- probe: Per-pass RTL for the function region captured for the control and for v02_both_late (tmp/grind/func_8003DE14/s28/d_base/, d_v02/) and compared with a UID-stripping normalizer (s28/norm.py) pass by pass: rtl, jump, cse, loop, cse2, flow, combine, sched, lreg, greg.
- result: Through combine the two bodies differ in real insn ORDER (records 3-12 of the 30-insn arm window are a different sequence: base emits the green lshiftrt/and pair before the red multiplies, v02 emits the red multiplies and red sum first). At sched they are position-for-position identical, with only pseudo numbers and insn_list UIDs differing. The scheduled arm order front-loads all six multiplies ahead of every sum, which pins the green carrier's span at 11 insns (birth at the green `and 248`, death at the green `plus`). Boundary measured: relocating src++ DOES change the dependence graph and yields a different 169-insn body at 58.
- verdict: CONFIRMED

## [s28] On the three-way-shared-sum chassis the surrounding bank rotation can be bought back by raising the green carrier's reference count with a REAL second job instead of the fold-prone `(X + g_src) - g_src` identity - carry the blue channel in `g_src` too.
- mechanism: The s27 rotation is priced: pseudo 126 fell to 12 refs / livelen 11 (32727) and lost ord 1 to 122 (px, 24 refs / livelen 20, 48000). Making g_src also the blue source and the blue complement product gives it three real defs and three real uses in the arm, so reg_n_refs rises honestly - the references survive to flow.c because the values are consumed - instead of depending on an identity that combine folds.
- probe: Nine complete bodies in tmp/grind/func_8003DE14/s28/v2/ (blue on g_src both ways, blue source only, split green carrier two ways, GRB order, blue on r_src, one carrier for all three sources, control) swept in one call; then .lreg register lines for the winner via s28/regs.sh and the residual via tmp/grind/func_8003DE14/s24/ed2.py.
- result: w01_gsrc_blue_both scores 36 / 173 - a 6-point drop from that chassis's 42 - and w08_gsrc_blue_bp ties it. The .lreg confirms the intended pricing change: the green carrier goes 12 refs / 11 insns / 2 deaths -> 24 refs / 12 insns / 3 deaths (priority 80000), the shared sum prints 18 / 13 / 3 (55384), and px falls to 12 refs and leaves block 10. Alternatives: blue source only on g_src 54, blue on r_src 45, split green carrier 45 (174 insns), GRB order 47, one carrier for all three sources 52. Banked as memory/grind/func_8003DE14/chassis_s28_threeway_w01_36.c.
- verdict: CONFIRMED

## [s28] The 36 body improves further under cheap spelling refinements around the output word and the channel source reads.
- mechanism: With the carrier pricing fixed by w01, the remaining rows are the surrounding bank; the untried cheap axes were reading the channel sources from `pixel` instead of the `px` copy, giving px a real job in the output word, naming the high bit, and relocating src++.
- probe: Seven complete bodies in tmp/grind/func_8003DE14/s28/v3/ swept against the 36 control.
- result: Reading either channel source from `pixel` ties at 36; routing the output word's high bit through px is 43 at 174 insns; a named `hi` local is 56; both src++ relocations are 58 at 169 insns. No refinement beats 36.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s28 three-way + blue-on-g_src chassis (memory/grind/func_8003DE14/chassis_s28_threeway_w01_36.c), j extender present

## [s29] The 12 residual rows of the p2 chassis are the C-level shape "each channel RESULT is written back into its own source carrier", combined with s27's three-way shared `sum`; spelled that way the floor drops from 12 to 5.
- mechanism: the three-way `sum` has three defs and three deaths, so local-alloc.c:470-476 gives it reg_qty = -1 and it becomes a global allocno owning $v0 across the arm (s27); writing each channel result back into r_src / g_src / px puts each shift's destination on the source's dead seat, which is literally the target's `sra $a1,$v0,15` / `sra $v1,$v0,10` / `sra $a0,$v0,5`, and simultaneously gives each source carrier three real defs and three real deaths, so the reference counts the s21 F1 g_src chain extender was buying are bought by real code instead.
- probe: eight bodies in tmp/grind/func_8003DE14/s29/v2/ then a 32-body cross product in tmp/grind/func_8003DE14/s29/v3/ (red result carrier x green result carrier x red mask placement x green mask placement x j extender), swept with tools/sweep_variants.py; residuals read with tmp/grind/func_8003DE14/s24/ed2.py; .lreg register header lines captured via tmp/grind/func_8003DE14/s29/regs.sh.
- result: 5 / 173 with the j extender and 12 / 173 without it, from a 12-point chassis. Banked as memory/grind/func_8003DE14/candidate.c and chassis_s29_targetmap_5.c. The g_src chain extender is deleted from the body.
- verdict: CONFIRMED

## [s29] The red and green channels must mask ASYMMETRICALLY - the red result masked inside its shift statement, the green result masked in the or-chain.
- mechanism: the two channel source carriers are symmetric allocnos; making their statement shapes symmetric makes the tie-break in local-alloc swap them, and the residual of the symmetric body prints $a1 <-> $v1 mirrored on every red/green row (t94, t100, t102, t108, t117-t122).
- probe: the same 32-body cross product (tmp/grind/func_8003DE14/s29/v3/).
- result: asymmetric 5; both-in-the-or 17; both-in-the-shift 22; red result in a fresh r_ch 17 (any mask placement); both results fresh 42-48.
- verdict: CONFIRMED

## [s29] The s21 j chain extender `((s32)dst_buf + j) - j` has an ORDINARY substitute: declaring `s32 j = 0;` inside the `if (total > 0)` block.
- mechanism: the extender's whole job is the $t4 = j / $t5 = complement seat pair. Declaring j after `complement` inside the guard changes j's birth point relative to complement's, which re-orders the two allocnos without any dead code.
- probe: seven bodies in tmp/grind/func_8003DE14/s29/v6/ and five in v7/ against the extender-free 12-point control.
- result: 8 / 173 - the best FAKE-free body this function has had (the standing number was 38). Its one remaining row group is the dst cursor init `addiu $a2,$sp,1040` sunk past the guard; moving the cursors inside the guard as well is 11, and all 24 per-row declaration permutations are 8.
- verdict: CONFIRMED

## [s29] KILL RE-AUDIT (mandated): the s21 j chain extender is worth nothing on the s28 w01 chassis.
- mechanism: w01's blue-on-g_src reuse already supplies the reference lift the extender was buying, so the two levers are redundant with each other.
- probe: the 2x2 in tmp/grind/func_8003DE14/s29/v1/ (w01 and the s27 three-way body, each with and without the extender, plus the p2 controls), one sweep.
- result: w01 36 with the extender and 36 without; three-way 42 / 46; p2 12 / 19. The banked p2 price is re-confirmed and w01 is re-classified as a FAKE-free body.
- verdict: CONFIRMED

## [s29] Giving the green carrier a REAL second job other than the blue channel (the b*factor product, the blue complement product, the pixel's high bit, the blue mask term, or the whole output word) restores its reference count and undoes the s27 bank rotation.
- mechanism: s28 showed the rotation is priced by reg_n_refs on pseudo 126; if the count is what matters, any real consumed value carried in g_src should buy it back.
- probe: nine complete bodies in tmp/grind/func_8003DE14/s29/v1/ (b1-b9) swept in one call against the 42-point s27 three-way control.
- result: 43 to 56 - every one is WORSE than the control. Which value the carrier holds, not how many references it has, is what moves the bank. Two banked in rejected/ (s29-bfactor-on-gsrc-threeway-46.c, s29-outword-on-gsrc-threeway-43.c).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s27 three-way-shared-sum chassis (memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c), j extender present, blue-path g_src extender absent by construction

## [s29] The red channel's two remaining rows (the shift's intermediate seated on the sum's register instead of r_src's dead seat) can be moved by respelling the red statement.
- mechanism: the target emits `sra $a1,$v0,15` then `andi $a1,$a1,31`, i.e. the shift's destination is r_src's seat and the mask is applied in place; our build emits the shift in place on the sum's register and the mask into r_src.
- probe: eight red spellings on the 5-point chassis (tmp/grind/func_8003DE14/s29/v4/ and v9/): a two-statement split, a compound-assignment split, a fresh local for the shift stage, `(u32)sum >> 15`, `& 31`, `(sum & 0xF8000) >> 15`, the red sum inlined, the red sum in a fresh local.
- result: six tie at 5 and the two split forms are 20 (the split produces the target's two-insn shape but swaps the red/green carriers, banked as rejected/s29-red-mask-split-two-statements-20.c). No spelling of the red statement alone moves those two rows.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_5.c), j chain extender present, g_src chain extender absent

## [s29] The latch's three rows (the two `lh` destinations swapped) can be moved by respelling the loop condition or the `total` computation.
- mechanism: qty_compare_1 (local-alloc.c:1660) prices the first load's quantity at floor_log2(6)*6/3*10000 = 40000 and the second at 2*6/2*10000 = 60000, so the shorter-span SECOND load takes $v0; the target has the FIRST load there.
- probe: nine bodies on the 5-point chassis (tmp/grind/func_8003DE14/s29/v5/ and v9/): operand swap in the latch, reversed `>`, `!= 0` wrapper, `<=` with a -1, signed casts on both halves, named w/h locals, no `total` local at all, `total` recomputed in the latch, operand swap at the top.
- result: every inert form ties at 5; the two that change anything are worse (top-level operand swap 7, `total` recomputed in the latch 7, `<=` form 8 at 174 insns). Condition spelling does not reach that seat pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_5.c), j chain extender present

## [s29] The per-row declaration order (total / src / dst / factor) reaches the allocator on the extender-free 8-point chassis.
- mechanism: with `s32 j = 0;` moved inside the guard, GCC sinks the dst cursor init past the guard; if declaration order reached the allocator, some permutation should pull it back.
- probe: all 24 permutations generated by tmp/grind/func_8003DE14/s29/gen8.py and swept in one call.
- result: all 24 score exactly 8 at 173 build insns. Byte-inert, consistent with s28's finding that sched.c normalizes statement order before local-alloc measures anything.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 extender-free target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_ordinary_8.c)

### LIVE FRONTIER (for s30)
1. **Reconcile the two halves of the j/dst seat problem.** The 5-point body has the
   dst cursor init in the right place but needs the j chain extender; the 8-point
   body needs no FAKE at all but sinks `addiu $a2,$sp,1040` past the guard. Probe:
   on the 8-point chassis, sweep forms that give the dst cursor an earlier real
   use or an earlier birth - `u16 *dst = &dst_buf[0];`, the cursors declared at the
   function scope and re-initialised per row, `dst` written through before the
   guard, the guard rewritten as an early `goto` / `continue` instead of an if -
   and read the .lreg birth points for the two cursors on every body that moves.
2. **The red shift's seat (t118/t119).** Eight spellings of the red statement are
   inert, so the input is not the statement - it is which quantity combine_regs
   ties the shift's intermediate to. Probe: capture BB2_QTY_DEBUG on the 5-point
   body, find the quantity that holds the shift intermediate, and attack the tie
   at the level that decides it (whether the sum pseudo is still live at the sra),
   e.g. by giving the red sum a carrier that is read once more after the shift.
3. **The latch's two `lh` seats (t133/t134/t136).** The priority arithmetic favours
   the second load; the target has the first. Probe: make the FIRST load's quantity
   shorter-spanned or higher-priced - e.g. a form in which `rect[2]` is also read
   by the loop body, or in which the multiply's operands are born one insn apart -
   and confirm with tools/ra_solver/inverse.py on the block-11 model rather than by
   spelling guesses.

## [s29] The 12 residual rows of the p2 chassis are the C-level shape 'each channel RESULT is written back into its own source carrier' combined with s27's three-way shared `sum` local.
- mechanism: The three-way `sum` has three defs and three deaths, so local-alloc.c:470-476 gives it reg_qty = -1 and it becomes a global allocno owning $v0 across the blend arm (s27's result). Writing each channel result back into r_src / g_src / px then puts each shift's destination on the source's dead seat, which is literally the target's `sra $a1,$v0,15` / `sra $v1,$v0,10` / `sra $a0,$v0,5`, and at the same time gives each source carrier three real defs and three real deaths, so the reference counts the s21 F1 g_src chain extender was buying with a dead identity are bought by real code instead.
- probe: Eight bodies in tmp/grind/func_8003DE14/s29/v2/ and then a 32-body cross product in tmp/grind/func_8003DE14/s29/v3/ (red result carrier x green result carrier x red mask placement x green mask placement x j extender), swept with tools/sweep_variants.py; residuals read with tmp/grind/func_8003DE14/s24/ed2.py; .lreg register header lines captured via tmp/grind/func_8003DE14/s29/regs.sh.
- result: 5 / 173 build insns with the j extender present and 12 / 173 without it, from a chassis that had been flat at 12 for six sessions. The .lreg on the 5-point body prints four three-death local-alloc-ineligible allocnos in block 10 (px 30 refs/27 insns, r_src 18/20, g_src 18/18, sum 18/6). The g_src chain extender is deleted from the candidate. Banked as memory/grind/func_8003DE14/candidate.c and chassis_s29_targetmap_5.c.
- verdict: CONFIRMED

## [s29] The red and green channels must mask asymmetrically - the red result masked inside its shift statement, the green result masked in the or-chain.
- mechanism: The two channel source carriers are symmetric allocnos; making their statement shapes symmetric makes local-alloc's tie-break swap them, and the symmetric body's residual prints $a1 <-> $v1 mirrored on every red/green row (t94, t100, t102, t108, t117-t122).
- probe: The 32-body cross product in tmp/grind/func_8003DE14/s29/v3/.
- result: Asymmetric 5; both masks in the or 17; both masks in the shift statement 22; the red result in a fresh r_ch local 17 at any mask placement; both results fresh 42-48. Banked as rejected/s29-both-channels-mask-in-or-17.c.
- verdict: CONFIRMED

## [s29] The s21 j chain extender `((s32)dst_buf + j) - j` has an ordinary substitute: declaring `s32 j = 0;` inside the `if (total > 0)` block after `complement`.
- mechanism: The extender's whole job is the $t4 = j / $t5 = complement seat pair. Declaring j after complement inside the guard changes j's birth point relative to complement's, re-ordering the two allocnos with no dead code anywhere.
- probe: Seven bodies in tmp/grind/func_8003DE14/s29/v6/ and five in v7/ swept against the extender-free 12-point control, then the residual via ed2.py.
- result: 8 / 173 - the best body this function has ever had that carries zero FAKE constructs (the standing number was 38, set in s26). Its one remaining row group is the dst cursor init `addiu $a2,$sp,1040` sunk past the guard. Banked as chassis_s29_targetmap_ordinary_8.c.
- verdict: CONFIRMED

## [s29] KILL RE-AUDIT (mandated): the s21 j chain extender changes the score on the s28 w01 chassis.
- mechanism: s26/s27 priced the extender at -7 on p2 and s28 inherited that price onto the w01 chassis without re-measuring it there; if w01's blue-on-g_src reuse already supplies the reference lift, the two levers are redundant.
- probe: A 2x2 in tmp/grind/func_8003DE14/s29/v1/ (w01 and the s27 three-way body, each with and without the extender) plus the p2 controls, all in one tools/sweep_variants.py call.
- result: w01 36 with the extender and 36 without - the extender is worth exactly 0 there, so w01 was already a FAKE-free 36. The other prices re-confirm: three-way 42 / 46, p2 12 / 19. This retired the s26 'honest ordinary-C floor is 38' number before any new probe ran.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s28 three-way + blue-on-g_src chassis (memory/grind/func_8003DE14/chassis_s28_threeway_w01_36.c), j chain extender the only FAKE construct present

## [s29] Giving the green carrier a real second job other than the blue channel - the b*factor product, the blue complement product, the pixel's high bit, the blue mask term, or the whole output word - restores its reference count and undoes the s27 bank rotation.
- mechanism: s28 showed the rotation is priced by reg_n_refs on pseudo 126, so if the count is what matters, any real consumed value carried in g_src should buy it back without displacing px the way w01's blue-on-g_src does.
- probe: Nine complete bodies (b1-b9) in tmp/grind/func_8003DE14/s29/v1/ swept in one call against the 42-point s27 three-way control, including the same jobs relocated to r_src, rp and gp.
- result: 43 to 56 - every one is worse than the control. Which value the carrier holds, not how many references it has, is what moves the bank. Two banked in rejected/ (s29-bfactor-on-gsrc-threeway-46.c, s29-outword-on-gsrc-threeway-43.c). This closes the s27/s28 'raise the green carrier's refs past 24' frontier item negatively, and the session's real progress came from the opposite direction.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s27 three-way-shared-sum chassis (memory/grind/func_8003DE14/chassis_s27_threeway_sum_42.c), j chain extender present, blue-path g_src extender absent by construction

## [s29] The red channel's two remaining rows (the shift's intermediate seated on the sum's register instead of r_src's dead seat) can be moved by respelling the red statement.
- mechanism: The target emits `sra $a1,$v0,15` then `andi $a1,$a1,31` - the shift's destination is r_src's seat and the mask is applied in place - while our build emits the shift in place on the sum's register and the mask into r_src.
- probe: Eight red spellings on the 5-point chassis (tmp/grind/func_8003DE14/s29/v4/ and v9/): a two-statement split, a compound-assignment split, a fresh local for the shift stage, `(u32)sum >> 15`, `& 31`, `(sum & 0xF8000) >> 15`, the red sum inlined, and the red sum in a fresh local.
- result: Six tie at 5 and the two split forms are 20 - the split does produce the target's two-insn shape but swaps the red/green source carriers, so it trades two rows for twelve. Banked as rejected/s29-red-mask-split-two-statements-20.c. No spelling of the red statement alone moves those two rows.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_5.c), j chain extender present, g_src chain extender absent

## [s29] The latch's three rows (the two `lh` destinations swapped) can be moved by respelling the loop condition or the `total` computation.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) prices the first load's quantity at floor_log2(6)*6/3*10000 = 40000 and the second at 2*6/2*10000 = 60000, so the shorter-span second load takes $v0 while the target has the first load there.
- probe: Nine bodies on the 5-point chassis (tmp/grind/func_8003DE14/s29/v5/ and v9/): latch operand swap, reversed `>`, `!= 0` wrapper, `<=` with a -1, signed casts on both halves, named w/h locals, no `total` local at all, `total` recomputed inside the latch, and the top-level `total` operand order swapped.
- result: Every inert form ties at 5; the forms that change anything are worse (top-level operand swap 7, `total` recomputed in the latch 7, the `<=` form 8 at 174 build insns). Condition spelling does not reach that seat pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_5.c), j chain extender present

## [s29] The per-row declaration order (total / src / dst / factor) reaches the allocator on the extender-free 8-point chassis and can pull the sunk dst cursor init back above the guard.
- mechanism: With `s32 j = 0;` moved inside the guard GCC sinks `addiu $a2,$sp,1040` past it; if declaration order reached the allocator some permutation should restore the earlier birth point.
- probe: All 24 permutations generated by tmp/grind/func_8003DE14/s29/gen8.py and swept in one tools/sweep_variants.py call.
- result: All 24 score exactly 8 at 173 build insns. Byte-inert, consistent with s28's finding that sched.c normalizes statement order before local-alloc measures any span. Moving the cursor initialisations inside the guard as well is 11 (rejected/s29-cursors-inside-guard-11.c) and reusing the dead `total` as the complement carrier is 18.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 extender-free target-map chassis (memory/grind/func_8003DE14/chassis_s29_targetmap_ordinary_8.c), no FAKE construct present

## s30 (SOLVER) — hypotheses

### H-s30-1 — KILLED (instance)
**Statement.** On the extender-free 8-point chassis, giving the dst cursor an
earlier birth or an earlier real use (or reshaping the `if (total > 0)` guard)
restores the target's `addiu $a2,$sp,1040` position, closing the three-row
dst-cursor group.
**Probe.** Ten bodies in `tmp/grind/func_8003DE14/s30/v1/`, sweeping s29's own
next-probe list plus five more.
**Result.** Six are byte-identical to the 8 control (`&dst_buf[0]`, both cursors
as `&x[0]`, guard as an early `goto`, `LoadImage(..., &dst_buf[0])`, cursors
after `factor`, guard as `while … break`); four are worse (9, 10, 14, 22); the
function-scope-cursor form collapses the loop to 124 insns and scores 108.
**Verdict.** KILLED, instance — measured on the s29 extender-free target-map
chassis (`chassis_s29_targetmap_ordinary_8.c`), no FAKE constructs present,
HEAD 2026-09-11.

### H-s30-2 — KILLED (instance)
**Statement.** The red shift's seat (t118/t119) moves if the shared `sum`'s tie
to the shift is broken by giving the red channel its own sum carrier, by
reordering the channels, or by keeping the red sum live past the shift.
**Mechanism.** `combine_regs` (local-alloc.c:1854-1897) ties the `sra` dest to
its dying source; the dying source is the three-way `sum`, which owns $v0.
**Probe.** Ten bodies in `tmp/grind/func_8003DE14/s30/v2/` on the 5-point chassis.
**Result.** Two ties at 5 (`(u32)sum >> 15`, red sum inlined into the shift) and
eight regressions: 11, 29, 32, 34, 34, 34, 35, 45. Every form that takes the red
sum out of the three-way carrier costs ~30 points.
**Verdict.** KILLED, instance — measured on `memory/grind/func_8003DE14/candidate.c`
(5 points, the s21 j chain extender present), HEAD 2026-09-11.

### H-s30-3 — CONFIRMED
**Statement.** The whole remaining residual of the 5-point body is register
allocation and nothing else: the register-blanked instruction multisets of our
stream and the target's are identical.
**Probe.** `inverse_compose.py classify code6cac_c2 func_8003DE14
--target-object build/src/code6cac_c2.o --ours-object
tmp/sandbox/func_8003DE14/code6cac_c2.o`.
**Result.** `FIRST DIVERGENCE: RA`, honest 173 insns / target 173 insns, with the
five differing rows printed as same-instruction/different-register pairs.
**Verdict.** CONFIRMED.

### H-s30-4 — KILLED (class)
**Statement.** While the latch emits `lh rect[2]` then `lh rect[3]` then `mult`
with both loads dying at that `mult` and carrying equal `qty_n_refs` and equal
`qty_size` as block-local quantities, `qty_compare_1` always prices the second
load strictly higher than the first, so the second load always takes $v0 and the
first always takes $v1 — the target's seating (first load in $v0) is not produced
by any latch-expression spelling with those inputs.
**Mechanism.** `pri = floor_log2(refs)*refs*size/(death-birth)*10000`
(tools/gcc-2.7.2/local-alloc.c:1660). Measured block-11 inputs: qty0 (lh 4($s0))
birth 4 death 8 refs 6 -> 30000; qty1 (lh 6($s0)) birth 6 death 8 refs 6 ->
60000. Both deaths coincide, so span(qty0) = span(qty1) + 2 identically, and
`find_free_reg`'s ascending scan gives the first-allocated quantity $v0 while the
second is excluded from $v0 over its strictly containing span.
**Probe.** `local_alloc.py` ground truth (order 8/8, assign 26/27 on this
function) plus six latch spellings in `tmp/grind/func_8003DE14/s30/v3/`.
**Result.** Four spellings tie at 5 byte-for-byte (operand swap, `!(j >= …)`,
`(s32)` casts, `rect[3]*rect[2] > j`); the two that move (`… - j > 0` = 7,
`j != …` = 7 at 172 insns) move by changing the instruction stream, not the
seats. Exactly what the formula predicts.
**Verdict.** KILLED, class. Predicate: tools/gcc-2.7.2/local-alloc.c:1660.
**What it leaves open (the next probe).** The kill is conditioned on the two
loads being equal-refs, equal-size, block-local quantities. Escapes, in
decreasing plausibility: (a) a third source-level reference to the `rect[2]`
pseudo inside block 11 (refs 6 -> 9 gives 67500 > 60000 outright, refs 8 ties and
the tie-break takes the LOWER qty number, which is qty0); (b) the `rect[2]`
pseudo acquiring a reference outside block 11 or a third death, which makes
local-alloc skip it entirely (local-alloc.c:470-476) and hands it to global.c;
(c) `qty_size` 2 (DImode). (a) and (b) are C-expressible; the difficulty is doing
either without adding an instruction, since classify proves the multiset is
already exact.

## [s30] The whole remaining residual of the 5-point body is register allocation and nothing else: the register-blanked instruction multisets of our stream and the target's are identical.
- mechanism: inverse_compose.py's funnel triage - the front end / cse / combine / loop passes build the insn multiset, RA renames a fixed multiset, sched.c orders a fixed allocated stream. Comparing the two objdump renderings in the same language tells you which stage first diverges.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_c2 func_8003DE14 --target-object build/src/code6cac_c2.o --ours-object tmp/sandbox/func_8003DE14/code6cac_c2.o
- result: FIRST DIVERGENCE: RA. honest 173 insns, target 173 insns. The five differing rows print as same-instruction/different-register pairs: andi a1,v0,0x1f vs andi a1,a1,0x1f; lh v0,6(s0)/lh v1,4(s0) vs lh v0,4(s0)/lh v1,6(s0); mult v1,v0 vs mult v0,v1; sra v0,v0,0xf vs sra a1,v0,0xf. No pre-RA work and no scheduler work remains on this body.
- verdict: CONFIRMED

## [s30] While the latch emits lh rect[2], lh rect[3], mult with both loads dying at that mult and carrying equal qty_n_refs and equal qty_size as block-local quantities, qty_compare_1 prices the second load strictly higher, so the second load takes $v0 and the first takes $v1 - no latch-expression spelling with those inputs produces the target's seating (first load in $v0).
- mechanism: pri = floor_log2(refs)*refs*size/(death-birth)*10000. Measured block-11 inputs from tools/ra_solver/local_alloc.py (order 8/8 blocks, assign 26/27 qtys on this function): qty0 = lh 4($s0), pseudo 149, birth 4, death 8, refs 6 -> 30000, got $v1; qty1 = lh 6($s0), pseudo 152, birth 6, death 8, refs 6 -> 60000, got $v0. Both deaths coincide at the mult, so span(qty0) = span(qty1) + 2 identically; the higher-priority quantity is allocated first and find_free_reg's ascending scan gives it $v0, after which the first load is excluded from $v0 across its own strictly-containing span. The suggested-register pass cannot intervene: qty_phys_sugg / qty_phys_copy_sugg are set only by a hard-register tie (local-alloc.c:1854-1897) and neither load touches a hard register, so both enter the main pass with nsugg = ncopysugg = 0.
- probe: local_extract.py --suggest + local_alloc.py --func func_8003DE14 for ground truth, then six latch spellings in tmp/grind/func_8003DE14/s30/v3/ measured against the prediction.
- result: Four spellings tie at 5 byte-for-byte (operand swap, !(j >= ...), (s32) casts, rect[3]*rect[2] > j) - exactly what the formula requires, since none of them can move refs, span or size. The two that move do so by changing the instruction stream rather than the seats: rect[2]*rect[3] - j > 0 is 7, and j != rect[2]*rect[3] is 7 at 172 insns (it deletes an insn the target has). The escapes the kill leaves open are named in hypotheses.md H-s30-4: a third source-level reference to the rect[2] pseudo inside block 11 (refs 6 -> 9 gives 67500 outright; refs 8 ties at 60000 and qty_compare_1 breaks ties by LOWER qty number, which is qty0), the rect[2] pseudo acquiring a reference outside block 11 or a third death so local-alloc skips it (local-alloc.c:470-476) and global.c seats it, or qty_size 2.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s29 target-map chassis memory/grind/func_8003DE14/candidate.c at 5/173, s21 j chain extender present
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1660

## [s30] On the 5-point target-map chassis, the ten C forms tried in s30 that break the shared sum's combine_regs tie to the red shift - giving the red or green channel its own sum carrier, rotating the channel order, reusing the red sum as the green accumulator, or relocating the red sum-and-shift pair - do not move the red shift's seat off $v0.
- mechanism: combine_regs (local-alloc.c:1854-1897) ties the sra's destination to its dying source. The dying source is the three-way shared sum, which owns $v0 across the blend arm, so the shift's intermediate inherits $v0; the target instead seats it on $a1, r_src's dead seat. Breaking the tie requires the sum pseudo to be live past the shift.
- probe: Ten bodies in tmp/grind/func_8003DE14/s30/v2/ swept with tools/sweep_variants.py on the 5-point chassis.
- result: Two ties at 5 ((u32)sum >> 15, red sum inlined into the shift statement) and eight regressions: red sum+shift moved between green and blue 11, channel order G,R,B 29, red sum+shift after blue 32, green given its own sum carrier 34, red sum reused as the green accumulator 34, channel order B,R,G 34 (174 insns), red given its own sum carrier rp 35, red and green both given their own carriers 45. Every form that takes a channel's sum out of the three-way carrier costs about 30 points, which is the s29 finding that the three-way shared sum is load-bearing, now measured from the opposite direction.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis memory/grind/func_8003DE14/candidate.c (5/173), s21 j chain extender present

## [s30] On the extender-free 8-point chassis, the ten cursor-birth and guard-shape spellings tried in s30 (including every shape s29's frontier named) restore the target's addiu $a2,$sp,1040 position and close the three-row dst-cursor group.
- mechanism: s29's reading was that GCC sinks the dst cursor init past the guard because the cursor's first real use is inside the guarded block, so anything giving the cursor an earlier birth or an earlier real use should restore the target's position.
- probe: Ten bodies in tmp/grind/func_8003DE14/s30/v1/ swept on chassis_s29_targetmap_ordinary_8.c.
- result: Six are byte-identical to the 8 control - u16 *dst = &dst_buf[0], both cursors as &x[0], the guard rewritten as if (total <= 0) goto row_done, LoadImage(..., &dst_buf[0]), the cursors declared after factor, and the guard as while (total > 0) { ... break; }. Four are worse: if (total != 0) 9, dst declared inside the guard 10, j declared before complement 14, complement hoisted to per-row scope 22. The function-scope-cursor form collapses the loop to 124 insns and scores 108. Together with s29's 24 declaration-order permutations (all 8), the sink is insensitive to cursor-birth and guard-shape spelling on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 extender-free target-map chassis memory/grind/func_8003DE14/chassis_s29_targetmap_ordinary_8.c (8/173), zero FAKE constructs present

## [s31] The latch's two halfword loads are born in SOURCE OPERAND ORDER, so writing the exit test as `rect[3] * rect[2]` flips which load local-alloc prices higher and lands BOTH target seats (lh $v0,4 / lh $v1,6).
- mechanism: expand walks MULT_EXPR operand 0 first, so op0's load is the first insn of the latch block and the longer-span quantity; qty_compare_1 (local-alloc.c:1660) then prices the second-born load higher (2*6/1 vs 2*6/2) and find_free_reg's ascending scan gives IT $v0. s30 measured the inputs but read the operand swap as inert because it only ever compared total scores.
- probe: v04 (`} while (j < rect[3] * rect[2]);`) compiled, dumped (tmp/grind/func_8003DE14/dumps .combine/.lreg/.sched2) and classified; the .combine dump shows insn 300 = sign_extend of offset 6 and insn 304 = offset 4 (the reverse of the control), and .lreg shows 149 "used 6 times across 3 insns in block 11" / 152 "across 2 insns".
- result: The two `lh` REGISTER rows disappear - classify's residual drops from 5 rows to 3 (andi a1,v0 / mult v1,v0 / sra v0,v0) - but the multiply now reaches RTL as mult(off6, off4) and prints `mult $v1,$v0` where the target prints `mult $v0,$v1`, and the loads are emitted in the reverse of the target's order. Net score unchanged at 5. s30's "operand swap ties at 5" is TRUE but not inert: it is a DIFFERENT 5.
- verdict: CONFIRMED

## [s31] Staging the latch bound's rect[3] through a fresh local assigned at the bottom of the row loop (`h = rect[3];` then `} while (j < rect[2] * h);`) decouples birth order from multiply-operand order and drops the floor 5 -> 4.
- mechanism: the staged assignment emits rect[3]'s load as its own statement BEFORE the exit test, so h is the first-born (span 2) and the rect[2] load inside the test is the second-born (span 1); qty_compare_1 gives the rect[2] load $v0 (target) and h $v1 (target), while the multiply keeps its source operand order (rect[2], h) and prints `mult $v0,$v1` (target).
- probe: tmp/grind/func_8003DE14/s31/v2 + v3 sweeps (13 bodies) via tools/sweep_variants.py, then objdump of tmp/sandbox/func_8003DE14/code6cac_c2.o (tmp/grind/func_8003DE14/s31/ours.txt).
- result: w01 (`h` staged, `j < rect[2] * h`) = 4/173 and w03 (`w = rect[2]` staged, `j < rect[3] * w`) = 4/173; w02/x03 (the staged value born SECOND) = 5; x02 (both values staged, rect[3] first) = 4; x05 (h declared at row scope) = 4. The 4-point residual is the unchanged red group (`sra $v0,$v0,15` / `andi $a1,$v0,31`) plus the two `lh` rows, which now differ ONLY in ORDER - registers and multiply match the target.
- verdict: CONFIRMED

## [s31] The final emission order of the latch's two loads is the RTL birth order end-to-end - no pass between local-alloc and the object file reorders them - so the target's `lh $v0,4` FIRST with $v0 on the FIRST-born load cannot come from any birth-order spelling.
- mechanism: reading the passes instead of inferring them. sched.c's rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) breaks equal-priority ties by INSN_LUID (original order), and both loads feed the same mult with equal priority; .sched, .sched2 and .dbr all show the same order as .combine, the emitted .s carries it, and the object file agrees. (s31's first reading that "something swaps the loads" came from treating inverse_compose classify's SORTED row list as positional - it is not.)
- probe: dumps captured for the control body and for v04 (tmp/grind/func_8003DE14/dumps/*), plus objdump of each measured body.
- result: control: .combine/.sched/.sched2/.dbr/.s all = (off4, off6) and the object = (off4 in $v1, off6 in $v0); v04 and w01: every stage = (off6, off4) and the object agrees. Order is preserved by every pass. Therefore the target's latch has its FIRST-born load in $v0, which qty_compare_1 cannot produce for two equal-refs, equal-size, block-local quantities dying at the same insn - one of the two loads must escape local-alloc (local-alloc.c:470-476) and be seated by global.c afterwards.
- verdict: CONFIRMED

## [s31] The remaining two latch rows (load ORDER) can be closed by giving the rect[3] latch pseudo a reference outside the latch block while its load stays inside it.
- mechanism: local-alloc.c:470-476 skips any pseudo with reg_basic_block < 0 (used in >1 block) or reg_n_deaths > 1; the skipped pseudo is seated by global.c AFTER local-alloc has already given the block's remaining quantity $v0. With the rect[3] load skipped, the rect[2] load is the only block-local quantity in block 11 and takes $v0 even when born first - the target's shape.
- probe: x01 (stage `h = rect[3]` at the TOP of the inner do-body, so the pseudo spans blocks) and x04 (stage h at the bottom but consume it in the row epilogue's new_y, with a row-top initialisation so it is always defined), swept on the 4-point chassis.
- result: Both fail, for different reasons. x01 = 10/173: making the pseudo cross-block also MOVES its load out of the latch block (the target's load is in the latch), costing 6 points. x04 = 115/69 insns: the row-top initialisation makes the inner assignment loop-invariant, loop.c hoists it and the inner loop collapses. Every other s31 shape (x02/x03/x05, v01-v10) leaves both loads block-local.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender + s31 h stage present

## [s31] Linking the latch's halfword loads to the row epilogue's reads by spelling the epilogue's `((u16 *)rect)[N]` accesses as signed `rect[N]` makes the latch pseudos cross-block (CSE propagating the latch value into the epilogue).
- mechanism: CSE in 2.7.2 works on extended basic blocks; if the epilogue continued the latch's path, the epilogue's rect[2]/rect[3] reads would reuse the latch pseudos, making them non-block-local and pushing them to global.c.
- probe: v01 (epilogue rect[3] signed), v02 (epilogue rect[2] signed), v03 (both) swept on the 5-point chassis.
- result: v01 = 6/173 - the epilogue's `lhu $v1,6($s0)` becomes `lh` (one new row) and NOTHING else changes, so no value was shared; v02 = 5/173 byte-identical to the control (combine keeps the zero-extending load because only the low half is consumed); v03 = 6. The row-loop epilogue block has TWO predecessors (the guard's `blez` at 8003DF28 branches straight to it), so it is never a continuation of the latch's extended basic block and CSE cannot propagate across that edge.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis memory/grind/func_8003DE14/candidate.c at 5/173, s21 j chain extender present

## [s31] Re-spelling the row loop as a `for` statement, or routing the latch's rect[] reads through a local `s16 *` alias, reaches the same latch RTL as the do-while chassis.
- mechanism: structural-modality sweep of the loop's chassis-level spellings (declaration order / statement re-association), on the theory that the exit-test expansion might differ.
- probe: w05/w06 (for-loop, both operand orders) and v08 (local `s16 *rp = rect;` used only by the latch), swept on the 5- and 4-point chassis.
- result: Both for-loop forms = 69 at 174 insns (loop.c emits a separate top test, so the guard and the latch both change shape), and the pointer alias = 22 at 174 insns (the alias keeps `rect` live and adds an insn). Neither is near the target's shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (5/173) and s31 staged-h chassis (4/173), s21 j chain extender present

## [s31] The latch's two halfword loads are born in SOURCE OPERAND ORDER, so writing the exit test as `rect[3] * rect[2]` flips which load local-alloc prices higher and lands BOTH target seats (lh $v0,4 / lh $v1,6).
- mechanism: expand walks MULT_EXPR operand 0 first, so op0's load is the first insn of the latch block and the longer-span quantity; qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) prices the second-born load higher (equal refs 6, equal size, spans 2 vs 1 -> 60000 vs 120000) and find_free_reg's ascending scan hands IT $v0.
- probe: v04 (`} while (j < rect[3] * rect[2]);`) compiled and dumped (tmp/grind/func_8003DE14/dumps .combine/.lreg/.sched2), then classified with tools/ra_solver/inverse_compose.py.
- result: The .combine dump shows insn 300 = sign_extend of offset 6 and insn 304 = offset 4 - the reverse of the control - and both `lh` REGISTER rows disappear (classify residual 5 rows -> 3). The multiply then reaches RTL as mult(off6, off4) and prints `mult $v1,$v0` where the target prints `mult $v0,$v1`, so the net score stays 5. s30's banked 'operand swap ties at 5' is true but it is a DIFFERENT 5.
- verdict: CONFIRMED

## [s31] Staging the latch bound's rect[3] through a fresh local assigned at the bottom of the row loop (`h = rect[3];` then `} while (j < rect[2] * h);`) decouples birth order from multiply-operand order and drops the floor 5 -> 4.
- mechanism: the staged assignment emits rect[3]'s load as its own statement BEFORE the exit test, so h is the first-born (span 2) and the rect[2] load inside the test is the second-born (span 1); qty_compare_1 gives the rect[2] load $v0 (target) and h $v1 (target), while the multiply keeps its source operand order (rect[2], h) and prints `mult $v0,$v1` (target).
- probe: 13 bodies swept with tools/sweep_variants.py (tmp/grind/func_8003DE14/s31/v2, v3), each measured with `sandbox func_8003DE14 --disable all` and read back from objdump of tmp/sandbox/func_8003DE14/code6cac_c2.o.
- result: w01 (`h` staged, `j < rect[2] * h`) = 4/173 and w03 (`w = rect[2]` staged, `j < rect[3] * w`) = 4/173; w02/x03 (the staged value born SECOND) = 5; x02 (both staged, rect[3] first) = 4; x05 (h declared at row scope) = 4. Residual is now the unchanged red group plus the two `lh` rows, which differ ONLY in order.
- verdict: CONFIRMED

## [s31] The final emission order of the latch's two loads is the RTL birth order end to end - no pass between local-alloc and the object file reorders them - so the target's `lh $v0,4` first with $v0 on the FIRST-born load does not come from any birth-order spelling.
- mechanism: sched.c's rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) breaks the two loads' equal-priority tie by INSN_LUID, i.e. original order, and gas does not reorder; so birth order, emission order and the qty_compare_1 verdict are welded together.
- probe: dumps captured for the control body and for v04 (.combine, .sched, .sched2, .dbr, .s in tmp/grind/func_8003DE14/dumps/), compared against objdump of the sandbox object for each body.
- result: control: every stage = (off4, off6) and the object = off4 in $v1 / off6 in $v0; v04 and w01: every stage = (off6, off4) and the object agrees. Consequence: the target's latch has its first-born load in $v0, which qty_compare_1 cannot produce for two equal-refs, equal-size, block-local quantities dying at the same insn - one load must escape local-alloc (local-alloc.c:470-476) and be seated by global.c afterwards. METHOD NOTE: inverse_compose classify prints its row list SORTED, not in program order; reading order out of it produced a wrong 'a pass swaps the loads' theory this session.
- verdict: CONFIRMED

## [s31] The two remaining latch rows can be closed by giving the rect[3] latch pseudo a reference outside the latch block while its load stays inside it (staging it at the top of the inner do-body, or consuming the staged value in the row epilogue).
- mechanism: local-alloc.c:470-476 skips any pseudo with reg_basic_block < 0 or reg_n_deaths > 1; the skipped pseudo is seated by global.c AFTER local-alloc has given the block's surviving quantity $v0, so the rect[2] load would take $v0 even when born first.
- probe: x01 (stage `h = rect[3]` at the TOP of the inner do-body) and x04 (stage h at the bottom, consume it in the row epilogue's new_y with a row-top initialisation so it is always defined), swept on the 4-point chassis.
- result: x01 = 10/173 - making the pseudo cross-block also MOVES its load out of the latch block, and the target's load is in the latch (6 points). x04 = 115 at 69 insns - the row-top initialisation is loop-invariant, loop.c hoists it and the inner loop collapses. Both forms are banked in memory/grind/func_8003DE14/rejected/.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender + s31 h stage present

## [s31] Spelling the row epilogue's `((u16 *)rect)[N]` reads as signed `rect[N]` makes CSE share the latch's halfword pseudos, turning them into cross-block (global) quantities.
- mechanism: CSE in 2.7.2 works on extended basic blocks; if the epilogue continued the latch's path, its rect[2]/rect[3] reads would reuse the latch pseudos and push them out of local-alloc.
- probe: v01 (epilogue rect[3] signed), v02 (epilogue rect[2] signed), v03 (both), swept on the 5-point chassis.
- result: v01 = 6/173 - the epilogue's `lhu $v1,6($s0)` becomes `lh` (one new row) and nothing else changes, so no value was shared; v02 = 5/173 byte-identical to the control (combine keeps the zero-extending load because only the low half is consumed); v03 = 6. The row-loop epilogue block has TWO predecessors (the guard's `blez` at 8003DF28 branches straight to it), so it never continues the latch's extended basic block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis memory/grind/func_8003DE14/candidate.c at 5/173, s21 j chain extender present

## [s31] Re-spelling the row loop as a `for` statement, or routing the latch's rect[] reads through a local `s16 *` alias, reaches the same latch RTL as the do-while chassis.
- mechanism: structural-modality sweep of the loop's chassis-level spellings (declaration order, statement re-association), on the theory that the exit-test expansion might differ.
- probe: w05/w06 (for-loop, both operand orders) and v08 (local `s16 *rp = rect;` used only by the latch), swept on the 5- and 4-point chassis.
- result: Both for-loop forms = 69 at 174 insns (loop.c emits a separate top test, changing both the guard and the latch); the pointer alias = 22 at 174 insns (the alias keeps `rect` live and adds an insn). Neither is near the target's shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis (5/173) and s31 staged-h chassis (4/173), s21 j chain extender present

## [s31] Swapping the ROW-TOP `total = rect[2] * rect[3]` operands moves the latch's seats too, because both blocks read the same expression.
- mechanism: if CSE or a canonical ordering linked the two computations, the row top's operand order would fix the latch's as well.
- probe: v05 (row top swapped only) and v06 (row top and latch both swapped), swept on the 5-point chassis.
- result: v05 = 7 and v06 = 7, both at 173 insns: the row-top swap costs exactly 2 rows of its own and the latch rows are unchanged by it. The two blocks' load orders are independent, which is why the latch had to be attacked in its own statement.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s29 target-map chassis memory/grind/func_8003DE14/candidate.c at 5/173, s21 j chain extender present

## [s32] The two remaining latch rows are an RA/birth-order fact that no C spelling can reach (s31's standing attribution).
- mechanism: s31 read .combine/.sched/.sched2/.dbr for two bodies and concluded emission order == RTL birth order at every stage, so qty_compare_1 (local-alloc.c:1660) welds order to seats.
- probe: body c04 (`w = rect[2]; total = rect[3];` staged in TARGET order, `j < w * total`) compiled with -da; .combine shows insn 298 = offset 4 then insn 303 = offset 6, .lreg (after the first scheduling pass) shows 303 -> 298 -> 308.
- result: DISPROVEN. The first scheduling pass (tools/gcc-2.7.2/sched.c) reorders the two loads; s31's claim held only for the bodies it compiled, in which RTL order already matched sched's preference. Order is decided by whether the destination pseudo is block-local, not by statement position.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender + s31 h stage present

## [s32] Giving the latch's rect[3] pseudo a reference in a SECOND basic block, with both references inside the inner loop, makes it escape local-alloc and lands the target's latch exactly (order + seats + multiply).
- mechanism: local-alloc.c:470-476 skips any pseudo with reg_basic_block < 0, so local-alloc seats only the block-local rect[2] load - which takes $v0, the target's seat - and global.c seats the escapee afterwards ($v1). The first scheduling pass emits the escaped pseudo's load FIRST, giving the target's `lh $v0,4($s0)` / `lh $v1,6($s0)` / `mult $v0,$v1`. Keeping the second reference INSIDE the loop is what avoids the two s31 failure modes: the load never leaves the latch block, and there is no loop-invariant initialiser for loop.c to hoist.
- probe: 33 bodies in eight sweeps (tmp/grind/func_8003DE14/s32/v1..v8) via tools/sweep_variants.py, each read back positionally with tmp/grind/func_8003DE14/s32/rowdiff.py.
- result: e05 - `h = target_color; *dst++ = h;` in the fast arm plus `} while (j < rect[2] * (h = rect[3]));` - measures 2/173, and the entire latch group (both `lh` rows and the `mult` row) is target-identical; the only residual is the red shift pair. Nearby spellings: e04 (operands reversed) 4, e06 5, e01/e02/e03 6, h01 (borrowing the existing `total` instead of a fresh `h`) 4, h02 (staged in the zero-pixel arm) 3.
- verdict: CONFIRMED

## [s32] An inline assignment inside the exit test (`j < rect[2] * (total = rect[3])`) can be used to make an operand born SECOND.
- mechanism: the hope was that expand walks MULT_EXPR operand 0 first, so the plain read would be emitted before the assignment.
- probe: b01/b05/b06/c05/c06 (single inline assignment in each operand position) plus c01/c03 (both operands inline-assigned), swept on the 4-point chassis.
- result: KILLED. The operand carrying the side effect is always evaluated first: b01 (`rect[2] * (total = rect[3])`) emits the offset-6 load first and is byte-identical to the two-statement staging (4). Two inline assignments do evaluate left to right (c01 = 4, c03 = 5), so the construct is only useful for choosing WHICH pseudo escapes, not for ordering.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis, s21 j chain extender present

## [s32] Making the rect[2] operand the escaping pseudo (`(total = rect[2]) * rect[3]`) lands the target's latch.
- mechanism: symmetric application of the escape lever - if the escapee's load is emitted first, escaping rect[2] should give the target's `lh ...,4($s0)` first.
- probe: c02 and c06 on the 4-point chassis, read positionally with rowdiff.py.
- result: KILLED at 5/173 both. The order IS the target's (off4 first), but the registers invert: the escapee is seated by global.c AFTER local-alloc has given $v0 to the surviving block-local quantity, so off4 gets $v1 and the multiply prints `mult $v1,$v0` - three wrong rows instead of two. The escape must be applied to the rect[3] operand.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender present

## [s32] The latch pseudo can be given its second-block reference from the row loop's epilogue or row top.
- mechanism: any second reference makes the pseudo non-block-local; the row top and the epilogue are the two cheapest sites outside the inner loop.
- probe: a01/a02/a04 (row-top carrier), b02 (row-top via the guard), d01-d06 (epilogue carrier), swept on the 4-point chassis.
- result: KILLED. Row top: a01 = 7 - the row-top pair flips the same way the latch does, costing 3 rows there; a02 = 124 at 68 insns and b02 = 123 at 64 insns (loop collapse). Epilogue: every d-form is 27-28 at 163 insns, because a row-scope carrier read after the loop lets loop.c/CSE delete the latch load entirely; even the bare epilogue stage costs 2 rows (d04 = 6). The second reference must sit INSIDE the inner loop.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis, s21 j chain extender present

## [s32] The red group's `sra` seat moves if the dying `sum` (or the shift's destination) escapes local-alloc, the way the latch's load did.
- mechanism: combine_regs (local-alloc.c:1854-1897) ties the shift's destination quantity to its dying source; if the source or the destination is not a local-alloc quantity at all, the tie cannot be formed and the destination should fall to global.c ($a1 in the target).
- probe: on the new 2-point chassis, g00-g03 (hoist `sum`'s declaration; make `sum` cross-block via the blend path's zero-pixel arm, via the fast arm's zero branch; make `rp` cross-block) and f01-f05 (route the shift's destination or the whole red chain through the cross-block `h` / `total`).
- result: KILLED. Hoisting `sum`'s declaration to the guard block is free (2), but every escape spelling costs: g01/g02/g03 = 3 and the `sra` still writes $v0 in each, so the tie survives the escape; routing the destination costs 16-35 (f02 16, f01/f04 20, f05 35) and the red-sum-in-`total` form is 5. The red pair is not reachable through the block-locality lever.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s32 2-point chassis memory/grind/func_8003DE14/candidate.c (s21 j chain extender + s32 `h` staging local present)

## [s32] The two remaining latch rows are welded to RTL birth order, because no pass between combine and the object file reorders the two halfword loads (s31's standing attribution).
- mechanism: s31 compared .combine/.sched/.sched2/.dbr/.s for two bodies and concluded order is preserved end to end, so qty_compare_1 (local-alloc.c:1660) welds emission order to the seats.
- probe: Body c04 (`w = rect[2]; total = rect[3];` staged as two statements in TARGET order, then `j < w * total`) compiled with the instrumented cc1 -da; read .combine and .lreg positionally.
- result: DISPROVEN. .combine has insn 298 = sign_extend of offset 4 then insn 303 = offset 6 (target order); .lreg - the next dump, after the first scheduling pass - has 303 -> 298 -> 308. sched reorders them. s31's claim held only for bodies whose RTL order already matched sched's preference.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender + s31 h stage present

## [s32] Giving the latch's rect[3] pseudo a reference in a second basic block, with both references inside the inner loop, makes it escape local-alloc and lands the target's latch (order, seats and multiply).
- mechanism: local-alloc.c:470-476 skips any pseudo with reg_basic_block < 0; local-alloc then seats only the block-local rect[2] load, which takes $v0 (the target's seat), and global.c seats the escapee afterwards ($v1). The first scheduling pass emits the escaped pseudo's load FIRST, producing the target's `lh $v0,4($s0)` / `lh $v1,6($s0)` / `mult $v0,$v1`. Keeping the second reference inside the loop avoids both s31 failure modes: the load never leaves the latch block and there is no loop-invariant initialiser for loop.c to hoist.
- probe: 33 bodies in eight sweeps (tmp/grind/func_8003DE14/s32/v1..v8) with tools/sweep_variants.py, each read back positionally with the new tmp/grind/func_8003DE14/s32/rowdiff.py.
- result: CONFIRMED at 2/173: `h = target_color; *dst++ = h;` in the fast arm plus `} while (j < rect[2] * (h = rect[3]));` in the latch. The whole latch group is target-identical; the only residual is the red shift pair. Nearby spellings: operands reversed 4, rect[2] also staged 5-6, borrowing the existing `total` instead of a fresh `h` 4, staging in the zero-pixel arm 3.
- verdict: CONFIRMED

## [s32] An inline assignment inside the exit test (`j < rect[2] * (total = rect[3])`) makes that operand born second, so it can be used to choose emission order.
- mechanism: expand walks MULT_EXPR operand 0 first, so a plain read written first should be emitted before an assignment written second.
- probe: b01/b05/b06/c05/c06 (one inline assignment per operand position) and c01/c03 (both operands inline-assigned), swept on the 4-point chassis and read positionally.
- result: KILLED. The operand carrying the side effect is evaluated first: b01 emits the offset-6 load first and is byte-identical to two-statement staging (4). Two inline assignments do evaluate left to right (c01 = 4, c03 = 5), so the construct only chooses WHICH pseudo escapes, never the order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender present

## [s32] Applying the escape to the rect[2] operand instead (`(total = rect[2]) * rect[3]`) lands the target's latch.
- mechanism: Symmetric use of the same lever - the escapee's load is emitted first, and the target emits the offset-4 load first.
- probe: c02 and c06 on the 4-point chassis, read positionally with rowdiff.py.
- result: KILLED at 5/173 both. The ORDER is the target's, but the registers invert: the escapee is seated by global.c after local-alloc has already given $v0 to the surviving block-local quantity, so offset 4 gets $v1 and the multiply prints `mult $v1,$v0` - three wrong rows instead of two.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender present

## [s32] The latch pseudo's second-block reference can come from the row top or from the row-loop epilogue.
- mechanism: Any second reference makes the pseudo non-block-local; the row top and the epilogue are the cheapest sites outside the inner loop.
- probe: a01/a02/a04 (row-top carrier), b02 (row-top consumed by the guard), d01-d06 (epilogue carrier), swept on the 4-point chassis.
- result: KILLED. Row top: a01 = 7 (the row-top load pair flips the same way, costing 3 rows there while the latch is unchanged), a02 = 124 at 68 insns, b02 = 123 at 64 insns (loop collapse). Epilogue: every d-form is 27-28 at 163 insns because a row-scope carrier read after the loop lets loop.c/CSE delete the latch load; the bare epilogue stage alone costs 2 rows (d04 = 6). The second reference has to sit inside the inner loop.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s31 4-point chassis memory/grind/func_8003DE14/candidate.c, s21 j chain extender present

## [s32] The red group's `sra` seat moves if the dying `sum`, the `rp` product or the shift's own destination escapes local-alloc the way the latch load did.
- mechanism: combine_regs (local-alloc.c:1854-1897) ties the shift's destination quantity to its dying source; if either end is not a local-alloc quantity, the tie cannot form and the destination should fall to global.c ($a1 in the target).
- probe: On the new 2-point chassis: g00-g03 (hoist `sum`'s declaration; make `sum` cross-block via the blend path's zero-pixel arm and via the fast arm's zero branch; make `rp` cross-block) and f01-f05 (route the shift's destination or the whole red chain through the cross-block `h` / `total`).
- result: KILLED. Hoisting `sum`'s declaration to the guard block is free (still 2), but g01/g02/g03 all cost 1 (= 3) and in every one the `sra` still writes $v0, so the tie survives the escape; routing the destination costs far more (f02 16, f01/f04 20, f05 35) and carrying the red sum in `total` is 5.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s32 2-point chassis memory/grind/func_8003DE14/candidate.c (s21 j chain extender + s32 `h` staging local present)

## [s33] The red shift pair is not a combine_regs tie to be broken but an EXPAND-LEVEL artefact of writing the shift and its mask as one expression; splitting `r_src = (sum >> 15) & 0x1F;` into two statements makes the shift's destination the already-multi-set `r_src` pseudo and prints the target's two-register form.
- mechanism: expand creates a fresh single-set temp for the inner `sum >> 15`, and combine_regs (local-alloc.c:1854-1897) ties THAT temp to the dying `sum` ($v0), so the shift prints `sra $v0,$v0,15` and the mask lands in r_src's seat. If the shift's destination is written directly to `r_src` - a pseudo already set twice in the block (the sll and the mflo) - combine_regs cannot form the tie and both insns print into r_src's own seat, which is the target's `sra $a1,$v0,15` / `andi $a1,$a1,0x1F`. The GREEN channel has carried this exact shape since s29 (`g_src = sum >> 10;` with the mask deferred), which is why green was already byte-exact while red was not.
- probe: tmp/grind/func_8003DE14/s33/v1/{r1,r2,r3,r4}.c on the s32 2-point chassis, read positionally with the s32 rowdiff.py.
- result: CONFIRMED for the SHAPE. r2 (`r_src = sum >> 15; r_src = r_src & 0x1F;`) prints `sra $a0,$v0,15` / `andi $a0,$a0,31` - the target's two-register form - and the WHOLE remaining residual collapses to one clean $a0<->$a1 swap between `px` and `r_src`. r3 (`r_src &= 0x1F;`) is byte-identical to r2. Moving the mask into the OR chain instead (r1/r4, `| (r_src & 0x1F)`) is 14 and does NOT produce the form. Score of r2 alone is 17 (the swap), banked as rejected/s33-red-split-alone-px-rsrc-a0a1-swap-17.c.
- verdict: CONFIRMED

## [s33] The $a0/$a1 swap the red split causes is a global.c allocno-priority inversion with a 2.8% margin, and handing `px` one more real reference (or `r_src` one more unit of live length) reverses it.
- mechanism: global.c orders allocnos by reg_n_refs/live_length. The split adds two references to r_src (6 -> 8 refs, weighted 18 -> 24 at loop depth 3) and nothing to px, so r_src overtakes px and takes $a0 first.
- probe: BB2_ALLOC_DEBUG=1 via tools/gcc-2.7.2/cc1 on the r2 body and on the d1 body (tmp/grind/func_8003DE14/s33/alloc.sh, log at tmp/grind/func_8003DE14/s33/alloc.log).
- result: CONFIRMED with exact numbers. r2: px = pseudo 123 nrefs=30 livelen=27 pri=44444 -> hardreg 5 ($a1, wrong); r_src = pseudo 124 nrefs=24 livelen=21 pri=45714 -> hardreg 4 ($a0, wrong). The printed priorities fit nrefs*40000/livelen exactly for both, so the thresholds are arithmetic: px wins at nrefs>=31 (i.e. +1 source reference, weighted +3) or livelen<=26, and r_src loses at livelen>=22. Reference counting confirmed against the target itself: the target's $a0 has 10 refs and its $a1 has 8, matching our px/r_src exactly - so the target's advantage for px is a LIVE-LENGTH difference, not a reference-count difference.
- verdict: CONFIRMED

## [s33] Spending px's extra reference on the red channel's source read (`(px & 0x1F) << 3` instead of `(pixel & 0x1F) << 3`) restores the seats and drops the floor to 1.
- mechanism: px and pixel hold the same value at that point, so the read is ordinary C; it moves one reference from pixel to px, taking px to nrefs=33 / pri=61111, far above r_src's 45714.
- probe: tmp/grind/func_8003DE14/s33/v5/d1.c, scored and read positionally.
- result: CONFIRMED at 1/173. Every row of the function is byte-exact except target `andi $v0,$t0,0x1F` vs our `andi $v0,$a0,0x1F` - the red source is read out of $a0 (px) where the target reads $t0 (pixel). This is memory/grind/func_8003DE14/candidate.c.
- verdict: CONFIRMED

## [s33] The same reference can be bought by splitting px's birth (`s32 px = pixel; px = px & 0xFFFF;`), which also reaches 1 but loses the target's redundant birth mask.
- mechanism: the split gives px two extra references without touching the red source read, so `(pixel & 0x1F) << 3` is preserved.
- probe: tmp/grind/func_8003DE14/s33/v3/b1.c plus the fold-resistance variants v8/{i2,i3,i5}.c.
- result: CONFIRMED at 1/173, banked as memory/grind/func_8003DE14/chassis_s33_pxsplit_1.c. Its single wrong row is the mirror of the candidate's: combine proves `& 0xFFFF` redundant against the lhu's nonzero_bits and folds the copy+and into `move $a0,$t0`, where the target keeps `andi $a0,$t0,0xFFFF`. i2 (`s32 px = *src;`) and i3 (`(s32)pixel`) fold identically; i5 (`px = pixel & 0xFFFF; px = px & 0xFFFF;`) also reaches 1 but is a dead self-assign and is NOT a usable form.
- verdict: CONFIRMED

## [s33] Other sites for px's extra reference, and the alternative of lengthening r_src's live range, all cost more than they buy.
- mechanism: any of the levers in the arithmetic above should flip the seats; the question is whether the lever is byte-free.
- probe: 33 bodies in eight sweeps (tmp/grind/func_8003DE14/s33/v1..v8) with tools/sweep_variants.py, each candidate re-read positionally with the s32 rowdiff.py.
- result: KILLED for every site except the two banked ones. Splitting the BLUE source (`px = (u32)px >> 7; px = px & 0xF8;`, v2/a1) flips the seats and reaches 2, but the target's blue srl writes a temp ($v0) so the split breaks that pair. The alpha mask on px (`(px & 0x8000)`, d2/d6) costs 25 at 175 insns. The zero-arm store `*dst = px` is 18. Staging the blue sum (`sum = px; sum = sum + b*factor;`, d7 / b2) is 17-33. Splitting the blue final mask (b3) is 3. A redundant `& 0xFFFF` inside the guard test (i4) or inside the green source (i7) is folded away and buys nothing (17, unchanged). On r_src's side: born before the zero guard (e1) is 3 - it flips the seats but denies reorg.c the `andi $v0,$t0,0x1F` delay-slot filler the target uses; inlining `r * factor` into the addu (c1) is 26; decl-order and rp/gp-order permutations (c3/c4/c6) and the green-mask split (c7) are all 17 (no flip); carrying the masked red result in `rp` (c8) is 24. Channel-block reorderings (c5, e6, e7) are 24-31.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s33 red-split chassis (tmp/grind/func_8003DE14/s33/v1/r2.c, 17/173), s21 j chain extender + s32 `h` staging local present

## [s33] Re-associating or re-ordering the final OR chain can shorten px's live range enough to flip the seats without spending a reference.
- mechanism: px's live range ends at its mask in the OR chain and r_src's ends at its `or`; changing the association or the operand order moves those endpoints, and the arithmetic above needs only one unit.
- probe: v6/{e2,e3,e4,e5} (operand reorderings and one re-association) and v7/{f1..f7} (all seven parenthesisations of the four-term chain) on the red-split chassis.
- result: KILLED. Re-association DOES flip the seats - e5 (`(A|r) | (G|P)`) and the identical f7 (`A | r | (G|P)`) both reach 2 - but it re-emits the tail of the chain: our `andi $a0,$a0,31744` / `or $v1,$v1,$a0` against the target's `or $v0,$v0,$v1` / `andi $v1,$a0,31744`, two wrong rows, no better than s32's floor. Every other parenthesisation is 28-40, and every operand REORDERING is 24-27. Banked as rejected/s33-or-chain-reassoc-tail-rows-2.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s33 red-split chassis (17/173), s21 j chain extender + s32 `h` staging local present

## [s33] The red shift pair is an expand-level artefact of writing the shift and its mask as one expression; splitting `r_src = (sum >> 15) & 0x1F;` into `r_src = sum >> 15; r_src = r_src & 0x1F;` makes the shift's destination the already-multi-set r_src pseudo and prints the target's two-register form.
- mechanism: The single expression makes expand allocate a fresh single-set temp for the inner shift, and combine_regs (local-alloc.c:1854-1897) ties THAT temp to the dying three-way `sum` ($v0), so the shift prints `sra $v0,$v0,15`. Writing the shift straight into `r_src` - a pseudo already set twice in the block, by the sll and by the mflo - leaves no tie-eligible destination, so both insns print into r_src's own seat. Our GREEN channel has carried exactly this shape since s29 (`g_src = sum >> 10;`, mask deferred) and has been byte-exact all along, which is the evidence that pointed at the spelling rather than at the tie.
- probe: tmp/grind/func_8003DE14/s33/v1/{r1,r2,r3,r4}.c on the s32 2-point chassis, scored with tools/sweep_variants.py and read positionally with tmp/grind/func_8003DE14/s32/rowdiff.py.
- result: CONFIRMED for the shape. r2 prints `sra $a0,$v0,15` / `andi $a0,$a0,31` - the target's two-register form - and the entire remaining residual collapses to one clean $a0<->$a1 swap between px and r_src. r3 (`r_src &= 0x1F;`) is byte-identical. Moving the mask into the OR chain instead (r1/r4) is 14 and does not produce the form. This supersedes the s29-s32 reading of the red pair as a combine_regs tie to be broken.
- verdict: CONFIRMED

## [s33] The $a0/$a1 swap the red split causes is a global.c allocno-priority inversion with a measured 2.8% margin, and one more px reference (or one more unit of r_src live length) reverses it.
- mechanism: global.c orders allocnos by reg_n_refs/live_length. The split takes r_src from 6 to 8 references (weighted 18 -> 24 at loop depth 3) and gives px nothing, so r_src overtakes px and takes $a0 first.
- probe: BB2_ALLOC_DEBUG=1 through the instrumented tools/gcc-2.7.2/cc1 (tmp/grind/func_8003DE14/s33/alloc.sh) on the red-split body and on the winning body; log at tmp/grind/func_8003DE14/s33/alloc.log.
- result: CONFIRMED with exact numbers. Red-split chassis: px = pseudo 123 nrefs=30 livelen=27 pri=44444 -> hardreg 5 ($a1, wrong); r_src = pseudo 124 nrefs=24 livelen=21 pri=45714 -> hardreg 4 ($a0, wrong). Both printed priorities fit nrefs*40000/livelen exactly, so the thresholds are arithmetic: px wins at nrefs>=31 (one more source reference, weighted +3) or livelen<=26; r_src loses at livelen>=22. Counting the target's own asm, its $a0 carries 10 references and its $a1 carries 8 - identical to our counts - so the target's advantage for px is a LIVE-LENGTH difference of at least one insn, not a reference-count difference.
- verdict: CONFIRMED

## [s33] Spending px's extra reference on the red channel's source read (`(px & 0x1F) << 3` instead of `(pixel & 0x1F) << 3`) restores the seats and drops the floor to 1.
- mechanism: px and pixel hold the same value at that point, so the read is ordinary C; it moves one reference from pixel to px, taking px to nrefs=33 / pri=61111, far above r_src's 45714.
- probe: tmp/grind/func_8003DE14/s33/v5/d1.c, scored with tools/sweep_variants.py and read positionally with rowdiff.py; re-measured after banking as memory/grind/func_8003DE14/candidate.c.
- result: CONFIRMED at 1/173 build insns, 173 target insns. Every row of the function is byte-exact except target `andi $v0,$t0,0x1F` vs our `andi $v0,$a0,0x1F` - the red source is read out of $a0 (px) where the target reads $t0 (pixel). This is the new candidate.c and it adds no FAKE construct (the two carried over from s21/s32 are unchanged).
- verdict: CONFIRMED

## [s33] The same reference can be bought by splitting px's birth (`s32 px = pixel; px = px & 0xFFFF;`), which also reaches 1 while keeping the red source read on `pixel`.
- mechanism: The split gives px two extra references without touching the red source read.
- probe: tmp/grind/func_8003DE14/s33/v3/b1.c plus the fold-resistance variants v8/{i2,i3,i5}.c.
- result: CONFIRMED at 1/173, banked as memory/grind/func_8003DE14/chassis_s33_pxsplit_1.c. Its single wrong row is the mirror image of the candidate's: combine proves the 0xFFFF mask redundant against the lhu's nonzero_bits and folds copy+and into `move $a0,$t0` where the target keeps `andi $a0,$t0,0xFFFF`. i2 (`s32 px = *src;`) and i3 (`(s32)pixel`) fold identically; i5 (`px = pixel & 0xFFFF; px = px & 0xFFFF;`) also reaches 1 but is a dead self-assign and is not a usable form.
- verdict: CONFIRMED

## [s33] Every other site for px's extra reference, and every tried way of lengthening r_src's live range, costs more rows than it buys on this chassis.
- mechanism: Any of the levers in the measured priority arithmetic should flip the seats; the question is whether the lever is byte-free.
- probe: 33 bodies in eight sweeps (tmp/grind/func_8003DE14/s33/v1..v8) via tools/sweep_variants.py, each re-read positionally with rowdiff.py.
- result: KILLED for every site except the two banked ones. px sites: blue source split 2 (flips the seats but breaks the blue srl/andi pair, whose target srl writes a temp), blue final-mask split 3, blue sum staging 17-33, zero-arm `*dst = px` 18, alpha mask on px 25 at 175 insns, redundant `& 0xFFFF` inside the guard test (i4) or the green source (i7) 17 - both folded away, buying no reference. r_src sites: born before the zero guard 3 (flips the seats but denies reorg.c the `andi $v0,$t0,0x1F` delay-slot filler the target uses), `r*factor` inlined into the addu 26, decl-order and rp/gp-order permutations and the green-mask split 17 (no flip), masked red result carried in `rp` 24, channel-block reorderings 24-31.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); s33 red-split chassis tmp/grind/func_8003DE14/s33/v1/r2.c at 17/173, with the s21 j chain extender and the s32 `h` staging local present

## [s33] Re-associating or re-ordering the final OR chain shortens px's live range enough to flip the seats without spending a reference, but re-emits the chain tail.
- mechanism: px's live range ends at its mask in the OR chain and r_src's ends at its `or`; changing association or operand order moves those endpoints, and the measured arithmetic needs only one unit.
- probe: tmp/grind/func_8003DE14/s33/v6/{e2,e3,e4,e5}.c (operand reorderings plus one re-association) and v7/{f1..f7}.c (all seven parenthesisations of the four-term chain) on the red-split chassis.
- result: KILLED. Re-association does flip the seats - e5 (`(A|r) | (G|P)`) and the identical f7 both reach 2 - but the chain tail comes out as our `andi $a0,$a0,31744` / `or $v1,$v1,$a0` against the target's `or $v0,$v0,$v1` / `andi $v1,$a0,31744`, two wrong rows, no better than the s32 floor. Every other parenthesisation is 28-40 and every operand reordering is 24-27.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; s33 red-split chassis at 17/173, with the s21 j chain extender and the s32 `h` staging local present

## s34 (REDERIVE) hypotheses

### H34.1 - KILLED (instance)
**Statement.** On the s33 red-split chassis with the red source read on `pixel`,
moving one existing statement inside the blend block (src++ resited, the green
source read moved after the red product, r_src declared after g_src, the blue
source read hoisted, `sum` hoisted into the guard block, all three source reads
computed up front) changes the pre-allocation live length of `r_src` or `px`.
**Mechanism.** global.c priority = nrefs*40000/live_length; live_length is the
REG_LIVE_LENGTH that flow.c recomputes after the first scheduling pass.
**Probe.** Eight reorderings generated (tmp/grind/func_8003DE14/s34/v5/g1..g8),
each scored and each read through BB2_ALLOC_DEBUG (tmp/.../s34/al.sh).
**Result.** g1/g3/g7 broke semantics or the stream (93-96 at 115-119 insns).
g5/g6/g8 are byte-neutral at 17/173 and their ALLOCDBG rows are numerically
identical to the unmoved body: px nrefs=30 livelen=27 pri=44444, r_src nrefs=24
livelen=21 pri=45714, g_src nrefs=18 livelen=18 pri=40000.  Not one unit moved.
The first scheduling pass canonicalises the block before flow.c recounts, so
statement position is inert for live length exactly as s28 found it inert for
emission order.  **This kills s33's frontier probe 1 as written.**
**measured_on.** HEAD 2026-09-11 (post -mel, post -msoft-float); s34/v2/b8.c
chassis (s33 red split + red source read on `pixel`), with the s21 j chain
extender and the s32 `h` staging local present.

### H34.2 - KILLED (instance)
**Statement.** Shortening `px`'s live range (target livelen <= 26 at nrefs 30)
by giving the blue channel its own carrier, or by removing the `px` variable
entirely, wins px the $a0 seat.
**Mechanism.** px loses the seat to r_src by 1270 points (2.8%); at nrefs 30 a
live length of 26 instead of 27 would give 46153 > 45714.
**Probe.** Thirteen bodies: no-px-variable with `pixel` carrying all three source
reads (s34/v1/a1,a2); blue carried by a fresh `b_src`, by `gp`, by `rp`, by
`total` (s34/v2/b1..b5); px shortened only at the tail (final shift into `bl`,
into `rp`, into `gp`; product and result both out of px) (s34/v3/c1..c6).
**Result.** Every px-shortening that keeps the stream byte-neutral scores 17,
unchanged from the un-shortened body - BB2_ALLOC_DEBUG on s34/v2/b1.c shows px
collapsing to nrefs=12 livelen=11 pri=32727 and still NOT taking $a0, because the
freed blue carrier becomes its own allocno at 18/16 pri=45000 and r_src is
untouched at 45714.  The tail-shortening bodies change the stream and score 9-43.
px's own priority is not the binding constraint; the binding constraint is
r_src's.
**measured_on.** HEAD 2026-09-11; s34/v2/b8.c and s33 candidate chassis, s21 j
chain extender + s32 `h` staging local present.

### H34.3 - CONFIRMED
**Statement.** With the red source read on `pixel` (the target's shape), the red
RESULT pseudo's allocno priority must land strictly inside (40000, 44444) for the
target's seat map (px $a0, red $a1, green $v1), and no even-raw-ref C spelling
lands there.
**Mechanism.** px is pinned at nrefs=30 livelen=27 pri=44444 and g_src at
nrefs=18 livelen=18 pri=40000 by the stream (H34.1); priority is
nrefs*40000/livelen with nrefs = 3 x raw refs at this loop depth, so the
attainable rows are 6 raw refs at livelen 17, 7 raw refs at livelen 19-20, 8 raw
refs at livelen 22-23.
**Probe.** Every red-tail spelling this session, read as ALLOCDBG rows rather
than scores.
**Result.** 8 raw / 21 -> 45714 (red steals $a0, score 17); 6 raw / 20 -> 36000
(green steals $a1, score 14); 6 raw / 19 -> 37894 (green steals $a1, score 14).
A C statement adds a def and a use together, so raw-ref counts come out even and
the 7-raw-ref row is the one no ordinary spelling reaches.  This is the residual
stated as arithmetic, and it is why every 2026-09-11 body is 14 or 17 except the
two 1-point and one 2-point bodies below.

### H34.4 - CONFIRMED
**Statement.** `sum = sum >> 15; r_src = sum & 0x1F;` on the pixel-read chassis
produces ALL THREE disputed seats exactly as the target has them, at 2/173.
**Mechanism.** It puts the red result pseudo at 6 raw refs / livelen 20
(pri=36000), which by the conflict graph - not by priority order - leaves $a1 for
red and $v1 for green; px at 44444 is then seated first and takes $a0.
**Probe.** tmp/grind/func_8003DE14/s34/v7/m2.c, scored and dumped.
**Result.** 2/173.  px pseudo 123 30/27 -> hardreg 4 ($a0, target); g_src 127
18/18 -> hardreg 3 ($v1, target); r_src 124 18/20 -> hardreg 5 ($a1, target).
Only rows 118/119 differ: ours `sra $v0,$v0,15` / `andi $a1,$v0,31` against the
target's `sra $a1,$v0,15` / `andi $a1,$a1,31`.  Banked as
memory/grind/func_8003DE14/chassis_s34_sumshift_2.c.  Its residual is the
combine_regs tie to the dying `sum` that s33 closed by making the shift's
destination multi-set - reintroduced here on purpose, because undoing it costs
r_src two references and moves it back out of the window (H34.3).

### H34.5 - KILLED (instance)
**Statement.** Routing the red shift and the red mask through two different
carriers (a fresh `rr`, the dead `rp`/`gp`, `sum`, or two alternating multi-set
red carriers) keeps the red result inside the (40000, 44444) window while
restoring `sra $a1,$v0,15`.
**Mechanism.** Splitting the tail across two pseudos should let the shift write a
multi-set pseudo (no combine_regs tie) while holding the red result's reference
count down.
**Probe.** Twelve bodies: s34/v6/h1..h4 and k1..k4 (separate `rr` carrier, mask
split and mask deferred, `rp` and `gp` reuse), s34/v8/n1..n4 (shift into r_src,
mask into rr/rp/gp/sum), s34/v9/p1..p4 (alternating two-carrier red chains).
**Result.** Best is 11 (p4); the cluster sits at 14 with green taking $a1 and red
$v1 (ALLOCDBG on n1: red 18/19 pri=37894 -> hardreg 3, green 18/18 pri=40000 ->
hardreg 5).  Splitting the tail always removes the pair of references that the
mask statement contributed, dropping the red result to 6 raw refs, which is below
green - the same failure as the deferred-mask form.
**measured_on.** HEAD 2026-09-11; s34/v2/b8.c and s33 candidate chassis, s21 j
chain extender + s32 `h` staging local present.

### H34.6 - KILLED (instance)
**Statement.** The `px` variable's existence or birth spelling is what produces
the target's `andi $a0,$t0,0xFFFF`, so re-deriving the body without it (or with a
plain widening) changes the seat outcome.
**Mechanism.** `andi $a0,$t0,0xFFFF` is the zero-extend GCC must emit for a u16
pseudo's SImode uses; s33's pxsplit chassis had combine fold it into `move`.
**Probe.** s34/v1/a1,a2 (no px variable at all; `pixel` carries all three source
reads and a fresh `b_src` carries blue) and s34/v7/m6 (`s32 px = pixel;`, no
explicit mask).
**Result.** All three are 17/173 with the andi present and correct - byte-identical
in that row to `s32 px = pixel & 0xFFFF;`.  px's birth is a free structural degree
of freedom; it decides nothing.
**measured_on.** HEAD 2026-09-11; s34/v2/b8.c chassis, s21 j chain extender +
s32 `h` staging local present.

## [s34] On the s33 red-split chassis with the red source read on `pixel`, moving one existing statement inside the blend block (src++ resited, the green source read moved after the red product, r_src declared after g_src, the blue source read hoisted, `sum` hoisted into the guard block, all three source reads computed up front) changes the pre-allocation live length of r_src or px.
- mechanism: global.c allocno priority = nrefs*40000/live_length, where live_length is the REG_LIVE_LENGTH flow.c recomputes after the first scheduling pass; s33's frontier assumed statement position could move one insn into or out of a pseudo's span.
- probe: Eight reorderings generated as tmp/grind/func_8003DE14/s34/v5/g1..g8, each scored with `sandbox func_8003DE14 --disable all` and each read through BB2_ALLOC_DEBUG via tmp/grind/func_8003DE14/s34/al.sh.
- result: g1/g3/g7 broke the stream (93-96 at 115-119 insns). g5/g6/g8 are byte-neutral at 17/173 and their ALLOCDBG rows are numerically IDENTICAL to the unmoved body: px nrefs=30 livelen=27 pri=44444, r_src nrefs=24 livelen=21 pri=45714, g_src nrefs=18 livelen=18 pri=40000. Not one unit moved. The first scheduling pass canonicalises the block before flow.c recounts, so statement position is inert for live length exactly as s28 found it inert for emission order. This retires s33's frontier probe 1 as written: the only lever on livelen/nrefs is how many C variables the channel's value passes through.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); tmp/grind/func_8003DE14/s34/v2/b8.c chassis (s33 red split + red source read on `pixel`), with the s21 j chain extender and the s32 `h` staging local present

## [s34] Shortening px's live range (to livelen <= 26 at nrefs 30) by giving the blue channel its own carrier, or by removing the px variable entirely, wins px the $a0 seat on the pixel-read chassis.
- mechanism: px loses the seat to r_src by 1270 points (2.8%); at nrefs 30 a live length of 26 instead of 27 would give 46153 > r_src's 45714, so one insn of px live range would flip the order.
- probe: Thirteen bodies: no-px-variable with `pixel` carrying all three source reads (s34/v1/a1,a2); blue carried by a fresh b_src, by gp, by rp, by total (s34/v2/b1..b5); px shortened only at the tail - final shift into bl, into rp, into gp, product and result both out of px (s34/v3/c1..c6).
- result: Every px-shortening that keeps the stream byte-neutral scores 17, unchanged from the un-shortened body. BB2_ALLOC_DEBUG on s34/v2/b1.c shows px collapsing to nrefs=12 livelen=11 pri=32727 and still NOT taking $a0, because the freed blue carrier becomes its own allocno at 18/16 pri=45000 while r_src is untouched at 45714. The tail-shortening bodies perturb the stream and score 9-43. px's own priority is not the binding constraint; r_src's is.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s34/v2/b8.c and the s33 candidate chassis, s21 j chain extender + s32 `h` staging local present

## [s34] With the red source read on `pixel` (the target's shape), the red RESULT pseudo's allocno priority must land strictly inside (40000, 44444) for the target seat map (px $a0, red $a1, green $v1), and the spellings measured this session all land outside it because each adds a def and a use together.
- mechanism: px is pinned at nrefs=30 livelen=27 pri=44444 and g_src at nrefs=18 livelen=18 pri=40000 by the emitted stream; priority is nrefs*40000/livelen with nrefs = 3 x raw refs at this loop depth, so attainable rows are 6 raw refs at livelen 17, 7 raw refs at livelen 19-20, 8 raw refs at livelen 22-23.
- probe: Every red-tail spelling of the session read as ALLOCDBG rows rather than as scores (tmp/grind/func_8003DE14/s34/al.sh over v2, v4, v6, v7, v8, v9).
- result: 8 raw / livelen 21 -> 45714 (red steals $a0, score 17, the s33 split); 6 raw / livelen 20 -> 36000 (green steals $a1, score 14, mask deferred to the OR chain); 6 raw / livelen 19 -> 37894 (green steals $a1, score 14, shift into r_src with the mask into a fresh rr). The 7-raw-ref row is the one that lands inside the window and no ordinary even-parity statement reaches it. This states the residual as arithmetic and explains why the whole 2026-09-11 sweep clusters at 14 and 17.
- verdict: CONFIRMED

## [s34] `sum = sum >> 15; r_src = sum & 0x1F;` on the pixel-read chassis produces all three disputed seats exactly as the target has them.
- mechanism: It puts the red result pseudo at 6 raw refs / livelen 20 (pri=36000), which by the conflict graph - not by priority order - leaves $a1 for red and $v1 for green, while px at 44444 is seated first and takes $a0.
- probe: tmp/grind/func_8003DE14/s34/v7/m2.c, scored with `sandbox func_8003DE14 --disable all` and dumped with BB2_ALLOC_DEBUG and tmp/grind/func_8003DE14/s32/rowdiff.py.
- result: 2/173. px pseudo 123 30/27 -> hardreg 4 ($a0, target); g_src 127 18/18 -> hardreg 3 ($v1, target); r_src 124 18/20 -> hardreg 5 ($a1, target). Only rows 118/119 differ: ours `sra $v0,$v0,15` / `andi $a1,$v0,31` against the target's `sra $a1,$v0,15` / `andi $a1,$a1,31`. Banked as memory/grind/func_8003DE14/chassis_s34_sumshift_2.c - a structurally different 2-point body from s32's, whose residual was the latch. It also exposes that seat order is NOT a pure function of priority order here: at 36000 red is seated after green and still gets $a1, while at 37894 with a one-insn-shorter interval (s34/v8/n1.c) green gets $a1 and red $v1.
- verdict: CONFIRMED

## [s34] Routing the red shift and the red mask through two different carriers (a fresh rr, the dead rp or gp, sum, or two alternating multi-set red carriers) keeps the red result inside the (40000,44444) window while restoring `sra $a1,$v0,15`.
- mechanism: Splitting the tail across two pseudos should let the shift write a multi-set pseudo (so combine_regs, local-alloc.c:1854-1897, cannot tie it to the dying `sum`) while holding the red result's reference count down.
- probe: Twelve bodies: s34/v6/h1..h4 and k1..k4 (separate rr carrier, mask split and mask deferred, rp and gp reuse), s34/v8/n1..n4 (shift into r_src with the mask into rr/rp/gp/sum), s34/v9/p1..p4 (alternating two-carrier red chains).
- result: Best is 11 (p4); the cluster sits at 14 with green taking $a1 and red $v1 - ALLOCDBG on n1 gives red 18/19 pri=37894 -> hardreg 3 and green 18/18 pri=40000 -> hardreg 5. Splitting the tail always removes the pair of references the mask statement contributed, dropping the red result to 6 raw refs, i.e. below green, which is the same failure mode as the deferred-mask form.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s34/v2/b8.c and the s33 candidate chassis, s21 j chain extender + s32 `h` staging local present

## [s34] The px variable's existence or birth spelling is what produces the target's `andi $a0,$t0,0xFFFF`, so re-deriving the body without a px variable (or with a plain widening) changes the seat outcome.
- mechanism: `andi $a0,$t0,0xFFFF` is the zero-extend GCC must emit for a u16 pseudo's SImode uses; s33's pxsplit chassis had combine fold that mask into a `move`, which suggested the birth spelling was load-bearing.
- probe: s34/v1/a1,a2 (no px variable at all: `pixel` carries all three source reads and a fresh b_src carries blue) and s34/v7/m6 (`s32 px = pixel;`, no explicit mask).
- result: All three measure 17/173 with the andi present and correct - byte-identical in that row to `s32 px = pixel & 0xFFFF;`. px's birth is a free structural degree of freedom and decides nothing; the seat is decided entirely by the red result pseudo's reference/live-length arithmetic.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s34/v2/b8.c chassis, s21 j chain extender + s32 `h` staging local present

## [s35] On the pixel-read chassis (the target's own red source shape), the red result pseudo's allocno priority can be pushed below px's by taking the red channel's `r * factor` product off r_src (6 raw refs instead of 8).
- mechanism: pri = nrefs*40000/livelen; dropping a def/use pair takes red from 24 refs to 18, which at livelen ~20 is 36000, well under px's 44444.
- probe: six bodies tmp/grind/func_8003DE14/s35/v1/a1..a6 - the factor product staged through `sum`, through `gp`, through `rp`, and the sum accumulated in `rp` rather than `sum`, each scored with `sandbox func_8003DE14 --disable all`.
- result: KILLED as a route. 21 (a3), 26 (a2), 35 (a4), 40 (a5), 41 (a1), 42 (a6) - all far above the 17 baseline of the un-modified pixel-read chassis. Taking the factor product off r_src re-orders the two multiply groups in the emitted stream (the `mflo` of `r*factor` no longer lands between the green source read and the green product), which costs more rows than the seat fix is worth.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); tmp/grind/func_8003DE14/s35/base.c with the red source read on `pixel` (= s34's b8 chassis, 17/173), s21 j chain extender + s32 `h` staging local present

## [s35] Re-associating the final OR chain is a LIVE lever on the blend block's allocno live lengths (not byte-inert), and it alone takes the pixel-read chassis from 17 to 2.
- mechanism: the OR chain is the last use of all three channel results and of px, so the textual position of each term fixes where that pseudo dies; flow.c's REG_LIVE_LENGTH (recomputed after the first scheduling pass) therefore moves with the association, and global.c's priority = nrefs*40000/livelen moves with it.
- probe: seventeen OR spellings, tmp/grind/func_8003DE14/s35/v7/i1..i6 and s35/v8/k3..k15, each scored, with BB2_ALLOC_DEBUG rows read for i1 and i3 via tmp/grind/func_8003DE14/s35/al.sh.
- result: CONFIRMED. Moving the red term from 2nd to 3rd place (`(pixel&0x8000) | (g_src&0x3E0) | r_src | (px&0x7C00)`, s35/v7/i1) takes red from nrefs=24/livelen=21/pri=45714 to 24/23/41739 and green from 18/18/40000 to 18/17/42352, which seats px $a0, green $v1, red $a1 - ALL THREE target seats - and scores 2/173 with a residual of exactly two rows (our `or` of the green term is emitted one slot before the `or` of the red term; the target has red's `or` first). Four more spellings reach 2-3 the same way: i3 (`v|G|B|R`, px 30/26=46153, red 24/25=38400), i5 (`(v|R)|(G|B)`), k5 (`v|R|(G|B)`), k9 (`R|(v|G)|B`), k10. This REFINES s33's kill of OR re-association: that kill was measured against px's live range on the px-read chassis and is correct there, but the lever is not inert - it is the cheapest known control on the red/green live lengths.
- verdict: CONFIRMED

## [s35] The whole remaining residual on the pixel-read chassis is the single inequality pri(px) > pri(red), i.e. red's live length must be >= 22 at 24 refs while px stays at 30 refs / livelen 27.
- mechanism: px is pinned at 30 refs (10 raw: the 0xFFFF birth, the zero test, the green and blue source reads, the blue product and result, the blue mask) and red at 24 refs (8 raw: source read, complement product, r*factor, the sum, the shift, the mask, the OR use) - both counts are exactly what the target's $a0 and $a1 carry, verified insn by insn against asm/funcs/func_8003DE14.s lines 96-138. With nrefs pinned on both sides the ONLY free variable is live length: 24*40000/21 = 45714 > 44444 (red steals $a0, score 17), 24*40000/22 = 43636 < 44444 (all three seats correct).
- probe: BB2_ALLOC_DEBUG rows for every body measured this session (s35/al.sh over v2, v3, v5, v7, v8, v11, v12), cross-checked against the register each pseudo actually received and against the row diff (s35/rowdiff.py).
- result: CONFIRMED, and it is a sufficient condition, not just necessary: EVERY body this session whose red row landed at livelen >= 22 with px at 30/27 came out with all three disputed seats target-correct (s35/v5/g1 at 24/22=43636, s35/v7/i1 at 24/23=41739, s35/v11/q2 at 24/23=41739, s35/v12/r4, r5), and every body with red at 24/21 came out with red in $a0 (17/173). The green/red priority ORDER does not matter (g1 has red above green and is correct; i1 has green above red and is correct) - only px above both.
- verdict: CONFIRMED

## [s35] Declaring the red source read ABOVE the `px == 0` test makes r_src live across the block boundary and buys exactly the one unit of live length the seat inequality needs.
- mechanism: a pseudo assigned in the pre-branch block and used in the blend block is live at the block boundary, so flow.c counts the boundary insns; this is the one live-length lever that survives the first scheduling pass (s34 killed every in-block statement move as livelen-inert, and this session's s35/v10/p3, p4, p5 - named sign-bit local, named store result, blue source read off `pixel` - all re-measured byte-neutral at 17 with identical ALLOCDBG rows, confirming that kill).
- probe: tmp/grind/func_8003DE14/s35/v5/g1 (`s32 r_src = (pixel & 0x1F) << 3;` hoisted above the test), g3, g4, s35/v6/h5, s35/v12/r5 (`* 8` instead of `<< 3`), scored and dumped.
- result: CONFIRMED for the seat. g1 gives red 24/22 = 43636 and px 30/27 = 44444, all three seats target-correct, 3/173. The residual is PLACEMENT, not allocation: because the whole expression now lives in the pre-branch block, GCC emits `andi $v0,$t0,0x1F` AND `sll $a1,$v0,3` before the branch, while the target has only the `andi` there (reorg.c put it in the `bnez` delay slot) and the `sll` at the top of the blend block. r5 (`* 8`) is byte-identical to g1; q2/r4 (mask hoisted alone, shift folded into the complement product as `(r_src * 8) * complement`) also reach 3/173 with all seats correct but reassociate to `sll $v0,$t5,3` / `mult $a1,$v0` - GCC moves the scale onto `complement`.
- verdict: CONFIRMED

## [s35] Splitting the hoist - the MASK hoisted above the test into its own local and the SHIFT applied inside the blend block - keeps the target's pre-branch block shape AND the cross-block live length.
- mechanism: the target's pre-branch insn `andi $v0,$t0,0x1F` writes a temp, not $a1, so the red allocno should be born at the `sll` inside the blend block while still being live-in from the mask temp.
- probe: s35/v6/h1 (`s32 rm = pixel & 0x1F;` hoisted, `s32 r_src = rm << 3;` inside), s35/v6/h2 (`rp = (rm << 3) * complement;`), s35/v11/q1 and q3 (mask hoisted into r_src itself, shift re-applied in-block as `r_src = r_src << 3;`), s35/v12/r2, r3, r6 (operand-order and parenthesisation variants of the folded product).
- result: KILLED. h1 = 17 (the split puts r_src's birth back inside the block, so its live length returns to 21 and red retakes $a0); h2 = 13 at 174 insns (an extra insn); q3 = 18 (the in-block `r_src = r_src << 3;` adds a def/use pair, taking red to 30 refs, which overshoots px again); q1/r3/r6 = 33-35 (a whole-function pointer-register reshuffle - $a3/$t0 swap in the row loop); r2 = 33. A separate carrier for the mask does not transfer the boundary liveness to r_src, and re-applying the shift in-block costs the reference budget the inequality depends on.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s35/base.c pixel-read chassis, s21 j chain extender + s32 `h` staging local present

## [s35] On the pixel-read chassis (the target's own red source shape), the red result pseudo's allocno priority can be pushed below px's by taking the red channel's `r * factor` product off r_src (6 raw refs instead of 8).
- mechanism: global.c priority = nrefs*40000/live_length; dropping one def/use pair takes red from 24 refs to 18, which at live length ~20 is 36000, well under px's 44444.
- probe: Six bodies tmp/grind/func_8003DE14/s35/v1/a1..a6 - the factor product staged through `sum`, through `gp`, accumulated in `rp` instead of `sum`, and two operand-order variants - each scored with `sandbox func_8003DE14 --disable all`.
- result: 21 (a3), 26 (a2), 35 (a4), 40 (a5), 41 (a1), 42 (a6), against the 17/173 of the unmodified pixel-read chassis. Taking the factor product off r_src re-orders the two multiply groups in the emitted stream (the `mflo` of `r*factor` no longer lands between the green source read and the green product), which costs far more rows than the seat fix buys.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); tmp/grind/func_8003DE14/s35/base.c, i.e. s34's b8 chassis with the red source read on `pixel`, s21 j chain extender + s32 `h` staging local present

## [s35] Re-associating the final OR chain is a live lever on the blend block's allocno live lengths, and it alone takes the pixel-read chassis from 17 to 2.
- mechanism: The OR chain is the last use of all three channel results and of px, so each term's textual position fixes where that pseudo dies; flow.c's REG_LIVE_LENGTH (recomputed after the first scheduling pass) moves with the association, and global.c's priority moves with it.
- probe: Seventeen OR spellings - tmp/grind/func_8003DE14/s35/v7/i1..i6 and s35/v8/k3..k15 - each scored, with BB2_ALLOC_DEBUG rows read for i1 and i3 through tmp/grind/func_8003DE14/s35/al.sh and row diffs through s35/rowdiff.py.
- result: Moving the red term from 2nd to 3rd place, `(pixel&0x8000) | (g_src&0x3E0) | r_src | (px&0x7C00)` (s35/v7/i1), takes red from nrefs=24/livelen=21/pri=45714 to 24/23/41739 and green from 18/18/40000 to 18/17/42352, seating px $a0, green $v1 and red $a1 - all three target seats - at 2/173, residual exactly two rows (our `or` of the green term is emitted one slot before the `or` of the red term; the target has red's `or` first). i3 (`v|G|B|R`) also 2/173 with px 30/26=46153 and red 24/25=38400; i5, k5, k9, k10 reach 2-3 the same way. This REFINES s33's kill of OR re-association, which was measured against px's live range on the px-read chassis: the lever is not byte-inert, it is the cheapest known control on the channel live lengths. Banked as memory/grind/func_8003DE14/chassis_s35_or_reassoc_2.c.
- verdict: CONFIRMED

## [s35] The whole remaining residual on the pixel-read chassis is the single inequality pri(px) > pri(red): red must reach live length 22 or more at its pinned 24 references while px stays at 30 references / live length 27.
- mechanism: px's 10 raw references and red's 8 raw references were counted insn by insn out of asm/funcs/func_8003DE14.s lines 96-138 and both already match what our C emits, so the only free variable is live length: 24*40000/21 = 45714 > px's 44444 (red is allocated first and steals $a0), 24*40000/22 = 43636 < 44444 (px keeps $a0).
- probe: BB2_ALLOC_DEBUG rows for every body measured this session (s35/al.sh over v2, v3, v5, v7, v8, v11, v12), cross-checked against the hard register each pseudo actually received and against the row diff.
- result: Sufficient, not merely necessary: every body with red at live length >= 22 and px at 30/27 came out with all three disputed seats target-correct (s35/v5/g1 at 24/22=43636, s35/v7/i1 at 24/23, s35/v11/q2 and s35/v12/r4, r5 at 24/23), and every body with red at 24/21 put red in $a0 at 17/173. The green-versus-red priority ORDER does not matter (g1 has red above green and is correct, i1 has green above red and is correct) - only px above both. Also answers s34's frontier probe 2 with a NO: the fast arm (asm lines 78-92) carries no $a1 at all, so the red allocno is not cross-arm.
- verdict: CONFIRMED

## [s35] Declaring the red source read above the `px == 0` test makes r_src live across the block boundary and buys exactly the one unit of live length the seat inequality needs.
- mechanism: A pseudo assigned in the pre-branch block and used in the blend block is live at the block boundary, so flow.c counts the boundary insns; this is the one live-length lever that survives the first scheduling pass, whereas s34's kill of in-block statement moves was re-confirmed here by three more byte-neutral rewrites (s35/v10/p3 named sign-bit local, p4 named store result, p5 blue source read off `pixel`), all 17/173 with numerically identical ALLOCDBG rows.
- probe: tmp/grind/func_8003DE14/s35/v5/g1, g3, g4, s35/v6/h5, s35/v12/r5, s35/v11/q2 and s35/v12/r4, plus s35/v13/t1..t4 (declaration-order and OR-combination variants of the hoist), scored and dumped.
- result: g1 gives red 24/22 = 43636 against px 30/27 = 44444, all three seats target-correct, 3/173. The residual is PLACEMENT, not allocation: with the whole expression in the pre-branch block GCC emits both `andi $v0,$t0,0x1F` and `sll $a1,$v0,3` before the branch, while the target has only the `andi` there (reorg.c delay-slot fill) and the `sll` at the top of the blend block. r5 (`* 8` instead of `<< 3`) is byte-identical to g1; q2/r4 (mask hoisted alone, scale folded into the complement product) also reach 3/173 with all seats correct but reassociate the scale onto `complement` (`sll $v0,$t5,3` / `mult $a1,$v0`). t1 (red declared before px) and t3 (hoisted read off px) also 3; t2 (hoist plus the i1 OR order) is 5. Banked as memory/grind/func_8003DE14/chassis_s35_hoist_seats_3.c.
- verdict: CONFIRMED

## [s35] Splitting the hoist - the red MASK hoisted above the test into its own local and the SHIFT applied inside the blend block - keeps both the target's pre-branch block shape and the cross-block live length.
- mechanism: The target's pre-branch insn `andi $v0,$t0,0x1F` writes a temp, not $a1, so the red allocno should be born at the `sll` inside the blend block while still being live-in from the mask temp.
- probe: s35/v6/h1 (`s32 rm = pixel & 0x1F;` hoisted, `s32 r_src = rm << 3;` inside), s35/v6/h2, s35/v11/q1 and q3 (mask hoisted into r_src itself, shift re-applied in-block), s35/v12/r2, r3, r6 (operand-order and parenthesisation variants of the folded product).
- result: h1 = 17 (a separate carrier does not transfer the boundary liveness, so red's live length returns to 21 and it retakes $a0); h2 = 13 at 174 insns; q3 = 18 (the in-block `r_src = r_src << 3;` adds a def/use pair, taking red to 30 refs and overshooting px again); q1/r3/r6 = 33-35 (a whole-function pointer-register reshuffle, $a3/$t0 swapped in the row loop); r2 = 33.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s35/base.c pixel-read chassis, s21 j chain extender + s32 `h` staging local present

## [s35] Giving px its one missing reference in the zero-pixel arm, by storing `*dst = px;` instead of `*dst = pixel;` (px and pixel hold the same value there), lifts px above red without costing a row.
- mechanism: px needs nrefs 33 (pri 48888 at live length 27) to clear red's 45714; the zero-arm store is the only remaining site where px and pixel are interchangeable in ordinary C.
- probe: tmp/grind/func_8003DE14/s35/v2/b1.c, scored with `sandbox func_8003DE14 --disable all`.
- result: 18/173, one worse than the 17 baseline: the store prints `sh $a0,0($a2)` where the target has `sh $t0,0($a2)`, so the reference costs exactly the row it buys.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; tmp/grind/func_8003DE14/s35/base.c pixel-read chassis, s21 j chain extender + s32 `h` staging local present

## [s36] Enumerating the OR-chain region in fully-named form (tools/spelling_enum.py) closes the last row of the s35 seat inequality.
- mechanism: s35 proved the residual is pri(px) > pri(red) in global.c's allocno ordering (prio = nrefs*40000/live_length), needing red at live length 22 with its pinned 24 refs while px stays 30/27. The OR chain is the last reference of all three channel results, so which sub-expressions of it are NAMED decides where each pseudo dies - a lever that survives the first scheduling pass, unlike in-block statement position (s34).
- probe: the chain was rewritten in fully-named form (`sign`, `gm`, `bm`, `sr`, `srg`) between ENUM markers on the s35 or-reassoc chassis (tmp/grind/func_8003DE14/s36/or_region.c); tools/spelling_enum.py --no-swaps generated all 104 inline/declaration-order spellings into tmp/grind/func_8003DE14/s36/enumA and tools/sweep_variants.py scored them in one pass (tmp/grind/func_8003DE14/s36/enumA.json).
- result: CONFIRMED - ENUMERATION: 104 spellings, best 0, 18 at the floor. Histogram: 18 x 0, 58 x 3, 24 x 17, 4 x 18. The minimal zero (v101) names exactly ONE intermediate, the green mask: `gm = g_src & 0x3E0;` with the chain `(pixel & 0x8000) | r_src | gm | (px & 0x7C00)`. BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s36/alloc.log): with gm, green 127 18/16 pri=45000 -> $v1, px 123 30/27 pri=44444 -> $a0, red 124 24/22 pri=43636 -> $a1 - all three target seats; without gm (same chain, mask inline) red 124 24/21 pri=45714 -> $a0 and the body scores 17. Naming the mask ends green's live range one insn earlier and pushes red's last reference one insn later, which is exactly the one unit of live length the s35 inequality needed, bought WITHOUT moving any expression across a block boundary (so the pre-branch block keeps the target's lone `andi $v0,$t0,0x1F`). SCORE 0/173 - the honest floor is 0. Banked as memory/grind/func_8003DE14/candidate.c and chassis_s36_match_0.c.
- verdict: CONFIRMED

## [s36] The green-mask carrier can be an EXISTING local (gp, rp, sum) or g_src itself instead of a fresh `gm`.
- mechanism: if a borrowed dead carrier worked, the construct would sit in the frozen variable-reuse family and need no named-intermediate claim at all.
- probe: tmp/grind/func_8003DE14/s36/x - a_gp (`gp = g_src & 0x3E0;`), c_rp, b_sum, d_gsrc (`g_src = g_src & 0x3E0;` at the chain), e_compound (`g_src &= 0x3E0;`), f_gmdecl_top (fresh `gm` declared with sum/rp/gp and assigned at the use site); plus tmp/grind/func_8003DE14/s36/w/w2.c (the mask split into the green tail right after `g_src = sum >> 10;`).
- result: only a FRESH carrier reaches 0 (f_gmdecl_top = 0, and the kept body uses that declaration style). gp = 22, rp = 24, sum = 43, `g_src = g_src & 0x3E0;` at the chain = 17, `g_src &= 0x3E0;` = 17, the mask split into the green tail = 17. This is prong (4) of the named-intermediate entry (fresh local, not a borrow) measured rather than asserted.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); the s36 zero chassis (s35 or-reassoc chassis + natural-order OR chain), s21 j chain extender and s32 `h` staging local present

## [s36] The parenthesisation the enumerator emitted is load-bearing, i.e. the zero body commits a non-natural or-tree grouping.
- mechanism: [[or-tree-shape-shift]] forbids enumerated operand orders/groupings outside the narrow 2026-08-20 carve-out, so it matters whether the winning chain is a natural ordering or a grouping artefact.
- probe: tmp/grind/func_8003DE14/s36/fin/noparen.c - the enumerator's `(((pixel & 0x8000) | r_src) | gm) | (px & 0x7C00)` rewritten as plain `(pixel & 0x8000) | r_src | gm | (px & 0x7C00)`.
- result: byte-identical, 0/173. The kept spelling is therefore plain left-to-right sign|red|green|blue - the natural channel order (same order as the function's r/g/b locals and color_info[0..2]) and the order the target's own bytes were emitted in (asm/funcs/func_8003DE14.s:134-139). No non-natural grouping is committed and the 2026-08-20 carve-out is not needed; [[or-tree-shape-shift]] "What IS allowed" governs.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the s36 zero chassis, s21 j chain extender and s32 `h` staging local present

## [s36] The inherited `h` staging local can be replaced by an ordinary-C construct now that the rest of the body matches.
- mechanism: `h`'s job (s32) is to make the latch's rect[3] pseudo non-block-local - local-alloc.c:470-476 skips any pseudo with reg_basic_block < 0, so global.c seats it and the first scheduling pass emits the two halfword loads in the target's `lh $v0,4` / `lh $v1,6` order. Any construct that writes one local in two basic blocks should do the same job; if the carrier were an EXISTING variable the construct would sit in the sanctioned staged-value-reused-variable family instead of being unclassifiable.
- probe: five waves on the s36 zero chassis - tmp/grind/func_8003DE14/s36/y (h1 latch-only, h2 fast-arm-only, h3 `while (j < total)`, j1 extender-free, j2 extender operand swap); s36/z (p1 h split into two once-written locals, p2 fast arm staged through a fresh block-local, p3 `*dst++ = h = target_color;`); s36/q (h renamed onto the existing locals total, blend_base, saved_y); s36/L (nine ordinary latch-expression spellings on an h-free chassis: natural, operand-swapped, `>`-form, `!=`-form, cast, subtraction, negated `>=`); s36/m and s36/n (sum, rp, gp, px, g_src, r_src hoisted to inner-loop scope to carry the latch write).
- result: KILLED - nothing ordinary reproduces it. Borrowing the genuinely-dead existing `total` = 2 (the closest miss, and the ONE form that would need no new family). Splitting h into two once-written locals = 3; dropping either of h's two references = 3; `*dst++ = h = target_color;` = 0 (same construct, denser spelling); all nine latch-expression spellings = 3 (six) or 5 (three); hoisted carriers = 3 (px), 10 (gp), 12 (rp), 20 (gm), 41 (sum), 42/43 (fast-arm variants), 100 (g_src, r_src). The j chain extender is equally load-bearing: plain `(s32)dst_buf` = 7, and the operand-swapped extender `((s32)dst_buf - j) + j` = 0 (same construct). `h` therefore remains in the body, and because it is a FRESH local written twice it is excluded from staged-value-reused-variable by bound 2 and from the named-intermediate entry by its multi-write property - hence this session's ruling-request.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the s36 zero chassis (candidate.c), s21 j chain extender present except where named

## [s36b] The `h`-free body built on the Judge's `total` grant reaches 2/173, and the fast-arm staging site `h` carried is not load-bearing.
- mechanism: the Judge's 2026-09-11 04:48 ruling grants [[staged-value-reused-variable]] for staging through the existing, genuinely-dead `total`; if the construct works at all it should work with the minimum number of sites, since the escaped-pseudo effect needs only one set inside the inner loop.
- probe: tmp/grind/func_8003DE14/s36b/base.c (both sites), waveB/b5_latch_only.c (latch only), waveB/b6_fast_only.c (fast arm only), waveD/d0_plain_nothing.c (no carrier), each scored with `sandbox func_8003DE14 --disable all`.
- result: both sites 2/173, latch only 2/173, fast arm only 3/173, no carrier 3/173. One site suffices and is kept; candidate.c is the latch-only body. The fast-arm staging that the rejected s36 body carried bought nothing on top of the latch site.
- verdict: CONFIRMED

## [s36b] The first scheduling pass, not the latch expression, decides which of the two loop-bound halfword loads is emitted first, and it decides it on where the carrier pseudo's OTHER set lives.
- mechanism: sched.c's rank_for_schedule breaks ties by INSN_LUID (original order), so an order change means the two loads are not tied; the only structural difference between the accepted-but-rejected `h` body and the `total` body is which pseudo the rect[3] load writes.
- probe: cc1 -da dumps for three bodies (tmp/grind/func_8003DE14/s36b/dumps_h from the rejected s36 candidate, dumps_b5 from the latch-only borrow, dumps_tot from the both-sites borrow), with the two loads located in every pass dump by tmp/grind/func_8003DE14/s36b/ord2.py, plus the full RTL of the two insns and the mult printed with s36b/show.py.
- result: CONFIRMED. After .combine all three bodies have the rect[2] load first and the rect[3] load second, and the RTL is byte-identical apart from the carrier's pseudo number (reg/v:SI 117 for `h`, reg/v:SI 101 for `total`) - same insn_list dependencies on the mult, same REG_DEAD notes, same patterns, same `mem/s:HI (plus:SI (reg/v:SI 72) (const_int 4|6))` addressing. The .sched dump shows the order preserved for `h` and SWAPPED for `total`, and every later dump (.lreg, .greg, .sched2, .dbr) inherits the .sched order. This kills the s31-era guess that the latch's source-level operand order or birth order decides it, and re-confirms the s32 attribution with a direct dump rather than by inference.
- verdict: CONFIRMED

## [s36b] The loop bound's latch expression, loop-tail structure, guard spelling and declaration placement can be spelled so that the two halfword loads come out in the target's order while the borrow is `total`.
- mechanism: if the order were an emission-order artefact of the source expression, some spelling of it would flip it; the enumerate modality's job is to settle that exhaustively rather than by sampling.
- probe: 48 complete bodies in five waves - wave A (18: latch expression x fast-arm spelling), wave B (13: carrier semantics, guard, declaration placement/scope), wave C (18: loop-tail structure - increment placement, ++j in the condition, comma operator, break-form, subtraction-form, ternary, label position), wave D (11: the width-carrier chassis x fast-arm staging x declaration scope), wave E (12: hoisted blend-block carriers) - generated by tmp/grind/func_8003DE14/s36b/gen{A,B,C,D,E}.py and scored in five sweeps with tools/sweep_variants.py.
- result: nothing reaches 1 or 0. Best is 2 and it is reached by many spellings (9 of 18 in wave A, 10 of 18 in wave C), i.e. the floor is flat across the whole spelling space of that region rather than sitting on one lucky form. The rowdiff of every 2-point body in the `total = rect[3]` family is the same pair of rows (the two lh emitted in the wrong order, registers correct), and the rowdiff of every 2-point body in the `total = rect[2]` family is the complementary pair (order and mult correct, the two lh destination registers swapped). Scores away from the floor are 3 (most), 4, 5, 7.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); the s36 zero chassis with `h` replaced by the Judge-granted `total` borrow, s21 j chain extender and s36 `gm` named intermediate present

## [s36b] Hoisting one of the blend block's dead locals (gp, rp, sum) to inner-loop scope to carry the latch read reproduces `h`'s shape with an EXISTING variable, now that `h` itself is gone.
- mechanism: the s36b dump evidence says the carrier needs its other set INSIDE the inner loop; gp/rp/sum are set in the blend block, which is inside the inner loop, so borrowing one of them should give the target's load order while staying inside [[staged-value-reused-variable]]. The s36 wave m/n kill of this route was measured with `h` still occupying the latch, so it needed re-auditing (mandated kill re-audit).
- probe: twelve bodies, tmp/grind/func_8003DE14/s36b/waveE - for each of gp, rp, sum: declaration hoisted to inner-loop scope with no carrier use (e_*_none), and used as the latch carrier in three latch spellings (height carrier, width carrier in both operand orders).
- result: the s36 kill HOLDS on the h-free chassis. The declaration hoist alone is byte-neutral - e_gp_none, e_rp_none, e_sum_none all score 3, identical to the no-carrier body d0 - so the cost is entirely in the carrier use: gp = 10, rp = 12, sum = 41, and the three latch spellings score identically within each carrier (10/10/10, 12/12/12, 41/41/41), confirming the cost is the carrier's extra live range through the latch block and not the bound's spelling.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the h-free `total`-borrow chassis (memory/grind/func_8003DE14/candidate.c), s21 j chain extender and s36 `gm` named intermediate present

## [s36b] Carrying the WIDTH operand in `total` instead of the height fixes the emission order and the mult operand order, leaving only the two load destination registers.
- mechanism: rule R1 says the escaped carrier's load is scheduled first; if the carrier holds rect[2] - the operand the target loads FIRST - then "carrier first" and "target order" agree, and the only thing left to fix is which hard register each load writes.
- probe: tmp/grind/func_8003DE14/s36b/waveC/c07.c, c09.c and waveD/d1_c07.c, d8_c07_swapmul.c, rowdiffed with tmp/grind/func_8003DE14/s36b/rdmulti.sh.
- result: CONFIRMED. `while (j < rect[3] * (total = rect[2]));` (d8) scores 2 with rows `lh $v1,0x4` / `lh $v0,0x6` against the target's `lh $v0,0x4` / `lh $v1,0x6` - order correct, `mult $v0,$v1` correct, registers swapped. The `(total = rect[2]) * rect[3]` spellings (c07, c09, d1) score 3 because they additionally print `mult $v1,$v0`. ALLOCDBG on d1 shows pseudo 101 at nrefs=10 livelen=11 pri=27272 taking hardreg 3 ($v1) at ord=7, while $v0 is already held in that block by the local-alloc-seated block-local load - i.e. R2 is a local-alloc-versus-global.c ordering fact, not a priority race global.c could win. Banked as memory/grind/func_8003DE14/chassis_s36b_borrow_regs_2.c.
- verdict: CONFIRMED

## [s36] The h-free body built on the Judge's `total` grant reaches 2/173, and the fast-arm staging site that the rejected s36 body carried is not load-bearing.
- mechanism: The Judge's 2026-09-11 04:48 ruling grants staged-value-reused-variable for staging through the existing, genuinely-dead `total`; the escaped-pseudo effect the latch needs requires only one set inside the inner loop, so the second staging site should be droppable.
- probe: Four bodies scored with `sandbox func_8003DE14 --disable all`: tmp/grind/func_8003DE14/s36b/base.c (both sites), waveB/b5_latch_only.c (latch only), waveB/b6_fast_only.c (fast arm only), waveD/d0_plain_nothing.c (no carrier at all).
- result: Both sites 2/173; latch only 2/173; fast arm only 3/173; no carrier 3/173. The latch-only body is kept as candidate.c - a single-site borrow of a single existing dead local, with the per-site liveness sentence the ruling requires written into its FAKE annotation.
- verdict: CONFIRMED

## [s36] The FIRST scheduling pass, not the latch expression or the birth order, is what orders the two loop-bound halfword loads, and it makes a different choice purely on which pseudo the rect[3] load writes.
- mechanism: sched.c's rank_for_schedule breaks ties by INSN_LUID (original order), so an order change means the two loads are not tied; the only structural difference between the rejected `h` body and the `total` body is the carrier's pseudo.
- probe: cc1 -da dumps for three bodies (tmp/grind/func_8003DE14/s36b/dumps_h, dumps_b5, dumps_tot); the two loads located in every pass dump by s36b/ord2.py and their full RTL printed with s36b/show.py.
- result: After .combine all three bodies have the rect[2] load first and the rect[3] load second, with byte-identical RTL apart from the carrier's pseudo number (reg/v:SI 117 for `h`, reg/v:SI 101 for `total`) - same insn_list dependencies on the mult, same REG_DEAD notes, same patterns. The .sched dump keeps that order for `h` and SWAPS it for `total`; .lreg, .greg, .sched2 and .dbr all inherit the .sched order. Re-confirms s32's attribution by direct dump instead of inference and kills the s31-era guess that source operand order decides it.
- verdict: CONFIRMED

## [s36] Some spelling of the latch expression, the loop-tail structure, the guard, or the declaration placement puts the two halfword loads in the target's order while the carrier is `total`.
- mechanism: If the emission order were an artefact of the source expression rather than of the carrier's pseudo, enumerating the region exhaustively would flip it.
- probe: 48 complete bodies in five generated waves (tmp/grind/func_8003DE14/s36b/gen{A,B,C,D,E}.py), scored in five sweeps with tools/sweep_variants.py: wave A 18 latch-expression x fast-arm spellings, wave B 13 carrier-semantics/guard/declaration-scope variants, wave C 18 loop-tail structures (increment placement, ++j in the condition, comma operator, break-form, subtraction-form, ternary, label position), wave D 11 width-carrier chassis variants, wave E 12 hoisted-carrier variants.
- result: ENUMERATION: 48 spellings, best 2, 21 at the floor. Nothing reaches 1 or 0. The floor is flat rather than sitting on one lucky form (wave A 9 of 18 at 2, wave C 10 of 18 at 2); every 2-point body in the `total = rect[3]` family shows the same two rows (loads in the wrong order, registers correct) and every 2-point body in the `total = rect[2]` family shows the complementary two rows (order and mult correct, load destination registers swapped). Off-floor scores are 3 (most), 4, 5, 7.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); the s36 zero chassis with `h` replaced by the Judge-granted `total` borrow, s21 j chain extender and s36 `gm` named intermediate present

## [s36] Hoisting one of the blend block's dead locals (gp, rp, sum) to inner-loop scope to carry the latch read reproduces the rejected `h` shape with an EXISTING variable, now that `h` itself is gone.
- mechanism: The dump evidence says the carrier needs its other set inside the inner loop; gp, rp and sum are set in the blend block, which is inside the inner loop, so borrowing one should give the target's load order while staying inside staged-value-reused-variable. The s36 wave m/n kill of this route was measured with `h` still occupying the latch, so the mandated kill re-audit re-measured it here.
- probe: Twelve bodies, tmp/grind/func_8003DE14/s36b/waveE: for each of gp, rp, sum, the declaration hoisted to inner-loop scope with no carrier use, and used as the latch carrier in three latch spellings (height carrier, width carrier in both operand orders).
- result: The s36 kill HOLDS on the h-free chassis. The declaration hoist alone is byte-neutral (e_gp_none, e_rp_none, e_sum_none all 3/173, identical to the no-carrier body), so the whole cost is the carrier use: gp = 10, rp = 12, sum = 41, and the three latch spellings score identically within each carrier (10/10/10, 12/12/12, 41/41/41).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the h-free `total`-borrow chassis (memory/grind/func_8003DE14/candidate.c), s21 j chain extender and s36 `gm` named intermediate present

## [s36] Carrying the WIDTH operand in `total` instead of the height fixes the emission order and the mult operand order, leaving only the two load destination registers.
- mechanism: The escaped carrier's load is scheduled first, so putting the carrier on rect[2] - the operand the target loads first - makes 'carrier first' and 'target order' agree, isolating the register question.
- probe: tmp/grind/func_8003DE14/s36b/waveC/c07.c, c09.c and waveD/d1_c07.c, d8_c07_swapmul.c, rowdiffed with s36b/rdmulti.sh and dumped with the s36 ALLOCDBG script.
- result: `while (j < rect[3] * (total = rect[2]));` scores 2 with rows `lh $v1,0x4` / `lh $v0,0x6` against the target's `lh $v0,0x4` / `lh $v1,0x6` - order correct, `mult $v0,$v1` correct, only the two destination registers swapped. The `(total = rect[2]) * rect[3]` spellings score 3 because they also print `mult $v1,$v0`. ALLOCDBG on d1 puts pseudo 101 at nrefs=10 livelen=11 pri=27272 taking hardreg 3 ($v1) at ord=7 while $v0 is already held in that block by the local-alloc-seated block-local load, so the register split is a local-alloc-before-global.c fact, not a priority race global.c can win. Banked as chassis_s36b_borrow_regs_2.c - exactly complementary to candidate.c.
- verdict: CONFIRMED

## s37 (enumerate) hypotheses

**H37-1 CONFIRMED (class-level mechanism, closed form).** The latch's emission ORDER is decided by
`birthing_insn_p` in the first scheduling pass, and that predicate is exactly
`reg_n_sets[dest] == 1` (tools/gcc-2.7.2/sched.c:2526, reached from `adjust_priority`,
sched.c:2542-2592). A latch carrier with two or more sets anywhere in the function loses the
priority lift and its load is emitted FIRST; a single-set carrier keeps the lift, ties with the
other load, and LUID order (= combine's order, offset 4 first) survives. Measured with
BB2_SCHED_DEBUG on the two s36 bodies: `total` carrier -> `birth=0`, `h` carrier -> `birth=1`.
This REPLACES s36's rule R1 ("both sets inside the inner loop"), which was a coincidence of the
two bodies compared: the relevant property is the COUNT of sets, not their location.

**H37-2 CONFIRMED.** The latch's SEATS are decided by whether the height pseudo is block-local.
Escape means "live at the end of some basic block" (tools/gcc-2.7.2/flow.c:1428, in
`propagate_block`), which requires a reference in another basic block; local-alloc then skips it
(local-alloc.c:194) and seats only the block-local rect[2] load, which takes $v0, leaving global.c
to give the height $v1 — the target's map. A single-set carrier with NO cross-block reference is
still block-local and scores 3 (v3/p0.c), identical to the no-carrier body.

**H37-3 KILLED (class).** On a latch whose two loop-bound halfword loads are both block-local
pseudos, the target's `lh $v0,4($s0)` first / `lh $v1,6($s0)` second cannot be produced.
`qty_compare_1` (local-alloc.c:1669-1684) ranks by
`floor_log2(n_refs)*n_refs*size/(death-birth)`; both loads die at the shared `mult`, so the
first-emitted load has the strictly larger denominator and, at equal `n_refs`, strictly lower
priority — it is sorted second and never gets $v0. Measured: blk=11 of v2/g6.c, off4 pri 30000 vs
off6 pri 60000 (qty_g6.log). A tie (which the qty-number tiebreak would resolve in the first
load's favour) needs `floor_log2(r0)*r0 >= 24`, i.e. a third in-block reference to the rect[2]
value = an extra instruction.

**H37-4 KILLED (instance).** Deleting the `total` local and spelling the inner loop as a genuine
`for`/`while` over the inline product — the reading of the original source that GCC 2.7.2's loop
inversion would naturally produce, given the target re-reads both halfwords at the row top AND at
the latch — costs 25 to 78 points on the current chassis (f7 = 27 with an inline guard and a
do-while; f1..f5 = 46-48; f6 = 80 at 174 insns). The damage is `complement`'s hoist position, not
the latch.

**H37-5 KILLED (instance).** Giving the single-set carrier its cross-block reference at any site
NOT dominated by the latch set (the row tail `new_y`, the `>= 0x200` arm, the fast/zero/blend arms
of the inner loop, the `LoadImage` chain-extender argument, the `factor`/`px`/`r_src` initialisers)
makes the carrier live on the guard-skip path, hence live across the outer row's
`LoadImage`/`DrawSync`; global.c then seats it callee-saved ($s1) and rotates every s-register.
34-40 across 18 spellings (p1-p5, q1/q2/q5/q8, r1-r8, s7).

**H37-6 KILLED (instance).** With the cross-block read placed inside `if (total > 0)` immediately
after the inner loop (dominated by the latch set), the latch becomes byte-exact but the carrier's
extra live range costs a $t4 <-> $t5 swap between `j` and `complement`: 6 rows, score 7, FLAT
across 10 read targets (`i`, `j`, `r`, `g`, `b`, `count`, `saved_y`, `blend_base`,
`target_color`, two-statement `+=`/`-=` forms), 8 chain-extender spellings including removing the
s21 extender entirely, and 8 `complement`/`j`/`factor` declaration and split-init spellings.

### Frontier left for s38

1. **Close the $t4 <-> $t5 swap on the h-escape chassis (7 -> 0).** The latch is already exact;
   the whole residual is that `j` wants $t4 and `complement` wants $t5 in the target, and we
   produce the reverse. This is a global.c allocno-priority question between two long-lived
   inner-loop pseudos, not a scheduling one. Probe: `BB2_ALLOC_DEBUG=1` via
   `tmp/grind/func_8003DE14/s37/qty.sh tmp/grind/func_8003DE14/s37/v6/s4.c s4`, read the allocno
   priorities of `j` and `complement`, and use `tools/ra_solver`'s global.c model +
   `inverse_compose.py classify` for a typed REACHABLE/FORECLOSED verdict before spelling more C.
   Note that on this chassis the s21 `j` chain extender is inert (u1 = 7 without it), so j's
   reg_n_refs is NOT the live lever it was on the `total`-borrow chassis.
2. **Find a byte-neutral, latch-dominated cross-block READ of the carrier that adds no live
   range.** Every read measured this session costs the $t4/$t5 rotation because the carrier must
   stay live from the latch to the read. A read whose consumer is already allocated to a register
   free on that edge (or a read that GCC sinks back into the latch block after flow has recorded
   it) would be free. Note the ordering that makes this possible at all: `reg_basic_block` and
   `reg_n_refs` are both computed in flow (before combine), so a `+ h - h` reference that combine
   later folds away still escapes the pseudo without emitting an instruction.
3. **A carrier that is single-set WITHOUT being a fresh local.** The Judge's standing constraint
   bans a fresh multi-written local and grants the `total` borrow — but H37-1 shows the `total`
   borrow can never satisfy the order predicate, because `total`'s row-top assignment is its
   second set. Candidates: restructure so `total` itself has exactly one set (guard spelled
   inline, which costs 25 on its own — f7) while retaining a dominated cross-block read; or find
   an existing variable in the body that is assigned exactly once and is legitimately readable
   after the inner loop.

## [s37] The first scheduling pass's choice of which loop-bound halfword load to emit first is decided by birthing_insn_p, which is literally reg_n_sets[dest]==1 — a latch carrier with two or more sets anywhere in the function loses adjust_priority's lift and has its load emitted FIRST, a single-set carrier keeps the lift, ties, and lets LUID order (combine's order, offset 4 first) survive.
- mechanism: tools/gcc-2.7.2/sched.c:2505-2537 birthing_insn_p returns (reg_n_sets[i] == 1) for a live REG destination; sched.c:2542-2592 adjust_priority raises such an insn's priority to max_priority when it has 0 REG_DEAD notes. Both latch loads have 0 deaths (the base register $s0 does not die), so the predicate alone decides the tie.
- probe: BB2_PRIO_DEBUG=1 BB2_SCHED_DEBUG=1 cc1 -da on the two s36 bodies (tmp/grind/func_8003DE14/s37/prio.sh). Latch load insns identified from the .combine dumps: g0 = insn 300 (off 4, pseudo 147) and insn 304 (off 6, pseudo 101 = total); hh = insn 303 (off 4, pseudo 148) and insn 307 (off 6, pseudo 117 = h).
- result: g0 (the `total` borrow, two sets) prints `SCHEDDBG ADJPRI insn=304 deaths=0 birth=0` — not birthing, no lift, order swapped. hh (the s36 `h` body) prints `birth=1` for BOTH latch loads — both lifted, tie, combine order preserved. This replaces s36's rule R1 ('escaped carrier's load is emitted second when both of its sets live inside the inner loop'), which was a coincidence of the two bodies compared: the property that matters is the COUNT of sets, not their location. Corollary the next session must not miss: the Judge-granted `total` borrow can NEVER satisfy this predicate, because `total`'s row-top assignment is its second set.
- verdict: CONFIRMED

## [s37] The latch's register seats are decided by whether the height pseudo is non-block-local, and a pseudo becomes non-block-local exactly when it is live at the end of some basic block, which requires a reference in another basic block.
- mechanism: tools/gcc-2.7.2/flow.c:1420-1430 (propagate_block) marks every pseudo live at a block end REG_BLOCK_GLOBAL; local-alloc.c:194 then skips it, so local-alloc seats only the block-local rect[2] load (it takes the first free hard reg, $v0) and global.c seats the height afterwards ($v1) — the target's map.
- probe: tmp/grind/func_8003DE14/s37/v3/p0.c: a FRESH, SINGLE-set carrier `h` at the latch with no other reference anywhere.
- result: 3/173, identical to the no-carrier body g6 — the carrier stayed block-local and bought nothing. Adding exactly one cross-block read of the same single-set carrier flipped both the order and the seats to the target's in every spelling measured (rowdiff rows 133 `lh $v0,4($s0)`, 134 `lh $v1,6($s0)`, 136 `mult $v0,$v1` all MATCH). Supporting pass-ordering fact: reg_basic_block and reg_n_refs are both computed in flow, which runs BEFORE combine, so a `+ h - h` reference that combine later folds away still escapes the pseudo without emitting an instruction.
- verdict: CONFIRMED

## [s37] On a latch whose two loop-bound halfword loads are both block-local pseudos with equal reg_n_refs and a common death at the shared mult, qty_compare_1 gives the later-born load strictly higher priority, so the first-emitted load is sorted second and cannot receive $v0.
- mechanism: tools/gcc-2.7.2/local-alloc.c:1669-1684 ranks quantities by floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth). Both loads die at the mult, so the first-emitted load's denominator is strictly larger; the qty-number tiebreak (which would favour the earlier birth) is only reached on an exact tie.
- probe: BB2_QTY_DEBUG=1 on tmp/grind/func_8003DE14/s37/v2/g6.c (the no-carrier latch), artifact tmp/grind/func_8003DE14/s37/qty_g6.log.
- result: blk=11 prints qty=1 reg1=150 (off 6) birth=6 death=8 refs=6 -> pri 60000 -> ord=0 -> got=2 ($v0); qty=0 reg1=147 (off 4) birth=4 death=8 refs=6 -> pri 30000 -> ord=2 -> got=3 ($v1). A tie would need floor_log2(r0)*r0 >= 24 while loop-depth weighting quantises r0 to multiples of 3 (2 refs -> 6 -> 12, 3 refs -> 9 -> 27), i.e. a third in-block reference to the rect[2] value, which is an extra instruction the target does not have. This is why the no-carrier body g6 scores 3 with its latch a pure $v0/$v1 rename of the target's.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); tmp/grind/func_8003DE14/s37/v2/g6.c, the 2/173 candidate chassis with the latch carrier removed; FAKE constructs present: gm named intermediate, s21 j chain extender; no latch carrier.
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1669

## [s37] Deleting the `total` local and spelling the inner loop as a for/while over the inline product rect[2]*rect[3] — the reading of the original source that GCC 2.7.2's loop inversion would naturally produce, given the target re-reads both halfwords at the row top AND at the latch — costs 25 to 78 points on the current chassis.
- mechanism: With `total` gone the `if (total > 0)` block disappears and `complement = blend_base - factor` has to be declared at row scope, which moves its computation ahead of the loop guard instead of into the loop preheader the target uses (subu $t5,$fp,$t3 sits AFTER the blez at func_8003DE14.s:8003DF28).
- probe: Wave f, 7 whole-body rewrites (tmp/grind/func_8003DE14/s37/v1/f1..f7.c), swept with tools/sweep_variants.py.
- result: f7 (explicit inline `if (rect[2]*rect[3] > 0)` guard + do-while) = 27/173; f1..f4 (for and while forms, complement at row scope) = 46; f5 (bound operands swapped) = 48; f6 (for nested inside the explicit guard) = 80 at 174 insns. The `total` local is load-bearing for at least 25 points on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the 2/173 candidate chassis with the guard and latch rebuilt; FAKE constructs present: gm named intermediate, s21 j chain extender; the `total` staging FAKE removed by construction.

## [s37] Giving a single-set latch carrier its cross-block reference at a site NOT dominated by the latch set — the row tail new_y expression, the `>= 0x200` arm, any of the inner loop's fast/zero/blend arms, the LoadImage chain-extender argument, or the factor/px/r_src initialisers — makes the carrier live on the guard-skip path and therefore live across the outer row's LoadImage and DrawSync calls.
- mechanism: flow's liveness sees the read reachable without the latch set (when `total <= 0` the inner loop never runs), so the carrier is live at the row-loop back edge; global.c counts the calls crossed and seats it callee-saved.
- probe: Waves p, q, r and s7: 18 spellings placing the surviving `+ h - h` read at those sites (tmp/grind/func_8003DE14/s37/v3, v4, v5, v6).
- result: 34-40/173 in every case. The latch itself is correct (rowdiff row 133 `lh $v0,4($s0)` matches) but the carrier takes $s1 and rotates every s-register: $s1->$s2, $s2->$s3, $s7->$s8, and the blend arm's mult scheduling shifts with it. Also banked: `x += h; x -= h;` and `x = x + (h - h);` fold at the TREE level and leave no reference (back to 3), and `total = total + h - h;` is dead-store-eliminated because total is dead after the inner loop (also 3) — the only spelling that survives to flow is `x = x + h - h;`.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the 2/173 candidate chassis with the latch carrier replaced by a fresh single-set `h`; FAKE constructs present: gm named intermediate, s21 j chain extender, the single-set h carrier and its fold-away read.

## [s37] With the cross-block read placed inside `if (total > 0)` immediately after the inner loop, so the read is dominated by the latch set, the latch becomes byte-exact and the residual collapses to a $t4 <-> $t5 swap between `j` and `complement`, which is flat at 7 across 26 read-target, chain-extender and declaration spellings.
- mechanism: The carrier is now live only from the latch to the loop-exit block, crosses no call, and global.c gives it a caller-saved temp — but it still consumes one more global allocno inside the inner loop's register pressure, and `j` and `complement` exchange $t4 and $t5 relative to the target.
- probe: Waves s, t, u, v: read targets i/j/r/g/b/count/saved_y/blend_base/target_color and two-statement forms; chain-extender spellings including its complete removal; complement/j/factor declaration, split-init and inlining spellings (tmp/grind/func_8003DE14/s37/v6, v7, v8, v9).
- result: 7/173 for s4, t1, t2, t6, t7, t9, u1, u3, u5, u8, v1, v5, v6, v7 — 14 distinct bodies at the same 6 rows (71 `subu $t5,$s8,$t3`, 95/103/111 `mult ...,$t5`, 137 `addiu $t4,$t4,1`, 139 `slt $v0,$t4,$t6`). Reading into j costs 3 more (10) because it perturbs j's own refs; blend_base/count/saved_y/target_color cost 11-15 more. Side finding: u1 removes the s21 `j` chain extender entirely and is still 7, so on this chassis that FAKE construct is inert — it is worth 7 points only on the `total`-borrow chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11; the 2/173 candidate chassis with the latch carrier replaced by a fresh single-set `h` read once inside the area guard; FAKE constructs present: gm named intermediate, the single-set h carrier and its fold-away read, s21 j chain extender (present in 13 of the 14, absent in u1).

## s38 (structural) hypotheses

**H38-1 CONFIRMED — the function matches.** Supplying the third in-block reference to the rect[2]
load pseudo that s37's H37-3 class kill named as the escape condition, via a combine-foldable
detour on the loop bound (`while (j < rect[2] * rect[3] + rect[2] - rect[2])`), flips
`qty_compare_1`'s ordering of the two halfword loads and produces the target latch with NO carrier
of any kind. Score 0/173, rules_dropped 0, full-tree `verify-oracle` SHA1 == oracle.

**H38-2 KILLED (instance).** Three of the eight detour spellings are folded before flow and are
inert (score 3, identical to the detour-free chassis): the parenthesised `A + (rect[2] - rect[2])`,
the inside-the-multiplicand `(rect[2] + rect[2] - rect[2]) * rect[3]`, and the detour written on
rect[3] instead of rect[2]. Two more (the two-statement `j = j + rect[2] - rect[2];` and its
compound-assignment split) survive but land the extra refs on `j`'s allocno and cost 12.

## [s38] Supplying a third in-block reference to the rect[2] load pseudo through an algebraically-null detour that combine folds away raises its flow-counted reg_n_refs without emitting an instruction, flipping qty_compare_1's ranking so the earlier-born load is sorted first and seated $v0 — the target's latch — on the no-carrier chassis.
- mechanism: tools/gcc-2.7.2/local-alloc.c:1669-1684 qty_compare_1 ranks quantities by floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth). Both bound loads die at the shared mult, so the earlier-born rect[2] load has the larger denominator: at 2 refs each (weighted x3 = 6) it scores 2*6/4 = 3 against the rect[3] load's 2*6/2 = 6 and is sorted second. The detour's two extra reads CSE onto the same pseudo before flow (reg_n_refs is computed in flow, which runs before combine — s37 H37-2), giving 4 refs (weighted 12) and 3*12/4 = 9 > 6.
- probe: 8 spellings in tmp/grind/func_8003DE14/s38/v1/ swept with tools/sweep_variants.py, then the winner applied to src/code6cac_c2.c and dumped with BB2_QTY_DEBUG/BB2_ALLOC_DEBUG (tmp/grind/func_8003DE14/s38/qty.sh -> qty_win.log).
- result: a1/a2/a7 = 0/173 at 173 build insns (target 173), rules_dropped 0; `verify-oracle` on the whole tree prints ok=true with build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (tmp/grind/func_8003DE14/s38/verify_oracle.log). qty_win.log:623-624 confirms the predicted flip: reg1=147 (offset 4) refs=12 -> ord=0 -> got=2 ($v0); reg1=150 (offset 6) refs=6 -> ord=1 -> got=3 ($v1); the detour-free s37/qty_g6.log has reg1=147 refs=6 -> got=3. Zero bytes materialize (173 with and without the detour), satisfying the combine-foldable chain-extender clause's extra prerequisite.
- verdict: CONFIRMED

## [s38] Three of the eight detour spellings are folded at the tree level (before flow) and are inert, and the two statement-form spellings land their extra references on `j`'s allocno instead of the rect[2] load's.
- mechanism: GCC 2.7.2's fold() collapses a parenthesised `x - x` sub-expression and a compound-assignment round trip at the tree level, so no reference ever reaches flow; and a detour written as a separate statement assigning to `j` adds the refs to `j`'s quantity, changing the $t4/$t5 seat pair the s21 extender is tuned for.
- probe: tmp/grind/func_8003DE14/s38/v1/a1-a8.c, swept with tools/sweep_variants.py in one call.
- result: a6 (`A + (rect[2] - rect[2])`) = 3, a5 (`(rect[2] + rect[2] - rect[2]) * rect[3]`) = 3, a8 (detour on rect[3]) = 3 — all identical to the detour-free chassis; a3 (`j = j + rect[2] - rect[2];`) = 12 and a4 (the two-statement split) = 12. Only the flat left-to-right in-expression form survives to flow, which is the same fold boundary s37 measured for the carrier read (`x = x + h - h` survives, `x += h; x -= h;` does not). Banked as rejected/s38-*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 2026-09-11 (post -mel, post -msoft-float); the s37/v2/g6.c no-carrier chassis (candidate.c with the latch carrier removed); FAKE constructs present: gm named intermediate, s21 j chain extender; no latch carrier.
