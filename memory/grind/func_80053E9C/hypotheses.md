# func_80053E9C hypotheses

- KILLED: literal m2c output is the original source shape. It scores 223 and
  introduces an extra saved register/frame mismatch.
- KILLED: a conventional `s16 *` cursor with post-increment matches the packed
  walk. It folds several increments into offset loads and scores 235.
- KILLED: an integer byte cursor alone restores the explicit increments. It
  improves the floor only to 219; the fixed-point block remains structurally
  different.
- OPEN: recover the original object type for the work record and retest member
  access alias behavior across the E4/E8 normalization and E0 divisor stores.
- OPEN: inspect GCC RTL for the target-like spill mechanism before introducing
  any named intermediate; the target's sp+0x10 store/load must come from real
  source semantics, not fabricated stack storage.
- OPEN: separately derive the polygon-loop source form that emits `bgtz` to the
  shared reject block while retaining the bottom-tested decrement path.
