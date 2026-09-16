# Evidence bank — _exeque

## [s1] recon — floor 187 (raw INCLUDE_ASM) -> 15

OBJECT MODEL: the DATA MODEL section flagged `_que_plus_0x4` / `_que_plus_0x8`
as SPLIT-AGGREGATE (no header decl) with a comment in include/gpu.h:70-80
saying the merge should land "when _exeque leaves INCLUDE_ASM". MATCHES: the
struct is already declared (`extern volatile GpuQueueItem _que[64];`,
include/gpu.h:81-88, fields func@0/arg@4/count@8/data@0xC) and already
consumed via `_que[idx].field` in the sibling `_addque2` (src/display.c:891-930,
not INCLUDE_ASM — already a completed pure-C consumer of the SAME struct).
Writing `_exeque` against `_que[D_8009BF7C].func/.arg/.count` (never touching
`_que_plus_0x4`/`_que_plus_0x8` as bare externs) is therefore hypothesis #1 and
IS the correct object model — measured: canonical distance dropped
187 -> 24 the moment the function was written in C at all (verdict flips
INCLUDE_ASM -> "C", pure-C distance 24/187), and the instruction-level diff
(tmp/grind/_exeque/s1/diagdiff.py) shows the struct-based `_que[idx].field`
sequences for the CALL SITE (`_que[D_8009BF7C].func(...)`) match target's
per-field base-address recompute pattern byte-for-byte in that region. The
struct declaration itself is NOT the source of the remaining residual (see
below) — no further data-model work is indicated for this function.

Baseline: `canonical _exeque` before any edit reported verdict C, pure-C
distance 187 (== raw INCLUDE_ASM asm-insn count; nothing had been attempted).
Target asm/funcs/_exeque.s is 187 real instructions (33 of them plain `nop`,
mostly load-delay slots since the target is essentially unscheduled /
strictly source-ordered around the volatile `_que`/`D_8009BF7C`/`D_8009BF78`
accesses).

### Full manual decode of asm/funcs/_exeque.s (verified against the C written this session)

```
s32 _exeque(void) {
    if (*D_8009BF54 & 0x01000000) return 1;          /* early exit, masked already */
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C+1)&0x3F) == D_8009BF78 && D_8009BE80 == 0)
                DMACallback(2, 0);
            while (!(*D_8009BF48 & 0x04000000)) {}
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            D_8009BF6C  = (s32)_que[D_8009BF7C].arg;
            D_8009BF70  = _que[D_8009BF7C].count;
            D_8009BF7C  = (D_8009BF7C+1)&0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)
        && D_8009BE7C != 0 && D_8009BE80 != 0) {
        D_8009BE7C = 0;
        ((s32(*)(void))D_8009BE80)();
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
```
This decode is measured CORRECT at the control-flow level: with it written
verbatim, sandbox distance is 15/187 (built 186 insns vs target 187), and the
remaining diff (below) is pure instruction-scheduling/ordering, not a missing
or wrong branch/condition.

### Bug found + fixed this session: `D_8009BF84` was never declared

`D_8009BF84` (the saved interrupt mask that _exeque writes at entry and reads
before the unlock `SetIntrMask(D_8009BF84)` call) had **no extern declaration
anywhere in src/display.c** before this session — a genuine bug, not a style
choice, that predates this session's edits (nothing in the file referenced it
before _exeque was written). GCC 2.7.2 accepted the undeclared identifier
silently (implicit extern) but this produced observably wrong codegen for the
`SetIntrMask(D_8009BF84)` call (the built object loaded `a0 = 0` into the
call instead of the global's value — confirmed via
`objdump -dr --disassemble=_exeque` showing `move a0,zero` in the delay slot
of the `jal SetIntrMask` reached via `R_MIPS_26 SetIntrMask` relocation,
tmp/grind/_exeque/s1/mine.txt before the fix). Adding
`extern s32 D_8009BF84;` (src/display.c, alongside D_8009BE7C/D_8009BE80)
dropped the sandbox score 24 -> 15 with build_insns 183 -> 186 (closer to
target's 187). Measured via `sandbox _exeque --disable all` both before and
after the one-line addition.

### Remaining residual (floor 15) — pure scheduling, NOT object-model or missing-logic

Using `engine.score.normalized_insns(..., mask=False)` on `build/src/display.o`
(target reference, byte-correct build) vs `tmp/sandbox/_exeque/display.o`
(our cheat-disabled build) — script banked at
tmp/grind/_exeque/s1/diagdiff.py — every remaining diff block after masking
address-only branch-target noise is confined to ONE region: the post-call
"debug record" triple-write

```c
D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
D_8009BF6C    = (s32)_que[D_8009BF7C].arg;
D_8009BF70    = _que[D_8009BF7C].count;
```

Target computes+stores each field STRICTLY IN SOURCE ORDER (load `.arg` ->
immediately `sw` into `D_8009BF6C` -> THEN reload `D_8009BF7C` fresh -> load
`.count` -> immediately `sw` into `D_8009BF70`). Our build's list scheduler
instead loads `.arg` into `$a1` and `.count` into `$a0` WITHOUT storing them
yet, interleaves the `D_8009BF7C = (D_8009BF7C+1)&0x3F` increment and the
loop-continuation reload of `D_8009BF78`/`D_8009BF7C`, and only THEN emits
both `sw $a1,...` / `sw $a0,...` back to back just before the loop-exit
`beq`. Net effect: same instructions, same values, DIFFERENT SCHEDULE — this
is the class of diff [[difficult-is-not-impossible]] and
[[no-compiler-divergence]] describe as "the C source structure, not the
compiler, decides the schedule."

RULED OUT as an object-model / volatile-declaration fix: checked
`volatile_extern_allowlist.txt:75` — the D_8009BF68 grant's own justification
TEXT explicitly says "the SAME printf folds the non-volatile adjacent
siblings D_8009BF6C and D_8009BF70" as the evidence D_8009BF68 needs its own
volatile and they do NOT. So marking D_8009BF6C/D_8009BF70 volatile would
contradict banked project evidence (get_alarm's own closing grant) and is not
the fix — the residual is a pure scheduling/statement-structure question,
not a declaration gap. Did not test it (no need — the census evidence already
rules it out; see OBJECT MODEL note in [[unannotated-fake-inflates-honest-floor]]-adjacent
caution: don't spend a measurement disproving something the ledger already
answers).

- [s1] canonical _exeque baseline (before any edit): verdict C, pure-C distance 187 (raw INCLUDE_ASM insn count).

- [s1] After writing the full struct-based C body: canonical distance 24, sandbox score 24 (build_insns 183 vs target 187).

- [s1] After adding the missing `extern s32 D_8009BF84;` declaration: sandbox score 15 (build_insns 186 vs target 187).

- [s1] engine.score.normalized_insns(mask=False) diff between build/src/display.o (target reference) and tmp/sandbox/_exeque/display.o (our disabled build) shows every remaining real (non-branch-target-masking) diff is confined to the post-call debug-record triple-write (D_8009BF68[0]=func; D_8009BF6C=arg; D_8009BF70=count) - target stores each field immediately after computing it, our build's scheduler groups both non-func stores at the end of the loop body.

- [s1] _addque2 (src/display.c:892-930), the sibling function writing the SAME GpuQueueItem struct, is already pure C and not INCLUDE_ASM - corroborates the struct object model is correct project-wide, not just hypothesized.

- [s1] volatile_extern_allowlist.txt:75 (D_8009BF68 grant) explicitly documents D_8009BF6C and D_8009BF70 as the non-volatile siblings in the same debug-record triple, ruling out a volatile fix for the remaining residual.
