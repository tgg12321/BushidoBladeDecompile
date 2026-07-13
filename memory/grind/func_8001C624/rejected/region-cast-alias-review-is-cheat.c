/* REJECTED (s2) — SELF-REJECTED AS A CHEAT, not because it fails.
 *
 * *** THIS FORM REPRODUCES THE TARGET EXACTLY. ***
 *   127/127 insns; the reloc-normalised instruction diff vs build/src/code6cac.o
 *   is EMPTY (identical mnemonics, identical operands, identical resolved
 *   addresses).  No do-while wrappers.  No dead temps.  No register pins.
 *   Sandbox reports 33 — that is ENTIRELY reloc-SPELLING noise (see below), not
 *   a byte difference.
 *
 * WHY IT WORKS (mechanism, fully traced):
 *   The three block-copies are `(mem:BLK (reg))` in RTL.  BLKmode =>
 *   SIZE_FOR_MODE == 0, and sched.c's memrefs_conflict_p returns CONFLICT
 *   whenever a size is 0 and the canonicalised bases are equal.  Re-spelling all
 *   ~28 region globals as `((s32 *)&D_80101F80)[k]` gives every one of them the
 *   SAME base symbol as the copies' addresses, so each copy becomes dependent on
 *   every region store => sched1 has no freedom => exact source order => target.
 *   The cast (as opposed to an aggregate DECLARATION) keeps each address a
 *   compile-time constant, so the at-form `lui $at ; sw ..,%lo(SYM+K)($at)`
 *   survives.  Best of both — which is exactly the tell.
 *
 * WHY IT IS A CHEAT (cheats-by-any-spelling, no-new-park-categories):
 *   1. No semantic purpose.  These globals have their own names.  No human
 *      writes `((s32 *)&D_80101F80)[23] = 0;` for a word named D_80101FDC.
 *   2. Invisible in the emitted output.  The re-view changes NO bytes; it exists
 *      solely to change GCC's alias analysis.
 *   3. Its justification cannot be stated without naming GCC internals
 *      ("makes memrefs_conflict_p return conflict for a BLKmode size-0 ref").
 *   4. Family: pointer-pun / typed re-view of globals — the
 *      [[pointer-alias-fake-exception]] family, whose narrow sanction covers a
 *      single C-level LOCAL alias as an annotated last resort, never a
 *      28-global re-basing.
 *   5. And it is affirmatively FALSE as a reconstruction: if the region really
 *      were one aggregate, the original would have been DECLARED that way, and
 *      an aggregate declaration provably emits a shared base register instead of
 *      the target's at-form (see
 *      rejected/whole-region-aggregate-decl-forces-base-register.c, 99 insns).
 *      The target's codegen is that of separate scalar symbols.  So this form
 *      asserts an object that the target's own bytes say does not exist.
 *
 * SECONDARY FINDING — THE SANDBOX CANNOT SCORE THIS CLASS:
 *   `((s32*)&D_80101F80)[23]` relocates as D_80101F80+92; the target relocates
 *   as D_80101FDC+0.  The linker resolves both to 0x80101FDC, so the LINKED
 *   BYTES ARE IDENTICAL — but the .o disassembly text differs (`sw zero,92(at)`
 *   vs `sw zero,0(at)`), and the engine's distance metric diffs that text.  So
 *   ANY base+offset respelling is structurally unable to reach sandbox 0 even
 *   when it is byte-perfect.  Only a full build + SHA1 can adjudicate that class.
 *   (Relevant to the driver's `candidate-ready == sandbox 0` contract.)
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    ((s32 *)&D_80101F80)[12] = 0;                                     /* FB0 */
    x = ((s32 *)&D_80101EC8)[0x36];
    y = ((s32 *)&D_80101F80)[9];                                      /* FA4 */
    z = ((s32 *)&D_80101F80)[10];                                     /* FA8 */
    ((s32 *)&D_80101F80)[13] = -0x384;                                /* FB4 */
    ((s32 *)&D_80101F80)[14] = 0;                                     /* FB8 */
    ((s32 *)&D_80101F80)[15] = x;                                     /* FBC */
    ((s32 *)&D_80101F80)[16] = y - 0x384;                             /* FC0 */
    ((s32 *)&D_80101F80)[17] = z;                                     /* FC4 */
    ((s32 *)&D_80101F80)[0] = x;
    ((s32 *)&D_80101F80)[1] = y;
    ((s32 *)&D_80101F80)[2] = z;
    *((Blk16 *)&((s32 *)&D_80101F80)[4]) = *((Blk16 *)&((s32 *)&D_80101F80)[0]);
    /* copy2's SOURCE must be spelled through its own symbol: if it is spelled
     * &((s32*)&D_80101F80)[12] the address rtx is identical to the FB0 store's,
     * CSE commons them into one base register, and the FB0 store degrades to
     * `sw zero,0(a2)` (v11: 125 insns).  With &D_80101FB0 the rtx differs, no
     * CSE, at-form preserved — and the copy is still fenced by its DESTINATION,
     * which is region-based. */
    *((Blk12 *)&((s32 *)&D_80101F80)[80]) = *((Blk12 *)&D_80101FB0);
    ((s32 *)&D_80101F80)[19] = 0;                                     /* FCC */
    ((s32 *)&D_80101F80)[20] = 0;                                     /* FD0 */
    ((s32 *)&D_80101F80)[21] = 0;                                     /* FD4 */
    *((Blk16 *)&((s32 *)&D_80101F80)[101]) = *((Blk16 *)&((s32 *)&D_80101F80)[19]);
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    ((s32 *)&D_80101F80)[23] = 0;                                     /* FDC */
    ((s32 *)&D_80101F80)[24] = 0;                                     /* FE0 */
    ((s32 *)&D_80101F80)[25] = 0;                                     /* FE4 */
    ((s32 *)&D_80101F80)[27] = 0;                                     /* FEC */
    ((s32 *)&D_80101F80)[28] = 0;                                     /* FF0 */
    ((s32 *)&D_80101F80)[29] = 0;                                     /* FF4 */
    ((s32 *)&D_80101F80)[31] = 0;                                     /* FFC */
    ((s32 *)&D_80101F80)[32] = 0;                                     /* 2000 */
    ((s32 *)&D_80101F80)[33] = 0;                                     /* 2004 */
    ((s32 *)&D_80101F80)[35] = 0;                                     /* 200C */
    ((s16 *)&D_80101F80)[74] = 0;                                     /* 2014 */
    ((s16 *)&D_80101F80)[76] = 0;                                     /* 2018 */
    ((s16 *)&D_80101F80)[77] = 0;                                     /* 201A */
    ((s16 *)&D_80101F80)[75] = 0;                                     /* 2016 */
    ((s32 *)&D_80101F80)[36] = ((s32 *)&D_80101F80)[1];               /* 2010 = F84 */
    func_8003FFE0(0);
}
