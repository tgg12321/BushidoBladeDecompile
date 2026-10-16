---
name: cop2-addressing-preamble-cluster
paths: ["inline_asm_canonical.txt"]
description: "Owner CLUSTER ruling (func_8002FDB0, 2026-08-17; widened 2026-09-01): 0x8001-0x8003 functions with the materialize-then-copy + cop2 hand-asm preamble inherit canonical-asm for the island, subject to a 4-point check."
metadata:
  type: project
---

# The materialize-then-copy + cop2 cluster (owner rulings 2026-08-17, 2026-09-01, 2026-09-02)

Records: docs/grind/decisions.md 2026-08-17 func_8002FDB0 CLUSTER RULING; 2026-09-01 "cop2 materialize-then-copy
WIDENED ANCHOR — OWNER GRANT"; 2026-09-02 OWNER RULING, Ruling A.

## The idiom

A redundant register copy feeding only a cop2 transfer, with unfilled cop2 load-delay slots and splat
`/* handwritten instruction */` tags — GCC 2.7.2 never copies a fresh value into a second register with no
other use:

```
addu  $t4, $a1, $zero    # redundant copy (source may be ANY register since 2026-09-01)
mtc2  $t4, $30           # ... or lw $t5/$t6/$t7 -> ctc2 / lwc2 / swc2
nop                      # unfilled cop2 load-delay slots
nop
```

## Membership — the scan is the source of truth

`addu $tN,$rX,$zero` with a cop2 transfer referencing `$tN` within the next 6 lines, in a function in the
0x8001-0x8003 band (`tmp/scan_cop2_widened.py` shape; re-run it rather than trusting any list). The original
2026-08-17 enumeration (28, `$aN` sources) is a subset of the widened set (68 in-band carriers, 2026-09-01).
Out-of-band carriers are NOT covered. Sub-families: LZCS/LZCR leading-zero-count (`mtc2 $t4,$30` → 2 nops →
`swc2 $31`); SetRotMatrix / long-vector transfer (`lw $t5/$t6/$t7` → `ctc2`, or `lwc2`/`swc2` triples);
`func_80018300` is a hybrid — check by hand.

## The per-function mechanical check (a check, not a lever)

A member's island inherits the canonical-asm disposition ONLY when all hold:

1. `sandbox <func> --disable all` == **0** (scored per inline-asm-policy's scorer ruling of 2026-09-25 once its
   engine fix lands; `0(reg)` == `(reg)` in a memory operand when units are recognized; nothing else
   normalized). The ruling admits no island by itself.
2. **Zero** register pins, `move %0,%1` aliasing blocks or scheduling barriers anywhere in the body.
3. In-island GPR instructions limited to the cop2 addressing preamble. **The template is the named Sony PsyQ
   GTE macro body** the island reproduces (2026-09-02): GPR instructions that are the macro's own published
   text (e.g. `gte_ldlv0`'s `lhu/lhu/sll/or` pack, PsyQ 4.5 `inline_c.h:101-110`) are admitted; nothing outside
   the named macro body. An island comment that names a macro names the one the island implements;
   macro name, header line and departures go in its row or the landing record
   ([[site-comment-content]], Q118).
4. Before `queue done`: fresh layer-2 `cheat-reviewer` on the applied diff AND `verify-oracle --rebuild`.

**The load-bearing negative.** Membership closes the TAIL ISLAND only, once the pure-C body independently
reaches sandbox 0 under the normal gates. It is not a shortcut for a function hundreds of instructions off.

**Bucket.** Every carrier is COMPLETED-INLINE-ASM-CANONICAL (allowlist line in `inline_asm_canonical.txt`
required) — never COMPLETED-C; the driver enforces this on the Judge PASS path. Preferred form: calls to
the vendored PsyQ macros ([[inline-asm-policy]] § PsyQ GTE macro calls); an island that reproduces a macro's
text is converted to that call.

## Related

[[canonical-asm-authorization-recipe]] · [[reload-spill-reg-reveals-asm-clobbers]] · [[inline-asm-policy]] ·
[[judge-sole-gate]]
