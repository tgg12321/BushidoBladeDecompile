/* func_80041188 / hirahira_w_ctrl - s5 candidate. sandbox --disable all == 1.
 * 132/132 insns, frame 72 == target 0x48. Pure C: no register pin, no inline
 * asm, no volatile, no dead code, no unused local.
 *
 * This is the s3 floor-1 form, RESTORED as the best legal candidate after the
 * s4 form (distance 0) was rejected at layer-1 review: its closing lever was a
 * do { } while (0) wrapping only loop1's LEADING HALF with the `loop1:` label
 * inside and the back-goto entering from outside - now a BANNED construct for
 * this function (docs/grind/decisions.md 2026-08-22 23:24).
 *
 * Single residual insn, slot 71: OURS `move s3,s6` vs TGT `addiu s3,s7,32`.
 *
 * s5 (synthesis) corrected the governing arithmetic. GCC 2.7.2 global.c ranks
 * allocnos by pri = floor_log2(reg_n_refs) * reg_n_refs / reg_live_length *
 * 10000, and floor_log2(3) == 1 (earlier ledger sessions used 2). Consequences,
 * all dump-measured this session:
 *   - the ONLY thing wrong in any of these chassis is the relative order of
 *     tbl (79), out2 (86) and the pa4 carrier (77); every other callee-saved
 *     seat is already target in all of them.
 *   - the MID (between-loops) statement `stptr = <X> + 0x20` is a single
 *     ref-token on a see-saw: reading out2 gives out2 4 refs / pa4 6 refs
 *     (1702.1 vs 1263.2 - correct seats, wrong opcode: `move`); reading pa4
 *     gives out2 3 refs / pa4 7 refs (714.3 vs 1473.7 - correct opcode
 *     `addiu s3,s7,32`, wrong seats). No third spelling of that one statement
 *     exists.
 *
 * s6 (forensics, 2026-08-23) re-verified this form at sandbox 1 / 132 insns and
 * instrumented the allocator end-to-end (BB2_ALLOC_DEBUG + BB2_FINDREG_DEBUG):
 * seats are decided by priority order alone (no copy preferences, no
 * someone_prefers, no REG_ALLOC_ORDER on MIPS), so out2 must be processed
 * before the pa4 carrier, i.e. pri(out2) in (1473.7, 1702.1). s6 also closed
 * the entire staged-copy ref-lift family: cse1 either propagates a single-use
 * copy away (flow then deletes it UNCOUNTED) or pulls a second consumer onto it
 * (combine can then never delete it, +1 insn). See evidence.md s6.
 */
void func_80041188(s32 a0, u8 *a1, u8 *a2, s32 a3, s32 *a4)
{
    s32 *pa4 = a4;
    s32 i = 1;
    s32 *tbl = D_80094CFC;
    s32 base = D_800A9A10[a0];
    s16 buf[3];
    s32 saved;
    s32 *out2;
    s32 stptr;
    s32 offset;
    u16 *p;
    s32 stptr2;
    saved = base + 0x94;
    out2 = (s32 *) (((u8 *) pa4) + 0x20);
    stptr = base;
    stptr += 0xFC;
    loop1:
    offset = (*tbl) * 6;
    p = (u16 *) (offset + (s32) a1);
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, pa4);
    tbl++;
    offset = offset + (s32) a2;
    p = (u16 *) offset;
    i++;
    buf[0] = p[0];
    buf[1] = -p[1];
    buf[2] = -p[2];
    func_8004A348(buf, out2);
    func_800523E0(pa4, out2, a3, stptr + 0x38);
    *((s16 *) (stptr + 6)) = 2;
    stptr += 0x68;
    if (i < 0x12) {
        goto loop1;
    }
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    stptr2 = saved + 0x750;
    /* FAKE: loop2's output pointer reuses the (dead-after-loop1) `stptr` walker
       local instead of a fresh local, and is re-initialised by READING `out2`,
       mechanism: GCC 2.7.2 global.c allocno priority
       (floor_log2(refs)*refs/live*10000) - the merged stptr pseudo scores 3409
       and lands s3 == target, while the out2 read is out2's 4th flow-counted
       ref (4/47 = 1702.13, an exact tie with tbl that breaks our way on allocno
       number) which keeps out2 in s6 == target. lever-exhaustion:
       memory/grind/func_80041188/evidence.md s3 + s5 */
    stptr = (s32) out2;
    loop2:
    func_80044DE4((s16 *) a1, (s16 *) a2, a3, stptr2 + 0x4C);
    a1 += 6;
    a2 += 6;
    buf[0] = *((u16 *) a1);
    a1 += 2;
    buf[1] = -(*((u16 *) a1));
    a1 += 2;
    buf[2] = -(*((u16 *) a1));
    a1 += 2;
    func_8004A348(buf, pa4);
    buf[0] = *((u16 *) a2);
    a2 += 2;
    buf[1] = -(*((u16 *) a2));
    a2 += 2;
    buf[2] = -(*((u16 *) a2));
    a2 += 2;
    func_8004A348(buf, (s32 *) stptr);
    func_800523E0(pa4, (s32 *) stptr, a3, stptr2 + 0x38);
    *((s16 *) (stptr2 + 6)) = 1;
    stptr2 += 0x68;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}
