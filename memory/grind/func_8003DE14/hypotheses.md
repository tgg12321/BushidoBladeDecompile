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
