# func_800693CC hypotheses

## Open

- The original source probably used a ten-word render context plus six scalar
  locals whose declaration order accounts for `sp+0x40` and `sp+0x48`, but the
  saved-mask store must remain live across the renderer calls without volatile
  or an out-of-bounds aggregate access.
- A canonical aggregate model for the table beginning at `D_8009BC04`
  (`available`, `visible`, then eight `{ action, mode }` byte pairs) may remove
  the final tail address hoist and reduce expansion-time temporaries.
- Revisit with a correctly imported permuter workspace whose baseline agrees
  with `engine sandbox`; the prior workspace's baseline was not comparable.

## Rejected

- Fifteen-word anonymous array as a final model: correct offsets, but 0x78 frame.
- Ten-word array plus standalone input/mask: 0x68 frame, wrong input offset, and
  dead mask store eliminated.
- Reusing unused parameters as general scratch carriers: worse score and no
  frame improvement; also lacks an independent semantic justification.
- Register storage class, inline loop helpers, direct expressions, and shared
  loop temporaries: measured, no frame improvement.
