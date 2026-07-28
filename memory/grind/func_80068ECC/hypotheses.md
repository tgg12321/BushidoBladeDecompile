# Hypothesis ledger — func_80068ECC

## s1 (recon)
- H1: mask-then-shift spelling gives per-expression srl+andi without CSE — KILLED
  (combine preserves literal order; emits andi;sra / andi;srl, never srl;andi).
- H2: in-place `v &= ~K; v |= b;` statements (not compound rebind) are required for the
  $v0/$v1 two-address group shape — CONFIRMED (compound form scored 31 vs 13).
- H3: three distinct-width unsigned casts (u32/u16/u8) on shift-first spellings defeat
  CSE while each folding to `srl;andi` — CONFIRMED, closes to sandbox 0.

Frontier: none — matched at 0; awaiting driver byte-verify + Judge.
