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
