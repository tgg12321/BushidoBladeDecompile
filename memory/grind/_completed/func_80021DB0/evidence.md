# func_80021DB0 — evidence (manual session 2026-09-23)

Stage spawn-point probe: tries 8 random compass directions (Judge sin/cos LUT,
radius 4000) from `pos` lifted 500 units, walks each ray in 40 steps with
func_8005344C, then drops a probe 4000 down and climbs in 100-unit steps
looking for a floor (normal.y == -0x1000). If it finds none, it falls back to
the nearest of the 4 per-stage start records (or the practice-lesson record
when D_800A38DC == 3).

Score trajectory (`sandbox --disable all`, target 285 insns):
- s1 natural draft (s32[3] arrays, per-element copies): 97
- struct Vec3 {x,y,z} locals + `*out = probe` struct copies: 21. The target's
  lw,lw,lw / sw,sw,sw groups are movstrsi block moves, and the 16-byte frame
  spacing (0x18/0x28/0x38/0x48) is BIGGEST_ALIGNMENT rounding of 12-byte BLKmode
  slots.
- declare `ofs` before `phase`: spill slots 0x70/0x78 are assigned in pseudo
  (declaration) order.
- the stage pointer is ONE s16* variable (slot 0x68), advanced in place:
  `stage += D_800A36A4*24` then `stage += j*6 + 3`. Score 8.
- `(dx * j) / 40` and `out->y + j * 100`: loop.c strength-reduces these givs,
  which gives the target's `move s0,s4` after `li s1,1` (not accumulator locals). Score 2.
- `for (i = 0, j = 0; ...)`: target zeroes i before j. Score **0** (285/285).
- A struct-pointer view of the stage records scored 0 too. The landed form
  uses plain `stage[i*6+3]` indexing instead, so there are no pointer casts.
full build SHA1 == oracle (verify-oracle --rebuild).
