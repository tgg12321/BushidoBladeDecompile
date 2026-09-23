# func_800678A8 — evidence (manual session 2026-09-23)

- First fresh decomp from asm (no prior C; pre-include-asm-body.c was a placeholder).
- Chassis: `outer = D_800A34EC` with `p2 = outer+2`, `p6C = outer+0x6C` pointer locals
  (asm hoists `addiu t0,a2,2` / `addiu t1,a2,0x6C` to the prologue; spelling p6C's store as
  `*(s16*)(outer+0x6C)` scores 40).
- Score path: first candidate 9 -> `(x+1)*16` instead of `<<4` = 8 (lh vs lhu at the p6C store)
  -> direct `D_800A3438[arg1]` / `D_800F0B98[arg0]` array indexing (no cur/lim pointer locals) = 0.
  Pointer-local spellings (`&a[i]`, `a+i`, int-cast sums) all left the sym-vs-shift materialization
  order flipped (score 8/9/21).
- GTE islands: verbatim DMPSX v3 inline_c.h gte_SetRotMatrix (:297-310) and gte_ReadGeomScreen
  (:1236-1242), fetched from shdecompilations/silent-hill-decomp include/psyq/inline_c.h
  ($PSLibId: Run-time Library Release 4.3$). cc1 seats the "r" operand in $t2 in both, matching target.
- verify-oracle --rebuild: SHA1 62efab4f... match.

## Layer-2 review 2026-09-23 — FAIL (authorization only)
- The cheat-reviewer cleared every C construct, both islands (verbatim inline_c.h, "r" operand, clobbers),
  and the one-line src diff. It FAILED the commit because the inline_asm_canonical.txt row was a
  self-grant: `scan_hand_coded --single` gives tier LOW (2/8, S3+S4), there's no owner_cluster_grants.txt
  row, and there's no ruling. `audit_asm_cheats --check-new` also flags a missing evidence tag, same-commit
  self-auth, and a missing `Pure-C attempts:` block.
- Resubmit path: an owner ruling or registry row lands FIRST in a rules: commit (precedent 789ce34d7).
  Then the Match commit carries a row tagged `gcc-cannot-emit:gte_cop2_control_transfer` (the 2026-09-18
  ReadGeomScreen/SetBackColor form), a `Pure-C attempts:` block, and a clean re-run of
  audit_asm_cheats --check-new + sandbox + verify-oracle + region hashes.
- Non-blocking: u8 return with no return statement → the real prototype is probably void. Callers in
  text1b.c declare `u8 func_800678A8(s32, s32)` 8x and ignore the value.

## Owner ruling + second layer-2 review 2026-09-23
- Owner ruling landed 4494f53a6 (inline-asm-policy.md § Owner ruling 2026-09-23; four layer-2 rounds).
- Layer-2 on auth+body FAILed the body on three C points, all fixed:
  1. `((s16 *)&D_800A34E8)[arg0]` was a pun. The real object is the s16 pair at 0x800A34F0 (arg0 = 4/5 ->
     base folded to 0x800A34F0-8; func_80067D14.s forms the same folded base). Merged into
     `extern s16 D_800A34F0[2]` (include/game.h), read as `D_800A34F0[arg0 - 4]`; func_80061C00 now
     writes `[0]`/`[1]`; D_800A34F2 dropped from undefined_syms_auto.txt and sdata_syms.txt.
  2. `D_800F0C10` became a header-canonical `Unk800F0C10Record D_800F0C10[4][3]` (12-byte records,
     36-byte rows, 0x90 bytes up to D_800F0CA0); D_800F0C14/C18 rows carry alias suffixes
     (func_80067200.s still references them).
  3. Island comments now name the pinned release (Run-time Library Release 4.3).
- The reviewer also asked for a `void` return. Measured: void makes reorg fill the final `beqz` delay slot
  with `addiu v0,a0,1` (282/283, oracle RED e741e3d3). The target's unfilled slot means v0 is live-out,
  i.e. the original was declared non-void with no return statement. u8 is kept (the sibling family
  func_80067D14/func_80068D88 is u8). Oracle GREEN with u8.
