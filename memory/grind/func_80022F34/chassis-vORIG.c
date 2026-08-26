/* func_80022F34 — SECOND CHASSIS "vORIG" (grind s7, solver, 2026-08-26)
 * sandbox --disable all = 11  (build_insns 69, target 70) — TIES candidate.c's
 * floor, but with an EXACTLY COMPLEMENTARY residual, and is the better handoff
 * chassis for the next modality.
 *
 * WHAT IT FIXES vs candidate.c (`base`): the +8 phantom frame slot is GONE.
 * Measured: .frame vars=0, regs=4/0, `subu $sp,$sp,32` with saves at
 * 16/20/24/28 — BYTE-IDENTICAL to target's prologue and epilogue. The first 39
 * normalized instructions (prologue, loop head, guard, the whole switch
 * including its compare chain, the sh store, the loop tail, the epilogue) match
 * target exactly. combine emits NO orphaned (use regN) — strand=0.
 *
 * WHAT IT COSTS: cse2 now shares ONE `la $5,D_801027BC` base for both loads,
 * where target re-materialises %hi/%lo per access. The entire 11 is that one
 * 15-insn block:
 *     ours    lui a1,%hi ; addiu a1,a1,%lo ; addu v1,v1,a1 ; lw a1,0(v1)
 *                                          ; addu v0,v0,a1 ; lw a2,0(v0)
 *     target  lui at,%hi ; addu at,at,v1 ; lw a1,%lo(at)
 *             lui at,%hi ; addu at,at,v0 ; lw a2,%lo(at)
 * Multiset delta is literally ONE shape (ours `addiu #,#,0` vs target
 * `lui #,0x0`) plus target's maspsx label `nop` (retirable via
 * maspsx_label_nop_funcs.txt on the DONE path).
 * `goal_from_tgt.py classify code6cac func_80022F34` on this chassis:
 * PRE-RA, only those three one-stream-only shapes (s7/classify_vORIG.txt).
 *
 * THE LAW s7 MEASURED (13 forms, s7/probe2.sh):
 *   separate EITHER D_801027BC load into its own named s32 temp
 *     => cse2 stops sharing => per-access fold x2 => ONE stranded address
 *        pseudo => vars=8, frame -40                     [base, vO2, vO4, vO6,
 *                                                         vO7, vO13, vO16]
 *   keep BOTH loads inside the single call expression
 *     => shared `la` base => no fold, no strand => vars=0, frame -32
 *                                                        [this file, vO3, vO5,
 *                                                         vO11, vO12, vO14]
 * Placement is inert (vO2 defines val1 AFTER the a0 reload and still strands).
 * Which load is separated is inert (vO7 separates val2 only — still strands).
 * Symbol type is inert (vO14/vO16, `extern s32 D_801027BC[];`).
 * Byte-pointer address arithmetic is inert (vO11/vO12).
 *
 * NEXT LEVER on this chassis: a cse.c cost lever that makes cse2 re-materialise
 * the symbol base per load WITHOUT lifting a load into its own statement.
 * Note target's form is one insn MORE expensive than ours (70 vs 69), which is
 * the direction a cost-model lever moves. This is a PRE-RA question only — s7's
 * solver classify forecloses ra_solver and sched_solver on BOTH chassis.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = (u16 *)&D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *a0 = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(a0 + 6) != 0) {
            s32 val = D_800A38DC;

            switch (val) {
                case 0:
                    *(s16 *)(a0 + 8) = (&D_80102782)[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(a0 + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                u8 *nxt = *(u8 **)a0;
                s16 idx1 = *(s16 *)(a0 + 0x4A);
                s16 idx2 = *(s16 *)(nxt + 0x4A);
                single_game_SetStatusUpData(i, (&D_801027BC)[idx1 * 5],
                                            (&D_801027BC)[idx2 * 5]);
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}
