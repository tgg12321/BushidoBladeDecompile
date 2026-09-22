# CD_cw — hypotheses / frontier (2026-09-22, after manual s1)

## Frontier: an HONEST spelling of `Intr` that GCC does not fold to `sym+k`
Floor 38 (candidate.c, re-measured 2026-09-22). The forbidden cast view reaches 0 — a
byte-exact preimage, so everything except an HONEST spelling of the Intr/Alarm addressing
is solved. Next levers, in order:
1. How the matched siblings reach Intr on main: `CD_sync`/`CD_ready` use
   `idx_1494 = &g_cd_status_a; idx_1495 = 1 + idx_1494;` (u8 handles, FAKE-annotated).
   Try that exact shape inside the SOTN inline helpers (u8 handle hoisted in CD_cw and
   passed to/used by helpers), since a u8-typed handle cast-free is what M1 lacks.
2. `static inline` helpers taking the Intr pointer as a parameter (integrate.c copies a
   non-constant-foldable arg into a pseudo) — e.g. `callback(volatile CD_intr *)`.
3. Re-measure CD_sync / CD_ready / CD_datasync with the SOTN inline-helper + volatile-Alarm
   structure. They have the SAME printf block and needed five FAKE staging constructs; the
   Alarm-struct lever alone moved this function's printf block to exact. If it works there,
   those siblings could shed their FAKEs (cheat-cleanup), and the shared Intr spelling
   found for them transfers here.
4. Permuter on candidate.c restricted to the Intr access sites.

## Ruled out (do not re-run)
- 2-D table `D_800A12FC[2][64]` (works, but `[com + 0x40]` is the reference spelling and
  avoids the scorer's dlabel gap).
- Real-symbol struct with direct `Intr.x` access (62), SOTN `((Alarm_t *)&Alarm)->` (56),
  array decay (56), u8 array extern (34/56), member aliases (25/31).
## Open decision for landing
- candidate.c uses a K&R definition (`u8 com; void *param; void *result; s32 async;`) because
  the file's two prototypes say `CD_cw(s32, void *, void *, s32)`; `u8` promotes to match.
  At landing, prefer changing those prototypes to SOTN's `(u8, u8 *, u8 *, s32)` with an
  ANSI definition, and let the oracle prove the callers (CdControl/CdControlB/CD_init) are
  byte-neutral.
