/* func_8001C624 — BEST HONEST FORM (s3, 2026-07-13).  125/127 insns, sandbox 73.
 *
 * NOT byte-matching, and NOT committable as-is.  It is here because it is the
 * ONLY wrapper-free, cheat-free form measured so far that is SEMANTICALLY
 * CORRECT — every block copy reads FRESH data.  Every plain-symbol form (incl.
 * the 61-scoring baseline and all 14 statement orders swept in s3) emits copies
 * that read STALE data; see rejected/plain-symbols-wrapper-free-baseline.c.
 *
 * Resume from HERE, not from the committed HEAD body (which is the regression:
 * two do-while(0) scheduling barriers, killed for sanction in s2).
 *
 * WHY IT WORKS (the semantic fix)
 * Typing each copied region as an aggregate gives the three block copies a base
 * symbol SHARED with the stores that feed them.  sched.c's memrefs_conflict_p
 * short-circuits on GET_MODE_SIZE(BLKmode)==0 only once the canonicalised bases
 * are rtx_equal, so a shared base symbol makes each copy conflict with every
 * store to that object at any offset.  The copies stop floating.  Statement
 * order then becomes fully INERT (s3 measured 5 orders: 73/73/73/74/73) because
 * the dependence graph is saturated and rank_for_schedule's LUID tie-break has
 * nothing left to decide.
 *
 * THE TWO REMAINING DELTAS (both register-allocation / CSE artifacts, -2 insns
 * net).  Neither is a semantic defect; both are named and mechanical:
 *
 *  (1) OFFSET-0 MEMBER STORE LOSES ITS at-form `lui` (-1 insn x 3 groups).
 *      Expand emits every COMPONENT_REF store's address into a pseudo:
 *          (insn 43 (set (reg:SI 84) (symbol_ref:SI ("D_80101FB0"))))
 *          (insn 45 (set (mem/s:SI (reg:SI 84)) (const_int 0)))
 *      For offsets 4/8 the pseudo is (const (plus (symbol_ref FB0) N)) with a
 *      SINGLE use, and combine folds it back into the MEM — at-form is restored
 *      (`sw v0,%lo(D_80101FB4)($at)` — verified in the emitted asm).  For the
 *      OFFSET-0 member the pseudo is the bare (symbol_ref FB0) — rtx-identical
 *      to what the block copy's copy_addr_to_reg needs — so CSE commons the two,
 *      the pseudo becomes MULTI-USE, and combine can no longer fold it.  The
 *      store is stuck at `sw zero,0(v1)`.  Insensitive to statement order
 *      (it is an expand+CSE fact, not a scheduling one).
 *
 *  (2) BLOCK-COPY CHUNKING num_regs=3 INSTEAD OF 2 (net +1 insn).
 *      mips.md's movstrsi_internal always requests 4 scratch regs; mips.c's
 *      output_block_move (line 2445) then counts how many do NOT collide with
 *      the two address registers (`safe_regs`) and recurses with that count.
 *      The target got 2 (lw,lw,sw,sw pairs); this form gets 3.  Pure
 *      register-allocation fallout — not spellable in C, but it should fall out
 *      once the live ranges match.
 *
 * The arithmetic closes exactly: 125 + 3 (restore the three at-form luis)
 * - 1 - 1 + 1 (num_regs 3->2 on copy1/copy3/copy2) = 127.
 *
 * DECLARATIONS this body requires (all nine words are used ONLY inside this
 * function — verified by grep over src/ — so the correction is fully contained):
 *
 *   file scope in src/code6cac.c:
 *     typedef struct { s32 a, b, c, d; } Blk16;
 *     typedef struct { s32 a, b, c; } Blk12;
 *     extern Blk16 D_80101F80;    extern Blk16 D_80101F90;
 *     extern Blk12 D_80101FB0;    extern Blk12 D_801020C0;
 *     extern Blk16 D_80101FCC;    extern Blk16 D_80102114;
 *   DELETE from include/code6cac.h:
 *     extern s32 D_80101F80/F84/F88/FB0/FB4/FB8/FCC/FD0/FD4;
 *   DELETE from src/code6cac.c:
 *     extern s32 D_80101F90/801020C0/80102114;
 *
 * NOTE: this typing is NOT yet Judge-cleared.  It is a header type correction
 * and must be argued against .claude/rules/header-type-correction-from-use-sites
 * (four prongs) before any commit.  s2's contrary reading — that the target's
 * at-form stores PROVE the region was not an aggregate — is strengthened by
 * delta (1) above: GCC 2.7.2 structurally cannot emit an at-form offset-0 store
 * for an object it also block-copies.  Whoever picks this up must resolve that
 * tension, not paper over it.
 */
void func_8001C624(void) {
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    /* `i` must stay a VARIABLE: as a literal 0x36, combine folds the address
     * into the symbol D_80101FA0 and the build loses 2 insns (125 vs 127). */
    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0.a = 0;
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB0.b = -0x384;
    D_80101FB0.c = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80.a = x;
    D_80101F80.b = y;
    D_80101F80.c = z;
    D_80101F90 = D_80101F80;
    D_801020C0 = D_80101FB0;
    D_80101FCC.a = 0;
    D_80101FCC.b = 0;
    D_80101FCC.c = 0;
    D_80102114 = D_80101FCC;
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
    D_80102010 = D_80101F80.b;
    func_8003FFE0(0);
}
