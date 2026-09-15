/* func_8005BA8C - grind candidate, session 2 (structural), 2026-09-15.
 * sandbox --disable all = 0 (169/169 insns) on HEAD 35777ac64, -mel -msoft-float.
 * No FAKE constructs. Three structural changes on top of the s1 v4 chassis (67):
 *   1. The write cursor is a local `u8 *p` copied from the s32 param at entry and
 *      advanced in place (`p += loc.len[i]`); `base` is the saved copy read only
 *      at the return. Because p and base share the return insn as their last use,
 *      cse1's make_regs_eqv keeps p canonical, base stays a 2-ref pseudo and is
 *      the spill victim (sw $s3,0x38($sp)); the param's pseudo is block-local and
 *      local-alloc leaves it in $a0, so `move $s3,$a0` is the p=hdr copy insn,
 *      which sched2's LUID tie-break orders after the a1/a2/a3 copies and which
 *      lengthens arg1's live range by one insn (89 &D_800EFC38 -> $s4, 73 -> $s5).
 *   2. The three table entries are copied field by field, not by struct
 *      assignment (a struct assignment expands to the movstrsi block move whose
 *      output routine emits lw/lw/sw/sw; the target's lw/nop/sw pairs are the
 *      scheduler honouring the store->load memory dependence between scalars).
 *   3. Loop counter i shared by all three loops (s1), u8 count (s1). */
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
s32 func_8005BA8C(s32 hdr, s32 arg1, s32 arg2, s32 arg3) {
    VabLoad loc;
    u8 *p;
    s32 base;
    s32 task;
    s32 size;
    u8 count;
    s32 i;
    u32 j;

    p = (u8 *)hdr;
    func_800858D0(0);
    for (i = 0; i < 3; i++) {
        SsVabClose(D_8009AD18[i]);
        D_800EFC38[D_8009AD18[i]] = 0;
        D_800EFB38[D_8009AD18[i]] = 0;
    }
    task = func_80036EA8(2, arg1 + 9);
    game_FrameLoop();
    replay_camera_Init(task, (s32)p);
    size = func_80036F28(task);
    game_FrameLoop();
    ((s32 *)p)[12] += (s32)p;
    func_80062020(((s32 *)p)[12]);
    count = 3;
    base = (s32)p;
    if (arg2 == arg3) {
        count = 2;
    }
    loc.ent[0].off = ((VabEnt *)p)[0].off;
    loc.ent[0].size = ((VabEnt *)p)[0].size;
    loc.ent[1].off = ((VabEnt *)p)[arg2 + 1].off;
    loc.ent[1].size = ((VabEnt *)p)[arg2 + 1].size;
    if (count == 3) {
        loc.ent[2].off = ((VabEnt *)p)[arg3 + 1].off;
        loc.ent[2].size = ((VabEnt *)p)[arg3 + 1].size;
    }
    for (i = 0; i < count; i++) {
        loc.ent[i].off += (s32)p;
        loc.len[i] = func_8005C2A8(loc.ent[i].off, D_8009AD18[i], (s32)p + size);
    }
    for (i = 0; i < count; i++) {
        for (j = 0; j < (u32)loc.len[i]; j++) {
            p[j] = ((u8 *)loc.ent[i].off)[j];
        }
        snd_VabFakeOpen((s32)p - loc.ent[i].off, D_8009AD18[i]);
        loc.ent[i].off = (s32)p;
        p += loc.len[i];
    }
    if (count == 2) {
        D_800EFC50 = D_800EFC44;
    }
    return (s32)p - base;
}
