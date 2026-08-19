# Hypothesis ledger — func_8002D518

## H1 — canonical island respell restores insn parity and lowers the floor
- **Statement:** respelling the pinned `.word` island in the
  func_800274BC-accepted canonical form makes the sandbox stop stripping it
  and byte-matches the island.
- **Probe [s1]:** applied the respell, ran `sandbox --disable all`.
- **Result:** floor 33 -> 30; build_insns 141 -> 144 == target; all 7 island
  words verified exact against target.
- **Verdict: CONFIRMED** (edit in place in src).

## H2 — a fresh local copy `u32 ud = disc;` reproduces target's second
##      disc register ($a0 copy in the beqz delay slot)
- **Statement:** target's `addu $a0,$a2,$zero` delay-slot copy (consumed by
  the island input + `srlv`) comes from a second C variable holding disc.
- **Probe [s1], spelling A:** `u32 ud = disc;` first stmt of the outer else;
  asm input `"r"(ud)`, shift `ud >> shift`. Result: copy FOLDED — no copy
  insn, everything reads $4, delay slot nop. Floor 30.
- **Probe [s1], spelling B:** same but asm input `"r"(disc)` and only the
  `srlv` reads `ud` (betting on the volatile-asm cse flush + cross-bb
  combine barrier). Result: identical — copy folded, floor 30, byte-identical
  build to spelling A.
- **Verdict: KILLED for these two spellings** (plain fresh-local copy, either
  operand arrangement). The hypothesis that a second VARIABLE existed remains
  live — what is killed is the plain-copy spelling. Next probes, in order:
  1. **Attribute the fold** (mandatory before more spellings):
     `pwsh tools/grinder/dump.ps1 func_8002D518`, read `.cse` (canon_reg
     replacement of ud's uses -> disc?) and `.combine` (single-use copy
     fold?). The delay-slot placement in target proves the copy survived to
     reorg in the original — identify which pass kills ours.
  2. `.claude/rules/cse-block-extension-controls-fold-span.md` escapes — the
     else block is reached via `beqz` whose target label has LABEL_NUSES==1,
     so cse1's extended block carries disc's equivalence into it. A real
     if/else restructure whose arm ends in jump+BARRIER is the one free
     escape (measured on func_8003B9D0).
  3. Sanctioned-family spellings if honest ones die: staged-value through an
     existing dead local (`staged-value-reused-variable`), param reuse
     (`threshold = disc;` — threshold is dead after the entrance chains),
     duplicated-statement-into-arms. Each needs its rule read + prerequisites
     honored BEFORE writing it.

## H3 — the remaining ~30 is one coupled register-allocation cascade
- **Statement:** disc-in-$a2 (freed by H2's surviving copy taking $a0) is
  the head of the cascade; dist_sq->$5, sqrt_val->$6, t1/t2 $3/$4 swap, and
  result->$5 follow from it plus at most small ordering levers (the two flips
  in evidence.md item 3).
- **Probe:** none yet (s1 is recon). Re-measure the whole map after H2 lands;
  only then attack the two ordering flips separately (mult-cascade dist_sq
  placement; div-prep denom sll placement) if they survive.
- **Verdict: OPEN** — do not spend spellings on #2/#3 diffs until H2 is
  resolved; they are likely coupled.

## [s1] Respelling the pinned .word LZCS/LZCR island in the func_800274BC-accepted canonical form stops the sandbox stripping it and byte-matches the island
- mechanism: engine cheat-stripper removes register-asm pins and .word GP bridges but keeps the canonical single-__asm__ cop2 form; %1 operand resolves to $a0 naturally so all 7 island words encode exactly as target
- probe: applied respell to src/code6cac_b.c, sandbox func_8002D518 --disable all, objdump word-compare of island region
- result: floor 33->30; build_insns 141->144 == target; island words 0x00806021/0x488CF000/nops/0x03A06021/0xE99F0000/0x8FA30000 all exact
- verdict: CONFIRMED

## [s1] A plain fresh local copy (u32 ud = disc;) reproduces target's second disc register (the addu $a0,$a2,$zero beqz-delay-slot copy feeding the island input and srlv)
- mechanism: original source must have carried disc in two pseudos; a fresh local is the naive spelling, but cse canon_reg / combine single-use fold coalesces the copy back into disc's pseudo
- probe: two spellings measured: (A) asm+srlv read ud; (B) asm reads disc, only srlv reads ud
- result: both fold: no copy insn emitted, delay slot stays nop, everything reads $4, floor unchanged at 30, byte-identical builds
- verdict: KILLED

## [s2] H2 RESOLVED — the disc copy survives under an if/else-spelled LZCS guard
- **statement:** target's `addu $a0,$a2,$zero` beqz-delay-slot copy comes from a
  second C variable (`u32 ud = disc;`); s1 killed the plain-if spellings, and
  the surviving question was which pass folds it and whether an honest spelling
  escapes.
- **mechanism:** the fold is COMBINE's, not cse's — `insn 248
  (set (reg/v:SI 132) (reg/v:SI 117))` is alive in `.cse` and
  `NOTE_INSN_DELETED` in `.combine`. Combine's blocks are CODE_LABEL-bounded (a
  conditional jump does NOT end one), so in the init-then-overwrite shape the
  copy, the guard branch and the island all sit in one label-to-label region.
  Respelling the guard as a real if/else puts a CODE_LABEL between the copy and
  the island; combine can no longer propagate and the copy survives to reorg.
- **probe:** variant M — `s32 lzcr; u32 ud = disc; if (disc < 0) { lzcr = 0; }
  else { <island>; lzcr = sp_tmp; }` — sandbox + objdump alignment.
- **result:** score 30, build_insns 144 (ZERO instruction cost). Copy emitted:
  build idx 90 `move $4,$3` vs target `8002D680 addu $a0,$a2,$zero`. Copy
  allocated **$a0 = target's register**, consumed by the island input (idx 98)
  and the `srlv` (idx 109) exactly as target. Only `disc`'s own register still
  differs ($3 vs target $a2/$6).
- **verdict: CONFIRMED.** Variant M is now `candidate.c` and is in src.

## [s2] H4 — source-level statement/declaration reordering can reach the 75-78 mult-shadow flip
- **statement:** the mult-shadow ordering flip (target: c_val chain then dist_sq
  addu; build: dist_sq addu first) is an emission/LUID-order effect reachable by
  reordering the `c_val` / `dist_sq` declarations or re-associating the `disc`
  expression.
- **mechanism claimed:** sched ties broken by INSN_LUID, which follows source
  statement order.
- **probe:** four variants, each a complete file measured with the
  Copy-Item runner (`tmp/grind/func_8002D518/s2/run3.ps1`), objdump-compared
  against the base build:
  A = swap the `c_val` / `dist_sq` declaration order;
  H = name the intermediate `dsq9 = dist_sq >> 9` and use it in `disc`;
  K = hoist all declarations and assign `dist_sq` / `disc` as statements;
  (plus C = swap the two multiply operands in `disc`, G = name `sum_sq`).
- **result:** A, H, K all score 30 AND are **byte-identical to the base build**
  (`diff` on the objdump slices). Codegen-neutral, every one.
- **verdict: KILLED.** Reason (attributed, not guessed): the flip is **sched2's**,
  not sched1's. `.sched` shows insns 194/196/197/200 all at `priority = 13` with
  the backward ready lists yielding TARGET order; `.sched2` shows post-reload
  priorities `197 (0x44)` vs `200 (0x18)`, and taking 197 at T-19 pushes 200
  earlier in the block. Post-reload priority is downstream of register
  allocation, so no source statement order can reach it. Banked at
  `rejected/decl-order-swap-c_val-dist_sq-codegen-neutral.c` and
  `rejected/decl-hoist-statement-assign-codegen-neutral.c`.

## [s2] H5 — reusing an existing dead local as dist_sq's carrier forces target's allocation
- **statement:** target allocates `dist_sq` to `$a1`, which is `x1_sq`'s
  register, and the addu therefore has to sit after `x1_sq`'s last use (a WAR
  edge) — reproducing the target ordering. Reusing the `x1_sq` variable itself
  to carry `dist_sq` should reproduce that allocation.
- **probe:** I2 (`x1_sq = ax_sq + az_sq;`, all later `dist_sq` uses incl. the
  `denom` retargeted) and I3 (same with `z1_sq`). Both complete files, measured.
- **result:** **I2 score 43 (build_insns 145), I3 score 57 (145)** — both add an
  instruction and diverge over ~110 more slots than the base. Strictly worse.
- **verdict: KILLED.** Banked at `rejected/reuse-x1_sq-var-for-dist_sq-score43.c`
  and `rejected/reuse-z1_sq-var-for-dist_sq-score57.c`. (Note: an earlier
  attempt at this variant was a C89 declaration-after-statement compile error
  and its 80/92 reading is NOT a measurement — the numbers above are the valid
  ones.)

## [s2] H6 — hoisting the `ud` copy above the 0x400 test escapes the fold
- **statement:** moving `u32 ud = disc;` into the outer `else` (above the
  `(u32)disc < 0x400` test) puts a CODE_LABEL between the copy and every use,
  so combine cannot propagate.
- **probe:** N (asm reads `ud`) and P (asm reads `disc`, only the `srlv` reads
  `ud`) — both complete files, measured + objdump-compared.
- **result:** both score 30 and are **byte-identical to the base build** — the
  copy still folds. In that placement cse's extended block
  (`Processing block from 144 to 289`) canonicalises the uses back to `disc`
  BEFORE combine ever runs, so moving the label earlier buys nothing.
- **verdict: KILLED.** Banked at
  `rejected/outer-hoisted-disc-copy-still-folds.c`. The working escape is H2's
  if/else guard, which puts the label between the copy and its uses rather than
  before the copy.

## [s2] src/ carried s1's candidate at s2 dispatch (the ledger floor 30 is live)
- mechanism: the Grinder commits the per-function ledger, not src/; s1's edits were never committed
- probe: read src/code6cac_b.c:1160-1218 and ran sandbox --disable all before any edit
- result: src still had the HEAD .word/register-pin island and measured floor 33, not 30; re-applying candidate.c restored 30 with build_insns 144 == target_insns 144
- verdict: KILLED

## [s2] GCC's combine pass, not cse, deletes the u32 ud = disc; copy that target keeps
- mechanism: combine's blocks are CODE_LABEL-bounded (a conditional jump does not end one), so the copy, the guard branch and the island all sit in one label-to-label region and combine propagates across them freely
- probe: pwsh tools/grinder/dump.ps1 func_8002D518; sliced the function region out of .rtl (line 9356), .cse (8396) and .combine (8565) and tracked reg/v:SI 132 (ud) against reg/v:SI 117 (disc)
- result: insn 248 (set (reg/v:SI 132) (reg/v:SI 117)) is ALIVE in .cse and NOTE_INSN_DELETED in .combine; cse's block does extend past the join (Processing block from 242 to 289) but canon_reg rewrites only the far srlv use (insn 270), leaving the guard branch (251) and the island (254) reading 132
- verdict: CONFIRMED

## [s2] Spelling the LZCS guard as a real if/else instead of init-lzcr-then-conditionally-overwrite makes the disc copy survive to reorg
- mechanism: the if/else emits a CODE_LABEL between the copy and the island, a hard boundary for combine's label-bounded blocks; this is the free escape named verbatim in .claude/rules/cse-block-extension-controls-fold-span.md
- probe: variant M: s32 lzcr; u32 ud = disc; if (disc < 0) { lzcr = 0; } else { <island>; lzcr = sp_tmp; } — sandbox --disable all plus a full 154-slot objdump alignment against asm/funcs/func_8002D518.s
- result: score 30, build_insns 144 (ZERO extra instructions). The copy materialises at build idx 90 as `move $4,$3` against target 8002D680 `addu $a0,$a2,$zero`, in the beqz delay slot; it is allocated $a0 = target's own register; it is consumed by the island input (idx 98) and the srlv (idx 109) exactly as target does. Only disc's own register still differs ($3 vs target $a2/$6).
- verdict: CONFIRMED

## [s2] Source-level statement/declaration reordering can reach the 75-78 mult-shadow ordering flip
- mechanism: claimed: scheduler ties are broken by INSN_LUID, which follows source statement order
- probe: variants A (swap the c_val/dist_sq declaration order), H (named intermediate dsq9 = dist_sq >> 9), K (all declarations hoisted, dist_sq/disc assigned as statements), plus C (swapped multiply operands in disc) and G (named sum_sq) — each a complete file, measured with the Copy-Item runner and objdump-compared against the base build
- result: A, H and K all score 30 AND are byte-identical to the base build. Attribution from the dumps: sched1 gets the order RIGHT (.sched: insns 194/196/197/200 all at priority = 13, backward ready lists give T-20:194, T-19:196, T-18:197, T-17:200 = target order); sched2 flips it (.sched2: ready list at T-19: 197 (0x44) 200 (0x18), 197 taken, pushing 200 earlier in the block). The flip is post-reload and therefore downstream of register allocation, unreachable from source statement order.
- verdict: KILLED

## [s2] Reusing an existing dead local (x1_sq or z1_sq) as dist_sq's carrier reproduces target's allocation of dist_sq to $a1 and its WAR-ordered placement
- mechanism: target writes dist_sq into $a1, which is x1_sq's register, so its addu must follow x1_sq's last use; carrying dist_sq in that same C variable should force the same reuse
- probe: variants I2 (x1_sq reused, all later dist_sq uses including denom retargeted) and I3 (z1_sq reused) — complete files, measured
- result: I2 score 43 / build_insns 145; I3 score 57 / 145. Both add an instruction and diverge over roughly 110 more slots than the base. Strictly worse. An earlier attempt at this variant was a C89 declaration-after-statement compile error; its 80/92 reading was discarded, not counted.
- verdict: KILLED

## [s2] Hoisting the ud copy above the 0x400 test escapes the fold by putting a CODE_LABEL between the copy and every use
- mechanism: the outer else's entry label would sit between the copy and the island / srlv, out of combine's reach
- probe: variants N (asm reads ud) and P (asm reads disc, only the srlv reads ud) — complete files, measured and objdump-compared
- result: both score 30 and are byte-identical to the base build; the copy still folds. In that placement cse's extended block (Processing block from 144 to 289) canonicalises the uses back to disc BEFORE combine runs, so an earlier label buys nothing.
- verdict: KILLED

## [s3] H7 — the residual cascade is reachable by VARIABLE choice in the divide tail (not by statement order)
- **statement:** the s1 residual items 2 and 3 (dist_sq $6 vs $a1, sqrt_val $2
  vs $a2, result $6 vs $a1, the t1/t2 register swap, and both ordering flips)
  are one coupled allocation cascade whose head is the divide block's live-pseudo
  count, and it is reachable by choosing which C variables carry the tail values.
- **mechanism:** target emits `sll $a2,$a2,9` — the `<<9` result occupies
  sqrt_val's own register, i.e. the source reused the variable rather than
  introducing a fresh `sq`. A fresh `sq` is one extra simultaneously-live pseudo
  through the divide chain; it takes $a1, pushes dist_sq to $6 and sqrt_val to
  $2, and the post-reload sched2 priorities that s2 measured (197 = 0x44 vs
  200 = 0x18) are computed from THOSE hard-register dependencies. Separately,
  target computes the denominator LAST before the div; naming the first
  quotient's numerator and assigning it before `denom` reproduces that order.
- **probe:** v18/v19 (`sqrt_val <<= 9;` in place, `sq` deleted), v20 (fresh `sq`
  kept, numerator named `num1` and assigned before `denom`), v21 (both),
  v22 (both numerators named), each a complete file measured with
  `& tmp/grind/func_8002D518/s3/run.ps1 -Variants ...` plus a 154-slot
  `objdump -dz` alignment.
- **result:** v18/v19 **21 / 144**; v20 **19 / 144**; v22 **19 / 144**;
  **v21 7 / 144**. greg dispositions for v21: 116 (dist_sq) in 5, 122
  (sqrt_val) in 6, 123 (result) in 5 — all three now match target. Both ordering
  flips (75-78 and 118-122) are gone.
- **verdict: CONFIRMED.** v21 is the new candidate; floor 30 -> 7.

## [s3] H8 — the LZCS guard should be spelled the way target's delay slots read (init lzcr = 0, then plain if)
- **statement:** target's `bltz $a2` carries `addu $v1,$zero,$zero` (lzcr = 0)
  in its delay slot, which is the signature of "initialise then conditionally
  overwrite" rather than s2's if/else; adopting the literal shape should be at
  least as good.
- **mechanism claimed:** the delay-slot content reflects the source shape.
- **probe:** v8 (`u32 ud = disc; s32 lzcr = 0; if (disc >= 0) { island }`), v9
  (lzcr declared first), v10 (same, island reads `disc`). All complete files,
  measured and objdump-compared.
- **result:** all three score 30 but the `u32 ud = disc;` copy **re-folds in
  combine** — slot 90 goes back to `nop` and `disc` back to `$4`. The if/else
  form keeps the copy at zero instruction cost.
- **verdict: KILLED.** The delay-slot `lzcr = 0` is a REORG artefact (reorg
  fills the branch's delay slot from the preceding insn), not evidence about the
  source shape. s2's if/else guard stays. Banked at
  `rejected/target-shaped-init-then-if-refolds-copy.c`.

## [s3] H9 — spelling the asm operand asymmetrically steers cse's canon_reg so the guard branch reads disc (s2 frontier item 3)
- **statement:** in the M form cse chose the copy (132) as the representative
  register for the guard branch; spelling the guard operand and the island input
  asymmetrically should make the branch read `disc` and change disc's live range
  across the beqz.
- **probe:** v7 — island reads `disc`, only the `srlv`/table index reads `ud`;
  complete file, measured and objdump-compared against base.
- **result:** **byte-identical to base** — same score, same words. The asm
  operand spelling has no effect on which register cse canonicalises to.
- **verdict: KILLED.** s2 frontier item 3 is closed, negative.

## [s3] H10 — the `disc < 0` arm is a real `return 0;` (target writes $v0 and jumps past the result move)
- **statement:** target slot 87/88 is `j .L8002D774 / addu $v0,$zero,$zero`,
  jumping to the epilogue label PAST `addu $v0,$a1,$zero`; that is what GCC
  emits for a `return 0;` statement, not for `result = 0;` joining the tail.
- **mechanism:** a `return` sets the return register and jumps to the function's
  return label; a `result = 0` assignment writes result's allocated register and
  falls into the join.
- **probe:** v11 (`if (disc < 0) return 0;` on the floor-30 chassis) and v24
  (the same on the v21 chassis); complete files, measured and objdump-compared.
- **result:** the emitted shape IS right, but **jump2 cross-jumps** the resulting
  2-insn block `{v0 = 0; j epilogue}` into the identical entrance return-0 block,
  deleting it: **142 insns** against target's 144. v11 = 33, v24 = **10** (v21
  base is 7). Note the entrance already contains TWO un-merged copies of that
  block in both build and target, so cross-jump is selective, not exhaustive.
- **verdict: KILLED AS SPELLED — the statement is probably TRUE but the
  construct is blocked by cross-jumping.** This is the live frontier item: find a
  structural spelling that keeps the block un-merged (see frontier).

## [s3] H11 — declaration order / operand naming around `disc` moves its allocation
- **statement:** `disc` (pseudo 117) takes `$3` because it is allocated first
  with `;; 117 preferences: 3`; re-associating or renaming the discriminant's
  operands, or moving the `result` / `sqrt_val` declarations, changes that.
- **probe:** v14 (named `dsq`/`c4`), v15 (`result` before `sqrt_val`), v16
  (`result` before `disc`), v25 (both products named, second product first);
  complete files, measured and objdump-compared.
- **result:** v14/v15/v16 all **30 / 144 and byte-identical to base**;
  v25 **27 / 144** (reorders the mult pair, strictly worse).
- **verdict: KILLED.** Declaration order and operand naming are codegen-neutral
  for `disc`; its allocation is driven by global.c's allocno priority (5 refs /
  6-insn live range = highest, allocated first) and by `set_preference`, not by
  source order.

## [s3] The s1 residual cascade (dist_sq $6 vs $a1, sqrt_val $2 vs $a2, result $6 vs $a1, the t1/t2 register swap and both ordering flips) is one coupled allocation cascade reachable by choosing which C variables carry the divide-tail values, not by statement order.
- mechanism: Target emits `sll $a2,$a2,9` - the <<9 result occupies sqrt_val's own register, i.e. the original reused the variable rather than introducing a fresh `sq`. A fresh `sq` is one extra simultaneously-live pseudo through the divide chain: it takes $a1, pushes dist_sq to $6 and sqrt_val to $2, and the post-reload sched2 priorities s2 measured (197=0x44 vs 200=0x18) are computed from exactly those hard-register dependencies. Separately, target computes the denominator LAST before the div, which naming the first numerator and assigning it before `denom` reproduces.
- probe: v18/v19 (`sqrt_val <<= 9;` in place, `sq` deleted), v20 (fresh `sq` kept, numerator named `num1` assigned before `denom`), v21 (both), v22 (both numerators named) - each a complete file measured with the Copy-Item runner plus a 154-slot objdump -dz alignment against asm/funcs/func_8002D518.s, and greg dispositions read from fresh cc1 dumps.
- result: v18/v19 = 21/144; v20 = 19/144; v22 = 19/144; v21 = 7/144. greg for v21: pseudo 116 (dist_sq) in 5, 122 (sqrt_val) in 6, 123 (result) in 5 - all three now match target. Both ordering flips (75-78 mult shadow and its 118-122 tail twin) are gone, and the whole divide/break scaffold plus the final bltz/slti/move tail are byte-identical.
- verdict: CONFIRMED

## [s3] The LZCS guard should be spelled the way target's delay slots read - initialise lzcr = 0 then a plain if - rather than s2's real if/else.
- mechanism: Claimed: target's `bltz $a2` carries `addu $v1,$zero,$zero` (lzcr = 0) in its delay slot, which is the signature of init-then-conditionally-overwrite.
- probe: v8 (`u32 ud = disc; s32 lzcr = 0; if (disc >= 0) { island }`), v9 (lzcr declared first), v10 (same, island reads disc) - complete files, measured and objdump-compared.
- result: All three score 30 but the `u32 ud = disc;` copy RE-FOLDS in combine: slot 90 goes back to nop and disc back to $4. The delay-slot lzcr = 0 is a reorg artefact (reorg fills the branch's delay slot from the preceding insn), not evidence about the source shape. s2's if/else guard is load-bearing.
- verdict: KILLED

## [s3] Spelling the asm operand and the guard operand asymmetrically steers cse's canon_reg so the guard branch reads disc instead of the copy (s2 frontier item 3).
- mechanism: Claimed: cse's canon_reg picks the quantity's representative register; making the island read disc while only the srlv reads ud would leave the branch on disc and change disc's live range across the beqz.
- probe: v7 - island reads disc, only the srlv/table index reads ud; complete file, measured and objdump-compared against base.
- result: Byte-identical to base - same score, same words. The asm operand spelling has no effect on which register cse canonicalises to.
- verdict: KILLED

## [s3] The disc < 0 arm is a real `return 0;` - target writes $v0 and jumps to the epilogue label past the result move.
- mechanism: A `return` sets the return register and jumps to the function's return label; `result = 0;` writes result's allocated register and falls into the join. Target slots 87/88 are `j .L8002D774 / addu $v0,$zero,$zero`, jumping PAST `addu $v0,$a1,$zero` at .L8002D770.
- probe: v11 (`if (disc < 0) return 0;` on the floor-30 chassis) and v24 (the same on the v21 chassis); complete files, measured and objdump-compared.
- result: The emitted shape IS right, but jump2 CROSS-JUMPS the resulting 2-insn block {v0 = 0; j epilogue} into the identical entrance return-0 block, deleting it: 142 insns against target's 144. v11 = 33, v24 = 10 (v21 base is 7). Note both build and target already carry TWO un-merged copies of that block in the entrance chain, so cross-jump is selective, not exhaustive - the merge is defeatable in principle.
- verdict: KILLED

## [s3] Declaration order or operand naming around `disc` moves its allocation off $v1.
- mechanism: Claimed: source order feeds LUIDs and therefore allocno ordering.
- probe: v14 (named dsq/c4 intermediates), v15 (result declared before sqrt_val), v16 (result declared before disc), v25 (both products named, second product first) - complete files, measured and objdump-compared.
- result: v14/v15/v16 all 30/144 AND byte-identical to base; v25 = 27/144 (reorders the mult pair, strictly worse). disc's allocation is driven by global.c allocno priority (5 refs / 6-insn live range = highest of 24, allocated first) and by set_preference, not by source order.
- verdict: KILLED

## [s3] Structural variants of the guard/copy that make the bltz read disc are free.
- mechanism: Claimed: an inverted guard or a copy duplicated into both arms puts disc on the branch without cost.
- probe: v1 (inverted guard: if (disc >= 0) { island } else { lzcr = 0; }), v2/v26 (`ud = disc;` duplicated into both arms), v4 (denom computed right after dist_sq), v6 (ud hoisted above the 0x400 test with all fast-path uses routed through it), v29 (ud dropped entirely).
- result: v1 = 32/146 (materialises the dead arm); v2 = 32/146 on the floor-30 chassis and v26 = 9/146 on the v21 chassis - it DOES put disc on the bltz but pays 2 instructions; v4 = 37/144; v6 = 31/145; v29 = 7/144 but with a NOP at slot 90 where target has `addu $a0,$a2,$zero` (the copy simply absent), so structurally strictly worse than v21 at the same score.
- verdict: KILLED
