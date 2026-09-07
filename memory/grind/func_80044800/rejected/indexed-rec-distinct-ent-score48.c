/* REJECTED (s1 recon re-run 2026-09-06, chassis 7d3c722f, no FAKE constructs): score 48 (202 insns).
 * Indexed form: `rec = (Rec4473C *)D_800A9CF8.unkC + i` at the top of every iteration plus a
 * distinct `ent` for the char-data record. `rec` becomes a giv of biv i: loop.c reduces
 * i*0x68 into $s5 and recomputes rec every iteration (lw v0,0(s4); addu s2,v0,s5 at the
 * loop head, unkC reloaded because stores through rec alias it), `.unk6` is read through
 * an in-register base instead of the target's lui/lh, and the target's `addiu s2,s2,0x68`
 * latch biv / `move s0,s2`..`move s2,s0` pair do not appear.
 * Diff: tmp/grind/func_80044800/s1/r2_pairdiff_varB.txt */
void func_80044800(void) {
    s16 sv[4];
    s32 mat[8];
    Rec4473C *rec;
    Rec4473C *ent;
    s32 i;
    s32 frame;
    s32 angle;
    s16 *scan;
    s16 cos_val;
    s16 sin_val;
    s32 cz;
    s32 sz;
    s32 cx;
    s32 sx;
    s32 last;
    s32 fade;
    s32 *list;

    for (i = 0; i < D_800A9CF8.unk6; i++) {
        rec = (Rec4473C *)D_800A9CF8.unkC + i;
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
            cos_val = Judge[(angle + 0x400) & 0xFFF];
            cz = cos_val * sv[2];
            sin_val = Judge[angle & 0xFFF];
            sz = sin_val * sv[2];
            cx = cos_val * sv[0];
            sx = sin_val * sv[0];
            rec->unk2C += (sz + cx) >> 12;
            rec->unk30 += sv[1];
            rec->unk34 += (cz - sx) >> 12;
            frame++;
            if (frame >= D_800A9CF8.unk2) {
                rec->unk60 = 0x1000;
            }
            rec->unk58 = frame;
            if (D_800A9CF8.unk4 == 0x12) {
                ent = (Rec4473C *)D_800A9CF8.unk10 + i;
                last = D_800A9CF8.unk2 - 1;
                scan = (s16 *)(D_800A9CF8.unk8 + (frame + last) * 12);
                sv[0] = *scan++;
                sv[1] = *scan++;
                sv[2] = *scan++;
                ent->unk10 = *scan++;
                ent->unk12 = *scan++;
                ent->unk14 = *scan++;
                ent->unk6 = 0;
                func_800417D0((s32 *)ent);
                ent->unk2C += sv[0];
                ent->unk30 += sv[1];
                ent->unk34 += sv[2];
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
