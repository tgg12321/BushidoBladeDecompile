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
