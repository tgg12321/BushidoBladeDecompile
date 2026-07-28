# Hypothesis ledger — func_80068ECC

## s1 (recon)
- H1: mask-then-shift spelling gives per-expression srl+andi without CSE — KILLED
  (combine preserves literal order; emits andi;sra / andi;srl, never srl;andi).
- H2: in-place `v &= ~K; v |= b;` statements (not compound rebind) are required for the
  $v0/$v1 two-address group shape — CONFIRMED (compound form scored 31 vs 13).
- H3: three distinct-width unsigned casts (u32/u16/u8) on shift-first spellings defeat
  CSE while each folding to `srl;andi` — CONFIRMED, closes to sandbox 0.

Frontier: none — matched at 0; awaiting driver byte-verify + Judge.

## s2 (recon, after Judge FAIL of the width-cast form)
- H4: `arg0 = arg0;` FAKE self-assign forces cse.c hash invalidation -> fresh srl —
  KILLED (measured 13/39 unchanged; same-pseudo copy elided at expand, CSE never
  sees a SET).
- H5: bit-extract respelling `(((u32)arg0 >> N) & 1) << M` (N=4,5,6; M=3,4,5) gives
  three structurally distinct expressions (no CSE merge possible) that combine folds
  to `srl 1; andi mask` — CONFIRMED, sandbox 0 (41/41), pure C, layer-1 PASS.

Frontier: none — re-matched at 0 with the structural respelling the Judge's
constraint invited; awaiting driver byte-verify + Judge.
