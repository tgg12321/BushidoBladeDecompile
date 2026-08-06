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

---

# Batch 2 — 2026-08-06

Population: the **7 canonical-eligible functions** from the ASM-SUSPECT/ASM-PARTIAL census
(`docs/grind/asm-suspect-census-2026-08-06.md`, buckets A1+A2; machine-readable
`tmp/census_2026-08-06.json`). All 7 carry queue verdict `ASM-PARTIAL`. Plus one
Wave-1 finding folded in: the **splat mis-scoping of `asm/funcs/ang_hosei.s`**, which
fuses the program's boot/entry stub onto a 9-instruction function.

Same posture as batch 1: **nothing here is authorized**, nothing was committed, no build
was run, and no build-pipeline file was touched. Default-FAIL — weak evidence returns the
function to the pure-C pool.

## Headline

**8 of 8 units rate DECISIVE or STRONG and are recommended FOR whole-function
authorization** (7 census functions + the boot stub, which needs a symbol split first).
This is the opposite of batch 1's Batch-A result, and it is opposite for a measurable
reason rather than a rhetorical one.

Two independent base-rate measurements, run fresh today over all 1,437 `asm/funcs/*.s`
(generators: `tmp/batch2_trap_check.py`, `tmp/batch2_neighbors.py`):

**1. Trapping arithmetic is GCC-2.7.2-impossible in this corpus — 64 vs 0.**

| Population | Functions containing `add` / `addi` / `sub` / `neg` (trapping forms) |
|---|---:|
| authorized canonical (`inline_asm_canonical.txt`) | **64** |
| still-queued | **3** — and they are exactly `ang_hosei`, `func_8004C388`, `func_80052788` |
| **already COMPLETED-C (0 rules, byte-matched)** | **0** |

Zero counter-examples. GCC 2.7.2's `addsi3`/`subsi3` MIPS patterns emit the `u` forms
because C addition does not trap; `add`/`sub`/`addi` are what a human types in an ASPSX
`.s` file. This clears the census's "decisive signal" bar (GCC-impossible with zero
COMPLETED-C counter-examples) with a wide margin — and note it is a *stronger* enrichment
than G1 ghost-callee-save (11 vs 2), which the census itself demoted to corroborating.

**2. Non-div-guard `break` codes form a closed 4-member cluster, 3 already authorized.**

| Function | Status | `break` codes |
|---|---|---|
| `func_80083698` | authorized canonical | 259 (0x103) |
| `md_gview_init` | authorized canonical | 260 (0x104) |
| `func_8008393C` | authorized canonical | 261 (0x105) |
| **`ang_hosei`** | **queued** | **263 (0x107)**, plus `break 0, 1` |

Those are the only four functions in the entire executable with a `break` whose code is
not the 6/7 division guard. Three are signed off; `ang_hosei` is the fourth member of the
same Marionation engine-call family, at the next code up.

**3. The candidates are holes in an otherwise-solid hand-written object file.** In the
address band `0x8004A000-0x80053000`, **60 of 73 functions are already authorized
canonical**. The non-canonical remainder is five trivial 2–88-instruction stubs, one real
C function at the band edge (`func_80052D00`, 385 insns / distance 384), and **six of our
seven candidates**. `func_8004C388` sits between authorized `func_8004C1F4` and authorized
`func_8004C404`. The whole `0x80052788-0x80052B7C` GTE run alternates
authorized/candidate/authorized with `func_800527FC`, `func_80052754`, `func_80052B44`,
`InitFadePanel`, `func_80052C28` already signed off. `ang_hosei` sits immediately after
authorized `md_gview_init` and immediately before authorized `bios_InitHeap`.

## On the island-vs-whole-function question

The task brief is right that for **most** ASM-PARTIAL functions the correct disposition is
"pure C body + canonical inline-asm island" (the `func_8002EA24` precedent,
`docs/grind/decisions.md` 2026-07-30), not whole-function authorization. That doctrine
presupposes an *ordinary C body* with a small no-C-form span inside it. **For these seven
that premise is false**, and the census's own density column shows where the line falls:

- Census bucket B/C ASM-PARTIALs: cop2 spans of **0–17%** inside 90–1,473-instruction
  bodies with frames, spills and calls. Island doctrine applies. `motion_SetExMotion`
  (15/1454 = 1%) is the extreme case.
- These seven: **43–58% cop2** (or 100% trapping-arith-and-shift) in **26–60-instruction
  leaves** with **no stack frame, no spills, and no calls**. A "pure C body plus island"
  form here would be a C function whose entire body is a sequence of back-to-back
  `__asm__` blocks with nothing between them — which is whole-function canonical asm
  written in a more fragile spelling, not pure C.

`func_80052930` is the only one where the question is genuinely close (34 of its 60
instructions are general-purpose), so I delimit its island decomposition below as an
explicit fallback and rate it STRONG rather than DECISIVE.

## Summary table

| Unit | File | Insns | Dist | Rules | Decisive signal | Rating | Disposition | Recommend |
|---|---|---:|---:|---:|---|---|---|---|
| `ang_hosei` (9-insn trampoline) | src/ings2.c | 9 | — | 0 | `break 0, 263`; custom arg-shift ABI | **DECISIVE** | whole-function | **FOR** |
| boot/entry stub @ `0x800836EC` | src/ings2.c (fused) | 42 | — | 0 | crt0: builds `$sp`/`$gp`/`$fp`, `$ra` in a global, trapping `addi`, `break 0, 1` | **DECISIVE** | **split to own symbol**, then whole-function | **FOR** (after split) |
| `func_8004C388` | src/text1b.c | 30 | 29 | 0 | 5× trapping `add` | **DECISIVE** | whole-function | **FOR** |
| `func_80052788` | src/text1b.c | 29 | 23 | 0 | trapping `sub`; duplicated `mfc2 $t0,$9`; `gpf`/`gpl` | **DECISIVE** | whole-function | **FOR** |
| `game_2d_CheckLifeGaugeNoDisp` | src/text1b.c | 26 | 20 | 1 | `swc2 $11` in the `jr $ra` delay slot | **DECISIVE** | whole-function | **FOR** |
| `func_80052A88` | src/text1b.c | 30 | 25 | 1 | `swc2 $11` in the `jr $ra` delay slot | **DECISIVE** (reaffirmed) | whole-function | **FOR** |
| `func_80052B7C` | src/text1b.c | 26 | 20 | 1 | `swc2 $11` in the `jr $ra` delay slot | **DECISIVE** (reaffirmed) | whole-function | **FOR** |
| `func_80052930` | src/text1b.c | 60 | 51 | 0 | mvmva-latency interleave; 10 cop2 regions | **STRONG** | whole-function (island fallback delimited) | **FOR** |

`func_80052A88` and `func_80052B7C` already carry DECISIVE packets in batch 1 above; they
are not re-litigated here, only reaffirmed with the new cluster evidence. Their proposed
`inline_asm_canonical.txt` entries in batch 1 stand unchanged.

## Standing finding — two candidates are carrying inline-asm injection right now

Independent of the authorization decision, the owner should know that `src/text1b.c`
contains hardcoded-`$N` `__asm__` templates for two of these functions. **Line numbers
below are `git show HEAD:src/text1b.c`** — the working tree was dirty from a concurrent
agent while this packet was written, and these constructs were verified present in HEAD,
so they are pre-existing and not that agent's in-flight edit:

- `func_8004C388` — HEAD `src/text1b.c:1339-1341` and `1352-1353`, five templates:
  `__asm__ volatile ("add $8, $8, $11" : "=r"(t0) : "0"(t0), "r"(t3));` and siblings.
- `func_80052788` — HEAD `src/text1b.c:1638`:
  `__asm__ volatile ("sub $11, $11, $6" : "=r"(t3) : "0"(t3), "r"(arg2));`.

These have constraints attached but the *template text* names `$8`/`$11`/`$6` literally, so
the emitted registers come from the template rather than from GCC's choice — the
[[inline-asm-injection]] signature with paperwork. Both functions also carry
`register s32 tN asm("$N")` pins. The census's "0 regfix rules" column is therefore
misleading for these two: they have zero *rules* but are not cheat-free.

Whole-function authorization moots this (the body becomes one `glabel` block and the pins
and templates are deleted). If the owner instead sends them back to the pure-C pool, these
constructs must be removed first, not left in place. Either way the current state should
not persist. The GTE-op templates in `func_80052788` (`mtc2 %0, $8`, `ori %0, $zero,
0x1000`, `.word 0x4B98003D`) use `%N` placeholders and are the legitimate canonical form —
this finding is only about the `add`/`sub` pair.

## Per-unit packets

### `ang_hosei` — DECISIVE — recommend FOR (whole-function, 9 instructions only)

- **File:** src/ings2.c:609 (`INCLUDE_ASM`) · **asm:** asm/funcs/ang_hosei.s lines 2-11 ·
  `0x800836C8-0x800836E8` · queue: active, ASM-PARTIAL, distance 51, 0 rules.
- **The function is 9 instructions, not 51.** The recorded distance of 51 counts the boot
  stub fused onto it (next section). The real body:

```
800836C8  addu  $a3, $a2, $zero      # arg shift: a2 -> a3
800836CC  addu  $a2, $a1, $zero      #            a1 -> a2
800836D0  addu  $a1, $a0, $zero      #            a0 -> a1
800836D4  break 0, 263               # Marionation engine call, code 0x107
800836D8  beqz  $v0, .L800836E4
800836DC   addu $v0, $v1, $zero      # (delay) success: return $v1
800836E0  addiu $v0, $zero, -0x1     # failure: return -1
800836E4: jr    $ra
800836E8   nop
```

- **Why it is unreachable from C.** Two independent reasons. (a) `break` with a custom
  code field cannot be emitted by GCC 2.7.2 at all, and maspsx cannot assemble `break`
  with an arbitrary code — the three authorized siblings all encode it as a raw `.word`
  for exactly this reason (`inline_asm_canonical.txt:131-133`). (b) The three `addu`
  shifts implement a **custom calling convention**: the callee reads its arguments from
  `$a1/$a2/$a3` and returns a pair in `$v0/$v1`, with `$a0` left free for the engine.
  No C function signature produces that shift, and no C construct reads a second return
  register.
- **Precedent fit.** Exact. `func_80083698` (code 0x103), `md_gview_init` (0x104) and
  `bios_FileReadRaw`/`func_8008393C` (0x105) are all authorized, all in the same
  address neighbourhood, all described in the existing entries as "break with custom code
  plus return-value handling." `ang_hosei` is code 0x107 and has the same shape plus the
  arg shift. The measured corpus census above shows these four are the *only* non-div-guard
  `break` users in the executable.
- **Naming note (not blocking).** `ang_hosei` is engine-call-0x107, not angle correction.
  Its call sites (`src/ings.c:141,143,170` — `ang_hosei(fd, 0, 2)`, `ang_hosei(fd,
  sector << 11, 0)`) read as a file seek/read/size primitive, which fits the
  `bios_FileReadRaw` family. `named_syms.txt` already flags two other `ang_hosei*` symbols
  as misnamed (lines 2497, 3457), so the prefix is known drift. Renaming is optional and
  independent of the authorization.
- **Proposed entry (approve verbatim, and only after the split below):**

```
ang_hosei  # Marionation engine call, break code 0x107: custom-ABI arg shift (a0/a1/a2 -> a1/a2/a3, leaving a0 for the engine), `break 0, 263`, then a $v0/$v1 two-register return select (beqz $v0 -> return $v1 else -1). `break` with a custom code field has no C form and maspsx cannot assemble it (the authorized siblings encode it as a raw .word); the arg shift and the $v1 second return value have no C signature. Fourth and last member of the authorized break-trampoline family (func_80083698 0x103, md_gview_init 0x104, func_8008393C 0x105).
```

### Boot/entry stub at `0x800836EC` — DECISIVE — recommend FOR, **after a symbol split**

This is the Wave-1 finding, assessed.

- **Currently:** lines 12-54 of `asm/funcs/ang_hosei.s`, with no symbol of its own, emitted
  inside `endlabel ang_hosei`. Nothing reaches it — `ang_hosei` returns at `0x800836E4`
  with a `nop` delay slot, so there is no fallthrough; the only entry is the PS-EXE
  header. `AGENTS.md:32`, `README.md:51` and `docs/ARCHITECTURE.md:33` all record
  **`0x800836EC` as the executable's entry point**, and grep confirms no symbol,
  `symbol_addrs.txt` entry, or `named_syms.txt` entry exists at that address.

- **Is it hand-written asm?** Yes, and this one is not a close call. It is a textbook crt0:

| Address | Instruction | Why no C form exists |
|---|---|---|
| `800836EC-8008370C` | `lui/addiu` bounds + `sw $zero` loop over `D_800A3308 -> D_801078E0` | BSS clear; runs *before* any C environment |
| `80083710-80083724` | `lw D_800A2690`; `addi $v0, $v0, -0x8` **(trapping, splat-tagged `handwritten instruction`)**; `or $sp, $v0, $t0` with `$t0 = 0x80000000` | **Assigns the stack pointer.** C has no construct that writes `$sp`. |
| `80083728-80083734` | `sll $a0,3 / srl $a0,3` | strips the KSEG0 bit off the heap base by shifting — a hand idiom, not a C mask |
| `80083760-80083764` | `sw $ra, D_800A3668` | **saves the return address into a global, not a stack slot** — because no frame exists yet |
| `80083768-8008376C` | `lui/addiu $gp, %hi/%lo(_gp)` | **assigns `$gp`.** GCC assumes `$gp` is already live; it never initialises it. |
| `80083770` | `addu $fp, $sp, $zero` | assigns the frame pointer directly |
| `80083774-80083778` | `jal bios_InitHeap` with `addi $a0, $a0, 4` **(trapping, splat-tagged)** in the delay slot | second trapping-arith instance |
| `8008377C-8008378C` | reload `$ra` from the global, `jal` into `main` | — |
| `80083790` | `break 0, 1` | program terminate; custom break code, no C form |

  A function with no prologue, no frame, no callee-saves, that *constructs* `$sp`, `$gp`
  and `$fp` and stores `$ra` in a global, is hand-written by definition — this is the code
  that establishes the environment C compilation presupposes. Two of its instructions
  carry splat's own `/* handwritten instruction */` tag, and it contributes 2 of the 3
  queued trapping-arith hits measured above.

  Secondary finding: the `jal` at `0x80083788` targets
  `cpu_set_move_command_and_dir_for_no_action_2`. A crt0's final call is `main`. That
  symbol name is almost certainly drift and worth re-checking independently of this packet.

- **Splat / symbol_addrs implications — read before acting.** The split is cheap but the
  obvious route is booby-trapped:

  1. **Do NOT add `0x800836EC` to `symbol_addrs.txt` and re-run splat.** `CLAUDE.md` and
     `splat.yaml` both record that **`bb2.ld` is hand-maintained and `make setup` must not
     be run** — it re-adds dead rodata lines and conflicts with the const declarations now
     living in `src/*.c`. Regenerating to pick up one symbol would cost a `bb2.ld`
     recovery.
  2. **The manual split is byte-neutral and is the route to take.** `INCLUDE_ASM` expands
     (`include/include_asm.h:7-15`) to `.section .text` + `.set noat/noreorder` +
     `.include "<folder>/<name>.s"`. So: cut lines 12-54 of `asm/funcs/ang_hosei.s` into a
     new `asm/funcs/<newsym>.s` with its own `glabel`/`endlabel`, close `ang_hosei.s` after
     line 11, and put two consecutive `INCLUDE_ASM` lines in `src/ings2.c` where line 609
     is now. The instruction stream, its order and its alignment are unchanged (both
     halves are 4-byte aligned and contiguous at `0x800836E8 -> 0x800836EC`), and
     `.section .text` at an already-aligned offset emits no padding. It adds a symbol and
     zero bytes.
  3. `symbol_addrs.txt` should still get the name so future splat runs and `named_syms.txt`
     agree, but it is documentation here, not the mechanism.
  4. **Oracle-verify the split on its own commit, before any authorization commit.** It is
     byte-neutral by construction but it touches the build pipeline, so it should stand or
     fall on `verify-oracle` alone rather than being bundled with a rules change.
  5. Suggested symbol name: `_start` or `main_entry` (whichever matches the project's
     convention for the PS-EXE entry; nothing currently claims either).

- **Consequence for the queue.** Once split, `ang_hosei`'s recorded distance of 51 is
  wrong by construction — the 9-instruction trampoline is the whole function, and the
  42-instruction stub becomes a separate item. `docs/HANDOFF-2026-08-06B.md:51` already
  records this ("recorded distance 51 overstates the 9-insn function"). A `queue regen`
  after the split will correct it.

- **Proposed entry (approve verbatim, for the new symbol):**

```
<newsym>  # PS-EXE entry point / crt0 at 0x800836EC (AGENTS.md records this as the executable's entry). No prologue, no frame, no callee-saves: zeroes BSS from D_800A3308 to D_801078E0, computes and ASSIGNS $sp from D_800A2690 via a trapping `addi` (splat-tagged 'handwritten instruction'), strips KSEG0 off the heap base by sll/srl, saves $ra into the GLOBAL D_800A3668 because no stack frame exists yet, loads $gp from _gp, sets $fp from $sp, calls bios_InitHeap with a second trapping `addi` in the delay slot, then jal's main and terminates with `break 0, 1`. Startup code that establishes the environment C compilation presupposes — $sp/$gp/$fp assignment and a custom-code break have no C form. Split from asm/funcs/ang_hosei.s, where splat had fused it onto the unrelated 9-instruction function ending at 0x800836E8.
```

### `func_8004C388` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1325) · **asm:** asm/funcs/func_8004C388.s · **30 instructions** ·
  queue: active, ASM-PARTIAL, distance 29, 0 rules (but see the injection finding above).
- **Shape.** A frameless, call-free leaf: six `lh` loads of two 3-vectors from `*$a0`/`*$a1`
  (`8004C388-8004C39C`), three trapping `add` (`8004C3A0`, `8004C3A4`, `8004C3A8`), three
  `sra 1` — a midpoint average — then three `sh` to `*$a2`. The tail
  (`8004C3C4-8004C3FC`) does the same midpoint on a packed byte pair out of halfword 3
  (`andi 0xFF00` / `andi 0xFF`, two more trapping `add`, `srl 1`, re-pack via `or`), with
  the final `sh $t3, 0x6($a2)` in the `jr $ra` delay slot.
- **Decisive signal.** **Five trapping `add`** at `8004C3A0/A4/A8/DC/E0`, every one carrying
  splat's `/* handwritten instruction */` tag. Per the corpus measurement above, trapping
  arithmetic appears in 64 authorized-canonical functions and **0 COMPLETED-C functions** —
  GCC 2.7.2 emits `addu` for C `+` unconditionally. There is no C input to the frozen
  compiler that produces these five bytes.
- **Why whole-function rather than island.** The five adds are not one span; they are
  interleaved through the whole body at instructions 8, 9, 10, 22 and 23 of 30. An island
  decomposition means five separate `__asm__` blocks with operand bindings surrounding
  ~25 instructions of load/shift/store — and the register allocation of that C would have
  to be steered to match, which is what the current source is doing with pins and hardcoded
  templates. The function is a 30-instruction hand-written leaf inside a 60-of-73
  authorized band; whole-function is the honest form.
- **Cluster fit.** Immediate neighbours `func_8004C1F4` (before) and `func_8004C404`
  (after) are both already authorized canonical, and both also contain trapping arith
  (22 and 16 instances). This function is a hole in a signed-off object file.
- **Proposed entry (approve verbatim):**

```
func_8004C388  # Hand-written midpoint-average leaf: 6x lh of two s16 3-vectors -> 5x TRAPPING `add` (splat-tagged 'handwritten instruction') -> sra/srl 1 -> 3x sh, plus a packed-byte-pair midpoint on halfword 3 with the final sh in the jr-ra delay slot. Frameless, call-free, no spills. GCC 2.7.2 emits `addu` for C addition unconditionally (addsi3); trapping `add` appears in 64 authorized-canonical functions and 0 COMPLETED-C functions corpus-wide, so these bytes are unreachable from any C. Sits between authorized func_8004C1F4 and func_8004C404 in a band where 60 of 73 functions are already authorized.
```

### `func_80052788` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1628) · **asm:** asm/funcs/func_80052788.s · **29 instructions** ·
  queue: active, ASM-PARTIAL, distance 23, 0 rules (see the injection finding above).
- **Shape.** GTE `gpf`/`gpl` interpolation leaf. 15 of 27 body instructions are cop2:
  `ori $t3, $zero, 0x1000` then a **trapping `sub`** to form `0x1000 - t` (`80052798`),
  `mtc2 $t3/$t0/$t1/$t2 -> $8/$9/$10/$11`, `gpf 1` (`800527B4`), a second `mtc2` quartet,
  two unfilled GTE latency `nop`s, `gpl 1` (`800527D8`), then `mfc2` reads and three `sh`
  with the last in the `jr $ra` delay slot.
- **Three independent decisive signals.**
  1. **Trapping `sub`** at `80052798`, splat-tagged — the 64-vs-0 argument above.
  2. **`mfc2 $t0, $9` emitted twice in a row** at `800527DC` and `800527E0`, both
     splat-tagged. The second read is mathematically redundant and its destination is
     immediately overwritten. GCC's CSE collapses an identical back-to-back read
     unconditionally; a hand coder writes it as a GTE result-latency pad. This is the
     census's X1 signal and it is the same *kind* of construct as the S8
     redundant-mask signal that carried `func_8007EDBC` to authorization
     (`.claude/rules/packed-multiply-cluster.md`).
  3. `gpf`/`gpl` and the hand-placed latency `nop`s have no C form.
- **Cluster fit.** Immediate neighbours `func_80052754` (before) and `func_800527FC`
  (after) are both authorized canonical.
- **Proposed entry (approve verbatim):**

```
func_80052788  # GTE gpf/gpl interpolation leaf, 15/27 cop2: TRAPPING `sub` forms 0x1000-t (splat-tagged; GCC 2.7.2 emits subu for C subtraction — trapping arith is present in 64 authorized-canonical and 0 COMPLETED-C functions corpus-wide), 4x mtc2 -> gpf 1 -> 4x mtc2 -> two unfilled GTE latency nops -> gpl 1 -> mfc2 reads with `mfc2 $t0,$9` emitted TWICE back-to-back as a result-latency pad (GCC's CSE collapses an identical adjacent read unconditionally), then 3x sh with the last in the jr-ra delay slot. Frameless leaf between authorized func_80052754 and func_800527FC.
```

### `game_2d_CheckLifeGaugeNoDisp` — DECISIVE — recommend FOR (whole-function)

- **File:** src/text1b.c (HEAD line 1746) · **asm:** asm/funcs/game_2d_CheckLifeGaugeNoDisp.s ·
  **26 instructions** · queue: active, ASM-PARTIAL, distance 20, 1 rule
  (`regfix.txt:3321 fill_delay @ 24 <- 23`).
- **Shape.** Verbatim PsyQ LIBGTE matrix-vector multiply. Eight `lw` from `*$a0`
  (`80052A20-80052A3C`) feeding **eight splat-tagged `ctc2 $t0-$t7, $0-$7`**
  (`80052A40-80052A5C`), `lwc2 $0, 0x0($a1)` / `lwc2 $1, 0x4($a1)`, two unfilled GTE
  latency `nop`s, `mvmva 1,0,0,0,0` (`80052A70`), a third latency `nop`, then
  `swc2 $9/$10` and **`swc2 $11, 0x8($a2)` in the `jr $ra` delay slot** (`80052A84`).
  Zero general-purpose computation — 14 of 24 body instructions are cop2 and the other
  ten are the `lw` feed.
- **Decisive signal.** The delay-slot `swc2`. Per the granted `func_80052B44` ruling
  (batch 1 above, from `tools/gcc-2.7.2/reorg.c`): `stop_search_p` halts the delay-slot
  search at `ASM_INPUT` and `resource_conflicts_p` returns 1 for volatile asm, so **GCC
  2.7.2 can never place a cop2 op in a `jr $ra` delay slot**. `cop2` can only enter
  compilation as inline asm. The bytes are unreachable from any C input.
- **Precedent fit.** This is the same construct as authorized `func_8007ED6C`
  (`inline_asm_canonical.txt:308`) — the census measures opcode-signature Jaccard 0.704
  against it — and the direct sibling of `func_80052A88` / `func_80052B7C`, both rated
  DECISIVE in batch 1. It is the plainest member of the family: no halfword packing at all.
- **Naming note (not blocking).** The symbol is misleading. `game_2d_CheckLifeGaugeNoDisp`
  reads as a 2D UI predicate returning a flag; the body is a GTE 3x3-matrix × vector
  multiply writing three words through `$a2` and returning nothing. Its two call sites
  (`src/config.c:263-264`) pass three pointers, consistent with the GTE reading and not
  with the name. Worth re-checking alongside the `ang_hosei` family drift.
- **Proposed entry (approve verbatim):**

```
game_2d_CheckLifeGaugeNoDisp  # LIBGTE matrix-vector leaf (MISNAMED — body is a GTE 3x3 x vector multiply, not a UI predicate): 8x lw <- *a0 -> 8x ctc2 $0-$7 (all splat-tagged 'handwritten instruction'), lwc2 $0/$1 <- *a1, two unfilled GTE latency nops, mvmva 1,0,0,0,0, a third latency nop, then swc2 $9/$10 and swc2 $11 IN the jr-ra delay slot (0x80052A84). Zero general-purpose computation. GCC 2.7.2 cannot fill a delay slot with asm (reorg.c stop_search_p halts at ASM_INPUT), so the bytes are unreachable from any C. Opcode-signature Jaccard 0.704 to authorized func_8007ED6C; direct sibling of authorized func_80052B44.
```

### `func_80052930` — STRONG — recommend FOR (whole-function; island fallback delimited)

- **File:** src/text1b.c (HEAD line 1674) · **asm:** asm/funcs/func_80052930.s · **60 instructions** ·
  queue: active, ASM-PARTIAL, distance 51, 0 rules. Currently carries 11 `register asm("$N")`
  pins in HEAD (`src/text1b.c:1675-1685`).
- **Shape.** The three-cycle sibling of the family: eight splat-tagged
  `ctc2 $t0-$t4, $0-$4` + `ctc2 $zero, $5-$7` (`80052948-80052964`) load a packed 3x3
  rotation matrix with zero translation, then **three `mvmva 1,0,0,0,0` cycles**
  (`8005299C`, `800529CC`, `80052A00`), each writing its vector via `mtc2 $0/$1` and
  reading the previous cycle's result via `mfc2 $t5/$t6/$t7, $9/$10/$11`.
- **Why STRONG and not DECISIVE.** There is no single GCC-impossible instruction here:
  the delay slot holds an ordinary `sh $t7, 0x10($a2)` (GCC fills those routinely), there
  is no trapping arithmetic, and volatile `__asm__` blocks are not reordered by GCC, so
  an island form is *technically* constructible. I am not going to call that decisive.
- **Why the evidence is still strong.**
  1. **The interleave is hand-scheduling.** Cycle N+1's `mtc2` setup and cycle N's `mfc2`
     result reads are placed *inside* the preceding `mvmva`'s latency window, and the
     general-purpose packing (`and`/`andi`/`or`/`sll`/`srl` at
     `8005297C-80052994`, `800529A0-800529A8`, `800529D0-800529DC`) is threaded into the
     same windows. `.claude/rules/gte-3x3.md` names exactly this — "the
     mvmva→mfc2→mtc2→nop→mvmva pipeline interleaving (cycle N+1's setup during cycle N's
     GTE latency) is canonical hand-scheduling" — as the signature that carried
     `calc_fc_frame_8007EC5C` to ASM-WHOLE authorization on 2026-05-31.
  2. **The `0xFFFF0000` mask is materialised once** into `$t9` at `80052944`, before the
     first `ctc2`, and held live across all 60 instructions for three uses. `$t9` is a
     caller-saved temp; GCC's CSE/remat would not reserve one across a 55-instruction span
     for a constant that costs one `lui`.
  3. **Cluster.** It sits in the middle of the `0x80052788-0x80052B7C` run, immediately
     after authorized `func_800527FC` and immediately before
     `game_2d_CheckLifeGaugeNoDisp`; the band is 60-of-73 authorized.
  4. It is a frameless, call-free, spill-free leaf (census: `S3:no-spills`).
- **Island fallback, delimited.** If the owner prefers the region-granular disposition,
  the cop2 spans are **10 regions**, 26 of 60 instructions:

  | Region | Range | Ops |
  |---|---|---|
  | 1 | `80052948-80052964` | `ctc2 $t0-$t4, $0-$4`; `ctc2 $zero, $5-$7` (8) |
  | 2 | `80052988` | `mtc2 $t5, $0` |
  | 3 | `80052990` | `mtc2 $t6, $1` |
  | 4 | `8005299C` | `mvmva 1,0,0,0,0` |
  | 5 | `800529AC-800529B4` | `mfc2 $t5/$t6/$t7, $9/$10/$11` |
  | 6 | `800529B8-800529BC` | `mtc2 $v0, $0`; `mtc2 $v1, $1` |
  | 7 | `800529CC` | `mvmva 1,0,0,0,0` |
  | 8 | `800529E0-800529E8` | `mfc2 $t5/$t6/$t7, $9/$10/$11` |
  | 9 | `800529EC-800529F0` | `mtc2 $v0, $0`; `mtc2 $v1, $1` |
  | 10 | `80052A00-80052A0C` | `mvmva`; `mfc2 $t5/$t6/$t7` |

  Ten `__asm__` blocks separated by one-to-four instructions of packing, in a
  60-instruction leaf, is the argument *against* the island form rather than for it — but
  the decomposition is recorded so the owner can rule either way on facts rather than on
  my framing.
- **Proposed entry (approve verbatim, whole-function disposition):**

```
func_80052930  # LIBGTE 3-cycle matrix-vector leaf: 5x ctc2 $0-$4 (packed 3x3 R matrix) + 3x ctc2 $zero to $5-$7 (zero translation), all splat-tagged 'handwritten instruction', then three mvmva 1,0,0,0,0 cycles with each cycle's mtc2 $0/$1 setup and the previous cycle's mfc2 $9/$10/$11 result reads placed INSIDE the preceding mvmva's latency window, and the halfword packing threaded into the same windows. Canonical hand-scheduling per the calc_fc_frame_8007EC5C precedent (.claude/rules/gte-3x3.md, authorized 2026-05-31). The 0xFFFF0000 mask is materialised once into caller-saved $t9 and held live across all 60 instructions for three uses. Frameless, call-free, spill-free leaf in the 0x80052788-0x80052B7C hand-written run.
```

## Counts and the recommend-FOR list

- **DECISIVE: 6** — `ang_hosei` (9-insn trampoline), the boot/entry stub at `0x800836EC`,
  `func_8004C388`, `func_80052788`, `game_2d_CheckLifeGaugeNoDisp`, plus the two batch-1
  reaffirmations (`func_80052A88`, `func_80052B7C`).
- **STRONG: 1** — `func_80052930`.
- **WEAK / recommend AGAINST: 0.**
- All dispositions are **whole-function**, not island. The island doctrine's premise
  (an ordinary C body containing a small no-C-form span) does not hold for any of these
  26–60-instruction frameless leaves; it holds for the census's bucket-B and bucket-C
  ASM-PARTIALs, which are not in this batch.

**Suggested owner sequencing:**

1. **Split `asm/funcs/ang_hosei.s`** into the 9-instruction function and the boot stub, on
   its own oracle-verified commit that touches no rules. This is a prerequisite for the
   `ang_hosei` authorization — authorizing the fused file today would sign off the entry
   point under the wrong symbol.
2. **Authorize the five uncontested new entries** — `func_8004C388`, `func_80052788`,
   `game_2d_CheckLifeGaugeNoDisp`, `func_80052930`, `ang_hosei` — plus the new boot-stub
   symbol, and the two batch-1 siblings if not already signed off.
3. **Either way, clean the two injection sites** at HEAD `src/text1b.c:1339-1341` /
   `1352-1353` and HEAD `src/text1b.c:1638`. Authorization removes them as a side effect; a decision to send
   these functions back to the pure-C pool does not, and they must not be left standing.

Machine-readable summary: `tmp/auth_packets2_2026-08-06.json`. Generators for the two
base-rate measurements: `tmp/batch2_trap_check.py` (trapping-arith and break-code census
over all 1,437 `asm/funcs/*.s`) and `tmp/batch2_neighbors.py` (address-ordered
canonical-density neighbourhood map).
