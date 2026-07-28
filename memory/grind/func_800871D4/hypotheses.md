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

## [s2] Declaring var_v1 as u16 (narrow type) will steer GCC toward the oracle's $a0/$v1 unmasked/masked split.
- mechanism: u16 lvalue triggers extra codegen at store points; may bias RA to keep an unmasked mirror.
- probe: Applied `u16 var_v1;` in src/main.c; sandbox --disable all.
- result: score=11, build_insns=52 (== target 52 but 11 mismatched positions); WORSE than u32 floor=10.
- verdict: KILLED

## [s2] Gate 1 (canonical-asm STRONG signals via scan_hand_coded) passes for func_800871D4.
- mechanism: S1 multu pacing / S2 empty branches / S6 BIOS jumptable are the strong tier per hand-coded-asm-recognition.
- probe: python3 tools/scan_hand_coded.py --single func_800871D4 (tmp/grind/func_800871D4/s2/scan_hand_coded.txt).
- result: tier=LOW, score 0/8, zero strong signals; the function is a trivial bit-flag setter with linear control flow, zero call sites, no GTE/BIOS/HW ops.
- verdict: KILLED

## [s2] Gate 2 (in-hand SOTN or in-tree COMPLETED-C precedent for the lhu+andi-0xFFFF-from-u16-global coercion shape) passes.
- mechanism: Standing 2026-07-27 auto-ruling requires a CITED SOTN file+line or a COMPLETED-C sibling precedent for the coercion family.
- probe: s1 cross-tree BB2 sibling census (tmp/grind/func_800871D4/s1/find_sibling2.py); no SOTN checkout in-repo.
- result: 5 BB2 siblings share the exact shape, ALL INCOMPLETE (2 parked ASM-STRUCTURAL, 2 ASM-SUSPECT, 1 active-C); zero COMPLETED-C sibling; no in-hand SOTN cite. Per the standing ruling, negative census = FAILED gate, not open question.
- verdict: KILLED
