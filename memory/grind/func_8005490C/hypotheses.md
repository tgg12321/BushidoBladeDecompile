# func_8005490C — hypotheses / ruled out (manual session 2026-09-25)

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
