/* REJECTED (s2, 2026-09-15): mechanism probe, not a submission: `a6 = 0;` in the else arm survives cse/flow as a second set of the buffer pseudo (reg_n_sets=2), which undoubles its REG_EQUIV live length AND makes its prologue lw non-birthing in sched1 -> full target allocation (s2=a6 s3=a1 s4=ret s7=a4 fp=a5) and prologue lw order; score 4 = the extra `move s2,zero` it emits. Semantically a no-op store (a6 already 0 there): T2 fail. */
/* candidate.c — func_80054604 (s1 recon, 2026-09-15). Honest floor 26 (sandbox --disable all).
   Residual = callee-saved register permutation only (target s2=a6 s3=a1 s4=ret s7=a4 fp=a5;
   this form gives s2=a1 s3=ret s4=a6 s7=a5 fp=a4) plus the prologue lw order that follows it.
   Splice over INCLUDE_ASM("asm/funcs", func_80054604) and delete the extern prototype used by
   func_80054884 (see tmp/grind/func_80054604/s1/splice.py). StageCtrl typedef is TU-local here
   for measurement only; the final form must move it to a shared header (aggregate-merge prong d).
   NOTE the pointer local `StageCtrl *s = &D_800EFAE8` is the pointer-alias family (FAKE-annotation
   required at submission; lever-exhaustion: direct-global form measured 82, rejected/direct-global.c). */
typedef struct StageCtrl {
    /* 0x00 */ s16 phase;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 flags;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ u8 *data;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
    /* 0x48 */ s16 unk48;
    /* 0x4A */ s16 unk4A;
} StageCtrl;
extern StageCtrl D_800EFAE8;
extern u8 *D_800EFB14;
extern s32 D_800A3770;
extern const char D_80015840[];
extern s32 func_80044FA0(s32, s32);
extern s32 func_80045080(s32);
extern void snd_StopBgm(void);
extern u8 *snd_LoadSelection(s32);
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
    StageCtrl *s = &D_800EFAE8;
    s32 id = a0 + 0x131;
    s32 ret;
    s16 *t;
    u8 *p;
    s32 v;
    s32 n;

    if (a6 != 0) {
        ret = func_80044FA0(id, a6);
        D_800EFB14 = (u8 *)a6;
    } else {
        if (func_80045080(id) < 0) {
            snd_StopBgm();
            printf(D_80015840);
        }
        D_800EFB14 = snd_LoadSelection(id);
        ret = 0;
        a6 = 0;
    }
    p = s->data;
    s->flags = *(s32 *)(p + *(s32 *)(p + 4));
    p = s->data;
    s->unk2 = *(u16 *)(p + *(s32 *)(p + 8));
    s->phase = 0;
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
    s->unk1E = (((s->flags >> 8) & 0x7F) << 14) / 360;
    if (s->flags < 0) {
        s->unk44 = -1;
    }
    if (!(s->flags & 0x40000000)) {
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
    if (s->flags & 0x3F) {
        n = (s->flags & 0x3F) - 1;
        if (a6 != 0) {
            game_StageCleanup(n, a6 + ret);
        } else {
            gpu_EnableDisplay();
            game_StageCleanup(n, (s32)&D_800A3770);
        }
    }
    if (s->flags & 0x8000) {
        func_8004659C(-1);
    }
    return ret;
}
