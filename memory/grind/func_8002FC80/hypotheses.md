# Hypothesis ledger — func_8002FC80

## s3 (2026-08-31, recon)

- H1: "The store-sink is a sched.c true_dependence struct/fixed exemption artifact, and the store's MEM_IN_STRUCT_P flag is the only C-visible lever." — **CONFIRMED** (source reading sched.c:817/614 + expr.c:4567; .sched dump shows sched1 doing the sink; memrefs_conflict_p returns conflict for all const-vs-reg pairs, so no other alias route exists). See evidence.md s3.
- H2: "A pointer-plus-constant store spelling (`*((s32 *)0x1F800300 + 0x18) = ...`) sets MEM_IN_STRUCT_P via expr.c's PLUS_EXPR rule and kills the sink without aggregate types." — **KILLED**: measured 34 @ 73/74, identical to plain; the front end folds constant pointer arithmetic before expand, no PLUS_EXPR survives.
- H3: "The layer-1-suggested load-side respelling can clear the exemption's load-side condition." — **KILLED** (analytical, compiler-source level): any read at pointer+4/+8 is an INDIRECT_REF over PLUS_EXPR, unconditionally MEM_IN_STRUCT_P=1 per expr.c:4567; only offset-0 reads escape, which cannot cover blocks 2-6 without extra pointer-advance insns.
- H4: "The VECTOR component-store form qualifies under proven-spelling-class-reconstruction (all 4 prongs), making the 19:45 ban a right-construct/wrong-citation case." — **CONFIRMED as a mapping** (evidence.md s3 has the prong-by-prong case + the 1:1 InitHiraRmd_80041AC8 precedent); execution requires a Judge unban — filed as this session's ruling-request. Measured floor of that form: 0 @ 74/74 (2026-08-31 chassis).

## s2 (2026-08-31, recon — grant integration)

- H1: "With both C store spellings foreclosed (VECTOR form banned by layer-1, plain s32-cast form measured sinking), the whole-body canonical block per the executed 2026-08-31 grant reproduces sandbox 0." — CONFIRMED: verbatim transcription of asm/funcs/func_8002FC80.s as a file-scope glabel block measures **0 (74/74, 0 rules dropped)** this session. No frontier remains; the function is candidate-ready on the grant path to COMPLETED-INLINE-ASM-CANONICAL (allowlist line already at inline_asm_canonical.txt:365).

## s1 (2026-08-31, recon)

- H1: "The banked candidate body, spliced over HEAD's INCLUDE_ASM, reproduces sandbox 0 under the executed canonical-asm grant." — CONFIRMED after one correction: the splice alone measured 42 (build 39/74) because HEAD's src/code6cac_b.c lacks `#include "gte.h"` and GCC 2.7.2 silently discards the six VECTOR-store statements on parse-error recovery (see evidence.md). Restoring the include (present in the candidate full-file snapshot) measures **0 (74/74, 0 rules dropped)**. No frontier remains — the function is candidate-ready on the grant path to COMPLETED-INLINE-ASM-CANONICAL.
