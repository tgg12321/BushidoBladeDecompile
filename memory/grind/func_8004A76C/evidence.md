# Evidence bank — func_8004A76C

## s1 (recon) — canonical gate verdict + OBJECT MODEL + prior audit

- `canonical func_8004A76C` returns **ASM-WHOLE**, reason "whole-function canonical-asm
  grant in inline_asm_canonical.txt". `sandbox func_8004A76C --disable all` this session:
  `{"score": 37, "no_c_body": true, "build_insns": 0}` — src/main/3AB48.c:30 is still a bare
  `INCLUDE_ASM("asm/funcs", func_8004A76C);` stub, so `--diff` prints "no diff: function is
  still whole-body INCLUDE_ASM — there is no C to compare" (no source-level/operand-only
  hunks exist to classify; there is no C body at all).
- `scan_hand_coded.py --single func_8004A76C`: **tier=STRONG, score=1/8**, driven by S7
  (callee-saved register used without save: unsaved `$s0`) — "GCC-impossible, decisive".
- `asm/funcs/func_8004A76C.s` (39 insns) is a 2-vertex-batch GTE perspective-transform loop:
  `lwc2`/lane loads into GTE data regs 0-5, `rtpt` (perspective-transform-triple), `mfc2`
  `$17`/`$18`/`$19` (GTE result SZ/flag-ish regs, flagged "handwritten instruction" by the
  assembler), `swc2` writes, and `sh` stores of the transformed `(x,y)` pair into the output
  array — classic PS1 3D pipeline code. GTE (cop2) ops have no C form at all (AGENTS.md,
  `inline-asm-policy`), so the body cannot be anything but hand/inline asm regardless of the
  S7 signal.
- **Already decisively audited.** `docs/audits/INLINE-ASM-AUDIT-2026-10-01.md` item A6 raised
  exactly the live question (whether a GCC file-scope global-register-variable
  `register T x asm("$16")` could produce the unsaved-$s0 shape from C) and closed it
  **"CLOSED, no change"**: (a) the only caller, `func_8004A4E0`, is itself whole-body hand
  asm (its own canonical-2026-09-29 audit: custom callee-saved ABI handing `$s0` live across
  `jal func_8004A76C`/`func_8004A808`/`func_8004A940`, which GCC cannot emit — `$s0` is
  advanced by this function at 0x8004A798 and read back by the caller after return at
  0x8004A5C0, something GCC assumes a callee-save cannot do); (b) the loop body is itself
  hand software-pipelined — `rtpt` for iteration N+1 is issued (0x8004A79C/0x8004A7D4) before
  iteration N's `sh` stores retire (0x8004A7A4-0x8004A7AC), and the `j` entry point
  (0x8004A794) jumps into the SECOND half of the unrolled loop to prime the pipeline — a GTE
  latency-hiding shape no GCC scheduler produces; (c) a global-register-variable is itself a
  register pin, independently banned by policy regardless of whether it would reproduce the
  bytes.
- OBJECT MODEL: no DATA MODEL section was dispatched for this function (no flagged globals
  in the brief). Independently confirmed by reading the body: the function touches only its
  four register-convention parameters ($a0 count, $s0 input vec ptr, $a1/$a2 output ptrs) and
  GTE coprocessor data/result registers — no global symbol, no `lw`/`sw` to any `D_8*`/`g_*`
  address anywhere in the 39-insn body. MATCHES n/a — no global object model applies.
- Sibling `func_8004A4E0` (23 sessions, floor flat at 163, rotated) is the caller of this
  function and shares the same ABI cluster; its ledger's s18/s18b Judge FAIL (2026-10-15
  16:50, docs/grind/decisions.md:1320) ruled that non-scanner GCC-impossible evidence cannot
  stand in for the `scan_hand_coded` STRONG bar for *that* function (func_8004A4E0 itself
  scans LOW 0/8, because it is the caller and saves everything). **func_8004A76C does not
  have that problem**: it scans STRONG 1/8 directly (S7), independent of A4E0's unresolved
  question, and additionally has the closed A6 audit specifically naming it.

- [s1] canonical func_8004A76C = ASM-WHOLE (whole-function canonical-asm grant in inline_asm_canonical.txt)

- [s1] sandbox func_8004A76C --disable all this session = {"score":37,"no_c_body":true,"build_insns":0}; --diff: 'no diff: function is still whole-body INCLUDE_ASM' (no hunks to classify, no C body at all)

- [s1] scan_hand_coded.py --single func_8004A76C = tier=STRONG score=1/8, driven by S7 (unsaved callee-saved $s0) — 'GCC-impossible, decisive'

- [s1] asm/funcs/func_8004A76C.s is a GTE (cop2) perspective-transform loop: lwc2 vertex loads, rtpt, mfc2 of GTE result regs ($17/$18/$19, flagged 'handwritten instruction' by the assembler), swc2/sh stores; GTE ops have no C form at all (AGENTS.md inline-asm-policy), independent of the S7 signal

- [s1] docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 already closed the exact open question this function's reopen asks ('decide afresh: hand-written asm vs compiled C'), with the global-register-variable alternative ruled out for the three reasons in H1

- [s1] OBJECT MODEL: no DATA MODEL section was dispatched for this function; independently confirmed by reading the 39-insn body that it touches only register-convention parameters and GTE coprocessor registers, no global symbol — MATCHES n/a, no global object model applies

- [s1] sibling func_8004A4E0 (the sole caller, 23 sessions, floor flat 163, rotated) scans LOW 0/8 and was Judge-FAILed on exactly the scan_hand_coded-STRONG bar (docs/grind/decisions.md:1320); func_8004A76C does not share that problem because it scans STRONG 1/8 directly and has its own closed A6 audit

## s2 (structural) — no C body to apply structural levers to; cluster scan completed

- Re-measured this session: `canonical func_8004A76C` still returns **ASM-WHOLE** (same
  reason, "whole-function canonical-asm grant in inline_asm_canonical.txt"); `sandbox
  func_8004A76C --disable all` still `{"score":37,"no_c_body":true,"build_insns":0}` —
  src/main/3AB48.c:30 is unchanged, still a bare `INCLUDE_ASM` stub. Nothing moved since s1.
- Structural-modality levers (block-local var splits, declaration order, type narrowing,
  statement re-association) operate on a C body under a sandbox-measured distance. This
  function has none — the canonical gate classifies the whole function as hand asm, so there
  is no `--diff` to classify into source-level/operand-only/not-scored hunks (confirmed again
  this session: `--diff` still prints "no diff: function is still whole-body INCLUDE_ASM").
  Every structural technique is inapplicable by construction, not merely unproductive.
- Completed the s1 frontier's cheap cluster-scan probe (item 2): ran
  `scan_hand_coded.py --single` on both remaining cluster siblings.
  - `func_8004A808`: **tier=STRONG, score=3/8** (S3 no-spills, S4 front-loads, S7 unsaved
    `$s0`) — "GCC-impossible, decisive" on S7 alone, same as func_8004A76C's own S7 hit.
  - `func_8004A940`: **tier=STRONG, score=3/8** (same S3/S4/S7 triplet; S7 fires on EIGHT
    unsaved callee-saves: `$s0`-`$s7`, the maximal shape).
  - All three jal-cluster siblings named in func_8004A4E0's custom-ABI evidence
    (func_8004A76C, func_8004A808, func_8004A940) now have an independently-measured
    scan_hand_coded STRONG verdict on file. Only the cluster's caller, func_8004A4E0 itself,
    scans LOW (0/8) — which is exactly the asymmetry its own s18/s18b Judge FAIL already
    turned on (docs/grind/decisions.md:1320): the caller saves everything it uses, the three
    callees don't, because the callees are the ones directly leaking unsaved `$sN` state back
    across the `jal` boundary.

- [s2] canonical func_8004A76C = ASM-WHOLE, unchanged since s1 (whole-function canonical-asm grant in inline_asm_canonical.txt)

- [s2] sandbox func_8004A76C --disable all this session = {"score":37,"no_c_body":true,"build_insns":0}; --diff reports no C body exists to diff

- [s2] scan_hand_coded.py --single func_8004A808 = tier=STRONG score=3/8 (S3+S4+S7, unsaved $s0)

- [s2] scan_hand_coded.py --single func_8004A940 = tier=STRONG score=3/8 (S3+S4+S7, unsaved $s0-$s7, all eight)

- [s2] All three func_8004A4E0-called cluster siblings (func_8004A76C, func_8004A808, func_8004A940) now carry independently-measured scan_hand_coded STRONG verdicts; only the caller func_8004A4E0 scans LOW

- [s3] sandbox func_8004A76C --disable all --diff this session: floor unchanged at 37, no_c_body=true, build_insns=0 (tmp/grind/func_8004A76C/s3/sandbox_diff.txt).

- [s3] asm/funcs/func_8004A76C.s (44 lines) is a 39-insn hand software-pipelined GTE perspective-transform loop using rtpt (cop2) with lwc2/swc2 vertex I/O and a pipelined jump-into-loop-body entry (.L8004A7B0) — consistent with s1's prior read of this file.

- [s3] tools/decomp-permuter/import.py src/main/3AB48.c func_8004A76C run directly (WSL venv) this session: 'Failed to find GLOBAL_ASM for function func_8004A76C' — confirms by direct tool attempt (not citation) that no permuter workspace can be built while this function has zero C bytes.

- [s3] This matches and reinforces s1 (recon, GTE loop / A6 audit closed) and s2 (structural modality inapplicable, no C body) — three independent modalities (recon, structural, permuter) have now each independently hit the same root fact: there is no C source for func_8004A76C to work on.
