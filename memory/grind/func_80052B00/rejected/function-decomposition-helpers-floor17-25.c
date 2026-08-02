/* func_80052B00 — REJECTED FORM FAMILY (grind session 9, rederive, 2026-08-01)
 *
 * WHY THIS IS DEAD: the FUNCTION-DECOMPOSITION axis — the last C-level
 * structural axis sessions 1-8 had never varied — does not move the honest
 * floor.  Every form measured in sessions 1-8 (32 hand spellings, ~164k
 * permuter iterations) was ONE FLAT FUNCTION BODY; they varied asm granularity,
 * operand shapes, parameter types, load/declaration order, the asm boundary,
 * and occupancy constructs, but never the number of C functions the source was
 * written as.  That mattered a priori because the real PsyQ source for this
 * construct is not flat (SetRotMatrix / SetTransMatrix are separate entry
 * points whose macro bodies are textually composed) and because insns entering
 * the RTL chain through the inliner are built by a different path than insns
 * from a single parse.
 *
 * MEASURED (tmp/grind/func_80052B00/s9/sweep9.py, sandbox --disable all,
 * results in s9/sweep9_results.txt, per-form engine JSON in s9/engout_*.txt):
 *
 *   split7plus1   score 17  (build_insns 18, target 17)  — ties the floor
 *   helpers8      score 25  (build_insns 26)             — strictly worse
 *   helpers4x2    score 17  (build_insns 18)             — ties the floor
 *
 * Nothing below 17.  helpers8 is worse for the obvious reason: eight separate
 * one-write asm insns re-serialize the loads and reintroduce the deferred load
 * plus load-delay nop that session 3's fused form had removed.
 *
 * split7plus1 was not an arbitrary granularity pick — it was DIRECTED at the
 * only eligibility clause in this backend that a single asm insn could ever
 * satisfy.  `ELIGIBLE_FOR_EPILOGUE_DELAY` (tools/gcc-2.7.2/config/mips/mips.h:
 * 2049-2052) requires `get_attr_dslot (INSN) == DSLOT_NO && get_attr_length
 * (INSN) == 1 && ! epilogue_reg_mentioned_p (PATTERN (INSN))`, so a
 * ONE-INSTRUCTION asm is the only asm shape whose length attribute could pass.
 * It never gets asked: the scan that would consult it,
 * `for (trial = get_last_insn (); ! stop_search_p (trial, 1); ...)` at
 * reorg.c:3248, terminates on the asm insn itself (stop_search_p, reorg.c:
 * 730-735, returns 1 unconditionally for ASM_INPUT / asm_noperands >= 0).
 * Measured standalone in s9/dumps/ctlC_split7plus1.s: the tail is a bare
 * `j $31` with the .dbr statistics reading ";; 3 got 0 delays" and no
 * (sequence) built — identical to the honest fused body.
 *
 * These forms are otherwise legitimate pure C (%N placeholders, "r"
 * constraints, no `register asm("$N")` pins, no hardcoded-$N GPRs, no dead
 * locals, no volatile coercion, no fake clobbers).  They are banked as a
 * measured dead end, not as a policy violation.  The floor-17 body to ship if
 * the canonical-asm disposition is ever refused remains
 * memory/grind/func_80052B00/best_pure_c_fused8_floor17.c.
 */

/* --- form 1: split7plus1 (score 17) --------------------------------------- */
void func_80052B00_split7plus1(s32 *matrix) {
    s32 t0, t1, t2, t3, t4, t5, t6, t7;
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    t5 = matrix[5];
    t6 = matrix[6];
    t7 = matrix[7];
    __asm__ volatile (
        "ctc2 %0, $0\n\t"
        "ctc2 %1, $1\n\t"
        "ctc2 %2, $2\n\t"
        "ctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\t"
        "ctc2 %5, $5\n\t"
        "ctc2 %6, $6"
        :: "r"(t0), "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5), "r"(t6));
    __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
}

/* --- form 2: helpers8 (score 25 — strictly worse) ------------------------- */
static inline void wr_cr0(s32 v) { __asm__ volatile ("ctc2 %0, $0" :: "r"(v)); }
static inline void wr_cr1(s32 v) { __asm__ volatile ("ctc2 %0, $1" :: "r"(v)); }
static inline void wr_cr2(s32 v) { __asm__ volatile ("ctc2 %0, $2" :: "r"(v)); }
static inline void wr_cr3(s32 v) { __asm__ volatile ("ctc2 %0, $3" :: "r"(v)); }
static inline void wr_cr4(s32 v) { __asm__ volatile ("ctc2 %0, $4" :: "r"(v)); }
static inline void wr_cr5(s32 v) { __asm__ volatile ("ctc2 %0, $5" :: "r"(v)); }
static inline void wr_cr6(s32 v) { __asm__ volatile ("ctc2 %0, $6" :: "r"(v)); }
static inline void wr_cr7(s32 v) { __asm__ volatile ("ctc2 %0, $7" :: "r"(v)); }

void func_80052B00_helpers8(s32 *matrix) {
    wr_cr0(matrix[0]);
    wr_cr1(matrix[1]);
    wr_cr2(matrix[2]);
    wr_cr3(matrix[3]);
    wr_cr4(matrix[4]);
    wr_cr5(matrix[5]);
    wr_cr6(matrix[6]);
    wr_cr7(matrix[7]);
}

/* --- form 3: helpers4x2, the library decomposition (score 17) -------------- */
static inline void set_rot4(s32 *m) {
    s32 a, b, c, d;
    a = m[0]; b = m[1]; c = m[2]; d = m[3];
    __asm__ volatile (
        "ctc2 %0, $0\n\t"
        "ctc2 %1, $1\n\t"
        "ctc2 %2, $2\n\t"
        "ctc2 %3, $3"
        :: "r"(a), "r"(b), "r"(c), "r"(d));
}

static inline void set_trans4(s32 *m) {
    s32 a, b, c, d;
    a = m[4]; b = m[5]; c = m[6]; d = m[7];
    __asm__ volatile (
        "ctc2 %0, $4\n\t"
        "ctc2 %1, $5\n\t"
        "ctc2 %2, $6\n\t"
        "ctc2 %3, $7"
        :: "r"(a), "r"(b), "r"(c), "r"(d));
}

void func_80052B00_helpers4x2(s32 *matrix) {
    set_rot4(matrix);
    set_trans4(matrix);
}
