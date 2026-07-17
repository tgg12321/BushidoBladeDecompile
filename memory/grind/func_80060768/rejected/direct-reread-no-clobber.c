/* REJECTED (s2, 2026-07-17): direct re-read with NO staging of the increment
 * value — sandbox distance 16 (8 per half). GCC 2.7.2 cse keeps the
 * mem:HI(D_800A32B4) == reg 98 equivalence ACROSS the two intervening
 * variable-address stores (sh 0x7DE/0x7DC($s2)): invalidate_memory only drops
 * mems whose address VARIES; symbol-addressed mems survive a varying-address
 * store. The clamp re-read folds to sll/sra/slti of the live increment
 * register instead of the target lh reload (build idx 55-57 / both halves).
 * Evidence: tmp/grind/func_80060768/s2/text1b.i.cse.func80060768 — after cse
 * only insns 92/97/145 touch D_800A32B4; the re-read insn is gone.
 * Fix that works: stage the increment through t1/t2 and REUSE the var for the
 * product, clobbering the equivalence register (see candidate.c). */
        cur1 = D_800A32B4;
        D_800A32B4 = cur1 + 1;               /* stored value lives untouched in reg 98 */
        t1 = (s32)((s16)cur1) * 0x1AA;
        *(s16 *)(arg0 + 0x7DE) = 2;
        *(s16 *)(arg0 + 0x7DC) = (s16)(t1 / 0x1E);
        if ((s16)D_800A32B4 >= 0x1F) {       /* folds to sll/sra of reg 98 — no lh */
            D_800A32B4 = 0x1E;
        }
