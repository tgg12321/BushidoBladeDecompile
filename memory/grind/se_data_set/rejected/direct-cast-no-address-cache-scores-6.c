/* REJECTED (s2, structural) — sandbox --disable all = 6 (93/93 insns).
 *
 * The ledger's headline hypothesis: "block-local CSE unifies the two identical
 * lb-address computations into a call-surviving pseudo, so the direct spelling
 * reproduces target's $s0 address cache with no pointer at all."  KILLED.
 *
 * Measured: each body site emits its own `lui %hi(D_8010277C); lb %lo(...)`
 * (tmp/grind/se_data_set/s2/probeB_direct_cast.txt lines 69-72 and 94-97)
 * instead of target's `lui/addiu $s0` + `lb 0($s0)` x2.
 *
 * Mechanism (why the hypothesis was wrong): in GCC 2.7.2/MIPS a non-PIC
 * SYMBOL_REF is a LEGITIMATE_ADDRESS, so both reads expand as MEM(SYMBOL_REF)
 * and there is no separate address expression for cse.c to unify — cse never
 * INSERTS insns, it only substitutes cheaper equivalents that already exist.
 * `la $s0, sym` is emitted only for a movsi of a SYMBOL_REF into a pseudo, i.e.
 * from a C-level `&` bound to a variable.  fold() collapses `*(&sym)` back to
 * MEM(sym), so the variable is required — an expression-level `&` will not do.
 */
        EndADRSound();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + (s8)D_8010277C), *(&D_8008E5A8 + D_8010277D));

        D_800A390E = D_800A36A4;
        D_800A30FC = *(&D_8008E5A8 + (s8)D_8010277C);
        D_800A30FD = *(&D_8008E5A8 + D_8010277D);
