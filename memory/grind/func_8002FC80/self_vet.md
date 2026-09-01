# SELF-VET — func_8002FC80

CONSTRUCTS: six s32 loads read through u8-pointer byte-offset casts of the three input point pointers (a0/a1/a2 at offsets 0/4/8), six direct s32 assignments of the computed differences to the fixed scratchpad addresses 0x1F800360–0x1F800378, three canonical GTE cop2 inline-asm islands (gte_SetRotMatrix / gte_ldlvl / GTE OP / gte_stlvnl), named locals v1/v2/p/ret, ratan2 tail with conditional +0x800 adjustment.

## T1 semantic purpose
Every construct is live and byte-material. The six load pairs and six difference assignments perform the function's actual work (two 3-D vector differences written into the GTE scratchpad workspace, consumed immediately by the islands); nothing is dead, discarded, unused, or address-of-only. The load spelling is not an inert decoration: the array-index alternative (`a1[i]`) emits DIFFERENT bytes (measured 34 @ 73/74 vs 0 @ 74/74 this session's chassis), so this is a choice between two live spellings of a required operation, not an addition with no observable effect. The islands encode cop2 operations that have no C form. PASS.

## T2 human-programmer
Byte-offset pointer casts are the established idiom of this exact file (`*(u16 *)(a0 + 0x272)`, `*(s16 *)(chk_obj + 0x86)`, dozens more), and func_8002FDB0 — the completed cluster member feeding the SAME six scratchpad slots into the SAME island sequence — ships the identical load idiom and the identical store spelling for this workspace. A 1998 PsyQ programmer writing two functions that stage the same GTE scratchpad workspace plausibly wrote both with the same house idiom. No element of this body raises a "why is this here?" question: no dead code, no extra variable, no annotation-requiring construct in the C. PASS.

## T3 GCC-internals justification
Honest statement: the CHOICE between the two natural load spellings was confirmed by measurement against target bytes, and the mechanism (MEM_IN_STRUCT_P / sched.c true_dependence) is documented in evidence.md s4. But the T3 cheat signal is a construct whose EXISTENCE is only explicable by a GCC pass — a semantically inert addition. These loads must exist in some spelling; selecting among live, ordinary-C spellings of required operations by measuring is the normal matching process (same class as sanctioned variable reuse, declaration order, sub-word reads — ordinary C, no annotation). No inert construct exists whose only purpose is pass manipulation. PASS.

## T4 permuter/search provenance
Not permuter/search output. Measuring this load spelling was mandated by the Judge's own 2026-08-31 20:06 constraint (measure the func_8002FDB0-shaped load spellings and explain the store-order difference), the spelling is taken from that completed cluster member's shipped source, and the mechanism was confirmed by pass-dump analysis on both functions (artifacts in tmp/grind/func_8002FC80/s1/). PASS.

## T5 family check
No forbidden family matches, by direct comparison: no register pins, no `$N` asm outside the granted islands, no scheduling barriers, no volatile in any spelling, no dead locals/arrays, no redundant same-value assignments, no constant holders, no do-while(0), no dead conditionals, no label pads, no alias renames. The u8* cast is address arithmetic on a live load — not a redundant WIDTH cast (F2 covers value-width casts; this cast changes the address expression's tree shape and is the idiom func_8002FDB0 ships for the same workspace). Against this function's driver ban list, checked entry by entry: (1) the aggregate-typed store form the 19:45 layer-1 review banned is ABSENT — the six difference assignments use exactly the scalar fixed-address spelling that review's "Next action" prescribed as the required fix; (2) no respelling of that banned choice is present — the store side is the prescribed form and the fix landed on the LOAD side, a different construct the Judge's 20:06 constraint directed be measured; (3) no whole-body asm block exists — the body is mixed C with three granted islands; (4) the OWNER-CLUSTER grant is cited only for the three cop2 islands, its intended object, never for whole-body replacement; (5) the distance-0 mixed candidate was kept and completed, not abandoned. PASS.

## T6 naming-announces-intent
Locals are v1, v2, p, ret — value/pointer/result names matching the file's conventions. No pad/dummy/unused/spill/slack names. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical-asm (cop2 addressing-preamble cluster islands — inline-asm authorization, not a coercion family; listed for completeness)
  SCOPE: "each member inherits the canonical-asm disposition subject to the same mechanical per-function check, applied by the Judge without re-escalation."
  PRECEDENT: inline_asm_canonical.txt:365

  (No SOTN coercion-family exception is claimed for any C construct — the C body is ordinary C.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no claimed family mandates a /* FAKE */ annotation; the canonical-asm islands are covered by the allowlist entry, which requires no in-source annotation, and carry the cluster-rule provenance comments).
