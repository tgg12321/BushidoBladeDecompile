# func_8002F770 self-vet

## Result

Author review: PASS. Fresh second layer-2 review of the exact corrected body:
PASS (2026-09-23), with no actionable findings.

## Object model and behavior

`angles` is an in/out array of three `s16` Euler angles. The function builds a
real PsyQ `MATRIX` in scratchpad at 0x1F800390, composes supplied Z/Y/X
arguments through RotMatrixZ/RotMatrixY/RotMatrixX and the input angles,
computes the inverse cofactors and determinant,
then converts the composed matrix back to Euler angles. The m00 local is the
matrix's real [0][0] value and is consumed three times.

## Match devices

- Two ordinary views of the same real scratchpad workspace: an arena base for
  identity initialization and a typed MATRIX for the cofactor reads. Their
  source lifetimes are separated by six rotation calls. `m00` is loaded with
  ordinary C.
- Two single-level `do { ... } while (0)` wrappers, both in the frozen allowed
  family and annotated at their sites with mechanism and individual ablation.
- Five canonical PsyQ GTE islands, character-identical to the reviewed sibling
  `func_8002F2D0` and covered by the owner-enumerated cop2 preamble cluster.

No volatile coercion, false prototype, fabricated object, dead store, unused
array, cross-symbol arithmetic, register pin, or output-rewriting rule exists.

## Measurements

- `sandbox func_8002F770 --disable all`: 0/298, build 298, target 298.
- Object-view ablation (one typed MATRIX view) -> 8; i2 wrapper -> 5;
  SetRot wrapper -> 8.
- Canonical authorization was corrected in commit `236424f69`; exactly five
  GTE region hashes remain. The rejected standalone `lh` is absent.
