/* func_8001F938 — REJECTED FORM (s2 structural, 2026-07-23)
 * u16-local-PHI truncation. sandbox floor 4, build_insns 106 (target 107).
 *
 * WHY MEASURED: the s1/s2 ledger claimed "NO register-level construct can force
 * the opaque (unfolded <<16>>15) shape; only a second TYPED memory view of the
 * field works" (s2 P2b, after killing the NON-branched `raw_or_3 = probe & 0xFFFF`
 * which GCC eliminates+folds -> floor 8). That claim was an OVER-GENERALIZATION:
 * it only tested the non-branched register mask. This form tests a u16-typed
 * local assigned in a BRANCH (PHI of {3, (u16)probe}). The PHI blocks GCC's
 * back-substitution, so the `& 0xFFFF`-equivalent truncation is NOT subsumed by
 * the <<16 and the fold is defeated -> unfolded sll16;sra15 emitted -> floor 4.
 *
 * RESULT: floor 4 (< clean floor 8), build_insns 106. ONE dereference (lh, signed,
 * for `probe`); the u16 truncation materializes in-register (andi) where target
 * has a SECOND memory load (lhu). So it is 1 insn short of target (the missing lhu)
 * and 4 off in coupled register/shape — cannot reach 0 without becoming the memory
 * dual-read (the pre-banned distance-0 form). Strictly dominated by the known
 * signed-cast-single-read distance-0 cheat (rejected/signed-cast-single-read.c).
 *
 * WHY REJECTED (NOT proposable): the `u16` type on raw_or_3 has ZERO observable
 * effect — for every probe value, ((s32)(u16)probe << 16) >> 15 == (probe << 16) >> 15
 * (the <<16>>15 idiom keeps only the low 16 bits). Its SOLE purpose is to defeat
 * GCC's fold so the shift stays opaque. That is the SAME signedness-split /
 * CSE-defeat intent the Judge pre-banned "in ANY spelling" (guarded ternary,
 * unconditional split, union, two-pointer, single-u16-read + (s16) cast) — this is
 * a 5th spelling (single-s16-read + u16-local truncation), same family. Fresh
 * cheat-reviewer FAIL (tmp/grind/func_8001F938/s2/cheat_reviewer_u16local.txt).
 *
 * CONCLUSION: corrects the s2 over-claim (register-level constructs CAN defeat the
 * fold — via a branch-PHI narrow-type truncation) but does NOT open a clean path:
 * every fold-defeat is the pre-banned signedness-split family. src/ kept at clean
 * floor-8. This spelling is now eliminated from future search.
 */
    {
        s32 probe = *((s16 *)(arg0 + 0x270));
        u16 raw_or_3;
        if (probe >= 4) {
            raw_or_3 = 3;
        } else {
            raw_or_3 = probe;
        }
        idx = (((s32)raw_or_3 << 16) >> 15);
    }
