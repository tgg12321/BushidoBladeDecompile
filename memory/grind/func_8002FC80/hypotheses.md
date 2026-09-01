# Hypothesis ledger — func_8002FC80

## s2 (2026-08-31, recon — grant integration)

- H1: "With both C store spellings foreclosed (VECTOR form banned by layer-1, plain s32-cast form measured sinking), the whole-body canonical block per the executed 2026-08-31 grant reproduces sandbox 0." — CONFIRMED: verbatim transcription of asm/funcs/func_8002FC80.s as a file-scope glabel block measures **0 (74/74, 0 rules dropped)** this session. No frontier remains; the function is candidate-ready on the grant path to COMPLETED-INLINE-ASM-CANONICAL (allowlist line already at inline_asm_canonical.txt:365).

## s1 (2026-08-31, recon)

- H1: "The banked candidate body, spliced over HEAD's INCLUDE_ASM, reproduces sandbox 0 under the executed canonical-asm grant." — CONFIRMED after one correction: the splice alone measured 42 (build 39/74) because HEAD's src/code6cac_b.c lacks `#include "gte.h"` and GCC 2.7.2 silently discards the six VECTOR-store statements on parse-error recovery (see evidence.md). Restoring the include (present in the candidate full-file snapshot) measures **0 (74/74, 0 rules dropped)**. No frontier remains — the function is candidate-ready on the grant path to COMPLETED-INLINE-ASM-CANONICAL.
