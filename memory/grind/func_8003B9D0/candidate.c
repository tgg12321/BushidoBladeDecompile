/* candidate.c — func_8003B9D0 — MATCHING FORM, CONFINED TO src/code6cac_c2.c.
 *
 * MEASURED THIS SESSION (s6, rederive, 2026-08-11 — third run of the modality;
 * the two earlier runs derived the same lever but ended without writing an
 * outcome JSON, so the driver discarded them and src/ was reverted each time.
 * Everything below was re-applied from a clean tree and re-measured here):
 *   & tools/wteng.ps1 main sandbox func_8003B9D0 --disable all
 *     -> "score": 0, target_insns 185, build_insns 185, rules_dropped 1.
 *        The function body carries ZERO __asm__ and ZERO register pins
 *        (cheat_asm_stripped 69 is the file's PAD_NOPS macros elsewhere).
 *   & tools/wteng.ps1 main build
 *     -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH.
 *
 * THE CLOSING LEVER — REGION A, in one line:
 *   `D_80101EDA` is declared as an incomplete ARRAY (`extern s16 D_80101EDA[];`)
 *   and the six accesses are written as plain ARRAY REFERENCES with NO pointer
 *   local at all.
 *
 * WHY THAT WORKS (mechanism, and why every earlier spelling failed).
 * Sessions 1-5 established (H6) that `find_best_addr` (cse.c:2621) folds any
 * non-REG MEM address by substituting the base pseudo's `qty_const`
 * (cse.c:2659-2665 + 5170-5180), and restores register+displacement only if the
 * folded constant's equivalence class already holds a cheaper member.  A pointer
 * local initialised from `&D_80101EDA` gives the base pseudo a `qty_const` of
 * `(symbol_ref)`, so all three displaced sites folded to `lui %hi(sym+1100)` +
 * `%lo(...)` — the 3-instruction, 6-point residual.  An ARRAY_REF on an array of
 * UNKNOWN (incomplete) size never creates that pseudo: the base address is
 * materialised once and the displaced accesses stay `(mem (plus (reg)
 * (const_int 1100)))` through cse, so cc1 emits target's `la $s0,D_80101EDA` +
 * `lh/sh $r,1100($s0)` at all three sites and the count lands on target's 185.
 * A mini-TU sweep (tmp/grind/func_8003B9D0/s6/) shows the effect is specific to
 * the array DECLARATION: a pointer local off the same array, a pointer-to-array
 * cast, an array-of-struct index and a cast-to-array-of-unknown-size ALL still
 * fold.  Only the declared-array direct reference works.
 *
 * WHY THE DECLARATION IS HONEST (not a coercion):
 *   * `src/code6cac_c_ab.c` (func_8003B10C) already indexes this object with a
 *     RUNTIME stride of 1100 bytes.  A stride-1100 runtime index is array
 *     semantics; the scalar declaration was what forced that site to pointer-pun
 *     through the address of the scalar.
 *   * `D_80102326` is exactly the element one stride further along (1100 bytes),
 *     so the two symbols splat emitted are element 0 and element 1 of the same
 *     per-player field.
 *   * The same TU already declares many game globals this way
 *     (`extern u8 D_800A4750[];`, `extern s16 D_800A7FE0[];`, ...), and the
 *     matched, ruleless sibling `func_800617C8` (src/text1b.c:3836) uses the
 *     identical shape (`extern u8 D_800F1160[];` + `D_800F1160[0]` /
 *     `D_800F1160[1]`) and emits the same `la` + register+displacement form.
 *
 * SCOPE NOTE (why the declaration is TU-local here).  The tidier end state puts
 * the array declaration at the canonical `extern` in include/code6cac.h and
 * drops the local one; that whole-tree form was ALSO measured this session and
 * is byte-neutral everywhere (func_8003C040 160/160, func_8003CE18 91/91,
 * func_8003AFFC 68/68, func_8003B10C 64/64, full build == oracle SHA1).  But a
 * grind candidate may touch only its own .c file (tools/grinder/scope_allow.txt),
 * so the committed form declares the array in this TU.  cc1 accepts the
 * disagreement with the header with a non-fatal "conflicting types" diagnostic —
 * this file already produces eight of those for pre-existing local
 * redeclarations.  Aligning the header is an operator step; the precedent for
 * widening the scope is the `replay_camera_Init include/code6cac.h` entry.
 *
 * INTEGRATION NOTE (operator step, not a matching issue): regfix.txt:1116 still
 * carries `func_8003B9D0: fill_delay @ 49 <- 52`.  The cheat-invisible sandbox
 * drops that rule and still scores 0, so it is now dead weight; retiring it is
 * the operator's/driver's step (grind sessions may not edit regfix.txt).
 *
 * REGION B is unchanged from session 2: the two flag-selected argument
 * initialisations are spelled if/ELSE.  All three session-1 cheat-asm constructs
 * remain deleted.
 */

/* declaration (src/code6cac_c2.c:166):
 *     extern s16 D_80101EDA[];
 */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    u8 flags;

    magic = 0x80190800;
    func_8001DA2C();
    game_Cleanup();
    if (D_800A3768 != 0x14) gpu_InitDisplay();
    if (D_800A3768 != 0xFF) gpu_DisableDisplay();
    func_800174F4();
    gpu_EnableDisplay();
    func_80020D38();
    disp_SetFramebufferMode(1, 0, 0, 0);
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            saved_first = D_80101EDA[0];
            saved_44c = D_80101EDA[0x226];
            if (qf & 0x10) D_80101EDA[0] = 0x32;
            if (q[3] & 0x20) D_80101EDA[0x226] = 0x32;
            func_8003AFFC();
            D_80101EDA[0] = saved_first;
            D_80101EDA[0x226] = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA[0]; else a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326; else a0_arg = -1;
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, p[0], 0, a3_arg, a0_arg, -1, -1, magic);
    func_80041688(0, 0);
    func_80041688(1, 0);
    if (((u8 *)D_800A3878)[3] & 0x40) func_8004659C(-1);
    if (D_8010277D == 0xE || D_8010277D == 0x1D) {
        func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
    }
    func_8001DBE4();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FA8();
    v0 = func_80036EA8(5, ((u8 *)D_800A3878)[1]);
    func_80036FD4(v0, ((u8 *)D_800A3878)[2]);
    func_80037260();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_DisableDisplay();
}
