# SELF-VET — func_8002FC80

CONSTRUCTS: (1) `#include "gte.h"` added to src/code6cac_b.c header block (typedef/macro-only header, needed for the VECTOR type); (2) six ordinary-C vector-difference stores through `((VECTOR *)0x1F8003x0)->vx/vy/vz` scratchpad pointers with named temps `v1`/`v2`; (3) four canonical GTE inline-asm islands (PsyQ libgte macro bodies: gte_SetRotMatrix, gte_ldlvl, the GTE OP `.word 0x4B70000C`, gte_stlvnl); (4) pointer local `p = (s32 *)0x1F800380` feeding the store island and the `ratan2` argument; (5) `ret` accumulator with conditional `+= 0x800`.

## T1 semantic purpose
(1) The include supplies the VECTOR type the body uses — without it GCC 2.7.2 parse-errors and silently discards the six stores (measured this session: score 42, build 39/74). (2) The stores compute and place the two difference vectors the GTE op consumes — fully semantic. (3) The islands ARE the function's computation: cop2 control-register loads, IR vector load, the GTE outer-product command, MAC1-3 store. cop2 ops have no C analog; the GPR `move $12, %0` / `lw $13-$15` lines are the SDK macro's own hardcoded addressing preamble, matching the target's hand-asm bytes. (4)(5) Ordinary consumed values. Nothing in the diff is byte-identical-with-or-without. PASS.

## T2 human-programmer
A PS1 programmer using the PsyQ SDK writes exactly this: subtract vectors into scratchpad, invoke the libgte macros, ratan2 the result. The islands are verbatim SDK macro bodies (gte.h documents the same forms). PASS.

## T3 GCC-internals justification
No construct is justified by any GCC pass. The islands are justified by hardware (cop2) and by the target's hand-written-asm evidence (redundant addressing copy + unfilled GTE delay nops + splat "handwritten instruction" tags — the cluster rule's evidence set). PASS.

## T4 permuter/search provenance
No construct came from permuter/auto-search. The body is the banked candidate from prior judged sessions; this session only spliced it and restored its missing include. PASS.

## T5 family check
The only non-plain-C constructs are the canonical GTE islands — the sanctioned canonical-inline-asm category (inline-asm-policy "canonical" row), NOT a coercion family. No register pins, no `move %0,%1` C-aliasing blocks, no barriers, no volatile coercion, no dead stores/locals. In-island GPR instructions are limited to the cop2 addressing preamble (cluster rule condition 3). PASS.

## T6 naming-announces-intent
Names: `v1`, `v2`, `p`, `ret` — all live, consumed values; no pad/dummy/spill/unused names. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical inline asm (COMPLETED-INLINE-ASM-CANONICAL, owner-cluster grant)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:96`
  (func_8002FC80 is enumerated there as a SetRotMatrix/long-vector-transfer sub-family member; the executed pipeline grant is `inline_asm_canonical.txt:365`, written by the driver after the 2026-08-31 Judge ESCALATE canonical-asm-grant — packet at docs/grind/decisions.md:16810.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. (Canonical islands carry provenance comments naming the PsyQ macros, not FAKE annotations; no rule mandates FAKE for the canonical category.)

Mechanical cluster check (cop2-addressing-preamble-cluster.md, all four conditions):
1. `sandbox func_8002FC80 --disable all` == 0 this session (74/74, 0 rules dropped).
2. Zero pins / aliasing blocks / barriers in the body.
3. In-island GPR instructions are only the cop2 addressing preamble (move + lw feeding ctc2/lwc2/swc2).
4. Layer-2 cheat-reviewer + `verify-oracle --rebuild` before `queue done` — driver/operator step (verify-oracle correctly refused this session on dirty build inputs; the driver re-verifies at integration).
