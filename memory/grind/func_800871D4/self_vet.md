# SELF-VET — func_800871D4  (session 6, solver modality, 2026-08-26)

Diff = replace `INCLUDE_ASM("asm/funcs", func_800871D4);` in src/main.c with the
C body in memory/grind/func_800871D4/candidate.c. Nothing else changes: no
regfix.txt / asmfix.txt / .ld / Makefile / engine / tools / rules edits, zero
rules added, zero inline asm, zero `/* FAKE */`.

CONSTRUCTS: `s32 bitsUpper` / `s32 bitsLower` (per-half key-off bit masks, psyz's own locals); `u16 voice` (psyz's own declaration of `_svm_cur.voice`); `s32 idx` (`voice * 54`, the byte offset of this voice's `_svm_voice[]` slot) with `*(T *)((u8 *)&D_800Fxxxx + idx)` addressing of the splat per-word globals; `s32 okof1` / `s32 okof2` (the two UPDATED pending-key-off masks, each written once and read twice); unused `s32 mode` parameter (Sony's signature `_SsVmKeyOffNow(int mode)`).

## T1 semantic purpose: 
- `bitsUpper` / `bitsLower`: carry the voice's bit into the correct 16-bit half of
  the 24-voice key-off mask pair. Removing either changes the result. Real.
- `voice` as `u16`: it IS a `u16` — `D_8010280A` is declared `extern u16` and holds
  the current voice index. The declaration is a type fact, not a coercion; no mask,
  cast or width trick appears anywhere in the body. (The two
  `andi $v1,$a0,0xFFFF` in the target are GCC 2.7.2 PROMOTE_MODE zero-extensions
  of that ordinary `u16` local, confirmed against tmp/grind/func_800871D4/dumps in
  session 5; this session added nothing on that axis.)
- `idx`: the byte offset of this voice's slot; used by all three slot clears.
  Removing it means recomputing `voice * 54` three times. Real.
- `okof1` / `okof2`: the two updated key-off masks. Each is stored back to its
  global AND used to clear the same bits out of the matching key-on word. Both
  reads are visible in the target's bytes — after `or $3,$3,$5 ; sh $3,D_801078D8`
  the target does `nor $3,$0,$3`, reusing the register rather than re-loading
  D_801078D8. The local names a value the original code demonstrably held.
- `mode`: unused in Sony's source too; it is the function's ABI, not a construct.
- The one thing this session actually changed relative to the banked score-6 form
  is STATEMENT ORDER, not a construct: the two key-off commits are grouped
  together and the two key-on updates are grouped together. That is Sony's own
  order (see PRECEDENT below). Statement order is not a construct; there is no
  "simpler form" it is a decoration of.

## T2 human-programmer: Yes. Read the two pending key-off words, OR in this
voice's bit, release the voice's slot, commit the two key-off words, then clear
those bits out of the two key-on words. A reader asks "why is this here?" about
nothing in the body: every local is written once and read, every statement writes
state the routine exists to write. The shape is, statement for statement, the
shape of Sony's `_SsVmKeyOffNow`, with one departure (the slot clears sit between
the read and the commit rather than before both) that is itself ordinary: you read
the state you are about to update before you tear down the slot it describes.

## T3 GCC-internals justification: No — and this is the material difference from
the s5 form that layer-1 correctly FAILed. The candidate header does contain a
global.c allocno-priority analysis, but it is an EXPLANATION of why the faithful
form matches, produced after the fact by tools/ra_solver; it is not the reason any
construct is present, because the winning change introduced no construct. The
change is "write the write-back group in the reference source's order instead of
the order session 5 invented". If GCC's allocator did not exist the code would
still be written this way — it is what the cited source says. No local, mask, cast,
alias, wrapper, duplicate, barrier or annotation was added to move a register.

## T4 permuter/search provenance: No permuter was run this session. The candidate
was not produced by an auto-search: the solver TYPED the residual (`goal_from_tgt.py
classify` -> RA, pure `$a1<->$a2` rename) and inverse.py ranked the needed atom
(`live_shrink pseudo 73: 21->17`), and the form that supplies it was chosen by
going back to the cited Sony reference's statement order — i.e. by fidelity, then
verified by measurement. It does not pass detectors "only because they don't catch
this spelling"; there is nothing to catch.

## T5 family check: No forbidden family is present. Specifically, against this
function's own banned list: (1) no `& 0xFFFF` and no dual-use-of-a-raw-load appears
anywhere — the redundant-width-cast family (F2) is absent; (2) no in-tree
precedent is cited as evidence for anything; (3) the FOUR-local lockstep form
(keyoff_lo/keyoff_hi/keyon_lo/keyon_hi, each global RMW split into separate
load/compute/store phases with the two halves advanced in lockstep) is NOT present
and is not respelled here — this body performs no phase split at all: each key-off
word is read-and-combined in one statement and committed in one statement, and the
key-on words are plain compound assignments on the globals exactly as in the
reference. Three other forms measured 0 this session
(tmp/grind/func_800871D4/s4/{v1,v2,v6}.c) and were REJECTED BY ME, not adopted,
precisely because they do split the RMW into load/compute/store phases and are
therefore respellings of the banned construct.

## T6 naming-announces-intent: No `pad` / `dummy` / `unused` / `spill` / `tmp` /
`slack` names. `bitsLower`, `bitsUpper` and `voice` are psyz's own identifiers;
`okof1` / `okof2` name Sony's `_svm_okof1` / `_svm_okof2`; `idx` is a slot offset.
Every one of them is read after it is written.

SANCTIONED-FAMILY-CLAIMS: none — this body claims no coercion family. It is
ordinary C: five locals, all live, and the reference source's statement order.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
