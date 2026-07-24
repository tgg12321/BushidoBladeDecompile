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

## s3 (structural) — floor stays 9. NEW lever found (loop-invariant def-placement); copy_end↔cam residual isolated; G1-placement + G2 KILLED.

- **m2c reconstruction (G3 DONE):** target body is ONE loop (header = both back-edges,
  same as my `retry` single-label form); s5 (sp+0x50) is a loop-invariant src-relative
  end pointer HOISTED to the outer preheader. Decomposition is identical to candidate —
  no alternate allocno set. `addiu $s5,$sp,0x50` sits in the preheader (loop-invariant,
  held across the loop); `addiu $a2,$sp,0x20` (src) is reinit per-iteration.

- **My top-def emitted asm is byte-structurally IDENTICAL to target except the 4-reg
  rotation.** copy_end's `addu $18,$sp,80` is in the outer preheader exactly like target's
  `addiu $s5,$sp,0x50`, loop-carried in both. The gap is purely register assignment.

- **flow.c reg_live_length is per-insn-live-count, NOT linear span.** Traced via
  BB2_FLOW_DEBUG (built into flow.c). top-def: copy_end(77) and const(76) are BOTH live in
  ALL blocks 0-4 incl. the bottom final-call block 4 (19 insns EACH — loop-carry IS
  counted there). copy_end's livelen deficit (32 vs 76) is almost entirely in **block 0**
  (const 14 insns vs copy_end 4) because const is defined earliest and held; block 2 (+2).

- **The 4 callee-saved competitors are ALL loop-invariant-hoisted** (const, cam, copy_end
  are constant/address invariants; index=jal-result is the only non-invariant). Their
  livelens are set by last-use position: copy_end (inner-loop bne, EARLIEST) < index/cam
  (cdrom call) < const (final call, latest). => copy_end is structurally the SHORTEST-lived
  => HIGHEST priority => grabs the lowest reg (s2/s3). Target needs it LOWEST priority (s5).

- **NEW WORKING LEVER — loop-invariant def-placement changes hoisted-invariant livelen:**
    top-def (const 1st):                     const L76 -> s5, copy_end s2   score 12
    const-last-before-retry (V_constlast):   const L62 -> s4, cam s5        score 11  [rejected/]
    copy_end-first + const-last (V_cefirst_constlast): index->s2 ✓, const->s4 ✓,
                                             copy_end L38->s3, cam L66->s5   score 10  [rejected/]
  Moving const's assignment later shortened its live range (76->62) and fixed const's slot;
  moving copy_end's assignment first raised it (L38, its MAX) and fixed index's slot.
  Residual at score 10: a copy_end<->cam 2-cycle (copy_end needs s5, cam needs s3).
  const-reassigned-INSIDE the retry loop breaks bytes (score 17, 69 insns) [rejected/].

- **The residual is UNREACHABLE structurally:** copy_end (L38 max) must exceed cam(66) AND
  const(62) to become lowest-priority. Its sole use is the inner-loop bne (structurally the
  earliest last-use of the four). It CANNOT be referenced later byte-neutrally — the only
  post-loop value is src(==copy_end addr) used by the Triple tail copy, and target reads
  that tail through src's reg (a2), not s5; expressing it via copy_end breaks the tail
  lw/sw bytes (reconfirmed; s2 copy_end-in-triple-copy = score 12). So copy_end's livelen
  is hard-capped at ~38 by its earliest-last-use, < const/cam. It can never be lowest-pri.

- **G2 (copy-preference) KILLED by the target itself:** target asm has NO `move`/copy insn
  to/from $s5 — copy_end reaches s5 purely via allocno PRIORITY (lowest), not a
  hard_reg_copy_preference. So inducing a copy-preference is NOT the target's mechanism;
  the only route is priority (G1), which the placement/reassociation axis exhausts.

- **CONCLUSION (s3):** confirms + extends s2. copy_end→s5 requires it to be BOTH callee-saved
  (must cross a call → needs a live range spanning a call → hoisted) AND lowest-priority
  (longest livelen). Hoisting gives it callee-saved status but livelen ~38 (earliest
  last-use) → high priority → s2/s3. Non-hoisted (block-local) → caller-saved t0 (floor-9).
  No pure-C structure makes a value that is simultaneously call-crossing and
  longest-lived when its only use is the earliest. This is the global.c:624 allocno-priority
  wall (marionation_Exec / cpu_side_move_dir_4 class). Structural axis EXHAUSTED. Next
  modality per ladder: permuter (directed PERM_* on statement order) or escalation.

- [s3] m2c: target = single loop (header=both back-edges) identical to candidate's retry form; s5 = loop-invariant src-relative end pointer (sp+0x50) hoisted to preheader; decomposition identical, no alternate allocno set.

- [s3] My top-def emitted asm is byte-structurally IDENTICAL to target except the 4-register rotation; copy_end's addu $18,$sp,80 is in the outer preheader exactly like target's addiu $s5,$sp,0x50, loop-carried in both. Gap is purely register assignment.

- [s3] flow.c reg_live_length is a per-insn live COUNT (BB2_FLOW_DEBUG-traced), not a linear span; it DOES count loop-carry (top-def: copy_end(77) and const(76) both live at all 19 insns of bottom block 4). copy_end's 32-vs-76 deficit is almost all in block 0 (copy_end 4 insns vs const 14) because const is defined earliest and held.

- [s3] The 4 callee-saved competitors are all loop-invariant-hoisted; livelen is set by last-use position: copy_end (inner-loop bne, earliest) < index/cam (cdrom call) < const (final call). copy_end is structurally shortest-lived -> highest priority -> lowest reg (s2/s3); target needs it lowest-priority -> s5.

- [s3] NEW LEVER: loop-invariant def-placement moves hoisted-invariant livelen. const assigned last-before-retry: L76->62, score 12->11 (72 insns). copy_end-first + const-last: index->s2 and const->s4 both CORRECT, residual is a copy_end<->cam 2-cycle, score 10 (72 insns).

- [s3] Residual UNREACHABLE: copy_end (L38 max, def-first) must exceed cam(66) and const(62) to be lowest-priority, but its only use is the earliest (inner-loop bne) and it cannot be referenced later byte-neutrally (target reads the Triple tail via src's reg a2, not s5; copy_end-in-tail breaks bytes = s2 score 12).

- [s3] copy_end must be BOTH callee-saved (cross a call -> hoisted) AND longest-lived (lowest priority); hoisting gives callee-saved but livelen ~38 -> high priority -> s2/s3; non-hoisted block-local -> caller-saved t0 (floor-9). No pure-C form yields a value that is both call-crossing and longest-lived when its sole use is the earliest. global.c:624 allocno-priority wall (marionation_Exec / cpu_side_move_dir_4 class).

## s4 (permuter) — floor stays 9. Permuter modality measured DEAD across 2 chassis (~46k iters). Escalated owner-gated.

- **Built a clean single-function permuter workspace** (tmp/grind/.../s4/ws): base.c =
  preprocessed full TU (score-10 seed body, pin-free); target.o from asm/funcs + prelude
  (`.set gp=64` dropped for r3000) so base sits at offset 0 and the score is the real diff.
  Validated: base==target==72 insns, residual = the copy_end<->cam 2-cycle (base_score 328
  in the permuter's reorder-weighted metric; sandbox objdump-metric 10). compile.sh extracts
  the func region and assembles with `.set noat/noreorder` prelude.

- **Chassis 1 (score-10 seed, copy_end hoisted):** 27,499 iters, ONE novel find
  (output-318-1, perm-score 318). Compiled + diffed vs target: SAME copy_end(s3)<->cam(s5)
  2-cycle as base; the 328->318 delta is pure statement-reorder penalty (li s4,128 moved),
  register allocation UNCHANGED. No score-0 match. Wall intact.

- **Chassis 2 (score-9 candidate, copy_end block-local -> caller-saved t0):** structurally
  distinct basin (70 insns, missing the s5 save/restore pair). ~18.5k iters, best perm-score
  113 (72 insns). EVERY find is a DIFFERENT wrong permutation of the 5 callee-saved values,
  never target {index=s2, cam=s3, const=s4, copy_end=s5, buf2=s1}:
    output-178: copy_end hoisted to s3 + extra `move t0,s3` (73 insns) — wrong.
    output-113: index->s3, cam->s4, const->s5, copy_end->s1, buf2->s2 — wrong permutation.
  The permuter can only shuffle WHICH callee-saved value takes WHICH slot; it cannot hit the
  target ordering because (s2 instrumented-cc1 proof) no C arrangement yields the allocno
  priority order that assigns it. No score-0 match in ~46k combined iterations.

- **AND-GATE 1 (canonical-asm authorization): FAIL.** `tools/scan_hand_coded.py --single
  special_camera_get_rot_dir` = tier LOW, score 2/8 (only S4 front-loads + the SPURIOUS S5
  self-cluster to func_80037348, which is the same 0x80037348 address). No S1 multu / S2
  empty-branch / S6 BIOS-jumptable STRONG signal. A 4-register allocno-priority rotation is
  ordinary GCC RA output, not a hand-coded signature. Canonical-asm NOT supportable.

- **AND-GATE 2 (coercion/SOTN precedent): FAIL.** The ONLY construct that lands copy_end in
  s5 is a `register Quad *copy_end asm("s5")` pin (or an equivalent hardcoded-$N __asm__) —
  a forbidden register-rotation cheat, score-inert under the masked sandbox, with ZERO
  SOTN-master precedent (register-rotation pins are explicitly non-sanctioned per
  no-new-park-categories.md). No pure-C closer exists in the measured structural+permuter space.

- **CONCLUSION (s4):** the permuter modality is measured dead. Combined with the s1-s3
  instrumented-cc1 structural proof (copy_end livelen hard-capped ~L38 << const's locked 76;
  find_reg pure priority order; copy_end has no copy-preference lever), every grind-advanceable
  axis is exhausted. This is the marionation_Exec / cpu_side_move_dir_4 allocno-priority-tie
  wall class (cpu_side_move_dir_4 REFUSED / OWNER-ACCEPTED INCOMPLETE 2026-07-18 under the
  endgame-lock-disposition policy). Filed OWNER-ESCALATION in docs/grind/decisions.md; returned
  owner-gated. NB: this function carries NO cheat (0 regfix/asmfix rules) — the clean floor-9
  candidate (block-local copy_end, pure C) is the best committable form; it is +2 insns short
  (missing the s5 save/restore), NOT a byte-match.

## s4 permuter artifacts (tmp/grind/special_camera_get_rot_dir/s4/)
- ws/ (chassis-1 workspace: base.c, compile.sh, target.o, base_ins.txt, tgt_ins.txt, output-318-1/)
- ws2/ (chassis-2 workspace: base.c + 231 output-*/ dirs, best output-113-1/)
- setup_ws.sh, setup_ws2.sh, compile/validate/check scripts, launch/wait/harvest scripts
- metrics/events.jsonl permuter-launch/harvest telemetry (labels s4-random-seed10, s4-random-seed9-blocklocal)

- [s4] Built a clean single-function permuter workspace: base.c = preprocessed full TU (pin-free seed body), target.o from asm/funcs + prelude with .set gp=64 dropped so both sit at offset 0. Validated base==target==72 insns, residual = copy_end<->cam 2-cycle (perm base_score 328, sandbox objdump-metric 10).

- [s4] Chassis 1 (score-10 hoisted seed): 27,499 iters, 1 novel find (318) = pure reorder noise, register rotation unchanged. No score-0.

- [s4] Chassis 2 (score-9 block-local seed): 18,554 iters, best 113 = a DIFFERENT wrong permutation of the 5 callee-saved values. Every find shuffles WHICH value takes WHICH slot; none is the target ordering. No score-0.

- [s4] ~46k combined iterations across two structurally-distinct fresh seeds produced ZERO byte matches — empirically confirms the s2 instrumented-cc1 proof that no C statement arrangement yields the allocno priority order placing copy_end (earliest last-use, livelen ~L38) in s5 (requires longest-lived / lowest priority).

- [s4] AND-GATE 1 (canonical-asm): scan_hand_coded LOW 2/8, no STRONG signal — ordinary RA output, not hand-coded; canonical-asm would launder the forbidden register pin.

- [s4] AND-GATE 2 (coercion/SOTN precedent): the only closer is a register-rotation pin (asm("s5") on copy_end), score-inert under the masked sandbox, zero SOTN precedent, explicitly non-sanctioned.

- [s4] This function carries NO cheat (0 regfix/asmfix rules) and does NOT byte-match; the clean floor-9 candidate (block-local copy_end, pure C) is the best committable form and is +2 insns short (missing the s5 save/restore pair). This is the marionation_Exec / cpu_side_move_dir_4 allocno-priority-tie wall class.

- [s4] Both my permuter campaigns harvested with --stop and confirmed registered_active=false; candidate.c matches src (clean pin-free floor-9 form), sandbox --disable all = 9.
