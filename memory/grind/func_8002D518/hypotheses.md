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
