# Hypothesis ledger — func_800400F8

## s2 (structural, 2026-07-14)
- H1 "judge's respell compiles byte-identically" — probe: apply + sandbox. Result: 16. **KILLED** (frame 0x20 + RA swap).
- H2 "small ordering/spelling sweep finds the natural form" — probe: 6-variant sweep (orderings, block-local decls, decl order, hoists). Result: best 12, none 0. **KILLED** for the if+literal-compare family.
- H3 "while/for loop spelling reproduces target naturally" — probe: 5 variants. Result: all exactly 2 (frame+RA correct; beqz slot steals the s1 copy). **KILLED as exact match, CONFIRMED as mechanism evidence** (variable-compare guard is load-bearing).
- H4 "frame 0x28 comes from the variable-compare's combine leftover" — probe: cc1 -da dumps v6 vs v11 vs literal forms. Result: `(use (reg 79))` -> reload stack slot sp+20 in exactly the variable-compare forms. **CONFIRMED**.
- H5 "beqz slot nop requires guard lh first in fall-through (may_trap_p barrier)" — probe: dumps + steal behavior across variants. **CONFIRMED**.
- H6 "constant-holder `zero` reproduces v6 with sanctioned spelling" — probe: 2 variants. Result: 13/18, 30 insns. **KILLED**.
