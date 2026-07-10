# Closer Phase 2 — close-out session: measurements, kills, verdicts (2026-07-09)

One line: **W3 ground truth landed (BB2 links Sony's PsyQ 4.0 BIOS.OBJ verbatim —
the twins are Sony library object code); W1 fixed the branch-destination residual
HONESTLY (faithful self-contained-arm structure), banked a strictly-better
masked-2 candidate with fewer FAKEs; the last transposition's full mechanism is
now named end-to-end, and the faithful-source standalone PROVES the window
(seats + pair order) falls out of the literal v1.86 source — blocked in our
fork only by a named fp-allocatability divergence with an in-context port
roadmap.**

## W3 — LIBCD.LIB ground truth (kill criterion MET, verdict decisive)

Full memo: memory/closer/libcd-groundtruth.md (written by the W3 agent).
- PsyQ 4.0 LIBCD.LIB's BIOS module = bios.c **v1.86 1997/03/28** (exact BB2 rcsid).
  All 1,526 words of its .text are masked-identical to BB2 0x80080828–0x80082000
  (995 bit-exact, 531 identical outside reloc fields). **BB2 links the SDK
  object verbatim** — Sony's build IS the target; Lightweight never compiled it.
- XDEF symbols: CD_sync@0x80080DB0 == cpu_side_move_dir_4 (160/160),
  CD_ready@0x80081030 == marionation_Exec (179/179).
- v1.77 (PsyQ 3.5) / v1.80 (3.6) objects differ from v1.86 only in li-spelling
  (ori vs addiu) — **the C source did not change v1.77→v1.86**, so
  sotn-decomp's bios.c is a faithful shape reference.
- SYMBOL CORRECTIONS: saEft01Init == **CD_datasync** (not CD_cw);
  tslTm2LoadImage@0x800812FC == CD_cw.
- Parsers banked: tmp/closer/psyq_lib.py (SN LIB/OBJ), tmp/closer/compare_groundtruth.py.
  Libs: tmp/closer/psyq/LIBCD_*.LIB (3.3/3.5/3.6/4.0, from sozud/psy-q corpus).

## W1 — marionation_Exec (= CD_ready): branch-destination residual SOLVED

Baseline reproduction: P6 candidate = masked 2 @179/179 + 2 unmasked branch
diffs (beqz→0x23c vs target 0x28c) + sll/addu pair. All measured via
`sandbox --disable all` + tmp/closer/difffn.sh.

**CONFIRMED (the honest fix):** the SOTN v1.77 source shows each check arm is
self-contained — `Intr.c = 0; _memcpy(result, buf, 8); return c;` — with the
`result==0` guard INSIDE the inlined `_memcpy` (which starts `if (pDst == NULL)
return;`). There is NO cross-arm `goto done` in the original. Rewriting the
arms this way (vA_T1): **both beqz destinations flip to 0x28c = target**
(cross-jump forward-redirects arm1's guard into arm2's identical `j;move v0,a2`
tail). Score stays masked 2 @179/179; residual reduces to the single pair.
- vA_T1_notailwrap: the tail do-while(0) FAKE is REMOVABLE on this chassis
  (identical diff without it). **Banked: candidates/marionation_vAT1_notailwrap.c
  — the new best form (masked 2, 1-pair residual, 2 fewer FAKE devices).**
- KILLED: dropping the nested check1-clear wrap (13), dropping the check2-clear
  wrap (13), P5 full-volatile on T0/T1 tails (8/8 — the +6 is entirely the
  do_timeout window: sync chain reseats v1-for-a0, addu/sw/lw defer).

## W1 — the sll/addu pair: full mechanism named (and re-verified on this chassis)

- w1 (t0-shift late, i.e. arg5 chain before t0's shift+add): **schedule becomes
  100% target-correct**; residual = pure v1↔a0 exchange on {t0-chain, arg5-value}
  (masked 6). The twins' residuals are THE SAME knot.
- QTYDBG on w1: reg104 (t0-sll temp, b18 d24 r4, pri 1.33) vs reg97 (arg5val,
  b20 d26 r4, pri 1.33) — exact tie, broken by qty/birth order in t0's favor
  (local-alloc.c:1646). Target needs arg5 first (v1), t0-side second (a0).
- rank_for_schedule (sched.c:2399): priority → class-vs-last-scheduled → LUID.
  With arg5's chain earlier in RTL, slot-58's {sll, lw} tie breaks by CLASS
  (lw is data-dep on the just-scheduled addu at load cost 2 → class 1; sll
  independent → class 3): **the target schedule is self-correcting under the
  faithful RTL order; only the allocation tie needs the faithful weights.**
- KILLED on this chassis (sweeps in tmp/marion_win, tmp/marion_stage, tmp/marion_ip):
  w3 arg5-first (7), w4 t0<<=2-early (7), x1 v0-stage (10), x2 cnt-stage (9),
  x4 i-stage (24), x5 v0-mult-stage (10), y1/y2/y4/y5 in-place spellings (7),
  y3 (6), x3 status-stage (5 — arg5 seat FIXED but t0 chain goes global →
  a3/s0 via `own_full_prefs: 7`, an unfixable a3 preference from the
  `lw a3,0(t0)` MEM-address preference in global.c set_preference).
- f1/f2 direct-printf-arg spellings on the goto chassis: 14/12 — the goto
  chassis cannot absorb the faithful arg shape.

## THE DECISIVE FINDING — the faithful source reproduces the window

tmp/closer/cdready_v1.c (literal SOTN v1.77 shape: `static volatile CD_intr
Intr` member reads, non-volatile `(Alarm_t*)&Alarm` casts, `while(1)`,
static-inline set_alarm/get_alarm/callback/_memcpy), compiled STANDALONE with
exact project flags (tmp/closer/build_standalone.sh = cpp|cc1|prologue_fix|
maspsx|as):

**The entire contested window matches — `lbu a0/lbu v0/lui+lw a1/sll v0/
addu v0,v0,s5/sll a0,a0/lw v1,0(v0)/lbu com/addu a0,a0,s5/sll/sw v1` — seats
AND pair order, from plain faithful C with ZERO FAKE devices.** The while(1)
loop's loop_depth ref-weighting is what the 155 grind sessions' FAKE wraps were
approximating.

Sole defect: loop.c hoists the 1-use `&CD_comstr` movable into **s8** (2 extra
prologue insns + `addu v0,v0,s8` addressing vs target's $at macro).
Mechanism fully named:
- loop.c move_movables: moved iff `threshold*savings*lifetime >= insn_count`;
  threshold = (loop_has_call?1:2)*(1+n_non_fixed_regs); this movable:
  savings 1, life 2; outer loop = 117 real insns. Boundary is razor-thin
  (2*threshold vs 117).
- **cc1psx PROOF (tmp/closer/standalone/psx.s):** the original SN compiler
  (GNU C 2.7.2.SN.1) compiles the SAME .i with the SAME flags to the TARGET
  shape — `lw $6,CD_comstr($2)`, zero reg-30 usage. SN keeps $fp
  non-allocatable (and its n_non_fixed_regs is 1 smaller → threshold 1 smaller
  → this exact movable fails the move test). Our decompals fork frees $fp.
  This is a REAL fork divergence on this input class, per the
  difficult-is-not-impossible self-disproof protocol — first observed instance
  where cc1psx produces the target and our fork does not.
- C-side kill of the hoist WORKS: pre-scaled-index spellings
  (`*(char**)((CD_com<<2)+(char*)CD_comstr)`, comma-staged com4) eliminate the
  movable (base-set life 1 → "not desirable" → combine folds the macro-mem ✓)
  BUT reshuffle the window schedule (com chain hoists to the front, seats
  rotate) — the schedule battle moves, not disappears.

## In-context port (started, incomplete)

Port attempts of the faithful form into system.c (helpers + volatile struct
decl): masked 70-75 — the TU context diverges from the standalone (alarm
member addressing comes out base+4/+8 vs target's per-member la; pointer
hoisting decomposition differs: mine 3 la's, target 2 la's + addiu-derived
+1/+2). NOT a dead end — the standalone proves the destination exists; the
port needs the decl/spelling shapes tuned (per-member Alarm symbols like the
committed siblings use; possibly the g_cd_status_* volatile u8 decls already
in the TU). Artifacts: tmp/marion_faith2/ (helpers.c + v1_faithful_port.c),
tmp/closer/apply_faithful.py, tmp/closer/apply_structdecl.py.

## Frontier for the next Closer session (ranked)

1. **In-context faithful port** (the close): make marionation_Exec's TU form
   produce the standalone's stream. Key deltas to hunt: (a) Alarm accesses —
   try per-member spelling (`D_800F19B8 = ...; D_800F19BC = 0; D_800F19C0 =
   name;` exactly as the current candidates do, NOT struct members) since the
   target's stores are per-symbol la's; (b) Intr as the volatile struct extern
   (gives the addiu +1/+2 derivation seen in target); (c) while(1) loop for
   the loop_depth weights; (d) the comstr-hoist kill (com4/shift-first
   spelling) — measure which reshuffle survives in-context.
2. If the comstr movable keeps winning s8 in-context: the +2-insn insn_count
   lever (raise the loop's RTL insn count past 2*threshold with
   later-folded spellings), or surface the fp-allocatability divergence as a
   ruling question (Ruling 2 wants the impossibility side named: this is
   "our fork can't emit it from the faithful C; the ORIGINAL compiler can and
   did" — with psx.s as the exhibit).
3. csmd4 (W2, untouched this session): the same faithful route applies —
   CD_sync's window is the same shape; do the port ONCE on marionation, then
   mirror. The h5 fakematch replacement should come from the same chassis.

## Artifacts index

- tmp/closer/cdready_v1.c / cdready_s1.c / cdready_faithful.c — standalone sources
- tmp/closer/build_standalone.sh / dump_standalone.sh — pipeline + dump runners
- tmp/closer/standalone/ — mine.txt/tgt.txt/psx.s/dumps
- tmp/closer/marq.sh / maralloc.sh / findreg.sh — instrumented-cc1 runners
- tmp/closer/win.py / win2.py / lreg_win.py / movables.py / rtl_order2.py
- tmp/marion_variants/ tmp/marion_win/ tmp/marion_stage/ tmp/marion_ip/
  tmp/marion_faith/ tmp/marion_faith2/ — all measured variant banks
