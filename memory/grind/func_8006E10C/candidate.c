/* func_8006E10C (text1b.c) — SANDBOX 0 form (s1, 2026-07-27). Pin-free.
 * The old 13-diff residual was ONE root cause: sched1 sank the dependence-free
 * `(set (reg ff0) (const_int 240))` from before the func_80036EA8 call to just
 * before its first use (4 calls later; the whole post-if region is one basic
 * block, so source order alone can never anchor it). The shrunk live range no
 * longer conflicted with the call-return pseudo, so ff0 stole $s0 and base was
 * pushed to $s1, cascading into all 13 diffs. The single-level sanctioned
 * do{}while(0) wrap (do-while-zero-exception, owner ruling 2026-07-06) emits
 * loop notes that fence the sink: li stays at the jal -> ff0 conflicts with
 * v0 -> ff0=$s1 (li reorg-filled into the jal delay slot), base shares $s0
 * with v0, base2=$s2. Verified: sandbox --disable all == 0, 103/103 insns,
 * li s1,240 in the jal delay slot. Dump evidence: tmp/grind/func_8006E10C/s1/. */
s32 func_8006E10C(void) {
    s32 ff0;
    s32 temp_s3 = D_800A3500;
    u8 rect[8];
    s32 v0;
    s32 a0v;
    s32 a1v;
    s32 base;
    s32 base2;

    __builtin_memcpy(rect, D_800A32D8, 8);
    if (((s32 *)D_800A3524)[8] & 1) {
        a0v = 2;
        a1v = 0x60;
    } else {
        a0v = 2;
        a1v = 7;
    }
    do { ff0 = 0xF0; } while (0); /* FAKE: loop notes fence sched1's constant-sink so the li stays at the jal */
    v0 = func_80036EA8(a0v, a1v);
    replay_camera_Init(v0, D_800A3500);
    game_FrameLoop();
    func_80036F28(v0);
    gpu_SetDispMask(0);
    base = (s32)&D_800F7438;
    gpu_InitDrawEnv(base, 0, 0, 0x280, ff0);
    gpu_InitDrawEnv(base + 0x4090, 0, ff0, 0x280, ff0);
    gpu_InitDispEnv(base + 0x5C, 0, ff0, 0x280, ff0);
    base2 = base + 0x40EC;
    gpu_InitDispEnv(base2, 0, 0, 0x280, ff0);
    D_800F74A4 = 0;
    D_800FB534 = 0;
    D_800F74A5 = 0;
    D_800FB535 = 0;
    gpu_DrawSync(0);
    func_8007B4D0((s32)rect, 0, 0, 0);
    gpu_DrawSync(0);
    gpu_LoadImage(rect, temp_s3 + 0x14);
    gpu_DrawSync(0);
    func_8007B9B0(base);
    func_8007BC08(base2);
    gpu_SetDispMask(1);
    return 1;
}
