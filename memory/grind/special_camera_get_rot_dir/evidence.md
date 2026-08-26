> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

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

## s5 (synthesis) — floor stays 9. Ledger MERGED + RESET: the wall is a LIVELEN CHAIN, not "copy_end->s5", and a second (SHRINK) route is open and unmeasured.

**Chassis re-measured this session** (all on the current HEAD chassis, src = the ledger body pasted
over the INCLUDE_ASM line):
- candidate.c (block-local copy_end): sandbox --disable all = **9**, build_insns 70. Floor CONFIRMED.
- V_cefirst_constlast (rejected/copyend-first-const-last-score10.c): **10**, 72 insns. CONFIRMED.
- top-def (constant_80, index, cam_base, copy_end, buf2_ptr — in THAT order): **12**, 72 insns,
  reproducing the s1/s2 number exactly. Body banked at `tmp/grind/special_camera_get_rot_dir/s5/topdef2.c`.
  NB a near-miss ordering (copy_end assigned AFTER buf2_ptr) scores **15** — the top-def chassis is
  sensitive to that single statement swap; use topdef2.c verbatim rather than re-deriving it.

**The RA model is EXACT for this function.** `tools/ra_solver/extract.py` + `simulate.py` reproduce
the instrumented-cc1 dump 8/8 dispositions AND the sort order on BOTH chassis. Reachability for this
function is therefore a calculation, not an estimate — future sessions should sweep the simulator
before compiling anything.

    score-10 (cefirst_constlast) chassis:
      72 dest n4 L38 pri2105 -> s0 | 78 buf2 n3 L32 937 -> s1 | 73 index n3 L34 882 -> s2
      77 copy_end n3 L38 789 -> s3 | 76 const n3 L62 483 -> s4 | 74 cam n3 L66 454 -> s5
    top-def (score-12) chassis:
      72 dest n4 L38 2105 -> s0 | 78 buf2 n3 L31 967 -> s1 | 77 copy_end n3 L32 937 -> s2
      73 index n3 L34 882 -> s3 | 74 cam n3 L66 454 -> s4 | 76 const n3 L76 394 -> s5

- **THE EXACT REACHABILITY SPEC (new — supersedes "copy_end must reach s5" as the frontier
  statement).** Assignment is pure priority order (pri = floor_log2(n)*n*10000/L, tie -> lower
  pseudo wins). Target = {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5}. With every callee-saved
  allocno at nrefs=3 (dest at 4), that is EXACTLY the live-length chain

        L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end)

  (the `<=` ties are safe: where a tie occurs the pseudo that must win is the lower-numbered one,
  73<74, 74<76, 76<77). Verified by exhaustive simulation over cam/const/copy_end in [28,46) on the
  score-10 model — 364 hits, all and only the chain (`tmp/grind/.../s5/sweep3.py`).
  Current chains: score-10 = 32 < 34 < **66 > 62 > 38** (TWO inversions);
                  top-def  = 31 < 34 < 66 <= 76 but **L(copy_end)=32 sits at the wrong end**
                  (ONE inversion — copy_end is far too short).

- **ROUTE A (what s1-s4 attacked): raise copy_end.** In the top-def chassis the ONLY defect is
  copy_end's live length; simulation says target is reached iff **L(copy_end) >= 76** at nrefs=3, or
  **>= 51** at nrefs=2. s2/s3 measured copy_end's structural cap at ~L38 (its sole use is the inner
  copy-loop `bne` — the earliest last-use of the four invariants). Route A stays KILLED.

- **ROUTE A' (get copy_end to nrefs=2) is KILLED with a mechanism, not an estimate.** `reg_n_refs`
  is LOOP-DEPTH WEIGHTED (`tools/gcc-2.7.2/flow.c:2081`, `reg_n_refs[regno] += loop_depth`), whereas
  `reg_live_length` is a plain per-insn-visit count (flow.c:1685, flow.c:2087). copy_end has only
  **2 textual RTL refs** — verified in the .flow/.combine/.lreg dumps: `(insn 11 ... (set (reg/v:SI 77)
  (plus (reg 30 $fp) (const_int 80))))` and `(jump_insn 87 ... (ne (reg/v:SI 82) (reg/v:SI 77)))` —
  but weighs 3 because the def sits at loop depth 1 (the retry-loop preheader) and the use sits at
  loop depth 2 (inside the do-while copy loop). nrefs=2 would require the compare to be at depth 1,
  i.e. the copy loop not being a loop. Unreachable. (The same weighting explains the induction vars:
  pseudo 81 = 6 textual / 8 weighted, pseudo 82 = 7 / 10.)
  **Corollary worth keeping: the TARGET's `$s5` also has exactly 2 machine refs** (def
  `addiu $s5,$sp,0x50` at 0x80037388 + the `bne $a2,$s5` at 0x800373E4), so the target's copy_end
  allocno is nrefs=3 as well. The target reaches s5 by satisfying L(copy_end) >= L(const), NOT by
  carrying a different reference count.

- **ROUTE B (NEVER ATTEMPTED — the live frontier): shrink cam and const BELOW copy_end.** The chain
  is symmetric: instead of lifting copy_end over const, drop const and cam under it. In the
  **score-10 chassis** copy_end already sits at L38 > L(index)=34, so the chain is satisfiable with
  copy_end UNTOUCHED: simulation gives the target assignment for every **L(cam) in [34,38] and
  L(const) in [L(cam),38]** (cam 66 -> <=38, const 62 -> <=38, both still above index's 34). In the
  top-def chassis Route B is FORECLOSED (copy_end L32 < index L34 leaves no room). So **Route B must
  be seeded from V_cefirst_constlast (score 10), not from top-def** — the two chassis open opposite
  routes, which is why the s3/s4 framing ("copy_end must become lowest-priority") looked like a wall:
  it is only a wall on the branch those sessions were standing on.

- **Chassis-selection rule for future sessions (both directions banked):**
    Route A work -> seed `tmp/grind/.../s5/topdef2.c` (score 12; cam/const already correctly ordered).
    Route B work -> seed `rejected/copyend-first-const-last-score10.c` (score 10; copy_end already
                    above index; cam and const are the two values to shorten).
    The floor-9 candidate (block-local copy_end -> caller-saved t0, 70 insns) is a THIRD, separate
    basin that satisfies neither chain — best committable form, but not a launch pad.

- **ra_solver `inverse.py` run (owner directive 2026-08-24 executed).** Goal `{74:19, 76:20, 77:21}`
  on the score-10 model, depth 2. Minimal solution size 2 atoms, 6 vectors — but EVERY vector pairs a
  cam/const `live_shrink` with `refs_down pseudo 77: refs 3->1`, which is physically unspellable (a
  1-ref pseudo is def-only or use-only). The solver missed Route B and the true Route A threshold
  because its live-length atom space is hard-bounded to +/-{2,4,8}
  (`tools/ra_solver/inverse.py:206-220`) while the deltas needed here are -28 (cam) and +44
  (copy_end). **Do not read inverse.py's ranked vectors as this function's search space; use the
  direct simulate.py sweeps banked in `tmp/grind/.../s5/sweep*.py`.** The run DID mechanically
  confirm the s3 G2 kill: all 24 preference atoms naming $s3/$s4/$s5 are reported FORECLOSED —
  "callee-saved registers cannot appear in pre-RA RTL from any C at all" — so no copy-preference
  route to s5 exists for any spelling, in any chassis.

- [s5] Floor re-measured on the current chassis: candidate = 9 (70 insns); score-10 form = 10 (72); top-def (topdef2.c) = 12 (72). Every banked score is chassis-valid; nothing needed re-deriving.
- [s5] ra_solver extract+simulate is EXACT for this function (8/8 dispositions, sort order MATCH) on both chassis, so reachability is now a calculation rather than a compile-and-see.
- [s5] Reachability spec: the target assignment holds iff L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end), all at nrefs=3. Verified by exhaustive simulation (sweep3.py: 364 hits = exactly the chain).
- [s5] Route A (raise copy_end): the top-def chassis reaches target iff L(copy_end) >= 76 at nrefs=3 or >= 51 at nrefs=2; the structural cap is ~38, so Route A stays KILLED.
- [s5] Route A' (copy_end at nrefs=2) is KILLED by mechanism: reg_n_refs is loop-depth weighted (flow.c:2081) while reg_live_length is not; copy_end's 2 textual RTL refs weigh 3 because the def is at depth 1 and the bne use at depth 2. nrefs=2 needs the copy loop not to be a loop.
- [s5] The target's $s5 carries exactly 2 machine refs, the same 2-textual/3-weighted profile as ours: the target reaches s5 by L(copy_end) >= L(const), not via a different reference count.
- [s5] Route B (shrink cam 66->[34,38] and const 62->[L(cam),38] with copy_end untouched at L38) reaches the target assignment in the score-10 chassis and has never been attempted in five sessions. It is FORECLOSED in the top-def chassis (copy_end L32 < index L34).
- [s5] Chassis selection is load-bearing and now banked: Route A seeds from tmp/grind/.../s5/topdef2.c (score 12), Route B from rejected/copyend-first-const-last-score10.c (score 10); the floor-9 candidate is a third basin satisfying neither chain.
- [s5] inverse.py's ranked vectors are NOT this function's search space: its live-length atoms are bounded to +/-{2,4,8} (inverse.py:206-220) while the needed deltas are -28 and +44, so every minimal vector it emits requires the unspellable refs 3->1. Its FORECLOSED section does mechanically confirm the s3 G2 kill (no callee-saved preference is reachable from any C).

## s5 artifacts (tmp/grind/special_camera_get_rot_dir/s5/)
- apply.py (LF-safe body swapper), topdef.c (score-15 near-miss ordering), topdef2.c (score-12 top-def chassis)
- topdef.model.json (exact RA model of the top-def chassis)
- sweep_model.py / sweep2.py / sweep3.py / sweep_topdef.py (simulate.py-driven reachability sweeps)
- flowdbg.py (BB2_FLOW_DEBUG per-block liveness harness — its output is TU-wide and needs
  per-function segmentation before it is usable; noted so s6 does not re-discover that)
- tmp/grind/special_camera_get_rot_dir/dumps/*.flow/.combine/.lreg/.greg (score-10 form, via tools/grinder/dump.ps1)

- [s5] Floor re-measured on the current HEAD chassis: candidate.c (block-local copy_end) = 9 at 70 build_insns; rejected/copyend-first-const-last-score10.c = 10 at 72; the top-def chassis = 12 at 72. Every banked score is chassis-valid — nothing in the s1-s4 ledger needed re-deriving.

- [s5] The top-def chassis is sensitive to one statement swap: with copy_end assigned AFTER buf2_ptr it scores 15, with copy_end assigned BEFORE buf2_ptr it scores 12. Both bodies are banked (rejected/topdef-copyend-after-buf2ptr-score15.c and rejected/topdef-score12-routeA-seed-chassis.c) so the chassis is never re-derived by hand.

- [s5] tools/ra_solver's forward model is EXACT for this function: extract.py + simulate.py reproduce the instrumented-cc1 dump 8/8 dispositions AND the sort order on both chassis. Reachability questions here are calculations — sweep the simulator before compiling.

- [s5] Exact allocno tables. score-10 chassis: 72 dest n4 L38 pri2105 -> s0 | 78 buf2 n3 L32 937 -> s1 | 73 index n3 L34 882 -> s2 | 77 copy_end n3 L38 789 -> s3 | 76 const n3 L62 483 -> s4 | 74 cam n3 L66 454 -> s5. top-def chassis: 72 n4 L38 2105 -> s0 | 78 n3 L31 967 -> s1 | 77 n3 L32 937 -> s2 | 73 n3 L34 882 -> s3 | 74 n3 L66 454 -> s4 | 76 n3 L76 394 -> s5.

- [s5] REACHABILITY SPEC (new, supersedes 'copy_end must reach s5' as the frontier statement): the target assignment holds iff L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end) with all four callee-saved allocnos at nrefs=3. The <= ties are safe because where a tie occurs the pseudo that must win is the lower-numbered one (73<74, 74<76, 76<77). Verified by exhaustive simulation: 364 hits, all and only the chain.

- [s5] Route A (raise copy_end) stays KILLED: from the top-def chassis the target needs L(copy_end) >= 76 at nrefs=3 (or >= 51 at nrefs=2), against a structural cap of ~38 measured in s2/s3 (copy_end's sole use is the inner copy-loop bne, the earliest last-use of the four invariants).

- [s5] Route A' (copy_end at nrefs=2) is KILLED by mechanism: reg_n_refs is loop-depth weighted (tools/gcc-2.7.2/flow.c:2081) while reg_live_length is not (flow.c:1685, flow.c:2087). copy_end has 2 textual RTL refs — (insn 11 (set (reg/v:SI 77) (plus (reg 30 $fp) (const_int 80)))) and (jump_insn 87 (ne (reg/v:SI 82) (reg/v:SI 77))) — weighing 3 because the def is at depth 1 and the use at depth 2.

- [s5] The target's own $s5 has exactly 2 machine refs (addiu $s5,$sp,0x50 at 0x80037388 plus bne $a2,$s5 at 0x800373E4), i.e. the same 2-textual/3-weighted profile as ours. The target therefore reaches s5 by satisfying L(copy_end) >= L(const), not by carrying a different reference count — which closes the 'the target must have a different ref profile' line of inquiry.

- [s5] Route B (shrink cam from L66 into [34,38] and const from L62 into [L(cam),38], copy_end untouched at L38) reaches the target assignment from the score-10 chassis and has never been attempted in five sessions. It is FORECLOSED from the top-def chassis because copy_end sits at L32, below index's L34.

- [s5] s2's single const-shortening probe (passing a literal 0x80 at the final call) is one spelling of Route B, not the axis: it dropped const's nrefs 3->2 but left livelen at 76 because CSE still held the value.

- [s5] inverse.py's ranked vectors are NOT this function's search space (live-length atoms bounded to +/-{2,4,8}, inverse.py:206-220; every minimal vector needs the unspellable refs 3->1). Its FORECLOSED section does mechanically confirm the s3 G2 kill: no callee-saved copy-preference is reachable from any C, in any chassis.

- [s5] The 2026-07-23 OWNER-ESCALATION (docs/grind/decisions.md:1579) and the 2026-07-27 REFUSED / OWNER-ACCEPTED INCOMPLETE ruling rest on the claim that every grind-advanceable axis is measured dead. That claim is now false: Route B and the sched_solver axis are both live and unmeasured. No new escalation is filed and none is warranted; the function is grindable and the ladder should continue (consistent with the owner's 2026-08-24 escalation-not-parked ruling that returned it to active).

- [s5] tmp/grind/special_camera_get_rot_dir/s5/flowdbg.py drives the BB2_FLOW_DEBUG per-insn liveness hook but its output is TU-wide (bnum is not function-scoped in the harness as written); it needs per-function segmentation before the per-block breakdown is usable. Banked so s6 does not re-discover that.
