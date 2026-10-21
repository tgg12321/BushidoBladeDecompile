/* 99 game functions. .text 0x80060A68 (ROM 0x51268). Start boundary: PHASE
 * (rodata-align section 9), moved by the per-file gp model (Q65). Ends at
 * eff_LinkParticleQuads: the menu controller after it is 596CC.c (seam, owner
 * ruling 2026-10-17). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>
#include <psxsdk/gtemac.h>

/* eff_StartCommand .. eff_BindScratchpad belong here by the per-file gp
 * evidence (Q65). */

/* Q65: this file's statics (.sbss, allocated per file in link order by
 * PSYLINK), in address order. */
static POLY_FT4 *D_800A3420;
static POLY_FT4 *D_800A3424;
/* not named by any code or data: size from the gap */
static s32 D_800A3428[2];
/* not named by any code or data: size from the gap */
static s32 D_800A3430[2];
static s16 D_800A3438[4];
static s16 D_800A3440;
static s32 D_800A3444;
static s32 D_800A3448;
static u32 D_800A344C[2];
static s32 D_800A3454[2];
static s16 D_800A345C[2];
static s32 D_800A3460;
static s32 D_800A3464;
static Unk1F800000Unk00 *D_800A3468;
static s16 *D_800A346C;
static s32 *D_800A3470;
static MATRIX *D_800A3474;
static s16 *D_800A3478;
static s32 *D_800A347C;
static s32 *D_800A3480;
static s32 *D_800A3484;
static TexRec *D_800A3488;
static TexRec *D_800A348C;
static s32 *D_800A3490;
static s32 *D_800A3494;
static u16 *D_800A3498;
static u16 *D_800A349C;
static u16 *D_800A34A0;
static u16 *D_800A34A4;
static s16 *D_800A34A8;
static s16 *D_800A34AC;
static s32 *D_800A34B0;
static s32 *D_800A34B4;
static s32 *D_800A34B8;
static s16 *D_800A34BC;
static s16 *D_800A34C0;
static s32 *D_800A34C4;
static s32 *D_800A34C8;
static s32 *D_800A34CC;
static s32 *D_800A34D0;
static u16 *D_800A34D4;
static u16 *D_800A34D8;
static u16 *D_800A34DC;
static u16 *D_800A34E0;
static u32 *D_800A34E4;
static u32 *D_800A34E8;
static Unk1F8000B8Union *D_800A34EC;
/* Two s16 slots at 0x800A34F0, indexed as one array (eff_PrepareParticles and
 * eff_UpdateParticles read the pair through one indexed access). */
static s16 D_800A34F0[2];
static s32 D_800A34F4; /* not named by any code or data: size from the gap */

/* Runs the command in the block D_800A3468 points at (idx = the block's low
 * halfword, unk00.h): copies the three words at unk0C and the three halfwords
 * at unk10 into unk20 / unk18, points D_800A347C / D_800A3478 at the copies,
 * clears g_eff_command_phase[idx], calls the D_8009BA7C entry
 * D_8009BA60[idx] + g_eff_command_phase[idx], stores the result through unk14,
 * and sets D_800A32BC to 0xA when bit 21 of unk00.w is set. Member reads (not
 * *(u16 *)D_800A3468) let the second idx read (the D_8009BA60 index) rise
 * above the D_800A347C store, as in the target. */
void eff_StartCommand(void) {
    extern s32 D_800A32BC;

    s32 result;

    g_eff_command_phase[D_800A3468->unk00.h] = 0;
    D_800A3468->unk20[0] = D_800A3468->unk0C[0];
    D_800A3468->unk20[1] = D_800A3468->unk0C[1];
    D_800A3468->unk20[2] = D_800A3468->unk0C[2];
    D_800A3468->unk18[0] = D_800A3468->unk10[0];
    D_800A3468->unk18[1] = D_800A3468->unk10[1];
    D_800A3478 = D_800A3468->unk18;
    D_800A3468->unk18[2] = D_800A3468->unk10[2];
    D_800A347C = D_800A3468->unk20;

    result =
        ((s32(*)(void))D_8009BA7C[D_8009BA60[D_800A3468->unk00.h] +
                                  g_eff_command_phase[D_800A3468->unk00.h]])();
    *D_800A3468->unk14 = result;

    if (D_800A3468->unk00.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}

void eff_StepCommand(void) {

    Unk1F800000Unk00 *var_0;
    s16 *dst16;
    s32 *dst_s32;
    u16 idx;
    s32 result;

    var_0 = D_800A3468;
    dst16 = D_800A346C;
    dst16[0] = var_0->unk04[0];
    dst16[1] = var_0->unk04[1];
    dst16[2] = var_0->unk04[2];

    dst_s32 = D_800A3470;
    dst_s32[0] = var_0->unk08[0];
    dst_s32[1] = var_0->unk08[1];
    dst_s32[2] = var_0->unk08[2];
    eff_SetRotMatrix(dst16, dst_s32, D_800A3474);

    idx = D_800A3468->unk00.h;
    result = ((
        s32(*)(void))D_8009BA7C[D_8009BA60[idx] + g_eff_command_phase[idx]])();

    *D_800A3468->unk14 = result;
}

extern u8 g_eff_command_status[0x1C];

void eff_ResetCommands(void) {
    s32 i = 0;
    s32 *p = g_eff_command_phase;
    do {
        *p = 0;
        g_eff_command_status[i] = 0;
        i++;
        p++;
    } while (i < 0x1C);
    D_800A345C[1] = 0;
    D_800A345C[0] = 0;
    D_800A3454[1] = 0;
    D_800A3454[0] = 0;
    D_800A344C[1] = 0;
    D_800A344C[0] = 0;
    D_800A3460 = 0;
    D_800A3444 = 0;
    D_800A3448 = 0;
}

s32 eff_LoadImagesAndSetup(s32 arg0, s32 arg1) {
    /* FAKE: single forward-order param alias; flips the s2/s1 save+copy pair
     * order to the target's (param-local-alias-prologue-pair-flip); arg0
     * throughout: score 4 */
    unsigned int new_var;
    RECT s;
    s32 v;
    s32 ret;
    new_var = arg0;
    cdrom_WaitIdle();
    v = D_8009BD24.unk14_0;
    if (v == 0) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x3C), arg0);
    } else if (v == 3) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x2F), new_var);
    } else if (v == 2) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x30), new_var);
    } else if (v == 5) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x31), new_var);
    } else {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0), new_var);
    }
    cdrom_WaitIdle();
    s.x = 0x380;
    s.y = 0;
    s.w = 0x80;
    s.h = 0x1DC;
    DrawSync(0);
    LoadImage(&s, (u32 *)new_var);
    DrawSync(0);
    s.w = 0x70;
    s.y = 0x1DC;
    s.h = 0x24;
    LoadImage(&s, (u32 *)(new_var + 0x1DC00));
    DrawSync(0);
    eff_ResetCommands();
    srand(rand());
    ret = arg1 + 0x4650;
    D_800A3420 = (POLY_FT4 *)arg1;
    D_800A3424 = (POLY_FT4 *)ret;
    return ret + 0x4650;
}

extern POLY_FT4 *g_eff_quad_buffer_base;

void eff_SelectQuadBuffer(s32 arg0) {
    g_eff_quad_cursor = arg0 != 0 ? D_800A3424 : D_800A3420;
    g_eff_quad_buffer_base = g_eff_quad_cursor;
}

#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)

void eff_BindScratchpad(s16 *arg0, s32 *arg1) {
    D_800A3468 = &SPAD51268->unk00;
    D_800A346C = SPAD51268->unk00.unk18;
    D_800A3470 = SPAD51268->unk00.unk20;
    D_800A3474 = &SPAD51268->unk30;
    D_800A3488 = &SPAD51268->unk50;
    D_800A3490 = &SPAD51268->unk58;
    D_800A3494 = &SPAD51268->unk5C;
    D_800A3498 = &SPAD51268->unk60;
    D_800A349C = &SPAD51268->unk62;
    D_800A34A0 = &SPAD51268->unk64;
    D_800A34A4 = &SPAD51268->unk66;
    D_800A34A8 = &SPAD51268->unk68;
    D_800A34AC = &SPAD51268->unk6A;
    D_800A34B0 = &SPAD51268->unk6C;
    D_800A34B4 = &SPAD51268->unk70;
    D_800A34B8 = SPAD51268->unk74;
    D_800A34BC = &SPAD51268->unk80;
    D_800A34C0 = &SPAD51268->unk82;
    D_800A34C4 = &SPAD51268->unk84;
    D_800A34C8 = &SPAD51268->unk88;
    D_800A34CC = &SPAD51268->unk8C;
    D_800A34D0 = SPAD51268->unk90;
    D_800A34D4 = &SPAD51268->unk98;
    D_800A34D8 = &SPAD51268->unk9A;
    D_800A34DC = &SPAD51268->unk9C;
    D_800A34E0 = &SPAD51268->unk9E;
    D_800A34E4 = &SPAD51268->unkA0;
    D_800A34E8 = &SPAD51268->unkA4;
    D_800A3480 = &SPAD51268->unkA8;
    D_800A3484 = &SPAD51268->unkAC;
    D_800A348C = &SPAD51268->unkB0;
    D_800A34EC = &SPAD51268->unkB8;
    SPAD51268->unk00.unk04 = arg0;
    SPAD51268->unk00.unk08 = arg1;
}

extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
extern void AddPrim(void *, void *);
void eff_StartCommand(void);
void eff_StepCommand(void);
void eff_ResetCommands(void);
void eff_BindScratchpad(s16 *arg0, s32 *arg1);

/* the message, alignment and an empty trailing string */
const char D_800158E0[24] = "eff prim over :%d \n";

void func_80061064(s16 *a0, s32 *a1) {
    s32 temp_a1;
    s32 i;
    eff_BindScratchpad(a0, a1);
    i = 0;
    do {
        D_800A3468->unk14 = &g_eff_command_status[i];
        if (g_eff_command_status[i] != 0) {
            D_800A3468->unk00.w = i;
            eff_StepCommand();
        }
        i += 1;
    } while (i < 0x1C);
    if (D_800A32BC >= 2) {
        D_800A32BC -= 1;
        light_SetPointLight(&D_800F1140, D_800A3464);
    } else if (D_800A32BC == 1) {
        light_DisablePointLight();
        D_800A32BC = 0;
    }
    temp_a1 = g_eff_quad_cursor - g_eff_quad_buffer_base;
    if (temp_a1 >= 0x1C2) {
        printf(&D_800158E0, temp_a1 - 0x1C2);
    }
}

void eff_StopCommands(void) {
    eff_ResetCommands();
    light_DisablePointLight();
    D_800A32BC = 0;
}

extern Unk1F800000Unk00 g_eff_command_block;

void func_800611A4(s32 *arg0, s16 *arg1) {
    s16 svec[3];
    s32 *p;
    s16 *q = arg1;
    svec[0] = *q++;
    svec[1] = *q++;
    D_800A3468 = &g_eff_command_block;
    svec[2] = *q;
    g_eff_command_block.unk10 = svec;
    g_eff_command_block.unk0C = arg0;
    g_eff_command_block.unk14 = &g_eff_command_status[26];
    g_eff_command_block.unk00.w = 0x21001A;
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFFFFEF;
}

void func_80061250(s32 *arg0) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    if (g_eff_command_status[9] != 0 && g_eff_command_status[10] != 0) {
        g_eff_command_status[10] = 0;
        g_eff_command_status[9] = 0;
    }
    if (g_eff_command_status[9] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[9];
        D_800A3468->unk00.w = 0x210009;
    } else if (g_eff_command_status[10] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[10];
        g_eff_command_block.unk00.w = 0x21000A;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0060;
}

s32 func_8006133C(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[4];
    g_eff_command_block.unk00.w = 0x210004;
    eff_StartCommand();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 1;
}

s32 func_800613C8(s32 *a0) {
    s32 *var_0 = a0;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x21000B;
    eff_StartCommand();
    D_800F1140.vx = *var_0++;
    D_800F1140.vy = *var_0++;
    D_800F1140.vz = *var_0++;
    D_800A3464 = 0x8080FF;
    return 16;
}

s32 func_80061454(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x29000B;
    eff_StartCommand();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 8;
}

s32 func_800614E0(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x31000B;
    eff_StartCommand();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 5;
}

void func_8006156C(s32 *arg0) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    if (g_eff_command_status[5] != 0 && g_eff_command_status[6] != 0) {
        g_eff_command_status[6] = 0;
        g_eff_command_status[5] = 0;
    }
    if (g_eff_command_status[5] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[5];
        D_800A3468->unk00.w = 0x210005;
    } else if (g_eff_command_status[6] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[6];
        g_eff_command_block.unk00.w = 0x210006;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF8080;
}

void func_80061658(s32 *arg0, s32 arg1) {
    s32 *p;

    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    switch (arg1) {
    case 0:
        g_eff_command_status[12] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[12];
        g_eff_command_block.unk00.w = 0x21000C;
        break;
    case 1:
        g_eff_command_status[13] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[13];
        g_eff_command_block.unk00.w = 0x21000D;
        break;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x10FFFF;
}

void func_80061710(s32 *arg0, s32 arg1) {
    s32 *p;

    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    switch (arg1) {
    case 0:
        g_eff_command_status[14] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[14];
        g_eff_command_block.unk00.w = 0x21000E;
        break;
    case 1:
        g_eff_command_status[15] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[15];
        g_eff_command_block.unk00.w = 0x21000F;
        break;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x10FF10;
}

void func_800617C8(s32 *arg0) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    if (g_eff_command_status[16] != 0 && g_eff_command_status[17] != 0) {
        g_eff_command_status[17] = 0;
        g_eff_command_status[16] = 0;
    }
    if (g_eff_command_status[16] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[16];
        D_800A3468->unk00.w = 0x210010;
    } else if (g_eff_command_status[17] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[17];
        g_eff_command_block.unk00.w = 0x210011;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xC06013;
}

void func_800618B4(s32 *arg0, s16 *arg1) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    g_eff_command_block.unk10 = arg1;
    if (g_eff_command_status[2] != 0 && g_eff_command_status[3] != 0) {
        g_eff_command_status[3] = 0;
        g_eff_command_status[2] = 0;
    }
    if (g_eff_command_status[2] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[2];
        D_800A3468->unk00.w = 0x210002;
    } else if (g_eff_command_status[3] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[3];
        g_eff_command_block.unk00.w = 0x210003;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0000;
}

void func_800619A4(s32 *a0) {
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[8];
    g_eff_command_block.unk00.w = 0x10008;
    eff_StartCommand();
}

void func_800619F0(s32 *a0) {
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk14 = &g_eff_command_status[7];
    g_eff_command_block.unk00.w = 0x10007;
    eff_StartCommand();
}

void func_80061A3C(s32 *a0, s16 a1, s32 a2, s32 a3) {
    s16 sp[4];

    D_800A3468 = &g_eff_command_block;
    sp[2] = 0;
    sp[0] = 0;
    sp[1] = a1;
    g_eff_command_block.unk0C = a0;
    g_eff_command_block.unk10 = sp;
    if (a3 == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[0];
        g_eff_command_block.unk00.w = a2 + 0x10000;
    } else {
        g_eff_command_block.unk14 = &g_eff_command_status[1];
        g_eff_command_block.unk00.w = a2 + 0x10001;
    }
    eff_StartCommand();
}

void func_80061ACC(s32 *arg0, s16 *arg1) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    g_eff_command_block.unk10 = arg1;
    if (g_eff_command_status[20] != 0 && g_eff_command_status[21] != 0) {
        g_eff_command_status[21] = 0;
        g_eff_command_status[20] = 0;
    }
    if (g_eff_command_status[20] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[20];
        D_800A3468->unk00.w = 0x210014;
        eff_StartCommand();
        D_800A3468->unk14 = &g_eff_command_status[7];
        D_800A3468->unk00.w = 0x10007;
    } else if (g_eff_command_status[21] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[21];
        D_800A3468->unk00.w = 0x210015;
        eff_StartCommand();
        D_800A3468->unk14 = &g_eff_command_status[7];
        D_800A3468->unk00.w = 0x10007;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF8080;
}

void func_80061C00(s32 *arg0, s32 arg1, s32 arg2) {
    SVECTOR sp10;
    SVECTOR sp18;
    VECTOR sp20;
    MATRIX sp30;
    s32 sp50;

    D_800A3468 = &g_eff_command_block;
    if (arg2 != 1) {
        arg2 = 0;
    }
    sp10.vy = -0xA00;
    sp10.vx = 0;
    sp10.vz = 0xA00;
    sp20.vz = 0;
    sp20.vy = 0;
    sp20.vx = 0;
    sp18.vy = arg1;
    sp18.vz = 0;
    sp18.vx = 0;
    RotMatrix(&sp18, &sp30);
    sp30.t[2] = 0;
    sp30.t[1] = 0;
    sp30.t[0] = 0;
    SetRotMatrix(&sp30);
    SetTransMatrix(&sp30);
    RotTrans(&sp10, &sp20, &sp50);
    sp10.vx = sp20.vx;
    sp10.vy = sp20.vy;
    sp10.vz = sp20.vz;
    D_800A3468->unk0C = arg0;
    D_800A3468->unk10 = &sp10.vx;
    if (g_eff_command_status[22] != 0 && g_eff_command_status[23] != 0) {
        g_eff_command_status[23] = 0;
        g_eff_command_status[22] = 0;
    }
    if (g_eff_command_status[22] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[22];
        D_800A3468->unk00.w = 0x10016;
        D_800A34F0[0] = arg2;
    } else if (g_eff_command_status[23] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[23];
        D_800A3468->unk00.w = 0x10017;
        D_800A34F0[1] = arg2;
    }
    eff_StartCommand();
}

void func_80061D74(s32 *arg0, s16 arg1) {
    SVECTOR sp10;
    SVECTOR sp18;
    VECTOR sp20;
    MATRIX sp30;
    s32 sp50;

    D_800A3468 = &g_eff_command_block;
    sp10.vy = -0xA00;
    sp10.vx = 0;
    sp10.vz = 0xA00;
    sp20.vz = 0;
    sp20.vy = 0;
    sp20.vx = 0;
    sp18.vz = 0;
    sp18.vy = arg1;
    sp18.vx = 0;
    RotMatrix(&sp18, &sp30);
    sp30.t[2] = 0;
    sp30.t[1] = 0;
    sp30.t[0] = 0;
    SetRotMatrix(&sp30);
    SetTransMatrix(&sp30);
    RotTrans(&sp10, &sp20, &sp50);
    sp10.vx = sp20.vx;
    sp10.vy = sp20.vy;
    sp10.vz = sp20.vz;
    D_800A3468->unk0C = arg0;
    D_800A3468->unk10 = &sp10.vx;
    if (g_eff_command_status[24] != 0 && g_eff_command_status[25] != 0) {
        g_eff_command_status[25] = 0;
        g_eff_command_status[24] = 0;
    }
    if (g_eff_command_status[24] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[24];
        D_800A3468->unk00.w = 0x10018;
    } else if (g_eff_command_status[25] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[25];
        D_800A3468->unk00.w = 0x10019;
    }
    eff_StartCommand();
}

void func_80061EC0(s32 *arg0) {
    s32 *p;
    D_800A3468 = &g_eff_command_block;
    g_eff_command_block.unk0C = arg0;
    if (g_eff_command_status[18] != 0 && g_eff_command_status[19] != 0) {
        g_eff_command_status[19] = 0;
        g_eff_command_status[18] = 0;
    }
    if (g_eff_command_status[18] == 0) {
        D_800A3468->unk14 = &g_eff_command_status[18];
        D_800A3468->unk00.w = 0x210012;
    } else if (g_eff_command_status[19] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[19];
        g_eff_command_block.unk00.w = 0x210013;
    }
    eff_StartCommand();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF00FF;
}

extern VECTOR D_8009BB74;

void eff_SetRotMatrix(s16 *a0, s32 *a1, MATRIX *a2) {
    SVECTOR *dest = &D_800A34EC->v80061FAC;
    dest->vx = a0[0];
    dest->vy = a0[1];
    dest->vz = a0[2];
    RotMatrix(dest, a2);
    a2->t[2] = 0;
    a2->t[1] = 0;
    a2->t[0] = 0;
    ScaleMatrixL(a2, &D_8009BB74);
    SetRotMatrix(a2);
}

extern s32 g_eff_billboard_draw_count;

void eff_SetBillboardList(Unk800F1198Record *arg0) {
    s32 i;

    g_eff_billboard_draw_count = 0;
    for (i = 0; arg0[i].unk0 & 1; i++) {
        g_eff_billboards[i].unk0 = arg0[i].unk0;
        g_eff_billboards[i].unk4 = arg0[i].unk4;
        g_eff_billboards[i].unk8 = arg0[i].unk8;
    }
    g_eff_billboards[i].unk0 = g_eff_billboards[i].unk4 =
        g_eff_billboards[i].unk8 = 0;
}

/* Draw the g_eff_billboards billboard list (see eff_SetBillboardList): per
   record until the terminator, pick a sprite size and an animation frame from
   the record's type, rotate/translate its position, project it, and when it
   lands in range emit one textured POLY_FT4 billboard and link it into the OT
   at its depth. */
void eff_DrawBillboardList(s16 *arg0, s32 *arg1) {
    extern s32 g_eff_billboard_draw_count;
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    extern s32 rand(void);
    Unk1F8000B8_800620B8 *base;
    s16 *w;
    s16 *h;
    VECTOR *var_3;
    VECTOR *v;
    SVECTOR *var_5;
    s32 *var_6;
    u32 *z;
    POLY_FT4 *prim;
    Unk1F800000Unk00 *var_9;
    s16 *dst16;
    s32 *dst32;
    MATRIX *rot;
    s16 i;
    s32 var_14;
    s32 var_15;
    s16 width;
    s16 height;

    eff_BindScratchpad(arg0, arg1);
    var_9 = D_800A3468;
    dst16 = D_800A346C;
    dst16[0] = var_9->unk04[0];
    prim = g_eff_quad_cursor;
    dst16[1] = var_9->unk04[1];
    dst16[2] = var_9->unk04[2];
    dst32 = D_800A3470;
    /* the matrix eff_SetRotMatrix builds from the angles at arg0 */
    rot = D_800A3474;
    dst32[0] = var_9->unk08[0];
    base = &D_800A34EC->v800620B8;
    dst32[1] = var_9->unk08[1];
    dst32[2] = var_9->unk08[2];
    g_eff_billboard_draw_count++;
    eff_SetRotMatrix(dst16, dst32, rot);
    SetRotMatrix(D_800A3474);
    w = &base->unk10;
    h = &base->unk12;
    var_3 = &base->unk14;
    v = &base->unk24;
    var_5 = &base->unk34;
    var_6 = &base->unk3C;
    z = &base->unk44;
    var_5->vz = 0;
    var_5->vy = 0;
    var_5->vx = 0;
    *D_800A34B0 = ReadGeomScreen() << 8;
    *D_800A3490 = 0x2F;
    for (i = 0; g_eff_billboards[i].unk0 & 1; i++) {
        switch (g_eff_billboards[i].unk4 & 7) {
        case 3:
            *D_800A34A8 = 0x151;
            *D_800A34AC = 0xA8;
            goto sel_a;
        case 0:
            *D_800A34A8 = 0x1C2;
            *D_800A34AC = 0xE1;
        sel_a:
            D_800A348C = D_800A3488 =
                &D_8009BA00[(u32)g_eff_billboard_draw_count % 6];
            if (D_8009BD24.unk20_0) {
                D_800A348C = &D_8009BA00[10];
            }
            *D_800A349C = D_800A3488->u + 0x1F;
            *D_800A34A4 = D_800A3488->v + 0x1F;
            break;
        case 2:
            *D_800A34A8 = 0x50;
            *D_800A34AC = 0x3C;
            goto sel_b;
        case 1:
            *D_800A34A8 = 0x64;
            *D_800A34AC = 0x78;
        sel_b:
            D_800A348C = D_800A3488 =
                &D_8009BA00[6 + (g_eff_billboard_draw_count & 3)];
            if (D_8009BD24.unk20_0) {
                D_800A348C = &D_8009BA00[11];
            }
            *D_800A349C = D_800A3488->u + 0xF;
            *D_800A34A4 = D_800A3488->v + 0x13;
            break;
        }
        v->vx = g_eff_billboards[i].unk0 / 2 - D_800A3470[0];
        v->vy = g_eff_billboards[i].unk4 / 8 - D_800A3470[1];
        v->vz = g_eff_billboards[i].unk8 - D_800A3470[2];
        ApplyRotMatrixLV(v, var_3);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the address 0x14
         * below var_3, it loads var_3 as the translation (there is no whole
         * MATRIX here). Spelled (MATRIX *)base: score 65. */
        SetTransMatrix((MATRIX *)((u8 *)var_3 - 0x14));
        RotTransPers(var_5, D_800A34B8, var_6, D_800A34CC);
        gte_stsz(D_800A34D0);
        *z = gpu_DepthToOtIndex(*D_800A34D0, 0);
        if (*z < 0x1005 && (*D_800A34B0 / 256 >> 4) < *z) {
            *D_800A3494 = (u16)(((D_800A348C->clut_x >> 4) & 0x3F) +
                                (D_800A348C->clut_y << 6));
            *D_800A3498 = D_800A3488->u;
            *D_800A34A0 = D_800A3488->v;
            *D_800A34D0 = *D_800A34D0 ? *D_800A34D0 : 1;
            *D_800A34B4 = *D_800A34B0 / *D_800A34D0;
            var_14 = *D_800A34A8 * *D_800A34B4;
            width = var_14 > 0x200 ? var_14 >> 8 : 2;
            *w = width / 2;
            var_15 = *D_800A34AC * *D_800A34B4;
            height = var_15 > 0x200 ? var_15 >> 8 : 2;
            *h = height;
            *h += (*h * rand() / 10) >> 14;
            SetPolyFT4(prim);
            prim->tpage = *D_800A3490;
            prim->clut = *D_800A3494;
            prim->r0 = 0xFF;
            prim->g0 = 0x80;
            prim->b0 = 0x80;
            prim->x0 = *D_800A34B8 - *w;
            prim->y0 = (*D_800A34B8 >> 16) - *h;
            prim->x1 = *w + *D_800A34B8;
            prim->y1 = (*D_800A34B8 >> 16) - *h;
            prim->x2 = *D_800A34B8 - *w;
            prim->y2 = *D_800A34B8 >> 16;
            prim->x3 = *w + *D_800A34B8;
            prim->y3 = *D_800A34B8 >> 16;
            prim->u0 = *D_800A3498;
            prim->v0 = *D_800A34A0;
            prim->u1 = *D_800A349C;
            prim->v1 = *D_800A34A0;
            prim->u2 = *D_800A3498;
            prim->v2 = *D_800A34A4;
            prim->u3 = *D_800A349C;
            prim->v3 = *D_800A34A4;
            SetShadeTex(prim, 1);
            SetSemiTrans(prim, 1);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                D_800A34E4 = g_gpu_ot_ptr + *z;
                D_800A34E8 = &prim->tag;
                *D_800A34E8 =
                    (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
                *D_800A34E4 =
                    ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
                prim++;
            }
        }
    }
    g_eff_quad_cursor = prim;
}

s32 func_8006288C(void) {
    s32 *pos;
    s16 *rot;
    s32 i;
    s32 mask;

    g_eff_command_phase[26] = 1;
    pos = D_800A347C;
    rot = D_800A3478;
    for (i = 0; i < 6; i++) {
        mask = 1 << i;
        if (!(D_800A3460 & mask)) {
            D_800F0FB8[i].x = pos[0];
            D_800F0FB8[i].y = pos[1];
            D_800F0FB8[i].z = pos[2];
            D_800F10A0[i].vx = rot[0];
            D_800F10A0[i].vy = rot[1];
            D_800A3460 |= mask;
            D_800F10A0[i].vz = rot[2];
            D_800F0C04[i] = 0;
            break;
        }
    }
    return 1;
}

/* Draw the up-to-6 slots func_8006288C spawns: per active slot, scale/rotate/
   translate its matrix, then emit three textured POLY_FT4 quads, and finally
   link the new quads into the OT. Returns 1 when quads were added, else the
   live-slot mask (0 once every slot has expired). */
s32 func_8006295C(void) {
    extern s16 D_8009BB84[];
    s32 count;
    MATRIX *var_1;
    s32 *var_2;
    u16 *var_3;
    s32 *scale;
    s32 *var_5;
    MATRIX *mats;
    MATRIX *m;
    s32 *sxy;
    POLY_FT4 *prim;
    s32 i;
    s32 j;
    s32 k;
    s32 v;
    s32 c;
    s32 bit;
    s16 *var_16;
    POLY_FT4 *end;

    /* FAKE: the work-area base (D_800A34EC) is staged through `prim` before
     * prim becomes the quad cursor; keeps `mats = base + 0x78` out of the
     * loop's giv init (target keeps `move s4,v0`) and puts base in s1. A
     * separate base local or the direct global: score 37. */
    prim = (POLY_FT4 *)D_800A34EC;
    count = 0;
    mats = (MATRIX *)((u8 *)prim + 0x78);
    var_1 = (MATRIX *)((u8 *)prim + 0x138);
    sxy = (s32 *)((u8 *)prim + 0x158);
    var_2 = (s32 *)((u8 *)prim + 0x168);
    var_3 = (u16 *)((u8 *)prim + 0x16C);
    scale = (s32 *)((u8 *)prim + 0x178);
    var_5 = (s32 *)((u8 *)prim + 0x188);
    prim = g_eff_quad_cursor;
    for (i = 0; i < 6; i++) {
        bit = 1 << i;
        if (!(D_800A3460 & bit)) {
            continue;
        }
        m = &mats[i];
        v = rsin(((D_800F0C04[i] + 6) << 10) / 6);
        scale[0] = scale[1] = scale[2] = v + (D_800F0C04[i] << 12) / 6;
        RotMatrixZYX(&D_800F10A0[i].vx, (u8 *)m);
        ScaleMatrix((u8 *)m, scale);
        m->t[0] = D_800F0FB8[i].x - D_800A3470[0];
        m->t[1] = D_800F0FB8[i].y - D_800A3470[1];
        m->t[2] = D_800F0FB8[i].z - D_800A3470[2];
        CompMatrix(D_800A3474, m, var_1);
        SetRotMatrix(var_1);
        SetTransMatrix(var_1);
        for (j = 0; j < 3; j++) {
            if (D_800F0C04[i] < 3) {
                *(s32 *)&prim->r0 = 0x808080;
            } else {
                c = ((6 - D_800F0C04[i]) << 7) / 3;
                *var_5 = c;
                *(s32 *)&prim->r0 = c + (c << 8) + (c << 16);
            }
            if (D_800F0C04[i] < 3) {
                D_800A3488 = j ? D_8009B960 : D_8009B958;
            } else {
                D_800A3488 = j ? D_8009B970 : D_8009B968;
            }
            if (j) {
                *D_800A349C = D_800A3488->u + 0x3F;
            } else {
                *D_800A349C = D_800A3488->u + 0x1F;
            }
            *D_800A34A4 = D_800A3488->v + 0x1F;
            *D_800A3490 = 0x2E;
            *D_800A3494 =
                (((D_800A3488->clut_x >> 4) & 0x3F) + (D_800A3488->clut_y << 6))
                << 16;
            *D_800A3490 = *D_800A3490 << 16;
            *D_800A3498 = D_800A3488->u;
            *D_800A34A0 = D_800A3488->v;
            *D_800A34D4 = *D_800A3498 + (*D_800A34A0 << 8);
            *D_800A34D8 = *D_800A349C + (*D_800A34A0 << 8);
            *D_800A34DC = *D_800A3498 + (*D_800A34A4 << 8);
            *D_800A34E0 = *D_800A349C + (*D_800A34A4 << 8);
            setlen(prim, 9);
            prim->code = 0x2E;
            *(s32 *)&prim->u0 = *D_800A34D4 + *D_800A3494;
            *(s32 *)&prim->u1 = *D_800A34D8 + *D_800A3490;
            *(u16 *)&prim->u2 = *D_800A34DC;
            *(u16 *)&prim->u3 = *D_800A34E0;
            SetPolyFT4(prim);
            SetShadeTex(prim, 1);
            SetSemiTrans(prim, 1);
            var_16 = &D_8009BB84[j * 16];
            RotTransPers4(var_16, var_16 + 4, var_16 + 8, var_16 + 12, sxy,
                          sxy + 1, sxy + 2, sxy + 3, var_2, D_800A34CC);
            gte_stsz(D_800A34D0);
            *D_800A34D0 = gpu_DepthToOtIndex(*D_800A34D0, 0);
            if (*D_800A34D0 == 0) {
                *D_800A34D0 = 1;
            }
            if (*D_800A34D0 < 0x1005) {
                var_3[count] = *D_800A34D0;
                *(s32 *)&prim->x0 = sxy[0];
                *(s32 *)&prim->x1 = sxy[1];
                *(s32 *)&prim->x2 = sxy[2];
                *(s32 *)&prim->x3 = sxy[3];
                count++;
                if (prim - g_eff_quad_buffer_base < 0x1C1) {
                    prim++;
                }
            }
        }
        D_800F0C04[i]++;
        if (D_800F0C04[i] >= 7) {
            D_800A3460 &= ~(1 << i);
        }
    }
    if (g_eff_quad_cursor != prim) {
        /* FAKE: `end` keeps the fill position and prim walks the same buffer
         * again to link each quad; with a fresh cursor local the registers
         * rotate: score 37. */
        end = prim;
        for (prim = g_eff_quad_cursor, k = 0; prim < end; prim++, k++) {
            AddPrim(g_gpu_ot_ptr + var_3[k], prim);
        }
        g_eff_quad_cursor = end;
        return 1;
    }
    if (D_800A3460 == 0) {
        g_eff_command_phase[26] = 0;
        return 0;
    }
    return D_800A3460;
}

/* Spawn a slot: claim the first free slot bit in D_800A3448 (of 12), copy
   the position at *D_800A347C into its D_800F0E38 record and reset its age.
   With every slot taken it writes slot 12 (0x800F0EC8, the next table) --
   the original has no guard. */
s32 eff_SpawnBillboardPair(void) {
    s32 i;
    s32 bit;
    s32 *src;

    g_eff_command_phase[8] = 1;
    for (i = 0; i < 12; i++) {
        bit = 1 << i;
        if (!(D_800A3448 & bit)) {
            D_800A3448 |= bit;
            break;
        }
    }
    src = D_800A347C;
    D_800F0E38[i].unk0 = src[0];
    D_800F0E38[i].unk4 = src[1];
    D_800F0E38[i].unk8 = src[2];
    D_800F0BEC[i] = 0;
    return 1;
}

/* Draw the up-to-12 slots eff_SpawnBillboardPair spawns: per live slot i
   (bit i of D_800A3448, age D_800F0BEC[i], world position D_800F0E38[i]) emit
   two textured POLY_FT4 billboards (j == 0, then j == 1 offset vertically by
   an rsin of its age), each rotated/translated relative to the camera,
   projected, sized from its
   depth and animated by its age, then linked into the OT at its depth. A slot
   whose age reaches 16 clears its bit. Returns nonzero while any slot is still
   live. */
s32 func_80063084(void) {
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80063084 *base;
    POLY_FT4 *prim;
    VECTOR *var_2;
    SVECTOR *var_3;
    SVECTOR *v;
    s32 *var_5;
    s32 *var_6;
    s32 *z;
    s16 i;
    s16 j;
    s32 bit;
    s32 scale;
    s32 var_12;

    base = &D_800A34EC->v80063084;
    prim = g_eff_quad_cursor;
    var_2 = &base->unk14;
    var_3 = &base->unk24;
    v = &base->unk2C;
    var_5 = &base->unk34;
    var_6 = &base->unk38;
    z = &base->unk3C;
    gte_SetRotMatrix(D_800A3474);
    v->vz = 0;
    v->vy = 0;
    v->vx = 0;
    *D_800A34B0 = ReadGeomScreen() * 1000;
    *D_800A34A8 = 0xC0;
    *D_800A34AC = 0x60;
    *D_800A3490 = 0x2E;
    *D_800A3490 = *D_800A3490 << 16;
    for (i = 0; i < 12; i++) {
        bit = 1 << i;
        if (!(D_800A3448 & bit)) {
            continue;
        }
        for (j = 0; j < 2; j++) {
            if (D_800F0BEC[i] < 16) {
                var_3->vx = D_800F0E38[i].unk0 - D_800A3470[0];
                var_3->vz = D_800F0E38[i].unk8 - D_800A3470[2];
                if (j != 0 && D_800F0BEC[i] >= 3) {
                    var_3->vy =
                        D_800F0E38[i].unk4 -
                        (rsin((D_800F0BEC[i] - 3) << 8) * 329 / 4096 - 69) /
                            10 -
                        D_800A3470[1];
                } else {
                    var_3->vy = D_800F0E38[i].unk4 - D_800A3470[1];
                }
                ApplyRotMatrix(var_3, var_2);
                /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the
                 * address 0x14 below var_2, it loads var_2 as the translation.
                 * Spelled (MATRIX *)base: score 55. */
                SetTransMatrix((MATRIX *)((u8 *)var_2 - 0x14));
                RotTransPers(v, D_800A34B8, var_5, D_800A34CC);
                gte_stsz(D_800A34D0);
                if (j == 0) {
                    D_800A3488 = D_8009B940;
                } else if (D_800F0BEC[i] < 11) {
                    D_800A3488 = D_8009B948;
                } else {
                    D_800A3488 = D_8009B950;
                }
                *D_800A3494 = (((D_800A3488->clut_x >> 4) & 0x3F) +
                               (D_800A3488->clut_y << 6))
                              << 16;
                *D_800A3498 = D_800A3488->u;
                *D_800A34A0 = D_800A3488->v;
                *D_800A349C = D_800A3488->u + 0x3F;
                *D_800A34A4 = D_800A3488->v + 0x3F;
                *D_800A34D4 = *D_800A3498 + (*D_800A34A0 << 8);
                *D_800A34D8 = *D_800A349C + (*D_800A34A0 << 8);
                *D_800A34DC = *D_800A3498 + (*D_800A34A4 << 8);
                *D_800A34E0 = *D_800A349C + (*D_800A34A4 << 8);
                *z = gpu_DepthToOtIndex(*D_800A34D0, 0);
                if (*z < 0x1005 && *D_800A34B0 / 1000 >> 4 < *z) {
                    *D_800A34B4 = *D_800A34B0 / *D_800A34D0;
                    /* Sprite size = base size * depth scale, in 1/256 units,
                       at least 8. */
                    *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x800
                                      ? *D_800A34A8 * *D_800A34B4 >> 8
                                      : 8;
                    *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x800
                                      ? *D_800A34AC * *D_800A34B4 >> 8
                                      : 8;
                    if (j == 0) {
                        if (D_800F0BEC[i] < 5) {
                            *D_800A34C0 =
                                *D_800A34C0 *
                                (rcos((D_800F0BEC[i] << 10) / 5) * 3 / 4096 +
                                 7) /
                                10;
                        } else if (D_800F0BEC[i] < 16) {
                            *D_800A34C0 =
                                *D_800A34C0 *
                                (rcos(((D_800F0BEC[i] - 5) << 10) / 11) * 7 /
                                 4096) /
                                10;
                        }
                        if (D_800F0BEC[i] < 10) {
                            *D_800A34C8 =
                                *D_800A34C8 *
                                (rsin((D_800F0BEC[i] << 10) / 10) * 85 / 4096) /
                                100;
                        } else if (D_800F0BEC[i] < 16) {
                            *D_800A34C8 =
                                *D_800A34C8 *
                                (rcos(((D_800F0BEC[i] - 10) << 10) / 6) * 85 /
                                 4096) /
                                100;
                        }
                        if (D_8009BD24.unk20_0) {
                            *(s32 *)&prim->r0 = 0x808080;
                        } else {
                            *(s32 *)&prim->r0 = 0xFF8080;
                        }
                    } else {
                        scale = rsin((D_800F0BEC[i] << 10) / 12) * 12 / 4096;
                        *D_800A34C0 = *D_800A34C0 * scale / 10;
                        *D_800A34C8 = *D_800A34C8 * scale / 10;
                        if (D_800F0BEC[i] >= 9) {
                            /* *var_6 = age - 15: the colour word below is built
                             * from -*var_6 * 128 / 5 and -*var_6 * 255 / 5. */
                            *var_6 = D_800F0BEC[i] - 15;
                            var_12 = (*var_6 * -128 / 5) & 0xFF;
                            *(s32 *)&prim->r0 =
                                var_12 + (var_12 << 8) +
                                ((*var_6 * -255 / 5 & 0xFF) << 16);
                        } else {
                            *(s32 *)&prim->r0 = 0;
                        }
                    }
                    setlen(prim, 9);
                    prim->code = 0x2E;
                    *D_800A34C0 = *D_800A34C0 >> 1;
                    *D_800A34BC = -*D_800A34C0;
                    *D_800A34C4 = *D_800A34C8 * 60 / 64;
                    *D_800A34C8 = 0;
                    *(s32 *)&prim->x0 =
                        *D_800A34B8 + *D_800A34BC - (*D_800A34C4 << 16);
                    *(s32 *)&prim->x1 =
                        *D_800A34B8 + *D_800A34C0 - (*D_800A34C4 << 16);
                    *(s32 *)&prim->x2 =
                        *D_800A34B8 + *D_800A34BC + (*D_800A34C8 << 16);
                    *(s32 *)&prim->x3 =
                        *D_800A34B8 + *D_800A34C0 + (*D_800A34C8 << 16);
                    *(s32 *)&prim->u0 = *D_800A34D4 + *D_800A3494;
                    *(s32 *)&prim->u1 = *D_800A34D8 + *D_800A3490;
                    *(u16 *)&prim->u2 = *D_800A34DC;
                    *(u16 *)&prim->u3 = *D_800A34E0;
                    D_800A34E4 = g_gpu_ot_ptr + *z;
                    D_800A34E8 = &prim->tag;
                    *D_800A34E8 =
                        (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
                    *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) |
                                  (*D_800A34E4 & 0xFF000000);
                    if (prim - g_eff_quad_buffer_base < 0x1C1) {
                        prim++;
                    }
                }
            } else {
                D_800A3448 &= 0xFFFF - (1 << i);
            }
        }
        D_800F0BEC[i]++;
    }
    g_eff_quad_cursor = prim;
    return D_800A3448 != 0;
}

u8 func_80063BD0(s32);

u8 func_80063AF0(void) {
    g_eff_command_phase[0] = 1;
    D_800A345C[0] = (D_800A3468->unk00.w >> 17) & 3;
    return func_80063BD0(0);
}

u8 func_80063B34(void) {
    g_eff_command_phase[1] = 1;
    D_800A345C[1] = (D_800A3468->unk00.w >> 17) & 3;
    return func_80063BD0(1);
}

s32 func_80063E10(s32);
void eff_BuildUsedSlotRotMatrices(s32 *count, MATRIX *m, s32 idx);

u8 func_80063B78(void) {
    *D_800A3480 = D_800A345C[0];
    return func_80063E10(0);
}

u8 func_80063BA4(void) {
    *D_800A3480 = D_800A345C[1];
    return func_80063E10(1);
}

extern SVECTOR D_800F1000[][10];

/* func_80063BD0 -- slot allocator for lane `idx`: D_800A344C[idx] counts live
 * entries, and D_800A3454[idx] is the per-slot in-use bitmask. While fewer
 * than 10 entries are live, take the lowest free bit, mark it, and fill that
 * slot's SVECTOR (D_800F1000[idx][slot]) and 3-word record
 * (D_800F0EC8[idx][slot]) from D_800A3478 / D_800A347C. Once the lane is
 * full, the counter wraps through 10..19 and slots are overwritten in
 * rotation. */
u8 func_80063BD0(s32 idx) {
    s32 mask;
    s32 i;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        for (i = 0; i < D_800A344C[idx]; i++) {
            if (!(D_800A3454[idx] & (mask = 1 << i))) {
                D_800A3454[idx] |= mask;
                D_800F1000[idx][i].vy = D_800A3478[1];
                D_800F1000[idx][i].vx = D_800F1000[idx][i].vz = 0;
                D_800F0EC8[idx][i].unk0 = D_800A347C[0];
                D_800F0EC8[idx][i].unk4 = D_800A347C[1];
                D_800F0EC8[idx][i].unk8 = D_800A347C[2];
                break;
            }
        }
    } else {
        D_800A344C[idx] = (D_800A344C[idx] + 1) % 10 + 10;
        D_800F1000[idx][D_800A344C[idx] - 10].vy = D_800A3478[1];
        D_800F1000[idx][D_800A344C[idx] - 10].vx =
            D_800F1000[idx][D_800A344C[idx] - 10].vz = 0;
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk0 = D_800A347C[0];
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk4 = D_800A347C[1];
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk8 = D_800A347C[2];
    }
    return 1;
}

/* Draw the live slots (up to 10, see func_80063BD0) of `arg0`: per slot whose
 * bit is set in D_800A3454[arg0], emit one textured POLY_FT4 billboard at the
 * slot's position (relative to *D_800A3470) through the composite of
 * D_800A3474 and the slot's matrix, keep it when its depth is in range, then
 * link every new quad into the OT at its depth. Returns 1. */
s32 func_80063E10(s32 arg0) {
    extern SVECTOR D_8009BBE4;
    extern SVECTOR D_8009BBEC;
    extern SVECTOR D_8009BBF4;
    extern SVECTOR D_8009BBFC;
    s32 count;
    MATRIX *mats;
    MATRIX *var_2;
    s32 *sxy;
    u16 *var_4;
    u16 *var_5;
    POLY_FT4 *prim;
    POLY_FT4 *end;
    s32 i;
    s32 k;
    s32 bit;

    /* FAKE: the work-area base (D_800A34EC) is staged through `prim` before
     * prim becomes the quad cursor: prim's pseudo lives across the calls and
     * gets s2, where the target holds the base (a fresh local gets v0:
     * score 12; direct global: 12). */
    prim = (POLY_FT4 *)D_800A34EC;
    mats = (MATRIX *)((u8 *)prim + 0x28);
    var_2 = (MATRIX *)((u8 *)prim + 0x168);
    sxy = (s32 *)((u8 *)prim + 0x188);
    var_4 = (u16 *)((u8 *)prim + 0x19C);
    var_5 = (u16 *)((u8 *)prim + 0x1B0);
    if (D_800A344C[arg0] < 10) {
        count = D_800A344C[arg0];
    } else {
        count = 10;
    }
    eff_BuildUsedSlotRotMatrices(&count, mats, arg0);
    D_800A3488 = &D_8009B920[*D_800A3480];
    prim = g_eff_quad_cursor;
    *D_800A3490 = 0xE;
    *D_800A3494 =
        (((D_800A3488->clut_x >> 4) & 0x3F) + (D_800A3488->clut_y << 6)) << 16;
    *D_800A3490 = *D_800A3490 << 16;
    *D_800A3498 = D_800A3488->u;
    *D_800A34A0 = D_800A3488->v;
    *D_800A349C = D_800A3488->u + 7;
    *D_800A34A4 = D_800A3488->v + 0xF;
    *D_800A34D4 = *D_800A3498 + (*D_800A34A0 << 8);
    *D_800A34D8 = *D_800A349C + (*D_800A34A0 << 8);
    *D_800A34DC = *D_800A3498 + (*D_800A34A4 << 8);
    *D_800A34E0 = *D_800A349C + (*D_800A34A4 << 8);
    *var_5 = 0;
    *D_800A34B0 = ReadGeomScreen();
    for (i = 0; i < count; i++) {
        /* FAKE: the slot's mask is named `bit` inside the test and not read
         * again; it stretches the D_800A3454[arg0] address's life so loop.c
         * hoists it (target spills it to 32(sp)), and keeps combine from making
         * the test srav/andi. Without `bit`: score 43. */
        if (!(D_800A3454[arg0] & (bit = 1 << i))) {
            continue;
        }
        setlen(prim, 9);
        prim->code = 0x2F;
        *(s32 *)&prim->u0 = *D_800A34D4 + *D_800A3494;
        *(s32 *)&prim->u1 = *D_800A34D8 + *D_800A3490;
        *(u16 *)&prim->u2 = *D_800A34DC;
        *(u16 *)&prim->u3 = *D_800A34E0;
        mats[i].t[0] = D_800F0EC8[arg0][i].unk0 - D_800A3470[0];
        mats[i].t[1] = D_800F0EC8[arg0][i].unk4 - D_800A3470[1];
        mats[i].t[2] = D_800F0EC8[arg0][i].unk8 - D_800A3470[2];
        gte_CompMatrix(D_800A3474, &mats[i], var_2);
        gte_SetRotMatrix(var_2);
        gte_SetTransMatrix(var_2);
        gte_ldv3(&D_8009BBE4, &D_8009BBEC, &D_8009BBF4);
        gte_rtpt();
        gte_stsxy3(sxy, sxy + 1, sxy + 2);
        gte_stsz(D_800A34D0);
        if (*D_800A34D0 <= 0) {
            continue;
        }
        *D_800A34D0 = gpu_DepthToOtIndex(*D_800A34D0 - 50, 0);
        if (*D_800A34D0 >= 0x1005) {
            continue;
        }
        if ((*D_800A34B0 >> 4) >= *D_800A34D0) {
            continue;
        }
        var_4[(*var_5)++] = *D_800A34D0;
        gte_ldv0(&D_8009BBFC);
        gte_rtps();
        gte_stsxy(sxy + 3);
        *(s32 *)&prim->x0 = sxy[0];
        *(s32 *)&prim->x1 = sxy[1];
        *(s32 *)&prim->x2 = sxy[2];
        *(s32 *)&prim->x3 = sxy[3];
        if (prim - g_eff_quad_buffer_base < 0x1C1) {
            prim++;
        }
    }
    /* FAKE: `end` keeps the fill position and prim walks the same buffer again
     * to link each quad; with a fresh tail cursor sxy and prim swap s2/s3:
     * score 36. */
    end = prim;
    for (prim = g_eff_quad_cursor, k = 0; prim < end; prim++, k++) {
        D_800A34E8 = &prim->tag;
        D_800A34E4 = g_gpu_ot_ptr + var_4[k];
        *D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
        *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
    }
    g_eff_quad_cursor = end;
    return 1;
}

/* eff_BuildUsedSlotRotMatrices -- rotates one matrix per enabled bit: for every
 * i < *count whose bit is set in D_800A3454[idx],
 * RotMatrix(&D_800F1000[idx][i], &m[i]).
 */
void eff_BuildUsedSlotRotMatrices(s32 *count, MATRIX *m, s32 idx) {
    s32 i;
    s32 mask;

    for (i = 0; i < *count; i++) {
        /* FAKE: setting mask twice keeps the target's `li v0,1; sllv` inside
         * the loop; `mask = 1 << i` hoists the 1 into s5: score 12. */
        mask = 1;
        mask <<= i;
        if (D_800A3454[idx] & mask) {
            RotMatrix(&D_800F1000[idx][i], &m[i]);
        }
    }
}

/* eff_SpawnJitteredBillboards -- for each group of four slots (bits of
 * D_800A3444), claim the first free slot: set its position D_800F0D78[slot] to
 * the position at D_800A347C jittered by (rand() & 0xFF) - 0x7F per axis, give
 * it a random 0..7 in D_800F0BCC, and mark its bit. */
s32 eff_SpawnJitteredBillboards(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;

    g_eff_command_phase[7] = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            mask = 1 << idx;
            if (!(D_800A3444 & mask)) {
                D_800F0D78[idx].x = D_800A347C[0] + (rand() & 0xFF) - 0x7F;
                D_800F0D78[idx].y = D_800A347C[1] + (rand() & 0xFF) - 0x7F;
                D_800F0D78[idx].z = D_800A347C[2] + (rand() & 0xFF) - 0x7F;
                D_800F0BCC[idx] = rand() & 7;
                D_800A3444 |= mask;
                break;
            }
        }
    }
    return 1;
}

/* Draw the up-to-16 effect slots eff_SpawnJitteredBillboards spawns: per live
   slot (bit i of D_800A3444) step its counter D_800F0BCC[i]; while counter / 4
   is below 7, project the slot's world position through D_800A3474 and, when
   its depth is in range, emit one textured, semi-transparent POLY_FT4 billboard
   sized by the screen depth and step the counter again; once counter / 4
   reaches 7 the slot is retired. Finally link every new quad into the OT at its
   depth. Returns 1 when any slot is still live, else 0. */
s32 func_800646E8(void) {
    Unk1F8000B8_800646E8 *base;
    u32 *var_1;
    s16 *w;
    s16 *h;
    s32 *var_4;
    VECTOR *var_5;
    VECTOR *pos;
    SVECTOR *var_7;
    s32 *p;
    POLY_FT4 **end;
    POLY_FT4 *prim;
    u32 *var_11;
    s16 i;
    s32 bit;

    base = &D_800A34EC->v800646E8;
    var_1 = base->unk48;
    w = &base->unk10;
    h = &base->unk12;
    var_4 = &base->unk14;
    var_5 = &base->unk18;
    pos = &base->unk28;
    var_7 = &base->unk38;
    p = &base->unk40;
    end = &base->unk44;
    prim = g_eff_quad_cursor;
    gte_SetRotMatrix(D_800A3474);
    var_7->vx = var_7->vy = var_7->vz = 0;
    *D_800A34B0 = ReadGeomScreen() * 1000;
    *D_800A34A8 = 0x40;
    *D_800A34AC = 0x20;
    *D_800A3490 = 0xE;
    for (i = 0, var_11 = var_1; i < 16; i++) {
        bit = 1 << i;
        if (!(D_800A3444 & bit)) {
            continue;
        }
        D_800F0BCC[i]++;
        *var_4 = D_800F0BCC[i] / 4;
        if (*var_4 < 7) {
            pos->vx = D_800F0D78[i].x - D_800A3470[0];
            pos->vy = D_800F0D78[i].y - D_800A3470[1];
            pos->vz = D_800F0D78[i].z - D_800A3470[2];
            ApplyRotMatrixLV(pos, var_5);
            /* FAKE: the MATRIX whose t[] is *var_5: SetTransMatrix reads only
             * m->t (there is no whole MATRIX here). Spelled
             * (MATRIX *)((u8 *)base + 4): score 68. */
            SetTransMatrix((MATRIX *)((u8 *)var_5 - 0x14));
            RotTransPers(var_7, D_800A34B8, p, D_800A34CC);
            gte_stsz(D_800A34D0);
            D_800A3488 = &D_8009B8E8[*var_4];
            *D_800A3494 = (u16)(((D_800A3488->clut_x >> 4) & 0x3F) +
                                (D_800A3488->clut_y << 6));
            *D_800A3498 = D_800A3488->u;
            *D_800A34A0 = D_800A3488->v;
            *D_800A349C = D_800A3488->u + 0x3F;
            *D_800A34A4 = D_800A3488->v + 0x1F;
            *var_11 = gpu_DepthToOtIndex(*D_800A34D0, 0);
            if (*var_11 >= 0x1005) {
                continue;
            }
            if ((*D_800A34B0 / 1000 >> 4) >= *var_11) {
                continue;
            }
            *D_800A34B4 = *D_800A34B0 / *D_800A34D0;
            *w = *D_800A34A8 * *D_800A34B4 > 0x800
                     ? (*D_800A34A8 * *D_800A34B4) >> 8
                     : 8;
            *h = *D_800A34AC * *D_800A34B4 > 0x800
                     ? (*D_800A34AC * *D_800A34B4) >> 8
                     : 8;
            *w += (*w * D_800F0BCC[i]) >> 5;
            *h += (*h * (D_800F0BCC[i] << 2)) >> 8;
            SetPolyFT4(prim);
            prim->tpage = *D_800A3490;
            prim->clut = *D_800A3494;
            if (D_8009BD24.unk20_0) {
                prim->r0 = 0x80;
                prim->g0 = 0x80;
                prim->b0 = 0x80;
            } else {
                prim->r0 = 0xA0;
                prim->g0 = 0x8C;
                prim->b0 = 0x50;
            }
            prim->x0 = *D_800A34B8 - *w / 2;
            prim->y0 = (*D_800A34B8 >> 16) - *h * 28 / 32;
            prim->x1 = *D_800A34B8 + *w / 2;
            prim->y1 = (*D_800A34B8 >> 16) - *h * 28 / 32;
            prim->x2 = *D_800A34B8 - *w / 2;
            prim->y2 = (*D_800A34B8 >> 16) + *h / 8;
            prim->x3 = *D_800A34B8 + *w / 2;
            prim->y3 = (*D_800A34B8 >> 16) + *h / 8;
            prim->u0 = *D_800A3498;
            prim->v0 = *D_800A34A0;
            prim->u1 = *D_800A349C;
            prim->v1 = *D_800A34A0;
            prim->u2 = *D_800A3498;
            prim->v2 = *D_800A34A4;
            prim->u3 = *D_800A349C;
            prim->v3 = *D_800A34A4;
            SetShadeTex(prim, 0);
            SetSemiTrans(prim, 1);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                prim++;
                var_11++;
            }
            D_800F0BCC[i]++;
        } else {
            D_800A3444 &= 0xFFFF - bit;
        }
    }
    if (D_800A3444 == 0) {
        return 0;
    }
    *end = prim;
    for (prim = g_eff_quad_cursor; prim < *end; prim++, var_1++) {
        D_800A34E4 = g_gpu_ot_ptr + *var_1;
        D_800A34E8 = &prim->tag;
        *D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
        *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
    }
    g_eff_quad_cursor = *end;
    return 1;
}

void func_80064E90(void) {
    D_800F0CA0[0].unk0 = D_800A347C[0];
    D_800F0CA0[0].unk4 = D_800A347C[1];
    D_800F0CA0[0].unk8 = D_800A347C[2];
    g_eff_command_phase[4] = 1;
    D_800F0BA8[0] = 0;
}

void func_80064ED8(void) {
    D_800F0CA0[1].unk0 = D_800A347C[0];
    D_800F0CA0[1].unk4 = D_800A347C[1];
    D_800F0CA0[1].unk8 = D_800A347C[2];
    g_eff_command_phase[5] = 1;
    D_800F0BA8[1] = 0;
}

void func_80064F20(void) {
    D_800F0CA0[2].unk0 = D_800A347C[0];
    D_800F0CA0[2].unk4 = D_800A347C[1];
    D_800F0CA0[2].unk8 = D_800A347C[2];
    g_eff_command_phase[6] = 1;
    D_800F0BA8[2] = 0;
}

s32 func_80064F68(void) {
    D_800F0CA0[3].unk0 = D_800A347C[0];
    D_800F0CA0[3].unk4 = D_800A347C[1];
    D_800F0CA0[3].unk8 = D_800A347C[2];
    g_eff_command_phase[9] = 1;
    D_800F0BA8[3] = 0x40;
    return 1;
}

s32 func_80064FB4(void) {
    D_800F0CA0[4].unk0 = D_800A347C[0];
    D_800F0CA0[4].unk4 = D_800A347C[1];
    D_800F0CA0[4].unk8 = D_800A347C[2];
    g_eff_command_phase[10] = 1;
    D_800F0BA8[4] = 0x40;
    return 1;
}

s32 func_80065000(void) {
    D_800F0CA0[5].unk0 = D_800A347C[0];
    D_800F0CA0[5].unk4 = D_800A347C[1];
    D_800F0CA0[5].unk8 = D_800A347C[2];
    g_eff_command_phase[11] = 1;
    D_800F0BA8[5] = 0;
    D_800A3440 = (D_800A3468->unk00.w >> 19) & 3;
    return 1;
}

void func_8006505C(void) {
    D_800F0CA0[6].unk0 = D_800A347C[0];
    D_800F0CA0[6].unk4 = D_800A347C[1];
    D_800F0CA0[6].unk8 = D_800A347C[2];
    g_eff_command_phase[12] = 1;
    D_800F0BA8[6] = 0;
}

void func_800650A4(void) {
    D_800F0CA0[7].unk0 = D_800A347C[0];
    D_800F0CA0[7].unk4 = D_800A347C[1];
    D_800F0CA0[7].unk8 = D_800A347C[2];
    g_eff_command_phase[13] = 1;
    D_800F0BA8[7] = 0;
}

void func_800650EC(void) {
    D_800F0CA0[10].unk0 = D_800A347C[0];
    D_800F0CA0[10].unk4 = D_800A347C[1];
    D_800F0CA0[10].unk8 = D_800A347C[2];
    g_eff_command_phase[14] = 1;
    D_800F0BA8[10] = 0;
}

void func_80065134(void) {
    D_800F0CA0[11].unk0 = D_800A347C[0];
    D_800F0CA0[11].unk4 = D_800A347C[1];
    D_800F0CA0[11].unk8 = D_800A347C[2];
    g_eff_command_phase[15] = 1;
    D_800F0BA8[11] = 0;
}

void func_8006517C(void) {
    D_800F0CA0[12].unk0 = D_800A347C[0];
    D_800F0CA0[12].unk4 = D_800A347C[1];
    D_800F0CA0[12].unk8 = D_800A347C[2];
    D_800F0BA8[12] = 0;
    D_800F0CA0[14].unk0 = D_800A347C[0];
    D_800F0CA0[14].unk4 = D_800A347C[1];
    D_800F0CA0[14].unk8 = D_800A347C[2];
    g_eff_command_phase[16] = 1;
    D_800F0BA8[14] = 0;
}

void func_800651F0(void) {
    D_800F0CA0[13].unk0 = D_800A347C[0];
    D_800F0CA0[13].unk4 = D_800A347C[1];
    D_800F0CA0[13].unk8 = D_800A347C[2];
    D_800F0BA8[13] = 0;
    D_800F0CA0[15].unk0 = D_800A347C[0];
    D_800F0CA0[15].unk4 = D_800A347C[1];
    D_800F0CA0[15].unk8 = D_800A347C[2];
    g_eff_command_phase[17] = 1;
    D_800F0BA8[15] = 0;
}

void func_80065264(void) {
    D_800F0CA0[16].unk0 = D_800A347C[0];
    D_800F0CA0[16].unk4 = D_800A347C[1];
    D_800F0CA0[16].unk8 = D_800A347C[2];
    g_eff_command_phase[18] = 1;
    D_800F0BA8[16] = 0;
}

void func_800652AC(void) {
    D_800F0CA0[17].unk0 = D_800A347C[0];
    D_800F0CA0[17].unk4 = D_800A347C[1];
    D_800F0CA0[17].unk8 = D_800A347C[2];
    g_eff_command_phase[19] = 1;
    D_800F0BA8[17] = 0;
}

u8 eff_DrawAnimatedQuads(s32);

u8 func_800652F4(void) {
    u8 v0 = eff_DrawAnimatedQuads(0);
    s16 *p = &D_800F0BA8[0];
    s16 v1 = *p;
    v1 += 0x1FF;
    *p = v1;
    if ((s16)v1 < 0x1001) {
        return v0;
    }
    return 0;
}

u8 func_80065344(void) {
    u8 v0 = eff_DrawAnimatedQuads(1);
    s16 *p = &D_800F0BA8[1];
    u16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}

u8 func_80065394(void) {
    u8 v0 = eff_DrawAnimatedQuads(2);
    s16 *p = &D_800F0BA8[2];
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}

u8 func_800653E4(void) {
    u8 v0 = eff_DrawAnimatedQuads(3);
    s16 *p = &D_800F0BA8[3];
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}

u8 func_80065434(void) {
    u8 v0 = eff_DrawAnimatedQuads(4);
    s16 *p = &D_800F0BA8[4];
    u16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}

u8 func_80065484(void) {
    unsigned int temp_v1;
    u8 v0;
    *D_800A3484 = D_800A3440;
    v0 = eff_DrawAnimatedQuads(5);
    temp_v1 = *D_800A3484;
    switch (temp_v1) {
    case 0: {
        s16 *p = &D_800F0BA8[5];
        *p = *p + 0x1FF;
        break;
    }
    case 1: {
        s16 *p = &D_800F0BA8[5];
        *p = *p + 0x3FE;
        break;
    }
    case 2: {
        s16 *p = &D_800F0BA8[5];
        *p = *p + 0x5FD;
        break;
    }
    }
    if (D_800F0BA8[5] < 0x2001) {
        return v0;
    }
    return 0;
}

u8 func_80065540(void) {
    u8 v0 = eff_DrawAnimatedQuads(6);
    s16 *p = &D_800F0BA8[6];
    u16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}

u8 func_80065590(void) {
    u8 v0 = eff_DrawAnimatedQuads(7);
    s16 *p = &D_800F0BA8[7];
    u16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}

u8 func_800655E0(void) {
    u8 v0 = eff_DrawAnimatedQuads(0xA);
    s16 *p = &D_800F0BA8[10];
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}

u8 func_80065630(void) {
    u8 v0 = eff_DrawAnimatedQuads(0xB);
    s16 *p = &D_800F0BA8[11];
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}

s32 func_80065680(void) {
    s16 *v1;
    s32 v0;
    eff_DrawAnimatedQuads(0xC);
    v1 = &D_800F0BA8[12];
    *v1 = *v1 + 1;
    v0 = eff_DrawAnimatedQuads(0xE);
    D_800F0BA8[14] = D_800F0BA8[14] + 1;
    v0 &= 0xFF;
    if (D_800F0BA8[14] >= 11) {
        v0 = 0;
    }
    return v0;
}

s32 func_800656EC(void) {
    s16 *s0 = &D_800F0BA8[13];
    s32 v0;
    eff_DrawAnimatedQuads(0xD);
    *s0 = *s0 + 1;
    v0 = eff_DrawAnimatedQuads(0xF);
    D_800F0BA8[15] = D_800F0BA8[15] + 1;
    if ((s16)*s0 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}

u8 func_80065760(void) {
    u8 v0 = eff_DrawAnimatedQuads(0x10);
    s16 *p = &D_800F0BA8[16];
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if (v1 < 0x11C8) {
        return v0;
    }
    return 0;
}

u8 func_800657B0(void) {
    u8 v0 = eff_DrawAnimatedQuads(0x11);
    s16 *p = &D_800F0BA8[17];
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}

/* Shared draw routine of the per-mode effect wrappers
 * (func_800652F4..func_800657B0, mode 0..0x11): projects this mode's D_800F0CA0
 * position record through the current camera, picks the texture/colour and quad
 * size from the mode and its D_800F0BA8 timer, shapes the four corners (switch
 * 2) and links one POLY_FT4 into the OT. Modes 6/7 re-run the body for the
 * paired mode (arg0 + 2, i.e. 8/9) via `goto again`; the re-run is guarded by
 * `arg0 < 9`, so modes 10/11 draw once. */
u8 eff_DrawAnimatedQuads(s32 arg0) {
    extern s16 D_800A3834;
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80065800 *var_0;
    POLY_FT4 *prim;
    s32 *var_2;
    VECTOR *p_t;
    VECTOR *p_in;
    SVECTOR *p_v;
    s16 *p_w;
    s16 *p_h;
    MATRIX *p_mat;
    s16 *var_9;
    s16 *var_10;
    VECTOR *dst;
    s32 n;
    s32 w;
    s32 var_14;
    s32 var_15;
    s32 h;
    s16 *t;
    /* FAKE: pointer alias of D_800F0BA8 - case 10/11's base address in its own
     * register ahead of the index shift */
    s16 *var_18;
    s32 i;

    var_0 = &D_800A34EC->v80065800;
    prim = g_eff_quad_cursor;
    var_2 = &var_0->unk10;
    p_t = var_0->unk14;
    p_in = &var_0->unk54;
    p_v = &var_0->unk64;
    p_w = &var_0->unk6C;
    p_h = &var_0->unk70;
    p_mat = &var_0->unk74;
    var_9 = &var_0->unk94;
    var_10 = &var_0->unk96;

    gte_SetRotMatrix(D_800A3474);
    p_in->vx = D_800F0CA0[arg0].unk0 - D_800A3470[0];
    p_in->vy = D_800F0CA0[arg0].unk4 - D_800A3470[1];
    p_in->vz = D_800F0CA0[arg0].unk8 - D_800A3470[2];
    ApplyRotMatrixLV(p_in, p_t);
    gte_SetTransMatrix((MATRIX *)var_0);
    p_v->vz = 0;
    p_v->vy = 0;
    p_v->vx = 0;
    gte_RotTransPers(p_v, D_800A34B8, var_2, D_800A34CC, D_800A34D0);
    *D_800A34D0 = *D_800A34D0 ? *D_800A34D0 << 2 : 1;

again:
    *D_800A34A8 = 0x20;
    *D_800A34AC = 0x20;
    *var_9 = 0x40;
    *var_10 = 0x40;
    *D_800A3490 = 0x2E;
    switch (arg0) {
    case 1:
    case 2:
        if (*D_800A34D0 > 4000) {
            D_800A3488 = D_8009B8D0;
        } else {
            D_800A3488 = D_8009B8D8;
        }
        prim->r0 = 0xFF;
        prim->g0 = 0x80;
        prim->b0 = 0x80;
        break;
    case 3:
    case 4:
        if (D_800F0BA8[arg0] < 0x96) {
            D_800A3488 = D_8009B980;
        } else {
            D_800A3488 = D_8009B978;
        }
        prim->r0 = 0xFF;
        prim->g0 = D_800F0BA8[arg0];
        prim->b0 = 0x60;
        *D_800A34A8 = 0xC0;
        *D_800A34AC = 0xC0;
        break;
    case 5:
        if (D_800F0BA8[4] > 0x800) {
            D_800A3488 = D_8009B8E0;
        } else {
            D_800A3488 = D_8009B8C8;
        }
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0xFF;
        break;
    case 0:
        D_800A3488 = D_8009B8C8;
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0xFF;
        break;
    case 10:
    case 11:
        if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            D_800A3488 = D_8009B9D8;
        } else {
            D_800A3488 = D_8009B990;
        }
        var_18 = D_800F0BA8;
        t = var_18 + arg0;
        if (*t > 0x96) {
            prim->r0 = 0;
            prim->g0 = (0xFF - *t) * 0xFF / 105;
        } else {
            prim->r0 = 0;
            prim->g0 = 0xFF;
        }
        prim->b0 = 0x7F - *t / 2;
        goto size_sel;
    case 6:
    case 7:
        if (D_800F0BA8[arg0] < 0x96) {
            if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
                D_800A3488 = D_8009B9D8;
            } else {
                D_800A3488 = D_8009B988;
            }
        } else if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            D_800A3488 = D_8009B9E0;
        } else {
            D_800A3488 = D_8009B990;
        }
        prim->r0 = 0x10;
        prim->g0 = ~D_800F0BA8[arg0];
        prim->b0 = 0xFF;
    size_sel:
        if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            *D_800A34A8 = 0x80;
            *D_800A34AC = 0x80;
            *D_800A3490 = 0x2F;
        } else {
            *D_800A34A8 = 0x20;
            *D_800A34AC = 0x80;
            *var_9 = 0x40;
            *var_10 = 0x10;
        }
        break;
    case 8:
    case 9:
        if (D_800F0BA8[arg0 - 2] < 0xC8) {
            D_800A3488 = D_8009B980;
        } else {
            D_800A3488 = D_8009B978;
        }
        prim->r0 = D_800F0BA8[arg0 - 2] * 2 / 3;
        prim->g0 = D_800F0BA8[arg0 - 2] / 2;
        prim->b0 = 0xFF;
        *D_800A34A8 = 0x100;
        *D_800A34AC = 0x100;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        prim->r0 = 0xC0;
        prim->g0 = 0x70;
        prim->b0 = 0x13;
        *D_800A34A8 *= 2;
        *D_800A34AC *= 2;
        if (D_800F0BA8[arg0] >= 8) {
            D_800A3488 = D_8009B8C8;
            n = 10 - D_800F0BA8[arg0];
            prim->r0 = n << 6;
            prim->g0 = n * 0x70 / 3;
            prim->b0 = n * 0x60 / 15;
            *D_800A3490 = 0x2E;
        } else {
            if (D_800F0BA8[arg0] >= 5) {
                D_800A3488 = D_8009B9F0;
            } else {
                D_800A3488 = D_8009B9E8;
            }
            *D_800A3490 = 0x2F;
        }
        break;
    case 16:
    case 17:
        D_800A3488 = D_8009B8C8;
        prim->r0 = 0xFF;
        prim->g0 = 0x60;
        prim->b0 = 0xFF;
        break;
    }
    *D_800A3494 =
        (u16)(((D_800A3488->clut_x >> 4) & 0x3F) + (D_800A3488->clut_y << 6));
    *D_800A3498 = D_800A3488->u;
    *D_800A34A0 = D_800A3488->v;
    *D_800A349C = *var_9 + D_800A3488->u - 1;
    *D_800A34A4 = *var_10 + D_800A3488->v - 1;
    *D_800A34B0 = ReadGeomScreen() * 1000;
    *D_800A34B4 = *D_800A34B0 / *D_800A34D0;
    *p_w =
        *D_800A34A8 * *D_800A34B4 > 0x200 ? *D_800A34A8 * *D_800A34B4 >> 8 : 6;
    *p_h =
        *D_800A34AC * *D_800A34B4 > 0x200 ? *D_800A34AC * *D_800A34B4 >> 8 : 6;
    SetPolyFT4(prim);
    prim->tpage = *D_800A3490;
    prim->clut = *D_800A3494;
    prim->u0 = *D_800A3498;
    prim->v0 = *D_800A34A0;
    prim->u1 = *D_800A349C;
    prim->v1 = *D_800A34A0;
    prim->u2 = *D_800A3498;
    prim->v2 = *D_800A34A4;
    prim->u3 = *D_800A349C;
    prim->v3 = *D_800A34A4;
    SetShadeTex(prim, 0);
    SetSemiTrans(prim, 1);
    switch (arg0) {
    case 0:
        *p_w = (u16)*p_w * 8 + (*p_w * rcos(D_800F0BA8[arg0] & 0xFFF) >> 9);
        *p_h = (u16)*p_h * 2 +
               (*p_h * rcos((D_800F0BA8[arg0] + 0x7FF) & 0xFFF) >> 11);
        prim->x0 = *D_800A34B8 - *p_w / 2;
        prim->y0 = (*D_800A34B8 >> 16) - *p_h / 2;
        prim->x1 = *D_800A34B8 + *p_w / 2;
        prim->y1 = (*D_800A34B8 >> 16) - *p_h / 2;
        prim->x2 = *D_800A34B8 - *p_w / 2;
        prim->y2 = (*D_800A34B8 >> 16) + *p_h / 2;
        prim->x3 = *D_800A34B8 + *p_w / 2;
        prim->y3 = (*D_800A34B8 >> 16) + *p_h / 2;
        break;
    case 1:
    case 2:
    case 16:
    case 17:
        var_14 = (rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25;
        *p_w = (*p_w * var_14 >> 13) / 2;
        var_15 = rsin((D_800F0BA8[arg0] << 11) / 4551) * 15;
        *p_h = (*p_h * var_15 >> 12) / 2;
        i = 0;
        p_v->vy = 0;
        p_v->vx = 0;
        p_v->vz = D_800F0BA8[arg0];
        RotMatrix(p_v, p_mat);
        dst = p_t;
        SetRotMatrix(p_mat);
        do {
            w = *p_w;
            if (!(i & 1)) {
                p_v->vx = -w;
            } else {
                p_v->vx = w;
            }
            h = *p_h;
            if (!(i & 2)) {
                p_v->vy = -h;
            } else {
                p_v->vy = h;
            }
            ApplyRotMatrix(p_v, dst);
            dst++;
        } while (++i < 4);
        prim->x0 = *D_800A34B8 + p_t[0].vx / 2;
        prim->y0 = (*D_800A34B8 >> 16) + p_t[0].vy / 4;
        prim->x1 = *D_800A34B8 + p_t[1].vx / 2;
        prim->y1 = (*D_800A34B8 >> 16) + p_t[1].vy / 4;
        prim->x2 = *D_800A34B8 + p_t[2].vx / 2;
        prim->y2 = (*D_800A34B8 >> 16) + p_t[2].vy / 4;
        prim->x3 = *D_800A34B8 + p_t[3].vx / 2;
        prim->y3 = (*D_800A34B8 >> 16) + p_t[3].vy / 4;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        *p_w = *p_w * rsin((D_800F0BA8[arg0] * 0x3FF / 10) & 0xFFF) >> 11;
        *p_h = *p_h * rsin((D_800F0BA8[arg0] * 0x3FF / 10) & 0xFFF) >> 11;
        goto quad;
    case 8:
    case 9:
        *p_w = *p_w * rsin((D_800F0BA8[arg0 - 2] / 2) & 0xFFF) >> 11;
        *p_h = *p_h * rsin((D_800F0BA8[arg0 - 2] / 2) & 0xFFF) >> 11;
        goto quad;
    case 3:
    case 4:
        *p_w = *p_w * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
        *p_h = *p_h * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
    quad:
        prim->x0 = *D_800A34B8 - *p_w;
        prim->y0 = (*D_800A34B8 >> 16) - *p_h / 2;
        prim->x1 = *p_w + *D_800A34B8;
        prim->y1 = (*D_800A34B8 >> 16) - *p_h / 2;
        prim->x2 = *D_800A34B8 - *p_w;
        prim->y2 = (*D_800A34B8 >> 16) + *p_h / 2;
        prim->x3 = *p_w + *D_800A34B8;
        prim->y3 = (*D_800A34B8 >> 16) + *p_h / 2;
        break;
    case 5:
        if (D_800F0BA8[arg0] < 0x1000) {
            *p_w = (u16)*p_w * 8 + (*p_w * rcos(D_800F0BA8[arg0] & 0xFFF) >> 9);
            *p_h = (u16)*p_h * 2 +
                   (*p_h * rcos((D_800F0BA8[arg0] + 0x7FF) & 0xFFF) >> 11);
        } else {
            *p_h = 0;
            *p_w = 0;
        }
        prim->x0 = *D_800A34B8 - *p_w / 4;
        prim->y0 = (*D_800A34B8 >> 16) - *p_h / 4;
        prim->x1 = *D_800A34B8 + *p_w / 4;
        prim->y1 = (*D_800A34B8 >> 16) - *p_h / 4;
        prim->x2 = *D_800A34B8 - *p_w / 4;
        prim->y2 = (*D_800A34B8 >> 16) + *p_h / 4;
        prim->x3 = *D_800A34B8 + *p_w / 4;
        prim->y3 = (*D_800A34B8 >> 16) + *p_h / 4;
        break;
    case 6:
    case 10:
        if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            *p_w = -*p_w;
        }
    case 7:
    case 11:
        if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            *p_w = *p_w * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
            *p_h = *p_h * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
        } else {
            *p_w = *p_w + (*p_w * rsin(D_800F0BA8[arg0] * 0x300 / 255) >> 11);
            *p_h = *p_h * rcos((D_800F0BA8[arg0] << 9) / 255) >> 12;
        }
        if (D_8009BD24.unk20_3 && D_800A3834 != 5) {
            prim->x0 = *D_800A34B8 - *p_w;
            prim->y0 = (*D_800A34B8 >> 16) - *p_h / 2;
            prim->x1 = *p_w + *D_800A34B8;
            prim->y1 = (*D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *D_800A34B8 - *p_w;
            prim->y2 = (*D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *p_w + *D_800A34B8;
            prim->y3 = (*D_800A34B8 >> 16) + *p_h / 2;
        } else if (D_800F0BA8[arg0] < 0x96) {
            prim->x0 = *D_800A34B8 - *p_w / 2;
            prim->y0 = (*D_800A34B8 >> 16) + *p_h / 2;
            prim->x1 = *D_800A34B8 - *p_w / 2;
            prim->y1 = (*D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *D_800A34B8 + *p_w / 2;
            prim->y2 = (*D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *D_800A34B8 + *p_w / 2;
            prim->y3 = (*D_800A34B8 >> 16) - *p_h / 2;
        } else {
            prim->x0 = *D_800A34B8;
            prim->y0 = (*D_800A34B8 >> 16) + *p_h / 2;
            prim->x1 = *D_800A34B8;
            prim->y1 = (*D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *p_w + *D_800A34B8;
            prim->y2 = (*D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *p_w + *D_800A34B8;
            prim->y3 = (*D_800A34B8 >> 16) - *p_h / 2;
        }
        if (arg0 < 9) {
            AddPrim(g_gpu_ot_ptr + 1, prim);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                prim++;
            }
            arg0 += 2;
            goto again;
        }
        break;
    }
    AddPrim(g_gpu_ot_ptr + 1, prim);
    if (prim - g_eff_quad_buffer_base < 0x1C1) {
        prim++;
    }
    g_eff_quad_cursor = prim;
    return 1;
}

u8 eff_SpawnParticles(s32, s32, s32);

u8 func_80066EC0(void) {
    u8 ret = eff_SpawnParticles(0, 0, 0);
    g_eff_command_phase[2] = 1;
    return ret;
}

u8 func_80066EF4(void) {
    u8 ret = eff_SpawnParticles(0, 0, 1);
    g_eff_command_phase[2] = 2;
    return ret;
}

u8 func_80066F28(void) {
    u8 ret = eff_SpawnParticles(1, 1, 0);
    g_eff_command_phase[3] = 1;
    return ret;
}

u8 func_80066F5C(void) {
    u8 ret = eff_SpawnParticles(1, 1, 1);
    g_eff_command_phase[3] = 2;
    return ret;
}

u8 func_80066F90(void) {
    u8 ret = eff_SpawnParticles(2, 1, 0);
    g_eff_command_phase[20] = 1;
    return ret;
}

u8 func_80066FC4(void) {
    u8 ret = eff_SpawnParticles(2, 1, 1);
    g_eff_command_phase[20] = 2;
    return ret;
}

u8 func_80066FF8(void) {
    u8 ret = eff_SpawnParticles(3, 0, 0);
    g_eff_command_phase[21] = 1;
    return ret;
}

u8 func_8006702C(void) {
    u8 ret = eff_SpawnParticles(3, 0, 1);
    g_eff_command_phase[21] = 2;
    return ret;
}

u8 func_80067060(void) {
    u8 ret = eff_SpawnParticles(4, 2, 0);
    g_eff_command_phase[22] = 1;
    return ret;
}

u8 func_80067094(void) {
    u8 ret = eff_SpawnParticles(4, 2, 1);
    g_eff_command_phase[22] = 2;
    return ret;
}

u8 func_800670C8(void) {
    u8 ret = eff_SpawnParticles(5, 3, 0);
    g_eff_command_phase[23] = 1;
    return ret;
}

u8 func_800670FC(void) {
    u8 ret = eff_SpawnParticles(5, 3, 1);
    g_eff_command_phase[23] = 2;
    return ret;
}

u8 func_80067130(void) {
    u8 ret = eff_SpawnParticles(6, 3, 0);
    g_eff_command_phase[24] = 1;
    return ret;
}

u8 func_80067164(void) {
    u8 ret = eff_SpawnParticles(6, 3, 1);
    g_eff_command_phase[24] = 2;
    return ret;
}

u8 func_80067198(void) {
    u8 ret = eff_SpawnParticles(7, 2, 0);
    g_eff_command_phase[25] = 1;
    return ret;
}

u8 func_800671CC(void) {
    u8 ret = eff_SpawnParticles(7, 2, 1);
    g_eff_command_phase[25] = 2;
    return ret;
}

extern SVECTOR D_800F0B78[];
extern Unk800EFC78Record g_eff_particles[][48];

/* eff_SpawnParticles -- spawns half (arg2 selects which) of the 0x30
 * g_eff_particles[arg1] particle records: each gets the D_800F0B78[arg1] vector
 * scaled per axis by its own random factor and turned by a random rotation
 * (spread chosen by mode arg0). arg2 == 0 first latches D_800A347C / D_800A3478
 * into the emitter. Returns 1. */
u8 eff_SpawnParticles(s32 arg0, s32 arg1, s32 arg2) {
    SVECTOR v;
    s32 r[6];
    MATRIX m;
    SVECTOR ang;
    VECTOR out;
    SVECTOR var_5;
    s16 base;
    s16 mask;
    s16 amask;
    s16 aoff;
    u8 count;
    s32 i;
    Unk800EFC78Record *p;

    if (arg0 < 4) {
        base = 0x41;
        mask = 0x3F;
    } else {
        base = 0x31;
        mask = 0x1F;
    }
    D_800A3438[arg1] = 0;
    gte_SetRotMatrix(D_800A3474);
    if (arg2 == 0) {
        D_800F0C10[arg1][0].unk0 = D_800A347C[0];
        D_800F0C10[arg1][0].unk4 = D_800A347C[1];
        D_800F0C10[arg1][0].unk8 = D_800A347C[2];
        D_800F0B78[arg1].vx = D_800A3478[0];
        D_800F0B78[arg1].vy = D_800A3478[1];
        D_800F0B78[arg1].vz = D_800A3478[2];
    }
    count = 0x30;
    r[5] = rand();
    if (arg0 < 2) {
        amask = 0xFF;
        aoff = 0x7F;
    } else if (arg0 < 4) {
        amask = 0xFFF;
        aoff = 0;
    } else if (arg0 < 6) {
        amask = 0x7F;
        aoff = 0x3F;
    } else if (arg0 < 8) {
        amask = 0x7F;
        aoff = 0x3F;
    }
    var_5.vx = D_800F0B78[arg1].vx;
    var_5.vy = D_800F0B78[arg1].vy;
    var_5.vz = D_800F0B78[arg1].vz;
    for (i = (count >> 1) * arg2; i < (count >> 1) * (arg2 + 1); i++) {
        r[0] = r[5] ^ rand();
        r[1] = r[0] ^ rand();
        r[2] = r[1] ^ rand();
        r[3] = r[2] ^ rand();
        r[4] = r[3] ^ rand();
        r[5] = r[4] ^ rand();
        ang.vy = r[0] = (r[0] & amask) - aoff;
        ang.vx = r[1] = (r[1] & amask) - aoff;
        ang.vz = r[2] = (r[2] & amask) - aoff;
        r[3] &= mask;
        r[4] &= mask;
        r[5] &= mask;
        v.vx = var_5.vx * (base + r[3]) / mask;
        v.vy = var_5.vy * (base + r[4]) / mask;
        v.vz = var_5.vz * (base + r[5]) / mask;
        p = &g_eff_particles[arg1][i];
        p->unk6 = i / 16;
        p->unk12 = 0;
        p->unk4 = 0;
        p->unk2 = 0;
        p->unk0 = 0;
        p->unk10 = 1;
        RotMatrix(&ang, &m);
        gte_ApplyMatrix(&m, &v, &out);
        p->unk8 = out.vx;
        p->unkA = out.vy;
        p->unkC = out.vz;
    }
    return 1;
}

u8 eff_PrepareParticles(s32, s32);
void eff_UpdateParticles(s32, s32);
u8 eff_LinkParticleQuads(s32, s32);

u8 func_800676C8(void) {
    eff_PrepareParticles(0, 0);
    eff_UpdateParticles(0, 0);
    return eff_LinkParticleQuads(0, 0);
}

u8 func_80067704(void) {
    eff_PrepareParticles(1, 1);
    eff_UpdateParticles(1, 1);
    return eff_LinkParticleQuads(1, 1);
}

u8 func_80067740(void) {
    eff_PrepareParticles(2, 1);
    eff_UpdateParticles(2, 1);
    return eff_LinkParticleQuads(2, 1);
}

u8 func_8006777C(void) {
    eff_PrepareParticles(3, 0);
    eff_UpdateParticles(3, 0);
    return eff_LinkParticleQuads(3, 0);
}

u8 func_800677B8(void) {
    eff_PrepareParticles(4, 2);
    eff_UpdateParticles(4, 2);
    return eff_LinkParticleQuads(4, 2);
}

u8 func_800677F4(void) {
    eff_PrepareParticles(5, 3);
    eff_UpdateParticles(5, 3);
    return eff_LinkParticleQuads(5, 3);
}

u8 func_80067830(void) {
    eff_PrepareParticles(6, 3);
    eff_UpdateParticles(6, 3);
    return eff_LinkParticleQuads(6, 3);
}

u8 func_8006786C(void) {
    eff_PrepareParticles(7, 2);
    eff_UpdateParticles(7, 2);
    return eff_LinkParticleQuads(7, 2);
}

extern s16 D_800EFC8A[];
extern s16 D_800F0B98[];

u8 eff_PrepareParticles(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &D_800A34EC->v800678A8;
    /* work-area fields through pointers to them, the style of the file's
     * scratchpad pointers (D_800A3490 .., set by eff_BindScratchpad) and of
     * eff_UpdateParticles's locals */
    u16 *p0 = &var_0->unk00;
    u16 *p2 = &var_0->unk02;
    u32 *p4 = &var_0->unk04;
    s16 *p6C = &var_0->unk6C;
    POLY_FT4 **p80 = &var_0->unk80;

    D_800A3724 = &var_0->unk1AC;
    *p80 = g_eff_quad_cursor;
    *p4 = 0x895440;
    *D_800A3490 = 0x2E;

    if (arg0 < 2) {
        D_800A3488 = D_8009B890;
        *p2 = 7;
        *p0 = 7;
        *D_800A34A8 = 0x40;
        *D_800A34AC = 0x20;
        D_800F0B98[arg0] = 3;
    } else if (arg0 < 4) {
        D_800A3488 = D_8009B890;
        *p2 = 7;
        *p0 = 7;
        *D_800A34A8 = 0x20;
        *D_800A34AC = 0x10;
        D_800F0B98[arg0] = 3;
    } else if (arg0 < 6) {
        s16 var_6 = D_800EFC8A[arg1 * 0x1E0] >> 3;
        var_0->unk70 = var_6;
        if (var_6 >= 4) {
            var_0->unk70 = 3;
        }
        if (D_800A34F0[arg0 - 4] != 0) {
            D_800A3488 = &D_8009B9B8[var_0->unk70];
        } else {
            D_800A3488 = &D_8009B998[var_0->unk70];
        }
        *p0 = 0x1F;
        *p2 = 0x20;
        *D_800A34A8 = 0xC0;
        *D_800A34AC = 0x30;
        *D_800A3490 = 0xF;
        D_800F0B98[arg0] = 1;
    } else if (arg0 < 8) {
        *D_800A3490 = 0x2E;
        D_800A3488 = D_8009B8B0;
        *p2 = 0xF;
        *p0 = 0xF;
        *D_800A34A8 = 0xC0;
        *D_800A34AC = 0x60;
        D_800F0B98[arg0] = 2;
    }

    *D_800A3494 =
        (((D_800A3488->clut_x >> 4) & 0x3F) + (D_800A3488->clut_y << 6)) << 16;
    *D_800A3490 <<= 16;
    *D_800A3498 = D_800A3488->u;
    *D_800A34A0 = D_800A3488->v;
    *D_800A349C = *p0 + D_800A3488->u;
    *D_800A34A4 = *p2 + D_800A3488->v;
    *D_800A34D4 = *D_800A3498 + (*D_800A34A0 << 8);
    *D_800A34D8 = *D_800A349C + (*D_800A34A0 << 8);
    *D_800A34DC = *D_800A3498 + (*D_800A34A4 << 8);
    *D_800A34E0 = *D_800A349C + (*D_800A34A4 << 8);

    gte_SetRotMatrix(D_800A3474);

    if (D_800A3438[arg1] < D_800F0B98[arg0]) {
        D_800F0C10[arg1][D_800A3438[arg1]].unk0 = D_800F0C10[arg1][0].unk0;
        D_800F0C10[arg1][D_800A3438[arg1]].unk4 = D_800F0C10[arg1][0].unk4;
        D_800F0C10[arg1][D_800A3438[arg1]].unk8 = D_800F0C10[arg1][0].unk8;
    }

    gte_ReadGeomScreen(D_800A34B0);

    *p6C = (D_800A3438[arg1] + 1) * 16;
    if (D_800A3438[arg1] < D_800F0B98[arg0] - 1) {
        D_800A3438[arg1]++;
    }
}

/* Per-frame update + draw of the g_eff_particles[arg1] records, the middle of
 * the eff_PrepareParticles / eff_UpdateParticles / eff_LinkParticleQuads trio.
 * For each live record of g_eff_particles[arg1] (state 1): step its unk12 and
 * derive r/g/b from it, take its D_800F0C10[arg1] entry less D_800A3470 through
 * the GTE rotation, set state 3 (and skip it) once r and g are both low or once
 * the squared length p_out[0..2] passes *var_1 plus a rand() term, project
 * three vertices, size the sprite by type (arg0), then emit up to three
 * POLY_FT4 quads, storing *D_800A34D0 for each in var_9[]. */
void eff_UpdateParticles(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &D_800A34EC->v800678A8;
    u32 *var_1;
    VECTOR *var_2;
    s16 *p_count;
    s16 *p_idx;
    u8 *p_r;
    u8 *p_g;
    u8 *p_b;
    Unk800F0C10Record **var_8;
    u16 *var_9;
    s32 *var_10;
    s32 *p_out;
    VECTOR *p_work;
    SVECTOR *var_13;
    s16 *var_14;
    s16 *p_n;
    POLY_FT4 **p_prim;
    Unk800EFC78Record **p_ent;
    u32 sum;
    s16 *sxy;

    D_800A3724 = &var_0->unk1AC;
    var_10 = &var_0->unk1AC;
    *var_10 = rand();
    var_1 = &var_0->unk04;
    p_out = var_0->unk10.t;
    p_work = &var_0->unk34;
    var_13 = var_0->unk44;
    var_2 = &var_0->unk5C;
    p_count = &var_0->unk6C;
    p_idx = &var_0->unk6E;
    var_14 = &var_0->unk70;
    p_r = &var_0->unk72;
    p_g = &var_0->unk73;
    p_b = &var_0->unk74;
    p_n = &var_0->unk78;
    p_prim = &var_0->unk80;
    p_ent = &var_0->unk84;
    var_8 = &var_0->unk88;
    var_9 = var_0->unk8C;

    for (*p_idx = 0; *p_idx < *p_count; (*p_idx)++) {
        *p_ent = &g_eff_particles[arg1][*p_idx];
        if ((*p_ent)->unk10 != 1) {
            continue;
        }
        (*p_ent)->unk12++;
        *var_14 = (*p_ent)->unk12;
        *p_r = (*var_14 << 2) < 0xFF ? ~(*var_14 << 2) : 0;
        *p_g = (*var_14 << 5) < 200 ? 200 - (*var_14 << 5) : 0;
        if (*p_r <= 0x50 && *p_g <= 0x20) {
            (*p_ent)->unk10 = 3;
            continue;
        }
        *p_b = (*var_14 << 5) < 100 ? 100 - (*var_14 << 5) : 0;

        *var_8 = &D_800F0C10[arg1][(*p_ent)->unk6];
        var_2->vx = (*var_8)->unk0;
        var_13[0].vx = var_2->vx - D_800A3470[0];
        var_2->vy = (*var_8)->unk4;
        var_13[0].vy = var_2->vy - D_800A3470[1];
        var_2->vz = (*var_8)->unk8;
        var_13[0].vz = var_2->vz - D_800A3470[2];

        gte_ApplyRotMatrix(var_13, p_out);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed p_out - 0x14,
         * i.e. &var_0->unk10 reached through p_out, it loads p_out as the
         * translation. Spelled &var_0->unk10: score 82; through a MATRIX *
         * local m = &var_0->unk10 (p_out = m->t): score 58. */
        gte_SetTransMatrix((u8 *)p_out - 0x14);

        var_13[1].vx = (*p_ent)->unk0;
        var_13[1].vz = (*p_ent)->unk4;
        var_13[1].vy = (*p_ent)->unk2;
        p_work->vx = (*p_ent)->unk8 / 30;
        (*p_ent)->unk0 += p_work->vx;
        var_13[0].vx = (*p_ent)->unk0;
        p_work->vz = (*p_ent)->unkC / 30;
        (*p_ent)->unk4 += p_work->vz;
        var_13[0].vz = (*p_ent)->unk4;
        p_work->vy = (*p_ent)->unkA / 30 + *var_14 * 9800 / 900;
        (*p_ent)->unk2 += p_work->vy;
        var_13[0].vy = (*p_ent)->unk2;
        var_13[2].vx = var_13[1].vx + (p_work->vx >> 1);
        var_13[2].vz = var_13[1].vz + (p_work->vz >> 1);
        var_13[2].vy = var_13[1].vy + (p_work->vy >> 1);

        gte_ldv3(&var_13[0], &var_13[1], &var_13[2]);
        p_work->vx = var_13[0].vx;
        p_work->vz = var_13[0].vz;
        p_work->vy = var_13[0].vy;
        gte_Square0(p_work, p_out);

        *var_10 ^= rand();
        if ((u32)(p_out[0] + p_out[1] + p_out[2]) >
            *var_1 + *var_10 * 3000 / 32768 * 3000) {
            (*p_ent)->unk10 = 3;
            continue;
        }

        gte_rtpt();
        gte_stsxy3(&D_800A34B8[0], &D_800A34B8[1], &D_800A34B8[2]);
        gte_stsz(D_800A34D0);

        if (*D_800A34D0 < 0) {
            continue;
        }
        D_800A34D0[1] = *D_800A34D0 ? *D_800A34D0 : 1;
        *D_800A34D0 = gpu_DepthToOtIndex(D_800A34D0[1], 0);
        if (*D_800A34D0 == 0) {
            *D_800A34D0 = 1;
        }
        *D_800A34CC = 1;
        if (*D_800A34D0 >= 0x1005 || *D_800A34CC == 0) {
            continue;
        }

        if (arg0 < 2) {
            *D_800A34B4 = *D_800A34B0 * 200 / D_800A34D0[1];
            *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x200
                              ? *D_800A34A8 * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x200
                              ? *D_800A34AC * *D_800A34B4 >> 9
                              : 1;
        } else if (arg0 < 4) {
            *D_800A34B4 = *D_800A34B0 * 200 / D_800A34D0[1];
            *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x200
                              ? *D_800A34A8 * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x200
                              ? *D_800A34AC * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C0 += *D_800A34C0 * 20 / (*var_14 * 6 + 1);
            *D_800A34C8 += *D_800A34C8 * 20 / (*var_14 * 6 + 1);
        } else if (arg0 < 6) {
            *D_800A34B4 = *D_800A34B0 * 200 / D_800A34D0[1];
            *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x200
                              ? *D_800A34A8 * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x200
                              ? *D_800A34AC * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C0 += *D_800A34C0 * *var_14 / 2;
            *D_800A34C8 += *D_800A34C8 * *var_14 / 2;
        } else if (arg0 < 8) {
            *D_800A34B4 = *D_800A34B0 * 200 / D_800A34D0[1];
            *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x200
                              ? *D_800A34A8 * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x200
                              ? *D_800A34AC * *D_800A34B4 >> 9
                              : 1;
            sxy = (s16 *)D_800A34B8;
            sxy[2] += rand() * 20 / 32768;
            sxy[3] += rand() * 10 / 32768;
        }
        if (*D_800A34C0 > *D_800A34A8 * 2) {
            *D_800A34C0 = *D_800A34A8 >> 1;
            *D_800A34C8 = *D_800A34AC >> 1;
        }
        *D_800A34BC = -*D_800A34C0;
        *D_800A34C4 = -*D_800A34C8;
        *D_800A34C8 <<= 16;
        *D_800A34C4 <<= 16;

        sum = p_out[0] + p_out[1] + p_out[2];
        if (*var_1 / 3 < sum) {
            *p_n = 1;
        } else if (*var_1 / 2 < sum) {
            *p_n = 0;
        } else {
            *p_n = 2;
        }
        /* Packed xy / uv words written through word views of the POLY_FT4, as
           the other emitters in this file do (whole-word coordinate and colour
           writes, halfword uv writes).
           SOTN: src/dra/8BEF8.c:185 @aa53500
           SOTN: src/st/cat/e_bone_ark.c:436 @aa53500 */
        for (; *p_n >= 0; (*p_n)--) {
            var_9[*p_prim - g_eff_quad_cursor] = *D_800A34D0;
            *(s32 *)&(*p_prim)->x0 =
                D_800A34B8[*p_n] + *D_800A34BC + *D_800A34C4;
            *(s32 *)&(*p_prim)->x1 =
                D_800A34B8[*p_n] + *D_800A34C0 + *D_800A34C4;
            *(s32 *)&(*p_prim)->x2 =
                D_800A34B8[*p_n] + *D_800A34BC + *D_800A34C8;
            *(s32 *)&(*p_prim)->x3 =
                D_800A34B8[*p_n] + *D_800A34C0 + *D_800A34C8;
            *(s32 *)&(*p_prim)->u0 = *D_800A34D4 + *D_800A3494;
            *(s32 *)&(*p_prim)->u1 = *D_800A34D8 + *D_800A3490;
            *(u16 *)&(*p_prim)->u2 = *D_800A34DC;
            *(u16 *)&(*p_prim)->u3 = *D_800A34E0;
            setlen(*p_prim, 9);
            if (arg0 < 2) {
                *(s32 *)&(*p_prim)->r0 = *p_r + (*p_g << 8) + 0x2E000000;
            } else if (arg0 < 4) {
                *(s32 *)&(*p_prim)->r0 =
                    *p_r + ((*p_r >> 2) << 8) + (*p_b << 16) + 0x2E000000;
            } else if (arg0 < 6) {
                if (*p_prim - g_eff_quad_buffer_base >= 0x1C1) {
                    continue;
                }
                if (D_8009BD24.unk20_0) {
                    *(s32 *)&(*p_prim)++->r0 = 0x2C242424;
                    break;
                } else if (D_800A34F0[arg0 - 4] != 0) {
                    *(s32 *)&(*p_prim)++->r0 =
                        (((rand() * 4 >> 12) + 0x3C) << 8) + 0x2C080038;
                    break;
                } else {
                    *(s32 *)&(*p_prim)++->r0 = 0x2C285A78;
                    break;
                }
            } else if (arg0 < 8) {
                if (*p_prim - g_eff_quad_buffer_base >= 0x1C1) {
                    break;
                }
                *(s32 *)&(*p_prim)++->r0 = 0x2EFF8080;
                break;
            }
            if (*p_prim - g_eff_quad_buffer_base < 0x1C1) {
                (*p_prim)++;
            }
        }
    }
}

u8 eff_LinkParticleQuads(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &D_800A34EC->v800678A8;
    s16 *p_idx = &var_0->unk6E;
    POLY_FT4 **p_end = &var_0->unk7C;
    POLY_FT4 **p_cur = &var_0->unk80;
    u16 *var_4 = var_0->unk8C;
    POLY_FT4 *var_5;
    POLY_FT4 *var_6;
    s32 count;
    s32 ret;

    D_800A3724 = &var_0->unk1AC;
    var_6 = g_eff_quad_cursor;
    var_5 = *p_cur;
    count = var_5 - var_6;

    if (count != 0) {
        *p_cur = var_6;
        *p_end = var_5;
        ret = 1;
        *p_idx = 0;

        if (*p_cur < *p_end) {
            u32 *p_b;
            do {
                D_800A34E4 = g_gpu_ot_ptr + var_4[*p_idx];
                p_b = &(*p_cur)->tag;
                D_800A34E8 = p_b;
                *p_b = (*p_b & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);

                *D_800A34E4 =
                    ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);

                (*p_cur)++;
                (*p_idx)++;
            } while (*p_cur < *p_end);
        }
        g_eff_quad_cursor = *p_end;
    } else {
        ret = 0;
    }

    return ret;
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 g_eff_billboard_draw_count = 0;
s32 D_800A32BC = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
POLY_FT4 *g_eff_quad_buffer_base;
s32 *D_800A3724;
POLY_FT4 *g_eff_quad_cursor;
