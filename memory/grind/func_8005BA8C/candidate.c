/* func_8005BA8C - grind candidate, session 1 (recon), 2026-09-15.
 * Honest floor: sandbox --disable all = 67 (target 169 insns, build 168).
 * Every diff line vs target is a REGISTER-SEAT rename plus its two consequences:
 *   (a) global.c spills arg3 (pseudo 75) where the target spills the `base` copy
 *       (sw $s3,0x38($sp) in the bne delay slot; lw $t0,0x38($sp) at the return);
 *   (b) dst=hdr coalesces onto $s3 in the target (no move), here it is `move s2,s3`.
 * Frame (0x80), all three loop shapes, the i-final-value `addiu $sN,$zero,3`
 * (loop.c biv final value, consumed by cse2 for the count==3 compare), the u8
 * count andi pattern, struct-copy lw/sw pairs and the tail store all match.
 * Root of the seat cascade (ra_solver, model validated 29/29): pseudo 80 (i,
 * pri 10114) is allocated before pseudo 212 (loop-3 ent-pointer giv, pri 10000);
 * target order is the reverse (212->$s1, then i->$s2). See hypotheses.md H5.
 * No FAKE constructs. */
typedef struct {
    s32 off;
    s32 size;
} VabEnt;
typedef struct {
    VabEnt ent[3];
    s32 len[3];
} VabLoad;
extern u32 D_800EFC38[];
extern u32 D_800EFB38[];
extern u8 D_8009AD18[];
extern void func_800858D0(s32);
extern void SsVabClose(s32);
extern s32 func_80036EA8(s32, s32);
extern s32 game_FrameLoop(void);
extern void replay_camera_Init(s32, s32);
extern s32 func_80036F28(s32);
extern void func_80062020(s32);
extern s32 func_8005C2A8(s32, s32, s32);
extern void snd_VabFakeOpen(s32, s32);
extern s32 D_800EFC44;
extern s32 D_800EFC50;
s32 func_8005BA8C(s32 *hdr, s32 arg1, s32 arg2, s32 arg3) {
    VabLoad loc;
    s32 base;
    s32 task;
    s32 size;
    u8 count;
    s32 i;
    u32 j;
    u8 *dst;

    func_800858D0(0);
    for (i = 0; i < 3; i++) {
        SsVabClose(D_8009AD18[i]);
        D_800EFC38[D_8009AD18[i]] = 0;
        D_800EFB38[D_8009AD18[i]] = 0;
    }
    task = func_80036EA8(2, arg1 + 9);
    game_FrameLoop();
    replay_camera_Init(task, (s32)hdr);
    size = func_80036F28(task);
    game_FrameLoop();
    hdr[12] += (s32)hdr;
    func_80062020(hdr[12]);
    count = 3;
    base = (s32)hdr;
    if (arg2 == arg3) {
        count = 2;
    }
    loc.ent[0] = ((VabEnt *)hdr)[0];
    loc.ent[1] = ((VabEnt *)hdr)[arg2 + 1];
    if (count == 3) {
        loc.ent[2] = ((VabEnt *)hdr)[arg3 + 1];
    }
    for (i = 0; i < count; i++) {
        loc.ent[i].off += (s32)hdr;
        loc.len[i] = func_8005C2A8(loc.ent[i].off, D_8009AD18[i], (s32)hdr + size);
    }
    dst = (u8 *)hdr;
    for (i = 0; i < count; i++) {
        for (j = 0; j < (u32)loc.len[i]; j++) {
            dst[j] = ((u8 *)loc.ent[i].off)[j];
        }
        snd_VabFakeOpen((s32)dst - loc.ent[i].off, D_8009AD18[i]);
        loc.ent[i].off = (s32)dst;
        dst += loc.len[i];
    }
    if (count == 2) {
        D_800EFC50 = D_800EFC44;
    }
    return (s32)dst - base;
}
