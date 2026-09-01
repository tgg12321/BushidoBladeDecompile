/* s10 H1: full struct-typed rederive of the D_800A36A0 block. */
typedef struct GmS {
/* 0x00 */ s32  unk00;
/* 0x04 */ s32 *prev;
/* 0x08 */ s16  unk08[2];
/* 0x0C */ s16  unk0C[2];
/* 0x10 */ s16  unk10[2];
/* 0x14 */ s16  unk14[2];
/* 0x18 */ s32  unk18;
/* 0x1C */ s32  unk1C;
/* 0x20 */ s32  unk20;
/* 0x24 */ s32  unk24[3];
/* 0x30 */ s32  unk30;
/* 0x34 */ s16  unk34;
/* 0x36 */ s16  unk36;
/* 0x38 */ s32  unk38;
/* 0x3C */ s16  unk3C[2];
/* 0x40 */ s16  unk40[2][2];
/* 0x48 */ s32  unk48[5];
/* 0x5C */ s16  unk5C[2];
/* 0x60 */ s16  unk60[2];
/* 0x64 */ u8   unk64;
/* 0x65 */ u8   unk65;
/* 0x66 */ u8   unk66;
/* 0x67 */ u8   unk67;
/* 0x68 */ u8   unk68[2];
/* 0x6A */ s16  unk6A[2][5];
/* 0x7E */ s16  unk7E[2][5];
} GmS;

s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    p_old = (s32 *)(arg0 + 0x58);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(D_800A374C, 0x1008);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        ((GmS *)p_old)->prev = prev;
        ((GmS *)p_old)->unk30 = 0;
        ((GmS *)p_old)->unk34 = 0;
    }
    t0 = 0;
    do {
        GmS *g = (GmS *)D_800A36A0;
        a2 = 0;
        g->unk10[t0] = 0;
        g->unk08[t0] = 0;
        g->unk0C[t0] = 0;
        g->unk14[t0] = 0;
        g->unk3C[t0] = 0;
        (&D_800A35D0)[t0 * 2 + 1] = 0;
        (&D_800A35D0)[t0 * 2] = 0;
        g->unk40[t0][1] = 0;
        g->unk40[t0][0] = 0;
        g->unk68[t0] = (u8)t0;
        {
            s16 *p_6a = ((GmS *)D_800A36A0)->unk6A[t0];
            s16 *p_7e = ((GmS *)D_800A36A0)->unk7E[t0];
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        ((GmS *)D_800A36A0)->unk5C[t0] = 0;
        ((GmS *)D_800A36A0)->unk60[t0] = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        GmS *g = (GmS *)D_800A36A0;
        g->unk20 = 0;
        g->unk1C = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            g->unk64 = (u8)((s16)sp[0] - 3);
        } else {
            g->unk64 = (u8)((s16)sp[1] - 3);
        }
    }
    if (((GmS *)D_800A36A0)->unk64 >= 3) {
        ((GmS *)D_800A36A0)->unk64 = 2;
    }
    {
        GmS *g = (GmS *)D_800A36A0;
        g->unk00 = arg1;
        g->unk65 = 0;
    }
    ((GmS *)D_800A36A0)->unk67 = 1;
    ((GmS *)D_800A36A0)->unk66 = (&D_8009BD21)[((GmS *)D_800A36A0)->unk67 * 2];
    D_800A35DC = 1;
    return 1;
}
