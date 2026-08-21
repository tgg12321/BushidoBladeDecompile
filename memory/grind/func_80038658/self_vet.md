# SELF-VET — func_80038658

CONSTRUCTS: switch-on-state-global; per-arm direct global stores
(`D_800A379E = 2/3/5/6/0xF;`); if/else two-constant select; arm-internal
`fail = 1;` / `fail = 4;` assignments; `goto fail_store;` mixed with inline
`return;` exits; shared end label `fail_store: D_800A379E = fail;`; bare
`return;` between the switch and the label.

## T1 semantic purpose
Every statement has observable effect on the function's output. Each
`D_800A379E = N` store posts a distinct result code the caller
(`func_80038734`) returns. `fail = 1/4` is written on the ret==0 path and
read at `fail_store` — a live, consumed value, not a dead store.
`close(D_800A3794)` and the `func_8003800C` sub-call are real calls. The bare
`return;` before `fail_store:` is control-flow-required (prevents the
default/fallthrough path from executing the error store). No construct is
byte-inert. PASS for all constructs.

## T2 human-programmer
The function is a two-state completion handler with a common "operation still
pending" error path. A switch on the state global, per-state result codes,
and a shared error-store label at the end (`goto fail_store`) is idiomatic
1990s C error handling — the matched sibling `func_800383A4` in the same TU
ships the same author's identical shared-`finish:`-label idiom. Nothing here
would prompt "why is this here?". PASS.

## T3 GCC-internals justification
The committed body is justified purely by program logic: dispatch, reap
status, close handle, post code. The session's ledger documents WHY rejected
alternatives failed in GCC terms (cse compare-reuse, cross-jump placement),
but no construct in the FINAL diff exists to manipulate a pass — the final
form is the plainest natural spelling tried, and the GCC-internals analysis
was used only to diagnose why less-natural forms (shared accumulator,
pre-branch assignment) diverged. No lever construct present. PASS.

## T4 permuter/search provenance
No permuter or auto-search used. All forms hand-derived from the target asm
+ m2c-style reading + .greg dump; each measured directly via sandbox. PASS.

## T5 family check
Checked against the forbidden-family catalog: no register pins, no `__asm__`,
no volatile, no dead stores/self-assigns, no constant-holders, no unused
locals/arrays/pads, no pointer aliases, no do-while(0), no empty bodies, no
cancellation pairs, no alias renames. The `goto fail_store` + inline
`return` mix is the "mixed exit forms" shape — ordinary C on the SOTN
FROZEN accepted list, and per the family-selection table
(`.claude/rules/cross-jump-store-tail-merge.md` row: "ordinary C; no FAKE
needed"). PASS.

## T6 naming-announces-intent
Locals are `ret` (call return) and `fail` (failure result code) — both named
for their program-logic role and both genuinely read. No pad/dummy/spill/
unused naming. PASS.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C throughout; no
FAKE-class exception family is claimed or needed. (The mixed
`goto fail_store` / inline-`return` exit shape is the ordinary-C
mixed-exit-forms idiom, listed "ordinary C; no FAKE needed" in the
family-selection table; it is noted here for completeness, not claimed as an
exception.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
