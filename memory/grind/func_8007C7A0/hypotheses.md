# Hypothesis ledger — func_8007C7A0

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
