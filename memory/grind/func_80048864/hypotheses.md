# Hypothesis ledger — func_80048864

## s1 (2026-09-10, recon) — floor 44 → 0
- H1 CONFIRMED: the mode dispatch is `switch (mode) { case 0: ... break; case 1: ... break; }` with no default. Mechanism: GCC 2.7.2 expands a 2-case switch as a compare chain (`beq 0`, `li 1; beq`, `j end`) followed by the case bodies in order; an if / else-if chain lays each body inline under its own test. Probe: v1 (if / else-if) = 44; v2 (switch, with H2 + H3) = 0.
- H2 CONFIRMED: the source-pointer increment is written per arm (`*dst++ = *src++; continue;` in the zero arm, `src++` in the non-zero arm), not as `p = *src++` before the test. Mechanism: an increment before the branch lives in the shared block and is the only candidate for the bnez delay slot; the target fills that slot with `andi $t0,$a3,0x1f` from the non-zero thread and each arm carries its own `addiu $t4,$t4,2`.
- H3 CONFIRMED: case 1 reuses r/g/b (`r = r*0x547; g = g<<11; b = b*0x2B8; r = (r+g+b)>>15; r = (r*mr)>>12; g = (r*mg)>>12; b = (r*mb)>>12;`). Mechanism: separate assignments keep the three weighted terms in distinct registers before the two adds, and the target multiplies the scaled r (not the luma) for g and b, which a separate `lum` local cannot produce.
- Frontier: empty — sandbox distance 0, candidate-ready.
