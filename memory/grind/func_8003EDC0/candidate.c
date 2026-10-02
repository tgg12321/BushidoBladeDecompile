typedef struct {
    s16 unk0;
    s16 unk2;
    u16 unk4;
    u8 unk6;
    u8 unk7;
    s32 unk8;
    s32 unkC;
} Unk800A4750Rec;
typedef struct {
    Unk80101DF0Record node;
    u8 unk58;
    u8 pad59[0xF];
} Unk800A6690Rec;
extern void (*g_anim_func_table[])(Unk80101DF0Rot *, Unk80101DF0Mat *);
extern s16 D_800A3678;
extern s32 D_800A3230;
extern void func_80052C10();
void func_8003EDC0(u16 *p, s32 arg1) {
    Unk800A4750Rec *r;
    Unk800A6690Rec *c;
    s32 i;
    s32 j;
    s16 n;
    s16 w;
    s16 x;
    s16 z;
    u16 tok;

    n = 0;
    while ((w = *p++) != -1) {
        r = &((Unk800A4750Rec *)D_800A4750)[n++];
        tok = *p++;
        r->unk4 = tok;
        if (tok & 0x8000) {
            r->unk4 = tok & 0x7FFF;
            r->unk7 = 8;
        } else {
            r->unk7 = 0;
        }
        r->unk0 = 0xC;
        r->unk6 = 0;
        r->unk2 = arg1;
        r->unk8 = (w % 32) * 2000 - 32000;
        r->unkC = (w / 32) * 2000 - 32000;
    }
    D_800A3368 = n;
    n = 0;
    while ((w = *p++) != -1) {
        c = &((Unk800A6690Rec *)D_800A6690)[n++];
        c->node.unk1 = 0;
        c->node.unkC = 0;
        c->node.unk8 = 0;
        c->node.unkA = 4;
        c->node.unk2 = w;
        c->node.unk4 = arg1;
        c->node.xf.mat.t[0] = *p++;
        c->node.xf.mat.t[0] |= *p++ << 16;
        c->node.xf.mat.t[1] = *p++;
        c->node.xf.mat.t[1] |= *p++ << 16;
        c->node.xf.mat.t[2] = *p++;
        c->node.xf.mat.t[2] |= *p++ << 16;
        c->node.xf.rot.vx = *p++;
        c->node.xf.rot.vy = *p++;
        c->node.xf.rot.vz = *p++;
        if (c->node.xf.rot.vz != (c->node.xf.rot.vx == c->node.xf.rot.vy)) {
            c->node.unk0 = 1;
        } else {
            c->node.unk0 = 0;
        }
        g_anim_func_table[c->node.unk8](&c->node.xf.rot, &c->node.xf.mat);
        c->unk58 = 0;
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            D_800A7FE0[i][j] = -1;
        }
    }
    i = 0;
    while ((w = *p++) != -1) {
        x = w % 32;
        z = w / 32;
        D_800A7FE0[z][x] = i;
        do {
            w = *p++;
            D_800A87E0[i++] = w;
        } while (!(w & 0x8000));
    }
    D_800A3230 = (D_800A3230 >= i) ? D_800A3230 : i;
    if (D_800A3230 >= 1000) {
        func_80052C10(D_800A3230);
    }
    D_800A3678 = 0;
    D_800A367A = 0;
    D_800A367C = 0;
}
