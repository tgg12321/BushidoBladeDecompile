# SELF-VET — func_80021A98

Session 3 (structural, 2026-08-07); re-verified verbatim by FOUR follow-up
structural sessions (all 2026-08-07, each dispatched as "s2" on a stale digest —
the s2+ ledger updates were uncommitted): in each, src had been rolled back
to the pre-s2 form, the identical banked diff was re-applied and sandbox
re-measured 0 (158/158, 19 rules dropped) live in src. Diff vs HEAD: (1) arg1 param retyped u8* -> s32
(src/code6cac.c signature + include/code6cac.h:442 prototype) and reused for the
byte value read at v0_50+6; (2) arg0 param reused for the else-arm table index via
split-init chain `arg0 = a3 << 2; arg0 += a3; arg0 <<= 2;`; (3) v1 retyped u16 -> s32
with in-place shift `v1 <<= 2;` per arm (replaces the `v1 * 4` subexpression);
(4) byte-offset addressing `*(s32 *)((u8 *)&D_801027B4 + arg0)` (replaces the
`(&D_801027B4)[idx]` word-index spelling); (5) second-table sums spelled
if-arm `*(u16 *)(v0 + 2) + D_80102768` / else-arm `*(s32 *)(... + arg0) + *(u16 *)(v0 + 2)`
(mixed operand order between the arms). Sandbox distance 0 (158/158, 19 rules
dropped, cheat-asm stripped) measured THIS session with these edits in place.

CONSTRUCTS: arg1-param-reuse (retype + live reassignment), arg0-param-reuse with
split-init accumulation chain, s32 v1 in-place shift, byte-offset pointer casts,
2-operand sum operand order (mixed between arms)

## T1 semantic purpose
Every construct is LIVE code with observable dataflow. arg1's reassigned value is
read (stored to s0+0x40); arg0's index value is read twice (both table loads);
v1's shifted value is read (both first-table sums); the byte-offset casts compute
the same addresses the word-index spelling did; operand order of a 2-operand `+`
is not an added construct at all — some order must be written. Nothing in the
diff is dead, unused, address-of-only, or discarded. PASS per construct.

## T2 human-programmer
Reusing dead scratch parameters as locals and writing staged arithmetic on them
is idiomatic 1990s C (and pervasive in this codebase's matched functions).
Byte-offset casts into table globals appear throughout src/code6cac.c. Both
operand orders of a binary `+` are natural C; no reader would flag any line as
purposeless. PASS.

## T3 GCC-internals justification
The DERIVATION of these spellings used the register-alloc-pure-c playbook
(instrumented cc1 allocno dumps, greg/lreg reading — the sanctioned diagnosis
route). The constructs themselves each carry ordinary program-logic content
(real values computed and consumed); none is a no-op whose only description is
compiler mechanics. The one construct whose defense referenced set_preference
mechanics — the mixed operand order — was explicitly submitted as a
ruling-request (session 2) and RULED ordinary expression spelling by the Judge
(docs/grind/decisions.md:4073-4075, 2026-08-07 22:21, PASS): "A 2-operand sum
has no parenthesization axis and no tree shape ... recovering the source order
that produces them is spelling reconstruction." PASS.

## T4 permuter/search provenance
No permuter or auto-search output in the diff. Every lever was a hand-derived
probe measured individually via sandbox (ledger P4/P9/P9b/P11/P12/P13,
evidence.md s2 table: 20 -> 15 -> 12 -> 10 -> 6 -> 2 -> 0). PASS.

## T5 family check
Forbidden families: none match. arg0/arg1 reassignments are NOT dead-param-assign
(Lever D) — that family is DEAD stores never read; both params here are read
after assignment (live reuse). No pins, no asm, no volatile, no dead locals, no
alias renames, no operand scrambling of a 3+-operand associative chain
(or-tree-shape-shift scope — see Judge ruling: 2-operand sums are outside it).
Sanctioned families claimed: variable reuse for codegen control; split-init
accumulation. See SANCTIONED-FAMILY-CLAIMS. PASS.

## T6 naming-announces-intent
Locals are v1/v0/a3/s0/idx-style register-role names matching the file's
established convention; reused params keep their arg0/arg1 names. No pad/dummy/
unused/spill/slack/_buf names; no declaration-only or address-of-only uses. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with \"FAKE but makes register allocation work\" comments."
  PRECEDENT: .claude/rules/no-new-park-categories.md:170 (SOTN-accepted techniques, resolved 2026-06-02 borderline-rule research; SOTN master src/dra evidence cited therein)

  FAMILY: Split-init accumulation (same-variable)
  SCOPE: "the **same-variable split-init accumulation** pattern is SANCTIONED — provisionally (\"for now\")."
  PRECEDENT: ad11a8c8 (the sanction's landing commit: "Match: func_80049C24 (text1b.c) — COMPLETED-C, retires 10 rules (USER-SANCTIONED split-init)"; also cited as a sanctioned class in the Judge PASS ruling on this very function, docs/grind/decisions.md:4075)

  FAMILY: 2-operand sum operand order (mixed between arms)
  SCOPE: "RULING: NO — it is ordinary expression spelling, and the candidate may proceed to candidate-ready on the banked zero form." ... "SCOPE OF THIS RULING: 2-operand commutative sums only. Multi-operand (3+) associative chains remain fully inside or-tree-shape-shift"
  PRECEDENT: docs/grind/decisions.md:4073-4075 (Judge PASS ruling on this exact function/construct, 2026-08-07 22:21, committed 9b326242)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The Judge ruling states
explicitly: "No FAKE annotation is required — this is not a sanctioned-exception
construct, it is ordinary spelling." The two claimed reuse families here are the
plain SOTN-sanctioned forms (live reads, no dead code), not the FAKE-annotated
last-resort carve-outs.
