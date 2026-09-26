# func_80032C50 — evidence (manual session 2026-09-25)

Sound-cue dispatcher: `kind` (0..72) selects a cue; cases 7..39 play on the
object itself (`func_800325E0(base + K, obj + 0xF4)`), cases 40..72 are the
same 33-entry pattern for the opponent `*(obj + 0)` (`base2`, `+0xF4` of the
opponent). Cases 0/1 are stage-11/14 positional cues through func_80061A3C /
func_80032854; 2..4 call func_80032854(…, 10, obj + 0x180/0x18C/0x174, 0).
The `base`/`base2` prologue is the same D_800A38DC==3 / player-1 bank-select
test func_80032854 opens with (different kind ranges).

## Measured trajectory (sandbox --disable all, --candidate)
- first full draft (u8 *obj local cast from s32 param, row/row2/tri/tri2 order): 23
- locals computed in target order row, tri, row2, tri2: 4 (only the
  `move t0,a1` prologue slot left)
- s32 `obj` parameter used directly (no u8* local copy): 2 — the one scored
  hunk is `lw v0,%lo(jtbl)(at)`, an artifact of the sandbox build still
  carrying `INCLUDE_RODATA(jtbl_80010698)` next to the compiler's own table.
- landed (INCLUDE_RODATA removed): full-build SHA1 == oracle.

## Landing chassis (same commit)
- Rodata: the compiler-emitted table replaces the jtbl_80010698 placeholder.
  The placeholder's trailing zero word was the `.align 3` pad in front of
  func_80033498's table (RODATA_ALIGN2_FILES rewrites `.align 3`→`.align 2`
  for code6cac_b, so the pad has to be explicit); it becomes
  `const u32 D_800107BC[1]` after the function — the pattern of D_800100E0
  (func_8001C8DC, 025f88d91) and D_80010428 (func_80021424, 23152ce9e).
  Without it the build is 4 bytes short and every later text address shifts.
- Data model (dossier INDEXED-ACCESS signals): D_8008EBCC, D_8008EBE0 → `s32 []`,
  D_8008E5A8 → `u8 []`; code6cac_b.c's `extern s32 D_800A384C` → `u8` (code6cac.c
  already declares it u8; target reads/writes it with lbu/sb). Existing users
  re-spelled from `*(&X + i)` / `*(u8 *)&X` puns to plain subscripts (bytes
  unchanged — oracle).
- `extern void func_80061A3C(s32 *, s16, s32, s32);` (matches text1b.c's
  definition; was implicitly declared).
