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
