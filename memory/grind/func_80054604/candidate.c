/* candidate.c -- func_80054604 (s2, 2026-09-15; re-proven s2 re-run 2 on HEAD c700d9136: sandbox 0/160 + oracle SHA1,
   full patch = memory/grind/func_80054604/candidate_merge.patch; INTEGRATION HANDOFF filed in docs/grind/decisions.md). sandbox --disable all = 0; full verify-oracle
   --rebuild SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa with this tree (include/game.h struct
   + src/text1b.c body + func_80054FDC/func_8005507C converted to the struct + src/text1b_b.c
   per-word externs removed). This file = the include/game.h declaration block followed by the
   src/text1b.c function block, exactly as integrated (tmp/grind/func_80054604/s2/integrate2.py).
   INTEGRATION HANDOFF (outside the grind surface, needs a tools/grinder/scope_allow.txt grant for
   undefined_syms_auto.txt, precedent rows func_80062020 / func_80033550): aggregate-merge prong (c)
   config half -- suffix the rows D_800EFB14/18/1C/20 with
   `/* alias of D_800EFAE8+0x2C..0x38; retire with func_8005490C */` (asm/funcs/func_8005490C.s is
   still INCLUDE_ASM and references them) and delete the rows D_800EFB0C/24/28 (no linked referrer:
   only the unlinked .s of the already-C func_8005507C / func_80054FDC name them).
   Constructs: pointer-alias local (FAKE-annotated at its declaration), compound-assignment split
   `a6 += ret;` (ordinary C, Ruling 4), aggregate merge (header-canonical). */

/* ---- include/game.h ---- */
/* Stage/match control block at 0x800EFAE8 (0x4C bytes). Object model evidence
 * (independent of and predating any byte-chasing): the original binary
 * addresses the whole block through ONE base register -- asm/funcs/func_80054604.s
 * forms $s1 = %hi/%lo(D_800EFAE8) once in its prologue and reaches offsets
 * 0x00/0x02/0x04/0x08/0x0C/0x10/0x14/0x1C/0x1E/0x20/0x2C/0x44/0x46/0x48/0x4A as
 * displacements off that single register (`lw $v1, 0x2C($s1)`, `sh $s5, 0x44($s1)`,
 * ...), and the still-asm per-frame handler asm/funcs/func_8005490C.s addresses
 * the same block the same way. The relocator func_80054FDC bumps the 0x2C..0x40
 * word group together by one base offset. Base+offset addressing of one object,
 * not symbol adjacency. Replaces the splat per-word scalars D_800EFAE8 /
 * D_800EFB0C / D_800EFB14 / D_800EFB18 / D_800EFB1C / D_800EFB20 / D_800EFB24 /
 * D_800EFB28. */
typedef struct {
    /* 0x00 */ s16 unk0;    /* phase (func_8005490C: -1 = done, 0 = init) */
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;    /* stage flags (bit31/bit30 tests, low 6 bits = cleanup index + 1) */
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
    /* 0x24 */ s32 unk24;   /* returned by address from func_8005507C */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;   /* loaded data base (census g_snd_data_buf_base); relocated by func_80054FDC */
    /* 0x30 */ s32 unk30;   /* relocated by func_80054FDC */
    /* 0x34 */ s32 unk34;   /* relocated by func_80054FDC when nonzero */
    /* 0x38 */ s32 unk38;   /* relocated by func_80054FDC when nonzero */
    /* 0x3C */ s32 unk3C;   /* relocated by func_80054FDC when nonzero */
    /* 0x40 */ s32 unk40;   /* relocated by func_80054FDC when nonzero */
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
    /* 0x48 */ s16 unk48;
    /* 0x4A */ s16 unk4A;
} Unk800EFAE8Ctrl;

extern Unk800EFAE8Ctrl D_800EFAE8;

/* ---- src/text1b.c ---- */
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
