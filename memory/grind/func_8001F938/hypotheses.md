# Hypothesis ledger — func_8001F938

## Live frontier (after s1 recon)

### H1 — kind-split lever (CONFIRMED, floor 11->6 alone)
`u32 kind_full = *(u16*)(a0+0x6A); u32 kind = kind_full & 0xFFFF;` provokes
target's redundant `lhu $a1,0x6A; andi $v1,$a1,0xFFFF`, fully matching the KIND
comparison region ($5/$3 split + the andi). Mechanism: two pseudos (raw + masked)
map to $a1/$v1 like target. Measured alone: floor 6. Combined with H3: floor 5.
BORDERLINE: redundant-mask / split-init family; needs a fresh cheat-reviewer PASS
(2026-06-15 reviewer called it "codegen-steering but secondary, dual-use with
sanctioned split-init-accumulation"). NEXT PROBE: run cheat-reviewer on the
kind-split in isolation; if PASS, it banks as clean.

### H2 — the +0x270 dual-load CSE wall (CONFIRMED as the residual-5 root cause)
Target emits TWO unconditional same-address loads `lh $v0,0x270; lhu $v1,0x270`.
GCC 2.7.2 CSE ALWAYS merges two same-address HImode reads into ONE load and
materializes the other signedness in-register (sign->zero = `andi`; zero->sign =
`sll;sra`). Measured this session: s16-first -> lh+move+andi; u16-first -> lhu+
sll;sra (floor 10, worse). No pure-C spelling produced two loads. The 5 residual
insns after H1+H3 ALL trace to this (nop+move for the missing 2nd load, andi-vs-
sll delay slot, coupled $v0-vs-$v1 register). NEXT PROBE: is there a semantically-
purposeful C construct that keeps two HImode loads (e.g. an intervening
memory-invalidating operation with real purpose, or a genuine union/overlapping-
field access in the entity struct)? If none exists, H2 is the reviewer-tension /
ruling question: the target provably contains the construct the reviewer's Test-1
("no semantic purpose") rejects. The only measured floor-2 form (unconditional
split-into-two-vars preload) was reviewer-FAILED and did NOT even produce two
loads — it matched more register/branch shape via the preload steering.

### H3 — branch-sense flip (CONFIRMED clean, ~1 distance)
Write the ternary as `if (probe >= 4) raw_or_3 = 3; else raw_or_3 = (u16)*p;`
(plain if/else). Flips GCC `slti;beqz` -> target `slti;bnez`. UNAMBIGUOUSLY clean.
Applied in candidate.c. Fully consumed (no further gain expected alone).

## Killed / disproven this session
- u16-first read order (rejected/u16-first-read-order.c) — floor 10, worse; single lhu.
- (inherited) unconditional dual-typed split (rejected/dual-type-probe-load.c) —
  floor 2 but reviewer-FAILED; codegen-steering, no semantic purpose.

## [s1] kind-split `u32 kind_full=lhu(0x6A); u32 kind=kind_full&0xFFFF` matches target's redundant `lhu $a1; andi $v1,$a1,0xFFFF` KIND region.
- mechanism: Two pseudos (raw + masked) allocate to $a1/$v1 like target; provokes the redundant andi target emits after the already-zero-extended lhu.
- probe: Apply kind-split alone, sandbox --disable all.
- result: floor 11 -> 6 (build_insns 108). KIND region fully matched.
- verdict: CONFIRMED

## [s1] branch-sense flip `if(probe>=4)raw_or_3=3; else raw_or_3=(u16)*p;` flips GCC slti;beqz to target's slti;bnez.
- mechanism: Ordering the >=4 arm first makes GCC branch-on-taken for the <4 case, matching target's bnez.
- probe: kind-split + branch-flip, sandbox.
- result: floor 6 -> 5. Unambiguously clean C.
- verdict: CONFIRMED

## [s1] GCC 2.7.2 emits target's TWO unconditional same-address loads (lh $v0,0x270 + lhu $v1,0x270) from some pure-C read-order/typing.
- mechanism: CSE cost asymmetry (zero->sign costs 2 insns) might trigger a reload for the 2nd signedness.
- probe: u16-first unconditional read then s16 read; disassemble.
- result: floor 10 (worse). Single lhu + sll;sra sign-extend; NEVER two loads. Both read orders collapse to one load. The 2nd lhu is unreachable in pure C.
- verdict: KILLED
