# SELF-VET — func_8002FC80

CONSTRUCTS: whole-body file-scope `__asm__("glabel func_8002FC80 ...")` canonical block (the ONLY construct in the diff; it replaces `INCLUDE_ASM("asm/funcs", func_8002FC80);` one-for-one — no C body, no locals, no stores, no islands, no other statements added or changed anywhere in the TU)

## T1 semantic purpose: PASS — the block IS the function's entire observable behavior (76 instructions transcribed verbatim from asm/funcs/func_8002FC80.s, byte-verified by sandbox 0 at 74/74 scored insns). Nothing in the diff exists to influence compiler analysis; cc1 emits the text verbatim and compiles no code for this function.
## T2 human-programmer: PASS — for a function whose accepted finished form is COMPLETED-INLINE-ASM-CANONICAL (hand-written GTE routine per the OWNER-CLUSTER ruling), the whole-body glabel block is exactly what a human maintainer of this project writes; it matches the established precedent blocks in src/ings.c, src/text1a_c.c, src/text1b_b.c.
## T3 GCC-internals justification: PASS — no GCC pass is the mechanism of anything here; the compiler is bypassed for this body by design of the sanctioned canonical-asm form. The justification is provenance (hand-written asm evidence: `addu $t4,$aN,$zero` cop2 addressing preamble ×2, unfilled cop2 load-delay nops, splat `/* handwritten instruction */` tags), not codegen coercion.
## T4 permuter/search provenance: PASS — no auto-search produced this form; it is a verbatim transcription of the target disassembly under an explicit Judge grant instruction ("integrate the whole-body form per canonical-asm-authorization-recipe").
## T5 family check: PASS — whole-body canonical asm is not in the forbidden-family catalog; it is the codified COMPLETED-INLINE-ASM-CANONICAL end state (inline-asm-policy "canonical-body" category), gated by the allowlist, and the allowlist entry for this function exists (inline_asm_canonical.txt:365, pipeline grant 2026-08-31). The two BANNED constructs for this function (VECTOR-typed scratchpad stores; the VECTOR-vs-plain-s32 store spelling choice) do NOT appear in the diff — there are no C stores at all.
## T6 naming-announces-intent: PASS — no locals exist; the only introduced name is the branch label `.L_func_8002FC80_ret`, named per the canonical-asm-authorization-recipe rule 2 (collision avoidance), which is assembly plumbing inside a sanctioned whole-body block, not a coercion name.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical-body whole-function asm (COMPLETED-INLINE-ASM-CANONICAL)
  SCOPE: "Full canonical-asm function body — original was hand-written asm, or the function emits via file-scope `__asm__(\"glabel ...\")` as its accepted finished form. Listed in `inline_asm_canonical.txt`."
  PRECEDENT: inline_asm_canonical.txt:365

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The canonical-body family's rule (canonical-asm-authorization-recipe / inline-asm-policy) mandates the inline_asm_canonical.txt allowlist entry, not a /* FAKE */ annotation; the allowlist entry exists at inline_asm_canonical.txt:365 (driver-written pipeline grant 2026-08-31, tier OWNER-CLUSTER, Judge packet docs/grind/decisions.md 2026-08-31), and the in-source comment above the block cites it.
