# func_8005FC9C — evidence (manual session 2026-09-22)

Fresh start (no prior ledger beyond the Campaign-4 placeholder). Closed in one
manual session: sandbox `--disable all` 267 → 88 → 62 → 26 → 18 → 0, full-build
SHA1 == oracle.

## Shape
Draws the double-buffer DR_AREA + a semi-transparent POLY_G4 "wipe" whose width
is driven by the frame counter D_800A3278 (off = (D_800A3278 - 0xB4) * 24), two
func_8007352C sprite blits per pass (records at D_8009B698, stride 12), two
passes (j), then SetDrawMode + AddPrim; bumps D_800A3278 while off <= 0x140.
Param block at sp+0x18 is the same layout as sibling func_800600C8's S60C8.

## Measured levers (score after each)
- first draft, separate `u8 *xy` pointer for the poly fields: 88
- POLY_G4 struct through ONE pointer (GCC's giv combine produces the s0=poly+0x1E
  base itself): 62
- `off` as s16 with **HImode-narrowed** arithmetic: s32 off + casts 26;
  s16 off with `s32 x = off + 0x140` 62 (x forces SImode sext, hoisted);
  **s16 x** 18. convert.c narrows `poly->x0 = 0x140 - off` to HImode, so no
  sext appears except at the multiply and the final compare — matches target.
- prologue `move s2,v0`: target reads the clip RECT through a temp then copies
  the pointer to a callee-save. `env` pointer for the reads + `clip = &env->clip`
  after them → 0. Without the clip local (env->clip. everywhere): 18. Assigning
  clip BEFORE the reads: 18.
- struct copy `r = *clip`: 71 (lwl/lwr block move) — target copies fieldwise.
- precomputing `0x140 - off` / `0x12C - off` into locals before the loop: 85
  (target's are loop.c hoists, not source locals).
