/* REJECTED (session 3) — sandbox --disable all == 11, build_insns 185.
 *
 * NOT a dead end in the usual sense: this is the FIRST form ever measured that
 * CLOSES REGION A.  objdump of tmp/sandbox/func_8003B9D0/code6cac_c2.o shows
 * exactly target's shape at all three displaced sites:
 *     lh  s1,1100(s2)      sh  v0,1100(s2)      sh  s1,1100(s2)
 * (vs the floor-6 candidate's `lui` + `%lo(D_80101EDA+1100)` pairs), and
 * build_insns is 185 == target_insns 185.  The normalized diff collapses to a
 * 128-insn byte-equal tail and only TWO clusters of divergence:
 *
 *   (1) MAGIC PLACEMENT (~7 of the 11 points).  Target materialises
 *       `magic = 0x80190800` in the PROLOGUE (tgt[1..3]: sw s3,44(sp) /
 *       lui s3,0x8019 / ori s3,s3,0x800), i.e. the original C initialises
 *       `magic` before everything and the 0x80 test only OVERWRITES it — a
 *       plain `if`.  This form has to spell that test as an `if/ELSE` (that
 *       else arm IS the cse-basic-block boundary that closes region A), which
 *       sinks the lui/ori into the else arm (mine[47..48]) and adds the `j`
 *       around it (mine[45]).
 *   (2) `la` PLACEMENT (~4 points).  Target materialises the base INSIDE the
 *       `qf & 0x30` block (tgt[55..56]); this form has to assign `eda` BEFORE
 *       the diamond (the boundary must sit between the pointer's definition
 *       and its displaced use), so the la lands at mine[38..39], where the
 *       scheduler drops it into the load-delay slot target fills with a nop.
 *
 * MECHANISM (measured, mini-TU sweep in tmp/grind/func_8003B9D0/s3/):
 *   cse's find_best_addr folds `(plus (reg) (const_int 1100))` into
 *   `(const (plus sym 1100))` whenever the base pseudo carries a qty_const.
 *   `cse_end_of_basic_block` resets the quantity tables at a CODE_LABEL it
 *   cannot extend past; an if/ELSE join label is exactly such a label (the
 *   arm ends in an unconditional jump + BARRIER), so a base defined BEFORE
 *   the diamond has NO qty_const at the uses AFTER it and the displacement
 *   survives.  A plain `if` is AROUND-extended (cse.c:8102-8184) and does not
 *   create the boundary — measured: the same form with the 0x80 test left as
 *   a plain `if` scores 10 with build_insns 187 (region A folds again).
 *
 * So region A is CLOSABLE; the open problem is finding a boundary that does
 * not cost the magic and la placements.  See hypotheses.md session-3 F1.
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
    s16 *eda;

    func_8001DA2C();
    game_Cleanup();
    if (D_800A3768 != 0x14) gpu_InitDisplay();
    if (D_800A3768 != 0xFF) gpu_DisableDisplay();
    func_800174F4();
    gpu_EnableDisplay();
    func_80020D38();
    disp_SetFramebufferMode(1, 0, 0, 0);
    eda = &D_80101EDA;
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    } else {
        magic = 0x80190800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            saved_first = eda[0];
            saved_44c = eda[0x226];
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            func_8003AFFC();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA; else a3_arg = -1;
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
