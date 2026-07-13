/* REJECTED as a closing form (score 61) — but KEEP THIS FILE: it is the CLEAN
 * BASELINE the s2 frontier (statement-order sweep) starts from, and scratch
 * (tmp/) is gitignored.
 *
 * This is the committed HEAD body with the two do-while(0) wrappers removed and
 * the t/u round-trip replaced by the Judge-approved setVector comma-assign
 * respell.  It is the honest spelling: every global under its own name, no
 * casts, no wrappers, no dead temps, no pins.  127/127 insns.
 *
 * WHY IT SCORES 61 — and why it is also WRONG, not merely mis-scheduled:
 *   GCC hoists `*(Blk12*)&D_801020C0 = *(Blk12*)&D_80101FB0;` to sit immediately
 *   after `sw zero,%lo(D_80101FB0)`, i.e. ABOVE the D_80101FB4 / D_80101FB8
 *   stores that feed it, so the emitted code copies STALE FB4/FB8.  The
 *   byte-matching build copies (0, -0x384, 0).  Cause: the copy is `(mem:BLK
 *   (reg))`, BLKmode size is 0, and sched.c's memrefs_conflict_p only reports a
 *   conflict when the canonicalised bases are rtx_equal — D_80101FB0 and
 *   D_80101FB4 are different symbol_refs, so GCC sees NO dependence at all.
 *   (Full trace in hypotheses.md.)
 *
 * DO NOT "fix" this by re-basing the symbols (rejected: cheat) or by declaring
 * the region as an aggregate (rejected: forces a shared base register).  The
 * un-swept lever is STATEMENT ORDER: sched.c's rank_for_schedule breaks priority
 * ties by INSN_LUID, i.e. by source order.  Sweep from HERE, and measure with
 * the reloc-normalised diff, not the raw score.
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    /* `i` must stay a VARIABLE: written as a literal 0x36, combine folds the
     * address into the symbol D_80101FA0 and the build loses 2 insns (125/127). */
    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB4 = -0x384;
    D_80101FB8 = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80 = x;
    D_80101F84 = y;
    D_80101F88 = z;
    *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
    *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
    D_80101FCC = 0;
    D_80101FD0 = 0;
    D_80101FD4 = 0;
    *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
    /* setVector idiom (Judge-approved respell of the t/u round-trip): the target
     * genuinely contains the self-copy stores — `local` is address-taken. */
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F84;
    func_8003FFE0(0);
}
