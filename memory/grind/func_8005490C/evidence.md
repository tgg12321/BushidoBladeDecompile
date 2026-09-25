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
