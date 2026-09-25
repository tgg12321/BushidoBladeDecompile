# func_80035828 — evidence (manual lane, 2026-09-25)

## Shape
Per-frame state machine of the practice/lesson front-end: sets vsync target
(D_800A36F1) to 1, rand() tick, promotes state 1 -> 7 on D_800A31D9, handles a
pending D_800A31DA voice cue, then `switch (D_800A3740)` over 1..14 (jtbl_800108EC,
14 entries, index = state-1). State 2 dispatches the result of func_80077B30 through
a second switch (jtbl_80010924, 5 entries for -1..3; case 0 -> outer end).
Tail: `if (D_800A3834 != 9) D_800A36F1 = 2`.

## Measurements
- First full-body candidate: sandbox 16 — sole source-level hunk was the inner
  switch compiled as a compare chain (4 case values < MIPS case_values_threshold 5).
- Adding the explicit `case 0: break;` (target table entry [1] = outer end label
  0x80035D9C) => 5 case values => ADDR_VEC emitted. Sandbox 4, 0 source-level,
  1 operand-only (`lw v0,56(at)` vs `lw v0,0(at)` = the second table's offset in
  OUR .rodata vs the target's external jtbl_80010924), rest not-scored.
- Residual 4 = the two `lui at,%hi(jtbl)` / `lw %lo(jtbl)(at)` pairs: the reference
  names external jtbl_* symbols, ours relocs against own .rodata — the same scorer
  artifact documented in the func_8006B578 Match commit (2a5db4c32). Not a codegen gap.

## Landing needs rodata placement (func_8006B578 / replay_camera precedent)
jtbl_800108EC and jtbl_80010924 are hand-transcribed arrays in
src/code6cac_b_rodata_post.c; they directly follow replay_camera_rob_back_loose2's
compiler-emitted jtbl_800108CC (0x800108CC+0x20 = 0x800108EC), and
code6cac_b2_post.o currently emits NO .rodata. The text order is
code6cac_b2_pre -> replay_camera_rob_back_loose2 -> code6cac_b2_post (one original
TU split in Phase B §15.1), so the rodata order jtbl_800108CC, jtbl_800108EC,
jtbl_80010924 is that TU's switch tables in text order. Move
code6cac_b2_post.o(.rodata) to directly after replay_camera_rob_back_loose2.o(.rodata)
and delete the two arrays; code6cac_b_rodata_post then starts at jtbl_80010938
(0x80010938, 8-aligned).

## Outcome (2026-09-25)
COMPLETED-C in 6a1eb023a (layer-2 cheat-reviewer PASS; oracle SHA1 match; sandbox 0 on the landed body with the rodata move). Queue done in 03242d5d0.
