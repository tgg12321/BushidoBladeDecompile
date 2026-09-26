# func_800646E8 — hypotheses / ruled out

- RULED OUT: fresh tail cursor (rejected/fresh-tail-cursor-34.c) — 34.
- RULED OUT: zp walks the tail instead of zbuf (rejected/zp-tail-walk-35.c) — 35.
- RULED OUT: MATRIX view at base+4 for SetTransMatrix (rejected/matrix-view-trans-5.c) — 5.
- RULED OUT: size clamp through a local + if/else or ?: (rejected/local-if-else-size-2.c) — 2..6.
- RULED OUT: D_8009B8E8 read directly for the clut (a2: 498 insns, *frame reloaded).
- OPEN (policy): reviewer may probe the TexRec struct view, D_8009BD44[0], the
  D_800F0BCC scalar->array decl and the repeated product in the size clamp; the
  measured mechanisms are in evidence.md.
