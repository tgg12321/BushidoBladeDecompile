# Hypothesis ledger — func_8005D814

- [manual 2026-09-26] CONFIRMED: func_8005E098's template (S5E098 descriptor + digit idioms) transplants; switch(j) with three written-out cases reproduces phase 3's three copies (cross-jump merges only the `sh glyph; sw cur` tail).
- [manual 2026-09-26] CONFIRMED: phase-4 d1 is `num_tens % 10` then a separate `digit[1] % 100` (both in target bytes: 0x66666667 then 0x51EB851F on the stored tens digit).
- [manual 2026-09-26] KILLED: pointer local `p` for the cell (phase 2/4) -- pointer lands in $a1 not $a0/$v1 (51 -> 31 when read back through s.table).
- [manual 2026-09-26] KILLED: tile store order x0,y0,h,w -- reduced giv base becomes fp+0xC (target fp+0xE); w,h order fixes it.
