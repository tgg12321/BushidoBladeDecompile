# Hypothesis ledger — func_8007C7A0

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
