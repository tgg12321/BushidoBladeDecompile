# Evidence bank — special_camera_get_rot_dir

## s1 (recon) — floor 12 -> 9. Mechanism fully mapped.

- canonical: verdict C, distance 12, 0 rules, 72 insns (pure-C target).
- The "near-duplicate lead func_80037348" is SPURIOUS: func_80037348 is the
  auto-name for THIS SAME address (0x80037348). Both .s files are byte-identical
  because they are the same function. There is no separate analog to copy.

- **The honest floor is a PURE 4-register rotation** of the 4 callee-saved
  values over s2/s3/s4/s5. The instruction stream is otherwise byte-identical
  (72==72 in the top-def form). Mine(top-def, score12) vs target:
    value        mine   target
    index        s3     s2
    cam_base     s4     s3
    constant_80  s5     s4
    copy_end     s2     s5
  i.e. copy_end has the HIGHEST allocno priority in my C (grabs s2) but must be
  LOWEST (s5). The other three are in the correct RELATIVE order in both.

- **CRITICAL CORRECTION to the s0/WIP claim "the masked sandbox is BLIND":**
  it is NOT blind. `sandbox --disable all` score == exact count of
  rotation-differing instructions (12 in top-def). It only strips register
  PINS (so editing pins can't move it), but it DOES respond to real C-structure
  changes. Proven: 3 structural edits moved the score 12 -> {14, 9, 22}.
  => The masked sandbox IS a usable gradient here. sandbox 0 <=> full byte match
  (the ONLY diff is the rotation).

- **BEST FORM (candidate.c, floor 9):** move `copy_end = &sp_buf[0x40]` INTO the
  copy block, just before the do-loop. Result: index->s2, cam_base->s3,
  constant_80->s4 ALL CORRECT. Residual: copy_end lands in caller-saved **t0**
  (build_insns=70, missing s5 save/restore) because its live range no longer
  crosses a call. Target needs copy_end in callee-saved **s5**.

- **Allocno priority formula (tools/gcc-2.7.2/global.c:604-624):**
    pri = floor_log2(n_refs) * n_refs / live_length * 10000 * size
    tiebreak: lower allocno number wins (line 624).
  NO loop-depth/frequency weighting — purely static n_refs and live_length.
  copy_end n_refs=2 (1 def+1 use); index/cam_base/constant_80 each n_refs=3.
  copy_end's SHORT live_length gives it 2/L_ce > 3/L_others -> highest priority
  in top-def. This is the documented global.c:624 allocno-priority near-tie
  wall (same class as marionation_Exec / cpu_side_move_dir_4).

## Measured probe results (sandbox --disable all):
  - top-def (constant_80,index,cam_base,copy_end order):  12  (baseline; copy_end->s2)
  - copy_end defined 2nd:                                 14  (WORSE) [rejected/]
  - copy_end block-local (in copy block):                 9   (BEST; copy_end->t0)
  - copy_end assigned inside retry before 1st call:       22  (WORSE) [rejected/]

## Ruled out (do not re-derive)
- Reordering index/cam_base BEFORE constant_80: breaks prologue (target assigns
  constant_80=0x80 FIRST). Keep constant_80 as the first assignment. (s0)
- copy_end def 2nd -> 14. (s1)
- copy_end assigned inside retry loop -> 22. (s1)

## Artifacts (tmp/grind/special_camera_get_rot_dir/s1/)
- mine.txt         — objdump of the score-9 build (copy_end in t0)
- pp.i.greg        — cc1 -da RTL dump (Register dispositions; multi-func file)
- pp.i.lreg, cc1.s, cc1.err
- dis.sh, greg.sh, g2.sh — repro scripts

- [s1] canonical: verdict C, distance 12, 0 rules, 72 insns.

- [s1] The near-duplicate lead func_80037348 is spurious: it is the auto-name for THIS same address 0x80037348; both .s are byte-identical (same function).

- [s1] Honest gap is a pure 4-register rotation over callee-saved s2/s3/s4/s5; instruction stream otherwise byte-identical.

- [s1] Masked sandbox IS a valid gradient (score == rotation-diff count); it only strips pins, not real C-structure changes. sandbox 0 <=> full byte match.

- [s1] global.c:604 allocno priority = floor_log2(n_refs)*n_refs/live_length*size; tiebreak = lower allocno number (line 624). No loop-depth weighting.

- [s1] copy_end (n_refs=2) currently outranks the n_refs=3 trio due to short live_length -> grabs s2; must become lowest priority -> s5.

- [s1] BEST form floor=9: copy_end defined block-locally lands index->s2, cam_base->s3, constant_80->s4 correctly; copy_end goes to caller-saved t0 (build_insns=70).

- [s1] Probe scores: top-def=12, copy_end-2nd=14, copy_end-block-local=9, copy_end-in-retry-before-call=22.

- [s1] Ruled out: reorder index/cam_base before constant_80 (breaks prologue; constant_80=0x80 must emit first).
