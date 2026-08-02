/* func_80052B00 — BEST HONEST PURE-C FORM, grind session 3 (structural).
 *
 * Honest floor: `sandbox func_80052B00 --disable all` = 17 (build 18 insns,
 * target 17).  This is the first movement of this function's floor in three
 * sessions — sessions 1 and 2 both measured 18, and session 2's 14-spelling
 * structural sweep found nothing below it.
 *
 * WHAT CHANGED. Sessions 1-2 varied the STATEMENT level (declaration order,
 * load order, walking pointer, base-copy handle, block scoping, interleaving,
 * aggregate reshaping, cop2 write order, trailing return, do-while(0)) while
 * always emitting eight SEPARATE one-operand `__asm__ volatile ("ctc2 %0,$N")`
 * statements.  Session 3 varied a lever none of those touched: the GRANULARITY
 * of the asm statements.  Fusing all eight cop2 writes into ONE `__asm__`
 * statement with eight "r" inputs forces all eight loaded values to be
 * simultaneously live at a single program point.  That is the only C-level
 * construct that can REQUIRE eight distinct hard registers without a
 * register-asm pin, and it removes both of the two non-register defects the
 * eight-separate-statements forms had:
 *   - the deferred `matrix[0]` load (GCC could no longer reuse the dying $a0 as
 *     a load destination, so the loads come out in strict ascending offset
 *     order 0,4,8,...,28 exactly like the target), and
 *   - the extra load-delay `nop` before the first ctc2 (build 19 -> 18 insns).
 *
 * WHAT THIS EMITS (honest, cheat-stripped; full dump in
 * tmp/grind/func_80052B00/s3/form_disasm.txt):
 *
 *     lw   v0,0(a0)   lw v1,4(a0)   lw a1,8(a0)   lw a2,12(a0)
 *     lw   a3,16(a0)  lw t0,20(a0)  lw t1,24(a0)  lw t2,28(a0)
 *     ctc2 v0,$0  ctc2 v1,$1  ctc2 a1,$2  ctc2 a2,$3
 *     ctc2 a3,$4  ctc2 t0,$5  ctc2 t1,$6  ctc2 t2,$7
 *     jr   ra
 *      nop
 *
 * The emitted body is now instruction-for-instruction ISOMORPHIC to the target
 * — same opcodes, same order, same memory offsets, same operand roles.  The
 * entire remaining 17-point residual is exactly two things:
 *   (1) 16 instructions (8 lw + 8 ctc2) that name the wrong GPRs: the allocator
 *       hands out {$v0,$v1,$a1,$a2,$a3,$t0,$t1,$t2} where the target uses
 *       {$t0..$t7};
 *   (2) 1 instruction for the `jr $ra` delay slot: build emits `nop`, target
 *       holds `ctc2 $t7, $7`.
 *
 * WHY IT CANNOT GO LOWER IN PURE C.
 *   (1) is H2, now with a compiler-source mechanism as well as measurements:
 *       `tools/gcc-2.7.2/config/mips/mips.h` defines NO `REG_ALLOC_ORDER`, so
 *       local-alloc's `find_free_reg` walks hard registers in plain ascending
 *       number order and takes the first non-conflicting one — $2, $3, (skip
 *       $4, the live base pointer), $5, $6, $7, $8, $9, $10.  The only two
 *       documented ways to move that start point are a conflict across a call
 *       and a copy suggestion to a named hard register; a call-free leaf has
 *       neither, and manufacturing either (a clobber list naming $2/$3/$5-$7,
 *       or a `register T x asm("$N")` pin) is a register pin by another
 *       spelling and score-inert under the sandbox besides.  Measured across
 *       18 distinct fused-form spellings in session 3 — const/array/void*
 *       parameter types, reversed declaration order, "d" vs "r" constraints, a
 *       ninth base-pointer operand, a copy chain through a second set of
 *       locals, unsigned temporaries, a walking pointer, reversed load order,
 *       reversed operand order, and 1/2/4/7/8-write asm granularities — the
 *       register SET came back {v0,v1,a1,a2,a3,t0,t1,t2} EVERY time.
 *   (2) is H1, confirmed in session 1 from `tools/gcc-2.7.2/reorg.c:730-735`
 *       (`stop_search_p` returns 1 unconditionally for ASM_INPUT /
 *       asm_noperands >= 0, halting `fill_simple_delay_slots`) and confirmed
 *       empirically by 14 forms in session 2 and 18 more in session 3: every
 *       single one ends `jr $ra ; nop`.  `ctc2` has no C analog, so the target's
 *       delay-slot instruction can only come from an `__asm__` block, and GCC
 *       2.7.2 never puts an `__asm__` insn in a delay slot.
 *
 * STATUS: NOT APPLIED to src/text1b.c.  The HEAD body carries eight
 * `register asm("$N")` pins which, together with `regfix.txt:3411`
 * (`func_80052B00: fill_delay @ 16 <- 15`), are what make the real build
 * byte-match today; swapping in this pin-free form would break the oracle.  It
 * is banked here as the honest pure-C floor and as the body an operator should
 * start from if the canonical-asm disposition is ever refused.
 *
 * The proposed disposition remains memory/grind/func_80052B00/candidate.c (the
 * whole-body glabel canonical-asm form), per the frontier carried since
 * session 1.
 */
void func_80052B00(s32 *matrix) {
    s32 t0;
    s32 t1;
    s32 t2;
    s32 t3;
    s32 t4;
    s32 t5;
    s32 t6;
    s32 t7;
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
        "ctc2 %6, $6\n\t"
        "ctc2 %7, $7"
        :: "r"(t0), "r"(t1), "r"(t2), "r"(t3),
           "r"(t4), "r"(t5), "r"(t6), "r"(t7));
}
