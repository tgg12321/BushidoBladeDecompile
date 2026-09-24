# func_8002F770 matching ledger

## Baseline

- Queue position: top active item on 2026-09-23.
- Canonical route: `ASM-PARTIAL`, 13/298 GTE/cop2 instructions.
- Honest baseline: score 298, no C body, 298 target instructions.

## Candidate 1

Reconstructed the matrix composition and matrix-to-Euler algorithm in ordinary C.
The latter is the same algorithm and object model as the matched sibling
`func_8002F2D0`; only canonical PsyQ GTE macro islands remain inline assembly.

Measurement: 107/298, 307 emitted instructions. The raw integer-address
spellings caused GCC to retain nine scratchpad pointers across the function,
inflating the frame from 0x40 to 0x58 and adding nine instructions. Replaced
them with one truthful aggregate symbol for the scratchpad MATRIX, so the
compiler can emit direct symbol-relative accesses as the target does.

## Candidate 2

Separated the symbol-backed identity stores from the numeric scratchpad pointer
passed to the rotation functions. Measurement: 31/298 with exactly 298 emitted
instructions: 18 displayed operand differences are the unresolved relocation
pair for each of nine symbol-relative stores; the 10 scored non-relocation
differences are two register-allocation swaps (`i2`/`ang_z` and `mat`/`ang_y`).

The matched sibling `func_8002F2D0` documents the same two allocation swaps and
closes each with one sanctioned `do { ... } while (0)` wrapper. Applied those
same measured devices here with function-specific annotations.

Measurement after both wrappers: 18/298, exactly 298 emitted instructions.
Every remaining difference is the unresolved object relocation on the nine
identity stores; all register allocation and scheduling matches.

## Candidate 3

Tested a canonical fixed-address store island to eliminate the relocations.
Scores moved 12 -> 6 -> 9 -> 2 as the zero operand and first store were made
ordinary C, but the volatile island necessarily kept the first call's `lui a1`
below the stores instead of at target slot 8. Rejected: it was less readable
than typed C and did not close.

Also tested explicit fixed-address loads for all nine matrix elements. That
transcribed the instruction order back into C/asm and scored 24/298. Rejected
under the workflow's transcribed-code rule.

## Rejected candidate after layer-2 review

A fixed-address `lh` island brought the typed-MATRIX candidate to 0/298, but
the fresh reviewer correctly rejected it: the load is ordinary C-expressible
GPR code and is outside the owner-granted cop2 addressing-preamble cluster.
Authorization was narrowed in commit `236424f69`; the load and its region hash
were removed. The reviewer also identified the real parameter object as an
array of three `s16` values and corrected the offset argument names/order.

## Final candidate

Expressed the first identity construction through its real scratchpad arena
base (`init_scr = 0x1F8002B8`, matrix at `+0xD8`), then opened the same arena as
`scr` after the six rotation calls for the later vector/matrix work. The
cofactor phase uses an ordinary typed `MATRIX *m`, including
`m00 = m->m[0][0]`. These are truthful views of the one transient scratchpad
workspace, with distinct source lifetimes; there is no volatile qualifier,
synthetic arithmetic, linker alias, or GPR assembly.

Measurement: **0/298**, 298 target instructions, 298 build instructions, zero
rules dropped. The five remaining GTE islands are source-hash identical to the
already reviewed `func_8002F2D0` islands.

## Ablations

Measured against the final body:

- One typed `MATRIX` view for both initialization and the later m00 read:
  8/298, 298 instructions. GCC retains the matrix base in `$s2` across the six
  rotation calls. The arena-base initialization plus later typed view is the
  honest object/lifetime spelling that removes that false live range.
- Remove the complete `i2` do-while wrapper: 5/298, 298 instructions.
- Remove the complete gte_SetRotMatrix do-while wrapper: 8/298,
  298 instructions.
- Final body with the two measured wrappers and scratchpad object views:
  0/298, 298 instructions.

No rejected `lh`, linker symbol, scratchpad asm store, or explicit nine-load
transcription remains in the final body.
