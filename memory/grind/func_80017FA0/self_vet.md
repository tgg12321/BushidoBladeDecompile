# SELF-VET — func_80017FA0

CONSTRUCTS: none

The whole diff against the previous candidate is ONE token-level change inside
the function body — the outer loop guard is spelled `if (i < ptr[1])` instead of
`if (ptr[1] > 0)` — plus a rewritten descriptive comment, a prototype widened to
its real parameter (`extern void func_80017FA0(s32 *);`) and the matching call
site `func_80017FA0(p0);`, and the replacement of the `INCLUDE_ASM("asm/funcs",
func_80017FA0);` line with the C body that has been banked in candidate.c since
s1. There is no `__asm__`, no register pin, no `volatile` beyond the scratchpad
MMIO-style pointers the function genuinely writes through, no dead local, no
pad, no self-assign, no alias, no `do {} while (0)`, no annotation.

## T1 semantic purpose: `i` is the loop counter; it is initialised to 0,
incremented in the loop body, and tested in the loop's own back-edge condition
`while (i < ptr[1])`. Spelling the ENTRY guard with the same comparison
(`i < ptr[1]`) is the same test the loop itself performs and is the rotated
form GCC produces from `for (i = 0; i < ptr[1]; i++)`. It is not a construct
added on top of the program — it is how the loop is written. The variable it
reads is live and semantically necessary in both spellings.

## T2 human-programmer: Yes. `for (i = 0; i < n; i++)` / `while (i < n)` is the
first thing a programmer writes for a counted loop; hoisting the guard out of
the rotated loop reproduces `if (i < ptr[1]) { ... do { ... } while (i < ptr[1]); }`,
which is the standard decomp spelling of a rotated `for`. A reader does not ask
"why is this here?" — asking why the guard compares the counter to the bound
would be asking why the loop is a loop. If anything the PREVIOUS spelling
(`ptr[1] > 0`, reversed operands, literal instead of the counter) is the one a
reader would query.

## T3 GCC-internals justification: No. The construct is justified by the program
logic (a counted loop tests its counter against its bound). GCC internals are
cited in candidate.c only as an EXPLANATION of why the byte-level residual
disappeared — `i` is a local that survives to frame layout, so `get_frame_size()`
reports vars=8 and `mips.c:compute_frame_size` emits the 8-byte leaf frame while
`i` still lives in a register so no frame store is emitted (the phantom-frame
artifact documented in memory/project/phantom-frame-slots-gcc272.md, byte-verified
in-tree on the COMPLETED-C function tslLineG5Init). Explanation of an observed
byte effect is not the same as a construct whose only reason for existing is that
effect: remove the GCC story entirely and `if (i < ptr[1])` is still the correct,
natural guard for this loop.

## T4 permuter/search provenance: The permuter DID surface this spelling
(tmp/perm_17fa0/output-0-3, one of 26 score-0 finds in 15,708 iterations), and
the campaign's other score-0 finds were dead-volatile-pad cheats which are
banked as REJECTED in rejected/perm-dead-volatile-local-frame-coercion.c. The
accepted form was not taken on the permuter's word: it was re-derived by hand as
variant vB (tmp/grind/func_80017FA0/s4/vB.c), measured independently
(`.frame $sp,8 # vars= 8`, 61/61 instructions byte-identical to
asm/funcs/func_80017FA0.s), applied to src, and confirmed by the ONLY authority
that matters — a full clean build whose SHA1 equals the oracle
62efab4f73f992798c43e8c730aa43baa10bb4fa. It does not pass detectors "because
they don't catch this spelling"; there is nothing to catch.

## T5 family check: No forbidden family applies, because no coercion construct
exists. It is not a dead local (i is read three times and written twice), not a
constant holder (i is not a constant — it counts), not an unused/written-never-read
array, not a self-assign, not an opaque arithmetic variable (no invented
decomposition — the comparison is the loop's real one), not a duplicated
statement, not a volatile coercion (the volatility on `scr`/`ac_base` is the
scratchpad MMIO the function actually writes and predates this session), not an
alias rename, not a scheduling barrier, not a DImode chain. The literal `0` that
was replaced was itself the value of `i` at that point, so nothing was added and
nothing was widened — one operand pair was spelled with the variable that holds
the value instead of the value.

## T6 naming-announces-intent: No. The only identifier involved is `i`, the loop
counter, which existed in the candidate before this session and is used for the
`i << 5` data offset, the `i++` step and the loop's back-edge test. No `pad`,
`dummy`, `unused`, `spill`, `_frame`, `slack` or similar name appears anywhere in
the function. (The permuter's own `pad` / `new_var` names appear only in the
REJECTED file, deliberately, as the record of what was refused.)

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no construct that needs a
family. Nothing in it is offered as an exception to any rule; it is ordinary C
whose bytes were verified against the oracle.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
