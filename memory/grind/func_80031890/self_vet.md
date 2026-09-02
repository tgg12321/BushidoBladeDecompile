# SELF-VET — func_80031890

CONSTRUCTS: cop2-island gte_SetRotMatrix (__asm__ volatile, "r"(mat), clobbers $12-$15), cop2-island gte_ldv0 (__asm__ volatile, "r"(vec), clobbers $12-$14, two explicit GTE load-delay nops), cop2 MVMVA (__asm__ volatile ".word 0x4A486012"), cop2-island gte_stlvnl (__asm__ volatile, "r"(vec), clobbers $12 + "memory"); everything else is ordinary C (no FAKE, no dead store, no constant holder, no alias, no do-while(0), no volatile).

## T1 semantic purpose: Each island performs real work the function's specification requires — loading the rotation matrix into the GTE control registers, loading the velocity vector into V0, running MVMVA, and storing MAC1..3 back over the velocity. Removing any of them changes the program's behaviour (the velocity is no longer rotated). The two nops inside gte_ldv0 are the GTE's architectural 2-cycle load delay, part of the SDK macro body. No construct is behaviour-neutral. The ordinary C (rng-gated angular nudge, identity matrix, RotMatrixY/X, /8-vs-/4 damping, pos += vel/2) is the function's logic verbatim.
## T2 human-programmer: A PsyQ-era programmer writing this would call gte_SetRotMatrix(m); gte_ldv0(v); gte_rtv0()/mvmva; gte_stlvnl(v) — the islands ARE those macro bodies expanded. A reader asks no "why is this here" question of any statement; the head local `av` read before rng_Next is the natural "read the old value, perturb it, write it back" order.
## T3 GCC-internals justification: None. No construct is explained by an allocator/scheduler/DCE/reorg mechanism. The only compiler fact invoked is the NEGATIVE one — GCC 2.7.2's MIPS backend emits no cop2 instructions, which is why the SDK ships these as inline asm macros in the first place.
## T4 permuter/search provenance: No permuter or search ran. The form was written by hand from the target asm and the func_8002E838 sibling, and matched on the first sandbox measurement.
## T5 family check: The islands are canonical inline asm under the owner COP2 addressing-preamble cluster ruling (2026-08-17, widened anchor 2026-09-01), with func_80031890 enumerated BY NAME as a confirmed carrier. They are not register pins (no `register ... asm("$N")`), not hardcoded-$N GPR injection (in-island GPR use is the SDK macro body's own addressing preamble; address operands are C values bound "r"), not move-aliasing blocks (the `move $12,%0` is inside the island as the macro's own materialize-then-copy, the same spelling the owner authorized at inline_asm_canonical.txt:367), not barriers. No forbidden family matches by analogy: the spelling is character-identical to two already-authorized in-tree siblings. The retired-chassis body's pins + `"move %0, %1"` aliasing blocks were NOT carried forward (banked in rejected/).
## T6 naming-announces-intent: Locals are mat, vec, angle1, angle2, sum_sq, adj, vx, vz, av — all describe program values and all are read. No pad/dummy/unused/spill names.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical inline asm — COP2 addressing-preamble cluster (owner cluster ruling; not a SOTN coercion family)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:156
  PRECEDENT: docs/grind/decisions.md:18120
  PRECEDENT: docs/grind/decisions.md:20262
  PRECEDENT: inline_asm_canonical.txt:367
  PRECEDENT: 49d6927e

Mechanical 4-point per-function check (self-applied): (1) sandbox --disable all == 0 at 163/163, rules_dropped 0; (2) zero pins / aliasing blocks / scheduling barriers; (3) in-island GPR instructions limited to the SDK macro bodies (move $12 preamble + the macro's own lw/lhu/sll/or), nothing else swallowed — every C-expressible instruction of the target is emitted from C; (4) verify-oracle ok=true, build_sha1 == oracle (tmp/grind/func_80031890/s1/verify_oracle_s1.txt). Fresh layer-2 cheat-reviewer is the driver's step.

Routing note for the driver/Judge (same as func_8002E838 and func_8002EA24): honest bucket is COMPLETED-INLINE-ASM-CANONICAL; scan_hand_coded is LOW 1/8 (GTE-wrapper artifact); func_80031890 has NO row in tools/grinder/owner_cluster_grants.txt (operator-only file) though it is named in the landed 2026-09-01 grant record — the inline_asm_canonical.txt line is the operator's/driver's to write, as for func_8002EA24 (decisions.md:18276) / func_8002FF20 (decisions.md:19971).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
