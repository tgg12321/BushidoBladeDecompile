# SELF-VET — func_8002E838
CONSTRUCTS: cop2-island-LZCS/LZCR (addu $t4,%1 -> mtc2 $t4,$30 -> 2x nop -> addiu $v0,$sp,0x10 -> addu $t4,$v0 -> swc2 $31,0($t4); "=m"(sp_tmp), clobbers $2,$12), cop2-island-SetRotMatrix-5word (move $12 -> lw $13/$14/$15 -> ctc2 $0..$4), cop2-island-ldv0 (move $12 -> lhu $14/$13 -> sll/or -> mtc2 $13,$0 -> lwc2 $1 -> 2x nop), cop2-op-MVMVA (.word 0x4A486012), cop2-island-stlvnl (move $12 -> swc2 $25/$26/$27). Everything else in the diff is ordinary C (pointer-deref arithmetic, two ratan2 calls, identity-matrix stores, RotMatrixY/RotMatrixX calls, LZC-shift table lookup). No FAKE constructs, no register pins, no move-aliasing blocks, no barriers, no volatile coercion, no dead locals.
## T1 semantic purpose: every island performs a real GTE transfer/op with an observable effect on the function's output (LZCR feeds the sqrt-shift; SetRotMatrix/ldv0/MVMVA/stlvnl rotate obj+0xA8 in place). The ordinary-C part is the natural spec of the function. No construct is byte-neutral filler.
## T2 human-programmer: the islands are literal PsyQ libgte inline-macro bodies (gte_SetRotMatrix, gte_ldv0, gte_rtv0, gte_stlvnl) plus the hand-written LZCS/LZCR block; a 1998 PS1 programmer wrote exactly these via the SDK macros. The C around them is plain.
## T3 GCC-internals justification: none. No construct is justified by allocator/scheduler/DCE behaviour; the islands reproduce hand-asm/SDK-macro regions that have no C form (cop2 ops).
## T4 permuter/search provenance: no permuter, no search. Body composed by hand from the target asm + the matched siblings func_8002EA24 / func_8001A67C / func_800203B4; matched on the first compile.
## T5 family check: no forbidden family matched. No pins (no register-asm declarations), no hardcoded-$N GPR injection outside a granted island, no move-aliasing block (the move $12,%0 is the SDK-macro body's own copy inside the island, the enumerated cluster idiom), no barriers, no volatile-coercion, no dead locals/arrays, no width casts.
## T6 naming-announces-intent: sp_tmp is the LZCR output slot the swc2 stores to (real consumed value, named identically in the authorized siblings); mat, vec, dist_sq, angle, dist, lzcr, shift, tbl all name consumed values. No pad/dummy/unused names.
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical cop2 inline-asm island — owner COP2 addressing-preamble CLUSTER grant (canonical-asm disposition door, not a SOTN spelling family)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: `tools/grinder/owner_cluster_grants.txt:22`
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:78`
  PRECEDENT: `inline_asm_canonical.txt:371`
  PRECEDENT: `inline_asm_canonical.txt:367`
  PRECEDENT: `inline_asm_canonical.txt:266`
ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no sanctioned-spelling family is claimed; the islands are canonical asm under the owner cluster grant, which mandates no /* FAKE */ annotation)

## Cluster per-function mechanical check (cop2-addressing-preamble-cluster.md, 4 conditions)
1. `sandbox func_8002E838 --disable all` == 0 (119/119) — measured s1 2026-09-02.
2. zero register pins, zero move-aliasing blocks, zero scheduling barriers — the retired-chassis body.c (which carried pins + aliasing) was NOT used; the new body has none.
3. in-island GPR limited to the cop2 addressing preamble + the SDK-macro bodies already enumerated in the grant (move $12 / lw $13-$15 / lhu+sll+or pack of gte_ldv0 — same islands as func_800203B4, `inline_asm_canonical.txt:367`, and the LZCS form of func_8001A67C, `inline_asm_canonical.txt:266`).
4. `verify-oracle` run once this session: ok=true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa. Layer-2 cheat-reviewer is the driver's step.
