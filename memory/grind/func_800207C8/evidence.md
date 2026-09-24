# func_800207C8 evidence

## 2026-09-24 Codex session

- Canonical route: `ASM-PARTIAL`; 44 of 317 instructions are COP2/GTE
  transfers or commands.
- Recovered a complete semantic body using only canonical SDK-style islands:
  22 bone transforms, the indexed attachment-vector set, the optional two-vector
  set, the root transform, two collision-height probes, and the two final
  `ratan2` angles.
- Corrected the three indexed data models during the experiment:
  `D_8008D864` is a byte array and `D_8008D86C`/`D_8008D88C` are pointer-word
  arrays.  These header edits were restored because the unmatched body did not
  land.
- First complete candidate: 325/317 instructions, score 171, 0x48 frame.
  Tightening all working pointers to phase-local scopes recovered the target's
  0x40 frame and seven saved registers: 320/317, score 99.
- Rewriting the vector destinations as paired base/end pointers regressed to
  325/317, score 133, and was rejected.
- Best residual is dominated by register allocation: the compiler swaps the
  long-lived player-data pointer and fourth output argument (`$s5/$s6`), then
  seats the first three transform loops' matrix/source/destination pointers in
  different argument registers.  Three extra instructions remain in the final
  root-copy/probe region.

Disposition: rotated with a fully recovered semantic draft and measured search
record.  The function remains ASM-PARTIAL; no noncanonical asm or register pins
were used or proposed.
