# Hypothesis ledger — func_8007C7A0

## s4 (2026-08-08, permuter, git HEAD 8be92044)

### H11 — the never-permuted stream51 chassis contains permuter-reachable legitimate levers the model scans could not see — CONFIRMED (floor 15 -> 12 stream-exact; two levers found and honest-verified)
Statement: all four prior permuter runs (rounds 1, 2, 12, 13; ~125k iters)
ran on the pre-stream51 chassis; the stream-exact body postdates them. A
campaign from stream51 is a model-independent empirical search that can also
reach pseudo-MERGING spellings (variable reuse), which the s0-s3 model
vocabulary (prefs/conflicts/refs/livelen of a fixed graph) cannot express.
Probe: three tools/permuter_campaign.py campaigns (~77k iters total, all
harvest-stopped in-session), each novel find honest-measured via
`sandbox --disable all` applied to src/display.c (then reverted).
Result: TWO legitimate levers found and verified — L1 tx-reuse as dispatch
discriminant (14/51), L2 hi-staged Y sign check (12/51 combined). The
12-form is stream-exact AND ties the overall floor; it fixes X-join->$v0
(the assignment T1 called impossible — T1's scope is the OLD graph only;
pseudo-merging builds a new graph) and ylim-save->$a0. Residual: 5 roles
(carrier a2->a3, xlim-save v1->a2, sxt(y) v1->a2, lo a0->v0, const v0->a0).
Verdict: CONFIRMED.

### H12 — the permuter can also close the remaining 5-role residual from the 12-form basin — KILLED (for random permutation of THIS chassis)
Statement: with the closer 12-base, continued random search reaches further
allocation flips.
Probe: campaign 3 from the 12-form base (weighted 75), 8 jobs, ran to a full
9-min zero-novel window (77k cumulative iters).
Result: ZERO legitimate novel finds. The entire sub-basin is the UB
dead-read-cross-arm family (stage x in one arm, read uninit in the other —
rejected/ub-dead-read-cross-arm-family.c). Its diagnostic content: the
missing conflict is a second x-carrying pseudo live ACROSS the dispatch
branch; all measured legitimate spellings of that intent fail (dup-into-arms
13, unconditional pre-dispatch copy 20/52-insns, split-assign inert 14).
Random permutation of the 12-chassis is exhausted; the residual needs either
a structurally different chassis seed or model re-extraction + backward
solve on the NEW merged-pseudo graph (structural/forensics frontier).
Verdict: KILLED.

## s3 (2026-08-08, structural, git HEAD bec399f1)

### H10 — a minimal atom set achieving the full 9/9 assignment exists at depth >= 4 and is spellable — KILLED (constructively: the unique minimal solution requires an unspellable atom AND byte-contradicted edges)
Statement: depth <= 3 was exhausted (0 hits); rather than brute-forcing depth
4+, constraint-solve BACKWARD from the target assignment for the minimal
perturbation sets that make it the unique ascending-scan outcome, then check
each member's spellability (the s2 frontier's stated next probe).
Probe: tmp/grind/func_8007C7A0/s3/backward_solve.py (backward derivation +
Sim verification + drop-one necessity + 191-atom substitution scan + random
depth-4..8 T1 spot-check + exhaustive grant+pairs) and s3/grant_triples.py
(exhaustive grant+triples, 1,161,280 combos).
Result: the unique minimal full-target solution is GRANT(hcdel 79~2) + S5 =
{nopref 77, nopref 79, conf +78~83, conf +78~94, conf +78~92} — 9/9
Sim-verified, first full hit ever found. It is NOT spellable, twice over:
(a) THEOREM T1 — hard_conf[79] ∋ 2 is invariant under the entire spellable
vocabulary at any depth and any order (find_reg excludes hard_conf in both
passes and the pref-upgrade filter; propagation only adds; no spellable atom
touches hard_conflicts), so 79 -> $v0 and hence 9/9 is impossible spellably,
period (0 violations in 4000 random depth-4..8 combos; explains every prior
scan's 7/9 ceiling — S5 alone scores 8/9 with exactly 79 wrong);
(b) all three S5 conflict edges require hi ($v1 holder, pseudo 78) live
across insns ~9-16, contradicted by target's own stream (r18 c7a0_v1_census:
first $v1 def at insn 39) — the ORIGINAL compilation could not have had them
either. Grant+pairs 0/18,336 and grant+triples 0/1,161,280 close all cheaper
completions; drop-one and substitution scans prove each S5 member necessary
and non-substitutable. CONSEQUENCE: the structural modality is exhausted —
the target allocation is inconsistent with GCC 2.7.2 global.c's mechanism on
ANY stream-exact input, conditional only on model fidelity (the forensics
frontier). Verdict: KILLED.

## s2 (2026-08-08, structural, git HEAD ef16e11d)

### H6 — a pairwise (2-atom) pref/conflict model perturbation reaches the full target assignment — KILLED
Statement: s0 scanned single atoms only; pairwise changes to the pref/conflict
graph were the last unexplored model space for the stream-exact body.
Probe: tmp/grind/func_8007C7A0/s2/pair_scan.py — all C(273,2)=37,128 atom
pairs simulated against the FULL 9-pseudo target assignment.
Result: 0 hits; best 7/9 requires pref 76->r3 AND pref 78->r3, both hard-reg
preferences a 2-param leaf cannot surface (s0 mechanism), and still leaves
tx/lo wrong. Extended to SPELLABLE-only vocabulary (192 atoms: refs/livelen/
±conflict/nopref) at depths 2 AND 3 exhaustively (18,336 + 1,161,280 combos):
0 hits, best 7/9. The allocation model is closed through every spellable
perturbation of size <= 3. Files: s2/pair_scan.{py,out},
s2/spellable_scan.{py,out}.

### H7 — a COMPLETED-C sibling with an analogous raw-halfword carrier copy evidences a reachable spelling family — KILLED (census complete, evidence negative)
Statement: verbatim-Sony module siblings (or any COMPLETED-C function with a
`move $a3,$a0` carrier prologue) would show how target-style allocations were
reached in pure C, or show the module uniformly carries debt.
Probe: mapped all census LIBGPU/SYS members to queue/completion states;
grepped asm/funcs repo-wide for the carrier-copy first insn; read the C of
both COMPLETED-C achievers (CdRead, _SsVmVSetUp).
Result: NO COMPLETED-C module member has the carrier-copy prologue — the only
two carriers in the module are the unmatched twins (func_8007C7A0/C86C, both
dist 20); five display.c neighbors incl. the 0xE2000000 builder are
COMPLETED-C without the pattern. The repo-wide achievers are model-consistent:
CdRead's $a2 holds a live 3rd param; _SsVmVSetUp's carrier conflicts with an
$a2-resident table value (insns 34-48), so ascending scan lands $a3 with no
special mechanism. No transferable spelling family exists in the corpus.

### H8 — the _SsVmVSetUp spelling (s32 params + narrow s16/u16 view locals, raw-param read in the in-range arm) reproduces the carrier=$a3 allocation — KILLED
Probe: applied the adapted spelling to src/display.c, honest sandbox.
Result: sandbox 25, build 44 (7 short). The clamp-result local coalesces into
$a0 (copy `cx = arg0` + arg0 dead after), so NO carrier move is emitted, and
the s16-param class's double lim-1 decrement + raw-limit saves are CSE-folded
away. Wide params cannot produce the 51-insn stream. rejected/
s32-params-narrow-view-locals.c.

### H9 — CLAMP-macro ternaries reassigning the params (SOTN CLAMP shape on BB2 globals) produce target's three-arm $v0 join + param-home copy — KILLED
Probe: `arg0 = (x<0) ? 0 : ((D-1<x) ? D-1 : arg0);` symmetric on both axes,
s32 params, honest sandbox.
Result: sandbox 29, build 31 (20 short) — GCC 2.7.2 folds the nested ternary
into a compact form (single shared lim-1, no saves, no carrier). The
if/else-nested s16-param spelling remains the only stream-exact class.
rejected/s32-params-clamp-macro-ternary.c.

## s1 (2026-08-08, recon, git HEAD fb7bfa90)

### H1 — K&R (old-style) parameter definition changes entry RTL — KILLED
Statement: the Sony provenance (get_cs, PsyQ 4.0 LIBGPU SYS, library-era code)
suggests the original may have used a K&R definition (`s32 f(arg0, arg1) s16
arg0, arg1; {...}`); K&R params are promoted to int with DECL_ARG_TYPE=int and
narrowed at entry, potentially producing different prologue RTL and a different
allocation than ANSI `s16` prototype params.
Probe: applied K&R spellings of BOTH inherited bodies (candidate.c and
candidate_stream51.c) and ran the honest sandbox.
Result: knr_candidate 12/50, knr_stream51 15/51 — EXACTLY the ANSI scores.
GCC 2.7.2 narrows K&R-promoted shorts identically to ANSI s16 params here; the
axis is inert. Files: rejected/knr-oldstyle-params.c,
tmp/grind/func_8007C7A0/s1/knr_*.c.

### H2 — published Sony reference C adopts — KILLED (different build)
Statement: sotn-decomp psxsdk or sozud/psy-q-decomp carries matched get_cs C
that could adopt.
Probe: fetched sotn-decomp src/main/psxsdk/libgpu/sys.c get_cs verbatim;
enumerated sozud/psy-q-decomp src tree via gh api.
Result: SOTN's get_cs is a STRUCTURALLY DIFFERENT library build — it clamps x
against constants (0x400-1) and y against `(D_8002C26C ? 0x400 : 0x200)-1` and
dispatches on a boolean global, while BB2's target clamps BOTH axes against
halfword globals (D_8009BE78/D_8009BE7A) and dispatches on a range check
`(u8)(D_8009BE74 - 1) < 2`. The SOTN spelling cannot even be transliterated
onto BB2's globals without changing semantics; its clamp SHAPE was already
measured in round 6 T1 at score 28 (rejected bank). sozud/psy-q-decomp
contains NO libgpu module at all (src/: snd, cd, etc, gs, api, press, c only).
Reference-adoption path is closed; ground-up (already done — stream51) stands.

### H3 — TU context affects codegen (module re-split lever) — KILLED
Statement: the original was compiled inside Sony's SYS module TU; GCC 2.7.2
keeps some TU-cumulative state (label_num), so compiling the same body at a
different TU position might change allocation — which would make the
census-backed module re-split a legitimate codegen lever.
Probe: tmp/grind/func_8007C7A0/s1/tu_probe.sh — cc1 (canonical flags) on full
src/display.c vs a bare 8-line mini TU (3 extern decls + stream51 body); diff
the function's emitted asm with local-label numbers normalized.
Result: IDENTICAL — all 86 instruction lines byte-equal; only .L numbering
differs (TU-cumulative label_num, no byte effect). TU position, preceding
functions, and file identity have ZERO effect on this function's codegen.
Module re-split can be justified only on organizational/provenance grounds; it
cannot change these bytes.

### H4 — some allocation ORDER reaches target (ref-lift family viability) — KILLED
Statement: the s0-ledger single-atom scan (273 atoms) perturbed prefs/
conflicts/nrefs one at a time; the sanctioned ref-lift families
(duplicated-statement-into-arms, F1 chain-extender) act ONLY through
reg_n_refs -> allocno priority -> allocation ORDER, and multi-allocno order
shifts were not exhaustively covered — maybe some order reaches the target
assignment.
Probe: tmp/grind/func_8007C7A0/s1/order_scan.py — re-extracted the RA model
from the stream51 body at current HEAD, then simulated ALL 9! = 362,880
allocation orders with prefs/conflicts/hard-conflicts held at their real
values, checking the full target assignment {tx:v0, hi:v1, lo:v0, limsave1:a2,
limsave2:a0, sxt_a0:a0, sxt_a1:a2, arg1:a1, carrier:a3}.
Result: 0 of 362,880 orders reach the full target; best any order achieves is
5/9. (The gating sub-goal limsave1->$a2 alone IS order-reachable — 72,576
orders — but every such order leaves carrier/tx/lo wrong; the conflict graph
itself forbids the full assignment.) CONSEQUENCE: every reg_n_refs/priority
lever — duplicated-statement-into-arms, F1 chain-extender, any spelling that
only re-orders allocation — is mechanically dead for this function. Target is
unreachable by order alone; the prefs/conflict graph must change, and the s0
atom scan already proved only 3 single atoms do that, each needing a hard reg
this 2-param leaf cannot surface.

### H5 — alternative opt levels produce target roles (diagnostic only) — KILLED
Statement: (diagnostic for the Sony-library-build-flags question, in the
cc1psx-self-disproof spirit — NEVER a fix path; flags frozen 2026-05-20.)
Maybe Sony built LIBGPU with different optimization flags and our cc1 at that
level shows the target's carrier=$a3 / limsave=$a2 roles.
Probe: tmp/grind/func_8007C7A0/s1/flag_probe.sh — cc1 on the mini TU under
-O1 / -O2 / -O3 / -O2 -fno-defer-pop / -O2 -fcaller-saves /
-O2 -fno-schedule-insns.
Result: ALL configs emit byte-identical roles (carrier=$a2 `move $6,$4`,
limsave=$v1 `move $3,$2`). The allocation is invariant across every
optimization config our cc1 offers. Combined with s0's cc1psx byte-identity,
retry-never-fires, and H4's order-space zero: no configuration of OUR compiler
reaches the target allocation from the stream-exact body.

## [s1] Sony library provenance implies a K&R (old-style) definition whose promoted-param narrowing changes entry RTL and allocation
- mechanism: K&R short params carry DECL_ARG_TYPE=int and are narrowed at entry by assign_parms, a different RTL path than ANSI s16 prototype params
- probe: applied K&R spellings of both inherited bodies, honest sandbox each
- result: knr_candidate 12/50, knr_stream51 15/51 — identical to ANSI scores; narrowing path is the same
- verdict: KILLED

## [s1] Published Sony reference C (sotn-decomp psxsdk get_cs, sozud/psy-q-decomp) adopts onto BB2
- mechanism: verbatim library code family; 65 prior functions closed by reference adoption
- probe: fetched sotn-decomp libgpu/sys.c get_cs verbatim; enumerated psy-q-decomp src tree
- result: SOTN get_cs clamps against constants (0x400/0x200-1) with boolean dispatch — structurally different build, not transliterable onto BB2's global-limit clamps + range-check dispatch (its shape = round-6 score 28); psy-q-decomp has no libgpu module at all
- verdict: KILLED

## [s1] TU context (Sony SYS module vs display.c) affects this function's codegen, making census-backed module re-split a codegen lever
- mechanism: GCC 2.7.2 TU-cumulative state (label_num) as candidate carrier of cross-function influence
- probe: tu_probe.sh: cc1 canonical flags on full display.c vs bare 8-line mini TU, diff function asm with labels normalized
- result: byte-identical instruction stream (86/86 lines); only label numbering differs; TU position has zero byte effect
- verdict: KILLED

## [s1] Some allocation ORDER reaches the target register assignment, leaving the sanctioned ref-lift families (duplicated-statement-into-arms, F1 chain-extender) viable
- mechanism: reg_n_refs -> allocno priority -> allocation order is the only mechanism those families act through; the s0 atom scan did not exhaust multi-allocno order shifts
- probe: order_scan.py: re-extracted RA model at HEAD, simulated all 9! = 362880 orders with prefs/conflicts held at real values
- result: 0 full-target hits; best 5/9; sub-goal limsave->$a2 alone hits in 72576 orders but always with carrier/tx/lo wrong — the conflict graph forbids the full assignment; ref-lift families mechanically dead here
- verdict: KILLED

## [s1] (diagnostic) an alternative optimization config of our cc1 produces the target's carrier=$a3/limsave=$a2 roles, evidencing Sony library build-flag divergence
- mechanism: opt-level-dependent RA differences (never a fix path; flags frozen 2026-05-20)
- probe: flag_probe.sh: -O1/-O2/-O3, +/-fno-defer-pop, +/-fcaller-saves, +/-fno-schedule-insns on the mini TU
- result: all configs emit identical roles (carrier=$a2, limsave=$v1); allocation invariant across every config our cc1 offers
- verdict: KILLED

## [s2] A pairwise (2-atom) pref/conflict model perturbation reaches the full target assignment
- mechanism: s0 scanned single atoms only; pairwise pref/conflict-graph changes were the last unexplored model space for the stream-exact body
- probe: tmp/grind/func_8007C7A0/s2/pair_scan.py: all C(273,2)=37,128 atom pairs simulated against the full 9-pseudo target assignment; then spellable-only vocabulary (192 atoms) exhaustively at depths 2 and 3 (18,336 + 1,161,280 combos) via s2/spellable_scan.py
- result: 0 full-target hits at every depth; best 7/9 requires two hard-register preferences (pref 76->r3 + pref 78->r3) that a 2-param leaf cannot surface, and still leaves tx/lo wrong; spellable-only best is also 7/9 with two of {79,92,83} always wrong
- verdict: KILLED

## [s2] A COMPLETED-C sibling with an analogous raw-halfword carrier copy evidences a reachable pure-C spelling family for carrier=$a3
- mechanism: same library build / same compiler era; a matched module member with the move $a3,$a0 prologue would be direct evidence of a reachable spelling; uniform module debt would support the toolchain-revision-divergence reading
- probe: mapped all census LIBGPU/SYS members (manifest_report.md:100-122) to queue/completion states; repo-wide grep of asm/funcs for the carrier-copy first insn; read the C of both COMPLETED-C achievers (CdRead src/system.c:1188, _SsVmVSetUp src/main.c:1262)
- result: No COMPLETED-C module member has the carrier-copy prologue - the only two carriers in the module ARE the unmatched twins; five display.c neighbors incl. the 0xE2000000 builder func_8007C97C are COMPLETED-C without it. Repo-wide achievers are model-consistent: CdRead's $a2 holds a live 3rd param; _SsVmVSetUp's carrier conflicts with an $a2-resident table value (insns 34-48), so ascending scan lands $a3 with no special mechanism. Nothing transfers.
- verdict: KILLED

## [s2] The _SsVmVSetUp spelling (s32 params + narrow s16 view locals, in-range clamp arm reading the RAW param) reproduces the carrier=$a3 allocation
- mechanism: wide param stays a genuinely multi-use raw pseudo (feeds the sign-extend AND the in-range read), copy-preferenced to the clamp result - a different pref/conflict graph than the s16-param class
- probe: applied the adapted spelling to src/display.c, honest sandbox
- result: sandbox 25, build_insns 44 (7 short): the clamp result coalesces into $a0 so no carrier move is emitted, and the s16-param class's double lim-1 decrement + raw-limit saves are CSE-folded away; wide params cannot produce the 51-insn stream
- verdict: KILLED

## [s2] CLAMP-macro ternaries reassigning the params (SOTN CLAMP shape on BB2's halfword globals) produce target's three-arm $v0 join + param-home copy
- mechanism: target's X-clamp routes all three arms through a $v0 join temp then copies into the param/carrier home - the classic lowering of x = x<0 ? 0 : x>lim-1 ? lim-1 : x reassigning the param
- probe: symmetric nested ternaries on both axes, s32 params, honest sandbox
- result: sandbox 29, build_insns 31 (20 short): GCC 2.7.2 folds the nested ternary into a compact form (single shared lim-1, no saves, no carrier)
- verdict: KILLED

## [s3] A minimal atom set achieving the full 9/9 target assignment exists at depth >= 4 and is spellable (the s2 frontier's stated next probe)
- mechanism: Backward constraint-solve the ascending-scan allocation: walk the priority order and derive per pseudo which pref/conflict/hard-conflict edits make the target register the unique outcome, then Sim-verify, prove necessity (drop-one), non-substitutability (191-atom replacement scan), and exhaustively close cheaper completions (grant+pairs, grant+triples)
- probe: tmp/grind/func_8007C7A0/s3/backward_solve.py + s3/grant_triples.py against tmp/ra_solver_work/func_8007C7A0.model.json via tools/ra_solver/simulate.Sim
- result: Unique minimal solution found and Sim-verified 9/9 (first full hit ever in this model space): GRANT(hcdel 79~2) + S5 = {nopref 77, nopref 79, conf +78~83, conf +78~94, conf +78~92}. NOT spellable, twice over: (a) THEOREM T1 — hard_conf[79] contains reg 2 ($v0) and is invariant under the entire spellable vocabulary (find_reg excludes hard_conf in pass0, pass1, and the pref-upgrade filter; propagation only adds; no spellable atom touches hard_conflicts), so 79->$v0 is impossible at ANY depth and ANY order (0 violations in 4000 random depth-4..8 combos; S5 alone scores 8/9 with exactly 79 wrong, explaining every prior scan's 7/9 ceiling); (b) all three S5 conflict edges require hi (pseudo 78, $v1) live across insns ~9-16, contradicted by target's own bytes (r18 c7a0_v1_census: first $v1 def is insn 39) — the original compilation could not have had them either. Grant+pairs 0/18,336; grant+triples 0/1,161,280 (best 7/9); every S5 member drop-one-necessary with zero single-atom substitutes.
- verdict: KILLED

## [s4] The never-permuted stream51 chassis contains permuter-reachable legitimate levers the s0-s3 model scans could not see (pseudo-MERGING spellings are outside the model's perturbation vocabulary)
- mechanism: Variable reuse merges the dispatch/staging value into an existing dead local's pseudo, changing the RTL pseudo GRAPH itself rather than prefs/conflicts of the fixed graph; all prior campaigns (rounds 1,2,12,13, ~125k iters) predate the stream51 body
- probe: Three tools/permuter_campaign.py campaigns (~77k iters, all harvest-stopped in-session); every novel find honest-measured via sandbox --disable all on src/display.c, then reverted
- result: L1 tx=(u32)(D_8009BE74-1) dispatch-discriminant reuse -> 14/51; +L2 hi=arg1 staged Y sign check -> 12/51 stream-exact. X-join now $v0, ylim-save now $a0. s32 holders fail (pkt=15 or breaks stream), pkt-staging fails (15), folded tail always +1. Residual 5 roles: carrier a2->a3, xlim-save v1->a2, sxt(y) v1->a2, lo a0->v0, const v0->a0
- verdict: CONFIRMED

## [s4] Continued random permutation from the 12-form basin closes the remaining 5-role residual
- mechanism: Closer base gives the random search a better start toward further allocation flips
- probe: Campaign 3 seeded from the 12-form (weighted 75), 8 jobs, run to a full 9-minute zero-novel window
- result: Zero legitimate novel finds; the entire sub-basin is the UB dead-read-cross-arm family (stage x in one arm, read uninit in the other). Legit spellings of its intent all measured dead: dup-into-arms 13, unconditional pre-dispatch copy 20 (52 insns), split-assign inert 14
- verdict: KILLED
