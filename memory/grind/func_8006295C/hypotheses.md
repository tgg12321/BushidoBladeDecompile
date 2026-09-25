# func_8006295C — hypotheses / ruled out (2026-09-25 manual lane)

All scores: `sandbox --disable all` (cheat-invisible). Variants in tmp/f295c/
(scratch); receipts banked in rejected/ (they use a local `PolyFT4T` typedef
name because they predate the POLY_FT4 typedef hoist, prep commit 1b52a751f,
and were measured before the explicit `return D_800A3460;` tail, which is
byte-neutral on the closing form: u6 -> r1 both 0).

## Ladder (420 -> 0)
| variant | change | score |
|---|---|---|
| base | first transcription, u8* prim, `* 0x33333333` diff | 143 |
| a | `bit = 1 << i` statement (inline form folds to srav/andi) | 139 |
| b | RotTransPers4 args via `sv = &D_8009BB84[j*16]` | 119 |
| d | `m = &mats[i]` (giv of i) + t via m->t[], decl order = spill order | 50 |
| f | POLY_FT4-typed prim, pointer-difference bound | 50 (same bytes class as d) |
| t3 | tail cursor = prim after `end = prim` | 41 |
| b1 | t3 + base staged through prim | 38 |
| u4/u6 | + tail index `zbuf[k]` (k fresh; reusing j also 0) | **0** |
| r1 | u6 + explicit `return D_800A3460;` (defined C) | **0** = candidate.c |

## H1 (B1) base staged through `prim` — LOAD-BEARING (FAKE, staged-value)
Target: `lw s1,D_800A34EC` ... `addiu v0,s1,120` (mats) FIRST ... `addiu s1,s1,392;
sw s1,80(sp)` ... `lw s1,D_800A37D4` ... `move s4,v0` (giv init at loop entry).
.loop dump (tmp/f295c/i/w6.i.loop.f): loop.c emits `(set r400 (reg mats))` as
the m-giv init; with a separate `base` local combine folds `mats = base+120`
into it (base pseudo is not set in between), so the target's `move s4,v0`
vanishes and base local-allocs to v0. Only a second write to base's pseudo
between insn 15 and the loop entry blocks the fold, and the only value the
target holds in s1 there is prim. Extra refs also lift prim's global priority
above j (prim s1 / j s2, as target).
Measured without it: rejected/fresh-base-local-37.c (37);
no base local, global re-read per init (e) 50; m as pointer biv (m1/m2) 93.

## H2 (T3) tail cursor = prim — LOAD-BEARING (FAKE)
Target tail: `beq v0,s1; move s2,s1 (end=prim)`; `move s1,v0` (cursor in prim's reg).
ALLOCDBG (tmp/f295c/adbg.sh u8): fresh cursor p pri 20000 (nrefs 10, livelen
15) is allocated before the zbuf[k] giv (12727) and takes s0; target has giv
s0 / cursor s1. Needs pri(p) < 12727 (livelen > ~24 or nrefs <= 7) without
conflicting with prim — no spelling found.
Tried, all 12+: while / for / p++ in the arg / &p[k] / (u32 *)ot / decl
orders / zp user var (37) / static inline link helper (z1-z3: 37/12/37).
Receipt: rejected/fresh-tail-cursor-12.c. Both fresh: rejected/fresh-base-and-cursor-49.c.

## H3 tail index
`zbuf[k]` puts the zp giv init after the loop entry test (target `lw s0,64(sp)`
after `beqz`); `*zp++` user pointer puts it before (37). Fresh k and reused j
both close; landing uses fresh k.

## Status (2026-09-25) — LANDED
COMPLETED-INLINE-ASM-CANONICAL: layer-2 PASS (FAKE 1 under staged-value bounds
1-6; FAKE 2 ruled same-role cursor walk). Commits 709c158ed (auth row),
c3eff5ec6 (Match), 4b52b50d5 (queue done). Pre-review record follows.
Landing prepared (src spliced, oracle green, sandbox 0 420/420) and handed to
the orchestrator for layer-2. Admissibility risk: H1/H2 are reuse of one local
across roles (base staging, fill cursor, link cursor); ordinary-c-judge-
decidable Rulings 5/6 do not cover them; the claim is staged-value-reused-
variable (borrowed variable has a real job, previous value dead, staged value
read by the following statements). If layer-2 FAILs: revert src to
INCLUDE_ASM, keep candidate.c, and the frontier is H1/H2 above.
