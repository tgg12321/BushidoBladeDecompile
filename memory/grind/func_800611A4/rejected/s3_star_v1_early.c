/* s3 V12 — REJECTED score=20 build_insns=43. Moving `*v1 = 0x21001A;` to
 * immediately after `D_800A3468 = (s32)v1;` (early pre-call) breaks the
 * scheduling. Target keeps *v1=K at the END of pre-call (last insn before
 * jal). Killing this variant reconfirms that *v1 must be the last pre-call
 * memory op. Not the RA lever. */
