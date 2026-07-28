# Hypothesis ledger — func_80038170

## s1 (2026-07-28, recon)
- H1 CONFIRMED: phantom +8 frame closable in pure C via same-base pair inside the conditional (phantom-frame-slots-gcc272). Probe: cc1 .frame bisect matrix. Result: vars 8->16 == target; floor 14->5; reviewer PASS.
- H1a KILLED: named u16 (HImode) temp in halfword copy loop triggers the slot — vars stays 8.
- H1b KILLED: named-pointer staging of the pair (u8 *t = &D_8008F19C + s3*2) — kills the temp, vars 8.
- H2 CONFIRMED-then-review-FAILED: chained `s1=s2=s3=0;` flips init emission to s3,s2,s1 -> floor 1 (text-artifact-only). Cheat-reviewer FAIL (order-flip family). Banked rejected/chained-zeroing-order.c. -> ruling-request filed s1.
- H3 CONFIRMED: floor-1 residual is symbol-vs-addend text artifact (D_8008F19C+1 vs D_8008F19D); linked bytes identical (word-diff proof). Sandbox 0 unreachable pre-integration by construction.
