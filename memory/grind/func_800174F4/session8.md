# Session 8 — layer-2 defect corrected

The first fresh layer-2 review FAILed body blob
`e2fbb78b67b14355615e6305f1b196dd791a3bdf`: although its function bytes
matched, `ings.c` under-declared `SetDefDrawEnv`, `PutDrawEnv`, and
`ClearOTagR` with `u8 *` parameters and therefore cast the real `DrawEnv` and
`u32 ot[2]` objects back to byte pointers. It also carried a no-op `u8 *` cast
at `DrawOTag`. This was the workflow's lying/under-typed-prototype defect.

The landing form moves `Rect`/`DrawEnv` before the declarations, gives the
four GPU APIs truthful object-pointer declarations, passes `&env` and `ot`
directly, and uses the meaningful `u32 *` conversion only at the byte-offset
`g_gpu_ot_ptr + 4` boundary. Re-measurement after that correction:

- `sandbox func_800174F4 --disable all --diff`: 0, 136/136, zero hunks.
- `verify-oracle --rebuild --allow-dirty`: SHA1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
- `git diff --check`: clean; `src/ings.c`: LF-only.

The correction is codegen-neutral but materially improves the object model.
The first re-review then caught the same-TU callers still passing raw `u8 *`
storage to the corrected declarations. Those sites now use explicit,
truthful `DrawEnv *` or `u32 *` conversions at their API boundaries.

Fresh layer-2 PASS:

- whole-file blob `cabf6782ee338bc84ce9f782dae729be889701c8`;
- function body blob `bd1bcff7c240e60ada38bbbca8d1a7a3ebf6d2e1`;
- independent sandbox 0/136, zero hunks, zero rules dropped;
- both Ruling 11 locals passed A-H; F7, `DrawEnv`, the signedness cast,
  target-function casts, and collateral boundary casts all passed.
