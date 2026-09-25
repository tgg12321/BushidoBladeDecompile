# Scorer: hidden zero runs and mixed asm blocks (design proposal, 2026-09-25)

Status: **PROPOSAL.** Nothing here is implemented. Every change below alters
what the cheat-invisible sandbox measures, so each one needs the owner answer
listed in § Policy questions before any `engine:` change lands. The landing
change also needs a layer-2 cheat-reviewer, because it changes cheat-stripping.

## 1. The two defects

**(a) Back-to-back `nop`s are invisible to the scorer.** `engine/score.py`
disassembles with `objdump -d`/`-dr` without `-z`. Without `-z`, objdump
replaces any run of zero bytes of 8 bytes or more with a bare `...` line. On
MIPS a zero word is `nop`, so any stretch of two or more consecutive `nop`s
disappears from both the target and our build before they are compared. A
missing or extra `nop` inside such a stretch never counts. A single `nop` is
still shown.

**(b) Mixed asm blocks are kept whole.** `engine/inlineasm.py` strips every
`__asm__` statement that has no cop2/BIOS/HW instruction. A statement that has
at least one such instruction is kept as written, including any ordinary GPR
instructions in the same statement (`move $12,%0`, `addu $t4,…`, `lw`, `nop`).
So the same `nop` is treated two ways depending on where it is written. As its
own statement, it is stripped and should cost score. Joined into a statement
that also holds a cop2 instruction, it is kept and costs nothing.

**How they interact.** The scorer ruling (inline-asm-policy.md § Scorer ruling
2026-09-25, engine-change requirement 1) asked why func_80018300 scores 0
although its grant row lists two bare `gte_nop` islands, which the sandbox
strips. The answer is (a). Each stripped `nop` sits next to another `nop`, so
the target's run and our shorter run both collapse to `...`, and the missing
2 instructions are never seen. Fixing (a) alone therefore moves func_80018300
from 0 to 2. Meanwhile (b) goes on hiding the `nop`s and `move`s written inside
joined statements.

## 2. Measurement

Scripts (scratch, gitignored): `tmp/zscan.py` (`-z` alone), `tmp/zscan2.py`
(the combined design D below), `tmp/zscan3.py` (follow-ups),
`tmp/mixed_census.py` (census of mixed blocks and stripped statements). Method:
one cheat-stripped build per `src/*.c` (the sandbox's `--disable all` recipe,
applied file-wide), with every function scored against `build/src/<stem>.o`.
Each queue item with a compilable ledger `candidate.c` was scored out of tree.

Coverage: 1,209 COMPLETED-C functions, 125 canonical-authorized functions, and
7 queue candidates. The other queue items have no `candidate.c`, or a
candidate that does not compile against the current tree (see the
2026-09-25 re-measure: func_80017848, func_800204C0, _exeque, _SsVmFlush,
func_800288C8).

**How much is hidden today.** 68 functions have target zero runs that the
scorer never sees, 512 zero words in all: 43 canonical, 22 COMPLETED-C,
3 queue.

**`-z` alone: scores that change**

| function | kind | today | `-z` | where the difference is |
|---|---|---|---|---|
| func_80018300 | canonical (C + islands) | 0 | 2 | target[265:267]: its two stripped bare `gte_nop` statements |
| InitGeom | canonical (whole body) | 0 | 2 | 2 trailing zero words after the function (padding) |
| ReadSZfifo3 | canonical (whole body) | 0 | 3 | 3 trailing padding words |
| SquareRoot0 | canonical (whole body) | 0 | 3 | 3 trailing padding words |
| SquareRoot12 | canonical (whole body) | 0 | 3 | 3 trailing padding words |
| func_800790A4 | canonical (whole body) | 0 | 6 | 6 trailing padding words |

No COMPLETED-C function changes, and none of the 7 queue candidates changes.
The 22 COMPLETED-C functions with hidden runs reproduce them exactly.

Five of the six are not code differences. The function's extent is computed
up to the next symbol, because maspsx emits no `.size`. In the target, that
range includes alignment padding before the next object. In the stripped build,
the neighbours differ (INCLUDE_ASM bodies are stripped), so the padding
differs. Without `-z`, objdump hides trailing zeros as well, which is why
these read 0 today. The same effect already produces **false 1s today**, with a
single visible trailing padding `nop`: GPU_cw, _patch_gte, SetSp and
_remove_ChgclrPAD all score 1 with byte-identical code.

**Census of mixed blocks** (`tmp/mixed_census.py`): 29 functions carry
statements that are kept whole, with 423 GPR lines inside them (ops `move`,
`nop`, `addu`, `addiu`, `lw`, `lhu`, `or`, `sll`, `sh`, `sw`). **All 29 are
canonical-authorized, and every one has a hash-matching region grant in
`tools/canonical_asm_regions.json`.** No COMPLETED-C function carries one. The
completion gate already refuses any asm island without a grant
(`engine/completion.py` `source_issues`). Two queue candidates carry ungranted
mixed blocks: func_8002A458 (4 blocks, active) and func_8002DAD0 (9 blocks,
rotated; "byte-perfect GTE/cop2 candidate, grant outside scope"). The only
cop2-free statements the sandbox strips inside a granted function are
func_80018300's two `gte_nop`s.

**Combined design D** (§ 3, measured with parts 1-3 applied together):

| function | today | `-z` alone | D |
|---|---|---|---|
| the 6 functions above | 0 | 2-6 | **0** |
| GPU_cw, _patch_gte, SetSp, _remove_ChgclrPAD | 1 (false) | 1 | **0** |
| every COMPLETED-C function (1,209) | unchanged | unchanged | unchanged |
| func_8002A458 (queue, ungranted mixed blocks) | 30 | 30 | **37** (33 without `-z`) |
| func_8002DAD0 (queue, ungranted mixed blocks) | 0 | 0 | **35** (27 without `-z`) |
| the other 5 measured queue candidates | — | unchanged | unchanged |

For the two queue rows, D strips only the GPR lines of each ungranted mixed
block and keeps its cop2 lines. Stripping those blocks whole would also delete
their cop2 instructions and overstate the gap: 102 and 89.

Side finding (not part of this design): in the first run, 155 text1b
functions read "not found in build/src/text1b.o", and one scored 14 against 0
target instructions. The rerun was clean. `score._objdump` ignores objdump's
exit status and returns empty stdout, so a transient failure (this machine's
DrvFS hiccups; see the 2026-09-25 auto-return fix) reads as "function absent"
or as an empty function. The scorer should check the return code and retry or
raise. That is an ordinary bug fix, filed here only because this study hit
it. It landed separately as 1ecfed3b5 (retry, then raise).

## 3. Proposed design

**Part 1: score every instruction.** Pass `-z` to every disassembly in
`engine/score.py` (`normalized_insns`, `func_byte_signature`, `insn_diff`), so a
missing or extra `nop` inside a run counts like any other instruction.

**Part 2: padding is not part of a function.** Before comparing, drop trailing
zero words from both sides' instruction lists, symmetrically. A delay-slot
`nop` after the final `jr` is dropped on both sides, so matching code still
compares equal. Where our build puts a real instruction in that slot and the
target has `nop`, the difference still counts (the target side ends one word
earlier). This closes the five `-z` padding regressions and today's four false
1s. Rejected alternative: taking the extent from the target and applying it to
both sides. That would read our build's following function as part of this
one.

**Part 3: one rule for stripped statements.** The sandbox's keep/strip
decision is taken per statement, by authorization, rather than by whether
a cop2 line shares the statement:

1. **Granted function (region hashes match):** every island the grant pins is
   kept as written. That includes func_80018300's bare `gte_nop`s and the 423
   GPR lines in the 29 functions above. The grant already fixes the exact text
   of each island, and any edit voids it (`completion.source_issues`). This
   gives the scorer the same view as the completion gate.
2. **Not granted:** header-exact GTE macro units are kept, unchanged
   (`engine/gtemacro.py`, 2026-09-25 scorer ruling). Every other statement is
   stripped as today. **In addition, the GPR lines of a mixed statement are
   stripped and its cop2 lines kept.** Joining a `move`/`nop`/`addu` into the
   same `__asm__` as a `mtc2` then no longer earns anything that writing it
   separately would not. The header-exact spelling (separate statements, as
   inline_o.h writes them) is still recognised by rule 2's first sentence.
3. **Draft grants.** Condition 4 / cluster Check 1 require sandbox 0 before a
   grant exists, and the scorer ruling's (C) rejected grant-hash recognition
   for exactly that reason: an ungranted body could never reach 0. The
   proposal resolves this by letting the grant path score **against the
   proposed grant**. The author computes the region hashes, the sandbox keeps
   those islands as in rule 1, and the reviewer judges the same islands the
   hashes pin. The grinder's ordinary floors, and every function without a
   proposed grant, keep rule 2.

Part 3 needs Part 1 in order to be visible, and Part 1 needs Part 3 so that it
does not penalise approved islands. They should land together, as one `engine:`
change with the tree-wide before/after distance table the scorer ruling's
requirement 3 asks for (§ 2 is that table for the current tree).

**`engine test` pins** (at landing): a missing `nop` inside a 3-`nop` run
counts 1; trailing padding of different lengths scores 0; a granted body's bare
`gte_nop` is kept, and the same body with one character edited is stripped (the
grant no longer matches); an ungranted mixed statement keeps its `mtc2` and
loses its `move`; a header-exact unit is still kept; func_80018300 stays 0.

## 4. Policy questions for the owner (plain language)

1. **Should the scorer count every instruction, including runs of blank
   `nop`s?** Today, two or more blank instructions in a row are skipped, so a
   missing or extra one is never noticed. *Recommendation: yes.* It changes no
   finished C function (all 1,209 measured), and it exposes the case below.

2. **Should filler bytes after a function's last instruction count?** They are
   alignment padding between functions, not code, and they differ between our
   test build and the real one for reasons unrelated to the function.
   *Recommendation: no.* Ignoring them removes five new false scores and four
   existing ones (GPU_cw, _patch_gte, SetSp, _remove_ChgclrPAD read 1 today
   with byte-identical code).

3. **For assembly you have already approved, should the scorer score that
   approved text exactly as approved?** The approval records a fingerprint of
   each approved block, and any edit breaks it. Today the scorer ignores the
   approval and decides block by block from the content. It deletes
   func_80018300's two approved blank-instruction lines, and only question 1's
   blind spot keeps its score at 0. *Recommendation: yes.* The scorer and the
   completion check would then agree on what was approved.

4. **Before approval, should blank or plain-register lines count just because
   they share a line with a GTE instruction?** Today `move`, `addu` and `nop`
   written in the same asm statement as a GTE instruction count as authentic,
   and written separately they count as cheats. *Recommendation: judge each
   line on its own.* Keep the GTE line, strip the plain-register lines, and
   keep Sony's exact header form as ruled on 2026-09-25. Consequence: two
   queue items read higher until they are approved or rewritten in the header
   form. func_8002A458 goes from 30 to 37, and func_8002DAD0 from 0 to 35
   (func_8002DAD0 is already waiting on an approval that is outside current
   scope).

5. **When a new assembly approval is being checked, may the scorer use the
   blocks the approval is about to lock in?** Without this, a body like
   func_80018300's could never reach the required score of 0 before approval,
   because approval requires 0 first (the reason the 2026-09-25 ruling's
   point (C) declined fingerprint recognition). *Recommendation: yes, only on
   the approval path.* The reviewer checks the same blocks the fingerprints
   will lock. Everyday grinding scores stay under question 4's rule.

Questions 1 and 2 are the engine-correctness part. Questions 3-5 decide
what the anti-cheat stripping keeps, so they are the owner's call.

## 5. Follow-ups found alongside this study (not part of this design)

- **Latent oracle gap: prologue_fix config files.** prologue_fix reads
  `tools/prologue_config.json`, `tools/delay_slot_ra_funcs.txt` and
  `tools/frame_fix_funcs.txt` in every faithful build. The queue's toolchain
  fingerprint covers all three (26f114f58). The oracle manifest
  (`engine/oracle.py` `CONFIG_FILES`) and the Makefile's `PIPELINE_DEPS`
  cover only `prologue_config.json`. The gap is latent today, because both
  `.txt` files contain only comments. Closing it touches the Makefile, which is
  substrate, and needs an oracle re-lock. It gets its own change, made with
  care and verified oracle-green, not bundled with scorer work.
