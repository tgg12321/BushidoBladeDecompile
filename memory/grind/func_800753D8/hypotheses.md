# Hypothesis ledger — func_800753D8

## s1 (2026-09-15, recon) — chassis: INCLUDE_ASM (floor 166) -> pure C, sandbox 0

H1 CONFIRMED — Structure: a 0x2C-byte descriptor local filled and passed to func_8007352C five times (player frame, two fixed elements from the 0x14 table, a 2-iteration loop over the 0x2C table), with SetDrawMode/AddPrim after each group; exactly the func_8007352C-descriptor sibling shape. Measured: first body (all fields, `p`/`x` locals) 166 -> 70; instruction count equal after H2.
H2 CONFIRMED (FAKE-family) — `s32 zero = 0;` constant-holder passed to both func_8006E480 calls reproduces the $s5 zero-holder. Measured 70 -> 61 on the same body; the inline literal 0 form is banked as rejected/literal-zero-arg-no-holder-score70.c. Mechanism: global.c seats the once-set, call-crossing constant pseudo in a callee-save; cse.c does not fold the constant across the entry-block diamond. Annotated per named-local-fake-exception.
H3 KILLED (instance) — caching `x = arg1 * 240` in a local (`x = arg1 * 240` scored 61; shift-chain `x = arg1 << 4; x -= arg1; x <<= 4;` scored 49) did not seat x in $s0 on the s1 pure-C chassis with the H2 holder present: global.c priority puts arg0 first (7222 vs 1985/2000). Both forms banked in rejected/.
H4 CONFIRMED — recomputing `arg1 * 240` at each of the three use sites (no local) seats x in $s0 and every callee-save correctly (61/49 -> 19). Mechanism: loop.c hoist of the loop instance + cse2 fold into the entry chain + local-alloc tie into callee-save $s0 (evidence.md s1). Ordinary C.
H5 CONFIRMED — a fresh per-site descriptor pointer (`s.sp18 = tbl[...]` assigned directly) plus a real `body = s.sp18 + 0xC` variable stored after the loop's sp2C diamond reproduces the $v0/$v1/$a2 seating and the store order (19 -> 11 -> 1 across variants v8/v10/v17).
H6 KILLED (instance) — moving the s.sp34 statement before the first descriptor fill (variants v11, v13) regressed 11 -> 25 on the s1 chassis with the H2 holder present. Banked rejected/sp34-before-first-descriptor-fill-score25.c.
H7 CONFIRMED — `u8 *base = D_800A36A0;` local (the neighbour func_80075670's spelling) makes `base + arg1` base-first (`addu $v0,$a1,$s3`); part of the 19 -> 11 step.
H8 CONFIRMED — `(arg1 << 2)` for the s16 byte offset keeps base first in the `lh` address (expr.c both_summands puts MULT terms first); `arg1 * 4` scores 1 (rejected/lh-address-arg1-times-4-mult-first-score1.c); `((s16 *)(base + 0x42))[arg1]` is a stride bug, never a candidate (rejected/lh-s16-index-wrong-stride-semantic-bug.c). Final: 0.

Frontier: none — matched. The remaining path is review/integration (layer-1 cheat-reviewer, Judge, verify-oracle, queue done by the driver).
