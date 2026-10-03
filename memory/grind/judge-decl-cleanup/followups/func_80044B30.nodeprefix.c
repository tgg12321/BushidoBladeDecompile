typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;
    s32 unk5C;
    s16 unk60;
    s8 pad62[6];
} Rec4473CN;
void func_80044B30(s32 a0, s32 a1) {
    Rec4473CN *p;
    s16 stage;

    if (a0 >= D_800A9CF8.unk6) return;

    p = (Rec4473CN *)D_800A9CF8.unkC + a0;
    if (p->unk58 != -1) return;

    stage = D_800A9CF8.unk4;
    if (stage == 4) goto case4;
    if (stage == 0x12) goto set_zero;
    goto do_store;

case4:
    if (a0 == 0) {
        a1 = 0x800;
    }
    if (a0 != 1) {
        goto do_store;
    }
set_zero:
    a1 = 0;

do_store:
    p->unk5C = a1;
    p->unk58 = 0;
    /* FAKE: p reused for the D_800A9CF8.unk10 entry; a separate local scores 4. */
    p = (Rec4473CN *)D_800A9CF8.unk10 + a0;
    p->node.unk2 = 1;
    {
        u16 cf8 = D_800A9CF8.unk0;
        s32 idx = p->node.unk8;
        p->node.xf.rot.vy = (s16)a1;
        p->node.unk0 = 0;
        p->node.unkC = 0;
        p->node.xf.rot.vx = 0;
        p->node.xf.rot.vz = 0;
        p->node.unk6 = 1;
        p->node.unk4 = cf8;
        g_anim_func_table[idx](&p->node.xf.rot, &p->node.xf.mat);
    }
    if (D_800A9CF8.unk4 == 0x12) {
        p->node.work.t[0] = p->node.xf.mat.t[0];
        p->node.work.t[1] = p->node.xf.mat.t[1];
        p->node.work.t[2] = p->node.xf.mat.t[2];
    }
}
