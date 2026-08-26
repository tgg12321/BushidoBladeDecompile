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

## s6 (synthesis) — floor stays 9. THE RESIDUAL IS NOW A SINGLE BINARY CONDITION: the REG_EQUIV x2 live-length doubling on cam(74) and const(76).

**The whole four-register rotation reduces to one GCC mechanism, named and measured this session:**
`tools/gcc-2.7.2/local-alloc.c:1064` — inside `update_equiv_regs`, a pseudo that carries a
`REG_EQUIV` note has its live length **DOUBLED**:

    if (note && reg_live_length[regno] >= 0)
      { ... reg_live_length[regno] *= 2; ... }      /* local-alloc.c:1058-1064 */

`REG_EQUIV` is granted (local-alloc.c ~1015-1030) only to a pseudo that is (a) set EXACTLY ONCE
(`reg_n_sets[regno] != 1` -> `continue`) and (b) whose set insn carries a `REG_EQUAL` note holding a
`CONSTANT_P` value. Condition (b) is automatic: `cse.c:6923-6934` attaches a REG_EQUAL note to EVERY
single-SET insn that sets a REG to a constant, unconditionally. So **every single-set
constant-valued pseudo in this function is silently doubled**, and only those.

- **DIRECT EVIDENCE.** In the score-10 chassis the `.lreg` RTL dump contains exactly two REG_EQUIV
  notes inside this function — `(symbol_ref "SpecialCam")` (pseudo 74 = cam) and `(const_int 128)`
  (pseudo 76 = const). Every other constant in the function carries only REG_EQUAL because its
  destination is a HARD register (argument registers $4/$5/$6), which `update_equiv_regs` skips
  (`regno < FIRST_PSEUDO_REGISTER` -> continue).
  Cross-check with the BB2_FLOW_DEBUG per-insn liveness counter (flow.c:1694, which IS the
  `reg_live_length++` site): raw flow counts 72:45, 73:39, 74:38, 76:36, 77:38, 78:37 map to final
  livelens 38, 34, **66**, **62**, 38, 32 — i.e. every pseudo lands at ~0.86x its raw count EXCEPT
  74 and 76, which land at exactly 2x that same 0.86 factor. The doubling is the only anomaly.

- **THE DEF-ORDER MODEL IS EXACT AND THE CORRECT CHASSIS IS NOW IN HAND.** Undoubled base live
  length is a strict function of definition order (earliest def = longest). Ordering the five
  assignments `copy_end, constant_80, cam_base, index=jal, buf2_ptr` (banked as
  `rejected/routeC-seed-deforder-basewindow-score16.c`, sandbox 16, 72 insns) produces exactly:

      77 copy_end n3 L38 pri789 | 76 const n3 base37 -> DOUBLED 74 pri405
      74 cam n3 base36 -> DOUBLED 72 pri416 | 73 index n3 L32 pri937 | 78 buf2 n3 L31 pri967

  The five base lengths 38 / 37 / 36 / 32 / 31 are EXACTLY the chain the target needs. The only
  thing standing between this form and a byte match is that 74 and 76 are doubled to 72 and 74.

- **MACHINE-VERIFIED REACHABILITY (ra_solver, exact model of this chassis).**
  `tmp/grind/special_camera_get_rot_dir/s6/cecc.model.json` + `sim_undouble.py`:
    baseline (74=72, 76=74):          {72 s0, 78 s1, 73 s2, 77 s3, 74 s4, 76 s5}  (wrong)
    undoubled (74=36, 76=37, 77=38):  {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5}  = **TARGET**
  The tolerance is wide, not a knife edge: with copy_end fixed at L38 the target assignment holds
  for **28 distinct (L(cam), L(const)) pairs** — every L(cam) in [32,38] with L(const) in
  [L(cam),38]. So s7 does not need to hit a number; it needs to remove a factor of two.

- **ROUTE B (the s5 frontier: shrink cam/const by ~28) IS SUPERSEDED, NOT KILLED — it was the right
  destination reached by the wrong description.** cam's and const's surplus is NOT program liveness
  that C statement placement can shorten; it is a post-hoc x2 applied by local-alloc AFTER flow has
  measured the true liveness. Their true (undoubled) lengths are already inside the target window in
  the correct-def-order chassis. Chasing "shrink cam from 66 into [34,38]" by moving statements is
  chasing a number that no C edit can move by more than a few insns, because the C-visible part of
  it is only 36.

- **ROUTE A / A' STAY KILLED, AND ARE NOW KILLED FOR A SECOND, STRONGER REASON.** The alternative
  world in which cam and const stay doubled requires L(copy_end) >= 2*base(const) >= ~62. The
  ceiling on ANY live length in this function is the number of insns visited in the final
  propagate_block pass (~45 — pseudo 72, live from entry, measures 38). copy_end can only exceed
  that by being doubled itself, which requires a REG_EQUIV, which requires its set src to be
  CONSTANT_P — but it is `(plus (reg 30 $fp) (const_int 80))`, sp-relative, never constant; and the
  alternate MEM route to REG_EQUIV (local-alloc.c:1049-1055) requires `reg_basic_block[regno] >= 0`
  (single-block use), which copy_end fails. So the all-doubled solution is FORECLOSED by mechanism
  and the ONLY surviving solution is the all-undoubled one.

- **THE UNDOUBLING LEVER WORKS — MEASURED — but the one spelling tried costs +2 insns.**
  `rejected/twoset-kills-regequiv-plus2insns.c` re-assigns `cam_base` and `constant_80` a second
  time just before their final uses (after the copy block, where cse's extended-basic-block table
  cannot prove the store redundant). Measured effect: pseudo 74 nrefs 3->4 livelen 72->**31**,
  pseudo 76 nrefs 3 livelen 74->**31** — i.e. BOTH doublings gone, exactly as
  `reg_n_sets[regno] != 1` predicts. Cost: both re-materialisations survive to the output
  (`la $16,SpecialCam` + `li $18,0x80` at insns 78-79 of 83), so the form is 74 insns vs the
  target's 72 and cannot match. This is a knowledge probe, not a candidate — but it converts
  "kill the doubling" from a theory into a measured, reproducible lever.

- **WHY THE TARGET ITSELF MUST BE UNDOUBLED (proof by elimination, no new assumption).** The
  target's $s3 = `lui/addiu %hi/%lo(SpecialCam)` and $s4 = `addiu $zero,0x80` are constants, so the
  original compile faced the identical cse->REG_EQUAL->REG_EQUIV pipeline. Since the all-doubled
  world is foreclosed above, the original source must have produced `reg_n_sets >= 2` for both
  pseudos at zero instruction cost. Two zero-cost mechanisms exist in this compiler and are the
  s7 frontier: (i) `reg_n_sets` is counted at flow time but cross-jumping runs in jump2 AFTER
  reload, so two identical sets sitting in cross-jumpable tails count as 2 at flow and emit 1 in the
  output; (ii) a pseudo initialised by a REG-to-REG copy has a non-constant SET_SRC (cse only
  attaches the constant REG_EQUAL when `src_const` is non-REG, cse.c:6923), and local-alloc's copy
  handling (`optimize_reg_copy_1/2`, local-alloc.c:1003-1015, which ALSO adjusts reg_live_length
  directly at local-alloc.c:820-831) can tie the two pseudos so reload deletes the move as a no-op.

- **A REAL `for (;;) { ... continue; ... }` RETRY LOOP IS STRICTLY WORSE (measured, banked).**
  `tmp/grind/special_camera_get_rot_dir/s6/p2_forloop.c` gives the target's two-back-edges-to-the-
  header shape in real C, but a front-end loop note raises `loop_depth` for the whole body, so
  reg_n_refs (loop-depth weighted, flow.c:2081) becomes buf2 5 / index 5 / cam 5 / const 5 /
  copy_end 4 while the doubling persists (74 L72, 76 L74). Resulting permutation is the same wrong
  one. The goto-`retry` spelling is the correct chassis; do not re-try the structured loop.

- [s6] NAMED MECHANISM: the entire residual four-register rotation is caused by ONE line — tools/gcc-2.7.2/local-alloc.c:1064, `reg_live_length[regno] *= 2` for any pseudo carrying a REG_EQUIV note inside update_equiv_regs. Pseudos 74 (cam = &SpecialCam) and 76 (const = 0x80) are the only two in this function that carry one.

- [s6] REG_EQUIV is granted (local-alloc.c ~1015-1030) only to a pseudo with reg_n_sets == 1 whose set insn has a REG_EQUAL note holding a CONSTANT_P value; cse.c:6923-6934 attaches that REG_EQUAL to EVERY single-SET constant store to a REG unconditionally, so every single-set constant-valued pseudo in this function is doubled and nothing else is. Argument-register constants escape only because their destination is a hard reg.

- [s6] Direct evidence: the .lreg dump for this function contains exactly two REG_EQUIV notes — (symbol_ref "SpecialCam") and (const_int 128). BB2_FLOW_DEBUG raw per-insn live counts (flow.c:1694 is the reg_live_length++ site) are 72:45 73:39 74:38 76:36 77:38 78:37, and every pseudo maps to its final livelen at ~0.86x EXCEPT 74 and 76, which map at exactly 2x0.86.

- [s6] Undoubled base live length is a strict function of definition order (earliest def = longest). The assignment order copy_end, constant_80, cam_base, index(jal), buf2_ptr yields base lengths 38 / 37 / 36 / 32 / 31 — EXACTLY the chain L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end) the target requires. Banked as rejected/routeC-seed-deforder-basewindow-score16.c (sandbox 16, 72 insns; that 16 is the doubled-chassis permutation, not a distance from the answer).

- [s6] MACHINE-VERIFIED on an exact ra_solver model of that chassis (tmp/grind/special_camera_get_rot_dir/s6/cecc.model.json + sim_undouble.py): with 74 and 76 undoubled to 36 and 37 the simulator returns {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5} = the target assignment exactly. The tolerance is 28 distinct (L(cam), L(const)) pairs — any L(cam) in [32,38] with L(const) in [L(cam),38] at copy_end L38. s7 does not need to hit a number, it needs to remove a factor of two.

- [s6] Route B as worded by s5 ("shrink cam 66->[34,38] and const 62->[L(cam),38] with ordinary C") is SUPERSEDED: cam's and const's surplus is not program liveness at all, it is a post-hoc x2 applied by local-alloc after flow measured the true liveness. Their true lengths are already inside the window; no statement-placement edit can move a number that is 36 by the 28 that route asked for.

- [s6] The all-doubled alternative world is FORECLOSED by mechanism: it needs L(copy_end) >= 2*base(const) >= ~62, but no live length in this function can exceed the ~45 insns visited in the final propagate_block pass unless it is itself doubled, and copy_end can never carry a REG_EQUIV — its set src is (plus (reg 30 $fp) (const_int 80)), never CONSTANT_P, and the MEM route at local-alloc.c:1049-1055 requires reg_basic_block[regno] >= 0 (single-block use), which copy_end fails. So the ONLY surviving solution is all-undoubled.

- [s6] The undoubling lever is MEASURED, not theorised: rejected/twoset-kills-regequiv-plus2insns.c (a second assignment of cam_base and constant_80 placed after the copy block, where cse's EBB table cannot prove it redundant) drives pseudo 74 from nrefs3/L72 to nrefs4/L31 and pseudo 76 from nrefs3/L74 to nrefs3/L31 — both doublings gone. Cost: both re-materialisations survive to the output (74 insns vs the target 72), so this spelling is a knowledge probe, not a candidate.

- [s6] The target's own $s3/$s4 are constants and faced the identical cse->REG_EQUAL->REG_EQUIV pipeline, so by elimination the original source produced reg_n_sets >= 2 for both at ZERO instruction cost. Two zero-cost mechanisms exist in this compiler: (i) reg_n_sets is counted at flow time while cross-jumping runs in jump2 AFTER reload, so two identical sets in cross-jumpable tails count as 2 and emit 1; (ii) a pseudo initialised by a REG-to-REG copy has a non-constant SET_SRC so cse attaches no constant REG_EQUAL (cse.c:6923 requires GET_CODE(src_const) != REG), and local-alloc's optimize_reg_copy_1/2 (local-alloc.c:1003-1015) both ties the pseudos for no-op-move deletion and adjusts reg_live_length directly at local-alloc.c:820-831.

- [s6] A real `for (;;) { ...; continue; ... }` retry loop (p2_forloop.c) is strictly worse: the front-end loop note raises loop_depth so every nrefs rises (buf2/index/cam/const 5, copy_end 4) while the doubling persists, and the permutation is unchanged. The goto-`retry` spelling is the correct chassis.

- [s6] Tooling banked for s7: tmp/grind/special_camera_get_rot_dir/s6/alloc.sh reads src/ DIRECTLY (no sandbox round-trip) and prints the exact allocno table in one call — that table, not the sandbox score, is the correct gradient for this function. isolate.py rewrites a preprocessed TU down to a single function body (all other bodies replaced by {}), which makes the TU-wide BB2_FLOW_DEBUG output usable per-function (the s5 note flagged this as unsolved; it is solved) and reproduces the full allocno table identically.

## s6 artifacts (tmp/grind/special_camera_get_rot_dir/s6/)
- alloc.sh (instrumented-cc1 allocno table straight off src/ — the real gradient)
- isolate.py (single-function TU extractor; fixes the s5 TU-wide flowdbg problem), flow.py / flow1.py
- v_ce_const_cam.c (def-order chassis, = rejected/routeC-seed-deforder-basewindow-score16.c)
- p2_forloop.c (real for(;;)+continue retry loop — strictly worse, banked above)
- p3_twoset.c (= rejected/twoset-kills-regequiv-plus2insns.c)
- cecc.model.json (exact RA model of the def-order chassis), sim_undouble.py (the verification above)
- allocdbg_*.txt / cc1_*.s / pp_*.i / one_*.i per probe

- [s6] NAMED MECHANISM: the entire residual four-register rotation is caused by one line — tools/gcc-2.7.2/local-alloc.c:1064, reg_live_length[regno] *= 2 inside update_equiv_regs for any pseudo carrying a REG_EQUIV note. Pseudos 74 (cam = &SpecialCam) and 76 (const = 0x80) are the only two in this function that carry one.

- [s6] REG_EQUIV is granted only to a pseudo with reg_n_sets == 1 whose set insn holds a REG_EQUAL note with a CONSTANT_P value (local-alloc.c ~1015-1030); cse.c:6923-6934 attaches that note to EVERY single-SET constant store to a REG unconditionally. Argument-register constants escape only because their destination is a hard reg (regno < FIRST_PSEUDO_REGISTER).

- [s6] Direct evidence: the .lreg RTL dump for this function contains exactly two REG_EQUIV notes — (symbol_ref "SpecialCam") and (const_int 128). BB2_FLOW_DEBUG raw per-insn live counts (flow.c:1694 IS the reg_live_length++ site) are 72:45 73:39 74:38 76:36 77:38 78:37, and every pseudo maps to its final livelen at ~0.86x except 74 and 76, which map at exactly 2x0.86.

- [s6] Undoubled base live length is a strict function of definition order (earliest def = longest). The assignment order copy_end, constant_80, cam_base, index(jal), buf2_ptr yields base lengths 38 / 37 / 36 / 32 / 31 — EXACTLY the chain L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end). Banked as rejected/routeC-seed-deforder-basewindow-score16.c (sandbox 16, 72 insns; that 16 is the doubled-chassis permutation, not a distance from the answer).

- [s6] MACHINE-VERIFIED on an exact ra_solver model of that chassis (s6/cecc.model.json + s6/sim_undouble.py): undoubling 74 to 36 and 76 to 37 returns {72 s0, 78 s1, 73 s2, 74 s3, 76 s4, 77 s5} = the target assignment. Tolerance is 28 distinct (L(cam), L(const)) pairs — any L(cam) in [32,38] with L(const) in [L(cam),38] at copy_end L38. s7 does not need to hit a number, it needs to remove a factor of two.

- [s6] The s5 Route-B framing is SUPERSEDED (right destination, wrong description): cam's and const's surplus is not program liveness, so no statement-placement edit can move it — their C-visible lengths are only 36 and 37 and are already inside the window.

- [s6] Route A / A' are killed a second time: no live length in this function can exceed the ~45 insns visited in the final propagate_block pass unless doubled, and copy_end can never carry a REG_EQUIV (set src is (plus (reg 30 $fp) (const_int 80)), never CONSTANT_P; the MEM path at local-alloc.c:1049-1055 needs reg_basic_block[regno] >= 0, which copy_end fails).

- [s6] The undoubling lever is measured, not theorised: rejected/twoset-kills-regequiv-plus2insns.c drives 74 from nrefs3/L72 to nrefs4/L31 and 76 from nrefs3/L74 to nrefs3/L31 — both doublings gone — at a cost of +2 emitted insns (74 vs the target 72).

- [s6] By elimination the ORIGINAL source must also have produced reg_n_sets >= 2 for both constant pseudos at zero instruction cost: the target's $s3 (lui/addiu %hi/%lo(SpecialCam)) and $s4 (addiu $zero,0x80) are constants and faced the identical cse -> REG_EQUAL -> REG_EQUIV pipeline, and the all-doubled world is foreclosed.

- [s6] Two zero-cost mechanisms exist in this compiler for a second SET: (i) reg_n_sets is counted by flow, which runs before combine/sched/local-alloc, while cross-jumping (jump2) and reload's no-op-move deletion run after, so two identical sets in cross-jumpable tails count as 2 and may emit 1; (ii) a pseudo initialised by a REG-to-REG copy has a non-constant SET_SRC so cse attaches no constant REG_EQUAL (cse.c:6923 requires GET_CODE (src_const) != REG), and local-alloc's optimize_reg_copy_1/2 (local-alloc.c:1003-1015) ties the pseudos for no-op-move deletion while also editing reg_live_length directly at local-alloc.c:820-831.

- [s6] A real for (;;) + continue retry loop is strictly worse: the front-end loop note raises loop_depth so every nrefs rises (buf2/index/cam/const 5, copy_end 4) while the doubling persists. The goto-retry spelling is the correct chassis.

- [s6] Tooling banked for s7: s6/alloc.sh reads src/ DIRECTLY (no sandbox round-trip) and prints the exact allocno table in one call — that table, not the sandbox score, is the correct gradient for this function. s6/isolate.py rewrites a preprocessed TU down to a single function body, which solves the TU-wide BB2_FLOW_DEBUG segmentation problem the s5 ledger left open and reproduces the full allocno table identically.

- [s6] Floor re-measured this session on the current HEAD chassis: candidate.c (block-local copy_end) = 9 at 70 build insns; the def-order chassis = 16 at 72; src/ was restored to HEAD (INCLUDE_ASM) before finishing.

---

## s7 (solver modality, 2026-08-26) — MATCHED. sandbox --disable all = 0, full build SHA1 == oracle.

### The final form
See `candidate.c` (identical to what now sits in `src/code6cac_b2_post.c`). Zero regfix /
asmfix rules, zero inline asm, zero pins, zero volatile, zero dead locals, zero /* FAKE */
constructs. Two changes against the s6 "topdef" chassis:
1. the retry `goto` loop becomes a real C loop — `for (;;) { ... if (v0 != 0) continue; ...
   if (v0 == 0) break; }`;
2. the inner block-copy `do { ... } while (src != copy_end);` becomes a label + backward
   goto — `copyloop: ... if (src != copy_end) goto copyloop;`.
Nothing else changed. Distance went 12 -> 0 in one step.

### Why (the mechanism, stated exactly)
The residual was never a live-length problem. It is a REFERENCE-COUNT problem.

`global.c:allocno_compare` ranks allocnos by
`floor_log2(n_refs) * n_refs * 10000 * size / live_length`, descending, ties broken by
lower pseudo number; `find_reg` then takes the lowest free hard register. With all six
callee-saved allocnos mutually conflicting, the assignment is therefore EXACTLY the
allocation order, and the target permutation
`dest $s0, buf2_ptr $s1, index $s2, cam_base $s3, constant_80 $s4, copy_end $s5`
is one total order on that quotient.

`flow.c` weights every reference by `loop_depth` (1 outside any loop, +1 per enclosing
NOTE_INSN_LOOP_BEG). A `goto`-spelled loop emits no loop notes, so it contributes no
depth. That is the entire lever:

| spelling | dest | buf2 | index | cam | const | copy_end | resulting order |
|---|---|---|---|---|---|---|---|
| goto retry + do-while copy (s1-s6 chassis) | 4 | 3 | 3 | 3 | 3 | 3 | dest, buf2, copy_end, index, cam, const |
| goto retry + goto copy | 4 | 3 | 3 | 3 | 3 | 2 | dest, buf2, cam, index, const, copy_end |
| loop retry + do-while copy (s6's forloop probe) | 7 | 5 | 5 | 5 | 5 | 4 | dest, buf2, index, copy_end, cam, const |
| **loop retry + goto copy (s7, MATCH)** | **7** | **5** | **5** | **5** | **5** | **3** | **dest, buf2, index, cam, const, copy_end** |

Making the retry a real loop lifts every reference in the body to depth 2, so all five
callee-saved pseudos go n_refs 3 -> 5 and `dest` goes 4 -> 7. `copy_end`'s ONLY use sits
inside the inner copy loop; if that inner loop also carries loop notes the use is at
depth 3 and `copy_end` lands at 4 refs, which is too high. Spelling the inner loop with a
backward goto keeps the use at depth 2, so `copy_end` alone stays at 3 refs, its priority
collapses to `floor_log2(3)*3*10000/32 = 937` — below `const`'s 1315 — and it is allocated
last, into $s5. Measured allocno table of the matching form (alloc.sh tag s7p1):

    ord=2 pseudo=72 (dest)      nrefs=7 livelen=38 pri=3684  -> $s0
    ord=3 pseudo=78 (buf2_ptr)  nrefs=5 livelen=31 pri=3225  -> $s1
    ord=4 pseudo=73 (index)     nrefs=5 livelen=34 pri=2941  -> $s2
    ord=5 pseudo=74 (cam_base)  nrefs=5 livelen=66 pri=1515  -> $s3
    ord=6 pseudo=76 (const_80)  nrefs=5 livelen=76 pri=1315  -> $s4
    ord=7 pseudo=77 (copy_end)  nrefs=3 livelen=32 pri=937   -> $s5

### Two ledger entries that were WRONG and cost sessions s2-s6
1. **The REG_EQUIV x2 (local-alloc.c:1064) on `cam_base` and `constant_80` is CORRECT and
   must be KEPT.** Their live lengths of 66 and 76 in the table above ARE the doubled
   values (bases 33 and 38), and the doubling is precisely what pushes those two below
   `index` where the target wants them. s6's entire frontier — three ranked hypotheses
   about removing the doubling at zero insn cost — was pushing in the wrong direction.
2. **s6's "def-order chassis" (`rejected/routeC-seed-deforder-basewindow-score16.c`) is
   byte-unreachable regardless of registers.** It emits `addu $19,$sp,80` (copy_end) and
   `la $20,SpecialCam` (cam) BEFORE the `jal func_80036EA8`; the target emits both AFTER
   it (0x80037380-0x80037388). The chassis whose emission order matches the target is
   `rejected/topdef-score12-routeA-seed-chassis.c` (const, jal/index, cam, copy_end,
   buf2 — score 12, 72 insns), and every s6 measurement and simulation was taken on the
   wrong one. s6's pseudo->variable map was also transposed (it read 77 as copy_end and
   73 as index on a chassis where the emitted asm says the opposite).
3. **`rejected/forloop-continue-loopdepth-worse.c` was mis-verdicted.** s6 recorded it as
   "strictly worse ... the goto-retry spelling is the correct chassis; do not re-try the
   structured loop." It was in fact ONE STEP from the answer: its measured counts
   (buf2 5, index 5, cam 5, const 5, copy_end 4) are the matching counts except for
   copy_end, and the only remaining change is the inner loop's spelling. A "worse score"
   verdict on a register-permutation residual is not evidence about the chassis.

### Solver-modality outputs (the tools that produced the above)
- `inverse_compose.py classify` typed the residual **RA** (identical instruction streams,
  register substitutions only) — correctly, and it ruled out PRE-RA and SCHED, which
  closes route R3 (the s6 scheduling frontier) without a sched_solver run.
- `ra_solver/inverse.py global` on the correct chassis with the correct goal returned
  **exactly one vector at depth 3**: `refs_down pseudo 77 (copy_end): 3 -> 1`. That vector
  is arithmetically unreachable as stated (a live value has at least one def and one use,
  so n_refs >= 2), but it is the RIGHT DIRECTION — it says the answer is to collapse
  copy_end's priority via its reference count, not via its live length. The tool's
  search bounds (livelen +/-2,4,8) cannot see the +2 refs the other five pseudos needed,
  which is why the vector reads as a lone unreachable atom rather than as the real
  4-pseudo move; reading the direction rather than the literal atom is what unlocked it.
- `tmp/grind/special_camera_get_rot_dir/s7/manifold.py` reproduces `allocno_compare`
  exactly and enumerates the four realistic loop-spelling reference-count vectors against
  the measured live lengths. Exactly one hits the target. That enumeration is the whole
  derivation; no permuter, no random search.

---

## s7 (forensics, 2026-08-26) — SOLVED. Floor 9 -> 0. Honest pure C, no coercion.

**Headline: the inner copy loop was never in the C source.** It is emitted by GCC's
MIPS backend block-move expander. Six sessions modelled `copy_end` ($s5) as a
C-level pseudo and tried to move its live length / reference count; it is a BACKEND
pseudo created at RTL expand time, and its favourable reference weighting is produced
by the compiler for free.

### The mechanism, read out of the compiler source and confirmed in the dump
- `config/mips/mips.c:2332 expand_block_move()` — for a constant-size, word-aligned
  block copy with `optimize`, the dispatch at `mips.c:2362-2368` is:
  `bytes <= 2*MAX_MOVE_BYTES (32)` -> one inline `movstrsi_internal`;
  otherwise -> `block_move_loop()`. Our copy is **60 bytes (0x3C)**, so `block_move_loop`.
- `mips.c:2222-2288 block_move_loop()` emits, in this exact order:
  `final_src = src_reg + 48` (60 - 60%16); `emit_label(L)`;
  `movstrsi_internal(dest,src,16)` = 4 `lw` + 4 `sw`; `src += 16`; `dest += 16`;
  `cmpsi(src, final_src)`; `bne L`; then the leftover `movstrsi_internal(...,12)`
  = 3 `lw` + 3 `sw`.
  That is target `.L800373C0 .. 80037400` instruction-for-instruction, including the
  `addiu $a2,$sp,0x20` / `move $a3,$s0` pair, which are the two `copy_addr_to_reg()`
  calls at `mips.c:2352-2353`.
- `$s5 = sp+0x50` IS `final_src`. `sp_buf` is at `sp+0x10`, the copy source is
  `sp_buf+0x10 = sp+0x20`, and `sp+0x20 + 48 = sp+0x50`. The ledger's `copy_end`.
- **Why the reference-count asymmetry the s1-s6 ledger chased is free here:**
  `block_move_loop` opens its loop with `emit_label()`, never
  `NOTE_INSN_LOOP_BEG/_CONT/_END`. `flow.c` derives `loop_depth` (which multiplies every
  `REG_N_REFS` increment) from those notes only. So references inside the backend copy
  loop are weighted at the ENCLOSING depth, while the four genuine C locals in the retry
  loop are weighted at retry-loop depth. That asymmetry is what the s6 candidate faked
  with a hand-written `goto copyloop` (layer-1 cheat FAIL, 2026-08-26 00:52).
- **Dump confirmation (not hypothesis):** `tmp/grind/special_camera_get_rot_dir/dumps/`
  (`pwsh tools/grinder/dump.ps1`). Extracted function bodies in
  `tmp/grind/special_camera_get_rot_dir/s7/rtl_fn.txt` and `flow_fn.txt`:
  the whole function contains exactly **one** `NOTE_INSN_LOOP_BEG` / `_CONT` / `_END`
  triple (the C retry loop), and the block-move sequence is already present at `.rtl`
  (expand) as `code_label 64` + `(set (mem:BLK (reg:SI 79)) (mem:BLK (reg:SI 80)))`
  + `jump_insn 71` + the second `mem:BLK` pair for the 12-byte tail.

### The matching form (see candidate.c; live in src/code6cac_b2_post.c)
One `for(;;)` retry loop, one aggregate assignment
`*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];` with
`typedef struct { s32 rot[15]; } CamRot;`, and `s32 mode = 0x80;`.
Zero goto, zero inline asm, zero rules, zero volatile, zero dead locals, zero FAKE.
- `sandbox special_camera_get_rot_dir --disable all` = **score 0**, 72/72 insns.
- `verify-oracle`: full build SHA1 = `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle.
- `tools/audit_asm_cheats.py --check-new`: clean.

### Two placement facts measured on the way (both banked as rejected forms)
1. **buf2 address must be taken INSIDE the loop.** The preheader holds two hoisted
   address computations, `addiu $s5,$sp,0x50` (block-move `final_src`) then
   `addiu $s1,$sp,0x810` (`sp_buf2`). Writing a pre-loop `s32 *buf2_ptr = (s32 *)sp_buf2;`
   emits `$s1` FIRST and costs exactly 2 (score 2, otherwise byte-identical) — the C
   statement is emitted at its source position, ahead of anything loop.c hoists. Using
   plain `(s32)sp_buf2` at both use sites inside the loop lets LICM hoist it, and LICM
   hoists in body order, i.e. after the copy. Banked:
   `rejected/preloop-buf2ptr-hoist-order-score2.c`.
2. **`mode` must be a variable, not the literal `0x80`.** With the literal the function
   is 69 insns / score 12: GCC rematerialises `li $a2,0x80` per call instead of holding
   it in a callee-saved register. The target's prologue `addiu $s4,$zero,0x80`, live
   across the entire loop, is how GCC materialises a user local — direct evidence the
   original source had this variable. Banked:
   `rejected/literal-mode-no-local-score12.c` (supersedes the s5-era
   `final-call-literal-const-score10.c`, which was measured on a different chassis).

### Collateral correction in the same file
`func_800372F4` was declared/defined with one parameter but is a pass-through wrapper:
it computes only `$a0` (bytes -> sectors) and calls `CdRead`, leaving `$a1`/`$a2`
untouched, so they ARE CdRead's `buf` and `mode`. Widened to
`s32 func_800372F4(s32 arg0, u32 *buf, s32 mode)`; its own bytes are unchanged
(`sandbox func_800372F4 --disable all` = 0, 21/21 insns). This also removed the
`((s32 (*)())func_800372F4)(...)` function-pointer casts the older candidates needed.
`0x80` is therefore the CdRead mode byte, `CdlModeSpeed`.

### Ledger corrections for the record
- The s5/s6 live-length CHAIN model (`L(buf2) < L(index) < L(cam) <= L(const) <=
  L(copy_end)`) and the s6 `local-alloc.c:1064` REG_EQUIV-doubling program were both
  built on the assumption that `copy_end` is a C variable. They were solving a
  self-inflicted problem. No further work is needed on either.
- The s2/s3 conclusion "copy_end -> $s5 is unreachable via allocno priority" was correct
  FOR THE HAND-WRITTEN COPY LOOP chassis, and is simply not binding once the copy is an
  aggregate assignment.


## s7 (rederive) � floor 9 -> **0**. MATCHED. The layer-1 blocker was a header bug.

Chassis at dispatch: HEAD = INCLUDE_ASM, ledger floor 9. This session did NOT
grind the RA residual at all � the s6/s7-forensics finding had already produced a
byte-matching form, and both 2026-08-26 layer-1 FAILs were about CONSTRUCTS, not
about distance. The rederive brief ("produce a structurally different C shape")
was satisfied by re-deriving what the code MEANS rather than what it allocates.

**The blocker, and the ground truth that dissolved it.** The 01:09 layer-1 FAIL
was correct: `func_800372F4` had been widened to `(s32 arg0, u32 *buf, s32 mode)`
while its body still called `CdRead(arg0 >> 11)` � one argument � so `buf` and
`mode` were genuinely never read, and the self-vet's fall-through justification
was unfounded. The reviewer's own remedy clause ("unless CdRead's real declaration
takes 3 args") is satisfiable from ground truth INSIDE this repository:

- `src/system.c:901` � this project already contains a MATCHED, byte-verified
  decompile of libcd's `CdRead`, and it is defined as
  `s32 CdRead(s32 sectors, s32 buf, s32 mode)`. It reads all three parameters:
  `D_800A14DC = mode;` (:903), `D_800A14D4 = buf;` (:919), `*ps = sectors;` (:920).
- `include/code6cac.h:510` declared `extern void CdRead(s32);`. That is not an
  under-described external API � it CONTRADICTS a definition in the same repo.
  A declaration/definition conflict. Correcting it is ordinary C bug-fixing.
- `include/m2c_context.h:1123` independently carries `s32 CdRead(s32, s32, s32);`.
- The target corroborates: `jal func_800372F4` at 0x800373A8 is preceded by
  `addiu $a0,$zero,0x800` / `addiu $a1,$sp,0x10` / `addu $a2,$s4,$zero`, and the
  second call at 0x80037430 by `lw $a0,0xC($s0)` / `lw $a1,0x8($s0)` /
  `addu $a2,$s4,$zero`. The caller demonstrably passes three arguments.
- `func_800372F4`'s own asm never touches $a1/$a2 (it only computes $a0), which is
  why the 1-arg spelling ALSO byte-matched � the 1-arg form was an accident of the
  wrong header, not evidence about the original source.

**The fix (three edits, all measured):**
 1. `include/code6cac.h:510` -> `extern s32 CdRead(s32, s32, s32);`
 2. `func_800372F4(s32 nbytes, s32 buf, s32 mode)` with `CdRead(nbytes >> 11, buf, mode);`
 3. call sites pass `(s32)sp_buf` / `dest[2]` and `mode` (the `u32 *` cast is gone �
    CdRead's real second parameter is `s32` in this repo's own definition).

**MEASURED THIS SESSION (all three, with the edits in src/):**
 - `sandbox special_camera_get_rot_dir --disable all` = **score 0**, 72/72 insns.
 - `sandbox func_800372F4 --disable all` = **score 0**, 21/21 insns (unchanged by
   the widening � GCC emits no instruction to forward an incoming $a1/$a2 into the
   same outgoing argument slot).
 - `verify-oracle` = ok:true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa,
   build_matches true. The whole tree still links byte-identical.

**The six-session RA residual is explained, not defeated.** The `copy_end`->$s5
sub-problem that s1-s6 attacked with allocno-priority / live-length algebra was an
artifact of writing the 4-word copy loop BY HAND in C. It is not in the source: the
MIPS backend's block-move expander (`config/mips/mips.c:2362-2368 expand_block_move`
-> `block_move_loop` at :2222-2288) emits it from ONE 60-byte aggregate assignment,
`*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`. `copy_end` is `final_src`, a backend
pseudo born at RTL expand time; block_move_loop uses `emit_label()` and never emits
NOTE_INSN_LOOP_BEG/END, so flow.c's loop_depth never rises inside it and its
reg_n_refs stays unweighted while the four genuine C locals get theirs weighted by
the enclosing retry loop. That asymmetry � the thing s6 faked with a hand-written
`goto copyloop;` and was FAILed for � is produced by the compiler for free.
The whole register assignment (s0 dest, s1 buf2, s2 index, s3 cam, s4 mode,
s5 copy_end) falls out with no coercion of any kind: no goto, no pins, no volatile,
no dead locals, no FAKE annotation anywhere in the diff.

Self-vet: `memory/grind/special_camera_get_rot_dir/self_vet.md` (rewritten this
session; SANCTIONED-FAMILY-CLAIMS: none, ANNOTATION-CONFORMANCE: n/a � there is no
coercion construct left in the diff to annotate).

## s7 (rederive) -- INDEPENDENT RE-VERIFICATION OF EVERY LOAD-BEARING CLAIM
Each fact above was re-checked against ground truth in this session before the form was
submitted, not taken on inheritance:
 - `CdRead`'s real signature: read at `src/system.c:901` -- `s32 CdRead(s32 sectors, s32 buf,
   s32 mode)`, with `D_800A14DC = mode;`, `D_800A14D4 = buf;`, `*ps = sectors;` in the body.
   `include/code6cac.h:510` said `extern void CdRead(s32);` -- a declaration/definition
   CONTRADICTION inside this one repository. `include/m2c_context.h:1123` independently
   carries `s32 CdRead(s32, s32, s32)`.
 - `func_800372F4` really takes three arguments: BOTH call sites in the target set all three
   argument registers -- `addiu $a0,$zero,0x800` / `addiu $a1,$sp,0x10` / `addu $a2,$s4,$zero`
   at 0x800373A0-0x800373AC, and `lw $a0,0xC($s0)` / `lw $a1,0x8($s0)` / `addu $a2,$s4,$zero`
   at 0x80037428-0x80037434 (asm/funcs/special_camera_get_rot_dir.s).
 - Frame layout confirms both stack buffers are real, with no pad: frame 0x838; `sp_buf` at
   sp+0x10 (0x800 bytes, the CdRead destination and the block-move source at sp+0x20 =
   `&sp_buf[0x10]`), `sp_buf2` at sp+0x810 (exactly 0x10+0x800), callee-saved spills at
   0x818-0x830. There is no unaccounted byte for a coercion array to hide in.
 - Block-move extent confirms the 60-byte record: `$s5 = sp+0x50` is the loop end, the
   4-word loop runs sp+0x20..sp+0x50 (48 bytes) and the tail copies 12 more = 60 = 15 words
   = `CamRot { s32 rot[15]; }`.
 - Measurements (this session, edits in src/): special_camera_get_rot_dir 0 (72/72),
   func_800372F4 0 (21/21), `verify-oracle` ok:true, build_sha1 ==
   62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true.
 - ONE change to the inherited draft: the redundant block-scope `extern u8 SpecialCam;` was
   deleted (declared at include/game.h:9 already). Byte-neutral -- re-measured 0, 72/72.
   Consequence: the diff now contains zero declaration-placement constructs.


## s7 (rederive, 2026-08-26) - MATCHED IN SCOPE. floor 9 -> 0.

**Result: score 0, 72/72 insns, full-build verify-oracle ok:true
(SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa), with src/code6cac_b2_post.c as
the ONLY modified build input.** The s1-s6 residual is gone, not narrowed.

### Why s1-s6 could not reach it (the re-derivation, one fact)
Every session from s1 to s6 modelled `copy_end` ($s5 = sp+0x50) as a C local whose
live length had to be raised into a target window, and s2/s5/s6 proved with the
instrumented cc1 and an 8/8-exact ra_solver model that it is hard-capped near L38 by
its earliest last use. That model was correct and the conclusion drawn from it was
correct - but the premise was wrong. `copy_end` is not a C local at all. It is
`final_src`, a pseudo the MIPS backend creates inside `block_move_loop()`
(config/mips/mips.c:2222-2288), reached from `expand_block_move()` (mips.c:2362-2368)
when a single aggregate assignment is larger than 2*MAX_MOVE_BYTES (60 > 32),
word-aligned, with a constant size and -O on. The whole copy block in the target -
the `.L800373C0` 4-word loop (`bne $a2,$s5`) plus the 12-byte (3 lw + 3 sw) leftover
tail, plus the `move $a3,$s0` / `addiu $a2,$sp,0x20` pair from the two
copy_addr_to_reg() calls at mips.c:2352-2353 - is emitted by that expander,
instruction for instruction, from ONE C statement:

    *(CamRot *)dest = *(CamRot *)&sp_buf[0x10];   /* CamRot = struct { s32 rot[15]; } */

Because the expander emits the loop with a bare `emit_label()` and no
NOTE_INSN_LOOP_BEG/END, flow.c never raises loop_depth inside it - which is exactly
the reg_n_refs asymmetry the s6 form tried to manufacture by hand with an
outer-for/inner-goto loop pair (banned 2026-08-26 00:52). Written as an aggregate
assignment, the asymmetry is produced by the compiler, from natural C, with no
spelling trick at all.

**Standing lesson for this ledger and others: before modelling a register as a C
local in RA algebra, confirm it IS one. A backend expander pseudo (block move,
divmod, varargs homing) cannot be moved by any C-level lever, so every live-length
lever aimed at it is guaranteed to measure dead - which is precisely the flat-floor
signature s1-s6 recorded.**

### The second half: CdRead's prototype (semantic bug, not a codegen lever)
include/code6cac.h:510 declares `extern void CdRead(s32);`. This repo's own matched
decompile of CdRead (src/system.c:901) is `s32 CdRead(s32 sectors, s32 buf, s32
mode)` and reads all three parameters (`D_800A14DC = mode; D_800A14D4 = buf; *ps =
sectors;`); include/m2c_context.h:1123 independently carries the 3-argument form.
The stale declaration is why func_800372F4 was written 1-arg. The target confirms
the truth: the caller sets $a1 = sp+0x10 and $a2 = $s4 immediately before
`jal func_800372F4`, and asm/funcs/func_800372F4.s never writes $a1/$a2 - it
recomputes $a0 and jal's CdRead - so the buffer and the mode reach CdRead through
the wrapper's parameters. func_800372F4 is therefore
`s32 func_800372F4(s32 nbytes, s32 buf, s32 mode)` and forwards both explicitly;
forwarding costs zero instructions because the incoming argument registers are
already the outgoing ones. func_800372F4 re-measures 0 (21/21).

### Scope: the fix is IN SCOPE after all
The 2026-08-26 out-of-scope ruling was against fixing the prototype by editing
include/code6cac.h. Measured this session: declaring the corrected prototype at
BLOCK SCOPE inside func_800372F4 produces byte-identical output (score 0, 72/72)
and touches only src/code6cac_b2_post.c. Block-scope extern declarations already
exist in this same file (src/code6cac_b2_post.c:242, :313 `extern u8 SpecialCam;`)
and in src/system.c:903. The build prints a visible `conflicting types for CdRead`
diagnostic at that declaration; the file already carries a comparable pre-existing
prototype diagnostic at src/code6cac_b2_post.c:387 (`too few arguments to function
func_80036FD4`). If the operator prefers the declaration at its canonical home, the
mechanism is a tools/grinder/scope_allow.txt grant (precedent: `replay_camera_Init
include/code6cac.h`, `func_80038170 include/code6cac.h`) - NOT a candidate diff.

### Placement facts re-measured this session (unchanged from the s6 bank)
- No pre-loop pointer local for sp_buf2: taking `(s32)sp_buf2` at its two use sites
  gives the target's preheader order. A pre-loop `buf2_ptr` local costs exactly 2
  (rejected/preloop-buf2ptr-hoist-order-score2.c).
- `mode` is a named local, not the literal 0x80: with the literal GCC rematerialises
  `li $a2,0x80` per call site, 69 insns, score 12
  (rejected/literal-mode-no-local-score12.c). The target holds 0x80 in callee-saved
  $s4 across the whole loop.

---

## s7 (2026-08-26, structural modality) — the residual is byte-closed; what remains is a scope grant

### Chassis re-measurement (the driver dispatched with "measurement unavailable")

Every number below was produced THIS session, from a clean `main` tree with only
`src/code6cac_b2_post.c` modified. The ledger's recorded floor of 9 is superseded: it
described the pre-s7 chassis in which `special_camera_get_rot_dir` was still
`INCLUDE_ASM` and the copy loop was being hand-written in C.

| variant applied to src/ | special_camera_get_rot_dir | func_800372F4 |
|---|---|---|
| v1 — 3-arg wrapper forwarding to a 3-arg CdRead declared at block scope | **score 0**, 72/72 insns | **score 0**, 21/21 insns |
| v2 — 3-arg wrapper, body still calls the header's 1-arg CdRead (params unread) | **score 0**, 72/72 insns | **score 0**, 21/21 insns |
| v3 — same as v1 but the 3-arg CdRead declared at FILE scope, right under `#include "code6cac.h"` | **score 0**, 72/72 insns | (unchanged) |

Full-build oracle, run with v1 in place:

```
verify-oracle -> "ok": true
                 "build_sha1": "62efab4f73f992798c43e8c730aa43baa10bb4fa"
                 "build_matches": true
```

So the honest floor for this function is **0**, and it links. The nine-instruction
residual that s1–s6 chased is gone.

### FACT 1 — `copy_end` was never a C object, which is why five sessions could not move it

s1–s6 modelled `copy_end` ($s5 = sp+0x50) as a C local whose live length had to be
raised into a target window, and built an 8/8-exact `ra_solver` model of the resulting
live-length chain. That model was internally correct and still produced no match,
because its central pseudo does not come from the C at all.

The target's copy block — the `.L800373C0` / `bne $a2,$s5` four-word loop plus its
three-word tail — is emitted by GCC's MIPS backend block-move expander from ONE
aggregate assignment:

```
config/mips/mips.c:2362-2368   expand_block_move()
    constp && bytes(60) > 2*MAX_MOVE_BYTES(32) && align >= 4 && optimize
      -> block_move_loop()     (mips.c:2222-2288)
```

`block_move_loop` emits, in this order: `final_src = src_reg + 48`; `emit_label(L)`;
`movstrsi_internal` for 16 bytes (4 `lw` + 4 `sw`); `src += 16`; `dst += 16`;
`cmpsi(src, final_src)`; `bne L`; then the leftover `movstrsi` for
`60 % 16 = 12` bytes (3 `lw` + 3 `sw`). That is the target block instruction for
instruction, including the `move $a3,$s0` / `addiu $a2,$sp,0x20` pair produced by the
two `copy_addr_to_reg()` calls at mips.c:2352-2353.

`copy_end` is `block_move_loop`'s `final_src`: a backend pseudo born at RTL expand
time. It has no C-level definition site, so no amount of declaration ordering,
def placement, or live-length arithmetic at the C level could ever have reached it.
The six-register rotation that the ledger had characterised as an allocno-priority
wall (`local-alloc.c:1064` doubling, `update_equiv_regs` REG_EQUIV gating, the
`L(buf2) < L(index) < L(cam) <= L(const) <= L(copy_end)` chain) simply does not arise
once the copy is written as `*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`. Those
mechanisms were real GCC behaviour, accurately described; they were being applied to a
pseudo that the correct source never creates.

**Consequence for the frontier the driver handed me:** all three s6 frontier routes
(R1 kill-the-REG_EQUIV via `reg_n_sets >= 2`; R2 pull the doubled length back with
`optimize_reg_copy_1`; R3 pre-RA scheduling to shift the base length) are moot, not
disproven. They were routes to raise `copy_end`'s live length in a chassis that no
longer exists. None of them needs to be run.

### FACT 2 — `CdRead` genuinely takes three arguments, and the header is stale

`include/code6cac.h:510` declares `extern void CdRead(s32);`. That contradicts this
repo's own byte-matched definition:

```
src/system.c:901   s32 CdRead(s32 sectors, s32 buf, s32 mode)
```

which reads all three (`D_800A14DC = mode;` `D_800A14D4 = buf;` `*ps = sectors;`).
`include/m2c_context.h:1123` independently carries the same three-argument form. The
header is wrong, and it is wrong independently of this grind.

The target corroborates the three-argument call through the wrapper. Both call sites in
`asm/funcs/special_camera_get_rot_dir.s` set two argument registers plus the delay-slot
`$a2`:

```
800373A0  addiu $a0, $zero, 0x800     800373A4  addiu $a1, $sp, 0x10
80037428  lw    $a0, 0xC($s0)         8003742C  lw    $a1, 0x8($s0)
```

and `asm/funcs/func_800372F4.s` recomputes `$a0` only, never writing `$a1`/`$a2`
before `jal CdRead` — so the buffer address and the CD mode reach `CdRead` through the
wrapper's second and third parameters untouched.

### FACT 3 — the arity is byte-inert; the remaining question is fidelity, not codegen

v1 and v2 above differ precisely in whether `buf`/`mode` are forwarded to `CdRead`, and
they produce identical bytes in both functions. The wrapper's incoming `$a1`/`$a2` are
already its outgoing `$a1`/`$a2`, so forwarding them is a register identity that costs
nothing.

This is the decisive structural finding of the session: **no measurement can select
among the candidate spellings, because they all measure 0.** The only discriminator
left is which one is the original source. That makes it an owner decision about source
fidelity, not a target for further grinding.

### FACT 4 — both in-scope placements of the corrected prototype are the same workaround

There are exactly three places the corrected `CdRead` prototype can go.

1. **`include/code6cac.h:510`, its canonical location.** Correct, one line, fixes a real
   repo defect for every TU. **Out of scope** for this function's candidates.
2. **Block scope, inside `func_800372F4`.** Layer-1 cheat-reviewer **FAIL**,
   2026-08-26 01:42: "a scope-gate workaround, not a reconstruction of the original
   source". Banked at `rejected/layer1-fail-0826-0142.c`.
3. **File scope, immediately below `#include "code6cac.h"`.** s7 measured it: GCC 2.7.2
   accepts the conflicting redeclaration and it scores 0 (72/72), so it is a genuinely
   byte-valid, genuinely in-scope form. It is banked as **rejected** anyway
   (`rejected/filescope-cdread-redecl-conflicts-header.c`) on semantics: no human
   programmer writes a prototype that contradicts the header two lines above it — they
   fix the header. It is placement 2's workaround moved one scope level outward, and
   proposing it would be respelling a construct that already FAILed layer 1.

Leaving the parameters unread (v2) is placement-free but is the shape that FAILed
layer 1 on 2026-08-26 01:09; banked at `rejected/unread-params-arity-inert.c`.

So there is no honest form confined to `src/code6cac_b2_post.c`. Per the standing
out-of-scope constraint on this function, the prescribed disposition is to file an
escalation requesting `include/code6cac.h` be added to `tools/grinder/scope_allow.txt`
and return `owner-gated`. That is what s7 did — see `docs/grind/decisions.md`,
2026-08-26 entry.

### FACT 5 — two placement facts inside the matching form (unchanged from s7's predecessor, re-confirmed)

- The `sp+0x50` (block-move `final_src`) and `sp+0x810` (`sp_buf2` address) preheader
  computations come out in `loop.c` hoist order. Taking `sp_buf2`'s address at its two
  use sites — i.e. **not** introducing a pre-loop pointer local — gives the target
  order. A pre-loop `buf2_ptr` local costs exactly 2
  (`rejected/preloop-buf2ptr-hoist-order-score2.c`). The matching form has *fewer*
  locals, not more.
- `mode` is a named variable, not the literal `0x80`. With the literal, GCC
  rematerialises `li $a2,0x80` at each call site and the function drops to 69 insns,
  score 12 (`rejected/literal-mode-no-local-score12.c`). The target holds `0x80` in
  callee-saved `$s4` across the whole loop — direct evidence the original source named
  the value.

- [s7] Chassis re-measured this session from a clean main tree: special_camera_get_rot_dir scores 0 at 72/72 insns and func_800372F4 scores 0 at 21/21 insns with the s7 form in src/code6cac_b2_post.c. The ledger's recorded floor of 9 is superseded — it described the pre-s7 chassis in which the function was still INCLUDE_ASM and the copy loop was being hand-written in C.

- [s7] Full-build oracle run with the form in place: verify-oracle returns "ok": true, "build_matches": true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa. The match links.

- [s7] The target's copy block is not in the source. The .L800373C0 / bne $a2,$s5 four-word loop plus its three-word tail is emitted by GCC's MIPS backend from ONE aggregate assignment: expand_block_move (config/mips/mips.c:2362-2368) sees constp && bytes(60) > 2*MAX_MOVE_BYTES(32) && align >= 4 && optimize and dispatches to block_move_loop (mips.c:2222-2288), which emits final_src = src_reg + 48, the label, movstrsi_internal for 16 bytes (4 lw + 4 sw), src += 16, dst += 16, cmpsi, bne, then the leftover movstrsi for 60 % 16 = 12 bytes (3 lw + 3 sw).

- [s7] This is why sessions s1-s6 could not move the floor: copy_end ($s5 = sp+0x50) is block_move_loop's final_src, a backend pseudo born at RTL expand time with no C-level definition site. Five sessions of declaration-order, def-placement and live-length work at the C level could not reach it. The s5/s6 ra_solver model was 8/8-exact and still yielded nothing precisely because its central pseudo is not a C object.

- [s7] CdRead genuinely takes three arguments. include/code6cac.h:510 declares `extern void CdRead(s32);`, contradicting this repo's own byte-matched definition `s32 CdRead(s32 sectors, s32 buf, s32 mode)` at src/system.c:901, which reads all three (D_800A14DC = mode; D_800A14D4 = buf; *ps = sectors). include/m2c_context.h:1123 independently carries the 3-argument form. The header is a defect independent of this grind.

- [s7] The target corroborates the 3-argument call through the wrapper: both call sites set two argument registers plus $a2 (800373A0 addiu $a0,$zero,0x800 / 800373A4 addiu $a1,$sp,0x10; 80037428 lw $a0,0xC($s0) / 8003742C lw $a1,0x8($s0); $a2 = $s4, the mode held callee-saved across the whole loop), and asm/funcs/func_800372F4.s recomputes $a0 only and never writes $a1/$a2 before jal CdRead.

- [s7] The CdRead arity is byte-inert: forwarding buf/mode to a 3-arg CdRead and calling the header's 1-arg CdRead produce identical bytes in both functions (0/72 and 0/21 either way). No measurement can select among the candidate spellings, which is what makes the remaining question one of source fidelity rather than codegen.

- [s7] All four placements of the corrected prototype are now enumerated and measured, and exactly one is honest: include/code6cac.h:510 (correct, OUT OF SCOPE); block scope inside func_800372F4 (layer-1 FAIL 2026-08-26 01:42, 'a scope-gate workaround, not a reconstruction of the original source'); file scope under the include (s7-measured, compiles, scores 0, in scope, but a prototype contradicting the header two lines above it — the same workaround one level outward); and omitting the forwarding so buf/mode are unread (layer-1 FAIL 2026-08-26 01:09).

- [s7] Two placement facts inside the matching form, re-confirmed: a pre-loop buf2_ptr pointer local costs exactly 2 (the matching form takes sp_buf2's address at its two use sites and has FEWER locals, not more — rejected/preloop-buf2ptr-hoist-order-score2.c); and mode must be a named variable, since with the literal 0x80 GCC rematerialises li $a2,0x80 at each call site and the function drops to 69 insns, score 12 (rejected/literal-mode-no-local-score12.c) — the target holds 0x80 in callee-saved $s4 across the whole loop.

- [s7] The prior escalations on this function (2026-07-23 OWNER-ESCALATION and the 2026-07-27 ruling, option (b) REFUSED / OWNER-ACCEPTED INCOMPLETE) are obsolete: both rested on an allocno-priority wall around copy_end that does not exist in the matching form.

- [s7] src/ was reverted to its committed state before this session ended; `git diff --stat src/` is empty. The only modified tracked files are docs/grind/decisions.md, the ledger under memory/grind/special_camera_get_rot_dir/, and metrics/events.jsonl.

- [s7] This is an INTEGRATION HANDOFF, not an endgame lock: the bytes are proven and the blocker is a single one-line edit to a surface this function's candidates may not touch. The operator still runs a fresh layer-2 cheat-reviewer on the C before acceptance.
