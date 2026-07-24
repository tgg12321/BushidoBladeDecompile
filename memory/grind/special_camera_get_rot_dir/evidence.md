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

## s2 (structural) — floor stays 9. F1/F2/F3 RESOLVED with exact allocno numbers.

- **Built a scratch instrumented cc1** (tmp/grind/.../s2/cc1_dbg): the in-tree
  tools/gcc-2.7.2/global.o ALREADY contains the BB2_ALLOC_DEBUG fprintf
  (nrefs/livelen/pri per allocno) but the build/ cc1 was linked before it was
  rebuilt. Relinked cc1 from the in-tree .o's into tmp/ (touched NOTHING in
  tools/). Run: `wsl bash tmp/grind/.../s2/alloc2.sh <tag>` after setting src +
  running sandbox. Gives exact nrefs/livelen/pri for special_camera's allocnos.

- **EXACT allocno table (nrefs, livelen, pri, hardreg), top-def (score 12):**
    dest 72:  n4 L38 pri2105 -> s0
    buf2 78:  n3 L31 pri967  -> s1
    copy_end 77: n3 L32 pri937 -> s2   (WRONG; must be s5)
    index 73: n3 L34 pri882  -> s3
    cam 74:   n3 L66 pri454  -> s4
    const 76: n3 L76 pri394  -> s5
  Assignment is PURE PRIORITY ORDER: highest pri -> lowest s-reg. Confirmed by
  find_reg (global.c:1011-1044): no MIPS REG_ALLOC_ORDER, so it takes the
  LOWEST available reg in ascending order; NO cost model favors higher regs.
  Only a copy-PREFERENCE (global.c:1046+) can override to a higher reg.

- **Target requires copy_end LOWEST priority (-> s5).** Order needed:
  dest>buf2>index>cam>const>copy_end. So copy_end pri must be < const's 394,
  i.e. copy_end livelen must be > 76 (nrefs 3 -> pri=30000/L).

- **copy_end's livelen is STRUCTURALLY CAPPED ~38** (its ONLY use is the
  mid-function do-loop; nothing after the loop uses the end pointer). Measured
  def-placement sweep of copy_end's livelen:
    block-local (def in copy block): L5  -> t0 (score 9, BEST)
    top-def (def 4th):               L32 -> s2 (score 12)
    def-1st (before const):          L38 -> s3 (score 14)  [MAX livelen]
  L38 << cam(64)/const(74-76). copy_end can NEVER drop below cam/const.

- **copy_end has NO copy-preference lever:** it is a leaf local address
  (`addiu sp+0x50`) with one use in a compare (`bne`). No copy insn to/from a
  hard reg => hard_reg_preferences[copy_end] is empty => find_reg cannot pull
  it to s5. (Generating one would require a register pin = cheat.)

- **const_80's livelen is LOCKED at 76** (needed at BOTH the first and final
  func_800372F4 calls; held in a callee-saved reg the whole span). Probe:
  passing literal 0x80 at the final call dropped const nrefs 3->2 but livelen
  STAYED 76 (CSE still holds it) -> pri 263, score 10 (WORSE). So the "tie
  const with copy_end" path (F2 allocno-tiebreak) is impossible: can't raise
  copy_end to 76 nor lower const below 38.

- **CONCLUSION:** with the fixed target instruction stream, GCC's allocno
  priority + find_reg cannot place copy_end in s5 (short-lived leaf value, no
  preference, priority structurally bounded above const). This is the
  global.c:624 allocno-priority wall class the s1 note flagged (marionation_Exec
  / cpu_side_move_dir_4 family). NOT a compiler wall — the match exists — but
  the lever is NOT copy_end def/use placement (that space is now exhausted).

## s2 probe scores (sandbox --disable all):
  block-local (candidate, floor):  9   [copy_end->t0]
  final-call literal const:        10  [rejected/]
  top-def:                         12
  copy_end used in Triple copy:    12  [rejected/, breaks tail bytes]
  copy_end def-first:              14  [rejected/]

## s2 artifacts (tmp/grind/special_camera_get_rot_dir/s2/)
- cc1_dbg (instrumented cc1, ALLOCDBG), link_cc1dbg.sh, alloc2.sh
- allocdbg_{score9,topdef,def1st,triple_ce,litconst}.txt (per-form allocno tables)
- pp_*.i, cc1_*.s

- [s2] Floor unchanged at 9 (block-local copy_end -> caller-saved t0; build_insns 70 vs target 72). Candidate.c is this form.

- [s2] Target (asm/funcs/special_camera_get_rot_dir.s) computes const_80(s4) first, then [call], index(s2), cam(s3), copy_end(s5), buf2(s1); our top-def source order already matches this emitted order, so the gap is purely register assignment, not instruction order.

- [s2] GCC allocno priority = floor_log2(nrefs)*nrefs/livelen*10000*size; tiebreak = lower allocno number first (global.c:604-624). find_reg picks the LOWEST available hard reg in ascending order (no MIPS REG_ALLOC_ORDER; global.c:1011-1044); only a copy-preference (1046+) can override to a higher reg.

- [s2] Exact instrumented numbers (top-def): copy_end nrefs3 livelen32 pri937->s2; index n3 L34 882->s3; cam n3 L66 454->s4; const n3 L76 394->s5. Assignment strictly follows priority order.

- [s2] copy_end livelen is structurally capped ~38 (def-first) because its only use is the mid-function do-loop; nothing after the loop uses the end pointer (post-loop Triple reuse extends range by 1 insn and breaks bytes).

- [s2] const_80 livelen is locked at 76 (needed at both the first and final func_800372F4 calls); literal-const probe kept livelen=76. So copy_end (<=38) can neither exceed nor tie const -> never lowest priority -> never s5 via priority.

- [s2] copy_end is a leaf local address (addiu sp+0x50) used once in a bne: it generates no copy insn, so hard_reg_preferences is empty and find_reg cannot pull it to s5. A pin to force it would be a cheat.

- [s2] This is the global.c:624 allocno-priority wall class flagged in s1 (marionation_Exec / cpu_side_move_dir_4 family). The match still provably exists; the dead axis is copy_end def/use placement / live-length, not the function.
