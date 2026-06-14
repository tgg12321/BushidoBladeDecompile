# func_80022F34 — WIP (callee-saved over-promotion / frame plateau)

## TL;DR (2026-06-14)
HEAD carries 11 regfix rules: 10 frame-offset substs (prologue idx 0-7 +
epilogue idx 59-63, shifting the frame down) + 1 `insert "nop" @ 36`. Unpinned
masked floor = **11** and NO pure-C form found this session beats it. Two
distinct issues:

1. **maspsx .L-label load-delay nop** (the `insert nop @ 36`): store-value
   consumer variant `lhu v0,0(s2)` -> `.L` switch-merge label -> `sh v0,8(a0)`.
   This is a LEGITIMATE pure-C retirement path via
   [[maspsx-label-nop-gate]] (add func to `maspsx_label_nop_funcs.txt`, retire
   drops the rule). NOT the blocker.

2. **Callee-saved over-promotion (the real blocker):** compiling pure C, GCC
   2.7.2 allocates SEVEN callee-saved regs (s0,s1,s2,s3,s4,s5,ra) and a -48
   frame; the TARGET uses only FOUR (s0,s1,s2,ra) and a -32 frame. GCC promotes
   3 extra long-lived values (the &D_801027BC base, val1, an index temp) to
   s3/s4/s5 where target keeps them caller-saved. The 10 frame-offset substs +
   the register renames are correcting this. No pure-C restructuring tried this
   session changes the promotion.

## Ruled out this session (all >= floor 11, none a cheat)
- inline the call args (drop named idx1/val1 intermediates) — floor 11
- switch directly on D_800A38DC (drop `val` temp) — floor 11
- do/while loop instead of goto — floor 26 (worse)

## Why blocked, not matched
COMPLETED-C needs 0 rules + 0 pins + SHA1 == oracle. Frame-padding to coerce
the size is FORBIDDEN ([[dead-vars-local-array]]) and anyway the divergence is
GCC over-promoting to callee-saved (frame too BIG, not too small) — no
"subtract a local" lever exists. The maspsx nop is retirable but the
callee-saved over-promotion is a register-allocation plateau.

## Next hypotheses
- Directed permuter from baseline targeting the s3/s4/s5 demotion (register
  allocation is exactly the permuter's domain; caveat: high-cascade-adjacent
  file code6cac.c).
- ALLOCDBG / -da RTL dump to identify WHICH values GCC promotes to s3/s4/s5 and
  whether a lifetime-shortening restructure can keep them caller-saved.
- If the promotion is immovable: canonical-asm authorization (the rules encode
  a register-allocation the compiler won't produce).
