# Canonical-asm authorization evidence packets — 2026-08-06

Prepared for owner review. **Nothing here is authorized**: no entry was added to
`inline_asm_canonical.txt`, no queue item was marked done, no build file was touched.
Every packet is default-FAIL — evidence has to affirmatively clear the
[[hand-coded-asm-recognition]] bar, and where it does not, the packet says so and
recommends AGAINST.

Populations covered:

- **Batch A — the 26 `ASM-STRUCTURAL` queue items** (honest distances 510–2989).
- **Batch B — the GTE-partial functions** on the owner shelf
  (`docs/HANDOFF-2026-08-06.md:74`), under the func_8002EA24 precedent
  (`docs/grind/decisions.md` 2026-07-30).

## Headline

**All 26 Batch-A items rate WEAK and are recommended AGAINST authorization.** They are
the *same 26 functions* the 2026-06-09 canonical-asm audit already rejected
(`.claude/rules/canonical-gate-distance-not-evidence.md` lists them by name and file,
and the file breakdown matches exactly: text1b 13, code6cac 4, code6cac_b 5,
code6cac_c_mid 1, code6cac_b2_post 1, text1b_b 1, main 1). Re-measured fresh today,
`scan_hand_coded.py` returns tier=LOW for 26/26 with score 0 or 1; the only signal that
ever fires is S4 (a load burst), which the rule classes as a tightness signal that is
*insufficient alone*. Zero functions fire S1, S2, S5, S6, S7 or S8. An independent
structural scan (`tmp/auth_signal_scan.py`) adds: 0 splat `handwritten instruction`
tags, 0 GTE/cop2 ops, 0 trapping `add/addi/sub/neg`, 0 ghost callee-save saves, 0
unsaved callee-save uses across all 26.

**Process note for the owner.** `engine/canonical.py` still carries the 2026-06-09 fix
(`NEAR_CERTAIN_DISTANCE` requires tier >= POSSIBLE before routing ASM-STRUCTURAL,
lines 39-52, 183-195), so the gate as written could not produce these verdicts today.
The queue records are therefore **stale pre-fix verdicts** carried through a `queue
regen` that also dropped the 2026-06-09 park reasons (`engine/queue.json` items keep
only func/file/distance/verdict/rules/status — the parked entries elsewhere in the file
do carry `park_reason`, so these lost theirs when they flipped to active). Suggested
owner action is not an authorization at all: re-run the gate on these 26 so they demote
to ASM-SUSPECT and return to the pure-C pool, and consider persisting the rejection
rationale so this population does not resurface a third time.

**Batch B is where the real authorizations are.** Three LIBGTE leaves rate DECISIVE and
are recommended FOR immediate sign-off; two GTE-island functions rate STRONG for the
island but remain contingent on the function otherwise matching, exactly as the
func_8002EA24 ruling requires.

## Summary table

| Function | File | Insns | Dist | Scanner | Independent signals | Rating | Recommend |
|---|---|---:|---:|---|---|---|---|
| func_80052B00 | src/text1b.c | 17 | 18 | LOW 0/8 (length-blind) | 8 splat handwritten tags; all-cop2 body; `ctc2 $t7,$7` in `jr $ra` delay | **DECISIVE** | **FOR** |
| func_80052A88 | src/text1b.c | 30 | 25 | LOW | 9 splat tags; ctc2/mtc2/lwc2/mvmva/swc2; `swc2 $11` in `jr $ra` delay | **DECISIVE** | **FOR** |
| func_80052B7C | src/text1b.c | 26 | 20 | LOW | 8 splat tags; ctc2 x8 + mvmva + swc2; `swc2 $11` in `jr $ra` delay | **DECISIVE** | **FOR** |
| func_8002BEA0 | src/code6cac_b.c | 132 | 9 | — | GTE-LZCS island (splat tag); sibling of authorized func_8001A67C/func_800274BC | STRONG (island) | FOR — contingent |
| func_8002EA24 | src/code6cac_b.c | 110 | 18 | TIGHT_C 3/8 | 3 splat tags; LZC block identical to authorized func_800274BC | STRONG (island) | FOR — contingent |
| special_camera_set_win_cam | src/code6cac_b2_post.c | 512 | 510 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8006F97C | src/text1b.c | 515 | 513 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| single_game_SetStatusUpData | src/text1b.c | 516 | 514 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8003993C | src/code6cac_c_mid.c | 526 | 524 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80079A30 | src/text1b_b.c | 535 | 533 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80034708 | src/code6cac_b.c | 544 | 542 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8005D814 | src/text1b.c | 545 | 544 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8006A880 | src/text1b.c | 552 | 550 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8005F1C8 | src/text1b.c | 564 | 562 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| calc_teasi_loc_fw | src/code6cac_b.c | 574 | 572 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| camera_set_target_zoom | src/code6cac.c | 588 | 586 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80022580 | src/code6cac.c | 621 | 619 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| saTan4GaugeMain | src/text1b.c | 622 | 620 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_800720FC | src/text1b.c | 690 | 688 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| replay_camera_attack | src/text1b.c | 698 | 696 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80030D7C | src/code6cac_b.c | 709 | 707 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_800198D0 | src/code6cac.c | 749 | 747 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8005C8A8 | src/text1b.c | 753 | 752 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8008C464 | src/main.c | 763 | 761 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_8005E54C | src/text1b.c | 799 | 798 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| motion_ShiftControl | src/text1b.c | 810 | 808 | LOW 0/8 (none) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80029454 | src/code6cac_b.c | 1025 | 1024 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| calc_loc_mat_fw_80055B60 | src/text1b.c | 1110 | 1108 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| calc_loc_mat_fw | src/code6cac_b.c | 1112 | 1110 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80023F08 | src/code6cac.c | 2983 | 2981 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |
| func_80058580 | src/text1b.c | 2991 | 2989 | LOW 1/8 (S4 front-loaded load burst) | 0 tags / 0 GTE / 0 trapping / 0 ghost-saves | WEAK | **AGAINST** |

---

# Batch B — GTE functions (DECISIVE first)

## The shelf item, identified

`docs/HANDOFF-2026-08-06.md:74` shelves "GTE-partial trio canonical authorizations when
their functions otherwise match (func_8002EA24 precedent)." Two readings exist and I
give both rather than guess silently:

1. **The three parked `ASM-PARTIAL` queue items** — `func_8002BEA0` (dist 9, 7 rules),
   `func_8002EA24` (dist 18, 10 rules), `func_80052B00` (dist 18, 1 rule). These are the
   only parked ASM-PARTIAL entries in `engine/queue.json`, all three carry GTE islands,
   and all three are blocked on precisely the "authorize the island once the rest
   matches" question the func_8002EA24 ruling poses. **This is the reading I consider
   correct.**
2. **The three LIBGTE siblings deferred by the func_80052B44 ruling** —
   `func_80052A88` / `func_80052B00` / `func_80052B7C`, which that ruling
   (`docs/grind/decisions.md` line 1805) explicitly held over: *"This authorization
   covers func_80052B44 ONLY — siblings ... each require their own per-function ruling
   when they reach the queue top."*

`func_80052B00` sits in both sets. Status today: **none of the three in reading (1) has
reached "otherwise matches,"** so the contingency in the shelf item is unmet for
func_8002BEA0 and func_8002EA24. func_80052B00 is a different case — it has no
non-GTE part at all, so the contingency can never be met by grinding and the question is
ripe now. The two remaining LIBGTE siblings are queue-active and equally ripe; I include
them because their evidence is decisive and they are the same construct the owner has
already signed off on once.

## func_80052B00 — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052B00.s · **17 instructions** ·
  queue: parked, ASM-PARTIAL, honest distance 18 (= the whole body), 1 rule
  (`regfix.txt:3411 fill_delay @ 16 <- 15`) plus 8 `register asm("$N")` pins in HEAD.
- **Signals.** Eight `lw` from `*$a0` at 0x80052B00-0x80052B1C feeding
  `ctc2 $t0-$t7, $0-$7` at 0x80052B20-0x80052B3C, every one splat-tagged
  `/* handwritten instruction */`; `jr $ra` at 0x80052B3C with **`ctc2 $t7, $7` in the
  delay slot** at 0x80052B40. Zero general-purpose computation.
- **Why it is unreachable from C** (the decisive part, and not a "we are stuck" claim):
  the granted func_80052B44 ruling established from `tools/gcc-2.7.2/reorg.c` that
  `stop_search_p` halts the delay-slot search at ASM_INPUT and `resource_conflicts_p`
  returns 1 for volatile asm — GCC 2.7.2 can **never** put a `ctc2` in a `jr $ra` delay
  slot. `ctc2` can only enter compilation as inline asm. The target bytes are therefore
  unreachable from any C input to the frozen compiler.
- **Precedent fit.** `inline_asm_canonical.txt:340` (func_80052B44, Judge-authorized
  2026-07-27) is the same construct one register-file wider; func_8007ED6C is the older
  verbatim-LIBGTE precedent. This function is *narrower* than both.
- **On the LOW scanner tier.** S1/S2/S6 target general-purpose kernels and S3/S4 need
  >= 40 instructions; a 17-instruction straight-line cop2 leaf cannot fire them. The
  func_80052B44 ruling already recorded that LOW does not disqualify this family.
- **Proposed entry (approve verbatim):**

```
func_80052B00  # LIBGTE SetRotMatrix leaf: 8x lw <- *a0 -> 8x ctc2 $0-$7 (3x3 R matrix + CR7), the last sitting IN the jr-ra delay slot at 0x80052B40. All eight cop2 ops splat-tagged 'handwritten instruction'; zero general-purpose computation. GCC 2.7.2 cannot fill a delay slot with asm (reorg.c stop_search_p halts at ASM_INPUT), so the bytes are unreachable from any C. Same construct as authorized sibling func_80052B44.
```

## func_80052A88 — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052A88.s · **30 instructions** ·
  queue: active, ASM-PARTIAL, distance 25, 1 rule.
- **Signals.** Nine splat `handwritten instruction` tags: `ctc2 $t0-$t3, $0-$3`
  (0x80052AA8-0x80052AB4), `ctc2 $t4-$t7, $4-$7` (0x80052AC8-0x80052AD4),
  `mtc2 $t0, $0` (0x80052AD8). Plus `lwc2 $1, 0x8($a1)` (0x80052ADC), two **unfilled GTE
  latency nops** (0x80052AE0/AE4), `mvmva 1,0,0,0,0` (0x80052AE8), a third latency nop,
  then `swc2 $9/$10/$11` — with **`swc2 $11, 0x8($a2)` in the `jr $ra` delay slot**
  (0x80052AFC). The `lh/lhu/sll/or` at 0x80052AB8-0x80052AC4 is the standard PsyQ
  16-bit-pair pack feeding `mtc2`.
- **Why unreachable from C.** Same reorg.c delay-slot impossibility as func_80052B00,
  here on `swc2`; `mvmva`, `lwc2` and the hand-placed GTE latency nops have no C form
  either. This is verbatim PsyQ LIBGTE (`RotTrans`-family matrix-vector multiply).
- **Precedent fit.** Directly inside the gte-wrapper carve-out
  (`.claude/rules/gte-wrapper-misroute-park.md`) and the func_80052B44 family; the
  func_80052B44 ruling named this function as needing its own sign-off.
- **Proposed entry (approve verbatim):**

```
func_80052A88  # LIBGTE matrix-vector leaf: 8x ctc2 $0-$7 + mtc2 $0 (9 splat 'handwritten instruction' tags), lwc2 $1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, then swc2 $9/$10/$11 with swc2 $11 IN the jr-ra delay slot (0x80052AFC). Verbatim PsyQ hand-written asm; the delay-slot swc2, the mvmva and the latency nops are all unreachable from C under GCC 2.7.2 (reorg.c stop_search_p). Sibling of authorized func_80052B44.
```

## func_80052B7C — DECISIVE — recommend FOR

- **File:** src/text1b.c · **asm:** asm/funcs/func_80052B7C.s · **26 instructions** ·
  queue: active, ASM-PARTIAL, distance 20, 1 rule.
- **Signals.** Eight splat-tagged `ctc2 $t0-$t7, $0-$7` (0x80052B9C-0x80052BB8) fed by
  five `lw` from `*$a0` and three `lh` from `*$a1`; `lwc2 $0/$1` (0x80052BBC/BC0), two
  unfilled GTE latency nops, `mvmva 1,0,0,0,0` (0x80052BCC), a third nop, then
  `swc2 $9/$10` and **`swc2 $11, 0x8($a3)` in the `jr $ra` delay slot** (0x80052BE0).
- **Why unreachable from C.** Identical argument to func_80052A88 — delay-slot cop2
  store plus mvmva plus hand-placed latency nops.
- **Proposed entry (approve verbatim):**

```
func_80052B7C  # LIBGTE matrix-vector leaf (4-arg form): 8x ctc2 $0-$7 (8 splat 'handwritten instruction' tags) from *a0 words + *a1 halfwords, lwc2 $0/$1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, swc2 $9/$10 and swc2 $11 IN the jr-ra delay slot (0x80052BE0). Verbatim PsyQ hand-written asm, unreachable from C under GCC 2.7.2. Sibling of authorized func_80052B44.
```

## func_8002BEA0 — STRONG for the island — recommend FOR, CONTINGENT

- **File:** src/code6cac_b.c · **asm:** asm/funcs/func_8002BEA0.s · 132 instructions ·
  queue: **parked**, ASM-PARTIAL, honest distance 9, 7 rules.
- **Signals.** One splat `handwritten instruction` tag on the GTE-LZCS island; the body
  is otherwise ordinary compiled C (GP-rel `lui/lw` global loads at 0x8002BEA0-AC,
  `mult/mflo` distance-squared, a `D_8008D118` table lookup, `break 6/7` division traps).
- **Status of the contingency.** NOT met. The park reason itself records that the GTE
  block already matches and the residual floor-4 diff is *pure-C* register allocation /
  strength reduction in the `/100` division subtree — i.e. compiled C that should match.
  Under the func_8002EA24 ruling an authorization must describe a finished function;
  this one is not finished, and the unfinished part is not the island.
- **What the owner can decide now:** nothing that requires a listing. The useful ruling
  is whether the LZCS island's clobber-list form (already settled for the sibling
  func_8002BC68 by the 2026-07-28 ruling, whose regfix.txt:607-608 `mfhi $13 -> $24`
  cheat is the same construct) is pre-approved here so the grind can bank it the moment
  the /100 subtree closes.

## func_8002EA24 — STRONG for the LZC island — recommend FOR, CONTINGENT

- **File:** src/code6cac_b.c · **asm:** asm/funcs/func_8002EA24.s · 110 instructions ·
  queue: **parked** (OWNER-ACCEPTED INCOMPLETE, standing ruling 2026-07-27), ASM-PARTIAL,
  distance 18 at park, honest floor 2 after 12 grind sessions, 10 rules.
- **Signals (verified in the 2026-07-30 ruling, re-checked here).** 3 splat
  `handwritten instruction` tags — `swc2 $26/$27` at 0x8002EA98/EA9C and `mtc2 $t4,$30`
  at 0x8002EB38; `$t4` written back-to-back with two unrelated values
  (0x8002EA74/0x8002EA90); unfilled GTE delay nops; and the LZC block at
  0x8002EB34-EB48 is instruction-for-instruction the construct the owner authorized on
  2026-06-10 for func_800274BC.
- **Status of the contingency.** NOT met, and the scope caveat from the 2026-07-30
  ruling still stands: the LZC block qualifies, the **vector/mvmva block as drafted does
  not** — it swallowed `addiu $v0, %0, 0xF8` / `0x100` (plain pointer arithmetic on a
  C-visible parameter) into hardcoded-`$N` template text, which is the
  [[inline-asm-injection]] signature. Any future authorization should be worded to cover
  the LZC island only, with the vector block narrowed to `addu $t4, %0, $zero` + cop2
  ops and the address bound as an operand.

---

# Batch A — the 26 ASM-STRUCTURAL items (all WEAK, all AGAINST)

## Method and what it found

Every function was measured three ways, fresh today:

1. `python3 tools/scan_hand_coded.py --single <func> --json` — the S1-S8 tier.
2. `tmp/auth_signal_scan.py` — an independent structural pass for the signals the
   scanner does not compute: splat `handwritten instruction` tags, GTE/cop2 opcodes,
   trapping `add/addi/sub/neg` (GCC 2.7.2 emits the `u` forms), `.word`/syscall/break,
   callee-saves used-but-unsaved and saved-but-unused, leaf-with-`$ra`-save, and
   unfilled `jr $ra` delay slots.
3. A multiply/divide pacing histogram (the S1 discriminator, widened to signed `mult`
   and `div` which the scanner's `multu`-only counter misses).

Results, uniform across all 26: **tier LOW, score 0-1**; the only firing signal is S4;
**zero** splat handwritten tags, **zero** cop2 ops, **zero** trapping arithmetic,
**zero** ghost callee-save saves, **zero** unsaved callee-save uses. Pacing is the
*opposite* of the hand-coder signature — e.g. calc_loc_mat_fw's 15 mult/mflo pairs are
spread over gaps {1:1, 2:2, 3:6, 4:6} and func_80058580's 25 pairs over {0:11, 1:8,
2:6}, where S1 requires a single uniform gap on every pair. The `break 6`/`break 7`
occurrences (special_camera_set_win_cam 8, func_80058580 12, func_8008C464 3) are GCC's
division-by-zero and overflow traps, which `engine/canonical.py:54-55` explicitly
excludes as an asm signal — they are positive evidence of *compiled* division.

Manual reads corroborate. `calc_loc_mat_fw` opens with a textbook GCC prologue saving
`$s0-$s7/$fp/$ra` and homing `$a0` to the frame (0x8002AB08-0x8002AB5C), then spills
through the body; the scratchpad constants (0x1F8002B8 etc.) are ordinary
`(void *)0x1F8002B8` material, not a hand-asm signature. `func_8008C464`'s six unfilled
`jr $ra` delay slots are six ordinary multi-return epilogues (e.g. 0x8008C960-0x8008C974:
`lw $ra/$s1/$s0` restore, `addiu $sp`, `jr $ra`, `nop`) — GCC leaves a nop whenever
nothing is available to fill, which is not the S6-class "GCC cannot express this"
discriminator.

`func_800198D0` deserves one explicit note because it is the one item with an unusual
shape: a 749-instruction **leaf** (zero `jal`). It does not save `$ra` (so S7 does not
fire), has no cop2 ops, no trapping arithmetic and no multiply pacing at all. A large
call-free dispatch body is a size anomaly, not a provenance signal.

## Recommendation for the whole batch

**AGAINST authorization, 26 of 26.** Per
`.claude/rules/canonical-gate-distance-not-evidence.md` these are large pure-C functions
whose accumulated RA/scheduling drift exceeds the distance threshold — the exact
confusion the 2026-06-09 audit diagnosed. They belong back in the decomp pool as
ASM-SUSPECT. The two largest (`func_80023F08` 2981, `func_80058580` 2989) are further
covered by [[slog-kengo-dead-end]], which already rules them multi-session pure-C work
rather than canonical-asm.

No proposed `inline_asm_canonical.txt` entry is offered for any Batch-A function. Writing
one would mean drafting a rationale the evidence does not support.

## Per-function measurements

### special_camera_set_win_cam

- **File:** src/code6cac_b2_post.c · **asm:** asm/funcs/special_camera_set_win_cam.s · **512 instructions** ·
  honest pure-C distance **510** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 527 insns, 2 spills, 9 distinct regs. S4: max load burst was 3 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  8; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80036938).
- **Multiply/divide pacing:** 8 pair(s), gap histogram {'0': 8} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8006F97C

- **File:** src/text1b.c · **asm:** asm/funcs/func_8006F97C.s · **515 instructions** ·
  honest pure-C distance **513** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 515 insns, 53 spills, 15 distinct regs. S4: max load burst was 3 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80070180).
- **Multiply/divide pacing:** 2 pair(s), gap histogram {'1': 2} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### single_game_SetStatusUpData

- **File:** src/text1b.c · **asm:** asm/funcs/single_game_SetStatusUpData.s · **516 instructions** ·
  honest pure-C distance **514** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 516 insns, 7 spills, 22 distinct regs. S4: 4 loads in 8-insn window @ insn 370.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  2; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80055940).
- **Multiply/divide pacing:** 7 pair(s), gap histogram {'0': 4, '1': 3} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8003993C

- **File:** src/code6cac_c_mid.c · **asm:** asm/funcs/func_8003993C.s · **526 instructions** ·
  honest pure-C distance **524** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 526 insns, 24 spills, 15 distinct regs. S4: 4 loads in 8-insn window @ insn 221.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  2; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8003A16C).
- **Multiply/divide pacing:** 1 pair(s), gap histogram {'0': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80079A30

- **File:** src/text1b_b.c · **asm:** asm/funcs/func_80079A30.s · **535 instructions** ·
  honest pure-C distance **533** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 548 insns, 57 spills, 14 distinct regs. S4: max load burst was 3 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8007A284).
- **Multiply/divide pacing:** 1 pair(s), gap histogram {'2': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80034708

- **File:** src/code6cac_b.c · **asm:** asm/funcs/func_80034708.s · **544 instructions** ·
  honest pure-C distance **542** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 566 insns, 10 spills, 19 distinct regs. S4: max load burst was 3 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80034F80).
- **Multiply/divide pacing:** 1 pair(s), gap histogram {'1': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8005D814

- **File:** src/text1b.c · **asm:** asm/funcs/func_8005D814.s · **545 instructions** ·
  honest pure-C distance **544** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 545 insns, 54 spills, 20 distinct regs. S4: max load burst was 2 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8005E090).
- **Multiply/divide pacing:** 11 pair(s), gap histogram {'1': 10, '6': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8006A880

- **File:** src/text1b.c · **asm:** asm/funcs/func_8006A880.s · **552 instructions** ·
  honest pure-C distance **550** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 552 insns, 64 spills, 14 distinct regs. S4: 5 loads in 8-insn window @ insn 232.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8006B118).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8005F1C8

- **File:** src/text1b.c · **asm:** asm/funcs/func_8005F1C8.s · **564 instructions** ·
  honest pure-C distance **562** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 564 insns, 66 spills, 18 distinct regs. S4: max load burst was 2 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8005FA90).
- **Multiply/divide pacing:** 8 pair(s), gap histogram {'0': 2, '1': 6} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### calc_teasi_loc_fw

- **File:** src/code6cac_b.c · **asm:** asm/funcs/calc_teasi_loc_fw.s · **574 instructions** ·
  honest pure-C distance **572** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 580 insns, 17 spills, 16 distinct regs. S4: 4 loads in 8-insn window @ insn 37.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (800283C8).
- **Multiply/divide pacing:** 6 pair(s), gap histogram {'0': 2, '1': 2, '2': 2} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### camera_set_target_zoom

- **File:** src/code6cac.c · **asm:** asm/funcs/camera_set_target_zoom.s · **588 instructions** ·
  honest pure-C distance **586** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 588 insns, 2 spills, 8 distinct regs. S4: 4 loads in 8-insn window @ insn 556.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8001D788).
- **Multiply/divide pacing:** 2 pair(s), gap histogram {'1': 2} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80022580

- **File:** src/code6cac.c · **asm:** asm/funcs/func_80022580.s · **621 instructions** ·
  honest pure-C distance **619** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 621 insns, 14 spills, 11 distinct regs. S4: 5 loads in 8-insn window @ insn 311.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80022F2C).
- **Multiply/divide pacing:** 1 pair(s), gap histogram {'2': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### saTan4GaugeMain

- **File:** src/text1b.c · **asm:** asm/funcs/saTan4GaugeMain.s · **622 instructions** ·
  honest pure-C distance **620** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 622 insns, 59 spills, 15 distinct regs. S4: 4 loads in 8-insn window @ insn 209.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8006CBCC).
- **Multiply/divide pacing:** 8 pair(s), gap histogram {'1': 8} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_800720FC

- **File:** src/text1b.c · **asm:** asm/funcs/func_800720FC.s · **690 instructions** ·
  honest pure-C distance **688** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 690 insns, 58 spills, 19 distinct regs. S4: 4 loads in 8-insn window @ insn 36.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80072BBC).
- **Multiply/divide pacing:** 2 pair(s), gap histogram {'1': 1, '7': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### replay_camera_attack

- **File:** src/text1b.c · **asm:** asm/funcs/replay_camera_attack.s · **698 instructions** ·
  honest pure-C distance **696** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 698 insns, 28 spills, 15 distinct regs. S4: 4 loads in 8-insn window @ insn 28.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80070C68).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80030D7C

- **File:** src/code6cac_b.c · **asm:** asm/funcs/func_80030D7C.s · **709 instructions** ·
  honest pure-C distance **707** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 709 insns, 12 spills, 16 distinct regs. S4: 6 loads in 8-insn window @ insn 249.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80031888).
- **Multiply/divide pacing:** 23 pair(s), gap histogram {'0': 9, '1': 9, '3': 2, '4': 3} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_800198D0

- **File:** src/code6cac.c · **asm:** asm/funcs/func_800198D0.s · **749 instructions** ·
  honest pure-C distance **747** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 749 insns, 3 spills, 19 distinct regs. S4: 4 loads in 8-insn window @ insn 286.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=True, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8001A47C).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8005C8A8

- **File:** src/text1b.c · **asm:** asm/funcs/func_8005C8A8.s · **753 instructions** ·
  honest pure-C distance **752** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 753 insns, 96 spills, 15 distinct regs. S4: max load burst was 2 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8005D464).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8008C464

- **File:** src/main.c · **asm:** asm/funcs/func_8008C464.s · **763 instructions** ·
  honest pure-C distance **761** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 763 insns, 9 spills, 8 distinct regs. S4: 5 loads in 8-insn window @ insn 564.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  3; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 6 (8008C970, 8008C9EC, 8008CD84, 8008CEA4, 8008CF54, 8008D040).
- **Multiply/divide pacing:** 1 pair(s), gap histogram {'3': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_8005E54C

- **File:** src/text1b.c · **asm:** asm/funcs/func_8005E54C.s · **799 instructions** ·
  honest pure-C distance **798** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 799 insns, 94 spills, 16 distinct regs. S4: max load burst was 2 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8005F1C0).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### motion_ShiftControl

- **File:** src/text1b.c · **asm:** asm/funcs/motion_ShiftControl.s · **810 instructions** ·
  honest pure-C distance **808** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 0/8. Fired: none.
  S3: 810 insns, 14 spills, 16 distinct regs. S4: max load burst was 3 in any 8-insn window.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80071C18).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80029454

- **File:** src/code6cac_b.c · **asm:** asm/funcs/func_80029454.s · **1025 instructions** ·
  honest pure-C distance **1024** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 1025 insns, 10 spills, 18 distinct regs. S4: 5 loads in 8-insn window @ insn 71.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8002A450).
- **Multiply/divide pacing:** 0 pair(s), gap histogram {} — no pairs to test.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### calc_loc_mat_fw_80055B60

- **File:** src/text1b.c · **asm:** asm/funcs/calc_loc_mat_fw_80055B60.s · **1110 instructions** ·
  honest pure-C distance **1108** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 1110 insns, 24 spills, 15 distinct regs. S4: 5 loads in 8-insn window @ insn 33.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80056CB0).
- **Multiply/divide pacing:** 3 pair(s), gap histogram {'0': 1, '2': 1, '3': 1} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### calc_loc_mat_fw

- **File:** src/code6cac_b.c · **asm:** asm/funcs/calc_loc_mat_fw.s · **1112 instructions** ·
  honest pure-C distance **1110** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 1112 insns, 46 spills, 20 distinct regs. S4: 5 loads in 8-insn window @ insn 364.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8002BC60).
- **Multiply/divide pacing:** 15 pair(s), gap histogram {'1': 1, '2': 2, '3': 6, '4': 6} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80023F08

- **File:** src/code6cac.c · **asm:** asm/funcs/func_80023F08.s · **2983 instructions** ·
  honest pure-C distance **2981** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 2983 insns, 36 spills, 13 distinct regs. S4: 8 loads in 8-insn window @ insn 1020.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  0; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (80026D9C).
- **Multiply/divide pacing:** 42 pair(s), gap histogram {'0': 25, '1': 3, '2': 1, '3': 3, '4': 2, '5': 5, '6': 1, '7': 2} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

### func_80058580

- **File:** src/text1b.c · **asm:** asm/funcs/func_80058580.s · **2991 instructions** ·
  honest pure-C distance **2989** · queue verdict ASM-STRUCTURAL (stale), 1 rule.
- **Scanner:** tier **LOW**, score 1/8. Fired: S4 front-loaded load burst.
  S3: 3009 insns, 18 spills, 16 distinct regs. S4: 5 loads in 8-insn window @ insn 499.
- **Independent signals:** splat handwritten tags 0; GTE/cop2 ops
  0; trapping add/addi/sub/neg 0; .word/syscall/break
  12; callee-saves used-but-unsaved none;
  saved-but-unused none; leaf=False, leaf-saves-$ra=False;
  unfilled `jr $ra` delay slots 1 (8005B434).
- **Multiply/divide pacing:** 25 pair(s), gap histogram {'0': 11, '1': 8, '2': 6} — non-uniform, S1 does not fire.
- **Rating: WEAK. Recommendation: AGAINST authorization** — return to the pure-C pool.

---

## Counts

- DECISIVE: 3 (func_80052B00, func_80052A88, func_80052B7C) — recommend FOR.
- STRONG, contingent: 2 (func_8002BEA0, func_8002EA24) — island evidence holds, the
  "otherwise matches" precondition is unmet; no listing today.
- WEAK: 26 (the whole ASM-STRUCTURAL batch) — recommend AGAINST.

Machine-readable summary: `tmp/auth_packets_2026-08-06.json`. Raw structural scan:
`tmp/auth_signals.json` (generator: `tmp/auth_signal_scan.py`).
