# Hypothesis ledger — func_800611A4

## [s1] Pin-free candidate scores floor=9 (43 insns == 43 target insns, pure register-name swap v0<->v1).
- mechanism: sandbox --disable all strips cheat-asm; residual is RA-only, no add/del/reorder.
- probe: Applied memory/grind/func_800611A4/candidate.c to src/text1b.c and ran `sandbox func_800611A4 --disable all`.
- result: score=9, target_insns=43, build_insns=43, rules_dropped=0, cheat_asm_stripped=393.
- verdict: CONFIRMED

## [s1] The RA pivot is the mask's 2-instruction lui+ori construction (0xFFFFEF/0xFF0060/0x10FFFF); when the mask fits in 1 insn (0xFF0000 in sibling func_800618B4), target flips RA and puts mask->$v0, 3rd load-temp->$v1.
- mechanism: GCC allocno priority orders lower-numbered pseudos to lower hard regs; the 2-insn mask has different ref/liveness weight than a 1-insn mask, so the mask-vs-load-temp tiebreak inverts between the two mask shapes.
- probe: Compared tail asm of func_800611A4 / 61250 / 61658 (2-insn masks) vs func_800618B4 (1-insn mask). All four share the identical `for i in 0..2: t=arg0[i]; STORE_i=t;` + `MASK_G=mask;` post-call cluster.
- result: Three 2-insn-mask siblings: mask lives in $v1, load-temp in $v0. One 1-insn-mask sibling: mask in $v0, load-temp in $v1 (on 3rd load). Mask-width is the RA pivot.
- verdict: CONFIRMED

## [s1] `restore-discarded-return-displaces-v0` does NOT apply here — func_80060A68 is declared `void` in-file AND its body has no return AND the sole caller (src/code6cac_b.c:3166) does not capture a return.
- mechanism: Rule requires an extern caller that writes `var = func(...)`; grep found none.
- probe: grep -rn 'func_800611A4\|func_80060A68' src/ include/ + read func_80060A68 body at text1b.c:13314.
- result: Both functions void-void; no extern return-capture caller. Lever ruled out for THIS function; may still apply to callers of the siblings — untested.
- verdict: KILLED
