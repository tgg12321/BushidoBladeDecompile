# Hypothesis ledger — func_800871D4

## [s1] The m2c-reconstructed shape (explicit `var_v1 = D_8010280A & 0xFFFF` twice, no in-place -=0x10) matches oracle semantics but our cc1 folds both masks and produces a worse layout.
- mechanism: cc1 nonzero_bits tracking proves the u16 lhu result is 16-bit-clean, so `& 0xFFFF` reduces to identity in combine.c; the shape reshuffles RA vs the in-place-modify shape.
- probe: Applied m2c-shape (var_v1 = D_8010280A & 0xFFFF; ...; var_v1 = D_8010280A & 0xFFFF in else) — sandbox --disable all
- result: score=12 (WORSE than direct-form floor=10)
- verdict: KILLED

## [s1] The 5 other BB2 functions containing the exact `lhu $r,GLOBAL; nop; andi $r2,$r,0xFFFF` pattern are ALL still INCOMPLETE (2 parked ASM-STRUCTURAL, 2 ASM-SUSPECT, 1 active). ZERO COMPLETED-C sibling in the tree produces this shape.
- mechanism: Cross-tree census (tmp/grind/func_800871D4/s1/find_sibling2.py) — the shape is a shared-failure family, not a per-function accident.
- probe: Regex scan of asm/funcs/*.s + queue.json status join.
- result: 6 matching funcs; 5 non-target (func_8002304C active-C, func_8003DE14 ASM-SUSPECT, saTan4FireDisp_80048864 ASM-SUSPECT, func_8008C464 parked ASM-STRUCTURAL, func_80023F08 parked ASM-STRUCTURAL). None matched.
- verdict: CONFIRMED
