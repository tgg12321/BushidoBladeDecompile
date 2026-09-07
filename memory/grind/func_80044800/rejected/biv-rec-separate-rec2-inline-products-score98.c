void func_80044800(void) {
    s16 sv[4];
    s32 mat[8];
    Rec4473C *rec;
    Rec4473C *rec2;
    s32 i;
    s32 frame;
    s32 angle;
    s16 *scan;
    s32 cosv;
    s32 sinv;
    s32 fade;
    s32 *list;

    rec = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; i++, rec++) {
        frame = rec->unk58;
        if (frame < 0) continue;
        if (frame < D_800A9CF8.unk2) {
            sv[0] = 0;
            angle = rec->unk5C;
            sv[2] = 0;
            scan = (s16 *)(D_800A9CF8.unk8 + frame * 12);
            sv[1] = angle;
            func_8004A348(sv, mat);
            sv[0] = *scan++;
            sv[1] = *scan++;
            sv[2] = *scan++;
            rec->unk10 = *scan++;
            rec->unk12 = *scan++;
            rec->unk14 = *scan++;
            rec->unk6 = 0;
            func_800417D0((s32 *)rec);
            MulMatrix2(mat, (s32 *)rec->pad18);
            cosv = Judge[(angle + 0x400) & 0xFFF];
            sinv = Judge[angle & 0xFFF];
            rec->unk2C += (sinv * sv[2] + cosv * sv[0]) >> 12;
            rec->unk30 += sv[1];
            rec->unk34 += (cosv * sv[2] - sinv * sv[0]) >> 12;
            frame++;
            if (frame >= D_800A9CF8.unk2) {
                rec->unk60 = 0x1000;
            }
            rec->unk58 = frame;
            if (D_800A9CF8.unk4 == 0x12) {
                rec2 = (Rec4473C *)D_800A9CF8.unk10 + i;
                scan = (s16 *)(D_800A9CF8.unk8 + (frame + (D_800A9CF8.unk2 - 1)) * 12);
                sv[0] = *scan++;
                sv[1] = *scan++;
                sv[2] = *scan++;
                rec2->unk10 = *scan++;
                rec2->unk12 = *scan++;
                rec2->unk14 = *scan++;
                rec2->unk6 = 0;
                func_800417D0((s32 *)rec2);
                rec2->unk2C += sv[0];
                rec2->unk30 += sv[1];
                rec2->unk34 += sv[2];
            }
        } else {
            fade = rec->unk60 - 0x40;
            if (fade < 0) {
                fade = 0;
            }
            if (fade == 0 && D_800A9CF8.unk4 != 4) {
                rec->unk58 = -2;
            }
            rec->unk60 = fade;
        }
        list = (s32 *)D_800A3820;
        D_800A3820 = (s32)(list + 1);
        *list = (s32)rec;
    }
}
