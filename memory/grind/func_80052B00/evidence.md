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

## Session 4 (permuter, 2026-08-01) — floor 17, unmoved; the permuter axis is now MEASURED dead

Sessions 2 and 3 both *argued* that a permuter campaign was not worth spending on
this function ("its mutation space is statement reordering, temp introduction and
type changes, all already enumerated by hand"). The driver mandated the permuter
modality anyway, which is the right call: an argument that a search modality is
futile is worth much less to the owner's audit trail than a measurement of it.
This session converts that argument into a measurement. It is now a KILL with
numbers behind it and no future session should re-run it.

### Workspace construction (reusable)

`tmp/grind/func_80052B00/s4/mkws.sh` builds a self-contained decomp-permuter
workspace for this function:

- `compile.sh` runs the REAL stage chain — `tools/gcc-2.7.2/build/cc1 -O2 -G0
  -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin` →
  `tools/prologue_fix.py` → `tools/maspsx/maspsx.py --aspsx-version=2.34` (with
  every gate list the Makefile passes) → `tools/multu_pad.py` — then extracts
  only the `.ent func_80052B00 … .end func_80052B00` region and assembles it with
  `mipsel-linux-gnu-as -march=r3000 -no-pad-sections -O1 -G0`. NOTE for whoever
  reuses this: maspsx emits its lines with NO leading tab, so the extraction awk
  must match `^[ \t]*\.ent[ \t]+<func>$`, not `^\t\.ent\t<func>$` (the
  `tools/mar_perm_workspace.sh` template has the tabbed form and silently
  produces an unassemblable fragment), and `compile.sh` must `source .venv/bin/
  activate` itself or maspsx/prologue_fix fail with an empty-stdin error.
- `target.o` = `tools/decomp-permuter/prelude.inc` (with `.set gp=64` stripped) +
  `asm/funcs/func_80052B00.s`, assembled the same way.

**Context-independence check (new fact, useful beyond the permuter).** The
minimal standalone TU (a `typedef signed int s32;` plus the function) emits
EXACTLY the body the in-tree `sandbox func_80052B00 --disable all` emits for the
same source: `lw v0,0(a0) … lw t2,28(a0) / ctc2 v0,$0 … ctc2 t2,$7 / jr ra /
nop`. So this function's codegen carries no dependence on `src/text1b.c`'s ~125k
tokens of preceding declarations — a standalone TU is a faithful, and far
cheaper, experiment vehicle for it.

### Campaign 1 — chassis `fused8-r-constraints` (the s3 best pure-C body)

Base: `memory/grind/func_80052B00/best_pure_c_fused8_floor17.c` verbatim (eight
`s32` locals loaded from `matrix[0..7]`, one fused `__asm__` with eight `"r"`
inputs). Permuter base score **140**.

Run with `tools/permuter_campaign.py launch … -j 8` (telemetry per the owner's
2026-07-07 directive), waited in-turn with `permuter_campaign.py wait`.

**Result: 31,871 iterations over 1,139 s (~19 min) on 8 workers — ZERO finds.**
Not "no improvement": zero outputs at all, i.e. the permuter never even found a
score-TIE mutation of this chassis. Harvested with `--stop`.

### Campaign 2 — chassis `struct-fields-direct-rvalue-fused8` (structurally different)

Per the fresh-seed rule, reseeded with a materially different chassis rather than
a new seed of the same one: the eight loads are `GteCtl` struct FIELD references
(`m->r0 … m->r7`) fed DIRECTLY as the fused asm's operands, with no named
temporaries at all. This is a different pycparser surface (field refs and a
struct type declaration to mutate; temp INTRODUCTION rather than temp
rearrangement is the permuter's move here) and it was never in sessions 1-3's
hand sweeps.

Sub-result worth banking on its own: this chassis compiles to the **identical**
17-instruction body as the s3 fused form — `lw v0..t2` / `ctc2 v0..t2,$0..$7` /
`jr ra` / `nop`. So the register set `{v0,v1,a1,a2,a3,t0,t1,t2}` survives the
removal of the temporaries entirely, extending H7's invariant from 18 spellings
to 19 and to a struct-typed chassis.

Base score **140** again (same emitted bytes ⇒ same score).

**Result: 85,443 iterations over 1,141 s (~19 min) — exactly ONE find, at score
140, i.e. a TIE with base, at 12 s in.** The find
(`s4/ws2/output-140-1/source.c`) is pure noise: the permuter appended a dead
`int new_var; if (new_var) { new_var = 1; }` after the asm. It changes no emitted
instruction, it is an uninitialized-read dead-local — the [[dead-vars-local-array]]
/ dead-local-holder cheat shape — and it is banked as
`memory/grind/func_80052B00/rejected/permuter-tie-dead-if-noise.c`, not proposed.
Harvested with `--stop`.

### What the numbers mean

117,314 permuter iterations across two structurally distinct chassis produced not
one candidate below the honest floor of 17 (permuter score 140). That is the
expected outcome for the reason s3 already established mechanically, and the
campaign confirms the mechanism rather than merely failing:

- The residual is 16 instructions naming the wrong GPRs plus the unfilled `jr $ra`
  delay slot (H6). The permuter mutates C — statements, temps, types, operand
  order, control flow. It has **no mutation that names a hard register**, so it
  cannot address the 16-point component at all; and it cannot address the
  delay-slot point either, because that is `reorg.c`'s unconditional ASM stop
  (H1, now 32/32 hand-measured spellings plus every one of these 117k
  permutations).
- Campaign 1's ZERO-tie result is itself informative: a 17-instruction body with
  no arithmetic, no control flow and no calls is a search space with essentially
  no interior. Almost every mutation the permuter can make either changes nothing
  (and is not saved) or adds an instruction (and scores worse). There is no basin
  to descend.

### Standing note for future sessions

Do not run another permuter campaign on func_80052B00. Two chassis, ~117k
iterations, telemetry in `metrics/events.jsonl` (`permuter-launch` /
`permuter-harvest` events, labels `fused8-r-constraints` and
`struct-fields-direct-rvalue-fused8`), logs at
`tmp/grind/func_80052B00/s4/ws/campaign.log` and
`tmp/grind/func_80052B00/s4/ws2/campaign.log`. The axis is closed by measurement,
not by argument.

- [s4] Honest floor unmoved at 17 this session (sandbox func_80052B00 --disable all; build 18 insns vs target 17). No edits were made to src/text1b.c at any point - all session-4 measurement ran in a standalone permuter workspace under tmp/.

- [s4] Campaign 1 `fused8-r-constraints` (base = memory/grind/func_80052B00/best_pure_c_fused8_floor17.c, permuter base score 140): 31,871 iterations / 1,139 s / 8 workers / ZERO outputs. Harvested with --stop; telemetry in metrics/events.jsonl.

- [s4] Campaign 2 `struct-fields-direct-rvalue-fused8` (base score 140): 85,443 iterations / 1,141 s / one output at score 140 (tie) at t=12 s. The output is a dead `int new_var; if (new_var) { new_var = 1; }` appended after the asm - no emitted-code change and an uninitialized dead-local cheat shape. Harvested with --stop. Both campaigns confirmed dead via `permuter_campaign.py status` (alive: false, registered_active: false) before this outcome was written; no process outlives the session.

- [s4] New reusable fact for anyone building a permuter workspace in this repo: maspsx emits its output lines with NO leading tab, so a function-region extraction awk must match `^[ \t]*\.ent[ \t]+<func>$`, not the `^\t\.ent\t<func>$` form in tools/mar_perm_workspace.sh (which silently yields an unassemblable fragment: '.frame outside of .ent'); and compile.sh must `source .venv/bin/activate` itself or prologue_fix/maspsx fail with an empty-stdin error.

- [s4] The minimal standalone TU reproduces the in-tree codegen for this function exactly, so TU context is not a variable here (H9).

- [s4] The struct-field / no-temporaries chassis emits the identical 18-instruction body and the identical {v0,v1,a1,a2,a3,t0,t1,t2} register set as the s3 eight-locals fused form - a 19th spelling with the register set unchanged (H7).

- [s4] Modality ladder status after this session: recon (s1), structural (s2 and s3, the second of which moved the floor 18 -> 17 by fusing the eight cop2 writes into one __asm__), and permuter (s4) are all measured dead or exhausted. The residual 17 points remain exactly (a) 16 instructions naming the wrong GPRs and (b) the one unfilled jr $ra delay slot, both with compiler-source mechanisms (no REG_ALLOC_ORDER in mips.h; reorg.c stop_search_p).

- [s4] This session did NOT touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, the Makefile, any linker script, or src/. Working-tree delta is the three func_80052B00 ledger files, one new rejected/ form, and metrics/events.jsonl (engine-written).

## Session 5 (permuter — DIRECTED, 2026-08-01) — floor 17, unmoved; the directed permuter sub-modality is now measured dead too

Session 4 killed the UNDIRECTED permuter axis (117,314 iterations of random
mutation across two chassis). The driver mandated the permuter modality again,
and the session-5 brief specifies the *directed* variant ("Directed permuter on
the diverging region: tools/permuter_annotate.py --func <f> --hint <rule-slug>").
Those are genuinely different searches — random mutation samples an
unstructured neighbourhood, whereas the PERM_* macros make decomp-permuter
ENUMERATE a cross-product the author chooses — so session 5 is not a re-run of
session 4, and the standing "do not run another permuter campaign" note from s4
is now discharged for both sub-modalities rather than left as a one-sided claim.

### tools/permuter_annotate.py has no applicable hint (recorded so nobody retries it)
`--list-hints` offers exactly four slugs: `register-asm-pins`,
`shared-end-label`, `loop-rotation-two-shift`, `loop-counter-fills-load-delay`.
Every one of them requires a construct func_80052B00 does not contain (a
register pin — forbidden here anyway; multiple return paths; a rotate; a loop).
The tool is an explicit proof-of-concept with an intentionally small catalog, so
the directed chassis was hand-authored per `.claude/rules/permuter-directives.md`
instead. That is the sanctioned route, not a workaround; the annotate helper is
a convenience layer over the same macros.

### Chassis 3 — `directed-lineswap8-x-asmshape4` (the exhaustive cross-product)
`tmp/grind/func_80052B00/s5/ws3/base.c`, built by copying session 4's validated
workspace (`compile.sh`, `target.o`, `settings.toml`, `prelude_r3k.inc`) and
replacing only `base.c`:

- `PERM_LINESWAP(...)` over the eight load statements ⇒ all **8! = 40,320**
  orderings of `tN = matrix[N];`.
- `PERM_GENERAL(...)` over **four** asm-operand shapes of the fused cop2 write:
  eight `"r"` inputs (the s3/s4 best pure-C body), eight `"d"` inputs, eight
  `"r"` inputs plus a ninth `"r"(matrix)` keeping the base pointer live through
  the asm, and a 7+1 split with the CR7 write in its own trailing `__asm__`.

Cross-product = **161,280 combinations**, and decomp-permuter confirmed it is
enumerating rather than sampling: the log's first lines read
`Will run for 161280 iterations.` and `[func_80052B00] base score = 140`.
Each of the four asm shapes had been measured individually at 17 in session 3;
what was never tested — by the s1-s3 hand sweeps (which covered exactly TWO load
orders, ascending and descending) or by s4's undirected campaigns — is whether
any of them improves under some OTHER load ordering.

### Result — zero outputs, and a strictly BIMODAL score distribution
46,653 iterations / 1,869 s / 8 workers, harvested with `--stop` under the
fresh-seed rule (~28 min past a fresh seed with no novel find). **Zero outputs**
— not one form at or below the base score was ever saved.

Histogram over the 46,658 scores logged
(`tmp/grind/func_80052B00/s5/score_histogram.txt`):

| score | samples | meaning |
|---|---|---|
| **140** | 35,111 | = base = the honest floor of 17 |
| **180** | 11,547 | base + 40 = eight additional register-name mismatches at the permuter's 5-points-per-register weight |
| anything else | **0** | — |

Nothing between 140 and 180, and **nothing below 140**. This is a sharper
version of session 4's "no interior" observation and it says something new and
specific: load ORDER is *not* inert here — it is the only thing in this whole
space that moves the score — but it moves it only UP, and only by permuting
which pseudo lands on which ascending hard register. It never moves the register
SET off `{v0,v1,a1,a2,a3,t0,t1,t2}`. That is precisely what H7's mechanism
predicts: with no `REG_ALLOC_ORDER` in `tools/gcc-2.7.2/config/mips/mips.h`,
local-alloc's `find_free_reg` scans hard registers in plain ascending number
order starting at `$2` and takes the first non-conflicting one, and a call-free
leaf offers neither a conflict nor a copy suggestion to move that start point.
Reordering the loads reshuffles the *assignment* within a fixed set; it cannot
change the set.

### Artifacts
- `tmp/grind/func_80052B00/s5/ws3/base.c` — the directed chassis
- `tmp/grind/func_80052B00/s5/ws3/campaign.log` — full campaign log
- `tmp/grind/func_80052B00/s5/score_histogram.txt` — the histogram above
- `memory/grind/func_80052B00/rejected/directed-perm-lineswap8-x-asmshape4-nothing-below-140.c`

- [s5] Honest floor unmoved at 17. No src/ edits at any point this session: all measurement ran in a standalone permuter workspace under tmp/, and `git status --porcelain` showed only metrics/events.jsonl both before and after.

- [s5] DIRECTED PERMUTER AXIS KILLED BY MEASUREMENT. Chassis `directed-lineswap8-x-asmshape4`: PERM_LINESWAP over the eight load statements (8! = 40,320 orderings) crossed with PERM_GENERAL over four asm-operand shapes (eight "r" inputs / eight "d" inputs / eight "r" plus a ninth "r"(matrix) / a 7+1 split with the CR7 write in its own trailing __asm__) = 161,280 combinations, which decomp-permuter enumerates exhaustively ("Will run for 161280 iterations", base score 140). 46,653 iterations / 1,869 s / 8 workers / ZERO outputs; harvested with --stop under the fresh-seed rule.

- [s5] The score distribution over the directed space is strictly BIMODAL: 140 (= base = honest floor 17) in 35,111 logged samples and 180 (= base + 40 = eight extra register-name mismatches at 5 points per register) in 11,547, with nothing in between and nothing below 140. New information beyond s4's "no interior": load ORDER is the one lever in this space that moves the score at all, and it moves it only UPWARD, by permuting which pseudo lands on which ascending hard register. It never moves the register SET off {v0,v1,a1,a2,a3,t0,t1,t2}, exactly as H7's no-REG_ALLOC_ORDER / ascending-find_free_reg mechanism predicts.

- [s5] The s1-s3 hand sweeps covered exactly TWO of the 40,320 load orderings (ascending and descending). Session 5 covered ~46.6k of the 161,280 cross-product points (~29%), spread across all four asm shapes. Combined with s4's 117,314 undirected iterations, the permuter modality has now consumed 163,967 iterations on this function across THREE chassis and two sub-modalities, with a single score-TIE (s4's dead-local noise form) as the entire yield.

- [s5] tools/permuter_annotate.py is NOT applicable to this function and a future session should not spend a turn on it: `--list-hints` offers only register-asm-pins, shared-end-label, loop-rotation-two-shift and loop-counter-fills-load-delay, each of which requires a construct this body does not contain (a register pin — forbidden here regardless; multiple return paths; a rotate; a loop). The catalog is an explicit proof-of-concept. Hand-authoring the PERM_* chassis per .claude/rules/permuter-directives.md is the sanctioned route.

- [s5] Reusable mechanic for any future directed campaign in this repo: session 4's workspace is a drop-in harness — copy tmp/grind/func_80052B00/s4/ws (compile.sh, target.o, settings.toml, prelude_r3k.inc), delete base.c/base.o, write the PERM_*-annotated base.c, and launch with tools/permuter_campaign.py. permuter_campaign.py launch reports `base_score: null` for a PERM_*-annotated base (it cannot pre-score a file containing macros), which is NOT an error — the real base score appears in campaign.log as `[func_80052B00] base score = 140`.

- [s5] Both prior campaigns (s4 ws and ws2) and this one confirmed dead via `permuter_campaign.py status` (alive: false, registered_active: false) before this outcome was written; harvest --stop reaped 9 processes. No campaign outlives the session.

- [s5] This session did NOT touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile, any linker script, or src/. No queue done, no retire, no commit.

- [s5] [s5] Honest floor unmoved at 17. No src/ edits at any point: all measurement ran in a standalone permuter workspace under tmp/, and `git status --porcelain` showed only metrics/events.jsonl both before and after.

- [s5] [s5] DIRECTED PERMUTER AXIS KILLED BY MEASUREMENT. Chassis `directed-lineswap8-x-asmshape4`: PERM_LINESWAP over the eight load statements (8! = 40,320 orderings) crossed with PERM_GENERAL over four asm-operand shapes (eight "r" inputs / eight "d" inputs / eight "r" plus a ninth "r"(matrix) / a 7+1 split with the CR7 write in its own trailing __asm__) = 161,280 combinations, which decomp-permuter enumerates exhaustively ('Will run for 161280 iterations', base score 140). 46,653 iterations / 1,869 s / 8 workers / ZERO outputs; harvested with --stop under the fresh-seed rule after ~28 min with no novel find.

- [s5] [s5] The score distribution over the directed space is strictly BIMODAL: 140 (= base = honest floor 17) in 35,111 logged samples and 180 (= base + 40 = eight extra register-name mismatches at 5 points per register) in 11,547, with nothing in between and nothing below 140. New information beyond s4's 'no interior' finding: load ORDER is the one lever in this space that moves the score at all, and it moves it only UPWARD, permuting which pseudo lands on which ascending hard register without ever moving the set off {v0,v1,a1,a2,a3,t0,t1,t2} - exactly as H7's no-REG_ALLOC_ORDER / ascending-find_free_reg mechanism predicts.

- [s5] [s5] The s1-s3 hand sweeps covered exactly TWO of the 40,320 load orderings (ascending and descending). Session 5 covered ~46.6k of the 161,280 cross-product points (~29%) spread across all four asm shapes. Combined with s4's 117,314 undirected iterations, the permuter modality has now consumed 163,967 iterations on this function across THREE chassis and BOTH sub-modalities, with a single score-TIE (s4's dead-local noise form) as the entire yield.

- [s5] [s5] tools/permuter_annotate.py is NOT applicable here and a future session should not spend a turn on it: `--list-hints` offers only register-asm-pins, shared-end-label, loop-rotation-two-shift and loop-counter-fills-load-delay, each requiring a construct this body does not contain (a register pin - forbidden here regardless; multiple return paths; a rotate; a loop). Its catalog is an explicit proof-of-concept, so hand-authoring the PERM_* chassis per .claude/rules/permuter-directives.md is the sanctioned route, not a workaround.

- [s5] [s5] Reusable mechanic for any future directed campaign in this repo: session 4's workspace is a drop-in harness - copy tmp/grind/func_80052B00/s4/ws (compile.sh, target.o, settings.toml, prelude_r3k.inc), delete base.c/base.o, write the PERM_*-annotated base.c, launch with tools/permuter_campaign.py. Note that `launch` reports "base_score": null for a PERM_*-annotated base (it cannot pre-score a file containing macros); that is NOT an error - the real base score appears in campaign.log as '[func_80052B00] base score = 140'.

- [s5] [s5] All three permuter campaigns on this function (s4 ws, s4 ws2, s5 ws3) confirmed dead via `permuter_campaign.py status` (alive: false, registered_active: false) before this outcome was written; harvest --stop reaped 9 processes. No campaign outlives the session.

- [s5] [s5] Modality ladder status: recon (s1), structural (s2 and s3, the latter moving the floor 18 -> 17 by fusing the eight cop2 writes into one __asm__), permuter-undirected (s4) and permuter-directed (s5) are all measured dead or exhausted. The residual 17 points remain exactly (a) 16 instructions naming the wrong GPRs and (b) the one unfilled jr $ra delay slot, both with compiler-source mechanisms (no REG_ALLOC_ORDER in mips.h; reorg.c:730-735 stop_search_p).

- [s5] [s5] This session did NOT touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile, any linker script, or src/. No queue done, no retire, no commit. No form from this session is proposed: the directed chassis is a measurement vehicle whose PERM_* macros are not compilable C, and its best expansion is byte-for-byte the already-banked best_pure_c_fused8_floor17.c.

- [s6] [s6] Forensics vehicle: the honest floor-17 fused body compiled in a minimal standalone TU (faithful per H9) with the INSTRUMENTED cc1 at tools/gcc-2.7.2/cc1 (NOT build/cc1, per instrumented-cc1-location) and -da, flags -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w. Full pass dump set written to tmp/grind/func_80052B00/s6/dumps/ (.rtl .jump .cse .loop .cse2 .flow .combine .sched .lreg .greg .sched2 .dbr). Scripts: dump.sh, controls.sh (ctlA/ctlB/ctlC), ctlD.sh, ctlE.sh, ctlF.sh (ctlF/ctlG).

- [s6] [s6] LEDGER CORRECTION carried by sessions 3, 4 and 5: the register naming is NOT decided by local-alloc's find_free_reg. combine folds all eight (mem) loads into the fused asm's operands (9 (set (reg ...)) insns survive through .flow, 0 survive .combine), so the .lreg dump contains a single insn - the asm_operands with eight (mem:SI (plus (reg:SI 4 a0) (const_int N))) operands - and local-alloc has nothing to allocate. The eight lw in the output are reload-generated. The observable conclusion (set = {v0,v1,a1,a2,a3,t0,t1,t2}) was right; the named mechanism was wrong.

- [s6] [s6] The correct mechanism is reload1.c:3606 order_regs_for_reload(), #else branch (mips.h defines no REG_ALLOC_ORDER): potential_reload_regs is filled with zero-use call_used regs in ascending regno, then zero-use call-saved regs, then already-used regs sorted by use count. reload1.c:486 copies regs_ever_live into regs_explicitly_used and :3651 adds large+1 uses plus a bad_spill_regs bit for those - which is exactly why $4 is skipped mid-run. The .greg dump prints the decision verbatim: ';; Need 8 regs of class GR_REGS (for insn 34)' / 'Spilling reg 2. 3. 5. 6. 7. 8. 9. 10.'

- [s6] [s6] The target's register set is REACHABLE, at a measured price. ctlE (fused body + five extra locals loaded from matrix[8..12] and stored back) emits lw $8,0($4) .. lw $15,28($4) and ctc2 $8,$0 .. ctc2 $15,$7 - the target's registers, order and offsets exactly - because $2,$3,$5,$6,$7 are occupied so reload's ascending scan starts at $8. Price: 10 extra instructions (27 vs the target's 17). Six sessions of "the register set never moves" was true of every spelling tried and false as a mechanism claim.

- [s6] [s6] No zero-instruction occupancy exists via the obvious route: unused register parameters (ctlF: s32 p1, s32 p2, s32 p3; ctlG: s32 p1) do not set regs_ever_live, and both .greg dumps come back with the unchanged spill list 2,3,5,6,7,8,9,10 and byte-identical bodies. Untested zero-cost candidates for any future session: a value returned in $v0, a DImode return occupying $v0+$v1, setjmp, alloca - all four either emit instructions or cannot occur in a 17-instruction leaf.

- [s6] [s6] combine CAN be defeated (ctlC gives each value a second use via an empty asm - a scheduling-barrier cheat, PROBE ONLY): all 9 sets survive, .greg needs no spills, and local-alloc assigns the eight pseudos DESCENDING $10,$9,$8,$7,$6,$5,$3,$2. Same candidate pool, reverse order, still nothing above $10. This retroactively explains session 3's volatile-pointer probe scoring 15 - volatile MEMs are non-substitutable, so that probe was measuring the local-alloc path, not a property of volatile.

- [s6] [s6] H1 now has a compiler-internal differential. Honest body .dbr header: ';; Reorg pass #1 ... 3 insns needing delay slots / 3 got 0 delays', and the return stays a bare (jump_insn ... (parallel[(return)(use (reg:SI 31 ra))]) 305 {return_internal}) instead of becoming a (sequence); cc1's own asm output is 'j $31' with no .set noreorder block. ctlA - the SAME body with one plain C store (matrix[0] = 0) after the asm - flips to '2 got 0 delays, 1 got 1 delays' and emits '.set noreorder / j $31 / sw $0,0($4)'. So reorg is willing and able to fill this jr; the asm insn is the only thing stopping it, exactly as reorg.c:730-735 stop_search_p specifies.

- [s6] [s6] Provenance inference for the owner (evidence, not a self-authorization): GCC in this toolchain can produce the target's register names OR the target's 17-instruction length, but not both, and it can never produce the delay-slot ctc2 from C at all. The target does all three. That is affirmative evidence the target bytes were not produced by a compiler allocating registers for these loads - consistent with the PsyQ libgte cop2-macro provenance already recorded for this family and with the canonical-asm disposition carried since session 1.

- [s6] [s6] This session did NOT touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile, any linker script, or src/. No sandbox edit was made, no queue done, no retire, no commit. The floor is unchanged at 17 and the proposed disposition is unchanged. One new rejected form was banked: rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c.

- [s6] Forensics vehicle: the honest floor-17 fused body (memory/grind/func_80052B00/best_pure_c_fused8_floor17.c) compiled in a minimal standalone TU with the INSTRUMENTED cc1 at tools/gcc-2.7.2/cc1 (NOT build/cc1, per instrumented-cc1-location) and -da, flags -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w. Full pass dump set (.rtl .jump .cse .loop .cse2 .flow .combine .sched .lreg .greg .sched2 .dbr) in tmp/grind/func_80052B00/s6/dumps/.

- [s6] LEDGER CORRECTION for the mechanism carried by sessions 3, 4 and 5: the register naming is not local-alloc's find_free_reg. combine folds all eight (mem) loads into the fused asm's operands (9 (set (reg ...)) survive .flow, 0 survive .combine), the .lreg dump holds a single asm_operands insn with eight MEM operands, and the eight lw in the output are reload-generated.

- [s6] The correct decision is reload1.c:3606 order_regs_for_reload(), #else branch: potential_reload_regs = zero-use call_used regs in ascending regno, then zero-use call-saved regs, then already-used regs by use count; reload1.c:486 + :3651 make explicitly-used regs bad spill regs, which is why $4 is skipped mid-run. .greg prints the decision verbatim.

- [s6] ctlE reproduces the target's register naming EXACTLY in pure C - lw $8,0($4) .. lw $15,28($4) / ctc2 $8,$0 .. ctc2 $15,$7 - by occupying $2,$3,$5,$6,$7 with five other live values; it costs 10 extra instructions (27 vs the target's 17). Banked as rejected/regocc5-reproduces-t0t7-costs-10-extra-insns.c.

- [s6] No zero-instruction occupancy exists via the obvious route: unused register parameters (ctlF, ctlG) do not set regs_ever_live and leave the spill list and the emitted body byte-identical. Untested zero-cost candidates for a future session: a $v0 return value, a DImode return occupying $v0+$v1, setjmp, alloca - all four either emit instructions or cannot occur in a 17-instruction leaf.

- [s6] combine CAN be defeated (ctlC, an empty second asm - a scheduling-barrier cheat, probe only): all 9 sets survive, no spills are needed, and local-alloc assigns the eight pseudos DESCENDING $10,$9,$8,$7,$6,$5,$3,$2. Same candidate pool, reverse order, nothing above $10. This also explains session 3's volatile probe scoring 15.

- [s6] H1 differential: the honest body's .dbr header reads '3 insns needing delay slots / 3 got 0 delays' with the return left as a bare {return_internal} jump_insn and cc1 emitting 'j $31' with no .set noreorder block; ctlA - the same body plus one plain C store after the asm - flips to '2 got 0 delays, 1 got 1 delays' and emits '.set noreorder / j $31 / sw $0,0($4)'.

- [s6] Provenance inference for the owner (evidence, not a self-authorization): GCC in this toolchain can produce the target's register names OR the target's 17-instruction length, never both, and can never produce the delay-slot ctc2 from C at all. The target does all three at once - affirmative evidence that these register names were not produced by a compiler allocating registers for these loads, consistent with the PsyQ libgte cop2-macro provenance already recorded for this family.

- [s6] Floor unchanged at 17. No form from this session is proposed: ctlC and ctlE are mechanism probes, and ctlE is banked as a rejected form with the arithmetic that kills it.

- [s6] This session did NOT touch regfix.txt, asmfix.txt, inline_asm_canonical.txt, engine/, tools/, .claude/rules/, the Makefile, any linker script, or src/. No sandbox edit, no queue done, no retire, no commit, no background process launched.

## Session 7 (forensics, 2026-08-01) — floor 17, unmoved

**Modality:** forensics. **Mandate from the s6 frontier:** one ctl sweep of the
four constructs that might occupy $2,$3,$5,$6,$7 at zero cost (a $v0 return
value, a DImode return, setjmp, alloca), read via the `.greg` spill list, to
close the last theoretical gap in the register argument. Explicitly NOT another C
spelling sweep and NOT a fourth permuter campaign.

**Vehicle.** The s6 vehicle reused verbatim: the honest floor-17 fused body in a
minimal standalone TU (faithful per H9), compiled with the instrumented
`tools/gcc-2.7.2/cc1` (NOT `build/cc1`, per [[instrumented-cc1-location]]) at
`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w
-da`. Scores come from splicing the same shapes into `src/text1b.c` and running
`sandbox func_80052B00 --disable all`, restoring the original body in a `finally`
block (the s3 harness).

### Facts established

1. **`reload1.c:3606 order_regs_for_reload()` has exactly two entries into
   `bad_spill_regs`** — `fixed_regs[i]`, and `regs_explicitly_used[i]` (which
   `reload1.c:486` copies wholesale from `regs_ever_live[]`) — plus the
   `hard_reg_n_uses[]` counts contributed by pseudos already assigned to hard
   registers. Source saved verbatim at
   `tmp/grind/func_80052B00/s7/reload1_order_regs_for_reload.txt`.

2. **The target cannot supply its own occupancy (H16).** func_80052B00's 17
   target instructions mention only $4, $8..$15 and $31. $2, $3, $5, $6 and $7
   appear nowhere. A C source emitting exactly those 17 instructions would leave
   all five available as spill registers, so `order_regs_for_reload` would hand
   the eight reloads $2,$3,$5,$6,$7,$8,$9,$10 — the observed honest output — not
   $8..$15. The shipped bytes are therefore not a fixed point of this compiler
   configuration.

3. **Six differential controls, all four frontier constructs measured**
   (`s7/ctl_occupancy.sh`, dumps in `s7/dumps/`, raw log
   `s7/ctl_occupancy_out.txt`):
   - `ctlH` s32 `return 0` — 18 cc1 insns, regs $3,$5,$6,$7,$8,$9,$10,$11.
     Occupies $2 only; `move $2,$0` is absorbed by the previously empty `jr $ra`
     delay slot, so emitted length is UNCHANGED. **The only zero-cost occupancy
     construct that exists for this body — and it buys one register, not five.**
   - `ctlI` `long long return 0` — 19 insns (+2), regs $5..$12. Occupies $2 and
     $3; `move $2,$0 / move $3,$0 / j $31`, only one move could have fit the slot
     and reorg took neither.
   - `ctlJ` varargs `(s32 *matrix, ...)` — 21 insns (+4), register set
     **UNCHANGED** at $2,$3,$5..$10 despite emitting `sw $5,4($sp) / sw $6,8($sp)
     / sw $7,12($sp)`. `setup_incoming_varargs` homing does not reach
     `regs_ever_live` in time for `order_regs_for_reload`. This kills
     [[fake-varargs-explicit-homing]] as a lever here.
   - `ctlK` DImode return + varargs — 23 insns, regs $5..$12: varargs adds
     nothing on top of the DImode return.
   - `ctlL` setjmp — 38 insns; the call destroys the leaf and all eight values
     round-trip through the stack.
   - `ctlM` alloca — 45 insns; frame pointer plus callee-saved $16..$23 with full
     save/restore.
   - `ctlBASE` — 17 cc1 insns (18 after maspsx inserts the delay-slot `nop`),
     regs $2,$3,$5..$10, `.greg` "Spilling reg 2. 3. 5. 6. 7. 8. …" exactly as s6
     recorded.

4. **The register axis is PRICED, not unreachable, and the price is
   score-positive (H18).** `s7/sweep_price.py` scored six shapes with
   `sandbox --disable all`:

   ```
   base-fused8               score=17  build=18  ctc2=v0,v1,a1,a2,a3,t0,t1,t2
   ret-s32-zero              score=17  build=18  ctc2=v1,a1,a2,a3,t0,t1,t2,t3
   ret-dimode-zero           score=19  build=20  ctc2=a1,a2,a3,t0,t1,t2,t3,t4
   occ3-param-store          score=20  build=21  ctc2=v0,v1,t0,t1,t2,t3,t4,t5
   occ5-param-store-dimode   score= 7  build=23  ctc2=t0,t1,t2,t3,t4,t5,t6,t7
   occ5-locals-ctlE          score=11  build=27  ctc2=t0,t1,t2,t3,t4,t5,t6,t7
   ```

   Sixteen of eighteen emitted instructions differ only in GPR name, so buying
   the right names is worth 16 points while each extra emitted instruction costs
   1 — six extra instructions net ten points (17 -> 7). **This corrects the
   wording carried since session 1**: the register axis was never "unreachable",
   it is reachable only by constructs that change the function's contract.
   `occ5-locals-ctlE` is the s6 shape whose score s6 never measured; it is 11.

5. **Both sub-17 forms are cheats and are banked as rejected, so the honest floor
   stays 17.** `occ5-param-store-dimode` adds three parameters no caller passes
   and writes their garbage contents to `matrix[8..10]`, and changes the return
   type from `void` to `long long`; `occ5-locals-ctlE` reads and writes
   `matrix[8..12]`. Both are register pins expressed through the function
   signature — same intent as `register T x asm("$N")`, one more spelling — and
   neither can reach 0 anyway (extra instructions have no home in the target's
   17; the delay slot is still `nop` per H1).

### Net effect on the disposition

Session 7 does not move the floor, and it was not expected to. What it adds is a
SECOND independent impossibility result. H1 (sessions 1-5) says the delay-slot
`ctc2` can never come from C because `reorg.c:730-735 stop_search_p` halts
`fill_simple_delay_slots` at any asm insn. H16 (this session) says the register
NAMES can never come from C either, because `reload1.c:3606` requires occupancy
of $2,$3,$5,$6,$7 that the target's own 17 instructions do not provide. The two
proofs are orthogonal and together they say the shipped bytes are not output of
GCC 2.7.2 in this configuration from any C source — affirmative provenance
evidence for hand-written asm, and the reason the HEAD body needs eight
`register asm("$N")` pins PLUS `regfix.txt:3411` to reproduce them.

### Artifacts

- `tmp/grind/func_80052B00/s7/ctl_occupancy.sh` — the six-control cc1 -da sweep
- `tmp/grind/func_80052B00/s7/ctl_occupancy_out.txt` — its raw output
- `tmp/grind/func_80052B00/s7/dumps/` — per-control `.c`, `.s` and cc1 pass dumps
  (`.greg`, `.lreg`, `.combine`, `.dbr`, …)
- `tmp/grind/func_80052B00/s7/reload1_order_regs_for_reload.txt` — the verbatim
  `order_regs_for_reload()` source that H16 rests on
- `tmp/grind/func_80052B00/s7/sweep_price.py` — the in-tree scoring harness
- `tmp/grind/func_80052B00/s7/sweep_price_results.txt` — the price curve
- `memory/grind/func_80052B00/rejected/occ5-param-store-dimode-score7-breaks-abi.c`
- `memory/grind/func_80052B00/rejected/ret-s32-zero-only-zero-cost-occupancy-buys-one-register.c`

- [s7] reload1.c:486 copies regs_ever_live[] wholesale into regs_explicitly_used[], and order_regs_for_reload() at reload1.c:3606 has exactly two entries into bad_spill_regs - fixed_regs[i], and regs_explicitly_used[i] (which also gets `large + 1` uses) - plus hard_reg_n_uses[] from pseudos already assigned to hard registers. Verbatim source saved at tmp/grind/func_80052B00/s7/reload1_order_regs_for_reload.txt.

- [s7] The target's 17 instructions mention only $4, $8..$15 and $31. $2, $3, $5, $6 and $7 appear nowhere in them, so a C source emitting exactly the target stream would leave all five spillable and reload would allocate $2,$3,$5,$6,$7,$8,$9,$10 instead of $t0..$t7. The shipped register allocation is unreachable from the shipped instruction stream (H16).

- [s7] H16 is orthogonal to H1: H1 (reorg.c:730-735 stop_search_p) says the delay-slot ctc2 can never come from C; H16 (reload1.c:3606) says the register names can never come from C either. Together the two proofs say these bytes are not GCC 2.7.2 output from any C source in this configuration - affirmative provenance evidence for hand-written asm, and the reason the HEAD body needs eight register asm("$N") pins PLUS regfix.txt:3411 to reproduce them.

- [s7] An s32 return value is the only zero-cost register-occupancy construct that exists for this body: it occupies $2 and its `move $2,$0` is absorbed by the previously empty jr $ra delay slot, leaving the emitted length unchanged at 18. It buys one of the five registers needed and the score stays 17.

- [s7] Varargs is dead as an occupancy lever here: ctlJ emits sw $5,4($sp) / sw $6,8($sp) / sw $7,12($sp) yet the reload register set is UNCHANGED at $2,$3,$5..$10, and it costs +4 instructions. This closes the fake-varargs-explicit-homing family for func_80052B00.

- [s7] setjmp (+21 insns, all eight values spilled across the call) and alloca (+28 insns, frame pointer plus callee-saved $16..$23) are both catastrophic, not levers.

- [s7] Price curve from `sandbox --disable all`: base-fused8 17 (build 18) / ret-s32-zero 17 (18) / ret-dimode-zero 19 (20) / occ3-param-store 20 (21) / occ5-param-store-dimode 7 (23) / occ5-locals-ctlE 11 (27). The last two reproduce ctc2 t0..t7 exactly. This is the first time any form has scored below 17 - and both are cheats (fake parameters, fake return type, dead stores past the array), so the honest floor is unchanged.

- [s7] Sessions 1-6 recorded the register axis as mechanically unreachable; the accurate statement, now measured, is that it is reachable only by constructs that change the function's contract, and that the trade is score-positive by 10 points. The ledger wording has been corrected in place (hypotheses.md H18, evidence.md Session 7, candidate.c header).

- [s7] src/text1b.c was left byte-identical to HEAD (the sweep harness restores the original body in a finally block; `git status --porcelain` shows only ledger files plus metrics/events.jsonl).
