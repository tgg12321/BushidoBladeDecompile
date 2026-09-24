# func_800693CC evidence

## 2026-09-24 Codex session

- Canonical route: `C`; target length: 307 instructions.
- Recovered behavior: menu-selection input handler.  It merges the two controller
  halves, decodes directional/action input, cycles through the eight-entry
  availability mask at `D_8009BC04`, updates the selected entry's mode bit,
  updates the visibility mask, renders the menu, and returns the accepted entry
  or `-1`/`-2` for the two cancellation paths.
- Best honest candidate: score 13, 307/307 instructions.  Apart from relocation
  cascades, the residual consisted of a 0x78 generated frame versus the target's
  0x60 frame and one tail address hoist.  The entire functional body through the
  four render calls and both action paths otherwise aligned.
- Target stack facts: context base is `sp+0x10`; the decoder word is at `sp+0x40`;
  the saved visibility word is at `sp+0x48`; saves are at `sp+0x50..0x58`.
  `func_8006E390` initializes ten words from the context base.
- Measured variants (sandbox score): natural structured C 104; m2c-shaped 83;
  control-flow/goto shape 41; unsigned availability mask 27; corrected action
  control and stack offsets 17; best loop spelling 13; ten-word context plus
  separate input/mask locals 16-30 depending on loop spelling (0x68 frame);
  parameter-carrier experiments 53; direct-expression form 37; shared loop
  temporaries 27/30; `static __inline__` loop extraction 13 (frame unchanged).
- A 15-word aggregate preserves both required stack stores but GCC 2.7.2 retains
  24 bytes of additional phantom local allocation for this CFG.  A ten-word
  context reduces the frame to 0x68 but places the decoder word at `sp+0x38` and
  optimizes away the otherwise-dead `sp+0x48` store.  Shrinking the aggregate and
  indexing beyond it would be undefined C; fake padding, volatile steering,
  dead parameter assignments, and register pins were rejected under
  `docs/DECOMP_WORKFLOW.md`.
- The old decomp-permuter import produced a scorer baseline of 789 rather than
  the engine's baseline and was not used as evidence for a match.

Disposition: rotated with this record after semantic recovery and measured lever
exhaustion.  This remains a pure-C problem; it is not proposed as a new park or
infrastructure category.
