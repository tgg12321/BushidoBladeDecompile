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

## [s4] H12 — decomp-permuter random restructuring of the v21 chassis reaches the disc-allocation residual
- **statement:** the residual 7 is a single-pseudo allocation difference (`disc`
  in $v1/$3 where target uses $a2/$6) plus one slot-88 exit-shape difference.
  decomp-permuter's random pass mutates statement/expression structure far more
  aggressively than a human enumerates, so if ANY ordinary-C restructuring of
  the function moves `disc` off $3 without costing an instruction, a long random
  campaign on the v21 chassis should surface it.
- **mechanism:** permuter's randomizer performs (among others) temp-variable
  introduction/removal, expression re-association, statement reordering,
  conditional inversion, and block splitting — exactly the class of edits that
  s3 proved CAN move this function's allocation (the `sqrt_val <<= 9` /
  `num1` edits were of that class and moved the floor 30 -> 7). If the class is
  productive at all beyond v21, sampling it densely finds a member.
- **probe:** a full-TU permuter workspace was built for this function
  (`tmp/perm_d518`, recipe banked below) whose base.c is the preprocessed
  `src/code6cac_b.c` carrying the v21 candidate; its validation diff reproduces
  EXACTLY the known 7-slot residual (154 insns == 154, differing words only at
  slots 85-89/91/93/97). Campaign
  `tools/permuter_campaign.py launch --func func_8002D518 --dir tmp/perm_d518
  --label s4-v21-chassis -j 6`, permuter base score 35. Ran 1132 s wall,
  **33,881 iterations**, two `wait` windows, harvested with `--stop`.
- **result:** **ZERO finds.** Best score across all 33,881 samples is exactly the
  base 35 (8,286 samples tie it; the next distinct scores up are 39 x21,
  40 x11, 45 x347, 50 x54 — i.e. the neighbourhood is strictly uphill).
  Nothing ever scored below base.
- **verdict: KILLED.** The v21 chassis is a strict local minimum for
  decomp-permuter's random pass. Random restructuring is not the lever for the
  `disc` allocation; the remaining 7 is not reachable by generic mutation and
  needs a targeted structural insight (frontier items (a)/(b) below).

## [s4] H13 — the cross-product of guard-shape x exit-shape x table-index-operand spellings contains a winner
- **statement:** s2/s3 measured those three axes ONE AT A TIME. Their
  interaction is untested, and the `disc` allocation is a whole-block property,
  so a combination could win where each single change is neutral or worse.
- **mechanism:** the three sites are (1) the outer `disc < 0` arm's exit shape
  (`result = 0;` fall-through vs `goto` the shared end vs a real `return 0;`),
  which controls whether jump2 cross-jumps and therefore the insn count; (2) the
  inner LZCS guard shape (`if (disc < 0) {lzcr=0;} else {island(ud);}` vs the
  same tested on `(s32)ud` vs the inverted `if (disc >= 0)` form vs the island
  reading `disc` instead of `ud`), which controls which pseudo the bltz reads
  and hence `disc`'s live range; (3) the slow-path table index reading `ud` vs
  `(u32)disc`, which controls whether `disc` has a use after the copy.
  Together they determine `disc`'s live length and therefore its global.c
  allocno priority.
- **probe:** a directed PERM chassis (`tmp/perm_d518b/base.c`, banked at
  `tmp/grind/func_8002D518/s4/directed_chassis_base.c`) encoding site 1 as a
  3-way `PERM_GENERAL`, site 2 as a 4-way `PERM_GENERAL` and site 3 as a 2-way
  `PERM_GENERAL` — the exhaustive 3x4x2 = 24-point cross-product. Campaign
  `--label s4-directed-guard-perm`; permuter enumerated all 24 and exited.
- **result:** **all 24 combinations measured, none below base 35.** The
  distribution is 35 (x6, the base-equivalent set), 230/235 (x7), 430 (x2),
  545 (x3), 605, 740 (x3), 800. Per-iteration scores banked at
  `tmp/grind/func_8002D518/s4/campaign_directed_scores.txt`.
- **verdict: KILLED.** The guard/exit/index spelling space is exhausted as a
  cross-product, not merely one axis at a time. Six spellings are exactly
  base-equivalent and eighteen are strictly worse; no interaction effect exists.
  Do NOT re-open any of these three axes.

## [s4] decomp-permuter's random pass, run long on the v21 chassis, reaches the residual 7 (the disc-in-$v1-vs-$a2 allocation plus the slot-88 exit shape).
- mechanism: permuter's randomizer performs temp-variable introduction/removal, expression re-association, statement reordering, conditional inversion and block splitting - exactly the edit class that s3 proved CAN move this function's allocation (the `sqrt_val <<= 9` and `num1` edits were of that class and took the floor 30 -> 7). If that class is productive anywhere beyond v21, dense sampling should find a member.
- probe: Built and VALIDATED a full-TU permuter workspace (tmp/perm_d518): base.c = preprocessed src/code6cac_b.c carrying the v21 candidate; compile.sh mirrors engine/buildconfig.py exactly (cc1 -O2 -G0 -mel, prologue_fix, maspsx with --expand-lb, multu_pad) and extracts the func_8002D518 region; target.o assembled from asm/funcs/func_8002D518.s plus the r3000-patched permuter prelude. Self-validation prints 'base insns: 154  target: 154' and a diff that is EXACTLY the known 7-slot residual, so the permuter metric is the honest residual. Campaign `permuter_campaign.py launch --func func_8002D518 --dir tmp/perm_d518 --label s4-v21-chassis -j 6`, permuter base score 35, two in-turn `wait` windows, harvested with --stop.
- result: 33,881 iterations / 1,132 s wall / ZERO finds. Best score across the entire run equals the base 35 (8,286 samples tie it). The next distinct scores upward are 39 x21, 40 x11, 45 x347, 50 x54 - the neighbourhood is strictly uphill in every direction the randomizer can reach. Nothing ever scored below base.
- verdict: KILLED

## [s4] The INTERACTION of the three guard-region axes contains a winner even though each axis is neutral or worse alone: {outer `disc < 0` arm exit shape} x {inner LZCS guard shape} x {slow-path table-index operand}.
- mechanism: s2 and s3 measured these three axes one at a time. `disc`'s live range - and therefore its global.c allocno priority, which s3 identified as the reason pseudo 117 is allocated first and takes its recorded $3 preference - is a whole-block property, so a combination could in principle win where each single change does not. Site 1 controls whether jump2 cross-jumps the return-0 block (the insn-count risk); site 2 controls which pseudo the inner bltz reads and hence how far past the copy disc stays live; site 3 controls whether disc has any use after the copy at all.
- probe: Directed PERM chassis tmp/perm_d518b/base.c: site 1 a 3-way PERM_GENERAL (`result = 0;` | `{ result = 0; goto done; }` | `return 0;`), site 2 a 4-way PERM_GENERAL (`if (disc < 0) {lzcr=0;} else {island(ud);}` | the same tested on `(s32)ud` | the inverted `if (disc >= 0)` form | the island reading `disc` instead of `ud`), site 3 a 2-way PERM_GENERAL (`ud` | `(u32)disc`). Campaign label s4-directed-guard-perm; permuter enumerated the full 3x4x2 = 24-point cross-product and exited.
- result: All 24 combinations measured; NONE below the base 35. Six are exactly base-equivalent, eighteen strictly worse (230, 235, 430, 545, 605, 740, 800). Per-iteration scores banked at tmp/grind/func_8002D518/s4/campaign_directed_scores.txt; the chassis itself is banked as a rejected form.
- verdict: KILLED

---

## s5 hypotheses (forensics modality) — 3 KILLED, 1 CONFIRMED

### H5.1 — CONFIRMED. `disc`'s `$3` preference is inherited from the discriminant's second multiply via global.c set_preference + expand_preferences.
* **Mechanism.** `set_preference` (global.c:1671, called at global.c:1484) reduces
  a non-copy SET's source with `src = XEXP (src, 0)` when the RHS format starts
  with `'e'`. For `(set (reg 121) (mult (reg 119) (reg 120)))` that is reg 119 —
  `dist_sq >> 9` — which local-alloc had placed in $3. So allocno 121 gets a
  preference for $3. `expand_preferences` (global.c:829) then unions 121's
  preference into 117 because the defining subu `(set (reg 117) (minus (reg 118)
  (reg 121)))` carries `REG_DEAD (reg 121)` and 117/121 do not conflict.
* **Probe.** Read `.rtl`/`.lreg`/`.greg` for the base chassis; matched the
  `;; 117 preferences: 3` line to the `;; Register 119 in 3.` local-alloc
  disposition and to the two global.c routines' source.
* **Result.** Confirmed, and immediately demoted to irrelevant by H5.2.

### H5.2 — KILLED. Removing 117's `$3` preference moves `disc` off `$3`.
(This was s4 frontier item (iii), the "re-associate the discriminant" probe,
never previously measured.)
* **Mechanism claimed.** Swap the multiply operands so `set_preference` picks up
  reg 120 (`c_val << 2`, in $2) instead of reg 119 ($3). $2 is in 117's
  hard-conflict set, so `prune_preferences` would drop it and 117 would allocate
  with no preference at all.
* **Probe.** `s32 disc = (dot2_9 * dot2_9) - ((c_val << 2) * (dist_sq >> 9));`
  applied to src/code6cac_b.c; `sandbox func_8002D518 --disable all`.
* **Result.** **score 10** (base 7), build_insns 144 == target 144. KILLED, and
  killed *structurally*, not just empirically: `config/mips/mips.h` defines no
  `REG_ALLOC_ORDER`, so `find_reg`'s no-preference fallback is the ascending
  hard-reg scan, which — with $2 hard-conflicted — also yields $3. The
  preference and the default scan agree; the preference axis cannot move disc.
  Banked `rejected/mult-operand-swap-score10.c`.

### H5.3 — CONFIRMED (pass attribution). The pass that kills `disc`'s live range is **cse.c**, via `make_regs_eqv`'s `qty_first_reg` canonicalisation — not global.c, not combine, not flow.
* **Mechanism.** `make_regs_eqv(new=132 ud, old=117 disc)` promotes `ud` to the
  quantity's canonical register when `ud` outlives the current cse extended basic
  block AND `uid_cuid[regno_last_uid[ud]] > uid_cuid[regno_last_uid[disc]]`. Both
  hold (ud feeds the `__asm__` island and `ud >> shift` in the join block; disc's
  last pre-cse use is the bltz), so every subsequent reference to the quantity —
  including the inner LZCS guard — is rewritten to `ud`.
* **Probe.** `.rtl` line 9946 has `(ge:SI (reg/v:SI 117))`; `.cse` line 8943 has
  `(ge:SI (reg/v:SI 132))`. Per-pass reference counts pin the change to cse
  exactly (117: 30→27, 132: 11→12 at rtl/jump → cse; flat before and after).
* **Result.** Confirmed. This supersedes the s1/s2 attribution of this region to
  *combine* — combine folds the *copy insn* when the guard is spelled as an
  init-then-overwrite, which is a different (already-solved) question. The
  register that the surviving copy's CONSUMERS read is chosen by cse.

### H5.4 — KILLED. Making `disc` outlive `ud` (so cse keeps `disc` canonical) buys the longer live range the frontier wanted.
* **Mechanism claimed.** Flip cse's `make_regs_eqv` predicate by shortening
  `ud`'s last use, so `disc` stays `qty_first_reg`, the bltz keeps reading disc,
  and disc's live range grows past the copy → lower allocno priority → later
  allocation → a better register.
* **Probe.** `(&D_8008D118)[(u32)disc >> shift]` instead of `[ud >> shift]`
  (so `ud`'s last use is the island). Measured, then dumped and read.
* **Result.** **score 7** — a tie, no gain — and a strictly worse platform.
  The predicate did flip, but cse then rewrote *every* `ud` use back to `disc`
  and allocno **132 disappeared** (`;; 23 regs to allocate`, not 24). 117 absorbed
  its refs (7/15, pri 9333, position 4 instead of 1) and moved $3 → $4.
  Since allocno 132 is the *only* allocno in this function that can ever block
  hard reg $4 for disc (see E4), deleting it forecloses $6 permanently.
  Banked `rejected/srlv-reads-disc-merges-away-ud-allocno-score7.c`.

### H5.5 — KILLED (arithmetic, no build needed). `disc` can be driven into `$6` by lowering its allocno priority / lengthening its live range.
This is the s1–s4 frontier head. It is now closed in closed form.
* **Mechanism.** `find_reg` blocks a hard reg only via 117's own hard-reg
  conflicts or via a hard reg already held by a CONFLICTING allocno.
  $6 needs 2,3,4,5 all blocked. The only $5-holding allocno that conflicts with
  117 is **116 (`dist_sq`)**, at position **22 of 24** with priority 909.
* **Probe.** Reproduced `allocno_compare`'s
  `floor_log2(refs)*refs/live_length` ranking by hand from `.lreg`; it matches
  the printed `;; 24 regs to allocate:` order exactly, ties included, so the
  model is validated. Then solved for the live_length that would put 117 below
  116: with 6 refs, `floor_log2(6)*6/L < 3/33` → **L > 132 insns**.
* **Result.** KILLED. A live range of 132+ insns is unreachable in a 144-insn
  function whose `disc` is computed at slot ~84. No spelling, no permuter find,
  no statement reordering can satisfy it. Priority is therefore the WRONG axis;
  the requirement is a new CONFLICT EDGE from disc to an early $5 holder
  (allocno 123 `result`, or 76/77), which no current source shape produces.

## [s5] pseudo 117 (`disc`) gets `;; 117 preferences: 3` by inheritance from the discriminant's second multiply, not from any direct copy.
- mechanism: global.c:set_preference (called at global.c:1484) reduces a non-copy SET source with `src = XEXP (src, 0)` when the RHS rtx format starts with 'e'. For `(set (reg 121) (mult (reg 119) (reg 120)))` that is reg 119 = `dist_sq >> 9`, which local-alloc placed in $3 (`.lreg`: `;; Register 119 in 3.`). global.c:expand_preferences (global.c:829) then unions allocno 121's preference into 117, because the defining subu `(set (reg 117) (minus (reg 118) (reg 121)))` carries `REG_DEAD (reg 121)` and 117/121 do not conflict.
- probe: pwsh tools/grinder/dump.ps1 func_8002D518; read .lreg (pseudo RTL + local-alloc dispositions) and .greg (allocno list, conflicts, preferences) slices, cross-checked against tools/gcc-2.7.2/global.c source.
- result: Confirmed exactly: the $3 preference chain is set_preference(121 <- reg119@$3) -> expand_preferences(121 -> 117) -> find_reg allocates 117 first and takes $3.
- verdict: CONFIRMED

## [s5] Removing 117's $3 preference (by re-associating the discriminant so the multiply's FIRST operand is the $2-resident pseudo) moves `disc` off $3. This was s4 frontier item (iii), never previously measured.
- mechanism: Swapping the operands makes set_preference pick reg 120 (`c_val << 2`, local-allocated to $2) instead of reg 119 ($3). $2 is already in 117's hard-conflict set, so prune_preferences drops it and 117 allocates with no preference at all.
- probe: `s32 disc = (dot2_9 * dot2_9) - ((c_val << 2) * (dist_sq >> 9));` applied to src/code6cac_b.c:1122ff; `wteng main sandbox func_8002D518 --disable all`.
- result: score 10 (base 7), build_insns 144 == target_insns 144. WORSE. And killed structurally, not just empirically: config/mips/mips.h defines no REG_ALLOC_ORDER, so find_reg's no-preference fallback is the ascending hard-reg scan, which with $2 hard-conflicted also yields $3. The preference and the default scan agree, so the preference axis can never move disc. Banked memory/grind/func_8002D518/rejected/mult-operand-swap-score10.c.
- verdict: KILLED

## [s5] The pass that makes the inner LZCS guard read `ud` (pseudo 132) instead of `disc` (pseudo 117) — thereby collapsing disc's live range to 6 insns — is cse.c, via make_regs_eqv's qty_first_reg canonicalisation. It is NOT combine (which s1/s2 attributed this region to), not flow, not global.
- mechanism: cse.c:make_regs_eqv(new=132, old=117) promotes `ud` to the quantity's canonical register when `uid_cuid[regno_last_uid[132]] > cse_basic_block_end` (or its first use precedes the block) AND `uid_cuid[regno_last_uid[132]] > uid_cuid[regno_last_uid[117]]`. Both hold: `ud` feeds the __asm__ island and `ud >> shift` in the join block, while `disc`'s last pre-cse use is the bltz. cse then rewrites every later reference to the quantity, including the guard's jump_insn.
- probe: Diffed the guard insn across dumps: .rtl line 9946 `(if_then_else (ge:SI (reg/v:SI 117)) ...)` vs .cse line 8943 `(ge:SI (reg/v:SI 132))`. Per-pass reference counts pin the single substitution to cse (117: 30 -> 27 and 132: 11 -> 12 between .rtl/.jump and .cse; flat through loop/cse2/flow/combine/sched/lreg).
- result: Confirmed. Supersedes the s1/s2 attribution: combine folds the COPY INSN when the guard is spelled init-then-overwrite (already-solved, different question); which register the surviving copy's CONSUMERS read is chosen by cse.
- verdict: CONFIRMED

## [s5] Flipping cse's make_regs_eqv predicate — so `disc` stays canonical and the bltz keeps reading it — buys the longer live range the s1-s4 frontier wanted.
- mechanism: Shorten `ud`'s last use below `disc`'s by indexing the slow-path table with disc, so condition (b) of make_regs_eqv fails and 117 remains qty_first_reg; disc then stays live past the copy, lowering its allocno priority and pushing it later in allocno_order.
- probe: `s32 tval = (&D_8008D118)[(u32)disc >> shift];` instead of `[ud >> shift]`; sandbox measure, then re-dump and read .greg/.lreg.
- result: score 7 — a TIE, no gain — and a strictly worse platform. The predicate did flip, but cse then rewrote every `ud` use back to disc and allocno 132 DISAPPEARED (`;; 23 regs to allocate`, not 24). 117 absorbed its refs (7 refs / 15 insns, priority 9333, position 4 instead of 1) and moved $3 -> $4; its conflict set grew to `108 116 117 131 2 3 12 29`. Since allocno 132 is the only allocno in this function that can ever block hard reg $4 for disc, deleting it forecloses $6 permanently. Banked memory/grind/func_8002D518/rejected/srlv-reads-disc-merges-away-ud-allocno-score7.c.
- verdict: KILLED

## [s5] THE s1-s4 FRONTIER HEAD: `disc` can be driven into $a2/$6 by lowering its allocno priority / lengthening its live range so it sorts below the allocnos that take $3/$4/$5.
- mechanism: global.c:allocno_compare sorts descending by floor_log2(n_refs)*n_refs/live_length. global.c:find_reg blocks a hard reg for allocno A only via A's own hard-reg conflict set or via a hard reg already assigned to an allocno that CONFLICTS with A (non-conflicting allocnos freely share a reg). $6 therefore requires $2,$3,$4,$5 all blocked at the moment 117 is allocated.
- probe: Reproduced allocno_compare's ranking by hand from the .lreg 'Register N used R times across L insns' lines; it matches the printed `;; 24 regs to allocate: 117 122 72 121 78 79 123 76 77 132 157 131 105 104 107 75 74 106 118 103 102 116 108 73` EXACTLY, including the 8889 (78/79/123) and 8000 (76/77) ties broken by ascending allocno index — so the model is validated. Then solved for the live_length that would put 117 below the $5 holder it conflicts with.
- result: KILLED in closed form. The ONLY $5-holding allocno that conflicts with 117 is 116 (`dist_sq`), at position 22 of 24 with priority 909 (3 refs / 33 insns). Putting 117 below it requires floor_log2(6)*6/L < 3/33, i.e. live_length > 132 insns, in a 144-insn function whose `disc` is not even computed until slot ~84. Unreachable by any spelling, any permuter find, any statement reordering. Priority is the wrong axis entirely.
- verdict: KILLED
