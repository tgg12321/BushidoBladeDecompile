/* REJECTED — cheat-reviewer FAIL (2026-06-10, session 2).
 *
 * Form: re-type the store target as a 2-element array so the STORE gets
 * MEM_IN_STRUCT_P=1, defeating sched.c true_dependence's escape clause from
 * the store side (the load side keeps its natural arg0[4] spelling):
 *
 *   extern s16 D_800A9A20[2];              // was: extern s16 D_800A9A20;
 *   ...
 *   D_800A9A20[0] = arg0[4];               // ARRAY_REF store -> /s=1
 *   ...
 *   // forced collateral in sibling saTan4FireDisp (only other reference):
 *   if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20[0]) { return; }
 *
 * MEASURED: sandbox --disable all = 0 (75/75 insns, was 2 on the w/h
 * candidate). Full-build SHA1 not run (reverted on reviewer FAIL before
 * retire).
 *
 * WHY IT FAILED REVIEW (verbatim crux):
 *  - Emitted bytes are IDENTICAL to the scalar form (symbol+0 address);
 *    element [1] is never accessed anywhere in the tree. A type change
 *    invisible in the output, justified entirely by its MEM_IN_STRUCT_P
 *    effect, is the same forbidden same-bytes alias-analysis-coercion
 *    family as the two load-side rejections (id-ptr-alias.c,
 *    byte-offset-cast.c) — only the coercion AXIS differs (store vs load).
 *  - The address-map gap at 0x800A9A22 (next symbol D_800A9A24) only
 *    PERMITS an array; it does not REQUIRE one (equally consistent with a
 *    scalar + 2 bytes alignment padding). Legitimate type reconstructions
 *    (u8 retype, lhu->lbu low-byte reads, narrow stack params) all produce
 *    DIFFERENT bytes that match target; this produces none.
 *
 * Do NOT re-derive this form, and do NOT re-derive per-element/struct
 * variants of it (s16[2] vs struct {s16 a,b;} vs s16[1]) — same family,
 * same verdict. The full reviewer verdict is recorded in meta.json
 * (sessions[1] + rejected_forms).
 */
void InitHiraRmd_80041AC8(s16 *arg0); /* see candidate.c for the live body */
