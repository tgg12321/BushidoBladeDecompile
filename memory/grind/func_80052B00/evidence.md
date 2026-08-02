# Evidence bank — func_80052B00

## Session 1 (recon, 2026-08-01)

### The function
`func_80052B00(s32 *matrix)` at `src/text1b.c:10969`. 17 target instructions,
no prologue, no frame. Target body (`asm/funcs/func_80052B00.s`):

```
lw   $t0..$t7, 0x00..0x1C($a0)     # 8 sequential word loads from *a0
ctc2 $t0..$t6, $0..$6              # cop2 control-register writes
jr   $ra
 ctc2 $t7, $7                      # <-- LAST ctc2 sits IN the jr delay slot
```

Every cop2 line is annotated `/* handwritten instruction */` by splat.
Semantically this is LIBGTE `SetRotMatrix` + `SetTransMatrix` fused: cop2
control regs CR0-CR4 are the packed 3x3 rotation matrix
(RT11RT12 / RT13RT21 / RT22RT23 / RT31RT32 / RT33) and CR5-CR7 are the
translation vector TRX/TRY/TRZ. There is NO general-purpose computation in
the body — the 8 `lw` exist solely to feed the hardcoded cop2 register
encodings (mechanical I/O packaging).

### Gate + floor (measured this session)
- `canonical func_80052B00` → **ASM-PARTIAL**, 8/17 insns canonical-asm,
  reason `GTE/cop2 op (ctc2)`, regions `[8,14]` and `[16,16]`.
  It reads PARTIAL (not ASM-WHOLE) only because the 8 mechanical `lw`
  packaging insns land in the denominator; the gate fix described in
  [[gte-wrapper-misroute-park]] excludes structural `nop`/`jr` but not `lw`.
  Per [[canonical-gate-distance-not-evidence]] the PARTIAL verdict is not
  evidence that a pure-C form exists.
- `sandbox func_80052B00 --disable all` → **score 18** (target 17 insns,
  build 19 insns, 1 regfix rule dropped). This is the honest pure-C floor
  and the session-1 baseline.
- The tree carries exactly ONE rule: `regfix.txt:3411`
  `func_80052B00: fill_delay @ 16 <- 15` — i.e. the ONLY build-time
  paperwork is moving the final `ctc2` into the `jr $ra` delay slot.
  That single rule is a precise fingerprint of the structural blocker below.

### What the honest (cheat-stripped) build actually emits
From `tmp/grind/func_80052B00/s1/sandbox_disasm.txt` — with the source's
`register s32 tN asm("$8".."$15")` pins stripped as cheat-asm:

```
lw   v0,4(a0)  ; lw v1,8(a0)  ; lw a1,12(a0) ; lw a2,16(a0)
lw   a3,20(a0) ; lw t0,24(a0) ; lw t1,28(a0) ; lw a0,0(a0)
nop                       # load-delay nop before the first ctc2
ctc2 a0,$0 ; ctc2 v0,$1 ; ctc2 v1,$2 ; ctc2 a1,$3
ctc2 a2,$4 ; ctc2 a3,$5 ; ctc2 t0,$6 ; ctc2 t1,$7
jr   ra
 nop                      # delay slot UNFILLED
```

Decomposition of the 18-point gap:
1. **8 register renames.** GCC allocates the caller-save pool
   `$v0,$v1,$a1,$a2,$a3,$t0,$t1` for a leaf's temps; the target uses
   `$t0..$t7`. Only the (forbidden) `register asm("$N")` pins currently in
   `src/text1b.c:10970-10977` reach the target allocation.
2. **Load-order inversion.** `matrix[0]` is loaded LAST, into `$a0`, because
   `$a0` is the live base pointer and GCC defers the self-clobbering load.
   Target loads strictly in ascending offset order into distinct `$tN`.
3. **One extra load-delay `nop`** between the final `lw` and the first `ctc2`.
4. **Unfilled `jr $ra` delay slot** (target has `ctc2 $t7, $7` there).

### CONFIRMED — item 4 is a byte-level impossibility, not a plateau
`tools/gcc-2.7.2/reorg.c:730-735`, `stop_search_p()`:

```c
case INSN:
  /* OK unless it contains a delay slot or is an `asm' insn of some type.
     We don't know anything about these.  */
  return (GET_CODE (PATTERN (insn)) == SEQUENCE
          || GET_CODE (PATTERN (insn)) == ASM_INPUT
          || asm_noperands (PATTERN (insn)) >= 0);
```

`fill_simple_delay_slots` scans backward from the jump for a candidate and
halts the moment `stop_search_p` returns 1. Any `__asm__` insn — bare
(`ASM_INPUT`) or with operands (`asm_noperands >= 0`) — is an unconditional
stop. Therefore **GCC 2.7.2 can never place an inline-`__asm__` instruction
into a `jr $ra` delay slot.**

The target's delay-slot instruction is `ctc2 $t7, $7`. `ctc2` is a cop2
control-register write with **no C analog whatsoever** — it can only ever be
produced by an `__asm__` block. The two facts together are a proof, not an
estimate: **no C source (pure C, or C plus canonical GTE inline asm) compiled
by this frozen toolchain can produce func_80052B00's target bytes.** This is
the same no-C-form-by-construction category as the jtbl-infra and GTE-leaf
carve-outs enumerated in [[no-new-park-categories]] — it is not a search
problem, and [[difficult-is-not-impossible]] does not apply (that rule governs
register-allocation and scheduling plateaus, where the matching C provably
exists; here the required opcode has no C spelling at all).

### Cited precedent — the adjacent authorized twin
`func_80052B44` sits at `src/text1b.c:10995`, the VERY NEXT function in the
same file, and is the same construct with a shorter matrix:
5x `lw` -> `ctc2 $0..$4` -> `ctc2 $zero, $5..$7`, last ctc2 in the `jr $ra`
delay slot. It was **Judge-authorized COMPLETED-INLINE-ASM-CANONICAL on
2026-07-27** on exactly this reasoning; see `inline_asm_canonical.txt:340`
(whose entry text names the same reorg.c `stop_search_p` / ASM_INPUT
mechanism verbatim) and the whole-body `__asm__("glabel func_80052B44\n" ...)`
form at `src/text1b.c:11001-11026`.

Already-authorized siblings in the same cop2-control-loader family:
- `gte_SetRotMatrix`   — `inline_asm_canonical.txt:326` (5x lw -> ctc2 $0..$4)
- `gte_SetColorMatrix` — `inline_asm_canonical.txt:325` (5x lw -> ctc2 $16..$20)
- `gte_SetTransVector` — `inline_asm_canonical.txt:324` (3x lw -> ctc2 $5..$7)
- `func_8007ED6C`      — `inline_asm_canonical.txt:308` (5x lw -> 5x ctc2 -> mvmva -> swc2)

func_80052B00 is the widest member of the family (8 loads, CR0-CR7) and is
the only one still sitting in the active queue.

### scan_hand_coded is LOW — expected, not disqualifying
`tools/scan_hand_coded.py --single func_80052B00` → `tier=LOW score=0/8`.
All eight signals are inapplicable by construction: S1 (no multu), S2 (no
branches at all), S3/S4 (require >= 40 insns; this is 17), S5 (jaccard < 0.5 —
the scanner's similarity lens does not cluster cop2-only bodies), S6 (no BIOS
jumptable), S7 (no callee-saves used), S8 (no mask-before-shift). The S1-S8
framework in [[hand-coded-asm-recognition]] discriminates hand-written asm
from *compiled C* bodies; it has no signal for "this opcode has no C form at
all." The GTE-leaf carve-out ([[gte-wrapper-misroute-park]]) is a SEPARATE,
pre-existing sanctioned category that does not route through the S-signals —
which is why `gte_SetRotMatrix` / `gte_SetColorMatrix` / `gte_SetTransVector`
/ `func_8007ED6C` / `func_80052B44` are all authorized despite the same LOW
tier. Recorded explicitly so a later session does not misread LOW as a failed
gate.

### Near-duplicate lead: STALE / no signal
The SessionStart hook's lead `func_80052B00 ~= func_8007EEEC (0.647)` is
flagged STALE. `func_8007EEEC` appears in neither `inline_asm_canonical.txt`
nor `src/display.c` under that name, so there is no COMPLETED-C analog body
to template from. The genuine analogs are the cop2-control-loader family
listed above, all of which are canonical-asm, none COMPLETED-C.

### Artifacts
- `tmp/grind/func_80052B00/s1/sandbox_disasm.txt`
- `tmp/grind/func_80052B00/s1/scan_hand_coded.txt`
- `tmp/grind/func_80052B00/s1/reorg_stop_search_p.txt`
- `memory/grind/func_80052B00/candidate.c` — whole-body glabel form, byte-exact
  transcription of the target, mirroring func_80052B44's authorized packaging.
  NOT applied to `src/` (applying it requires an `inline_asm_canonical.txt`
  entry, a surface this session may not touch).

- [s1] canonical func_80052B00 -> ASM-PARTIAL, 8/17 insns canonical-asm, reason 'GTE/cop2 op (ctc2)', regions [8,14] and [16,16]. It reads PARTIAL rather than ASM-WHOLE only because the 8 mechanical lw packaging insns land in the denominator; the gate fix in gte-wrapper-misroute-park excludes structural nop/jr but not lw. Per canonical-gate-distance-not-evidence, PARTIAL is not evidence that a pure-C form exists.

- [s1] sandbox func_80052B00 --disable all -> score 18 (target 17 insns, build 19 insns, 1 regfix rule dropped). Session-1 baseline floor.

- [s1] The function is LIBGTE SetRotMatrix + SetTransMatrix fused: 8 word loads from *a0 into cop2 control regs CR0-CR4 (packed 3x3 rotation matrix RT11RT12/RT13RT21/RT22RT23/RT31RT32/RT33) and CR5-CR7 (translation vector TRX/TRY/TRZ). Zero general-purpose computation; splat annotates every cop2 line '/* handwritten instruction */'.

- [s1] tools/gcc-2.7.2/reorg.c:730-735 stop_search_p returns 1 for ASM_INPUT / asm_noperands >= 0, halting fill_simple_delay_slots — so an __asm__ instruction can never occupy a jr $ra delay slot in GCC 2.7.2. The target's delay slot holds ctc2 $t7, $7, an opcode with no C analog. This is a byte-level impossibility, not a search plateau; difficult-is-not-impossible governs RA/scheduling plateaus where the matching C provably exists, which is not this case.

- [s1] The tree carries exactly ONE rule for this function: regfix.txt:3411 'func_80052B00: fill_delay @ 16 <- 15'. The only build-time paperwork in existence is the delay-slot move — a precise fingerprint of the single structural blocker.

- [s1] PRECEDENT IN HAND (file+line citable): func_80052B44 at src/text1b.c:10995 — the very next function in the same file — is the identical construct with a 5-word matrix (5x lw -> ctc2 $0..$4 -> ctc2 $zero,$5..$7, last ctc2 in the jr delay slot). It was Judge-authorized COMPLETED-INLINE-ASM-CANONICAL on 2026-07-27 citing the same reorg.c stop_search_p / ASM_INPUT mechanism verbatim; entry at inline_asm_canonical.txt:340, whole-body glabel form at src/text1b.c:11001-11026.

- [s1] Same already-authorized family: gte_SetRotMatrix (inline_asm_canonical.txt:326), gte_SetColorMatrix (:325), gte_SetTransVector (:324), func_8007ED6C (:308). func_80052B00 is the widest member (8 loads, CR0-CR7) and the only one still in the active queue.

- [s1] scan_hand_coded.py --single func_80052B00 -> tier=LOW score=0/8. All eight signals are inapplicable by construction (S1 no multu; S2 no branches at all; S3/S4 need >=40 insns and this is 17; S5 jaccard < 0.5; S6 no BIOS jumptable; S7 no callee-saves used; S8 no mask-before-shift). The S1-S8 framework discriminates hand-written asm from COMPILED-C bodies and has no signal for 'this opcode has no C form at all'. The GTE-leaf carve-out is a separate pre-existing sanctioned category that does not route through S1-S8 — which is why all five siblings above are authorized at the same LOW tier. Recorded so a later session does not misread LOW as a failed gate.

- [s1] STANDING CONSTRAINT for future sessions: the body has zero general-purpose computation — 8 array reads feeding 8 cop2 writes. No arithmetic to restructure, no control flow to reshape, no call to reorder, no type to correct. The entire pure-C lever catalog (register-alloc-pure-c A-D, cross-jump, scheduling, fold-defeat, width/addressing) operates on constructs this function does not contain. Do not spend a session running that catalog here.

- [s1] No src/ edits were made this session; the working tree is unchanged apart from ledger/scratch files.

- [s1] memory/grind/func_80052B00/candidate.c holds a whole-body glabel canonical-asm form, a byte-exact transcription of the target using func_80052B44's authorized packaging (duplicated TAB+SPACE .set directives per maspsx-noreorder-stripping; decimal memory offsets because maspsx parses 0x0($a0) as base-10). It is deliberately NOT applied to src/ — applying it requires an inline_asm_canonical.txt entry, a surface this session may not touch.

## Session 2 (structural, 2026-08-01) — floor UNCHANGED at 18

### The structural axis is measured dead (14 forms, none below 18)
Session 1 left a standing constraint ("the pure-C lever catalog operates on
constructs this function does not contain"). Session 2 did not take that on
faith — it MEASURED the axis. `tmp/grind/func_80052B00/s2/sweep.py` and
`sweep2.py` each rewrite the body of `func_80052B00` in `src/text1b.c`, run
`sandbox func_80052B00 --disable all`, and restore the original body; round 2
additionally objdumps the sandbox object and records the emitted tail
instructions. No variant contains a `register asm("$N")` pin (score-inert and
forbidden); the only inline asm in any variant is the canonical `ctc2` with
`%N` placeholders and an `"r"` constraint.

Baseline (HEAD body, its 8 pins stripped by the sandbox) = 18.

| form | score | note |
|---|---|---|
| plain-locals | **18** | 8 named locals loaded in ascending order, then 8 ctc2 |
| walking-pointer | **18** | `s32 *p = matrix; s32 tN = *p++;` |
| reverse-load-order | **18** | load `matrix[7]` first, `matrix[0]` last |
| base-copy-handle | **18** | second pointer handle `p[N]` to keep `$a0` live |
| ctc2-reverse-write | **18** | cop2 writes emitted `$7` down to `$0` |
| last-write-own-block | **18** | final `ctc2` in its own trailing `{ }` block |
| do-while-zero-probe | **18** | whole body in `do { } while (0)` (PROBE ONLY) |
| trailing-return | **18** | explicit `return;` after the last `ctc2` |
| interleaved-pairs-c89 | 21 | load 2 / write 2, decls hoisted (C89-legal) |
| direct-operand | 25 | `matrix[N]` inline in the asm operand |
| interleaved-single | 25 | load 1 / write 1 |
| struct-block-copy | 43 | 8-word struct block copy, then field writes |
| local-array | 46 | `s32 m[8]; m[N] = matrix[N];` |
| interleaved-pairs (r1) | void | C89-illegal (decls after statements); redone in r2 |

Eight distinct spellings TIE the floor at exactly 18 and five are strictly
worse; **not one is better**. There is no gradient in the structural axis:
declaration order, load order, walking pointers, extra pointer handles, block
scoping, statement interleaving, aggregate/type reshaping, cop2 write order and
a trailing return are all measured inert here.

### H1 (the delay-slot impossibility) is now EMPIRICAL, not source-read
Round 2 records the last three emitted instructions per form. Every one of the
14 forms ends `... | jr $ra | nop` — the eighth `ctc2` is always hoisted ABOVE
the jump and the delay slot is always `nop`, exactly as `reorg.c:730-735`
(`stop_search_p` returning 1 for `ASM_INPUT` / `asm_noperands >= 0`) predicts.
The strongest single data point is the `do-while-zero-probe`: `do { } while (0)`
emits `NOTE_INSN_LOOP_BEG`, which is the ONE documented lever that perturbs
reorg.c's delay-slot / relax_delay_slots behaviour ([[do-while-zero-exception]]),
and it leaves the slot `nop` too. Session 1's compiler-source argument is now
corroborated by 14 independent measurements. (The do-while(0) form was run as a
falsification probe only; it produced no benefit, is not a proposed form, and
its sanctioned-use prerequisites were never in play.)

### Family census (frontier item 3) — COMPLETE, and it partitions cleanly
`tmp/grind/func_80052B00/s2/census.py` scans every `asm/funcs/*.s` whose opcode
set is a subset of {lw, ctc2, jr, nop} and contains at least one `ctc2`, then
cross-checks each against `inline_asm_canonical.txt` and `engine/queue.json`.
Exactly SIX bodies in the whole tree qualify (raw output:
`tmp/grind/func_80052B00/s2/census.txt`):

| body | insns | jr delay slot in TARGET | disposition |
|---|---|---|---|
| `func_80052B00` | 17 | **`ctc2 $t7, $7`** | ACTIVE in queue — this function |
| `func_80052B44` | 14 | **`ctc2 $zero, $7`** | canonical-authorized 2026-07-27 (`inline_asm_canonical.txt:340`) |
| `func_8007EEEC` | 12 | `nop` | = `gte_SetRotMatrix` (`named_syms.txt:640`), canonical-authorized 2026-06-07 |
| `func_8007EF1C` | 12 | `nop` | = `gte_SetColorMatrix` (`named_syms.txt:641`), canonical-authorized 2026-06-07 |
| `func_8007EF4C` | 8 | `nop` | = `gte_SetTransVector` (`named_syms.txt:642`), canonical-authorized 2026-06-07 |
| `tslDmaDrawListDelAll` | 4 | `nop` | DONE in pure C (`src/display.c:2491`) — a single `ctc2` of the incoming param, no loads at all |

Two corrections to the raw census output, both name-resolution artifacts:
`func_8007EEEC/EF1C/EF4C` DO appear in `inline_asm_canonical.txt`, under their
`named_syms.txt` names (`gte_*`), so the script's "unauthorized" count of 5 is
wrong — the true count of unauthorized members is **one: func_80052B00**.
This also finally settles session 1's STALE near-duplicate lead: `func_8007EEEC`
IS a real sibling (it is `gte_SetRotMatrix`), but it is canonical-asm, not a
COMPLETED-C template — confirming session 1's KILL of that lead by a second,
independent route.

The census partitions the family by exactly the H1 criterion: the two members
whose TARGET puts a cop2 write in the `jr $ra` delay slot are `func_80052B44`
and `func_80052B00`; `func_80052B44` was authorized canonical on precisely that
ground. Every member with a `jr; nop` tail is either canonical for other reasons
or (in the one case with no loads to allocate) closed in pure C. The family is
settled; func_80052B00 is its last unauthorized member.

### Artifacts
- `tmp/grind/func_80052B00/s2/sweep.py` + `sweep_results.txt` (round 1, 8 forms)
- `tmp/grind/func_80052B00/s2/sweep2.py` + `sweep2_results.txt` (round 2, 6 forms + tails)
- `tmp/grind/func_80052B00/s2/census.py` + `census.txt` (family census)
- `memory/grind/func_80052B00/rejected/structural-sweep-14-forms-floor-18.c`
- `src/text1b.c` is byte-identical to HEAD; both sweep scripts restore the
  original body in a `finally` block and `git status` was verified clean.

- [s2] STRUCTURAL AXIS MEASURED DEAD: 14 distinct pure-C spellings of the body scored with `sandbox --disable all`; 8 tie the floor at exactly 18 (plain-locals, walking-pointer, reverse-load-order, base-copy-handle, ctc2-reverse-write, last-write-own-block, do-while(0) probe, trailing-return) and 5 are strictly worse (interleaved-pairs-c89 21, direct-operand 25, interleaved-single 25, struct-block-copy 43, local-array 46). NONE improved on 18. Declaration order, load order, walking pointers, extra pointer handles, block scoping, statement interleaving, aggregate/type reshaping, cop2 write order and a trailing return are all inert here.

- [s2] H1 CONFIRMED EMPIRICALLY (was source-read only in s1): all 14 forms emit `... | jr $ra | nop` — the eighth ctc2 is hoisted above the jump and the delay slot is never filled, matching reorg.c:730-735 stop_search_p. The do-while(0) probe (NOTE_INSN_LOOP_BEG, the one documented reorg-perturbing lever) also leaves the slot nop. Probe only; not a proposed form.

- [s2] FAMILY CENSUS COMPLETE: exactly 6 bodies in the tree have an opcode set within {lw,ctc2,jr,nop} incl. a ctc2 — func_80052B00 (17 insns, ACTIVE), func_80052B44 (14, canonical 2026-07-27), func_8007EEEC/EF1C/EF4C (= gte_SetRotMatrix/SetColorMatrix/SetTransVector per named_syms.txt:640-642, all canonical 2026-06-07) and tslDmaDrawListDelAll (4 insns, DONE in pure C at src/display.c:2491 — a single ctc2 of the incoming param, no loads to allocate). func_80052B00 is the ONLY unauthorized member. The family partitions on exactly the H1 criterion: the two members whose target holds a cop2 write in the jr delay slot are func_80052B44 (authorized on precisely that ground) and func_80052B00.

- [s2] The s1 STALE near-duplicate lead is settled by a second route: func_8007EEEC does exist and IS a genuine sibling, but it is gte_SetRotMatrix — canonical-asm, not a COMPLETED-C template. s1's KILL of that lead stands.

- [s2] No src/ edits persist: both sweep scripts restore the original body in a finally block; `git status --porcelain` shows only metrics/events.jsonl modified.

- [s2] Baseline re-measured this session: sandbox func_80052B00 --disable all = score 18, target_insns 17, build_insns 19, 1 regfix rule dropped, 324 cheat-asm instances stripped. Floor unchanged from session 1.

- [s2] STRUCTURAL AXIS DEAD BY MEASUREMENT (not by argument): 14 pure-C spellings scored. Ties at 18 — plain-locals, walking-pointer (s32 *p = matrix; s32 tN = *p++), reverse-load-order (matrix[7] first), base-copy-handle (second pointer handle p[N]), ctc2-reverse-write (cop2 writes $7 down to $0), last-write-own-block (final ctc2 in its own trailing block), do-while(0) probe, trailing-return. Strictly worse — interleaved-pairs-c89 21, direct-operand (matrix[N] inline in the asm operand) 25, interleaved-single 25, struct-block-copy 43, local-array (s32 m[8]) 46.

- [s2] GCC 2.7.2 rejects declarations after statements (C89), which silently voided the round-1 'interleaved-pairs' variant (bogus build_insns=7); round 2 re-ran it correctly with hoisted declarations at score 21. Any future sweep script for this toolchain must hoist all declarations to the top of the block.

- [s2] DELAY SLOT: all 14 variants emit '... | jr $ra | nop'; the eighth ctc2 is always hoisted above the jump. This includes the do { } while (0) probe, whose NOTE_INSN_LOOP_BEG is the single documented lever that perturbs reorg.c's delay-slot / relax_delay_slots behaviour. H1 is now empirical.

- [s2] FAMILY CENSUS COMPLETE: 6 cop2-control-loader leaves exist in the whole tree (func_80052B00, func_80052B44, func_8007EEEC, func_8007EF1C, func_8007EF4C, tslDmaDrawListDelAll). Five are already disposed of — four canonical-authorized, one (tslDmaDrawListDelAll) closed in pure C because it has no loads to allocate and a jr;nop tail. func_80052B00 is the only one left.

- [s2] Session 1's STALE near-duplicate lead is settled by a second independent route: func_8007EEEC does exist and IS a genuine sibling, but it is gte_SetRotMatrix (named_syms.txt:640) — canonical-asm, not a COMPLETED-C template. The s1 KILL stands.

- [s2] The residual 18 decomposes unchanged: 8 register renames (target wants $t0..$t7; GCC's leaf caller-save pool gives $v0,$v1,$a1..$t1), the deferred matrix[0] load (GCC reuses the dying $a0 as its destination), one extra load-delay nop, and the unfilled jr delay slot. Only a forbidden register asm("$N") pin reaches the target register set, and pins are score-inert under the sandbox.

- [s2] PERMUTER IS NOT WORTH A SESSION HERE: the body is 17 instructions with no control flow and no arithmetic; the permuter's mutation space (statement reordering, temp introduction, type changes) is exactly what sweep.py/sweep2.py enumerated by hand, and the residual is dominated by a hard register-set requirement plus a provably unreachable delay slot. Recorded so a later session does not spend a fresh-seed campaign on it.

- [s2] No src/ edits persist: both sweep scripts restore the original body in a finally block, and `git status --porcelain` after the sweeps showed only metrics/events.jsonl plus the session's own ledger files as modified.

- [s2] This session did not touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile or any *.ld; no queue done, no retire, no commit.

## Session 3 (structural, 2026-08-01) — FLOOR MOVED 18 -> 17

### The lever sessions 1-2 never varied: asm-statement GRANULARITY
Sessions 1 and 2 swept the STATEMENT level (declaration order, load order,
walking pointer, base-copy handle, block scoping, interleaving, aggregate
reshaping, cop2 write order, trailing return, do-while(0)) — 14 forms, all
scoring >= 18. Every one of those forms emitted eight SEPARATE one-operand
`__asm__ volatile ("ctc2 %0, $N" :: "r"(tN))` statements. Session 3 held the
statement level roughly fixed and varied the granularity of the asm statements
themselves.

Fusing all eight cop2 writes into ONE `__asm__` statement with eight `"r"`
inputs forces all eight loaded values to be simultaneously live at a single
program point. That is the only C-level construct that can REQUIRE eight
distinct hard registers without a `register asm("$N")` pin, and it removes both
non-register defects of the eight-statement forms at once:

| form (round 3, `tmp/grind/func_80052B00/s3/sweep3.py`) | score |
|---|---|
| **fused8** (all 8 ctc2 in one asm, operands in CR order) | **17** |
| fused8-reverse-operands | 17 |
| fused8-direct (matrix[N] rvalues as the 8 inputs, no locals) | 17 |
| fused7-plus-last (CR0-CR6 fused, CR7 its own statement) | 17 |
| first-plus-fused7 | 17 |
| fused-pairs (4 asm statements x 2 writes) | 17 |
| fused-quads (2 asm statements x 4 writes) | 17 |
| fused8-u32 (unsigned temporaries) | 17 |
| fused8-walking (walking pointer feeding the fused asm) | 17 |
| fused8-reverse-loads (loads matrix[7]..matrix[0]) | 17 |
| volatile-param-PROBE (PROBE ONLY — volatile coercion is a cheat) | 15 |

Round 4 (`sweep4.py`) then asked whether any legal spelling moves the
allocator's starting register: `const s32 *` parameter, `s32 matrix[8]`
parameter, `void *` parameter with an in-body cast, reversed declaration order,
`"d"` instead of `"r"` constraints, a ninth `"r"(matrix)` operand keeping the
base pointer live through the asm, and a copy chain through a second set of
eight locals. **All seven: score 17, register set identical.**

### What the fused form emits (full dump: `s3/form_disasm.txt`)
```
lw   v0,0(a0)   lw v1,4(a0)   lw a1,8(a0)   lw a2,12(a0)
lw   a3,16(a0)  lw t0,20(a0)  lw t1,24(a0)  lw t2,28(a0)
ctc2 v0,$0  ctc2 v1,$1  ctc2 a1,$2  ctc2 a2,$3
ctc2 a3,$4  ctc2 t0,$5  ctc2 t1,$6  ctc2 t2,$7
jr   ra
 nop
```
versus the target's `lw $t0..$t7, 0..0x1C($a0)` / `ctc2 $t0..$t6, $0..$6` /
`jr $ra` / `ctc2 $t7, $7`. The body is now instruction-for-instruction
ISOMORPHIC to the target: same opcodes, same order, same memory offsets, same
operand roles, 18 emitted insns vs the target's 17. Two defects that survived
all of sessions 1-2 are GONE:
  - the deferred `matrix[0]` load (GCC could no longer reuse the dying `$a0` as
    a load destination, so the loads come out in strict ascending offset order);
  - the extra load-delay `nop` before the first `ctc2` (build 19 -> 18 insns).

### The residual 17 is now exactly two things
1. **16 instructions naming the wrong GPRs** (8 `lw` + 8 `ctc2`): the allocator
   hands out `{$v0,$v1,$a1,$a2,$a3,$t0,$t1,$t2}`; the target uses `{$t0..$t7}`.
2. **1 instruction for the `jr $ra` delay slot**: build emits `nop`, target
   holds `ctc2 $t7, $7`.

### H2 upgraded: a compiler-source MECHANISM for the register set
`tools/gcc-2.7.2/config/mips/mips.h` contains **no `REG_ALLOC_ORDER`
definition** (grepped this session). Without it, local-alloc's `find_free_reg`
walks hard registers in plain ascending number order and takes the first with no
conflict: `$2, $3, (skip $4 — the live base pointer), $5, $6, $7, $8, $9, $10` =
exactly the observed `v0,v1,a1,a2,a3,t0,t1,t2`. The only two documented ways to
move that start point are (a) a conflict across a call and (b) a copy suggestion
to a named hard register. A call-free leaf with no computation has neither, and
manufacturing either — a clobber list naming `$2/$3/$5-$7`, or a
`register T x asm("$N")` pin — is a register pin by another spelling (and
score-inert under the cheat-invisible sandbox besides). Session 1 argued this
from allocator behaviour; it is now grounded in the machine description and
measured invariant across 18 distinct fused-form spellings.

### H1 re-confirmed a third time
All 18 session-3 forms end `... | jr $ra | nop`, matching sessions 1-2. Total
across the three sessions: 32/32 measured C spellings leave the delay slot
`nop`. `ctc2` has no C analog, so the target's delay-slot instruction can only
originate in an `__asm__` block, and `reorg.c:730-735` (`stop_search_p`) halts
`fill_simple_delay_slots` unconditionally at any such insn.

### Artifacts
- `tmp/grind/func_80052B00/s3/sweep3.py` + `sweep3_results.txt` (11 forms)
- `tmp/grind/func_80052B00/s3/sweep4.py` + `sweep4_results.txt` (7 forms)
- `tmp/grind/func_80052B00/s3/dump_forms.py` + `form_disasm.txt` (full bodies vs target)
- `tmp/grind/func_80052B00/s3/verify_banked.py` + `verify_banked.txt`
- `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` — the floor-17 body,
  re-spliced from the banked file and re-measured at 17 to prove the artifact
  reproduces.

- [s3] FLOOR MOVED FOR THE FIRST TIME: sandbox func_80052B00 --disable all = 17 (was 18 in s1 and s2), build_insns 18 vs target 17. The lever is asm-statement GRANULARITY, which sessions 1-2 never varied: fusing all eight cop2 writes into ONE __asm__ statement with eight "r" inputs. 18 fused-form spellings measured across two rounds; every single one scores 17.

- [s3] The fused form makes the emitted body instruction-for-instruction ISOMORPHIC to the target — same opcodes, same order, same memory offsets. Two long-standing defects vanish: the deferred matrix[0] load (all eight values are simultaneously live, so GCC can no longer reuse the dying $a0 as a load destination and the loads come out in strict ascending offset order) and the extra load-delay nop before the first ctc2 (build 19 -> 18 insns).

- [s3] The residual 17 decomposes into exactly two items: 16 instructions (8 lw + 8 ctc2) naming the wrong GPRs — allocator gives {v0,v1,a1,a2,a3,t0,t1,t2}, target wants {t0..t7} — plus 1 for the unfilled jr $ra delay slot. There is nothing else left in the diff.

- [s3] H2 now has a machine-description mechanism, not just observed behaviour: tools/gcc-2.7.2/config/mips/mips.h defines NO REG_ALLOC_ORDER, so local-alloc's find_free_reg walks hard registers in ascending number order and takes the first non-conflicting one — $2,$3,(skip $4 = live base pointer),$5,$6,$7,$8,$9,$10, exactly the observed set. The only documented ways to move the start point are a conflict across a call or a copy suggestion to a named hard register; a call-free leaf has neither, and manufacturing either is a register pin by another spelling and score-inert under the sandbox.

- [s3] REGISTER SET MEASURED INVARIANT across 18 spellings: fused8, fused8-reverse-operands, fused8-direct, fused7-plus-last, first-plus-fused7, fused-pairs, fused-quads, fused8-u32, fused8-walking, fused8-reverse-loads (round 3) and f8-const-param, f8-array-param, f8-void-param-cast, f8-decls-reversed, f8-constraint-d, f8-ninth-base-operand, f8-copy-chain (round 4). Score 17 every time; ctc2 register sequence v0,v1,a1,a2,a3,t0,t1,t2 every time (round-3 orderings differ where the asm is split, but the SET never does).

- [s3] PROBE ONLY, NOT A PROPOSED FORM: a volatile-qualified pointer (volatile s32 *vp = matrix) feeding the fused asm scores 15. Volatile coercion of a non-hardware access is a cheat under inline-asm-policy / legitimate-volatile-interrupt-touched, so this is recorded purely as a measurement of how much the load scheduling is worth (2 points) and must not be proposed as a form.

- [s3] H1 CONFIRMED A THIRD TIME: all 18 session-3 forms end 'jr $ra | nop'. Running total across sessions 1-3: 32/32 measured C spellings leave the delay slot unfilled, matching reorg.c:730-735 stop_search_p.

- [s3] The banked artifact was re-verified end-to-end: verify_banked.py splices the exact body out of memory/grind/func_80052B00/best_pure_c_fused8_floor17.c into src/text1b.c, measures 17, and restores. The file reproduces its claimed number; it is not a transcription.

- [s3] src/text1b.c is byte-identical to HEAD (git status shows only metrics/events.jsonl plus the session's own ledger/scratch files). The floor-17 form is deliberately NOT applied: the HEAD body's eight register pins plus regfix.txt:3411 are what make the real build byte-match today, and swapping in the pin-free form would break the oracle.

- [s3] This session did not touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile or any *.ld; no queue done, no retire, no commit.

- [s3] FLOOR MOVED FOR THE FIRST TIME: `sandbox func_80052B00 --disable all` = 17 (was 18 in s1 and s2); build_insns 18 vs target 17. The lever is asm-statement GRANULARITY, which sessions 1-2 never varied: all eight cop2 writes fused into one __asm__ statement with eight "r" inputs. 18 fused-form spellings measured across two rounds; every one scores 17.

- [s3] The fused form makes the emitted body instruction-for-instruction ISOMORPHIC to the target - same opcodes, same order, same memory offsets, same operand roles. Two long-standing defects vanish: the deferred matrix[0] load (all eight values simultaneously live, so GCC can no longer reuse the dying $a0 as a load destination and the loads come out in strict ascending offset order) and the extra load-delay nop before the first ctc2 (build 19 -> 18 insns).

- [s3] The residual 17 decomposes into exactly two items and nothing else: 16 instructions (8 lw + 8 ctc2) naming the wrong GPRs - allocator gives {v0,v1,a1,a2,a3,t0,t1,t2}, target wants {t0..t7} - plus 1 for the unfilled jr $ra delay slot.

- [s3] H2 now has a machine-description mechanism rather than observed behaviour alone: tools/gcc-2.7.2/config/mips/mips.h defines NO REG_ALLOC_ORDER, so local-alloc's find_free_reg walks hard registers in ascending number order and takes the first non-conflicting one - $2,$3,(skip $4 = live base pointer),$5,$6,$7,$8,$9,$10, exactly the observed set. Only a call conflict or a copy suggestion to a named hard register can move it; a call-free leaf has neither, and manufacturing either is a register pin by another spelling (and score-inert under the sandbox).

- [s3] REGISTER SET MEASURED INVARIANT across 18 spellings: fused8, fused8-reverse-operands, fused8-direct, fused7-plus-last, first-plus-fused7, fused-pairs, fused-quads, fused8-u32, fused8-walking, fused8-reverse-loads (round 3) and f8-const-param, f8-array-param, f8-void-param-cast, f8-decls-reversed, f8-constraint-d, f8-ninth-base-operand, f8-copy-chain (round 4). Score 17 every time; ctc2 register sequence v0,v1,a1,a2,a3,t0,t1,t2 every time.

- [s3] PROBE ONLY, NOT A PROPOSED FORM: a volatile-qualified pointer feeding the fused asm scores 15. Volatile coercion of a non-hardware access is a cheat under inline-asm-policy; recorded solely as a measurement that the load scheduling is worth 2 points.

- [s3] H1 CONFIRMED A THIRD TIME: all 18 session-3 forms end 'jr $ra | nop'. Running total across sessions 1-3: 32/32 measured C spellings leave the delay slot unfilled, matching reorg.c:730-735 stop_search_p.

- [s3] The banked artifact was re-verified end-to-end: verify_banked.py splices the exact body out of memory/grind/func_80052B00/best_pure_c_fused8_floor17.c into src/text1b.c, measures 17, and restores. The file reproduces its claimed number rather than being a transcription.

- [s3] src/text1b.c is byte-identical to HEAD (git status shows only metrics/events.jsonl plus this session's ledger/scratch files). The floor-17 form is deliberately NOT applied: the HEAD body's eight register pins plus regfix.txt:3411 are what make the real build byte-match today, and swapping in the pin-free form would break the oracle.

- [s3] No touch of regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile or any *.ld; no queue done, no retire, no commit.
