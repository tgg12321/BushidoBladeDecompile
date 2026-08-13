# Evidence bank — func_80083794

## Session 1 (recon, 2026-08-13)

### Identity — this function is GCC's `__main` (libgcc2.c), not game code
- `canonical func_80083794` → verdict **C**, asm_insns 0, total 28, distance 23.
- Sole caller: `main()` at `src/ings.c:589`, and it is the **first statement of
  main** — exactly where GCC 2.7.2 auto-emits `jal __main`. (BB2's decomp spells
  the call explicitly because naming a function `main` here is codegen-neutral and
  does not trigger `expand_main_function` — see memory
  `naming-main-rename-codegen-neutral`.)
- Body is libgcc2.c's `__main` with `__do_global_ctors` inlined:
  `static int initialized; if (!initialized) { initialized = 1; <ctor loop>; }`
  * `D_800A2668` = the `initialized` flag.
  * `D_8008D070` = ctor table base (`la $s0, %hi/%lo(D_8008D070)`).
  * `D_00000000` = a link-time ABSOLUTE symbol whose *value* (0 here) is the ctor
    count — target does `lui $s1,(0x0>>16); addiu $s1,$s1,0x0`, an address
    computation, not a load. Our C spells it `(s32)&D_00000000`, which reproduces
    the `la` pair exactly.
- `asm/funcs/func_80083794.s` contains TWO bodies: 0x80083794 (the scored one,
  28 insns) and an unlabelled twin at 0x80083804 (identical except the flag test
  is inverted — `beqz` instead of `bnez`, and no `initialized = 1` store; i.e. the
  `__do_global_dtors`-shaped sibling). Only the first 28 insns are scored.
- Region context: this sits immediately after `_start` (0x800836EC) and the
  Marionation break trampolines (func_800836B8 / func_800836C8 / func_80083698),
  **all four of which are already owner-authorized canonical asm** for ings2.c
  (inline_asm_canonical.txt, grant 2026-08-06). func_80083794 is the crt0/libgcc
  continuation of that cluster.

### Measured floor progression (honest sandbox, `--disable all`)
| form | score | build_insns |
|---|---|---|
| inherited (register pins + hardcoded-`$17` `__asm__` + `jalr` asm) | 23 | 16 (asm stripped ⇒ body gutted) |
| clean pure C, `p` assigned before `count` | 22 | 28 |
| clean pure C, `count` assigned before `p` | **18** | 28 |
| same, with `count` **declared** before `p` | 18 | 28 (declaration order is INERT here) |

Score 18 is the session floor and the state left in `src/ings2.c`.

### Structural result: our build is instruction-for-instruction isomorphic to target
At floor 18 our 28 insns line up 1:1 with the target's 28 in the same order,
including the two subtleties that usually cost a session:
- the `lui/lw` of the `initialized` flag is emitted **before** `addiu $sp` (the
  MIPS prologue in this cc1 IS schedulable RTL, so this is free), and
- the `bnez` branch delay slot is filled from the prologue/flag-store region.
The whole residual is register naming + frame geometry, not control flow.

### Residual class A — frame geometry (the dominant, ~8-insn class)
- Target frame: `addiu $sp,-0x10`; saves `s0@4, s1@8, ra@0xC`; 4 pad bytes at
  offset 0. **Zero bytes of o32 outgoing-argument area**, despite the body
  containing a `jalr`.
- Our build: `addiu $sp,-0x20`; saves `s0@16, s1@20, ra@24`; i.e. the standard
  16-byte `REG_PARM_STACK_SPACE` argument area at the bottom of the frame.
- Mechanism (compiler source, not speculation):
  `tools/gcc-2.7.2/config/mips/mips.h:1822` — `REG_PARM_STACK_SPACE(FNDECL) =
  (MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET(FNDECL)`, and
  `FIRST_PARM_OFFSET(FNDECL)` is `0` (mips.h:1811) for our config ⇒ 16 bytes,
  unconditionally, for any function that makes a call.
  `tools/gcc-2.7.2/config/mips/mips.c:4466` — `args_size =
  MIPS_STACK_ALIGN(current_function_outgoing_args_size)` and
  `total_size = var_size + args_size + extra_size` (mips.c:4476), with the saved
  registers placed ABOVE `args_size`. There is no C-level input that zeroes
  `current_function_outgoing_args_size` for a calling function.
- **Corpus check (tmp/grind/func_80083794/s1/framescan.py + .txt):** scanned all
  1437 `asm/funcs/*.s`; of every function that contains a `jal`/`jalr` and has a
  register-save `sw`, the number whose lowest save offset is `< 16` is **1** —
  func_80083794 itself. Every other calling function in the shipped executable
  carries the 16-byte argument area. This is a corpus-wide uniqueness result, not
  a plateau observation.

### Residual class B — register assignment
- Target: `$s0` = ctor pointer `p`, `$s1` = `count`, `$t0` = the flag temp AND the
  loaded function pointer.
- Ours: `$s0` = `count`, `$s1` = `p`, `$v0` = the flag temp / loaded fn pointer.
- `p` has 3 refs (`la`, `lw` base, `addiu +4`), `count` has 4 (`la`, `beqz`,
  `addiu -1`, `bnez`), so `count` outranks `p` for `$s0` under allocno priority.
  Target's assignment is the reverse.
- `$v0`-vs-`$t0` for the temp: GCC 2.7.2's call-clobbered allocation order takes
  `$v0` first; nothing in the C changes which caller-save class member is picked
  when only one is needed.

### Residual class C — `ori` vs `li`
- Target sets the flag with `ori $t0, $zero, 1` (encoding `0x34080001`); our build
  emits `li $v0,1` = `addiu $v0,$zero,1` (`0x24020001`). regfix.txt:110 is a
  standing `subst "addiu" "ori" @ 6` papering over exactly this.
- `ori $rX, $zero, imm` is NOT unique to this function: 53 of 1437 `asm/funcs/*.s`
  contain the form, so it is reachable — but no C spelling producing it from a
  plain `= 1` store was found this session (untested axis, see hypotheses H3).

### Gate results (recorded so no future session re-runs them)
- `tools/scan_hand_coded.py --single func_80083794` → **tier=LOW, score 0/8**;
  S1/S2/S6 all negative. The canonical-asm endgame gate (STRONG tier required)
  therefore **FAILS on the standard scanner**. Note the scanner has no frame-
  geometry signal, which is the one anomaly this function actually exhibits.
- `tmp/duplicates_leads.txt` has no entry for 0x80083794 / 0x80083804; no sibling
  analog outside the file's own twin body.

- [s1] Identity: func_80083794 is GCC's __main from libgcc2.c with __do_global_ctors inlined. Its sole caller is main() at src/ings.c:589, as the FIRST statement of main — exactly where GCC 2.7.2 emits `jal __main`. D_800A2668 is the `initialized` flag, D_8008D070 is the ctor table base, D_00000000 is a link-time ABSOLUTE symbol whose VALUE (0) is the ctor count, which is why target does an address computation (lui/addiu) and not a load.

- [s1] canonical func_80083794 -> verdict C, asm_insns 0, total 28, distance 23. Honest sandbox floor moved 23 -> 22 -> 18 this session; src/ings2.c is left carrying the score-18 pure-C form.

- [s1] At floor 18 our 28 instructions line up 1:1 with the target's 28 in the same order. Both subtleties that usually cost a session are already free: the lui/lw of the flag is emitted BEFORE addiu $sp (the MIPS prologue is schedulable RTL in this cc1), and the bnez delay slot is filled from the prologue/flag-store region. The entire residual is register naming plus frame geometry — no control-flow gap.

- [s1] Residual class A (dominant, ~8 of the 18): frame geometry. Target addiu $sp,-0x10 with s0@4/s1@8/ra@0xC and zero outgoing-arg bytes; ours addiu $sp,-0x20 with s0@16/s1@20/ra@24. Corpus-unique (1/1437) per framescan.py.

- [s1] Residual class B: register roles. Target $s0=ctor pointer p, $s1=count, $t0=flag temp and loaded fn pointer. Ours $s0=count, $s1=p, $v0=temp. p has 3 refs (la, lw base, addiu +4), count has 4 (la, beqz, addiu -1, bnez), so count outranks p for $s0 under global.c allocno priority — target's assignment is the reverse.

- [s1] Residual class C: target sets the flag with `ori $t0,$zero,1` (0x34080001); we emit `li $v0,1` = addiu (0x24020001). regfix.txt:110 is a standing `subst "addiu" "ori" @ 6` papering over exactly this. The form is NOT unreachable in principle: 53 of 1437 asm/funcs files contain `ori $rX,$zero,imm`.

- [s1] asm/funcs/func_80083794.s holds TWO bodies: the scored 28-insn function at 0x80083794 and an unlabelled twin at 0x80083804 (same shape, inverted flag test, no flag store) — the __do_global_dtors-shaped sibling. Only the first 28 insns are scored.

- [s1] Region context: func_80083794 sits immediately after _start (0x800836EC) and the Marionation break trampolines func_800836B8 / func_800836C8 / func_80083698 — all four already owner-authorized canonical asm for ings2.c (inline_asm_canonical.txt, grant 2026-08-06). This is the crt0/libgcc continuation of that authorized cluster.

- [s1] GATE RESULT (recorded so it is not re-run): tools/scan_hand_coded.py --single func_80083794 -> tier=LOW, score 0/8, with S1/S2/S6 all negative. The canonical-asm endgame gate FAILS on the standard scanner. The scanner has no frame-geometry signal, which is the one anomaly this function actually exhibits — that mismatch is a question for a future ruling-request, not a self-granted exception.

- [s1] No sibling/duplicate analog: tmp/duplicates_leads.txt has no entry for 0x80083794 or 0x80083804.

- [s1] HOUSEKEEPING for the next session / operator: the 9 regfix rules at regfix.txt:105-113 were calibrated against the pin form that this session replaced, so the integrated build is expected to disagree until they are retired. The honest sandbox floor is the gradient that matters; regfix.txt was not touched (out of session scope).

## Session 2 (structural, 2026-08-13)

### Starting-state correction (IMPORTANT for future sessions)
`src/ings2.c` did NOT carry session 1's score-18 pure-C body at the start of this
session — it still held the inherited register-pin + hardcoded-`$17` `__asm__`
form. Only the LEDGER from s1 was committed (`b26eadc6`), not the src edit. This
session re-applied `memory/grind/func_80083794/candidate.c` to `src/ings2.c` and
re-measured the floor as **18** (target_insns 28, build_insns 28, rules_dropped 9,
cheat_asm_stripped 10), reproducing s1's number exactly. Treat `candidate.c`, not
`src/`, as the authoritative carrier of the floor.

### The exact allocation numbers (cc1 -da; this session's decisive measurement)
`tmp/grind/func_80083794/s2/greg.sh` dumps every RTL pass for the whole TU into
`tmp/grind/func_80083794/s2/dump/`. From `ings2.i.lreg` / `ings2.i.greg`:

    ;; 2 regs to allocate: 72 73
    ;; 72 conflicts: 72 73 2 29
    ;; 73 conflicts: 72 73 2 29
    ;; Register dispositions:
    72 in 16  73 in 17  74 in 2  75 in 2  76 in 2
    ;; Hard regs used:  2 16 17 31

    Register 72 used 8 times across 8 insns; dies in 0 places; crosses 1 call; GR_REGS or none.
    Register 73 used 7 times across 7 insns; dies in 0 places; crosses 1 call; GR_REGS or none; pointer.

- pseudo **72 = `count`**, 8 loop-weighted refs -> hard reg 16 = `$s0`
- pseudo **73 = `p`** (flagged `pointer`), 7 loop-weighted refs -> hard reg 17 = `$s1`
- pseudos 74/75/76 are the three block-local temps (flag load, constant 1, loaded
  function pointer); local_alloc gives all three hard reg 2 = `$v0`, where target
  uses `$t0` for all three.

`global.c:635 allocno_compare` ranks by
`floor_log2(n_refs) * n_refs / live_length * 10000 * size`, so the numerators are
`floor_log2(8)*8 = 24` for `count` against `floor_log2(7)*7 = 14` for `p`. That is
a 1.7x gap, and `live_length` cannot invert it (both pseudos span essentially the
same range, `count`'s being the longer of the two, which if anything favours `p`
only marginally). The 8-vs-7 split is loop-weighted `2 out + 2 in` for `count`
against `1 out + 2 in` for `p`, and it is fixed by the algorithm: `count` is
necessarily referenced by its `la`, the initial `beqz` guard, the `addiu -1` and
the closing `bnez`, while `p` is referenced only by its `la`, the `lw` base and
the `addiu +4`. Adding a reference to `p` means adding an instruction, and the
28-instruction target has no room for one.

### Structural sweep — 18 forms, zero movement (H2 killed)
Driver: `tmp/grind/func_80083794/s2/sweep.py` + `variants.json` (splice a body into
src, run `sandbox --disable all`, objdump the produced `.o`, record score plus
which hard register is the loop `lw` base). Raw results in
`tmp/grind/func_80083794/s2/sweep_out.json` and `sweep2_out.json`; the full table
is reproduced in
`memory/grind/func_80083794/rejected/structural-refcount-forms-do-not-flip-s0-s1.c`.

Forms covered: hoisted load into a named local; `for (;count;count--)` header;
`if (count) do {...} while (--count);`; `while (count--)`; `p[0]() / p = p + 1 /
count = count - 1`; call-then-advance; both statement orders of the two `la`s;
`count` and `p` each initialised at declaration (outside the `if`); a block-local
`q = p` split; the flag store moved after both `la`s; an early-return guard (with
both `la` orders); `u32` type narrowing of `count`; a named flag temp with
`D_800A2668 = flag + 1`; and an end-pointer walk with no counter at all.

**Every single form leaves `p` in `$s1`, the temp in `$v0`, and the frame at
-32.** The best forms tie the session-1 floor at 18; the rest are 21-23 (the
score-22 family differs only in the emission order of the two masked `la` pairs,
i.e. session-1 finding C1, and is strictly worse). Nothing in the structural
modality moves residual class B.

### H1 closed at the COMPILER-SOURCE level (probe (b), negative — decisively)
Session 1 left H1 as an uncertified negative claim with `cc1 -da` frame notes as
the next probe. The dumps turn out not to be needed: the code path is
unconditional.

- `tools/gcc-2.7.2/calls.c:1246-1252` (`expand_call`, constant-args branch):
  `args_size.constant = MAX (args_size.constant, reg_parm_stack_space);`, guarded
  only by `#ifdef REG_PARM_STACK_SPACE`. The one escape hatch immediately below is
  `#ifdef MAYBE_REG_PARM_STACK_SPACE / if (reg_parm_stack_space == 0)
  args_size.constant = 0;` — and **`MAYBE_REG_PARM_STACK_SPACE` is not defined
  anywhere in `config/mips/mips.h`** (the only matches in the tree are inside
  `calls.c` itself, lines 647 and 1248). The `#ifndef
  OUTGOING_REG_PARM_STACK_SPACE` subtraction that would cancel the area out is
  likewise disabled, because `mips.h:1830` defines `OUTGOING_REG_PARM_STACK_SPACE`.
- `mips.h:1822-1823`: `REG_PARM_STACK_SPACE(FNDECL) =
  (MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET(FNDECL)` with
  `FIRST_PARM_OFFSET` = 0 (`mips.h:1811`) => **16, unconditionally, with no
  dependence on the callee's signature or on the call's argument count.**
- `calls.c:1400-1401`: `if (needed > current_function_outgoing_args_size)
  current_function_outgoing_args_size = needed;`.
- `mips.c:4464` (`compute_frame_size`): `args_size = MIPS_STACK_ALIGN
  (current_function_outgoing_args_size);`, `total_size = var_size + args_size +
  extra_size` (4474), callee-saves laid above `args_size`.

So for **any** function whose body expands a call,
`current_function_outgoing_args_size >= 16` and the frame necessarily contains a
16-byte outgoing-argument block. The target's entire frame is 16 bytes and already
holds 12 bytes of register saves (`s0@4, s1@8, ra@0xC`), so it demonstrably does
not contain that block. **H1 CONFIRMED: no C source compiled by this frozen
pipeline can produce func_80083794's frame.** Probe (c) is subsumed by session 1's
corpus scan, which already found exactly 1/1437 calling functions with a lowest
save offset below 16 — this one.

### H3 closed with an assembler-level proof plus a corpus census (KILLED)
`ori $rX,$zero,imm` is not a C-level spelling choice at all — it is the GNU
assembler's expansion of `li` for immediates that do not fit a signed 16-bit
`addiu`. Measured directly (`tmp/grind/func_80083794/s2/astest.sh`, `astest2.sh`,
`mipsel-linux-gnu-as -march=r3000 -O1 -G0`):

    li $8,0x8000  -> 34088000  ori   t0,$zero,0x8000
    li $8,0xffff  -> 3408ffff  ori   t0,$zero,0xffff
    li $8,0x1000  -> 24081000  addiu t0,$zero,0x1000
    li $8,1       -> 24080001  addiu t0,$zero,1

The corpus census (`tmp/grind/func_80083794/s2/oriscan.py` + `oriimm.py`) matches
that rule exactly. Of the 100 `ori $rX,$zero,imm` instances across the 1437
`asm/funcs/*.s` files:

- **97 instances / 50 functions have imm >= 0x8000** — precisely the range where
  the assembler is forced into the zero-extended form. 23 of those functions are
  already matched PURE-C with zero rules, which is why the form looked "reachable"
  to session 1's file-count heuristic.
- **3 instances have imm < 0x8000**: `func_80052788` and `func_800527FC` (both
  `ori $t3,$zero,0x1000`, both `INCLUDE_ASM` bodies whose neighbouring lines are
  literally annotated `/* handwritten instruction */` — GTE `mtc2` code), and
  **`func_80083794`'s `ori $t0,$zero,0x1`**.

cc1 emits `li` for every SImode constant load (the `movsi_internal2` pattern is
visible in the `.greg` dump), the constant 1 fits `addiu`, so GNU as will always
pick `0x24080001`. A genuine C-level `|` cannot help either: GCC folds an OR with
a constant operand, and it never allocates a pseudo to `$zero`, so
`ori $rX,$zero,1` has no C preimage. **H3 KILLED — and in this executable the
small-immediate `ori` is a hand-written-assembly fingerprint (3/3 instances).**

### Where that leaves the 18-instruction residual
All three residual classes are now individually characterised, and two of the
three are proven unreachable from any C source under the frozen pipeline:

| class | residual | status after s2 |
|---|---|---|
| A frame geometry (~8 insns) | the 16-byte outgoing-arg block we must emit and target does not have | **PROVEN unreachable** (calls.c / mips.h / mips.c; corpus 1/1437) |
| B register roles (~5 insns) | `count`/`p` swapped; temp `$v0` vs `$t0` | structural axis dead (18 forms); mechanism quantified (8 vs 7 refs) |
| C `ori` vs `li` (1 insn) | `ori $t0,$zero,1` | **PROVEN unreachable** (as expands `li 1` to `addiu`; 3/3 small-imm instances are hand-written asm) |

Two independent, corpus-corroborated no-C-form signals now point the same way:
these bytes were not produced by GCC 2.7.2 from C. That is consistent with the
function's identity (SN Systems / PsyQ crt0 `__main`) and its position immediately
after `_start`, in a cluster of four functions that are ALREADY owner-authorized
canonical asm for ings2.c. It also directly contradicts `scan_hand_coded.py`'s
tier=LOW / 0-of-8 verdict — the scanner has neither a frame-geometry signal nor an
assembler-macro-expansion signal, which are exactly the two anomalies this
function exhibits.

- [s2] [s2] STARTING-STATE CORRECTION: src/ings2.c did NOT carry session 1's score-18 pure-C body at session start — it still held the inherited register-pin + hardcoded-$17 __asm__ form, because only s1's LEDGER was committed (b26eadc6), not its src edit. This session re-applied memory/grind/func_80083794/candidate.c to src/ings2.c and reproduced floor 18 exactly (target_insns 28, build_insns 28, rules_dropped 9, cheat_asm_stripped 10). Treat candidate.c, not src/, as the authoritative carrier of the floor.

- [s2] [s2] cc1 -da on the whole TU (tmp/grind/func_80083794/s2/greg.sh -> dump/ings2.i.{lreg,greg}) gives the exact allocation state: '2 regs to allocate: 72 73', '72 in 16  73 in 17  74 in 2  75 in 2  76 in 2', 'Hard regs used: 2 16 17 31'. Register 72 (count) used 8 times across 8 insns, crosses 1 call. Register 73 (p) used 7 times across 7 insns, crosses 1 call, flagged 'pointer'. 74/75/76 are the three block-local temps (flag load, constant 1, loaded fn pointer) and local_alloc gives all three $v0, where target uses $t0 for all three.

- [s2] [s2] global.c:635 allocno_compare formula is floor_log2(n_refs)*n_refs/live_length*10000*size, so count scores 24/L and p scores 14/L. The ref split is structural: count = la + initial beqz guard + addiu -1 + bnez (2 out-of-loop, 2 in-loop); p = la + lw base + addiu +4 (1 out-of-loop, 2 in-loop). Raising p's count requires adding an instruction the 28-insn target has no room for.

- [s2] [s2] 18 semantics-preserving structural forms measured with sandbox --disable all plus an objdump register check: p lands in $s1 in every one, the temp in $v0 in every one, the frame at -32 in every one. Scores: 18 for v0/v1/v2/v3/v9/w3/w5/w6/w8, 21 for v4/v6, 22 for v5/v7/v8/w1/w2/w4, 23 for w7. Full table banked in memory/grind/func_80083794/rejected/structural-refcount-forms-do-not-flip-s0-s1.c; raw JSON in tmp/grind/func_80083794/s2/sweep_out.json and sweep2_out.json.

- [s2] [s2] COMPILER-SOURCE PROOF for the frame: calls.c:1246-1252 MAXes args_size.constant against reg_parm_stack_space unconditionally for MIPS, because MAYBE_REG_PARM_STACK_SPACE is undefined in config/mips/mips.h (its only tree matches are calls.c:647 and calls.c:1248) and mips.h:1830 defines OUTGOING_REG_PARM_STACK_SPACE (killing the compensating subtraction). mips.h:1822 makes REG_PARM_STACK_SPACE a constant 16. calls.c:1400 promotes it. mips.c:4464 does args_size = MIPS_STACK_ALIGN(current_function_outgoing_args_size) and 4474 adds it into total_size below the callee-saves. No C-level input reaches it.

- [s2] [s2] ASSEMBLER-LEVEL PROOF for the ori: mipsel-linux-gnu-as -march=r3000 -O1 -G0 assembles li $8,0x8000 -> 34088000 (ori), li $8,0xffff -> 3408ffff (ori), li $8,0x1000 -> 24081000 (addiu), li $8,1 -> 24080001 (addiu). The zero-extended form is reachable only for immediates >= 0x8000.

- [s2] [s2] CORPUS CENSUS of `ori $rX,$zero,imm` across all 1437 asm/funcs/*.s: 100 instances. 97 instances / 50 functions have imm >= 0x8000 (the assembler-forced range; 23 of those functions are already matched PURE-C with zero rules, which is what made the form look C-reachable to s1). The only 3 small-immediate instances are func_80052788 and func_800527FC — both INCLUDE_ASM, both with neighbouring lines annotated /* handwritten instruction */ (GTE mtc2 code) — and func_80083794's own `ori $t0,$zero,0x1`. Small-immediate ori is a hand-written-assembly fingerprint in this executable, 3 for 3.

- [s2] [s2] The 18-instruction residual is now fully partitioned: class A frame geometry ~8 insns PROVEN unreachable; class B register roles ~5 insns structural-axis-dead with the mechanism quantified; class C the single ori PROVEN unreachable. Honest distance 0 is therefore not attainable in pure C for this function.

- [s2] [s2] The two no-C-form proofs, the two corpus-uniqueness results (1/1437 calling functions without an outgoing-arg area; 3/3 small-immediate ori instances hand-written), the function's identity as SN Systems / PsyQ crt0 __main, its placement immediately after _start, and its neighbourhood (func_800836B8 / func_800836C8 / func_80083698 / _start, all four already owner-authorized canonical asm for ings2.c under the 2026-08-06 grant) all point the same way. The countervailing fact is scan_hand_coded.py returning tier=LOW 0/8 — but that scanner has neither a frame-geometry signal nor an assembler-macro-expansion signal, i.e. it cannot see either anomaly this function exhibits. That tension is the disposition question, and it is the owner's to resolve.

## Session 3 (structural, 2026-08-13)

### Starting state (same correction as s2 — it recurred)
`src/ings2.c` AGAIN held the inherited register-pin + hardcoded-`$17` `__asm__`
body at session start (s2's src edit was not committed either; only its ledger
was, in `abc06667`). Re-applied `memory/grind/func_80083794/candidate.c` and
reproduced the floor exactly: **score 18, target_insns 28, build_insns 28,
rules_dropped 9, cheat_asm_stripped 10.** `candidate.c` remains the authoritative
carrier of the floor. NOTE: `motion_Close` (src/ings2.c:636), the twin body at
0x80083804, still carries the same pin form and is a separate queue item.

### The exact 28-instruction accounting (new — s1/s2 had class estimates only)
Full side-by-side table: `tmp/grind/func_80083794/s3/sidebyside.md`. 9 of the 28
instructions match; 19 differ, of which the scorer counts 18 (the four masked
`la` instructions at positions 11-14 are compared on destination register only —
session-1 finding C1). The accounting closes cleanly against the class
partition — with one class s1/s2 never isolated.

### Residual class D (NEW) — prologue callee-save EMISSION ORDER
- Target prologue stores ASCENDING: `sw $s0,0x4($sp); sw $s1,0x8($sp);
  sw $ra,0xC($sp)`. Target epilogue loads DESCENDING: `lw $ra,0xC; lw $s1,0x8;
  lw $s0,0x4`. The target's prologue is the reverse of its own epilogue.
- Our build stores AND loads descending (`sw ra,24; sw s1,20; sw s0,16` — the
  last one stolen into the `bnez` delay slot by reorg).
- Compiler source: `tools/gcc-2.7.2/config/mips/mips.c:4680` is a single loop
  `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` and it serves BOTH
  the `store_p` (prologue, called at mips.c:5045) and `!store_p` (epilogue,
  mips.c:5175/5359) paths. cc1 therefore emits both runs descending; only the
  post-reload scheduler can reverse a run.
- The register-to-slot ASSIGNMENT is GCC-consistent in target (highest regno at
  the highest offset: `$ra@0xC`). Only the ORDER is inverted.

### Corpus scans for class D (tmp/grind/func_80083794/s3/)
- `savescan.py` / `savescan.txt`: of 566 shipped functions with a multi-save
  prologue run, 193 are descending (the cc1-natural shape), 114 ascending, 259
  mixed/split by delay-slot stealing. So ascending is NOT rare — the axis is
  live, not structurally forbidden.
- `contig.py` / `contig.txt`: restricting to CONTIGUOUS ascending runs (target's
  shape — back-to-back stores with nothing interleaved) gives 38 runs, of which
  21 sit in functions that are matched pure C with zero rules, so the frozen
  pipeline demonstrably emits ascending runs. But every one of those 21 is a run
  of LENGTH 2. func_80083794 is the only function in the executable with a
  contiguous ascending run of length 3 (the second listing in contig.txt is its
  own unlabelled twin body at 0x80083804). Third corpus-uniqueness result for
  this function, after the frame (1/1437) and the small-immediate `ori` (3/3).

### The mechanism behind every ascending run, and why it is coupled here
Inspecting the clean-C ascending functions (`func_80069A30`, `player_Destroy`,
`func_8004046C`, `func_8001A538`, `ClearOTag`, `func_80023E40`) shows one shape
in all of them: an in-block anti-dependence. A body instruction in the same
basic block WRITES `$sN` (typically `addu $s0,$a0,$zero` sitting in a `jal`
delay slot), so the scheduler must place it after `sw $sN` and hoists that store
to unblock it.

Measured directly on this function
(`rejected/hoisted-la-flips-save-order-but-costs-5.c`): moving the two `la` pairs
ahead of the `if` puts the writes to `$s0`/`$s1` in the entry block, and the
prologue flips to the target's ascending order (`sw s0,16; ...; sw s1,20; ...;
sw ra,24`). Score 18 -> 23. That is the coupled constraint: the writes to
`$s0`/`$s1` ARE the two `la` pairs, so the anti-dependence exists only when the
`la`s are in the entry block — and target emits both `la` pairs AFTER the `bnez`,
where they exert no pressure. Target has ascending saves AND post-branch `la`s at
the same time; this compiler + scheduler gives one or the other.

### Sharper statement of residual class A
`mips.c:4464-4476`: `total_size = var_size + args_size + extra_size` (each
`MIPS_STACK_ALIGN`ed) with `gp_reg_size` on top. Target's 0x10 frame decomposes
as `var_size 0 + args_size 0 + extra_size 0 + 12 bytes of callee-saves`, rounded
to 16 — i.e. byte-exactly the frame cc1 emits for a LEAF function with three
callee-saves, for a body that contains a `jalr`. That is a tighter phrasing of
s2's H1 than "the arg area is missing": the whole frame is leaf-shaped.

- [s3] STARTING STATE recurred: src/ings2.c again held the pin form at session start (s2's src edit uncommitted, only its ledger landed in abc06667). Re-applied candidate.c; floor reproduced exactly at 18 (target_insns 28, build_insns 28, rules_dropped 9, cheat_asm_stripped 10). candidate.c is the authoritative carrier. motion_Close (src/ings2.c:636) is the twin body at 0x80083804 and still carries the same pin form as a separate queue item.

- [s3] EXACT 28-INSTRUCTION ACCOUNTING (tmp/grind/func_80083794/s3/sidebyside.md): 9 of 28 instructions match, 19 differ, scorer counts 18 (positions 11-14, the two masked la pairs, are compared on destination register only). The accounting revealed a residual class s1/s2 never isolated.

- [s3] NEW RESIDUAL CLASS D — prologue callee-save EMISSION ORDER. Target prologue stores ASCENDING (sw $s0,0x4; sw $s1,0x8; sw $ra,0xC) while its own epilogue loads DESCENDING (lw $ra,0xC; lw $s1,0x8; lw $s0,0x4). Ours stores and loads descending. mips.c:4680 is a SINGLE loop `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` serving both the store_p (prologue, mips.c:5045) and !store_p (epilogue, mips.c:5175/5359) call sites, so cc1 emits both runs descending; only the post-reload scheduler can reverse one. The register-to-slot assignment in target is GCC-consistent ($ra at the highest offset) — only the order is inverted.

- [s3] CORPUS SCAN (savescan.py/.txt): 566 shipped functions have a multi-save prologue run — 193 descending, 114 ascending, 259 mixed. Ascending is common, so class D is a live scheduler axis, not a structural impossibility.

- [s3] CORPUS SCAN (contig.py/.txt): restricting to CONTIGUOUS ascending runs (target's shape) gives 38, of which 21 are in functions matched pure C with zero rules — the frozen pipeline does emit ascending runs. But all 21 are runs of LENGTH 2. func_80083794 is the ONLY function in the executable with a contiguous ascending run of LENGTH 3 (the duplicate listing is its own twin body at 0x80083804). Third corpus-uniqueness result for this function.

- [s3] MECHANISM for every ascending run in the corpus: an in-block ANTI-DEPENDENCE — a body insn in the same basic block writes $sN (classically `addu $s0,$a0,$zero` in a jal delay slot), forcing the scheduler to hoist `sw $sN` to unblock it. Verified across func_80069A30, player_Destroy, func_8004046C, func_8001A538, ClearOTag, func_80023E40.

- [s3] MEASURED on this function: hoisting the two la pairs ahead of the `if` puts the $s0/$s1 writes in the entry block and DOES flip the prologue to the target's ascending order (sw s0,16 / sw s1,20 / sw ra,24). Score 18 -> 23. Banked as rejected/hoisted-la-flips-save-order-but-costs-5.c. The constraint is COUPLED: the $s0/$s1 writes ARE the two la pairs, so ascending saves require the la's in the entry block, while target emits both la pairs AFTER the bnez. Target has both properties at once; this compiler gives one or the other.

- [s3] SHARPER CLASS A: mips.c:4464-4476 total_size = var_size + args_size + extra_size (each MIPS_STACK_ALIGNed) plus gp_reg_size. Target's 0x10 frame decomposes as var_size 0 + args_size 0 + extra 0 + 12 bytes of saves rounded to 16 — byte-exactly the frame cc1 emits for a LEAF function with three callee-saves, for a body containing a jalr.

- [s3] STARTING-STATE CORRECTION RECURRED: src/ings2.c again held the inherited register-pin + hardcoded-$17 __asm__ body at session start, because session 2's src edit was not committed either (only its ledger landed, in abc06667). Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly: score 18, target_insns 28, build_insns 28, rules_dropped 9, cheat_asm_stripped 10. candidate.c remains the authoritative carrier of the floor; src/ is left carrying it again at session end.

- [s3] EXACT 28-INSTRUCTION ACCOUNTING (new; s1/s2 had only class-size estimates): 9 match, 19 differ, scorer counts 18. Full table in tmp/grind/func_80083794/s3/sidebyside.md. This is what surfaced a residual class the previous two sessions never isolated.

- [s3] NEW RESIDUAL CLASS D - prologue callee-save EMISSION ORDER. Target's prologue stores ASCENDING (sw $s0,0x4; sw $s1,0x8; sw $ra,0xC) while target's own epilogue loads DESCENDING (lw $ra,0xC; lw $s1,0x8; lw $s0,0x4). Our build stores and loads descending (the last store, sw s0,16(sp), stolen into the bnez delay slot by reorg). mips.c:4680 is a single loop serving both directions, so cc1 emits both runs descending; the register-to-slot ASSIGNMENT in target is GCC-consistent ($ra at the highest offset) and only the ORDER is inverted.

- [s3] CORPUS SCAN savescan.py over all 1437 asm/funcs/*.s: 566 functions have a multi-save prologue run - 193 descending, 114 ascending, 259 mixed. Ascending prologues are common, so class D is a live scheduler axis, not a structural impossibility.

- [s3] CORPUS SCAN contig.py (contiguous ascending runs only - target's exact shape): 38 runs, 21 of them in functions matched pure C with ZERO rules, proving the frozen pipeline emits ascending runs. Every one of those 21 is LENGTH 2. func_80083794 is the ONLY function in the executable with a contiguous ascending run of LENGTH 3. That is the THIRD corpus-uniqueness result for this function, after the frame (1/1437 calling functions lack the outgoing-arg block) and the small-immediate ori (3/3 instances hand-written).

- [s3] MECHANISM behind every ascending run in the corpus, verified across func_80069A30, player_Destroy, func_8004046C, func_8001A538, ClearOTag and func_80023E40: an in-block anti-dependence - a body instruction in the same basic block writes $sN (classically 'addu $s0,$a0,$zero' in a jal delay slot), forcing the scheduler to hoist 'sw $sN' to unblock it.

- [s3] MEASURED on func_80083794: hoisting the two 'la' pairs ahead of the 'if' puts the $s0/$s1 writes in the entry block and DOES flip the prologue to the target's ascending order - and costs 5 points (18 -> 23). The constraint is coupled, because the $s0/$s1 writes ARE the two 'la' pairs and target emits both after the bnez. Banked as rejected/hoisted-la-flips-save-order-but-costs-5.c.

- [s3] SHARPER CLASS A: target's 0x10 frame = var_size 0 + args_size 0 + extra_size 0 + 12 bytes of callee-saves rounded to 16 (mips.c:4464-4476) - byte-exactly cc1's LEAF-function frame, for a body containing a jalr.

- [s3] HOUSEKEEPING: motion_Close (src/ings2.c:636) is the unlabelled twin body at 0x80083804 and still carries the same register-pin + hardcoded-$17 __asm__ form; it is a separate queue item and was not touched. The 9 regfix rules at regfix.txt:105-113 remain calibrated against the old pin form, so the integrated build is expected to disagree until an operator retires them; regfix.txt was not touched (out of session scope).

## Session 4 (permuter) — facts

- The honest floor is **18** (target_insns 28, build_insns 28) with the
  `candidate.c` body applied to `src/ings2.c`. Re-measured at the start and the
  end of session 4; `src/ings2.c` was found carrying the s1 register-pin +
  hardcoded-`$17` `__asm__` body again at session start (only the ledgers are
  committed, never the src edit) and was re-seeded from `candidate.c`.
- **A permuter workspace for this function must truncate the target.**
  `asm/funcs/func_80083794.s` is 60 lines and contains TWO 28-instruction
  bodies: `func_80083794` (lines 1-31) and its unlabelled twin at 0x80083804
  (lines 32-59, the `motion_Close` body). Assembling the whole file as
  `target.o` gives a 56-instruction target against a 28-instruction base and
  makes the permuter score meaningless. `tmp/grind/func_80083794/s4/mkws.sh`
  builds the workspace correctly (prelude.inc with `.set gp=64` stripped for
  r3000, `sed -n '1,31p'`, `endlabel`).
- **`--stack-diffs` must stay ON for this function** (it is the campaign
  wrapper's default since 2026-07-13). The permuter's default scorer normalizes
  sp-relative offsets away, and this function's dominant residual IS a
  frame-size/stack-offset shift — the default scorer would false-match at 0.
- **The minimal-TU chassis is faithful.** A standalone TU containing only
  `typedef int s32;`, the three externs and the function body compiles, through
  the full `cpp | cc1 -mel | prologue_fix | maspsx | multu_pad | as` pipeline,
  to the same 28 instructions as the full-TU sandbox object. `src/ings2.c`'s
  file-scope `__asm__`/`INCLUDE_ASM` blocks would otherwise have to go through
  pycparser.
  One assembler detail: the permuter prelude's `.set noat` must be cancelled
  with `.set at` before the compiled body, because `la $16,D_00000000` needs
  `$at`; the real build assembles with `at` enabled, so this is faithful, not a
  divergence.
- **Campaign telemetry** (both harvested with `--stop` in-session; neither
  outlived the session; `permuter_campaign.py status` shows both `alive: false`):
  | campaign | chassis | base_score | iterations | wall | best |
  |---|---|---|---|---|---|
  | `s4/ws`  | score-18 floor form            | 383  | 48,633 | ~22 min | 340 |
  | `s4/ws2` | s3 hoisted-`la` (class-D) form | 1168 | 58,431 | ~22 min | 383 |
  Find timings in campaign A: 383 @ 19 s, 378 @ 28 s, 340 @ 522 s, 378 @ 752 s,
  340 @ 992 s — i.e. the basin yielded everything it had inside the first ~9
  minutes and then repeated itself, exactly the shape the fresh-seed rule
  describes.
- **Honest re-measurements of the permuter's proposals** (the permuter score is
  NOT the honest metric; every proposal was re-scored):
  - permuter 340 (`volatile unsigned char new_var` guard): **29 emitted
    instructions vs target's 28**, frame -40 (worse than the floor's -32),
    `$s0`/`$s1` still inverted, no `ori`. Strictly worse honestly; also a cheat.
  - permuter 378 (`do { while (...) {...} } while (0);`): spliced into
    `src/ings2.c` and sandboxed — **score 18, build_insns 28**, i.e. exactly the
    floor. The permuter delta is label-numbering noise.
  - permuter 383 forms: tie the base.
- **Minimum honest score observed across 107,064 iterations: 18.** No form from
  either chassis beat the floor.

- [s4] src/ings2.c was found at session start carrying the s1 register-pin + hardcoded-$17 __asm__ body again (only the ledgers get committed, never the src edit); it was re-seeded from memory/grind/func_80083794/candidate.c and re-measured at score 18 / target_insns 28 / build_insns 28 at both the start and the end of the session.

- [s4] A permuter workspace for this function MUST truncate the target: asm/funcs/func_80083794.s is 60 lines and contains TWO 28-instruction bodies — func_80083794 (lines 1-31) and its unlabelled twin at 0x80083804 (lines 32-59). Assembling the whole file gives a 56-instruction target against a 28-instruction base and the score is meaningless. tmp/grind/func_80083794/s4/mkws.sh builds it correctly and is reusable.

- [s4] --stack-diffs must stay ON for this function (the campaign wrapper's default since 2026-07-13): the permuter's default scorer normalizes sp-relative offsets away and this function's dominant residual IS a frame-size/stack-offset shift, so the default scorer would false-match at 0.

- [s4] The minimal-TU chassis is faithful: a standalone TU with only 'typedef int s32;', the three externs and the body compiles through the full real pipeline to the same 28 instructions as the full-TU sandbox object, which keeps pycparser away from ings2.c's file-scope __asm__/INCLUDE_ASM blocks. The permuter prelude's '.set noat' must be cancelled with '.set at' before the compiled body ('la $16,D_00000000' needs $at); the real build assembles with at enabled, so this is faithful.

- [s4] Campaign A find timings: 383 @ 19 s, 378 @ 28 s, 340 @ 522 s, 378 @ 752 s, 340 @ 992 s. The basin yielded everything it had inside the first ~9 minutes and then repeated itself — exactly the shape the fresh-seed rule predicts, and the reason both campaigns were stopped at ~22 min rather than left to simmer.

- [s4] Honest re-measurement is not optional here: the permuter's best form (score 340) is WORSE on the honest metric (29 insns vs 28) than the form it 'beat'. The permuter's diff rewarded making the three callee-save stores contiguous, which cost an extra instruction and 8 more bytes of frame.

- [s4] The do-while(0) wrap that the permuter surfaced at 378 measures exactly 18 in the honest sandbox — so the sanctioned carve-out's prerequisites are moot for this function: there is nothing for the wrapper to buy.

- [s4] Both campaigns were harvested with --stop before the session ended; permuter_campaign.py status reports alive:false for both PIDs (5069, 498721). No campaign outlived the session.

- [s4] Cumulative: every sanctioned C axis is now measured dead for this function — declaration order (s1), the pin/asm form (s1), both la statement orders (s1/s2), 18 structural forms (s2), the ori spelling (s2), frame minimality by three independent routes (s1/s2), the prologue-save-order axis (s3), and now randomized search from both the floor chassis and the class-D chassis (s4).

## Session 5 (permuter, 2026-08-13)

### Starting state (the recurring correction, fourth time)
`src/ings2.c` again held the s1 register-pin + hardcoded-`$17` `__asm__` body at
session start (only ledgers are committed, never the src edit). Re-applied
`memory/grind/func_80083794/candidate.c` and reproduced the floor exactly:
**score 18, target_insns 28, build_insns 28**, re-verified again at session end
with `src/ings2.c` left carrying the floor form.

### The NEW instrument — a normalized-target permuter workspace
Sessions 2 and 3 proved two of the four residual classes have no C preimage:
class A (frame geometry) and class C (`ori $t0,$zero,1`). Those classes
contribute a CONSTANT penalty to every permuter score, so the randomizer's
gradient in s4 was dominated by noise no mutation could ever move — which is
exactly why its best finds were frame-junk forms. Session 5 therefore built a
**normalized target**: `asm/funcs/func_80083794.s` lines 1-31 with ONLY the
proven-unreachable classes rewritten to the form our pipeline can emit
(`addiu $sp,-0x10` -> `-0x20`; saves `0x4/0x8/0xC` -> `0x10/0x14/0x18`;
`ori $t0,$zero,0x1` -> `li $t0,1`), leaving target's register roles ($s0 = `p`,
$s1 = `count`, $t0 = temp) and target's ASCENDING prologue save order untouched.
Builder: `tmp/grind/func_80083794/s5/mkws3.py` (asserts exactly 9
normalizations) and `mkws4.py`.
**`ws3/target.o` and `ws4/target.o` are MEASUREMENT INSTRUMENTS, not match
targets — a score of 0 against them is NOT a byte match.** Against the
normalized target the floor form matches 15 of 28 positions (vs 9 against the
real target) and the entire prologue/epilogue frame block matches, so the
remaining objective is exactly classes B (register roles) + D (save order).

### The proven lower bound on ANY pure-C form (new, and it closes the modality)
Position accounting against the real target (s3's `sidebyside.md`, re-derived
from the normalized-target diff): the target instructions that reference the
frame are positions 3 (`addiu $sp,-0x10`), 4/5/6 (`sw $s0,0x4` / `$s1,0x8` /
`$ra,0xC`), 23/24/25 (the matching `lw`s) and 26 (`addiu $sp,0x10`) — eight
instructions whose immediates our pipeline can never emit, because class A is
proven unreachable. Position 8 (`ori $t0,$zero,0x1`) is a ninth, by the class-C
assembler proof. So **no pure-C form under the frozen pipeline can score better
than 9 honest instructions of residual (>=8 even discounting the one position
the scorer masks), against a current floor of 18.** A permuter campaign on this
function cannot reach distance 0 no matter how many iterations it runs — the
modality is closed by arithmetic, not by exhaustion. The most any future search
could buy is classes B+D, roughly half the residual.

### Campaign telemetry (both harvested with `--stop` in-session; `status` shows `alive:false` for both PIDs)
| campaign | chassis | base | iterations | wall | best |
|---|---|---|---|---|---|
| `s5/ws3` | score-18 floor form vs normalized target | 308 | 95,501 | ~30 min | 263 |
| `s5/ws4` | s3 hoisted-`la` (class-D) form vs normalized target | 970 | 63,304 | ~15 min | 478 |

- ws3's best (263, found at 190 s and never beaten in the following ~27 min) is
  the s4-340 cheat family re-found under two new spellings:
  `volatile unsigned short new_var; if (D_800A2668 == (new_var = 0))` and
  `s32 new_var2; s32 *new_var3 = &new_var2; if (*new_var3 == 0)` — a dead
  volatile frame-slot local, and an address-taken dead local. Both measured
  honestly: **29 emitted instructions, frame -40, `$s0`/`$s1` still inverted,
  save order still descending** — they touch neither class B nor class D. Even
  with the frame noise removed from the objective the permuter's alignment-based
  diff still rewards adding a frame slot, which is why this family keeps coming
  back; it is a property of the permuter's metric, not a lead.
- ws4 (class-D chassis) never re-entered the sub-308 region: base 970, best 478.

### The one genuinely new lead, and why it is dead (the session's main result)
`ws4/output-478-1` is **the first form in five sessions whose loop reads through
`$s0`** (`lw v0,0(s0)`, `beqz s1`, `bnez s1`) — i.e. target's register roles,
residual class B flipped. It flips them because it TESTS `new_var` while
decrementing `count`, so the decrement is dead, flow deletes it, and the counter
pseudo loses its two in-loop references, inverting the allocno priority. It is
an infinite loop — the permuter does not preserve semantics.
Six semantics-preserving spellings of that same ref-split intent were then
measured in the honest sandbox (`tmp/grind/func_80083794/s5/sweep.py` +
`variants.json`, raw in `sweep_out.json`): scores 18/18/22/22/23/24, `p` in
`$s1` in every one, temp `$v0` in every one, frame -32 in every one. GCC's
copy-propagation folds `n = count;` back into one pseudo, so a
semantics-preserving split is not a split. Banked:
`rejected/count-ref-split-forms-do-not-flip-s0-s1.c`.

- [s5] STARTING STATE (fourth recurrence): src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again at session start; re-applied candidate.c and reproduced score 18 / target_insns 28 / build_insns 28, re-verified at session end. src/ings2.c is left carrying the floor form.

- [s5] NEW INSTRUMENT — the normalized-target permuter workspace (tmp/grind/func_80083794/s5/mkws3.py, mkws4.py). Classes A (frame geometry) and C (ori) are proven to have no C preimage, so they add a constant penalty that drowns the real search signal; the s5 workspaces assemble a target with EXACTLY those two classes rewritten to our reachable form (addiu $sp,-0x10 -> -0x20; saves 0x4/0x8/0xC -> 0x10/0x14/0x18; ori $t0,$zero,0x1 -> li $t0,1) and nothing else. Against it the floor form matches 15/28 positions (vs 9/28 against the real target) and the whole frame block matches, leaving classes B (register roles) + D (prologue save order) as the sole objective. WARNING for future sessions: ws3/ws4 target.o is a MEASUREMENT INSTRUMENT — a score of 0 against it is NOT a byte match.

- [s5] PROVEN LOWER BOUND: eight target instructions reference the frame (positions 3, 4, 5, 6, 23, 24, 25, 26 — the addiu $sp pair and the three sw / three lw), and class A makes every one of their immediates unreachable; position 8's `ori $t0,$zero,0x1` is a ninth by the class-C assembler proof. Therefore NO pure-C form under the frozen pipeline can score below 9 honest residual instructions (>=8 discounting the single position the scorer masks), against the current floor of 18. A permuter campaign here cannot reach distance 0 by construction — the modality is closed by arithmetic, not by exhaustion, and the most any future search can buy is classes B+D.

- [s5] CAMPAIGN ws3 (normalized-target-BD-isolation): base 308, 95,501 iterations, ~30 min, best 263 found at 190 s and never beaten afterwards. The 263 family is the s4-340 cheat re-found under two new spellings — `volatile unsigned short new_var; if (D_800A2668 == (new_var = 0))` and `s32 *new_var3 = &new_var2; if (*new_var3 == 0)`. Honest re-measurement of both: 29 emitted instructions, frame -40, $s0/$s1 still inverted, save order still descending. Removing the frame noise from the OBJECTIVE does not stop the permuter's alignment-based diff from rewarding an added frame slot; that is a property of the permuter metric, not a lead.

- [s5] CAMPAIGN ws4 (class-D-chassis-vs-normalized-target): base 970, 63,304 iterations, ~15 min, best 478 — it never re-entered the sub-308 region, corroborating s3/s4's coupled-constraint result from a second angle.

- [s5] FIRST-EVER CLASS-B FLIP, and why it is dead: ws4/output-478-1 emits `lw v0,0(s0)` / `beqz s1` / `bnez s1`, i.e. target's register roles. It achieves that only by TESTING `new_var` while decrementing `count`, which makes the decrement dead code that flow deletes, stripping two in-loop references off the counter and inverting the allocno priority. The form is an infinite loop (the permuter does not preserve semantics) and its emitted body is missing target's `addiu $s1,$s1,-0x1` entirely.

- [s5] The ref-split family measured honestly (s5/sweep.py + variants.json + sweep_out.json), six semantics-preserving spellings: x1_guard_count_loop_copy 23, x2_plain_copy_rename 18, x3_guard_count_endptr_loop 22 (29 insns), x4_copy_before_p 22, x5_guard_copy_while 24 (30 insns), x6_countdown_predec_guarded 18. `p` is in $s1 in ALL six, temp $v0 in ALL six, frame -32 in ALL six. GCC's copy-propagation folds `n = count;` back into a single pseudo, so a semantics-preserving split is not a split; the only way to remove the counter's two in-loop references is to stop the loop decrementing the variable it tests, which is the semantic break. Banked as rejected/count-ref-split-forms-do-not-flip-s0-s1.c.

- [s5] SHARPER STATEMENT OF CLASS B: target's own body references `count` ($s1) four times (la, beqz guard, addiu -1, bnez) and `p` ($s0) three times (la, lw base, addiu +4) — the SAME 4-vs-3 split our build has — yet assigns them the opposite hard registers. Under global.c:635 allocno_compare that assignment is anti-priority, i.e. target's register choice is not what this cc1's allocator produces from ANY C source with a correct loop, independently of spelling. That is a fourth no-C-form signal, alongside the frame (class A), the small-immediate ori (class C) and the length-3 ascending save run (class D).

- [s5] STARTING STATE (fourth recurrence): src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again at session start, because only the ledgers are ever committed and never the src edit. Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly — score 18, target_insns 28, build_insns 28 — and re-verified it at session end. src/ings2.c is left carrying the floor form.

- [s5] NEW INSTRUMENT — the normalized-target permuter workspace (tmp/grind/func_80083794/s5/mkws3.py builds ws3, mkws4.py builds ws4; mkws3.py asserts exactly 9 normalizations so the instrument cannot silently drift). It assembles asm/funcs/func_80083794.s lines 1-31 with ONLY the proven-unreachable classes A and C rewritten to the form our pipeline can emit, leaving target's register roles ($s0 = p, $s1 = count, $t0 = temp) and target's ASCENDING prologue save order in the objective. WARNING for future sessions: ws3/ws4 target.o is a MEASUREMENT INSTRUMENT — a score of 0 against it is NOT a byte match and must be re-measured honestly with sandbox --disable all against the real target.

- [s5] PROVEN LOWER BOUND: positions 3, 4, 5, 6, 23, 24, 25 and 26 of the target all reference the frame and class A makes their immediates unreachable; position 8's ori is a ninth by the class-C assembler proof. No pure-C form under the frozen pipeline can score below 9 (>= 8 discounting the masked position), against a floor of 18. A permuter campaign on this function cannot reach distance 0 by construction.

- [s5] CAMPAIGN ws3 (normalized-target-BD-isolation): base_score 308 (vs 383 against the real target), 95,501 iterations, ~30 min wall, best 263 found at 190 s and never beaten afterwards — the basin-yields-early shape the fresh-seed rule predicts.

- [s5] CAMPAIGN ws4 (class-D-chassis-vs-normalized-target): base_score 970 (vs 1168 against the real target), 63,304 iterations, ~15 min wall, best 478; it never re-entered the sub-308 region, a second corroboration of s3/s4's coupled-constraint result from inside the ascending-save basin.

- [s5] Both s5 campaigns were harvested with --stop in-session; tools/permuter_campaign.py status reports alive:false for PIDs 1098766 and 2018275. No campaign outlived the session.

- [s5] FIRST-EVER CLASS-B FLIP: tmp/grind/func_80083794/s5/ws4/output-478-1 emits lw v0,0(s0) / beqz s1 / bnez s1 — target's register roles — but only because it tests new_var while decrementing count, making the decrement dead code that flow deletes. It is an infinite loop and its body is missing target's addiu $s1,$s1,-0x1 entirely. The permuter does not preserve semantics; this is a proposal, and it is a semantic break, not a lead.

- [s5] The six semantics-preserving ref-split forms measured honestly: x1_guard_count_loop_copy 23, x2_plain_copy_rename 18, x3_guard_count_endptr_loop 22 (29 insns), x4_copy_before_p 22, x5_guard_copy_while 24 (30 insns), x6_countdown_predec_guarded 18. p is in $s1 in ALL six, temp $v0 in ALL six, frame -32 in ALL six. GCC's copy-propagation folds `n = count;` back into one pseudo, so a semantics-preserving split is not a split.

- [s5] SHARPER CLASS B (fourth no-C-form signal): target's own body references count ($s1) four times (la, beqz guard, addiu -1, bnez) and p ($s0) three times (la, lw base, addiu +4) — the SAME 4-vs-3 split our build has — yet assigns them the opposite hard registers. Under global.c:635 allocno_compare that assignment is anti-priority, i.e. it is not what this cc1's allocator produces from ANY C source with a correct loop, independently of spelling. This joins the leaf-shaped frame (1/1437), the small-immediate ori (3/3 hand-written) and the length-3 contiguous ascending save run (1/1437).

- [s5] The 263 cheat family re-found this session adds one NEW spelling to the s4 bank: an address-taken dead local (`s32 new_var2; s32 *new_var3 = &new_var2; if (*new_var3 == 0)`) alongside the volatile dead local. Both are frame-coercion cheats by the expanded catalog and both are measurably worse honestly (29 insns vs target's 28).

## Session 6 (forensics, 2026-08-13)

### Starting state (fifth recurrence of the same correction)
`src/ings2.c` again held the s1 register-pin + hardcoded-`$17` `__asm__` body at
session start (only ledgers are ever committed, never the src edit). Re-applied
`memory/grind/func_80083794/candidate.c` and reproduced the floor exactly:
**score 18, target_insns 28, build_insns 28, rules_dropped 9,
cheat_asm_stripped 10.** `src/ings2.c` is left carrying the floor form.
`motion_Close` (the twin body) was NOT touched — it is a separate queue item.

### RESIDUAL CLASS E (NEW) — delay-slot PROVENANCE, proven unreachable
Target position 7-8 is `bnez $t0,.L800837EC` with `ori $t0,$zero,0x1` in the
delay slot. That filler **writes `$t0`, the very register the branch tests**, so
it cannot be an entry-block instruction (it would destroy the comparison); it is
the conditional arm's `initialized = 1` value materialisation, moved *backwards*
into the slot from the fall-through path. In `tools/gcc-2.7.2/reorg.c`,
`fill_simple_delay_slots` can only do that from its **fall-through scan at
reorg.c:3075**, and that scan is guarded at **reorg.c:3048** by
`if (slots_filled != slots_to_fill && ...)` — i.e. it runs ONLY when the
**backward scan at reorg.c:2960** ("Now, scan backwards from the insn to search
for a potential delay-slot candidate") found no candidate. In target, three
eligible callee-save stores sit immediately in front of the branch.

Measured directly with the **instrumented cc1** (`tools/gcc-2.7.2/cc1`,
`BB2_DBR_DEBUG=1`; log `tmp/grind/func_80083794/s6/dbr_mini.log`):

    DBRDBG simp insn=11 trial=73 refset=0 setset=0 setneed=0
    DBRDBG simp insn=11 trial=73 elig=1

insn 11 is the flag-test branch, trial 73 is `sw $16,16($sp)` — the nearest
preceding prologue save. It is eligible (no resource conflict) and it IS taken;
the emitted asm confirms it (`bne $2,$0,.L2` / delay slot `sw $16,16($sp)`).
So for ANY C form whose entry block ends with the callee-save run followed by
the flag test, our pipeline necessarily consumes one save into the branch delay
slot and can never leave the run intact — while the target keeps all three saves
AND fills the slot from the arm. This is independent of residual class D: even
if the save order were flipped to ascending, the third save would still be eaten.

**Corpus check (`tmp/grind/func_80083794/s6/slotscan.py`):** of the 1434
`asm/funcs/*.s` files, 1346 have no >=3-save run early, 13 have the branch slot
filled by another save, 36 have the run not followed by a branch, and **39 do
keep an intact >=3 save run with a non-save filler** — but in every one of those
39 the filler is an ENTRY-BLOCK computation that the backward scan legitimately
preferred because it sat nearest the branch (`addiu $s3,$a0,-0x1` from the
incoming `$a0` in 35 of them, `lh $s7,0x60($s2)` in 2). Spot-checked
`func_8004C994` (zero regfix/asmfix rules, not in the queue, i.e. matched
pure C): 8 saves intact, `beqz $a0`, slot `addiu $s3,$a0,-0x1` — a value computed
from a parameter, not an arm insn clobbering the branch's own test register.
**No shipped function other than func_80083794 (and its twin) fills a branch
delay slot with an insn that clobbers the tested register while eligible saves
sit adjacent.** Fifth corpus-uniqueness / no-C-form result for this function.

### Class D confirmed at the PASS level with RTL dumps (s3 had source-reading only)
`tmp/grind/func_80083794/s6/dump/` (minimal-TU floor form, `cc1 -O2 -G0 -mel -da`).
The three callee-save stores are **absent from `.greg`** and first appear in
`.jump2` as RTL insns 69/71/73 — i.e. they are inserted by prologue threading
after register allocation, as `sw $ra,24 / sw $s1,20 / sw $s0,16`, DESCENDING,
exactly as `mips.c:4680`'s single `for (regno = GP_REG_LAST; regno >=
GP_REG_FIRST; regno--)` loop dictates. **`.sched2` leaves insn order 67-69-71-73
untouched** (the post-reload scheduler does not reorder the run absent an
anti-dependence), and `.dbr` then pulls insn 73 into the branch delay slot. So
the pass that fixes the order is `save_restore_insns` (mips.c), the only pass
that could reverse it is `schedule_insns`/sched2, and this function gives it no
reason to. That closes s3's mechanism claim with dump evidence rather than
source reading.

### H7 KILLED — the unfilled RETURN delay slot is NOT a hand-assembly signal
Target ends `addiu $sp,$sp,0x10; jr $ra; nop`, i.e. the stack restore is NOT
sunk into the return delay slot. Census of all 1434 shipped functions
(`tmp/grind/func_80083794/s6/epilogue_scan.py`): **838 have exactly that shape**,
288 are leaves with a nop slot, 158 have some other filler, and only **94** have
the restore in the slot. The shape is the pipeline's norm here, not an anomaly.
Recorded so no future session mistakes it for evidence.

### H8 KILLED — external corroboration is NOT available from the second executable
`disc/STR/MOVOVL.EXE` is a second, independently linked PS-EXE produced by the
same PsyQ toolchain for this same game, so it was the one offline route to
corroborate the crt0/libgcc identity from OUTSIDE the main executable (frontier
F1's named next probe). Scanned it (`tmp/grind/func_80083794/s6/movovl_scan.py`,
opcode-exact word signatures): **0 hits for the leaf-frame `addiu $sp,-0x10 /
sw $s0,4 / sw $s1,8 / sw $ra,0xC` shape, 0 hits for the ctor-walk loop
(`lw $t0,0($s0); addiu $s0,$s0,4; jalr $t0; addiu $s1,$s1,-1`)**, and exactly one
contiguous ascending >=3 `sw`-to-`$sp` run — which disassembly shows is
`sw zero,64/72/80($sp)` inside the overlay's vsprintf, not a prologue. The
overlay's entry (0x801DA084) is just `lui gp / addiu gp / j 0x801D91CC`: it has
no ctor machinery at all. The same scan over the main EXE finds the leaf frame
and the ctor loop exactly twice — func_80083794 and its twin. Two web searches
for a primary SN Systems / PsyQ `crt0` `__main` listing returned nothing
primary (only the secondary observation that crt0 objects are the PsyQ objects
that fail PSYQ->ELF conversion). **This probe is closed; do not re-run it.**

### The twin body carries every anomaly too
The unlabelled twin at 0x80083804 (`motion_Close`, a separate queue item) has the
same 0x10 leaf frame, the same ascending `s0@4/s1@8/ra@0xC` save run, the same
anti-priority roles ($s0 = the pointer, $s1 = the counter, $t0 = temp) — and its
`beqz $t0` delay slot is an unfilled `nop` with three eligible saves sitting
right in front of it, which reorg.c's backward scan would have consumed. Two
independent function bodies in the same crt0 cluster exhibit the identical five
anomalies.

- [s6] STARTING STATE (fifth recurrence): src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again; re-applied candidate.c and reproduced score 18 / target_insns 28 / build_insns 28. src/ings2.c left carrying the floor form; motion_Close untouched.

- [s6] NEW RESIDUAL CLASS E — delay-slot PROVENANCE. Target's `bnez $t0` delay slot holds `ori $t0,$zero,0x1`, an insn that CLOBBERS the register the branch tests, so it can only have come from the conditional arm via reorg.c's fall-through scan (reorg.c:3075). That scan is guarded at reorg.c:3048 by `slots_filled != slots_to_fill`, i.e. it runs only when the BACKWARD scan (reorg.c:2960) finds nothing — and in target three eligible callee-saves sit immediately before the branch. Instrumented cc1 (BB2_DBR_DEBUG=1) logs `DBRDBG simp insn=11 trial=73 refset=0 setset=0 setneed=0` / `elig=1`: the nearest preceding save (`sw $16,16($sp)`) is eligible and IS consumed into the slot. Our pipeline therefore can never leave the save run intact ahead of the flag test, independently of class D's ordering.

- [s6] CORPUS CENSUS for class E (s6/slotscan.py over 1434 files): 1346 have no >=3-save run early, 13 fill the branch slot with another save, 36 have no branch after the run, and 39 keep an intact run with a non-save filler — but all 39 fillers are ENTRY-BLOCK computations the backward scan preferred because they sat nearest the branch (35x `addiu $s3,$a0,-0x1` from the incoming $a0, 2x `lh $s7,0x60($s2)`). Spot-check func_8004C994: zero rules, not queued (matched pure C), 8 saves intact, slot = `addiu $s3,$a0,-0x1`. No shipped function except func_80083794 and its twin fills a branch slot with an insn clobbering the branch's own test register while eligible saves sit adjacent.

- [s6] CLASS D CONFIRMED AT PASS LEVEL (s6/dump/, cc1 -da on the minimal-TU floor form): the three saves are ABSENT from .greg and first appear in .jump2 as RTL insns 69/71/73 in descending order (ra@24, s1@20, s0@16) — inserted by prologue threading after allocation, per mips.c:4680's single GP_REG_LAST->GP_REG_FIRST loop. .sched2 leaves the order untouched; .dbr then steals insn 73 into the branch delay slot. Ordering pass = save_restore_insns (mips.c); only schedule_insns/sched2 could reverse it, and this function supplies it no anti-dependence.

- [s6] H7 KILLED: the unfilled RETURN delay slot (`addiu $sp,0x10; jr $ra; nop`) is NOT a hand-assembly fingerprint. Census of all 1434 shipped functions (s6/epilogue_scan.py): 838 have exactly that shape, 288 leaf+nop, 158 other filler, only 94 sink the restore into the slot. Do not cite it as evidence.

- [s6] H8 KILLED: external corroboration from the second executable is unavailable. disc/STR/MOVOVL.EXE (independently linked, same toolchain) has ZERO hits for the leaf-frame save signature and ZERO for the ctor-walk loop; its only contiguous ascending >=3 sw-to-sp run is `sw zero,64/72/80($sp)` inside vsprintf, and its entry 0x801DA084 is `lui gp / addiu gp / j 0x801D91CC` — no ctor machinery at all. The same scan on the main EXE hits exactly twice: func_80083794 and its twin. Two web searches found no primary SN Systems / PsyQ crt0 `__main` listing. Frontier F1's named external-corroboration probe is CLOSED as measured-unavailable, not merely untried.

- [s6] The twin body at 0x80083804 (motion_Close, separate queue item) carries every one of the anomalies: 0x10 leaf frame with a jalr in the body, ascending s0@4/s1@8/ra@0xC saves, anti-priority roles ($s0 = pointer, $s1 = counter), $t0 temp — and an unfilled `nop` in its `beqz $t0` slot with three eligible saves in front of it. Two independent bodies, same five anomalies, same cluster.

- [s6] [s6] Starting state, FIFTH recurrence: src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again (only ledgers are ever committed, never the src edit). Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly — score 18, target_insns 28, build_insns 28, rules_dropped 9, cheat_asm_stripped 10. src/ings2.c is left carrying the floor form; motion_Close (the twin, a separate queue item) was not touched.

- [s6] [s6] NEW RESIDUAL CLASS E — delay-slot PROVENANCE, proven unreachable. Target's `bnez $t0` slot holds `ori $t0,$zero,0x1`, which clobbers the branch's own test register, so it can only have come from the conditional arm via reorg.c's fall-through scan (reorg.c:3075); reorg.c:3048 guards that scan with `slots_filled != slots_to_fill`, so it is reached only when the backward scan (reorg.c:2960) fails — and three eligible saves sit immediately before target's branch. Instrumented cc1 (BB2_DBR_DEBUG=1): `DBRDBG simp insn=11 trial=73 refset=0 setset=0 setneed=0` / `elig=1`, i.e. the nearest preceding save `sw $16,16($sp)` is eligible and IS taken into the slot.

- [s6] [s6] Class E is INDEPENDENT of class D: even if the save order were flipped to target's ascending shape, reorg would still eat the third save into the delay slot, so target's arrangement (three saves intact AND an arm insn in the slot) is doubly unreachable.

- [s6] [s6] CORPUS CENSUS for class E (s6/slotscan.py, all 1434 asm/funcs/*.s): 1346 no >=3-save run early, 13 branch slot = another save, 36 run not followed by a branch, 39 intact run + non-save filler. All 39 of those fillers are entry-block computations the backward scan preferred because they sat nearest the branch (35x `addiu $s3,$a0,-0x1` from the incoming $a0, 2x `lh $s7,0x60($s2)`); spot-check func_8004C994 is a zero-rule, unqueued (matched pure-C) function of exactly that shape. None is an arm insn clobbering the branch's test register — func_80083794 and its twin are the only instances.

- [s6] [s6] CLASS D CONFIRMED AT PASS LEVEL (s6/dump/): the three callee-saves are absent from .greg, first appear in .jump2 as RTL insns 69/71/73 descending (ra@24, s1@20, s0@16), survive .sched2 in the same order, and .dbr steals insn 73 into the branch delay slot. Ordering pass = save_restore_insns (mips.c:4680); the only possible reorderer is schedule_insns/sched2, which this function gives no anti-dependence to act on.

- [s6] [s6] H7 KILLED — the unfilled RETURN delay slot is NOT a hand-asm fingerprint: 838 of 1434 shipped functions have exactly target's `addiu $sp,+N; jr $ra; nop` shape, 288 are leaf+nop, 158 other filler, only 94 sink the restore into the slot (s6/epilogue_scan.py).

- [s6] [s6] H8 KILLED — external corroboration is measured-unavailable. disc/STR/MOVOVL.EXE (second, independently linked PsyQ executable) has zero hits for the leaf-frame save signature and zero for the ctor-walk loop; its only ascending >=3 sw-to-sp run is `sw zero,64/72/80($sp)` in vsprintf; its entry 0x801DA084 is `lui gp / addiu gp / j 0x801D91CC` with no ctor machinery. The same scan on the main EXE hits exactly twice (func_80083794 + its twin). Two web searches found no primary SN Systems / PsyQ crt0 `__main` listing. Frontier F1's named external-corroboration probe is CLOSED — do not re-run it.

- [s6] [s6] The twin body at 0x80083804 (motion_Close, separate queue item) carries every anomaly: the 0x10 leaf frame for a body containing a jalr, the ascending s0@4/s1@8/ra@0xC save run, the anti-priority roles ($s0 = pointer, $s1 = counter, $t0 = temp), and an unfilled `nop` in its `beqz $t0` slot with three eligible saves in front of it — which reorg.c's backward scan would have consumed. Two independent bodies in the same crt0 cluster, identical five anomalies.

- [s6] [s6] The evidence for the disposition question is now FIVE independent mechanism/corpus results (A frame geometry 1/1437, B anti-priority allocation, C small-immediate ori 3/3 hand-written, D length-3 ascending save run 1/1437, E delay-slot provenance) against a scan_hand_coded.py verdict of tier=LOW 0/8 from a scanner that has no frame-geometry, no assembler-macro, no prologue-order, no allocation-priority and no delay-slot-provenance signal. s5's arithmetic lower bound (no pure-C form can score below 9) is unchanged.

## Session 7 (forensics, 2026-08-13)

### Starting state (the recurring correction, fifth time)
`src/ings2.c` again held the s1 register-pin + hardcoded-`$17` `__asm__` body at
session start (only ledgers are committed, never the src edit). Re-applied
`memory/grind/func_80083794/candidate.c` and reproduced the floor exactly:
**score 18, target_insns 28, build_insns 28, rules_dropped 9,
cheat_asm_stripped 10**. `src/ings2.c` is left carrying the floor form.
`motion_Close` (the twin at 0x80083804) still carries its own pin form and
remains a separate queue item — untouched.

### The decisive new instrument: the ORIGINAL PsyQ compiler as a differential
Six sessions of evidence rested on mechanism arguments read out of
`tools/gcc-2.7.2/` — i.e. out of the decompals FORK. The standing counter-
explanation for every one of the five residual classes was therefore always
available to a skeptic: *"this is fork-vs-cc1psx divergence, not hand-written
asm."* `.claude/rules/difficult-is-not-impossible.md` mandates settling exactly
that empirically before any pessimistic claim, and no session had done it.
`tools/cc1psx_wrapper.sh` runs the ORIGINAL PsyQ `cc1psx.exe`
(**GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation**, per its own banner) —
the compiler that actually built this game — as a drop-in cc1. It is
DIAGNOSTIC-ONLY here ([[cc1psx-calibration-only]], [[no-compiler-divergence]]);
nothing in this session proposes changing the build.

Driver: `tmp/grind/func_80083794/s7/probe_cc1psx.sh` (+ `olevels.sh`), input
`tmp/grind/func_80083794/s7/mini.i` (= s6's minimal TU carrying the exact
score-18 floor body), flags `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-w` (the canonical `CC_FLAGS` minus the GNU-only tokens cc1psx does not accept).

**cc1psx output (`s7/psx.s`), verbatim shape:**

    .frame  $sp,32,$31    # vars= 0, regs= 3/0, args= 16, extra= 0
    lw    $2,D_800A2668
    subu  $sp,$sp,32
    sw    $31,24($sp)
    sw    $17,20($sp)
    bne   $2,$0,$L2
    sw    $16,16($sp)          <- delay slot: the nearest preceding SAVE
    li    $2,0x00000001        <- NOT `ori`
    sw    $2,D_800A2668
    la    $16,D_00000000       <- $s0 = count
    la    $17,D_8008D070       <- $s1 = p
    beq   $16,$0,$L2
    $L5:  lw $2,0($17) / addu $17,$17,4 / jal $31,$2 / addu $16,$16,-1
    bne   $16,$0,$L5

Class by class, against target:

| class | target | cc1psx (ORIGINAL compiler) | our fork |
|---|---|---|---|
| A frame | `-0x10`, args=0 (leaf-shaped) | **32, `args= 16`** | 32, `args= 16` |
| B roles | `$s0`=p, `$s1`=count, temp `$t0` | **`$s0`=count, `$s1`=p, temp `$v0`** | identical |
| C const | `ori $t0,$zero,1` | **`li $2,0x00000001`** | identical |
| D saves | ASCENDING `s0@4,s1@8,ra@0xC` | **DESCENDING `ra@24,s1@20,s0@16`** | identical |
| E slot | arm insn (`ori`) clobbering the tested reg | **nearest preceding save `sw $16,16($sp)`** | identical |

Modulo label spelling (`$L2` vs `.L2`) and the assembler-directive preamble, the
two compilers emit the SAME instruction sequence for this function
(`diff` of `s7/ours.s` vs `s7/psx.s` in the ledger's artifacts).

`s7/olevels.sh` sweeps `-O0/-O1/-O2/-O3` on BOTH compilers:
`args= 16` at **every** level on **both** (`-O0` gives `.frame $fp,40` with
`vars= 8, args= 16`; `-O1/-O2/-O3` all give `.frame $sp,32 ... args= 16`), and
cc1psx emits the callee-save run descending (`sw $31,24 / sw $17,20 /
sw $16,16`) at every optimizing level. The leaf frame is not an optimization-
level artifact in either compiler.

**Consequence: "fork divergence" is eliminated as the explanation for ALL FIVE
residual classes at once.** The residual is not a property of the decompals
fork; the compiler that shipped this game produces our code, not target's.

### Class C: the last alternative explanation closed (5537 to 3)
s2 proved GNU `as` expands `li rX,1` to `addiu rX,$zero,1`. The one surviving
alternative was that the ORIGINAL assembler (ASPSX 2.34, which maspsx emulates)
expanded `li` to `ori rX,$zero,imm` for small non-negative immediates — which
would have made target's `ori $t0,$zero,1` ordinary toolchain output. cc1psx's
output above shows the original compiler does emit `li` here, so the question is
purely what ASPSX did with it, and the shipped executable answers it directly:
`tmp/grind/func_80083794/s7/li_form_census.py` over all 1434 `asm/funcs/*.s`
counts **5537 `addiu $rX,$zero,imm` with imm < 0x8000** against **3
`ori $rX,$zero,imm` with imm < 0x8000**. ASPSX 2.34 expanded small-immediate
`li` to `addiu`, 5537 times out of 5540. The three exceptions are
`func_80052788` and `func_800527FC` (both `ori $t3/$v0,$zero,0x1000`, both
`INCLUDE_ASM` GTE bodies whose neighbouring lines are annotated
`/* handwritten instruction */`) and func_80083794's own `ori $t0,$zero,0x1`.

### The twin body as a WITHIN-REGION control experiment for class E (new)
`asm/funcs/func_80083794.s` lines 32-59 are the unlabelled twin at 0x80083804 —
the same routine with the flag test inverted and no `initialized = 1` store.
Its branch is `beqz $t0,.L80083854` at 0x8008381C and its delay slot is
**`nop`** — with the SAME three callee-save stores (`sw $s0,0x4 / sw $s1,0x8 /
sw $ra,0xC`) sitting immediately ahead of it, exactly as in the scored body.

Measured (`tmp/grind/func_80083794/s7/twin.sh`, input `s7/twin.i` = the same
minimal TU with `if (D_800A2668 != 0)` and no flag store): BOTH our fork and
cc1psx compile that shape to

    beq   $2,$0,.L2
    sw    $16,16($sp)          <- the save IS stolen into the slot

i.e. every compiler in reach fills the twin's slot with a save. Target leaves it
`nop`. So within a single 60-instruction region the shipped code has two
instances of one branch shape with identical eligible fillers, and fills one
slot with an insn pulled out of the conditional arm while leaving the other
empty. `reorg.c`'s `fill_simple_delay_slots` backward scan (reorg.c:2960) would
fill BOTH; nothing in the pass is stateful across functions. This is a control
experiment inside the target itself, independent of any corpus census, and it is
the sixth corpus/mechanism result pointing the same way.

- [s7] STARTING STATE recurred for the FIFTH session: src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again (only ledgers are ever committed, never the src edit). Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly at score 18 / target_insns 28 / build_insns 28 / rules_dropped 9 / cheat_asm_stripped 10. src/ings2.c is left carrying the floor form; motion_Close (the twin at 0x80083804) still carries its own pin form and is a separate queue item.

- [s7] THE ORIGINAL PsyQ COMPILER PRODUCES OUR CODE, NOT TARGET'S. tools/cc1psx_wrapper.sh runs cc1psx.exe (banner: 'GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation') — the compiler that built this game — as a drop-in cc1; it is diagnostic-only per .claude/rules/cc1psx-calibration-only.md and nothing here proposes changing the build. Fed the exact score-18 floor body (tmp/grind/func_80083794/s7/mini.i) at the canonical flags, cc1psx emits `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16, extra= 0`, saves DESCENDING (sw $31,24 / sw $17,20 / sw $16,16), fills the flag-test branch's delay slot with the nearest preceding SAVE (`bne $2,$0,$L2` / `sw $16,16($sp)`), materialises the flag with `li $2,0x00000001` (not `ori`), and assigns $16=count / $17=p with the temp in $2 — i.e. it reproduces ALL FIVE residual classes exactly as our fork does. Modulo label spelling and the directive preamble the two compilers emit the same instruction sequence (s7/ours.s vs s7/psx.s).

- [s7] The leaf frame is not an optimization-level artifact: tmp/grind/func_80083794/s7/olevels.sh sweeps -O0/-O1/-O2/-O3 on BOTH compilers and every single configuration reports `args= 16` (-O0: .frame $fp,40 vars= 8 args= 16; -O1/-O2/-O3: .frame $sp,32 vars= 0 args= 16), with cc1psx emitting the callee-save run descending at every optimizing level. Target's frame is 0x10 with args=0 and ascending saves.

- [s7] CONSEQUENCE: 'fork-vs-cc1psx divergence' is eliminated as the explanation for all five residual classes simultaneously. Sessions 1-6 argued each class from the decompals fork's own source (calls.c / mips.h / mips.c / global.c / reorg.c), which left that counter-explanation formally open; .claude/rules/difficult-is-not-impossible.md mandates settling it empirically with cc1psx before any pessimistic claim, and no prior session had run it. It is now run and negative.

- [s7] CLASS C's last alternative explanation is closed 5537 to 3. The surviving possibility was that ASPSX 2.34 (which maspsx emulates) expanded small-immediate `li` to `ori $rX,$zero,imm` — cc1psx does emit `li $2,0x00000001` here, so the question was purely the assembler's. tmp/grind/func_80083794/s7/li_form_census.py over all 1434 asm/funcs/*.s counts 5537 `addiu $rX,$zero,imm` with imm < 0x8000 against 3 `ori $rX,$zero,imm` with imm < 0x8000. ASPSX expanded small `li` to addiu 5537 times out of 5540; the three exceptions are func_80052788 and func_800527FC (both `ori $rX,$zero,0x1000`, both INCLUDE_ASM GTE bodies with neighbouring `/* handwritten instruction */` annotations) and func_80083794's own `ori $t0,$zero,0x1`.

- [s7] WITHIN-REGION CONTROL EXPERIMENT for class E (new, and independent of any corpus census): the unlabelled twin at 0x80083804 (asm/funcs/func_80083794.s lines 32-59) is the same routine with the flag test inverted and no flag store. Its branch `beqz $t0,.L80083854` at 0x8008381C has an UNFILLED delay slot (nop) with the SAME three callee-saves (sw $s0,0x4 / sw $s1,0x8 / sw $ra,0xC) sitting immediately ahead of it. Measured with tmp/grind/func_80083794/s7/twin.sh on s7/twin.i (the minimal TU with `if (D_800A2668 != 0)` and no flag store): BOTH our fork and cc1psx emit `beq $2,$0,.L2` with `sw $16,16($sp)` in the slot. So the shipped code contains two instances of the same branch shape with identical eligible fillers 0x70 bytes apart, fills one with an insn pulled out of the conditional arm (clobbering the tested register) and leaves the other empty — a combination reorg.c's backward-scan-first fill_simple_delay_slots cannot produce for either instance.

- [s7] The honest floor is UNCHANGED at 18 and no new C form was proposed or measured this session; forensics added evidence about provenance, not gradient. s5's arithmetic lower bound (no pure-C form can score below 9) is untouched.

- [s7] [s7] STARTING STATE recurred for the FIFTH session: src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body again (only ledgers are ever committed, never the src edit). Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly at score 18 / target_insns 28 / build_insns 28 / rules_dropped 9 / cheat_asm_stripped 10. src/ings2.c is left carrying the floor form; motion_Close (the twin at 0x80083804) still carries its own pin form and is a separate queue item.

- [s7] [s7] THE ORIGINAL PsyQ COMPILER PRODUCES OUR CODE, NOT TARGET'S. cc1psx.exe (banner 'GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation'), fed the exact score-18 floor body at the canonical flags, emits '.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16, extra= 0', saves DESCENDING (sw $31,24 / sw $17,20 / sw $16,16), fills the flag-test branch's delay slot with the nearest preceding SAVE ('bne $2,$0,$L2' / 'sw $16,16($sp)'), materialises the flag with 'li $2,0x00000001' (not ori), and assigns $16=count / $17=p with the temp in $2 - i.e. it reproduces all five residual classes exactly as our fork does. Modulo label spelling and the directive preamble the two compilers emit the same instruction sequence (s7/ours.s vs s7/psx.s). cc1psx is diagnostic-only here per .claude/rules/cc1psx-calibration-only.md; nothing in this session proposes a build change.

- [s7] [s7] The leaf frame is not an optimization-level artifact: olevels.sh sweeps -O0/-O1/-O2/-O3 on BOTH compilers and every configuration reports args= 16 (-O0: .frame $fp,40 vars= 8 args= 16; -O1/-O2/-O3: .frame $sp,32 vars= 0 args= 16), with cc1psx emitting the callee-save run descending at every optimizing level. Target's frame is 0x10 with args=0 and ascending saves.

- [s7] [s7] CONSEQUENCE: 'fork-vs-cc1psx divergence' is eliminated as the explanation for all five residual classes simultaneously. Sessions 1-6 argued each class from the decompals fork's own source, which left that counter-explanation formally open; .claude/rules/difficult-is-not-impossible.md mandates settling it empirically before any pessimistic claim, and it is now settled negative.

- [s7] [s7] CLASS C's last alternative explanation is closed 5537 to 3: li_form_census.py over all 1434 asm/funcs/*.s counts 5537 `addiu $rX,$zero,imm` with imm < 0x8000 against 3 `ori $rX,$zero,imm` with imm < 0x8000. ASPSX 2.34 expanded small `li` to addiu 5537 times out of 5540; two of the three exceptions are INCLUDE_ASM GTE bodies annotated /* handwritten instruction */ and the third is func_80083794 itself.

- [s7] [s7] WITHIN-REGION CONTROL EXPERIMENT for class E, independent of any corpus census: the twin at 0x80083804 has the same branch shape behind the same three eligible callee-saves and leaves its delay slot EMPTY, while both our fork and cc1psx fill that exact shape with 'sw $16,16($sp)'. The shipped code fills one slot from the conditional arm (clobbering the tested register) and leaves the other empty - a combination reorg.c's backward-scan-first fill_simple_delay_slots cannot produce for either instance.

- [s7] [s7] The honest floor is UNCHANGED at 18 and no new C form was proposed or measured this session; forensics added evidence about provenance, not gradient. s5's arithmetic lower bound (no pure-C form can score below 9) is untouched. No cheat construct was written, considered or banked.


## Session 8 (rederive, 2026-08-13)

STARTING STATE recurred for the SIXTH session: src/ings2.c held the s1
register-pin + hardcoded-$17 `__asm__` body (only ledgers are ever committed,
never the src edit). Re-applied memory/grind/func_80083794/candidate.c and
reproduced the floor exactly: score 18 / target_insns 28 / build_insns 28 /
rules_dropped 9 / cheat_asm_stripped 10. src/ings2.c was reverted to HEAD at the
end of the session (candidate.c remains the authoritative carrier of the floor).

### The modality's own two probes, both negative
`tools/m2c/m2c.py --target mipsel-gcc-c` on asm/funcs/func_80083794.s
(s8/m2c_rederive.txt) reconstructs the candidate body verbatim -- same guard,
same flag store, same walking pointer + descending counter, same `do/while`
under an `if (count != 0)` pre-test; the only deltas are m2c's `var_s0 += 4`
pointer arithmetic in bytes and its reading of the link-time-absolute count as
the literal 0. There is no structurally different shape hiding in the target: the
28 target instructions admit exactly one dataflow.

Sibling/transplant hunt across the 3754-scratch decomp.me corpus
(s8/frame_probe.txt, bottom section): 40 candidates by name (`__main`,
`global_ctors`, `crt0`, `main`) or by ctor-walk shape (`jalr` + `addiu
$sN,$sN,0x4`); NONE is a ctor-table walk and only two of the shape hits are
`is_matching` at all. No PS1 project in the corpus has matched a libgcc/crt0
`__main` from C. There is no transplant source and no sibling to diff against.

### NEW EXTERNAL CORROBORATION -- the decomp.me matched corpus as a control
tmp/decomp_me_corpus holds 3754 scratches; 1740 are `is_matching` under a
gcc2.7.2 / psyq3.5 compiler. Every one of those is a target function that
somebody ALREADY closed from real C with this toolchain family, so it is a
control group for "can C produce shape X?" that is completely independent of
BB2's own binary (which is what sessions 1-7's censuses used).

**Class A (frame geometry) -- 0 counterexamples in 1246.** Of the matching
scratches whose target contains a `jal`/`jalr`, 1246 have callee-save stores.
The distribution of the LOWEST callee-save offset: 1019 at exactly +16, 108 at
+24, 37 at +32, 12 at +40, ... and **zero below +16** (7 at +20 are the only
non-multiple-of-8 entries and they still clear the block). Not one matched
calling function in the corpus stores a callee-save inside the bottom 16 bytes.
func_80083794's target stores $s0 at +4 of a 0x10 frame. The o32
outgoing-argument block is universal in this toolchain family exactly as
calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474 predict.
(s8/frame_probe.txt)

**Class E (delay-slot provenance) -- 0 counterexamples in 484.** The GENERAL
shape "branch delay slot holds an insn that writes the branch's own test
register" is ORDINARY: 484 of the 1740 matching scratches contain it, so s6's
claim must be stated in its narrow form, and this session states it precisely.
BB2's configuration is the narrow one: the filler is taken from the conditional
arm even though the reorg.c backward scan (reorg.c:2960) had a run of adjacent
preceding callee-save stores available. Measured over those same 484 instances:
480 have ZERO adjacent preceding callee-save stores, 4 have exactly one, and
**0 have two or more**. func_80083794 has three (`sw $s0,0x4 / sw $s1,0x8 /
sw $ra,0xC`). (s8/classE_probe.txt)

**Class D (prologue save order) -- NOT a no-C-form class, as s3 already
measured.** 33 matching scratches emit a >=3-deep callee-save run in ASCENDING
offset order (vs 143 descending and 331 mixed), so ascending IS reachable from
C. This corroborates s3's finding that the hoisted-`la` form produces the
ascending order -- and leaves s3's JOINT-unreachability result (ascending costs
+5, 18 -> 23) as the operative constraint. (s8/corpus_census.txt)

### CLASS C REINTERPRETED -- an ASSEMBLER-VERSION fingerprint, not a
### hand-written-assembly fingerprint
This is the session's most consequential correction, and it revises the
INFERENCE s7 drew (not its measurement).

Corpus fact: 591 of the 1740 matching scratches contain a small-immediate
`ori $rX,$zero,imm` (imm < 0x8000) -- including plain `ori $v0,$zero,0x1`.
These are MATCHED reconstructions, i.e. the shape is ordinary compiler+assembler
output somewhere in the GCC-2.7.2/PS1 world, not a handwriting artifact.
Broken down by the same function's `la` flavour (s8/ori_style_probe.txt):
238 sit in wholly ori-flavoured functions (`lui`+`ori` for `la`), 100 mix, 169
have no `la`, and **84 sit in functions whose `la`s are entirely addiu-flavoured
(`lui`+`addiu %lo`) -- exactly BB2's flavour**, so the two spellings are
independent knobs and the mix target exhibits is a real, attested combination.
Worked example: decomp.me scratch `co4Jn` (func_80089174, gcc2.7.2-psx,
`-O2 -G8 -g2`, is_matching) has `addiu $s0,$s0,%lo(D_800B2384)` AND
`jal func_8009CF78 / ori $a0,$zero,0x1` in the delay slot, from the plain C
argument `func_8009CF78(1, ...)`.

Mechanism, established from our own frozen toolchain's source:
  * cc1 NEVER emits `ori` for a constant load. mips.md's only `ori` producer is
    `iorsi3` (mips.md:1899-1908) whose operand 1 is a REGISTER (`uns_arith_operand`
    with a "d" constraint), and the constant-move path is mips.c's
    `mips_move_1word`, which emits `li\t%0,%X1` for positive constants. So the
    ori/addiu choice is made BELOW cc1, in the assembler.
  * our maspsx decides it in `expand_load_immediate` (tools/maspsx/maspsx/__init__.py
    :213-241): `0 < imm <= 0x7FFF` -> `addiu $rD,$zero,imm`; `0x7FFF < imm <
    0x10000` -> `ori $rD,$zero,imm`. The function carries the in-source comment
    "ori is actually addiu on ASPSX 2.56+" -- i.e. the split is an ASPSX VERSION
    property, and older ASPSX modes emit `ori` where 2.56+ emits `addiu`.

Consequences, kept separate on purpose:
  1. UNCHANGED: class C is unreachable under OUR frozen pipeline. No C source
     can produce `ori $t0,$zero,1` here, because cc1 emits `li 1` and maspsx maps
     `li 1` to `addiu`. The floor arithmetic (s5: >= 9) is untouched.
  2. REVISED: s7's H11 kill established that BB2's own executable is 5537 addiu
     to 3 ori, and s7 read that as "the 3 exceptions are hand-written". The
     corpus shows the ori spelling is what a DIFFERENT ASPSX MODE emits for an
     ordinary `li`. The better reading of func_80083794's `ori $t0,$zero,0x1` is
     therefore: this object code was assembled by a different assembler mode than
     the other 5537 constant-load sites in the shipped executable -- a FOREIGN
     TOOLCHAIN / PREBUILT OBJECT fingerprint rather than a handwriting one.
  3. That reading UNIFIES all five residual classes under one explanation
     instead of five: func_80083794's bytes were not produced by this project's
     compilation at all, they were linked in from a prebuilt PsyQ/SN object
     (crt0 / libgcc `__main`, whose identity s1 established from the call graph).
     A foreign build explains simultaneously the non-o32 leaf frame (A), the
     anti-priority register assignment (B), the foreign assembler's ori (C), the
     ascending prologue run (D) and the arm-sourced delay-slot fill (E) -- and it
     is consistent with s7's cc1psx differential, which showed the compiler that
     shipped THIS GAME produces our code from this C, not target's.

- [s8] [s8] STARTING STATE recurred for the SIXTH session: src/ings2.c held the s1 register-pin + hardcoded-$17 __asm__ body (only ledgers are ever committed, never the src edit). Re-applied memory/grind/func_80083794/candidate.c and reproduced the floor exactly - score 18 / target_insns 28 / build_insns 28 / rules_dropped 9 / cheat_asm_stripped 10 - then reverted src/ings2.c to HEAD at the end of the session. candidate.c remains the authoritative carrier of the floor.

- [s8] [s8] The rederive modality is SPENT on this function. m2c (tools/m2c/m2c.py --target mipsel-gcc-c) reconstructs the candidate body verbatim from the target: same guard, same flag store, same walking pointer plus descending counter, same do/while under an if (count != 0) pre-test. The 28 target instructions admit exactly one dataflow, so there is no structurally different C shape to derive.

- [s8] [s8] There is NO transplant source: a name+shape sweep for __main / __do_global_ctors / crt0 / ctor-table walks over all 3754 decomp.me scratches returns 40 candidates, none of them a ctor walk, and no matched __main in any PS1 project. Session 6 closed the offline external-corroboration route (MOVOVL.EXE); session 8 closes the corpus route too.

- [s8] [s8] NEW CONTROL GROUP: tmp/decomp_me_corpus holds 3754 scratches of which 1740 are is_matching under gcc2.7.2 / psyq3.5. Each is a target function somebody ALREADY closed from real C with this toolchain family, so it is a reachability control that is independent of BB2's binary - the evidence base sessions 1-7 lacked.

- [s8] [s8] CLASS A, 0 counterexamples in 1246: among matching scratches whose target calls (jal/jalr) and has callee-saves, the lowest callee-save offset is 1019x +16, 108x +24, 37x +32, 12x +40, 11x +48, 9x +56, 9x +80, 7x +20, ... and ZERO below +16. func_80083794's target stores $s0 at +4 of a 0x10 frame. The o32 outgoing-argument block (calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474) is universal in this toolchain family.

- [s8] [s8] CLASS E, sharpened and 0 counterexamples in 484: the general shape 'delay slot writes the branch's own test register' is ORDINARY (484 of 1740 matching scratches), so s6's claim only holds in its narrow form - and in that form it holds absolutely. Of those 484, 480 have ZERO adjacent preceding callee-save stores, 4 have exactly one, NONE has two or more; func_80083794 has three. reorg.c's backward scan (2960) had three eligible fillers and the shipped code used an arm insn anyway.

- [s8] [s8] CLASS D is C-reachable: 33 matching scratches emit a >=3-deep ASCENDING callee-save run (vs 143 descending, 331 mixed). This corroborates s3's hoisted-la measurement and leaves s3's joint-unreachability result (ascending costs +5, 18 -> 23) as the operative constraint.

- [s8] [s8] CLASS C REINTERPRETED (the session's most consequential correction, and it revises s7's INFERENCE, not its measurement): 591 matching corpus scratches contain a small-immediate ori $rX,$zero,imm, including plain ori $v0,$zero,0x1, and 84 of them are in functions whose la's are entirely addiu-flavoured exactly like BB2's - so the two spellings are independent knobs and target's mix is an attested combination. Worked example: decomp.me scratch co4Jn (func_80089174, gcc2.7.2-psx, -O2 -G8 -g2, is_matching) has addiu $s0,$s0,%lo(D_800B2384) AND jal func_8009CF78 / ori $a0,$zero,0x1 in the delay slot, produced by the plain C argument func_8009CF78(1, ...).

- [s8] [s8] MECHANISM for the ori/addiu split, from our own frozen toolchain's source: cc1 never emits ori for a constant load (mips.md:1899-1908 iorsi3 takes a REGISTER operand 1; the constant-move path is mips.c mips_move_1word, which emits li\t%0,%X1 for positive constants), and the choice is made below cc1 in the assembler layer - tools/maspsx/maspsx/__init__.py expand_load_immediate (lines 213-241) maps 0 < imm <= 0x7FFF to addiu $rD,$zero,imm and only 0x8000-0xFFFF to ori $rD,$zero,imm, carrying the in-source comment 'ori is actually addiu on ASPSX 2.56+'. The spelling is an ASPSX-VERSION property, not a C-level or handwriting one.

- [s8] [s8] CONSEQUENCES kept separate: (1) UNCHANGED - class C is unreachable under our frozen pipeline, because cc1 emits li 1 and maspsx maps li 1 to addiu; s5's arithmetic lower bound (no pure-C form scores below 9) is untouched and the floor stays 18. (2) REVISED - func_80083794's ori is best read as evidence that this object code was assembled by a different assembler mode than the other 5537 constant-load sites in the shipped executable, i.e. a FOREIGN-TOOLCHAIN / PREBUILT-OBJECT fingerprint rather than a hand-written-assembly one.

- [s8] [s8] UNIFYING READING for the disposition: all five residual classes collapse into ONE claim instead of five - func_80083794's bytes were linked in from a prebuilt PsyQ/SN object (crt0 / libgcc __main, whose identity s1 established from the call graph), not compiled from this project's C. That single explanation covers the non-o32 leaf frame (A), the anti-priority register assignment (B), the foreign assembler's ori (C), the ascending prologue run (D) and the arm-sourced delay-slot fill (E), and it is consistent with s7's cc1psx differential showing the compiler that shipped THIS GAME produces our code from this C, not target's. NOTE for the eventual escalation entry: 'prebuilt foreign object' is NOT the same claim as 'hand-written asm', so it does not make the canonical-asm gate easier.

- [s8] [s8] No new C form was proposed, and no cheat construct was written, considered or banked. The honest floor is UNCHANGED at 18.

## Session 9 (escalation) — disposition reached, both endgame gates measured FAIL

Modality: escalation (driver-declared exhaustion: floor FLAT at 18 across sessions 1-8 and
five distinct modalities — recon, structural x2, permuter x2, forensics x2, rederive —
including 265,869 total decomp-permuter iterations from four chassis). This session ran no
new grind variants; it re-measured the two facts a disposition needs and filed the ruling.

### Re-measured on main this session (not inherited)
1. `sandbox func_80083794 --disable all` against the COMMITTED body (register pins +
   hardcoded-`$17` `__asm__` decrement): **score 23**, build_insns 16, target_insns 28,
   rules_dropped 9, cheat_asm_stripped 18. This is the state of `src/ings2.c` at HEAD —
   note the committed source is still the pin form; only the ledger was ever committed.
2. Same command with `memory/grind/func_80083794/candidate.c` applied verbatim to
   `src/ings2.c`: **score 18**, build_insns 28, target_insns 28. The honest pure-C floor is
   confirmed at 18 for the ninth consecutive session. `src/ings2.c` was then reverted with
   `git checkout --`, so the tree is clean and the pin form remains committed.
3. Rule inventory: `grep -c func_80083794 regfix.txt asmfix.txt` → **regfix 9, asmfix 0**.
   The byte-match on main is held by those 9 regfix rules plus the cheat-asm body.

### GATE 1 (canonical-asm / hand-coded signals): FAIL
`tools/scan_hand_coded.py --single func_80083794` →
`HAND_CODED: tier=LOW score=0/8 (54 insns)`; artifact
`tmp/grind/func_80083794/s9/scan_hand_coded.txt`. All eight signals negative: S1 0 multu/mflo
pairs, S2 no empty-body branches, S3 6 spills over 3 distinct regs, S4 max load burst 1,
S5 no high-similarity siblings, S6 no BIOS jumptable pattern, S7 all callee-save uses have a
`$sp` save, S8 no redundant mask-before-shift. LOW is not STRONG, so the gate does not open.

### GATE 2 (in-hand SOTN-master precedent for a closing construct): FAIL
There is no candidate construct to cite a precedent for. The dominant residual class A is a
frame-size SHORTFALL IN THE WRONG DIRECTION: our build must emit the 16-byte o32
outgoing-argument block for any function that expands a call, while target's entire frame is
16 bytes and already holds 12 bytes of callee-saves. Every sanctioned family on the frozen
list can only ADD frame bytes or ADD instructions (written-never-read local array,
constant-holder locals, dead stores/self-assigns, pointer aliases, duplicated statement into
arms, do-while(0)); none can REMOVE a compiler-mandated argument area. So no family is even
applicable, and no file:line citation exists to make. Classes B/C/D/E were each measured
unreachable (or jointly unreachable) in sessions 2-8, and the search modality is closed by
arithmetic: 8 frame-referencing insns + the `ori` put a hard lower bound of 9 on any pure-C
score.

### NEW FACT this session — crt0 contiguity with `_start`
`asm/funcs/_start.s` spans 0x800836EC-0x80083790 and ends `jal main; nop; break 0,1`.
func_80083794 begins at the very next word (0x80083794), i.e. it is byte-contiguous with the
entry stub. `_start` is already carried in `src/ings2.c` as `INCLUDE_ASM("asm/funcs",
_start)` — the project already treats the immediately preceding function of this same linked
region as asm rather than pure C. This is direct provenance corroboration for the
"prebuilt PsyQ/SN crt0 object" conclusion the ledger reached in sessions 6-8 (the frame
geometry, the anti-priority allocation, the ASPSX-flavoured `ori`, the prologue save order,
the delay-slot provenance, the cc1psx differential, and the decomp.me matched-corpus
censuses). It is NOT a `scan_hand_coded` signal and NOT a SOTN precedent, so under the
owner's 2026-07-27 standing ruling it does not change the disposition — but it is the single
cheapest piece of evidence in the whole ledger and it is recorded here and in
docs/grind/decisions.md so no future session has to rediscover it.

### Disposition filed
`docs/grind/decisions.md` — `## 2026-08-13 — func_80083794 — **OWNER-ESCALATION — RESOLVED
BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**`. Terminal: nothing
pending on the owner; the driver parks the function and the queue advances. The honest
pure-C floor of 18 stands; the best honest body remains banked at candidate.c.

- [s9] Re-measured this session: sandbox --disable all with candidate.c applied = score 18, build_insns 28 == target_insns 28; with HEAD's committed pin/__asm__ body = score 23, build_insns 16. src/ings2.c reverted to HEAD afterwards; working tree carries only docs/grind/decisions.md + the two ledger files + metrics.

- [s9] What holds the byte-match on main: 9 regfix.txt rules (regfix.txt:105-113) plus the cheat-asm body; 0 asmfix.txt rules.

- [s9] GATE 1 FAIL: scan_hand_coded.py --single func_80083794 -> tier=LOW score=0/8, every signal negative (S1 0 multu/mflo pairs, S2 no empty-body branches, S3 6 spills/3 regs, S4 max load burst 1, S5 no similar siblings, S6 no BIOS jumptable, S7 all callee-saves have $sp saves, S8 no redundant mask).

- [s9] GATE 2 FAIL: no sanctioned family is applicable, because the residual requires the frame to SHRINK past a compiler-mandated 16-byte outgoing-argument block, and every family on the frozen SOTN list only adds frame bytes or instructions. No file:line precedent exists to cite because there is no closing construct to cite one for.

- [s9] Exhaustion inherited and unchallenged: floor FLAT at 18 across sessions 1-8 and five distinct modalities (recon, structural x2, permuter x2, forensics x2, rederive), 265,869 total decomp-permuter iterations from four chassis against both the shipped target and a normalized-target instrument, plus an arithmetic lower bound of 9 (8 frame-referencing insns + the ori) that makes distance 0 unreachable by search at any iteration count.

- [s9] NEW: func_80083794 is byte-contiguous with _start (asm/funcs/_start.s ends at 0x80083790 with `break 0,1`), and _start is already carried as INCLUDE_ASM("asm/funcs", _start) at src/ings2.c:610 - the two are adjacent members of the same linked crt0 region.

- [s9] Disposition filed by this session at docs/grind/decisions.md: '## 2026-08-13 - func_80083794 - **OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**'. Terminal - nothing pending on the owner.

- [s9] No cheat, coercion, or new construct was written this session; no candidate-ready is claimed and no self_vet is required (the diff to src/ is empty).
