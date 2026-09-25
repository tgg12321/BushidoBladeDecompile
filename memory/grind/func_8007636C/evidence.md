# func_8007636C evidence

## 2026-09-24 independent reconstruction

- Canonical gate: `C`, hand-coded tier `LOW`; pure C is required.
- Target: 348 instructions and a 0x70-byte frame.
- The archived pre-INCLUDE_ASM body is explicitly only a placeholder.
- Fresh m2c with minimal type repairs compiled to 275 instructions at honest
  score 248. It incorrectly modeled the descriptor at stack offsets 0x18..0x43
  as unrelated scalar locals, allowing most stores to disappear.
- Replacing those scalars with the evidenced 0x2c descriptor aggregate used by
  nearby matched rendering functions restored the live writes and produced
  355 instructions at honest score 184. The candidate has a 0x78 frame because
  m2c's SSA-style locals retain an extra spill; the target has 0x70.
- No candidate was promoted; main retains `INCLUDE_ASM`.

## Measured attempts

1. Fresh literal m2c plus actual one-argument callee arity: score 248/348,
   275 instructions.
2. Restore the real stack descriptor aggregate and pass its address to
   `func_8007352C`: score 184/348, 355 instructions.

No FAKE construct, inline assembly, volatile coercion, register pin, fabricated
padding, or toolchain change was attempted.
