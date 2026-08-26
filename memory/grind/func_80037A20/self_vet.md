# SELF-VET — func_80037A20

Diff under vet: `src/code6cac_c.c` — the single `INCLUDE_ASM("asm/funcs", func_80037A20);`
line replaced by the C body below.  Nothing else in the tree is touched (no regfix.txt,
no asmfix.txt, no rules, no headers, no Makefile, no linker script).

```c
s32 func_80037A20(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    func_80079A30(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (bios_firstfile_B(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (bios_nextfile_B(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
```

Measured: `sandbox func_80037A20 --disable all` = **0**, 33/33 insns, `rules_dropped: 0`.
`canonical func_80037A20` = verdict **C**.

CONSTRUCTS: none

## T1 semantic purpose
Every statement is load-bearing for the function's behaviour and would be required by
any correct implementation of "sprintf a memory-card wildcard path, then count the
directory entries that firstfile/nextfile walk":
 - `var_s0 = (s32 *)&D_80102810;` — the DIRENTRY scratch buffer passed to firstfile.
 - `func_80079A30(sp10, &g_str_memcard_fmt, arg0, arg1)` — the sprintf that formats the
   path from the two parameters into the 32-byte stack buffer.
 - `var_s1 = 0;` — the count returned when firstfile finds nothing.
 - the `do { var_s1++; var_s0 += 0x28; } while (nextfile(var_s0));` loop — one count per
   directory entry, advancing the DIRENTRY pointer by the 0x28-byte record stride.
 - `D_800A38C8 = var_s1; return var_s1;` — publish and return the count.
Delete any one of them and the function computes a different value.  There is no
statement, local, cast, wrapper, annotation, or declaration in the diff whose removal
would leave behaviour byte-identical.  PASS.

## T2 human-programmer
Yes.  This is the textbook PsyQ memory-card file-count idiom: format the path, call
`firstfile`, then walk with `nextfile` counting as you go.  A reader given only the
specification would write exactly this, including the `do/while` (the first entry is
already known to exist because firstfile succeeded) and the byte-stride pointer advance
(`0x28` is `sizeof(struct DIRENTRY)`; the local is typed `s32 *` to match the splat
symbol's type, so the advance is spelled through `u8 *`).  Nothing in the body would
make a reader ask "why is this here?".  PASS.

## T3 GCC-internals justification
The body is NOT justified by a GCC internal; it is justified by the program logic above,
and it is the SIMPLEST spelling that was measured (the session also measured a
three-pseudo base/walking-pointer chassis and a loop-temp variant, both of which reach 0
or near-0, and DISCARDED them in favour of this plain one-pointer form).  cc1 internals
appear in this session's ledger only as *explanation after the fact* of why the ordinary
form matches: cse1 no longer const-folds the counter's first increment because that
increment now sits after the loop's CODE_LABEL (cse's extended-basic-block scan in
`cse_end_of_basic_block` stops at a CODE_LABEL — the dump shows block 1 shrink from
insns 2..47 to 2..44), and reorg.c steals that loop-top increment into the `bnez` delay
slot, redirecting the branch to a fresh label 106 and emitting its own compensating
`s1 -= 1` after the loop.  Both effects are consequences of writing the increment at the
top of the loop — which is where a human writes it — not levers reached for to move
bytes.  No pass name is load-bearing for any construct, because there is no construct.
PASS.

## T4 permuter/search provenance
No permuter, no auto-search, no solver output is in this diff.  The form was derived by
hand from a first-hand read of `tools/gcc-2.7.2/cse.c` and the `.cse`/`.dbr` RTL dumps
and then measured directly.  It is not a spelling that survives only because a detector
misses it — it is the plain idiom with zero constructs to detect.  PASS.

## T5 family check
No forbidden family is present, even by analogy: no register-asm pin, no `__asm__` of any
kind, no hardcoded `$N`, no regfix/asmfix rule, no scheduling barrier, no volatile, no
alias rename, no dead store, no self-assign, no constant holder, no dead local, no local
array pad, no dead conditional/goto, no `if (1)`, no `do { } while (0)`, no DImode chain,
no width-cast padding, no linker/rodata reordering.  The diff adds three ordinary locals
(all read), five ordinary statements and one loop.  PASS.

## T6 naming-announces-intent
Locals are `var_s0`, `var_s1`, `sp10` — the project's standing m2c-derived convention for
not-yet-semantically-named locals, used throughout `src/code6cac_c.c` (e.g.
`func_80037AA4` immediately below uses `var_v1`/`var_a1`/`var_a0`/`var_v0`).  None of the
banned intent-announcing names (`pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`,
`tail`, `_frame_pad`) appears, and every local is genuinely read: `var_s0` is passed to
both BIOS calls, `var_s1` is stored and returned, `sp10` is written by the sprintf and
read by firstfile.  PASS.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
