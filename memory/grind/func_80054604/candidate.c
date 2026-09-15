/* func_80054604 -- s3 (2026-09-15) LANDED form: sandbox 0/160, verify-oracle SHA1 match with
 * candidate_merge.patch applied + undefined_syms_auto.txt rows edited under scope grant
 * (tools/grinder/scope_allow.txt:76). Body identical to s2. Requires the Unk800EFAE8Ctrl
 * aggregate in include/game.h (see candidate_merge.patch). */
extern s32 D_800A3770;
extern const char D_80015840[];
extern s32 func_80044FA0(s32, s32);
extern s32 func_80045080(s32);
extern void snd_StopBgm(void);
extern s32 *snd_LoadSelection(s32);
extern s16 *stage_GetDataPtr(void);
extern s32 stage_GetId(void);
extern s32 func_8004153C(s32);
extern void func_8003FFC4(s32);
extern void game_SetPlayerCount(s32);
extern s32 disp_CalcFov(s32);
extern void SetGeomScreen(s32);
extern void gpu_EnableDisplay(void);
extern void game_StageCleanup(s32, s32);
extern void func_8004659C(s32);
s32 func_80054604(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6) {
    /* FAKE: second C handle to the global ctrl block (pointer-alias family);
       mechanism: expand/cse address materialisation -- the pointer local seats
       %hi/%lo(D_800EFAE8) in one callee-saved base register ($s1) for the whole
       body, whereas the direct D_800EFAE8.field form re-materialises the address
       per extended basic block; lever-exhaustion: direct-global form measured 82
       vs 26 (memory/grind/func_80054604/evidence.md s1,
       rejected/direct-global-no-pointer-local-82.c). */
    Unk800EFAE8Ctrl *s = &D_800EFAE8;
    s32 id = a0 + 0x131;
    s32 ret;
    s16 *t;
    s32 p;
    s32 v;
    s32 n;

    if (a6 != 0) {
        ret = func_80044FA0(id, a6);
        D_800EFAE8.unk2C = a6;
    } else {
        if (func_80045080(id) < 0) {
            snd_StopBgm();
            printf(D_80015840);
        }
        D_800EFAE8.unk2C = (s32)snd_LoadSelection(id);
        ret = 0;
    }
    p = s->unk2C;
    s->unk4 = *(s32 *)(*(s32 *)(p + 4) + p);
    p = s->unk2C;
    s->unk2 = *(u16 *)(*(s32 *)(p + 8) + p);
    s->unk0 = 0;
    t = stage_GetDataPtr();
    t += stage_GetId() * 24 + a1 * 6;
    s->unkC = *t++;
    s->unk10 = *t++;
    s->unk14 = *t++;
    s->unk1C = 0;
    s->unk20 = 0;
    s->unk44 = a2;
    s->unk46 = a3;
    s->unk48 = a4;
    s->unk4A = a5;
    s->unk1E = (((s->unk4 >> 8) & 0x7F) << 14) / 360;
    if (s->unk4 >= 0) {
        s->unk44 = -1;
    }
    if (!(s->unk4 & 0x40000000)) {
        s->unk46 = -1;
    }
    v = func_8004153C(0);
    if (v != 0) {
        func_8003FFC4(v);
    }
    v = func_8004153C(1);
    if (v != 0) {
        func_8003FFC4(v);
    }
    s->unk8 = a1;
    game_SetPlayerCount(0);
    SetGeomScreen(disp_CalcFov(0x2D));
    if (s->unk4 & 0x3F) {
        n = (s->unk4 & 0x3F) - 1;
        if (a6 != 0) {
            a6 += ret;
            game_StageCleanup(n, a6);
        } else {
            gpu_EnableDisplay();
            game_StageCleanup(n, (s32)&D_800A3770);
        }
    }
    if (s->unk4 & 0x8000) {
        func_8004659C(-1);
    }
    return ret;
}
