/* REJECTED (s2) — the Judge's named avenue #2, "whole-region struct typing in
 * the header".  MEASURED DEAD.  Do not re-propose in any spelling (struct with
 * named fields, s32[] array, or a `s32 *rg = (s32*)&D_80101F80` pointer view).
 *
 * THE IDEA (and it is a correct diagnosis of the bug):
 *   func_8001C624's three block-copies are `(mem:BLK (reg))` in RTL.  BLKmode
 *   has SIZE_FOR_MODE == 0, and sched.c's memrefs_conflict_p returns CONFLICT
 *   whenever a size is 0 and the two canonicalised bases are equal.  So a block
 *   copy conflicts with EVERY access to the same base symbol at ANY offset, and
 *   with NOTHING at a different symbol.  Our `extern s32 D_80101FB0; extern s32
 *   D_80101FB4; ...` spelling therefore gives the copies ZERO dependence on the
 *   stores that feed them.  Declaring the region as ONE object restores the
 *   dependence — and it genuinely does (measured: the copies stop hoisting).
 *
 * WHY IT IS DEAD — the aggregate DECLARATION changes the ADDRESSING FORM:
 *   GCC 2.7.2 expands a COMPONENT_REF / ARRAY_REF of an aggregate VAR_DECL by
 *   forcing the object's base address into a register (expand_assignment ->
 *   store_field), then CSE commons that register across every member access.
 *   Result: one `lui/addiu` base + `off(reg)` accesses, instead of the target's
 *   per-store `lui $at ; sw ..,%lo(SYM)($at)` at-form.
 *     v10  extern s32 D_80101F80[105]  ->  99 insns  (target 127) — 28 `lui`s
 *                                          gone; copies became `lw v0,-48(v1)`
 *                                          / `sw v0,272(v1)` off a shared base.
 *     v4   per-vector `typedef struct {s32 x,y,z,pad;}` externs
 *                                      -> 125 insns; `sw a0,0(v1)` reg-base for
 *                                          the offset-0 field + block-copy shape
 *                                          changed to lw,lw,lw,sw,sw,sw.
 *     v7   per-vector `extern s32 D_80101FB0[3]`
 *                                      -> 125 insns; same offset-0 reg-base.
 *     v2   `s32 *rg = (s32 *)&D_80101F80;` pointer view
 *                                      ->  the pointer stays live in a register;
 *                                          `sw a1,0(v0)` reg-base stores.
 *
 * THE LOAD-BEARING CONSEQUENCE (this is the real result, record it):
 *   The TARGET emits at-form, per-symbol stores AND per-copy `lui/addiu` bases.
 *   That is the codegen of SEPARATE SCALAR SYMBOLS, not of an aggregate.  So the
 *   ORIGINAL SOURCE DID NOT DECLARE THIS REGION AS AN AGGREGATE.  Any "fix the
 *   aliasing by re-basing the symbols" idea is therefore not a reconstruction of
 *   the original — it is an artifact.  See
 *   rejected/region-cast-alias-reView-is-cheat.c for the spelling that DOES
 *   reach the exact target bytes this way, and why it is still a cheat.
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x, y, z, i;

    /* header carried: extern s32 D_80101F80[105];  (all the per-word D_8010....
     * externs for the region deleted) */
    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101F80[12] = 0;                                     /* FB0 */
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101F80[9];                                      /* FA4 */
    z = D_80101F80[10];                                     /* FA8 */
    D_80101F80[13] = -0x384;                                /* FB4 */
    D_80101F80[14] = 0;                                     /* FB8 */
    D_80101F80[15] = x;                                     /* FBC */
    D_80101F80[16] = y - 0x384;                             /* FC0 */
    D_80101F80[17] = z;                                     /* FC4 */
    D_80101F80[0] = x;
    D_80101F80[1] = y;
    D_80101F80[2] = z;
    *((Blk16 *)&D_80101F80[4]) = *((Blk16 *)&D_80101F80[0]);
    *((Blk12 *)&D_80101F80[80]) = *((Blk12 *)&D_80101F80[12]);
    D_80101F80[19] = 0;
    D_80101F80[20] = 0;
    D_80101F80[21] = 0;
    *((Blk16 *)&D_80101F80[101]) = *((Blk16 *)&D_80101F80[19]);
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101F80[23] = 0;
    D_80101F80[24] = 0;
    D_80101F80[25] = 0;
    D_80101F80[27] = 0;
    D_80101F80[28] = 0;
    D_80101F80[29] = 0;
    D_80101F80[31] = 0;
    D_80101F80[32] = 0;
    D_80101F80[33] = 0;
    D_80101F80[35] = 0;
    ((s16 *)D_80101F80)[74] = 0;
    ((s16 *)D_80101F80)[76] = 0;
    ((s16 *)D_80101F80)[77] = 0;
    ((s16 *)D_80101F80)[75] = 0;
    D_80101F80[36] = D_80101F80[1];
    func_8003FFE0(0);
}
