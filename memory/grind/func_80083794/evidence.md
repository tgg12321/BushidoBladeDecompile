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
