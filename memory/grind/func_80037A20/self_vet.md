# SELF-VET — func_80037A20

Diff under vet: `src/code6cac_c.c` — the single `INCLUDE_ASM("asm/funcs", func_80037A20);`
line (line 268) replaced by the C body below.  Nothing else in the tree is touched: no
regfix.txt, no asmfix.txt, no rules, no headers, no Makefile, no linker script, no
new declarations (every symbol used — `sprintf`, `firstfile`, `nextfile`,
`D_80102810`, `g_str_memcard_fmt`, `D_800A38C8` — is already declared in
src/code6cac_c.c / include/code6cac.h at HEAD).

```c
s32 func_80037A20(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    sprintf(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (firstfile(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (nextfile(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
```

Measured THIS session (s13):
 - `sandbox func_80037A20 --disable all` = **0**, target_insns 33 / build_insns 33,
   `rules_dropped: 0`.
 - `wteng main build` = **MATCH**, link SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`
   == the oracle, with this body in place.  (This is the decisive check: an earlier
   draft of the same body spelled the callees `func_80079A30` / `bios_firstfile_B` /
   `bios_nextfile_B`, which ALSO scored sandbox 0 — object-level relocations are masked
   — but failed the link with three "undefined reference" errors.  The symbol names
   `sprintf` / `firstfile` / `nextfile` are the ones the linker resolves.)

CONSTRUCTS: none

## T1 semantic purpose
Every statement is load-bearing and would be required by any correct implementation of
"format a memory-card wildcard path from the two arguments, then count the directory
entries firstfile/nextfile walk":
 - `var_s0 = (s32 *)&D_80102810;` — the DIRENTRY scratch buffer handed to firstfile.
 - `sprintf(sp10, (s32)&g_str_memcard_fmt, arg0, arg1);` — formats the path into the
   32-byte stack buffer.  The `(s32)` cast is required by the existing project
   declaration `extern void sprintf(s32 *, s32, s32, s32);` (include/code6cac.h:501).
 - `var_s1 = 0;` — the count returned when firstfile finds nothing.
 - the `do { var_s1++; var_s0 += 0x28; } while (nextfile(var_s0));` loop — one count per
   entry, advancing by the 0x28-byte DIRENTRY stride.
 - `D_800A38C8 = var_s1; return var_s1;` — publish and return the count.
Removing any one of them changes the value the function computes.  There is no
statement, local, cast, wrapper, annotation or declaration whose removal would leave
behaviour byte-identical.  PASS.

## T2 human-programmer
Yes.  This is the textbook PsyQ memory-card file-count idiom: format the path, call
firstfile, then walk with nextfile counting as you go.  The `do/while` is the natural
shape because firstfile already proved the first entry exists; the increment sits at the
top of the loop body where a counting loop puts it; the byte-stride advance is spelled
through `u8 *` because the local is typed `s32 *` to match the splat symbol.  Nothing
here would make a reader ask "why is this here?".  PASS.

## T3 GCC-internals justification
No.  There is no construct in the diff to justify, and no GCC internal is load-bearing
for any line — the body is the simplest spelling measured across thirteen sessions (the
ledger's three-pseudo vJ chassis, the base/walking-pointer split, the hoisted zero-init
and the source-level peel/decrement are all DISCARDED in favour of this plain form).
Two cc1 behaviours explain, after the fact, why the ordinary form matches, and both are
first-hand observable in the emitted bytes rather than inferred: (a) target/our output
carries `addiu $s1,$s1,1` in the `bnez` delay slot plus a compensating
`addiu $s1,$s1,-1` after the loop, neither of which is written in the C — that is
reorg.c's delay-slot steal of the loop-top increment plus its own compensation, which is
why writing the decrement in C over-counts (rejected/s13-source-level-decrement-*.c);
(b) the increment survives as `addiu $s1,$s1,1` rather than the `li $s1,1` that every
earlier chassis produced, because the surviving increment sits after the loop's
CODE_LABEL, which terminates cse's extended basic block, so the dominating `var_s1 = 0`
is not const-propagated into it.  Neither was reached for as a lever; both are
consequences of writing the loop the way a human writes it.  PASS.

## T4 permuter/search provenance
No permuter, no auto-search, no solver output is in this diff.  The form is the plain
idiom, derived by hand.  It is not a spelling that survives only because a detector
misses it — there is nothing to detect.  It is additionally proven by the strongest
available oracle (full-build link SHA1), not by a detector-blind score.  PASS.

## T5 family check
No forbidden family is present, even by analogy: no register-asm pin, no `__asm__` of any
kind, no hardcoded `$N`, no regfix/asmfix rule, no scheduling barrier, no volatile, no
alias rename, no dead store or self-assign, no constant holder, no dead local, no local
array pad, no dead conditional or goto, no `if (1)`, no `do { } while (0)` wrapper (the
`do/while` here is the function's real loop, with a real condition and a real body), no
DImode chain, no redundant width cast, no linker/rodata reordering.  The diff adds three
ordinary locals — all read — five ordinary statements and one loop.  PASS.

## T6 naming-announces-intent
Locals are `var_s0`, `var_s1`, `sp10` — the project's standing m2c-derived convention for
not-yet-semantically-named locals, used throughout src/code6cac_c.c (e.g. func_80037AA4
immediately below uses `var_v1`/`var_a1`/`var_a0`/`var_v0`).  None of the banned
intent-announcing names (`pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`, `tail`,
`_frame_pad`) appears, and every local is genuinely read: `var_s0` is passed to both BIOS
calls, `var_s1` is stored and returned, `sp10` is written by sprintf and read by
firstfile.  PASS.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
