/* REJECTED s2: on the v6 chassis (42) giving loop 1 its own counter k scores 46: the final-value
 * `addiu $s2,$zero,3` must be the same pseudo as the loop-2/3 counter for the seats to cascade. */
/* v8: v6 + loop 1 uses its own counter k */
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
s32 func_8005BA8C(u8 *hdr, s32 arg1, s32 arg2, s32 arg3) {
    VabLoad loc;
    s32 base;
    s32 task;
    s32 size;
    u8 count;
    s32 i;
    u32 j;
    s32 k;

    func_800858D0(0);
    for (k = 0; k < 3; k++) {
        SsVabClose(D_8009AD18[k]);
        D_800EFC38[D_8009AD18[k]] = 0;
        D_800EFB38[D_8009AD18[k]] = 0;
    }
    task = func_80036EA8(2, arg1 + 9);
    game_FrameLoop();
    replay_camera_Init(task, (s32)hdr);
    size = func_80036F28(task);
    game_FrameLoop();
    ((s32 *)hdr)[12] += (s32)hdr;
    func_80062020(((s32 *)hdr)[12]);
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
    for (i = 0; i < count; i++) {
        for (j = 0; j < (u32)loc.len[i]; j++) {
            hdr[j] = ((u8 *)loc.ent[i].off)[j];
        }
        snd_VabFakeOpen((s32)hdr - loc.ent[i].off, D_8009AD18[i]);
        loc.ent[i].off = (s32)hdr;
        hdr += loc.len[i];
    }
    if (count == 2) {
        D_800EFC50 = D_800EFC44;
    }
    return (s32)hdr - base;
}
