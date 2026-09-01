# SELF-VET — func_800203B4

> STATUS NOTE (2026-09-01 retry session): this session's outcome is `ruling-request`, not
> `candidate-ready` — the cluster-membership question (evidence.md fact 9) must be ruled on
> before the family below can be claimed. This vet is corrected and kept current so the
> session that submits after a YES ruling can reuse it verbatim. The prior discard reason
> (0 verbatim SCOPE sentences) is fixed: the SCOPE line below is a single-line verbatim
> quote from the cited grant line, and PRECEDENT is a literal file:line.

CONSTRUCTS: four `__asm__ volatile` cop2 island blocks (gte_SetRotMatrix 5-word load,
gte_ldv0 pack+load with two in-block cop2 load-delay nops, `.word 0x4A486012` MVMVA,
gte_stlvnl store); one block-scoped intermediate `new_var` holding the
game_GetPlayerData return value; `arg0 += 0x354` pointer bump feeding island 4's operand.

## T1 semantic purpose
Islands: each performs real cop2 I/O (loads rotation matrix into GTE control regs, loads V0,
executes MVMVA, stores MAC1-3 to `arg0+0x354`) — removing any of them changes observable GTE
state and the function's output. `new_var`: holds the call result consumed by the table-index
load on the next line — real dataflow. `arg0 += 0x354`: computes the real store destination.
No construct is byte-decorative. PASS for all.

## T2 human-programmer
The islands are the PsyQ SDK's own inline GTE macro bodies (libgte inline_c.h family) — the
exact thing a 1998 PsyQ programmer wrote, and cop2 has no C form so inline asm is the ONLY
spelling. `new_var = game_GetPlayerData(...); src = *(s32 *)(idx*4 + new_var);` is ordinary
call-then-index code any programmer writes. PASS.

## T3 GCC-internals justification
No construct is justified by any GCC pass. The islands exist because cop2 has no C analog
(CLAUDE.md: "GTE (cop2) ops have no C analog — inline __asm__ for those is canonical").
No lever reasoning, no allocator/scheduler mechanism anywhere in the diff. PASS.

## T4 permuter/search provenance
No permuter or auto-search was run; the body is the pre-migration matched head (commit
83dc0e5d) plus a transplant of the owner-authorized func_8002FDB0 island spelling. PASS.

## T5 family check
Diff contains ZERO forbidden-family constructs: no register-asm pins, no move-aliasing
blocks, no free-standing GPR asm / bare nop blocks (the two GTE load-delay nops sit INSIDE
the granted-island-shape cop2 block, exactly as FDB0's authorized gte_ldv0-class block
carries them), no volatile coercion, no dead locals (historical m0/m1/m2 deliberately
dropped), no dead stores, no do-while(0), no scheduling barriers. The in-island GPR
instructions (move/lw/lhu/sll/or) are strictly the cop2 addressing/packing preamble of the
SDK macro bodies, which is the admissibility bound the grant states. OPEN ITEM: whether the
cluster ruling's membership extends to this function's $v0/$s0-source idiom spelling is the
subject of the 2026-09-01 ruling-request — until answered YES, the family claim below is
NOT spendable.

## T6 naming-announces-intent
Locals: mat, vec, src, new_var — all read, all semantically named, no pad/dummy/unused
naming. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical inline-asm GTE island (COMPLETED-INLINE-ASM-CANONICAL path) via the
    func_8002FDB0 cluster ruling, CONTINGENT on the pending 2026-09-01 ruling-request
  SCOPE: "Cluster ruling: the 26 queued siblings sharing this idiom inherit this disposition subject to the same per-function mechanical check (sandbox --disable all == 0, zero pins/aliasing/barriers, in-island GPR limited to cop2 addressing preamble)."
  PRECEDENT: inline_asm_canonical.txt:268
  Mechanical check status (re-measured 2026-09-01): sandbox --disable all == 0 (65==65,
    rules_dropped 0; artifact tmp/grind/func_800203B4/s1/code6cac_sandbox0_retry.o);
    zero pins / zero move-aliasing / zero barriers; in-island GPR = addressing/packing
    preamble only. Membership evidence + gap: evidence.md facts 3, 4, 9.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no rule cited above mandates a /* FAKE */
annotation; canonical-asm grants are annotated in inline_asm_canonical.txt by the driver,
not in source).
