# PutDispEnv — evidence (manual session 2026-09-23)

COMPLETED-C in de62daf26 (layer-2 cheat-reviewer PASS; oracle SHA1 match).

- Reference: Xeeynamo/psyz decomp/src/libgpu/sys.c `PutDispEnv` (local copy
  tmp/psyz-ref). Adopted near-verbatim; first measurement scored 28.
- The whole 28 came from binding `&g_gpu_disp_env` to a local pointer: GCC
  CSE'd the base into $s4 (extra save/restore, `lhu N(s4)` instead of
  per-access `%hi/%lo`). Addressing the global directly through a
  `(*(_dispenv *)&g_gpu_disp_env)` view (the reference's `info.disp` is a
  static global) scored 0.
- Residual sandbox hunks are masked reloc addends only (target relocs name
  split D_8009BEE8/…, ours g_gpu_disp_env+off; same address).
- `candidate.c` is the landed body, kept for provenance.
