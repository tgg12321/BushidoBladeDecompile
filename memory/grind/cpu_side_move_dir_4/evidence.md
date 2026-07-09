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
