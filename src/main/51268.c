/* 147 game functions. .text 0x80060A68 (ROM 0x51268). Start boundary: PHASE
 * (rodata-align section 9), moved by the per-file gp model (Q65). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>
#include <psxsdk/gtemac.h>

/* func_80060A68 .. func_80060E38 belong here by the per-file gp evidence
 * (Q65). */

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
/* Two s16 slots at 0x800A34F0, indexed as one array (func_800678A8 and
 * func_80067D14 read the pair through one indexed access). */
static s16 D_800A34F0[2];
static s32 D_800A34F4; /* not named by any code or data: size from the gap */
static s32 D_800A34F8;
static Unk800A34FCRec *D_800A34FC;
static s32 D_800A3500;
/* not named by any code or data: size from the gap */
static s32 D_800A3504[2];
static s16 D_800A350C[2];
static s16 D_800A3510[2];
static s32 D_800A3514;
static s32 D_800A3518;
static Unk8006E49CRec *D_800A351C;
static Unk8006E49CRec *D_800A3520;
static Unk8009BD24Block *D_800A3524;
static s16 D_800A3528;
static s32 D_800A352C;

/* Runs the command in the block D_800A3468 points at (idx = the block's low
 * halfword, unk00.h): copies the three words at unk0C and the three halfwords
 * at unk10 into unk20 / unk18, points D_800A347C / D_800A3478 at the copies,
 * clears D_800F10D0[idx], calls the D_8009BA7C entry
 * D_8009BA60[idx] + D_800F10D0[idx], stores the result through unk14, and sets
 * D_800A32BC to 0xA when bit 21 of unk00.w is set. Member reads (not
 * *(u16 *)D_800A3468) let the second idx read (the D_8009BA60 index) rise
 * above the D_800A347C store, as in the target. */
void func_80060A68(void) {
    extern s32 D_800A32BC;

    s32 result;

    D_800F10D0[D_800A3468->unk00.h] = 0;
    D_800A3468->unk20[0] = D_800A3468->unk0C[0];
    D_800A3468->unk20[1] = D_800A3468->unk0C[1];
    D_800A3468->unk20[2] = D_800A3468->unk0C[2];
    D_800A3468->unk18[0] = D_800A3468->unk10[0];
    D_800A3468->unk18[1] = D_800A3468->unk10[1];
    D_800A3478 = D_800A3468->unk18;
    D_800A3468->unk18[2] = D_800A3468->unk10[2];
    D_800A347C = D_800A3468->unk20;

    result = ((s32(*)(void))D_8009BA7C[D_8009BA60[D_800A3468->unk00.h] +
                                       D_800F10D0[D_800A3468->unk00.h]])();
    *D_800A3468->unk14 = result;

    if (D_800A3468->unk00.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}

void func_80060B70(void) {

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
    func_80061FAC(dst16, dst_s32, D_800A3474);

    idx = D_800A3468->unk00.h;
    result = ((s32(*)(void))D_8009BA7C[D_8009BA60[idx] + D_800F10D0[idx]])();

    *D_800A3468->unk14 = result;
}

extern u8 D_800F1150[0x1C];

void func_80060C60(void) {
    s32 i = 0;
    s32 *p = D_800F10D0;
    do {
        *p = 0;
        D_800F1150[i] = 0;
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

s32 func_80060CB8(s32 arg0, s32 arg1) {
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
    func_80060C60();
    srand(rand());
    ret = arg1 + 0x4650;
    D_800A3420 = (POLY_FT4 *)arg1;
    D_800A3424 = (POLY_FT4 *)ret;
    return ret + 0x4650;
}

extern POLY_FT4 *D_800A3720;

void func_80060E04(s32 arg0) {
    D_800A37D4 = arg0 != 0 ? D_800A3424 : D_800A3420;
    D_800A3720 = D_800A37D4;
}

#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)

void func_80060E38(s16 *arg0, s32 *arg1) {
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

extern s32 func_8005C2A8(Unk8005C2A8Pack *, s16, s32);
s32 printf(s32 *, s32);
extern void AddPrim(void *, void *);
void func_80060A68(void);
void func_80060B70(void);
void func_80060C60(void);
void func_80060E38(s16 *arg0, s32 *arg1);

/* the message, alignment and an empty trailing string */
const char D_800158E0[24] = "eff prim over :%d \n";

void func_80061064(s16 *a0, s32 *a1) {
    s32 temp_a1;
    s32 i;
    func_80060E38(a0, a1);
    i = 0;
    do {
        D_800A3468->unk14 = &D_800F1150[i];
        if (D_800F1150[i] != 0) {
            D_800A3468->unk00.w = i;
            func_80060B70();
        }
        i += 1;
    } while (i < 0x1C);
    if (D_800A32BC >= 2) {
        D_800A32BC -= 1;
        func_80041E10(&D_800F1140, D_800A3464);
    } else if (D_800A32BC == 1) {
        func_800421A4();
        D_800A32BC = 0;
    }
    temp_a1 = D_800A37D4 - D_800A3720;
    if (temp_a1 >= 0x1C2) {
        printf(&D_800158E0, temp_a1 - 0x1C2);
    }
}

void func_80061178(void) {
    func_80060C60();
    func_800421A4();
    D_800A32BC = 0;
}

extern Unk1F800000Unk00 D_800F116C;

void func_800611A4(s32 *arg0, s16 *arg1) {
    s16 svec[3];
    s32 *p;
    s16 *q = arg1;
    svec[0] = *q++;
    svec[1] = *q++;
    D_800A3468 = &D_800F116C;
    svec[2] = *q;
    D_800F116C.unk10 = svec;
    D_800F116C.unk0C = arg0;
    D_800F116C.unk14 = &D_800F1150[26];
    D_800F116C.unk00.w = 0x21001A;
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFFFFEF;
}

void func_80061250(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    if (D_800F1150[9] != 0 && D_800F1150[10] != 0) {
        D_800F1150[10] = 0;
        D_800F1150[9] = 0;
    }
    if (D_800F1150[9] == 0) {
        D_800A3468->unk14 = &D_800F1150[9];
        D_800A3468->unk00.w = 0x210009;
    } else if (D_800F1150[10] == 0) {
        D_800F116C.unk14 = &D_800F1150[10];
        D_800F116C.unk00.w = 0x21000A;
    }
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0060;
}

s32 func_8006133C(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[4];
    D_800F116C.unk00.w = 0x210004;
    func_80060A68();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 1;
}

s32 func_800613C8(s32 *a0) {
    s32 *var_0 = a0;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[11];
    D_800F116C.unk00.w = 0x21000B;
    func_80060A68();
    D_800F1140.vx = *var_0++;
    D_800F1140.vy = *var_0++;
    D_800F1140.vz = *var_0++;
    D_800A3464 = 0x8080FF;
    return 16;
}

s32 func_80061454(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[11];
    D_800F116C.unk00.w = 0x29000B;
    func_80060A68();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 8;
}

s32 func_800614E0(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[11];
    D_800F116C.unk00.w = 0x31000B;
    func_80060A68();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 5;
}

void func_8006156C(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    if (D_800F1150[5] != 0 && D_800F1150[6] != 0) {
        D_800F1150[6] = 0;
        D_800F1150[5] = 0;
    }
    if (D_800F1150[5] == 0) {
        D_800A3468->unk14 = &D_800F1150[5];
        D_800A3468->unk00.w = 0x210005;
    } else if (D_800F1150[6] == 0) {
        D_800F116C.unk14 = &D_800F1150[6];
        D_800F116C.unk00.w = 0x210006;
    }
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF8080;
}

void func_80061658(s32 *arg0, s32 arg1) {
    /* FAKE: local pointer alias to D_800F116C keeps the address live in $a0
     * across the switch; direct global: score 5 (as in func_80061710) */
    Unk1F800000Unk00 *v1 = &D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = v1;
    D_800F116C.unk0C = arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        q = &D_800F1150[12];
        break;
    case 1:
        val = 0x21000D;
        q = &D_800F1150[13];
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F116C.unk14 = q;
    v1->unk00.w = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x10FFFF;
}

void func_80061710(s32 *arg0, s32 arg1) {
    /* FAKE: local pointer alias to D_800F116C keeps the address live in $a0
     * across the switch; direct global: score 5 */
    Unk1F800000Unk00 *v1 = &D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = v1;
    D_800F116C.unk0C = arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        q = &D_800F1150[14];
        break;
    case 1:
        val = 0x21000F;
        q = &D_800F1150[15];
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F116C.unk14 = q;
    v1->unk00.w = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x10FF10;
}

void func_800617C8(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    if (D_800F1150[16] != 0 && D_800F1150[17] != 0) {
        D_800F1150[17] = 0;
        D_800F1150[16] = 0;
    }
    if (D_800F1150[16] == 0) {
        D_800A3468->unk14 = &D_800F1150[16];
        D_800A3468->unk00.w = 0x210010;
    } else if (D_800F1150[17] == 0) {
        D_800F116C.unk14 = &D_800F1150[17];
        D_800F116C.unk00.w = 0x210011;
    }
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xC06013;
}

void func_800618B4(s32 *arg0, s16 *arg1) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    D_800F116C.unk10 = arg1;
    if (D_800F1150[2] != 0 && D_800F1150[3] != 0) {
        D_800F1150[3] = 0;
        D_800F1150[2] = 0;
    }
    if (D_800F1150[2] == 0) {
        D_800A3468->unk14 = &D_800F1150[2];
        D_800A3468->unk00.w = 0x210002;
    } else if (D_800F1150[3] == 0) {
        D_800F116C.unk14 = &D_800F1150[3];
        D_800F116C.unk00.w = 0x210003;
    }
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0000;
}

void func_800619A4(s32 *a0) {
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[8];
    D_800F116C.unk00.w = 0x10008;
    func_80060A68();
}

void func_800619F0(s32 *a0) {
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = a0;
    D_800F116C.unk14 = &D_800F1150[7];
    D_800F116C.unk00.w = 0x10007;
    func_80060A68();
}

void func_80061A3C(s32 *a0, s16 a1, s32 a2, s32 a3) {
    s16 sp[4];

    D_800A3468 = &D_800F116C;
    sp[2] = 0;
    sp[0] = 0;
    sp[1] = a1;
    D_800F116C.unk0C = a0;
    D_800F116C.unk10 = sp;
    if (a3 == 0) {
        D_800F116C.unk14 = &D_800F1150[0];
        D_800F116C.unk00.w = a2 + 0x10000;
    } else {
        D_800F116C.unk14 = &D_800F1150[1];
        D_800F116C.unk00.w = a2 + 0x10001;
    }
    func_80060A68();
}

void func_80061ACC(s32 *arg0, s16 *arg1) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    D_800F116C.unk10 = arg1;
    if (D_800F1150[20] != 0 && D_800F1150[21] != 0) {
        D_800F1150[21] = 0;
        D_800F1150[20] = 0;
    }
    if (D_800F1150[20] == 0) {
        D_800A3468->unk14 = &D_800F1150[20];
        D_800A3468->unk00.w = 0x210014;
        func_80060A68();
        D_800A3468->unk14 = &D_800F1150[7];
        D_800A3468->unk00.w = 0x10007;
    } else if (D_800F1150[21] == 0) {
        D_800F116C.unk14 = &D_800F1150[21];
        D_800A3468->unk00.w = 0x210015;
        func_80060A68();
        D_800A3468->unk14 = &D_800F1150[7];
        D_800A3468->unk00.w = 0x10007;
    }
    func_80060A68();
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

    D_800A3468 = &D_800F116C;
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
    if (D_800F1150[22] != 0 && D_800F1150[23] != 0) {
        D_800F1150[23] = 0;
        D_800F1150[22] = 0;
    }
    if (D_800F1150[22] == 0) {
        D_800A3468->unk14 = &D_800F1150[22];
        D_800A3468->unk00.w = 0x10016;
        D_800A34F0[0] = arg2;
    } else if (D_800F1150[23] == 0) {
        D_800A3468->unk14 = &D_800F1150[23];
        D_800A3468->unk00.w = 0x10017;
        D_800A34F0[1] = arg2;
    }
    func_80060A68();
}

void func_80061D74(s32 *arg0, s16 arg1) {
    SVECTOR sp10;
    SVECTOR sp18;
    VECTOR sp20;
    MATRIX sp30;
    s32 sp50;

    D_800A3468 = &D_800F116C;
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
    if (D_800F1150[24] != 0 && D_800F1150[25] != 0) {
        D_800F1150[25] = 0;
        D_800F1150[24] = 0;
    }
    if (D_800F1150[24] == 0) {
        D_800A3468->unk14 = &D_800F1150[24];
        D_800A3468->unk00.w = 0x10018;
    } else if (D_800F1150[25] == 0) {
        D_800A3468->unk14 = &D_800F1150[25];
        D_800A3468->unk00.w = 0x10019;
    }
    func_80060A68();
}

void func_80061EC0(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F116C.unk0C = arg0;
    if (D_800F1150[18] != 0 && D_800F1150[19] != 0) {
        D_800F1150[19] = 0;
        D_800F1150[18] = 0;
    }
    if (D_800F1150[18] == 0) {
        D_800A3468->unk14 = &D_800F1150[18];
        D_800A3468->unk00.w = 0x210012;
    } else if (D_800F1150[19] == 0) {
        D_800F116C.unk14 = &D_800F1150[19];
        D_800F116C.unk00.w = 0x210013;
    }
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF00FF;
}

extern VECTOR D_8009BB74;

void func_80061FAC(s16 *a0, s32 *a1, MATRIX *a2) {
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

extern s32 D_800A32B8;

void func_80062020(Unk800F1198Record *arg0) {
    s32 i;

    D_800A32B8 = 0;
    for (i = 0; arg0[i].unk0 & 1; i++) {
        D_800F1198[i].unk0 = arg0[i].unk0;
        D_800F1198[i].unk4 = arg0[i].unk4;
        D_800F1198[i].unk8 = arg0[i].unk8;
    }
    D_800F1198[i].unk0 = D_800F1198[i].unk4 = D_800F1198[i].unk8 = 0;
}

/* Draw the D_800F1198 effect particles (see func_80062020): per record until
   the terminator, pick a sprite size and an animation frame from the record's
   type, rotate/translate its position, project it, and when it lands in range
   emit one textured POLY_FT4 billboard and link it into the OT at its depth. */
void func_800620B8(s16 *arg0, s32 *arg1) {
    extern s32 D_800A32B8;
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

    func_80060E38(arg0, arg1);
    var_9 = D_800A3468;
    dst16 = D_800A346C;
    dst16[0] = var_9->unk04[0];
    prim = D_800A37D4;
    dst16[1] = var_9->unk04[1];
    dst16[2] = var_9->unk04[2];
    dst32 = D_800A3470;
    /* the matrix func_80061FAC builds from the angles at arg0 */
    rot = D_800A3474;
    dst32[0] = var_9->unk08[0];
    base = &D_800A34EC->v800620B8;
    dst32[1] = var_9->unk08[1];
    dst32[2] = var_9->unk08[2];
    D_800A32B8++;
    func_80061FAC(dst16, dst32, rot);
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
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
        switch (D_800F1198[i].unk4 & 7) {
        case 3:
            *D_800A34A8 = 0x151;
            *D_800A34AC = 0xA8;
            goto sel_a;
        case 0:
            *D_800A34A8 = 0x1C2;
            *D_800A34AC = 0xE1;
        sel_a:
            D_800A348C = D_800A3488 = &D_8009BA00[(u32)D_800A32B8 % 6];
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
            D_800A348C = D_800A3488 = &D_8009BA00[6 + (D_800A32B8 & 3)];
            if (D_8009BD24.unk20_0) {
                D_800A348C = &D_8009BA00[11];
            }
            *D_800A349C = D_800A3488->u + 0xF;
            *D_800A34A4 = D_800A3488->v + 0x13;
            break;
        }
        v->vx = D_800F1198[i].unk0 / 2 - D_800A3470[0];
        v->vy = D_800F1198[i].unk4 / 8 - D_800A3470[1];
        v->vz = D_800F1198[i].unk8 - D_800A3470[2];
        ApplyRotMatrixLV(v, var_3);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the address 0x14
         * below var_3, it loads var_3 as the translation (there is no whole
         * MATRIX here). Spelled (MATRIX *)base: score 65. */
        SetTransMatrix((MATRIX *)((u8 *)var_3 - 0x14));
        RotTransPers(var_5, D_800A34B8, var_6, D_800A34CC);
        gte_stsz(D_800A34D0);
        *z = func_80052C28(*D_800A34D0, 0);
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
            if (prim - D_800A3720 < 0x1C1) {
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
    D_800A37D4 = prim;
}

s32 func_8006288C(void) {
    s32 *pos;
    s16 *rot;
    s32 i;
    s32 mask;

    D_800F10D0[26] = 1;
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
    prim = D_800A37D4;
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
            *D_800A34D0 = func_80052C28(*D_800A34D0, 0);
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
                if (prim - D_800A3720 < 0x1C1) {
                    prim++;
                }
            }
        }
        D_800F0C04[i]++;
        if (D_800F0C04[i] >= 7) {
            D_800A3460 &= ~(1 << i);
        }
    }
    if (D_800A37D4 != prim) {
        /* FAKE: `end` keeps the fill position and prim walks the same buffer
         * again to link each quad; with a fresh cursor local the registers
         * rotate: score 37. */
        end = prim;
        for (prim = D_800A37D4, k = 0; prim < end; prim++, k++) {
            AddPrim(g_gpu_ot_ptr + var_3[k], prim);
        }
        D_800A37D4 = end;
        return 1;
    }
    if (D_800A3460 == 0) {
        D_800F10D0[26] = 0;
        return 0;
    }
    return D_800A3460;
}

/* Spawn a flare: claim the first free slot bit in D_800A3448 (of 12), copy
   the position at *D_800A347C into its D_800F0E38 record and reset its age.
   With every slot taken it writes slot 12 (0x800F0EC8, the next table) --
   the original has no guard. */
s32 func_80062FEC(void) {
    s32 i;
    s32 bit;
    s32 *src;

    D_800F10D0[8] = 1;
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

/* Draw the up-to-12 flare slots func_80062FEC spawns: per live slot i (bit i
   of D_800A3448, age D_800F0BEC[i], world position D_800F0E38[i]) emit two
   textured POLY_FT4 billboards -- j == 0 the flare itself, j == 1 a halo that
   bobs above it -- each rotated/translated relative to the camera, projected,
   sized from its depth and animated by its age, then linked into the OT at its
   depth. A slot whose age reaches 16 clears its bit. Returns nonzero while any
   slot is still live. */
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
    prim = D_800A37D4;
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
                *z = func_80052C28(*D_800A34D0, 0);
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
                    if (prim - D_800A3720 < 0x1C1) {
                        prim++;
                    }
                }
            } else {
                D_800A3448 &= 0xFFFF - (1 << i);
            }
        }
        D_800F0BEC[i]++;
    }
    D_800A37D4 = prim;
    return D_800A3448 != 0;
}

u8 func_80063BD0(s32);

u8 func_80063AF0(void) {
    D_800F10D0[0] = 1;
    D_800A345C[0] = (D_800A3468->unk00.w >> 17) & 3;
    return func_80063BD0(0);
}

u8 func_80063B34(void) {
    D_800F10D0[1] = 1;
    D_800A345C[1] = (D_800A3468->unk00.w >> 17) & 3;
    return func_80063BD0(1);
}

s32 func_80063E10(s32);

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
    /* FAKE: named intermediate - reading the D_800A3454[idx] word before
     * `1 << i` hoists its address ahead of the constant 1; read in the test or
     * after mask: score 6 */
    s32 bits;
    s32 mask;
    s32 i;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        for (i = 0; i < D_800A344C[idx]; i++) {
            bits = D_800A3454[idx];
            mask = 1 << i;
            if (!(bits & mask)) {
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
    func_800644FC(&count, mats, arg0);
    D_800A3488 = &D_8009B920[*D_800A3480];
    prim = D_800A37D4;
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
        *D_800A34D0 = func_80052C28(*D_800A34D0 - 50, 0);
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
        if (prim - D_800A3720 < 0x1C1) {
            prim++;
        }
    }
    /* FAKE: `end` keeps the fill position and prim walks the same buffer again
     * to link each quad; with a fresh tail cursor sxy and prim swap s2/s3:
     * score 36. */
    end = prim;
    for (prim = D_800A37D4, k = 0; prim < end; prim++, k++) {
        D_800A34E8 = &prim->tag;
        D_800A34E4 = g_gpu_ot_ptr + var_4[k];
        *D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
        *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
    }
    D_800A37D4 = end;
    return 1;
}

/* func_800644FC -- rotates one matrix per enabled bit: for every i < *count
 * whose bit is set in D_800A3454[idx], RotMatrix(&D_800F1000[idx][i], &m[i]).
 */
void func_800644FC(s32 *count, MATRIX *m, s32 idx) {
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

/* func_800645B0 -- for each group of four slots (bits of D_800A3444), claim
 * the first free slot: set its position D_800F0D78[slot] to the position at
 * D_800A347C jittered by (rand() & 0xFF) - 0x7F per axis, give it a random
 * 0..7 in D_800F0BCC, and mark its bit. */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;

    D_800F10D0[7] = 1;
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

/* Draw the up-to-16 effect slots func_800645B0 spawns: per live slot (bit i
   of D_800A3444) step its counter D_800F0BCC[i]; while counter / 4 is below
   7, project the slot's world position through D_800A3474 and, when its depth
   is in range, emit one textured, semi-transparent POLY_FT4 billboard sized by
   the screen depth and step the counter again; once counter / 4 reaches 7 the
   slot is retired. Finally link every new quad into the OT at its depth.
   Returns 1 when any slot is still live, else 0. */
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
    prim = D_800A37D4;
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
            *var_11 = func_80052C28(*D_800A34D0, 0);
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
            if (prim - D_800A3720 < 0x1C1) {
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
    for (prim = D_800A37D4; prim < *end; prim++, var_1++) {
        D_800A34E4 = g_gpu_ot_ptr + *var_1;
        D_800A34E8 = &prim->tag;
        *D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
        *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
    }
    D_800A37D4 = *end;
    return 1;
}

void func_80064E90(void) {
    D_800F0CA0[0].unk0 = D_800A347C[0];
    D_800F0CA0[0].unk4 = D_800A347C[1];
    D_800F0CA0[0].unk8 = D_800A347C[2];
    D_800F10D0[4] = 1;
    D_800F0BA8[0] = 0;
}

void func_80064ED8(void) {
    D_800F0CA0[1].unk0 = D_800A347C[0];
    D_800F0CA0[1].unk4 = D_800A347C[1];
    D_800F0CA0[1].unk8 = D_800A347C[2];
    D_800F10D0[5] = 1;
    D_800F0BA8[1] = 0;
}

void func_80064F20(void) {
    D_800F0CA0[2].unk0 = D_800A347C[0];
    D_800F0CA0[2].unk4 = D_800A347C[1];
    D_800F0CA0[2].unk8 = D_800A347C[2];
    D_800F10D0[6] = 1;
    D_800F0BA8[2] = 0;
}

s32 func_80064F68(void) {
    D_800F0CA0[3].unk0 = D_800A347C[0];
    D_800F0CA0[3].unk4 = D_800A347C[1];
    D_800F0CA0[3].unk8 = D_800A347C[2];
    D_800F10D0[9] = 1;
    D_800F0BA8[3] = 0x40;
    return 1;
}

s32 func_80064FB4(void) {
    D_800F0CA0[4].unk0 = D_800A347C[0];
    D_800F0CA0[4].unk4 = D_800A347C[1];
    D_800F0CA0[4].unk8 = D_800A347C[2];
    D_800F10D0[10] = 1;
    D_800F0BA8[4] = 0x40;
    return 1;
}

s32 func_80065000(void) {
    D_800F0CA0[5].unk0 = D_800A347C[0];
    D_800F0CA0[5].unk4 = D_800A347C[1];
    D_800F0CA0[5].unk8 = D_800A347C[2];
    D_800F10D0[11] = 1;
    D_800F0BA8[5] = 0;
    D_800A3440 = (D_800A3468->unk00.w >> 19) & 3;
    return 1;
}

void func_8006505C(void) {
    D_800F0CA0[6].unk0 = D_800A347C[0];
    D_800F0CA0[6].unk4 = D_800A347C[1];
    D_800F0CA0[6].unk8 = D_800A347C[2];
    D_800F10D0[12] = 1;
    D_800F0BA8[6] = 0;
}

void func_800650A4(void) {
    D_800F0CA0[7].unk0 = D_800A347C[0];
    D_800F0CA0[7].unk4 = D_800A347C[1];
    D_800F0CA0[7].unk8 = D_800A347C[2];
    D_800F10D0[13] = 1;
    D_800F0BA8[7] = 0;
}

void func_800650EC(void) {
    D_800F0CA0[10].unk0 = D_800A347C[0];
    D_800F0CA0[10].unk4 = D_800A347C[1];
    D_800F0CA0[10].unk8 = D_800A347C[2];
    D_800F10D0[14] = 1;
    D_800F0BA8[10] = 0;
}

void func_80065134(void) {
    D_800F0CA0[11].unk0 = D_800A347C[0];
    D_800F0CA0[11].unk4 = D_800A347C[1];
    D_800F0CA0[11].unk8 = D_800A347C[2];
    D_800F10D0[15] = 1;
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
    D_800F10D0[16] = 1;
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
    D_800F10D0[17] = 1;
    D_800F0BA8[15] = 0;
}

void func_80065264(void) {
    D_800F0CA0[16].unk0 = D_800A347C[0];
    D_800F0CA0[16].unk4 = D_800A347C[1];
    D_800F0CA0[16].unk8 = D_800A347C[2];
    D_800F10D0[18] = 1;
    D_800F0BA8[16] = 0;
}

void func_800652AC(void) {
    D_800F0CA0[17].unk0 = D_800A347C[0];
    D_800F0CA0[17].unk4 = D_800A347C[1];
    D_800F0CA0[17].unk8 = D_800A347C[2];
    D_800F10D0[19] = 1;
    D_800F0BA8[17] = 0;
}

u8 func_80065800(s32);

u8 func_800652F4(void) {
    u8 v0 = func_80065800(0);
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
    u8 v0 = func_80065800(1);
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
    u8 v0 = func_80065800(2);
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
    u8 v0 = func_80065800(3);
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
    u8 v0 = func_80065800(4);
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
    v0 = func_80065800(5);
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
    u8 v0 = func_80065800(6);
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
    u8 v0 = func_80065800(7);
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
    u8 v0 = func_80065800(0xA);
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
    u8 v0 = func_80065800(0xB);
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
    func_80065800(0xC);
    v1 = &D_800F0BA8[12];
    *v1 = *v1 + 1;
    v0 = func_80065800(0xE);
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
    func_80065800(0xD);
    *s0 = *s0 + 1;
    v0 = func_80065800(0xF);
    D_800F0BA8[15] = D_800F0BA8[15] + 1;
    if ((s16)*s0 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}

u8 func_80065760(void) {
    u8 v0 = func_80065800(0x10);
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
    u8 v0 = func_80065800(0x11);
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
u8 func_80065800(s32 arg0) {
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
    /* FAKE: named intermediate - *p_w read once before the corner sign test
     * (inline: score 7) */
    s32 w;
    /* FAKE: named intermediate - the factor multiplied into *p_w whole
     * (inline: score 10) */
    s32 var_14;
    /* FAKE: named intermediate - the factor multiplied into *p_h whole
     * (inline: score 9) */
    s32 var_15;
    /* FAKE: named intermediate - *p_h read once before the corner sign test
     * (inline: score 88) */
    s32 h;
    s16 *t;
    /* FAKE: pointer alias of D_800F0BA8 - case 10/11's base address in its own
     * register ahead of the index shift */
    s16 *tbl;
    s32 i;

    var_0 = &D_800A34EC->v80065800;
    prim = D_800A37D4;
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
        tbl = D_800F0BA8;
        t = tbl + arg0;
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
            if (prim - D_800A3720 < 0x1C1) {
                prim++;
            }
            arg0 += 2;
            goto again;
        }
        break;
    }
    AddPrim(g_gpu_ot_ptr + 1, prim);
    if (prim - D_800A3720 < 0x1C1) {
        prim++;
    }
    D_800A37D4 = prim;
    return 1;
}

u8 func_80067200(s32, s32, s32);

u8 func_80066EC0(void) {
    u8 ret = func_80067200(0, 0, 0);
    D_800F10D0[2] = 1;
    return ret;
}

u8 func_80066EF4(void) {
    u8 ret = func_80067200(0, 0, 1);
    D_800F10D0[2] = 2;
    return ret;
}

u8 func_80066F28(void) {
    u8 ret = func_80067200(1, 1, 0);
    D_800F10D0[3] = 1;
    return ret;
}

u8 func_80066F5C(void) {
    u8 ret = func_80067200(1, 1, 1);
    D_800F10D0[3] = 2;
    return ret;
}

u8 func_80066F90(void) {
    u8 ret = func_80067200(2, 1, 0);
    D_800F10D0[20] = 1;
    return ret;
}

u8 func_80066FC4(void) {
    u8 ret = func_80067200(2, 1, 1);
    D_800F10D0[20] = 2;
    return ret;
}

u8 func_80066FF8(void) {
    u8 ret = func_80067200(3, 0, 0);
    D_800F10D0[21] = 1;
    return ret;
}

u8 func_8006702C(void) {
    u8 ret = func_80067200(3, 0, 1);
    D_800F10D0[21] = 2;
    return ret;
}

u8 func_80067060(void) {
    u8 ret = func_80067200(4, 2, 0);
    D_800F10D0[22] = 1;
    return ret;
}

u8 func_80067094(void) {
    u8 ret = func_80067200(4, 2, 1);
    D_800F10D0[22] = 2;
    return ret;
}

u8 func_800670C8(void) {
    u8 ret = func_80067200(5, 3, 0);
    D_800F10D0[23] = 1;
    return ret;
}

u8 func_800670FC(void) {
    u8 ret = func_80067200(5, 3, 1);
    D_800F10D0[23] = 2;
    return ret;
}

u8 func_80067130(void) {
    u8 ret = func_80067200(6, 3, 0);
    D_800F10D0[24] = 1;
    return ret;
}

u8 func_80067164(void) {
    u8 ret = func_80067200(6, 3, 1);
    D_800F10D0[24] = 2;
    return ret;
}

u8 func_80067198(void) {
    u8 ret = func_80067200(7, 2, 0);
    D_800F10D0[25] = 1;
    return ret;
}

u8 func_800671CC(void) {
    u8 ret = func_80067200(7, 2, 1);
    D_800F10D0[25] = 2;
    return ret;
}

extern SVECTOR D_800F0B78[];
extern Unk800EFC78Record D_800EFC78[][48];

/* func_80067200 -- spawns half (arg2 selects which) of the 0x30
 * D_800EFC78[arg1] particle records: each gets the D_800F0B78[arg1] vector
 * scaled per axis by its own random factor and turned by a random rotation
 * (spread chosen by mode arg0). arg2 == 0 first latches D_800A347C / D_800A3478
 * into the emitter. Returns 1. */
u8 func_80067200(s32 arg0, s32 arg1, s32 arg2) {
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
        p = &D_800EFC78[arg1][i];
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

u8 func_800678A8(s32, s32);
void func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);

u8 func_800676C8(void) {
    func_800678A8(0, 0);
    func_80067D14(0, 0);
    return func_80068D88(0, 0);
}

u8 func_80067704(void) {
    func_800678A8(1, 1);
    func_80067D14(1, 1);
    return func_80068D88(1, 1);
}

u8 func_80067740(void) {
    func_800678A8(2, 1);
    func_80067D14(2, 1);
    return func_80068D88(2, 1);
}

u8 func_8006777C(void) {
    func_800678A8(3, 0);
    func_80067D14(3, 0);
    return func_80068D88(3, 0);
}

u8 func_800677B8(void) {
    func_800678A8(4, 2);
    func_80067D14(4, 2);
    return func_80068D88(4, 2);
}

u8 func_800677F4(void) {
    func_800678A8(5, 3);
    func_80067D14(5, 3);
    return func_80068D88(5, 3);
}

u8 func_80067830(void) {
    func_800678A8(6, 3);
    func_80067D14(6, 3);
    return func_80068D88(6, 3);
}

u8 func_8006786C(void) {
    func_800678A8(7, 2);
    func_80067D14(7, 2);
    return func_80068D88(7, 2);
}

extern s16 D_800EFC8A[];
extern s16 D_800F0B98[];

u8 func_800678A8(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &D_800A34EC->v800678A8;
    /* work-area fields through pointers to them, the style of the file's
     * scratchpad pointers (D_800A3490 .., set by func_80060E38) and of
     * func_80067D14's locals */
    u16 *p0 = &var_0->unk00;
    u16 *p2 = &var_0->unk02;
    u32 *p4 = &var_0->unk04;
    s16 *p6C = &var_0->unk6C;
    POLY_FT4 **p80 = &var_0->unk80;

    D_800A3724 = &var_0->unk1AC;
    *p80 = D_800A37D4;
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

/* Per-frame update + draw of the D_800EFC78[arg1] records, the middle of
 * the func_800678A8 / func_80067D14 / func_80068D88 trio. For each live record
 * of D_800EFC78[arg1] (state 1): step its unk12 and derive r/g/b from it, take
 * its D_800F0C10[arg1] entry less D_800A3470 through the GTE rotation, set
 * state 3 (and skip it) once r and g are both low or once the squared length
 * p_out[0..2] passes *var_1 plus a rand() term, project three vertices, size
 * the sprite by type (arg0), then emit up to three POLY_FT4 quads, storing
 * *D_800A34D0 for each in var_9[]. */
void func_80067D14(s32 arg0, s32 arg1) {
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
        *p_ent = &D_800EFC78[arg1][*p_idx];
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
        *D_800A34D0 = func_80052C28(D_800A34D0[1], 0);
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
            var_9[*p_prim - D_800A37D4] = *D_800A34D0;
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
                if (*p_prim - D_800A3720 >= 0x1C1) {
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
                if (*p_prim - D_800A3720 >= 0x1C1) {
                    break;
                }
                *(s32 *)&(*p_prim)++->r0 = 0x2EFF8080;
                break;
            }
            if (*p_prim - D_800A3720 < 0x1C1) {
                (*p_prim)++;
            }
        }
    }
}

u8 func_80068D88(s32 arg0, s32 arg1) {
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
    var_6 = D_800A37D4;
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
        D_800A37D4 = *p_end;
    } else {
        ret = 0;
    }

    return ret;
}

void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04.rows_on;
    s32 v = *p;
    v &= ~0x1;
    v |= arg0 & 0x1;
    v &= ~0x2;
    v |= arg0 & 0x2;
    v &= ~0x4;
    v |= arg0 & 0x4;
    v &= ~0x8;
    v |= (((u32)arg0 >> 4) & 1) << 3;
    v &= ~0x10;
    v |= (((u32)arg0 >> 5) & 1) << 4;
    v &= ~0x20;
    v |= (((u32)arg0 >> 6) & 1) << 5;
    v &= ~0x40;
    v |= (arg0 << 3) & 0x40;
    v &= ~0x80;
    v |= arg0 & 0x80;
    *p = v;
}

extern s32 D_800A372C;
extern u8 D_800A32C0[8];

s32 func_80068F70(s32 arg0, Unk8009BD24Block *arg1) {
    RECT buf;
    s32 temp_s0;
    s32 var_2;
    Unk800A34FCRec *v0_e49c;

    D_800A3500 = arg0;
    D_800A351C = (Unk8006E49CRec *)arg0;
    temp_s0 = arg0 + 0x58;
    D_800A3500 = temp_s0;
    func_80046AA0();
    func_8006E950(2, (Unk8006E950Head *)D_800A3500);
    D_800A372C = D_800A3500;
    var_2 = func_8006919C(D_800A3500);
    D_800A3500 = var_2;
    v0_e49c = (Unk800A34FCRec *)func_8006E49C(var_2, D_800A351C);
    v0_e49c->unk_24 = (Unk8006919CRec *)temp_s0;
    D_800A3500 = (s32)v0_e49c;
    D_800A34FC = v0_e49c;
    D_800A3500 = (s32)(v0_e49c + 1);
    {
        s32 var_4 = -0x10;
        s32 var_5;
        s32 flags = D_800A34F8;
        var_5 = D_8009BC04.rows_on;

        flags &= var_4;
        D_800A34F8 = flags;
        if (!((u32)var_5 & 1)) {
            u32 mask;
            s32 var_8;
            mask = (u32)-0x10;
            var_8 = D_8009BC04.rows_on;
            do {
                s32 next;
                s32 lo;
                flags = D_800A34F8;
                lo = flags;
                lo &= 0xF;
                if (lo >= 7) {
                    D_800A34F8 = flags & mask;
                    break;
                }
                next = lo + 1;
                flags = (flags & mask) | (next & 0xF);
                D_800A34F8 = flags;
                flags &= 0xF;
                flags = (u32)var_8 >> flags;
                flags &= 1;
            } while (!flags);
        }
        {
            /* FAKE: one s16 * holder per stored work-block field, set before
             * the D_800A3524 store; plain `*p` stores keep the D_800A3528 and
             * D_800A34F8 stores in place. Member stores through a holder: score
             * 28; straight through D_800A34FC: 39. */
            s16 *p_2A;
            s16 *p_28;
            s16 *p_0E;
            s16 *p_0C;
            s16 *p_10;
            s16 *p_12;
            s32 value;

            value = 5;
            /* FAKE: block fence keeps li v0,5 at the join-block head so
               reorg steals it into all three incoming jump delay slots; without
               it: score 4 */
            do {
            } while (0);
            p_2A = &D_800A34FC->unk_28.half[1];
            p_28 = &D_800A34FC->unk_28.half[0];
            p_0E = &D_800A34FC->unk_0C[1];
            p_0C = &D_800A34FC->unk_0C[0];
            p_10 = &D_800A34FC->unk_10;
            p_12 = &D_800A34FC->unk_12;
            D_800A3524 = arg1;
            /* FAKE: sched fence keeps the D_800A3524 store adjacent to the
               D_800A34FC load instead of sinking below the zero-stores; without
               it: score 2 */
            do {
            } while (0);
            D_800A3518 = 0;
            D_800A3528 = 0;
            *p_2A = value;
            *p_28 = value;
            flags = D_800A34F8 & ~0x1C00;
            D_800A3510[1] = 0;
            D_800A3510[0] = 0;
            D_800A350C[1] = 0;
            D_800A350C[0] = 0;
            flags |= 0x1000;
            D_800A34F8 = flags;
            *p_12 = 0;
            *p_10 = 0;
            *p_0E = 0;
            *p_0C = 0;
            __builtin_memcpy(&buf, D_800A32C0, 8);
            DrawSync(0);
            MoveImage(&buf, 0x3C0, 0x1FE);
            DrawSync(0);
        }
    }
    D_800A34FC->unk_30 = D_800A3524->unk20_0;
    return 1;
}

Unk8006E49CRec *func_80069120(s32 a0) {
    Unk8009BD24Block *v0 = D_800A3524;
    Unk800A34FCRec *v1 = D_800A34FC;
    if (v1->unk_30 != v0->unk20_0) {
        func_8006E8CC(D_800A372C);
    }
    v0 = D_800A3524;
    v1 = D_800A34FC;
    v1->unk_30 = v0->unk20_0;
    return &D_800A351C[a0];
}

s32 func_8006919C(Unk8006919CRec *a0) {
    s32 i = 0;
    Unk8009B0E0Record ***p = &a0->unk_14;
    do {
        func_8006920C((s32 *)a0, (s32)*p);
        p++;
        i++;
    } while (i < 12);
    func_8005C2A8(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);
    return a0->unk_00.unk_04;
}

void func_8006920C(s32 *a0, s32 a1) {
    s32 *p = (s32 *)a1;
    if (!*p)
        return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)a0;
        }
        p++;
    }
}

extern void func_8006E390();

s32 func_80069250(s32 arg0, s32 arg1) {
    s32 sp10[10];
    D_800A3514 = 0;
    func_8006E390(sp10, &D_800A3518);
    func_80069E18(sp10, 0);
    if ((arg1 & 0x400040) != 0) {
        func_8005C650(1, 0x7F, 0x7F);
        return 1;
    }
    return 0;
}

extern u32 D_800A32D0[2];

s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 one;
    s32 sum;
    s32 i;
    s32 a3_off;
    s32 bitpos;
    u32 *p;
    u32 maskA;
    u32 maskB;
    u32 v;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    one = 1;
    /* FAKE: do{}while(0) around the loop seats sum in $t2 and bitpos in $t1
     * (without it: score 7); the `one` constant-holder schedules `li $t6,1`
     * at the preheader (the literal: score 2; both: 9). */
    do {
        a3_off = 0;
        bitpos = 0;
        p = D_800A32D0;

        do {
            /* FAKE: byte-offset step (arg3 += a3_off with an element count
             * scores 3) */
            arg3 = (s16 *)((u8 *)arg3 + a3_off);
            /* FAKE: dead store -- v is reassigned from *arg0 before any read;
             * removed, loop.c strength-reduces D_800A32C8[i] to a walking
             * pointer (lui/addiu t3, addiu t3,t3,4) instead of the target's sll
             * / addu indexed load: score 13 */
            v = i * 4;
            maskB = *p << arg1;
            maskA = D_800A32C8[i] << arg1;
            if (*arg2 == 0) {
                v = *arg0;
                if (v & maskA) {
                    *arg3 = one;
                } else if (v & maskB) {
                    *arg3 = -1;
                }
            } else {
                v = *arg0;
                if (v & maskA) {
                    *arg2 = 6;
                } else if (v & maskB) {
                    *arg3 = -6;
                }
                sval = *arg2;
                if (sval >= 6) {
                    sum += one << bitpos;
                    *arg3 = 0;
                    *arg2 = 0;
                } else if (sval < -5) {
                    sum += 2 << bitpos;
                    *arg3 = 0;
                    *arg2 = 0;
                }
            }
            a3_off += 2;
            bitpos += 0x10;
            p++;
            i++;
            *arg2 = (u16)*arg2 + (u16)*arg3;
            arg2++;
        } while (i < 2);

        return sum;
    } while (0);
}

s32 func_800693CC(s32 arg0, s32 arg1) {
    extern void func_80069AE4(s32 *, s32, GpuDb *);
    extern void func_8006A880(u8 *, u16 *, s32);
    extern void func_80069F80(s32 *, s32);
    extern void func_8006A1A0(s32 *, s32);
    /* FAKE: frame layout. func_8006E390 fills words 1..9 (siblings: s32
     * sp10[10]); the folded `arg1` word sits at word 12 and the
     * saved row mask at word 14 only to reproduce the target's 0x60 frame
     * (sp+0x40 / sp+0x48); separate locals give the wrong frame: score 11. */
    s32 var_0[15];
    s32 result;
    GpuDb *var_2;
    var_2 = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(var_0, &D_800A3518);
    var_0[12] = (arg1 & 0xFFFF) | ((u32)arg1 >> 16);
    result =
        func_800692C0((u32 *)&var_0[12], 0, D_800A34FC->unk_0C, D_800A350C);
    if ((result >> 16) == 1)
        goto increment;
    if ((result >> 16) == 2)
        goto decrement;
    goto after_move;
increment: {
    /* FAKE: the index-field clear mask, named so its load sits ahead of
     * the alias (the literal moves `li -16` two slots: score 2). */
    s32 var_3 = ~0xF;
    do {
        s32 flags = D_800A34F8;
        s32 index;
        D_800A34F8 = (flags & var_3) | (index = ((flags & 0xF) + 1) & 0xF);
        if (index >= 8) {
            D_800A34F8 &= var_3;
        }
    } while (!(((u32)D_8009BC04.rows_on >> (D_800A34F8 & 0xF)) & 1));
}
    goto moved;
decrement: {
    do {
        s32 flags = D_800A34F8;
        s32 index = flags & 0xF;
        if (index == 0) {
            D_800A34F8 = (flags & ~0xF) | 7;
        } else {
            D_800A34F8 = (flags & ~0xF) | ((index - 1) & 0xF);
        }
    } while (!(((u32)D_8009BC04.rows_on >> (D_800A34F8 & 0xF)) & 1));
}
moved:
    D_800A3514 = 0;
    D_800A3524->unk20_3 = 0;
after_move:
    if (((result & 0xFF) < 3) && ((result & 0xFF) != 0) &&
        (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 3)) {
        D_800A3524->unk20_3 ^= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    if (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 2) {
        D_800A3524->unk20_3 = 1;
    } else if (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 1) {
        D_800A3524->unk20_3 = 0;
    }
    if ((D_800A34F8 & 0xF) != 7) {
        if (((D_800A34F8 & 0xF) - 6) >= 0) {
            var_0[14] = D_8009BC04.unk4 & 0x80;
            D_8009BC04.unk4 = var_0[14] | (0x1F << ((D_800A34F8 & 0xF) - 5));
        } else {
            D_8009BC04.unk4 = 0x9F;
        }
    }
    func_80069AE4(var_0, 0, var_2);
    func_8006A880(
        (u8 *)var_0, (u16 *)var_2, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    func_80069F80(var_0, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    func_8006A1A0(var_0, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    if (arg1 & 0x40) {
        switch (D_8009BC04.opt[D_800A34F8 & 0xF].state) {
        case 4:
        case 5:
            if (g_pad_state.valid[1] == 0) {
                func_8005C650(2, 0x7F, 0x7F);
                func_8005C6D0();
                goto cancel;
            }
        }
        goto accept;
    }
    if (arg1 & 0x400000) {
        switch (D_8009BC04.opt[D_800A34F8 & 0xF].state) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            goto reject;
        }
        if (g_pad_state.valid[0] == 0) {
        reject:
            func_8005C650(2, 0x7F, 0x7F);
            goto cancel;
        }
    accept:
        func_8005C650(1, 0x7F, 0x7F);
        if (D_8009BC04.opt[D_800A34F8 & 0xF].state != 7) {
            snd_CloseVab1();
        }
        return D_8009BC04.opt[D_800A34F8 & 0xF].state;
    }
    if (arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (arg1 & 0x50005000) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    return -1;
}

void func_80069898(s32 *arg0, u16 *arg1, s32 arg2) {
    TILE *p = (TILE *)arg0[6];

    SetTile(p);
    p->r0 = 0xFF;
    p->g0 = 0xFF;
    p->b0 = 0xFF;
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = arg1[0];
    p->y0 = arg1[1];
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 0);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    SetTile(p);
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = arg1[0];
    p->y0 = arg1[1] - 1;
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    SetTile(p);
    p->r0 = 0x40;
    p->g0 = 0x40;
    p->b0 = 0x40;
    p->x0 = arg1[0];
    p->y0 = arg1[1] - 2;
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    arg0[6] = (s32)p;
}

void func_80069A30(void *arg0) {
    TILE *a0 = arg0;
    Unk8009BD24Block *p = func_80077D00();
    s32 v0;
    if (p->unk20_0) {
        v0 = 0x22;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x4C;
        a0->r0 = v0;
        a0->g0 = v0;
        v0 = 0x6C;
    }
    a0->b0 = v0;
}

void func_80069A8C(void *arg0) {
    POLY_F4 *a0 = arg0;
    Unk8009BD24Block *p = func_80077D00();
    s32 v0;
    if (p->unk20_0) {
        v0 = 8;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x31;
        a0->r0 = 0;
        a0->g0 = 0;
    }
    a0->b0 = v0;
}

void func_80069AE4(s32 *arg0, s32 mode, GpuDb *arg2) {
    TILE *p;
    POLY_F4 *poly;
    Unk8009B0E0Record **qbase;
    Unk8009B0E0Record **q;
    s32 i;
    Unk8007352CRec s;

    p = (TILE *)arg0[6];

    if (mode == 2) {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x4E;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x166;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else if (mode == 1) {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x3F;
        p->y0 = 0x30;
        p->w = 0x202;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x130;
        p->y0 = 0x3A;
        p->w = 0x126;
        p->h = 0xAB;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    }
    arg0[6] = (s32)p;
    s.ot_idx = 0x12;
    s.has_color = 0;
    s.semi = 0;
    qbase = ((Unk8006919CRec *)arg0[1])->unk_34;
    s.x = 0;
    s.y = 0;
    q = qbase;
    i = 0;
    do {
        s.header = *q;
        s.table = s.header->cells;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        q++;
        i++;
    } while (i < 3);

    {
        s.header = qbase[0];
        SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x12, (DR_MODE *)arg0[7]);

    poly = (POLY_F4 *)arg0[3];
    arg0[7] += 0xC;
    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0;
    poly->y0 = 0xB9;
    poly->x1 = 0x122;
    poly->y1 = 0;
    poly->x2 = 0;
    poly->y2 = 0xEF;
    poly->x3 = 0x122;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0x15E;
    poly->x1 = 0x27F;
    poly->y0 = 0;
    poly->y1 = 0;
    poly->x2 = 0x15E;
    poly->y2 = 0xEF;
    poly->x3 = 0x27F;
    poly->y3 = 0x36;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0x122;
    poly->y0 = 0;
    poly->x1 = 0x15E;
    poly->y1 = 0;
    poly->x2 = 0x122;
    poly->y2 = 0xEF;
    poly->x3 = 0x15E;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    arg0[3] = (s32)poly;
}

void func_80069E18(s32 arg0) {
    TILE *tile;
    Unk8009B0E0Record **ptr;
    Unk8007352CRec s;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *p1;

    tile = (TILE *)*(s32 *)(arg0 + 0x18);
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0xFF;
    tile->b0 = 0xFF;
    tile->w = 0x280;
    tile->x0 = 0;
    tile->y0 = 0;
    tile->h = 0xF0;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + 0x14, tile);
    *(s32 *)(arg0 + 0x18) = (s32)(tile + 1);

    ptr = (*(Unk8006919CRec **)(arg0 + 4))->unk_14;
    s.ot_idx = 0x10;
    s.semi = 0;
    s.x = 0;
    s.y = 0;
    s.has_color = 0;

    s.header = ptr[0];
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x11, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;

    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);

    s.header = ptr[1];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);

    s.header = ptr[2];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
}

/* FAKE: frame layout (oversized live object): the descriptor plus an
 * unwritten tail, so the locals region reaches the target's frame. The plain
 * descriptor scores 12 (func_80069F80) / 14 (func_8006A1A0). */
typedef struct {
    Unk8007352CRec fr00;
    s32 tail[4];
} Unk80069F80Rec;

void func_80069F80(s32 *arg0, s32 arg1) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * func_80073728 / func_8007352C); tail[0..3] is unwritten padding that
     * brings the locals to the target's 0x70 frame (the 0x2C descriptor alone
     * gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The plain
     * descriptor: score 12. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *tbl;

    if (arg1 & 2) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.fr00.header = ptr[0];
        if (arg1 & 1) {
            s.fr00.x = 0x9C;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (D_800A3524->unk20_3) {
            if (arg1 & 1) {
                s.fr00.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 3;
        tbl = s.fr00.header->cells;
        s.fr00.table = tbl;
        s.fr00.ft4_out = arg0[2];
        arg0[2] = func_80073728(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[1];
        tbl = s.fr00.header->cells;
        s.fr00.table = tbl;
        s.fr00.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.fr00);
        if (arg1 & 1) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            s.fr00.table = &s.fr00.header->cells[1];
            s.fr00.sprt_out = arg0[5];
            arg0[5] = func_8007352C(&s.fr00);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 3, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }
}

void func_8006A1A0(s32 *arg0, s32 arg1) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * func_80073728 / func_8007352C); tail[0..3] is unwritten padding that
     * brings the locals to the target's 0x70 frame (the 0x2C descriptor alone
     * gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The plain
     * descriptor: score 14. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *tbl;

    if (arg1 & 1) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.fr00.header = ptr[2];
        if (arg1 & 2) {
            s.fr00.x = -1;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (!(D_800A3524->unk20_3)) {
            if (arg1 & 2) {
                s.fr00.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.x = x0 + 0x32;
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 2;
        tbl = s.fr00.header->cells;
        s.fr00.table = tbl;
        s.fr00.ft4_out = arg0[2];
        arg0[2] = func_80073728(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[3];
        tbl = s.fr00.header->cells;
        s.fr00.table = tbl;
        s.fr00.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.fr00);
        if (arg1 & 2) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            tbl = s.fr00.header->cells;
            s.fr00.table = tbl;
            s.fr00.sprt_out = arg0[5];
            arg0[5] = func_8007352C(&s.fr00);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 2, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }
}

void func_8006A3CC(s32 *arg0, Unk8007352CRec *arg1) {
    arg1->header = ((Unk8006919CRec *)arg0[1])->unk_1C[4];
    arg1->x = 0;
    arg1->y = 0;
    arg1->has_color = 0;
    arg1->semi = 0;
    arg1->ot_idx = 1;
    arg1->table = arg1->header->cells;
    arg1->sprt_out = arg0[5];
    arg0[5] = func_8007352C(arg1);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(arg1->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
}

void func_8006A494(s32 *arg0, Unk8007352CRec *arg1) {
    arg1->header = ((Unk8006919CRec *)arg0[1])->unk_1C[9];
    arg1->x = 0;
    arg1->y = 0;
    arg1->has_color = 0;
    arg1->semi = 0;
    arg1->ot_idx = 1;
    arg1->scale_x = 0x100;
    arg1->scale_y = 0x100;
    arg1->table = arg1->header->cells;
    arg1->ft4_out = arg0[2];
    arg0[2] = func_80073728(arg1, 0);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(arg1->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
}

void func_8006A564(u8 *arg0, Unk8007352CRec *arg1, s32 arg2) {
    TILE *tile;
    Unk8009B0E0Record **tbl;
    s32 s4;

    tile = (TILE *)*(s32 *)(arg0 + 0x18);
    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
        s4 = 0;
    } else {
        setRGB0(tile, 0x50, 0x50, 0x50);
        s4 = 0x20;
    }
    tile->x0 = 0x5F;
    tile->y0 = arg1->y + 0xF;
    tile->w = arg1->x + 0x19;
    tile->h = 1;
    SetSemiTrans(tile, arg1->semi);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
    } else {
        setRGB0(tile, 0x20, 0x20, 0x20);
    }
    tile->x0 = arg1->x;
    tile->y0 = arg1->y + 0xE;
    tile->w = 0x78;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r >> 1, 0, arg1->col_b >> 1);
    } else {
        setRGB0(tile, 0x10, 0x10, 0x10);
    }
    tile->x0 = arg1->x + 0x40;
    tile->y0 = arg1->y + 0xD;
    tile->w = 0x38;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);

    tile = tile + 1;
    *(s32 *)(arg0 + 0x18) = (s32)tile;
    tbl = (*(Unk8006919CRec **)(arg0 + 4))->unk_1C;
    {
        /* FAKE: single-use holder of the cell table ahead of the descriptor's
         * table store; stored directly: score 7. */
        Unk8009B400Record *v1;
        /* FAKE: the header store through a pointer to the member: stored as
           arg1->header the member store is scheduled past the D_800A34F8 load
           (MEM_IN_STRUCT_P); score 7. */
        Unk8009B0E0Record **var_7 = &arg1->header;

        *var_7 = tbl[10];
        if ((D_800A34F8 & 0xF) == arg2) {
            arg1->col_g = 0;
            arg1->col_r = arg1->col_r >> 1;
            arg1->col_b = arg1->col_b >> 1;
        } else {
            arg1->col_b = 0x28;
            arg1->col_g = 0x28;
            arg1->col_r = 0x28;
        }

        arg1->x = 0;
        v1 = arg1->header->cells;
        arg1->y += 0xF;
        arg1->table = v1;

        arg1->sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C(arg1);

        arg1->header = tbl[11];
        v1 = arg1->header->cells;
        arg1->table = v1;
        arg1->sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C(arg1);
    }

    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(arg1->header, s4), 0);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}

/* Draws the eight-row list screen from the MOD.BIN resource (ctx+4): the row
 * sheets (cursor row flashing, rows switched on in D_8009BC04.rows_on lit), a
 * counter frame, row 7, the frame icons and a TILE. arg0 = the words
 * func_8006E390 fills (+0x8 POLY_FT4 cursor, +0x14 SPRT cursor,
 * +0x18 TILE cursor, +0x1C DR_MODE cursor, +0x20 DR_AREA cursor,
 * +0x24 DR_OFFSET cursor), accessed as byte offsets like func_8006A564;
 * arg1 = the display environment's clip RECT and ofs[2]; arg2 (passed by
 * func_800693CC) is unused. */
void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2) {
    Unk8007352CRec s;
    RECT rect;
    s16 ofs[2];
    /* Ruling 11 (ordinary-c-judge-decidable): holds five values, each a
     * sprite-sheet list of the MOD.BIN
     * root: unk_18 (option rows; first sheet, the row loop and row 7), unk_40
     * (counter frames), unk_24 twice (icon frames at [8 + frame], then FT4
     * frames at [frame]). */
    Unk8009B0E0Record **var_3;
    /* Ruling 11: holds two values, each a mask with one bit per option row:
       D_8009BC04.unk4 (scanned for the first row drawn) and .rows_on (rows
       switched on). */
    u32 row_mask;
    /* Ruling 9: the sheet's cell array; every sheet drawn here has one 12-byte
     * header, so the cells start at +0xC. FAKE: single-use holder of the cell
     * table ahead of the descriptor's table store; stored directly: score 15.
     */
    Unk8009B400Record *var_5;
    s32 yofs;
    s32 bit;
    s32 i;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
    row_mask = D_8009BC04.unk4;
    bit = 0;
    s.header = var_3[0];
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    rect.x = arg1[0];
    rect.y = arg1[1] + 0x3A;
    rect.w = 0xF6;
    rect.h = 0x92;
    SetDrawArea((DR_AREA *)*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_AREA *)*(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    yofs = 0;
    for (; !(row_mask & (1 << bit)); bit++) {
        yofs -= 0x18;
    }
    ofs[0] = arg1[4];
    ofs[1] = arg1[5] + yofs;
    SetDrawOffset((DR_OFFSET *)*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_OFFSET *)*(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    row_mask = D_8009BC04.rows_on;
    s.ot_idx = 0xB;
    for (i = 0; i < 7; i++) {
        s32 y = i * 0x18 + 0x3F;

        s.x = 0x76;
        s.has_color = 1;
        if ((D_800A34F8 & 0xF) == i) {
            s.semi = 0;
            s.col_r =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
            s.col_g =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
            s.col_b = 0x30;
            s.y = D_800A34FC->unk_0C[1] + y;
            D_800A3514++;
        } else if ((row_mask >> i) & 1) {
            s.col_r = 0x80;
            s.col_g = 0x6C;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
            if (D_800A3524->unk20_0) {
                s.has_color = 0;
            }
        } else {
            s.col_r = 0x30;
            s.col_g = 0x30;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
        }
        s.header = var_3[i];
        var_5 = s.header->cells;
        s.table = var_5;
        s.sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
        SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                    func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
        *(s32 *)(arg0 + 0x1C) += 0xC;
        s.has_color = 1;
        func_8006A564(arg0, &s, i);
    }

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_40;
    s.ot_idx = 0xA;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.y = 0x41;
    s.header = var_3[D_800A34F8 & 0xF];
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
    rect.x = arg1[0];
    rect.y = arg1[1];
    rect.w = arg1[2];
    rect.h = arg1[3];
    SetDrawArea((DR_AREA *)*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_AREA *)*(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    ofs[0] = arg1[4];
    ofs[1] = arg1[5];
    SetDrawOffset((DR_OFFSET *)*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_OFFSET *)*(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    s.x = 0x76;
    s.has_color = 1;
    if ((D_800A34F8 & 0xF) == 7) {
        s.semi = 0;
        s.col_r =
            ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
        s.col_g =
            ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
        s.col_b = 0x30;
        s.y = D_800A34FC->unk_0C[1] + 0xCF;
        D_800A3514++;
    } else if ((row_mask >> 7) & 1) {
        s.col_r = 0x80;
        s.col_g = 0x6C;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
        if (D_800A3524->unk20_0) {
            s.has_color = 0;
        }
    } else {
        s.col_r = 0x30;
        s.col_g = 0x30;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
    }
    s.ot_idx = 9;
    s.header = var_3[7];
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    s.has_color = 1;
    func_8006A564(arg0, &s, 7);
    func_8006A494((s32 *)arg0, &s);

    s.has_color = 0;
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = var_3[(D_800A34F8 & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = var_3[D_800A34F8 & 0xF];
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    var_5 = s.header->cells;
    s.table = var_5;
    s.ft4_out = *(s32 *)(arg0 + 8);
    *(s32 *)(arg0 + 8) = func_80073728(&s, 0);

    SetTile((TILE *)*(s32 *)(arg0 + 0x18));
    ((TILE *)*(s32 *)(arg0 + 0x18))->r0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->g0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->b0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->x0 = 0x142;
    ((TILE *)*(s32 *)(arg0 + 0x18))->y0 = 0x51;
    ((TILE *)*(s32 *)(arg0 + 0x18))->w = 0x100;
    ((TILE *)*(s32 *)(arg0 + 0x18))->h = 0x59;
    SetSemiTrans((TILE *)*(s32 *)(arg0 + 0x18), 0);
    AddPrim(g_gpu_ot_ptr + 1, (TILE *)*(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) += 0x10;

    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
}

/* One argument: the caller's s32[10] (asm reads a0 only). */
void func_8006B120(s32 *arg0);

void func_8006B120(s32 *arg0) {
    Unk8007352CRec s;
    u16 r[4];
    Unk8009B0E0Record **tbl;
    s32 i;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 11. */
    Unk8009B400Record *p1;

    s.has_color = 0;
    s.ot_idx = 10;
    tbl = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = tbl[0];
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
    s.x = 0;
    i = 0;

    do {
        s.header = tbl[i + 1];
        if ((((u32)D_800A34F8 >> 10) & 7) == i) {
            s.has_color = 1;
            s.y = D_800A34FC->unk_0C[1];
            s.semi = 0;
            s.col_r = s.col_g = s.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 47) >> 12) - 0x80;
        } else {
            s.has_color = 0;
            s.y = 0;
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        i++;
    } while (i < 6);

    s.y = 0;
    i = 0;
    do {
        s.header = tbl[i + 7];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_0 == i) {
            s.semi = 0;
            if (!(D_800A34F8 & 0x1C00)) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 9];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_1 == i) {
            s.semi = 0;
            if ((D_800A34F8 & 0x1C00) == 0x400) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 11];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_2 == i) {
            s.semi = 0;
            if ((D_800A34F8 & 0x1C00) == 0x800) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        i++;
    } while (i < 2);

    tbl = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = tbl[1];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
    r[2] = 0xAF;
    r[0] = 0xE8;
    r[1] = 0x25;
    r[3] = 1;
    func_80069898(arg0, r, 0x11);
}

/* func_8006B578 — menu/config input dispatch. Its second switch's 6-entry
 * jump table lands at 0x80015988 in this TU's .rodata. */
s32 func_8006B578(s32 *arg0, u32 *arg1) {
    u32 v;
    s32 sp10;
    s32 ret;
    s32 hi;
    s32 var_s2 = 0;

    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0((u32 *)&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    hi = ret >> 16;
    switch (hi) {
    case 1: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0x1400) {
            D_800A34F8 = a0 & ~0x1C00;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) + 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    case 2: {
        u32 a0 = D_800A34F8;
        if ((a0 & 0x1C00) == 0) {
            D_800A34F8 = (a0 & ~0x1C00) | 0x1400;
        } else {
            u32 m = a0 & ~0x1C00;
            s32 c = ((a0 >> 10) & 7) - 1;
            m |= (c & 7) << 10;
            D_800A34F8 = m;
        }
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }
    }

    if (((u32)D_800A34F8 >> 10 & 7) >= 6) {
        goto tail;
    }
    switch ((u32)D_800A34F8 >> 10 & 7) {
    case 0:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_0 ^= 1;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 1:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_1 ^= 1;
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto shared_400040;
    case 2:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_2 ^= 1;
            func_8005C650(0, 0x7F, 0x7F);
        }
    shared_400040:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            {
                u32 f2 = D_800A34F8;
                u32 m2 = f2 & ~0x1C00u;
                s32 c2 = ((f2 >> 10) & 7) + 1;
                D_800A34F8 = m2 | ((c2 & 7) << 10);
            }
        }
        goto tail;
    case 3:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
            var_s2 = 2;
        }
        goto tail;
    case 4:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 3;
        }
        goto tail;
    case 5:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s2 = 1;
        }
        goto tail;
    }
tail:
    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s2 = 1;
    }
    return var_s2;
}

/* Keeps the original .rodata run contiguous around the compiler-generated
 * switch tables. */
const u8 D_800159A0[16] = "warning\n";

s32 func_8006B898(s32 arg0, u32 arg1) {
    s32 sp10[10];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006B120(sp10);
    return func_8006B578(&arg0, &arg1);
}

s32 func_8006B92C(s32 *arg0, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            u32 m = a0 & 0xFFFF1FFF;
            s32 c = ((a0 >> 13) & 7) + 1;
            m |= (c & 7) << 13;
            D_800A34F8 = m;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        } else {
            u32 m = a0 & 0xFFFF1FFF;
            s32 c = ((a0 >> 13) & 7) - 1;
            m |= (c & 7) << 13;
            D_800A34F8 = m;
        }
    do_call:
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }

    idx = ((u32)D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         ((((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
    return var_s0;
}

void func_8006BB68(s32 *arg0) {
    Unk8007352CRec s;
    u16 var_1[4];
    s32 i;
    Unk8009B0E0Record **q;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 11. */
    Unk8009B400Record *p1;

    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xA;
    s.has_color = 0;
    q = ((Unk8006919CRec *)arg0[1])->unk_2C;
    {
        s.header = q[0];
        s.semi = 0;
        p1 = s.header->cells;
        s.table = p1;
    }
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    for (i = 0; i < 3; i++) {
        s.header = q[1];
        p1 = s.header->cells;
        s.table = p1;
        if ((((u32)D_800A34F8 >> 13) & 7) == i) {
            s.y = D_800A34FC->unk_0C[1];
            s.semi = 0;
        } else {
            s.y = 0;
            s.semi = 1;
        }
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        q++;
    }

    q = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = q[1];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    var_1[2] = 0xAF;
    var_1[0] = 0xE8;
    var_1[1] = 0x25;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

extern DR_MODE *D_800A36E0;
extern u8 *D_800A36E4;

void func_8006BD28(s32 arg0, s32 arg1, Unk8007352CRec *arg2, s32 arg3) {
    /* sprite-sheet header pointers, two per arg0: the unk_20 list of the
       MOD.BIN root (D_800A34FC->unk_24) */
    Unk8009B0E0Record **var_0;
    /* FAKE: named intermediate (no-new-park-categories entry 6): the sheet's
     * 8-byte cells start at header + 0xC; inline, fold reassociates the sum and
     * loop.c strength-reduces it into its own register. Inline: score 31. */
    Unk8009B400Record *var_1;
    s32 i, j, n;

    var_0 = D_800A34FC->unk_24->unk_20;

    arg2->col_b = 0x30;
    arg2->col_g = 0x30;
    arg2->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        /* FAKE: operand grouping (or-tree-shape-shift carve-out): with
         * (var_0 + i) + arg0 * 2, arg0 * 8 is hoisted alone as in the target;
         * var_0[arg0 * 2 + i]: score 33. */
        arg2->header = *(var_0 + i + arg0 * 2);
        if (arg2->header == (Unk8009B0E0Record *)-1)
            return;

        n = (arg0 == 0x12) ? 3 : 1;

        for (j = 0; j < n; j++) {
            arg2->x = 0;
            arg2->y = arg1;
            arg2->ot_idx = 8;
            if (arg0 != 0x12 || j == arg3 || j == 2) {
                arg2->has_color = 0;
            } else {
                arg2->has_color = 1;
            }
            arg2->semi = 0;
            arg2->sprt_out = (s32)D_800A36E4;
            var_1 = arg2->header->cells;
            arg2->table = var_1 + j;
            D_800A36E4 = (u8 *)func_8007352C(arg2);
        }

        SetDrawMode(D_800A36E0, 1, 0, func_8006E480(arg2->header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 8, D_800A36E0);
        D_800A36E0++;
    }
}

extern s32 D_800A3900;
extern TILE *D_800A36DC;
extern u8 D_800F11E0[];
extern DR_MODE D_800F1438[];
extern TILE D_800F1498[];

void func_8006BEC4(s32 arg0, s32 arg1) {
    Unk8007352CRec sp10;
    s32 var_1;
    Size2s16 *pos;
    s16 i;
    s32 r;
    s32 w;
    s32 h;
    s32 x0;

    D_800A3900 = 0;
    var_1 = D_800A36AC & 1;
    D_800A36E4 = D_800F11E0 + var_1 * 0x12C;
    D_800A36E0 = D_800F1438 + var_1 * 4;
    D_800A36DC = D_800F1498 + var_1 * 4;
    func_8006BD28(arg0, 0, &sp10, 0);
    h = 0;
    pos = D_800A34FC->unk_24->unk_48;
    pos += arg0;
    if (arg1 != -1) {
        w = arg1 ? 0x1F : 0x28;
        h = 0x10;
        D_800A3900 = (pos->y + 0x10) >> 1;
        func_8006BD28(0x12, D_800A3900, &sp10, arg1);
        x0 = (arg1 & 1) * 0x3A + 0x113;
        for (i = 0; i < 3; i++) {
            SetTile(D_800A36DC);
            if (i == 0) {
                r = 0xFF;
                SetSemiTrans(D_800A36DC, 0);
            } else {
                r = 0xFF - ((i - 1) << 7);
                SetSemiTrans(D_800A36DC, 1);
            }
            D_800A36DC->r0 = r;
            D_800A36DC->g0 = 0;
            D_800A36DC->b0 = 0;
            D_800A36DC->y0 = D_800A3900 + 0x7C - i;
            D_800A36DC->x0 = x0;
            D_800A36DC->w = w;
            D_800A36DC->h = 1;
            AddPrim(g_gpu_ot_ptr + 8, D_800A36DC);
            D_800A36DC++;
        }
    }
    SetTile(D_800A36DC);
    D_800A36DC->r0 = 0;
    D_800A36DC->g0 = 0;
    D_800A36DC->b0 = 0;
    D_800A36DC->x0 = 0x140 - (pos->x >> 1);
    D_800A36DC->y0 = 0x78 - (pos->y >> 1);
    D_800A36DC->w = pos->x;
    D_800A36DC->h = pos->y + h;
    SetSemiTrans(D_800A36DC, 1);
    AddPrim(g_gpu_ot_ptr + 8, D_800A36DC);
    D_800A36DC++;
}

extern s32 func_8006B92C();

s32 func_8006C168(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006BB68(sp10);
    return func_8006B92C(&arg0, &arg1);
}

s32 func_8006C1FC(s32 a0, s32 a1) { return func_8006C168(a0, a1); }

#define setXYWH(p, _x0, _y0, _w, _h)                                           \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0), \
    (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h), (p)->x3 = (_x0) + (_w),           \
    (p)->y3 = (_y0) + (_h)

void func_8006C21C(s32 *arg0) {
    Unk8007352CRec s;
    s32 j;
    Unk8009B0E0Record **table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *var_5;
    TILE *tile;
    POLY_G4 *poly;
    /* FAKE: constant-holder (named-local-fake-exception) for the 0 passed as
     * func_8006E480's second argument; held in $s5 as in the target. A literal
     * 0 at every call: score 3. */
    s32 mode;
    /* FAKE: holds several values (Ruling 11, ordinary-c-judge-decidable): the
     * phase-2 unlock-bit index,
     * the phase-4 sprite index, the phase-6 tile row and the phase-8 gauge
     * level; one counter per phase: score 90. */
    s32 work;
    /* FAKE: per-branch constant holder (named-local-fake-exception, Q27 (B)) --
     * the bar's far-corner red, 0 in two arms and 0x80 in the other two; set in
     * each arm, it is not hoisted out of the row loop. The literals: score 8.
     */
    s32 col;
    /* FAKE: always-zero narrow locals (named-local-fake-exception, Q27 (A)) --
     * SetDrawMode's dither and texture-window arguments and the descriptor
     * position, read up to the phase-4 head; their dead extension temps take
     * the target's four untouched frame slots (0x60-0x78). `var_14` reaches
     * SetDrawMode's RECT *tw through a
     * `(RECT *)var_14` conversion (Q113). Literals at every read: score 37. */
    s16 var_11;
    s16 xpos;
    s16 ypos;
    s16 var_14;
    s32 var_15;
    s32 x;
    s32 row;
    s32 var_18;
    /* the sprite sheet's 8-byte cells, just past its 12-byte header. Ruling 9:
     * one meaning, header + 0xC at every write. FAKE: single-use holder of the
     * cell table ahead of the descriptor's table store; stored directly:
     * score 12. */
    Unk8009B400Record *var_19;

    mode = 0;
    var_11 = 0;
    xpos = 0;
    ypos = 0;
    var_14 = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    s.x = xpos;
    s.header = table[0];
    s.semi = 0;
    var_19 = s.header->cells;
    s.table = var_19;
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, var_11, func_8006E480(s.header, mode),
                (RECT *)var_14);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.ot_idx = 9;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    for (var_15 = 0; var_15 < 2; var_15++) {
        s.x = var_15 * 280;
        if (D_800A34FC->unk_28.half[var_15] < 3) {
            for (work = 0; work < 4; work++) {
                if (D_800A3524->unk17[D_800A34FC->unk_28.half[var_15]] &
                    ((1 << work) << (var_15 * 4))) {
                    s.header = table[work + 13];
                    s.semi = 0;
                    var_19 = s.header->cells;
                    s.table = var_19;
                    s.sprt_out = arg0[5];
                    arg0[5] = func_8007352C(&s);
                }
            }
        }
    }
    s.header = table[13];
    SetDrawMode((DR_MODE *)arg0[7], 1, var_11, func_8006E480(s.header, mode),
                (RECT *)var_14);
    AddPrim(g_gpu_ot_ptr + 9, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.x = xpos;
    s.y = ypos;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.header = table[1];
    s.semi = 0;
    var_19 = s.header->cells;
    s.table = var_19;
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    for (work = 0; work < 6; work++) {
        s.header = table[work + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        var_19 = s.header->cells;
        s.table = var_19;
        for (j = 0; j < 2; j++) {
            s.x = j ? 280 : 0;
            s.sprt_out = arg0[5];
            arg0[5] = func_8007352C(&s);
        }
    }
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.header = table[1];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    recs = D_800A34FC->unk_24->unk_44;
    tile = (TILE *)arg0[6];
    for (work = 0; work < 11; work++) {
        var_5 = &recs[work];
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = var_5->r;
            tile->g0 = var_5->g;
            tile->b0 = var_5->b;
            tile->x0 = var_5->x + j * 280;
            tile->y0 = var_5->y;
            tile->w = var_5->w;
            tile->h = var_5->h;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + 0xC, tile);
            tile++;
        }
    }
    arg0[6] = (s32)tile;

    var_18 = ((rcos((D_800A3518 << 7) & 0xF80) * 32) >> 12) + 0xD0;
    poly = (POLY_G4 *)arg0[4];
    x = 0;
    for (j = 0; j < 2; j++) {
        work = D_800A34FC->unk_28.half[j];
        rec = &recs[work + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (work == 5) {
                SetSemiTrans(poly, 1);
                col = 0;
                poly->r0 = var_18;
                poly->g0 = var_18;
                poly->b0 = var_18;
                poly->r1 = var_18;
                poly->g1 = var_18;
                poly->b1 = var_18;
                poly->r2 = col;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -4 * row + 2);
            } else {
                SetSemiTrans(poly, 0);
                col = 0x80;
                poly->r0 = var_18;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = var_18;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = col;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -2 * row + 1);
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
            SetPolyG4(poly);
            if (work == 5) {
                SetSemiTrans(poly, 1);
                col = 0;
                poly->r0 = var_18;
                poly->g0 = var_18;
                poly->b0 = var_18;
                poly->r2 = var_18;
                poly->g2 = var_18;
                poly->b2 = var_18;
                poly->r1 = col;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y3 = rec[1].y - 1;
            } else {
                SetSemiTrans(poly, 0);
                col = 0x80;
                poly->r0 = var_18;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = var_18;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = col;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y3 = rec[1].y - 1;
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
        }
        SetDrawMode((DR_MODE *)arg0[7], 1, 0, 0x40, 0);
        AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
        x += 280;
    }
    arg0[4] = (s32)poly;
}

void func_8006CBD4(s32 arg0, s32 arg1) {
    s32 code;
    s16 i;
    s32 mask;

    if (arg1 & (0x10 << (arg0 * 16))) {
        code = 1;
    } else if (arg1 & (0x40 << (arg0 * 16))) {
        code = 2;
    } else if (arg1 & (0x80 << (arg0 * 16))) {
        code = 3;
    } else if (arg1 & (0x20 << (arg0 * 16))) {
        code = 0;
    }

    mask = (1 << code) << (arg0 * 4);

    for (i = 0; i < 3; i++) {
        if (i == D_800A34FC->unk_28.half[arg0]) {
            D_800A3524->unk17[i] |= mask;
        } else {
            D_800A3524->unk17[i] &= ~mask;
        }
    }
}

s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 var_4;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 var_8;
    s16 var_9;

    ret = 0;
    if ((D_800A34FC->unk_28.word == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    var_4 = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A34FC->unk_28.half[i] <= 0) {
                D_800A34FC->unk_28.half[i] = lim;
            } else {
                D_800A34FC->unk_28.half[i] = D_800A34FC->unk_28.half[i] - 1;
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A34FC->unk_28.half[i] >= lim) {
                D_800A34FC->unk_28.half[i] = 0;
            } else {
                D_800A34FC->unk_28.half[i] = D_800A34FC->unk_28.half[i] + 1;
            }
        }

        var_9 = D_800A34FC->unk_28.half[i];
        switch (var_9) {
        case 0:
        case 1:
        case 2:
            if (*arg1 & (0xF0 << shift)) {
                func_8005C650(0, 0x7F, 0x7F);
                func_8006CBD4(i, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    var_8 = *(rec + 0x1A) & (nib << var_4);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + var_8);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    var_8 = *(rec + 0x1D) & (nib << var_4);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + var_8);
                }
            }
            break;
        }
        var_4 += 4;
    }
    return ret;
}

/* The two per-row sprite counters: the target clears both with one word
 * store, so the storage is the counter array plus one word view (Q33); every
 * other access goes through count[]. */
typedef union {
    s16 count[2];
    s32 word;
} Counts_8006CFBC;

s32 func_8006CFBC(s32 *arg0) {
    Unk8007352CRec s;
    Counts_8006CFBC counts;
    Unk8009B0E0Record **table;
    s16 var_3;
    s16 column;
    s16 row;
    s16 result;
    /* Ruling 11 (ordinary-c-judge-decidable): holds three values, each
     * s.header + 0xC stored to s.table
     * (for table[column + 8], table[12] and table[17..19]). FAKE: single-use
     * holder of the cell table ahead of the descriptor's table store; stored
     * directly: score 8. */
    Unk8009B400Record *temp;

    result = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.ot_idx = 8;
    s.has_color = 0;
    s.semi = 0;

    var_3 = 0;
    do {
        counts.word = 0;
        s.y = var_3 * 16;
        column = 0;
        do {
            s.header = table[column + 8];
            temp = s.header->cells;
            s.table = temp;
            for (row = 0; row < 2; row++) {
                if (D_800A3524->unk17[var_3] & ((1 << (row * 4)) << column)) {
                    s.x = row * 280 + counts.count[row] * 23;
                    s.sprt_out = arg0[5];
                    arg0[5] = func_8007352C(&s);
                    counts.count[row]++;
                }
            }
            column++;
        } while (column < 4);

        row = 0;
        do {
            if (counts.count[row] == 0) {
                s.x = row * 280;
                result |= 1 << row;
                s.header = table[12];
                temp = s.header->cells;
                s.table = temp;
                s.sprt_out = arg0[5];
                arg0[5] = func_8007352C(&s);
            }
            row++;
        } while (row < 2);
        var_3++;
    } while (var_3 < 3);

    row = 0;
    do {
        if (D_800A34FC->unk_28.word == 0x50005) {
            s.header = table[17];
        } else if ((result >> row) & 1) {
            s.header = table[19];
        } else {
            s.header = table[18];
        }
        s.x = row * 280;
        s.y = 0;
        temp = s.header->cells;
        s.table = temp;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        row++;
    } while (row < 2);

    s.header = table[14];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    {
        u16 var_8[4];
        var_8[2] = 0xE1;
        var_8[0] = 0xCF;
        var_8[1] = 0x25;
        var_8[3] = 1;
        func_80069898(arg0, var_8, 0x11);
    }
    return (s16)result;
}

void func_8006D324(void) {
    D_800A34FC->unk_28.half[1] = 5;
    D_800A34FC->unk_28.half[0] = 5;
}

extern void func_8006C21C(s32 *);
extern s32 func_8006CFBC(s32 *);

s32 func_8006D338(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    s32 r;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 2, t);
    func_8006C21C(sp10);
    r = func_8006CFBC(sp10);
    return func_8006CCC8(&arg0, &arg1, (s32)((r << 16) >> 16));
}

void func_8006D3DC(s32 *arg0) {
    Unk8007352CRec s;
    u16 var_1[4];
    Unk8009B0E0Record **q;
    /* FAKE: constant holder for func_8006E480's 0; the literal: score 8. */
    s32 var_3 = 0;
    s16 i = 0;
    /* FAKE: constant holder for s.col_r / g / b; the literal: score 25. */
    u8 var_5 = 0x40;

    s.ot_idx = 0xA;
    q = ((Unk8006919CRec *)arg0[1])->unk_38;
    s.x = 0;
    s.has_color = 1;

    for (; i < 6; i++) {
        s.has_color = 1;
        if (i == 0) {
            s.y = 0;
            s.has_color = 0;
            s.semi = 0;
        } else if (i == D_800A3528 + 1) {
            s.y = D_800A34FC->unk_0C[1];
            s.col_r = s.col_g = s.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.semi = 0;
        } else if (i != 1 && i != 2 && i != 3 && i == D_800A3528 + 4) {
            s.col_r = s.col_g = s.col_b = 0x80;
            s.y = 0;
            s.semi = 0;
        } else {
            s.col_r = s.col_g = s.col_b = var_5;
            s.y = 0;
            s.semi = 1;
        }
        s.header = q[i];
        s.table = s.header->cells;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, var_3), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    var_1[0] = 0xDA;
    var_1[1] = 0x25;
    var_1[2] = 0xCB;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

s32 func_8006D5D4(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 result = 0;
    s32 ret;
    s32 sval;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    if ((ret >> 16) == 1) {
        D_800A3528 = D_800A3528 + 1;
        func_8005C650(0, 0x7F, 0x7F);
    } else if ((ret >> 16) == 2) {
        D_800A3528 = D_800A3528 - 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    if ((s16)D_800A3528 < 0) {
        D_800A3528 = 2;
    }
    D_800A3528 = (s16)D_800A3528 % 3;
    if (arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        result = -1;
    } else if (arg1 & 0x400040) {
        func_8005C650(1, 0x7F, 0x7F);
        sval = (s16)D_800A3528;
        if (sval == 2) {
            result = -1;
        } else {
            D_800A3524->unk14_17 = sval;
            result = 1;
        }
    }
    return result;
}

extern s32 func_8006D5D4(s32, u32);

s32 func_8006D74C(s32 arg0, s32 arg1) {
    s32 sp_buf[22];
    s32 result;
    GpuDb *db;
    D_800A3514 += 1;
    db = &g_gpu_db[D_800A36AC & 1];
    func_8006E390((s32)&sp_buf[0], (s32)&D_800A3518);
    func_80069AE4((s32)&sp_buf[0], 1, db);
    func_8006D3DC((s32)&sp_buf[0]);
    result = func_8006D5D4(arg0, arg1);
    func_8005C6D0();
    return result;
}

s32 func_8006D7FC(void) {
    D_800A352C = 0;
    return 1;
}

void func_8006D808(
    s32 *arg0, s32 *arg1, Unk8006D808Rec *arg2, s32 arg3, s32 arg4) {
    Unk8007352CRec s;
    /* FAKE: oversized digit array (dead-vars-local-array OVERSIZED-LOCALS):
     * only d[0]/d[1] are used; the unwritten tail brings the locals to the
     * target's 0x88 frame (s16 d[2] gives 0x80): score 26. */
    s16 d[5];
    s16 i;
    s16 k;
    s16 idx;
    s16 n;

    s.ot_idx = arg3;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.has_color = 0;
    s.col_r = s.col_g = s.col_b = 0xA0;
    s.header = arg2->unk_00[2];
    for (i = 0; i < 3; i++) {
        s.header = arg2->unk_00[i];
        s.table = s.header->cells;
        s.sprt_out = *arg0;
        *arg0 = func_8007352C(&s);
    }
    s.header = arg2->unk_00[0];
    SetDrawMode((DR_MODE *)*arg1, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;

    n = 4;
    if (arg4 == -1) {
        n = 3;
    }
    for (i = 0; i < n; i++) {
        idx = D_800A3524->unk21[i].unk3;
        if (idx >= 12 && idx < 22) {
            idx -= 2;
        }
        s.header = arg2->unk_0C;
        s.table = arg2->unk_10 + 3 + idx * 3;
        s.x = arg2->unk_1C[idx];
        s.y = i * 26;
        if (i == 3) {
            s.x -= 50;
            s.y = 98;
        }
        s.sprt_out = *arg0;
        *arg0 = func_8007352C(&s);
        if (idx == 8) {
            s.table = arg2->unk_10;
            s.sprt_out = *arg0;
            *arg0 = func_8007352C(&s);
        }
    }

    s.header = arg2->unk_0C;
    SetDrawMode((DR_MODE *)*arg1, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;

    for (i = 0; i < 3; i++) {
        for (k = 0; k < n; k++) {
            if ((D_800A36AC & 1) && ((k == arg4 && k < 3) || k == 3)) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            switch (i) {
            case 0:
                d[0] = d[1] = D_800A3524->unk21[k].unk0;
                break;
            case 1:
                d[0] = d[1] = D_800A3524->unk21[k].unk1;
                break;
            case 2:
                d[0] = d[1] = D_800A3524->unk21[k].unk2;
                break;
            }
            d[0] = d[0] % 10;
            d[1] = d[1] / 10 % 10;
            s.header = arg2->unk_14;
            s.header->ubase = d[0] * 24;
            s.table = arg2->unk_18;
            s.x = i * 58 + 24;
            s.y = k * 26 + 2;
            if (k == 3) {
                s.x = i * 58 + 3;
                s.y = 100;
            }
            s.sprt_out = *arg0;
            *arg0 = func_8007352C(&s);
            s.header->ubase = d[1] * 24;
            s.x -= 24;
            s.sprt_out = *arg0;
            *arg0 = func_8007352C(&s);
        }
    }

    s.header = arg2->unk_14;
    SetDrawMode((DR_MODE *)*arg1, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;
}

/* FAKE: frame layout (oversized live object), as Unk80069F80Rec; the plain
 * descriptor: score 21. */
typedef struct {
    Unk8007352CRec dd94_00;
    s32 tail[2];
} Unk8006DD94Rec;

void func_8006DD94(s32 *arg0) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * func_8007352C); tail[0..1] is unwritten padding that brings the locals to
     * the target's 0x78 frame with var_1 at sp+0x50 (an 8-aligned slot,
     * BIGGEST_ALIGNMENT -- mips.h:1082) (dead-vars-local-array
     * OVERSIZED-LOCALS). The plain descriptor: score 21. */
    Unk8006DD94Rec s;
    u16 var_1[4];
    s16 i;
    Unk8006DD94List *q;
    /* FAKE: constant holder for s.dd94_00.semi and func_8006E480's 0; the
     * literal: score 8. */
    s32 var_4 = 0;

    s.dd94_00.ot_idx = 0xA;
    q = ((Unk8006919CRec *)arg0[1])->unk_3C;
    s.dd94_00.x = 0;
    s.dd94_00.semi = var_4;

    for (i = 0; i < 3; i++) {
        s.dd94_00.has_color = 1;
        if (i == D_800A352C + 1) {
            s.dd94_00.y = D_800A34FC->unk_0C[1];
            s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
        } else {
            if (i == 0) {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x80;
            } else {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x40;
            }
            s.dd94_00.y = 0;
        }
        s.dd94_00.header = q->unk_20[i];
        s.dd94_00.table = s.dd94_00.header->cells;
        s.dd94_00.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.dd94_00);
        SetDrawMode((DR_MODE *)arg0[7], 1, 0,
                    func_8006E480(s.dd94_00.header, var_4), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], &q->unk_00, s.dd94_00.ot_idx, -1);

    var_1[2] = 0x96;
    var_1[0] = 0xF5;
    var_1[1] = 0x25;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

s32 func_8006DF68(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 ret;
    s32 result = 0;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    if (((ret >> 16) & 0xFF) != 0) {
        D_800A352C += 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    D_800A352C &= 1;
    if ((arg1 & 0x100010) != 0) {
        result = -1;
        func_8005C650(2, 0x7F, 0x7F);
    } else if ((arg1 & 0x400040) != 0) {
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A352C != 0) {
            result = -1;
        } else {
            result = 1;
        }
    }
    func_8005C6D0();
    return result;
}

extern s32 func_8006DF68();

s32 func_8006E068(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006DD94(sp10);
    return func_8006DF68(arg0, arg1);
}

extern RECT D_800A32D8;

s32 func_8006E10C(void) {
    /* FAKE: read at entry and held in $s3 across the calls; read at the
     * LoadImage call: score 22 */
    s32 temp_s3 = D_800A3500;
    RECT rect;
    s32 v0;

    rect = D_800A32D8;
    if (D_800A3524->unk20_0) {
        v0 = cdrom_GetFileIndex(2, 0x60);
    } else {
        v0 = cdrom_GetFileIndex(2, 7);
    }
    cdrom_StartRead(v0, D_800A3500);
    cdrom_WaitIdle();
    cdrom_GetFileSize(v0);
    SetDispMask(0);
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    g_gpu_db[0].disp.isinter = 0;
    g_gpu_db[1].disp.isinter = 0;
    g_gpu_db[0].disp.isrgb24 = 0;
    g_gpu_db[1].disp.isrgb24 = 0;
    DrawSync(0);
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(temp_s3 + 0x14));
    DrawSync(0);
    PutDrawEnv(&g_gpu_db[0].draw);
    PutDispEnv(&g_gpu_db[1].disp);
    SetDispMask(1);
    return 1;
}

extern RECT D_800A32E0;

s32 func_8006E2A8(void) {
    RECT rect;
    SetDispMask(0);
    PutDrawEnv(&g_gpu_db[D_800A3518 & 1].draw);
    PutDispEnv(&g_gpu_db[D_800A3518 & 1].disp);
    DrawSync(0);
    rect = D_800A32E0;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    SetDispMask(1);
    return 1;
}

Unk8006E49CRec *func_80069120(s32);

void func_8006E390(s32 *a0, s32 *a1) {
    s32 *s0 = a0;
    Unk8006E49CRec *v0;
    a1[0]++;
    v0 = func_80069120(a1[0] & 1);
    s0[1] = (s32)D_800A34FC->unk_24;
    s0[3] = (s32)v0->unk_04;
    s0[2] = v0->unk_00;
    s0[4] = (s32)v0->unk_08;
    s0[5] = v0->unk_10;
    s0[6] = (s32)v0->unk_0C;
    s0[7] = (s32)v0->unk_14;
    s0[8] = (s32)v0->unk_18;
    D_800A3520 = v0;
    s0[9] = (s32)v0->unk_1C;
}

void func_8006E440(s32 *a0) {
    s32 *p = a0;
    if (*p == -1)
        return;
    while (*p != -1) {
        *p = *p + (s32)a0;
        p++;
    }
}

s32 func_8006E480(Unk8009B0E0Record *arg0, s32 a1) {
    return (arg0->tp0 & 0xFE1F) + (arg0->tp1 << 7) + a1;
}

s32 func_8006E49C(s32 arg0, Unk8006E49CRec *arg1) {
    u8 *base1;
    u8 *base2;
    u8 *base3;
    u8 *base4;
    arg1[0].unk_00 = arg0;
    base1 = (u8 *)arg1[0].unk_00 + 0x9C40;
    arg1[0].unk_08 = base1 + 0x5DC0;
    arg1[0].unk_04 = base1;
    base2 = base1 + 0x6838;
    arg1[0].unk_0C = base1 + 0x61F8;
    arg1[0].unk_14 = base2 + 0x1B58;
    arg1[0].unk_18 = base2 + 0x1DB0;
    arg1[0].unk_1C = base2 + 0x1E28;
    arg1[0].unk_20 = base2 + 0x1EA0;
    arg1[0].unk_10 = (s32)base2;
    arg1[1].unk_00 = (s32)(base2 + 0x1FB0);
    base3 = (u8 *)arg1[1].unk_00 + 0x9C40;
    arg1[1].unk_08 = base3 + 0x5DC0;
    arg1[1].unk_04 = base3;
    base4 = base3 + 0x6838;
    arg1[1].unk_0C = base3 + 0x61F8;
    arg1[1].unk_14 = base4 + 0x1B58;
    arg1[1].unk_18 = base4 + 0x1DB0;
    arg1[1].unk_1C = base4 + 0x1E28;
    arg1[1].unk_20 = base4 + 0x1EA0;
    arg1[1].unk_10 = (s32)base4;
    return (s32)(base4 + 0x1FB0);
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 D_800A32B8 = 0;
s32 D_800A32BC = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
TILE *D_800A36DC;
DR_MODE *D_800A36E0;
u8 *D_800A36E4;
POLY_FT4 *D_800A3720;
s32 *D_800A3724;
s32 D_800A372C;
POLY_FT4 *D_800A37D4;
s32 D_800A3900;
