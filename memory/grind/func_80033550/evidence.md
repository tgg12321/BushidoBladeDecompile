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
