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
