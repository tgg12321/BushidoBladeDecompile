# func_8005490C — evidence (manual session 2026-09-25)

Start: honest floor 399/399 (INCLUDE_ASM, no prior C; pre-include-asm-body.c is a placeholder).
End of session (2026-09-25):
- **Admissible floor 47 (397/399).** `candidate.c` is single-role C throughout: per-block rotation
  temps, per-site player locals. It is built against the reshaped header landed in a9c634304.
- **0 exists, but layer-2 FAILed it.** rejected/vz-obj-multiwrite-0.c is 0 but needs the two reuse
  carriers `vz` and `obj`. The verdict and reasoning are in rejected/vz-obj-multiwrite-0.md.
- Byte-neutral prep landed in a9c634304: the ctrl-block arrays, g_anim_func_table[], and the VECTOR
  typedef move.

## Object-model findings (from the original binary, not from byte-chasing)

- **0x84-byte pose record at sp+0x28.** func_800198D0 copies 0x21 words into its third argument
  (asm/funcs/func_800198D0.s loops `sltiu 0x21`); func_80023F08 passes two of them at sp+0x18 and
  sp+0x9C (0x84 apart). Declaring the local as that record fixed the whole 0x80-byte frame gap
  (frame 0x90 -> 0x110), 84 -> 61.
- **Ctrl block arrays.** The target walks 0x34/0x38 with one pointer (`lw 0x34($s0)`, `s0 += 4`,
  i < 2), 0x34/0x3C with `s4 += 4`, 0x44/0x48 with `s2 += 2`: unk34[2], unk3C[2], unk44[2],
  unk48[2]. 0x24/0x26/0x28 are halfword stores of the negated camera rotation: s16 unk24[4]
  (func_8005507C returns its address; the other func_80061064 caller passes an s16 rotation as
  `(s32 *)`, so the cast there is consistent).
- **g_anim_func_table is a table.** Target forms `&D_800F66A0` once in $s1 and reloads `lw 0($s1)`
  before each jalr. The TU's scalar declaration `extern s32 (*g_anim_func_table)(s16 *, s16 *)`
  scores 66; the array-of-function-pointer declaration scores 0. func_80049718's own call is
  byte-identical under the array declaration (tmp/490c/tucmp.sh: SAME).

## Measured ladder (sandbox --disable all, candidate file in tmp/490c/)

| form | score |
|---|---|
| first draft (Vec3S 8-byte pose) | 100 (401 insns) |
| work.t[0] stored first (s4 base = +0x4C) + `ang` local (lh, not lhu) | 84 |
| 0x84-byte pose record | 61 |
| function-scope x/z/c/sn temps, d-style z reuse | 42 |
| block-scoped rotation temps (all four inner spellings identical) | 49 |
| block-scoped temps + `vec.vy += unk10` after the rotation | 38 |
| + hoisted `ty = s->unk10` (load-before-store) | 36 |
| **function-scope `vz` shared by both rotations** (all else block-scoped) | **0** |
| same, but `obj` split into per-site locals | 9 (397 insns) |
| same, but direct `D_800EFAE8.field` everywhere (no pointer local) | 141 (420 insns) |
| same, scalar g_anim_func_table | 66 |

## Mechanism (dump-proven; tmp/490c/pd/ instrumented cc1, BB2_PRIO_DEBUG)

- **vz:** in the target, `subu v1,t1,t2; sra t0,v1,12` — the subtract and the shift are NOT tied.
  local-alloc.c:469-476 only gives a quantity to a pseudo with reg_basic_block >= 0 and
  reg_n_deaths == 1; combine_regs (local-alloc.c:1829) refuses to tie when the SET reg has
  reg_qty == -1. A per-block vz is local, gets tied to the subtract (a0/v1), and the shift result
  then lands in the wrong seat. sched2 then orders the y-translation chain after the nz store
  (anti-dependence on the product register), which is the 36-38 residual. A vz referenced in both
  blocks is global, global-alloc seats it in t0 (c's register, dead after the last mult), the
  anti-dependence on the last `mult` lifts nz's priority, and sched2 reproduces the target order.
  No single-write spelling can make the pseudo non-local: it must be referenced in two blocks or
  set twice.
- **obj:** the target copies each func_8004153C(0/1) result into $s0 (`move s0,v0; beqz s0`).
  An init-only variable does not cross a call, so global-alloc gives it v0 and the two moves
  vanish (397 insns). It lands in $s0 only when it is the same pseudo as the loop's
  `obj = func_8004153C(i)`, which lives across calls.
- **s pointer:** the target keeps &D_800EFAE8 in $s3 for the whole body but addresses
  0x2C..0x38 in the init through absolute %hi/%lo (D_800EFB14..20), exactly the mixed shape of
  the landed sibling func_80054604 (pointer-alias family, FAKE-annotated there).

## Construct status (for review)

- `vz` and `obj` are each written more than once. `vz`: two textually identical writes, one per
  rotation block, each read once by `vec.vz = vz;` (Ruling 5 shape, but no selector — the
  2026-09-23 extension's (B) does not fit). `obj`: three writes (init player 0, player 1, loop
  player i); consumers differ (call arg vs field read) — fails Ruling 5 1(a) and Ruling 6 (A)
  on the letter. Needs a layer-2 ruling.

## [s2] 2026-10-01 laneB — post-Q65 re-baseline, data model, Ruling 11 package (landing body = candidate.c)

Unparked f78231287. All scores: scratch TU copies of src/text1b.c + src/code6cac_c2.c built with the build
pipeline (r11/scripts/mk.py, xbuild.py, xbm.sh); every other function in both TUs byte-identical unless noted.
- **Rebase.** The 2026-09-25 0-body, rebased (Judge[], D_800A3250[0], D_800F62E0[n], math_* names,
  MotionFrame), is still 0/399 under the Q65 per-file gp model (tmp sandbox and scratch TU).
- **Ctrl block holds integers (datamodel/ptr_typing.txt).** unk2C/unk30/unk34[]/unk3C[] typed `u8 *` (with
  a6/a7/magic retyped) breaks func_80054FDC (6: all six relocation `addu a0,x` become `addu x,a0`) and
  func_80054604 (2: `addu v0,v0,v1` -> `v1,v0`); C pointer arithmetic puts the pointer operand first. The
  original held these addresses as integers, so the members stay s32 and this body converts at its uses
  (`*(s32 *)(p + 0xC)`, `*(s32 *)s->unk34[j]`, `(u32 *)s->unk34[j]`, `(u32 *)s->unk3C[i]`).
- **Minimal (A), byte-neutral (r11/scripts/mk.py default):** text1b.c `extern void func_8003FFC4(s32)` ->
  `(s32 *)` (its definition, src/code6cac_c2.c `void func_8003FFC4(s32 *a0)`), and func_80054604's `v` ->
  `s32 *v` with its two `(s32)func_8004153C(...)` casts dropped. text1b 263 funcs / code6cac_c2 75 funcs: 0 differ.
- **Motion frame.** The frame is read lh three times (0x80054D10 +0, 0x80054D28 +2, 0x80054D2C +4) and passed
  to func_80040D48's `s16 *` parameter; MotionFrame (include/code6cac.h) has u16 unk_02/unk_04 because
  func_80023F08 reads them lhu. Measured: `s16 frame[0x42]` 0 (datamodel/frame-s16-array-0.c, landing form);
  MotionFrame + `(s16)` casts 0 (motionframe-s16-casts-0.c); MotionFrame + `s16 ang`/`s16 dist` locals 3
  (lhu + sll/sra; motionframe-s16-locals-3.c); MotionFrame, s32 ang = unk_02 1 (lhu; motionframe-ang-u16-1.c).
- **Pointer alias `s` (FAKE, pointer-alias family).** Using `s->` for every member (no mixed
  D_800EFAE8.field handles) is 0 (r11/bodies/alias_s_everywhere.c); the direct-global form is 161, 420 insns
  (rejected/direct-global-no-pointer-local-161.c). cse folds the known base in the first-frame block, which
  gives the target's absolute %hi/%lo accesses there.
- **Loop counters.** The first-frame loop has its own block-scoped `j`; sharing the player loop's `i` is
  byte-identical (r11/bodies/loopcounter_shared_i.c, 0), so no counter is multi-write.
- **Casts left in the body** (all interface or integer-address conversions): the integer-held ctrl-block
  addresses above; `g_anim_func_table[0]` called through `(void (*)(SVECTOR *, MATRIX *))` (the table is
  `extern s32 g_anim_func_table[]`, as in the 6 landed consumers; retype question sent to the orchestrator);
  `(SVECTOR *)` / `(MATRIX *)` on D_80101DF0's local-tag Rot/Mat (same layout, include/code6cac.h);
  completed-callee interfaces `(s32 *)` math_MatrixToAnglesYXZ, `(u16 *)` math_TransposeMatrixInPlace,
  `(s32 *)` func_800418D0, `(u32 *)frame` func_800198D0; `(u16 *)0x1F800000` (scratchpad);
  `*(s16 *)((u8 *)player + 0x12)` (byte offset on a typed pointer, as func_80049718's `vehicle + 0x12`).

### Ruling 11 package (both variables; landing body candidate.c)
- Values. `player`: I0 = func_8004153C(0), I1 = func_8004153C(1) (first frame), L = func_8004153C(i) (loop);
  each read by its own block only. `rot_z`: Z0 = rotated z of the camera position, Z1 = rotated z of player
  i's root offset; each read once by `vec.vz = rot_z;` in its block.
- (A) function-scope declaration is the innermost scope enclosing both blocks' writes; address never taken.
- (B)(1) every write is read. (B)(2) Ruling 5 2(c) record: `player`'s loop write for i == 1 re-stores I1 on
  the path unk0 == 0, unk34[0] == 0, unk34[1] != 0 (func_8003FFC4 does not write g_player_ptrs); on the
  path unk34[0] != 0 it holds func_8004153C(0) there, so the value differs on a feasible path. The loop
  write for i == 0 follows I1 or (unk0 != 0) no write. `rot_z`'s loop write follows Z0 or the previous
  iteration's value, computed from a different vector.
- (C) one-variable-per-value spellings recorded with the same statement list; every value is a call result
  or arithmetic in the target.
- (D)(1)/(2) r11/d_proof.txt (+ r11/alloc_{reuse,zsplit,psplit,split}.txt, scripts/dumps.sh, r11dbg.sh):
  rot_z shared = pseudo 76, no basic block, 2 deaths -> reg_qty -1 (local-alloc.c:470-477), so
  combine_regs (local-alloc.c:1836) cannot tie it to the subtraction (163 -> v1 / 301 -> v0) and global
  alloc gives it t0: `subu $3,$9,$10 / sra $8,$3,12`, `subu $2,$10,$12 / sra $8,$2,12` = target
  0x80054B24/30, 0x80054E04/0C. Split: rot_z0 160 and rot_z1 299 are block-local, tied into the
  subtraction's quantity (both "in 3"): `sra $3,$3,12`. player shared = pseudo 75, crosses 1 call (the loop
  value across func_800198D0), global.c find_reg pass0_used excludes every call-clobbered reg -> 16 ($s0)
  for all three values: `move $16,$2` after each func_8004153C call (target 0x80054A50, 0x80054A6C,
  0x80054CF8). Split: init allocnos 82/83 cross no call, own_copy_prefs {2,4} -> $2, the two moves vanish.
- (D)(3)/(4) r11/scores.txt (landing-body generator r11/scripts/r11all.sh; bodies in r11/bodies/):
  | spelling | score |
  |---|---|
  | landing (player shared, rot_z shared) | 0 |
  | player partitions, rot_z shared: split 9, {I0}{I1,L} 3, {I1}{I0,L} 3, {I0,I1}{L} 9 | 3..9 |
  | same with rot_z split | 44..50 |
  | rot_z split, player shared | 41 |
  | structural, player: init as a loop 14/55, `if ((p = f()) != 0)` 9/50, per-call blocks 9/50, fn-scope locals 9/50 | |
  | structural, rot_z: vx temp 50/59, no x/z copies 41/50, both temps 41/50, no temp vz first 38/47, fn-scope locals 41/50 | |
  | Ruling 4 split assignments of per-block rot_z (a/b/c/e x both/first/loop) | 12..60 |
  (x/y = other variable shared / split.) Permuter from the full split body: r11/permuter_harvest.txt
  (17.7k iterations, best 1566 vs the reuse body's 140 calibration; finds are rot_z re-splits, chained
  vec.vz stores, empty-if / inline-fn noise).
- (E) names `player` (each value is a player object) and `rot_z` (each value is a rotated z).
- (F) comments at both declarations in candidate.c.
- Retire on landing: the 11 undefined_syms_auto.txt rows marked "retire with func_8005490C" (D_800EFB14..20,
  D_80101E00/02/04/08/3C/40/44); every other referrer is completed C or the unlinked asm/6CAC.s.
