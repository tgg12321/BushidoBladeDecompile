/* REJECTED s1: `s32 count` with explicit (u8) casts at the ==3 / loop-1-bound / ==2 sites scores 96 vs
 * `u8 count` plain (95) on the same chassis (v2). u8 count reproduces the target's andi pattern exactly
 * (andi at ==3/!=0/loop-1 bound via one CSE'd zero-extend, raw reg at loop-3 beqz/slt, fresh andi at ==2). */
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
    u8 *p;
    u8 *q;
    s32 n;
    s32 task;
    s32 size;
    s32 count;
    s32 i;
    u32 j;
    u8 *dst;

    func_800858D0(0);
    p = D_8009AD18;
    q = (u8 *)((s32)p + 3);
    do {
        SsVabClose(*p);
        D_800EFC38[*p] = 0;
        D_800EFB38[*p] = 0;
        p++;
    } while ((s32)p < (s32)q);
    n = 3;
    task = func_80036EA8(2, arg1 + 9);
    game_FrameLoop();
    replay_camera_Init(task, (s32)hdr);
    size = func_80036F28(task);
    game_FrameLoop();
    hdr[12] += (s32)hdr;
    func_80062020(hdr[12]);
    count = n;
    if (arg2 == arg3) {
        count = 2;
    }
    loc.ent[0] = ((VabEnt *)hdr)[0];
    loc.ent[1] = ((VabEnt *)hdr)[arg2 + 1];
    if ((u8)count == n) {
        loc.ent[2] = ((VabEnt *)hdr)[arg3 + 1];
    }
    for (i = 0; i < (u8)count; i++) {
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
    if ((u8)count == 2) {
        D_800EFC50 = D_800EFC44;
    }
    return dst - (u8 *)hdr;
}
