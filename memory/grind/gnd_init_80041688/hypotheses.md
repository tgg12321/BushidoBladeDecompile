# Hypothesis ledger — gnd_init_80041688

## [s1] Reordering the C source statements so `b = ...` appears before `r`/`g` in the else branch will make sched1 emit b's lbu first, matching target's [b,r,g] order.
- mechanism: LUID/source-order tiebreak in sched1 when picking among ready insns.
- probe: Edited src/text1a.c to `b=...; r=...; g=...;` then ran `sandbox gnd_init_80041688 --disable all`.
- result: Score stayed at 2. Emitted asm identical to the r,g,b source-order build; the three lbu's still emit in [r,g,b] order. Statement/LUID order does NOT override sched1 INSN_PRIORITY when priorities differ.
- verdict: KILLED
