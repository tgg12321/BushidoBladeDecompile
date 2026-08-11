/*
 * CANDIDATE — func_800401CC (src/text1a_pre.c), session s8 (forensics,
 * 2026-08-11; driver session 3). SANDBOX 0/78, verified this session,
 * form APPLIED in src/text1a_pre.c.
 *
 * THE CLOSING FORM IS THE OTag BITFIELD SPELLING — no staged variables,
 * no FAKE annotations, no mask literals anywhere in the function. The
 * packet-link statements are written exactly like the PsyQ SDK's addPrim
 * macro (P_TAG bitfield struct access), using the project's existing
 * sanctioned OTag type (include/gpu.h; user-sanctioned 2026-06-11 when it
 * closed ot_Insert/ot_Link in gpu.c — see
 * .claude/rules/bitfield-direction-divergence.md, confirmed-case section,
 * and src/gpu.c AddPrim which is this exact two-statement idiom).
 *
 * Requires `#include "gpu.h"` in src/text1a_pre.c (added this session).
 *
 * WHY IT CLOSES (s8 forensic dump, tmp/grind/func_800401CC/s3/):
 * expmed's bitfield insert/extract expansion materializes the two field
 * masks pre-combine with DIFFERENT reference counts than any
 * mask-arithmetic spelling can produce:
 *   QTYDBG blk=5: low-mask qty reg1=117 birth=18 death=48 refs=4 got=$6
 *                 high-mask qty reg1=121 birth=28 death=46 refs=3 got=$7
 * refs=4 vs refs=3 flips qty_compare_1 (local-alloc.c:1660): low mask
 * priority floor_log2(4)*4*4/30 = 32/30 beats high mask 12/18 -> low mask
 * allocated FIRST -> takes $6, high mask takes $7 (both = target). This is
 * exactly the honest refs-lift that s1's frontier F1 asked for and that
 * s7's K11 declared site-less — K11 only considered adding a ref via a
 * duplicated STATEMENT (new bytes); the bitfield expansion adds the 4th
 * ref inside the expansion itself and combine folds to the same 78 insns.
 * The high mask is also born LATER (28 vs 22 in the literal form), so the
 * emission order (lui+ori low mask first, then the 1-insn high mask li)
 * matches target too — both residual classes close at once. The s7
 * corollary ("only the banned dual-staged shape reaches the bytes") was
 * scoped to mask-ARITHMETIC spellings and is superseded by this form.
 *
 * Byte-level proof: tmp/grind/func_800401CC/s3/sandbox_disasm.txt
 * (sandbox object tail: lui a2,0xff; ori a2,ffff; lui a3,0xff00; 4 ands
 * on a2/a3; or-dests and sw placement all target-identical).
 */
void func_800401CC(s32 a0, s32 a1, s32 a2) {
    s16 buf[4];
    u16 *tbl;
    s16 u, v;
    OTag *pkt;
    OTag *ot;

    a2 = D_800A36AC & 1;
    if (a2 != D_800A3234) {
        D_800A3378 = (s32)(&D_800A9830 + a2 * 240);
        D_800A3234 = a2;
    }
    if ((s32 *)D_800A3378 != (s32 *)(&D_800A9920 + D_800A3234 * 240)) {
        tbl = &D_80094AF4 + a1 * 6;
        buf[0] = *tbl++;
        buf[1] = *tbl++;
        buf[2] = *tbl++;
        buf[3] = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf[0] = buf[0] + 0x80;
            u = u + 0x80;
        }
        SetDrawMove((s32)(s32 *)D_800A3378, buf, (s16)u, (s16)v);
        pkt = (OTag *)D_800A3378;
        ot = (OTag *)D_800A378C;
        pkt->addr = ot[0x3FFC / 4].addr;
        ot[0x3FFC / 4].addr = (u32)pkt;
        D_800A3378 = (s32)(pkt + 6);
    }
}
