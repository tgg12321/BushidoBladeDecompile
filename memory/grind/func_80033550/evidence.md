# Evidence bank — func_80033550

## s1 (2026-07-20, recon)

- **Baseline:** canonical verdict C (34 insns, distance 4). Honest floor measured
  THIS session: **4** with the pin-free do-while form (candidate.c), **5** without
  the wrap. HEAD's old form carried a `register s32 * asm("a3")` pin (cheat,
  stripped by sandbox → its 4 was the same wrap effect, the pin bought nothing).
- **The whole residual is ONE RA choice:** arg0's pointer pseudo (72) is homed in
  $a1 by our build, $a3 by target. 4 diffs = `move a1,a0` (entry) + the base reg
  of the three `lw`s. Everything else — loop, sb, idx chain, w-value regs
  (w0=$v1, w1=$a0, w2=$a1), store pattern — matches exactly.
- **RTL dump facts** (tmp/grind/func_80033550/s1/probe.i.greg, clean form):
  - Only 2 global pseudos: 72 = arg0-copy (→ a1), 73 = i (→ v1).
  - `72 conflicts: 73 + hard {2 (v0: idx/temps), 3 (v1: w0), 4 (a0: w1), 29}`.
    First free in alloc order = **5 (a1)**.
  - w2 (pseudo 76, local-alloc) ALSO sits in a1: born exactly at ptr's death
    (its own `lw a1,8(a1)`), so no conflict — death/birth adjacency lets them
    share. **Any geometry whose last ptr use is w2's load leaves a1 shareable**,
    so first-free can never skip to a3 without TWO extra conflicts (a1 AND a2
    occupants overlapping ptr's live range).
  - Nothing occupies a2 in target's emitted code either — so the original's RA
    input must have had pseudo(s)/hard-reg pressure invisible in the final
    bytes (no-op-move-deleted copies, or a structure we haven't found).
- **Scheduling residual (solved):** without the wrap, sched1 hoists the three
  lw's above the final `sll v0,v0,2` (+1 reordering → 5). The single-level
  do-while(0) around `idx = new_var; w0 = arg0[0];` fences it. This is NOW
  SANCTIONED: do-while-zero-exception was rewritten 2026-07-06 (owner ruling —
  ANY codegen effect, FAKE annotation required). The 2026-06-07 park predates
  that ruling and its "not sanctioned" note is obsolete.
- **Prior sweep (park b07abaa5, 2026-06-07):** 27+ structural variants all ≥5
  (loop forms, idx spellings, statement orders, pointer pre-computes, types);
  reads-first form = 10. Do not re-run these.
- **Sibling precedent:** func_8001CD68 (COMPLETED-C, code6cac.c) also copies
  a0→a3 naturally — but there a0/a1/a2 are all consumed by real conflicting
  values (div magic constants, global reload, minutes). Confirms the mechanism
  needed; our function lacks visible occupants.
- **Probe kills this session:** i*3-array-index spelling → byte-identical
  (combine canonicalizes). `register` keyword on the alias → still 4 AND the
  engine's cheat-stripper counts the register-alias as strippable
  (cheat_asm_stripped 369→370) — avoid that spelling.
- Artifacts: tmp/grind/func_80033550/s1/{probe.i.greg,probe.i.lreg,probe.i.sched,
  dump.sh,diffit.sh} (full -da dump set present).

- [s1] canonical: verdict C, 34 insns, pure-C target

- [s1] sandbox floors this session: 5 (clean, no wrap), 4 (pin-free FAKE-annotated do-while wrap) — 4 is now a ZERO-cheat floor, unlike the pinned HEAD form

- [s1] residual diff = move a1,a0 at entry + base reg of the three lw's (a1 vs a3); w-value homes (v1,a0,a1), loop, sb, idx chain, stores all match

- [s1] .greg proof: only 2 global pseudos (72=ptr, 73=i); 72's conflict set {v0,v1,a0}; w2 shares a1 with ptr by death/birth adjacency — any geometry ending ptr's life at w2's load leaves a1 shareable

- [s1] prior park b07abaa5 (2026-06-07): 27+ structural variants all >=5; its 'do-while not sanctioned' note is OBSOLETE after the 2026-07-06 owner ruling

- [s1] sibling func_8001CD68 (COMPLETED-C) gets a0->a3 naturally because real values (div magic consts, reload) occupy a0-a2 — confirms the required mechanism

- [s1] engine stripper counts a plain `register` (no asm) pointer alias as cheat-asm (369->370) — avoid that spelling in candidates

## s2 (2026-07-20, structural)

- **Floor unchanged: 4.** Pin-free candidate applied to src (HEAD's asm("a3")
  pin removed from the tree); 24 structural variants measured across 3 rounds
  (logs: tmp/grind/func_80033550/s2/logs/*.{json,diff,greg,lreg}).
- **THE FLIP IS POSSIBLE — v07 moved ptr a1→a2** (first movement ever):
  staging w2 through dead new_var (`new_var = arg0[2]; w2 = new_var;` after
  w1's load) makes .greg read `72 conflicts: 74 + hard {v0,a0,a1}` → first
  free = a2. Score 7: the staged global pseudo (74) takes v1 in global RA,
  and local-alloc then rotates w-homes to w0=a0, w1=a1, w2=v1 (target needs
  v1,a0,a1) = 3 extra diffs. Every staged placement tried: after-w1 7,
  mid 6, in-wrap 19, pre-idx 5 (kills the move entirely, like v09),
  stage-w1 6, double-stage-via-i 14 (i lands a1, ptr lands v1!).
- **No REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h** → global.c
  find_reg scans hard regs numerically: v0,v1,a0,a1,a2,a3. ptr reaches a3
  ONLY if a1 AND a2 both conflict. Confirmed empirically (v07 skipped
  exactly the conflict set).
- **Value census closes the geometry space:** the tail has exactly 5 values
  (i→v1, idx-chain→v0, w0→v1, w1→a0, w2→a1-after-ptr-death) + at-temps.
  The only possible a1/a2 occupants are the w's; any form where a w
  conflicts with ptr necessarily re-homes it off its target reg (measured
  min +2). Byte-free double-conflict therefore requires TWO zero-byte
  occupants: X in a1 conflicting {v0,v1,a0}+ptr, Y in a2 conflicting X+ptr
  — no C construct found this session produces one.
- **Copies cannot conjure conflicts byte-free** (measured): a second pointer
  handle either ties to incoming a0 (no-op deleted, v11/v19) or *becomes*
  the single a1 copy (v01-v04, v12); two overlapping un-tied handles emit a
  second move (v02: 13). found_idx=i at the found label ties to v1, no-op
  deleted, byte-identical (v22).
- **Inert probes (byte-identical to base):** per-load do-while wraps
  promoting w's to global pseudos (v21), constant-holder one=1 for the sb
  immediate (v23), decl order (v10), u32 idx/new_var (v15), i*3 (s1).
- **Wider-signature theory dead:** single caller (DispPracticeMenuTex_C,
  19BA4) sets ONLY a0 (from s4) in the jal delay slot; a1 holds a stale
  callee return. No evidence for extra live-in params (which are RA-inert
  anyway once flow deletes unused param copies).
- Artifacts: tmp/grind/func_80033550/s2/{sweep.sh,sweep2.sh,sweep3.sh,
  sweep4.sh,differ.py,splice.py,variants*/,logs/} (24 variant bodies,
  per-variant sandbox JSON + normalized diff, greg/lreg for rounds 2-3).

- [s2] No REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h -> find_reg scans numerically v0,v1,a0,a1,a2,a3; ptr reaches a3 only if a1 AND a2 both conflict (v07 empirically consistent)

- [s2] v07 .greg: '72 conflicts: 72 74 2 4 5 29', dispositions 72->a2, 74(staged new_var)->v1, w0(75)->a0, w1(76)->a1 — conflict injection works but rotates homes

- [s2] Tail value census: exactly 5 values (i, idx-chain, w0, w1, w2) + at-temps; the w's are the only possible a1/a2 occupants; every conflicting arrangement re-homes them off target (min +2 diffs measured)

- [s2] Copies cannot create conflicts byte-free: tied copies coalesce to no-op (deleted), un-tied copies emit real moves (v02=13); found_idx=i byte-identical

- [s2] Closing requirement now precise: TWO zero-byte occupants — X in a1 conflicting {v0,v1,a0}+ptr, Y in a2 conflicting X+ptr — no C construct found produces one

- [s2] Floor-4 candidate (pin-free, FAKE-annotated do-while wrap) applied in src/code6cac_b.c; final sandbox 4, cheat_asm_stripped 369 (file baseline)

## s3 (2026-07-20, structural)

- **Floor unchanged: 4.** 14 variants measured (12 loop-region + 2 refinement
  probes; logs: tmp/grind/func_80033550/s3/logs/*.{json,diff,greg,lreg}).
  Src reset by driver between sessions — candidate.c re-applied (baseline 4,
  stripped 369) and left in place at session end (final.json: 4/369).
- **Loop-region census-invariance PROVEN:** for / do-while / while /
  goto+flag-temp(s32) / goto+flag-temp(u8) / per-iteration address-temp
  (`p = &D_800A3918 + i`) ALL score 4 with literally identical .greg:
  `2 regs to allocate: 74 72; 72 conflicts: 72 74 2 3 4 29`. cse/flow
  canonicalize every loop spelling to the same RTL pre-RA. The loop region
  CANNOT change the pseudo census — frontier item "loop-region respelling
  supplies invisible occupants" is dead.
- **Live loop pointers always cost bytes:** flags[] loop+store 19,
  loop-only 10, walker+index hybrid 9, store-via-addr-temp 11. They DO add
  a conflicting pseudo (75) but target's at-macro address forms make any
  materialized pointer diverge.
- **REG_EQUIV path structurally dead:** three const table pointers
  (t0/t1/t2 = &D_8010785x) → byte-identical, .greg has only 2 allocnos —
  cse folds uses + flow deletes the sets PRE-RA (global.c never sees them).
  Same result with the inits inside a do-while(0) fake loop (Z4): loop
  notes do not keep them alive. Conflict lists cannot be shaped by
  const-address pseudos from any C spelling found.
- **DImode pair-occupant mechanism CONFIRMED / spelling KILLED:** s64 t=i
  (22) and u64 t=(u32)i (22, zero high half = cheapest possible) both emit
  unavoidable pair-half insns; but their .greg conflict set is exactly
  {v0,v1,a0}+ptr — a byte-free DImode pseudo WOULD pair-home a1+a2 and push
  ptr to a3. No C spelling makes one byte-free (GCC 2.7.2 always emits both
  half-sets even when the high half is dead/zero).
- **Full disposition record (base):** `72 in 5 (ptr→a1); 74 in 3 (i→v1);
  local-alloc 75→v1 (w0), 76→a0 (w1), 77→a1 (w2); hard regs used 2 3 4 5`.
  Allocation order i-then-ptr (priority), irrelevant to first-free outcome.
- **Combined s2+s3 closure:** tail geometry (s2) + loop-region census (s3) +
  REG_EQUIV + DImode structural spellings (s3) all measured dead. The
  structural modality is EXHAUSTED for this function; remaining axes are
  permuter search (whole-function exotic geometries) and cc1 forensics
  (post-conflict-build deletions: no-op-move coalescence/reg_may_share,
  reload inheritance; plus a cc1psx calibration cross-check).
- Artifacts: tmp/grind/func_80033550/s3/{baseline.sh,sweep.sh,sweep2.sh,
  greg.sh,extract.py,conflicts.py,variants/,variants2/,logs/}.

- [s3] Loop respellings are RA-census-invariant: 6 spellings, identical conflict sets and bytes (score 4) — the loop cannot supply a1/a2 occupants

- [s3] Const-address pointer pseudos are deleted pre-RA (2 allocnos, byte-identical), even inside a do-while(0) fake loop — REG_EQUIV conflict-shaping unreachable from C spellings

- [s3] DImode temp conflict set {v0,v1,a0}+ptr proves a byte-free pair pseudo would flip ptr to a3 via a1+a2 pair-homing; s64=22, u64-zero-high=22 — no byte-free spelling exists

- [s3] Structural modality EXHAUSTED (s2 tail + s3 loop/REG_EQUIV/DImode); remaining: permuter whole-function search, cc1 forensics on post-conflict-build deletions, cc1psx calibration cross-check

- [s3] s3 baseline: candidate.c re-applied to src (driver had reset it to the old pinned form); sandbox 4, stripped 369; final state re-verified 4/369 with candidate in place

- [s3] Loop respellings are RA-census-invariant: 6 spellings, identical conflict sets and bytes — the loop region cannot supply a1/a2 occupants

- [s3] Const-address pointer pseudos are deleted pre-RA (2 allocnos, byte-identical) even inside a do-while(0) fake loop — REG_EQUIV conflict-shaping is unreachable from C spellings

- [s3] DImode conflict set {v0,v1,a0}+ptr proves a byte-free pair pseudo WOULD flip ptr to a3 via a1+a2 pair-homing; s64=22, u64-zero-high=22 — no byte-free spelling

- [s3] Full base dispositions banked: 72->a1(ptr), 74->v1(i), local-alloc w0->v1 w1->a0 w2->a1, hard regs 2 3 4 5, allocation order i-then-ptr

- [s3] Structural modality EXHAUSTED: s2 closed tail geometry, s3 closed loop-region census + REG_EQUIV + DImode spellings

## s4 (2026-07-20, permuter)

- **Floor unchanged: 4.** Four permuter campaigns via tools/permuter_campaign.py
  (~104k total iterations, all harvested + stopped in-session): c1 candidate-seed
  random (26.5k iters, 2 finds), c2 directed staged-load PERM cross-product
  (31.7k, 1 find), c3 v07-flip-neighborhood (15k, 12 finds), c4 c3-20-seed
  neighborhood (31k, 2 finds). Src reset by driver again (old pinned form) —
  candidate re-applied at session start; final state verified 4/369.
- **Permuter base score for the candidate = 20** (4 reg-diffs x 5), consistent
  with the known ptr a1-vs-a3 residual. NOTHING sub-20 found in any basin.
- **The score-20 attractor class is universal:** every basin converges to
  equal-score members of one family — pointer-alias second handles
  (`new_var2 = arg0;` feeding 1-2 loads; the s2-killed class), split/staged
  idx computation (`new_var2 = i; new_var2 *= 12;` or idx-before-sb), and
  w2 staging through dead-at-that-point new_var. All are byte-equivalent to
  the candidate residual.
- **c3 (v07 staged-flip seed, base 50) descended 50->40->30->20 within 5
  minutes** — but the descent leads back INTO the ptr=a1 attractor, not to a
  home-preserving flip: its best find (idx-presb + double-staged w2) was
  honest-measured sandbox 4 with the IDENTICAL diff (move a1,a0 + three lw
  via a1 vs target a3). Banked as rejected/permuter-idx-presb-double-stage-
  still-a1-4.c. Late find output-25-1 (score 25) also above floor-class.
- **c4 proves the attractor is locally rigid:** 31k mutations seeded AT the
  new score-20 geometry produced only equal-score attractor members (alias
  handle + split multiply), never sub-20.
- **Permuter whole-function axis is now measured near-dead:** random +
  directed + two distinct sub-basins all fail to find any C geometry whose
  RA outcome differs from ptr=a1 (or the known byte-costing flips). This
  independently corroborates the s2/s3 structural closure by stochastic
  search over exotic statement geometries.
- Artifacts: tmp/grind/func_80033550/s4/{setup.sh,chassis2_base.c,
  chassis3_base.c,*_campaign.log,*_campaign_meta.json,*_base.c,finds/}.

- [s4] Permuter candidate base score 20 = 4 reg-diffs x 5; four campaigns (~104k iters, 4 basins: random, directed-staged, v07-flip-neighborhood, c3-20-neighborhood) found NOTHING sub-20

- [s4] Universal score-20 attractor: alias handles / split multiplies / idx-presb staging — all byte-equivalent to the candidate residual (ptr=a1); c3-20 form honest-measured sandbox 4 with identical diff

- [s4] c3 v07-flip basin descends 50->20 in <5 min but lands back in the ptr=a1 attractor — mutation cannot preserve the a2-flip while restoring w-homes

- [s4] Permuter whole-function axis measured near-dead; corroborates s2/s3 structural closure stochastically; remaining live axes: cc1 forensics (post-conflict-build deletions) + cc1psx calibration cross-check

- [s4] Permuter base score for the floor-4 candidate is 20 (4 reg-diffs x 5), consistent with the known ptr a1-vs-a3 residual

- [s4] Universal score-20 attractor class across all 4 basins: pointer-alias second handles, split/staged idx computation (incl. idx-before-sb), w2 staging through new_var — all byte-equivalent to the candidate residual

- [s4] c3 best find honest-measured in sandbox: score 4, cheat_asm_stripped 369, diff identical to candidate (build move a1,a0 + lw via a1; target addu a3,a0,zero + lw via a3) — banked as rejected/permuter-idx-presb-double-stage-still-a1-4.c

- [s4] Driver had reset src to the old pinned form (stripped 371); pin-free candidate re-applied at session start and verified in place at session end (final sandbox 4/369)

- [s4] All four campaigns launched via tools/permuter_campaign.py, waited in-turn, and harvest --stop'd before session end (fresh-seed discipline; zero orphaned campaigns)

## s5 (2026-07-20, permuter)

- **Floor unchanged: 4.** Two fresh-basin campaigns via tools/permuter_campaign.py
  (~34k iters, both harvested + stopped in-session): c5 DImode-pair chassis seed
  (rejected Z1 form, base 325, 17,078 iters) and c6 walker/flags-pointer chassis
  seed (rejected L7 form, base 450, 16,887 iters). Neither basin was searched in
  s4. Src reset by driver again — candidate re-applied at session start,
  verified 4/369 (sandbox JSON identical to s2-s4 baselines).
- **Both basins converge to the SAME score-20 ptr=a1 attractor as all four s4
  basins** (6 basins total now): c5 reaches 20 by degrading t to int (split
  multiply — known attractor member); c6 reaches 20 by folding the flags
  pointer away entirely (direct global addressing = the candidate itself).
  Zero sub-20 finds in either basin over full ~30-min fresh-seed windows.
- **NEW MECHANISM FACT (c5 output-30-1, score 30 = 6 reg diffs, 0 ins/del):**
  mutation moved `t = i;` below the `(s32)t * 12` read → upward-exposed
  uninit use. The DImode pseudo SURVIVED TO RA BYTE-FREE and occupied $a3
  (build idx chain reads a3-garbage; honest diff banked at
  tmp/grind/func_80033550/s5/out30_vs_target.diff). First zero-byte
  register occupant ever observed on this function. ptr did NOT flip
  (loads still via a1). Semantically invalid (garbage idx) — banked as
  rejected/permuter-uninit-dimode-read-garbage-a3-30.c. Implication: the
  post-conflict-build / dead-def occupancy channel is REAL in this GCC, but
  the only spelling that reaches it is an uninit read; any valid init
  re-emits the pair-half bytes (22+). Frontier-3 (FAKE-family occupants)
  should target this channel; frontier-1 forensics should read how flow
  keeps upward-exposed uses in the conflict graph.
- **Tooling fact (root-caused, no fix applied — tools/ frozen):** standalone
  invocations of the permuter workspace compile.sh die in maspsx
  (`.file` split: `_, num, filename = line.split()`, maspsx/__init__.py:940)
  because the repo path contains spaces; the permuter itself compiles from
  space-free temp dirs, so campaigns are unaffected. Measure finds by
  copying source.c to /tmp first (tmp/grind/func_80033550/s5/measure30.sh).
- Artifacts: tmp/grind/func_80033550/s5/{setup.sh,c5_base.c,c6_base.c,
  c5_campaign.log,c5_campaign_meta.json,c6_campaign.log,c6_campaign_meta.json,
  measure30.sh,out30_vs_target.diff,watch.sh,finds/}.

- [s5] Six independent permuter basins (s4's four + s5 DImode + s5 walker) all converge to the score-20 ptr=a1 attractor; zero sub-20 finds anywhere — permuter modality is now measured DEAD for this function

- [s5] c5 output-30-1: uninit-read DImode pseudo survives to RA byte-free and occupies $a3 (0 ins/del, 6 reg diffs) — first zero-byte occupant ever observed; channel real but only reachable via semantically-invalid uninit read

- [s5] compile.sh standalone is broken by the space-containing repo path (maspsx .file split crash); permuter campaigns unaffected (space-free temp dirs) — measure finds from /tmp copies

- [s5] Six independent permuter basins across s4+s5 (~138k cumulative iterations) all terminate in the score-20 ptr=a1 attractor with zero sub-20 finds — permuter modality measured DEAD for func_80033550

- [s5] c5 output-30-1: uninit-read DImode pseudo survives to RA byte-free and occupies $a3 (first zero-byte register occupant ever observed on this function); banked as rejected/permuter-uninit-dimode-read-garbage-a3-30.c with honest diff tmp/grind/func_80033550/s5/out30_vs_target.diff

- [s5] c6 walker basin: every score improvement removes the pointer variable — no materialized-pointer form below 50; the basin floor IS the candidate

- [s5] Driver reset src to the old pinned form again; pin-free candidate re-applied at session start and verified 4/369 at session start and end

- [s5] Tooling: standalone permuter-workspace compile.sh crashes in maspsx (.file line split, maspsx/__init__.py:940) because the repo path contains spaces; campaigns unaffected (permuter compiles from space-free temp dirs); measure finds via /tmp copies (measure30.sh pattern)

## s6 (2026-07-20, forensics)

- **Floor unchanged: 4.** Src reset by driver again — pin-free candidate re-applied
  at session start, sandbox 4 / stripped 369 (identical JSON to s2-s5 baselines).
  No src edits after that measurement.
- **cc1psx cross-check KILLS the compiler-fork theory:** GCC 2.7.2.SN.1 (the real
  PsyQ cc1psx via tools/cc1psx_wrapper.sh) compiled the exact candidate C
  (minimal TU, same input to both) → INSTRUCTION-IDENTICAL output to our fork:
  `move $5,$4`, all three lw via $5 (a1), same schedule; only label syntax
  ($L vs .L) and banner differ (s6/ours.s vs s6/psx.s). The original a3 did NOT
  come from a different compiler — it came from different SOURCE.
- **s3's DImode mechanism claim CORRECTED (architectural):** mips.c:3446-3447
  `mips_hard_regno_mode_ok`: GP-reg multi-word values require EVEN regno
  (`(regno & 1) == 0 || size <= UNITS_PER_WORD`). A DImode pseudo can never
  start at $5/a1; legal pairs are (v0,v1) ($2,$3), (a0,a1) ($4,$5), (a2,a3)
  ($6,$7). With conflicts {v0,v1,a0} the ONLY legal placement is a2+a3 — which
  BLOCKS a3 and leaves a1 free. This exactly explains c5 output-30-1 (uninit
  DImode landed a2+a3, reads seen on $7, ptr stayed a1). "Byte-free DImode
  would pair-home a1+a2 and push ptr to a3" is impossible; the DImode axis is
  closed by HARD_REGNO_MODE_OK, not merely by spelling failures.
- **The exact allocation decision, fully instrumented** (tmp/gccdbg/cc1
  BB2_FINDREG_DEBUG=72, dump s6/findreg_72_74.txt; dbg cc1 output verified
  identical to build cc1): find_reg (global.c:920) for pseudo 72:
  conflicts {2,3,4,29}, own_copy_prefs EMPTY, own_full_prefs EMPTY,
  someone_prefers EMPTY; pass-0 numeric scan (global.c:1018-1043, no
  REG_ALLOC_ORDER) picks first free = 5 (a1). a1 was NOT excluded by
  ¬used_so_far (local-alloc already used 2,3,4,5 for w/temp qtys). To get 7
  (a3), regs 5 AND 6 must both be in `used` — with all preference sets empty,
  ONLY hard_reg_conflicts[72] ⊇ {5,6} can do it.
- **Preference channel closed at source level:** set_preference (global.c:
  1590-1675) fires only on SET insns pairing a hard reg with a global pseudo;
  find_reg's copy-preference override (global.c:1057-1090) could seat ptr in
  a3 over a free a1, but the only hard-reg SET in a call-free leaf is the
  incoming `ptr = $a0` copy. No call → no a1/a2/a3-rooted preference exists
  for ANY C spelling. (Target has no calls, so the original couldn't use this
  channel either.)
- **Byte-free occupancy channels enumerated and ALL closed for valid C:**
  (a) real-def SImode occupant → emits bytes (s2/s3 measured, v07 class);
  (b) coalesced copy → final.c:1800-1806 deletes reg-reg moves only when
  REGNO(src)==REGNO(dst), so a byte-free copy-occupant necessarily sits in
  its source's home ∈ {v0,v1,a0} — can never occupy a1/a2;
  (c) DImode pair → a2+a3 only (above);
  (d) dead-def pseudo → flow.c:1479 insn_dead_p deletion runs PRE-RA, pseudo
  never reaches conflict construction (s3 empirical);
  (e) upward-exposed-use pseudo → the one REAL byte-free channel (c5-30), but
  a semantically valid read must carry a defined value, which forces a def
  (bytes) or aliasing an existing same-reg value (no new conflict); the only
  byte-free spelling is an uninit read whose garbage flows into the stores —
  invalid. CONCLUSION: no semantically-valid C producing THIS exact 34-insn
  shape can home ptr in a3 under this compiler. The original source must
  differ in pre-RA census in a way that still emits these 34 bytes — and by
  the closure above, no such valid C exists within the searched shape space;
  frontier-3's FAKE families are predicted inert (dead stores flow-deleted
  pre-RA per (d); named-locals fold or emit bytes; duplicated-statement only
  lifts priority, and allocation ORDER is outcome-irrelevant here — s3).
- **Wider-signature axis now MEASURED dead (was reasoning-only in s2):**
  ANSI 4-param (3 unused), 2-param, and K&R-style 4-param variants all
  compile to byte-identical asm with IDENTICAL conflict sets (72 conflicts
  {i,2,3,4,29}) and ptr in a1 (s6/p4.c.greg, p2.c.greg, pk.c.greg). Unused
  parm copies are flow-deleted with no residual entry liveness for a1-a3.
- Artifacts: tmp/grind/func_80033550/s6/{psxcheck.sh,probe.c,ours.s,psx.s,
  findreg.sh,findreg2.sh,findreg_72_74.txt,probe.c.greg,parmtest.sh,
  p4.c.greg,p4.s,p2.c.greg,p2.s,pkdiag.sh,pk.c.greg,pk.s}.

- [s6] cc1psx (GCC 2.7.2.SN.1) output is instruction-identical to our fork on the candidate C (ptr in a1, same schedule) — compiler-fork RA divergence KILLED; the original a3 came from different source, not a different compiler

- [s6] mips.c:3447 even-regno rule for multi-word GP values: DImode can only pair-home (v0,v1)/(a0,a1)/(a2,a3); with conflicts {v0,v1,a0} only a2+a3 is legal — s3's a1+a2 pair-home mechanism is architecturally impossible; explains c5-30 exactly

- [s6] BB2_FINDREG_DEBUG on pseudo 72: conflicts {2,3,4,29}, ALL preference sets empty, someone_prefers empty; pass-0 numeric scan picks 5; a3 requires hard_reg_conflicts ⊇ {5,6} — no other channel exists

- [s6] Preference channel to a3 is source-closed: set_preference needs a hard-reg SET; a call-free leaf has only the a0 parm copy; target has no calls either

- [s6] Closure theorem: channels (a) real-def=bytes, (b) coalesce=source-home-only (final.c:1800), (c) DImode=a2+a3-only, (d) dead-def=flow-deleted-pre-RA (flow.c:1479), (e) uninit-use=invalid — no valid C with this 34-insn shape homes ptr in a3

- [s6] Wider signatures (ANSI-4/2-param/K&R-4, unused extras) measured census-identical and byte-identical — s2's reasoning-kill now .greg-proven

- [s6] cc1psx (GCC 2.7.2.SN.1) output is instruction-identical to our fork on the candidate C — the original a3 came from different SOURCE, not a different compiler

- [s6] mips.c:3447: GP multi-word values need even start regno; DImode pairs are only (v0,v1)/(a0,a1)/(a2,a3) — s3's a1+a2 pair-home claim was architecturally impossible; c5-30's a2+a3 landing exactly explained

- [s6] BB2_FINDREG_DEBUG=72: conflicts {2,3,4,29}, own_copy_prefs/full_prefs/someone_prefers ALL empty; pass-0 numeric scan picks 5(a1); ptr->a3 requires hard_reg_conflicts >= {5,6} — conflicts are the ONLY channel

- [s6] set_preference (global.c:1590) fires only on hard-reg<->pseudo SETs; call-free leaf has only the a0 parm copy, so no a1/a2/a3 preference is reachable from C

- [s6] final.c:1800-1806: post-RA no-op move deletion requires same src/dst reg — a byte-free coalesced occupant can only sit in its source's home ({v0,v1,a0}), never a1/a2

- [s6] Closure theorem: no semantically-valid C producing this exact 34-insn shape homes ptr in a3 under this compiler; FAKE-family occupants predicted inert (dead stores flow-deleted pre-RA; named-locals fold or emit bytes; duplicated-statement lifts only priority, and order is outcome-irrelevant here per s3)

- [s6] Wider signatures (ANSI-4/2/K&R-4 with unused params) measured byte- and census-identical — s2's reasoning-kill now .greg-proven

- [s6] Baseline re-verified at session start: sandbox 4, cheat_asm_stripped 369 (driver had reset src to the old pinned form again; pin-free candidate re-applied and left in place)

## s7 (2026-07-21, forensics)

- **Floor unchanged: 4.** Src reset by driver again (old pinned form) — pin-free
  candidate re-applied at session start (sandbox 4/369) and re-verified in place
  at session end (4/369). FAKE-family sweep (frontier-3, last unmeasured
  sanctioned axis) measured across 5 rounds / 27 variants with per-variant
  cc1 -da dumps (tmp/grind/func_80033550/s7/out*/).
- **Dead-store + named-local families: INERT, .greg-proven** (5 variants:
  fake=i tail, fake=arg0[0] entry, two dead stores, k0/k1/k2 keep-copies,
  slot=i live copy — ALL byte-identical to base, census identical 2 allocnos).
  Pass named: the dead-store pseudo (79) exists in .rtl and is ALREADY GONE in
  .jump — **jump_optimize pass 1** deletes it (earlier than s6's flow.c:1479
  prediction; same conclusion: never reaches conflict construction). Named
  copies coalesce in cse. Frontier-3's ds/nl prediction confirmed.
- **Duplicated-statement-into-arms family: NOT inert — NEW byte-free conflict
  channel (f), missed by the s6 closure enumeration.** Identical duplicated
  arms survive to RA as real pseudos/conflicts, then post-reload
  jump_optimize cross_jump (jump2) merges them (dup2: .greg 21 insns ->
  .jump2 16) and jump.c delete_computation deletes the dead cond computes
  (X1: sltiu present in .greg, absent in final asm). The branch + compare
  can be FULLY byte-free (single 2-arm merge) while having reshaped
  global.c's conflict graph. This empirically amends the s6 closure theorem:
  its channel enumeration (a)-(e) did not include post-RA cross-jump deletion.
- **First valid-C ptr->a3 EVER: dupU** (arms={sw3} x3, two big-const unsigned
  ptr compares): loads via $7/a3, entry copy `move $7,$4` present, w0=v1 and
  w1=a0 BOTH target-correct — but idx->a1, w2->a2 and ~5 residual cond bytes
  (partial 3-arm merge; two li + sltu + beq survive). s6's "no valid C homes
  ptr in a3" is DISPROVEN as a general claim (it survives only as "...at
  distance <= 4 within the exact 34-insn shape").
- **The family's rotation algebra (measured):** arms={sw3} globalizes idx+w2.
  (1) Temp-free/small-const conds (arg0!=0, (u32)arg0<2U, shadow copies,
  2-3 arms — SIX spellings, identical census) -> rigid attractor idx=a1,
  w2=a2, ptr=a0-coalesced (move deleted, 33 insns), w0=v0, w1=v1: honest
  sandbox **11**. (2) Big-const cond (dupM): the lui/sltu temps seat in the
  v0 region, pushing locals to TARGET seats (w0=v1, w1=a0) and rotating
  ptr=a2, w2=a3, idx=a1 + ONE stray li survives delete_computation: honest
  sandbox **11**. Musical chairs invariant: filling a1+a2 vacates a0 ->
  ptr coalesces there unless the lui/sltu pressure holds the low seats.
- **Reg-reg equality conds self-destruct:** (u32)arg0 != (u32)w2 -> cse
  specializes the else arm (substitutes the known-equal reg), arms diverge,
  merge fails, branch survives (dupS2/S3). Viable opaque conds are only:
  null-compare arg0!=0 (temp-free) and (u32)arg0 < N (sltiu, deletable).
- **Why distance-0 stays out of reach in this family (so far):** target needs
  seats idx=v0, w0=v1, w1=a0, w2=a1, ptr=a3 with a byte-free a2 occupant
  conflicting with ptr. Any arm-use of a store operand globalizes it;
  globalized idx always loses v0 (locals/temps grab it first); deletable
  cond temps seat LOW (local-alloc first-free), displacing idx/w's from
  their target seats. Every measured rotation is a cyclic shift, min
  honest score 11. The un-searched residue: cond/temp spellings whose
  deleted temps seat EXACTLY at a2, and label-placement merge steering
  (motion_SetMotion precedent).
- **Sanctioning status OPEN:** duplicated-statement-into-arms is owner-
  sanctioned (2026-07-01) including cross-jump re-merge + reg_n_refs lift,
  but the rule presupposes arms that EXIST in control flow (SOTN 7-arm/
  11-arm switches). This function's tail is straight-line: the only way to
  place arms after the sb is an INVENTED opaque always-true/unknown
  condition with identical arms, fully merged away by jump2. Whether the
  invented branch is within the sanction or is itself a cheat-by-spelling
  is unclassifiable from existing rules -> s7 outcome is ruling-request.
- Artifacts: tmp/grind/func_80033550/s7/{mkvariants*.py,dump.sh,sweep*.sh,
  diffs.sh,forensic*.sh,variants*/,out/,out2/,out3/,out4/,out5/}.

- [s7] Dead-store/named-local FAKE families INERT (5 variants byte-identical, census-identical): dead-store pseudo deleted by jump_optimize pass 1 (present .rtl, gone .jump) — never reaches RA

- [s7] NEW channel (f): identical duplicated arms are real conflicts pre-RA, then jump2 cross-jump merges them and jump.c delete_computation kills the cond compute — byte-free conflict-graph reshaping missed by the s6 closure enumeration

- [s7] dupU = first valid-C ptr->a3 (loads via $7, entry move present, w0=v1 w1=a0 correct); s6's blanket "no valid C homes ptr in a3" disproven — survives only as "not at distance <=4 in the 34-insn shape"

- [s7] Family rotation algebra: sw3-arms attractor (6 spellings census-identical) = idx a1/w2 a2/ptr a0-coalesced, honest 11; big-const cond rotation = w0 v1+w1 a0 CORRECT, ptr a2/w2 a3/idx a1 + stray li, honest 11

- [s7] Reg-reg equality conds self-destruct (cse specializes the else arm); viable opaque conds: arg0!=0 (temp-free) and (u32)arg0<N (sltiu, deleted with branch)

- [s7] Distance-0 blocker in the arms family: globalized idx always loses v0, deletable temps seat low; every rotation is a cyclic shift (min 11); unsearched: temps seating exactly a2 + label-placement merge steering

- [s7] Sanctioning question OPEN (ruling requested): invented opaque identical-arms branch, fully merged away — within duplicated-statement-into-arms or cheat-by-spelling?

## s8 (2026-07-21, forensics — post-ruling escalation session)

- **Judge ruled on the s7 ruling-request (2026-07-21 00:19, decisions.md): FAIL.**
  The invented identical-arms branch is a cheat-by-spelling, outside the
  duplicated-statement-into-arms sanction (whose SOTN/MGS evidence all uses
  pre-existing arms). Channel (f) via manufactured branches is CLOSED; the
  family residue (a2-seated deletable temps, label-placement merge steering)
  is unreachable without the manufactured branch and dies with it. The ruling
  itself verifies: "with the invented-branch spelling ruled out, every
  sanctioned axis is measured dead" and directs OWNER-ESCALATION per
  endgame-lock-disposition.
- **Floor unchanged: 4.** Src found at HEAD's pinned form; measured BOTH ways
  this session: pinned form sandbox 4 / cheat_asm_stripped 371 (the asm("a3")
  pin is strippable cheat-asm and score-inert), pin-free candidate re-applied
  sandbox 4 / 369 (identical JSON to s2-s7 baselines). Candidate left in
  place in src at session end.
- **Hand-coded certification measured: scan_hand_coded LOW 0/8** (no S1 multu
  pacing, no S2 empty branch, S3/S4 N/A at 34 insns, no S5 sibling cluster,
  no S6 BIOS jumptable, no S7 unsaved $sN, no S8 redundant mask) —
  dispositive REFUSE for canonical-asm under endgame-lock-disposition
  criterion 1.
- **OWNER-ESCALATION FILED** in docs/grind/decisions.md (2026-07-21 entry,
  "filed by grind s8 forensics"): options (a) canonical-asm (not supportable,
  LOW 0/8 + cc1psx instruction-identity) vs (b) INCOMPLETE-owner-accepted
  with the asm("a3") pin retained solely to hold the oracle match. Same
  species/protocol as motion_SetMotion, saTan0Init, cpu_side_move_dir_4,
  func_80057CC8 (all ruled option (b)). Awaiting owner ruling; grind parks.
- Artifacts: tmp/grind/func_80033550/s8/{scan_hand_coded.txt,
  sandbox_pinned.json,sandbox_candidate.json,escalation_entry.md}.

- [s8] Judge FAIL 2026-07-21 00:19: invented identical-arms branch = cheat-by-spelling; channel (f) closed; every sanctioned axis now measured dead per the ruling's own verification

- [s8] Pinned HEAD form measured: sandbox 4 / stripped 371 — the asm("a3") pin is strippable cheat-asm and score-inert; it exists on main solely to hold the oracle byte match

- [s8] scan_hand_coded --single func_80033550: LOW 0/8, no strong signals — canonical-asm refused by endgame-lock-disposition criterion 1

- [s8] OWNER-ESCALATION filed in docs/grind/decisions.md (2026-07-21, grind s8); function proceeds to owner-gated park until the owner rules

- [s7] Judge FAIL 2026-07-21 00:19 (docs/grind/decisions.md): invented identical-arms branch is a cheat-by-spelling; channel (f) via manufactured branches is CLOSED; ruling verifies every sanctioned axis measured dead and directs OWNER-ESCALATION per endgame-lock-disposition

- [s7] scan_hand_coded --single func_80033550: LOW 0/8 (no S1-S8 signals) — canonical-asm refused by endgame-lock-disposition criterion 1; artifact tmp/grind/func_80033550/s8/scan_hand_coded.txt

- [s7] HEAD pinned form measured this session: sandbox 4 / stripped 371 — the register s32 *a3_arg asm("a3") pin is strippable cheat-asm and score-inert; it holds the oracle byte match on main (0 regfix/asmfix rules)

- [s7] Pin-free candidate re-applied to src/code6cac_b.c: sandbox 4 / stripped 369, identical to s2-s7 baselines; left in place at session end (driver restores the pinned form on park to keep the oracle green)

- [s7] OWNER-ESCALATION filed: docs/grind/decisions.md entry '2026-07-21 — func_80033550 (src/code6cac_b.c) — OWNER-ESCALATION (filed by grind s8 forensics...)' with both options honestly stated (option (a) not supportable at LOW 0/8 + cc1psx instruction-identity; option (b) INCOMPLETE-owner-accepted per the 4-for-4 same-species precedent)

## s10 (2026-08-20, forensics — post-unpark, F6/F7 verification)

STATUS CHANGE INHERITED: the 2026-07-22 owner ruling (decisions.md:1292,
option (b) REFUSED / OWNER-ACCEPTED INCOMPLETE) is SPENT. The 2026-08-19
owner-directed stale-park re-audit (docs/grind/borderline.md:68) unparked
func_80033550 explicitly on the ground "F6+F7 seam — session must verify or
ruling-request". Sessions must NOT cite the 2026-07-21/22 escalation as a
live disposition: its refusal ground was superseded, and the function is a
normal active queue item again. This session performed the mandated
verification.

- [s10] CHASSIS RE-MEASURED (post asm-until-matched migration, 2026-08-19).
  On main the function is now `INCLUDE_ASM("asm/funcs", func_80033550);`
  (src/code6cac_b.c:2673); migration_pin.json pins floor 4. Applying
  memory/grind/func_80033550/candidate.c verbatim scores sandbox
  --disable all = **4**, 34/34 insns, rules_dropped 0, cheat_asm_stripped
  237 (the strip count is TU-wide, not this function). The ledger floor is
  chassis-invariant; every s1-s9 spelling conclusion re-verified below is
  therefore still valid on this chassis.

- [s10] F6 EMPTY-IF / FABRICATED-REDUNDANT-CONDITION SHAPE — MEASURED INERT,
  pass named. Five placements (`if (!i) { }` after `found`, after
  `new_var = i*12`, `if (!w2) { }` after the w2 read, `if (!arg0) { }`, and
  `if (arg0 && arg0) { }`) all score 4 / 34 insns. Pass attribution from
  tools/grinder/dump.ps1 output: insn counts per pass are
  base .rtl=34 .jump=24 .cse=22 .loop=22 .cse2=22 .flow=21 .combine=20
  .lreg=20 .greg=20; empty-if .rtl=**35** then .jump=**24** and identical
  thereafter. jump_optimize pass 1 (jump.c) deletes the jump-to-next-insn
  and the compare dies with it, BEFORE local-alloc. .greg is byte-identical
  to baseline: `2 regs to allocate: 74 72`, `72 conflicts: 72 74 2 3 4 29`,
  dispositions `72 in 5` ($a1). Zero conflict-graph effect.

- [s10] F6 CANCELLATION-PAIR SHAPE (`x++; x--;`) — the dichotomy, 16
  placements. NON-OVERLAPPING pairs (on i, idx, new_var, a fresh staged
  local `t = i;` cancelled after the loads, w1/w2 after the last arg0 read):
  .cse2=24 -> .flow=**21** (== baseline) — deleted by flow.c life-analysis
  DCE pre-RA; .greg identical to baseline; byte-free but INERT. One
  intermediate case (`w2++; w2--;` right after the w2 read) is byte-free AND
  promotes a new allocno into global.c (`3 regs to allocate: 77 74 72`,
  `77 conflicts: 77 2 3 4 29`) but 77 does NOT conflict with 72 (72 is dead
  after the last arg0 read), so both land in $a1 and nothing moves — the
  first byte-free allocno injection ever recorded here, and provably
  useless because it cannot overlap the pointer.
  OVERLAPPING pairs (w0 before w1, w0 between w1/w2, w1 before w2, w0+w1,
  w0 twice, w0-before-w1 + w1-after-w2): the pair survives .flow=23 and
  .combine=21 into RA; the new allocno conflicts with 72 and with hard
  2,3,4 — but .greg then prints `72 preferences: 4` (NEVER present in
  baseline) and 72's hard-reg-4 conflict DISAPPEARS, i.e. the entry copy
  `move a1,a0` became coalescable. global.c find_reg tries the preferred
  register first, so 72 is seated in $a0, the entry move is deleted (33
  insns) and the score rises to 6-7. Measured: d1 7, d2 6, d3 6, d4 7,
  d5 7, d6 6, d8 7, c5 6.

- [s10] THEOREM (refines the s6 closure theorem and s9's preference finding).
  Pseudo 72 is the destination of the function's ONLY hard-reg copy
  (`72 <- $a0`; s9 proved $a0 is the only hard GPR anywhere in this
  call-free single-parameter function's pre-RA RTL, so no $a3 preference is
  C-reachable). Consequently 72 has exactly two allocation regimes: (A) the
  copy is NOT coalescable -> 72 conflicts with hard 4 and find_reg's numeric
  scan v0,v1,a0,a1,a2,a3 gives $a1, and no sanctioned construct can seat an
  allocno in $a1/$a2 byte-free (F6-inert, dead-store/named-local inert per
  s7, REG_EQUIV pseudos die pre-RA per s3); or (B) enough liveness is
  injected that the copy becomes coalescable -> `72 preferences: 4` wins
  BEFORE the numeric scan and 72 lands in $a0 at a byte cost. Conflict
  injection is exactly the thing that flips (A) into (B). $a3 is
  unreachable from either regime. This is why every F6 placement is either
  inert or $a0-seating, and it predicts the same outcome for any future
  liveness-only lever.

- [s10] F7 (unconditional-common-store duplication into both branch arms) is
  INAPPLICABLE on its own prerequisites, not merely unpromising. F7 requires
  pre-existing if/else arms in which the duplicated stores' values are "real
  and required". This function's only real branches are the search loop's
  two exits and `if (i == 6) return;`; all of them merge ABOVE the tail, and
  the tail (the six lines writing D_800A3918[i] and the three D_801078xx
  words) is straight-line. Duplicating the tail stores into the i==6 arm
  would write the table when it is full — a semantic change. There is no
  legitimate F7 site here. (s7 already measured the sanctioned natural-arms
  family at honest floor 11.)

- [s10] F6+F7 SEAM IS MOOT, not unruled. Both banked seam forms re-measured
  on this chassis: dup-arms-bigconst-cond-ptr-a2-w2-a3-stray-li-11.c = 11
  (36 insns), dup-arms-sw3-attractor-idx-a1-w2-a2-ptr-coalesced-11.c = 11
  (33 insns) — identical to their s7 numbers. The seam construct (invented
  identical-arms branch, jump2 cross_jump-merged) is the ONLY thing that has
  ever homed 72 in $a3, and it costs 11 versus the floor of 4. A ruling that
  authorized the composition therefore could not close the function, so no
  ruling-request is filed; the seam is recorded dead on measurement, and the
  standing Judge constraint (2026-07-21 00:19 FAIL, manufactured branches)
  is not re-tested.

- [s10] DISPOSITION. Floor holds at 4; the unpark's stated lever is measured
  dead in both halves with the deleting pass named for each. src/code6cac_b.c
  was restored to its INCLUDE_ASM line (no source dirt). No escalation entry
  filed: the mandated modality is forensics, not escalation, and the 2026-07
  escalation ground is spent — a fresh disposition, if one is ever warranted,
  must be filed on post-2026-08-19 grounds by an escalation-modality session.

- [s8] The 2026-07-22 owner ruling (docs/grind/decisions.md:1292, option (b) REFUSED / OWNER-ACCEPTED INCOMPLETE) is SPENT: the 2026-08-19 owner-directed stale-park re-audit (docs/grind/borderline.md:68) unparked func_80033550 on the ground 'func_80033550 (F6+F7 seam - session must verify or ruling-request)'. Future sessions must not cite that escalation as a live disposition; the function is a normal active queue item again.

- [s8] Chassis re-measured after the asm-until-matched migration: main carries INCLUDE_ASM("asm/funcs", func_80033550) at src/code6cac_b.c:2673 and migration_pin.json pins floor 4; applying memory/grind/func_80033550/candidate.c verbatim scores sandbox --disable all = 4, 34/34 insns, rules_dropped 0. The floor is chassis-invariant, so the s1-s9 spelling conclusions still hold on this chassis.

- [s8] Per-pass insn census for the baseline candidate (tools/grinder/dump.ps1): .rtl=34 .jump=24 .cse=22 .loop=22 .cse2=22 .flow=21 .combine=20 .lreg=20 .greg=20. This is the reference series a future session should diff any probe against - it localises the deleting pass in one read.

- [s8] F6 empty-if is deleted by jump_optimize pass 1 (jump.c): .rtl 35 -> .jump 24 == baseline 24, and .lreg/.greg are byte-identical to baseline (2 allocnos, 72 conflicts {72 74 2 3 4 29}, 72 in $a1).

- [s8] F6 cancellation pairs are deleted by flow.c life-analysis DCE (.cse2 24 -> .flow 21 == baseline 21) whenever the cancelled value is not consumed across the pointer's live range - byte-free but RA-inert.

- [s8] When an F6 cancellation pair DOES overlap the pointer's live range it survives into global.c as a real conflicting allocno, but .greg then prints `72 preferences: 4` and 72 loses its hard-reg-4 conflict: the entry copy `move a1,a0` becomes coalescable and find_reg takes the preferred $a0 before the numeric scan, deleting the entry move (33 insns) at distance 6-7. Measured across 8 overlapping placements; never byte-free, always toward $a0.

- [s8] THEOREM (refines the s6 closure theorem with s9's preference finding): pseudo 72 is the destination of the function's only hard-reg copy (72 <- $a0; s9 proved $a0 is the only hard GPR anywhere in this call-free single-parameter function's pre-RA RTL, so no $a3 preference is C-reachable). 72 therefore has exactly two allocation regimes - (A) the copy is not coalescable, 72 conflicts with hard 4 and the numeric scan gives $a1, and no sanctioned construct seats an allocno in $a1/$a2 byte-free; or (B) enough liveness is injected that the copy coalesces, `72 preferences: 4` wins before the scan and 72 lands in $a0 at byte cost. Conflict injection is precisely what flips (A) into (B). $a3 is unreachable from either regime, which predicts the same outcome for any future liveness-only lever.

- [s8] F7 has no legitimate site in this function: the only real branches (the search loop's two exits and `if (i == 6) return;`) merge above the straight-line tail, and duplicating the tail stores into the i==6 arm would write the table when it is full - the 'values real and required' prerequisite fails.

- [s8] Both banked F6+F7 seam forms re-measured chassis-invariant at honest 11 (36 and 33 insns) versus the floor of 4, so the seam cannot close the function even if it were sanctioned - which is why this session files no ruling-request.

- [s8] src/code6cac_b.c was restored to its INCLUDE_ASM line at end of session; git status shows no source dirt (only metrics/events.jsonl, engine-written).

- [s9, 2026-08-20, escalation modality] CHASSIS RE-MEASURED. main carries
  `INCLUDE_ASM("asm/funcs", func_80033550);` at src/code6cac_b.c:2673 and
  scores 34 with no C body (tmp/grind/func_80033550/s9/sandbox_baseline_include_asm.json).
  Applying candidate.c verbatim: sandbox --disable all = 4, target_insns 34,
  build_insns 34, rules_dropped 0, zero cheat-asm
  (tmp/grind/func_80033550/s9/sandbox_candidate.json). The floor is unchanged
  at 4 and is chassis-invariant across the asm-until-matched migration.
  NOTE for future sessions: candidate.c is stored with CRLF line endings, so
  applying it into src/*.c requires a CRLF->LF normalisation pass afterwards
  or the toolchain silently sees a CRLF block.

- [s9] THE RESIDUAL, QUANTIFIED AS A REGISTER-NUMBER ARITHMETIC (new framing,
  measured on this chassis from tmp/grind/func_80033550/dumps/code6cac_b.greg:22914):
      ;; 2 regs to allocate: 74 72
      ;; 72 conflicts: 72 74 2 3 4 29        <- NO `72 preferences:` line
      ;; Register dispositions: 72 in 5  74 in 3  75 in 3  76 in 4  77 in 5
  Pseudo 72 is the incoming pointer. Preference set EMPTY (call-free,
  single-parameter -> global.c set_preference never sees a hard-reg SET other
  than the entry copy), so find_reg falls through to the numeric scan
  $v0,$v1,$a0,$a1,$a2,$a3 and takes the first register outside {2,3,4,29} =
  **$a1 (5)**. Target seats it in **$a3 (7)**. Therefore closing this function
  requires 72's hard-conflict set to grow to a superset of {2,3,4,5,6}: TWO
  additional register occupants, seated in $a1 AND $a2, live across the
  pointer's range, emitting ZERO bytes, WHILE the hard-4 conflict is preserved.
  That is the precise, falsifiable statement of the wall — any future lever
  must be evaluated against it directly, and s8's (A)/(B) regime measurement
  shows the two halves of the requirement are mutually exclusive under the
  observed mechanism (any liveness reaching 72's range makes the entry copy
  coalescable, `72 preferences: 4` appears, the hard-4 conflict vanishes, and
  find_reg takes $a0 before the scan — 33 insns, distance 6-7, measured over 8
  placements).

- [s9] FRONTIER ITEM 2 (phantom-frame-slot volatile pad) is KILLED on its
  prerequisite, not merely "likely to fail". Target func_80033550 has NO STACK
  FRAME AT ALL: across all 34 instructions of asm/funcs/func_80033550.s there
  is no `addiu $sp,$sp,-N`, no callee-save store, and the epilogue is
  `jr $ra` + `nop`. There is consequently no untouched target stack slot for a
  pad local to reserve, and any pad local would ADD a frame (prologue +
  epilogue instructions) = added bytes. The phantom-slot family's prerequisite
  is unmet by direct inspection of the target; do not re-open it.

- [s9] AND-GATE #1 RE-MEASURED: `tools/scan_hand_coded.py --single
  func_80033550` = tier=LOW score=0/8 (tmp/grind/func_80033550/s9/scan_hand_coded.txt),
  every one of S1-S8 negative. Unchanged from s8 on the new chassis.

- [s9] AND-GATE #2 SEARCHED AND NEGATIVE: docs/reference/sotn-construct-index.md
  (1,365 PSX-master entries) has no entry for a byte-free REGISTER occupant.
  The only adjacent family is `pad_dummy_local` (index line 29, 816 entries) —
  a FRAME-SLOT family whose prerequisite fails here per the no-stack-frame
  finding above. A negative census is a FAILED gate, not an open question.

- [s9] DISPOSITION FILED. Both endgame-lock AND-gates fail, so the owner's
  standing 2026-07-27 auto-ruling applies: appended
  `## 2026-08-20 — func_80033550 (src/code6cac_b.c) — **OWNER-ESCALATION —
  RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED
  INCOMPLETE**` at docs/grind/decisions.md:8216, filed on POST-2026-08-19
  grounds (the unpark's stated F6+F7 ground is spent/measured dead), and
  returned result=owner-gated. TERMINAL — nothing pending on the owner. Any
  future unpark must name a NEW lever and must clear the {2,3,4,5,6}
  conflict-set arithmetic above. src/code6cac_b.c restored to its INCLUDE_ASM
  line; no source dirt.

- [s9] Chassis: main carries INCLUDE_ASM("asm/funcs", func_80033550); at src/code6cac_b.c:2673 and scores 34 with no C body; candidate.c applied verbatim scores sandbox --disable all = 4 with 34/34 insns, 0 rules, 0 cheat-asm. No cheat is holding anything on main - the function is simply four base-register diffs short.

- [s9] The residual is now stated as a falsifiable register-number arithmetic (measured from tmp/grind/func_80033550/dumps/code6cac_b.greg:22914): '2 regs to allocate: 74 72'; '72 conflicts: 72 74 2 3 4 29' with NO '72 preferences:' line; dispositions '72 in 5'. Pseudo 72 is the incoming pointer; its preference set is empty because the function is call-free and single-parameter, so global.c find_reg falls through to the numeric scan v0,v1,a0,a1,a2,a3 and takes the first register outside {2,3,4,29} = $a1.

- [s9] Seating the pointer in target's $a3 (7) therefore requires 72's hard-conflict set to become a superset of {2,3,4,5,6}: TWO additional register occupants, seated in $a1 AND $a2, live across the pointer's range, emitting zero bytes, WHILE the hard-4 conflict is preserved. s8 measured the two halves to be mutually exclusive: any liveness injection reaching 72's range makes the entry copy 'move a1,a0' coalescable, '72 preferences: 4' appears, the hard-4 conflict vanishes, and find_reg takes $a0 before the numeric scan (33 insns, distance 6-7) - measured across 8 placements, never byte-free, never toward $a1/$a2/$a3.

- [s9] Target has no stack frame: no addiu $sp adjustment, no callee-save store, epilogue jr $ra + nop, across all 34 instructions. This kills the phantom-frame-slot pad frontier item on its prerequisite.

- [s9] scan_hand_coded.py --single func_80033550 = tier LOW score 0/8, all of S1-S8 negative (tmp/grind/func_80033550/s9/scan_hand_coded.txt).

- [s9] SOTN-master construct index census negative for a byte-free register occupant; pad_dummy_local (line 29) is a frame-slot family and inapplicable here.

- [s9] The 2026-08-19 unpark ground (docs/grind/borderline.md:68, 'F6+F7 seam - session must verify or ruling-request') is spent: s8/s10 forensics measured F6 dead in both halves (empty-if deleted by jump_optimize pass 1; cancellation pair either flow.c-DCE'd inert or flips the entry copy to coalescable) and F7 inapplicable on prerequisites (the tail is straight-line; duplicating into the i==6 arm writes the table when it is full - a semantic change). Both banked seam forms re-measure chassis-invariant at 11 > 4.

- [s9] Exhaustion: 8 prior sessions, 4 distinct modalities (recon, structural x2, permuter x2, forensics x3), ~138k cumulative permuter iterations across 6 basins all converging on the score-20 ptr=$a1 attractor with zero sub-20 finds, 17 rejected forms banked.

- [s9] Disposition filed this session at docs/grind/decisions.md:8216 as '## 2026-08-20 - func_80033550 (src/code6cac_b.c) - OWNER-ESCALATION - RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE', on post-2026-08-19 grounds (not a re-citation of the spent 2026-07-22 ruling at decisions.md:1292). Terminal; nothing pending on the owner.

- [s9] src/code6cac_b.c was restored to its INCLUDE_ASM line at end of session; git status shows only docs/grind/decisions.md (this session's entry) and metrics/events.jsonl (engine-written).

- [s9] Housekeeping for future sessions: memory/grind/func_80033550/candidate.c is stored with CRLF line endings - normalise to LF after pasting it into src/*.c or the toolchain silently sees a CRLF block.

## s10 (2026-08-25, escalation modality — owner directive 2026-08-24, F1 chain-extender)

**Chassis.** HEAD carries `INCLUDE_ASM("asm/funcs", func_80033550);` at
src/code6cac_b.c:1855. candidate.c spliced in verbatim (LF-normalised):
`sandbox func_80033550 --disable all` = score 4, target 34, build 34,
rules_dropped 0, cheat_asm_stripped 47 (whole-TU figure, not this function).
src/code6cac_b.c restored to HEAD at end of session.

**The residual, restated from the objdump diff (exact, 4 insns).**
build `move a1,a0` / `lw v1,0(a1)` / `lw a0,4(a1)` / `lw a1,8(a1)`
target `addu $a3,$a0,$zero` / `lw $v1,0($a3)` / `lw $a0,4($a3)` / `lw $a1,8($a3)`
Everything else is byte-identical, including the loop, the `sb`, the
sll/addu/sll index chain and all three `lui/addu/sw` store triples.

**NEW: the exact allocation model (ra_solver extract, first time banked).**
`tmp/ra_solver_work/func_80033550.model.json` — only TWO global allocnos:
  * 74 = `i` : nrefs 9, livelen 12, pri 22500, hard conflicts {2,29} -> $v1
  * 72 = arg0: nrefs 5, livelen 16, pri  6250, hard conflicts {2,3,4,29} -> $a1
  * everything else is LOCAL-alloc'd and already matches target:
    75(w0)->$v1, 76(w1)->$a0, 77(w2)->$a1, and the index chain ->$v0.
  * prefs / full_prefs / copy_prefs are EMPTY for both allocnos.
  * prera_hard = [4] (the incoming $a0 parameter).
So the seat is decided entirely by find_reg's ascending scan over
{2,3,4,29}-minus: $a1 is the first free register. Target's $a3 requires the
hard-conflict set to contain 5 and 6 as well — a LIVE-RANGE fact, not a
priority fact. MIPS defines no REG_ALLOC_ORDER in this tree
(tools/ra_solver/README.md), so the scan really is numeric-ascending.

**Solver verdict (typed, first ever for this function).**
`inverse.py global ... --goal '{"72": 7}'` at depth 2 and depth 3:
NEGATIVE over 48 atoms in 5 classes, plus a FORECLOSED verdict on the
preference route ("$a3 never appears as a hard reg in this function's pre-RA
RTL, so global.c set_preference can never record a preference for it").
Honest scope limit: CONFLICT_ADD atoms are pseudo-pseudo only; the solver does
not model "add a HARD conflict on $a1/$a2", which is the one axis left, and
that axis is the previously-measured closed channel.

**Chain-extender measurements (the directive).** Three modes, all 34/34,
all score 4 — banked as rejected/s10-f1-chain-*.c. Mode A (plain alias) is
deleted before flow.c and never bumps refs. Mode B (folding detour on all
three loads) survives combine only by replacing the entry copy
(`addiu a1,a0,4` where target has `addu a3,a0,zero`) — count-neutral, not
byte-neutral. Mode C (partial detour) splits the pointer into two global
allocnos at 34 insns (72->$a0, 79->$a1) — the first time a second register
occupant ever survived to RA here — but it, too, pays for itself with the
entry-copy slot, and it moves arg0 to $a0, away from target. The law: with the
three loads already in direct base+offset form and arg0 runtime-dependent, a
detour is either folded pre-flow (inert) or slot-substituting (byte-changing).

**Gates.** scan_hand_coded LOW 0/8 (all of S1-S8 negative). No SOTN-master
precedent for a byte-free register occupant (s9 census, re-affirmed by mode C's
measurement that the occupant is not byte-free).

- [s10] Chassis: HEAD carries INCLUDE_ASM("asm/funcs", func_80033550); at src/code6cac_b.c:1855; candidate.c applied gives sandbox --disable all = score 4, 34/34 target/build insns, rules_dropped 0. src/code6cac_b.c restored to HEAD at end of session.

- [s10] The residual is exactly 4 instructions (objdump diff): build `move a1,a0` / `lw v1,0(a1)` / `lw a0,4(a1)` / `lw a1,8(a1)` vs target `addu $a3,$a0,$zero` / `lw $v1,0($a3)` / `lw $a0,4($a3)` / `lw $a1,8($a3)`. All other 30 instructions are byte-identical, including the search loop, the sb, the sll/addu/sll index chain and all three lui/addu/sw store triples.

- [s10] FIRST ra_solver model banked for this function: only TWO global allocnos — 74 = i (nrefs 9, livelen 12, pri 22500, hard conflicts {2,29}) -> $v1 which MATCHES target, and 72 = arg0 (nrefs 5, livelen 16, pri 6250, hard conflicts {2,3,4,29}) -> $a1 where target has $a3. prefs / full_prefs / copy_prefs are EMPTY for both; prera_hard = [4]. Everything else is local-alloc'd and already matches target (w0->$v1, w1->$a0, w2->$a1, index chain->$v0).

- [s10] MIPS defines no REG_ALLOC_ORDER in this tree (tools/ra_solver/README.md), so find_reg's scan is numeric-ascending: $a1 is simply the first register not in {2,3,4,29}. Reaching target's $a3 requires 5 AND 6 in the hard-conflict set — a live-range-overlap fact, not a priority fact, which is why every priority lever (including the chain-extender) is structurally inert on this residual.

- [s10] inverse.py returns NEGATIVE at depth 2 and depth 3 for goal {72: $a3}, and FORECLOSES the preference route with a named mechanism ($a3 never appears as a hard reg in the pre-RA RTL, so set_preference can never name it).

- [s10] Chain-extender mode (C) is the first spelling in ten sessions to seat a SECOND surviving global allocno at 34 instructions — and it prices the occupant precisely: it consumes the entry-copy slot, converting `addu a3,a0,zero` into `addiu a1,a0,8`. That is direct measurement that the byte-free-occupant channel is closed in this shape, replacing the previous narrative argument.

- [s10] Gate #1: scan_hand_coded --single func_80033550 = tier LOW, score 0/8 (tmp/grind/func_80033550/s10/scan_hand_coded.txt). Corroborated by s6's finding that cc1psx is instruction-identical on this C — the divergence is ordinary GCC register allocation, not hand-written assembly.

- [s10] Gate #2: no SOTN-master precedent for a byte-free register occupant (s9 census of the 1,365-entry PSX index); pad_dummy_local is a frame-slot family and target func_80033550 has no stack frame (no $sp adjustment, no callee-save, epilogue is jr $ra + nop).

- [s10] Exhaustion: floor FLAT at 4 across ten sessions and five distinct modalities (recon, structural, permuter, forensics, escalation); ~138k cumulative permuter iterations over 6 basins all converging to the score-20 ptr=$a1 attractor; 20 rejected forms banked in memory/grind/func_80033550/rejected/.

- [s10] Disposition filed by this session at docs/grind/decisions.md:12231 — OWNER-ESCALATION RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE. No standard-lowering packet was filed: the only YES that would close this function is a no-precedent family grant for a byte-free register occupant, which is the pre-decided-NO auto-reject class under the owner's second 2026-08-24 ruling.

## s11 (2026-08-30, ESCALATION modality — disposition re-affirmed after owner ruling 10)

- **Chassis re-measured, not quoted.** candidate.c over the INCLUDE_ASM line at src/code6cac_b.c:2012
  → `sandbox func_80033550 --disable all` = score 4, target_insns 34, build_insns 34, rules_dropped 0.
  src reverted to INCLUDE_ASM before the session ended; tree clean.
- **The residual, exactly (new artifact — first 1:1 correspondence banked here).**
  tmp/grind/func_80033550/s11/insn_correspondence.txt. 34 vs 34, THIRTY identical instructions; the
  four differing ones are stream indices 0, 20, 21, 22 and differ in one register field only:

        idx  target                      build
         0   addu $a3, $a0, $zero        move a1,a0
        20   lw   $v1, 0x0($a3)          lw v1,0(a1)
        21   lw   $a0, 0x4($a3)          lw a0,4(a1)
        22   lw   $a1, 0x8($a3)          lw a1,8(a1)

  Both builds seat the three loaded words identically ($v1, $a0, $a1) — the ONLY divergence in the
  whole function is the pointer's home. No ordering, frame, or count divergence.
- **Why $a1 is free for w2 (mechanism, previously unstated):** w2 is born at index 22, exactly where
  the pointer's live range ends, so it never conflicts with pseudo 72. The function's entire
  register-carrying population is $v0 (loop/index temp), pseudo 74 (`i`) and the three loaded words;
  {2,3,4} is the largest hard-conflict set they can produce, which is precisely what `.greg` shows.
- **Instruction-budget closure.** Because all 34 target slots are already occupied by
  semantically-required operations that our C reproduces byte-for-byte, any additional register
  carrier must take a 35th slot or substitute for one of the 34. s10 already priced the substitution
  three ways (the surviving F1 detour always eats the entry-copy slot). Therefore a byte-free
  occupant cannot exist in this shape — the s6 source-level closure theorem now has a counting
  proof at the byte level.
- **Preference route closed by fidelity too.** s10 typed route (ii) FORECLOSED because $a3 never
  appears as a hard reg pre-RA; the one legitimate way to introduce it is a 4th declared parameter
  (`expand_function_start` parm copy). The sole caller `func_800290B8` (asm/funcs/func_800290B8.s:203)
  sets ONLY $a0 in the jal delay slot, so the function is genuinely one-parameter and a wider
  signature would be a false fidelity claim in addition to being inert.
- **Both endgame-lock AND-gates re-measured FAILING.** scan_hand_coded tier=LOW score=0/8, S1-S8 all
  negative (tmp/grind/func_80033550/s11/scan_hand_coded.txt); no in-hand SOTN-master precedent for a
  byte-free register occupant (the adjacent pad_dummy_local family is frame-slot-based and the target
  has no stack frame at all).
- **Disposition filed:** docs/grind/decisions.md:15517 — OWNER-ESCALATION RESOLVED BY STANDING RULING
  (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE. Nothing pends the owner. Ruling 10's bookkeeping
  ground is discharged. Re-open only on (1) an `inverse.py` HARD_CONFLICT_ADD atom class returning
  REACHABLE for `--goal {"72": 7}` (operator/infra; `tools/` is out of scope for grind sessions), or
  (2) an owner-landed family that keeps a value alive in a register across another value's live range
  without emitting an instruction — and it must beat the entry-copy-slot price s10 measured.

- [s11] Chassis floor re-measured THIS session: sandbox func_80033550 --disable all = score 4, target_insns 34, build_insns 34, rules_dropped 0, zero cheat-asm. src/code6cac_b.c was reverted to INCLUDE_ASM before the session ended; the tree is clean apart from ledger/docs files.

- [s11] First 1:1 position-aligned disassembly correspondence banked for this function: 34 vs 34, thirty instructions identical, four differing (idx 0, 20, 21, 22) in exactly one register field — target $a3 vs build $a1. No ordering, frame or count divergence.

- [s11] Instruction-budget closure: because all 34 target slots are already occupied by semantically-required operations our C reproduces byte-for-byte, any additional register carrier must take a 35th slot or substitute for one of the 34 — so no byte-free occupant CAN exist in this shape. This upgrades s6's source-level closure theorem and s9's arithmetic restatement from 'none found' to 'none can exist'.

- [s11] Mechanism for the free $a1 seat, previously unstated in the ledger: w2 is born at stream index 22, exactly where the pointer's live range ends, so it never conflicts with pseudo 72; the entire carrier population ($v0, pseudo 74 = i, three loaded words) can only produce the hard-conflict set {2,3,4} that .greg reports.

- [s11] Preference route to $a3 is now closed by fidelity as well as mechanism: the sole caller func_800290B8 (asm/funcs/func_800290B8.s:203) sets only $a0 in the jal delay slot, so the 4-ary signature that would put $a3 in the pre-RA RTL is a false claim about the original.

- [s11] scan_hand_coded --single func_80033550 re-run this session: tier=LOW score=0/8, S1-S8 all negative (tmp/grind/func_80033550/s11/scan_hand_coded.txt).

- [s11] Disposition FILED this session at docs/grind/decisions.md:15517 as OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE. It asks for no standard to be lowered (no family grant, no canonical evidence-bar override, no 'accept the debt' wording), so it is not auto-reject class; nothing pends the owner.

- [s11] Owner ruling 10 of the 2026-08-30 escalation batch (decisions.md:14870) returned this item to ACTIVE on a bookkeeping ground only ('the queue lagged the ledger'), naming no new lever; that ground is discharged by this session's fresh measurements and the new entry.

## s12 (2026-09-01, STRUCTURAL modality — owner Ruling-A named probe EXECUTED, plus a measured RA model)

**Chassis re-measure.** HEAD is `INCLUDE_ASM("asm/funcs", func_80033550);`
(src/code6cac_b.c:2276). `memory/grind/func_80033550/candidate.c` pasted over that line
(plus three `extern u8 D_80107850/54/58;` declarations, which exist only in
`undefined_syms_auto.txt:994-996`, not in any header) measures
**score 4, target_insns 34, build_insns 34, rules_dropped 0** — the ledger floor holds on
this chassis. src/ was restored to HEAD at session end (git clean except metrics/events.jsonl).

### 1. Owner Ruling-A named probe — duplicated-statement-into-arms on the search-loop's REAL branches: **KILLED, three separate measured mechanisms**

The reopen note (hypotheses.md:719) directed a re-test of the sanctioned
duplicated-statement-into-arms family against "the search loop's two-exit tail", on the
ground that the s8 Judge FAIL only rejected an *invented* branch. This function has exactly
two real branch sites, and both were probed:

**Site 1 — the loop's two exits (`beqz` early-found vs the `i<6` fall-through), merging at
.L80033580.** Variants dR1 (duplicate `w0=arg0[0]` into both exits, **11**), dR2 (all three
loads, **12**), dR3 (`w1`+`w2`, **20**), dR4 (loads in arms + do-while wrap in tail, **18**),
dR7 (duplicate `new_var = i*12` into both exits, **16**, 33 insns).
All stay at 33-34 insns, so the duplication IS count-free — but it is **not tail-free**:
the two loop exits merge **UPSTREAM of the `i == 6` test**, so cross_jump hoists the merged
copy out of the tail region entirely. dR4's disassembly
(tmp/grind/func_80033550/s12/, `dis.sh`) shows the merged `lw a3,4(a0); lw a2,8(a0)` landing
*before* `beq a1,v0` — i.e. executing on the not-found return path — and the resulting
rotation coalesces the pointer into **$a0** (the entry copy is deleted, which is what keeps
the count at 34) with `i` rotated to $a1. This is the structural reason the family cannot
work at site 1: any statement duplicated into the loop's arms is relocated ahead of the
tail, so it buys conflicts in the wrong region and pays with the entry copy.

**Site 2 — the real `if (i == 6) return;` branch.** dR5 (duplicate all three loads into the
return arm, **5**, 34 insns) and dR6 (duplicate the real `i==6` exit test into both loop
arms, **5**, 34 insns): the duplicated copies are dead on the return path and are deleted
before conflict construction — **RA-inert**, `.greg` seating unchanged, pointer still $a1.
The only residue is a sched1 side-effect (the final `sll v0,v0,2` migrates behind the three
`lw`s once the do-while(0) fence is dropped) worth exactly +1.

**Site 2 with a non-DCE-able statement.** The only statements at this site that survive DCE
are the four stores, and duplicating any of them into the `i == 6` arm writes
`D_80107850[6*12]` on the not-found path — **semantically invalid C**, so it is not a
candidate at all. Measured anyway as a diagnostic (dX): it does not even cross-jump merge
(**44 insns**, score 20, the arms' tails differ), and it coalesces the pointer into $a0
rather than seating it in $a3.

**Ruling-A verdict.** The s7 `dupU` result does NOT transfer to a real branch. `dupU`'s
byte-freeness depended on an invented always-merged branch wrapping *identical* statement
sequences whose only asymmetry was the merged-away condition; both of this function's real
branches have **asymmetric arms** (one returns, one falls through), so cross_jump can never
merge them away completely. At site 1 the merge point is upstream of the tail; at site 2 the
duplicable statements are all DCE-able and the non-DCE-able ones are semantically excluded.
The owner's ground for reopening ("the s11 counting theorem is contradicted by dupU") is
answered: dupU was count-free only inside a shape the Judge has already ruled a cheat, and
it never reached distance <= 4 anyway (honest 11).

### 2. NEW — the residual's RA arithmetic is now MEASURED, not argued (three constructions)

s9/s10/s11 argued from `.greg` that reaching $a3 needs hard conflicts
superset-of {2,3,4,5,6}. This session built the constructions and measured them.

- **`$a1` is blocked exactly by extending the pointer's live range past the last load.**
  dS4 = base + a trailing `arg0[0] = w0;` (35 insns, score 5). Disassembly
  (s12/dS4.dis.txt): `move a2,a0`, `lw v1,0(a2)`, `lw a0,4(a2)`, `lw a1,8(a2)` — every seat
  target-correct (idx=v0, w0=v1, w1=a0, w2=a1, i=v1) and the pointer moved **$a1 -> $a2**.
  Cost: one instruction. Reason: w2 (the last load) normally *reuses* the pointer's register
  because the pointer dies in that very insn, so it is not a conflict; extend the range and
  it becomes one, and find_reg's ascending scan lands on the next free seat, $a2.
- **No load-order permutation blocks $a1 for free.** dS5 (`w2` first) and dS6 (`w0,w2,w1`)
  both measure 4 at 34 insns, and dS5's disassembly shows the emitted order following the
  source (`lw a0,8(a1); lw v1,0(a1); lw a1,4(a1)`) with the **last-emitted** load again
  taking the pointer's own register. Law: whichever load is emitted last reuses the
  pointer's hard register, so $a1 can only be blocked by an extra use, never by reordering.
  dS1 (all three stores read `arg0[...]` directly, 33 insns, score 9) and dS3 (33 insns,
  score 8) fold the loads into the store operands and lose the count instead.
- **The full seating is reachable, and its price is now known exactly.** dZ (DIAGNOSTIC,
  not a candidate — it adds a fourth load and two stores, 37 insns, score 10) gives the
  pointer a fifth simultaneously-live block-local value plus the live-range extension, and
  the pointer seats in **$a3**: `move a3,a0` / `lw a2,0(a3)` / `lw v1,4(a3)` /
  `lw a0,8(a3)` — the first ordinary, semantically-valid C in this function's history to
  reproduce the target's entry copy AND all three target base registers.
  **So the requirement is exactly: FIVE block-local values live simultaneously with the
  pointer (occupying v0,v1,a0,a1,a2 by local-alloc's ascending first-free scan) plus a
  pointer use after the last of them.** The target affords only four (idx, w0, w1, w2), and
  dZ needed three extra instruction slots to buy the fifth and the extension.

This sharpens s11's instruction-budget closure from a counting argument into a named,
constructed one: the missing ingredient is a **byte-free fifth block-local allocno whose
defining insn is deleted after local-alloc** (post-RA deletion: cross_jump / delay-slot fill
/ final), **together with a byte-free pointer use after it**. Both must be free; dS4 proves
the extension alone already costs a slot when spelled as a store, and s7 proved the only
known post-RA-deletion channel (arm duplication + cross_jump) is unavailable here at real
branches (section 1).

Artifacts: tmp/grind/func_80033550/s12/{apply.py,sweep.ps1,dis.sh,variants/*.c,
dR5.dis.txt,dS4.dis.txt,dS5.dis.txt,dX.dis.txt,dZ.dis.txt}.
Banked forms: memory/grind/func_80033550/rejected/s12-*.c (9 files).

- [s12] Chassis re-measured this session: candidate.c pasted over the INCLUDE_ASM at src/code6cac_b.c:2276 (plus three `extern u8 D_80107850/54/58;` decls, which live only in undefined_syms_auto.txt:994-996 and in no header) gives score 4, target_insns 34, build_insns 34, rules_dropped 0. src/ was restored to HEAD at session end; the tree is clean apart from the ledger and metrics/events.jsonl.

- [s12] Owner Ruling-A (the reopen note at memory/grind/func_80033550/hypotheses.md:719) is EXECUTED IN FULL and KILLED: duplicated-statement-into-arms is unavailable at this site not by policy but by structure - the loop's two exits merge upstream of the i==6 test, and the i==6 arm's duplicable statements are all DCE-able while its non-DCE-able ones are semantically invalid.

- [s12] The owner's stated ground for reopening (that s11's counting theorem is contradicted by s7's dupU) is answered on the merits: dupU was count-free only inside the invented-symmetric-arm shape the Judge already ruled a cheat, it required symmetric arms that neither real branch here provides, and it never reached distance <= 4 anyway (honest 11).

- [s12] MEASURED RA LAW (new): the last-emitted load of arg0 always reuses the pointer's hard register, because the pointer dies in that insn and is therefore not a conflict. Verified across load-order permutations (dS5/dS6) - the reuse follows emission order, not source order.

- [s12] MEASURED RA LAW (new): blocking $a1 == extending the pointer past the last load. Spelled honestly (dS4, one trailing store) it costs exactly one instruction and moves the pointer $a1 -> $a2 with every other seat target-exact.

- [s12] MEASURED RA LAW (new): the pointer reaches $a3 iff FIVE block-local values are live simultaneously with it (filling v0,v1,a0,a1,a2 by local-alloc's ascending first-free scan) AND the pointer is used after the last of them. The target affords only four such values (idx,w0,w1,w2); dZ needed three extra instruction slots to buy the fifth and the extension.

- [s12] Consequently the residual is now a fully specified shopping list rather than an unexplained register seat: (i) a byte-free fifth block-local allocno whose defining insn is deleted AFTER local-alloc (post-RA deletion channels only: cross_jump, delay-slot fill in reorg.c, final.c), and (ii) a byte-free use of the pointer after that allocno's last use. s7 + s12 together close the only known post-RA-deletion channel (arm duplication + cross_jump) at both of this function's real branch sites.

- [s12] 9 forms banked to memory/grind/func_80033550/rejected/s12-*.c, each named for the mechanism that killed it; two of them (dS4, dZ) are positive-result diagnostics kept for their disassembly, not proposals.

## s13 (2026-09-01) — ESCALATION modality — DISPOSITION

- [s13] Chassis re-measured this session: `memory/grind/func_80033550/candidate.c` pasted over the
  `INCLUDE_ASM` at src/code6cac_b.c:2276 (plus the three `extern u8 D_80107850/54/58;` decls, which
  live only in undefined_syms_auto.txt:994-996) gives **score 4, target_insns 34, build_insns 34,
  rules_dropped 0**. src/ restored to HEAD at session end; tree clean apart from metrics/events.jsonl.

- [s13] The owner's 2026-09-01 Ruling-A directive is SPENT: s12 executed the named probe
  (duplicated-statement-into-arms at the search loop's real two-exit tail) in full over 7 variants
  and killed it at both real branch sites. Nothing in the reopen note remains unexecuted except the
  "Ruling C lane" fallback (adding a HARD_CONFLICT_ADD atom to tools/ra_solver/inverse.py), which is
  a TOOL edit and therefore outside a grind session's allowed surface — it is an operator task, and
  it is recorded as a re-activation trigger in the decisions entry rather than attempted here.

- [s13] NEW MEASUREMENT — the target's own register census (tmp/grind/func_80033550/s13/
  target_register_census.txt): the TARGET seats $a3=arg0, $v1=i then w0, $v0=probe byte / const 6 /
  const 1 / idx, $a0=w1, $a1=w2, $at=fixed scratch — and **$a2 never appears in any of the 34
  instructions**. So the target has exactly FOUR block-local values live with the pointer, the same
  four our build has, in the same four registers.

- [s13] CONSEQUENCE: s12's "five block-locals" seating law is **sufficient but not necessary**.
  Under an ascending find_reg scan (confirmed this session: `REG_ALLOC_ORDER` is undefined in
  tools/gcc-2.7.2/config/mips/mips.h, so global.c scans 0..FIRST_PSEUDO_REGISTER in plain numeric
  order), four conflicts {$v0,$v1,$a0,$a1} = {2,3,4,5} seat a pointer at **$a2**, not $a3. The
  original therefore carried a fifth conflict, at $a2, that **emits no instruction**. The byte-free
  occupant is thus PROVEN to have existed in the original's pre-RA RTL — it is not a hypothetical
  device — and the residual is exactly our inability to spell one, not a proof that none can exist.

- [s13] Endgame-lock AND-gates re-measured on this chassis, both FAIL. Gate (a) canonical-asm:
  `scan_hand_coded --single func_80033550` = **tier=LOW score=0/8**, S1-S8 all negative
  (s13/scan_hand_coded.txt). Gate (b) SOTN precedent: fresh census of
  docs/reference/sotn-construct-index.md (2,746 lines) returns **zero** PSX-master entries for a
  byte-free register occupant / conflict injection; the only adjacent family, `pad_dummy_local`
  (index line 29), is frame-slot-based and this target has no stack frame at all.

- [s13] Standing 2026-07-27 ruling applied (.claude/rules/endgame-lock-disposition.md) with the
  2026-08-31 silent-foreclosure shape: proof-of-foreclosure record appended to docs/grind/decisions.md.

- [s13] Chassis re-measured this session: candidate.c applied over the INCLUDE_ASM at src/code6cac_b.c:2276 (plus the three extern u8 D_80107850/54/58 decls from undefined_syms_auto.txt:994-996) gives score 4, target_insns 34, build_insns 34, rules_dropped 0. src/ restored to HEAD; tree clean apart from ledger/docs edits and metrics/events.jsonl.

- [s13] TARGET register census (new, tmp/grind/func_80033550/s13/target_register_census.txt): $a3=arg0, $v1=i then w0, $v0=probe byte / const 6 / const 1 / idx, $a0=w1, $a1=w2, $at=fixed scratch, and $a2 NEVER appears in any of the 34 instructions.

- [s13] REG_ALLOC_ORDER is undefined in tools/gcc-2.7.2/config/mips/mips.h (grep-verified), so global.c find_reg scans hard registers in plain ascending numeric order - confirming the scan model the ledger has assumed since s9.

- [s13] Consequence: four conflicts {$v0,$v1,$a0,$a1}={2,3,4,5} seat a pointer at $a2; the original reached $a3 with only four block-locals, so its pre-RA RTL carried a fifth conflict at $a2 that emits NO instruction. The byte-free occupant is PROVEN to have existed - the residual is our inability to spell one, not a proof that none can exist.

- [s13] scan_hand_coded --single func_80033550 = tier LOW score 0/8, S1-S8 all negative (s13/scan_hand_coded.txt).

- [s13] SOTN-master census over docs/reference/sotn-construct-index.md returns zero PSX entries for a byte-free register occupant / conflict injection; pad_dummy_local is frame-slot-based and this target has no stack frame.

- [s13] Exhaustion: 13 sessions, 6 modalities (recon, structural, permuter, forensics, escalation, disposition), floor flat at 4 throughout, ~138k cumulative permuter iterations across 6 basins, 31 rejected forms banked in memory/grind/func_80033550/rejected/.

- [s13] Foreclosure record filed this session at docs/grind/decisions.md:18726.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=13
