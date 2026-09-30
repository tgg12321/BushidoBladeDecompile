# func_80048BA4 — evidence

## 2026-09-30 — ff-c, retro-audit fix-forward (Q37 class A; audit tmp/audit-2026-09-29/review/batch_00.md)

No ledger existed before this entry (landing 869284111, 2026-09-21).

### Audit finding

`s32 *vec; vec = &D_800FF56C; ApplyMatrix(*(s32 *)player, rotp, vec); vec[0] += ...;
D_800FF570 += ...; D_800FF574 += ...;` is an unannotated C-level pointer alias to a global,
load-bearing (removing it: 70). D_800FF558..D_800FF574 is one MATRIX split into 12 scalars
(split-scalars-hide-aggregate). Lying prototypes (`ApplyMatrix(s32, s16 *, s32 *)`,
`math_RotMatrixZYX(s16 *, s16 *)`, `gte_MulMatrix0ClearTrans(s32, s16 *, s16 *)`). `rotp` inert.

### Object-model evidence: 0x800FF558 is one PsyQ MATRIX (`s16 m[3][3]; u16 pad; s32 t[3];`, 0x20 bytes)

- func_8004A940 (asm/funcs/func_8004A940.s:602-603, 672-678, INCLUDE_ASM) materializes the
  single base `%hi/%lo(D_800FF558)` and (a) reads `0x14/0x18/0x1C($v0)` = t[0..2], (b) passes the
  base as arg 1 of gte_MulMatrix0ClearTrans (a MATRIX *), (c) reads words `0x0..0x10($v0)` and
  `ctc2`s them to GTE control regs 0..4 (the SetRotMatrix sequence) — the rotation part of a
  MATRIX. It never names D_800FF55A..D_800FF574.
- func_80048BA4 writes the nine s16 at +0..+0x10 (a transposed copy of a local MATRIX) and passes
  +0x14 as the VECTOR *out of PsyQ ApplyMatrix, then adds a MATRIX's t[k] to each of +0x14/+0x18/
  +0x1C.
- Consumers (engine dossier data model + grep of src/, include/, asm/): only func_80048BA4 (C) and
  func_8004A940 (asm). The 12 names are undefined_syms_auto.txt rows (no .data/.bss dlabel to
  move); after the merge C declares only `extern MATRIX D_800FF558;`; the per-word linker rows stay
  (asm-only, not C handles).
- `player` (game_GetPlayerData result) is an array of MATRIX pointers: element 0 is ApplyMatrix's
  and gte_MulMatrix0ClearTrans's MATRIX * argument; elements 0/1/2 are read at +0x14/+0x18/+0x1C
  (t[0]/t[1]/t[2]); elements [index], [18] (+0x48), [19] (+0x4C) are copied 0x20 bytes =
  sizeof(MATRIX) into the packet at +0x18. The landing's 8-word `_struct_copy_func48BA4` was that
  MATRIX copy.
- Prototypes: ApplyMatrix is the verbatim-linked PsyQ LIBGTE MTX_05 (src/display.c:2123);
  libgte.h: `extern VECTOR* ApplyMatrix(MATRIX* m, SVECTOR* v0, VECTOR* v1);`
  (tmp/croc-ref/include/psyq/libgte.h:296). math_RotMatrixZYX(SVECTOR *, MATRIX *) and
  gte_MulMatrix0ClearTrans(MATRIX *, MATRIX *, MATRIX *) typed from their use here.

### Spellings measured (tmp/ffc/score_full.py = engine sandbox_score, --disable all, strip
cheat-asm, full-file substitution because the forms change file-scope declarations; diffs vs main
in spellings/)

| spelling | func_80048BA4 | func_80049718 |
|---|---|---|
| v0 control (main verbatim) | 0 (237/237) | 0 (197/197) |
| v1 scalars, no alias, no rotp | 70 (236/237) | — |
| v2 `extern MATRIX D_800FF558`, honest prototypes, no alias | 0 | 0 |
| v2b same, `(VECTOR *)&D_800FF558.t` | 0 | — |
| v2c same, player raw offsets kept | 0 | — |
| v3 MATRIX + `s32 *t = D_800FF558.t` alias | 0 | — |
| v4 v2 + `MATRIX **player`, MATRIX copies (typedef dropped) | 0 | 0 |
| **v5 v4 + libgte.h `VECTOR *` return, func_80049718 redecl kept** | **0 (237/237)** | **0 (197/197)** |

Reading: the alias was load-bearing only against the split-scalar object model. With one MATRIX
object, `D_800FF558.t` (= sym+0x14) is the same constant the ApplyMatrix argument materializes,
so CSE reuses the callee-saved register holding it for the t[0] read-modify-write — the target's
`lw/sw 0($s0)` — with no second C handle. Plain C, no FAKE needed; v3 (alias) is not chosen.

Chosen: v5. Side effect outside this function: func_80049718's one ApplyMatrix call now casts its
untyped buffers to the libgte.h parameter types (`(MATRIX *)(... + 0x18)`, `(SVECTOR *)sp10`,
`(VECTOR *)(obj + 0x2C)`) — forced by the corrected shared prototype; its score stays 0.

### Applied to src/text1b.c (uncommitted, awaiting fresh layer-2), 2026-09-30

Under the landing lock (ff-c) on HEAD a9381e69b: `lock.ps1 rebuild` = verify-oracle
build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches, artifact_matches true);
tools/check_completion_integrity.py OK; `engine.cli sandbox --disable all` against the applied
src: func_80048BA4 0 (237/237), func_80049718 0 (197/197). `layer2 hash func_80048BA4` =
2ce3607ca024d608 (body_kind c).

### Layer-2 round 1 (rev-48ba4): FAIL, approach upheld — round-2 fixes (2026-09-30)

Upheld: prong (a) from func_8004A940.s, the prototypes, `(VECTOR *)D_800FF558.t`. Fixed:
1. Prong (c): undefined_syms_auto.txt rows D_800FF55A..D_800FF574 (11) deleted; D_800FF558 kept. Also the
   ten per-word census rows in named_syms.txt (g_camera_view_state_plus_2..16, g_text1b_addr_800FF570/574;
   the base row g_camera_view_state stays). Remaining references: only asm/funcs/func_80048BA4.s (not
   built; the C body is) and the unbuilt monolith asm/text1b.s.
2. Prong (d): `extern struct MATRIX D_800FF558;` in include/game.h with the evidence comment (struct tag,
   so game.h needs no gte.h: text1b_b.c / text1b_tu1c.c / code6cac.c / code6cac_tu2.c include game.h and
   define their own GTE typedefs, which a gte.h include in game.h would collide with). text1b.c now
   `#include "gte.h"`; its local SVECTOR/MATRIX/VECTOR typedefs are gone.
3. Judge: `extern s16 Judge[];` once at the top of text1b.c (as ings.c / sound.c / text1a_c.c declare it;
   no shared header declares Judge, and code6cac*.c still declare the scalar form, so a header declaration
   would collide); every `(&Judge)[i]` / `&Judge + i` / `*(&Judge + i)` spelled `Judge[i]` / `&Judge[i]`.
   Users: func_80048BA4, func_80056CB8, func_800571C0, func_80057CC8 — all sandbox 0.
4-8. func_80049718: reopened (Q37), see memory/grind/func_80049718/evidence.md.
Applied under the lock: rebuild build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches,
artifact_matches). check_completion_integrity flags only func_80049718 (INCLUDE_ASM, not yet in queue)
until the `queue reopen` that follows the commit.

### Layer-2 round 2 (rev-48ba4): PASS on the tmp/ffc/backup_r2 bytes; landed

Recorded PASS: func_80048BA4 15c9297d74261ae9, func_80056CB8 321ac34da73efdfe, func_800571C0
a35c351a3f8051ca, func_80057CC8 1c688bdfc67df599 (scope cheat-cleanup). Re-applied byte-identical and
rebuilt under the lock: build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa.
Non-blocking follow-ups from the reviewer: named_syms.txt's g_camera_view_state duplicates the
D_800FF558 base linker name; 36 scalar `&Judge` uses remain in code6cac_b.c / code6cac_b_tu2.c /
code6cac_tu2.c (those TUs still declare `extern s16 Judge;`).
