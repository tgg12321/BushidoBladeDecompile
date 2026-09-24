# Heavy-12 residual triage — 2026-08-05 (read-only measurement)

Method: `sandbox --disable all` for the honest score; target (`build/src/<stem>.o`) vs the
cheat-invisible sandbox object normalized through `objdump -dr` and aligned with difflib,
each difference bucketed RENAME / FRAME / OFFSET / MOVED / MISSING / EXTRA / STRUCT;
cc1 `.frame vars=` + callee-save slot maps for the frame check;
`sandbox --disable all --keep-cheat-asm` to attribute residual to pins/GTE;
`canonical` re-run for all 12. Scripts + raw data: `tmp/triage12/`.

## Ranked

| # | func | rules | honest | keep-asm | canonical | dominant groups | class | campaign tool |
|---|---|---|---|---|---|---|---|---|
| 1 | camera_set_zoom | 47 | 48 | 48 | C | RENAME 41 (85%) | **RA** near-pure | ra_solver (5 `$A<->$B` swap rules name the pseudos) |
| 2 | func_80074B18 | **29** (queue 41) | 79 | **27** | ASM-SUSPECT | FRAME 24, RENAME 15, STRUCT 14 | **RA**, frame derived | ra_solver + Phase-5 local_alloc |
| 3 | DispSchoolBG | 45 | 97 | **17** | ASM-PARTIAL 24/182 GTE | mostly stripped cop2 | **canonical-GTE + small RA** | GTE authorization, then RA on 17 |
| 4 | hirahira_w_ctrl_2 | 63 | 58 | 58 | ASM-SUSPECT | RENAME 25, MISS 17, EXTRA 13, FRAME 6 | MIXED (type 30%, RA 34%) | halfword type-correction (`lhu`+`sll/sra` vs our `lh`) |
| 5 | gnd_land_hit_char_tsuba | 88 | 65 | 65 | ASM-SUSPECT | FRAME 22, RENAME 17 | MIXED (FRAME 34%, RA 27%) | frame sweep: target has **+16 B locals** |
| 6 | decBs0 | **39** (queue 40) | 58 | 52 | ASM-SUSPECT | FRAME 21, RENAME 13 | MIXED (FRAME 34%, RA 21%) | frame sweep: **+8 B locals**, then base-reg RA |
| 7 | mk_leaf_newpos | 43 | 65 | 65 | ASM-SUSPECT | RENAME 39 (60%), STRUCT 9 | MIXED (RA 60%, LICM 15%) | defeat-licm-hoist-var-reuse, then ra_solver |
| 8 | marionation_Exec | 42 | 56 | 40 | ASM-SUSPECT | RENAME 33 (59%), STRUCT 11 | RA + prologue-order + byte-andi | ra_solver; save slots match, only emission order |
| 9 | func_8002C61C | 42 | 44 | 44 | C | OFFSET 15, MISS/EXTRA 12 each | MIXED (copy-canon 40%, SCHED 25%) | param-reuse-base-copy-cse-canon |
| 10 | saTan0KiWareMoveA | 65 | 56 | 53 | ASM-PARTIAL 8/202 GTE | RENAME 27 (46%), MISS 16 | MIXED (RA 46%, divmod-sched 25%) | GTE auth + divmod-coalesce-reuse-var |
| 11 | marionation_camera_Exec | 75 | 103 | 76 | ASM-PARTIAL 10/153 GTE | MISS 35, RENAME 31, STRUCT 23 | MIXED (GTE/scratchpad 35%, RA 27%) | GTE auth + scratchpad-gte addressing |
| 12 | exec_game | 104 | **121** | 121 | ASM-SUSPECT | RENAME 40, STRUCT 29, EXTRA 23 | **STRUCTURAL** (CSE/induction) + RA | defeat-CSE on two globals, then induction form |

## Key notes

- **Queue rule counts are stale for two entries.** `04e22879` (2026-08-04) retired 20
  `-mel`-obsoleted offset rules: `func_80074B18` is now 29 (queue 41), `decBs0` 39 (queue 40).
  Honest distances unchanged — those rules were already inert.
- **`--keep-cheat-asm` is the best RA-triage instrument found.** `func_80074B18` 79 → 27:
  three `register … asm("sN")` pins, whose only effect is register choice, are worth 52
  points. Same reading smaller: DispSchoolBG 97 → 17, marionation_camera_Exec 103 → 76,
  marionation_Exec 56 → 40, decBs0 58 → 52, saTan0KiWareMoveA 56 → 53.
- **Frame deltas are single-root-cause.** Wherever a frame mismatch exists the callee-save
  *set* is identical and every slot shifts by one constant, so 20+ diff lines collapse to
  one locals-block change (phantom-frame-slot class): gnd_land_hit_char_tsuba +16 (20 lines),
  decBs0 +8 (19), hirahira_w_ctrl_2 −8 (4), func_80074B18 −16 (22, derived from its RA state).
- **Measure frames on the unstripped source.** DispSchoolBG's real frame (56) already matches
  target; the +16 in the stripped stream is an artifact of removing pins. Only
  gnd_land_hit_char_tsuba, hirahira_w_ctrl_2 and decBs0 have mismatches confirmed unstripped.
- **`$A <-> $B` swap rules are literal register-permutation repairs** and directly name RA
  leads: camera_set_zoom 5, marionation_Exec 5 (two rename chains), exec_game 4,
  mk_leaf_newpos 4, marionation_camera_Exec 3 (arg-register rotation), func_80074B18 2, decBs0 1.
- **exec_game's six `subst_multi` rules re-create loads CSE deleted** — each replaces a
  register-held value with `lw D_800A2D3C`/`lw D_800A2D40` + `nop` (two also re-insert a lost
  `addu $t1,$t1,1`). The original C re-reads those globals at every use; ours caches them.
  exec_game is also frame-less (`.frame $sp,0` both sides), so no frame lever exists.
- **`cheat_asm_stripped` in the sandbox JSON is per-TU, not per-function** (334 for every
  `code6cac_b` function). Don't read it as a per-function cheat count.
- marionation_Exec's save *slots* already match target; only prologue emission order differs —
  the same residual shape as tslGlobalMemFree.
