extern s32 D_800A3230;
extern void func_80052C10();
extern s32 D_800F66A0[];
extern s16 D_800A3678;
typedef struct {
    s16 kind;
    s16 arg;
    u16 id;
    u8 b6;
    u8 flags;
    s32 x;
    s32 z;
} C2Rec16;
typedef struct {
    s8 unk0;
    s8 unk1;
    s16 unk2;
    u16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    s16 unk18[10];
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s8 pad38[0x20];
    u8 unk58;
    s8 pad59[0xF];
} C2Rec68;
typedef void (*C2RecFn)(s16 *, s16 *);

void func_8003EDC0(u16 *p, s32 arg1) {
    s16 pos;
    s16 n;
    s16 m;
    s16 x;
    s16 z;
    u16 tok;
    s32 i;
    s32 j;
    C2Rec16 *r;
    C2Rec68 *c;

    n = 0;
    while ((pos = *p++) != -1) {
        r = &((C2Rec16 *)D_800A4750)[n++];
        tok = *p++;
        r->id = tok;
        if (tok & 0x8000) {
            r->id = tok & 0x7FFF;
            r->flags = 8;
        } else {
            r->flags = 0;
        }
        r->kind = 0xC;
        r->b6 = 0;
        r->arg = arg1;
        r->x = (pos % 32) * 2000 - 32000;
        r->z = (pos / 32) * 2000 - 32000;
    }
    D_800A3368 = n;
    m = 0;
    while ((pos = *p++) != -1) {
        c = &((C2Rec68 *)D_800A6690)[m++];
        c->unk1 = 0;
        c->unkC = 0;
        c->unk8 = 0;
        c->unkA = 4;
        c->unk2 = pos;
        c->unk4 = arg1;
        c->unk2C = *p++;
        c->unk2C |= *p++ << 16;
        c->unk30 = *p++;
        c->unk30 |= *p++ << 16;
        c->unk34 = *p++;
        c->unk34 |= *p++ << 16;
        c->unk10 = *p++;
        c->unk12 = *p++;
        c->unk14 = *p++;
        if (c->unk14 != (c->unk10 == c->unk12)) {
            c->unk0 = 1;
        } else {
            c->unk0 = 0;
        }
        ((C2RecFn)D_800F66A0[c->unk8])(&c->unk10, c->unk18);
        c->unk58 = 0;
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            D_800A7FE0[i][j] = -1;
        }
    }
    i = 0;
    while ((pos = *p++) != -1) {
        x = pos % 32;
        z = pos / 32;
        D_800A7FE0[z][x] = i;
        do {
            pos = *p++;
            D_800A87E0[i++] = pos;
        } while (!(pos & 0x8000));
    }
    D_800A3230 = (D_800A3230 >= i) ? D_800A3230 : i;
    if (D_800A3230 >= 1000) {
        func_80052C10(D_800A3230);
    }
    D_800A3678 = 0;
    D_800A367A = 0;
    D_800A367C = 0;
}
