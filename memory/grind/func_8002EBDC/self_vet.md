# SELF-VET — func_8002EBDC

CONSTRUCTS: ten canonical GTE cop2 islands (two GTE passes x {LZCS/LZCR leading-zero-count with "=m" sp_tmp operand [pass 1 only], gte_SetRotMatrix, gte_ldlv0 (PsyQ 4.5 inline_c.h:101-110 body incl. the lhu/lhu/sll/or VX0/VY0 pack), gte_rtv0 = cop2 MVMVA .word 0x4A486012, gte_stlvnl}); everything else is ordinary C. No register pins, no `move %0,%1` aliasing blocks, no scheduling barriers, no FAKE constructs, no dead stores, no constant holders, no volatile.

## T1 semantic purpose
Every island performs the function's real GTE work (leading-zero count feeding the magnitude estimate; loading the rotation matrix into cop2 control regs; loading V0; the MVMVA rotate; storing MAC1..3). Removing any of them changes the function's output. The C statements (ratan2 calls, squared magnitude, LUT lookups, identity matrix, RotMatrix* calls, three signed /256 scalings, negated second pass) are all consumed. PASS.

## T2 human-programmer
A PsyQ programmer writes exactly this: `gte_SetRotMatrix(mat); gte_ldlv0(v); gte_rtv0(); gte_stlvnl(out);` plus the LZCS/LZCR count, with the rest as plain C. The islands are the published SDK macro bodies; the head/tail read as the obvious spelling of the spec (rotate a vector by the direction's yaw/pitch, scale it, rotate back). Nothing invites "why is this here?". PASS.

## T3 GCC-internals justification
No construct is justified by a GCC pass. The islands exist because cop2 ops have no C form (canonical, inline-asm-policy). The ordinary-C facts recorded in evidence.md (cse EBB folding of the scratchpad constant, `$s2` constant hold, signed-divide idiom) are DESCRIPTIONS of why the natural spelling matched, not levers steering any pass. PASS.

## T4 permuter/search provenance
No permuter or search was run. The body is a hand-written transplant of the matched sibling func_8002E838's structure onto this function's signature; it scored 0 on the first measurement. PASS.

## T5 family check
The islands are the owner-sanctioned COP2 addressing-preamble cluster template (func_8002EBDC enumerated by name in the rule's census table, `.claude/rules/cop2-addressing-preamble-cluster.md:80`), character-identical to func_8002E838's islands (inline_asm_canonical.txt:373) and func_800203B4's (:367). The 4-point mechanical check: (1) sandbox --disable all == 0 (182/182); (2) zero pins / aliasing blocks / barriers; (3) in-island GPR limited to the macro bodies' own text (move $12 preamble, lw/ctc2, the gte_ldlv0 pack admitted by owner Ruling A 2026-09-02, cluster rule :163-175); (4) layer-1/Judge + verify-oracle are the driver's steps (verify-oracle --rebuild refused this session by the dirty-inputs guard). No forbidden family matches by analogy: no `.word` GPR injection, no hardcoded-$N outside the granted macro bodies, no coercion of any kind in the C. PASS.

## T6 naming-announces-intent
Locals: sp_tmp (the LZCR "=m" landing slot, read into lzcr — same name as every sibling island), scr, mat, vec, angle, dist_sq, dist, lzcr, shift, tbl. All are read after being written; none is pad/dummy/unused/spill. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: COP2 addressing-preamble cluster (canonical GTE islands; owner ruling 2026-08-17, widened anchor 2026-09-01, condition 3 clarified 2026-09-02)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:80`
  PRECEDENT: `inline_asm_canonical.txt:373`
  PRECEDENT: `inline_asm_canonical.txt:367`
  PRECEDENT: `docs/grind/decisions.md:20268`
  FAMILY: canonical inline asm (GTE/cop2) — inline-asm-policy
  SCOPE: "Two-category inline-asm policy: CANONICAL (GTE/cop2/BIOS/HW) is authentic and fine; CHEAT (register pins, INLINE_MOVE_ALIASING, scheduling barriers) is forbidden — a function carrying any cheat-asm is INCOMPLETE."
  PRECEDENT: `.claude/rules/inline-asm-policy.md:4`
  PRECEDENT: `src/code6cac_b.c:1366`

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Every island comment names the SDK macro it reproduces (gte_SetRotMatrix / gte_ldlv0 with inline_c.h:101-110 / gte_rtv0 / gte_stlvnl; LZCS/LZCR block citing func_8002E838 / func_8001A67C) per the 2026-09-02 Ruling A comment requirement.

HONEST BUCKET: COMPLETED-INLINE-ASM-CANONICAL (islands present, not yet allowlisted) — the driver must route a Judge PASS through the owner-cluster grant door and write the inline_asm_canonical.txt line before `queue done`, exactly as for func_8002E838 / func_8002EA24 / func_80031890. func_8002EBDC has no row in tools/grinder/owner_cluster_grants.txt (operator-only); the landed 2026-08-17 cluster ruling's census table naming it is the authority (same as the func_80031890 PASS, decisions.md:20268).
