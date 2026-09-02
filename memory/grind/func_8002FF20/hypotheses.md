# Hypothesis ledger — func_8002FF20

## H1 — CONFIRMED (s1, 2026-09-01)
**Statement:** The whole queue distance (34, pre-migration chassis) was cheat-stripping of the cop2
tail, not codegen divergence; the pure-C head is already byte-exact, and respelling the three
islands in the func_8002FDB0 / func_800203B4 authorized single-block form (no pins, no move
aliasing, nops inside the gte_ldv0 block) closes the function.
**Mechanism:** engine/inlineasm.py strips pins + `move %0,%1` blocks + bare `nop` blocks before
scoring, collapsing the whole tail; the authorized island form contains no strippable construct.
**Probe:** apply body (tmp/grind/func_8002FF20/s1/body.c); canonical; sandbox --disable all; full build.
**Result:** sandbox 0 (99/99, rules_dropped 0); build SHA1 == oracle MATCH.

## H2 — CONFIRMED-by-construction (s1)
**Statement:** Islands 2 and 3 must take the SAME named local (`vec`) as operand so cse.c emits
`addiu $v0,$s0,0x2C` once and island 3 reuses $v0 (.s L84 `addu $t4,$v0,$zero`).
**Result:** first build with the shared-local form matched at 101 insns. The alternative
(two distinct operand expressions) was NOT measured; predicted +1 insn. Not worth a session.

## H3 — KILLED (s1)
**Statement:** The canonical-asm rule-3 STRONG-scanner path is available.
**Result:** scan_hand_coded tier LOW, score 1/8 (S4 only). Same as func_800203B4. Irrelevant now:
membership is via the 2026-09-01 widened-anchor owner GRANT, which names this function.

## Frontier
Empty for codegen. Remaining step is the grant path under the normal grind gates
(evidence.md fact 8). If the Judge FAILs the island body on a construct, the only respelling
axis is the island partition (thin-island variants) — func_800203B4 s6 measured those 12/4/8
non-zero for the identical macro set; expect the same here.

## H4 — CONFIRMED (s1b, 2026-09-02)
**Statement:** The banked s1 body is chassis-stable: restored onto HEAD 63e6bcf6 (asm-until-matched chassis,
INCLUDE_ASM replaced) it still scores sandbox 0 (99/99, rules_dropped 0) and the full build SHA1 == oracle.
**Mechanism:** none needed — the body compiles in the same TU context; the migration touched only the
INCLUDE_ASM line for this function.
**Probe:** apply s1 body; canonical; sandbox --disable all; full build; comment-only Ruling A edits; sandbox again.
**Result:** 0 / 0 / MATCH / 0. Submitted candidate-ready under cluster condition 3 (Ruling A). Frontier stays
empty for codegen; the only remaining step is the driver's grant-door integration.
