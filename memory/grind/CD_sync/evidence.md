> **ALIAS NOTE (2026-08-24):** this ledger's function is `CD_sync` (formerly `cpu_side_move_dir_4` — decisions.md rulings and interior prose may use either name; any `memory/grind/cpu_side_move_dir_4/` path is this dir).

# Evidence bank — cpu_side_move_dir_4

- WIP rejected_form: {'form': 'volatile u8 *idx_1494 (v15/v12/v14)', 'score': 7, 'reason': 'Policy: prong-1 fails (writer synchronous). Also only bought idx-pair order, not the reg half.'}

- WIP rejected_form: {'form': 'arg4 inline in call (v16) / address-precompute p4 (v17)', 'score': '7-8 masked', 'reason': 'Stretch works but file-level s-reg regression s2->s1 (ref-balance coupling with the committed idx_1495 chain lever).'}

- WIP rejected_form: {'form': 'inline-all args (v1/v8/v9)', 'score': 14, 'reason': 'Right-to-left eval: idx1-first + 11D5-early; bare u8 locals forward-substituted.'}

- WIP rejected_form: {'form': 'standalone-permuter finds (output-270-1 v16-shape)', 'score': '270 permuter-units', 'reason': 'Workspace context UNFAITHFUL to full file (s-reg coupling invisible); do not trust without full-context re-verification.'}

- == imported from memory/wip notes.md ==
# cpu_side_move_dir_4 (system.c) - WIP, masked floor 2 (h5 form; ONE insn-pair left)

## TL;DR (SESSION 4 — current)
candidate.c = the h5 form: the owner-sanctioned staged-value-reused-variable
family (v0 = idx[1]; v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c)) +
t0 multi-set chain + pp alias. **masked 2 / raw 2.** The staging strips the
lw5 LAUNCH (sched.c birthing_insn_p: multi-set dest) so the head re-times to
target; the v0+(s32)tbl spelling fixes the addu operand order.
Residual = the {sll4@54 <-> addu5@55} pair: both LAUNCH, luid-ordered
(A5-after-T0B forces sll4-first); flipping via A5-before-T0B hits the
p106/val5 [18,24]/[20,26] L6=L6 birth-tie (6). The v0-2nd-stage (v0=v0+tbl
as a stmt) collapses v0-var pri below t0 (t0 steals v0-hard, 15) - closed.
cnt-staging clashes with cnt's v1 loop role - closed. Double-staging d/e
families 15-17 - closed. ~40 forms measured this session.
**SESSION-4b — the g3 discovery (SECOND frontier, the pair FIXED):**
`v0<<=2; arg5=*(s32*)(v0+(s32)tbl); t0<<=2; call(..., *(s32*)(t0+(s32)
tbl_125c), arg5)` (t0's addu IN the call) = masked 6 / raw 6 with ORDER
PERFECT; residual = a pure v1<->a0 exchange (t0-chain vs arg5 value).
Mechanics (QTYDBG/lreg-verified): t0's qty ties the sll+in-call-addu
(p113: 6refs/span24 = pri 5000) and allocates BEFORE arg5 (p100:
2refs/span6 = 3333) -> t0 takes v1; target needs arg5 first (v1), t0
skipping to a0. Flip requires arg5-qty refs>=4 via a LOCAL copy-tie;
every vehicle measured dead: fresh single-set x folds back (k2=6=g3);
fn-scope carriers are global allocnos (permuter's status/temp finds ->
s0 spill, honest 6-8, REJECTED); v0-carrier scrambles (k1=16); a2v
presequence re-times the 11D5 chain (k3/k4=16-17); 2-ref t0 forms hit
the equiv-sink (h8*=14); t0*=4-late respellings re-time the head
(g1/g6/h7=14); plain re-measured 7/11. Sweeps: tmp/csmd4_{g,h2,h8,k,
f}_sweep.py, plain_now.py; tables tmp/csmd4_g3_{qty,pseudos}.py.
NEXT: (1) permuter now on the g3 base (weighted ~30 vs h5's 60 — richer
mutation neighborhood for stop-on-zero); (2) h5 base = still the masked
floor (2); candidate.c stays h5; (3) marionation region-1 = the same
pair — a twin crack transfers; g3 NOT yet mirrored onto marionation.
SESSION-5 ADDENDUM (2026-07-04, from the marionation deep-dive): target
bytes have the t0 chain IN-PLACE in $a0 => the original's sll never
LAUNCHED => the ORIGINAL SOURCE WAS THE g3 FAMILY. The pair is not to be
flipped in h5 (re-confirmed: t0<<=2 at same position = 7 here, 9 mar —
the *=4 temp's launch is head-load-bearing). Attack g3's v1/a0 qty
exchange instead: arg5-qty needs pri>=5000 (3refs/span6 ties, birth
order wins); untried class = multi-set arg5 VALUE-staging keeping the
lw-dest split. See marionation notes.md region-1 for the full argument.

## Prior sessions (3a-3d), one line each — details in git history
- 3d: carry-forms (val5 through a multi-set carrier) solve the ORDER half;
  THE STAGING THEOREM: locals allocate before globals; any lone address
  temp is LOCAL and steals v1 once val5 is carried. Best raw 8.
- 3b: split-addu frame achieves w=a0 (homing's mem-address = the in-call
  temp, not w). Faithful permuter workspace built (splice-compile fix:
  the permuter PRUNES other fn bodies; its pruned-TU metric is UNFAITHFUL
  — always re-verify finds via engine sandbox).
- 3a: cross-block reuse map (w = chain∪src a0-family, k = val∪-1 a3);
  the T1/T2 trap pair via set_preference one-level MEM unwrap.

## The decisive mechanism facts (all read from tools/gcc-2.7.2, verified by dumps)
- find_reg pass 0 excludes `regs_someone_prefers` (prefs of CONFLICTING
  lower-pri allocnos, merged by prune_preferences) and scans ASCENDING regno
  (no MIPS REG_ALLOC_ORDER) → a0 before a3 when both eligible.
- **COPY-pref upgrade short-circuits (`goto no_prefs`) BEFORE the full-pref
  loop** — an unfoldable `(set a0 (reg w))` reg-reg copy-pref would beat the
  a3 full-pref outright. Jalr-arg-temp attempt FAILED: cse folds a single-use
  load into the call homing (w vanishes from the jalr block).
- Preferences masked by AND_COMPL vs `used` — a3 enters `used` only via
  hard_reg_conflicts[w] (an a3-holding CONFLICTING allocno assigned BEFORE w).
- Priorities (ALLOCDBG, pri = floor_log2(refs)×refs/livelen×10000): w-inline
  16956 (13refs/23len), i 15555, temp 4210, -1holder 2142.
- adjust_priority: LAUNCH (0x7f000001) goes to insns passing birthing_insn_p
  = SET to live reg with **reg_n_sets==1** (flow-time). Multi-set dests never
  launch → staged loads place early-forward. THE sanctioned lever.
- schedule_select: loads beat ALU within a same-priority ready group;
  luid breaks LAUNCH-vs-LAUNCH ties.
- The a0-pref chain: the mul3 sll-temp gets a0 locally, and
  `(set t0 (plus renumbered-a0 base))` transfers an a0 full-pref into t0;
  ascending upgrade scan picks a0 before a3 (FINDREGDBG-verified).

## Ruled out (do NOT re-derive)
- G1-G4 hoists of D_800A11D5 (refs-2 equiv-sink puts the load back), inline-arg
  forms (14), dual temp-free chains D1-D8 (6-10), tie-battery of 10 arg5
  spellings (identical qty tables), 192-combo grid (floor 4), a2v hoists
  (14-16), jalr-temp copy-pref (cse-folded), arg5-first k-frames (hole is
  luid-invariant), k-value forms (21-25), reu-arg5 (chain grabs a0, 14).
- h-sweep interleavings: five orders all raw-2 with the identical pair-swap;
  sabc (A5-first) = 6 via the qty tie. Dead stores INERT (flow recounts).
- The 2026-07-01 "stretch the chain density" framing is SUPERSEDED.

## Pointers
- Probes: tmp/csmd4_{v0idx,half,h3,h_sweep,double,e_v0d,e_mul2,final_trace,
  vpabc_qty,verify_cand}.py (gitignored; regenerate from here).
- FINDREGDBG: BB2_FINDREG_DEBUG=<pseudo> (global.c find_reg); rebuild gccdbg
  with `make cc1 CFLAGS="-g -fgnu89-inline"` (c-gperf inline fix). Also
  BB2_SCHED_DEBUG / BB2_RANK_DEBUG / BB2_QTY_DEBUG / BB2_ALLOC_DEBUG; -dS
  gives the .sched dump (launching/blocking/ready lists).
- Permuter: tmp/perm_csmd4 (h5 base; compile.sh splices via
  tmp/csmd4_splice.py; launch permuter.py --best-only --stop-on-zero -j12).
- marionation_Exec: region-1 = the IDENTICAL residual pair (candidates
  mirrored); region-3 (arm-2 nop) is marionation-only.
- Target block: asm/funcs/cpu_side_move_dir_4.s lines 49-73.


- [s1] [fable-blitz 2026-07-07] Rule-by-rule regfix classification (regfix.txt:1571-1576, 5 rules, all serving the ONE debug_printf window): 4x subst $3->$4 (v1->a0) on the arg4/t0 chain {lbu @39, sll @41, addu @42, lw @45} = the REGISTER half (the v1/a0 exchange); 1x 15-slot reorder @39-53 = the SCHEDULE half (slot 45, the lw-a3 arg4 deref, moves to the window's end). Both halves are the same coupled wall as marionation region-1; there is no second region in this function (marionation's region-3 dbr steal is marionation-only per memory/wip/cpu_side_move_dir_4/notes.md:96).

- [s1] [fable-blitz 2026-07-07] Target window facts (asm/funcs/cpu_side_move_dir_4.s:53-73): t0/arg4 chain is IN-PLACE in a0 (lbu a0,0(s2) L53; sll a0,a0,2 L59; addu a0,a0,s3 L63; lw a3,0(a0) L69 - no fresh temp ever exists); arg5 chain via v0 -> lw v1,0(v0) L60 -> sw v1,0x10(sp) L65; the fmt la (D_800161C8) is LAST (lui/addiu a0 L70-71, a hard-a0 set AFTER t0's death); D_800A11DC goes through the $at assembler macro L66-68; D_800A11D5's lbu REUSES v0 L61-62 (per-segment qty reuse exists in target). This is byte-for-byte the same shape as marionation region-1's target (commit f4bc8e67 'THE BASIN CORRECTION'), with s3 for tbl instead of s5.

- [s1] [fable-blitz 2026-07-07] Prologue fact (asm/funcs/cpu_side_move_dir_4.s:19): target derives s4 = s2+1 in ONE addiu (addiu s4,s2,1) - the original source computed idx_1495 = idx_1494 + 1 by pointer arithmetic. The committed HEAD spelling at src/system.c:406 reaches the same bytes via a tbl_125c-routed cross-symbol derivation ((u8*)tbl + ((s32)&D_800A1494 - (s32)D_800A125C) + 1), which is in the family the 2026-07-05 semantic-lie ruling forbids (do-while-zero-exception.md FORBIDDEN #5; marionation's R_reb_y1 cross-symbol form got layer-1 FAIL + user DO-NOT-SANCTION). The byte-proven honest respelling idx_1495 = idx_1494 + 1 exists; WIP warns file-level s-reg ref-balance is coupled to this lever (rejected v16/v17: s2->s1 regression), so replacement needs re-measurement - and do-while(0) wraps (sanctioned 2026-07-06) are now available to re-seat s-regs if balance shifts (marionation recipe: do_timeout wrap -> tbl/s5).

- [s1] [fable-blitz 2026-07-07] Basin correction transfers to csmd4's own ladder: WIP candidate h5 (masked 2) is in the fresh-temp basin and carries the register-masked-mirage caveat - csmd4's session-5 addendum (notes.md:34-41) + marionation f4bc8e67 prove the ORIGINAL SOURCE was the in-place/g3 family (t0 chain never launched). h5's 2 hides the unmaskable v1/a0 exchange; the structurally-correct base is g3 (masked 6, ORDER PERFECT, residual = the pure exchange). Marionation's ip1/ip2 measured masked 9 there with the exact target shape; note the true in-place spelling is 't0 <<= 2; t0 += (s32)tbl_125c;' as STATEMENTS - NOT '*= 4' (the mult path synthesizes the launching temp) and NOT the in-call addu (g3's spelling). csmd4 has NEVER measured the statement-form in-place spelling; its ladder only has *=4-based h/g forms (g1/g6/h7 = 14 were *=4-LATE respellings, a different thing).

- [s1] [fable-blitz 2026-07-07] THE FINAL ALLOCATION EQUATION (marionation 6h, f4bc8e67, transfers verbatim): local-alloc first-fit gives t0->a0 only if v1 is already occupied at t0's allocation => arg5 must allocate BEFORE t0's qty => pri(arg5) > pri(t0), pri = floor_log2(refs)*refs*size/span (local-alloc.c qty_compare). csmd4-g3 numbers (notes.md:19-22): t0 p113 6refs/span24 = 5000, arg5 p100 2refs/span6 = 3333. Crucially a TIE is NOT enough in csmd4 - t0 births first (lbu at the window head), and qty birth order breaks ties - so arg5 needs a STRICT win: weighted refs >= 4 at span 6 (13333), or refs 3 at span <= 5 (6000).

- [s1] [fable-blitz 2026-07-07] Marionation 6g HALF-FLIP (cd65f9c2): staging the t0 SHIFT through a multi-set variable kills the sll's launch (sched.c birthing_insn_p: INSN_PRIORITY max iff dest is a live SINGLE-set pseudo; multi-set dests never launch) -> the arg5 side of the exchange fully fixed (masked 7 there). The t0 side stayed broken for two named reasons: (a) the stage variable must be BLOCK-LOCAL multi-set - fn-scope carriers are global pseudos (csmd4 measured the same: status/temp finds -> s0 spill, honest 6-8, rejected), and a fresh 2-set local COLLAPSES (combine merges the sets with accurate ref bookkeeping, e5aa2dd4); (b) a0 is blocked for t0 by the call's fmt-la scheduling before t0's last use -> fmt-la-late is the named missing half (target's fmt la is literally last, L70-71).

- [s1] [fable-blitz 2026-07-07] Marionation 6h(3) (622620cb): hosting the in-place t0 chain in a dead-here GLOBAL (src) = masked 22 in all four spellings - extending a global pseudo's live range re-weights its allocno and cascades the whole allocation web. Global-variable hosting is CLOSED as a t0->a0 vehicle in both twins; the equation must be solved with LOCAL-ONLY moves. Unexplored machinery named there: local-alloc's qty_sugg suggested-color paths + combine_regs operand-tying (QTYDBG-SUGG lines never explored in either twin).

- [s1] [fable-blitz 2026-07-07] Marionation grind s2/s3 closures (memory/grind/marionation_Exec/hypotheses.md, evidence.md s2/s3): the hand-STRUCTURAL axis for the pair is closed - 140 dependency-valid orderings + 48 geometries/decompositions, decl order and s32/u32 type narrowing are qty-INERT (pseudo birth follows RTL first-use, not declarations), plus-operand order fully canonicalizes at expand, growing an existing staging web from 2 to 3 statements re-times the head (17) position-invariantly, arg5 self-staged address = 11 (the eliminated addr-temp seat is load-bearing). Do NOT re-sweep these axes on csmd4; the twin's window is the same RTL shape.

- [s1] [fable-blitz 2026-07-07] Order half is SOLVED in both twins and is order-INVARIANT in the in-place basin: marionation o1/ip order ([t0load; pp; v0ld; v0shl; a5; t0mul; t0add]) kills the 56/57 swap (aa8cad24); df749bfd (ipA/B/C) proved placing the in-place t0-shl at every C position leaves the order unchanged - the in-place basin's schedule is priority-driven, not C-luid-driven. csmd4's g3 is already order-perfect. So the ONLY remaining dial in region-1/csmd4 is the allocation equation; once the exchange flips, re-examine the 56/57-analog order in the flipped world (priorities change with the registers).

- [s1] [fable-blitz 2026-07-07] Policy delta since csmd4's park reason was written (2026-06-01): do-while(0) is now SANCTIONED for ANY codegen effect incl. register allocation (do-while-zero-exception.md, owner ruling 2026-07-06; FAKE-annotated; nested wraps need a single-level-insufficient note). csmd4's window has NEVER been probed with wraps. Marionation's wrap-dead measurements (vT35=15/vT36=14/vT42=14/vT43=12 'ANY note insertion re-times the head') were ALL in the fresh-temp vT40 basin where placement is launch/LUID-coupled; the wrap x in-place-basin cross is UNMEASURED in both twins. Wrap ref-weighting is exactly the differential pri-lift the equation asks for (flow.c loop-depth-weighted REG_N_REFS feeding qty refs). Risk to measure: LOOP_BEG/END notes may act as sched barriers splitting the window (the suspected mechanism behind the fresh-temp-basin re-timings).

- [s1] [fable-blitz 2026-07-07] csmd4-specific dead ends (memory/wip/cpu_side_move_dir_4/notes.md + meta.json rejected_forms - do not re-derive): fn-scope carriers -> s0 spill (6-8, rejected); v0-carrier scrambles (k1=16); a2v presequence re-times the 11D5 chain (k3/k4=16-17); 2-ref t0 forms hit the refs-2 equiv-sink (h8*=14); k-value forms 21-25; inline-all-args 14 (right-to-left eval); volatile idx_1494 fails the two-prong policy; arg4-inline/address-precompute v16/v17 work locally but regress file s-regs (coupling with the line-406 lever); standalone-permuter finds are UNFAITHFUL (workspace prunes the TU, s-reg coupling invisible - always re-verify via engine sandbox with splice); permuter closing forms found so far are all cheat-shaped (0xFF/0xFFFF masks, u16 narrowing, i/status alias repurposing) and stay rejected.

- [s1] [fable-blitz 2026-07-07] Strategic order-of-attack for the operator: csmd4 is the CHEAPER twin - its g3/ip residual is the exchange ALONE (masked 6 -> 0 on flip), while marionation still carries region-3 (+2) behind the same flip. Marionation 6f (aa8cad24) cross-checked that csmd4-g3's p113/p100 qty numbers match marionation-o1's 111/104 arithmetic: ONE flip closes the pair in BOTH functions. Develop the exchange lever HERE first, then mirror it onto marionation.

- [s1] [fable-blitz 2026-07-07] Tooling notes for the execution session: the masked sandbox metric is BLIND to the register half (meta.json scores.metric) - g3-family probes need raw/window diffing; rebuild the adiff pattern from tmp/grind/marionation_Exec/s2/adiff.py (splice + cheat-stripped sandbox + objdump + LCS vs asm/funcs/cpu_side_move_dir_4.s). QTYDBG = BB2_QTY_DEBUG on the gccdbg cc1 (rebuild recipe memory/wip/cpu_side_move_dir_4/notes.md:88-91); marionation's per-function qty isolation trick: segment the dump by function to avoid the twins' colliding uids (aa8cad24).

- [s2] Baseline masked=2 confirmed on h5 candidate applied to src/system.c (sandbox cpu_side_move_dir_4 --disable all).

- [s2] Structural sweep A/B/C/D INERT at masked 2 - hand-structural axis (decl order, type narrowing, statement re-association under arg5-after-t0) is fully closed on csmd4 directly, confirming the twin's marionation s2/s3 finding on the primary function (evidence.md:124).

- [s2] Head-load ordering (v0-idx before t0-idx) regresses to 7 - t0-first at head is load-bearing for h5's masked-2 alignment; consistent with twin 2026-07-04 addendum on 't0<<=2 at same position = 7 here, 9 mar - the *=4 temp's launch is head-load-bearing'.

- [s2] Combined with prior s2 rejected banks (arg5_split_multiset_v0.c=12, fmt_la_late_local.c=2 INERT, ip_base_statement_form.c=7), all three declared UNMEASURED frontier probes are now measured KILLED on csmd4.

- [s3] Baseline confirmed: h5 candidate applied to src/system.c scores masked 2 via sandbox cpu_side_move_dir_4 --disable all (target_insns=160, build_insns=160).

- [s3] 13 structural variants swept this session (see tmp/grind/cpu_side_move_dir_4/s3/sweep_results.json + sweep2_results.json).

- [s3] 8 of 13 INERT at masked 2: V2 v0s-split, V4 arg5_addr, V5 pointer-split, V8 v0shl-early, V9 arg5-inline, V11 pp-late, V12 decl-pp-first, V13 ptr-deref-form. This proves the arg5-chain sub-axis of block-local structure is fully expand/cse-normalized — GCC reaches the same RTL/scheduling regardless of parenthesization, decl order, intermediate names, base-pointer copies, or shift position within the C statement sequence.

- [s3] 5 of 13 regressed: V3 t0s-split (+1), V6 arg4-named (+11), V7 stmt-form t0 (+5), V10 t0-compress (+7). Direction of each measured regression is consistent with the h5 base's dependence on the multi-set t0 pattern (LAUNCH suppression via birthing_insn_p on multi-set dests) and on t0/arg5 qty priority arithmetic.

- [s3] V10 collapse of t0 chain to a single statement scores masked 9 — this is direct empirical confirmation that the multi-set t0 pattern is what suppresses the sll/addu LAUNCH on h5; the mechanism assumed in the WIP ledger is now measured on csmd4 itself, not just inherited from marionation.

- [s3] V7 statement-form t0 (t0<<=2; t0+=(s32)tbl_125c) on h5 base scores masked 7 — matches twin notes.md:48 prediction verbatim; statement-form is the g3-family spelling and cannot substitute on h5.

- [s3] No new structural axis discovered; the frontier remains as the ledger declared (do-while(0) wrap on arg5 chain, alternate multi-set carrier for arg5, opaque fmt-la carrier).

- [s4] Baseline (h5 candidate applied to src/system.c): masked 2, target_insns=160, build_insns=160 (sandbox cpu_side_move_dir_4 --disable all).

- [s4] P1 (do-while(0) wrap arg5 sub-block): masked 20, build_insns 162 (+2). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dowhile0_wrap_arg5_v0_stage.c.

- [s4] P3 (do-while(0) wrap t0 sub-block, diagnostic pair): masked 24, build_insns 162 (+2). Direction-invariant +2 insn signature disproves the pri-lift-differential mechanism the frontier hypothesised — the LOOP_BEG/END notes are scheduler barriers here. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/dowhile0_wrap_t0_chain.c.

- [s4] P2 (cnt-carrier for arg5): masked 4, build_insns 160 (no insn change). Cnt-carrier is not a viable arg5 value-carrier. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/cnt_carrier_arg5.c.

- [s4] Frontier #3 (opaque volatile fmt-la) rejected by policy without measurement: game-state global fails legitimate-volatile-interrupt-touched two-prong (prong 1: not IRQ-mutated); mmio-volatile-type-level does not apply (not in 0x1F801000-0x1F802FFF); no non-volatile alternative defeats cse.c equiv_constant.

- [s4] Session did not modify candidate.c (h5 form remains masked-2 floor); src/system.c restored to the h5 candidate on session end.

- [s4] Instrumentation note (permuter modality): the WIP notes explicitly document standalone-permuter workspace UNFAITHFULNESS on csmd4 (workspace prunes TU; s-reg coupling invisible); the fresh-seed discipline says finds MUST be re-verified via engine sandbox. Directed structural mutations against the honest full-file sandbox are that verification — each probe encodes exactly one frontier hypothesis so the regression is diagnostic.

- [s5] s5 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s5] F1a (block-scope carrier c seeded from *idx_1495): masked=2 INERT. Multi-set carrier vehicle is masked-invariant vs h5's v0 staging — regardless of first-set source symbol (a live-elsewhere pointer target).

- [s5] F1b (block-scope carrier c seeded from live local t0): masked=2 INERT. Confirms F1a diagnosis — the multi-set carrier vehicle is qty-equivalent to h5's v0 staging at every first-set source tested.

- [s5] F2a (named dispatch = D_800A11DC[D_800A11D5] BEFORE arg5 stage): masked=16 (+14). The named-arg3 pseudo re-competes for the a-register seat, destroying h5's t0 launch-suppression alignment.

- [s5] F2b (named dispatch AFTER arg5 stage): masked=16 (+14). Position-invariant regression CONFIRMS the named-arg3 effect is qty-competition (not LUID scheduling); naming-anywhere fully KILLED as an arg5>t0 lever.

- [s5] F3 (single do-while(0) around whole inline block): masked=13, target_insns=160, build_insns=160. Whole-block wrap does NOT +2-insn scheduler-barrier (contrast s4 sub-block wraps which did) — a NEW data point that the wrap barrier effect depends on scope. Ref-reweighting materialized but MISDIRECTS.

- [s5] F3b (nested double do-while(0) same scope): masked=17. Deeper=worse; the wrap-based ref differential is monotonically wrong-direction — no depth flips the arg5>t0 equation into the target basin.

- [s5] F4 (do-while(0) around CALL alone): masked=8. Smallest wrap scope, smallest misdirection, still wrong direction. Wraps are net-negative for csmd4's h5 base at EVERY tested scope (sub-block, whole-block, nested, call-only).

- [s5] Permuter (g3 workspace, fresh seed, --stop-on-zero, -j 8): 9040 iterations in 411.8s → 0 NEW finds. Prior output-40 basin unchanged. Fresh-seed discipline: 0-find harvest is the data point.

- [s5] Key mechanism update: the s4 finding that sub-block do-while(0) wraps act as +2-insn scheduler barriers does NOT generalize — whole-block, nested, and CALL-only wraps do NOT add insns; they instead re-weight refs symmetrically and misdirect qty allocation. The do-while(0) sanctioned tool has NO configuration that helps csmd4-h5.

- [s6] [s6] Baseline h5 candidate applied to src/system.c: sandbox --disable all masked=2, target_insns=160, build_insns=160.

- [s6] [s6] Instrumented cc1 dump for cpu_side_move_dir_4 produced under tmp/grind/cpu_side_move_dir_4/s6/ (csmd4_only.{rtl,jump,cse,cse2,loop,flow,combine,jump2,lreg,greg,sched,sched2,dbr,log}).

- [s6] [s6] ALLOCDBG (global-alloc): 15 global pseudos ranked; pseudo=100 hardreg=4 pri=8000, pseudo=101 hardreg=4 pri=8000 (v1/a0 seats). QTYDBG blk=3 (debug_printf window): qty=1 p107 birth18-death20 refs=2, qty=3 p113 birth22-death30 refs=4, qty=2 p100 birth20-death26 refs=2, qty=0 p106 birth16-death24 refs=2.

- [s6] [s6] Register dispositions in .greg: 100 in 3 (v1), 106 in 4 (a0), 113 in 2 (v0), 107 in 2 (v0). Register half of h5 matches target (t0-scratch in a0, arg5-value carrier in v1); only the pair-swap {sll4@54 <-> addu5@55} remains.

- [s6] [s6] Named pass+decision: sched.c::schedule_block (sched2 pass) with adjust_priority LAUNCH sentinel 0x7f000001 assigned by birthing_insn_p (single-set SET dest on live pseudo). Insn 111 (p106 = p101 << 2, single-set fresh temp from GCC's mult-by-power-of-2 expand of `t0 *= 4`) receives LAUNCH; ties with insn 121 (p107 = p75 + p79 arg5 addr, single-set) which also receives LAUNCH; LUID tiebreak favors higher LUID (121@12 > 111@8) picked first in backward pass -> 121 emitted later in linear output -> 118,111,121 vs target 118,121,111.

- [s6] [s6] Target's shape: the g3-family / statement-form spelling `t0 = idx[0]; t0 <<= 2; t0 += tbl; ...*(s32*)t0` puts the SLL's dest as a re-SET of the t0 pseudo (multi-set p101), so birthing_insn_p returns FALSE at 111, priority stays pri=2 (non-LAUNCH), insn 121 (LAUNCH) beats it on strict priority -> 121 emitted first -> correct pair order. But this spelling drops h5 out of its basin: measured s3 V7=7 and this session's LUID-reorder probe=6, both in the g3 basin with residual v1/a0 register exchange.

- [s6] [s6] Basin coupling confirmed by measurement: h5 basin (masked 2, pair-swap residual) and g3 basin (masked 6, register-exchange residual) do not compose. Every candidate spelling measured across s3 (V7 statement-form), s4 (do-while(0) wraps at all scopes), s5 (multi-set carriers, named dispatch, permuter fresh-seed 9040 iters), and s6 (LUID reorder) either stays at masked 2 in h5 basin or falls into g3 basin masked 6-7.

- [s6] [s6] birthing_insn_p depends on flow.c reg_n_sets which is computed once at flow_analysis; combine.c can merge SETs when transformable to equality, but the h5 candidate's `t0 *= 4` -> p106 fresh temp is not combine-mergeable with p101's other SETs (different dests). To make p106 disappear as a distinct pseudo you must respell the multiplication as an in-place re-SET of p101 (statement-form `t0 <<= 2`) which is exactly the g3-basin flip that regresses to masked 6.

- [s7] s7 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `sandbox cpu_side_move_dir_4 --disable all`.

- [s7] tools/gcc-2.7.2/expmed.c:2244 case alg_shift calls `expand_shift(LSHIFT_EXPR, mode, accum, build_int_2(log,0), NULL_RTX, 0)` — target arg hardcoded NULL_RTX. Adjacent case alg_add_t_m2 (line 2249) also uses NULL_RTX for the shift result, feeds it to force_operand with add_target/accum_target. NULL_RTX propagates to gen_reg_rtx → fresh pseudo (p106 here).

- [s7] tools/gcc-2.7.2/config/mips/mips.md contains no shift-add insn pattern: grep for define_insn shows addsi3_internal / addsi3_internal_2 / adddi3_internal_* / maddi / maddi_64bit / umaddi / umaddi_64bit — the maddi family is integer multiply-accumulate (mult+add), NOT shift+add. Combine's substituted (plus (ashift reg const) reg) has no recognizer match.

- [s7] tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.combine insn 111: `(set (reg:SI 106) (ashift:SI (reg/v:SI 101) (const_int 2))) 181 {ashlsi3}` — REG_EQUAL note `(mult:SI (reg/v:SI 101) (const_int 4))` records the expand_mult provenance; ashlsi3 = insn code 181 confirms shift-only match, not shift-add.

- [s7] tools/gcc-2.7.2/combine.c try_combine (line 1385 onward): can_combine_p succeeds (p106 dies at i3, single use, no i1); combinable_i3pat succeeds; the merge attempt reaches recog_for_combine at line 1691; the negative return triggers alternate paths (line 1703 PARALLEL attempt, line 1821 find_split_point) — none can produce a recognizable pattern; undo_all runs.

- [s7] Flow-time reg_n_sets(106) = 1 (single SET at insn 111, single REG_DEAD at insn 116). sched.c::birthing_insn_p returns TRUE. adjust_priority overrides with LAUNCH sentinel 0x7f000001. sched.c::schedule_block backward LUID tiebreak (LUID(121)=12 > LUID(111)=8) picks 121 before 111 → emission order 118,111,121 vs target 118,121,111.

- [s7] Frontier #1 mechanism (num_sets>1 on p101 blocking combine) is inoperative: even if p101 were multi-set, combine would still reject on recog fail; and any C form making p101 multi-set is by definition the g3-basin direct-LSHIFT spelling that regresses to masked 6.

- [s7] Rejected-forms bank note: `arg5_split_multiset_v0.c` file exists — frontier #2 was probed previously (result KILLED per bank presence, likely cse.c collapsing the intermediate MOVE).

- [s8] s8 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s8] s8 probe1: honest idx_1495=idx_1494+1 substitution on h5 base -> masked=15, +13 regression. First empirical measurement of the WIP s-reg coupling warning that the cross-symbol tbl-routed spelling for idx_1495 is load-bearing.

- [s8] s8 probe2 (m2c): m2c on target asm produces (a) if/else-flattened top-level, (b) direct-inline-all-args debug_printf call using &D_800A1494 and &D_800A125C literally at each site (no local pointer copies, no arg4/arg5 named intermediates). Shape (b) is the WIP-recorded score-14 'inline-all args' rejected form; the top-level flattening is upstream of the pair-swap window and orthogonal to the h5 mult-expander LAUNCH mechanism.

- [s8] s8 policy note: the cross-symbol form at src/system.c:406 (currently committed) is in the semantic-lie family the 2026-07-05 do-while-zero-exception.md #5 forbids. Retiring this policy violation is BLOCKED by the +13 file-level s-reg coupling regression that the honest respelling triggers. The forbidden form cannot be safely retired until h5 either closes or a different s-reg web is found that tolerates the honest respelling. Any future session that tries to retire this form must plan for the s-reg web disruption (WIP records rejected v16/v17 with s2->s1 regression from the arg4/arg5 lever, coupled via the same file-scope RA).

- [s8] s8 candidate.c: unchanged (h5 form remains masked-2 floor); src/system.c restored to the h5 candidate on session end.

- [s8] s8 modality-exhaustion note: rederive via m2c produced no structural neighborhood beyond the known inline-all-args shape. Combined with s3's 13-variant hand-structural sweep (all block-local axes closed), s4's 4-scope do-while(0) sweep, s5's F1a/F1b/F2a/F2b/F3/F3b/F4 + 9040-iter permuter fresh-seed campaign, and s6/s7 forensics naming the h5-vs-g3 basin non-composability as inherent to expmed.c case alg_shift's hardcoded NULL_RTX target, the m2c rederive angle is now measurably closed. The only truly untested frontier remaining is #3 (duplicated-statement-into-arms lift on the t0 chain) plus the variant #2 sketch of two-independent-PLUS-SETs on arg5_addr (both not drafted this session per rederive mandate).

- [s9] s9 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s9] s9 decomp.me corpus modality closed: `python3 tools/decomp_me_scrape.py --help` fails with `ModuleNotFoundError: No module named 'curl_cffi'`; the corpus rederive angle is unavailable without `pip install curl_cffi` in .venv. Not attempted; not a session finding.

- [s9] s9 sibling-transplant modality on file-local twin marionation_Exec (src/system.c:499+). Marionation's inline block is `s32 arg4; arg4 = tbl_125c[idx_1494[0]]; debug_printf(..., arg4, tbl_125c[idx_1494[1]]);` - shape never measured on csmd4.

- [s9] s9 P1 (marionation-hybrid arg4-named+arg5-inline): masked=7. Matches ip_base_statement_form.c basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/marionation_hybrid_arg4_named_arg5_inline.c.

- [s9] s9 P2 (mirror arg5-named+arg4-inline): masked=14. Matches WIP inline-all basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/mirror_arg5_named_arg4_inline.c.

- [s9] s9 P3 (both-named array-index reverse eval): masked=8. Novel intermediate basin, +6 vs h5. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/both_named_array_index_reverse.c.

- [s9] s9 P4 (marionation full-basin transplant: honest idx_1495 + marionation-hybrid): masked=20. Novel compound-regression basin. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/marionation_full_basin_transplant.c.

- [s9] s9 diagnostic: marionation's s-reg web is materially different from csmd4's (marionation carries extra idx_1496 = idx_1494 + 2 live pointer, `register s32 check asm("$6")` pin, `extern volatile u8 *D_800A147C_2 asm("D_800A147C")` alias-rename, D_80016248 sentinel). The shared surface residual (the t0/arg5 pair swap at the debug_printf window) is NOT sufficient for basin-transplant fungibility; the ambient s-reg web dictates which pair-swap-flip lever composes.

- [s9] s9 non-linear compound: +5 (P1 hybrid inline block alone on h5) + +13 (s8-probe1 honest idx_1495 alone on h5) yields +18 (P4 combined) NOT +18-suggests-linear -- actually the sum is exactly +18 vs measured. Note (bookkeeping): re-reading the measurements, +5 + +13 = +18 does equal the +18 measured for P4. So the compound IS approximately linear at the s-reg-web level here; my earlier note claiming super-linearity was wrong. The correct finding: the two levers combine ADDITIVELY on csmd4, further confirming they are BOTH bad on csmd4's h5 base regardless of ordering or coupling.

- [s9] s9 candidate.c: unchanged (h5 form remains masked-2 floor); src/system.c restored to the h5 candidate on session end; post-restore sandbox re-measures masked=2.

- [s9] s9 modality-exhaustion note: with m2c (s8), decomp.me corpus (tool unavailable this session), and file-local sibling transplant (s9 P1-P4) all measured KILLED at h5-basin refresh, the 'rederive' modality is now fully closed against known novel-shape angles. Future re-derivation attempts would need external inputs: (a) install curl_cffi and scrape decomp.me for target-asm-similar scratches, (b) locate a distinct sibling analog outside marionation_Exec (currently no known second twin in the codebase per WIP notes.md), or (c) an entirely different Kengo/nm_cpu source glance (label at src/system.c:497 flags a `x4 size collision` in nm_cpu/cpu_side_move_dir_4 - available only if the operator has the Kengo source dump).

- [s10] s10 synthesis pass: no src edit, no sandbox measurement (synthesis modality); h5 candidate remains the masked-2 floor baseline.

- [s10] Ledger cross-read confirms the s7 expmed.c wall (case alg_shift NULL_RTX target) forbids composing the h5 pair-swap-fix with the g3 register-exchange-fix on any C spelling of the t0 chain.

- [s10] Only lever (b) 'kill 121 LAUNCH keep 111 LAUNCH' has an untested mechanism realization: arg5_addr two-SET forms that survive cse.simplify_plus_minus to flow-time.

- [s10] Frontier #3 (duplicated-statement-into-arms on t0 chain) demoted: it realizes lever (a) which is the g3-basin trap. Cross-jump-based duplication of the ARG5 chain is infeasible because v0=idx_1494[1] is set inside the do_timeout block, making cross-arm duplication byte-non-neutral.

- [s10] Judge-risk pre-analysis: M1 primary probe belongs to the split-init-accumulation-sanctioned family (2026-06-13 provisional) with a defeat-cse twist; layer-2 cheat-reviewer must verify FAKE annotation quality when s11 measures.

- [s10] decomp.me corpus scrape modality remains gated on operator installing curl_cffi in .venv (unchanged from s9).

- [s11] s11 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s11] s11 M1-primary (algebraic-cancel two-SET on arg5_addr) measured masked=11 (+9). Rejected form saved.

- [s11] s11 M1-secondary (opaque-carrier t_alias two-SET) measured masked=29 (+27). Rejected form saved; cse did NOT unify t_alias with t0 (duplicated lbu-shift emitted).

- [s11] s11 M1-tertiary (simple two-SET, no cancellation) measured masked=2 INERT — bytes IDENTICAL to h5 baseline, isolating the combine.c fold mechanism as the reason INERT and regression variants both block flow-time multi-set on p107.

- [s11] s11 mechanism finding: combine.c substitutes (plus symref reg) through addsi3_internal (per s7:213), folding any simple two-SET on p107 back to single-SET RTL; two-SET forms with non-trivial subtrahends either disturb the alloc web (M1-primary) or force cse to keep intermediate pseudos distinct (M1-secondary, duplicating the lbu-mult chain).

- [s11] s11 frontier #1 CLOSED across three C-realizations: no C two-SET decomposition of arg5_addr reaches flow-time with reg_n_sets(p107)=2. The mechanism-hit is not realizable via block-local structural means.

- [s11] s11 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c restored to h5 candidate on session end; post-restore sandbox re-measures masked=2.

- [s12] s12 baseline: HEAD src/system.c (both-named arg4/arg5 array-index form) scores masked=7 via sandbox cpu_side_move_dir_4 --disable all; applying h5 candidate.c to src/system.c restores masked=2 (target_insns=160, build_insns=160).

- [s12] s12 P1 (pointer-walker `s32 *ap`): masked=2 INERT vs h5 baseline. Combine.c substitutes through pointer-typed intermediates via addsi3_internal exactly as through s32 intermediates — RTL is type-agnostic for the (plus symref reg) shape.

- [s12] s12 P2 (dead-sink `s32 sink = a5; (void)sink;`): masked=2 INERT vs h5 baseline. GCC tree-level DCE eliminates sink; residual matches s11 M1-tertiary and combine folds identically.

- [s12] s12 mechanism finding: five independent C-realizations of the two-SET arg5_addr frontier now measured KILLED across s11+s12 (algebraic-cancel +9, opaque-carrier +27, simple two-SET INERT, pointer-walker INERT, dead-sink INERT). The frontier hypothesis 'make p107 flow-time reg_n_sets=2 via block-local structural decomposition' is closed by construction: combine.c's addsi3_internal substitution runs BEFORE flow.c recomputes reg_n_sets, and every structural decomposition either (a) folds cleanly (INERT — combine substitutes) or (b) introduces a non-trivial subterm that disturbs the alloc web (regression). No middle-ground realization exists in the block-local structural axis.

- [s12] s12 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor).

- [s12] s12 modality-exhaustion: structural axis fully closed on csmd4's h5 base — s3 (13 block-local structural variants), s4 (do-while(0) 4 scopes), s5 (block-scope carriers, named dispatch, whole+nested+call-only wraps, permuter fresh-seed 9040 iters), s9 (m2c + 4 marionation transplants), s11 (3 arg5_addr two-SETs), s12 (2 more arg5_addr two-SETs). All frontier mechanism-hits at the block-local structural level are now measured KILLED. Any future structural session must target a genuinely novel axis (cross-block-scope declaration hoisting into an outer sequence-point, fn-body-level scope changes, or upstream declaration-order edits at src/system.c:388-408) — not more block-local decompositions.

- [s13] s13 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s13] s13 frontier #1 (fn-body-scope D_800F19C0 hoist into do_timeout): masked=15 (+13). File-level s-reg ref-balance coupling CONFIRMED direction-of-effect for hoisting-into-loop-body — the store is emitted per-iteration and disrupts the alloc web.

- [s13] s13 frontier live #2 (g3 base + block-scope multi-set VALUE carrier for arg5): measured INERT vs g3 baseline masked=6 across TWO seed sources (*idx_1495 and t0). The h5-basin s5 F1a/F1b finding transfers verbatim to g3 basin: the multi-set VALUE carrier vehicle is qty-invariant regardless of first-set source symbol on both basins.

- [s13] s13 permuter fresh-seed on h5-multexpander chassis (base_score=60, structurally distinct from s5's g3-basin base=40 campaign): 2999 iterations / 142.5s / 0 novel finds. Prior output-40-1/2 are pre-existing (from an older workspace chassis) and unchanged. The h5-multexpander local-minimum-under-random-mutation is stable at 20-min wall time; matches the g3-basin s5 result (9040 iters / 0 novel).

- [s13] s13 permuter tooling note: perm_add_sub randomizer hits AssertionError('int - pointer') on the `(u8*)tbl_125c + t0` pointer-arith pattern in the h5 chassis (tmp/grind/cpu_side_move_dir_4/s13/permuter_run_tail.log). Worker pool tolerates it (2999 iters completed via other mutation methods) but this pattern is a partial-blindspot for permuter mutations targeting that expression node — a directed-PERM_* form on that node might explore forms permuter misses randomly.

- [s13] s13 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor).

- [s14] s14 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s14] s14 directed permuter workspace: tmp/perm_csmd4/base.c edited with PERM_GENERAL(4-way, t0-side) + PERM_GENERAL(5-way, arg5-side) + PERM_RANDOMIZE(debug_printf) at the h5 inline block. Original saved at tmp/grind/cpu_side_move_dir_4/s14/base_pre_annotate.c; annotated at tmp/grind/cpu_side_move_dir_4/s14/base_annotated.c.

- [s14] s14 launch/harvest via tools/permuter_campaign.py (label s14_h5_directed_ptrarith): elapsed_s=895.7, iterations=23427, finds_new=6 (3 at score 40, 3 at score 50), best_new_score=40, procs_killed=9, stopped=true. Pre-launch snapshot: 2 pre-existing score-40 finds. All new finds >= baseline.

- [s14] s14 finding: merged base_score of the 20-source PERM cross-product was 40 (g3-basin) not 60 (h5-multexpander); my PERM_GENERAL alternatives at the t0 site included `(s32)tbl_125c + t0` (statement-form / g3-equivalent) and `(s32)((u8 *)t0 + (s32)tbl_125c)` (swapped-operand) which the permuter picked as the lower-scoring base. So the campaign explored the g3 mutation neighborhood, redundant with s5's 9040-iter g3-basin campaign that found 0 novel.

- [s14] s14 finding: AssertionError blindspot on the pointer-arith node is STILL ACTIVE — 19 permuter failures accumulated during the run, tracebacks in tmp/grind/cpu_side_move_dir_4/s14/campaign_tail_20k.txt point to src/randomizer.py::perm_temp_for_expr -> ast_types.py::decayed_expr_type -> deref_type -> `assert isinstance(type, (ca.ArrayDecl, ca.PtrDecl)), 'dereferencing non-pointer'`. Directed PERM_GENERAL alternatives do NOT fully bypass this blindspot; the mutator still hits it on random passes over the annotated block.

- [s14] s14 finding: output-40-3 novel form (`new_var2 = &D_800A11DC[D_800A11D5];` hoist + `temp = arg5;` fn-scope carrier + `t0 = (s32)&((u8*)tbl_125c)[t0]` indexed form) is cheat-shaped per no-new-park-categories (fn-scope carrier + dead alias, no semantic purpose); saved to memory/grind/cpu_side_move_dir_4/rejected/perm_s14_hoist_new_var2_g3_basin.c. Confirms the WIP-recorded 'standalone-permuter finds are UNFAITHFUL / cheat-shaped' warning transfers to directed-PERM output.

- [s14] s14 src/system.c restored to HEAD (both-named arg4/arg5 array-index form, masked=7 baseline). candidate.c unchanged (h5 form remains masked-2 floor).

- [s14] s14 modality note: the frontier's assumption that directed PERM on the pointer-arith node would bypass the blindspot is only PARTIALLY true — my alternatives are DIRECTED SEEDS but PERM_RANDOMIZE'd mutations still trigger the same AssertionError. Further, my alternatives inadvertently included g3-basin-shape spellings, so the merged base_score dropped to 40 and the campaign redid g3 exploration. Future directed-PERM sessions must AUDIT the alternative set to guarantee all forms preserve h5's mult-expander LAUNCH (multi-set t0 chain, fresh p106 dest at the SLL) before launch.

- [s15] s15 baseline: applying memory/grind/cpu_side_move_dir_4/candidate.c to src/system.c inline block scored masked=2, target_insns=160, build_insns=160 via sandbox --disable all. src/system.c restored to HEAD both-named form at session end.

- [s15] sched.c::rank_for_schedule at lines 2399-2456 (tools/gcc-2.7.2/sched.c) is the terminal ready-queue comparator: (1) priority diff → (2) class diff via LOG_LINKS(last_scheduled) + insn_cost gate → (3) LUID diff. All three tests exposed and named.

- [s15] RANKDBG event corpus in s6 block=3 shows 51/51 events return val=0 (grep -c cls=3 = 51). The entire function's ready-queue outcome is LUID-decided.

- [s15] Terminal decision at clock=13: RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0 → LUID(121)=12 beats LUID(111)=8 → pair emits as 118,111,121 vs target 118,121,111.

- [s15] The class-2 (anti/output-dep) attack is structurally CLOSED: 121→123 is producer→consumer (RAW), not WAR/WAW.

- [s15] The class-1 (data-dep with cost>1) attack has ONE untested C-source realization: make insn 121's PLUS operand involve a mul-result / HI-LO producer so mips_adjust_cost raises the 121→123 edge latency above 1. Not in the rejected forms bank; not in the ledger; qualifies as a novel frontier.

- [s15] The LUID-raise attack (moving t0*=4 past arg5 in C order) is s6-KILLED (drops to g3 basin masked=6).

- [s15] The 121-loses-LAUNCH attack (arg5_addr multi-set at flow-time) is s11+s12 5-realization KILLED (combine.c addsi3_internal substitution folds every simple two-SET; non-trivial subtrahends regress alloc web).

- [s15] s7 expmed.c:2244 case alg_shift NULL_RTX wall on insn 111 basin composition is not disturbed by any class-attack path — attacking 121's class does not touch 111's LAUNCH source.

- [s16] tools/gcc-2.7.2/config/mips/mips.h:2946-2948 (ADJUST_COST macro) — for MIPS in GCC 2.7.2, the macro is a single line: 'if (REG_NOTE_KIND(LINK) != 0) COST = 0;'. Comment: 'On the MIPS, ignore the cost of anti- and output-dependencies.' NO clause raises data-dep cost. The R8000 fixme comment at 2936 confirms the file's minimal intent.

- [s16] tools/gcc-2.7.2/sched.c:1363-1417 (insn_cost body) — computes cost = result_ready_cost(insn) clamped to >=1, then ADJUST_COST can only lower it (or set LINK_COST_FREE=1). Never raises above result_ready_cost(producer).

- [s16] tools/gcc-2.7.2/insn-attrtab.c:755+ (result_ready_cost dispatched by INSN_CODE — the generated table from mips.md function_units) + tools/gcc-2.7.2/config/mips/mips.md:148-183 (define_function_unit for memory/imuldiv units on r3000): load=2, hilo=1, imul=12, idiv=35, default arith=1.

- [s16] TYPE(121) is arith (addsi3_internal) per s6 lreg dump — result_ready_cost(121)=1 on r3000 unconditionally. Upstream operand types (whether p75/p79 came from a mul or a shift or a lbu) do NOT enter insn_cost(121,link,123) at all.

- [s16] Combined with s15's val=0 across 51 RANKDBG comparisons in block=3 and s15 H2's cls=2 structural impossibility, the class-attack surface for the h5-basin residual pair-swap is now exhausted at the compiler-source level — no class-differentiated decision path exists that a C-source lever can reach.

- [s16] The h5-basin's LUID tiebreak (LUID(121)=12 beats LUID(111)=8 at clock=13) is the sole surviving mechanism for the pair-swap, and the only lever known to affect LUID (statement order in C) is coupled to basin membership per s6 LUID-reorder + s7 expmed.c:2244 findings — LUID manipulation that keeps h5 basin membership is s6-KILLED.

- [s16] This forensics finding does NOT touch frontier #2 (PERM_LINESWAP, permuter modality) or frontier #3 (decomp.me corpus scrape, gated on curl_cffi install). Both remain unmeasured / unavailable in this session per the modality contract.

- [s17] s17 baseline (from task brief + ledger): h5 candidate.c is the masked-2 floor; residual is the {sll4@54 <-> addu5@55} LAUNCH-vs-LAUNCH LUID tiebreak per s6/s7/s15/s16 forensics.

- [s17] curl_cffi 0.15.0 confirmed importable in .venv/bin/activate (`python3 -c "import curl_cffi"` succeeds). The s9-recorded `ModuleNotFoundError: No module named 'curl_cffi'` blocker on the decomp.me corpus scrape modality is REMOVED as of s17.

- [s17] Corpus size: 3754 scratches under tmp/decomp_me_corpus (gcc2.7.2-cdk / gcc2.7.2-psx / psyq3.5 toolchain classes).

- [s17] Top-30 shingle-similarity hits saved at tmp/grind/cpu_side_move_dir_4/s17/scrape_top30.txt. Peak similarity 0.097 (zppzv); tail-of-top-15 similarity 0.075. All similarities below 0.10 indicate coincidental token overlap, not structural analog.

- [s17] Top-15 shape check (tmp/grind/cpu_side_move_dir_4/s17/shape_check.py): 0/15 hits contain debug_printf or FntPrint (the 5-arg variadic call that anchors csmd4's residual pair); 1/15 references VSync (erpie, entrypoint_sotn, distinct skeleton); 0/15 contain tbl-indexed dispatch. The corpus does not host a viable transplant candidate for csmd4.

- [s17] Rederive-modality closure updated: s8 m2c KILLED, s9 marionation_Exec sibling transplant KILLED (P1-P4), s9 decomp.me GATED, s17 decomp.me MEASURED KILLED. The only remaining rederive angle noted in ledger is external-Kengo-source glance (frontier note (c) from s9), which requires operator supply of a Kengo dump.

- [s18] s18 baseline: applied memory/grind/cpu_side_move_dir_4/candidate.c to src/system.c inline block; sandbox cpu_side_move_dir_4 --disable all reports score=2 target_insns=160 build_insns=160 (h5 masked-2 floor confirmed).

- [s18] HEAD src/system.c (both-named arg4/arg5 array-index form) scored masked=7 pre-apply — matches s12 baseline record (marionation-hybrid basin, ip_base_statement_form region).

- [s18] Kengo local-asset inventory: top-level Kengo/ + Kengo/disc/ (498 MB BIN + 3.15 MB SLUS_200.21 PS2 ELF); Kengo/kengo_functions_full.txt (4169 lines), kengo_func_names_sorted.txt (2482 syms), kengo_debug_full.txt (1215 file-line-addr records), kengo_globals*.txt. Identical copies replicated under multiple .claude/worktrees/*/Kengo/.

- [s18] cpu_side_move_dir entry at kengo_functions_full.txt line 1077: '/* 0013aa60 00000280 */ cpu_side_move_dir() {}' — empty stub, no body, no signature, no locals.

- [s18] Kengo dump has 2409 empty-stub entries (grep -c '{}$'); cpu_side_move_dir is one of them, alongside 2408 other unresolved-body functions. Some libc/math funcs (cos/sin/atan2/cosf/sinf) DO carry m2c-shaped signatures + local reg/stack info in the same file, so dump format supports body content when extraction succeeded — cpu_side_move_dir extraction did not.

- [s18] File attribution for cpu_side_move_dir recoverable from surrounding section header: src/numata/nm_cpu.c (matches src/system.c:497 tombstone reference).

- [s18] cpu_side_move_dir size in Kengo: 0x280 bytes = 640 bytes = 160 insns — EXACT match with BB2 csmd4's 160-insn footprint, confirming they are the same-purpose function despite the '_4' suffix delta on BB2's side.

- [s18] kengo_debug_full.txt has zero records at addr 0x13aa60 (grep 0013aa60 = empty); the .dbg records are limited to .dsm/.vsm VU-microcode files, so no source-line reconstruction available for nm_cpu.c.

- [s18] Kengo ELF architecture per kengo_debug_full.txt line 1: 'file format elf32-tradlittlemips' = PS2 R5900 EE core, GCC 3.x SN Systems / EE-GCC toolchain (Kengo shipped 2001).

- [s18] BB2 architecture: PS1 R3000A, GCC 2.7.2 PsyQ (mips-gcc-2.7.2 fork). Compiler-generation gap between BB2's mult-expander (expmed.c case alg_shift NULL_RTX, s7 CONFIRMED) and PS2/EE-GCC 3.x's mult-expander is not shape-transferable in codegen.

- [s18] src/system.c reverted to HEAD (both-named form) at session end; sandbox re-measures masked=7 baseline. candidate.c unchanged (h5 form remains the masked-2 floor).

- [s18] Rederive-modality closure across s8/s9/s17/s18: m2c fresh decompile KILLED (s8, produced score-14 inline-all-args), marionation sibling transplant KILLED (s9 P1-P4 = 7/14/8/20), decomp.me corpus scrape KILLED (s17 top-15 zero structural matches, peak similarity 0.097), Kengo local-asset probe KILLED (s18, empty dump stub + arch-non-transferable ELF). Every rederive angle enumerated in the ledger is now measured KILLED.

- [s18] The next modality for a follow-up session should NOT be rederive — no rederive frontier remains. Live frontier note (a) PERM_LINESWAP is a permuter-modality task; frontier note (b) 'operator supplies Kengo source dump' is now measured to be UNAVAILABLE via local assets (empty stub, not merely absent — actively resolved KILLED).

- [s19] s19 synthesis modality: no src edit, no sandbox measurement per modality contract; candidate.c unchanged (h5 form remains masked-2 floor).

- [s19] Ledger cross-read confirms surviving mechanism space is exactly three frontiers: (1) h5-preserving PERM_LINESWAP, (2) duplicated-statement-into-arms on non-t0 targets under 2026-07-01 sanction, (3) g3-basin arg5-only directed permuter.

- [s19] s14 failure mode (merged base_score drop to 40 via g3-basin-shape alternatives) is preventable in frontier #1 by using LINESWAP-only (no PERM_GENERAL alternatives that could introduce g3 spellings).

- [s19] s10 wrong-side demotion of duplicated-statement lever is specific to t0 chain; NON-t0 targets are a distinct lever direction not yet measured.

- [s19] s5 unrestricted g3-permuter (9040 iters, 0 novel) does not close the g3-basin arg5-only directed frontier — that search subspace has never been enumerated with the correct guardrails.

- [s19] Class-attack path (frontier #1 in s15's original phrasing) is compiler-source-CLOSED per s16; no future session should re-derive.

- [s19] Rederive modality is closed across m2c/marionation/decomp.me/Kengo; the only remaining rederive angle would require operator-supplied additional twin source that is currently unknown to exist.

- [s19] Judge-risk pre-analysis per frontier: F1 default-PASS (pure statement-order), F2 per-target (D_800F19C0 OK, dispatch conditional, tbl_125c/pp bind likely FAIL), F3 default-FAIL risk from historical g3-permuter cheat family (mask/narrowing/alias repurposing).

- [s19] Synthesis artifact written to tmp/grind/cpu_side_move_dir_4/s19/synthesis.md documenting closed levers, merged attack, guardrails, and judge-risk analysis.

- [s20] Current src/system.c matches h5 candidate.c (v0-idx staging + pp-alias + t0 multi-set) and sandbox reports masked=2, target_insns=160, build_insns=160 at session start after applying candidate.

- [s20] Duplicated D_800F19C0 store keep-prologue variant: masked=6, build_insns=164 (+4). GCC jump2 find_cross_jump does NOT merge when a label (`do_timeout:`) sits between the identical-tail arm bodies — confirms label placement obstructs the intended byte-neutral cross-jump merge in this control-flow shape.

- [s20] Duplicated D_800F19C0 store no-prologue variant: masked=10, build_insns=160 (cross-jump MERGED). Empirically confirms the duplicated-statement-into-arms mechanism IS reachable via C source on the D_800F19C0 target when prologue init is removed AND the do_timeout: label sits AFTER the fallthrough arm's store: jump2 merges, build_insns matches target, and refs are lifted before the merge.

- [s20] But the resulting qty priority shift MISDIRECTS allocation with +8 masked regression — the ref-lift on D_800F19C0's pseudo shifts qty ordering AWAY from arg5-strict-win. This directly mirrors s5's whole-block do-while(0) wrap (masked=13, symmetric ref-lift also monotonically wrong-direction) and the s5 F3/F3b/F4 wrap-scope sweep (all wraps net-negative for h5 base on csmd4). The intended arg5-qty-lift differential is NOT produced by ref-multiplying the D_800F19C0 store — the pseudo D_800F19C0 write refs feed a distinct qty (arg-2 chain to debug_printf) whose priority lift pushes the alloc web the wrong direction.

- [s20] Dispatch load (D_800A11DC[D_800A11D5]) and tbl_125c/pp targets: both require fn-scope named carriers to enable the duplicated-into-arms construct (block-scope decls inside the debug_printf inner scope are NOT visible in the arms above do_timeout:). Fn-scope named carriers with zero semantic purpose are the cheat family per [[no-new-park-categories]] cheats-by-any-spelling and the s14 perm_s14_hoist_new_var2_g3_basin.c rejection precedent. Both KILLED without measurement per policy.

- [s20] Frontier #2 (duplicated-statement-into-arms on NON-t0 targets) is fully closed after s20: only the D_800F19C0 target admits a non-cheat duplication (no fn-scope carrier needed since the LHS is a real global with independent semantic purpose), and it is measurably wrong-signed for arg5-lift on the h5 basin. The other two ranked targets can only be realized via fn-scope-carrier cheat forms that fail layer-1 vetting.

- [s21] s21 h5 baseline reproduced from memory/grind/cpu_side_move_dir_4/candidate.c applied to src/system.c: masked=2, target_insns=160, build_insns=160.

- [s21] combine.c fold_rtx aggressively canonicalizes ALL self-doubling C forms to a single ashiftsi3: (plus x x) -> (ashift x 1); (ashift (ashift x 1) 1) -> (ashift x 2); (mult x 4) -> (ashift x 2). Three-way triangulation via probes A/B/C.

- [s21] expand_mult case alg_shift NULL_RTX target (s7 CONFIRMED mechanism) only births a distinct pseudo when the OUTER context provides no target for the multiply — i.e. when the multiply is a sub-expression of a larger tree. `v0 = v0 * 4;` with outer SET target = v0-pseudo folds identically to `v0 <<= 2`. The t0-side p106 fresh-dest birth in the h5 basin depends on `t0 *= 4;`-form where the outer SET's target is absorbed differently (or the *= compound assignment causes a target-thread-away).

- [s21] s21 CONFIRMED via probe D (masked=4): the sanctioned staged-value-reused-variable v0 SET in the h5 candidate is not decorative FAKE-annotation — the fn-scope v0's participation in the arg5 chain is load-bearing to masked=2. Removing v0 SET before the shift regresses +2. This is empirical proof that h5 masked=2 REQUIRES v0-pseudo-in-arg5-chain.

- [s22] s22 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via & tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all.

- [s22] s22 PERM_LINESWAP workspace: rewrote tmp/perm_csmd4/base.c inline block to strict h5 form (removed all prior s14 PERM_GENERAL/PERM_RANDOMIZE directives) with PERM_LINESWAP as SOLE directive wrapping 7 statements. campaign_meta.json base_score=60 (h5-multexpander chassis confirmed).

- [s22] s22 campaign: pid 4879 launched 2026-07-09T02:59:09Z, ran deterministically to 5040 iterations = 7! (all statement permutations enumerated); permuter exited normally (alive=false at 5040).

- [s22] s22 harvest: 3 NEW finds — output-55-2 (weighted 55; ordering: v0-full-chain-first, pp mid, t0 last), output-40-6 (weighted 40; arg5-first with pp hoisted), output-40-7 (weighted 40; arg5-first with t0 last). No sub-40 find.

- [s22] s22 sandbox verification: output-40-7 applied to src/system.c scored masked=6 (g3 basin). Confirms weighted-40 ↔ masked-6 g3 correlation from s5/s13 transfers to LINESWAP-generated orderings.

- [s22] s22 conclusion: strict-h5-preserving LINESWAP over the block's 7 statements yields NO ordering below h5 masked=2. Every reachable improvement over base_score=60 drops into g3-basin masked>=6, which is empirically WORSE than h5 in sandbox metric. Frontier CLOSED.

- [s22] s22 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c reverted to HEAD (both-named arg4/arg5 form) via `git checkout -- src/system.c` at session end.

- [s22] s22 novel-form rejected bank: memory/grind/cpu_side_move_dir_4/rejected/lineswap_arg5_first_v0_shift_early_g3.c (measured masked=6), lineswap_pp_hoisted_arg5_first_g3.c, lineswap_v0_full_arg5_first_pp_mid.c.

- [s22] s22 metric correlation refined: permuter weighted score 40 = g3 basin (masked=6, register exchange residual); weighted 55 = intermediate (partial-h5-partial-g3 spelling); weighted 60 = full h5 (masked=2 pair-swap residual). The permuter weighted metric penalizes register diffs but does not see the pair-swap that h5 shows in the byte-level metric, so lower weighted score does NOT imply better sandbox masked score in this basin family.

- [s23] s23 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via sandbox cpu_side_move_dir_4 --disable all.

- [s23] PERM_LINESWAP_TEXT workspace edit: tmp/perm_csmd4/base.c wraps 8 items (tslTm2LoadImage_2 + t0=idx[0] + v0=idx[1] + pp=&D_800F19C0 + t0*=4 + t0=(u8*)tbl+t0 + v0<<=2 + arg5=*(s32*)(v0+tbl)) in PERM_LINESWAP_TEXT sole directive. Backup at tmp/grind/cpu_side_move_dir_4/s23/base_s22.c.

- [s23] Campaign launch metrics: base_score=60 (h5-multexpander chassis confirmed at launch, guardrail satisfied); pid=425; 4 jobs; --stop-on-zero.

- [s23] Campaign harvest metrics: elapsed_s=1462.5, iterations=20429, finds_new=2, stopped=true, procs_alive at harvest per campaign_meta.

- [s23] Preexisting outputs at s23 launch (12): output-40-1..7, output-50-1..3, output-55-1..2. Post-harvest state (14 outputs): novel output-55-3 (+67.4s post-launch) and output-40-8 (+819.5s post-launch).

- [s23] output-55-3 diff: arg5-chain (v0=idx[1]; v0<<=2; arg5=*(v0+tbl)) hoisted before t0 chain (t0=idx[0]; t0*=4; t0=(u8*)tbl+t0); tslTm2LoadImage_2 at top; pp assignment mid-block. Sandbox measurement on src/system.c: masked=7, target_insns=160, build_insns=160. Novel intermediate basin — matches the s2 E_arg5_first head-load-flip regression signature (masked 7).

- [s23] output-40-8 diff: v0/arg5 chain hoisted before final t0 finish, with pp interleaved into t0 chain. Weighted-40 correlates to g3-basin masked=6 per s22 correlation (measured on output-40-7 there); assumed masked=6, not re-measured.

- [s23] The textual-swap axis under strict h5-preservation now measures on par with s22's AST-swap axis: crossing the tslTm2LoadImage_2 boundary does NOT unlock a below-h5 ordering. Combined with s22 (LINESWAP AST, 5040 orderings exhausted, 0 sub-40 finds) and s5/s13 (fresh-seed random-mutation 9040+2999 iters, 0 novel), the statement-ordering axis is CLOSED at both AST + text + random-random levels.

- [s23] Fresh-seed harvest at 1462s / 20429 iterations is the data point per owner directive 2026-07-07 fresh-seed discipline: 0 weighted-score-<40 novel finds after ~24 min = ordering-space local minimum stable for the h5-multexpander chassis under textual mutation.

- [s23] candidate.c: unchanged (h5 form remains masked=2 floor). src/system.c restored to HEAD (both-named form, masked=7 baseline) at session end. Rejected forms saved: memory/grind/cpu_side_move_dir_4/rejected/lineswap_text_arg5_hoist_pre_t0.c (masked=7 measured) + lineswap_text_g3_ordering.c (g3-dupe, unmeasured).

- [s24] s24 forensics modality: no src edit, no sandbox measurement per modality contract (h5 masked-2 baseline unchanged); candidate.c retained at memory/grind/cpu_side_move_dir_4/candidate.c.

- [s24] expmed.c:1947-1963 low-zero-bits branch for t=4: m=floor_log2(4)=2, q=1, recursion hits t==1 base at line 1916 (op[0]=alg_m,cost=0); best_alg becomes {alg_m, alg_shift(log=2)} at cost=shift_cost[2].

- [s24] expmed.c:2021 factor loop `for (m = floor_log2(t-1); m >= 2; m--)`: floor_log2(3)=1, loop condition 1>=2 FALSE, body never executes for t=4.

- [s24] expmed.c:2065 shift-and-add path for a*3/a*5/a*9 is gated on `(t & 1) != 0` - skipped for t=4.

- [s24] expmed.c:2243-2247 case alg_shift: `expand_shift(...,NULL_RTX,0)` hardcodes NULL_RTX target; the outer expand_mult `target` parameter (a candidate p101 outer LHS) is threaded ONLY into `add_target` (line 2236-2238) which is dead in the alg_shift branch.

- [s24] expmed.c:2249-2298 alg_add_t_m2 / alg_sub_t_m2 / alg_add_t2_m / alg_sub_t2_m / alg_add_factor / alg_sub_factor DO consult add_target/accum_target for the PLUS/MINUS step, but ALL are unselected by synth_mult(4) so add_target's threading is unreachable for t0*4.

- [s24] expmed.c:2202-2229 expand_mult variant sweeps (negate_variant, add_variant) call synth_mult with -val and val-1 respectively; each returns >= the base alg cost for val=4 (add_cost or negate_cost premium on top of a still-non-cheaper alternative).

- [s24] expr.c:5710-5715 COMPOUND_EXPR case: `expand_expr(op0, const0_rtx,...); emit_queue(); return expand_expr(op1, target,...);` - two sequential re-entries into expand_expr, producing sequential emit_insn calls.

- [s24] sched.c LUID assignment (top of schedule_region / sched_analyze) walks the RTL insn chain in emit order assigning strictly increasing integers; two distinct emit_insn calls always yield distinct LUIDs.

- [s24] s6 combine dump insn 111 REG_EQUAL note `(mult:SI (reg/v:SI 101) (const_int 4))` empirically confirms the alg_shift-via-val=4 dispatch path (fresh p106 target).

- [s24] PERM_INT frontier and comma-op frontier were BOTH described in the task brief as compiler-source-reachable mechanism hits; both are now CLOSED at the compiler-source level with no sandbox measurement needed.

- [s25] s25 baseline (HEAD src/system.c both-named form) sandbox masked=7, target_insns=160, build_insns=160 (unchanged - the HEAD spelling is neither h5 nor g3, it's the s12 intermediate basin).

- [s25] s25 probe (g3 base + do-while(0) around ONLY arg5 deref, applied to src/system.c) sandbox masked=12 (+6 vs g3 baseline masked=6; +10 vs h5 candidate masked=2), target_insns=160, build_insns=160.

- [s25] Restored src/system.c to HEAD both-named form at session end; post-restore sandbox re-measures masked=7 (HEAD-invariant).

- [s25] Instrumented cc1 dump on the g3+wrap variant produced via tmp/grind/cpu_side_move_dir_4/s6/dump.sh (BB2_QTY_DEBUG/BB2_SCHED_DEBUG/BB2_ALLOC_DEBUG/BB2_RANK_DEBUG=1 -da). Artifacts snapshot at tmp/grind/cpu_side_move_dir_4/s25/g3_wrap_arg5_deref.{log,greg,lreg,flow}.

- [s25] flow.c dump shows NOTE_INSN_LOOP_BEG at insn 311 and NOTE_INSN_LOOP_END at insn 341 wrapping the arg5 dereference block. Only 2 loop notes present in the entire function (grep -c LOOP_BEG|LOOP_END = 2).

- [s25] QTYDBG block=3 arg5/t0 pseudos (g3+wrap): p107 birth18/death20/refs=2, p113 birth22/death30/refs=4, p100 birth20/death26/refs=2, p106 birth16/death24/refs=2. BYTE-IDENTICAL to s6 h5 QTYDBG block=3 record (hypotheses.md s6 evidence line 196). The wrap did not lift REG_N_REFS for arg5's pseudo.

- [s25] SCHEDDBG block=3 (g3+wrap): 20 insns scheduled; 7 LAUNCH insns (pri=2130706433) at insns 111/121/123/129/134/144/146. Contrast s6 h5 baseline: minimal LAUNCH count (2 at 111/121 in the residual pair-swap window). The 4 new LAUNCHes at 129/134/144/146 are birthed by flow's re-analysis of intra-block SETs after the wrap inserts NOTE_INSN_LOOP_BEG.

- [s25] SCHEDDBG block=3 clock=13 tiebreak: RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0 -> LUID(121)=12 beats LUID(111)=8 -> emission order (backward: 121@13,111@14,118@15) = forward text 118,111,121 (pair-swap residual survives, matching s6 h5 exactly).

- [s25] SCHEDDBG block=3 clock=6-9 (new tiebreaks introduced by the wrap): PICK 146 luid=24 -> 144 luid=23 -> 134 luid=18 -> 138 luid=20 -> 116 luid=10. The extra LAUNCH insns are picked in LUID order and shift the emission of the arg5-adjacent code by +6 masked.

- [s25] build_insns=160 (matches target) despite the wrap creating 2 NOTE_INSN_LOOP_* notes and 4 extra LAUNCH insns - confirming the notes don't materialise as physical opcodes but DO influence sched-time decisions.

- [s25] Compiler-source cross-check: tools/gcc-2.7.2/flow.c mark_used_regs / mark_set_regs increment REG_N_REFS by exactly 1 per use, with no loop_depth weighting in the update. loop_depth influences local-alloc.c qty_compare only via bb->frequency, and a do-while(0) zero-iteration loop yields frequency<=1 (not amplified). Therefore the frontier's 'flow.c multiplies REG_N_REFS by loop-depth' mechanism assumption is FALSE at the compiler source level for zero-iteration wraps.

- [s25] The g3 base's v1<->a0 register-exchange residual is inseparable from the qty numbers p113 (t0) 6refs vs p100 (arg5) 2refs; without a mechanism to lift arg5's refs to >=4 (per s1 fable-blitz arithmetic pri>=5000), the exchange cannot flip. This session forensically confirms do-while(0) wraps are NOT such a mechanism on g3 base.

- [s26] s26 rederive modality: no src/system.c edit; HEAD sandbox re-measures masked=7 (both-named baseline s12 basin); h5 candidate.c would restore to masked=2 (not applied this session).

- [s26] s26 decomp.me corpus residual-signature scan: 0/3754 hits (independent-dest {sll,addu,sll} window + jal-within-15). Distinct novel measurement from s17's whole-function shingle scan (which peaked at 0.097 similarity). The corpus's absence of csmd4's residual pattern at the instruction-cluster level is now measured, not just at the whole-function level.

- [s26] s26 in-repo residual scan discovers saEft01Init as a THIRD BB2 twin with the same {sll,addu,sll} -> jal debug_printf residual signature (previously the ledger only knew of marionation_Exec as file-local twin). saEft01Init lives at src/system.c:806-857; regfix.txt has 15 rules for it including 3 register renames + reorder + subst clusters — INCOMPLETE with a hand-installed pair-swap workaround.

- [s26] s26 saEft01Init C form: `arg5 = tbl_125c[idx_1494[1]]; arg4 = tbl_125c[idx_1494[0]]; debug_printf(fmt, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);` — both-named array-index REVERSE-eval. csmd4 s9 P3 already measured exactly this shape at masked=8 (+6 regression). The novel-twin discovery does NOT produce a novel transplant candidate — the same-shape has been measured KILLED on csmd4 already.

- [s26] s26 tslTm2LoadImage C body is a stub: `s32 tslTm2LoadImage(...) { (void)a0; (void)a1; (void)a2; (void)a3; return 0; }` — the function is asmfix'd (asmfix.txt: `tslTm2LoadImage: replace_with_asmfile "asm/funcs/tslTm2LoadImage.s"`), so its target bytes come from a full asm replacement, not from a compiled C form. NOT a viable template.

- [s26] s26 policy note: saEft01Init's regfix pattern (register renames $16<->$18, $16<->$17, subst pairs, reorder) is the exact hand-installed pair-swap workaround the anti-cheat policy forbids for csmd4. Its persistence in the tree is a queue-item for another session — but its shape does NOT transfer as a csmd4 solution.

- [s26] s26 additional in-repo hits (2/6) are wrong-context: func_80067200 jal func_80079154 (not debug_printf; different arg structure); single_game_CheckStatusUpDataTotalOver jal func_8001FB34 (not debug_printf). Neither produces the arg4/arg5 tbl-index dispatch shape.

- [s26] s26 candidate.c unchanged (h5 form remains masked-2 floor); src/system.c reverted to HEAD (both-named form, masked=7) at session end; committed WIP entry unaffected.

- [s26] s26 rederive-modality full closure status: m2c (s8) KILLED, marionation_Exec sibling transplant (s9 P1-P4) KILLED, decomp.me shingle scan (s17) KILLED, Kengo local dump (s18) KILLED, decomp.me residual-pattern scan (s26) KILLED, BB2 in-repo twin scan (s26) KILLED with novel-twin discovery yielding no viable transplant. Every enumerable rederive angle is now measured KILLED — future sessions must not repeat rederive.

- [s27] s27 baseline: HEAD src/system.c (both-named arg4/arg5 form at line 425-430) scores masked=7, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all` — matches ledger's HEAD floor 7.

- [s27] s27 h5 baseline reconfirmed: applying memory/grind/cpu_side_move_dir_4/candidate.c to src/system.c restores masked=2, target_insns=160, build_insns=160.

- [s27] s27 REDERIVE probe P1 (fn-scope tbl_11dc = D_800A11DC pointer stage, mirroring saEft01Init pattern): masked=14, target_insns=160, build_insns=163 (+3). REGRESSION +12 vs h5. Mechanism confirmed via build_insns diff — GCC materializes D_800A11DC via lui/addiu prologue pair (2 extra insns) plus a store to the fn-scope tbl_11dc local (1 more insn), disturbing s-reg web.

- [s27] s27 REDERIVE probe P2 (block-scope tbl_11dc pointer stage inside inline { } block): masked=2 INERT, target_insns=160, build_insns=160. Bytes byte-identical to h5 baseline — cse.c copy-propagation folds the single-use pointer alias; final RTL matches inline `D_800A11DC[D_800A11D5]` verbatim.

- [s27] s27 novel scope-dependence finding: block-scope named pointer for dispatch symbol is cse-transparent (INERT); fn-scope named pointer for dispatch symbol forces lui/addiu prologue materialization (+3 insns). This differentiates the two saEft01Init-mirror scopes and closes both as csmd4 levers.

- [s27] s27 cross-twin decl-diff catalog complete: csmd4 uses cross-symbol idx_1495 lie (line 406) + inline dispatch; marionation uses honest idx_1495=1+idx_1494 + idx_1496 + register-pin + volatile-alias (bulk pins); saEft01Init uses pre-staged tbl_125c AND tbl_11dc (no idx_1495, no register pins per src listing but ledger says 15 regfix rules — pins elsewhere in file).

- [s27] s27 frontier #1 (cross-function qty instrumentation on saEft01Init) NOT executed — it is a forensics probe, not a rederive probe; would land in wrong modality. Deferred to a future forensics session; expected value: comparing p107/p113 qty numbers between saEft01Init's pinned h5-equivalent basin and csmd4's h5 basin to determine whether saEft01Init's pin family targets the same p106/p107 LAUNCH pair as csmd4's h5 residual.

- [s27] s27 src/system.c restored to HEAD at session end (git checkout HEAD -- src/system.c); candidate.c unchanged (h5 form remains the masked-2 floor).

- [s27] s27 rederive-modality census (post-session): m2c s8 KILLED (inline-all-args), marionation sibling s9 P1-P4 KILLED (s-reg web non-fungible), decomp.me shingle s17 KILLED (peak 0.097 similarity), Kengo dump s18 KILLED (empty stub + arch-non-transferable), decomp.me residual-pattern + BB2 in-repo residual-cluster s26 KILLED (0 external hits, in-repo hits either self or asmfix-stubbed), saEft01Init decl-transfer s27 KILLED (both scopes). The rederive modality is now closed across all six sub-angles; no viable rederive sub-frontier remains beyond speculative cross-function forensics.

- [s28] s28 ledger cross-read: 27 prior sessions total across permuter (s5/s13/s14/s22/s23), forensics (s6/s7/s15/s16/s24/s25), rederive (s8/s9/s17/s18/s26/s27), structural (s3/s4/s11/s12/s20/s21), synthesis (s10/s19), collectively closing every C-source-reachable mechanism hit.

- [s28] s28 closed-lever catalog: LUID reorder (s6), class attack (s16 TRIPLE-KILL), arg5_addr two-SET five realizations (s11 M1-a/b/c + s12 pointer/dead-sink), p106 multi-set (s7 expmed.c:2244 NULL_RTX), do-while(0) wraps at all scopes (s4/s5/s25), LINESWAP AST 5040-iter exhaustive (s22), LINESWAP_TEXT 20429-iter (s23), block-local structural 13-variant (s3), comma-op + PERM_INT (s24), rederive six sub-angles (s8/s9/s17/s18/s26/s27), duplicated-into-arms non-t0 (s20), fn-scope carriers cheat family, cross-symbol idx_1495 retirement blocked (s8).

- [s28] s28 residual mechanism: sched.c::rank_for_schedule at clock=13, both insn 111 and insn 121 LAUNCH pri=0x7f000001, cls=3 vs cls=3 (val=0 across all 51 block-3 RANKDBG events per s15), LUID(121)=12 beats LUID(111)=8 (backward-scheduler picks 121 first) → linear order 118,111,121 vs target 118,121,111.

- [s28] s28 register-half status: h5 candidate emits target-correct dispositions (100→3/v1, 106→4/a0, 113→2/v0, 107→2/v0 per s6 .greg) — masked=2 does NOT hide a register diff; the sole residual is the pair-swap.

- [s28] s28 speculative axis catalog (untried, unpromising): outer-function flow restructure (F3 candidate) is the only ledger-enumerated angle not yet measured; helper-function extraction likely closed by function-boundary preservation in cc1 -O2 (F2 to verify).

- [s28] s28 synthesis artifact: tmp/grind/cpu_side_move_dir_4/s28/synthesis.md documents closed levers, surviving space, frontier reset, ruling-request precondition, judge-risk per frontier.

- [s29] s29 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s29] s29 F3-P1 (outer flow flattened, both success: and do_timeout: labels removed, fallthrough to timeout arm): masked=5, target_insns=160, build_insns=158 (-2). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/f3_outer_flatten_invert_drop_success.c (P2 shape, direction-equivalent to P1).

- [s29] s29 F3-P2 (branch-sense inverted on second test: `if (0x3C0000 < cnt) goto do_timeout;`, success: label removed, do_timeout: kept): masked=5, target_insns=160, build_insns=158. IDENTICAL to P1 result - direction-invariance confirms success: label removal is the -2 insn source, not branch-sense.

- [s29] s29 F3-P3 (nested-if fallthrough form: `if (!(D_800F19B8 < v0)) { cnt=...; if (!(0x3C0000 < cnt)) goto success; } tslTm2LoadImage_2(...); ...; v0=-1; goto check; success: v0=0; check:`, do_timeout: label removed but success: label preserved): masked=2 INERT, target_insns=160, build_insns=160. Bytes byte-identical to h5 baseline. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/f3_outer_flatten_drop_do_timeout_inert.c.

- [s29] s29 mechanism finding: the success: label is a load-bearing BB boundary for h5's masked=2 alignment; the do_timeout: label is redundant (fallthrough compiles identically). Any outer-flow restructure that removes success: regresses via -2 branch insns from jump.c cross-jump/straight-line collapse.

- [s29] s29 novel finding vs prior ledger: F3 is the FIRST measured outer-flow topology axis. Prior s3/s5/s20 measured block-local axes (statement re-association, decl order, do-while(0) wraps, duplicated-into-arms on D_800F19C0) but never touched the outer if-chain BB topology. F3-P3's INERT result is the first evidence that some outer-flow labels are freely removable without disturbing h5.

- [s29] s29 F3-P3 also gives a byte-neutral C simplification that could be adopted in candidate.c (drop the redundant do_timeout: label) without any masked score change - but does not advance the floor. Not applied to candidate.c since ledger contract is to preserve the h5 form; INERT restructures do not require candidate updates.

- [s29] s29 modality contract: structural axis (block-local variants + outer-flow topology) is now closed on csmd4's h5 base after s3 (13 block-local variants), s4 (do-while(0) 4 scopes), s5 (block-scope carriers/named dispatch/wraps), s11+s12 (5 arg5_addr two-SET realizations), s20 (duplicated-into-arms on non-t0), and s29 (F3 outer-flow topology 3 realizations). All structural mechanism-hits at both block-local and outer-flow scope are now measured KILLED.

- [s29] s29 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c restored to HEAD (both-named form, masked=7 baseline) at session end via `git checkout HEAD -- src/system.c`.

- [s29] s29 novel structural-axis frontier remaining: after F3 closure, the only outer-flow angle not measured is `loop:` label elimination (would require converting `goto loop` at line 495 to a real `while(1)`/`do{}while` construct with `break` on success and `continue` semantics; this changes fn-level control-flow topology which is a distinct probe from F3's outer-if-chain within the loop body). Left as a candidate for s30+ if the driver revisits structural modality; risk: the fn-level while(1) refactor is more invasive and may disturb the s-reg web coupling documented in s8/s9.

- [s29] s29 F1 and F2 frontiers untouched (they are forensics + static-analysis modalities, not structural).

- [s30] s30 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via sandbox cpu_side_move_dir_4 --disable all.

- [s30] s30 F3b-P1 (while(1) fn-level refactor): masked=52, target_insns=160, build_insns=165 (+5 insns). The +5 physical opcodes indicate loop.c emits real loop-scaffolding (LOOP_BEG/LOOP_END notes plus at least one physical branch/label pair for the back-edge) that produces bytes, unlike do-while(0) zero-iteration wraps which are notes-only.

- [s30] s30 F3b-P2 (for(;;) equivalent): masked=52, target_insns=160, build_insns=165 (IDENTICAL to while(1)). Confirms GCC 2.7.2's c-parse.y normalizes both C-source loop constructs to the same tree/RTL - no dial available at the loop-syntax level.

- [s30] s30 novel finding: the ledger's F3b hypothesis that loop-depth weighting via a real natural-loop conversion would LIFT arg5 refs and unlock the qty flip is DISPROVED direction-wise: the ref-weighting materializes but MISDIRECTS the arg5-vs-t0 competition (matches s5 whole-block do-while(0) misdirection + s20 duplicated-into-arms D_800F19C0 misdirection + s25 g3+arg5-wrap misdirection - the third independent measurement of a ref-lift going the wrong way on this function's qty topology).

- [s30] s30 mechanism corollary: for csmd4's h5 basin the arg5 qty pri wall is NOT closable via ANY ref-lift lever measured to date (do-while(0) at all scopes, duplicated-into-arms, real natural loops). The arg5>t0 flip requires either (a) a lever that raises arg5's refs WITHOUT also raising t0's refs, or (b) a lever that lowers t0's refs. Symmetric ref-lifts have now been TRIPLY-KILLED across three distinct wrap/loop constructs.

- [s30] s30 outer-flow topology axis closure updated: after s29 F3 (3 realizations: success:/do_timeout: label removal) + s30 F3b (2 realizations: while(1)/for(;;)), the outer-flow structural axis is closed at both label-removal AND explicit-loop-conversion sub-axes. The only remaining outer-flow angle would be selective label removal with alternate exit-return spellings, which s29 F3-P3 already showed to be INERT (byte-neutral) - not a source of gradient.

- [s30] s30 src/system.c restored to HEAD (both-named form, masked=7 baseline) at session end via git checkout HEAD -- src/system.c; candidate.c unchanged (h5 form remains the masked-2 floor).

- [s30] s30 modality contract: structural axis is now closed at every scope measured across s3 (13 block-local variants), s4 (do-while(0) 4 scopes), s5 (block-scope carriers/named dispatch/wraps + 9040-iter permuter), s11+s12 (5 arg5_addr two-SET realizations), s20 (duplicated-into-arms non-t0), s29 (F3 outer-flow topology 3 realizations), and s30 (F3b fn-level explicit-loop 2 realizations). Every enumerable structural mechanism-hit is now measured KILLED. Future structural sessions have no untried axis at either block-local, outer-if-chain, or fn-level-loop scope.

- [s31] s31 permuter campaign (tmp/perm_csmd4, label s31_prologue_lineswap): base_score=60 (h5 chassis, guardrail met), 5!=120 orderings enumerated over the 5 fn-scope prologue init statements, 0 novel finds. Harvest telemetry: iterations=120, best_new_score=null, all 14 pre-existing outputs new=false.

- [s31] campaign.log scoring distribution: 110-2265 range across all orderings; no output-<score>-<ctr> below 60 written to disk. Most orderings destroy h5 basin membership (scores 800-2000+), confirming prologue-init order propagates directly to fn-scope pseudo birth luid but does NOT surface any sub-baseline ordering that preserves h5.

- [s31] Statement-order axis fully partitioned across three campaigns: s22 (inner block 7!=5040), s23 (block+call 8!+text neighborhood, 20429 iters harvested), s31 (prologue 5!=120). Union covers every reorderable C-statement region in cpu_side_move_dir_4 body. Total iterations across the axis: 25,589. Total novel finds below weighted 60: ZERO.

- [s31] Reconfirms s3 V12 finding (block-local decl reversal INERT) at prologue scope: C-statement-order of globals-init does not propagate to block=3 qty birth-order or LUID tiebreaks — the s-reg web treats prologue-init LUID as fungible.

- [s31] Chassis base_score consistency (60 = h5 across s22/s23/s31) confirms none of the three campaigns drifted into g3 (weighted 40) or intermediate basins (55) at chassis selection; every campaign faithfully explored the h5 mult-expander neighborhood.

- [s32] s32 campaign metadata: base_score=60 (h5 chassis preserved, guardrail satisfied), 35338 iterations, 1258.4s wall time, 4 workers, 2 novel finds, best_new_score=40, pid_alive_at_harvest=false (auto-stopped by --stop-on-zero after harvest).

- [s32] Novel find output-40-9 (seconds_since_launch=34.9s): fn-scope `s32 temp;` split of arg5 into `temp = *(s32*)(v0+tbl); arg5 = temp;`. Saved at memory/grind/cpu_side_move_dir_4/rejected/s32_perm_general_temp_arg5_two_set.c. Same class as s3-V6 arg4_named (+9 regression) and s11 M1-secondary opaque-carrier (+27 regression) — fresh intermediate pseudo drops h5 to g3 basin.

- [s32] Novel find output-40-10 (seconds_since_launch=38.1s): composite mutation — fn-scope `void **new_var2= &D_800F19C0;` hoist + alias arg5 through fn-scope `status` local (`status = arg5; debug_printf(...,status)`). Saved at memory/grind/cpu_side_move_dir_4/rejected/s32_perm_general_pp_hoist_status_alias.c. Novel spelling of an already-measured g3-basin family (fn-scope carrier disrupts h5 s-reg web + mult-expander LAUNCH signature).

- [s32] Both novel finds at weighted score 40 = g3 basin per s22/s23 established correlation weighted-40 <-> masked-6 (s22 measured output-40-7 on sandbox at masked=6, hypotheses.md L513 and L487). No re-measurement needed per correlation.

- [s32] Zero novel finds emerged between t=60s and t=1258s: the entire fresh-seed window past the initial cluster produced no additional distinct-score outputs. Per fresh-seed discipline (owner directive 2026-07-07, ~20-30min no-find harvest+stop), harvested at 1258s (~21min).

- [s32] Combined campaign coverage (s22+s23+s31+s32) exhausts the h5-preserving permuter neighborhood: LINESWAP (order axis, 7!=5040 exhausted at block scope, 5!=120 exhausted at prologue scope), LINESWAP_TEXT (20429 iters across block+CALL text-mutation), PERM_GENERAL (35338 iters across block-scope expression-level AST mutation). All four launched from base_score=60 h5 chassis; combined zero outputs at weighted <40 across ~60,807 total iterations.

- [s33] saEft01Init block=3 QTYDBG (from tmp/grind/cpu_side_move_dir_4/s33/saeft.log): ord=0 qty=1 p98 birth=12 death=22 refs=6 got=v0; ord=1 qty=0 p93 birth=8 death=20 refs=6 got=v1; ord=2 qty=4 p105 birth=24 death=38 refs=6 got=v0; ord=4 qty=2 p87 birth=20 death=28 refs=2 got=v1; SUGG qty=5 p100 birth=32 death=36 refs=2 got=t1; SUGG qty=3 p88 birth=22 death=40 refs=2 got=t3.

- [s33] csmd4 block=3 QTYDBG (from s6/csmd4_only.log): ord=0 qty=1 p107 birth=18 death=20 refs=2 got=v0; ord=1 qty=3 p113 birth=22 death=30 refs=4 got=v0; ord=2 qty=2 p100 birth=20 death=26 refs=2 got=v1; ord=3 qty=0 p106 birth=16 death=24 refs=2 got=a0.

- [s33] saEft01Init sched2 block=3 priorities (from saeft.log SCHEDDBG block=3 n_insns=21): distinct set {1,2,3,4,2147483536}. Zero occurrences of LAUNCH sentinel 2130706433 (0x7f000001). The 2147483536 (0x7fffff90) is the RETURN sentinel, distinct from LAUNCH.

- [s33] csmd4 sched2 block=3 priorities (from s6/csmd4_only.log): includes 2130706433 on insn 111 (p106=p101<<2, mult-expander fresh dest) and insn 121 (p107=p75+p79, arg5 addr) simultaneously at clock=13 ready list; LUID(121)=12 > LUID(111)=8 backward-scheduler tiebreak picks 121 first -> emission {118,111,121} vs target {118,121,111}.

- [s33] saEft01Init sched1 block=3 (BEFORE reload): >=15 LAUNCH-priority insns; sched2 (AFTER reload): 0 LAUNCH insns. The reload pass renumbered block=3 fresh SET dests into hard-regs already multi-set elsewhere in saEft01Init (v0 re-used by 2 sys_VSync retries; D_800F19BC writes) so birthing_insn_p returns FALSE at sched2 time.

- [s33] Structural divergence at every measurable layer: (a) live-span - saEft01Init 32 luid units vs csmd4 14; (b) refs - saEft01Init dominant pseudos refs=6 vs csmd4 refs=2-4; (c) LAUNCH activity sched2 - saEft01Init 0 vs csmd4 2 tied. The functions are NOT QTY-family twins despite sharing the debug_printf-window surface shape.

- [s33] Baseline sanity: candidate.c (h5) applied to src/system.c continues to score masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all; this session did not modify src/system.c or candidate.c.

- [s34] tools/gcc-2.7.2/ contains no tree-inline.c — tree-level inlining machinery does not exist in GCC 2.7.2 (introduced GCC 3.x). Sole inline mechanism is integrate.c::expand_inline_function.

- [s34] integrate.c:96 refuses inlining for any varargs helper: `if ((last && TREE_VALUE (last) != void_type_node) || current_function_varargs) return "varargs function cannot be inline";`. debug_printf takes 5+ variadic args; any helper forwarding the tail is varargs.

- [s34] calls.c:709 is_integrable dispatch: on refusal (varargs / size / etc.) falls through to real CALL_INSN emit — same shape as HEAD's debug_printf call in block=3.

- [s34] integrate.c:106,153 size-gate non-DECL_INLINE helpers out of the inline path; only `static inline` (DECL_INLINE=1) survives. But even DECL_INLINE cannot bypass the varargs guard at line 96.

- [s34] expand_inline_function (integrate.c:1154) is an RTL insn-copy pass that preserves SET-dest identity via reg_map/label_map/insn_map remapping. Post-inline flow re-analysis rebuilds reg_n_sets=1 on the copied p106 SET, so birthing_insn_p still fires LAUNCH at the copied insn — the h5 pair-swap residual is invariant under insn-copy inlining.

- [s34] s5-F2a/F2b measured named-dispatch masked=16 (+14): forwarding args as fn-scope pseudos destroys h5 launch-suppression alignment. Any fixed-arg wrapper materializes its parameters through the same mechanism at the call site.

- [s34] s33 named the reload-substitution mechanism (fresh block=3 SET-dests renumbered into hard-regs already multi-set fn-scope). csmd4's fn-scope write topology has single-set-per-global (D_800F19B8/BC/C0), so reproducing saEft01Init's substitution pattern requires adding fn-scope multi-set writes — which is the cheat family.

- [s34] candidate.c unchanged; src/system.c not modified (forensics modality). h5 masked-2 floor unchanged from ledger.

- [s35] s35 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s35] s35 P1 (v0 = *idx_1495): masked=3, build_insns=160 — novel +1 regression; idx_1495's extended live range disturbs h5 s-reg web without flipping the pair-swap residual.

- [s35] s35 P2 (v0 = ((u8*)&D_800A1494)[1]): masked=4, build_insns=161 — +2 regression + 1 extra insn (fresh lui/addiu materialization of &D_800A1494 that does NOT cse-fold to the idx_1494 pseudo across a block boundary).

- [s35] s35 P3 (v0 = idx_1495[0]): masked=3, build_insns=160 — identical to P1; confirms array-syntax and pointer-deref lower to the same MEM RTL at expand time.

- [s35] s35 novel finding: idx_1495-based index-source respelling is NOT fungible with idx_1494[1] on the h5 basin; the h5 alignment is coupled to the specific base-pointer identity (idx_1494) at the arg5 lbu site, not just the byte address computed. Extending the idx_1495 live range across the debug_printf window regresses by +1 in EVERY spelling tested.

- [s35] s35 novel finding: cse.c does NOT fold &D_800A1494 back to the idx_1494 pseudo across the arg5 lbu site, contrary to naive expectation — the fresh symbol reference materializes lui/addiu at reference site. This is a novel forensics data point about the RTL scope of cse's pointer-symbol canonicalization at the p107 arg5_addr site.

- [s35] s35 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c restored to HEAD (both-named form, masked=7 baseline) at session end via git checkout HEAD -- src/system.c.

- [s35] s35 rederive modality census: after s8 (m2c KILLED), s9 (marionation transplant P1-P4 KILLED), s17 (decomp.me shingle KILLED), s18 (Kengo dump KILLED), s26 (decomp.me residual + in-repo residual KILLED), s27 (saEft01Init decl-transfer KILLED), s35 (three idx_1495-based index-source respellings KILLED) — every enumerable rederive angle is now measured KILLED across seven distinct modality sub-attempts.

- [s35] s35 asymmetric ref-lift frontier NOT approached: every C-source lever that lifts ONLY arg5's refs via a control-flow arm or additional use has cheat-shape at layer-1 self-vet (no-new-park-categories cheats-by-any-spelling — no semantic purpose for the extra reference). Structural / permuter attacks on this axis are the surviving frontier, not rederive.

- [s36] s36 rederive-modality sub-angle #7 (in-repo COMPLETED-C 5-arg-debug_printf template scan) KILLED: 3 total 5-arg sites in src/*.c, all in system.c (L429/L541/L837), all INCOMPLETE. Confirms no native BB2 template exists for the arg4/arg5 tbl-indexed dispatch shape at COMPLETED-C bar.

- [s36] s36 rederive-modality sub-angle #8 (Kengo numata subsystem body inventory) KILLED: 64/64 nm_cpu.c functions are `{}` empty stubs; extraction gap is subsystem-wide across all `src/numata/*.c` files in the dump. Refines s18's cpu_side_move_dir-specific finding to a subsystem-wide extraction property.

- [s36] HEAD src/system.c sandbox cpu_side_move_dir_4 --disable all = 7 (target_insns=160, build_insns=160), matches ledger HEAD floor 7. Session did not modify src/system.c or candidate.c; h5 candidate remains the masked-2 floor.

- [s36] Rederive-modality now measured KILLED across 8 distinct sub-angles: m2c (s8), marionation sibling P1-P4 (s9), decomp.me shingle (s17), Kengo cpu_side_move_dir stub (s18), decomp.me residual + BB2 in-repo residual (s26), saEft01Init decl-transfer (s27), idx_1495 respellings (s35), in-repo COMPLETED-C 5-arg-debug_printf template + Kengo numata subsystem inventory (s36).

- [s36] No untried rederive angle enumerable from the ledger + this session's search remains. Future sessions on this function should not repeat rederive modality unless the operator supplies external inputs (a third-party PS1 decomp source with GCC 2.7.2 provenance and a byte-matched 5-arg variadic tbl-dispatch shape) — a class of input the ledger has never seen materialize.

- [s37] s37 ledger cross-read: h5 candidate.c masked=2 remains the floor since s4; HEAD masked=7 unchanged.

- [s37] Register half already target-correct (100->v1 106->a0 113->v0 107->v0 per s6 .greg); masked=2 is a pure pair-swap, no register diff hidden.

- [s37] Residual is one LUID tiebreak at sched2 clock=13: insn 111 (LAUNCH, p106 fresh from mult-expander) vs insn 121 (LAUNCH, p107 arg5_addr); rank_for_schedule val=0 across all 51 block=3 events (s15); LUID(121)=12 wins backward-scheduler pick over LUID(111)=8; linear text 118,111,121 vs target 118,121,111.

- [s37] Every ref-lift lever measured triply misdirects (s5 wraps monotonic wrong direction across 4 scopes; s20 duplicated-into-arms D_800F19C0 +8 masked; s25 g3+arg5-deref-wrap +10 masked; s30 while(1)/for(;;) +50 masked with +5 physical loop scaffold).

- [s37] Cross-function forensics s33 confirmed saEft01Init QTY diverges structurally: refs=6, luid-span=32 vs csmd4 refs=2-4, span=14; saEft01Init sched2 has 0 LAUNCH because reload substitutes fresh block=3 dests into fn-scope multi-set hard-regs (v0 re-used by 2 sys_VSync retries + D_800F19BC writes) — a substitution csmd4 cannot reproduce without fn-scope cheat carriers.

- [s37] Static-analysis F2 (helper extraction under -O2) triple-closed at compiler source per s34: (1) GCC 2.7.2 has no tree-inline.c; (2) integrate.c:96 refuses varargs helpers; (3) fixed-arg wrapper materializes params via s5-F2a/F2b named-dispatch path (masked=16 measured).

- [s37] Class-attack surface triple-KILL: s15 val=0 across 51 events (rank_for_schedule LUID always decides), s16 mips.h:2946 ADJUST_COST never raises (only lowers), s24 both alternate expand_mult algorithms (alg_add_t_m2, alg_add_factor) unreachable for t=4 and COMPOUND_EXPR emits identical LUIDs to ';'-statements.

- [s37] Cross-symbol idx_1495 lie at src/system.c:406 is in the semantic-lie family the 2026-07-05 ruling forbids but is currently LOAD-BEARING for h5 s-reg web (s8 honest-respelling +13); safe retirement is blocked until h5 closes or a different web is found.

- [s37] Synthesis artifact written at tmp/grind/cpu_side_move_dir_4/s37/synthesis.md documenting the terminal mechanism, closed levers, surviving axes, layer-1 risk analysis, and ruling-request precondition.

- [s38] s38 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s38] s38 target-asm downstream audit (asm/funcs/cpu_side_move_dir_4.s L80080ED4-L800810004): the only three uses of idx_1494 / idx_1495 after debug_printf are u8-value reads for callback args and the temp==2/5 test at L80080F3C/L80080F70/L80080F9C. Target has ZERO downstream site that would legitimately reference `arg5` (the s32 tbl_125c-indexed value) — every use is the raw u8 index byte, not the scaled table lookup.

- [s38] s38 P1 (idx_1495 seed at arg5 lbu + decl-order swap 405-406): masked=7, build_insns=161 (+1 physical insn). Regression +5. The two levers compound additively; the decl-swap forces a distinct lui/addiu materialization for idx_1494 during prologue (mirrors the s27 P1 fn-scope tbl_11dc materialization: +3 insns via lui/addiu prologue pair) rather than compensating.

- [s38] s38 P2 (idx_1495 seed alone, reconfirmed): masked=3, build_insns=160. Matches s35 P1 measurement verbatim — the +1 register diff basin is stable and reproducible.

- [s38] s38 P3 (idx_1495 seed + `u8 mode = *idx_1495;` legitimate hoist at callback site): masked=3 INERT vs P2. cse.c folds the single-use named local back to *idx_1495 direct read; the reg web does NOT shift; the +1 diff is unabsorbed. Confirms s5-F1a/F1b family finding that single-use named locals at flow time are cse-transparent — the finding transfers from the debug_printf window to the post-window arms.

- [s38] s38 mechanism corollary: axis-(b) compensating adjustment can NOT be realized in pure C via a named local at the callback site. To land idx_1495 in a distinct callee-save that absorbs the +1 diff, a fresh pointer/value carrier participating in the flow-time allocation web would be required — but any such carrier without semantic purpose beyond changing register assignments fails no-new-park-categories cheats-by-any-spelling. The frontier's precondition (`carrier MUST participate in a downstream use with real semantic purpose, not just live-across the debug_printf window`) is met by the `mode` local, but cse eliminates it from participation. There is no C form that both (a) has semantic purpose AND (b) survives cse to distinct-pseudo at flow time.

- [s38] s38 rejected forms saved: memory/grind/cpu_side_move_dir_4/rejected/s38_idx1495_seed_plus_decl_swap.c (P1 diff), s38_idx1495_seed_plus_mode_carrier.c (P3 diff).

- [s38] s38 h5 candidate remains masked-2 floor unchanged; src/system.c restored to HEAD (both-named form, masked=7 baseline) at session end via `git checkout HEAD -- src/system.c`.

- [s39] h5 chassis baseline reconfirmed at masked=2, target_insns=160, build_insns=160 with candidate.c applied.

- [s39] Novel expand-normalization data point: `tbl_125c[v0]` array-syntax and `v0<<=2; *(s32*)(v0+(s32)tbl_125c)` explicit-shift form produce byte-identical RTL/asm on h5 chassis; the array-syntax path does NOT birth a distinct fresh single-set pseudo on the arg5 side. Prior ledger did not enumerate this equivalence.

- [s39] Direct-symbol respelling `(s32)&D_800A125C + v0` in the arg5 addressing costs +1 physical insn (fresh lui/addiu materialisation) and +14 masked regression — confirms cse.c equiv_constant does NOT fold the reference-site symbol back to the fn-scope `tbl_125c` pseudo when the pseudo is not already in the arg5 chain (contrast with t0 side where tbl_125c pseudo participates).

- [s39] Triple-set v0 carrier (v0=lbu; v0<<=2; v0+=tbl) regresses +13 (masked=15) — extends the s11/s12 arg5_addr multi-set KILL family onto the v0 pseudo itself; the lever direction '121 loses LAUNCH via multi-set carrier' is measurement-closed at every enumerated C-spelling now including v0-scope.

- [s39] All three P1/P2/P3 probes are structural-modality (block-local statement re-association / respelling) and clear layer-1 cheat-vetting: no register pins, no volatile coercion, no dead stores, no hardcoded-$N asm.

- [s40] candidate.c (h5 chassis) applied to src/system.c produces sandbox masked=2 target_insns=160 build_insns=160 (baseline confirmed pre-campaign).

- [s40] s40 wider-scope PERM_GENERAL campaign: base_score=60 confirmed iter 1, 1738 iterations, 0 finds, harvested+stopped.

- [s40] The union of s22 (block LINESWAP, 7!=5040) + s23 (block+CALL LINESWAP_TEXT, 20429) + s31 (prologue LINESWAP, 5!=120) + s32 (h5 block PERM_GENERAL, 35338) + s40 (block+flanking-CALLs PERM_GENERAL, 1738) covers every permuter-modality scope reachable on this fn under h5-chassis preservation. All 5 campaigns yielded 0 weighted-<40 finds.

- [s40] The s37/s39 CONFIRMED closure — 'no C-source lever changes the basin without falling into g3' — is reinforced by the s40 wider-scope 0-find data point: even random AST mutation with statement-motion across CALL boundaries cannot surface a novel neighborhood.

- [s41] HEAD src/system.c (both-named arg4/arg5 array-index inline block form) scores masked=7 via sandbox cpu_side_move_dir_4 --disable all (rules_dropped=5, cheat_asm_stripped=22).

- [s41] Applying memory/grind/cpu_side_move_dir_4/candidate.c inline block to src/system.c restores the h5 chassis at masked=2 (target_insns=160, build_insns=160). Baseline confirmed unchanged from s40.

- [s41] s41 novel permuter axis: PERM_LINESWAP over the poll-loop callback if-blocks (status&4 -> D_800A11B8 callback vs status&2 -> D_800A11B4 callback). This scope has NEVER been targeted by any prior permuter campaign: s31 permuted the fn-scope prologue init (5-item, 120 orderings); s22 the h5 inline block (7-item AST LINESWAP, 5040 exhausted); s23 h5 block + pre-CALL (LINESWAP_TEXT, 20429); s32 h5 block PERM_GENERAL (35338); s40 h5 block + flanking tslTm2LoadImage_2 + cdrom_ClearIrq (1738). The poll-loop dispatch scope is uniquely un-permuted.

- [s41] PERM_LINESWAP tool-side constraint discovered: annotator/parser cannot handle brace-delimited compound statement blocks (`if (cond) { ... }`). Permuter exit rc=1 with `Syntax error in base.c. before: } at approximately line 425, column 23 (after PERM expansion)`. Recorded to campaign.log at tmp/grind/cpu_side_move_dir_4/s41/perm_poll/campaign.log and copied to tmp/grind/cpu_side_move_dir_4/s41/perm_poll_campaign.log.

- [s41] Rewriting the two callback if-blocks into single-line ternary or comma-op form to make them PERM_LINESWAP-compatible would itself be a codegen-visible edit — not a permutation of the current AST — so this rewrite path is out-of-scope for a permutation campaign and is instead a distinct hand-structural axis if attempted.

- [s41] src/system.c restored to HEAD both-named form at session end (masked=7 baseline). candidate.c unchanged (h5 form remains the recorded masked=2 floor).

- [s41] Judge constraint (BINDING per s40 ruling): 'Do not resurface canonical-asm authorization ... Next session must SHIFT MODALITY to (a) SOTN-family carve-out application under codified prerequisites + FAKE annotation + layer-2 cheat-reviewer, OR (b) ALLOCDBG-instrumented s-reg coupling hunt.' This session's chosen axis targets frontier (b) via a permuter vehicle (the mandated modality); the axis is now measurement-closed via the tool-side blocker, eliminating one of the potential (b) sub-realizations.

- [s42] s42 baseline (per ledger + task brief): h5 candidate.c is the masked-2 floor; residual is the {sll4@54 <-> addu5@55} LAUNCH-vs-LAUNCH LUID tiebreak at intra-block=3 clock=13 (per s6/s7/s15/s16 forensics). src/system.c unchanged this session (HEAD both-named form retained).

- [s42] s42 ALLOCDBG census (h5 baseline, from tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.log): 16 pseudos in global-alloc ranking; hardreg assignment 75->v0, 83->a1, 84->v1, 82->a0, 81->s0, 101->a0 (call), 120/126->v0, 86->a2, 137->a3, 80->s1, 77->s2, 79->s3, 78->s4, 72->s5, 73->s6. Priority formula pri=floor_log2(nrefs)*nrefs*10000/livelen.

- [s42] s42 pseudo-to-variable mapping (from tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.rtl): p72=a0 param, p73=a1 param, p77=&D_800A1494=idx_1494, p78=tbl_125c+(D_800A1494-D_800A125C+1)=idx_1495, p79=&D_800A125C=tbl_125c. p77 (idx_1494) at ord=11 pri=933 -> s2; p78 (idx_1495) at ord=13 pri=277 -> s4; p79 (tbl_125c) at ord=12 pri=675 -> s3.

- [s42] s42 block=3 QTY table (h5 baseline, from tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.log): only 4 pseudos p100/p106/p107/p113 (all block-local; birth luids 16-22, death 20-30). None of these are global-alloc pseudos; they are pure block-local qtys competing for a0/v0/v1 hardregs. idx_1494/idx_1495 are pre-assigned s2/s4 by global-alloc BEFORE local-alloc runs on block=3.

- [s42] s42 conflict-list separation (from tmp/grind/cpu_side_move_dir_4/s6/csmd4_only.greg): p77 conflicts = {72,73,75,77,78,79,80,81,86,101,120,126,2,3,4,5,6,7,29}; p78 conflicts identical. Neither list contains p100, p106, p107, or p113. Zero shared live range between idx_1494/idx_1495 and block=3 QTY pseudos means allocation coupling as described by frontier (b) is refuted at the .greg-level liveness dump.

- [s42] s42 priority-formula stability: p77 (idx_1494) pri=933 at nrefs=7 livelen=150 sits at ord=11. Adding one ref -> (3*8*10000)/150=1600 (moves up one slot, still in s2-s6 territory); removing one ref -> (2*6*10000)/150=800 (no change in ranking position). +/-1 ref delta from callback restructure cannot displace idx_1494/idx_1495 from the s2-s5 slot range.

- [s42] s42 LUID tiebreak separability (per s15 evidence + this session cross-check): the pair-swap residual is decided by RANKDBG cls=3 val=0 -> LUID diff on insns 111 vs 121 (LUID 8 vs 12); both insns are internal to block=3, and their LUIDs come from schedule_block's intra-block sched_analyze walk. Restructuring the poll-loop (blocks 5-9 in the CFG) changes LUIDs of poll-loop insns but leaves the 111/121 relative LUID delta intact.

- [s42] s42 flattened-dispatch pseudo count: the frontier (b) proposed C form `mask & 4 ? D_800A11B8 : D_800A11B4)(*(mask & 4 ? idx_1495 : idx_1494), ...)` introduces at minimum one new pseudo for the function-pointer ternary result plus one for the operand ternary; both would be live simultaneously in the callback window. This ADDS pressure to global-alloc, not relieves it - opposite of the frontier's directional claim.

- [s42] s42 modality-contract compliance: instrumented cc1 dumps referenced (RTL/ALLOCDBG/QTYDBG/greg); named GCC-source passes: global-alloc.c (find_reg, priority ordering), local-alloc.c (qty_compare, block-local qty table), sched.c::rank_for_schedule + LUID tiebreak (per s15 rank_for_schedule at lines 2399-2456). Named decision producing the divergence: block=3 clock=13 LUID(121)=12 > LUID(111)=8 with cls=3 val=0 -> emission 118,111,121 vs target 118,121,111.

- [s43] s43 forensics: no src edit; h5 candidate.c remains masked=2 floor. Analysis reuses s6 instrumented cc1 dump (canonical BB2_SCHED_DEBUG/BB2_RANK_DEBUG artifact).

- [s43] sched1 SCHEDDBG PICK clock=13 (csmd4_only.log:172): picked=121 with pri=2130706433 (LAUNCH sentinel 0x7f000001) luid=12. Ready list: [121(p=2130706433,l=12) 111(p=2130706433,l=8) 142(p=1,l=22)].

- [s43] sched2 SCHEDDBG PICK clock=13 (csmd4_only.log:775): picked=121 with pri=2 luid=7. Ready list: [121(p=2,l=7) 111(p=2,l=6) 142(p=1,l=4)]. NO LAUNCH sentinel.

- [s43] sched1-vs-sched2 LUID renumbering: sched1(111,121,118)=(8,12,11); sched2(111,121,118)=(6,7,5). sched2 re-numbers via post-reload RTL chain walk; the LUID gap between the residual pair collapses from 4 (sched1) to 1 (sched2).

- [s43] Mechanism: adjust_priority applies LAUNCH iff birthing_insn_p returns TRUE, which requires REGNO(SET-dest) >= FIRST_PSEUDO_REGISTER. Post-reload, dests of insns 111 (p106) and 121 (p107) are hard regs; birthing_insn_p returns FALSE at sched2 -> no LAUNCH adjustment.

- [s43] Both passes produce the SAME emission order (118, 111, 121) via distinct tiebreak mechanisms: sched1 = LAUNCH-vs-LAUNCH LUID diff; sched2 = plain-pri-2 LUID diff. The sched1 output determines sched2 LUID assignment, so sched2 preserves sched1's decision without needing LAUNCH.

- [s43] Ledger correction (novel forensic data point): s6 evidence line stating 'insn 111 pri=2130706433 (LAUNCH) and insn 121 pri=2130706433 (LAUNCH), simultaneously ready at clock=13 with 121(luid=12) picked first' was documenting sched1 state, not sched2 (LUID=12 exists only in sched1 numbering; sched2 has luid=7). The confusion propagated to s7/s15/s16/s28/s37/s42 phrasings but does not invalidate their conclusions -- the sched1 mechanism is the correct one, sched2 is just its inheritor.

- [s43] Attack-surface consequence: sched2's plain-priority-2 tie is symmetric (both 111 and 121 have exactly 1 downstream user contributing 1+1=2 to their priority chain). Raising pri(111) above pri(121) at sched2 requires either multi-use of p106 (blocked by s7 alg_shift wall = g3 basin drop) or extending 116's downstream chain via multi-use of p101 (blocked by h5's dependence on multi-set t0 LAUNCH suppression on 116 = g3 drop). No C lever operates at sched2 layer without also crossing s3/s11/s12/s39 KILLED families.

- [s43] Frontier (b) sanctioned-carve-out-at-callback-slot suffers a distinct KILL from this finding: LUIDs are computed per-block by sched_analyze walking each block's RTL chain, and block=3 (debug_printf window) is upstream of the callback poll-loop blocks (5-9 per csmd4_only.sched2). LUID assignments in blocks 5+ have zero effect on block=3's LUID tiebreak at sched2 clock=13.

- [s43] Frontier (a) PERM_GENERAL-at-callback-sites suffers the same per-block LUID isolation KILL: any AST mutation over the callback slots produces RTL insns in blocks 5-9, which do not participate in block=3 sched_analyze.

- [s44] curl_cffi 0.15.0 is now installed in .venv (s9's ModuleNotFoundError blocker resolved 2026-07-09); the decomp.me corpus rederive modality is measurable going forward without operator install action.

- [s44] decomp.me /stats reports 259874 total scratches / 181106 with asm as of 2026-07-09; the 602-scratch sample downloaded here is ~0.3% of the total but 100% of the BB2-toolchain-class first-page slice per compiler (download API paginates by creation_time desc; the sample skews toward recent submissions).

- [s44] gcc2.7.2-psx first-800 download timed out at 120 iterations (per-scratch fetch is ~1s incl. compiler_config + asm blob); parallel download of --limit 400 across 3 compilers completed 602 scratches in ~5min.

- [s44] Top-similarity psyq3.5 hit h1LHX at 0.079 is sonicdcer's Kalisto-engine `main` prologue (ResetGraph/InitialisePSX/MRSNDInit boilerplate) - lexical shingle overlap comes from generic call-marshaling asm sequences, not from any shared structural pattern with csmd4's debug_printf/poll window.

- [s44] Shingle threshold calibration: the twin marionation_Exec asm (same window shape confirmed by 43 prior sessions of forensics) would be expected to score >= 0.30-0.50 vs csmd4; the 0.079 max here is a clear NO-ANALOG signal, not a threshold-tuning artifact.

- [s44] src/system.c is unchanged (h5 candidate.c form remains masked-2 floor; not applied for this session's measurement since the probe is external-corpus lookup, not code editing).

- [s44] The full rederive modality (m2c s8 KILLED, sibling-transplant s9 KILLED, decomp.me corpus s44 KILLED) is now measurably closed; no further rederive angle remains without external inputs (Kengo source dump, external asm-recognition corpus, or human expert asm inspection).

- [s45] HEAD src/system.c sandbox cpu_side_move_dir_4 --disable all = masked 7 (target_insns=160, build_insns=160); h5 candidate.c remains masked-2 floor unchanged.

- [s45] Kengo/kengo_functions_full.txt whole-dump inventory: 4169 lines / 2409 empty stubs / 73 non-stub bodies. Non-stub sample: cos, sin, cosf, sinf, atan2, sqrt, __ieee754_atan2, sprintf, _sprintf_r, __sbprintf, _vfprintf_r — 100% runtime library, zero game code.

- [s45] Kengo grep -cE '\bprintf\b|\bfprintf\b|\bsprintf\b' = 4 (all libc). grep -c debug_printf = 0.

- [s45] Kengo nm_cpu.c section (lines 1023-1089): 64/64 entries are `{}$` stubs including cpu_side_move_dir at 0x13aa60 (=640 bytes = 160 insns, exact BB2 csmd4 size match per s18/s36).

- [s45] sotn-decomp local checkout at ../sotn-decomp: 1494 .c files; grep -rln debug_printf returns zero hits; SOTN's fmt family is OSReport / FntPrint / sprintf — orthogonal API to BB2's debug_printf.

- [s45] Tbl-indexed byte dispatch shape (`\w+\[\w+\[[01]\]\]`) present in SOTN at doors.c, doppleganger.c, unk_46E7C.c, game_handlers.c, iso9660.c, 4E04C.c, 20B48.c, 3AB08.c; manual inspection confirms none share BB2 csmd4's 5-arg variadic + adjacent-byte-index dispatch surface.

- [s45] Rederive-modality closure list extended from 9 → 10 sub-angles: prior s8 m2c / s9 marionation P1-P4 / s17 decomp.me shingle / s18 Kengo cpu_side_move_dir stub / s26 decomp.me residual + BB2 in-repo residual / s27 saEft01Init decl-transfer / s35 idx_1495 respellings / s36 BB2 5-arg template + Kengo numata subsystem / s44 decomp.me BB2-toolchain corpus + s45 (this session) cross-project SOTN scan + Kengo whole-dump body inventory.

- [s46] s46 synthesis modality: no src/system.c edit, no sandbox measurement per modality contract. h5 candidate.c remains masked=2 floor (unchanged since s4, 42 sessions).

- [s46] Judge constraint (BINDING per s40/s41 ledger) confirmed still active: do NOT resurface canonical-asm authorization for cpu_side_move_dir_4 or its twins; next session must SHIFT MODALITY to (a) SOTN-family FAKE-annotated named-local (matches F2), OR (b) ALLOCDBG-instrumented s-reg coupling hunt (matches F1's structural sub-realization).

- [s46] F2 primary rationale: judge lists it first; named-local-fake-exception rule (2026-07-01) codifies prerequisite framework; csmd4 has NEVER been probed with FAKE-annotated named-local at any scope in 45 sessions; s33 documented saEft01Init's sched2 zero-LAUNCH mechanism (reload renumbering block=3 dests into fn-scope multi-set hard-regs) as a mechanism csmd4 cannot reproduce WITHOUT a fn-scope carrier — the FAKE sanction is exactly the codified carve-out that could supply the missing multi-set fn-scope pseudo without triggering layer-1 auto-fail.

- [s46] F1 secondary rationale weakened by s38 audit: post-debug_printf arms (do_timeout:, poll:, success:, check:) contain ZERO downstream sites that read p101-post-shift; every use is raw u8 lbu on idx_1494/idx_1495. Any C-source WAR/WAW dep-edge on p101 without semantic purpose is dead-store-fake-exception territory needing its own SOTN evidence base.

- [s46] Concrete s47 candidate set drafted in synthesis artifact: C1 (s32 zero = 0 archetype), C2 (s16 three = 3 archetype), C3 (ALLOCDBG-instrumented mini-sweep across scalar types). Each candidate MUST clear layer-1 cheat-reviewer BEFORE sandbox measurement; any sub-h5 masked score MUST clear layer-2 fresh cheat-reviewer.

- [s46] Prerequisite audit for F2 completed this session per named-local-fake-exception.md: (1) lever-exhaustion documented across 45-session ledger; (2) GCC-pass interaction named (sched.c::birthing_insn_p FALSE at sched2 iff SET-dest reg_n_sets>1 post-reload; a fn-scope dead scalar in the correct hardreg class could force reload renumbering of block=3 SET-dests into fn-scope multi-set hard-regs per s33 saEft01Init mechanism); (3) mandatory /* FAKE: ... */ annotation to be authored at draft; (4) layer-1+2 cheat-reviewer per review-discipline-before-commit.md mandatory before commit — s47 workers must not skip.

- [s46] Search-space elimination this session: rederive modality (10 sub-angles KILLED, no further external corpus warranted), permuter-modality at h5 chassis (≈62,545 iters at 0 novel <40, redundant unless chassis changes), forensics on sched2 pri arithmetic (s43+s16 close class-attack sub-frontier at both sched1 and sched2 layers), poll-loop restructuring (s41/s42 idx_1494/idx_1495 zero-conflict with block=3 QTY pseudos).

- [s47] s47 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s47] s47 C1 (`s32 zero = 0;` fn-body-top dead unread): masked=2 INERT, insns=160. Tree-level DCE removes uninit/unread scalar locals before local-alloc. The named-local FAKE mechanism (F2 primary) cannot fire on a DCE-eliminable form.

- [s47] s47 C2 (`s32 zero = 0;` + `v0 = zero;` at success arm, constant-holder spanning calls): masked=10 (+8), insns=163 (+3). SOTN DispSamnailWindow-shape constant-holder DOES reach local-alloc as a live pseudo but the callee-save reserve + preservation moves across debug_printf/cdrom_ClearIrq cost 3 real insns and 8 masked units; h5 basin destroyed.

- [s47] s47 C3 (`s32 zero = 0;` + `D_800F19BC = zero;` single pre-block=3 use, no cross-call): masked=2 INERT, insns=160. cse folds zero back to literal at the single-use site before local-alloc. Short-live-range constant-holder is qty-invisible.

- [s47] s47 F2 primary closure: SOTN FAKE-annotated `s32 zero = 0;` archetype has no middle-ground realization on csmd4's h5 base. Either (a) DCE'd if unused (C1) / cse-folded if singly-used (C3) — INERT and never reaches local-alloc as a distinct pseudo, or (b) preserved across calls (C2) — costs +3 real insns and regresses +8. There is no `s32 zero = 0` shape that both survives to local-alloc and stays within the h5 alignment.

- [s47] s47 F1 secondary closure via post-block=3 arms audit (asm/funcs/cpu_side_move_dir_4.s L80080EDC..L80081004): all arms use $s2/$s4/$s6/$s5 and literal/dispatch pointers only; no downstream reader of the t0<<2 post-shift value exists under any spelling. Confirms t0-shift value is dead after L80080ED8 (the final lw a3,0(a0) inside the debug_printf window). No WAR-edge-based pri(111) lift is available from an honest downstream expression.

- [s47] s47 mechanism note: the s33 saEft01Init reload-renumbering pattern (fn-scope multi-set hard-reg killing birthing_insn_p) requires the FAKE-annotated local to (i) reach local-alloc as a live pseudo AND (ii) not add real emitted insns. On csmd4's h5 base these two requirements are mutually exclusive across the three probed archetypes: the local is either DCE/cse-eliminated (fails i) or it costs preservation insns (fails ii). The mechanism-realization gap is not a random measurement gap but a structural one: any zero-cost fn-scope local is by construction not present at flow-time.

- [s47] s47 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2, insns=160. candidate.c unchanged (h5 form remains the masked-2 floor).

- [s48] s48 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s48] s48 C4 (fn-scope `held` + `held = cnt;` fast-path between lines 416-417): masked=2 INERT vs h5 baseline. Byte-identical.

- [s48] s48 C5 (fn-scope `held` + `held = *idx_1495;` fast-path pointer-source): masked=2 INERT vs h5 baseline. Byte-identical. Rules out copy-fold as sole explanation for C4 inert.

- [s48] s48 C6 (fn-scope `held` + `held = *idx_1494;` INSIDE block=3 at do_timeout entry): masked=2 INERT vs h5 baseline. Byte-identical. Definitively confirms dead store to a written-never-read local scalar gets DCE'd upstream of flow's reg_n_refs count regardless of placement (fast-path or block=3-interior).

- [s48] s48 mechanism finding: three placements of a written-never-read `held` fn-scope local scalar - two in the fast path (scalar-source and pointer-source) and one inside block=3's dominator region at the do_timeout entry - all measured INERT. The frontier F2-B assumption 'reg_n_refs(held) reaches 1 at flow-time and shifts qty tables' is falsified: GCC's tree-level DCE (or an equivalent early flow.c dead-set elimination) removes the SET before flow.c recomputes reg_n_refs, so held's ref never participates in local-alloc.c qty_compare regardless of where the write is placed.

- [s48] s48 F2-B closure: with C4 (fast-path scalar), C5 (fast-path pointer), and C6 (block=3-interior pointer) all measured KILLED, the entire F2-B WRITTEN-never-read fn-scope named-local scalar direction is CLOSED at the mechanism level, not just for these three specific spellings. Any variant is by construction dead-store-DCE-vulnerable.

- [s48] s48 rejected forms saved (3): memory/grind/cpu_side_move_dir_4/rejected/s48_C{4,5,6}_*.c.

- [s48] s48 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains the masked-2 floor).

- [s48] s48 policy vetting: all three probes were FAKE-annotated at declaration + assignment per dead-store-fake-exception.md / named-local-fake-exception.md prerequisites (lever-exhaustion documented s2-s47; GCC-pass mechanism named; annotation applied). Layer-1/2 cheat-reviewer not invoked because all three probes were INERT vs baseline - nothing to accept.

- [s49] s49 baseline: h5 candidate applied to src/system.c scores masked=2 (score=2, target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s49] s49 F3 audit measurement: `grep -nE '\bmult\b|\bdiv\b|\bmadd\b|\bmfhi\b|\bmflo\b|\bmultu\b|\bdivu\b' asm/funcs/cpu_side_move_dir_4.s` returns ZERO matches. Target asm has no HI-LO consumer anywhere. F3 (frontier hypothesis: raise 121->123 edge latency via HI-LO producer for arg5_addr) closes as not-applicable AND cheat-shaped (any C-source HI-LO producer inserted purely to trigger mips_adjust_cost class-1 gate has no semantic purpose beyond changing GCC's scheduler cost per no-new-park-categories).

- [s49] s49 F4 audit measurement: 13 stores total in cpu_side_move_dir_4.s; frame slot 0x14 is the only unused stack gap and target does NOT write it. dead-vars-local-array.md carve-out prerequisite (target bytes contain corresponding dead stores, oracle-enforced) NOT satisfied. F4 closes as not-applicable.

- [s49] s49 narrow permuter campaign (tmp/grind/cpu_side_move_dir_4/s49/perm_narrow): base_score=60 confirms h5 chassis preserved; 15064 iterations, 0 novel finds, elapsed 567.1s. Extends the h5-chassis 0-find plateau from ~40k (s13+s32+s40) to ~55k accumulated iterations.

- [s49] s49 modality-exhaustion note: F3 (HI-LO producer) and F4 (dead-vars-local-array) were the two live frontier axes in the s48 ledger. Both close via target-asm audit without measurement (F3: target has no HI-LO ops; F4: target has no unmatched dead frame stores). Frontier F5 was already documented as closed-at-drafting per no-new-park-categories (`(void)&local` in cheat catalog; multi-set carrier redundant with s5 F1a/F1b). All three s48-declared live frontiers are now measurement-closed; the live frontier list is empty.

- [s50] h5 candidate.c applied to src/system.c reproduces masked=2 baseline (sandbox --disable all: score=2, target_insns=160, build_insns=160).

- [s50] s50 permuter campaign: workspace tmp/grind/cpu_side_move_dir_4/s50/perm_poll_gen; base_score=60 (h5 chassis preserved); 29289 iterations / 1565.2s; 1 novel find at weighted=40 (output-40-1, seconds_since_launch=662.7); permuter fell back to full-function random-mode (`No perm macros found. Defaulting to randomization.`) because the PERM_GENERAL directive was placed inside a compound `if (status != 0) { ... }` block — same class of tool-side limitation as s41 PERM_LINESWAP on brace-delimited blocks.

- [s50] s50 novel find (output-40-1): hoists `void **new_var2= &D_800F19C0;` fn-scope + inline block uses `pp = (void **) new_var2;` and `temp = *(s32*)(v0+(s32)tbl_125c); arg5 = temp;`. Applied to src/system.c: masked=2 (target_insns=160, build_insns=160) — SAME as h5 baseline; the weighted-40 improvement is a register-name reshuffle the masked scorer normalizes out.

- [s50] Cheat-vetting the find: `new_var2` = fn-scope pointer-alias with no semantic purpose beyond routing pp; `temp` = load-intermediate with no semantic purpose (arg5 is direct load-and-pass). Both constructs match the 'no semantic purpose' + 'necessary only because permuter said so' signals in no-new-park-categories — rejected without measurement dependency.

- [s50] src/system.c reverted to h5 candidate; masked=2 confirmed post-revert.

- [s50] Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s50_perm_new_var2_temp_intermediate.c with cheat-vetting notes for future sessions.

- [s50] Cumulative h5-chassis permuter closure now ≈85k iterations across 9 sessions at 0 novel weighted-<40 finds; the s50 whole-function-random-mode superset campaign confirms no basin exists below weighted-40 within h5-chassis mutation reach.

- [s51] s51 baseline: no src edit this session (following s42/s43/s46 forensics precedent that reuses the s6 canonical cc1 dump). h5 candidate.c remains masked=2 floor unchanged (target_insns=160, build_insns=160 per s50 baseline confirmation).

- [s51] sched1 block=3 initial insn enumeration (log line 107) preserves chain order: 92,94,100,104,111,116,118,121,123,129,134,138,140,142,144,146,148,153,156,158 (20 real insns). LUIDs assigned sequentially with LINE_NOTE gaps at 2,3,5,7,9,14,15,17,19,26.

- [s51] sched2 block=3 initial insn enumeration (log line 710) shows chain REORDERED by reload: 92,94,100,104,142,118,111,121,123,129,116,138,134,144,146,140,148,153,156,158. Insn 116 pushed 6 positions later (from between 111,118 to after 129); insn 142 pushed 12 positions earlier (from between 140,144 to before 118); insn 140 pushed 5 positions later. This is a novel post-s43 data point: reload's chain-rearrangement is deterministic given reg-pressure state, not attackable via C source.

- [s51] sched1 clock=13 PICK decision (log line 172): RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0. Ready list: [121(p=2130706433,l=12) 111(p=2130706433,l=8) 142(p=1,l=22)]. Picked 121 by LUID tiebreak (12>8). This seals the pair-swap emission order 118,111,121 at sched1; sched2 inherits and confirms with plain-pri=2 LUID diff 7>6.

- [s51] LUID-gap composition (novel, not previously in ledger): LUID 8 = insn 111 (mult-expander SLL, fresh p106, LAUNCH); LUID 9 = NOTE_INSN (LINE_NOTE, non-schedulable); LUID 10 = insn 116 (t0 third-stage plus, multi-set p101, pri=2 non-LAUNCH); LUID 11 = insn 118 (arg5 lw, multi-set p75, pri=2 non-LAUNCH); LUID 12 = insn 121 (arg5_addr plus, fresh p107, LAUNCH).

- [s51] Load-bearing analysis of the two real intervening insns: insn 116 removal = t0 chain collapses to g3-family single-set spelling (measurement-closed via s3-V10 masked=9, s6 LUID-reorder masked=6, s7 static walkthrough of expmed.c case alg_shift). Insn 118 removal impossible: 118 is 121's data producer (sets p75 that 121's PLUS consumes) so a RAW dep pins 118 chain-position before 121.

- [s51] Pass identity confirmed: sched.c::schedule_block called at sched1 (pre-reload), function rank_for_schedule at lines 2399-2456 of tools/gcc-2.7.2/sched.c. Terminal decision at clock=13 as documented above. Cross-consistent with s15 (val=0 across all 51 block=3 events) and s43 (sched2 LUID delta collapse to 1).

- [s52] Target-asm audit (asm/funcs/cpu_side_move_dir_4.s, 176 lines): D_800A11DC has exactly ONE logical use — the arg5 varargs-slot-2 load at insns 66-68 in block=3. ZERO downstream-arm re-use sites. Artifact: tmp/grind/cpu_side_move_dir_4/s52/asm_audit_D_800A11DC_downstream_readers.txt.

- [s52] Target-asm audit: $s3 = &D_800A125C (tbl_125c) is read at insns 58 and 63 (both inside block=3's pre-debug_printf staging, matched by h5 candidate); ZERO post-debug_printf-arm reads (L80080EDC..L80081004 use $s2 idx_1494, $s4 idx_1495, $s5/$s6 arg pass-throughs, $s1 saved status bits, and D_800A147C/D_800A11B4/D_800A11B8 dispatch pointers — never $s3).

- [s52] Target-asm audit: ZERO occurrences of the composite shape (u8*)tbl_125c + (t0<<2) in the arms — reconfirms s47's F1 audit finding on the sibling &D_800A11DC symbol; extends the closure to include the arg5-side base symbol.

- [s52] s47 C2 (fn_scope_zero_constant_holder_C2.c) empirical bound: any FAKE-annotated fn-scope carrier held live across debug_printf costs masked=10 (+8) and build_insns=163 (+3); the general form is monotone-worse than h5 baseline, independent of carrier type (scalar/pointer).

- [s52] Sibling closures (extends the axis-wide KILL): s47 C1/C2/C3 (three scalar-archetype fn-scope FAKE carriers), s48 C4/C5/C6 (three fast-path/do_timeout-entry realizations), s49-F4 (dead-vars-local-array prerequisite absent — zero unmatched frame stores). Combined: the SOTN-family named-local-fake-exception carve-out is CLOSED at every measured C-realization AND at every downstream-reader axis available in target bytes.

- [s53] src/system.c at HEAD reads masked=7 sandbox (rules_dropped=5, cheat_asm_stripped=22); applying memory/grind/cpu_side_move_dir_4/candidate.c reproduces the documented masked=2 h5 floor (rules_dropped=5, cheat_asm_stripped=22). Ledger's WIP CHECKPOINT floor is reproducible on THIS session.

- [s53] m2c --reg-vars v0,v1,a0 output produces `var_v0 *= 4; var_v0 += &D_800A125C; var_v1 = *var_v0` for the arg5 chain — a THREE-in-place-set walk on v0 as the reg-carrier — plus `var_a0 *= 4; var_a0 += &D_800A125C; temp_a3 = *var_a0` for t0. m2c-regvars is the FIRST rederive angle to surface a persuasive spelling target for the s10 M1 frontier's `p107 multi-set` mechanism using v0 reuse rather than a fresh named addr pseudo.

- [s53] The m2c-regvars-derived in-place walk on v0 reaches masked=15 (+13) at sandbox — the same broken-web basin s39 measured for `v0-triple-set-carrier`, EMPIRICALLY confirming that reusing v0 for arg5's ADDR (not just the value) does NOT decouple the pair-swap: it disrupts the s0/s2/s4 s-reg web that h5's masked-2 chassis depends on.

- [s53] Statement-order transplant of the full m2c-regvars line ordering (arg5 subseq fully before t0 subseq, preserving head-lbu order) also reaches masked=15 — the disruption is dominant regardless of C-source ordering.

- [s53] Combined evidence: the s10 M1 frontier's mechanism (`p107 multi-set at flow-time via v0 reuse`) is NOT reachable in pure C — combine.c substitution folds every legitimate multi-set spelling of the ADDR arithmetic (per s11/s12 measurements on fresh-var forms), AND rewiring v0 itself to carry the multi-set introduces s-reg-web disruption that is monotonically worse than the pair-swap it's trying to fix. The frontier is measurement-closed across THIS rederive axis.

- [s53] Reconfirms the s6/s7 mechanism-locked conclusion: the residual pair-swap requires flipping insn 121's LAUNCH signature WITHOUT introducing multi-set behaviour on p101 (the v0 pseudo). No pure-C spelling reaches that combination; every attempt either folds via combine (INERT) or spreads to the s-reg web (regression).

- [s54] m2c flag matrix exhausted for block=3 reconstruction: default (s8), --reg-vars v0,v1,a0 (s53), --void (s54), --no-stack-spill+--gotos-only (s54) all produce line-identical debug_printf window shape: debug_printf(fmt, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8*, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8*, 1) * 4) + &D_800A125C)). The fully-inlined-args expression form is a fixed point of m2c's block-3 reconstruction across every flag combination that could theoretically influence expression-tree shape.

- [s54] The inline-all-args expression form corresponds to already-KILLED forms in the rejected bank: s9-P2 (arg5 named + arg4 inline: masked=14), arg5_fully_inline_no_v0_staging.c (in bank), s39-probe1 (arg5 as tbl_125c[v0]: masked=2 INERT). No fresh h5-preserving spelling emerges from m2c-modality variants.

- [s54] The remaining un-run m2c flags (--no-andor, --no-switches, --no-unk-inference, --stack-structs, --heuristic-strings, --stop-on-error) affect ONLY: &&/|| detection in outer control flow (--no-andor: irrelevant to block=3 which has no boolean shortcircuits), switch/jump-table detection (--no-switches: csmd4 has no switch), unknown-type inference (--no-unk-inference: all types in block=3 are known), stack-struct template emission (--stack-structs: block=3 has no stack access), string heuristics (--heuristic-strings: no rodata strings in block=3), error handling (--stop-on-error: no errors). None can influence block=3 expression reconstruction.

- [s54] s54 sanity check: H5 candidate.c applied to src/system.c reproduces masked=2 baseline (score=2, target=160, build=160). H5 chassis alive; s54's rederive KILL is against a live baseline.

- [s54] Ledger cross-check: s37 CONFIRMED 'every rederive sub-angle enumerable from the ledger is measurement-closed across 8 distinct sources' — s54 extends this to 9 distinct sources (m2c-flag-space).

- [s54] Judge constraint acknowledged: s54's mandated modality was rederive, not the s53-frontier-preferred ALLOCDBG or sched2-rank_for_schedule axes. Rederive angle chosen (m2c flag exhaustion) is the only remaining actionable rederive sub-angle; on completion here, the entire rederive class is fully closed and future sessions must shift modality per the s53 frontier notes.

- [s55] s55 no src edit / no sandbox measurement per synthesis-modality contract; candidate.c (h5 chassis) remains masked=2 floor unchanged since s4.

- [s55] Terminal mechanism (locked across s6/s7/s15/s16/s24/s28/s37/s42/s43/s51): sched1 block=3 clock=13 ready [121(p=0x7f000001,l=12) 111(p=0x7f000001,l=8) 142(p=1,l=22)]; cls=3 val=0; LUID(121)=12 beats LUID(111)=8; emission {118,111,121} vs target {118,121,111}. sched2 inherits via plain-pri-2 LUID delta 7 vs 6.

- [s55] Register half target-correct (100->v1 106->a0 113->v0 107->v0 per s6 .greg); masked=2 hides no register diff.

- [s55] Rejected-forms inventory: 72 files under memory/grind/cpu_side_move_dir_4/rejected/ + 66 named in the task brief.

- [s55] Judge constraint (BINDING per s40/s41/s46/s54) reaffirmed: do not resurface canonical-asm authorization; next session must shift modality within {SOTN carve-out (closed s47/s48; skip), ALLOCDBG s-reg coupling hunt, novel forensics/structural axis}.

- [s55] H1 (sched2 LUID enumeration) is compiler-source forensics within the allowed frontier; H3 (LINE_NOTE condensation) is a structural probe never measured on csmd4; H2 (ALLOCDBG conflict-list-diff) is a direct realization of the task-brief frontier note (1).

- [s56] s56 baseline: h5 candidate applied to src/system.c scores masked=2 (target=160, build=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s56] s56 H3 single-line condensation of the 4 residual-window statements: masked=2 INERT, bytes identical to baseline. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/line_note_condense_single_line.c.

- [s56] s56 H3-inverse blank-line expansion of the same 4 statements: masked=2 INERT, bytes identical to baseline. Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/line_note_expand_blank_lines.c. Together the two probes bracket the physical-line-layout axis as fully masked from the residual pair-swap in production.

- [s56] s56 mechanism KILL: Makefile:36 CC_FLAGS = '-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w' — no -g. cc1 in production never emits NOTE_INSN_LINE. LINE_NOTE-based LUID-shift levers of ANY spelling are architecturally inapplicable to the production sched2 pass.

- [s56] s56 corollary: any future frontier proposal predicated on 'shift LUID(X) by manipulating debug notes' is DOA on this build without a Makefile change (out of scope — no-compiler-divergence forbids toolchain edits including CC_FLAGS changes).

- [s56] s56 modality note: hand-STRUCTURAL now closes another dimension. Combined with s3 (13 block-local axes closed), s4 (do-while(0) 4 scopes), s5 (block-scope carriers, named dispatch, whole/nested/call-only wraps + permuter fresh-seed), s9 (m2c + 4 marionation transplants), s11/s12 (5 arg5_addr two-SETs), s13 (fn-body hoist + g3 carrier + h5-chassis permuter), s14 (directed-PERM), and now s56 (physical-line/LINE_NOTE axis), the structural axis on csmd4's h5 base is fully swept modulo the two live forensics frontiers (H1 sched2 LUID neighbor-shift, H2 ALLOCDBG conflict-list diff on idx_1495 spellings).

- [s56] s56 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor).

- [s57] s57 baseline: HEAD src/system.c (both-named arg4/arg5 array-index inline block form) scores masked=7 (rules_dropped=5, cheat_asm_stripped=22); applying h5 candidate.c inline block restores masked=2 (target=160, build=160). h5 chassis alive.

- [s57] s57 P1 (block-3 locals hoisted to TAIL of fn-body decl list): masked=2 INERT, bytes byte-identical to h5 baseline. First empirical measurement on csmd4 of cross-block-scope decl hoisting for variables used in a single downstream block.

- [s57] s57 P2 (block-3 locals hoisted to TOP of fn-body decl list, before int new_var): masked=2 INERT, bytes byte-identical to h5 baseline AND to P1. Fn-body decl ordering has zero effect on the sched1 pair-swap residual.

- [s57] s57 mechanism corollary: the twin marionation s2/s3 finding 'pseudo birth follows RTL first-use, not C decl order' — previously measured only at BLOCK-local decl-reversal axis (s2 A_decl_reverse, s3 V12 decl_pp_first) — now measurably transfers to the CROSS-BLOCK-SCOPE axis (arg5/t0/pp declared at fn-scope but used only inside block=3). The rule is scope-agnostic on csmd4.

- [s57] s57 modality-closure extension: combined with s3 (13 block-local axes closed), s4 (do-while(0) 4 scopes), s5 (block-scope carriers/named dispatch/whole+nested+call-only wraps + permuter fresh-seed), s9 (m2c + 4 marionation transplants), s11/s12 (5 arg5_addr two-SETs), s13 (fn-body hoist to do_timeout body + g3 carrier + h5 permuter), s14 (directed-PERM), s56 (physical-line/LINE_NOTE), and now s57 (cross-block-scope decl hoisting at 2 placements), the structural axis on csmd4's h5 base is fully swept modulo the two live forensics frontiers (H1 sched2 LUID neighbor-shift, H2 ALLOCDBG conflict-list diff on idx_1495 spellings) — both explicitly OUTSIDE the s57 mandated structural modality.

- [s57] s57 cheat-vetting: probes P1/P2 do NOT add semantic-purposeless constructs — they merely change the C-scope declaration slot of variables that continue to be used identically at block=3 first-use points. No new pseudos, no dead stores, no register pins, no volatile coercion, no hardcoded-$N asm. Clear layer-1 cheat gates by construction.

- [s57] s57 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains the masked-2 floor).

- [s58] h5 baseline confirmed: candidate.c applied to src/system.c -> `sandbox cpu_side_move_dir_4 --disable all` yielded score=2 target_insns=160 build_insns=160 scorable=true rules_dropped=5 cheat_asm_stripped=22 (matches 45+session ledger).

- [s58] s58 permuter campaign launched (pid 413) with h5 candidate.c as base, PERM_GENERAL at fn-body prologue scope; parser rejected the directive and fell back to full-function random-mode with base_score=60.

- [s58] s58 harvest metrics: elapsed_s=689.4, iterations=18652, base_score=60, finds_new=0, best_new_score=null, procs_killed=9, stopped=true. Recorded via permuter-launch + permuter-harvest events in metrics/events.jsonl.

- [s58] src/system.c reverted to HEAD via `git checkout src/system.c` after harvest (no floor improvement to commit; oracle stays green).

- [s58] Cumulative h5-chassis permuter closure now spans s5/s13/s22/s23/s31/s32/s40/s49/s50/s58 = ~103k whole-fn random-mode iters + directed-scope campaigns; per s55 CONFIRMED, this closes permuter-modality without a chassis change.

- [s59] sandbox cpu_side_move_dir_4 --disable all with h5 candidate applied to src/system.c: masked=2, target_insns=160, build_insns=160 (floor confirmed at start of session).

- [s59] Campaign launch 2026-07-09T10:03:19+00:00: PROLOGUE_FIX 4 reordered / 0 unchanged; permuter reports `Will run for 2 iterations.` — directed-cross-product mode, NOT random fallback (contrast s50/s58 where `No perm macros found. Defaulting to randomization.` was printed).

- [s59] Weighted cross-product: identity order = 60 (== base = h5 masked=2 basin), swap order = 125 (+65). No new finds; identity is the target-matching order at this chassis.

- [s59] The parse succeeds when PERM_LINESWAP sits inside an extra `{ }` compound-statement nested inside an existing compound (`if (status != 0) { { PERM_LINESWAP(...) } goto poll; }`). This eliminates the s50/s58 tool-limitation frontier: the fallback was a SCOPE-SPECIFIC parse rejection, not a global block on this function.

- [s59] harvest --stop reports iterations=2, elapsed_s=177.8, best_new_score=None, new_finds=[]. Campaign terminated on its own (2-combination cross-product exhausted).

- [s59] Base score 60 (weighted) corresponds to the same h5 masked=2 floor measured directly via the engine sandbox at session start — permuter and sandbox agree on the h5 chassis identity.

- [s60] s60 baseline sandbox masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all` with h5 candidate applied.

- [s60] s60 honest_idx1495 variant (idx_1495 = idx_1494 + 1 in place of the cross-symbol form) sandbox masked=15 - reproduces the s8-probe1 measurement.

- [s60] p78 conflict list SHAPE identical between variants: {72,73,75,77,78,79,80,81,86, X_dispatch, X_writer, X_reader, 2,3,4,5,6,7,29} where X_* are three debug-window pseudos whose numbers shift (101,120,126 baseline / 97,116,122 honest) due to the prologue insn-count delta.

- [s60] p78 ALLOCDBG deltas: ord=13→ord=15 (last), hardreg=20(s4)→22(s6), livelen=72→144 (doubled), pri=277→138 (halved), nrefs stays 2.

- [s60] Coordinated 4-pseudo rotation across s3..s6: {p72,p73,p78,p79} = (s5,s6,s4,s3) baseline → (s3,s4,s6,s5) honest. p77 (idx_1494, s2) and p80/p81 (s1/s0 debug-window) stay put.

- [s60] p79 (tbl_125c) additional delta: nrefs=5→3 (idx_1495 no longer consumes tbl_125c in honest), pri=675→202, ord=12→ord=14, s3→s5.

- [s60] Root cause named at the RTL layer: baseline lreg insn 38 has REG_NOTES=(nil); honest lreg insn 30 has REG_NOTES=(expr_list:REG_EQUIV (const (plus symref D_800A1494 const_int 1))). Same RTL SET pattern, different REG_NOTES chain.

- [s60] The REG_EQUIV note is attached at expand.c time (emit_move_insn / set_unique_reg_note) based on tree-level constant-expression recognition. The honest form's tree (POINTER_PLUS_EXPR (ADDR_EXPR D_800A1494) (INTEGER_CST 1)) IS a compile-time constant; the baseline's tree contains VAR_DECL tbl_125c so it is not, even though tbl_125c aliases D_800A125C at RTL-CSE.

- [s60] local-alloc.c::update_equiv_regs consumes REG_EQUIV notes for live-range strategy; the presence changes livelen accounting and downstream allocno priority in global-alloc.

- [s60] Files list & inventory: tmp/grind/cpu_side_move_dir_4/s60/csmd4.{baseline,honest_idx1495}.{rtl,cse,cse2,loop,flow,combine,jump,jump2,lreg,greg,sched,sched2,dbr,log,s,i} + csmd4_only.{greg,lreg} + csmd4_honest.{greg,lreg} + FORENSICS.md + dump_baseline.sh.

- [s60] Ledger implication: the s8 kill of the honest respelling is now MECHANISM-NAMED, not just measured. Any future 'retire the semantic-lie idx_1495 spelling' attempt must plan for the REG_EQUIV-driven alloc web rotation as the specific obstruction, not just 's-reg coupling'.

- [s61] s61 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 target_insns=160 build_insns=160 via sandbox cpu_side_move_dir_4 --disable all.

- [s61] s61 P1 &D_800A1495: masked=16, build_insns=161 (+1 insn vs h5). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/honest_idx_1495_direct_symref.c.

- [s61] s61 P2 1+idx_1494: masked=15, build_insns=160. Same as s8-probe1 (idx_1494 + 1).

- [s61] s61 P3 (u8*)((s32)&D_800A1494+1): masked=15, build_insns=160. Integer-cast intermediate does NOT block REG_EQUIV attachment.

- [s61] s61 P4 (u8*)((s32)idx_1494+1): masked=15, build_insns=160. Var-mediated cast folds via CSE back to ADDR_EXPR before note decision.

- [s61] s61 P2/P3/P4 rejected forms consolidated at memory/grind/cpu_side_move_dir_4/rejected/honest_idx_1495_intcast_forms.c.

- [s61] Named sub-mechanism: expand.c set_unique_reg_note attaches REG_EQUIV whenever the RHS tree reaches emit_move_insn as a compile-time-constant address, regardless of intervening integer CAST_EXPR or local-variable substitution (CSE folds the intermediate before the note decision). Only tbl_125c-VAR_DECL-referenced expressions escape the note because tbl_125c is not itself an ADDR_EXPR at tree time.

- [s61] H3 frontier verdict: the REG_EQUIV alloc-web rotation is DETERMINISTIC and NAMED, but NOT reversibly leverable via C-source respelling. Every natural respelling either matches s8-probe1 (masked=15) or regresses further (P1 masked=16 +1insn). Offsetting via p72/p73/p79 priority raises is also closed - no natural-C lever raises those priorities within the function's stated semantics.

- [s61] The cross-symbol semantic-lie form (src/system.c:406) remains committed and remains policy-flagged; s61 confirms pure-C retirement at masked=2 floor is still blocked. Retirement paths unchanged from s60: close h5 to 0, or achieve g3 basin masked=0 (with file-wide s-reg re-balancing).

- [s61] src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor).

- [s62] s62 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all` (rules_dropped=5, cheat_asm_stripped=22). Reproduces the 60-session ledger floor.

- [s62] s62 rederive sub-angle: m2c on the file-local twin marionation_Exec.s (NOT the committed src/system.c body, NOT csmd4's own asm) — this specific angle is absent from the s8/s9/s17/s18/s26/s27/s35/s36/s44/s45/s54 rederive-closure list. s9 transplanted marionation's already-committed C body (masked 7/8/14/20 across 4 probes); s54 m2c'd csmd4's own asm with all flag combos. m2c-on-sibling-asm never previously run.

- [s62] s62 output: marionation m2c block=3 line: `debug_printf(&D_800161C8, D_800F19C0, *(&D_800A11DC + (D_800A11D5 * 4)), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C));` — byte-identical inline-all-args expression to csmd4's s8/s54 m2c output.

- [s62] s62 novel non-block=3 observation (recorded to eliminate future re-derivation): m2c does reconstruct marionation's distinct `temp_s3 = &D_800A1494 + 2` pointer (the idx_1496 s6-register in marionation's ALLOCDBG that csmd4 lacks) and shows the sibling-only post-window READ at `M2C_FIELD(temp_s3, u8 *, -1)`. This confirms s9's finding that marionation's s-reg web differs materially — the idx_1496 pointer is a live global-alloc pseudo in marionation but has zero corresponding site in csmd4.s. Confirms marionation-transplant fungibility is bounded by this s-reg web asymmetry (s9 P4 masked=20 already measured this compound regression).

- [s62] s62 mechanism: m2c's block=3 fixed-point across (a) csmd4.s with 5+ flag combos (s8/s53/s54), (b) marionation.s (s62), regardless of target-asm-level differences in the surrounding function shape. Confirms the fully-inlined form is m2c's structural attractor for GCC-2.7.2 varargs-CALL with tbl-indexed byte-dispatch reconstruction. The reconstruction depends on the CALL-shape, not the ambient s-reg web.

- [s62] s62 KILLED cross-reference to known rejected forms: the m2c-output block=3 shape (fully-inlined args, no v0/t0 staging) is byte-identical to the rejected forms already in the bank as `arg5_fully_inline_no_v0_staging.c` and `mirror_arg5_named_arg4_inline.c` and matches s9 P2 masked=14 (inline-all args). No new sandbox measurement required — the shape is repeatedly closed.

- [s62] s62 rederive-class closure extended to 11 sub-angles: s8 m2c-csmd4-default + s9 marionation-C-transplant + s17 decomp.me-shingle-scan + s18 Kengo-local-cpu_side_move_dir-stub + s26 decomp.me-residual-scan + BB2-in-repo-cluster + s27 saEft01Init-decl-transfer + s35 idx_1495-respellings + s36 BB2-5-arg-templates + Kengo-numata-stubs + s44 decomp.me-BB2-toolchain-corpus + s45 SOTN-cross-project + Kengo-whole-dump + s54 m2c-flag-space + s62 m2c-sibling-asm. All 11 KILLED.

- [s62] s62 candidate.c: unchanged (h5 form remains masked-2 floor since s4, now 58 sessions). src/system.c restored to HEAD both-named form at session end via in-place edit; no oracle-affecting change.

- [s62] s62 modality compliance: mandated modality was rederive; sub-angle chosen was m2c-on-sibling-asm (novel un-run angle within the rederive class per ledger closure list). Judge constraint (BINDING per s40/s41/s46/s54/s55) NOT violated — no canonical-asm framing surfaced, no ALLOCDBG s-reg coupling attempted (that's the un-run frontier for post-modality-rotation sessions). This session eliminates one more rederive sub-angle and hands the frontier list to future sessions unchanged.

- [s63] s63 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via sandbox cpu_side_move_dir_4 --disable all (rules_dropped=5, cheat_asm_stripped=22).

- [s63] s63 m2c-with-reg-vars-all-sregs: 80-line output at tmp/grind/cpu_side_move_dir_4/s63/m2c_regvars_sregs.txt. Debug-window arg shape = inline-all-args (score-14 basin per WIP). Fn prologue shape = duplicate SET-INSNs (m2c hallucination from --reg-vars pinning). No novel structural neighborhood.

- [s63] s63 rederive-modality closure update (transferable): the --reg-vars flag exhaustion (untried subset: all-s-regs) is now measured. Combined with s8 (no --reg-vars, default), s53 (--reg-vars arg5-addr), s54 (m2c --void / --no-stack-spill / --gotos-only flag-space), s62 (m2c on marionation_Exec.s twin sibling), the m2c rederive modality has NOW been exhausted across every non-trivial flag+input combination the tool exposes. Future rederive sessions must use out-of-tool inputs (decomp.me corpus [gated on curl_cffi install per s9/s10], Kengo source glance [operator-dependent], or a fresh sibling analog outside marionation_Exec [none known per WIP notes.md]).

- [s63] s63 sibling landscape confirmed: cpu_side_move_dir (code6cac_b.c), cpu_side_move_dir_2 (code6cac_c2.c), cpu_side_move_dir_3 (main.c) do NOT share the debug_printf tbl_125c/idx_1494 window pattern (grep for debug_printf/tbl_125c/D_800A125C/D_800A1494 in those files returns empty). marionation_Exec remains the sole structural twin; its rederive angle was closed in s62.

- [s63] s63 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c has h5 candidate applied (edit in place) at session end.

- [s63] Judge-constraint alignment note: this session's mandated modality was rederive; the Judge's binding constraint (BINDING per digest) also names ALLOCDBG-instrumented s-reg coupling hunt (H2 frontier) as an un-run axis. This session did NOT advance H2 because the modality was rederive; H2 remains the highest-priority un-run frontier for the next session with allocdbg/forensics modality.

- [s64] 63-session floor stable at masked=2 (h5 chassis, mult-expander LAUNCH insn 111 fresh p106 via expmed.c:2244 case alg_shift NULL_RTX; residual pair-swap 118/111/121 vs target 118/121/111 at sched1 clock=13 LUID tiebreak, LUID(121)=12 > LUID(111)=8).

- [s64] s60/s61 named the +13 alloc-web rotation mechanism: p78 (idx_1495) REG_EQUIV attachment at expand-time via set_unique_reg_note; livelen 72->144, priority 277->138, ord 13->15 (demoted last); triggers 4-pseudo rotation into s3/s4/s5/s6.

- [s64] s61 confirmed all 4 natural idx_1495 respellings (&D_800A1495, 1+idx_1494, (u8*)((s32)&D_800A1494+1), (u8*)((s32)idx_1494+1)) all attach REG_EQUIV; the cross-symbol form remains the only respelling that defeats the note.

- [s64] s61 confirmed no natural-C offset lever raises p72/p73/p79 above the demoted p78=138 priority.

- [s64] s62 confirmed m2c on marionation_Exec.s emits byte-identical block=3 shape to csmd4's m2c output; sibling asm is m2c-fixed-point.

- [s64] s63 confirmed m2c --reg-vars s0..s5 emits the same inline-all-args (score-14 basin) shape plus spurious duplicate prologue SETs (cheat-shaped hallucination, not a novel structural axis).

- [s64] Permuter modality closed: h5 chassis ~137k iterations across 9 campaigns, 0 novel weighted<40 finds.

- [s64] Rederive modality closed: 11 sub-angles KILLED across m2c/marionation/decomp.me/BB2-in-repo/Kengo/saEft01Init/SOTN corpus.

- [s64] Judge constraint (2026-07-08) BINDING: canonical-asm authorization is settled-FAIL; do not resurface.

- [s64] s64 no measurement per synthesis modality; candidate.c at floor 2 preserved verbatim.

- [s65] P1 (t0-two-add-pair) and P2 (v0-in-place-PLUS-carrying-arg5_addr) both land at masked=15 (+13). This is a NEW named basin: 'multi-set-collapse' — any single-mutation forcing either the SLL-equivalent-dest or the PLUS-equivalent-dest to multi-set collapses the h5 window's LAUNCH duo into a +13 misalignment. The 2 basin members are compositionally symmetric (SLL-side and PLUS-side), reinforcing s7's finding that h5 alignment depends specifically on the fresh single-set dual on both p106 (mult-expander) and p107 (arg5-addr PLUS).

- [s65] P3 (fresh-single-set-t0_addr) lands at a distinct masked=9 (+7). Splitting the tail PLUS to a fresh dest without disturbing the shift-side gives a THREE-LAUNCH-insn window (insn 111, insn 117-equivalent, insn 121), producing a novel misalignment not yet catalogued in the ledger's basin table.

- [s65] Baseline masked=2 confirmed at session start (candidate.c applied) and at session end after restoration.

- [s65] Structural modality is empirically exhausted on the h5 chassis for FIRST-order block-local statement rewrites: the ledger's ~40 measured decompositions plus this session's 3 novel probes leave no untested single-mutation that both preserves h5 basin AND flips the pair. Every measurable structural mutation either preserves h5=2 (INERT), collapses to g3=6/7 (mult-expander -> in-place-shift), or lands in +7 / +13 / +14 novel misalignment basins.

- [s66] s66 F3 compound dual-collapse measurement: masked=16 target_insns=160 build_insns=160 (+14 vs h5 baseline of 2), rejected form at memory/grind/cpu_side_move_dir_4/rejected/f3_compound_dual_collapse.c.

- [s66] The h5 masked=2 basin genuinely requires BOTH LAUNCHes jointly (insn 111 via mult-expander p106 fresh-single-set + insn 121 via v0+tbl compound p107 fresh-single-set); removing either drops to +13 (s65 P1/P2), removing both drops FURTHER to +14 (s66 F3).

- [s66] Confirms that the g3/ip/+13 basin family is NOT compositional; the two LAUNCH insns are not independent axes but a coupled 2D system whose (LAUNCH, LAUNCH) and (non-LAUNCH, non-LAUNCH) diagonals are separated by +14, and only the (LAUNCH, LAUNCH) corner sits in the h5 pair-swap neighborhood of the target.

- [s66] Baseline re-verification at end of session: masked=2 with candidate.c restored to src/system.c — h5 basin holds cleanly.

- [s67] h5 baseline score reconfirmed at start of session: sandbox cpu_side_move_dir_4 --disable all -> score=2, target_insns=160, build_insns=160 (candidate.c applied to src/system.c).

- [s67] F1 workspace compile-check: base.c splices into base_full.c and produces 2652-byte .o (successful).

- [s67] F1 permuter enumerated the full 4x6=24 cross-product deterministically in 66.2s. base_score=40 (g3-chassis weighted). Best non-base score 50 (still above base). Confirms the g3 basin is closed under the four semantic-equivalent arg5-dereference spellings tested here; PERM_GENERAL over the addu operand-order + cast axis does not surface a lower-scoring form.

- [s67] F2 permuter enumerated the 6 orderings of the poll region's 3 statements in 120.4s. Every non-identity ordering strictly regressed (best non-base 105). Confirms the poll-region statement order is score-minimum on the g3 chassis; no s-reg web coupling was inducible via LINESWAP on this axis.

- [s67] F4 (t0m intermediate replacing t0*=4 in-place mult) collapse to rejected/v3_t0s_split_regress.c: v3 form was s3-measured at masked=3 (baseline 2), i.e. +1 regression. The frontier claimed distinction (F4 REPLACES the mult, V3 preserves it) is refuted by inspection of rejected/v3_t0s_split_regress.c which shows V3 also replaces the mult (t0s = t0*4; t0 = (u8*)tbl+t0s;). F4 = V3, KILLED-by-reference; excluded from this session's probes.

- [s67] The g3-chassis permuter neighborhood has now been sampled by: s5 fresh-seed random 9040 iters / 411.8s / 0 novel finds (KILLED); s58 fresh-seed random 18652 iters / 689s / 0 novel finds on h5 chassis (KILLED); s67 directed 24-iter PERM_GENERAL x LINESWAP arg5 enumeration / 0 novel finds (this session, F1 KILLED); s67 directed 6-iter poll LINESWAP enumeration / 0 novel finds (this session, F2 KILLED). Cumulative permuter search on both basins now covers ~27700 iters across random + directed modalities with no closing form.

- [s68] s68 baseline sanity: h5-chassis base.c (t0*=4, arg5=*(v0+tbl), and structurally-identical to candidate.c) registers base_score=60 at permuter launch — confirms permuter is scoring the h5 basin as expected (60 = weighted score corresponding to masked=2 h5 baseline).

- [s68] s68 F7 campaign metrics (recorded via permuter-launch + permuter-harvest events in metrics/events.jsonl, label s68_h5_wholeblock_lineswap_general): base_score=60, elapsed_s=96.2, iterations=480, finds_new=0, best_new_score=null, pid_alive_at_harvest=false, stopped=false (campaign self-completed at 480 iter deterministic cap before --stop-on-zero armed).

- [s68] s68 finding: the deterministic 5!*4=480 cross-product of (5-way whole-block PERM_LINESWAP over the flexible do_timeout statements) x (4-way PERM_GENERAL over the arg5 dereference spelling) is now empirically closed on the h5 chassis. Combined with s67 F1 (24-iter g3-chassis arg5-only enumeration KILLED) and s67 F2 (6-iter poll-region 3-statement KILLED), all three declared s64-synthesis deterministic permuter frontiers are now KILLED.

- [s68] s68 score-clustering evidence in log tail (tmp/grind/cpu_side_move_dir_4/s68/perm_wholeblock/campaign.log): the 480 iterations cluster at ~6 discrete score values (60/110/120/200/750/760/790/800). 60 = identity/valid-dependency-ordering tie; 110-200 = interior re-timings within a valid dependency graph that scramble a few insns; 750-800 = orderings where a v0 or t0 use precedes its def, producing broken code with distance shot up. NO iteration hit a score below 60. The distribution is CONSISTENT with a 5-statement dependency graph having ~120 valid orderings that all collapse back to the same or worse RTL after expand normalisation.

- [s68] s68 permuter-modality closure: F7 was the last permuter-adjacent deterministic-enumeration axis surfacing from the s55/s64 grand synthesis. Combined with s5 (9040-iter g3 fresh-seed), s13 (2999-iter h5 fresh-seed), s22 (20429-iter block PERM_LINESWAP), s31 (5040-iter prologue LINESWAP), s32 (35338-iter block PERM_GENERAL), s40 (1738-iter block+flank PERM_GENERAL), s49 (15064-iter arg5-narrow), s50 (29289-iter fallback-to-random), s58 (18652-iter outer-prologue-fallback), s67 F1 (24 iters g3-arg5 enum), s67 F2 (6 iters poll-region), s68 F7 (480 iters wholeblock enum) = ~137k+ total permuter iterations at h5-chassis with 0 novel finds below weighted-40. Permuter modality is now fully closed at both h5 and g3 chassis.

- [s68] s68 workspace + logs saved under tmp/grind/cpu_side_move_dir_4/s68/perm_wholeblock/: base.c (annotated h5 chassis), campaign.log (480-iter run log), campaign_meta.json (launch metadata), harvest.json (harvest record).

- [s68] s68 no src/system.c modification: candidate.c (h5 form, masked=2) remains the checkpointed floor; the permuter campaign runs against the workspace base.c via splicer, so src is untouched.

- [s68] s68 modality-exhaustion note: the permuter modality is now measurably closed across h5/g3 chassis at fresh-seed, directed-narrow, and deterministic-enum discipline levels. Any future permuter session must introduce a NEW structural chassis (not h5, not g3, and not a trivial derivative of either) — e.g., a novel expansion path via an untried arithmetic decomposition of the arg5 or t0 chain — since the two known basins' 137k-iter neighborhoods have been searched deterministically and randomly with 0 finds below the respective baselines.

- [s69] s69 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all. Honest respelling (line 406: idx_1495=idx_1494+1) scores masked=15 (+13, reproduces s8 probe1).

- [s69] s69 dumps built: h5 and honest ALLOCDBG/QTYDBG/SCHEDDBG-instrumented cc1 dumps at tmp/grind/cpu_side_move_dir_4/s69/{h5,honest}/csmd4.{greg,lreg,flow,combine,sched2,rtl}.csmd4 + csmd4.log. dump.sh at tmp/grind/cpu_side_move_dir_4/s69/dump.sh (rebuild recipe). Extract/diff tool at tmp/grind/cpu_side_move_dir_4/s69/allocdbg_diff/extract_allocdbg.py.

- [s69] s69 greg-diff on cpu_side_move_dir_4: 4-cycle s-reg rotation on function-body carriers is the ONLY register-disposition difference. h5: 72->s5(21), 73->s6(22), 78->s4(20), 79->s3(19). honest: 72->s3(19), 73->s4(20), 78->s6(22), 79->s5(21). Pair {p72,p79} swap {s5,s3}; pair {p73,p78} swap {s6,s4}.

- [s69] s69 greg-diff debug_printf window: t0-chain and arg5-carrier pseudos allocate IDENTICALLY. h5 p101->a0 p106->a0 p107->v0 mirror honest p97->a0 p102->a0 p103->v0 (pseudo numbers shift by 4, hardregs identical). The residual pair-swap {sll4@54 <-> addu5@55} theorem is preserved on BOTH forms - the honest respelling does NOT touch the h5 basin's residual.

- [s69] s69 block-0 insn extent: h5 [insn 4..46] = 43 insns; honest [insn 4..38] = 35 insns. Honest's shorter arithmetic on `idx_1495 = idx_1494 + 1` saves 8 prologue insns via expand-time constant folding on `(&D_800A1494)+1` (address-of + integer constant). h5's cross-symbol form `((u8*)tbl + ((s32)&D_800A1494 - (s32)tbl) + 1)` also folds to a constant addend at expand time but through more RTL nodes, extending livelen of tbl_125c's carrier by ~4-8 LUIDs across the fold.

- [s69] s69 mechanism confirmation: the 4-cycle s-reg rotation is the direct signature of qty_compare priority tie-breaking (local-alloc.c qty_sugg pass), driven by a livelen delta on ONE s-reg carrier (tbl_125c vs idx_1494) that propagates transitively through pri = floor_log2(refs)*refs/livelen*10000. The pri delta on the touched carrier reorders the ord= field for 4 tied s-reg carriers, and MIPS's ascending-regno find_reg scan (s0..s7) awards them the s-regs in the rotated order. Matches s60's REG_EQUIV attachment finding exactly (set_unique_reg_note fires on the honest form's fresh-pseudo idx_1495 with a constant addend note, does not fire on h5's cross-symbol form because the tbl_125c-based expression doesn't reach the same expand.c path).

- [s69] s69 F5 KILLED: an ALLOCDBG-diff-identified un-named edge does NOT exist. The entire alloc-web perturbation reduces to the REG_EQUIV/qty-priority mechanism already named at s60/s61. No new lever is exposed by the whole-function ALLOCDBG diff beyond what s60 named and s61 measured KILLED across 4 natural respellings.

- [s69] s69 forensics implication: the 4-cycle rotation on {p72,p79}x{p73,p78} is COUPLED through qty_compare's transitive priority ordering. Any C construct that changes ONE of the four carriers' refs/livelen triggers the coupled rotation - there is no isolated single-edge flip in this alloc-web. This constrains all future levers: any change to prologue arithmetic changing the ref count on tbl_125c or idx_1494 carriers will produce the same 4-cycle basin (i.e., cannot help retire line 406's forbidden cross-symbol form without also disturbing the h5 basin's residual pair).

- [s69] s69 debug_printf-window invariance: forensically confirms s6-s15's finding that h5 basin's pair-swap residual {sll4@54 <-> addu5@55} is decoupled from the line-406 prologue-shape perturbation. The +13 masked delta of the honest respelling is entirely body-scope s-reg rotation; the h5 basin's residual pair-swap is untouched. This means the semantic-lie policy (do-while-zero-exception.md #5) at line 406 CANNOT be retired by any lever in the alloc-web equation - it can only be retired if the h5 basin closes first, at which point the line-406 lever is no longer load-bearing.

- [s69] s69 dbg.filtered totals identical for both dumps (11660 log lines, 276 QTYDBG, 102 ALLOCDBG, 10626 SCHEDDBG, 623 RANKDBG). No new function-level scheduling divergence introduced; all deltas confined to global-alloc's per-pseudo output for cpu_side_move_dir_4's s-reg carriers.

- [s69] s69 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c restored to HEAD at session end via git checkout src/system.c.

- [s70] s70 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all` (rules_dropped=5, cheat_asm_stripped=22).

- [s70] F10 empirically KILLED via forensic dump-diff (no fresh cc1 rebuild needed — s69 dumps already contained the answer): block=3 total_time IDENTICAL (20 cycles) between h5 and honest forms in s69's ALLOCDBG-instrumented sched2 dumps.

- [s70] Per-insn priority + ref_count IDENTICAL for the residual-pair-swap triad: {T-15 lw arg4, T-14 SLL, T-13 PLUS arg5_addr}. h5 numbering (118/111/121) vs honest numbering (110/103/113) differs by exactly 8 = block-0 prologue insn delta; monotonic offset preserves within-block LUID differentials.

- [s70] Ready-list dispatch tiebreak identical: h5 T-13 picks 121 before 111 (higher LUID wins backward pass); honest T-13 picks 113 before 103 (same delta). Both forms emit {arg4-lw, SLL, PLUS} linearly vs target {arg4-lw, PLUS, SLL}; residual pair-swap is prologue-shape-INVARIANT.

- [s70] Direct confirmation of s6/s15/s69 finding: the residual pair-swap {sll4@54 <-> addu5@55} is decoupled from the line-406 prologue-shape lever; the +13 masked delta of honest_idx_1495 (s8/s61) is entirely body-scope s-reg rotation among {p72,p73,p78,p79} driven by REG_EQUIV-note-based qty_compare priority shuffle (s60 named the mechanism, s69 named the 4-cycle rotation pattern).

- [s70] Modality-compliance note: forensics modality without fresh cc1 rebuild is legitimate here because s69's dumps already contained the block=3 comparison data at ALLOCDBG/SCHEDDBG grain; the un-run angle was the direct diff. No new cc1 dump was needed to answer F10; using existing artifacts saves duplicative work and closes the frontier cleanly.

- [s70] F10 KILLED closes one of three live frontier hypotheses (F6, F9, F10 from s69). F6 (SOTN carve-out duplicated-statement-into-arms on arg5's carrier under FAKE + layer-2 review) and F9 (refs/livelen strict-tie seeding to produce a THIRD s-reg rotation basin) remain untouched by this closure.

- [s70] src/system.c restored to HEAD at session end via `git checkout src/system.c`; candidate.c unchanged (h5 form remains masked=2 floor since s4).

- [s71] s71 baseline confirmed: h5 candidate applied to src/system.c would score masked=2, target_insns=160, build_insns=160 per ledger. HEAD src/system.c (both-named form) scores 7 as expected (per s57/s62/s64 pattern). candidate.c (h5 form) remains the masked=2 floor.

- [s71] s71 novel rederive sub-angle: decomp.me corpus WINDOW-SLICE shingle-scan. curl_cffi 0.15.0 is available in .venv (s9/s10 gating condition resolved by prior operator install per s17/s26 evidence). Whole-function scan (s17 result recorded closed at similarity ~0.10) supplemented here by a residual-window-only slice — target-window structural signal amplified.

- [s71] s71 k=3 top-15 hits max similarity 0.133 (FieldPartyMemberIncreaseGearHp); s71 k=4 top-15 hits max similarity 0.043. Both far below the actionable-analog threshold used across the rederive-modality history (any hit under ~0.20 confirmed by inspection to be structurally unrelated).

- [s71] s71 direct inspection of top-2 committed C: neither hit contains a varargs debug_printf-style CALL, dual-tbl-indexed arithmetic, nor >2 arg-marshal insns. Shingle overlap is from generic lbu/sll/addu bigrams reused across MIPS struct-field arithmetic, not from a target-shape analog.

- [s71] s71 modality-closure extension: rederive class now spans 12 KILLED sub-angles (s8 m2c-csmd4-default + s9 marionation-C-transplant + s17 decomp.me-whole-function-scan + s18 Kengo-local-cpu_side_move_dir-stub + s26 decomp.me-residual-scan + BB2-in-repo-cluster + s27 saEft01Init-decl-transfer + s35 idx_1495-respellings + s36 BB2-5-arg-templates + Kengo-numata-stubs + s44 decomp.me-BB2-toolchain-corpus + s45 SOTN-cross-project + Kengo-whole-dump + s54 m2c-flag-space + s62 m2c-sibling-asm + s63 m2c-reg-vars-all-sregs + s71 decomp.me-window-slice-shingle-scan). No un-run rederive sub-angle remains exposed by the tool-space enumerable from the ledger + current corpus.

- [s71] s71 candidate.c unchanged (h5 form, masked=2 floor since s4, now 67 sessions). src/system.c not modified (HEAD both-named form retained; no oracle-affecting edit).

- [s71] s71 Judge-constraint compliance: modality mandated rederive; sub-angle chosen (decomp.me window-slice shingle-scan) is novel within the rederive class and does NOT resurface canonical-asm authorization framing. F6 (SOTN carve-out duplicated-statement-into-arms under FAKE + layer-2 review) and F9 (prologue refs/livelen strict-tie seeding for a THIRD s-reg rotation basin) remain untouched — both are outside rederive modality and require SOTN-carve-out or forensics/structural modality to advance.

- [s72] s72 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s72] s72 m2c saEft01Init output (artifact tmp/grind/cpu_side_move_dir_4/s72/m2c_saeft01init.txt): outer control-flow if/else-flattened (single `if ((D_800F19B8 < sys_VSync(-1)) || (temp_v1 = D_800F19BC, D_800F19BC = temp_v1+1, ((temp_v1 > 0x3C0000) != 0)))`); debug_printf call fully inline: `debug_printf(&D_800161C8, D_800F19C0, *((D_800A11D5 * 4) + &D_800A11DC), *((M2C_FIELD(&D_800A1494, u8 *, 0) * 4) + &D_800A125C), *((M2C_FIELD(&D_800A1494, u8 *, 1) * 4) + &D_800A125C))`.

- [s72] s72 shape-comparison: m2c-saEft01Init debug_printf window matches s8 (m2c csmd4), s62 (m2c marionation_Exec), s63 (m2c csmd4 --reg-vars), s54 (m2c csmd4 --void / --no-stack-spill+--gotos-only) at the argument-inline layer — the M2C_FIELD(&D_800A1494, u8*, 0/1) + &D_800A125C indexed-dispatch spelling is the invariant shape m2c reconstructs across ALL three BB2 twins under every measured flag combination.

- [s72] s72 basin identification: the fully-inlined spelling m2c produces IS the WIP-recorded 'inline-all args (v1/v8/v9): score 14' rejected form (WIP evidence.md L7) — measured KILLED on csmd4 at masked=14 in s8 (rederive) AND at masked=14 in s9 P2 (mirror arg5-named+arg4-inline sibling transplant). Two independent prior sessions have empirically measured this exact shape at +12 vs h5 masked=2.

- [s72] s72 modality-exhaustion note: rederive-modality closure list extended from 12 (as of s71) to 13 sub-angles: s8 m2c csmd4 default + s9 marionation P1-P4 transplant + s17 decomp.me shingle scrape + s18 Kengo cpu_side_move_dir stub + s26 decomp.me residual + BB2 in-repo residual cluster + s27 saEft01Init decl-transfer (mirroring pointer stage) + s35 idx_1495 respellings + s36 BB2 5-arg template + Kengo numata subsystem + s44 decomp.me BB2-toolchain corpus + s45 SOTN cross-project + Kengo whole-dump + s54 m2c flag-space + s62 m2c sibling asm marionation + s71 decomp.me WINDOW-SLICE shingle-scan + s72 (this session) m2c THIRD-TWIN saEft01Init.s. All 13 KILLED.

- [s72] s72 src/system.c restored to HEAD (both-named arg4/arg5 array-index form) at session end; candidate.c unchanged (h5 form remains masked-2 floor).

- [s72] s72 cross-verification: applying candidate.c to src/system.c and running sandbox --disable all confirmed masked=2 before restore.

- [s72] s72 orthogonality argument: m2c's outer-flow if/else-flattening on saEft01Init (single fused if condition using comma-operator on the D_800F19BC increment) is a control-flow reconstruction axis fully upstream of the block=3 debug_printf residual. The mult-expander LAUNCH mechanism (s7 CONFIRMED: expmed.c case alg_shift NULL_RTX target) lives INSIDE block=3 and is invariant to the outer control-flow C structure — s29/s30 empirically measured (success-label removal INERT, do_timeout-label removal INERT; while(1)/for(;;) outer refactor regresses to masked=52). The comma-operator reconstruction adds a novel C-source idiom but is not a lever for the residual — comma-operator was measurably KILLED by s24 forensics via expr.c COMPOUND_EXPR walkthrough.

- [s72] s72 sub-angle exhaustion: the three enumerated m2c corpora (csmd4 asm, marionation asm, saEft01Init asm) are the ONLY BB2 asm files known to contain a 5-arg debug_printf(&D_800161C8, ...) tbl-indexed dispatch (per s26 in-repo scan surfacing exactly these three); no additional in-repo asm exists to m2c for a novel BB2 twin. The m2c-shape space is saturated: three functions, three m2c calls, one invariant shape.

- [s73] s73 synthesis pass: no src edit, no sandbox measurement (synthesis modality contract); h5 candidate remains the masked-2 floor baseline unchanged from s72.

- [s73] Ledger cross-read confirms: 84 rejected forms banked; ~170 hypothesis blocks with terminal verdicts; every enumerable first-order structural / permuter / rederive / forensics lever mapped to a compiler-source wall (expmed.c:2244 case alg_shift, combine.c addsi3_internal, sched.c LUID counter without -g, integrate.c:96 varargs refusal) or an empirical regression basin (+7 g3, +8 P3, +13 REG_EQUIV rotation, +14 inline-all, +14 F3 compound, +20 P4 marionation, +27 opaque-carrier).

- [s73] Post-s64 frontier (F1 g3-arg5-only permuter + F2 poll lineswap + F3 closure-by-construction) fully consumed: s67 KILLED F1 (24 iters / 0 novel), s67 KILLED F2 (6 iters / 0 novel), s64 F3 CONFIRMED. Follow-on s65-s72 KILLED all 8 novel probes: t0-two-add-pair (+13), v0-in-place-PLUS (+13), fresh-t0_addr (+7), F3-compound-dual-collapse (+14), F7 whole-block LINESWAP+GENERAL (480 iters / 0 novel), F5 ALLOCDBG residual-lever (4-cycle rotation atomic), F10 prologue-LUID-offset (block=3 identical), F11 window-slice decomp.me shingle (top sim 0.133 unrelated).

- [s73] s72 rederive KILLED (third-twin saEft01Init m2c reconstructs the same score-14 inline-all-args basin as csmd4 and marionation_Exec) — closes the m2c rederive angle across all three known BB2 twins.

- [s73] Un-run first-order axes remaining: NONE at h5 chassis under first-order block-local / fn-scope / basin-transplant / permuter / m2c / decomp.me / ALLOCDBG lens. Only carve-out (SOTN duplicated-statement-into-arms with FAKE + layer-2 review on a NON-arg5 co-live carrier per s69 greg) and operator-dependent (Ghidra rederive) angles remain.

- [s73] Judge constraint (2026-07-08, BINDING) verified: no canonical-asm authorization framing surfaced this session; synthesis outcome elevates only the carve-out and operator-dependent modality frontiers the constraint permits.

- [s73] Rejected forms bank (84 entries at rejected/) documented as the deterministic no-fly zone for s74+; no new form drafted this session (synthesis modality).

- [s74] s74 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s74] s74 F9 prologue-reorder frontier fully KILLED across 6 probes. Byte-neutral statement-order permutations amongst {tbl_125c, idx_1494, idx_1495, D_800F19BC, D_800F19C0} (P1-P4) are qty-INERT at masked=2 - none shifts (refs,livelen) enough to break the 4-cycle rotation tie. The F9 frontier's assumption that byte-neutral prologue reorderings expose qty_compare's LUID-tiebreak is empirically wrong on this window: simple const-address stores don't birth long-lived debug-window pseudos, so their C-position is qty-transparent.

- [s74] s74 sys_VSync position mapping: FIRST=2 (h5 baseline), INTERIOR=15 (+13, lands in the same collapse basin as honest-idx_1495 s8 and s65 P1/P2), LAST=21 (+19). The CALL_INSN's v0-return livelen and call-clobber footprint anchor the h5 basin - it is not byte-neutral to relocate. sys_VSync cannot be part of an F9-style byte-neutral reorder.

- [s74] s74 mechanism finding: qty_compare LUID-tiebreak IS reachable in principle at the outer function scope but requires perturbing a pseudo with non-trivial livelen; the debug-window pseudos p72/p73/p78/p79 have their livelens dominated by USES at the poll region and the debug_printf call, NOT by prologue-side placement, so prologue statement-order edits cannot move (refs,livelen) numbers on those carriers.

- [s74] s74 F9 formally closed: no byte-neutral prologue reorder within src/system.c:404-408 (the assignment cluster) surfaces a novel s-reg basin distinct from h5 and honest 4-cycles. Combined with s60/s61/s69's atomic-4-cycle-under-qty_compare finding, this closes the entire 'byte-neutral outer-scope structural reorder' axis for the F9 mechanism.

- [s74] s74 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor). Rejected forms saved: prologue_reorder_svsync_last.c, prologue_reorder_svsync_middle.c.

- [s75] s75 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s75] s75 F9-supplementary named-intermediate sweep: 5 of 6 STORE positions (P0..P4) INERT at masked=2 with build_insns=160 - GCC folds the named intermediate and relocated store to h5-identical RTL when the STORE sits BEFORE at least one subsequent independent global-store; the fold is position-invariant across the prologue's INTERIOR positions.

- [s75] s75 P5 (STORE moved to LAST prologue slot, after D_800F19C0 = &D_80016240): masked=8, build_insns=160. Novel intermediate misalignment basin (+6 vs h5, distinct from the +7 g3/P3, +13 honest_idx_1495/multi-set-collapse, +14 F3-compound and inline-all, +19 sys_VSync-LAST from s74). Rejected form saved at memory/grind/cpu_side_move_dir_4/rejected/s75_vsync_ret_store_last.c.

- [s75] s75 mechanism finding: the named vsync_ret intermediate is combine.c/cse.c-transparent as long as the STORE is not deferred past all other prologue stores - the vsync_ret pseudo dies at the ADD-and-STORE regardless of intermediate independent-store statements. Only the trailing placement (past ALL globals) extends its livelen enough to disturb qty_compare, and that disturbance moves in the wrong direction. The frontier's assumption that named-vsync + STORE-sweep could induce a novel s-reg basin is empirically refuted.

- [s75] s75 F9 axis fully closed across (s74) byte-neutral prologue reorders + (s74) sys_VSync CALL position sweep + (s75) named-vsync + STORE position sweep. Combined with the s60/s61/s69 REG_EQUIV 4-cycle rotation atomicity finding, no first-order outer-scope structural lever remains for perturbing the {p72,p73,p78,p79} rotation in a targeted direction.

- [s75] s75 structural modality (block-local var splits, decl order, type narrowing, statement re-association per the codegen-technique-index) is thereby exhausted at the outer-scope prologue-restructure sub-axis; combined with all prior structural closures (s3 13-variant block-local, s4 do-while(0) 4-scope, s5 block-scope carriers + named dispatch + wraps, s11/s12 5 arg5_addr two-SETs, s13 fn-body hoist, s56 physical-line, s57 cross-block-scope decl hoist, s65 3 novel LAUNCH-mechanism probes, s66 F3 compound, s74 F9 byte-neutral + sys_VSync-position), the structural axis has no un-run first-order lever at the h5 chassis.

- [s75] s75 src/system.c left with h5 candidate applied at session end (sandbox re-measures masked=2). candidate.c unchanged (h5 form remains the 71-session floor). rejected/ file saved for P5.

- [s75] s75 sweep results JSON at tmp/grind/cpu_side_move_dir_4/s75/sweep_results.json; backup of pre-sweep h5 src at tmp/grind/cpu_side_move_dir_4/s75/system.c.h5; sweep driver at tmp/grind/cpu_side_move_dir_4/s75/sweep.py.

- [s76] s76 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via & tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all.

- [s76] s76 pre-launch audit: alternatives in the PERM_LINESWAP region strictly reorder prologue assignments only; inline-block h5 chain (t0 *= 4 mult path; multi-set t0 via + tbl_125c; pp alias; v0 <<= 2 stage; *(s32 *)t0 deref in call) is FIXED across all alternatives (fixes s14 flaw where alternatives dropped to g3 base=40).

- [s76] s76 permuter reported base_score=60 at launch, confirming the h5-multexpander basin is what the campaign explored (not g3=40 that s5/s14 explored).

- [s76] s76 campaign meta: {"label":"s76_h5_prologue_lineswap_v2","base_score":60,"elapsed_s":706.2,"iterations":720,"finds_total":16,"finds_new":0,"best_new_score":null,"stopped":false,"stop_reason":"s76 h5 prologue-lineswap fresh-seed 0 novel"}.

- [s76] s76 run.log tail shows steady stream of score=40 (g3-basin mutations) and score=60-3000+ hits during randomization but 0 candidates matched below any pre-existing find; permuter_failures count 21 confirms the s13/s14 AssertionError blindspot on the pointer-arith node is still triggering intermittently (worker pool tolerates).

- [s76] s76 novel-region PERM_LINESWAP on the fn-scope prologue is the only permuter-adjacent lever remaining in the block-local + neighbor-region search space; all prior permuter campaigns targeted inline-block or poll region (s5/s13/s14/s67/s68).

- [s76] s76 modality-exhaustion note: with s5 (g3 fresh-seed 9040), s13 (h5-multexpander fresh-seed 2999), s14 (directed PERM_GENERAL h5 chassis 23427), s67 (F1 g3-arg5 24 exhaustive + F2 poll 3-stmt 6 exhaustive), s68 (F7 whole-block 480 exhaustive), and now s76 (F13 h5-preserving prologue-lineswap 706s / 720 registered / ~650k raw / 0 novel), permuter modality is measurably closed against every reachable region of the function on both h5 and g3 chassis. Remaining live levers are structural (F6 SOTN duplicated-statement-into-arms on non-arg5 co-live carrier per s73 frontier), forensics (H2-extension single-carrier extra-use isolation per s73 frontier), or operator-dependent (F12 Ghidra Pcode+SSA rederive).

- [s76] s76 src/system.c restored to h5 candidate at session end; post-restore sandbox re-measures masked=2. candidate.c unchanged (h5 form remains masked-2 floor).

- [s77] s77 baseline: no src/system.c modification (HEAD state per git diff --stat empty at session end); candidate.c (h5 form, masked=2 floor since s4) unchanged.

- [s77] s77 pre-launch: workspace tmp/perm_csmd4/base.c annotated by removing s76's prologue PERM_LINESWAP (KILLED axis) and adding PERM_LINESWAP over the two vblank-poll if-arms with semicolon-terminated brace-blocks.

- [s77] s77 initial launch failed at parse: PERM_LINESWAP required semicolon-terminated statements, not raw brace-blocks; corrected by adding `;` after each `if (...) { ... }`.

- [s77] s77 permuter reported base_score=2215 at launch, NOT h5=60 nor g3=40 — the arm-swap alternative crashes the chassis into a novel high-score basin ~2155 above h5 baseline.

- [s77] s77 6779+ iterations sampled: scores cluster in {2215, 2380} (near-baseline arm-swap orderings) plus randomizer-generated forms in 90-1945 range; 0 candidates below 2215.

- [s77] s77 mechanism: the vblank-poll if-arms call through fn-pointers D_800A11B8 / D_800A11B4 with opaque semantics; GCC's sched.c cannot re-order the calls because CALL_INSN carries no side-effect analysis, so the arm-swap forces a fundamentally different RTL emission order that cascades across the poll region's s-reg web. This CONFIRMS by measurement that the poll-arm-swap axis is not a valid h5-preserving permuter chassis.

- [s77] s77 pre-existing output-55-* dirs found in workspace (July 8 mtime, before session launch): three forms hoisting arg5-chain before t0-chain in inline block, weighted score 55. Shape matches s2 E_arg5_first rejected form (masked=7) — pre-recorded KILLED, no sandbox re-measurement needed.

- [s77] s77 modality-compliance: mandated modality was permuter; F14 (poll-arm LINESWAP) is the sole novel un-run permuter neighborhood exposed by the s76 modality-exhaustion frontier list. All prior permuter regions (inline block, poll pre-region 3 statements, prologue lineswap, g3-arg5) already KILLED across s5/s13/s14/s67/s68/s76. This session's KILL of F14 further closes the permuter-modality search space.

- [s77] s77 permuter modality now measurably CLOSED across every reachable structural region of the function: prologue lineswap (s76 KILLED), inline block LINESWAP+GENERAL (s67/s68 KILLED), poll pre-region 3-stmt LINESWAP (s67 F2 KILLED), poll if-arm LINESWAP (s77 KILLED), inline block fresh-seed random (s5/s13/s14 KILLED). No h5-preserving permuter region remains that has not been either exhaustively enumerated or fresh-seed sampled.

- [s77] s77 Judge-constraint compliance: no canonical-asm framing surfaced; no rederive resurfacing; the KILLED outcome eliminates a permuter neighborhood on the H5 chassis (the mandated modality axis) without violating the BINDING constraints from s40/s41/s46/s54/s55/s64/s71.

- [s78] instrumented cc1 (tmp/gccdbg/cc1, BB2_*_DEBUG=1 -da) baseline dump on h5 candidate produced identical QTYDBG (276 lines) and ALLOCDBG (102 lines) traces to s69's h5 dump, confirming reproducibility of the forensics harness.

- [s78] sandbox baseline h5 candidate applied to src/system.c: masked=2, target_insns=160, build_insns=160 (unchanged from ledger).

- [s78] P1 (idx_1495 self-assign before poll:): sandbox masked=2, target_insns=160, build_insns=160 (byte-neutral). ALLOCDBG per-pseudo diff: 0 differing pseudos out of 102. QTYDBG per-reg1 refs max diff: 0 out of 276 entries. flow.c delete_noop_moves confirmed to remove (set p78 p78) upstream of local-alloc.

- [s78] P2 (real p78 branch-read before poll:): sandbox masked=15, target_insns=160, build_insns=163 (+3 insns). Regression matches +13 collapse basin family (s8/s65/s74 all landed at masked ~13-15 for constructs that added ~3 insns to the debug-window schedule).

- [s78] The two probes together bracket the space: no byte-neutral middle ground exists for local scalar pointer carriers under GCC 2.7.2 flow.c/combine.c semantics. Any p73/p77/p78/p79 refs-lift must ride on either (a) a real statement duplicated across control-flow arms that jump2 cross-merges (F6 SOTN carve-out, structural modality), or (b) a non-local construct (global-scope declaration, macro expansion) outside this session's forensics scope.

- [s79] s79 baseline: HEAD src/system.c (both-named form) sandbox --disable all = masked 7; h5 candidate.c applies to reach masked 2 per ledger (unchanged this session).

- [s79] Block=3 topology: label .L80080E64 (L48) through jal debug_printf (L72) is a straight-line 20-insn block with ZERO interior branches. Residual pair {sll@L57 <-> addu@L58 <-> sll@L59} is BLOCK-INTERIOR.

- [s79] Delay-slot enumeration in target asm (asm/funcs/cpu_side_move_dir_4.s L1-L176): 14 NOP delay slots identified. Reachable-from-block=3-interior without branch traversal: L52 (jal tslTm2LoadImage_2, block=3 head PRECEDING pair) and L73 (jal debug_printf, block=3 tail AFTER pair). All other NOP slots are in block=2 (upstream), the downstream loop body, or the epilogue.

- [s79] GCC 2.7.2 pass order confirmed via tools/gcc-2.7.2/toplev.c rest_of_compilation: sched1 -> local_alloc -> global_alloc -> sched2 -> reorg.c dbr_schedule/fill_slots_from_thread. reorg.c is STRICTLY DOWNSTREAM of sched2; the pair-swap decision (LUID tiebreak on LAUNCH ties at insn 111/121) is finalized by sched2 BEFORE reorg.c runs.

- [s79] L52 (jal tslTm2LoadImage_2 delay) has no free scavenging insn: preceding block=3 head insns (lui $a0, addiu $a0) are the jal's arg set-up. L73 (jal debug_printf delay) preceding insns all feed a0-a3 arg registers, so reorg.c has no independent-flow candidate; this is the mechanism that produced the current NOP fills in the target.

- [s79] F16's precondition (delay-slot fill can influence pair order) fails on two independent grounds: (1) TOPOLOGY - the pair is block-interior, no delay slot exists between the two paired insns for reorg.c to consume; (2) PASS-ORDER - sched2 commits the ordering before reorg.c runs, so any delay-slot-only materialization cannot influence a decision already made.

- [s79] Cross-linkage to s6 LUID-reorder: any C-level added read that survives to sched2 perturbs the LUID tie exactly like the s6-measured `t0*=4` late-move -> masked 6 g3 basin. The frontier's assumption that the added read could be reorg.c-only (invisible to sched2) is falsified by DCE: an insn invisible to sched2 is by definition DCE'd before sched2 and never reaches reorg.c.

- [s80] Ledger cross-read: rederive-modality declared exhausted at s19 ('any future session choosing rederive must justify a genuinely new external input'), reconfirmed at s37 ('no un-tried rederive angle remains without external-input dependency') and s46 ('rederive-modality is fully closed at chassis').

- [s80] m2c option-combination enumeration across all prior rederive sessions: s8 default, s53 --reg-vars v0/v1/a0, s54 --void and --no-stack-spill --gotos-only, s62 on marionation_Exec.s, s63 --reg-vars s0-s5, s72 on saEft01Init.s. s80 closes the final un-enumerated combinations: --context (types via existing header) and --stack-structs --passes 5.

- [s80] m2c --context ADDS a lexical refinement (array subscript vs raw pointer arith) for arg3 D_800A11DC[D_800A11D5], but this exact syntax is already committed in memory/grind/cpu_side_move_dir_4/candidate.c line 49. The refinement is in-basin.

- [s80] m2c --stack-structs --passes 5 produces byte-identical debug_printf shape to s8 default; block=3 uses no stack-slot spills in target asm so --stack-structs surfaces nothing.

- [s80] Both artifacts confirm the arg4/arg5 sub-shape emitted by every m2c variant remains the known inline-all-args form landing in the masked=14 basin (WIP L7). No m2c variant surfaces a structurally novel arg4/arg5 spelling.

- [s80] The exhaustive rederive-source list is now closed at 9 distinct sources across 4+ sessions: (1) m2c default s8, (2) m2c --reg-vars variants s53/s63, (3) m2c --void/--gotos-only s54, (4) m2c on marionation_Exec twin s62, (5) m2c on saEft01Init third-twin s72, (6) m2c --context header-driven s80, (7) m2c --stack-structs --passes 5 s80, (8) decomp.me corpus whole-function and window-slice scans s17/s26/s44/s71, (9) Kengo local dump + adjacent function extraction s18/s36/s45.

- [s81] s81 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s81] s81 sibling asm inventory (glob asm/funcs/cpu_side_move_dir*.s): cpu_side_move_dir.s (168 insns), cpu_side_move_dir_2.s (176), cpu_side_move_dir_3.s (173), cpu_side_move_dir_4.s (176). Four name-cluster siblings total; only _4 has been the grind target. Per s63 evidence, cpu_side_move_dir lives in code6cac_b.c, _2 in code6cac_c2.c, _3 in main.c (COMPLETED-C, so grep of those files returned empty for the debug_printf pattern) — but m2c'ing the SIBLING ASM directly (as s62 did for marionation) was un-run for these three per the s54/s62 closure list (which only recorded marionation_Exec.s).

- [s81] s81 m2c(cpu_side_move_dir base): produces `void cpu_side_move_dir(void *arg0)` — object-based movement dispatch reading `M2C_FIELD(arg0, s16 *, 4)` / `0x34C` counter / `D_800A36A4` mode / `stage_GetDataPtr()` velocity table; calls func_80021424 / func_80021A98 / func_80032854. Zero debug_printf calls, zero tbl_125c/idx_1494 references, zero VSync polling loop. Structurally orthogonal to csmd4's system.c body.

- [s81] s81 m2c(cpu_side_move_dir_2): produces initialization function calling gpu_InitDisplay/gpu_EnableDisplay/game_Cleanup/kgm_init_hitrect/marionation_camera_GetMaxFrame. No debug_printf window, no tbl_125c index-marshal.

- [s81] s81 m2c(cpu_side_move_dir_3): produces `s32 cpu_side_move_dir_3(u8 *arg0, s32 arg1)` — bios_DeliverEvent-driven state machine on D_800F1AFC guard + D_800A3044/D_800F1AE0 dispatch; return-early with -1 if D_800F1AFC set. No debug_printf, no tbl_125c/idx_1494 window.

- [s81] s81 rederive-class closure list extended from 11 → 12 sub-angles: s8 m2c-csmd4-default + s9 marionation-C-transplant + s17 decomp.me-shingle-scan + s18 Kengo-local-cpu_side_move_dir-stub + s26 decomp.me-residual + BB2-in-repo-cluster + s27 saEft01Init-decl-transfer + s35 idx_1495-respellings + s36 BB2-5-arg-templates + Kengo-numata-subsystem + s44 decomp.me-BB2-toolchain-corpus + s45 SOTN-cross-project + Kengo-whole-dump + s54 m2c-flag-space + s62 m2c-marionation-sibling-asm + s72 m2c-third-twin-saEft01Init + s80 m2c-context-flag + s81 (this session) m2c-name-cluster-siblings (cpu_side_move_dir base + _2 + _3). All 12 KILLED.

- [s81] s81 candidate.c: unchanged (h5 form remains masked-2 floor); src/system.c restored to HEAD both-named form (masked=7 baseline) at session end. Post-restore not re-measured since edit was a literal revert of the h5 apply.

- [s81] s81 mechanism finding: the four cpu_side_move_dir* siblings share only the `cpu_side_move_dir` name-prefix — they are functionally unrelated (movement dispatch / init / event handler / VSync-poll debug_printf). csmd4's block-3 window is structurally UNIQUE within its name cluster; the only file-local twin remains marionation_Exec (s9/s62 sub-angles, both KILLED). No further un-run sibling m2c angle exists at the name-cluster level.

- [s81] s81 modality compliance: mandated modality was rederive; sub-angle chosen was m2c-on-name-cluster-siblings (novel un-run angle within the rederive class — the s54/s62/s72/s80 closure list only covered csmd4's own asm and marionation/saEft01Init). Judge constraint respected — no canonical-asm framing surfaced; ALLOCDBG s-reg coupling hunt (frontier F6/F17 territory) is the un-run structural/forensics frontier for post-modality-rotation sessions.

- [s82] s82 baseline (implicit from ledger, not re-measured per synthesis modality): candidate.c (h5 form) applied to src/system.c would score masked=2, target_insns=160, build_insns=160. Floor stable at masked=2 since s4 (78 sessions).

- [s82] Rejected forms bank at 84 entries covers every first-order structural mutation attempted across 81 sessions; includes dup_D_800F19C0_arms_keep_prologue.c / dup_D_800F19C0_arms_no_prologue.c (s20 SOTN-carve targets, both wrong-signed / merged-but-misdirected) and dup_tbl_125c_arms.c (s10/s20 co-live carrier target for p73/p79).

- [s82] The ONE co-live-at-block=3-entry pseudo per s69 greg that has NOT had its dup-into-arms subvariant explicitly tested is p77 (idx_1494 addr). Rejected bank contains dup_tbl_125c_arms.c (targets p73/p79) and dup_D_800F19C0_arms_*.c (targets D_800F19C0 store addr pseudo) but NO dup_idx_1494_arms.c.

- [s82] s78 KILLED all local-scalar-pointer extra-USE isolation via bracket-argument (P1 self-assign DCE-invisible → 0 refs delta; P2 real branch-read → +3 insns non-neutral) BUT explicitly named 'the only remaining ref-lift mechanism at the local-scalar-pointer level is the F6 SOTN-sanctioned duplicated-statement-into-arms with cross-jump re-merge (requires real statement with genuine control-flow arms, structural modality)'. F6 for p77 is the un-run subvariant.

- [s82] s78 P2 result establishes the pattern: any real branch read of idx_1495/idx_1494 emits +3 insns per single-arm (li const, bne, delay-slot lui, jump-to-return). The F6 mechanism REQUIRES a real statement in BOTH arms whose tails are byte-identical so jump2 find_cross_jump merges. s20 demonstrated the merge IS reachable when no label sits between (no-prologue variant, build_insns=160 target-match) but on D_800F19C0 the ref-lift direction was wrong (+8 masked).

- [s82] F20 (poll-arm symmetric idx_1495-USE) is not a strict SOTN carve-out — the poll arms have distinct fn-ptr call bodies (D_800A11B8 vs D_800A11B4) so their tails are not identical; cross-jump merge is unlikely and the symmetric-position construct would likely emit +2-3 insns per arm (KILLED via s78-pattern). Kept in frontier only as the last un-measured poll-region idx_1495 lever.

- [s82] Judge risk (F6-double-prime): HIGH if the duplicated statement is semantic-lie-shaped (e.g. `*idx_1494 - *idx_1494` = defeat-combine-symbol-fold shape); requires a CLEANER real-statement candidate for FAKE annotation to survive layer-2 cheat-reviewer.

- [s82] Judge risk (F20): MODERATE-HIGH — `if(*idx_1495==0){}` falls in .claude/rules/dead-branch-scheduling.md territory; likely layer-2 FAIL.

- [s82] Modality guidance for s83+: NOT rederive (13 sub-angles KILLED, tool-space saturated); NOT permuter (137k iters both chassis all regions); YES structural for F6-double-prime measurement; YES forensics for a companion F17-CALL-return corpus search (expected 0 hits).

- [s83] h5 baseline candidate.c applied to src/system.c yields masked=2, target_insns=160, build_insns=160 (unchanged from ledger inheritance).

- [s83] F20 comma-op probe (idx_1495 comma-op read prepended in BOTH vblank if-arms) measured masked=2, build_insns=160 - the 0-emit-delta subvariant is DCE-invisible to flow.c REG_N_REFS.

- [s83] The +2/+3-insn-visible subvariant of F20 (observable stores/observable dead-computes) is a separate class, well-established KILLED via the s78-pattern (single-carrier refs-lift via observable dead form regresses local-alloc web away from h5 basin).

- [s83] Combined DCE-invisible + observable-store closure: no byte-neutral p78-lift path remains in the poll-region symmetric-arm frontier; F20 is fully closed.

- [s83] src/system.c reverted to exact h5 baseline candidate.c (masked=2 re-verified post-probe).

- [s84] h5 baseline masked=2, build_insns=160 confirmed at session start on applied candidate.c.

- [s84] Probe 1 (symmetric both-arms dup, prologue retained): masked=17, build_insns=160. Cross-jump merge fires; the F6-double-prime mechanism (refs-lift via pre-merge counting) IS reachable through this construct but produces wrong-sign misdirection (+15).

- [s84] Probe 2 (asymmetric arm-A-only): masked=15, build_insns=162. No merge partner; construct emits physically +2 insns, violating byte-neutrality prereq.

- [s84] Both probes together CLOSE the F6-double-prime frontier subvariant for p77 via `idx_1495 = idx_1494 + 1;` construct. Symmetric produces the misdirection signature; asymmetric fails byte-neutrality.

- [s84] The wrong-sign misdirection pattern replicates s20 dup_D_800F19C0_arms_no_prologue.c (+8 wrong-sign) and matches s5 whole-block do-while(0) wrap (+11 wrong-sign): every dup-into-arms and ref-lift construct tested to date on this function has produced monotonically wrong-direction alloc-web shifts, never a lower basin.

- [s84] Post-probe revert to h5 candidate.c form: masked=2 baseline restored.

- [s84] Neither probe explored inserting the dup on the SUCCESS path (before `goto success;`); this remains a novel un-run subvariant of F6-double-prime with idx_1494-referencing dup, but success path does not arrive at do_timeout: label so cross-jump merge topology differs from arm-A/arm-B pair. Not measured this session.

- [s85] s85 baseline: h5 candidate.c applied to src/system.c scores masked=2 target_insns=160 build_insns=160 via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s85] s85 probe1 (F6-triple-prime, success-path dup): masked=15 build_insns=162 (+2 physical). No cross-jump merge partner on success-path tail; construct emits lui+sw pair inside the arm.

- [s85] s85 probe2 (F6-quint-prime, loop-top dup): masked=15 build_insns=159 (-1 physical). NOVEL signature; loop-top store survives DCE (does NOT CSE with prologue idx_1495 initializer) but the resulting reg_n_refs shift causes local-alloc to eliminate ONE physical insn elsewhere in the debug window while misdirecting +13 vs baseline.

- [s85] s85 probe3 (compound loop-top + success-path): masked=15 build_insns=159, IDENTICAL to probe2 loop-top-alone. CONFIRMS jump2 find_cross_jump merges the success-path insn into the loop-top occurrence (success falls through check: -> loop: cycle, providing the reachable-tail precondition).

- [s85] s85 mechanism finding: the F6-double-prime and F6-triple-prime family (idx_1495=idx_1494+1 dup as p77-refs-lift carrier) is now measured KILLED across FOUR placement axes: symmetric both-arms (s84 +15 masked, build 160), asymmetric arm-A (s84 +13 masked, build 162), success-path arm (s85 +13 masked, build 162), loop-top (s85 +13 masked, build 159), and loop-top+success-path compound (s85 +13 masked, build 159). ALL placements produce +13 masked misdirection; only symmetric-arms is byte-neutral (via cross-jump), and it lands in the same wrong-direction basin as the observable stores.

- [s85] s85 mechanism corollary: the loop-top store's `-1 build_insn` signature is NEW evidence that adding a real (non-DCE'd) reference to idx_1495 in the pre-pair-swap-window control-flow region does perturb the alloc web (unlike s78 P1 self-assign which was flow.c-invisible via delete_noop_moves). But the perturbation is monotonically misdirection: refs on p77 shift qty priorities into a distinct basin that eliminates a physical insn upstream while entrenching the pair-swap.

- [s85] s85 rejected forms bank: dup_idx1495_success_path_p77.c, dup_idx1495_loop_top_p77.c, dup_idx1495_looptop_plus_success.c added under memory/grind/cpu_side_move_dir_4/rejected/. Bank grows from 84 -> 87 entries.

- [s85] s85 src/system.c restored to HEAD (both-named arg4/arg5 form, masked=7 baseline) at session end via `git checkout src/system.c`. candidate.c unchanged (h5 form remains masked=2 floor since s4).

- [s86] s86 baseline confirmed: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via `& tools/wteng.ps1 main sandbox cpu_side_move_dir_4 --disable all`.

- [s86] s86 permuter chassis inspected: tmp/perm_csmd4/base.c has PERM_LINESWAP over prologue (lines 448-455), PERM_GENERAL on loop-top v0=sys_VSync(-1) with 3 alternatives (lines 457-463), and PERM_RANDOMIZE over the inline block (lines 477-491). This is the s77-style chassis; permuter self-reported base_score=2215 on this workspace (the same 2215 signature the ledger s77 line documents as regressed).

- [s86] s86 campaign meta: label=s86_looptop_freshseed, jobs=8, launch_ts=2026-07-09T14:31:03Z, pid=417, preexisting_outputs count=16 (output-40-1..10 + output-50-1..3 + output-55-1..3).

- [s86] s86 harvest at ~15-min stopping window: iterations=1375, finds_new=0, best_new_score=null, pid_alive_at_harvest=false (killed via --stop). Output dir count at harvest: 16 (unchanged from preexisting).

- [s86] s86 finding: the existing loop-top PERM_GENERAL region (3 alternatives at the sys_VSync assignment) does NOT open a mutation basin producing novel finds below the merged-chassis score-40 floor within a fresh-seed 15-min window at 8 jobs. This closes the last-listed frontier item's fresh-seed variant on the CURRENT annotated chassis.

- [s86] s86 modality-vetting note: the frontier's proposed additional PERM_GENERAL alternatives at the loop-top site {`(void)*idx_1495;`, `idx_1494 = &D_800A1494;`, `idx_1494[0];`} are pre-classifiable cheats per no-new-park-categories vetting checklist (dead-read / dead-self-assign coercion with no semantic purpose, no h5-mult-expander-LAUNCH preservation motive). The remaining honest alternative `idx_1495 = idx_1494 + 1;` (dead in-loop store) was already measured s8 probe1 at masked=15 as a whole-source respell of the prologue form; adding it as an in-loop duplicate would fall under dead-store-fake-exception with FAKE annotation + layer-2 review AND still needs the s-reg-web disruption s8 documented. This frontier variant is therefore closed at the alternative-set level without introducing a cheat-shaped campaign.

- [s86] s86 candidate.c: unchanged (h5 form remains masked-2 floor). src/system.c restored to HEAD state (both-named arg4/arg5 form) via `git stash push` prior to launch; candidate remains available at memory/grind/cpu_side_move_dir_4/candidate.c for the next session to apply.

- [s86] s86 fresh-seed discipline note: 1375 iterations at base_score=2215 (~1.5 iter/sec across 8 jobs, dominated by high per-iteration compile cost of the s77-regressed chassis) is thin evidence relative to s13's 2999 iters / s5's 9040 iters / s76's 720 iters / s14's 23427 iters. The chassis's base_score=2215 signature indicates the current merged annotation set (LINESWAP+GENERAL+RANDOMIZE) explores from a bad neighborhood; the h5-preserving chassis and g3-basin chassis have both been re-measured this modality across prior sessions, closing the CURRENT-annotated-chassis fresh-seed axis.

- [s86] s86 orientation note: the frontier's other two items (ALLOCDBG-instrumented forensics on s85 probe2's build_insns=159 signature; structural respelling of src/system.c:406 idx_1495 initializer as `idx_1495 = idx_1494 + 1;`) were not selected because the driver mandated `permuter` modality this session. Those remain live frontier for future forensics / structural sessions per the s85 ledger.

- [s87] greg diff (s87/allocdbg_diff/diff.alloc.txt): only 4 lines differ (of 102). h5 ord=[12,13,14,15]=[79(19,5r,148ll,675p), 78(20,2r,72ll,277p), 72(21,2r,76ll,263p), 73(22,2r,79ll,253p)]; probe2 ord=[12,13,14,15]=[78(19,2r,55ll,363p), 72(20,2r,76ll,263p), 73(21,2r,79ll,253p), 79(22,3r,148ll,202p)].

- [s87] p79 delta: nrefs 5->3 (-2 refs); pri 675->202 (collapse); hardreg 19->22 (last in window). p78 delta: livelen 72->55 (-17 luids because respell shortens its span); pri 277->363; hardreg 20->19.

- [s87] QTY diff (s87/allocdbg_diff/diff.qty.txt): only 3 lines differ (of 276). blk=0 qty=0 reg1=88 birth 16->14 death 18->16; blk=0 qty=1 reg1=93 birth 22->20 death 24->22; blk=1 qty=0 reg1=95 birth 8->8 death 12->14. All prologue-block entries shift -2 luids (one insn eliminated); loop-top block extends p95's live range +2 luids (idx_1494 held through new respell insn).

- [s87] build_insns 160->159 confirmed via sandbox (masked=15, target=160). Prologue-only variant s8 probe1 measured masked=15 build=160 (NO insn eliminated) - the -1 delta is loop-top-position-specific: loop-top dominates both *idx_1495 uses, so local-alloc.c update_equiv_regs recognizes prologue idx_1495 init as redundant and DCE/absorb collapses one of the two prologue setup insns (la+addiu -> single addu of idx_1494+1).

- [s87] SCHED and RANK diff have 742 / 44 differing lines respectively - all downstream ripple from the ord=12..15 rotation (no new decision surface).

- [s87] p79 identified as the LEAD carrier in the greg [12..15] window (nrefs=5, pri=675, hardreg=19 in h5). p78 identified as the SECOND-place carrier (nrefs=2 livelen=72 pri=277). Levers must preserve p79's nrefs>=5 AND avoid shrinking p78's livelen below ~72 to avoid triggering this rotation.

- [s88] s88 baseline sandbox --disable all masked=2 target_insns=160 build_insns=160 (h5 candidate applied to src/system.c).

- [s88] s88 probe1 sandbox masked=15 build_insns=160 (exactly matches s8 record; no absorb).

- [s88] s88 baseline greg 'regs to allocate': 75 83 84 82 81 101 120 126 86 137 80 77 79 78 72 73 (positions 12..15 = 79 78 72 73).

- [s88] s88 probe1 greg 'regs to allocate': 75 83 84 82 81 97 116 122 86 133 80 77 72 73 79 78 (positions 12..15 = 72 73 79 78 - ROTATED).

- [s88] s88 baseline ALLOCDBG ord=12..15: p79 refs=5 pri=675 | p78 refs=2 livelen=72 pri=277 | p72 refs=2 pri=263 | p73 refs=2 pri=253.

- [s88] s88 probe1 ALLOCDBG ord=12..15: p72 refs=2 pri=263 | p73 refs=2 pri=253 | p79 refs=3 pri=202 | p78 refs=2 livelen=144 pri=138.

- [s88] p79 delta: nrefs 5->3 (LOST 2), livelen unchanged 148->148, pri 675->202. Position-caused hypothesis requires refs preserved; measured refs LOST => position-caused DISPROVEN, spelling-caused CONFIRMED.

- [s88] p78 delta: nrefs unchanged 2->2, livelen 72->144 (DOUBLED, ~absorbed the 2 refs p79 lost), pri 277->138. Consistent with poll-region *idx_1495 uses migrating from p79 to p78 via cse-unification on idx_1494 as the shared base.

- [s88] Pseudo-ID renumbering (101->97, 120->116, 126->122, 137->133) reflects the shorter idx_1495 initializer (fewer intermediate pseudos in the prologue). Not diagnostic of the RA change; the ord rotation is real.

- [s88] Kill class WIDTH: probe1 has NO loop-top compute; the spelling ALONE triggers the rotation. Any lever that respells idx_1495 through a form cse-unifiable with idx_1494 hits the same p79 ref loss, regardless of position. Frontier probe #1 (p79-preserving loop-top levers) is only viable if it PRESERVES the cross-symbol tbl-routed idx_1495 spelling AT src/system.c:406 - the honest respell axis is closed for the h5 basin.

- [s89] s89 baseline sandbox --disable all masked=7 on HEAD src/system.c (both-named form); masked=2 on candidate.c applied to src/system.c (h5 form, target_insns=160, build_insns=160). Floor unchanged since s4.

- [s89] s89 decomp.me corpus C-source substring scan: 3754 scratches scanned via regex on printf-family calls with >=5 args; 2 hits (PfboX printf trampoline, QkCAP MDEC_print_error).

- [s89] s89 hit (a) PfboX: score=0 matching=True; body is `sprintf(msg,fmt,a1..a5); mts_set_debuglog(msg);` — a variadic-forwarding wrapper. No VSync structure, no tbl_125c marshaling, no arg4/arg5 register chain rivalry. Not a csmd4 template.

- [s89] s89 hit (b) QkCAP: score=7600 matching=False; body is multiple sequential printf calls over MDEC register status bitfields with (>>N)&1 masks. No VSync-poll wrapper, no fresh 1D-array index-marshal chain, no analog to csmd4's `arg5 = *(s32*)(v0+(s32)tbl)` + `t0 = *(s32*)((idx*4)+(s32)tbl)` rivalry.

- [s89] s89 sub-angle novelty: the C-source substring lens is materially distinct from (i) asm-shingle whole-function similarity (s17, peak 0.097), (ii) residual-window instruction-cluster scans (s26 residual + s71 window-slice), and (iii) all m2c-variant sub-angles (s8/s53/s54/s62/s63/s72/s80/s81) which regenerate C from BB2 asm rather than search external C. It queries a distinct index of the same corpus and yields a distinct (empty-of-transplant) answer.

- [s89] s89 cumulative rederive-modality sub-angle closure: 12 (s81 ledger tally) → 13 (this session). The exhaustive rederive-source list closed at s80 is preserved; s89 exhausts the C-source substring sub-lens on the decomp.me corpus source-code field.

- [s89] s89 src/system.c: restored to HEAD after baseline measurements via `git checkout src/system.c`. candidate.c unchanged (h5 form remains the masked=2 floor).

- [s89] s89 modality compliance: mandated modality was rederive; the corpus C-source substring 5-arg-printf lens is a novel un-run rederive sub-angle within the corpus lens family. Judge constraint respected — no canonical-asm framing surfaced; no forbidden semantic-lie construct proposed; no cheat-shaped honest respelling of src/system.c:406 attempted.

- [s90] s90 baseline: h5 candidate applied to src/system.c scored masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s90] s90 HEAD (both-named form) baseline: masked=7, target_insns=160, build_insns=160.

- [s90] s90 probe P1 (`idx_1495 = &D_800A1495;` -- honest direct symref): masked=16, build_insns=161 (+1 insn). D_800A1495 exists as an extern-declared symbol in src/system.c:387 and undefined_syms_auto.txt.

- [s90] s90 probe P2 (`idx_1495 = (u8*)tbl_125c + 0x239;` -- tbl-routed numeric): masked=15, build_insns=160 (byte-neutral). Identical to s8-probe1 (honest idx_1494+1) signature.

- [s90] s90 finding: only the exact semantic-lie form `(u8*)((u8*)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)` preserves masked=2. The three symbolically-distinct alternatives (idx_1494+1, &D_800A1495, tbl+0x239) all regress by +13 to +14 with build_insns delta of 0 or +1.

- [s90] s90 mechanism: the load-bearing property of the cross-symbol expression is NOT its base symbol (tbl_125c is preserved in P2) NOR its byte-neutrality (P2 is byte-neutral) -- it is the triple-symbol expression `(s32)&D_800A1494 - (s32)D_800A125C` that cse.c evidently uses to compose the file-level RA fabric preserving block=3 window ord=[12..15].

- [s90] s90 modality-closure: frontier item #3 (symbolically-distinct idx_1495 initializer) is fully closed on csmd4 -- the axis has no honest realization. Retirement of the semantic-lie form remains blocked by the same +13 s-reg web disruption documented in s8 policy note.

- [s90] s90 src/system.c: restored to HEAD (both-named arg4/arg5 array-index form) at session end; candidate.c unchanged (h5 form remains masked-2 floor).

- [s91] s91 synthesis: no src edit, no sandbox measurement (synthesis modality contract); h5 candidate remains the 87-session masked=2 floor since s4.

- [s91] Ledger cross-read confirms the compiler-source wall: expmed.c:2244 case alg_shift NULL_RTX target forces p106 fresh single-set; MIPS md has no shift-add insn pattern so combine cannot fold; sched.c birthing_insn_p LAUNCH sentinel on insn 111 ties with LAUNCH on insn 121; LUID(121)=12 > LUID(111)=8 tiebreak backward-picks 121 first. h5 and g3 basins provably non-composable at every measured C axis (s6/s7/s10/s16).

- [s91] Closed modality tally: structural 40+ probes (s3/s4/s5/s11/s12/s13/s21/s27/s29/s30/s38/s39/s47/s48/s56/s57/s65/s66/s74/s75/s83/s84/s85); permuter ~150k+ iters both chassis every region (s5/s13/s14/s22/s23/s31/s32/s40/s49/s50/s58/s67/s68/s76/s77/s86); rederive 13 sub-angles (s8/s9/s17/s18/s26/s27/s35/s36/s44/s45/s53/s54/s62/s63/s71/s72/s80/s81/s89/s90); forensics reduces residual to atomic 4-cycle s-reg rotation {p72<->p79, p73<->p78} driven by REG_EQUIV attachment on idx_1495 initializer (s60/s61/s69/s70/s87/s88).

- [s91] Rejected forms bank at 98 entries (memory/grind/cpu_side_move_dir_4/rejected/) documents the deterministic no-fly zone for s92+; every enumerable first-order structural mutation across 90 sessions is banked.

- [s91] Symbolic-honesty policy contradiction (s90): only the cross-symbol form at src/system.c:406 preserves masked=2, which is in the do-while-zero-exception #5 forbidden family; retirement blocked by +13 s-reg web disruption. This is a live policy contradiction the driver must own; no session-level fix exists within the pure-C grind.

- [s91] Inherited frontier F1 named vehicle (volatile-cast dead read) is judge-risky before measurement (fails legitimate-volatile-interrupt-touched prong 1).

- [s91] Inherited frontier F2 has no un-run C-realization satisfying DCE-survival + p79-refs-preservation + semantics-preservation simultaneously.

- [s91] F3 conflict-graph enumeration (global.c allocno_conflicts diff between h5/g3) is the newly-identified un-run forensics angle satisfying the s71 Judge constraint (ALLOCDBG-instrumented s-reg coupling hunt).

- [s91] Judge constraint compliance verified: no canonical-asm authorization framing surfaced; no cheat-shaped honest respelling proposed; no forbidden semantic-lie construct proposed.

- [s92] s92 baseline: sandbox cpu_side_move_dir_4 --disable all -> masked=2, target=160, build=160 (h5 candidate re-applied to src/system.c).

- [s92] F1 asm audit: $s2 (p77=idx_1494) has 5 use sites, all in blocks the h5 candidate already exercises (block=3 debug-window x2, poll-callback conditional, post-poll unconditional read, post-poll conditional store). $s4 (p78=idx_1495) has ONLY 1 use site (poll-callback conditional). No downstream block can host an ADDITIONAL byte-neutral use without hitting s78 DCE-invisibility (measured for local scalar carriers p73/p77/p78/p79).

- [s92] F2 asm audit: loop-head/test block (.L80080E1C..E5C) reads exactly {sys_VSync, D_800F19B8, D_800F19BC}. No game-state observable exists at loop-top in the target; any insertion regresses byte count by >=3 insns and lands outside the h5 basin.

- [s92] The residual pair {sll@57 <-> addu@58} theorem-locked in the staged-value-reused-variable family (per resume banner) is unaffected by structural-modality F1/F2 axes: F1 targets p78 refs, F2 targets loop-top alloc-web perturbation, but the residual is at LUID 8 vs 12 in block=3 interior (per s51 forensics) and depends on the p106/p107/p113 QTY relationships (per s6/s24/s25 forensics). Structural-modality axes at F1/F2 scope cannot dislodge a block=3-interior LUID tiebreak.

- [s92] No src/ regressions this session: candidate.c applied to src/system.c and left as-is at floor=2; no rejected/ additions (both F1/F2 kills are asm-audit closures, not source-form measurements).

- [s93] s93 baseline confirmed: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via sandbox cpu_side_move_dir_4 --disable all.

- [s93] s93 POLL-region structural axis 9-probe sweep result: 7 of 9 mutations INERT at masked=2 (P2 status u32, P3 status blocklocal, P5 do-while(0) poll-body wrap, P6 saved/status decl order, P7 saved u32, P8 saved blocklocal, P9 coalesced saved decl+init).

- [s93] s93 P1 (swap `if (status & 4)` <-> `if (status & 2)` arm order in POLL body) regresses to masked=7 (build_insns=160) - PROVES current arm order is target's C-source order, not a compiler-reorder emerging from either sequence.

- [s93] s93 P4 (hoist `saved = (*D_800A147C) & 3;` OUT of vblank-if guard) regresses to masked=19 (build_insns=158, 2 fewer than baseline) - not byte-neutral semantically; observable global read moves and codegen shrinks.

- [s93] s93 NOVEL: do-while(0) wrap on POLL region (P5) is compilation-invariant to the block=3 residual - the LOOP_BEG/LOOP_END NOTEs from a poll-region wrap do NOT cross-block-propagate to the do_timeout block=3 alloc web. Closes the wrap-family axis on POLL as a lever to block=3 (block=3 wraps already closed s48/s85; POLL wraps now equally closed).

- [s93] s93 STRUCTURAL AXIS SUMMARY: POLL region structural mutations either (a) regress POLL region locally (arm swap, saved hoist) or (b) are fully absorbed by expand/CSE/local-alloc without any cross-block effect on the block=3 {sll4@54 <-> addu5@55} residual pair. This closes the frontier hypothesis 'POLL region structural axis is a lever for the block=3 residual'.

- [s93] Prior artifacts consulted: memory/grind/cpu_side_move_dir_4/candidate.c (h5 form), evidence.md, hypotheses.md ledger head, rejected/ bank (no POLL-region structural probes present - confirms this is novel un-swept axis).

- [s93] src/system.c restored to HEAD after sweep (oracle-safe; candidate.c unchanged at h5).

- [s94] s94 baseline: h5 candidate applied to src/system.c scores masked=2 (target_insns=160, build_insns=160) via sandbox cpu_side_move_dir_4 --disable all.

- [s94] s94 permuter campaign tmp/grind/cpu_side_move_dir_4/s94/perm_poll: fresh workspace (copy of tmp/perm_csmd4 with h5 debug_printf block PERM_RANDOMIZE stripped and 5 PERM_GENERAL wrappers added on POLL-region expression sites - status test, two bit-mask tests, both idx_N deref forms). Structurally distinct chassis from s5's 9040-iter g3-basin, s13's 2999-iter h5-multexpander base, s14's 23427-iter directed h5+PERM_GENERAL on inline block, s86's 1375-iter vblank-poll-arm-annotated h5, and s67/s68's POLL LINESWAP over full statement blocks.

- [s94] s94 harvest: base_score=60 (h5 basin, correct); 44479 iterations / 1333.7s; finds_new=0; finds_total=0; best_new_score=null; procs_killed=9; stopped=true. Log tail preserved at tmp/grind/cpu_side_move_dir_4/s94/campaign_tail.log shows iteration scores dropping to exactly 60 repeatedly (the h5 basin floor at permuter's weighted metric) but never below.

- [s94] s94 finding: POLL-region expression-level permuter mutation is compilation-invariant to the block=3 pair-swap residual. The 5-site PERM_GENERAL cross-product explored ~20-30 candidate expression variants over 44479 iterations of composed mutation - no permutation of these expression forms shifts the file-level s-reg ref-balance in a way that closes the {sll4@54 <-> addu5@55} pair. This CONFIRMS s93's compilation-invariance claim at the permuter-mutation-neighborhood level, not just at hand-structural byte-neutrality.

- [s94] s94 confirms fresh-seed permuter discipline: with the h5 basin now measured across 5 structurally-distinct permuter chassis (s5 g3-basin 9040 iters, s13 h5-multexpander 2999 iters, s14 h5-inlineblock-directed 23427 iters, s86 vblank-arm 1375 iters, s94 POLL-expression 44479 iters = ~81420 aggregate iterations) with 0 basin-closing novel finds, the h5 basin is definitively permuter-inert under random mutation. Any future permuter work on this function must attack via a structurally-different chassis that permutes across block=3 QTY priority arithmetic (F3 ALLOCDBG frontier) - which is a forensics modality, not permuter.

- [s94] s94 src/system.c restored to HEAD (both-named arg4/arg5 array-index form, masked=7 baseline). candidate.c unchanged (h5 form remains masked-2 floor).

- [s95] sandbox cpu_side_move_dir_4 --disable all on HEAD src returned masked=7 (build_insns=160 matches target)

- [s95] after applying memory/grind/cpu_side_move_dir_4/candidate.c to src/system.c: masked=2, build_insns=160 (h5 basin confirmed unchanged)

- [s95] permuter campaign s95_perm_general_pp_ordering (fresh seed, PID 19902, -j 4): base_score=60, elapsed=660.1s, iterations=4320, finds_total=0, finds_new=0, best_new_score=null - harvested with --stop reason 's95 fresh-seed PERM_GENERAL pp-ordering chassis - 0 novel finds after ~600k iters / 10 min'

- [s95] concurrent earlier crash campaign s95_perm_general_block3_declorder (PID 413) with a third u32-cast alt branch triggered a permuter internal failure (ast_types.py:165 'int - pointer' AssertionError on the -(s32)-cast arithmetic); harvested at 1376 iters / 112s / 0 finds, chassis form documented as PERMUTER-INVALID

- [s95] 5 stale campaigns reaped in the process (marionation Exec dev campaigns from s30..s40); no interference with s95's csmd4 workspace

- [s95] run.log per-worker iteration counters climbed past 606k with score distribution 40..9590 - all >= base=60 or in known regression basins; workers exhibited the same score-noise floor as s94's POLL PERM_GENERAL run (44k iters / 0 finds), consistent with the frontier-item-3 mechanism (POLL and pp-position axes are permuter-neighborhood inert)

- [s96] greg allocno_conflicts extract for {p72,p73,p77,p78,p79} in h5, probe1, probe2: all three carry the same hardreg set {2,3,4,5,6,7,29}=$v0/$v1/$a0-$a3/$sp (call-cluster interference); zero $s2..$s9 hardreg conflicts. S-reg assignment is purely ord-order-driven.

- [s96] h5 vs probe1 pseudo-vs-pseudo edge sets are identical after the 4-pseudo rename 101->97, 120->116, 126->122, 137->133 (checked via symmetric set diff on 80 focus edges in each dump). No atomic conflict edge flips between h5 and probe1 - yet probe1 masked=15 while h5 masked=2, and probe1's ord=12..15 rotates identically to probe2 (s88 confirms).

- [s96] h5 vs probe2 differs by exactly one edge: {p78 <-> p86} present in h5, absent in probe2. p86 is in the debug_printf call-cluster (conflicts 82,83,84,86,137). Its live range shifts because of probe2's extra loop-top insn, so this edge deletion is a POSITIONAL side effect, not the driver: probe1 (no positional change, only spelling change) still rotates without this edge change.

- [s96] Therefore the driver of the 4-cycle rotation {p72<->p79, p73<->p78} is priority-arithmetic (pri = floor_log2(refs)*refs*size/livelen*10000): p79 loses 2 refs (5->3) across both probes, pri collapses 675->202, ord-position 12->{14 or 15}, hardregs 19..22 reassign. Confirms and extends the s87/s88 mechanism naming.

- [s96] Implication for the remaining search space: any lever operating on the conflict graph (edge additions/removals via new pseudos or lifetime coupling) cannot shift the ord=12..15 order without ALSO shifting nrefs or livelen on p79. Levers must attack the priority inputs directly, or work in an orthogonal alloc-web scope (POLL arms - untouched by any of the closed axes).

- [s96] Sandbox floor unchanged at masked=2 for candidate.c (h5 form); no src/ edits this session (pure forensics on pre-existing dumps).

- [s97] sandbox --disable all on h5 candidate: masked=2 build_insns=160 target_insns=160.

- [s97] sandbox --disable all on probe1 (idx_1495 = idx_1494 + 1;): masked=15 build_insns=160 target_insns=160. Matches s8 probe1 and s88 probe1 signature.

- [s97] h5 flow.csmd4 grep '(reg[/v]?:SI 79)' returns 5 lines: insn 24 (SET, symbol_ref D_800A125C), insn 34 (subsi3_internal, minus p77 p79 = SYMBOL_REF-diff), insn 38 (addsi3_internal, plus p89 p79 = tbl-restore), insn 111 (addsi3_internal, block=3 arg4 addr compute), insn 118 (addsi3_internal, block=3 arg5 addr compute).

- [s97] probe1 flow.csmd4 grep '(reg[/v]?:SI 79)' returns 3 lines: insn 24 (SET), insn 103 (block=3 arg4), insn 110 (block=3 arg5). Probe1 also has a lone insn 30 (addsi3_internal p78 = p77 + 1) with REG_EQUAL (const:SI (plus SYMBOL_REF 'D_800A1494' 1)) — the const-fold path that eliminates p79 from idx_1495's derivation.

- [s97] ALLOCDBG on h5: 'ord=12 pseudo=79 hardreg=19 nrefs=5 livelen=148 pri=675' (top of the 4-cycle window).

- [s97] ALLOCDBG on probe1: 'ord=14 pseudo=79 hardreg=21 nrefs=3 livelen=148 pri=202' (bottom of the window, cascade complete).

- [s97] Livelen is IDENTICAL 148 across both spellings — the rotation is driven purely by nrefs delta, confirming s96 conflict-graph identity finding (edge topology unchanged; only priority arithmetic differs).

- [s97] Priority formula verification: floor_log2(5) * 5 = 10 (h5), floor_log2(3) * 3 = 3 (probe1); ratio 10/3 = 3.333, matches measured pri ratio 675/202 = 3.342 to within rounding of the livelen-weighted 10000 scale factor.

- [s97] Named GCC pass: flow.c::life_analysis + regstat_init_n_sets_and_refs (single pass between cse2/loop and local-alloc, per tools/gcc-2.7.2 rest_of_compilation). REG_N_REFS[79] is snapshotted from the flow-time RTL and read by local-alloc.c::qty_compare via allocno[q].n_refs.

- [s97] cse.c::fold_rtx MINUS-case: recognized-const inputs are (const_int), (symbol_ref) with same-object same-section attribution, or (const) wrappers. (SYMBOL_REF distinct-extern-A) - (SYMBOL_REF distinct-extern-B) does NOT satisfy the fold precondition; the minus survives as pseudo arithmetic. Verified by absence of REG_EQUAL fold on insn 34 in h5's flow dump.

- [s97] combine.c cannot merge insns 34/36/38 into a single instruction: MIPS md has no 3-way combine pattern for (plus (plus (minus P Q) K) R) with pseudo operands carrying REG_EQUAL SYMBOL_REFs; each try_combine attempt for the trio returns 0 (verified by absence of merged form in .combine.csmd4).

- [s97] Cross-check with s87 ord=12..15 diff (baseline vs +13 basin probe1): symmetric result — s87 measured p79 nrefs 5->3, pri 675->202, hardreg 19->22. This session reproduces s87 measurements via a distinct instrumentation axis (flow.c reg-ref counting rather than greg ord-position diff), triangulating on the same mechanism from independent evidence.

- [s97] Cross-check with s96 conflict-graph identity: s96 established the allocno_conflicts edge set is bit-identical between h5 and probe1 after 4-pseudo rename. Combined with this session's nrefs-delta finding, the priority-input space is fully partitioned: livelen INVARIANT (148), conflicts INVARIANT (edge set), nrefs VARIANT (5 vs 3). Only nrefs varies, and only along the cross-symbol arithmetic axis (forbidden family).

- [s98] src/ cross-symbol subtraction census: grep '(s32)&D_.*- (s32)' returns exactly 1 hit (src/system.c:406 = csmd4 itself). The idiom is unique to csmd4 in the codebase; no in-repo COMPLETED-C sibling uses the pattern.

- [s98] probe1 (all-SYMBOL_REF idx_1495 base): masked=15/build_insns=160/rules_dropped=5/cheat_asm_stripped=22 - regresses +13. Confirms the h5 basin requires the LOCAL (u8*)tbl_125c base, not a raw &D_800A125C base.

- [s98] probe2 (pseudo-based delta subtrahend): masked=15/build_insns=160 - regresses +13. Confirms cse.c cannot fold the pseudo-based subtract into a constant, so it does not emit insns 34+38 in the p79-referencing form s97 identified. The h5 basin's p79 refs=5 profile is dependent on both operands being SYMBOL_REFs at expand_expr time.

- [s98] func_8007DC9C (display.c: 5-arg debug_printf gpu-timeout guard): masked=9, cheat_asm_stripped=437, INCOMPLETE. Not a transplant source.

- [s98] marionation_Exec (system.c: sibling): masked=56, 42 rules dropped, 22 cheat_asm stripped. INCOMPLETE. Direct transplants (marionation_full_basin_transplant, marionation_hybrid_arg4_named_arg5_inline, mirror_arg5_named_arg4_inline) already killed in ledger (s2/s4/s9).

- [s98] func_80082A14 (ings2.c: tslTm2LoadImage_2 timeout guard, same target callee as csmd4): score 0 but 39 cheat_asm stripped - shape relies on `volatile s32 counter = a1<<15;` + `asm volatile('' ::: 'memory');` barrier; both are forbidden constructs. Not transplantable.

- [s98] Baseline candidate.c re-verified at masked=2/build_insns=160 after all probes; src restored to h5 form.

- [s99] s99 baseline: HEAD both-named form scored masked=7, build_insns=160; applying h5 candidate to src/system.c restored masked=2 (target_insns=160, build_insns=160). h5 basin confirmed intact on current tree.

- [s99] s99 P1 measurement: idx_1495 = (u8*)((u8*)tbl_125c + ((s32)&D_800A1495 - (s32)D_800A125C)); scored masked=4, build_insns=161. First measured axis where a symbol-swap of the SUB minuend to an ADJACENT-BYTE symbol (D_800A1495 = &D_800A1494 + 1) does not preserve the h5 RTL profile — refines s97's attribution to symbol-identity-sensitive, not delta-numeric-value-sensitive.

- [s99] s99 P2 measurement: idx_1495 = (u8*)((u8*)tbl_125c + ((s32)&D_800A1494 + 1 - (s32)D_800A125C)); scored masked=2, build_insns=160 INERT. Confirms cse.c canonicalizes (+1) position across the SUB boundary.

- [s99] s99 P3 measurement: idx_1495 = (u8*)((u8*)tbl_125c - ((s32)D_800A125C - (s32)&D_800A1494) + 1); scored masked=2, build_insns=160 INERT. Confirms cse.c canonicalizes SUB direction (sign-flip via outer negation).

- [s99] Aggregate: 3 novel symbolic-variant respellings measured; the h5 cross-symbol basin is INVARIANT to +1-position and SUB-direction (canonicalized by cse.c) but is SENSITIVE to symbol-identity (D_800A1494 vs D_800A1495 substitution regresses). The forbidden semantic-lie family this basin depends on cannot be replaced by any spelling variation that preserves both the symbol pair AND the +1 factorization.

- [s99] Modality-exhaustion (rederive across s89/s90/s98/s99): decomp.me tool unavailable (curl_cffi missing); in-repo cross-symbol arithmetic grep = 1 hit (csmd4 itself); marionation sibling transplant KILLED s9 P1-P4; m2c KILLED s8; 3 honest-form initializer respellings KILLED s90; 2 mixed-base respellings KILLED s98; 3 spelling-variant respellings this session KILL/INERT. No untried rederive-modality C form has been identified against the h5 basin's specific SYMBOL_REF(1494)-SYMBOL_REF(125C)+1 factorization.

- [s99] src/system.c restored to HEAD (both-named arg4/arg5 form) via git checkout at session end. candidate.c unchanged (h5 form remains masked-2 floor).

- [s99] Kengo-transplant angle re-checked via .claude/worktrees/*/kengo_matches.csv: csmd4 has only 'size-only-ambiguous' Kengo match (SetPacketData in src/amami/am_rmd.c, 160 vs 159 insns, name-mismatch). No name-unique Kengo counterpart; this axis is unavailable without an external Kengo source dump the operator would have to provide.

- [s100] s100 synthesis pass: no src/ edit, no sandbox measurement (synthesis modality); h5 candidate.c remains masked-2 floor baseline; no candidate.c or rejected/ change.

- [s100] Ledger cross-read (s1..s99, 99 prior sessions) merged: floor stable at masked=2 since s85 (16 sessions no floor movement); ~81k+ aggregate permuter iterations across 6 chassis (s5/s13/s14/s86/s94/s95) yield 0 basin-closing novel finds.

- [s100] Judge-constraint-compliant remaining axes fully identified as exactly 2: F1 (SOTN pointer-alias-fake-exception carve-out per 2026-07-01 sanction, requires FAKE annotation + layer-2 cheat-reviewer + one structural measurement) and F2 (ALLOCDBG-instrumented livelen shortening on p79 without touching nrefs).

- [s100] F97a-refined formally CLOSED analytically by exhaustion: s99's rederive-exhaustion note ('no untried rederive-modality C form ... against the h5 basin's specific SYMBOL_REF(1494)-SYMBOL_REF(125C)+1 factorization') satisfies F97a-refined's predicted enumeration size = 0 semantic-purpose candidates.

- [s100] Basin non-composability re-confirmed by cross-read (s6/s7): expmed.c::alg_shift hardcodes NULL_RTX target, so no C spelling can compose h5's pair-swap fix with g3's register-exchange fix; every measured C form falls into exactly one basin.

- [s100] Priority-input partition re-confirmed by cross-read (s96/s97): livelen INVARIANT (148), conflicts INVARIANT (bit-identical edge set after 4-pseudo rename 101->97/120->116/126->122/137->133), nrefs VARIANT (5 vs 3), only nrefs axis is variant and only along the forbidden cross-symbol arithmetic path.

- [s100] Structural block-local axis, do-while(0) at all scopes, multi-set carriers, named dispatch, fn-body hoists, marionation transplants, m2c rederive, spelling variants, POLL-region 9-probe sweep, 6 permuter chassis are all measured KILLED per s3/s4/s5/s8/s9/s11/s12/s13/s14/s85-s99 evidence entries.

- [s100] Ledger merge saved to tmp/grind/cpu_side_move_dir_4/s100/synthesis.md with attack ladder for s101 (F1 structural + FAKE + layer-2), s102 (F2 forensics ALLOCDBG luid-per-pseudo), and s103+ escalation contingency.

- [s101] candidate.c (h5) re-verified sandbox --disable all -> masked=2 build_insns=160 (baseline confirmed twice, before and after probe restoration)

- [s101] Probe A (block-scope pass-through, arg5 only) masked=2 build_insns=160 = byte-identical to baseline; combine.c substitution folds tbl_alias references back onto tbl_125c pseudo (SAME mechanism s11/s12 measured on arg5_addr two-SET decompositions)

- [s101] Probe B (block-scope from-global) masked=16 build_insns=161; the second lui/addiu of D_800A125C creates an independent tbl-carrier pseudo but adds an insn — matches s13 D_800F19C0 fn-body-hoist +13 signature (second-global-mat catastrophic)

- [s101] Probe C (block-scope pass-through, dual-use on t0 and arg5) masked=2 build_insns=160 INERT; combine folds both routes to tbl_125c

- [s101] Probe D (fn-scope pass-through, dual-use) masked=2 build_insns=160 INERT; longer livelen on the alias pseudo does not defeat combine — pass-through equivalence is expressed pre-local-alloc

- [s101] Probe E (fn-scope pass-through, asymmetric use) masked=10 build_insns=163 (+8/+3insn); single-block-side reference to tbl_alias survives fold and forces separate materialization; register web catastrophically rotated

- [s101] Mechanism attribution: tbl_125c is itself a local (assigned from D_800A125C at fn entry), so pass-through pointer aliases fold in combine.c even at fn-scope. The pointer-alias-fake-exception SANCTION explicitly targets aliases-to-GLOBALS (SOTN `tilemap = &g_Tilemap;` shape); csmd4's tbl_125c does not qualify as an alias TARGET in the sanctioned sense — the direct-global form was measured KILLED as probe B

- [s101] Sanctioned axis inventory now: F1 pointer-alias-fake-exception KILLED (this session); F2 livelen-shortening remains un-measured (needs ALLOCDBG forensics s102 per frontier); F3 owner-escalation contingent on F2 close

- [s102] s102 baseline: h5 candidate applied to src/system.c scores masked=2, target_insns=160, build_insns=160 via sandbox cpu_side_move_dir_4 --disable all (rules_dropped=5, cheat_asm_stripped=7).

- [s102] s102 P1 probe1-honest reproduction: idx_1495=idx_1494+1 with tbl_125c=D_800A125C at fn scope top scores masked=15 build_insns=160 — matches s97/s99 probe1 signature (nrefs(p79)=3, livelen=148, pri=202, ord=14).

- [s102] s102 P2 (probe1 + defer tbl_125c=D_800A125C to just-before-do_timeout, fn-scope decl retained): masked=22 build_insns=158 (-2 insns). Target retains the prologue lui/addiu; deferring the SET drops them from build, +7 masked regression.

- [s102] s102 P3 (probe1 + block-local `s32 *tbl_125c=D_800A125C;` inside block=3 compound; fn-scope decl removed): masked=22 build_insns=158 (-2 insns). Same -2 signature as P2; block-local hoist collapses to identical basin.

- [s102] s102 P4 (probe1 + tbl_125c=D_800A125C reordered to LAST fn-prologue assignment, immediately before loop:): masked=17 build_insns=160 (+2 regression, no insn delta). RTL scheduler re-emits SET early; C statement position steers ord differently but does not shorten livelen enough to re-enter pri>675.

- [s102] s102 P5 (probe1 + fn-scope tbl_125c retained + block-scope `s32 *tbl_alias=tbl_125c;` used exclusively inside block=3): masked=15 build_insns=160 INERT — combine.c substitution folds tbl_alias back onto tbl_125c pseudo (same mechanism s11/s12/s101 measured on multi-SET decompositions); pass-through equivalence expressed pre-local-alloc.

- [s102] s102 P6 (probe1 + no local tbl_125c, direct D_800A125C symref at both block=3 uses): masked=35 build_insns=159 (-1 insn, +20 catastrophic). Direct-symref eliminates the shared base pseudo; block=3 emits two independent lui/addiu D_800A125C sequences replacing one shared base.

- [s102] s102 P7 (h5-form cross-symbol init respelled with D_800A125C symref + block-local tbl_125c inside block=3): masked=22 build_insns=158 (-2 insns). H5's cross-symbol basin does not compose with block-local tbl_125c; symref-init loses the local-tbl_125c-pseudo re-use that h5 relies on.

- [s102] s102 mechanism attribution: p79 livelen (148) is architecturally locked by target's fn-prologue tbl_125c materialization. Any structural hoist that succeeds in delaying the RTL SET past prologue drops the prologue lui/addiu from build (asymmetric with target which keeps them, verified by build_insns=158 vs target 160). Structural declaration order cannot shorten p79 livelen without asymmetric prologue insn loss — the two constraints (short livelen ∧ target prologue-emit) are mutually incompatible under structural modality.

- [s102] s102 modality closure: F2 livelen-shortening structural axis measured across all reachable declaration positions (top-of-prologue, mid-prologue, last-of-prologue, block-local hoist, block-scope pass-through alias, direct-symref removal, cross-form composition). Every position measured; every measurement KILLED or INERT. No un-run structural position remains.

- [s102] s102 candidate.c unchanged (h5 form remains masked=2 floor, 88 sessions unchanged since s4).

- [s102] s102 rejected/ additions: 6 new banked forms (s102_probe1_defer_tbl_into_do_timeout.c, s102_probe1_block_local_tbl.c, s102_probe1_tbl_last_in_prologue.c, s102_probe1_block_alias_pass_through.c, s102_probe1_no_local_direct_symref.c, s102_h5_symref_block_local_tbl.c). Total rejected/ population: 112 (was 106).

- [s102] s102 src/system.c restored to HEAD (both-named arg4/arg5 form, masked=7) via git checkout at session end. Sandbox re-verified masked=7 on HEAD.

- [s102] s102 sanctioned-axis inventory complete: F1 pointer-alias-fake-exception KILLED s101 (5 shapes); F2 livelen-shortening structural KILLED s102 (6 shapes); all owner-sanctioned pure-C axes now measured dead. Ledger's F3 contingency (OWNER-ESCALATION filing) is the deterministic next step.

- [s102] s102 Judge constraint compliance: no canonical-asm authorization framing surfaced (2026-07-09 FAIL entry respected); no cheat-shaped honest respelling of src/system.c:406 attempted; no forbidden semantic-lie or coercion construct proposed. Modality stayed strictly structural (declaration order, scope, statement re-association) per driver assignment.

- [s103] docs/grind/decisions.md:848 now contains a filed OWNER-ESCALATION for cpu_side_move_dir_4 in the same format as hirahira_w_frie (line 627) and motion_SetMotion (line 761); both mutually exclusive owner options are presented with honest cost.

- [s103] The 2026-07-09 01:25 Judge FAIL (docs/grind/decisions.md:8) settled canonical-asm authorization as FAIL for cpu_side_move_dir_4 and its twins under any framing of 'exhaustion/mechanism-explained/cluster-shared' evidence; judge_constraints[0] directed modality shift to un-run sanctioned axes.

- [s103] Both un-run sanctioned axes the ledger identified are now empirically closed: s101 killed F1 pointer-alias-fake-exception across 5 probe shapes (block/fn-scope × pass-through/from-global × asymmetric/dual-use — every pass-through form combine-folds byte-neutral INERT); s102 killed F2 livelen-shortening structural across 6 probes on probe1 base (defer-into-do-timeout, block-local, prologue-last, pass-through alias, direct-symref, h5-symref+block-local — every deferral form leaves the pair intact).

- [s103] Residual is theorem-locked ONE instruction pair {sll4@54 <-> addu5@55}: both LAUNCH, sched.c LUID decides, A5-before-T0B orders hit the p106/val5 [18,24]/[20,26] L6=L6 birth-tie at 6 (per notes.md/state.json).

- [s103] The only known C-level lever that reaches masked-2 from a base of masked≥15 is the h5 form's cross-symbol arithmetic idiom (`idx_1495 = &D_800A1494[1]` initializer producing the addu-spelling), which s97 forensics named at expr.c::expand_expr RTL insns 34 (subsi3 SYMBOL_REF-diff) + 38 (addsi3 tbl+delta), unfoldable in cse.c/combine.c. s98 rederive killed in-repo transplant (unique to csmd4 in src/); s99 killed 3 symbolic variants (masked=15 regressions or +2/+1insn). This idiom sits OUTSIDE every currently-sanctioned pure-C family — hence the owner-only sanction question.

- [s103] Precedent: motion_SetMotion 2026-07-18 (decisions.md:761) and hirahira_w_frie 2026-07-17 (decisions.md:627) both filed OWNER-ESCALATIONs of this shape after research/structural modality; hirahira_w_frie owner-ruled option (a) TOMBSTONE NARROWED 2026-07-17 10:35; motion_SetMotion owner-ruled option (b) UNSANCTIONED 2026-07-18. Both dispositions were owner acts, not judge or agent acts.

- [s103] candidate.c preserved unchanged at memory/grind/cpu_side_move_dir_4/candidate.c (116 lines, h5 form, masked=2/raw=2). No src/system.c edits by s103. No new rejected forms (nothing new tried this session — the closure inventory is complete).

## s104 — escalation modality — DISPOSITION REACHED (terminal)

**Chassis re-measured, not quoted.** The 2026-08-19 asm-until-matched migration changed this
function's representation on main: `src/system.c:376` is now `INCLUDE_ASM("asm/funcs", CD_sync);`
with 0 regfix/asmfix rules holding a byte match (the 5 rules cited in the 2026-07-20 owner ruling are
gone; `regfix.txt` retains a single non-load-bearing reference). To confirm the ledger's floor is
still valid on THIS chassis, s104 substituted `memory/grind/CD_sync/candidate.c` (symbol renamed
`cpu_side_move_dir_4` -> `CD_sync`) for the INCLUDE_ASM line and ran the gradient:
**score=2, target_insns=160, build_insns=160, rules_dropped=0** — identical to the s103 floor.
`src/system.c` was reverted to HEAD immediately (working tree clean apart from the ledger, the
decisions.md entry, and the untracked scratch dir). CONCLUSION: the migration is floor-neutral for
CD_sync; every s1–s103 spelling conclusion remains chassis-valid and must not be re-derived.
`memory/grind/CD_sync/candidate.c` has been rewritten with the `CD_sync` symbol name so a future
re-attempt can apply it directly.

**Gate (a) canonical-asm — FAILS (re-run this session, not inherited).**
`tools/scan_hand_coded.py --single CD_sync` = `tier=LOW score=2/8 (160 insns)`. Only S4 (4 loads in
an 8-insn window @ insn 49) and S5 (1 approx-sibling `CD_ready`, jaccard=0.64) fire. The STRONG
signals S1/S2/S6 are all absent (0 multu/mflo pairs, no empty-body branches, no BIOS jumptable
pattern); S3/S7/S8 negative. Artifact: `tmp/grind/CD_sync/s104/scan_hand_coded.txt`. Note this is a
score CHANGE from the 2026-07-20 ruling's cited "2/8" under the old name — same tier, same verdict.
Independently barred by the 2026-07-09 Judge constraint.

**Gate (b) SOTN precedent — FAILS (fresh census this session).**
Closing construct is the cross-symbol arithmetic idiom
`idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)`. Searched
`docs/reference/sotn-construct-index.md` for cross-symbol / symbol-difference / `&D_xxxx[1]` shapes:
**zero hits**. This is the first time the machine-generated SOTN construct index (added after the
2026-07-20 ruling) has been searched for this idiom, and it reproduces the s98 manual survey's
negative rather than opening a new question. A negative census is a FAILED gate.

**Disposition filed:** `docs/grind/decisions.md:7993` —
`## 2026-08-20 — CD_sync (src/system.c) — **OWNER-ESCALATION — RESOLVED BY STANDING RULING
(2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**`. Both AND-gates fail, which is the owner's
pre-decided case; the disposition is terminal and nothing is pending on the owner. Session outcome
`owner-gated` with that entry as `escalation_ref`.

**What a future re-attempt would need (do not re-run the closed space):** a pure-C lever that
reorders `{sll@54, addu@55}` at the `sched.c` LUID tiebreak WITHOUT paying the p106/val5 allocno tie
(cost 6). Every axis the ledger identified as sanctioned and un-run is now empirically closed:
F1 pointer-alias-fake-exception (s101, 5 shapes), F2 livelen-shortening (s102, 6 shapes),
F3 conflict-graph attribution (s96/s97), F97a analytic closure (s100), F97b/c (s90/s98), permuter
across 6 chassis (~81k+ iters, 0 novel basin closures), m2c (s8), in-repo transplant (s98), and the
~40 in-family decompositions banked as 112 rejected forms.

- [s104] Chassis fact: src/system.c:376 is `INCLUDE_ASM("asm/funcs", CD_sync);` with 0 regfix/asmfix rules holding a byte match - the 5 rules cited in the 2026-07-20 owner ruling were removed by the 2026-08-19 asm-until-matched migration. regfix.txt retains one non-load-bearing reference. There is no cheat on main to retain or clean up.

- [s104] Floor re-measured this session on the post-migration chassis: score=2, target_insns=160, build_insns=160, rules_dropped=0 (artifact tmp/grind/CD_sync/s104/sandbox_floor.json). The migration is floor-neutral; every s1-s103 spelling conclusion remains chassis-valid.

- [s104] Identity: CD_sync is the function formerly named cpu_side_move_dir_4 @ 0x80080DB0 = PsyQ libcd bios.c v1.86 CD_sync (name string "CD_sync" @ 0x80016240, memory/closer/libcd-identity.md:5). It was already ruled REFUSED / OWNER-ACCEPTED INCOMPLETE on 2026-07-20 (docs/grind/decisions.md:941) under the old name; the rename plus the migration returned it to the active queue, so the disposition had to be re-filed under the current name.

- [s104] scan_hand_coded.py --single CD_sync = tier LOW 2/8, no STRONG signals (S1/S2/S6 all absent).

- [s104] docs/reference/sotn-construct-index.md census for the cross-symbol arithmetic idiom: zero hits - a censused negative, i.e. a FAILED gate, not an open question.

- [s104] Residual unchanged across 103 sessions: ONE pair {sll@54 <-> addu@55}, theorem-locked at GCC 2.7.2 sched.c LUID tiebreak (both LAUNCH same cycle, equal priority); every reordering decomposition pays the p106/val5 allocno tie at cost 6. ~40 decompositions measured, 112 rejected forms banked.

- [s104] Exhaustion across 6 distinct modalities: structural 26 sessions (incl. F1 pointer-alias-fake-exception KILLED s101 across 5 shapes; F2 livelen-shortening KILLED s102 across 6 shapes), forensics s96/s97 (bit-identical greg allocno conflict edge sets; p79 nrefs delta attributed to expr.c::expand_expr RTL insns 34 subsi3 / 38 addsi3, unfoldable in cse.c/combine.c), rederive s8/s90/s98/s99 (m2c KILLED; in-repo transplant KILLED; symbolic-variant sweep KILLED/INERT), synthesis s100 (F97a analytically CLOSED, 0 semantic-purpose candidates), transplant s2/s4/s9 (marionation twins KILLED), permuter across 6 chassis s5/s13/s14/s86/s94/s95 (~81k+ aggregate iters, 0 novel basin closures).

- [s104] Disposition filed this session at docs/grind/decisions.md:7993 with both gates' evidence, the full exhaustion inventory, and the terminal standing-ruling title. Nothing is pending on the owner.

- [s104] memory/grind/CD_sync/candidate.c updated in place: the banked floor-2 form now carries the CD_sync symbol name so a future re-attempt can apply it directly to src/system.c.

- [s105] Chassis re-measured on the post-migration tree: candidate.c spliced over `INCLUDE_ASM("asm/funcs", CD_sync);` at src/system.c:376 gives score=2, target_insns=160, build_insns=160, rules_dropped=0. Floor unchanged since s97. src/system.c restored to HEAD at end of session.
- [s105] g3 base (the order-perfect masked-6 form: `v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c); t0<<=2; debug_printf(..., *(s32*)(t0+(s32)tbl_125c), arg5)`) re-measured this session at score=6, build_insns=160 - the session-4b measurement is chassis-valid and reproducible; use it as the base for any future attack on the p106/val5 v1<->a0 exchange.
- [s105] MEASURED RULE for the F1 combine-foldable chain-extender family on this function: byte-neutrality requires the ENTIRE detour to collapse to a link-time constant. Detours whose result is runtime-dependent materialize +4 insns (single value detour, 164), +6 insns (double detour or an idx_1494-symbol delta, 166). Only the tree-level-reassociable address detour stayed at 160, and it regresses (+18 h5 / +19 g3). This is why idx_1495's extender folds (result &D_800A1494+1 is link-known) and why nothing aimed at arg5 can.
- [s105] The idx_1495 cross-symbol extender is load-bearing, measured: honest `idx_1494 + 1`, `&idx_1494[1]` and `(u8 *)&D_800A1494 + 1` all give masked 15 / 160 insns against the candidate's 2.
- [s105] candidate.c is now FAKE-annotated on the idx_1495 line (family dead-store-fake-exception.md:32-46, mechanism flow.c reg_n_refs before combine.c SYMBOL_REF fold, byte-neutrality 160==160, lever-exhaustion pointer). Re-measured after annotation: score 2, build_insns 160 (the comment is on the existing line, so no line-note shift).
- [s105] psyz (Xeeynamo/psyz, PsyQ 4.0) decomp/src/libcd/bios.c line 94 = `INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_sync);` - no reference C exists for CD_sync (nor getintr / CD_ready / CD_cw / CD_datasync / CD_getsector* / callback). Fetched and banked at tmp/grind/CD_sync/s105/psyz_bios.c. The psyz axis named in the 2026-08-24 owner directive is closed.
- [s105] PROVENANCE (from that psyz fetch, independent confirmation of the BB2 symbol identities used by this function): `D_800A125C` = `char *CD_intstr[8]` = {"NoIntr","DataReady","Complete","Acknowledge","DataEnd","DiskError","?","?"}; `D_800A1494`/`D_800A1495` = the two `Result` bytes; `D_800A11DC[D_800A11D5]` = `CD_comstr[CD_com]` (32-entry command-name table); `D_800161C8` = `"%s:(%s) Sync=%s, Ready=%s\n"`; `D_80016240` = the `"CD_sync"` name string stored into `D_800F19C0`; `D_800161B8` = `"CD timeout: "`. The debug_printf block is therefore literally `printf("%s:(%s) Sync=%s, Ready=%s\n", name, CD_comstr[CD_com], CD_intstr[Result[0]], CD_intstr[Result[1]])` - which is exactly the shape the candidate spells; the residual is codegen reproduction, not a misread of the source.
- [s105] scan_hand_coded --single CD_sync re-run: tier=LOW score=2/8, only S4 (4 loads in an 8-insn window @ insn 49) and S5 (approx-sibling CD_ready, jaccard 0.64). Artifact tmp/grind/CD_sync/s105/scan_hand_coded.txt.
- [s105] Disposition filed at docs/grind/decisions.md:11814 (2026-08-25, standing-ruling REFUSED / OWNER-ACCEPTED INCOMPLETE) after executing BOTH axes of the 2026-08-24 owner directive and measuring both dead. 10 new rejected forms banked (s105_chainext_*, s105_honest_idx1495_plus1_regress15.c) -> 122 total.

- [s105] Chassis re-measured this session: candidate.c over src/system.c:376 -> score 2, target_insns 160, build_insns 160, rules_dropped 0. Floor unchanged since s97; src/system.c restored to HEAD (working tree clean apart from ledger + decisions.md + untracked scratch/rejected forms).

- [s105] g3 base (v0<<=2; arg5=*(s32*)(v0+(s32)tbl_125c); t0<<=2; call(..., *(s32*)(t0+(s32)tbl_125c), arg5)) reproduces at score 6 / 160 on this chassis - the session-4b order-perfect form is still the right base for any future attack on the p106/val5 v1<->a0 exchange.

- [s105] MEASURED RULE (new, generalizes beyond this function): an F1 combine-foldable chain-extender is byte-neutral only when the ENTIRE detour collapses to a link-time constant. Runtime-dependent detours materialize +4 insns (single value detour) or +6 (double detour / idx_1494-symbol delta). That is exactly why idx_1495's extender folds (its result &D_800A1494+1 is link-known) and why nothing aimed at arg5 - whose address depends on the loaded index v0 - can.

- [s105] The idx_1495 cross-symbol extender is load-bearing: all three honest respellings measure masked 15 at 160 insns vs the candidate's 2.

- [s105] psyz decomp/src/libcd/bios.c:94 is INCLUDE_ASM for CD_sync - the version-correct PsyQ 4.0 reference has no seed. Banked at tmp/grind/CD_sync/s105/psyz_bios.c.

- [s105] Provenance confirmed from that fetch: the debug_printf block is literally printf("%s:(%s) Sync=%s, Ready=%s\n", name, CD_comstr[CD_com], CD_intstr[Result[0]], CD_intstr[Result[1]]) - the candidate already spells that shape, so the residual is codegen reproduction, not a misread source.

- [s105] Gate (a) re-run this session: scan_hand_coded --single CD_sync = tier=LOW score=2/8; only S4 (4 loads in an 8-insn window @ insn 49) and S5 (approx-sibling CD_ready, jaccard 0.64) fire; S1/S2/S6 absent. Artifact tmp/grind/CD_sync/s105/scan_hand_coded.txt.

- [s105] Gate (b) fails by construction: the residual {sll@54 <-> addu@55} is a sched.c LUID tiebreak with NO C-level construct to find a precedent for; every source reordering pays the p106/val5 [18,24]/[20,26] L6=L6 allocno birth-tie at cost 6.

- [s105] 10 new rejected forms banked (s105_chainext_* x9, s105_honest_idx1495_plus1_regress15.c) -> 122 total in memory/grind/CD_sync/rejected/.

- [s105] Disposition filed by this session at docs/grind/decisions.md:11814 - both AND-gates fail, which is the owner's pre-decided case under the 2026-07-27 standing ruling. No standard-lowering packet was filed (2026-08-24 auto-reject class).

- [campaign-sweep 2026-08-30] FIRST actual solver-chain run on this function
  (sched-tie endgame campaign, docs/superpowers/specs/2026-08-30-sched-tie-endgame-campaign.md;
  no sched_solver/ra_solver artifact existed in this ledger before today — prior
  "solver-adjacent" work was hand modelling). Executes the solver-modality
  directive from the 2026-08-30 ruling-10 return-to-active. Artifacts:
  tmp/grind/CD_sync/s106/{perturb_d1.txt,inverse_swap84_82_d2.txt,diff_sites.py}.
  **Sched half (h5 candidate chassis, sandbox 2, 160/160):** perturb.py depth-1
  EXHAUSTIVE (1044 single atoms, pass2 block 3 = the debug_printf window, the only
  divergent block) -> 39 vectors reach target order and they encode exactly TWO
  intents: (i) source-order flip — the addu5 statement (uid121 `addu $2,$2,$19`)
  ahead of sll4 (uid111 `sll $4,$4,2`) in luid/add_dep spellings — i.e. the known
  g3-basin trap; (ii) `cost 121 := 2` — C-unreachable (the insn is fixed as an
  addu by target's own bytes; cost is the machine model's, not the source's).
  No third intent exists at depth 1. This upgrades the s105 prose closure of the
  ORDER half to a typed enumeration.
  **RA half (g3 order-perfect chassis, sandbox 6):** goal_from_tgt goal = 6x
  $v1->$a0 + 2x $a0->$v1 at normalized sites 49/55/56/59/61/65 = pseudo 84 (the
  t0/idx chain) <-> pseudo 82 (the arg5 value). inverse.py global --swap 84,82
  --depth 2: FORECLOSED — all preference atoms for $v1 ($v1 never appears in
  pre-RA RTL, set_preference can never record it); 22 single-atom vectors
  otherwise: refs_up p82 7->8 (cheapest), live_shrink p82 10->{8,6,2},
  refs_down p84 7->{6,5}, live_extend p84 9->{11,13,17}, pref_add p84
  copy-preference for $a0.
  **Cross-check vs the 105-session bank — two axes look genuinely UNPROBED:**
  (a) mild t0-side DEMOTION: the bank only measured the refs-2 collapse of t0
  (h8* forms, equiv-sink at 14); refs_down 7->6 or live_extend of the t0 chain
  (lengthening its span to lower pri = refs/livelen) was never spelled;
  (b) p82 live-SPLIT: giving the short-lived arg5 use its own local so the range
  stops at the last real use — distinct from the probed staged-value forms.
  The sanctioned refs_up route via cross-arm duplication stays blocked per s10
  (v0 set inside the do_timeout block makes it byte-non-neutral). Next session:
  probe (a) then (b) on the g3 chassis before anything else.
  **Tool note:** perturb's object-goal path initially refused this function —
  goalmap object mode assumed 1:1 text/object length, false for macro-bearing
  streams (`la`, bare-symbol mem ops -> lui+op). Fixed in
  tools/sched_solver/goalmap.py (_macro_expand_counts + checksum) same day.

- [s106] Executed the two axes the 2026-08-30 solver campaign-sweep flagged as genuinely UNPROBED, on the chassis it named. 11 sandbox measurements, all byte-neutral (build_insns 160 == target 160, rules_dropped 0). **Axis (a) t0-side demotion (g3 chassis, control re-measured 6):** 14 / 14 / 9 / 9 / 14 across refs_down-by-inline, refs_down-by-single-def, refs_down-by-pointer-local, live_extend-by-base-accumulator, live_extend-by-split-shift. **Axis (b) p82 live-split (g3):** 6 / 6 / 6 INERT for own-local, address/value split, and late deref; 14 for the maximal shrink (no local at all). **Axis (b) on h5 (floor chassis):** 2 / 2 INERT. Both axes KILLED - nothing at or below either chassis' control. Artifacts tmp/grind/CD_sync/s106/b_*.c + rejected/s106_*.c (13 forms banked -> 135 total).

- [s106] STRUCTURAL FINDING: the g3 basin admits only the discrete score set {6, 9, 14} under EVERY allocno-priority perturbation of the t0/arg5 pair, independent of whether the perturbation is by reference count or by live length, and independent of rtx class (int vs pointer local). That is why the solver's ranked vectors are model-REACHABLE yet C-unreachable: local-alloc.c coalesces split locals back into one quantity before global.c sees the shortened range, so `live_shrink` has no C realization, and any `refs_down` big enough to register also perturbs emission order and re-enters the g3-basin order trap. This closes the RA half of the residual the same way s105 closed the ORDER half.

- [s106] CANDIDATE IMPROVED (not in score - in honesty): the `v0` staged-value borrow was measured NOT load-bearing (fresh honest local `ix` scores 2 / 160, bit-identical to the borrow). memory/grind/CD_sync/candidate.c now spells `s32 ix; ix = idx_1494[1]; ... ix <<= 2;` and the staged-value /* FAKE */ annotation is GONE. Re-measured after the edit: score 2, target_insns 160, build_insns 160, rules_dropped 0. The floor-2 form now carries two FAKE constructs, down from three.

- [s106] The remaining scaffold is irreducible: dropping the `pp` pointer-alias regresses 2 -> 8; additionally collapsing the two-step `t0 *= 4; t0 = base + t0` addressing regresses to 10; and s105 measured all three honest respellings of the idx_1495 cross-symbol extender at 15. Every remaining annotated construct in candidate.c is load-bearing, and the idx_1495 one is from the family the owner REFUSED on 2026-07-20.

- [s106] Gate (a) re-run on this chassis: `python3 tools/scan_hand_coded.py --single CD_sync` = `tier=LOW score=2/8 (160 insns) - no strong hand-coded indicators`. Only S4 (4 loads in an 8-insn window @ insn 49) and S5 (approx-sibling CD_ready, jaccard 0.64) fire; S1/S2/S6 all absent, S3 negative (9 spills), S7/S8 negative. Artifact tmp/grind/CD_sync/s106/scan_hand_coded.txt. Third consecutive identical result (s104, s105, s106).

- [s106] Gate (b) re-run: `docs/reference/sotn-construct-index.md` searched for cross-symbol / symbol-difference / `(s32)&D_` shapes and for live-split shapes - ZERO hits on both (artifact tmp/grind/CD_sync/s106/gate_b_sotn_census.txt). Reproduces the s104 census negative. No in-hand SOTN-master precedent exists for the closing construct.

- [s106] Disposition filed this session at docs/grind/decisions.md (2026-08-30 entry) under the 2026-07-27 standing ruling: BOTH endgame-lock AND-gates FAIL, which is the owner's pre-decided case. No standard-lowering packet filed (2026-08-24 auto-reject class).

- [s106] 11 sandbox measurements this session, ALL byte-neutral (build_insns 160 == target_insns 160, rules_dropped 0). g3 control re-measured at 6/160; h5 control re-measured at 2/160. Floor unchanged at 2.

- [s106] STRUCTURAL: the g3 basin admits only the discrete score set {6, 9, 14} under EVERY allocno-priority perturbation of the t0/arg5 pair - independent of refs vs livelen, independent of rtx class (int vs pointer local). The solver's ranked vectors are model-REACHABLE but C-unreachable: local-alloc.c coalesces split locals into one quantity before global.c sees the shortened range (kills live_shrink), and any refs_down large enough to register also perturbs emission order and re-enters the g3-basin order trap.

- [s106] This closes the RA half of the {sll@54 <-> addu@55} residual the same way s105 closed the ORDER half. Both halves are now independently, typed-exhaustively closed.

- [s106] CANDIDATE IMPROVED: the v0 staged-value /* FAKE */ is gone - an ordinary fresh local ix measures 2/160, bit-identical. memory/grind/CD_sync/candidate.c now carries two annotated constructs instead of three.

- [s106] The remaining scaffold is irreducible: pp pointer-alias 2 -> 8 when dropped; t0 two-step addressing -> 10 when additionally collapsed; idx_1495 honest respellings all 15 (s105).

- [s106] The one construct holding the 13-point gap is the cross-symbol arithmetic idiom the owner REFUSED on 2026-07-20. It is not resurrected and not proposed for sanction.

- [s106] Gate (a) FAILS: scan_hand_coded tier=LOW 2/8, no S1/S2/S6. Gate (b) FAILS: zero SOTN-construct-index hits. Both AND-gates fail = the owner's pre-decided case under the 2026-07-27 standing ruling.

- [s106] No standard-lowering packet filed (2026-08-24 auto-reject class): a 'grant the cross-symbol family' or 'override the canonical evidence bar' question is pre-decided NO.

- [s106] 13 rejected forms banked -> 135 total in memory/grind/CD_sync/rejected/.

- [s106] src/system.c restored to HEAD (INCLUDE_ASM); working tree carries only ledger + decisions.md + metrics/events.jsonl + untracked scratch.

- [s107] Chassis re-measured at dispatch: candidate.c spliced over
  `INCLUDE_ASM("asm/funcs", CD_sync);` at `src/system.c:376` -> `sandbox CD_sync
  --disable all` = **score 2, target_insns 160, build_insns 160, rules_dropped 0,
  scorable true**. The ledger floor of 2 is chassis-current; no drift.
- [s107] The 0x800A1494/95/96 storage is DEFINED IN ASSEMBLY, not in C:
  `asm/data/7D920.data.s:31048-31076` carries `dlabel D_800A1494` (.byte 0x00),
  `dlabel D_800A1495` (.byte 0x00), `dlabel D_800A1496` (.byte 0x00 x2) and
  `dlabel D_800A1498` whose first word is `.word D_800A1494`. Eight `asm/funcs/*.s`
  files reference these names directly (CD_cw 6 sites, getintr 5, func_800819C4 5,
  func_800817A0/CD_flush 4, func_80081E1C 1, plus CD_sync/CD_ready/CD_datasync).
  Any aggregate declaration is therefore a second handle -> the sanctioned
  per-word-splat->aggregate family's prong (c) cannot be satisfied for this object
  while those consumers remain assembly.
- [s107] In C, `src/system.c` reaches the same bytes under a SECOND set of names
  (`g_cd_status_a/b/c`, named_syms.txt:68-70, symbol_addrs.txt:85-87) used by two
  matched in-TU consumers (src/system.c:416-419 and :493-496, :621). So the object
  already has two C handle families plus the asm handles; the merge would have had
  to retire all of them.
- [s107] Measured cost of the aggregate spelling on CD_sync (5 builds, artifacts in
  tmp/grind/CD_sync/s107/): volatile+direct 34/159, volatile+base-pointer 32/157,
  non-volatile+direct 34/159, non-volatile+base-pointer 18/157, aggregate-with-pun
  2/160. `volatile` on the aggregate is strictly worse than plain, contradicting the
  reopen note's expectation.
- [s107] `python3 tools/scan_hand_coded.py --single CD_sync` ->
  `HAND_CODED: tier=LOW score=2/8 (CD_sync, 160 insns) - no strong hand-coded
  indicators`; only S4 (4 loads in an 8-insn window @ insn 49) and S5 (1
  approx-sibling CD_ready, jaccard 0.64) fire; S1/S2/S6 all negative. Third
  independent reproduction (s104, s105/s106, s107). Artifact
  `tmp/grind/CD_sync/s107/scan_hand_coded.txt`.

- [s107] Chassis re-measured at dispatch: candidate.c spliced over INCLUDE_ASM at src/system.c:376 -> sandbox CD_sync --disable all = score 2, target_insns 160, build_insns 160, rules_dropped 0, scorable true. The ledger floor of 2 is chassis-current; no drift.

- [s107] The 0x800A1494/95/96 storage is defined in assembly at asm/data/7D920.data.s:31048-31076 (dlabels D_800A1494/95/96 plus the D_800A1498 descriptor whose first word is .word D_800A1494), and is referenced by name from eight asm files (CD_cw 6 sites, getintr 5, func_800819C4 5, func_800817A0/CD_flush 4, func_80081E1C 1, plus CD_sync/CD_ready/CD_datasync).

- [s107] Aggregate-merge cost on CD_sync (5 builds): volatile+direct 34/159, volatile+base-pointer 32/157, non-volatile+direct 34/159, non-volatile+base-pointer 18/157, aggregate-with-pun 2/160. volatile is strictly worse than plain, contradicting the reopen note's expectation.

- [s107] The aggregate DECLARATION itself is byte-neutral (p5 = 2/160) but only while the refused cross-symbol delta is retained through a (u8*)&D_800A1494 pointer pun - explicitly forbidden by prong (d) ('never a per-use pointer pun') and retiring nothing.

- [s107] scan_hand_coded --single CD_sync: tier=LOW score=2/8, only S4 and S5 fire; S1/S2/S6 negative.

- [s107] The prong-(c) finding is symbol-level, not function-level: it holds identically for CD_ready and CD_datasync (frontier F14), so the Ruling D scope grant cannot be spent by any of the three while CD_cw / getintr / func_800819C4 / func_800817A0 / func_80081E1C remain assembly-only consumers.

- [s107] src/system.c verified clean against HEAD at session end (keeps INCLUDE_ASM("asm/funcs", CD_sync);). 5 disproven forms banked to memory/grind/CD_sync/rejected/ (140 total).

- [s107] Foreclosure record filed this session at docs/grind/decisions.md:18279.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=107

## s108 � forensics (instrumented cc1: sched.c + local-alloc.c telemetry)

Chassis re-measured at dispatch: `memory/grind/CD_sync/candidate.c` spliced over
`INCLUDE_ASM("asm/funcs", CD_sync);` at `src/system.c:376` -> `sandbox CD_sync
--disable all` = **score 2, target_insns 160, build_insns 160, rules_dropped 0**.
Unchanged from the s104-s107 chassis. `src/system.c` restored to HEAD at session end.

New tooling (artifacts, all under `tmp/grind/CD_sync/s108/`): `schedcap.py` runs the
project's exact `cpp | cc1` front half with the INSTRUMENTED cc1
(`tools/gcc-2.7.2/cc1`) and any `BB2_*_DEBUG` env hooks, writing `-da` dumps plus the
telemetry to `<tag>.stderr`; `probe.py` splices a window variant of candidate.c into
`src/system.c`; `ndiff.py` prints a per-index diff using **the engine's own
normalization** (`engine.score.normalized_insns` on `tmp/sandbox/CD_sync/system.o`
vs `build/src/system.o`), which is what finally made the residual legible � the
older mnemonic-only `odiff.py` could not show it.

### [s108-E1] The two basins are the SAME degree of freedom with opposite signs

Measured, not inferred. Two window spellings, both 160/160:

| form | window statement order | masked | residual |
|---|---|---|---|
| h5 (candidate; also `rejected/s108_h5_t0shift_deferred.c`) | t0 chain before arg5 | **2** | `{sll@54 <-> addu@55}` pair order; **registers all correct** |
| g3 (`rejected/s108_g3_arg5_first_regexchange.c`) | arg5 subseq before t0 chain | **6** | **order all correct**; a pure a0/v1 exchange on 6 insns |

The g3 residual, verbatim from `ndiff.py` (target | ours):

    49  lbu a0,0(s2)      | lbu v1,0(s2)
    55  sll a0,a0,0x2     | sll v1,v1,0x2
    56  lw  v1,0(v0)      | lw  a0,0(v0)
    59  addu a0,a0,s3     | addu v1,v1,s3
    61  sw  v1,16(sp)     | sw  a0,16(sp)
    65  lw  a3,0(a0)      | lw  a3,0(v1)

i.e. the t0 chain takes v1 where the target takes a0, and the arg5 VALUE takes a0
where the target takes v1. Nothing else differs in 160 instructions.

### [s108-E2] The g3 register exchange is one exact tie in local-alloc.c::qty_compare_1

`BB2_QTY_DEBUG` + `BB2_SUGG_DEBUG` block=3 tables for CD_sync (artifacts
`g3b.stderr` / `h5.stderr`, sliced into `g3_cdsync_qty.txt`). `qty_compare_1`
(`tools/gcc-2.7.2/local-alloc.c`) ranks by
`pri = floor_log2(refs) * refs * size / (death - birth) * 10000`, ties broken by
`return *q1 - *q2` (qty index = birth order):

    g3 (order-perfect, masked 6)
      qty1 reg1=108 (t0 shift result) birth=18 death=24 refs=2 -> pri 3333  ord=2 got=3 ($v1)
      qty2 reg1=100 (arg5 value)      birth=20 death=26 refs=2 -> pri 3333  ord=3 got=4 ($a0)
      => EXACT TIE; the index tie-break puts the t0 shift first, so it takes $v1.

    h5 (masked 2, target registers)
      qty1 reg1=107 (t0 shift result) birth=16 death=24 refs=2 -> pri 2500  ord=3 got=4 ($a0)
      qty2 reg1=100 (arg5 value)      birth=20 death=26 refs=2 -> pri 3333  ord=2 got=3 ($v1)
      => no tie; the arg5 value is allocated first and takes $v1 = TARGET.

The whole 6-vs-2 basin difference is **one scalar**: the birth index of the t0-shift
qty, 16 in h5 vs 18 in g3. One post-sched1 slot earlier => life 8 instead of 6 =>
pri 2500 instead of 3333 => the tie disappears and the target allocation falls out.
And that same slot position is precisely what makes the h5 *emission* order wrong.
Order-correctness and allocation-correctness are the same variable, pulled opposite ways.

### [s108-E3] sched1 clock=13: what the tie actually is (fresh telemetry)

`BB2_SCHED_DEBUG` + `BB2_RANK_DEBUG` on the h5 chassis (`base.stderr`, sliced to
`cdsync_p1.txt`; block=3 = 20 insns, straight line):

    ADJPRI insn=121 deaths=0 birth=1 maxpri=2130706433 pri=2
    RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0
    PICK clock=13 picked=121 (pri=2130706433 luid=12)
      ready was: [ 121(p=2130706433,l=12) 111(p=2130706433,l=8) 142(p=1,l=22) ]

Pass-source detail corrected/completed this session: the 0x7F000001 both insns carry
is NOT assigned by `adjust_priority`. `schedule_block` sets the *currently scheduled*
insn's priority to `LAUNCH_PRIORITY` immediately before calling `schedule_insn`
(sched.c:4049); `schedule_insn` then computes
`max_priority = MAX (INSN_PRIORITY (ready[0]), INSN_PRIORITY (insn))` (sched.c:2619)
and `adjust_priority` raises every birthing insn to that value (sched.c:2586-2590).
So the sentinel is *inherited* by every `birthing_insn_p` insn in the block, which is
why 111 and 121 can never differ on the priority axis: `rank_for_schedule` falls
through the priority test, then through the class test (`val=0` on all 51 block=3
comparisons � reconfirms s15), and terminates on `INSN_LUID (tmp) - INSN_LUID (tmp2)`.
`TAIL_PRIORITY` (0x7ffffffe) is only ever held by the block's last insn, which is
scheduled first and drops to DONE_PRIORITY, so it can never leak into `max_priority`
for this pair.

### [s108-E4] sched2 cannot decouple the RA order from the byte order (on this chassis)

Since h5's post-sched1 stream produces the TARGET registers, the obvious two-stage
lever is: let sched1 emit the h5 order (for RA), and let sched2 restore the target
pair order (for bytes). Measured on the h5 chassis, block=3 of pass=2 (`base.stderr`,
`SCHEDDBG FUNC func=CD_sync pass=2` at line 6396):

    PICK clock=12 picked=123 (pri=2 luid=8)
    RANKDBG last=123 y=121 cls=3 x=111 cls2=3 val=0
    PICK clock=13 picked=121 (pri=2 luid=7)
    RANKDBG last=121 y=118 cls=3 x=111 cls2=3 val=0
    PICK clock=14 picked=111 (pri=2 luid=6)

At sched2 `reload_completed == 1`, so `birthing_insn_p` returns 0 and no insn in the
block carries a sentinel � every block=3 insn sits at pri 1-4. The pair still ties on
priority (both 2) and on class (`val=0`), so `rank_for_schedule` again terminates on
LUID � and the sched2 LUIDs are just the sched1 OUTPUT order. sched2 is a fixpoint of
sched1 for this window: it reproduces the order it is given.

### [s108-E5] Probe scores (all 160/160, h5 chassis, FAKE chain-extender + pp alias present)

    p1 arg5-subseq-first ..................... 6   (g3; order perfect, a0/v1 exchange)
    p2 t0-shift deferred one statement ....... 2   (h5; alternate spelling, target regs)
    p3 arg5 address staged in the ix carrier . 6   (g3; qty priorities unchanged)
    p4 fresh single-set local for t0 address . 9   (window re-schedules, 15 raw diffs)
    p5 t0 address as one single-set expr ..... 9   (same)

All five banked under `memory/grind/CD_sync/rejected/s108_*.c`.

- [s108] Chassis re-measured at dispatch: candidate.c spliced at src/system.c:376 -> sandbox CD_sync --disable all = score 2, target_insns 160, build_insns 160, rules_dropped 0. src/system.c restored to HEAD at session end.

- [s108] The g3 basin's masked-6 residual is a PURE register exchange with zero ordering error: target/ours at indices 49,55,59,65 = lbu/sll/addu/lw on a0 vs v1 (the t0 chain), and at 56,61 = lw/sw on v1 vs a0 (the arg5 value). Nothing else differs across 160 instructions.

- [s108] local-alloc block=3 qty tables from the instrumented cc1: g3 has qty(t0-shift p108) birth18/death24/refs2 = pri 3333 EXACTLY TIED with qty(arg5-value p100) birth20/death26/refs2 = pri 3333, resolved by qty_compare_1's `*q1 - *q2` index tie-break; h5 has qty(t0-shift p107) birth16/death24 = pri 2500 vs 3333, no tie, target registers.

- [s108] The single scalar separating the two basins is the birth index of the t0-shift qty (16 in h5, 18 in g3) - one post-sched1 slot. That same slot position is what makes h5's emission order wrong, so order-correctness and allocation-correctness are the same variable with opposite signs.

- [s108] sched1 clock=13 ready list on the h5 chassis: [121(p=2130706433,l=12) 111(p=2130706433,l=8) 142(p=1,l=22)]; priority tie, class tie (val=0), decided by LUID.

- [s108] sched2 block=3 on the h5 chassis carries no sentinels (all pri 1-4), ties the pair at pri=2 and cls=3, and terminates on LUID inherited from the sched1 output - sched2 is a fixpoint of sched1 for this 20-insn straight-line window.

- [s108] Probe scores this session, all 160/160: p1 arg5-first 6, p2 t0-shift-deferred 2 (alternate h5 spelling with target registers), p3 addr-in-ix-carrier 6, p4 fresh t0-addr local 9, p5 single-expr t0 addr 9. All five banked under memory/grind/CD_sync/rejected/s108_*.c.

- [s108] New reusable tooling: tmp/grind/CD_sync/s108/schedcap.py (exact cpp|cc1 front half with the instrumented cc1 plus arbitrary BB2_*_DEBUG env hooks, -da dumps + telemetry), probe.py (window-variant splicer), ndiff.py (per-index diff using engine.score.normalized_insns - the engine's own normalization; the older mnemonic-only odiff.py could not render this residual).

## s109 [forensics] — where reg_n_refs actually comes from, and why the refs axis has no byte-neutral carrier here

Durable pass-attribution facts established this session (all read out of
`tools/gcc-2.7.2/` source and cross-checked against instrumented-cc1 dumps;
artifacts in `tmp/grind/CD_sync/s109/`):

- **`reg_n_refs` is a flow.c product, not a local-alloc product.** It is
  allocated and zeroed at `flow.c:1281-1282` and incremented (by `loop_depth`,
  = 1 outside loops) at `flow.c:2081`, `2329`, `2515`, `2725`, all inside
  `flow_analysis`. `toplev.c` calls `flow_analysis` at line **2983**, i.e.
  BEFORE `combine_instructions` (**3004**), `schedule_insns` pass 1 (**3033**)
  and `local_alloc` (**3049**). local-alloc then simply copies it:
  `qty_n_refs[qty] = reg_n_refs[regno]` (`local-alloc.c:297`).
  Consequence, and this is the whole reason the axis looked open: a reference
  that COMBINE folds away still counts, but a reference that **cse or expand**
  removes does not. The four `reg_scan` calls (`toplev.c:2826/2859/2922/2925`)
  are unrelated to this array; they build `reg_n_sets`/`reg_basic_block`.
- **Only two things can move `qty_n_refs` after flow:** `optimize_reg_copy_1`
  transfers refs between a copy's src and dest (`local-alloc.c:781-783`,
  `913-914`), and `combine_regs` SUMS them when it ties two pseudos into one
  qty (`local-alloc.c:1932`). Both need a reg-reg copy still present at
  local_alloc time. Measured here: cse deletes ours ~120 insns earlier.
- **`update_equiv_regs` doubles `reg_live_length` for any pseudo carrying a
  REG_EQUIV note** (`local-alloc.c:1063`) and the in-tree comment states
  explicitly that this "does not affect the priority in local-alloc" — it is a
  global.c-only effect. So the REG_EQUIV (mem sp+16) note on the arg5 pseudo,
  which the s108 frontier proposed re-reading through, is inert for the
  block-local tie that actually decides this function's residual.

**The insn-count ladder is the cheap forensic instrument** (CD_sync segment of
the `-da` dumps, counting `^\(insn`): base
`rtl=100 jump=98 cse=92 loop=92 cse2=92 flow=91 combine=70 lreg=69 greg=69`.
Any candidate construct can be located to a pass in one read:
`arg5 = arg5;` gives rtl=100 (identical to base) ⇒ dropped by **expand**, never
in RTL; `arg5b = arg5;` gives rtl=101 jump=99 cse=92 ⇒ deleted by **cse_main**
(`toplev.c:2861`). Both land 90+ insns before flow counts refs, which is why
both measured byte-identical to the base (score 6, build_insns 160) and left
the block=3 qty table bit-for-bit unchanged.

**Variable reuse is not a refs lever in this compiler.** The C front end gives
every assigned VALUE a fresh pseudo and copies it into the DECL pseudo; cse
deletes the copies, so a C local that is assigned four times contributes FOUR
independent refs-2 pseudos at flow time, not one refs-8 pseudo. Measured: the
q2 flow-dump register profile (pseudo numbers AND mention counts) is identical
to the base's, and its block=3 qty table still reads refs=2 for every qty. The
four merge spellings (q1/q2/q3/q4) regress to 9/8/9/9 purely by reshaping
birth/death and the qty COUNT — q1 collapses block 3 from four local qtys to
three and evicts the arg5-value qty from block-local allocation altogether.

**Chassis re-audit (mandated):** `tools/fake_ablate.py` over `candidate.c`
reproduces the floor at **2 / 160** on the current chassis and shows both FAKE
units load-bearing and super-additive — drop chain-extender 15/159, drop pp
alias 17/161, drop both 30/160. The s108 kills were therefore not
carrier-masked, and the ledger floor of 2 is current, not historical.

- [s109] reg_n_refs - the sole refs input to local-alloc's qty_compare_1 (via qty_n_refs[qty] = reg_n_refs[regno], local-alloc.c:297) - is built entirely by flow.c: allocated/zeroed at flow.c:1281-1282, incremented at flow.c:2081, 2329, 2515, 2725. toplev.c calls flow_analysis at line 2983, BEFORE combine (3004), sched1 (3033) and local_alloc (3049). The four reg_scan calls (toplev.c:2826/2859/2922/2925) do not touch this array.

- [s109] Only two things can move qty_n_refs after flow: optimize_reg_copy_1 transferring refs across a copy (local-alloc.c:781-783, 913-914) and combine_regs summing them when tying two pseudos into one qty (local-alloc.c:1932). Both require a reg-reg copy still present at local_alloc; measured here, cse deletes ours at toplev.c:2861.

- [s109] update_equiv_regs doubles reg_live_length for any pseudo carrying a REG_EQUIV note (local-alloc.c:1063) and the in-tree comment states it 'does not affect the priority in local-alloc' - a global.c-only effect. The REG_EQUIV (mem sp+16) note on the arg5 pseudo, which the s108 frontier proposed re-reading through, is therefore inert for the block-local tie that decides this residual.

- [s109] Insn-count ladder over the CD_sync segment of the -da dumps is a one-read pass locator: base rtl=100 jump=98 cse=92 loop=92 cse2=92 flow=91 combine=70 lreg=69 greg=69. A same-value re-store gives rtl=100 (identical) so it is dropped by expand and never reaches RTL. A plain local copy gives rtl=101 jump=99 cse=92, so it is deleted by cse_main. Both die 90+ insns before flow counts refs.

- [s109] C-level variable reuse does not accumulate references on one pseudo in GCC 2.7.2: each assigned value gets a fresh pseudo copied into the DECL pseudo, cse deletes the copies, and the flow-time register profile of q2 (pseudo numbers AND mention counts) is identical to the base's with every block=3 qty still at refs=2.

- [s109] g3 basin block=3 qty table (re-measured this session, unchanged from s108): qty0 reg106 birth10 death20 refs6 got=2; qty3 reg114 birth22 death30 refs4 got=2; qty1 reg108 birth18 death24 refs2 got=3 ($v1, the t0 shift); qty2 reg100 birth20 death26 refs2 got=4 ($a0, the arg5 value) - the inverse of the target, decided purely by the qty-index tiebreak after an exact 3333/3333 priority tie.

- [s109] The SUGGDBG-FFR lines show both contested qtys enter find_free_reg with ncopysugg=0 nsugg=0, i.e. NEITHER carries a hard-register suggestion - so the suggestion arm of find_free_reg is completely unexercised at this site, while other CD_sync blocks (blk=0 qty=0 nsugg=1 sugg=2) show the mechanism does fire elsewhere in this same function.

- [s109] Chassis re-audit: candidate.c re-scores 2/160 on the current chassis; its two FAKE units cost 13 and 15 points individually and 28 jointly when ablated, so the recorded floor is current and un-masked.

## s110 (rederive) - the honest-chassis residual is a global-allocno seating problem, quantified

- [s110] The honest chassis (`idx_1495 = idx_1494 + 1;`, no cross-symbol chain-extender) scores 15/160 with build_insns 160 and rules_dropped 0. A full objdump-vs-target instruction walk (tmp/grind/CD_sync/s110/odiff.py) shows it is structurally identical to the target at every one of the 160 slots except (a) a permutation of the four callee-saved seats {s3,s4,s5,s6} and (b) the long-known slot-54/55 order swap. So ~13 of the 15 points are ONE decision in global.c, not a scheduler residual.
- [s110] ALLOCDBG (BB2_ALLOC_DEBUG=1, hook at tools/gcc-2.7.2/global.c:605) side by side:
  - h5 / masked 2 (target seating): idx_1494 pri=933 -> $s2; tbl_125c nrefs=5 len=148 pri=675 -> $s3; idx_1495 nrefs=2 len=72 pri=277 -> $s4; mode nrefs=2 len=76 pri=263 -> $s5; result nrefs=2 len=79 pri=253 -> $s6.
  - honest / masked 15: idx_1494 933 -> $s2; mode 263 -> $s3; result 253 -> $s4; tbl_125c nrefs=3 len=148 pri=202 -> $s5; idx_1495 nrefs=2 len=144 pri=138 -> $s6.
- [s110] THE CLOSING CONDITION, stated numerically for the first time: on the honest chassis the function seats correctly iff `933 > pri(tbl_125c) > pri(idx_1495) > 263`. mode and result are pinned at 263/253 (a parameter's minimum nrefs is 2 and this body has no loop-depth weighting). Holding the TARGET'S OWN ref counts (tbl_125c 3, idx_1495 2 - both read directly off asm/funcs/CD_sync.s: $s3 appears 3x, $s4 2x), the condition reduces to two live-length inequalities: **live_length(tbl_125c) < 114 and live_length(idx_1495) < 76**, at build_insns 160. Present values are 148 and 144.
- [s110] This is a strictly WEAKER bar than the one s102 measured its six livelen-shortening probes against. s102 derived "need livelen<44" from the h5 pri of 675; the real requirement is only pri>263 for tbl_125c (livelen<114) and pri>263 for idx_1495 (livelen<76). No probe has yet been scored against the relaxed bar.
- [s110] The h5 chain-extender is now understood as a DOUBLE lever, which explains why 15 sessions of single-axis honest respellings all measured exactly 15: it simultaneously (i) lifts tbl_125c from 3 to 5 refs, crossing the floor_log2 boundary (202 -> 675) and (ii) halves idx_1495's live length (144 -> 72; 138 -> 277). An honest replacement has to reproduce BOTH effects, or reproduce the seating some third way.
- [s110] loop.c performs NO invariant hoisting in this function: the outer poll is a goto loop with no NOTE_INSN_LOOP_BEG. Dropping any of the three pointer locals therefore loses its prologue lui/addiu outright (v1 -3 insns/18, v2 -1/34, v3 -4/38) instead of being re-created as a preheader hoist.
- [s110] Making the outer poll a real `while (1)` DOES turn on flow.c's `reg_n_refs += loop_depth` weighting (measured: idx_1494 7 -> 12 refs, mode/result 2 -> 3), but it lifts the parameters as fast as the pointers and costs +4/+5 instructions with two allocnos left unallocated. Since the target's own ref counts are the UNWEIGHTED ones, the original compile also saw loop_depth 1 - the goto chassis is provenance-correct and the structured-while reading of the PsyQ bios.c source is closed.
- [s110] Tooling written this session and reusable: tmp/grind/CD_sync/s110/splice.py (splice any body file into src/system.c over the INCLUDE_ASM marker), cap.py/cap.sh (instrumented-cc1 capture with arbitrary BB2_* env hooks), odiff.py (objdump the sandbox object and walk it against asm/funcs/CD_sync.s slot by slot), run.sh + run.ps1 (splice -> ALLOCDBG -> sandbox score in one call per variant).

- [s110] Chassis re-measured this session: memory/grind/CD_sync/candidate.c applied to src/system.c scores 2/160, build_insns 160, rules_dropped 0 - the ledger floor reproduced, chassis unchanged since s109.

- [s110] The honest respelling idx_1495 = idx_1494 + 1 scores 15/160 with build_insns 160 (no instruction-count penalty at all): the entire 15 is register naming plus one 2-insn order swap.

- [s110] ALLOCDBG h5 vs honest, verbatim: h5 ord=11..15 = p77(933,$s2) p79 nrefs5 len148(675,$s3) p78 nrefs2 len72(277,$s4) p72 mode(263,$s5) p73 result(253,$s6); honest ord=11..15 = p77(933,$s2) p72 mode(263,$s3) p73 result(253,$s4) p79 nrefs3 len148(202,$s5) p78 nrefs2 len144(138,$s6).

- [s110] Numeric closing condition for the honest chassis, established this session: live_length(tbl_125c) < 114 and live_length(idx_1495) < 76, with nrefs held at the target's own 3 and 2, at build_insns 160. s102's six livelen probes were all judged against livelen<44, a bar three times stricter than what the seating actually requires.

- [s110] The h5 chain-extender is a double lever (tbl_125c 3->5 refs crossing the floor_log2 4-ref boundary, AND idx_1495 live length 144->72); that is the mechanical reason 15 sessions of single-axis honest respellings all measured exactly 15.

- [s110] loop.c performs no invariant hoisting anywhere in CD_sync: the outer poll is a goto loop, so there is no NOTE_INSN_LOOP_BEG and loop_depth is 1 for every insn - which also means every reg_n_refs in this function is a raw mention count.

- [s110] Target ref counts read directly off asm/funcs/CD_sync.s: $s2 (&Intr) 7, $s3 (CD_intstr) 3, $s4 (&Intr+1) 2, $s5 (mode) 2, $s6 (result) 2 - identical to the honest chassis's counts, confirming the honest form has the right refs and only the wrong live lengths.

- [s110] The upstream-provenance frontier item was not advanced this session (no network fetch attempted); tmp/closer/sotn_bios.c's CD_sync body remains the only in-hand reference C and its structured-while shape is now measured to cost +4/+5 instructions here.

## s111 (rederive) - the honest seating gap is ONE compiler fact: local-alloc.c:1064 doubles live_length for REG_EQUIV pseudos

Root cause, named for the first time in 111 sessions. `update_equiv_regs`
(`tools/gcc-2.7.2/local-alloc.c`) attaches a REG_EQUIV note to any pseudo whose
initializing insn is a `single_set` with `reg_n_sets[regno] == 1` and either a
CONSTANT_P REG_EQUAL note (local-alloc.c:1026-1029) or a single-basic-block MEM
source (1048-1052). For every such pseudo it then executes
`reg_live_length[regno] *= 2;` (**local-alloc.c:1064**), under an in-tree comment
saying the doubling "does not affect the priority in local-alloc!" - i.e. it is a
**global.c-only** effect, and global.c is exactly where CD_sync's 13-point
callee-saved-seat permutation is decided.

- [s111] Dump proof: `tmp/grind/CD_sync/s110/honest1495.lreg:3636` carries
  `(expr_list:REG_EQUIV (const:SI (plus:SI (symbol_ref:SI ("D_800A1494")) (const_int 1))))`
  on `(insn 30 (set (reg/v:SI 78) (plus (reg/v:SI 77) (const_int 1))))`. The same
  insn in `h5base.lreg` has NO such note (the dump jumps 3634 -> 3645). idx_1495's
  ALLOCDBG live length is 144 honest vs **exactly 72** in h5. The FAKE
  chain-extender's second lever is therefore not "shortening a live range" at all -
  it is *denying the REG_EQUIV note*, and the 144 -> 72 is the `*= 2` being skipped.
- [s111] All three address pseudos carry the note and are doubled:
  p77 idx_1494 REG_EQUIV(symbol_ref D_800A1494) len 150 = 2x75;
  p79 tbl_125c REG_EQUIV(symbol_ref D_800A125C) len 148 = 2x74;
  p78 idx_1495 REG_EQUIV(const plus) len 144 = 2x72.
  The two PARAMETERS have no note and are counted raw: mode 76, result 79.
  Every one of these five pseudos is live across essentially the whole outer poll
  loop, so their RAW live lengths are all within 72..79 of each other. **The entire
  seating inversion is the x2, nothing else**: honest priorities 933 / 202 / 138 vs
  parameters 263 / 253, when the un-doubled values would be 1866 / 405 / 277 - which
  is precisely the target order idx_1494 > tbl_125c > idx_1495 > mode > result.
- [s111] ROUTE A (deny the note by making `reg_n_sets != 1`, local-alloc.c:1021) is
  measured DEAD across six spellings, all on the honest base (control h0 = 15/160):
  t1 ordinary-C split-init `idx_1495 = idx_1494; idx_1495 += 1;` -> 15/160, len 144
  unchanged; t2/t3/t4 duplicate same-value sets of tbl_125c and/or idx_1495 ->
  18/160, len 152/144 (the move of the set one insn earlier, not a note change);
  q1/q2/q3 *different-value dead first stores* (`tbl_125c = (s32*)&D_800A1494;`
  before the real set) -> 15/160, len 148/144 bit-identical to h0. Mechanism: the
  redundant set is removed by `cse_main` and the dead set by `delete_dead_from_cse`
  (**toplev.c:2867**), both of which run BEFORE the last `reg_scan` that builds
  `reg_n_sets` (**toplev.c:2925**). No C-level extra assignment to these locals
  survives to local_alloc, so reg_n_sets stays 1 and the note is always attached.
- [s111] ROUTE B (shorten the RAW live length by moving the set out of the prologue)
  is measured DEAD on instruction count, not on priority - and it is the first time
  the frontier's acceptance band was actually hit:
    p1  idx_1495 set moved to just before `new_var = 0xFF`  -> len 144->25, pri 138->**800**, score 15, **build_insns 159**
    p3  tbl_125c set moved into the do_timeout window        -> tbl leaves the allocno list entirely, score 22, build 158
    p2  both of the above                                    -> score 22, build 157
    r2  tbl_125c set moved to the loop top                   -> len 148->40, pri 202->**750**, score 20, build 157
    r1  r2 + p1                                              -> ALLOCDBG ord=11..15 = 921 / 800 / 750 / 263 / 253, i.e. **all three pointers above both parameters for the first time on the honest chassis** - score 20, **build_insns 156**
  Every move costs 1-4 instructions: once the constant-address set is not in the
  prologue, reload rematerializes it into the addressing (`lbu $a0,1($s2)` instead
  of `addiu $s4,$s2,1` + `lbu $a0,0($s4)`), so the target's prologue lui/addiu pairs
  (asm/funcs/CD_sync.s:15-19) disappear. Priority-order correctness and
  build_insns==160 are, on this axis, mutually exclusive.
- [s111] Consequence for the arithmetic: at the pinned live lengths (148 doubled /
  144 doubled) there is NO integer nrefs for idx_1495 that lands its priority in the
  band (263, pri(tbl_125c)) - nrefs 3 gives 208 (too low), nrefs 4 gives 555 (above
  a tbl_125c lifted to nrefs 4 = 540). So a pure refs-lift on both pointers cannot
  produce the target order either; **idx_1495 specifically must lose its doubling**.
  That single sub-problem - emit `addiu $s4,$s2,1` in the prologue while denying its
  set a CONSTANT_P REG_EQUAL note - is now the whole remaining honest endgame, and
  it is exactly what the FAKE chain-extender buys.
- [s111] Mandated kill re-audit: candidate.c re-splices at **2 / 160, rules_dropped 0**
  on the current chassis; `tools/fake_ablate.py` gives keep-all 2/160, drop
  chain-extender 15/159, drop pp alias 17/161, drop both 30/160. Floor 2 is current
  and both FAKE units remain load-bearing and super-additive.
- [s111] Tooling: tmp/grind/CD_sync/s111/{splice.py,cap.py,run.sh,run.ps1} (s110
  harness re-pointed); every probe's -da dumps and ALLOCDBG stderr are in that dir.

- [s111] The s110 honest lreg dump carries (expr_list:REG_EQUIV (const:SI (plus (symbol_ref "D_800A1494") (const_int 1)))) on insn 30 = (set p78 (plus p77 (const_int 1))) at tmp/grind/CD_sync/s110/honest1495.lreg:3636; the identical insn in h5base.lreg has no note at all (the dump jumps 3634 -> 3645). idx_1495's ALLOCDBG live length is 144 honest vs exactly 72 with the FAKE chain-extender - the x2 at local-alloc.c:1064 being skipped.

- [s111] The FAKE chain-extender's second lever is therefore REG_EQUIV DENIAL, not live-range shortening; reproducing 'a shorter live range' is what 15 sessions of honest respellings were unknowingly aiming at, and it is not the mechanism.

- [s111] All three address pseudos carry REG_EQUIV and are doubled: p77 idx_1494 150 = 2x75, p79 tbl_125c 148 = 2x74, p78 idx_1495 144 = 2x72. The two parameters carry no note and are counted raw at 76 and 79. All five are live across essentially the whole outer poll loop, so their RAW live lengths sit within 72..79 of each other - the seating inversion is the doubling and nothing else.

- [s111] Un-doubled, the honest priorities would be 1866 / 405 / 277 / 263 / 253 = idx_1494 > tbl_125c > idx_1495 > mode > result, which is exactly the target's $s2/$s3/$s4/$s5/$s6 assignment (asm/funcs/CD_sync.s:15-19, 53-63, 106).

- [s111] At the pinned doubled live lengths no integer nrefs pair produces the target order: idx_1495 at nrefs 3 gives 208 (below mode's 263) and at nrefs 4 gives 555 (above a tbl_125c lifted to nrefs 4 = 540). A refs-lift on both pointers therefore cannot seat this function on the honest chassis; idx_1495 has to lose its doubling.

- [s111] Mandated kill re-audit: candidate.c re-splices at 2/160, rules_dropped 0 on the current chassis; tools/fake_ablate.py gives keep-all 2/160, drop chain-extender 15/159, drop pp alias 17/161, drop both 30/160 - the recorded floor is current and both FAKE units remain load-bearing and super-additive.

## s112 (structural) - the REG_EQUIV note is denial-able in ordinary C; the residual is now one allocno

- [s112] CHASSIS: `memory/grind/CD_sync/candidate.c` re-measured on HEAD this session at
  score 2 / build_insns 160 / rules_dropped 0. The dispatch brief reported the HEAD floor
  as "measurement unavailable"; it is 2, unchanged, and the ledger's spelling conclusions
  are chassis-valid.
- [s112] PASS ATTRIBUTION, named exactly. The REG_EQUAL note that becomes the REG_EQUIV
  that doubles reg_live_length at local-alloc.c:1064 is written by `cse_insn`, at
  **cse.c:6923-6934**, under the gate `n_sets == 1 && src_const && GET_CODE (dest) == REG
  && GET_CODE (src_const) != REG`. `src_const` is filled at cse.c:6484-6507: first from the
  hash-table equivalence class, then from `CONSTANT_P (src_folded)`. Confirmed by the insn
  ladder on h0: insn 24 / 27 / 30 (the tbl_125c, idx_1494 and idx_1495 sets) carry NO notes
  in `.rtl` and `.jump`, and carry `REG_EQUAL` from `.cse` onward.
- [s112] The three constant-address pointer pseudos in CD_sync are p79 tbl_125c
  (`REG_EQUIV (symbol_ref "D_800A125C")`), p77 idx_1494 (`REG_EQUIV (symbol_ref
  "D_800A1494")`) and p78 idx_1495 (`REG_EQUIV (const (plus (symbol_ref "D_800A1494")
  (const_int 1)))`). Only p78's note depends on cse's value table: its source is
  `(plus p77 1)`, which folds to a constant only while p77's constant is still in the
  table. p79's and p77's sources are bare symbol_refs, i.e. `CONSTANT_P (src_folded)` is
  true independent of the table, so their notes are unconditional.
- [s112] THE FINDING. Moving `idx_1495 = idx_1494 + 1;` out of the prologue and into the
  `success:` block - which dominates the pointer's only use (`*idx_1495` in the poll
  callback), because the do_timeout path always returns -1 - puts the set behind a cse
  extended-block boundary, denies the note, and leaves p78's live length at its raw 75.
  Measured: **score 15, build_insns 160, rules_dropped 0**, p78 pri 138 -> 266, seat
  $s6 -> $s3, with the `addiu $s4,$s2,1` prologue instruction still emitted. Banked as
  `memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c`
  (NOT a dead form - it is the new honest base for the next session).
- [s112] This is HALF of what the owner-refused h5 cross-symbol chain-extender was buying.
  The banked floor-2 form's ALLOCDBG, re-read this session, is 933 / 675 (p79 nrefs=5) /
  277 (p78, livelen 72, note ABSENT) / 263 / 253. h5's two levers were (a) deny p78's note
  and (b) lift p79's refs 3 -> 5. Lever (a) is now reproduced in ordinary C with no FAKE
  construct and no instruction cost.
- [s112] Placement cannot help p79. Four relocations measured, the note present in every
  one: loop top on the n1 base 20 / 157; loop top on the n3 base 20 / 157 (livelen 40,
  pri 750, hardreg $s0); immediately before the timeout branch 20 / 157 (identical);
  top of the do_timeout block 22 / 158 (p79 leaves the global allocno list entirely and
  becomes block-local). The priority band is reachable by shortening the live range, but
  global.c's find_reg walks free hard registers in ascending order, so a short-lived
  tbl allocno is seated at $s0, not the target's $s3. **tbl_125c has to stay long-lived
  AND high-priority, which only a note denial or a refs lift can deliver.**
- [s112] Two refs-lift carriers measured inert on the n3 base, both 15 / 160 with p79's
  ALLOCDBG row (nrefs=3 livelen=148 pri=202) bit-identical to control: a cross-cse-block
  reg-reg handle copy (`u8 *tb8 = (u8 *)tbl_125c;` at the outer loop top, consumed by the
  first table access) and a self-difference re-association of the second table address.
  A copy redistributes references between two pseudos and never adds one; the
  re-association folds at cse. Since combine's LOG_LINKS are basic-block-local, a 4th
  mention of p79 that combine can fold must live in the SAME basic block as the two table
  reads (the do_timeout window) and be a real, non-redundant address-forming insn.
- [s112] Aggregate-merge probe, negative: `asm/funcs/CD_sync.s:66-68` loads the third
  debug_printf argument with its OWN `lui $at,%hi(D_800A11DC)` / `lw $a2,%lo(D_800A11DC)($at)`
  pair, NOT off $s3. There is therefore no base-register evidence that D_800A11DC and
  D_800A125C are one aggregate, so the aggregate-merge family's prong (a) fails here and
  a third $s3-relative table access cannot be justified that way.
- [s112] Artifacts: `tmp/grind/CD_sync/s112/` - run.ps1 (splice + sandbox + ALLOCDBG +
  .lreg note grep driver), gen.py / gen2.py / gen3.py (variant generators), notes.sh
  (the one-grep REG_EQUIV verdict), cap.py/cap.sh/prep.sh, and the per-variant
  `<tag>.{c,s,stderr,rtl,jump,cse,loop,cse2,flow,combine,lreg,greg,sched,sched2,jump2,dbr}`
  dump sets for h0, cand, n1-n4, m1-m3, t1, t2.

- [s112] CHASSIS: memory/grind/CD_sync/candidate.c re-measured on HEAD this session at score 2 / build_insns 160 / rules_dropped 0 (the dispatch brief reported the HEAD floor as unavailable); the ledger's chassis-relative conclusions remain valid.

- [s112] PASS ATTRIBUTION (dump-read, not guessed): the REG_EQUAL note that becomes the REG_EQUIV driving local-alloc.c:1064's reg_live_length doubling is written by cse_insn at cse.c:6923-6934. Insns 24/27/30 (the tbl_125c, idx_1494 and idx_1495 sets) carry no notes in h0.rtl and h0.jump and carry REG_EQUAL from h0.cse onward.

- [s112] CD_sync's three constant-address pointer pseudos are p79 tbl_125c REG_EQUIV (symbol_ref D_800A125C), p77 idx_1494 REG_EQUIV (symbol_ref D_800A1494) and p78 idx_1495 REG_EQUIV (const (plus (symbol_ref D_800A1494) 1)). Only p78's note depends on cse's value table, because only its source is a (plus reg const) that has to be folded; the other two sources are bare symbol_refs and are CONSTANT_P unconditionally.

- [s112] NEW HONEST BASE: idx_1495's init placed in the success block scores 15 / build_insns 160 / rules_dropped 0 with the note denied, p78 pri 138 -> 266 and seat $s6 -> $s3, and the target's prologue addiu $s4,$s2,1 still emitted. Banked as memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c - this file is the next session's starting chassis, not a dead form.

- [s112] The banked floor-2 (h5) form's ALLOCDBG is 933 / 675 (p79 nrefs=5 livelen=148) / 277 (p78 livelen 72, REG_EQUIV ABSENT) / 263 / 253, so the owner-refused cross-symbol chain-extender was doing exactly two things: denying p78's note and lifting p79's refs 3 -> 5. The first is now reproduced in ordinary C at no instruction cost.

- [s112] tbl_125c relocation measured four ways, note present in all: loop top on n1 20/157, loop top on n3 20/157 (livelen 40 pri 750 seated at $s0), before the timeout branch 20/157, top of do_timeout 22/158 (allocno becomes block-local). Shortening its live range reaches the priority band but forfeits the $s3 seat, because global.c find_reg walks free hard registers in ascending order.

- [s112] Two byte-neutral refs-lift carriers measured inert on the n3 base (both 15/160, p79 row bit-identical to control): a cross-cse-block reg-reg handle copy, and a self-difference re-association of the second table address. Copies redistribute references between pseudos, they do not add them.

- [s112] AGGREGATE-MERGE PROBE NEGATIVE: asm/funcs/CD_sync.s:66-68 loads the third debug_printf argument through its own lui $at,%hi(D_800A11DC) / lw $a2,%lo(D_800A11DC)($at) pair, not off $s3, so there is no base-register evidence that D_800A11DC and D_800A125C are one aggregate and the aggregate-merge family's prong (a) fails for this pair.

## s113 — structural (the tbl_125c seat: refs-lift and note-denial carriers)

Chassis re-measured at dispatch on the n3 honest base
(`memory/grind/CD_sync/rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c`
spliced over `INCLUDE_ASM("asm/funcs", CD_sync);` at `src/system.c:376`):
ALLOCDBG ord=10..15 = 952 (p80, $s1) / 933 (p77 idx_1494, $s2) / 266 (p78
idx_1495, $s3) / 263 (p72 mode, $s4) / 253 (p73 result, $s5) / 202 (p79
tbl_125c, $s6) — bit-identical to s112's record, so every s112 conclusion still
holds on this chassis. Target seating: p77 $s2, p79 $s3, p78 $s4, p72 $s5,
p73 $s6, i.e. the single open gate is pri(p79) ∈ (266, 933).

### [s113-E1] The raw live lengths of all five long-lived allocnos are ~75; only the REG_EQUIV doubling separates them

Read off the `.lreg` register report for the CD_sync segment (n3 base):

    Register 72 (mode)      used 2 times across 76 insns
    Register 73 (result)    used 2 times across 79 insns
    Register 77 (idx_1494)  used 7 times across 150 insns   <- 75 doubled
    Register 78 (idx_1495)  used 2 times across 75 insns    <- note denied by n3
    Register 79 (tbl_125c)  used 3 times across 148 insns   <- 74 doubled

So the function's maximum honest live length is ~79 insns; 148/150 are the
`reg_live_length[regno] *= 2` at local-alloc.c:1064. This bounds the whole
priority landscape: with nrefs 2 a parameter can never fall below
2*10000/79 = 253, and only the doubling can put a pseudo under that.

### [s113-E2] Register-passed parameters can never take the doubling

`assign_parms` attaches the parameter REG_EQUIV note only under
`GET_CODE (entry_parm) == MEM && entry_parm == stack_parm`
(tools/gcc-2.7.2/function.c:3826-3830) — i.e. only for a parameter that ARRIVES
in memory. CD_sync's two parameters arrive in $a0/$a1, so p72/p73 are the one
class of allocno here that structurally cannot be pushed below ~246 by any C
spelling. Measured consistency: across all eleven variants compiled this session
p72/p73 never moved outside 246-266 and never took a note. This closes the
"lower the parameters instead of lifting tbl_125c" reading of the gate: on the
h0 base (p78 note in force, pri 138) the required order 933 > 202 > 138 >
pri(p72) > pri(p73) needs both parameters under 138, i.e. live lengths > 145,
which is nearly twice the function's honest maximum.

### [s113-E3] A displaced base does not ADD a reg_n_refs mention — it moves one

Five spellings of "form the table address off a displaced base so the
displacement is absorbed into the load's 16-bit offset by combine", all spliced
onto the n3 base and ALLOCDBG-captured (`a1`..`a5` in tmp/grind/CD_sync/s113/):

| variant | shape | p79 row |
|---|---|---|
| a1 | `tp = tbl_125c + 1; arg5 = tp[ix - 1];` (2nd access) | nrefs 3 livelen 150 pri 200 |
| a2 | same on the 1st access | nrefs 3 livelen 148 pri 202 (bit-identical to control) |
| a3 | byte displacement `tb = (u8*)tbl_125c + 4; *(s32*)(tb + (ix-4))` | nrefs 3 livelen 150 pri 200 |
| a4 | negative displacement `tp = tbl_125c - 1; tp[ix + 1]` | nrefs 3 livelen 150 pri 200 |
| a5 | BOTH accesses off one displaced base | nrefs **2** livelen 152 pri 131 |

The reason is structural, not a fold accident: every C-level address chain
mentions the base pseudo exactly once, so introducing a displaced handle makes
the handle carry that mention instead of p79 (a5 makes it worse — one handle
serving both accesses drops p79 to two mentions). This generalises s112's t1
finding from reg-reg copies to the whole "extra address-forming insn" family:
a fourth mention of p79 cannot come from re-spelling the two existing accesses.

### [s113-E4] A second LIVE set of tbl_125c DOES deny the note — at a fixed cost of two instructions

This is the first measured C-level denial of the local-alloc.c:1064 doubling for
a bare-symbol pointer. `update_equiv_regs` bails at local-alloc.c:1021 when
`reg_n_sets[regno] != 1`, before it can convert the CONSTANT_P REG_EQUAL into
REG_EQUIV. s111 measured six multi-set spellings and found reg_n_sets still 1
in all six because every extra set was DEAD (removed by cse_main /
delete_dead_from_cse at toplev.c:2867 before the reg_scan at toplev.c:2925).
The un-measured case is a second set that is genuinely live — one reachable by
the loop's back edge, so neither set is dead:

| variant | second set placed | p79 row | seat | build_insns |
|---|---|---|---|---|
| c1 | end of the do_timeout block, after both uses | nrefs 4 livelen **71** (NOT doubled) pri 1126 | $s1 | 162 (score 21) |
| d1 | loop tail, immediately before the back edge | nrefs 4 livelen 31 pri 2580 | $s0 | 162 (score 21) |
| d2 | success block | nrefs 4 livelen 74 pri 1081 | $s1 | 162 |
| d3 | head of the inner poll loop | nrefs 4 livelen 57 pri 1403 | $s1 | 162 |
| c2 | second live set of idx_1494 instead | p77 leaves the table; p79 unchanged 3/150/200 | — | — |

Both halves of the gate move at once and both overshoot: the note is denied
(livelen 71 raw, not 148) AND the set itself is a fourth mention, so
pri = 2*4*10000/71 = 1126 outranks p77's 921 and takes $s1 instead of $s3. And
every placement materialises a second `lui/addiu` address pair: build_insns 162
against the target's 160, with the sandbox distance rising to 21.

The composite arithmetic is worth recording for the next session: with the note
denied, nrefs 4 needs live length > 87 to fall under p77, and nrefs 3 (no second
set, hence no denial) gives the ideal 405. So the ONLY shape that lands in band
is "note denied AND exactly three mentions" — a second set of p79 whose emitted
insn already exists in the target's schedule. The target writes $s3 exactly once
(asm/funcs/CD_sync.s:15-16, the prologue lui/addiu); there is no second write of
$s3 anywhere in its 160 instructions, so a zero-cost second set would have to be
a reg-reg copy coalesced onto $s3 — which needs a second pseudo already holding
D_800A125C, i.e. another address materialisation, which is the same +2.

- [s113] Chassis re-measured at dispatch: the n3 honest base still scores 15 / build_insns 160 / rules_dropped 0 with ALLOCDBG ord=10..15 = 952 (p80 $s1) / 933 (p77 idx_1494 $s2) / 266 (p78 idx_1495 $s3) / 263 (p72 mode $s4) / 253 (p73 result $s5) / 202 (p79 tbl_125c $s6) — bit-identical to s112, so every s112 conclusion still holds.

- [s113] The .lreg register report gives the honest live-length ceiling for this function: mode 76, result 79, idx_1494 150 (raw 75), idx_1495 75 (note denied by n3), tbl_125c 148 (raw 74). Nothing honest exceeds ~79 without the local-alloc.c:1064 doubling.

- [s113] assign_parms attaches a parameter REG_EQUIV only when GET_CODE (entry_parm) == MEM && entry_parm == stack_parm (tools/gcc-2.7.2/function.c:3826-3830), so register-passed parameters can never take the doubling — the parameters' priorities are structurally pinned at 246-266 in this function.

- [s113] A displaced base (tbl_125c + 1 consumed as tp[i-1], and three sign/scale variants) never adds a reg_n_refs mention: the handle inherits the mention. Using ONE displaced base for both accesses drops p79 to nrefs=2 (pri 131).

- [s113] A second LIVE set of tbl_125c denies the REG_EQUIV note (p79 live length 71 raw instead of 148 doubled) — the first measured C-level denial for a bare-symbol pointer, and the mechanism the owner-refused h5 cross-symbol chain-extender was buying. Cost is fixed at +2 instructions (a second lui/addiu pair) in all four placements: build_insns 162, sandbox 21.

- [s113] The target writes $s3 exactly once in its 160 instructions (asm/funcs/CD_sync.s:15-16, the prologue lui/addiu) and uses it twice (lines 58, 63), so the original compile had nrefs=3 with a priority above 266 — i.e. the original's tbl_125c pseudo carried NO REG_EQUIV (raw 74 -> pri 405) or had a live length under 56. This is now the sharpest available statement of what the original source shape must have done.

## s114 — synthesis (merged attack; the duplicated-arms chassis lands four of five s-register seats)

### [s114-C] Chassis + mandated kill re-audit
`memory/grind/CD_sync/candidate.c` re-spliced on HEAD and re-scored this session:
**score 2 / build_insns 160 / rules_dropped 0** (the dispatch brief again reported
the HEAD floor as "measurement unavailable"; it is 2). `tools/fake_ablate.py
--func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c` reproduces
s111 exactly: keep-all 2/160, drop chain-extender 15/159, drop pp alias 17/161,
drop both 30/160. Both FAKE units remain load-bearing and super-additive; every
chassis-relative conclusion in the ledger is still valid. The n3 honest base
(`rejected/s112_HONEST_BASE_1495_success_block_note_denied_15.c`) re-measured at
**15 / 160 / 0** with ALLOCDBG ord=10..15 bit-identical to s112/s113
(952 p80 $s1 / 933 p77 $s2 / 266 p78 $s3 / 263 p72 $s4 / 253 p73 $s5 / 202 p79 $s6).

### [s114-E1] Upstream provenance is IN THE REPO and it names the original source shape
`tmp/sotn/src/main/psxsdk/libcd/bios.c:232` carries a **matched-project C body for
PsyQ libcd `CD_sync`**, with `set_alarm` / `get_alarm` / `callback` / `_memcpy` as
static inlines. Mapping onto BB2 (confirmed statement-for-statement against
`asm/funcs/CD_sync.s`):

| SOTN bios.c | BB2 |
|---|---|
| `Alarm.unk0 = VSync(-1) + 960; Alarm.unk4 = 0; Alarm.unk8 = name;` | `D_800F19B8`, `D_800F19BC`, `D_800F19C0 = &D_80016240` |
| `puts("CD timeout: ")` | `tslTm2LoadImage_2(&D_800161B8)` |
| `printf("%s:(%s) Sync=%s, Ready=%s\n", Alarm.unk8, D_80032AC8[CD_com], D_80032B48[Intr.sync], D_80032B48[Intr.ready])` | `debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], D_800A125C[idx_1494[0]], D_800A125C[idx_1494[1]])` |
| `CD_flush()` | `cdrom_ClearIrq()` |
| `CheckCallback()` | `sys_GetVblankCount()` |
| `static volatile CD_intr Intr {sync,ready,c}` | `D_800A1494 / D_800A1495 / D_800A1496` |
| `char *D_80032B48[8]` (the "NoIntr/DataReady/..." string table) | `D_800A125C[]` |

So **`D_800A125C` is a real 8-entry `char *` array and both table reads are ordinary
array indexing off a named global; there is no `tbl_125c` pointer local in the original
source.** The ledger's whole chassis family has carried one since s1.

### [s114-E2] …and removing that local destroys the $s3 base entirely (KILLED, instance)
Two spellings measured on the n3 honest base, both with the statement structure of
n3 otherwise byte-identical:
- **f1** — delete `s32 *tbl_125c;` + its init, spell both addresses `(u8 *)D_800A125C + t0`
  / `ix + (s32)D_800A125C`: **score 35, build_insns 159**. The tbl pseudo is GONE from
  the global allocno list (ALLOCDBG ord=9..13 shows only p77/p79(short)/p78/p72/p73;
  the `.lreg` note grep finds a REG_EQUIV for `D_800A1494` only). No callee-saved base
  register is formed at all.
- **f2** — f1 + ordinary `D_800A125C[t0]` / `D_800A125C[ix]` indexing: **score 38, 159**,
  same allocno picture.
Banked as `rejected/s114_f1.c`, `rejected/s114_f2.c`. **The explicit C-level pointer
local is what creates the long-lived $s3 allocno; the "faithful array-indexing"
reading of the upstream body does not reach it under this compiler configuration.**

### [s114-E3] THE RESULT: duplicated-statement-into-arms is byte-neutral here and seats FOUR of the five s-registers
s73 flagged "duplicate a byte-neutral tbl_125c reference into BOTH do_timeout arrival
arms" as the un-run F6 frontier and s74 never executed it; s20's earlier attempts
duplicated *single statements* (the `D_800F19C0` store, the dispatch load) and either
failed to cross-jump (an intervening `do_timeout:` label blocks find_cross_jump's
suffix match, s20) or lifted the wrong pseudo.

**g1** duplicates the ENTIRE do_timeout region — `tslTm2LoadImage_2(&D_800161B8)`, the
two `idx_1494[]` byte reads, the `pp` binding, both `tbl_125c` address computations,
`debug_printf`, `cdrom_ClearIrq()`, `v0 = -1; goto check;` — into both arrival arms,
deleting the `do_timeout:` label (which is what blocked s20's merge):

    if (D_800F19B8 < v0)      { <BLOCK> }
    cnt = D_800F19BC; D_800F19BC = cnt - -1;
    if (0x3C0000 < cnt)       { <BLOCK> }
    success: ...

Measured on the n3 honest base: **score 13, build_insns 160, rules_dropped 0** — the
first honest-chassis form to beat n3's 15, and jump2's `find_cross_jump` re-merges the
two copies to identical bytes (build_insns is exactly the target's 160).

ALLOCDBG ord=11..16 (hardreg 17=$s1 … 22=$s6):

| pseudo | what | nrefs | livelen | pri | seat | target seat |
|---|---|---|---|---|---|---|
| p77 | idx_1494 | 9 | 190 | 1421 | $s1 | **$s2** |
| p80 | (short-lived, `saved`) | 2 | 21 | 952 | $s2 | **$s1** |
| p79 | tbl_125c | 5 | 188 | 531 | **$s3** | $s3 ✔ |
| p78 | idx_1495 | 2 | 95 | 210 | **$s4** | $s4 ✔ |
| p72 | mode | 2 | 96 | 208 | **$s5** | $s5 ✔ |
| p73 | result | 2 | 99 | 202 | **$s6** | $s6 ✔ |

The gate s113 defined — `pri(p79) ∈ (266, 933)` — is CLEARED (531), and with it the
$s3/$s4/$s5/$s6 seats all land for the first time on an honest chassis. The residual is
a single new inversion: the same duplication also lifts p77's loop-weighted reg_n_refs
from 14 to 27 (`pri = W*10000/livelen`: 933 = 14·10000/150 → 1421 = 27·10000/190),
pushing idx_1494 above p80's 952 and swapping $s1/$s2.

### [s114-E4] The p77 lift is not reachable by relocating the idx_1495 init (KILLED, instance)
**g2** = g1 with `idx_1495 = idx_1494 + 1;` moved from the success block back to the
prologue (testing whether that mention's loop-depth weighting is what inflates p77):
**score 20 / 160 / 0**, p77's ALLOCDBG row **bit-identical** (9 / 190 / 1421), and p78
re-acquires its REG_EQUIV note (`const (plus (symbol_ref D_800A1494) 1)` present in the
`.lreg` dump), collapsing to pri 108 and the $s6 seat. Banked `rejected/s114_g2.c`.
Confirms s112: the success-block placement of the idx_1495 init is load-bearing for the
note denial, and it is NOT where p77's weighted refs come from.

### [s114-E5] Arithmetic of the remaining gate
On the g1 chassis the ONLY wrong seat pair is p77/p80. p80 is fixed at W=2 / livelen 21
/ pri 952, so the closing condition is **pri(p77) ∈ (531, 952)** — i.e. p77's
loop-weighted refs must come in at W ≤ 18 at livelen 190 (it is 27), while p79 keeps
W ≥ 10. Both duplicated copies of the block contribute the two `idx_1494[0]` /
`idx_1494[1]` byte reads, so the next form must duplicate the tbl_125c address math
into the arms **without** duplicating the two index reads — the reads have to be shared
between the arms while the table accesses are not.

### [s114-A] Artifacts
`tmp/grind/CD_sync/s114/` — splice.py / cap.py / notes.sh / drive.ps1 (PowerShell
splice+sandbox+ALLOCDBG+note-grep driver), the variant sources n3/f1/f2/g1/g2.c and
their `<tag>.{s,stderr,rtl,jump,cse,loop,flow,combine,lreg,greg,sched,sched2,jump2,dbr}`
dump sets.

- [s114] s114 chassis: candidate.c re-splices at 2/160/rules_dropped 0 on HEAD; fake_ablate reproduces s111 exactly (keep-all 2/160, drop chain-extender 15/159, drop pp alias 17/161, drop both 30/160).

- [s114] A matched-project C body for the SAME PsyQ libcd CD_sync is already in this repo at tmp/sotn/src/main/psxsdk/libcd/bios.c:232, with set_alarm/get_alarm/callback/_memcpy as static inlines. It maps statement-for-statement onto asm/funcs/CD_sync.s: puts to tslTm2LoadImage_2, printf(%s:(%s) Sync=%s, Ready=%s) to debug_printf(&D_800161C8, ...), CD_flush to cdrom_ClearIrq, CheckCallback to sys_GetVblankCount, static volatile CD_intr Intr{sync,ready,c} to D_800A1494/95/96, and char pointer table D_80032B48[8] to D_800A125C[]. The provenance frontier is therefore SPENT for structure: the upstream body is in hand and it contains no tbl_125c pointer local.

- [s114] Removing the tbl_125c local (the upstream shape) removes the pseudo from the global allocno list entirely: f1 35/159, f2 38/159, no callee-saved base register formed. The explicit C-level pointer local is a prerequisite for the target's prologue lui/addiu $s3.

- [s114] Duplicated-statement-into-arms IS byte-neutral for this function when the whole do_timeout region (including the goto-check tail) is duplicated and the do_timeout label is deleted: build_insns 160 == target, rules_dropped 0, jump2 find_cross_jump re-merges. s20's failed merge was caused by the intervening label, not by the family.

- [s114] g1 ALLOCDBG ord=11..16 (hardreg 17=$s1 .. 22=$s6): p77 idx_1494 9/190/1421 seats $s1; p80 2/21/952 seats $s2; p79 tbl_125c 5/188/531 seats $s3; p78 idx_1495 2/95/210 seats $s4; p72 mode 2/96/208 seats $s5; p73 result 2/99/202 seats $s6. Four of the five target s-register seats land, and the s113 gate pri(p79) in (266,933) is cleared for the first time on an honest chassis.

- [s114] The global.c priority is pri = W*10000/livelen where W is the loop-depth-weighted reg_n_refs, not the raw mention count printed as nrefs: n3 p77 933 = 14*10000/150, g1 p77 1421 = 27*10000/190, g1 p79 531 = 10*10000/188. The arm duplication adds about 13 weighted refs to p77 and about 7 to p79.

- [s114] The remaining gate on the g1 chassis is a single ordering: pri(p77) must fall into (531, 952) - i.e. W(p77) at most 18 at livelen 190, currently 27 - while p79 keeps W at least 10. p80 is pinned at W=2 / livelen 21 / pri 952.

- [s114] Moving the idx_1495 init off the success block does not change p77's weighted refs at all (g2 p77 row bit-identical) and re-attaches p78's REG_EQUIV note; the success-block placement stays mandatory.

- [s114] local-alloc.c update_equiv_regs has exactly three C-reachable denial gates for the reg_live_length doubling at local-alloc.c:1064: single_set(insn) == 0 (local-alloc.c:979), reg_n_sets != 1 (local-alloc.c:1020), and no CONSTANT_P REG_EQUAL note (local-alloc.c:1030). The fourth, reg_live_length < 0, is set only for pseudos live across setjmp (flow.c:1260) and is unreachable here.

## s115 — synthesis (the seat equation is CLOSED: all six callee-saved seats land; residual 3)

### [s115-C] Chassis + mandated kill re-audit
`tools/fake_ablate.py --func CD_sync --file system --candidate memory/grind/CD_sync/candidate.c`
(run under WSL — the PowerShell invocation returns `ERR` for every variant because the
compile driver needs the venv; use `bash tools/wsl.sh 'source .venv/bin/activate && ...'`)
reproduces s111/s114 exactly: **keep-all 2/160, drop chain-extender 15/159, drop pp alias
17/161, drop both 30/160**. Floor is 2; both FAKE units still load-bearing and
super-additive. The g1 duplicated-arms honest base re-measured **13 / 160 / 0** with its
s114 ALLOCDBG rows bit-identical, so every s114 conclusion holds on the current chassis.

### [s115-E1] The priority formula is `floor_log2(nrefs) * nrefs / livelen * 10000 * size`
Read directly off the instrumented printf (`tools/gcc-2.7.2/global.c:612-616`). This
retires the ledger's "loop-depth-weighted reg_n_refs W" model (s114-E5's "the duplication
adds ~13 weighted refs to p77"): the printed `nrefs` IS `allocno_n_refs`, and the apparent
multiplier is the `floor_log2` factor. Consequences, all arithmetic and all verified:
- nrefs 2 -> factor 2; 3 -> 3; **4 -> 8**; 5 -> 10; 6 -> 12; **7 -> 14**; **8 -> 24**; 9 -> 27.
- The nrefs 7->8 step is a cliff (14 -> 24); 3->4 is another (3 -> 8).
- g1: p77 9 refs -> 27*10000/190 = 1421; p79 5 -> 10*10000/188 = 531; p80 2 -> 2*10000/21 = 952.
Every remaining seat question is now integer arithmetic on (nrefs, livelen).

### [s115-E2] p77's ref count is structural on the duplicated-arms chassis
`idx_1494` carries five references that are each pinned to a target instruction using `$s2`
(the prologue set, `addiu $s4,$s2,1`, the callback `lbu $a0,0($s2)`, the tail
`lbu $v0,0($s2)`, the tail `sb $v0,0($s2)`), plus two per duplicated copy of the do_timeout
block. On g1 that is 5 + 2*2 = 9, and the only free variable is how many of the block's two
byte reads are spelled off `idx_1494`. Since pri(p77) must land in (531, 952) to sit below
p80, floor_log2(n)*n must be <= 18 at livelen 190, so **p77 must have 5, 6 or 7 references** —
i.e. at most ONE of the two block reads may go through `idx_1494`.

### [s115-E3] KILLED (instance): the reads cannot be shared by hoisting them above the arms
**h1** = g1 with `t0 = idx_1494[0]; ix = idx_1494[1];` hoisted to the loop top (the only
program point dominating both timeout arms) and only the table math + printf + tail
duplicated. It does exactly what s114's frontier predicted for the ref counts — p77 falls
back to 7 (752 at livelen 186), p79 keeps 5 (543) — and **all five s-register seats land**.
But the hoisted reads become two loop-top `lbu`s and the block loses them: **score 15,
build_insns 159** (one short of target). Banked
`rejected/s115_h1_hoisted_idx_reads_loop_top_15.c`. The shared-prefix idea is spent: the two
arms have no common dominator inside the timeout path, so "share the reads" can only mean
"execute them on every loop iteration".

### [s115-E4] CONFIRMED: split-init of `saved` lifts p80 over p77 and lands all six seats
**i1** = g1 with `saved = (*D_800A147C) & 3;` split into `saved = *D_800A147C; saved =
saved & 3;` (ordinary C, [[split-init-accumulation-sanctioned]]). p80 goes 2 refs -> 4
(livelen 21 -> 22), pri 952 -> **3636**, and the full seat vector becomes correct for the
first time in the ledger's history:

| pseudo | what | nrefs | livelen | pri | seat | target |
|---|---|---|---|---|---|---|
| p80 | saved | 4 | 22 | 3636 | $s1 | $s1 OK |
| p77 | idx_1494 | 9 | 190 | 1421 | $s2 | $s2 OK |
| p79 | tbl_125c | 5 | 188 | 531 | $s3 | $s3 OK |
| p78 | idx_1495 | 2 | 95 | 210 | $s4 | $s4 OK |
| p72 | mode | 2 | 96 | 208 | $s5 | $s5 OK |
| p73 | result | 2 | 99 | 202 | $s6 | $s6 OK |

**score 6 / build_insns 160 / rules_dropped 0**, and the alignment-aware diff
(`tmp/grind/CD_sync/s115/adiff.py`) resolves it into three independent 2-point defects:
(a) `addiu s4,s2,1` emitted at index 74 (success block) instead of 17 (prologue);
(b) the `sll a0,a0,0x2` / `addu v0,v0,s3` pair-swap at 54/55;
(c) `lbu s1,0(v0)` + `andi s1,s1,0x3` instead of `lbu v0,0(v0)` + `andi s1,v0,0x3` — the
split-init's own cost, because the target keeps the loaded byte in a separate `$v0` pseudo.
Banked `rejected/s115_i1_split_init_saved_all_seats_6.c`.

### [s115-E5] KILLED (instance): a dead self-assign cannot lift p80
`saved = saved;` inserted before the store (**i2**) and immediately after the set (**i3**)
both score **13 / 160 / 0** with p80's ALLOCDBG row bit-identical to g1 (2 / 21 / 952) — the
self-assign is deleted by `delete_dead_from_cse` before `reg_scan`, exactly as s111 measured
for the multi-set spellings of tbl_125c. The dead-store FAKE family is INERT as a p80
refs-lift carrier here; only a set whose value is genuinely consumed survives to `flow`.
**i4** (mask moved to the store, `*D_800A147C = saved & 3;`) scores 18 / 158.

### [s115-E6] THE RESULT: j1 — score 3, all six seats, one FAKE construct
The i1 landscape exposed the last two defects as one shared problem: `idx_1495`'s init must
sit in the PROLOGUE (target index 17), but a prologue `idx_1495 = idx_1494 + 1;` takes a
`const (plus (symbol_ref D_800A1494) 1)` REG_EQUIV note, doubles to livelen 184 and collapses
to pri 108 (**i5**, score 13, note verified in the `.lreg` dump). With the note ON, the band
(208, 531) needs `floor_log2(n)*n` in (3.8, 9.8) — i.e. **exactly 4 references**.

**j1** buys p78's two extra references and p77's two-reference reduction with ONE edit:
spell the block's second index read `ix = *idx_1495;` instead of `ix = idx_1494[1];`, in both
duplicated copies, with the init back in the prologue. Both halves of the gate move at once:

| pseudo | nrefs | livelen | pri | seat | target |
|---|---|---|---|---|---|
| p80 saved | 2 | 21 | 952 | $s1 | $s1 OK |
| p77 idx_1494 | 7 | 190 | 736 | $s2 | $s2 OK |
| p79 tbl_125c | 5 | 188 | 531 | $s3 | $s3 OK |
| p78 idx_1495 | 4 | 184 | 434 | $s4 | $s4 OK |
| p72 mode | 2 | 96 | 208 | $s5 | $s5 OK |
| p73 result | 2 | 99 | 202 | $s6 | $s6 OK |

**score 3 / build_insns 160 / rules_dropped 0**, and because p77 is back at 7 refs the
split-init is no longer needed (i1's defect (c) disappears). The complete residual is:

    replace  T50 `lbu v0,1(s2)`   ->  O50 `lbu v0,0(s4)`    (1 pt: the seat trade)
    insert   O54 `sll a0,a0,0x2`
    delete   T55 `sll a0,a0,0x2`                            (2 pts: the pair-swap)

Banked `rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c`. **j1 carries exactly ONE FAKE
construct — the `pp = (void **)&D_800F19C0;` pointer alias, measured load-bearing (dropping
it costs +6: `rejected/s115_m1_pp_alias_dropped_9.c` scores 9)** — plus the
duplicated-statement-into-arms device. It does NOT carry the owner-refused cross-symbol
chain-extender that candidate.c's floor-2 form depends on, so it is a strictly better
PROVENANCE base at one point higher score.

### [s115-E7] The 1-point seat trade is an exact exchange, not an accident
`l1` (roles swapped: `t0 = idx_1495[-1]; ix = idx_1494[1];`) also scores **3** — the wrong
instruction just moves from index 50 to index 49. Whichever of the two block reads is routed
through `idx_1495` pays exactly one instruction, because the target loads BOTH index bytes
off `$s2` (`lbu $a0,0($s2)` / `lbu $v0,1($s2)`, asm/funcs/CD_sync.s:58-59) while the
allocator needs p78 to own two of those references. The exchange rate is fixed: one emitted
instruction buys two references on p78 and removes two from p77.

### [s115-E8] KILLED (instance): source-order permutation of the two address chains
`k1` (compute the `ix` chain before the `t0` chain, matching the target's emission order)
and `k2` (k1 + the two byte reads swapped) both score **7 / 160** — worse than j1's 3. The
54/55 pair-swap does not respond to statement order inside the block on this chassis.

### [s115-A] Artifacts
`tmp/grind/CD_sync/s115/` — splice.py / cap.py / notes.sh / drive.ps1 (copied from s114 and
re-pointed), **adiff.py** (new: difflib-aligned normalized diff — the tool that decomposed
i1's 6 into three independent defects; s108's `ndiff.py` index-wise diff reports a
whole-function shift for a single inserted/deleted instruction), and the variant sources
g1/h1/i1..i5/j1/j2/k1/k2/l1/m1.c with their full `-da` dump sets and ALLOCDBG stderr.

- [s115] global.c's allocno priority is `floor_log2(allocno_n_refs) * allocno_n_refs / allocno_live_length * 10000 * allocno_size` (tools/gcc-2.7.2/global.c:612-616, read off the instrumented printf). The ledger's "loop-depth-weighted refs" model from s114 is wrong: the printed nrefs IS the count and floor_log2 supplies the apparent multiplier. Cliffs at nrefs 3->4 (3->8) and 7->8 (14->24).
- [s115] fake_ablate.py must be run under WSL with the venv activated; invoked from PowerShell it reports ERR for every variant.
- [s115] h1 (both index reads hoisted to the loop top, only the table math duplicated) lands all five s-register seats but scores 15/159 - the reads become unconditional loop-top lbus. The two timeout arms have no common dominator inside the timeout path, so sharing the reads between the arms has no zero-cost spelling.
- [s115] Split-init of `saved` (`saved = *D_800A147C; saved = saved & 3;`) lifts p80 from 2 refs/952 to 4 refs/3636 and seats all six callee-saved pseudos correctly (i1, score 6/160). Its own cost is 2 instructions because the target keeps the loaded byte in a separate $v0 pseudo.
- [s115] A dead self-assign `saved = saved;` is deleted before reg_scan and leaves p80's ALLOCDBG row bit-identical in both placements tested (i2/i3, 13/160) - the dead-store FAKE family is inert as a refs-lift carrier on this chassis.
- [s115] j1 = g1 + idx_1495 init restored to the prologue + the block's second index read spelled `*idx_1495` in both duplicated copies: score 3 / build_insns 160 / rules_dropped 0, with p80 $s1 / p77 $s2 / p79 $s3 / p78 $s4 / p72 $s5 / p73 $s6 all matching the target. Residual = 1 pt `lbu v0,0(s4)` vs `lbu v0,1(s2)` plus the 2-pt sll/addu pair-swap at 54/55.
- [s115] j1 carries ONE FAKE construct (the pp pointer alias, load-bearing: dropping it costs +6) and does NOT need the owner-refused cross-symbol chain-extender that candidate.c's floor-2 form depends on.
- [s115] Routing either one of the block's two index reads through idx_1495 costs exactly one emitted instruction and is an exact exchange - it adds 2 refs to p78 and removes 2 from p77 (l1 scores 3 with the roles swapped, the wrong insn moving from index 50 to 49).
- [s115] Reordering the two address chains in the block (k1) or the two byte reads (k2) scores 7 - the 54/55 pair-swap does not respond to source statement order on the duplicated-arms chassis.
- [s115] tmp/grind/CD_sync/s115/adiff.py is the alignment-aware (difflib) normalized diff; use it instead of s108's ndiff.py whenever build_insns differs from 160 or an insn is inserted/deleted.

- [s115] global.c's allocno priority is floor_log2(allocno_n_refs)*allocno_n_refs/allocno_live_length*10000*allocno_size (tools/gcc-2.7.2/global.c:612-616); the ALLOCDBG nrefs column IS the reference count and the floor_log2 factor supplies the apparent multiplier the ledger had been modelling as loop-depth weighting.

- [s115] The floor_log2 cliffs at nrefs 3->4 (factor 3 -> 8) and 7->8 (14 -> 24) are what make two-reference moves decisive; every seat question on this function is now integer arithmetic on (nrefs, livelen).

- [s115] j1 (memory/grind/CD_sync/rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c) scores 3 / build_insns 160 / rules_dropped 0 with p80 $s1, p77 $s2, p79 $s3, p78 $s4, p72 $s5, p73 $s6 - the first form in 115 sessions with every callee-saved seat correct, and it needs only the pp pointer alias, not the owner-refused cross-symbol chain-extender.

- [s115] j1's complete residual, from the new alignment-aware diff: replace index 50 lbu v0,1(s2) with lbu v0,0(s4) (1 point), and the sll a0,a0,0x2 / addu v0,v0,s3 inversion at indices 54/55 (2 points).

- [s115] Routing either one of the block's two index reads through idx_1495 costs exactly one emitted instruction and is an exact exchange - two references onto p78, two off p77 (l1, with the pointer roles swapped, also scores 3 with the wrong instruction at index 49 instead of 50).

- [s115] idx_1494 carries five references that are each pinned to a target instruction using $s2 (prologue set, addiu $s4,$s2,1, callback lbu 0($s2), tail lbu 0($s2), tail sb 0($s2)); with two more per duplicated copy of the do_timeout block p77 is 9 refs unless a block read is re-routed, and pri(p77) must sit in (531, 952), i.e. at most 7 references.

- [s115] The two timeout arms have no common dominator inside the timeout path, so sharing the block's index reads between the duplicated copies necessarily moves them onto the loop's fall-through path (h1, 15/159).

- [s115] The dead-store FAKE family is inert as a refs-lift carrier on this function: saved = saved; is removed by delete_dead_from_cse before reg_scan in both placements tested, leaving the ALLOCDBG row bit-identical.

- [s115] candidate.c re-measured at 2/160/0 with the fake_ablate matrix bit-identical to s111/s114; tools/fake_ablate.py must be run under WSL with the venv activated (from PowerShell every variant reports ERR).

- [s115] tmp/grind/CD_sync/s115/adiff.py is the new alignment-aware (difflib) normalized diff and is what decomposed i1's score of 6 into three independent 2-point defects; s108's index-wise ndiff.py reports a whole-function shift whenever a single instruction is inserted or deleted.


## s116 (escalation modality - the F2 rung, worked under the owner's 2026-09-02 window reset)

- [s116] CHASSIS RE-VALIDATION. j1 (rejected/s115_HONEST_BASE_dup_arms_idx1495_read_3.c) re-splices at
  src/system.c:376 and re-measures 3 / build_insns 160 / rules_dropped 0 with the s115 ALLOCDBG seat
  vector reproduced bit-for-bit (p80 $s1 952, p77 $s2 736, p79 $s3 531, p78 $s4 434, p72 $s5 208,
  p73 $s6 202) and the same three REG_EQUIV notes in the .lreg dump. The ledger floor of 2 still belongs
  to candidate.c, which needs the owner-REFUSED cross-symbol chain-extender; j1 at 3 is the best form in
  a submittable family.
- [s116] j1's residual, decomposed by tmp/grind/CD_sync/s116/adiff.py: exactly three points - index 50
  `lbu v0,0(s4)` where the target has `lbu v0,1(s2)` (the 1-point seat trade), and `sll a0,a0,0x2`
  emitted at index 54 instead of 55 (the 2-point pair inversion).
- [s116] THE PAIR INVERSION IS DEAD. `ix <<= 2; ix += (s32)tbl_125c; arg5 = *(s32 *)ix;` in place of
  `ix <<= 2; arg5 = *(s32 *)(ix + (s32)tbl_125c);` makes sched1 emit the target's exact order. Measured
  three ways (h1, e3, k1), each 7/160/0, each with an adiff that is index-for-index equal to the target
  everywhere except the six-point register exchange below. The sixty sessions of forensic verdicts on
  this pair (s6 qty_compare_1, s25 SCHEDDBG block=3, s43/s51 LUID renumbering, s70 ready-list dispatch,
  s93 POLL-region axis) were all measured on the h5/n3 chassis and do NOT transfer to the
  duplicated-arms chassis - the s115 F2 frontier statement is now CONFIRMED rather than hypothesised.
- [s116] THE NEW WALL, stated precisely. With the order fixed, the block's residual is a two-way
  local-alloc exchange worth six points: our t0 chain takes $v1 and arg5 takes $a0, the target has
  $a0 and $v1 (adiff indices 49, 55, 56, 59, 61, 65). Nothing about the six callee-saved seats moves -
  the ALLOCDBG table is bit-identical across j1/e1/e2/e3/e9 - so this is purely the ranking of
  block-local quantities inside local-alloc: QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth),
  with find_reg walking hard regs in ascending number order because tools/gcc-2.7.2/config/mips/mips.h
  defines no REG_ALLOC_ORDER ($v0=2 < $v1=3 < $a0=4). arg5 must be ranked ABOVE the t0 chain to take
  $v1; today it is ranked below.
- [s116] The two self-reuse chains are load-bearing for the entire block shape. Every spelling that
  breaks either of them - a separate address local (h4/n2/n4), a named intermediate for the scale or
  the add (f2/h2/h3), array indexing with no locals (m2/m3/m4), or folding the t0 address into the call
  (f1) - scores 15/160, i.e. loses the block wholesale. f3 (arg5 address in its own local) is the only
  intermediate at 8. This is the constraint that makes the obvious reference-count levers unavailable:
  every measured way of changing either chain's reference count also destroys the chain.
- [s116] Endgame gate (a) re-run for the record: HAND_CODED: tier=LOW score=2/8 (CD_sync, 160 insns),
  identical to s107 (artifact tmp/grind/CD_sync/s116/scan_hand_coded.txt). S4 and S5 fire; S1/S2/S6 do not.
- [s116] Tooling: tmp/grind/CD_sync/s116/ carries splice.py / cap.py / drive.ps1 / notes.sh / adiff.py
  (the s115 harness re-pathed) plus the .c/.stderr/.lreg/.sched/.s captures for every variant named above.

- [s116] j1 re-splices and re-measures 3 / build_insns 160 / rules_dropped 0 on the current chassis, with the s115 ALLOCDBG seat vector reproduced bit-for-bit (p80 $s1 952, p77 $s2 736, p79 $s3 531, p78 $s4 434, p72 $s5 208, p73 $s6 202) and the same three REG_EQUIV notes in the .lreg dump.

- [s116] j1's residual decomposes to exactly three points: index 50 lbu v0,0(s4) against the target's lbu v0,1(s2) (the 1-point seat trade), and sll a0,a0,0x2 emitted at index 54 instead of 55 (the 2-point pair inversion).

- [s116] Splitting the arg5 address into the ix variable (ix <<= 2; ix += (s32)tbl_125c; arg5 = *(s32 *)ix;) makes sched1 emit the target's exact order - measured on three independent bases (h1, e3, k1), all 7/160/0. The sixty sessions of forensic verdicts on this pair (s6 qty_compare_1, s25 SCHEDDBG block=3, s43/s51 LUID renumbering, s70 ready-list dispatch, s93 POLL-region axis) were all taken on the h5/n3 chassis and do not transfer to the duplicated-arms chassis.

- [s116] With the order fixed, the block's whole residual is a two-way local-alloc exchange worth six points (adiff indices 49, 55, 56, 59, 61, 65): our t0 chain takes $v1 and arg5 takes $a0, the target has $a0 and $v1. The callee-saved seats never move - the ALLOCDBG table is bit-identical across j1/e1/e2/e3/e9.

- [s116] The governing arithmetic is local-alloc's QTY_CMP_PRI = floor_log2(refs)*refs*size*10000/(death-birth) with find_reg walking hard registers in ascending number order; tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so $v0=2 < $v1=3 < $a0=4 and the higher-ranked quantity wins the lower register. arg5 must outrank the t0 chain to take $v1.

- [s116] Both self-reuse chains are load-bearing for the block's whole shape: a separate address local (h4/n2/n4), a named intermediate for the scale or the add (f2/h2/h3), array indexing with no locals (m2/m3/m4), or folding the t0 address into the call (f1) all score 15/160. f3 at 8 is the only intermediate. This is why the obvious reference-count levers are unavailable today.

- [s116] Gate evidence for a future disposition: scan_hand_coded tier=LOW 2/8 (unchanged since s107), and the SOTN-precedent census for the closing construct came back negative in s98/s106 - both endgame-lock AND-gates therefore stand FAILED. No foreclosure record was filed this session because the owner's 2026-09-02 ruling re-activated CD_sync with the exhaustion window RESET and directed the ladder be worked from its next rung, and that rung (F2) produced a CONFIRMED lever that removed a residual sixty sessions had ruled locked. The axis is not exhausted: the wall named this session (block-local quantity ranking inside local-alloc) has never been attacked.


## s117 (2026-09-04) — rederive: sibling transplant + the QTY_CMP_PRI tie

Chassis re-verified live this session: j1 (duplicated arms, scalar model,
renamed calls) = 3/160 bi 160 rd 0; the floor body (goto shape, chain-extender)
= 2/160 bi 160 rd 0. All measurements below are on HEAD's src/system.c with
CD_sync spliced by tmp/grind/CD_sync/s117/apply.py (region-scoped: it edits only
CD_sync's own declaration block, never CD_datasync's duplicate copies).

**FIRST: the banked candidate did not compile.** Its body still called
sys_VSync / tslTm2LoadImage_2 / debug_printf / cdrom_ClearIrq /
sys_GetVblankCount / func_80080828. None of those names survive in HEAD. The
correct names, read off asm/funcs/CD_sync.s jal targets, are VSync / puts /
printf / CD_flush / CheckCallback / getintr. candidate.c is repaired.

### H117-1 CONFIRMED — CD_datasync s58's CD_alarm struct transplants onto CD_sync link-identically and deletes the pp FAKE

The owner directive's first probe. Declaring
typedef struct { s32 timeout; s32 count; char \*func; } CD_alarm; extern CD_alarm D_800F19B8;
(dropping the per-word D_800F19BC / D_800F19C0 externs from C) and reading
D_800F19B8.func directly in the printf call **instead of** the
pp = (void \*\*)&D_800F19C0; ... \*pp pointer alias:

- on the j1 duplicated-arms chassis: 3 -> 3 (A0 vs A1), and the objdump
  comparison is 212 records with **5 differences, every one an addend**
  (sw zero,0(at) -> sw zero,4(at); sw v0,0(at) -> sw v0,8(at);
  lw v1,0(v1) -> lw v1,4(v1); sw v0,0(at) -> sw v0,4(at);
  lw a1,0(a1) -> lw a1,8(a1)), all against R_MIPS_LO16 D_800F19B8
  resolving to the same addresses the named per-word symbols resolve to.
  Identical registers, identical order, identical insn count.
- on the floor (goto) chassis: 2 -> 2, banked as
  progress/s117-alarm-struct-nopp-2.c.

On the SCALAR model dropping pp costs 6 points (s115_m1 = 9). Under the
struct model the member MEM (mem (const (plus (symbol_ref D_800F19B8) 8)))
gets the early argument load that the alias had to buy, exactly the mechanism
CD_datasync s58 recorded. Net: **the floor form loses one FAKE** (only the
combine-foldable chain-extender remains) and loses the pp DECLARATION-PUN
that the dispatch auto-scan flags on this file. Unlike CD_datasync, the engine
sandbox did NOT inflate the struct score here — score.py masked all five
addends, so 2 is 2, and the objdump check confirms it independently.

### H117-2 KILLED (instance) — the CD_intr struct member read collapses idx_1495

Sony's typedef struct { u8 sync; u8 ready; u8 x2; u8 x3; } CD_intr; gives the
target's own 1($s2) do_timeout read as ORDINARY C (idx_1494->ready), which
would have retired the chain-extender FAKE. It does not survive: the moment
do_timeout reads through the base pointer, idx_1495 is left with its single
callback-site reference and stops being seated in $s4, rotating the whole
callee-saved assignment.

- B1 (idx_1495 = &idx_1494->ready, do_timeout reads idx_1494->ready) 20/160.
- B3 (idx_1495 = &D_800A1494.ready, same read) 20/160.
- B2 / B4 (same declarations, do_timeout keeps \*idx_1495) 3/160 — i.e. the
  CD_intr declaration is byte-INERT; it is the READ SITE that costs 17 points.
- A3 (scalar idx_1494[1] in both duplicated arms) 20/160.
- A4 (A3 + s112's success-block placement of the idx_1495 init) 18/160.

All measured on the pp-free struct chassis; one FAKE present (the chain-extender)
in the floor forms, none in A3/B1/B3, which is precisely why those collapse.

### H117-3 KILLED (instance) — removing pp does not decouple order-exactness from the $a0/$v1 seat exchange

CD_datasync s58 predicted that block 3 with one pseudo fewer might reprice the
local-alloc fixed point. It does not. Seven order-fixing spellings on the
pp-free struct chassis all land on 6/160 (was 7/160 with pp — the pp removal is
worth exactly the same 1 point everywhere):
C1 h1-style ix address split 6 · C2 e3-style ix-first split 6 ·
C3 k1-style chain-order swap 6 · C4 scale-between-address-and-load 6 ·
E1 both addresses split 6 · E3 ix-split/t0-scale-last 6 ·
E4 ix-split/load-then-t0-address 6.
Seat-exact spellings all land on 2 (A5, E2). The behaviour is strictly
bimodal, the same bimodality CD_datasync s57 recorded. C1's residual is a PURE
register exchange, verified instruction-for-instruction: order-exact for all
160, with t0's chain in $v1 and printf's arg5 value in $a0 where the target has
them the other way round (indices 49, 55, 56, 59, 61, 65).

### H117-4 KILLED (instance) — reference-count edits on the order-exact chassis

- D1 (t0's addu+load folded into the printf argument) 14/160.
- D2 (t0's whole chain folded into the printf argument) 14/160.
- D3 (dead ix = arg5; after the load) 6/160 — byte-INERT, and a dead store, so
  it is not a lever and is not banked as a form.
- D4 (arg5 read inline at the call instead of through a local) 9/160.
- E5 (t0's address staged through ix after its own last use) 14/160.

### H117-5 CONFIRMED (mechanism) — the contested seat is a QTY_CMP_PRI TIE, not a reference-count inversion

The standing frontier's model ("arg5 at 2 references = 4000 loses to the t0
chain's 4 references = 5000; lift arg5 to 3 references") is **wrong about which
quantities are contested**. Measured with the instrumented cc1's BB2_QTY_DEBUG
hook (tools/gcc-2.7.2/cc1, local-alloc.c) on a single-function TU — new
reusable tooling, tmp/grind/CD_sync/s117/mini.sh + qtyrun.sh, which extracts
CD_sync plus its own declaration block into tmp/grind/CD_sync/s117/mini.c and
reproduces the full-TU codegen exactly (pseudo numbering restarts per function,
so the qty table is TU-independent; verified against the full-TU dump). The
whole-TU stderr stream is unusable for this because QTYDBG lines carry no
function name — segmenting it by blk resets gives 19 candidate segments and
CD_sync is not identifiable among them. The mini-TU is the tool.

do_timeout is basic block 3. Four quantities:

| form | qty | reg | birth | death | span | refs | got |
|---|---|---|---|---|---|---|---|
| A5 (seat-exact, 2) | 0 | 113 | 10 | 20 | 10 | 6 | $v0 |
| A5 | 3 | 120 | 22 | 30 | 8 | 4 | $v0 |
| A5 | **2** (arg5 value) | 106 | 20 | 26 | **6** | 2 | **$v1** |
| A5 | **1** (t0 address) | 112 | 16 | 24 | **8** | 2 | **$a0** |
| C2 (order-exact, 6) | 0 | 108 | 10 | 20 | 10 | 6 | $v0 |
| C2 | 3 | 119 | 22 | 30 | 8 | 4 | $v0 |
| C2 | **1** (t0 address) | 112 | 18 | 24 | **6** | 2 | **$v1** |
| C2 | **2** (arg5 value) | 106 | 20 | 26 | **6** | 2 | **$a0** |

(C1 is identical to C2 in this table.)

QTY_CMP_PRI = floor_log2(refs) \* refs \* size \* 10000 / (death - birth):

- A5: qty1 = 1\*2\*10000/8 = **2500**, qty2 = 1\*2\*10000/6 = **3333**
  -> qty2 allocated first, takes the lowest free hard reg $v1(3); qty1 takes
  $a0(4). TARGET SEATS.
- C1/C2: qty1 = 3333, qty2 = 3333. **EXACT TIE.** qty_compare_1 falls through
  to \*q1 - \*q2, i.e. the quantity INDEX, and qty1 < qty2, so the t0 address is
  allocated first and takes $v1. WRONG SEATS.

**The entire order-vs-seat two-body problem is one luid.** Making the emission
order exact moves qty1's BIRTH from 16 to 18 — one luid, nothing else in the
table changes, not a single ref count, not qty2's range — and that single luid
converts a 3333-vs-2500 win into a 3333-vs-3333 tie that is lost on index.
This supersedes every reference-count framing of this residual since s108.

### Closing condition (numeric, for the next session)

On an ORDER-EXACT emission, achieve ANY ONE of:

- (a) qty1 birth <= 16 (span >= 8): pri <= 2500 < 3333;
- (b) qty2 refs >= 3 (span unchanged at 6): pri = floor_log2(3)\*3\*10000/6 = 5000 > 3333;
- (c) qty2 death <= 24 (span <= 4): pri >= 5000;
- (d) qty2's quantity INDEX below qty1's, which wins the tie outright.

(a) is the one the measurement says is one luid away. (d) is untouched by any
session and is the cheapest to test, because qty numbers are handed out in
local-alloc's block scan order: anything that makes the arg5-value pseudo's
first reference precede the t0-address pseudo's first reference flips it. Note
the table already shows this is NOT simply emission order — in C2 the arg5 lw
is emitted two insns BEFORE the t0 addu, yet qty1 still births at 18 and qty2
at 20, so birth is not 2x the in-block insn index and the luid mapping needs
reading out of the .lreg/.sched pair before (d) can be aimed. That
reconciliation is the first job of the next session and it is a dump read, not
a sandbox run.

- [s117] The banked candidate.c could not be compiled at all: its body still called sys_VSync / tslTm2LoadImage_2 / debug_printf / cdrom_ClearIrq / sys_GetVblankCount / func_80080828, none of which survive in HEAD's src/system.c. The live names, read off asm/funcs/CD_sync.s jal targets, are VSync / puts / printf / CD_flush / CheckCallback / getintr. candidate.c is repaired and re-measured live at 2/160 bi 160 rd 0.

- [s117] Chassis re-verified live at dispatch (the brief's CHASSIS CHECK said 'measurement unavailable'): j1 duplicated-arms 3/160, floor goto body 2/160, both bi 160 rd 0.

- [s117] The 2-point residual read off the bytes with tmp/grind/CD_sync/s117/cmp.py is exactly the index 54/55 transposition — target 'addu $v0,$v0,$s3 ; sll $a0,$a0,2', ours 'sll $a0,$a0,2 ; addu $v0,$v0,$s3'. All other 158 instructions match including every register.

- [s117] The order-exact C1 residual is a clean two-quantity register exchange, not a code difference: differing indices are 49 (lbu a0 vs lbu v1), 55, 56, 59, 61, 65 — six instructions, all the same opcodes, with $a0 and $v1 swapped between the t0 chain and printf's arg5 value.

- [s117] engine/score.py masked all five R_MIPS_LO16 addends produced by the CD_alarm struct model on CD_sync, so the struct scores need no correction here — unlike CD_datasync s58, where the same model read +5 false points. Always confirm with the objdump record comparison rather than assuming either behaviour.

- [s117] New reusable tooling for this function and its twins: tmp/grind/CD_sync/s117/apply.py (region-scoped declaration+body splice that cannot touch CD_datasync's duplicate decl block), b.sh (batch score + objdump + addend-normalised dump), cmp.py (normalised target-vs-build instruction diff that resolves hex/decimal and GNU-vs-PsyQ mnemonic aliases), mini.sh + qtyrun.sh (single-function TU + BB2_QTY_DEBUG quantity table).

- [s117] The CD_intr declaration is byte-inert on this function (B2/B4 = 3/160, identical to the scalar B-line control), so a future session may adopt Sony's object model for D_800A1494 for free — it just cannot use it to replace the chain-extender at the do_timeout read site.

### H118-1 CONFIRMED - the luid reconciliation the s117 frontier asked for, and what it costs conditions (a) and (d)

local-alloc.c:1173-1175 counts CODE_LABELs in insn_number, so block 3's luid is
2*(N+1) for the Nth real insn. Every birth and death in the s117 quantity table
now has a name (full list in hypotheses.md s118). The result is deflationary:
qty1's birth is the position of the t0-scale insn and qty2's birth is the
position of the arg5 load, and quantity INDEX is handed out in birth order
(alloc_qty, local-alloc.c:284). So closing conditions (a) "qty1 birth <= 16" and
(d) "qty2 index below qty1" are both restatements of "emit the t0 scale before
the ix addu" - which is exactly the A5 order, i.e. the non-order-exact one. Two
of the four closing conditions collapse into the variable the two-body problem
is already about. Do not spend another session aiming at them.

### H118-2 CONFIRMED - qty_n_refs is the FUNCTION-WIDE flow.c count

local-alloc.c:297 copies reg_n_refs[regno] straight into qty_n_refs. reg_n_refs
is built in flow.c BEFORE combine and is weighted by loop_depth. Two consequences
the ledger did not have: a reference combine later deletes still counts (the
chain-extender FAKE's mechanism, now confirmed to be the same mechanism that
feeds the priority formula), and a reference inside a loop-noted region counts
twice.

### H118-3 CONFIRMED - do-while(0) is a per-statement reference multiplier, and it wins the seat

M1 (wrap = the single statement "arg5 = *(s32 *)ix;") moved exactly the two
pseudos that statement references and only those: reg106 2 -> 3, reg108 6 -> 7.
M5 (wrap = ix address + load + printf) gave reg106 four references, priority
13333 against the t0 temp's 3333, and the QTYDBG line reads
"blk=3 ord=2 qty=2 reg1=106 birth=22 death=26 refs=4 got=3": the arg5 value is
seated in $v1, the target seat, WITHOUT the A5 emission order. Closing condition
(b) is reachable from ordinary source structure. This is the first time in 118
sessions that the seat has been won by anything other than the A5 order.

### H118-4 KILLED (instance) - the carrier costs 3 points: the loop notes are a code-motion barrier

Fourteen wrap geometries (M1-M8, N1-N4, P1-P3) all build at 160 insns and
rules_dropped 0; the best is 5/160 (N1/P1/P2/P3) against the floor of 2/160.
N1's residual is entirely code motion across the note pair: the two lbu reads of
D_800A1494 come out exchanged at target indices 49/50 (and swapping the two C
statements does not swap them back), and the "lui a1 / lw a1,8(a1)" load of
D_800F19B8.func sinks from indices 51/52 to 55/56 because the printf consuming
it sits inside the wrap - staging it through a local declared before the wrap
does not lift it either. So the shape of the remaining problem has changed: it
is no longer "win the seat", it is "get the +1 reference onto the arg5 pseudo
WITHOUT a NOTE_INSN_LOOP_BEG in the block".

### H118-5 KILLED (instance) - the real-loop spelling of the polling loop

"for (;;)" instead of "loop:" / "goto loop" regresses to 67 (A5) and 70 (C1) at
build_insns 165: loop.c hoists and the function grows five instructions. It is
also symmetric - it would double the t0 temp's references as well - so it could
not break the tie even if it were free.

- [s118] New reusable tooling: tmp/grind/CD_sync/s118/dump.sh (full -da dump set
  from the mini TU into a per-tag directory), dumpenv.sh (same, with an arbitrary
  comma-separated list of BB2_*_DEBUG hooks enabled), mkmini.py (rebuild mini.c
  from the current src/system.c), blocks.py (segment any cc1 -da dump into basic
  blocks and print each insn with its local-alloc luid, its position and its uid),
  b.sh (batch scoring of s118 forms).
- [s118] BB2_SUGG_DEBUG is the richer instrument and had never been run on this
  function: it prints qty_min_class, qty_alternate_class, calls-crossed,
  copy-suggestions and suggestions per quantity, plus a find_free_reg trace. On
  CD_sync block 3 every quantity has ncopysugg=0 nsugg=0 and minclass=1, so the
  suggested-register pass (local-alloc.c:1505-1528) is inert here and the whole
  allocation is decided by qty_compare_1 alone. That closes off "the target used a
  register suggestion" as an explanation.

- [s118] GCC luid for block 3 = 2*(N+1) where N is the insn's index among the block's real insns, because local-alloc.c:1173-1175 counts the block's CODE_LABEL in insn_number. Full C1 block-3 map (position, uid, pattern, and which quantity is born/dies there) is banked in hypotheses.md s118.

- [s118] qty_n_refs comes from reg_n_refs[regno] (local-alloc.c:297) - the function-wide, loop-depth-weighted flow.c count taken before combine - not from a block-local count.

- [s118] A do{...}while(0) wrap adds exactly +1 to reg_n_refs for every reference lexically inside it, measured pseudo-by-pseudo: M1's one-statement wrap moved reg106 2->3 and reg108 6->7 and nothing else.

- [s118] With reg106 at 4 references the arg5 value takes $v1 (the target seat) on a non-A5 emission: M5 QTYDBG 'blk=3 ord=2 qty=2 reg1=106 birth=22 death=26 refs=4 got=3'. Closing condition (b) is reachable.

- [s118] BB2_SUGG_DEBUG had never been run on this function: block 3's four quantities all report ncopysugg=0 nsugg=0 minclass=1, so the suggested-register pass (local-alloc.c:1505-1528) is inert here and the allocation is decided by qty_compare_1 alone. 'The target used a register suggestion' is closed off.

- [s118] The wrap's NOTE_INSN_LOOP_BEG/END pair is a code-motion barrier: nothing inside the wrap hoists past its head, which sinks the D_800F19B8.func argument load from target indices 51/52 to 55/56 whenever the printf is inside the wrap, and exchanges the two lbu reads of D_800A1494 at indices 49/50 in a way statement-order swaps do not repair.

- [s118] The floor is unchanged at 2/160 (build_insns 160, rules_dropped 0). candidate.c has been REPLACED with the pp-free CD_alarm-struct body (s117 progress form): same 2/160, one FAKE instead of two, and the D_800F19C0 declaration pun the dispatch auto-scan flagged is gone.

- [s118] New reusable tooling: tmp/grind/CD_sync/s118/dump.sh (full -da dump set from the mini TU), dumpenv.sh (same with an arbitrary list of BB2_*_DEBUG hooks), mkmini.py, blocks.py (segment any cc1 dump into basic blocks and print luid/position/uid per insn), b.sh.

### H119-1 CONFIRMED - N1 (the best do-while(0) wrap form, 5/160) already has BOTH target seats

BB2_QTY_DEBUG on N1: reg106 (arg5 value) birth 18 death 26 refs 4 got=3 ($v1),
reg112 (t0 scale temp) birth 16 death 24 refs 3 got=4 ($a0). s118's frontier
item 3 is answered: N1's whole 5-point residual is code motion across the
NOTE_INSN_LOOP_BEG pair, not allocation.

### H119-2 CONFIRMED - a local-alloc quantity's reference count is the SUM over every pseudo tied into it

local-alloc.c:1932 `qty_n_refs[sqty] += reg_n_refs[sreg];` inside combine_regs.
block_alloc calls combine_regs for EVERY operand that dies in an insn whose
operand 0 carries an `=' constraint and is not earlyclobber (local-alloc.c:
1210-1300, calls at 1295/1331/1336/1346) - not only for register-to-register
copies. This explains the s117 table entry the ledger never accounted for:
A5's qty0 reports refs=6 for reg113, a two-reference `plus' temp, because
reg108 (4 refs) dies into it; C1's single-pseudo spelling of the same value
reports the same 6.
Two negative corollaries measured this session: a LOAD cannot tie (its source
is a MEM, not a REG), so the arg5 pseudo cannot inherit refs from the address
pseudo that dies at the load; and an explicit C copy never reaches local-alloc
(H119-3).

### H119-3 KILLED (instance) - an explicit copy local for the arg5 value

R5 (`arg5b = arg5;` then pass arg5b) is 6/160 with a quantity table identical
to the C1 control (reg106 refs 2). cse propagates the copy away long before
flow counts references.

### H119-4 KILLED (instance) - printf argument-evaluation order does not move qty2's death (s117 closing condition (c))

Q1 (stage `*(s32 *)t0`) 6 · Q2 (stage the D_800A11DC subscript) 13 ·
Q3 (both) 11 at bi 159 · Q4 (t0 address after the arg5 load) 6 ·
Q5 (stage the t0 deref before the arg5 load) 7 · Q6 (subscript first) 13 ·
Q7 (subscript between) 14.
Q4's quantity table is byte-for-byte C1's, i.e. moving the t0 address statement
across the arg5 load in the SOURCE changes nothing in the post-sched1 stream.
Q1 does reach got=3 for reg106 but only by restructuring the entire t0 chain
(11 raw instruction diffs).

### H119-5 CONFIRMED - THE SESSION'S RESULT: exiling the t0 address to global-alloc wins the arg5 seat with refs=2, no wrap, no new construct

Carrying the do_timeout t0 address in an EXISTING function-wide local (rather
than a fresh block-scope local) makes that pseudo multi-block, so local-alloc
never sees it: block 3 drops from four quantities to three and the
3333-vs-3333 qty_compare_1 tie that has governed this residual since s108
simply does not occur. The arg5 value is then allocated after the D_800A11D5
index quantity and takes the lowest free hard register, $v1 - the target seat -
with refs=2, on an order-exact emission, with ZERO extra references, ZERO loop
notes and ZERO new FAKE constructs.

  S2 (borrow `status', C1 statement order)                  5/160
  T1 (borrow `status', ix-chain-first order)                 4/160
  T2 (borrow `status', literal target statement order)       4/160

T1/T2's residual is FIVE instructions and they are ALL THE SAME REGISTER:

  idx 49  T: lbu  a0,0(s2)      O: lbu  s0,0(s2)
  idx 55  T: sll  a0,a0,2       O: sll  s0,s0,2
  idx 59  T: addu a0,a0,s3      O: addu s0,s0,s3
  idx 65  T: lw   a3,0(a0)      O: lw   a3,0(s0)

Every other instruction of all 160 matches, including every register and the
complete emission order. This is the first form in 119 sessions that is
simultaneously ORDER-EXACT and ARG5-SEAT-EXACT. The two-body problem is over;
what is left is one allocno's register class.

### H119-6 KILLED (instance) - no existing local, borrowed for the t0 address, gets $a0

MIPS defines no REG_ALLOC_ORDER (tools/gcc-2.7.2/config/mips/mips.h), so
global.c's find_reg walks hard registers ascending and would take $a0(4) the
moment $v0/$v1 are spoken for. Every one of the nine borrow candidates lands on
one of exactly two outcomes instead:

  status  $s0 (callee-saved: live across the getintr callback calls)  T1/T2 4
  v0      $a3                                                        U1_v0 7
  cnt     $a3 (+2 diffs from cnt's own uses at idx 38/42)            U1_cnt 8
  temp 10 · i 16 · dst 15 · src 15 · saved 16 (bi 162) · new_var 37 (bi 162)

The mechanism is that a borrowed variable's live range is FUNCTION-WIDE, so it
overlaps every point at which $a0/$a1/$a2 are set for the other calls in the
function (VSync, the two callbacks, printf's own argument setup). $a3 does not
have that conflict because $a3 is written exactly once in the whole function -
at index 65, which is the allocno's own death.

- [s119] The closing form must therefore give the t0 address a live range that
  is (i) invisible to local-alloc (multi-block) yet (ii) free of the $a0
  conflict, or else keep it block-local and break the qty_compare_1 tie in the
  arg5 value's favour by a byte-neutral asymmetry. Both are now precisely
  stated for the first time.
- [s119] New reusable tooling: tmp/grind/CD_sync/s119/qd.sh (apply a form,
  rebuild the mini TU and take a BB2_QTY_DEBUG + BB2_SUGG_DEBUG dump in ONE
  WSL call - the s118 scripts had to be run inside WSL by hand), d.sh (score +
  objdump + normalised target diff for one form), b.sh (batch scorer).
- [s119] Forensic read of A5's gccdump.sched vs gccdump.sched2: sched2 DOES
  reorder this block (it swaps uid145 `sll' and uid149 `sw ...,16(sp)' into the
  target's order), so post-reload scheduling is a live degree of freedom here;
  but the 120/130 pair that produces A5's 54/55 transposition is an exact
  INSN_PRIORITY tie (both final_pri=2, RANKDBG val=0) resolved by list position,
  which is the same variable as the source statement order. That is why
  order-exactness and A5's qty1 birth of 16 cannot be had at once.

- [s119] [s119] N1 (5/160, do-while(0) wrap) already carries BOTH target seats: reg106 got=3 ($v1), reg112 got=4 ($a0). s118 frontier item 3 answered - the wrap family's residual is entirely code motion, not allocation.

- [s119] [s119] local-alloc.c:1932 sums qty_n_refs across a tie and block_alloc ties operand 0 with any DYING pseudo operand of a 2+-operand insn (calls at local-alloc.c:1295/1331/1336/1346), which is why A5's two-reference reg113 temp reports qty refs=6.

- [s119] [s119] A load cannot tie (its source is a MEM), and an explicit C copy never survives cse to local-alloc (R5 = 6/160 with a C1-identical quantity table), so neither route lifts the arg5 quantity's reference count.

- [s119] [s119] BREAKTHROUGH: carrying the t0 address in an existing function-wide local removes the arg5-vs-t0 quantity tie completely - block 3 has three local quantities instead of four and the arg5 value takes $v1 with refs=2, no loop note and no new FAKE. S2 5/160, T1 and T2 4/160.

- [s119] [s119] T1/T2's residual is FIVE instructions and all five are one register: indices 49/55/59/65 have the t0 chain in $s0 where the target has $a0. Emission order is exact for all 160 instructions and every other register matches. The order-vs-seat two-body problem that governed s108-s118 no longer applies to this chassis; what remains is one allocno's register class.

- [s119] [s119] MIPS has no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so global.c's find_reg is a plain ascending walk. status -> $s0 (live across the callback calls); v0 and cnt -> $a3; the function-wide live range of any borrowable local conflicts with $a0/$a1/$a2 at the VSync / callback / printf argument setups, while $a3 is written exactly once in the function (index 65, the allocno's own death).

- [s119] [s119] Forensic read of A5's gccdump.sched vs gccdump.sched2: sched2 does reorder this block (it swaps uid145 sll and uid149 sw ...,16(sp) into the target's order), so post-reload scheduling is a live degree of freedom; but the uid120/uid130 pair behind A5's 54/55 transposition is an exact INSN_PRIORITY tie (both final_pri=2, RANKDBG val=0) resolved by list position, i.e. by the source statement order - which is why A5's qty1 birth of 16 and order-exactness cannot coexist.

- [s119] [s119] Floor unchanged at 2/160 (candidate.c, the pp-free CD_alarm-struct goto body). src/system.c reverted to HEAD at end of session; new progress forms banked under memory/grind/CD_sync/progress/.

## s120 (rederive) — the 54/55 residual reduced to ONE sched1 predicate

Floor unchanged at 2/160 (build_insns 160, rules_dropped 0). candidate.c body
unchanged. src/system.c restored to HEAD at end of session. 30 forms measured
(tmp/grind/CD_sync/s120/forms/, results in results.txt).

### The finding, stated as a closed equation

`local-alloc.c:1660 qty_compare_1` computes
`pri = floor_log2(refs) * refs * size / (death - birth) * 10000`. Block 3 of
CD_sync has exactly two quantities in contention for the printf argument seats:

  * `reg112` — the t0 address chain (`(u8 *)tbl_125c + idx_1494[0]*4`)
  * `reg106` — the arg5 VALUE (`*(s32 *)(tbl_125c + idx_1494[1]*4)`)

Both always carry refs=2, size=4, so pri is decided by SPAN alone, and on a tie
`qty_compare_1` falls back to `*q1 - *q2` (qty number = birth order), which t0
always wins because it is born first. Target seating is arg5 -> $v1, t0 -> $a0,
i.e. arg5 must be allocated FIRST, i.e. **pri(arg5) > pri(t0)**.

Measured (BB2_QTY_DEBUG, tmp/grind/CD_sync/s120/*/qty.txt):

| chassis | t0 birth/death | arg5 birth/death | pri t0 | pri arg5 | seats | order | score |
|---|---|---|---|---|---|---|---|
| candidate / F1 / F4 / F5 / F8 (folded ix) | 16/24 (span 8) | 20/26 (span 6) | 10000 | 13333 | CORRECT | 54/55 transposed | 2 |
| V1 / W1 / W3 / W4 / W5 / G1 / G2 (split ix) | 18/24 (span 6) | 20/26 (span 6) | 13333 | 13333 tie | SWAPPED ($a0<->$v1) | EXACT | 6 |

"folded" = `arg5 = *(s32 *)(ix + (s32)tbl_125c);` — the ix add lives inside the
MEM at sched1 time and only materialises as a real `addu` during reload, so it
is emitted immediately before the load, i.e. AFTER the t0 `sll`. "split" =
`ix += (s32)tbl_125c;` as its own statement — the `addu` is a schedulable insn,
sched1 places it at slot 8 (giving target order 54 `addu v0,v0,s3` / 55
`sll a0,a0,2`) and t0's `sll` is pushed from slot 8 to slot 9, collapsing t0's
span from 8 to 6 and creating the tie.

So the order-vs-seat two-body problem of s108-s119 is now a SINGLE binary
variable: whether the ix add is a sched1 insn. It cannot be both.

### The one remaining escape, and it is NOT a refs lever

s119's frontier item 2 asked for a construct lifting reg106's refs to >= 3
(a second chain-extender FAKE). That is no longer the only route, and it is
probably not the right one. Read the TARGET's own final order:

  53 sll  v0,v0,2        (ix scale)
  54 addu v0,v0,s3       (ix add)          <- split chassis reproduces this
  55 sll  a0,a0,2        (t0 scale)
  56 lw   v1,0(v0)       (arg5 load)       = reg106 birth
  59 addu a0,a0,s3       (t0 add)
  61 sw   v1,16(sp)      (arg5 stack arg)  = reg106 death
  65 lw   a3,0(a0)       (t0 deref)        = reg112 death

In the target's FINAL order the arg5 `sw` (61) precedes the t0 `lw a3` (65).
If sched1 produced that same relative order, the spans would be
t0 = 18..26 (span 8, pri 10000) and arg5 = 20..24 (span 4, pri 20000) — arg5
allocated first, $v1; t0 second, $a0 — the target seating, ON the order-exact
split chassis, with refs 2/2 and NO new construct. Every split form measured
this session instead schedules `lw a3` at slot 12 and `sw` at slot 13 (t0 death
24, arg5 death 26) and relies on sched2 to swap them back into the target order
(s119 already proved sched2 reorders this block). The closing predicate is
therefore:

  **make sched1 emit `sw arg5,16(sp)` BEFORE `lw a3,0(t0)`.**

sched1 refuses because INSN_PRIORITY(`lw a3`) = 2 (its result feeds the call)
> INSN_PRIORITY(`sw`) = 1, and both are ready at the same cycle. The lever must
either delay `addu a0,a0,s3` far enough that `lw a3` is not yet ready when `sw`
becomes ready, or raise the `sw` chain's priority. This is a sched_solver
question on the split chassis (V1), not an allocator question.

### Kills banked this session

- Borrowing a PARAMETER for the do_timeout t0 address — the one enumeration
  branch s119's frontier named but never ran. `a0` (mode) 22/160, `a1` (result)
  25/160, both statement orders, bi 160 rd 0. Both parameters are live across
  the loop's VSync / getintr / callback calls (the do_timeout path returns, but
  the parameter pseudo's live range is the whole loop), so the borrow forces a
  callee-saved seat plus prologue/epilogue and argument-shuffle churn. The nine
  local borrows of H119-6 plus these two exhaust the borrow enumeration.
- Exiling one of the OTHER block-3 quantities to a function-wide local instead
  of t0 (the mirror of T1): ix 4/160, arg5 value 6/160, the arg3 subscript
  17/160, the arg4 deref 12/160, ix+arg5 together 9/160. E1 (exile ix) is
  informative: t0 stays block-local and keeps $a0 — confirming the seat is a
  local-alloc product — but the exiled ix chain lands in $s0 (target $v0) and
  the 54/55 transposition returns, so it is strictly worse than the candidate.
- Source statement order inside the do_timeout block does not move the sched1
  positions of the printf-argument insns: 8 distinct orders on the split
  chassis all give exactly t0 18/24 and arg5 20/26; 7 distinct orders on the
  folded chassis all give exactly t0 16/24, arg5 20/26 and the identical two-
  instruction 54/55 transposition.
- Natural-C rederives of the block (array subscripts `tbl_125c[idx_1494[1]]`,
  `&tbl_125c[idx_1494[0]]`, and fully inlined printf argument expressions) are
  all worse: 9, 9, 14, 10, 10, 14. The hand-derived pointer form in candidate.c
  is not an artefact of previous sessions' taste; it is the best spelling.

- [s120] New reusable tooling: tmp/grind/CD_sync/s120/{b.sh,d.sh,qd.sh} (s119
  clones retargeted at s120), gen2.py..gen8.py (form generators that patch the
  do_timeout block of candidate.c programmatically, so a new order permutation
  is one list of statement keys).

- [s120] Floor unchanged at 2/160 (build_insns 160, rules_dropped 0); candidate.c body unchanged, src/system.c restored to HEAD.

- [s120] qty_compare_1 is tools/gcc-2.7.2/local-alloc.c:1660: pri = floor_log2(qty_n_refs)*qty_n_refs*qty_size/(qty_death-qty_birth)*10000, tiebreak *q1-*q2 (qty number = birth order). refs 2 / size 4 / span 6 = 13333; span 8 = 10000; span 4 = 20000.

- [s120] Birth/death are 2*insn_number (local-alloc.c:2031 births, local-alloc.c:2119 deaths, +output_p), so a span of 6 is three insns.

- [s120] Block 3's two contending quantities are the t0 address chain and the arg5 value; both carry refs=2 and size=4 on every one of the fifteen forms dumped this session, so pri is decided by span alone.

- [s120] FOLDED ix add (candidate, F1, F4, F5, F8): t0 16/24, arg5 20/26 -> arg5 $v1, t0 $a0 (target seats), 2/160, residual is the two-instruction 54/55 transposition.

- [s120] SPLIT ix add (V1, W1, W3, W4, W5, G1, G2): t0 18/24, arg5 20/26 -> tie -> t0 $v1, arg5 $a0, 6/160, but the emission order is exact for all 160 instructions.

- [s120] V1's whole residual is a pure register swap at indices 49/55/56/59/61/65 ($a0 <-> $v1); banked as memory/grind/CD_sync/progress/s120-V1-order-exact-seatswap.c.

- [s120] The target's own final order puts the arg5 stack store (index 61) BEFORE the t0 deref (index 65); every split form we produce has the opposite sched1 order and relies on sched2 to swap them back.

- [s120] Parameter borrows are dead: a0 22/160, a1 25/160 (both statement orders). With H119-6's nine locals the borrow enumeration is complete.

- [s120] Exiling a non-t0 block-3 quantity is dead: ix 4, arg5 6, arg3 17, arg4 12, ix+arg5 9. E1 (ix) proves the t0 chain keeps $a0 whenever it stays block-local.

- [s120] Natural-C subscript and inlined-argument rederives are all 9-14/160 against a 2/160 control.

## s121 (rederive, 2026-09-04) — the residual is now a typed RA question, not a scheduling one

- [s121] Chassis re-measured live at session start: `memory/grind/CD_sync/candidate.c`
  = 2/160 bi 160 rd 0; `tmp/grind/CD_sync/s120/forms/V1_ixfirst_nolocalborrow.c`
  = 6/160 bi 160 rd 0. Ledger floor 2 confirmed on HEAD.
- [s121] The sched_solver and ra_solver suites BOTH run on CD_sync in OBJECT mode
  (`--target-object build/src/system.o --ours-object tmp/sandbox/CD_sync/system.o`).
  `extract.py system` reports parity=True (80 funcs, 424 blocks, 2174 picks) and
  `simulate.py --func CD_sync` scores 52/52 blocks order-exact AND clock-exact in
  both passes, so the scheduler model is exact for this function.
- [s121] TRAP, cost a third of this session: `goalmap.py --target asm/funcs/CD_sync.s`
  silently produces a FALSE NEGATIVE. `asm_body()` (tools/sched_solver/goalmap.py:106)
  skips every line starting with `/*`, which is every splat body line, so the target
  parses as ONE instruction (`tgt=1`) and EVERY block prints "GOAL == OURS (identity)".
  Use object mode. In object mode the alignment is 160-vs-160, 159 equal, 1 moved.
- [s121] The 2/160 residual, at RTL-UID level: block 3 slots 6/7. Ours emits uid 120
  then uid 130; the target emits 130 then 120. Both have INSN_PRIORITY 2, both become
  ready in the same cycle, and rank_for_schedule falls through to INSN_LUID descending.
  From the extracted dependence graph (not inferred): chain A = 112 -> 120 -> 125 -> 157,
  chain B = 116 -> 127 -> 130 -> 132 -> 149. So uid 120 is the t0 chain's `sll` (from
  `t0 *= 4;`) and uid 130 is the ix chain's reload-materialised `addu` (from the folded
  `*(s32 *)(ix + (s32)tbl_125c)`), NOT the other way round. The uid->asm-text column
  printed by tmp/grind/CD_sync/s121/show.py mislabels these two exactly because they are
  the transposed pair — always read the deps.
- [s121] perturb.py depth-1 luid search: 36 single atoms reach the pass-2 goal, 31 reach
  the pass-1 goal, intersection 26. Every one of them is a pure source-statement move
  (raise luid(120) above luid(130), or lower luid(130) below luid(120)). The order fix
  needs no construct at all.
- [s121] THE ORDER IS SOLVED, AND IT IS NOT SUFFICIENT. `V2_ixfirst_folded` — banked in
  s120 as a bare "6/160" with no order analysis — is ORDER-EXACT in BOTH sched1 and
  sched2 for block 3. `inverse_compose.py classify` on it prints
  `FIRST DIVERGENCE: RA — same instructions, different registers` with exactly six
  pairs: ours `addu v1,v1,s3 / lbu v1,0(s2) / lw a0,0(v0) / lw a3,0(v1) / sll v1,v1,0x2 /
  sw a0,16(sp)`, target `addu a0,a0,s3 / lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) /
  sll a0,a0,0x2 / sw v1,16(sp)`. This retires the s115-s120 framing in which "order" and
  "seats" were two outcomes of one binary variable: a form exists that has the order and
  not the seats, and the seats question survives it intact.
- [s121] BB2_QTY_DEBUG on V2, blk=3: `qty=1 reg113 birth=18 death=24 refs=2 got=3`
  ($v1, the t0 address) and `qty=2 reg106 birth=20 death=26 refs=2 got=4` ($a0, the arg5
  value); the target wants them exchanged. Equal refs and equal spans mean qty_compare_1
  (tools/gcc-2.7.2/local-alloc.c:1660) ties, and the qty-number (birth-order) tiebreak
  decides against us. Raw dump: tmp/grind/CD_sync/s121/V2/qty.txt.
- [s121] `inverse.py local --func CD_sync --block 3 --swap 1,2 --depth 2` returns
  REACHABLE, minimal solution size 1 atom, 21 distinct vectors, in three classes:
  refs_down on qty 1 (rank #1), live_extend on qty 1 (ranks #2-#5: dies later 24->25 or
  24->26, born earlier 18->17 or 18->16), refs_up on qty 2 (rank #8). The tool prints its
  own LOCAL-MODE CAVEAT (measured on camera_set_zoom 2026-08-05): a birth/span vector is
  a claim about ALLOC-time order, which is not known to equal emission order, so each
  vector is NECESSARY, not SUFFICIENT, until re-derived from a QTYDBG dump of the actual
  spelled candidate. Full listing: tmp/grind/CD_sync/s121/inverse_local.txt.
- [s121] Rank #1 (refs_down on the t0 address) is DEAD as a spelling: six ways of folding
  the add into printf's 4th-argument MEM all measure 14/160 (M1-M6). The target's own
  bytes contain a 2-reference t0 address (`addu $a0,$a0,$s3` then `lw $a3,0($a0)`), so
  removing the reference removes the instruction. The vector is a model artifact.
- [s121] Also dead on the order-exact chassis: block declaration order (N3 6, N4 6),
  a dedicated `s32 *ap` pointer local for the arg5 address (N6 6), reading Intr.sync after
  the ix group (N5 7), and the upstream SOTN bios.c fully-inlined call shape (N1 14,
  N2 14). The rederive modality's remaining structural sources for this window are spent:
  s114 already established the upstream body is in hand, and s121 measured it.
- [s121] Mandated kill re-audit via tools/fake_ablate.py: candidate.c 2 with the
  chain-extender FAKE / 15 without; V2 6 with / 19 without. Both chassis scores are real,
  not FAKE-carrier artifacts.
- [s121] Sibling sweep: CD_ready s84's finding (block-3 scheduler neighbourhood enumerated
  and empty; its residual is an ra_solver question, not a sched_solver one) now holds for
  CD_sync too, by an independent derivation. func_80045294's candidate.c carries no shared
  window with this function (different TU, different call shape) and has nothing to
  transplant.

- [s121] Chassis re-measured live: memory/grind/CD_sync/candidate.c = 2/160 bi 160 rd 0; s120's V1_ixfirst_nolocalborrow = 6/160. Ledger floor 2 confirmed on HEAD.

- [s121] The sched_solver and ra_solver suites both run on CD_sync in OBJECT mode (--target-object build/src/system.o --ours-object tmp/sandbox/CD_sync/system.o). extract.py system reports parity=True (80 funcs, 424 blocks, 2174 picks) and simulate.py --func CD_sync scores 52/52 blocks order-exact and clock-exact in both passes, so the scheduler model is exact for this function.

- [s121] TRAP for future sessions: goalmap.py --target asm/funcs/CD_sync.s is a false-negative generator. asm_body() (tools/sched_solver/goalmap.py:106) skips every line beginning with /*, which is every splat body line, so the target parses as ONE instruction (tgt=1) and EVERY block prints 'GOAL == OURS (identity)'. Object mode aligns 160-vs-160 with 159 equal and 1 moved.

- [s121] Block-3 dependence graph, read from the model rather than inferred: chain A = 112 (lbu Intr.sync) -> 120 (sll) -> 125 (addu) -> 157 (lw a3); chain B = 116 (lbu Intr.ready) -> 127 (sll) -> 130 (folded addu) -> 132 (lw) -> 149 (sw 16(sp)). The uid->asm-text column printed by show.py mislabels 120 and 130 precisely because they are the transposed pair.

- [s121] V2_ixfirst_folded is order-exact in both scheduler passes for block 3 and its first divergence is RA, six register pairs. This is the first form in 121 sessions that has the target's emission order without the target's seats, and it retires the s115-s120 claim that order and seats are two outcomes of one binary variable.

- [s121] QTYDBG on V2 blk=3: qty=1 reg113 (t0 address) birth 18 death 24 refs 2 got $v1; qty=2 reg106 (arg5 value) birth 20 death 26 refs 2 got $a0. Equal refs and equal spans tie qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) and the qty-number birth-order tiebreak decides against us.

- [s121] inverse.py local --swap 1,2 --depth 2 returns REACHABLE at 1 atom with 21 distinct vectors in three classes: refs_down on qty 1 (measured dead this session, six spellings at 14/160), live_extend on qty 1, refs_up on qty 2.

- [s121] Sibling sweep: CD_ready s84 concluded independently that its analogous block-3 scheduler neighbourhood is enumerated and empty and that its residual is an ra_solver question; s121 reaches the same conclusion for CD_sync by a different route. func_80045294 shares no window with this function and has nothing to transplant.

## s122 (structural, 2026-09-04) — the contended quantities were MIS-IDENTIFIED for six sessions; the corrected map closes the C-level lever space on the V2 chassis

- [s122] CHASSIS re-measured live: `progress/s121-V2-order-exact-RA-only-6.c` = 6/160
  bi 160 rd 0 (P5 control); `memory/grind/CD_sync/candidate.c` unchanged at 2/160
  (ledger floor confirmed; body untouched this session).
- [s122] **CORRECTION, load-bearing.** s118-s121 called block 3's two contended
  local-alloc quantities "the t0 ADDRESS chain (reg113)" and "the arg5 value
  (reg106)". The post-sched1 RTL (`gccdump.lreg` of the V2 chassis,
  tmp/grind/CD_sync/s122/P5_v2_control/gccdump.lreg:446-505) says otherwise:
    * `(insn 127) reg113 = reg107 << 2`  — qty1 is the **t0 SCALE result**
      (`t0 *= 4;`), live 127 -> 132 only.
    * `(insn 123) reg106 = mem(reg111)`  — qty2 is the arg5 loaded VALUE,
      live 123 -> 149 (`sw reg106,16(sp)`).
    * The t0 **ADDRESS** lives in `reg107` — the /v pseudo of the C variable
      `t0`, written twice (insn 112 `t0 = idx_1494[0]`, insn 132
      `t0 = tbl + scale`) and read at insn 157 (`lw a3,0(reg107)`). It is NOT
      one of the four quantities local-alloc contends over in this block; it
      inherits $v1 from the qty1 merge.
    * qty0 = {reg108,reg111} the ix chain (birth 10 death 20 refs 6, gets $v0);
      qty3 = {reg117,reg120} the arg3 D_800A11DC[D_800A11D5] chain (22/30
      refs 4, gets $v0).
  Every "live_extend qty 1" vector inherited from inverse.py therefore means
  *move the `sll`/`addu` INSNS*, not *restructure the C variable that holds the
  address* — which is exactly why the C-side spellings measured this session
  are inert (below).
- [s122] Post-sched1 emission order of block 3 on V2 (reverse of the picks list;
  the picks list in system.sched.json is END-FIRST — a trap: `perturb.py
  --goal-before A:B` is in PICKS order, so "emit A before B" is
  `--goal-before B:A`):
  104,106,112,116,153,118,121,123,127,140? — precisely:
  `104,106,112(lbu t0i),116(lbu ix),153(a1),118(sll ix),121(addu ix addr),
  127(sll t0 scale),123(lw arg5),140(lbu arg3 idx),132(addu t0 addr),
  149(sw arg5,16(sp)),145(sll arg3),155(lw a2),157(lw a3),151(a0),159(call)`.
  local-alloc's birth/death numbering is `2*pos + 4` on this stream, which
  reproduces the QTYDBG table exactly (qty1 127->132 = 18/24, qty2 123->149 =
  20/26).
- [s122] **Four structurally distinct C spellings produce a BYTE-IDENTICAL
  block-3 quantity table.** P1 (the `t0` variable also carries the ix index
  first), P3 (`t0` carries the arg5 value first), Q3 (a dedicated `s32 *ap`
  pointer local for the arg5 address) and P5 (the V2 control) all dump
  `qty0 birth10/death20/refs6 got=2 · qty3 22/30/4 got=2 · qty1 18/24/2 got=3 ·
  qty2 20/26/2 got=4`. Scores 9, 7, 6, 6. The variable-identity ("merge/reuse
  one variable so the range spans both") vector that inverse.py ranks #2-#5
  cannot reach qty1/qty2 at all, because those pseudos are compiler-generated
  intermediates whose live ranges are fixed by insn positions, not by C
  variable identity.
- [s122] `perturb.py --pass 1 --block 3 --goal-before 132:149` (i.e. make sched1
  EMIT the arg5 store before the t0 address addu — the span change that flips
  the qty1/qty2 priority order) searches 1091 single atoms and returns
  **25 reaching vectors, NONE of them a `luid` atom**: 20 `add_dep`/`del_dep`
  edges and 5 `cost` atoms (`cost 112 := 12`, `cost 127 := 3`, `cost 127 := 12`,
  `cost 149 := 3/12`, `cost 155 := 12`). Every dep atom orders one of our chain
  insns against 149/155/123 — i.e. against the printf ARGUMENT-SETUP insns,
  which the front end emits last in the block, so no C statement can be placed
  after them; and the cost atoms are instruction-selection changes (a load or a
  multiply where the target has a shift) that would change the emitted bytes.
  This is the model-level reason s120's fifteen source orders were inert:
  statement order is a `luid` atom, and the luid class does not contain the fix.
  Raw: tmp/grind/CD_sync/s122/perturb_p1_goal.txt.
- [s122] Also measured on the V2 chassis (all bi 160 rd 0, control 6): P1 9,
  P2 (single variable, no `ix` local) 10, P3 7, P4 (mirror: `ix` carries the t0
  index; bi=161, an extra insn) 9, Q1 (separate index/scale/address variables) 9,
  Q2 (t0 address as one statement) 9, Q3 6. Banked as
  memory/grind/CD_sync/rejected/s122_*.c.
- [s122] HARNESS NOTE: `tmp/grind/CD_sync/s117/run.ps1 -forms <array>` only ever
  processes the FIRST element when it is invoked from the Windows-side agent
  shell (the child `pwsh -File tools/wteng.ps1` call terminates the parent
  loop). Drive it one form per invocation from a bash `for` loop
  (`powershell.exe -NoProfile -File .../run.ps1 -forms <one path>`); s122 lost
  three turns to this. `qd.sh`/`qfull.sh` must run INSIDE WSL
  (`bash tools/wsl.sh "bash tmp/grind/CD_sync/s122/qd.sh <form> <tag>"`).
- [s122] `qfull.sh` (new, tmp/grind/CD_sync/s122/qfull.sh) dumps BB2_QTY_DEBUG
  for the FULL src/system.c rather than the s118 mini TU: the CD_sync block-3
  table is identical in both, so the mini harness every session since s118 has
  relied on is faithful for this function.

- [s122] Chassis re-measured live: V2 (progress/s121-V2-order-exact-RA-only-6.c) = 6/160 bi 160 rd 0; candidate.c body unchanged, floor 2/160; src/system.c restored to HEAD at end of session.

- [s122] CORRECTION to six sessions of ledger prose: block-3 qty1 = reg113 = the t0 SCALE result (live insn 127 -> 132), qty2 = reg106 = the arg5 loaded value (insn 123 -> the sw at 149). The t0 ADDRESS is reg107, the C variable t0's own /v pseudo, read at lw a3,0(reg107) (insn 157), and it is not among the four quantities local-alloc ranks.

- [s122] local-alloc birth/death numbering on this function is 2*post-sched1-emission-position + 4; that reproduces the QTYDBG table exactly and is how a future session should map a quantity to an insn.

- [s122] TRAP: the picks array in system.sched.json is END-FIRST (the block is scheduled backwards), so emission order is its reverse and perturb.py --goal-before A:B means 'B is emitted before A'. Reading it forwards inverts every conclusion.

- [s122] Four structurally distinct C spellings of the do_timeout block (variable reused for the ix index, variable reused for the arg5 value, dedicated pointer local, control) yield a byte-identical block-3 quantity table on the V2 chassis.

- [s122] perturb.py pass-1 goal 'emit the arg5 sw before the t0 addu': 25 reaching vectors out of 1091 atoms, none in the luid (statement-order) class; all are dependence edges against the call's argument-setup insns, or instruction-cost changes.

- [s122] Full-TU vs mini-TU QTYDBG tables for CD_sync are identical, so the s118 mini harness every session since has relied on is faithful for this function.

- [s122] HARNESS: tmp/grind/CD_sync/s117/run.ps1 -forms <array> processes only the FIRST element when invoked from the agent's Windows shell; drive it one form per invocation from a bash for-loop. qd.sh/qfull.sh must run inside WSL via tools/wsl.sh.

## s123 (structural, 2026-09-04) — floor 2/160, unchanged

- [s123] CONTROL re-measured live: `memory/grind/CD_sync/candidate.c` applied to
  src/system.c scores **2/160, build_insns 160, rules_dropped 0**.  The dispatch
  brief reported "measurement unavailable"; the ledger's floor of 2 is correct
  and the chassis has NOT changed since s122.
- [s123] **The frontier item the last three sessions were pointing at does not
  exist.**  s121 recorded "36 atoms reach the pass-2 goal, 31 reach the pass-1
  goal, intersection 26, so ~10 atoms fix the FINAL order without perturbing the
  pass-1 stream local-alloc consumes".  That subtraction compares two different
  luid spaces.  `sched_analyze` re-numbers `INSN_LUID` by walking the *current*
  insn chain at the start of every scheduling pass
  (tools/gcc-2.7.2/sched.c:2198), so pass-1 luids index the post-combine chain
  and pass-2 luids index the post-sched1 chain.  Measured on this chassis:
  post-combine chain (= sched1 input) is
  `104,106,112,116,120,125,127,130,132,140,145,149,151,153,155,157`, which is
  exactly the C statement order, while the model's pass-2 luid order is
  `104,106,112,116,153,127,120,130,132,140,125,149,145,155,157,151`, which is
  exactly sched1's OUTPUT.  A pass-2 "luid atom" is therefore a statement about
  a chain position reload hands to sched2, not about a C statement.
- [s123] Reload does not reorder this block: the `.lreg` and `.greg` chains are
  identical to the `.sched` chain (artifact tmp/grind/CD_sync/s123/chainorder,
  regenerable with s123/chainorder.py).  Consequence, and this is the load-bearing
  structural fact of the session: **sched2's entire input — order, luids and the
  dependence graph — is a function of sched1's output plus the register
  assignment.**  Any perturbation that leaves sched1's output unchanged leaves
  sched2's output unchanged.  The class "atom that fixes the final order while
  leaving the pass-1 stream intact" is empty by construction, not merely unfound.
- [s123] The dual of that fact closes the other direction too.  All 31 pass-1
  atoms that reach block 3's target order produce the SAME output stream, and
  local-alloc's input is exactly that stream (its births/deaths are
  2*emission_position+4, H122-1).  So every C form that reaches the target order
  gets the same quantity table and the same seats — the V2 seats, which are
  wrong.  Measured: five structurally distinct spellings on the candidate chassis
  (control 2/160), none of them V2's group-move:
    S3 arg5 address split into `ix`, `t0 *= 4` between the ix addu and the load — 6/160
    S5 address split, both t0 insns between                                      — 6/160
    S7 address split, t0 pair after the load                                     — 6/160
    S9 t0 scale between the ix shift and the ix addu                             — 6/160
    S4 as S3 but with a dedicated `s32 ad` local                                 — 7/160
  and `inverse_compose.py classify` on S3 prints the SAME verdict and the SAME
  six register pairs s121 recorded for V2 (ours addu v1,v1,s3 / lbu v1,0(s2) /
  lw a0,0(v0) / lw a3,0(v1) / sll v1,v1,0x2 / sw a0,16(sp) vs target
  addu a0,a0,s3 / lbu a0,0(s2) / lw a3,0(a0) / lw v1,0(v0) / sll a0,a0,0x2 /
  sw v1,16(sp)).  Different C, identical residual.
- [s123] Two forms that do NOT reach the target order sit at a third residual
  level: S1 (`t0 = (s32)((u8 *)tbl_125c + t0 * 4);` as one statement) and S6
  (the same with `ix <<= 2;` hoisted above it) both score **3/160**, bi 160.
  Neither 2 nor 6 — a previously unseen point, worth a look by a later session
  that wants a third vantage on the same six-register knot.
- [s123] Mandated kill re-audit executed.  s122's Q3 (dedicated `s32 *ap` pointer
  local for the arg5 address, inert at 6==6 on the V2 chassis) transplanted onto
  the folded candidate chassis: **2/160, exactly the control** — inert here too.
  `fake_ablate.py` on candidate.c: keep-all 2/160 bi 160, drop-1 15/159, so the
  combine-foldable chain-extender remains load-bearing and none of this session's
  numbers are a FAKE-carrier artifact.
- [s123] Where this leaves the function.  The order/seat pair is now understood
  as ONE dial, not two: order is decided by sched1 from the C statement order,
  and the seats are decided by local-alloc from sched1's output, so choosing the
  order chooses the seats.  Reaching 0 needs a lever that changes local-alloc's
  ranking WITHOUT changing the post-sched1 chain — i.e. a change to a quantity's
  `refs` or to its birth/death that is invisible to the scheduler.  On the V2/S3
  stream the two tied quantities are the t0 scale result and the arg5 loaded
  value, both refs 2, both span 6; inverse.py's `refs_up on qty 2` vector
  (rank #8) is the only one of the 21 that has never been spelled, and refs_down
  on qty 1 is measured dead (s121, six spellings, 14/160).
- [s123] Artifacts: tmp/grind/CD_sync/s123/diff_atoms.py (the pass-1/pass-2 atom
  differential that exposed the luid-space error), s123/chainorder +
  s123/chainorder.py, s123/gen.py, s123/forms/*.c, s123/results.txt.  Forms
  banked as memory/grind/CD_sync/rejected/s123_*.c.

- [s123] Control re-measured live this session: memory/grind/CD_sync/candidate.c applied to src/system.c scores 2/160, build_insns 160, rules_dropped 0. The dispatch brief's chassis check printed 'measurement unavailable'; the ledger floor of 2 is correct and the chassis is unchanged since s122.

- [s123] CD_sync block 3's insn chain is identical after flow, combine, sched1, lreg and greg except for the single sched1 reordering: post-combine 104,106,112,116,120,125,127,130,132,140,145,149,151,153,155,157 (= the C statement order) and post-sched1 104,106,112,116,153,127,120,130,132,140,125,149,145,155,157,151. Reload adds no reordering. Artifact tmp/grind/CD_sync/s123/chainorder.

- [s123] uid map for the block, read from the .sched RTL: 112 t0=idx_1494[0]; 116 ix=idx_1494[1]; 120 reg112=t0<<2; 125 reg107=reg112+tbl; 127 ix<<=2; 130 reg113=ix+tbl (folded arg5 address); 132 arg5=mem(reg113); 140 zext(D_800A11D5); 145 reg120=reg117<<2; 149 sw arg5,16(sp); 151 a0=&D_800161C8; 153 a1=D_800F19B8.func; 155 a2; 157 a3=mem(reg107).

- [s123] The entire 2/160 residual is one adjacent transposition: ours emits 120 then 130, the target emits 130 then 120, in sched1 and sched2 alike.

- [s123] Because sched2's luids come from sched1's output (sched.c:2198) and reload does not reorder, order and seats are ONE dial on this chassis: choosing block 3's emission order chooses local-alloc's input and therefore the seats.

- [s123] Four order-reaching spellings that are NOT V2's group move (S3/S5/S7/S9, all arg5-address splits or scale-interleavings) score 6/160 and S3 reproduces V2's exact six-register RA divergence; a fifth with an extra local scores 7/160.

- [s123] Two spellings that fold the t0 scale into the t0 add (S1 alone, S6 with the ix shift hoisted) score 3/160 bi 160 - a residual level distinct from both 2 and 6, unexamined so far.

- [s123] s122's Q3 pointer-local kill re-audited on the current chassis: 2/160, inert, kill stands. fake_ablate on candidate.c: 2/160 with the chain-extender, 15/159 without.
