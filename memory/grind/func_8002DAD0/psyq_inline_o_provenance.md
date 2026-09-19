# func_8002DAD0 — island provenance: Sony PsyQ `inline_o.h` (DMPSX v3)

Established session 6 (2026-09-18). Durable copy of the evidence, because the
header itself is only present in this tree as a gitignored scratch clone at
`tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_o.h`
(rood-reverse PSX header set). Header banner, verbatim:

```
/*
 * $PSLibId: Run-time Library Release 4.5$
 */

/*
 *	Macro definitions of DMPSX version 3
 *	inline_o.h
 *	Copyright(C) 1996, Sony Computer Entertainment Inc.
 *	All rights reserved.
 */
```

## Why this matters

The cop2-addressing-preamble cluster (`.claude/rules/cop2-addressing-preamble-cluster.md`,
28 members incl. func_8002DAD0 at :76) describes its members' shared
`addu $t4, $aN, $zero` + hard-`$12..$15` cop2 idiom as a *handwritten*
signature of unknown origin. It is not handwritten: it is the **verbatim
expansion of Sony's DMPSX `inline_o.h` macro set**, which spells every GTE
primitive as a *sequence of single-instruction `__asm__ volatile` blocks* with
the address staged through a hard `$12` (`move $12,%0`) and the payload
registers hard-coded `$13/$14/$15`. PsyQ's *other* GTE header, `inline_c.h`,
spells the same primitives with `%0`-relative offsets and NO `move` preamble —
which is why every attempt to attribute these islands to `inline_c.h` left the
`move`/`addu` preamble and the delay `nop`s unaccounted for (the two layer-1
citation FAILs of 2026-09-18).

## Island-by-island mapping (all line numbers in `inline_o.h`)

| func_8002DAD0 island | Sony macro | inline_o.h | body match |
|---|---|---|---|
| 1 — OP diagonal load @ obj+0xA8 | `gte_ldopv1(r1)` | 192-200 | exact: `move $12,%0` · `lw $13,($12)` · `lw $14,4($12)` · `ctc2 $13,$0` · `lw $15,8($12)` · `ctc2 $14,$2` · `ctc2 $15,$4` |
| 2a — IR vector load @ obj+0xB8 | `gte_ldopv2(r1)` | 201-206 | exact: `move $12,%0` · `lwc2 $11,8($12)` · `lwc2 $9,($12)` · `lwc2 $10,4($12)` |
| 2b — OP invocation | `gte_op0()` | 711-715 (shape) | `nop` · `nop` · `.word <op>`; inline_o.h/inline_c.h carry DMPSX placeholder words (`0x0000127f`), our `.word 0x4B70000C` is the real cop2 encoding this project's assembler needs (same convention as `include/gte.h`'s `gte_mvmva`) |
| 3, 7, 9 — MAC1..3 store | `gte_stlvnl(r1)` | 904-909 | exact: `move $12,%0` · `swc2 $25,($12)` · `swc2 $26,4($12)` · `swc2 $27,8($12)` |
| 4 — leading-zero count | `gte_Lzc(r1,r2)` = `gte_ldlzc` + 2× `gte_nop` + `gte_stlzc` | 207-211, 1095, 1074-1077 (`gte_Lzc` combination in `gtemac.h`:174-178) | exact: `move $12,%0` · `mtc2 $12,$30` · `nop` · `nop` · `move $12,%0` (from `&sp_tmp`: `addiu $v0,$sp,0x10` + `addu $t4,$v0,$zero`) · `swc2 $31,($12)` |
| 5 — rotation matrix load | `gte_SetRotMatrix(r1)` | 272-284 | exact, all 11 instructions incl. the `$13`/`$14` re-use order |
| 6, 8 — V0 pack + load | `gte_ldlv0(r1)` | 95-103 | exact: `move $12,%0` · `lhu $14,4($12)` · `lhu $13,($12)` · `sll $14,$14,16` · `or $13,$13,$14` · `mtc2 $13,$0` · `lwc2 $1,8($12)` |
| 6b, 8b — MVMVA invocation | `gte_rtv0()`-class | 426-430 (shape) | `nop` · `nop` · `.word 0x4A486012` = `gte_mvmva(1,0,0,3,0)` under `include/gte.h`'s existing encoder macro |

Every non-cop2 instruction in every island (the `move`/`addu` preambles, the
`lw`/`lhu`/`sll`/`or` packing, the delay `nop`s) is accounted for by a named
Sony macro body. Nothing in these islands is invented, and nothing is
program logic smuggled into asm: the whole of func_8002DAD0's own logic is in
the C around them.

## Consequence for the whole cluster

Any cluster member can now have each island cited to a named macro at a header
line, which is exactly what `.claude/rules/cop2-addressing-preamble-cluster.md`
condition 3 asks for. It also explains the cluster's LOW `scan_hand_coded`
tier (1/8 for func_8002DAD0, S4 only): the scanner looks for *hand-written*
signatures (multu pacing, empty branches, no spills) and these functions are
ordinary compiled C whose GTE macros happen to expand to register-pinned asm.
The tier will never be STRONG, so the STRONG-tier canonical-asm door cannot
open for this family by construction — the owner-cluster registry door
(`tools/grinder/owner_cluster_grants.txt`, operator-only) is the one that fits.
