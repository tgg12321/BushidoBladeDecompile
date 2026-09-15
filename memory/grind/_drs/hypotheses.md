# Hypothesis ledger — _drs

## s1 (recon, 2026-09-15)

H1 — "`_drs` is the psyz `sys.c` StoreImage body; the `_dws` spelling (already matched at
`src/display.c:745`) lands `_drs` unchanged apart from the three predicted structural
deltas." **CONFIRMED**: sandbox 161/161 instruction-identical; full build SHA1 == oracle;
sandbox 0 after the reference refresh.

H2 — "The first-measurement 28 is a real codegen residual." **KILLED (instance)**: all 28
diffs are `lui 0x0/lw 0(rX)` reference-side unresolved dlabel pairs
([[score-symtab-blind-to-asm-data-dlabels]]); zero mnemonic/register/immediate diffs;
SHA1 match proves the bytes. Measured on HEAD chassis f9d438a8f (-mel -msoft-float), no
FAKE constructs present.

Frontier: empty — function is at 0 with the oracle proven. Nothing left to grind.
