# func_8005490C â€” hypotheses / ruled out (manual session 2026-09-25)

Frontier (2026-09-25):
- candidate.c scores **47 (397/399)** and is fully single-role.
- The 0-form (rejected/vz-obj-multiwrite-0.c) got a layer-2 FAIL on `vz` / `obj` reuse.
- The residual in the 47 form comes from two separate mechanisms.
  - (a) 2 insns + regs: the init player pointers land in v0, not $s0. The target has
    `move s0,v0; beqz s0; ...; move a0,s0` twice.
  - (b) about 38 points: the rotation's `subu`/`sra` pair gets tied by local-alloc. The target keeps
    them untied, with nz in t0. sched2 then orders the y-translation chain after the nz store.

## Ruled out (measured)
- Rotation spellings with per-block temps:
  - 16 inner forms (x/z copies, z-only, nx/nz temps, none): all score 49 identically.
  - Store orders (6 permutations of vx/vz/vy): 38..43.
  - Mult operand orders / association: 38..45.
  - s16 vs s32 cos/sin: no change.
  - Sin declared before cos: 58.
  - Inline Judge reads without temps: 118..125 (429 insns).
- Translation (`vec.vy += unk10`) placed before the rotation: 49..56.
- t[] / xf.rot store reorders: 36..70. t[1] stored first moves the $s4 base to +0x50.
- Hoisting the unk10 load into a named local: 36 (also a carrier).
- Init player pointer written twice in one init-only local (the func_80054604 `v` shape), with the
  loop's separate: 9 (397 insns). Two block-scoped init locals give the same 9.
- Scalar `g_anim_func_table` declaration: 66. Now fixed on main by a9c634304.

## Why both residuals are allocator-structural (dump-proven, tmp/490c/pd/)
- (b) The subtract/shift pair can only stay untied if one of two things holds:
  - The SET pseudo is non-local. local-alloc.c:469-476 requires one basic block and n_deaths == 1,
    and combine_regs refuses `reg_qty[sreg] == -1`.
  - The subtract result does not die at the shift.
  The rotated z is read only in its own block. So a single write cannot make the pseudo non-local.
  It must be referenced in two blocks, or set twice.
- (a) The init pointer lands in $s0 only when it is the same pseudo as a call-crossing one. The
  init value dies before func_8003FFC4 (the move sits in the jal delay slot). No init-only
  spelling crosses a call.

## Untried ideas (for the next session)
- Is there a real second reader of the rotated z or of the init player pointer that the target
  folds away? A reader that genuinely exists but emits no instructions would change pseudo
  locality honestly. None was found by reading the target.
- Does any single-role structure make the subtract result live past the shift (for example a real
  second use of `z*c - x*sn`)? The target shows none.
- Permuter from the 47 candidate (tmp/perm_490c_c, 3 workers, 18,864 iterations, 2026-09-25):
  1660 -> 1453 permuter units, then flat. Finds were an addend swap (`s->unk14 + vec.vz`) and an
  `i = s->unk10` counter-reuse carrier (inadmissible). The random basin is exhausted; the next
  run should use directed PERM_* or a new structure.

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated (floor 47/399). `candidate.c` (single-role) = 47. `rejected/vz-obj-multiwrite-0.c` = 0 + oracle SHA1 but failed layer-2 on `vz`/`obj` (`rejected/vz-obj-multiwrite-0.md`). Byte-neutral prep a9c634304 landed.
- RE-OPENABLE: rotated before Ruling 11 (262db111c, 2026-09-26); `.claude/rules/ordinary-c-judge-decidable.md:40-41` lets Ruling 11 admit what Rulings 5/6/9 fail. Never tried under Ruling 11.
- CONSTRAINTS: `obj` on the Ruling 9 banned list (`.claude/rules/reused-local-meaning-source.md:60`); Ruling 5 (1a, 1c) and Ruling 6 (A, C) failures listed in vz-obj-multiwrite-0.md.
- BLOCKER [F]: dump-proven in `tmp/490c/pd/` (prio.txt, .lreg, .greg - gitignored, bank them): `vz` - local-alloc.c:469-476 + combine_regs (:1829) tie a per-block vz; only a two-block pseudo stays untied. `obj` - init pointers reach $s0 only when sharing a pseudo with the loop's call-crossing pointer.
- PLAN:
  1. Rebase all banked .c files (STALE): all six use `(&Judge)[...]` but Judge is now `s16 Judge[0x1000]` (`include/code6cac.h:27`) -> `Judge[...]`; restore the `g_anim_func_table[]` declaration that 016788ef0 deleted from src/text1b.c (scalar form 66, array form 0).
  2. Re-measure 0-body, 47-body, 9-body.
  3. Ruling 11 package: rename `obj` -> `player` (each value is a `func_8004153C(n)` result), `vz` -> `rot_z`; bank dumps for both spellings; splits measured 38, 9, 141; permuter from the split body (18.9k iters) done; add the (F) comment; the pointer-alias `s` FAKE stays.
  4. Fresh manual layer-2; operator `unpark`s it.
  5. On landing retire the 11 undefined_syms rows marked "retire with func_8005490C".
- DEPENDS: `g_anim_func_table[]` declaration shared with func_80049718.
- ODDS/LANE: manual, ~1 session, ~65% [I].

## [s2] 2026-10-01 laneB — BANKED at run close (src/include clean; unpark f78231287 stays committed)
- Floor: candidate.c = 0/399 on post-Q65 main (scratch-TU build with the (A) below; every other text1b /
  code6cac_c2 function byte-identical). Full Ruling 11 package for `player` / `rot_z` measured on that exact
  body: evidence.md [s2], r11/ (scores, d_proof, alloc_*, permuter 16094 it. best 1335 vs 140 calibration).
- (A) plan, byte-neutral, was staged and reverted at close: text1b.c `extern void func_8003FFC4(s32)` ->
  `(s32 *)`; func_80054604 `s32 v` -> `s32 *v`, drop its two `(s32)func_8004153C(n)` casts. SHA1 == oracle
  and func_80054604 sandbox 0 were verified; layer2 hash b97c4f8f5486cce0; message draft:
  cleanup_A_msg_draft.txt. Land (A), then the Match.
- Ctrl-block integer evidence: typing unk2C/unk30/unk34[]/unk3C[] as `u8 *` swaps addu operands,
  func_80054FDC 6 and func_80054604 2 (datamodel/ptr_typing.txt); the members stay s32, casts at uses.
- OPEN DECISION (orchestrator/owner) — `g_anim_func_table` (extern s32 g_anim_func_table[], D_800F66A0):
  (1) cast-and-disclose: call `((void (*)(SVECTOR *, MATRIX *))g_anim_func_table[0])(...)` as the 6
      landed consumers do (text1b.c func_80046BF4, the 0x80047xxx bone/camera bodies, func_80049718
      (68dfe8107, PASS today), text1a_post.c func_800417D0 / func_800418D0). Checklist item 2 bans
      (T *)int, so this needs an explicit acceptance.
  (2) pointer retype in an (A): no byte evidence for the integer model here (a function-pointer table type
      is byte-neutral for every call site measured: lw 0($s1); jalr). Scope: 5 TUs (text1a_c.c store
      `(s32)math_RotMatrixZYX`, text1a_post.c x2, text1b.c x4, text1b_b.c scalar decl
      `s32 (*g_anim_func_table)(s16 *, s16 *)`), 8 consumers, and the table's other slots D_800F66A8 /
      D_800F66B0 / D_800F66B4 are separate splat symbols (text1a_c.c stores, text1b.c D_800F66B0 call), so
      an aggregate merge with its consumer paperwork follows. Unprototyped `void (*[])()` avoids
      per-consumer arg casts; `(SVECTOR *, MATRIX *)` needs SVECTOR/MATRIX views in consumers with
      mistyped locals (func_80046BF4's s16 rot[3] / s32 matrix_buf[8]).
- Other reviewer points (decided, measured): frame as `s16 frame[0x42]` (MotionFrame u16 unk_02/unk_04
  give lhu; MotionFrame + (s16) casts also 0); FAKE `s` (direct global 157); simplest form (no p local, no
  x/z copies, block-scoped `j`). Match message draft: match_msg_draft.txt
  (fill the (A) commit and the review line).
- Retire on the Match: 11 undefined_syms_auto.txt rows "retire with func_8005490C".

## [s3] 2026-10-02 oct2-b1 â€” READY_FOR_REVIEW
- Frontier closed pending layer-2: candidate.c (FAKE-relabelled) is the staged body; see evidence.md [s3].
