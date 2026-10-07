/* 147 game functions. .text 0x80060A68 (ROM 0x51268). Start boundary: PHASE
 * (rodata-align section 9), moved by the per-file gp model (Q65). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"

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
 * clears D_800F10D0[idx], calls the chractar_use_pset_combo_id_table entry
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

    result = ((s32(*)(void))chractar_use_pset_combo_id_table
                  [D_8009BA60[D_800A3468->unk00.h] +
                   D_800F10D0[D_800A3468->unk00.h]])();
    *D_800A3468->unk14 = result;

    if (D_800A3468->unk00.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}

void func_80060B70(void) {

    Unk1F800000Unk00 *outer;
    s16 *dst16;
    s32 *dst_s32;
    u16 idx;
    s32 result;

    outer = D_800A3468;
    dst16 = D_800A346C;
    dst16[0] = outer->unk04[0];
    dst16[1] = outer->unk04[1];
    dst16[2] = outer->unk04[2];

    dst_s32 = D_800A3470;
    dst_s32[0] = outer->unk08[0];
    dst_s32[1] = outer->unk08[1];
    dst_s32[2] = outer->unk08[2];
    func_80061FAC(dst16, dst_s32, D_800A3474);

    idx = D_800A3468->unk00.h;
    result =
        ((s32(*)(void))chractar_use_pset_combo_id_table[D_8009BA60[idx] +
                                                        D_800F10D0[idx]])();

    *D_800A3468->unk14 = result;
}

extern u8 D_800F1150[];

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
     * order to the target's (param-local-alias-prologue-pair-flip) */
    unsigned int new_var;
    RECT s;
    s32 v;
    s32 ret;
    new_var = arg0;
    game_FrameLoop();
    v = D_8009BD24.unk14_0;
    if (v == 0) {
        cdrom_StartRead(func_80036EA8(2, 0x3C), arg0);
    } else if (v == 3) {
        cdrom_StartRead(func_80036EA8(2, 0x2F), new_var);
    } else if (v == 2) {
        cdrom_StartRead(func_80036EA8(2, 0x30), new_var);
    } else if (v == 5) {
        cdrom_StartRead(func_80036EA8(2, 0x31), new_var);
    } else {
        cdrom_StartRead(func_80036EA8(2, 0), new_var);
    }
    game_FrameLoop();
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
extern s32 D_800F1180;
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

void game_Cleanup(void) {
    func_80060C60();
    func_800421A4();
    D_800A32BC = 0;
}

extern u8 D_800F116A;
extern Unk1F800000Unk00 D_800F116C;

void func_800611A4(s32 *arg0, u16 *arg1) {
    u16 svec[3];
    s32 *p;
    svec[0] = arg1[0];
    /* FAKE: integer-address loads keep the arg1[1] load ahead of the D_800A3468
     * / D_800F1178 stores; arg1[1] / arg1[2] score 15 */
    svec[1] = *(u16 *)((s32)arg1 + 2);
    D_800A3468 = &D_800F116C;
    svec[2] = *(u16 *)((s32)arg1 + 4);
    D_800F117C = (s32)(&svec[0]);
    D_800F1178 = (s32)arg0;
    D_800F1180 = (s32)(&D_800F116A);
    D_800F116C.unk00.w = 0x21001A;
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFFFFEF;
}

void func_80061250(s32 *arg0) {
    extern u8 D_800F1154[];
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)arg0;
    if (D_800F1154[5] != 0) {
        if (D_800F1154[6] != 0) {
            D_800F1154[6] = 0;
            D_800F1154[5] = 0;
        }
        if (D_800F1154[5] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = &D_800F1154[5];
    D_800A3468->unk00.w = 0x210009;
    goto end;
check_one_zero:
    if (D_800F1154[6] == 0) {
        D_800F1180 = (s32)&D_800F1154[6];
        D_800F116C.unk00.w = 0x21000A;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0060;
}

extern u8 D_800F1154[];

s32 func_8006133C(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)D_800F1154;
    D_800F116C.unk00.w = 0x210004;
    func_80060A68();
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x8080FF;
    return 1;
}

extern u8 D_800F115B;

s32 func_800613C8(s32 *a0) {
    s32 *ap = a0;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
    D_800F116C.unk00.w = 0x21000B;
    func_80060A68();
    D_800F1140.vx = *ap++;
    D_800F1140.vy = *ap++;
    D_800F1140.vz = *ap++;
    D_800A3464 = 0x8080FF;
    return 16;
}

s32 func_80061454(s32 *a0) {
    s32 *p = a0;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
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
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
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
    D_800F1178 = (s32)arg0;
    if (D_800F1154[1] != 0) {
        if (D_800F1154[2] != 0) {
            D_800F1154[2] = 0;
            D_800F1154[1] = 0;
        }
        if (D_800F1154[1] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = &D_800F1154[1];
    D_800A3468->unk00.w = 0x210005;
    goto end;
check_one_zero:
    if (D_800F1154[2] == 0) {
        D_800F1180 = (s32)&D_800F1154[2];
        D_800F116C.unk00.w = 0x210006;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF8080;
}

extern u8 D_800F115C;

void func_80061658(s32 *arg0, s32 arg1) {
    /* FAKE: local pointer alias to D_800F116C keeps the address live in $a0
     * across the switch; the direct-global form does not match (as in
     * func_80061710) */
    Unk1F800000Unk00 *v1 = &D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        q = &D_800F115C;
        break;
    case 1:
        val = 0x21000D;
        q = &D_800F115C + 1;
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F1180 = (s32)q;
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
     * across the switch; the direct-global form does not match */
    Unk1F800000Unk00 *v1 = &D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        q = &D_800F115C + 2;
        break;
    case 1:
        val = 0x21000F;
        q = &D_800F115C + 3;
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F1180 = (s32)q;
    v1->unk00.w = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0x10FF10;
}

extern u8 D_800F1160[];

void func_800617C8(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[0] != 0) {
        if (D_800F1160[1] != 0) {
            D_800F1160[1] = 0;
            D_800F1160[0] = 0;
        }
        if (D_800F1160[0] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1160;
    D_800A3468->unk00.w = 0x210010;
    goto end;
check_one_zero:
    if (D_800F1160[1] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 1);
        D_800F116C.unk00.w = 0x210011;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xC06013;
}

extern u8 D_800F1152[];

void func_800618B4(s32 *arg0, s16 *arg1) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)arg0;
    D_800F117C = (s32)arg1;
    if (D_800F1152[0] != 0) {
        if (D_800F1152[1] != 0) {
            D_800F1152[1] = 0;
            D_800F1152[0] = 0;
        }
        if (D_800F1152[0] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1152;
    D_800A3468->unk00.w = 0x210002;
    goto end;
check_one_zero:
    if (D_800F1152[1] == 0) {
        D_800F1180 = (s32)(D_800F1152 + 1);
        D_800F116C.unk00.w = 0x210003;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140.vx = *p++;
    D_800F1140.vy = *p++;
    D_800F1140.vz = *p;
    D_800A3464 = 0xFF0000;
}

extern s32 D_800F1158;

void func_800619A4(s32 *a0) {
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F1158;
    D_800F116C.unk00.w = 0x10008;
    func_80060A68();
}

void func_800619F0(s32 *a0) {
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)(D_800F1154 + 3);
    D_800F116C.unk00.w = 0x10007;
    func_80060A68();
}

extern u8 D_800F1151;

void func_80061A3C(s32 *a0, s16 a1, s32 a2, s32 a3) {
    s16 sp[4];

    D_800A3468 = &D_800F116C;
    sp[2] = 0;
    sp[0] = 0;
    sp[1] = a1;
    D_800F1178 = (s32)a0;
    D_800F117C = (s32)sp;
    if (a3 == 0) {
        D_800F1180 = (s32)&D_800F1150;
        D_800F116C.unk00.w = a2 + 0x10000;
    } else {
        D_800F1180 = (s32)&D_800F1151;
        D_800F116C.unk00.w = a2 + 0x10001;
    }
    func_80060A68();
}

extern u8 D_800F1164[];

void func_80061ACC(s32 *arg0, s32 arg1) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)arg0;
    D_800F117C = arg1;
    if (D_800F1164[0] != 0) {
        if (D_800F1164[1] != 0) {
            D_800F1164[1] = 0;
            D_800F1164[0] = 0;
        }
        if (D_800F1164[0] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1164;
    D_800A3468->unk00.w = 0x210014;
    func_80060A68();
    D_800A3468->unk14 = D_800F1164 - 0xD;
    D_800A3468->unk00.w = 0x10007;
    goto end;
check_one_zero:
    if (D_800F1164[1] == 0) {
        D_800F1180 = (s32)(D_800F1164 + 1);
        D_800A3468->unk00.w = 0x210015;
        func_80060A68();
        D_800A3468->unk14 = D_800F1164 - 0xD;
        D_800A3468->unk00.w = 0x10007;
    }
end:
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
    if ((D_800F1164 + 2)[0] != 0) {
        if ((D_800F1164 + 2)[1] != 0) {
            (D_800F1164 + 2)[1] = 0;
            (D_800F1164 + 2)[0] = 0;
        }
        if ((D_800F1164 + 2)[0] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1164 + 2;
    D_800A3468->unk00.w = 0x10016;
    D_800A34F0[0] = arg2;
    goto end;
check_one_zero:
    if ((D_800F1164 + 2)[1] == 0) {
        D_800A3468->unk14 = D_800F1164 + 3;
        D_800A3468->unk00.w = 0x10017;
        D_800A34F0[1] = arg2;
    }
end:
    func_80060A68();
}

extern u8 D_800F1168[];

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
    if (D_800F1168[0] != 0) {
        if (D_800F1168[1] != 0) {
            D_800F1168[1] = 0;
            D_800F1168[0] = 0;
        }
        if (D_800F1168[0] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1168;
    D_800A3468->unk00.w = 0x10018;
    goto end;
check_one_zero:
    if (D_800F1168[1] == 0) {
        D_800A3468->unk14 = D_800F1168 + 1;
        D_800A3468->unk00.w = 0x10019;
    }
end:
    func_80060A68();
}

void func_80061EC0(s32 *arg0) {
    s32 *p;
    D_800A3468 = &D_800F116C;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[2] != 0) {
        if (D_800F1160[3] != 0) {
            D_800F1160[3] = 0;
            D_800F1160[2] = 0;
        }
        if (D_800F1160[2] != 0)
            goto check_one_zero;
    }
    D_800A3468->unk14 = D_800F1160 + 2;
    D_800A3468->unk00.w = 0x210012;
    goto end;
check_one_zero:
    if (D_800F1160[3] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 3);
        D_800F116C.unk00.w = 0x210013;
    }
end:
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
    s32 t;
    t = arg0[0].unk0;
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0)
        goto end;
    do {
        D_800F1198[i].unk0 = arg0[i].unk0;
        D_800F1198[i].unk4 = arg0[i].unk4;
        D_800F1198[i].unk8 = arg0[i].unk8;
        i = i + 1;
        t = arg0[i].unk0;
    } while ((t & 1) != 0);
end:
    D_800F1198[i].unk0 = D_800F1198[i].unk4 = D_800F1198[i].unk8 = 0;
}

/* Draw the D_800F1198 effect particles (see func_80062020): per record until
   the terminator, pick a sprite size and an animation frame from the record's
   type, rotate/translate its position, project it, and when it lands in range
   emit one textured POLY_FT4 billboard and link it into the OT at its depth. */
void func_800620B8(s16 *arg0, s32 *trans) {
    extern s32 D_800A32B8;
    extern TexRec D_8009BA00[6];
    extern TexRec D_8009BA30[4];
    extern TexRec D_8009BA50[1];
    extern TexRec D_8009BA58[1];
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    extern s32 rand(void);
    Unk1F8000B8_800620B8 *base;
    s16 *w;
    s16 *h;
    VECTOR *tv;
    VECTOR *v;
    SVECTOR *sv;
    s32 *interp;
    u32 *z;
    POLY_FT4 *prim;
    Unk1F800000Unk00 *outer;
    s16 *dst16;
    s32 *dst32;
    MATRIX *rot;
    s16 i;
    s32 proj_w;
    s32 proj_h;
    s16 width;
    s16 height;
    /* FAKE: pointer aliases of the four sprite tables
     * (pointer-alias-fake-exception): strip32 is set at the loop top so it is
     * hoisted where the target sets $fp; the other three are rebuilt in $t0 at
     * each use. Tables used directly: 47; any one of the three: 3-7. */
    TexRec *strip32; /* FAKE: alias of D_8009BA00 */
    TexRec *alt32;   /* FAKE: alias of D_8009BA50 */
    TexRec *strip16; /* FAKE: alias of D_8009BA30 */
    TexRec *alt16;   /* FAKE: alias of D_8009BA58 */

    func_80060E38(arg0, trans);
    outer = D_800A3468;
    dst16 = D_800A346C;
    dst16[0] = outer->unk04[0];
    prim = D_800A37D4;
    dst16[1] = outer->unk04[1];
    dst16[2] = outer->unk04[2];
    dst32 = D_800A3470;
    /* the matrix func_80061FAC builds from the angles at arg0 */
    rot = D_800A3474;
    dst32[0] = outer->unk08[0];
    base = &D_800A34EC->v800620B8;
    dst32[1] = outer->unk08[1];
    dst32[2] = outer->unk08[2];
    D_800A32B8++;
    func_80061FAC(dst16, dst32, rot);
    SetRotMatrix(D_800A3474);
    w = &base->unk10;
    h = &base->unk12;
    tv = &base->unk14;
    v = &base->unk24;
    sv = &base->unk34;
    interp = &base->unk3C;
    z = &base->unk44;
    sv->vz = 0;
    sv->vy = 0;
    sv->vx = 0;
    *D_800A34B0 = ReadGeomScreen() << 8;
    *D_800A3490 = 0x2F;
    alt32 = D_8009BA50;   /* FAKE: alias; direct use scores 3 */
    strip16 = D_8009BA30; /* FAKE: alias; direct use scores 3 */
    alt16 = D_8009BA58;   /* FAKE: alias; direct use scores 7 */
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
        strip32 = D_8009BA00; /* FAKE: alias, set here so loop.c hoists it */
        switch (D_800F1198[i].unk4 & 7) {
        case 3:
            *D_800A34A8 = 0x151;
            *D_800A34AC = 0xA8;
            goto sel_a;
        case 0:
            *D_800A34A8 = 0x1C2;
            *D_800A34AC = 0xE1;
        sel_a:
            /* FAKE: `- strip32 + strip32` round trip: combine folds it away,
             * but the two extra uses rank strip32 high enough to get $fp;
             * without it: 35. As pointer arithmetic the addu operands swap: 4;
             * plain `&strip32[n]`: 38. (combine-foldable chain-extender,
             * dead-store-fake-exception) */
            D_800A348C = D_800A3488 =
                (TexRec *)(((u32)D_800A32B8 % 6) * sizeof(*strip32) +
                           (s32)strip32 - (s32)strip32 + (s32)strip32);
            if (D_8009BD24.unk20_0) {
                D_800A348C = alt32;
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
            /* FAKE: the record address is an integer sum converted to TexRec *:
               as `strip16 + n` or `&strip16[n]` strip16 comes first in the addu
               (`addu v0,t0,v0` for the target's `addu v0,v0,t0`): score 1. */
            D_800A348C = D_800A3488 =
                (TexRec *)((D_800A32B8 & 3) * sizeof(*strip16) + (s32)strip16);
            if (D_8009BD24.unk20_0) {
                D_800A348C = alt16;
            }
            *D_800A349C = D_800A3488->u + 0xF;
            *D_800A34A4 = D_800A3488->v + 0x13;
            break;
        }
        v->vx = D_800F1198[i].unk0 / 2 - D_800A3470[0];
        v->vy = D_800F1198[i].unk4 / 8 - D_800A3470[1];
        v->vz = D_800F1198[i].unk8 - D_800A3470[2];
        ApplyRotMatrixLV(v, tv);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the address 0x14
         * below tv, it loads tv as the translation (there is no whole MATRIX
         * here). Spelled (MATRIX *)base: score 65. */
        SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));
        RotTransPers(sv, D_800A34B8, interp, D_800A34CC);
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");
        *z = func_80052C28(*D_800A34D0, 0);
        if (*z < 0x1005 && (*D_800A34B0 / 256 >> 4) < *z) {
            *D_800A3494 = (u16)(((D_800A348C->clut_x >> 4) & 0x3F) +
                                (D_800A348C->clut_y << 6));
            *D_800A3498 = D_800A3488->u;
            *D_800A34A0 = D_800A3488->v;
            *D_800A34D0 = *D_800A34D0 ? *D_800A34D0 : 1;
            *D_800A34B4 = *D_800A34B0 / *D_800A34D0;
            proj_w = *D_800A34A8 * *D_800A34B4;
            width = proj_w > 0x200 ? proj_w >> 8 : 2;
            *w = width / 2;
            proj_h = *D_800A34AC * *D_800A34B4;
            height = proj_h > 0x200 ? proj_h >> 8 : 2;
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
    extern s32 D_800F1138;
    s32 *pos;
    s16 *rot;
    s32 i;
    s32 mask;

    D_800F1138 = 1;
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
    extern TexRec D_8009B958[];
    extern TexRec D_8009B960[];
    extern TexRec D_8009B968[];
    extern TexRec D_8009B970[];
    extern s16 D_8009BB84[];
    s32 count;
    MATRIX *cm;
    s32 *interp;
    u16 *zbuf;
    s32 *scale;
    s32 *shade;
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
    s16 *sv;
    POLY_FT4 *end;

    /* FAKE: the work-area base (D_800A34EC) is staged through `prim` before
     * prim becomes the quad cursor; keeps `mats = base + 0x78` out of the
     * loop's giv init (target keeps `move s4,v0`) and puts base in s1. */
    prim = (POLY_FT4 *)D_800A34EC;
    count = 0;
    mats = (MATRIX *)((u8 *)prim + 0x78);
    cm = (MATRIX *)((u8 *)prim + 0x138);
    sxy = (s32 *)((u8 *)prim + 0x158);
    interp = (s32 *)((u8 *)prim + 0x168);
    zbuf = (u16 *)((u8 *)prim + 0x16C);
    scale = (s32 *)((u8 *)prim + 0x178);
    shade = (s32 *)((u8 *)prim + 0x188);
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
        CompMatrix(D_800A3474, m, cm);
        SetRotMatrix(cm);
        SetTransMatrix(cm);
        for (j = 0; j < 3; j++) {
            if (D_800F0C04[i] < 3) {
                *(s32 *)&prim->r0 = 0x808080;
            } else {
                c = ((6 - D_800F0C04[i]) << 7) / 3;
                *shade = c;
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
            sv = &D_8009BB84[j * 16];
            RotTransPers4(sv, sv + 4, sv + 8, sv + 12, sxy, sxy + 1, sxy + 2,
                          sxy + 3, interp, D_800A34CC);
            /* gte_stsz(r0) --- inline_c.h :1042-1046 */
            __asm__ volatile(
                "swc2   $19, 0(%0)\n"
                :: "r"(D_800A34D0) : "memory");
            *D_800A34D0 = func_80052C28(*D_800A34D0, 0);
            if (*D_800A34D0 == 0) {
                *D_800A34D0 = 1;
            }
            if (*D_800A34D0 < 0x1005) {
                zbuf[count] = *D_800A34D0;
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
            AddPrim(g_gpu_ot_ptr + zbuf[k], prim);
        }
        D_800A37D4 = end;
        return 1;
    }
    if (D_800A3460 == 0) {
        D_800F1138 = 0;
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

    D_800F10F0 = 1;
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
    extern TexRec D_8009B940[];
    extern TexRec D_8009B948[];
    extern TexRec D_8009B950[];
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80063084 *base;
    POLY_FT4 *prim;
    VECTOR *tv;
    SVECTOR *sv;
    SVECTOR *v;
    s32 *interp;
    s32 *fade;
    s32 *z;
    s16 i;
    s16 j;
    s32 bit;
    s32 scale;
    s32 level;

    base = &D_800A34EC->v80063084;
    prim = D_800A37D4;
    tv = &base->unk14;
    sv = &base->unk24;
    v = &base->unk2C;
    interp = &base->unk34;
    fade = &base->unk38;
    z = &base->unk3C;
    /* gte_SetRotMatrix(r0) --- PsyQ 4.3 inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");
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
                sv->vx = D_800F0E38[i].unk0 - D_800A3470[0];
                sv->vz = D_800F0E38[i].unk8 - D_800A3470[2];
                if (j != 0 && D_800F0BEC[i] >= 3) {
                    sv->vy =
                        D_800F0E38[i].unk4 -
                        (rsin((D_800F0BEC[i] - 3) << 8) * 329 / 4096 - 69) /
                            10 -
                        D_800A3470[1];
                } else {
                    sv->vy = D_800F0E38[i].unk4 - D_800A3470[1];
                }
                ApplyRotMatrix(sv, tv);
                /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the
                 * address 0x14 below tv, it loads tv as the translation.
                 * Spelled (MATRIX *)base: score 55. */
                SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));
                RotTransPers(v, D_800A34B8, interp, D_800A34CC);
                /* gte_stsz(r0) --- PsyQ 4.3 inline_c.h :1042-1046 */
                __asm__ volatile(
                    "swc2   $19, 0(%0)\n"
                    :: "r"(D_800A34D0) : "memory");
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
                            /* fade = age - 15 <= 0: the halo fades out over
                             * ages 9..15. */
                            *fade = D_800F0BEC[i] - 15;
                            level = (*fade * -128 / 5) & 0xFF;
                            *(s32 *)&prim->r0 =
                                level + (level << 8) +
                                ((*fade * -255 / 5 & 0xFF) << 16);
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

extern s32 D_800F10D4;

u8 func_80063B34(void) {
    /* FAKE: the word is read through an s32 *: a member read rises above
     * the D_800F10D4 store and fills its load delay: score 6 */
    s32 *v1 = &D_800A3468->unk00.w;
    D_800F10D4 = 1;
    D_800A345C[1] = (*v1 >> 17) & 3;
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

/* Draw lane `lane`'s live slots (up to 10, see func_80063BD0): per slot whose
 * bit is set in D_800A3454[lane], emit one textured POLY_FT4 billboard at the
 * slot's position (relative to *D_800A3470) through the composite of
 * D_800A3474 and the slot's matrix, keep it when its depth is in range, then
 * link every new quad into the OT at its depth. Returns 1.
 * GTE islands, one PsyQ 4.3 inline_c.h macro each: gte_SetRotMatrix,
 * gte_ldclmv, gte_rtir, gte_stclmv, gte_SetTransMatrix, gte_ldlv0, gte_rt,
 * gte_stlvnl, gte_ldv3, gte_rtpt, gte_stsxy3, gte_stsz, gte_ldv0, gte_rtps,
 * gte_stsxy -- the header's text except separators/whitespace and, in the four
 * command macros, the post-DMPSX command word for the header's DMPSX
 * placeholder. The run from the first gte_SetRotMatrix through gte_stlvnl is
 * gtemac.h gte_CompMatrix (= gte_MulMatrix0 +
 * gte_SetTransMatrix/gte_ldlv0/gte_rt/gte_stlvnl), written out macro by macro.
 */
s32 func_80063E10(s32 lane) {
    extern TexRec D_8009B920[];
    extern SVECTOR D_8009BBE4;
    extern SVECTOR D_8009BBEC;
    extern SVECTOR D_8009BBF4;
    extern SVECTOR D_8009BBFC;
    s32 count;
    MATRIX *mats;
    MATRIX *cm;
    s32 *sxy;
    u16 *zbuf;
    u16 *zn;
    POLY_FT4 *prim;
    POLY_FT4 *end;
    s32 i;
    s32 k;
    s32 bit;

    /* FAKE: the work-area base (D_800A34EC) is staged through `prim` before
     * prim becomes the quad cursor: prim's pseudo lives across the calls and
     * gets s2, where the target holds the base (a fresh local gets v0). */
    prim = (POLY_FT4 *)D_800A34EC;
    mats = (MATRIX *)((u8 *)prim + 0x28);
    cm = (MATRIX *)((u8 *)prim + 0x168);
    sxy = (s32 *)((u8 *)prim + 0x188);
    zbuf = (u16 *)((u8 *)prim + 0x19C);
    zn = (u16 *)((u8 *)prim + 0x1B0);
    if (D_800A344C[lane] < 10) {
        count = D_800A344C[lane];
    } else {
        count = 10;
    }
    func_800644FC(&count, mats, lane);
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
    *zn = 0;
    *D_800A34B0 = ReadGeomScreen();
    for (i = 0; i < count; i++) {
        /* FAKE: the slot's mask is named `bit` inside the test and not read
         * again; it stretches the D_800A3454[lane] address's life so loop.c
         * hoists it (target spills it to 32(sp)), and keeps combine from making
         * the test srav/andi. */
        if (!(D_800A3454[lane] & (bit = 1 << i))) {
            continue;
        }
        setlen(prim, 9);
        prim->code = 0x2F;
        *(s32 *)&prim->u0 = *D_800A34D4 + *D_800A3494;
        *(s32 *)&prim->u1 = *D_800A34D8 + *D_800A3490;
        *(u16 *)&prim->u2 = *D_800A34DC;
        *(u16 *)&prim->u3 = *D_800A34E0;
        mats[i].t[0] = D_800F0EC8[lane][i].unk0 - D_800A3470[0];
        mats[i].t[1] = D_800F0EC8[lane][i].unk4 - D_800A3470[1];
        mats[i].t[2] = D_800F0EC8[lane][i].unk8 - D_800A3470[2];
        /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
        __asm__ volatile(
            "lw     $12, 0(%0)\n"
            "lw     $13, 4(%0)\n"
            "ctc2   $12, $0\n"
            "ctc2   $13, $1\n"
            "lw     $12, 8(%0)\n"
            "lw     $13, 12(%0)\n"
            "lw     $14, 16(%0)\n"
            "ctc2   $12, $2\n"
            "ctc2   $13, $3\n"
            "ctc2   $14, $4\n"
            :: "r"(D_800A3474) : "$12", "$13", "$14");
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"(&mats[i]) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517; post-DMPSX word 0x4A49E012
           replaces the header's DMPSX placeholder .word 0x000001ff */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"(cm) : "$12", "$13", "$14", "memory");
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"((u8 *)&mats[i] + 2) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517; post-DMPSX word 0x4A49E012
           replaces the header's DMPSX placeholder .word 0x000001ff */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"((u8 *)cm + 2) : "$12", "$13", "$14", "memory");
        /* gte_ldclmv(r0) --- inline_c.h :150-159 */
        __asm__ volatile(
            "lhu    $12, 0(%0)\n"
            "lhu    $13, 6(%0)\n"
            "lhu    $14, 12(%0)\n"
            "mtc2   $12, $9\n"
            "mtc2   $13, $10\n"
            "mtc2   $14, $11\n"
            :: "r"((u8 *)&mats[i] + 4) : "$12", "$13", "$14");
        /* gte_rtir() --- inline_c.h :514-517; post-DMPSX word 0x4A49E012
           replaces the header's DMPSX placeholder .word 0x000001ff */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A49E012\n");
        /* gte_stclmv(r0) --- inline_c.h :1148-1157 */
        __asm__ volatile(
            "mfc2   $12, $9\n"
            "mfc2   $13, $10\n"
            "mfc2   $14, $11\n"
            "sh     $12, 0(%0)\n"
            "sh     $13, 6(%0)\n"
            "sh     $14, 12(%0)\n"
            :: "r"((u8 *)cm + 4) : "$12", "$13", "$14", "memory");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"(D_800A3474) : "$12", "$13", "$14");
        /* gte_ldlv0(r0) --- inline_c.h :101-110 */
        __asm__ volatile(
            "lhu    $13, 4(%0)\n"
            "lhu    $12, 0(%0)\n"
            "sll    $13, $13, 16\n"
            "or     $12, $12, $13\n"
            "mtc2   $12, $0\n"
            "lwc2   $1, 8(%0)\n"
            :: "r"(mats[i].t) : "$12", "$13");
        /* gte_rt() --- inline_c.h :494-497; post-DMPSX word 0x4A480012
           replaces the header's DMPSX placeholder .word 0x000000ff */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A480012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(cm->t) : "memory");
        /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
        __asm__ volatile(
            "lw     $12, 0(%0)\n"
            "lw     $13, 4(%0)\n"
            "ctc2   $12, $0\n"
            "ctc2   $13, $1\n"
            "lw     $12, 8(%0)\n"
            "lw     $13, 12(%0)\n"
            "lw     $14, 16(%0)\n"
            "ctc2   $12, $2\n"
            "ctc2   $13, $3\n"
            "ctc2   $14, $4\n"
            :: "r"(cm) : "$12", "$13", "$14");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"(cm) : "$12", "$13", "$14");
        /* gte_ldv3(r0, r1, r2) --- inline_c.h :34-42 */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            "lwc2   $2, 0(%1)\n"
            "lwc2   $3, 4(%1)\n"
            "lwc2   $4, 0(%2)\n"
            "lwc2   $5, 4(%2)\n"
            :: "r"(&D_8009BBE4), "r"(&D_8009BBEC), "r"(&D_8009BBF4));
        /* gte_rtpt() --- inline_c.h :489-492; post-DMPSX word 0x4A280030
           replaces the header's DMPSX placeholder .word 0x000000bf */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A280030\n");
        /* gte_stsxy3(r0, r1, r2) --- inline_c.h :906-912 */
        __asm__ volatile(
            "swc2   $12, 0(%0)\n"
            "swc2   $13, 0(%1)\n"
            "swc2   $14, 0(%2)\n"
            :: "r"(sxy), "r"(sxy + 1), "r"(sxy + 2) : "memory");
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");
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
        zbuf[(*zn)++] = *D_800A34D0;
        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(&D_8009BBFC));
        /* gte_rtps() --- inline_c.h :484-487; post-DMPSX word 0x4A180001
           replaces the header's DMPSX placeholder .word 0x0000007f */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A180001\n");
        /* gte_stsxy(r0) --- inline_c.h :900-904 */
        __asm__ volatile(
            "swc2   $14, 0(%0)\n"
            :: "r"(sxy + 3) : "memory");
        *(s32 *)&prim->x0 = sxy[0];
        *(s32 *)&prim->x1 = sxy[1];
        *(s32 *)&prim->x2 = sxy[2];
        *(s32 *)&prim->x3 = sxy[3];
        if (prim - D_800A3720 < 0x1C1) {
            prim++;
        }
    }
    /* FAKE: `end` keeps the fill position and prim walks the same buffer again
     * to link each quad; with a fresh tail cursor sxy and prim swap s2/s3. */
    end = prim;
    for (prim = D_800A37D4, k = 0; prim < end; prim++, k++) {
        D_800A34E8 = &prim->tag;
        D_800A34E4 = g_gpu_ot_ptr + zbuf[k];
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

    D_800F10EC = 1;
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
   of D_800A3444) advance its counter, and while the counter's frame (/4) is
   below 7 project the slot's world position through D_800A3474 and emit one
   textured, semi-transparent POLY_FT4 billboard sized by the screen depth;
   once the frame reaches 7 the slot is retired. Finally link every new quad
   into the OT at its depth. Returns 1 when any slot is still live, else 0. */
s32 func_800646E8(void) {
    extern TexRec D_8009B8E8[];
    Unk1F8000B8_800646E8 *base;
    u32 *zbuf;
    s16 *w;
    s16 *h;
    s32 *frame;
    VECTOR *trans;
    VECTOR *pos;
    SVECTOR *sv;
    s32 *p;
    POLY_FT4 **end;
    POLY_FT4 *prim;
    u32 *zp;
    s16 i;
    s32 bit;

    base = &D_800A34EC->v800646E8;
    zbuf = base->unk48;
    w = &base->unk10;
    h = &base->unk12;
    frame = &base->unk14;
    trans = &base->unk18;
    pos = &base->unk28;
    sv = &base->unk38;
    p = &base->unk40;
    end = &base->unk44;
    prim = D_800A37D4;
    /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");
    sv->vx = sv->vy = sv->vz = 0;
    *D_800A34B0 = ReadGeomScreen() * 1000;
    *D_800A34A8 = 0x40;
    *D_800A34AC = 0x20;
    *D_800A3490 = 0xE;
    for (i = 0, zp = zbuf; i < 16; i++) {
        bit = 1 << i;
        if (!(D_800A3444 & bit)) {
            continue;
        }
        D_800F0BCC[i]++;
        *frame = D_800F0BCC[i] / 4;
        if (*frame < 7) {
            pos->vx = D_800F0D78[i].x - D_800A3470[0];
            pos->vy = D_800F0D78[i].y - D_800A3470[1];
            pos->vz = D_800F0D78[i].z - D_800A3470[2];
            ApplyRotMatrixLV(pos, trans);
            /* FAKE: the MATRIX whose t[] is *trans: SetTransMatrix reads only
             * m->t (there is no whole MATRIX here). Spelled
             * (MATRIX *)((u8 *)base + 4): score 68. */
            SetTransMatrix((MATRIX *)((u8 *)trans - 0x14));
            RotTransPers(sv, D_800A34B8, p, D_800A34CC);
            /* gte_stsz(r0) --- inline_c.h :1042-1046 */
            __asm__ volatile(
                "swc2   $19, 0(%0)\n"
                :: "r"(D_800A34D0) : "memory");
            D_800A3488 = &D_8009B8E8[*frame];
            *D_800A3494 = (u16)(((D_800A3488->clut_x >> 4) & 0x3F) +
                                (D_800A3488->clut_y << 6));
            *D_800A3498 = D_800A3488->u;
            *D_800A34A0 = D_800A3488->v;
            *D_800A349C = D_800A3488->u + 0x3F;
            *D_800A34A4 = D_800A3488->v + 0x1F;
            *zp = func_80052C28(*D_800A34D0, 0);
            if (*zp >= 0x1005) {
                continue;
            }
            if ((*D_800A34B0 / 1000 >> 4) >= *zp) {
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
                zp++;
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
    for (prim = D_800A37D4; prim < *end; prim++, zbuf++) {
        D_800A34E4 = g_gpu_ot_ptr + *zbuf;
        D_800A34E8 = &prim->tag;
        *D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*D_800A34E4 & 0xFFFFFF);
        *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
    }
    D_800A37D4 = *end;
    return 1;
}

extern s32 D_800F10E0;

void func_80064E90(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[0].unk0 = D_800A347C[0];
    D_800F0CA0[0].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F10E0 = 1;
    D_800F0BA8[0] = 0;
    D_800F0CA0[0].unk8 = last;
}

extern s32 D_800F10E4;

void func_80064ED8(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[1].unk0 = D_800A347C[0];
    D_800F0CA0[1].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F10E4 = 1;
    D_800F0BA8[1] = 0;
    D_800F0CA0[1].unk8 = last;
}

extern s32 D_800F10E8;

void func_80064F20(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[2].unk0 = D_800A347C[0];
    D_800F0CA0[2].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F10E8 = 1;
    D_800F0BA8[2] = 0;
    D_800F0CA0[2].unk8 = last;
}

extern s32 D_800F10F4;

s32 func_80064F68(void) {
    /* FAKE: the third word is read through a pointer: as D_800A347C[2] the
     * D_800F10F4 store moves above it and the unk4 store: score 7 */
    s32 *q = &D_800A347C[2];
    /* FAKE: named intermediate - with word 2 read inline, the D_800F10F4 store
       rises above the unk0 / unk4 stores and the word-2 read, and the
       D_800F0BA8 store above the unk4 store and the word-2 read (score 12; with
       q as well, 8) */
    s32 last;
    D_800F0CA0[3].unk0 = D_800A347C[0];
    D_800F0CA0[3].unk4 = D_800A347C[1];
    last = *q;
    D_800F10F4 = 1;
    D_800F0BA8[3] = 0x40;
    D_800F0CA0[3].unk8 = last;
    return 1;
}

extern s32 D_800F10F8;

s32 func_80064FB4(void) {
    /* FAKE: the third word is read through a pointer: as D_800A347C[2] the
     * D_800F10F8 store moves above it and the unk4 store: score 7 */
    s32 *q = &D_800A347C[2];
    /* FAKE: named intermediate - with word 2 read inline, the D_800F10F8 store
       rises above the unk0 / unk4 stores and the word-2 read, and the
       D_800F0BA8 store above the unk4 store and the word-2 read (score 12; with
       q as well, 8) */
    s32 last;
    D_800F0CA0[4].unk0 = D_800A347C[0];
    D_800F0CA0[4].unk4 = D_800A347C[1];
    last = *q;
    D_800F10F8 = 1;
    D_800F0BA8[4] = 0x40;
    D_800F0CA0[4].unk8 = last;
    return 1;
}

extern s32 D_800F10FC;

s32 func_80065000(void) {
    /* FAKE: the word is read through an s32 *, not as a member: a member read
     * rises above the D_800F10FC store and fills its load delay: score 9 / 8 */
    s32 *q = &D_800A3468->unk00.w;
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[5].unk0 = D_800A347C[0];
    D_800F0CA0[5].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F10FC = 1;
    D_800F0BA8[5] = 0;
    D_800F0CA0[5].unk8 = last;
    D_800A3440 = (*q >> 19) & 3;
    return 1;
}

extern s32 D_800F1100;

void func_8006505C(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[6].unk0 = D_800A347C[0];
    D_800F0CA0[6].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F1100 = 1;
    D_800F0BA8[6] = 0;
    D_800F0CA0[6].unk8 = last;
}

extern s32 D_800F1104;

void func_800650A4(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[7].unk0 = D_800A347C[0];
    D_800F0CA0[7].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F1104 = 1;
    D_800F0BA8[7] = 0;
    D_800F0CA0[7].unk8 = last;
}

extern s32 D_800F1108;

void func_800650EC(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[10].unk0 = D_800A347C[0];
    D_800F0CA0[10].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F1108 = 1;
    D_800F0BA8[10] = 0;
    D_800F0CA0[10].unk8 = last;
}

extern s32 D_800F110C;

void func_80065134(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[11].unk0 = D_800A347C[0];
    D_800F0CA0[11].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F110C = 1;
    D_800F0BA8[11] = 0;
    D_800F0CA0[11].unk8 = last;
}

extern s32 D_800F1110;

void func_8006517C(void) {
    /* FAKE: named intermediates - with a copy's word 2 stored directly, that
       copy's D_800F0BA8 store (`sh`) rises above its unk4 store and the word-2
       read (score 4 each, 8 both) */
    s32 t;
    s32 t2;
    D_800F0CA0[12].unk0 = D_800A347C[0];
    D_800F0CA0[12].unk4 = D_800A347C[1];
    t = D_800A347C[2];
    D_800F0BA8[12] = 0;
    D_800F0CA0[12].unk8 = t;
    D_800F0CA0[14].unk0 = D_800A347C[0];
    D_800F0CA0[14].unk4 = D_800A347C[1];
    t2 = D_800A347C[2];
    D_800F1110 = 1;
    D_800F0BA8[14] = 0;
    D_800F0CA0[14].unk8 = t2;
}

extern s32 D_800F1114;

void func_800651F0(void) {
    /* FAKE: named intermediates - with a copy's word 2 stored directly, that
       copy's D_800F0BA8 store (`sh`) rises above its unk4 store and the word-2
       read (score 4 each, 8 both) */
    s32 t;
    s32 t2;
    D_800F0CA0[13].unk0 = D_800A347C[0];
    D_800F0CA0[13].unk4 = D_800A347C[1];
    t = D_800A347C[2];
    D_800F0BA8[13] = 0;
    D_800F0CA0[13].unk8 = t;
    D_800F0CA0[15].unk0 = D_800A347C[0];
    D_800F0CA0[15].unk4 = D_800A347C[1];
    t2 = D_800A347C[2];
    D_800F1114 = 1;
    D_800F0BA8[15] = 0;
    D_800F0CA0[15].unk8 = t2;
}

extern s32 D_800F1118;

void func_80065264(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[16].unk0 = D_800A347C[0];
    D_800F0CA0[16].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F1118 = 1;
    D_800F0BA8[16] = 0;
    D_800F0CA0[16].unk8 = last;
}

extern s32 D_800F111C;

void func_800652AC(void) {
    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store
       (`sh`) rises above the unk4 store and the word-2 read (score 4) */
    s32 last;
    D_800F0CA0[17].unk0 = D_800A347C[0];
    D_800F0CA0[17].unk4 = D_800A347C[1];
    last = D_800A347C[2];
    D_800F111C = 1;
    D_800F0BA8[17] = 0;
    D_800F0CA0[17].unk8 = last;
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
 * `arg0 < 9`, so modes 10/11 draw once.
 * GTE: eight PsyQ 4.3 inline_c.h macros; gte_rtps carries the post-DMPSX
 * command word. */
u8 func_80065800(s32 arg0) {
    extern s16 D_800A3834;
    extern TexRec D_8009B8C8[];
    extern TexRec D_8009B8D0[];
    extern TexRec D_8009B8D8[];
    extern TexRec D_8009B8E0[];
    extern TexRec D_8009B978[];
    extern TexRec D_8009B980[];
    extern TexRec D_8009B988[];
    extern TexRec D_8009B990[];
    extern TexRec D_8009B9D8[];
    extern TexRec D_8009B9E0[];
    extern TexRec D_8009B9E8[];
    extern TexRec D_8009B9F0[];
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80065800 *outer;
    POLY_FT4 *prim;
    s32 *p_dp;
    VECTOR *p_t;
    VECTOR *p_in;
    SVECTOR *p_v;
    s16 *p_w;
    s16 *p_h;
    MATRIX *p_mat;
    s16 *p_tw;
    s16 *p_th;
    VECTOR *dst;
    s32 n;
    /* FAKE: named intermediate - *p_w read once before the corner sign test
     * (inline: score 7) */
    s32 w;
    /* FAKE: named intermediate - the width scale multiplied into *p_w whole
     * (inline: score 10) */
    s32 sw;
    /* FAKE: named intermediate - the height scale multiplied into *p_h whole
     * (inline: score 9) */
    s32 sh;
    /* FAKE: named intermediate - *p_h read once before the corner sign test
     * (inline: score 88) */
    s32 h;
    s16 *t;
    /* FAKE: pointer alias of D_800F0BA8 - case 10/11's base address in its own
     * register ahead of the index shift */
    s16 *tbl;
    s32 i;

    outer = &D_800A34EC->v80065800;
    prim = D_800A37D4;
    p_dp = &outer->unk10;
    p_t = outer->unk14;
    p_in = &outer->unk54;
    p_v = &outer->unk64;
    p_w = &outer->unk6C;
    p_h = &outer->unk70;
    p_mat = &outer->unk74;
    p_tw = &outer->unk94;
    p_th = &outer->unk96;

    /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");
    p_in->vx = D_800F0CA0[arg0].unk0 - D_800A3470[0];
    p_in->vy = D_800F0CA0[arg0].unk4 - D_800A3470[1];
    p_in->vz = D_800F0CA0[arg0].unk8 - D_800A3470[2];
    ApplyRotMatrixLV(p_in, p_t);
    /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
    __asm__ volatile(
        "lw     $12, 20(%0)\n"
        "lw     $13, 24(%0)\n"
        "ctc2   $12, $5\n"
        "lw     $14, 28(%0)\n"
        "ctc2   $13, $6\n"
        "ctc2   $14, $7\n"
        :: "r"((MATRIX *)outer) : "$12", "$13", "$14");
    p_v->vz = 0;
    p_v->vy = 0;
    p_v->vx = 0;
    /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
    __asm__ volatile(
        "lwc2   $0, 0(%0)\n"
        "lwc2   $1, 4(%0)\n"
        :: "r"(p_v));
    /* gte_rtps() --- inline_c.h :484-487; post-DMPSX word 0x4A180001 (RTPS
       sf=1) replaces the header's DMPSX placeholder .word 0x0000007f */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4A180001\n");
    /* gte_stsxy(r0) --- inline_c.h :900-904 */
    __asm__ volatile(
        "swc2   $14, 0(%0)\n"
        :: "r"(D_800A34B8) : "memory");
    /* gte_stdp(r0) --- inline_c.h :1018-1022 */
    __asm__ volatile(
        "swc2   $8, 0(%0)\n"
        :: "r"(p_dp) : "memory");
    /* gte_stflg(r0) --- inline_c.h :1024-1030 */
    __asm__ volatile(
        "cfc2   $12, $31\n"
        "nop\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34CC) : "$12", "memory");
    /* gte_stszotz(r0) --- inline_c.h :1082-1089 */
    __asm__ volatile(
        "mfc2   $12, $19\n"
        "nop\n"
        "sra    $12, $12, 2\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34D0) : "$12", "memory");
    *D_800A34D0 = *D_800A34D0 ? *D_800A34D0 << 2 : 1;

again:
    *D_800A34A8 = 0x20;
    *D_800A34AC = 0x20;
    *p_tw = 0x40;
    *p_th = 0x40;
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
            *p_tw = 0x40;
            *p_th = 0x10;
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
    *D_800A349C = *p_tw + D_800A3488->u - 1;
    *D_800A34A4 = *p_th + D_800A3488->v - 1;
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
        sw = (rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25;
        *p_w = (*p_w * sw >> 13) / 2;
        sh = rsin((D_800F0BA8[arg0] << 11) / 4551) * 15;
        *p_h = (*p_h * sh >> 12) / 2;
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

extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);

u8 func_80066EC0(void) {
    u8 ret = func_80067200(0, 0, 0);
    D_800F10D8 = 1;
    return ret;
}

u8 func_80066EF4(void) {
    u8 ret = func_80067200(0, 0, 1);
    D_800F10D8 = 2;
    return ret;
}

extern s32 D_800F10DC;

u8 func_80066F28(void) {
    u8 ret = func_80067200(1, 1, 0);
    D_800F10DC = 1;
    return ret;
}

u8 func_80066F5C(void) {
    u8 ret = func_80067200(1, 1, 1);
    D_800F10DC = 2;
    return ret;
}

extern s32 D_800F1120;

u8 func_80066F90(void) {
    u8 ret = func_80067200(2, 1, 0);
    D_800F1120 = 1;
    return ret;
}

u8 func_80066FC4(void) {
    u8 ret = func_80067200(2, 1, 1);
    D_800F1120 = 2;
    return ret;
}

extern s32 D_800F1124;

u8 func_80066FF8(void) {
    u8 ret = func_80067200(3, 0, 0);
    D_800F1124 = 1;
    return ret;
}

u8 func_8006702C(void) {
    u8 ret = func_80067200(3, 0, 1);
    D_800F1124 = 2;
    return ret;
}

extern s32 D_800F1128;

u8 func_80067060(void) {
    u8 ret = func_80067200(4, 2, 0);
    D_800F1128 = 1;
    return ret;
}

u8 func_80067094(void) {
    u8 ret = func_80067200(4, 2, 1);
    D_800F1128 = 2;
    return ret;
}

extern s32 D_800F112C;

u8 func_800670C8(void) {
    u8 ret = func_80067200(5, 3, 0);
    D_800F112C = 1;
    return ret;
}

u8 func_800670FC(void) {
    u8 ret = func_80067200(5, 3, 1);
    D_800F112C = 2;
    return ret;
}

extern s32 D_800F1130;

u8 func_80067130(void) {
    u8 ret = func_80067200(6, 3, 0);
    D_800F1130 = 1;
    return ret;
}

u8 func_80067164(void) {
    u8 ret = func_80067200(6, 3, 1);
    D_800F1130 = 2;
    return ret;
}

extern s32 D_800F1134;

u8 func_80067198(void) {
    u8 ret = func_80067200(7, 2, 0);
    D_800F1134 = 1;
    return ret;
}

u8 func_800671CC(void) {
    u8 ret = func_80067200(7, 2, 1);
    D_800F1134 = 2;
    return ret;
}

extern SVECTOR D_800F0B78[];
extern Unk800EFC78Record D_800EFC78[][48];

/* func_80067200 -- GTE: gte_SetRotMatrix (twice), gte_ldv0, gte_rtv0,
 * gte_stlvnl (PsyQ 4.3 inline_c.h). gte_rtv0 carries the post-DMPSX word
 * 0x4A486012 (MVMVA sf=1 mx=rot v=V0 cv=none lm=0) for the header's DMPSX
 * placeholder `.word 0x0000013f`. */
u8 func_80067200(s32 arg0, s32 arg1, s32 arg2) {
    SVECTOR v;
    s32 r[6];
    MATRIX m;
    SVECTOR ang;
    VECTOR out;
    SVECTOR sc;
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
    /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");
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
    sc.vx = D_800F0B78[arg1].vx;
    sc.vy = D_800F0B78[arg1].vy;
    sc.vz = D_800F0B78[arg1].vz;
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
        v.vx = sc.vx * (base + r[3]) / mask;
        v.vy = sc.vy * (base + r[4]) / mask;
        v.vz = sc.vz * (base + r[5]) / mask;
        p = &D_800EFC78[arg1][i];
        p->unk6 = i / 16;
        p->unk12 = 0;
        p->unk4 = 0;
        p->unk2 = 0;
        p->unk0 = 0;
        p->unk10 = 1;
        RotMatrix(&ang, &m);
        /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
        __asm__ volatile(
            "lw     $12, 0(%0)\n"
            "lw     $13, 4(%0)\n"
            "ctc2   $12, $0\n"
            "ctc2   $13, $1\n"
            "lw     $12, 8(%0)\n"
            "lw     $13, 12(%0)\n"
            "lw     $14, 16(%0)\n"
            "ctc2   $12, $2\n"
            "ctc2   $13, $3\n"
            "ctc2   $14, $4\n"
            :: "r"(&m) : "$12", "$13", "$14");
        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(&v));
        /* gte_rtv0() --- inline_c.h :499-502, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A486012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(&out) : "memory");
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
extern TexRec D_8009B890[];
extern TexRec D_8009B8B0[];
extern TexRec D_8009B998[];
extern TexRec D_8009B9B8[];

u8 func_800678A8(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;
    /* FAKE: unk00 / unk02 / unk04 / unk6C / unk80 are reached through the
     * plain pointers p0 / p2 / p4 / p6C / p80: member stores are
     * MEM_IN_STRUCT_P and let scalar accesses move across them (as members, p0:
     * 6, p4: 2, p80: 2, p2: 24, p6C: 38; all five: 49). */
    u16 *p0 = &outer->unk00;
    u16 *p2 = &outer->unk02;
    u32 *p4 = &outer->unk04;
    s16 *p6C = &outer->unk6C;
    POLY_FT4 **p80 = &outer->unk80;

    D_800A3724 = &outer->unk1AC;
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
        s16 lv = D_800EFC8A[arg1 * 0x1E0] >> 3;
        outer->unk70 = lv;
        if (lv >= 4) {
            outer->unk70 = 3;
        }
        if (D_800A34F0[arg0 - 4] != 0) {
            D_800A3488 = &D_8009B9B8[outer->unk70];
        } else {
            D_800A3488 = &D_8009B998[outer->unk70];
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

    /* gte_SetRotMatrix(r0) --- PsyQ 4.3 inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");

    if (D_800A3438[arg1] < D_800F0B98[arg0]) {
        D_800F0C10[arg1][D_800A3438[arg1]].unk0 = D_800F0C10[arg1][0].unk0;
        D_800F0C10[arg1][D_800A3438[arg1]].unk4 = D_800F0C10[arg1][0].unk4;
        D_800F0C10[arg1][D_800A3438[arg1]].unk8 = D_800F0C10[arg1][0].unk8;
    }

    /* gte_ReadGeomScreen(r0) --- PsyQ 4.3 inline_c.h :1236-1242 */
    __asm__ volatile(
        "cfc2   $12, $26\n"
        "nop\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34B0) : "$12", "memory");

    *p6C = (D_800A3438[arg1] + 1) * 16;
    if (D_800A3438[arg1] < D_800F0B98[arg0] - 1) {
        D_800A3438[arg1]++;
    }
}

/* Per-frame update + draw of one lane's (arg1) effect particles, the middle of
 * the func_800678A8 / func_80067D14 / func_80068D88 trio. For each live record
 * of D_800EFC78[arg1] (state 1): age it and derive the fade colour, move it
 * toward its D_800F0C10 target under a rotation, drop it (state 3) once it is
 * faded or outside the lane radius, project the three trail vertices, size the
 * sprite by type (arg0), then emit one to three POLY_FT4 quads and link them
 * into the context's OT index table.
 * GTE: eleven PsyQ 4.3 inline_c.h macros; gte_rtv0, gte_sqr0 and gte_rtpt
 * carry the post-DMPSX command word (noted at each island). */
void func_80067D14(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;
    u32 *p_rad;
    VECTOR *p_tv;
    s16 *p_count;
    s16 *p_idx;
    u8 *p_r;
    u8 *p_g;
    u8 *p_b;
    Unk800F0C10Record **p_tgt;
    u16 *zbuf;
    s32 *p_seed;
    s32 *p_out;
    VECTOR *p_work;
    SVECTOR *p_vert;
    s16 *p_life;
    s16 *p_n;
    POLY_FT4 **p_prim;
    Unk800EFC78Record **p_ent;
    u32 sum;
    s16 *sxy;

    D_800A3724 = &outer->unk1AC;
    p_seed = &outer->unk1AC;
    *p_seed = rand();
    p_rad = &outer->unk04;
    p_out = outer->unk10.t;
    p_work = &outer->unk34;
    p_vert = outer->unk44;
    p_tv = &outer->unk5C;
    p_count = &outer->unk6C;
    p_idx = &outer->unk6E;
    p_life = &outer->unk70;
    p_r = &outer->unk72;
    p_g = &outer->unk73;
    p_b = &outer->unk74;
    p_n = &outer->unk78;
    p_prim = &outer->unk80;
    p_ent = &outer->unk84;
    p_tgt = &outer->unk88;
    zbuf = outer->unk8C;

    for (*p_idx = 0; *p_idx < *p_count; (*p_idx)++) {
        *p_ent = &D_800EFC78[arg1][*p_idx];
        if ((*p_ent)->unk10 != 1) {
            continue;
        }
        (*p_ent)->unk12++;
        *p_life = (*p_ent)->unk12;
        *p_r = (*p_life << 2) < 0xFF ? ~(*p_life << 2) : 0;
        *p_g = (*p_life << 5) < 200 ? 200 - (*p_life << 5) : 0;
        if (*p_r <= 0x50 && *p_g <= 0x20) {
            (*p_ent)->unk10 = 3;
            continue;
        }
        *p_b = (*p_life << 5) < 100 ? 100 - (*p_life << 5) : 0;

        *p_tgt = &D_800F0C10[arg1][(*p_ent)->unk6];
        p_tv->vx = (*p_tgt)->unk0;
        p_vert[0].vx = p_tv->vx - D_800A3470[0];
        p_tv->vy = (*p_tgt)->unk4;
        p_vert[0].vy = p_tv->vy - D_800A3470[1];
        p_tv->vz = (*p_tgt)->unk8;
        p_vert[0].vz = p_tv->vz - D_800A3470[2];

        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(p_vert));
        /* gte_rtv0() --- inline_c.h :499-502; post-DMPSX command word
           0x4A486012 (MVMVA sf=1 mx=rot v=V0 cv=none lm=0) for the header's
           placeholder .word 0x0000013f */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A486012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(p_out) : "memory");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-369; it reads only the
         * translation, so it is handed the address 0x14 below the rotated
         * vector. */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"((u8 *)p_out - 0x14) : "$12", "$13", "$14");

        p_vert[1].vx = (*p_ent)->unk0;
        p_vert[1].vz = (*p_ent)->unk4;
        p_vert[1].vy = (*p_ent)->unk2;
        p_work->vx = (*p_ent)->unk8 / 30;
        (*p_ent)->unk0 += p_work->vx;
        p_vert[0].vx = (*p_ent)->unk0;
        p_work->vz = (*p_ent)->unkC / 30;
        (*p_ent)->unk4 += p_work->vz;
        p_vert[0].vz = (*p_ent)->unk4;
        p_work->vy = (*p_ent)->unkA / 30 + *p_life * 9800 / 900;
        (*p_ent)->unk2 += p_work->vy;
        p_vert[0].vy = (*p_ent)->unk2;
        p_vert[2].vx = p_vert[1].vx + (p_work->vx >> 1);
        p_vert[2].vz = p_vert[1].vz + (p_work->vz >> 1);
        p_vert[2].vy = p_vert[1].vy + (p_work->vy >> 1);

        /* gte_ldv3(r0, r1, r2) --- inline_c.h :34-42 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            "lwc2   $2, 0(%1)\n"
            "lwc2   $3, 4(%1)\n"
            "lwc2   $4, 0(%2)\n"
            "lwc2   $5, 4(%2)\n"
            :: "r"(&p_vert[0]), "r"(&p_vert[1]), "r"(&p_vert[2]));
        p_work->vx = p_vert[0].vx;
        p_work->vz = p_vert[0].vz;
        p_work->vy = p_vert[0].vy;
        /* gte_ldlvl(r0) --- inline_c.h :112-117 (no clobber list) */
        __asm__ volatile(
            "lwc2   $9, 0(%0)\n"
            "lwc2   $10, 4(%0)\n"
            "lwc2   $11, 8(%0)\n"
            :: "r"(p_work));
        /* gte_sqr0() --- inline_c.h :719-722; post-DMPSX command word
           0x4AA00428 (SQR sf=0 lm=1) for the header's placeholder .word
           0x00000f3f */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(p_out) : "memory");

        *p_seed ^= rand();
        if ((u32)(p_out[0] + p_out[1] + p_out[2]) >
            *p_rad + *p_seed * 3000 / 32768 * 3000) {
            (*p_ent)->unk10 = 3;
            continue;
        }

        /* gte_rtpt() --- inline_c.h :489-492; post-DMPSX command word
           0x4A280030 (RTPT sf=1) for the header's placeholder .word 0x000000bf
         */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A280030\n");
        /* gte_stsxy3(r0, r1, r2) --- inline_c.h :906-912 */
        __asm__ volatile(
            "swc2   $12, 0(%0)\n"
            "swc2   $13, 0(%1)\n"
            "swc2   $14, 0(%2)\n"
            :: "r"(&D_800A34B8[0]), "r"(&D_800A34B8[1]),
               "r"(&D_800A34B8[2]) : "memory");
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");

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
            *D_800A34C0 += *D_800A34C0 * 20 / (*p_life * 6 + 1);
            *D_800A34C8 += *D_800A34C8 * 20 / (*p_life * 6 + 1);
        } else if (arg0 < 6) {
            *D_800A34B4 = *D_800A34B0 * 200 / D_800A34D0[1];
            *D_800A34C0 = *D_800A34A8 * *D_800A34B4 > 0x200
                              ? *D_800A34A8 * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C8 = *D_800A34AC * *D_800A34B4 > 0x200
                              ? *D_800A34AC * *D_800A34B4 >> 9
                              : 1;
            *D_800A34C0 += *D_800A34C0 * *p_life / 2;
            *D_800A34C8 += *D_800A34C8 * *p_life / 2;
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
        if (*p_rad / 3 < sum) {
            *p_n = 1;
        } else if (*p_rad / 2 < sum) {
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
            zbuf[*p_prim - D_800A37D4] = *D_800A34D0;
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
    Unk1F8000B8_800678A8 *outer = &D_800A34EC->v800678A8;
    s16 *p_idx = &outer->unk6E;
    POLY_FT4 **p_end = &outer->unk7C;
    POLY_FT4 **p_cur = &outer->unk80;
    u16 *zbuf = outer->unk8C;
    POLY_FT4 *cur_init;
    POLY_FT4 *prev_init;
    s32 count;
    s32 ret;

    D_800A3724 = &outer->unk1AC;
    prev_init = D_800A37D4;
    cur_init = *p_cur;
    count = cur_init - prev_init;

    if (count != 0) {
        *p_cur = prev_init;
        *p_end = cur_init;
        ret = 1;
        *p_idx = 0;

        if (*p_cur < *p_end) {
            u32 *p_b;
            do {
                D_800A34E4 = g_gpu_ot_ptr + zbuf[*p_idx];
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
    s32 *p = &D_8009BC04;
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
    s32 v0_efc;
    Unk800A34FCRec *v0_e49c;

    D_800A3500 = arg0;
    D_800A351C = (Unk8006E49CRec *)arg0;
    temp_s0 = arg0 + 0x58;
    D_800A3500 = temp_s0;
    snd_StopAll();
    func_8006E950(2, (Unk8006E950Head *)D_800A3500);
    D_800A372C = D_800A3500;
    v0_efc = func_8006919C(D_800A3500);
    D_800A3500 = v0_efc;
    v0_e49c = (Unk800A34FCRec *)func_8006E49C(v0_efc, D_800A351C);
    v0_e49c->unk_24 = (Unk8006919CRec *)temp_s0;
    D_800A3500 = (s32)v0_e49c;
    D_800A34FC = v0_e49c;
    D_800A3500 = (s32)(v0_e49c + 1);
    {
        s32 init_mask = -0x10;
        s32 outer_cache;
        s32 flags = D_800A34F8;
        outer_cache = D_8009BC04;

        flags &= init_mask;
        D_800A34F8 = flags;
        if (!((u32)outer_cache & 1)) {
            u32 mask;
            s32 cache;
            mask = (u32)-0x10;
            cache = D_8009BC04;
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
                flags = (u32)cache >> flags;
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
               reorg steals it into all three incoming jump delay slots */
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
               D_800A34FC load instead of sinking below the zero-stores */
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
    /* FAKE: do{}while(0) around the loop seats sum in $t2 and bitpos in $t1;
     * the `one` constant-holder schedules `li $t6,1` at the preheader. */
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

s32 func_800693CC(s32 held, s32 pressed) {
    extern s32 D_8009BC08;
    extern void func_80069AE4(s32 *, s32, GpuDb *);
    extern void func_8006A880(u8 *, u16 *, s32);
    extern void func_80069F80(s32 *, s32);
    extern void func_8006A1A0(s32 *, s32);
    /* FAKE: frame layout. The draw context func_8006E390 fills is ten words
     * (siblings: s32 sp10[10]); the decoded input sits at word 12 and the
     * saved row mask at word 14 only to reproduce the target's 0x60 frame
     * (sp+0x40 / sp+0x48); separate locals give the wrong frame. */
    s32 context[15];
    s32 result;
    GpuDb *render_base;
    render_base = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(context, &D_800A3518);
    context[12] = (pressed & 0xFFFF) | ((u32)pressed >> 16);
    result =
        func_800692C0((u32 *)&context[12], 0, D_800A34FC->unk_0C, D_800A350C);
    if ((result >> 16) == 1)
        goto increment;
    if ((result >> 16) == 2)
        goto decrement;
    goto after_move;
increment: {
    /* FAKE: the index-field clear mask, named so its load sits ahead of
     * the alias (the literal moves `li -16` two slots). */
    s32 clear_mask = ~0xF;
    /* FAKE: keep the availability address across the loop so GCC reloads
     * its word each iteration; direct global reads hoist the word. */
    s32 *available = &D_8009BC04;
    do {
        s32 flags = D_800A34F8;
        s32 index;
        D_800A34F8 = (flags & clear_mask) | (index = ((flags & 0xF) + 1) & 0xF);
        if (index >= 8) {
            D_800A34F8 &= clear_mask;
        }
    } while (!(((u32)*available >> (D_800A34F8 & 0xF)) & 1));
}
    goto moved;
decrement: {
    /* FAKE: keep the availability address across the loop so GCC reloads
     * its word each iteration; direct global reads hoist the word. */
    s32 *available = &D_8009BC04;
    do {
        s32 flags = D_800A34F8;
        s32 index = flags & 0xF;
        if (index == 0) {
            D_800A34F8 = (flags & ~0xF) | 7;
        } else {
            D_800A34F8 = (flags & ~0xF) | ((index - 1) & 0xF);
        }
    } while (!(((u32)*available >> (D_800A34F8 & 0xF)) & 1));
}
moved:
    D_800A3514 = 0;
    D_800A3524->unk20_3 = 0;
after_move:
    if (((result & 0xFF) < 3) && ((result & 0xFF) != 0) &&
        (D_8009BC0C[D_800A34F8 & 0xF].mode == 3)) {
        D_800A3524->unk20_3 ^= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    if (D_8009BC0C[D_800A34F8 & 0xF].mode == 2) {
        D_800A3524->unk20_3 = 1;
    } else if (D_8009BC0C[D_800A34F8 & 0xF].mode == 1) {
        D_800A3524->unk20_3 = 0;
    }
    if ((D_800A34F8 & 0xF) != 7) {
        if (((D_800A34F8 & 0xF) - 6) >= 0) {
            /* FAKE: one handle for the mask read-modify-write; the direct
             * global form re-addresses D_8009BC08. */
            s32 *mask_ptr = &D_8009BC08;
            context[14] = *mask_ptr & 0x80;
            *mask_ptr = context[14] | (0x1F << ((D_800A34F8 & 0xF) - 5));
        } else {
            D_8009BC08 = 0x9F;
        }
    }
    func_80069AE4(context, 0, render_base);
    func_8006A880(
        (u8 *)context, (u16 *)render_base, D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_80069F80(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    func_8006A1A0(context, D_8009BC0C[D_800A34F8 & 0xF].mode);
    if (pressed & 0x40) {
        switch (D_8009BC0C[D_800A34F8 & 0xF].state) {
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
    if (pressed & 0x400000) {
        switch (D_8009BC0C[D_800A34F8 & 0xF].state) {
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
        if (D_8009BC0C[D_800A34F8 & 0xF].state != 7) {
            snd_CloseVab1();
        }
        return D_8009BC0C[D_800A34F8 & 0xF].state;
    }
    if (pressed & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (pressed & 0x50005000) {
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

void func_80069A30(void *prim) {
    TILE *a0 = prim;
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

void func_80069A8C(void *prim) {
    POLY_F4 *a0 = prim;
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

void func_80069AE4(s32 *arg0, s32 mode, GpuDb *unused_arg) {
    TILE *p;
    POLY_F4 *poly;
    Unk8009B0E0Record **qbase;
    Unk8009B0E0Record **q;
    s32 i;
    Unk8007352CEnv s;

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
    Unk8007352CEnv s;
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
 * unwritten tail, so the locals region reaches the target's frame. */
typedef struct {
    Unk8007352CEnv env;
    s32 tail[4];
} Env_69F80;

void func_80069F80(s32 *arg0, s32 arg1) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * func_80073728 / func_8007352C); tail[0..3] is unwritten padding that
     * brings the locals to the target's 0x70 frame (the 0x2C descriptor alone
     * gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The plain
     * descriptor: score 12. */
    Env_69F80 s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *tbl;

    if (arg1 & 2) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.env.header = ptr[0];
        if (arg1 & 1) {
            s.env.x = 0x9C;
            s.env.has_color = 1;
        } else {
            s.env.x = 0x4E;
            s.env.has_color = 0;
        }
        x0 = s.env.x;
        if (D_800A3524->unk20_3) {
            if (arg1 & 1) {
                s.env.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.env.col_b = c;
                s.env.col_g = c;
                s.env.col_r = c;
            }
            s.env.y = 0;
            s.env.scale_y = 0x100;
            s.env.scale_x = 0x100;
        } else {
            s.env.col_b = 0x70;
            s.env.col_g = 0x70;
            s.env.col_r = 0x70;
            s.env.scale_y = 0x80;
            s.env.scale_x = 0x80;
            s.env.y = 0xA;
        }
        s.env.semi = 0;
        s.env.ot_idx = 3;
        tbl = s.env.header->cells;
        s.env.table = tbl;
        s.env.ft4_out = arg0[2];
        arg0[2] = func_80073728(&s.env, 0);
        s.env.y = 0;
        s.env.x = x0;
        s.env.has_color = 0;
        s.env.header = ptr[1];
        tbl = s.env.header->cells;
        s.env.table = tbl;
        s.env.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.env);
        if (arg1 & 1) {
            s.env.ot_idx = 2;
            s.env.semi = 0;
            s.env.header = ptr[6];
            s.env.x = 0;
            s.env.y = 0;
            s.env.has_color = 1;
            s.env.table = &s.env.header->cells[1];
            s.env.sprt_out = arg0[5];
            arg0[5] = func_8007352C(&s.env);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.env.header, 0), 0);
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
    Env_69F80 s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *tbl;

    if (arg1 & 1) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.env.header = ptr[2];
        if (arg1 & 2) {
            s.env.x = -1;
            s.env.has_color = 1;
        } else {
            s.env.x = 0x4E;
            s.env.has_color = 0;
        }
        x0 = s.env.x;
        if (!(D_800A3524->unk20_3)) {
            if (arg1 & 2) {
                s.env.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.env.col_b = c;
                s.env.col_g = c;
                s.env.col_r = c;
            }
            s.env.y = 0;
            s.env.scale_y = 0x100;
            s.env.scale_x = 0x100;
        } else {
            s.env.x = x0 + 0x32;
            s.env.col_b = 0x70;
            s.env.col_g = 0x70;
            s.env.col_r = 0x70;
            s.env.scale_y = 0x80;
            s.env.scale_x = 0x80;
            s.env.y = 0xA;
        }
        s.env.semi = 0;
        s.env.ot_idx = 2;
        tbl = s.env.header->cells;
        s.env.table = tbl;
        s.env.ft4_out = arg0[2];
        arg0[2] = func_80073728(&s.env, 0);
        s.env.y = 0;
        s.env.x = x0;
        s.env.has_color = 0;
        s.env.header = ptr[3];
        tbl = s.env.header->cells;
        s.env.table = tbl;
        s.env.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.env);
        if (arg1 & 2) {
            s.env.ot_idx = 2;
            s.env.semi = 0;
            s.env.header = ptr[6];
            s.env.x = 0;
            s.env.y = 0;
            s.env.has_color = 1;
            tbl = s.env.header->cells;
            s.env.table = tbl;
            s.env.sprt_out = arg0[5];
            arg0[5] = func_8007352C(&s.env);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.env.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 2, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }
}

void func_8006A3CC(s32 *arg0, Unk8007352CEnv *arg1) {
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

void func_8006A494(s32 *arg0, Unk8007352CEnv *arg1) {
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

void func_8006A564(u8 *arg0, Unk8007352CEnv *arg1, s32 arg2) {
    TILE *tile;
    Unk8009B0E0Record **tbl;
    s32 s4;

    tile = (TILE *)*(s32 *)(arg0 + 0x18);
    SetTile(tile);
    {
        /* FAKE: colour staged ahead of the g0 store (held across the arms for
         * b0); col_r read at its use: score 2. */
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = arg1->col_r;
            tile->g0 = 0;
            tile->r0 = v0;
            v0 = arg1->col_b;
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile->r0 = v0;
            tile->g0 = v0;
        }
        tile->b0 = v0;
        tile->x0 = (0x5F);
        tile->y0 = (arg1->y + 0xF);
        tile->w = (arg1->x + 0x19);
        tile->h = 1;
    }
    SetSemiTrans(tile, arg1->semi);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    {
        /* FAKE: colour staged ahead of the g0 store (held across the arms for
         * b0); col_r read at its use: score 2. */
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = arg1->col_r;
            tile->g0 = 0;
            tile->r0 = v0;
            v0 = arg1->col_b;
            tile->b0 = v0;
        } else {
            v0 = 0x20;
            tile->r0 = v0;
            tile->g0 = v0;
            tile->b0 = v0;
        }
        tile->x0 = arg1->x;
        tile->y0 = (arg1->y + 0xE);
        tile->w = 0x78;
        tile->h = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    {
        /* FAKE: colour staged ahead of the g0 store (held across the arms for
         * b0); col_r read at its use: score 2. */
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = arg1->col_r;
            tile->g0 = 0;
            v0 = (u32)v0 >> 1;
            tile->r0 = v0;
            v0 = arg1->col_b;
            v0 = (u32)v0 >> 1;
            tile->b0 = v0;
        } else {
            v0 = 0x10;
            tile->r0 = v0;
            tile->g0 = v0;
            tile->b0 = v0;
        }
        tile->x0 = (arg1->x + 0x40);
        tile->y0 = (arg1->y + 0xD);
        tile->w = 0x38;
        tile->h = 1;
    }
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
        Unk8009B0E0Record **hp = &arg1->header;

        *hp = tbl[10];
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

extern s32 D_8009BC08;

/* Draws the eight-row list screen from the MOD.BIN resource (ctx+4): the row
 * sheets (cursor row flashing, rows switched on in D_8009BC04 lit), a counter
 * frame, row 7, the frame icons and a TILE. arg0 = the draw
 * context func_8006E390 fills (+0x8 POLY_FT4 cursor, +0x14 SPRT cursor,
 * +0x18 TILE cursor, +0x1C DR_MODE cursor, +0x20 DR_AREA cursor,
 * +0x24 DR_OFFSET cursor), accessed as byte offsets like func_8006A564;
 * arg1 = the display environment's clip RECT and ofs[2]; arg2 (passed by
 * func_800693CC) is unused. */
void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2) {
    Unk8007352CEnv s;
    RECT rect;
    s16 ofs[2];
    /* Ruling 11 (ordinary-c-judge-decidable): holds five values, each a
     * sprite-sheet list of the MOD.BIN
     * root: unk_18 (option rows; first sheet, the row loop and row 7), unk_40
     * (counter frames), unk_24 twice (icon frames at [8 + frame], then FT4
     * frames at [frame]). */
    Unk8009B0E0Record **sheets;
    /* Ruling 11: holds two values, each a mask with one bit per option row:
       D_8009BC08 (scanned for the first row drawn) and D_8009BC04 (rows
       switched on). */
    u32 row_mask;
    /* Ruling 9: the sheet's cell array; every sheet drawn here has one 12-byte
     * header, so the cells start at +0xC. FAKE: single-use holder of the cell
     * table ahead of the descriptor's table store; stored directly: score 15.
     */
    Unk8009B400Record *cells;
    s32 yofs;
    s32 bit;
    s32 i;

    sheets = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
    row_mask = D_8009BC08;
    bit = 0;
    s.header = sheets[0];
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

    row_mask = D_8009BC04;
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
        s.header = sheets[i];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
        SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                    func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
        *(s32 *)(arg0 + 0x1C) += 0xC;
        s.has_color = 1;
        func_8006A564(arg0, &s, i);
    }

    sheets = (*(Unk8006919CRec **)(arg0 + 4))->unk_40;
    s.ot_idx = 0xA;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.y = 0x41;
    s.header = sheets[D_800A34F8 & 0xF];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    sheets = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
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
    s.header = sheets[7];
    cells = s.header->cells;
    s.table = cells;
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

    sheets = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = sheets[(D_800A34F8 & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    sheets = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = sheets[D_800A34F8 & 0xF];
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header->cells;
    s.table = cells;
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

/* One argument: the caller's s32[10] draw context (asm reads a0 only). */
void func_8006B120(s32 *arg0);

void func_8006B120(s32 *arg0) {
    Unk8007352CEnv s;
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
    /* FAKE: the frame's GpuDb taken ahead of func_8006E390; at its use: score
     * 23 */
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006B120(sp10);
    return func_8006B578(&arg0, &arg1);
}

s32 func_8006B92C(s32 *unused, u32 *arg1) {
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
    Unk8007352CEnv s;
    u16 rect[4];
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

    rect[2] = 0xAF;
    rect[0] = 0xE8;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898(arg0, rect, 0x11);
}

extern DR_MODE *D_800A36E0;
extern u8 *D_800A36E4;

void func_8006BD28(s32 arg0, s32 arg1, Unk8007352CEnv *arg2, s32 arg3) {
    /* sprite-sheet header pointers, two per arg0: the unk_20 list of the
       MOD.BIN root (D_800A34FC->unk_24) */
    Unk8009B0E0Record **sheets;
    /* FAKE: named intermediate (no-new-park-categories entry 6): the sheet's
     * 8-byte cells start at header + 0xC; inline, fold reassociates the sum and
     * loop.c strength-reduces it into its own register. Inline: score 31. */
    Unk8009B400Record *cells;
    s32 i, j, n;

    sheets = D_800A34FC->unk_24->unk_20;

    arg2->col_b = 0x30;
    arg2->col_g = 0x30;
    arg2->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        /* FAKE: operand grouping (or-tree-shape-shift carve-out): with
         * (sheets + i) + arg0 * 2, arg0 * 8 is hoisted alone as in the target;
         * sheets[arg0 * 2 + i]: score 33. */
        arg2->header = *(sheets + i + arg0 * 2);
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
            cells = arg2->header->cells;
            arg2->table = cells + j;
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
    Unk8007352CEnv sp10;
    s32 par;
    Vec2s16 *pos;
    s16 i;
    s32 r;
    s32 w;
    s32 h;
    s32 x0;

    D_800A3900 = 0;
    par = D_800A36AC & 1;
    D_800A36E4 = D_800F11E0 + par * 0x12C;
    D_800A36E0 = D_800F1438 + par * 4;
    D_800A36DC = D_800F1498 + par * 4;
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
    Unk8007352CEnv s;
    s32 j;
    Unk8009B0E0Record **table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *tile_rec;
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
     * the bar's far-corner red, 0 on the pulsing bar and 0x80 otherwise; set in
     * each arm, it is not hoisted out of the row loop. The literals: score 8.
     */
    s32 col;
    /* FAKE: always-zero narrow locals (named-local-fake-exception, Q27 (A)) --
     * SetDrawMode's dither and texture-window arguments and the descriptor
     * position, read up to the phase-4 head; their dead extension temps take
     * the target's four untouched frame slots (0x60-0x78). `tw` reaches
     * SetDrawMode's RECT *tw through a
     * `(RECT *)tw` conversion (Q113). Literals at every read: score 37. */
    s16 dtd;
    s16 xpos;
    s16 ypos;
    s16 tw;
    s32 pl;
    s32 x;
    s32 row;
    s32 pulse;
    /* the sprite sheet's 8-byte cells, just past its 12-byte header. Ruling 9:
     * one meaning, header + 0xC at every write. FAKE: single-use holder of the
     * cell table ahead of the descriptor's table store; stored directly:
     * score 12. */
    Unk8009B400Record *cells;

    mode = 0;
    dtd = 0;
    xpos = 0;
    ypos = 0;
    tw = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    s.x = xpos;
    s.header = table[0];
    s.semi = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    SetDrawMode(
        (DR_MODE *)arg0[7], 1, dtd, func_8006E480(s.header, mode), (RECT *)tw);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.ot_idx = 9;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    for (pl = 0; pl < 2; pl++) {
        s.x = pl * 280;
        if (D_800A34FC->unk_28.half[pl] < 3) {
            for (work = 0; work < 4; work++) {
                if (D_800A3524->unk17[D_800A34FC->unk_28.half[pl]] &
                    ((1 << work) << (pl * 4))) {
                    s.header = table[work + 13];
                    s.semi = 0;
                    cells = s.header->cells;
                    s.table = cells;
                    s.sprt_out = arg0[5];
                    arg0[5] = func_8007352C(&s);
                }
            }
        }
    }
    s.header = table[13];
    SetDrawMode(
        (DR_MODE *)arg0[7], 1, dtd, func_8006E480(s.header, mode), (RECT *)tw);
    AddPrim(g_gpu_ot_ptr + 9, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.x = xpos;
    s.y = ypos;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.header = table[1];
    s.semi = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = arg0[5];
    arg0[5] = func_8007352C(&s);
    for (work = 0; work < 6; work++) {
        s.header = table[work + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        cells = s.header->cells;
        s.table = cells;
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
        tile_rec = &recs[work];
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = tile_rec->r;
            tile->g0 = tile_rec->g;
            tile->b0 = tile_rec->b;
            tile->x0 = tile_rec->x + j * 280;
            tile->y0 = tile_rec->y;
            tile->w = tile_rec->w;
            tile->h = tile_rec->h;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + 0xC, tile);
            tile++;
        }
    }
    arg0[6] = (s32)tile;

    pulse = ((rcos((D_800A3518 << 7) & 0xF80) * 32) >> 12) + 0xD0;
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
                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r1 = pulse;
                poly->g1 = pulse;
                poly->b1 = pulse;
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
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
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
                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r2 = pulse;
                poly->g2 = pulse;
                poly->b2 = pulse;
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
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse;
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
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 masked;
    s16 field;

    ret = 0;
    if ((D_800A34FC->unk_28.word == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    fade = 0;
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

        field = D_800A34FC->unk_28.half[i];
        switch (field) {
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
                    masked = *(rec + 0x1A) & (nib << fade);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) +
                             masked);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1D) & (nib << fade);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) +
                             masked);
                }
            }
            break;
        }
        fade += 4;
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
    Unk8007352CEnv s;
    Counts_8006CFBC counts;
    Unk8009B0E0Record **table;
    s16 outer;
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

    outer = 0;
    do {
        counts.word = 0;
        s.y = outer * 16;
        column = 0;
        do {
            s.header = table[column + 8];
            temp = s.header->cells;
            s.table = temp;
            for (row = 0; row < 2; row++) {
                if (D_800A3524->unk17[outer] & ((1 << (row * 4)) << column)) {
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
        outer++;
    } while (outer < 3);

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
        u16 rect[4];
        rect[2] = 0xE1;
        rect[0] = 0xCF;
        rect[1] = 0x25;
        rect[3] = 1;
        func_80069898(arg0, rect, 0x11);
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
    Unk8007352CEnv s;
    u16 rect[4];
    Unk8009B0E0Record **q;
    /* FAKE: constant holder for func_8006E480's 0; the literal: score 8. */
    s32 semi = 0;
    s16 i = 0;
    /* FAKE: constant holder for the dimmed colour; the literal: score 25. */
    u8 dim = 0x40;

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
            s.col_r = s.col_g = s.col_b = dim;
            s.y = 0;
            s.semi = 1;
        }
        s.header = q[i];
        s.table = s.header->cells;
        s.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s);
        SetDrawMode((DR_MODE *)arg0[7], 1, 0, func_8006E480(s.header, semi), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    rect[0] = 0xDA;
    rect[1] = 0x25;
    rect[2] = 0xCB;
    rect[3] = 1;
    func_80069898(arg0, rect, 0x11);
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
    s32 *arg0, s32 *arg1, Unk8006D808Set *arg2, s32 arg3, s32 arg4) {
    Unk8007352CEnv s;
    /* FAKE: oversized digit array (dead-vars-local-array OVERSIZED-LOCALS):
     * only d[0]/d[1] are used; the unwritten tail brings the locals to the
     * target's 0x88 frame (s16 d[2] gives 0x80): score 26. */
    s16 d[5];
    s16 i;
    s16 k;
    s16 idx;
    s16 n;
    /* FAKE: holder of the character's x offset for both x stores; read at each:
     * score 27. */
    s32 w;

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
        w = arg2->unk_1C[idx];
        s.y = i * 26;
        s.x = w;
        if (i == 3) {
            s.x = w - 50;
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

/* FAKE: frame layout (oversized live object), as Env_69F80. */
typedef struct {
    Unk8007352CEnv env;
    s32 tail[2];
} Env_8006DD94;

void func_8006DD94(s32 *arg0) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * func_8007352C); tail[0..1] is unwritten padding that brings the locals to
     * the target's 0x78 frame with rect at sp+0x50 (an 8-aligned slot,
     * BIGGEST_ALIGNMENT -- mips.h:1082) (dead-vars-local-array
     * OVERSIZED-LOCALS). The plain descriptor: score 21. */
    Env_8006DD94 s;
    u16 rect[4];
    s16 i;
    Unk8006DD94List *q;
    /* FAKE: constant holder for the semi flag and func_8006E480's 0; the
     * literal: score 8. */
    s32 semi = 0;

    s.env.ot_idx = 0xA;
    q = ((Unk8006919CRec *)arg0[1])->unk_3C;
    s.env.x = 0;
    s.env.semi = semi;

    for (i = 0; i < 3; i++) {
        s.env.has_color = 1;
        if (i == D_800A352C + 1) {
            s.env.y = D_800A34FC->unk_0C[1];
            s.env.col_r = s.env.col_g = s.env.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
        } else {
            if (i == 0) {
                s.env.col_r = s.env.col_g = s.env.col_b = 0x80;
            } else {
                s.env.col_r = s.env.col_g = s.env.col_b = 0x40;
            }
            s.env.y = 0;
        }
        s.env.header = q->unk_20[i];
        s.env.table = s.env.header->cells;
        s.env.sprt_out = arg0[5];
        arg0[5] = func_8007352C(&s.env);
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, func_8006E480(s.env.header, semi), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], &q->unk_00, s.env.ot_idx, -1);

    rect[2] = 0x96;
    rect[0] = 0xF5;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898(arg0, rect, 0x11);
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
    s32 ff0;
    /* FAKE: read at entry and held in $s3 across the calls; read at the
     * LoadImage call: score 9 (frame 48 for 56) */
    s32 temp_s3 = D_800A3500;
    RECT rect;
    s32 v0;
    /* FAKE: the per-branch 2 (li a0,2 in each arm's delay slot); the literal at
     * the call: score 9 */
    s32 a0v;
    s32 a1v;

    rect = D_800A32D8;
    if (D_800A3524->unk20_0) {
        a0v = 2;
        a1v = 0x60;
    } else {
        a0v = 2;
        a1v = 7;
    }
    /* FAKE: loop notes fence sched1's constant-sink so the li stays at the jal;
     * plain `ff0 = 0xF0;`: score 13 */
    do {
        ff0 = 0xF0;
    } while (0);
    v0 = func_80036EA8(a0v, a1v);
    cdrom_StartRead(v0, D_800A3500);
    game_FrameLoop();
    cdrom_GetFileSize(v0);
    SetDispMask(0);
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, ff0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, ff0, 0x280, ff0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, ff0, 0x280, ff0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, ff0);
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

s32 func_8006E480(Unk8009B0E0Record *hdr, s32 a1) {
    return (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7) + a1;
}

s32 func_8006E49C(s32 arg0, Unk8006E49CRec *arg1) {
    u8 *base1;
    u8 *base2;
    int tail;
    u8 *base3;
    u8 *base4;
    arg1[0].unk_00 = arg0;
    base1 = (u8 *)arg0 + 0x9C40;
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
    tail = 0x1FB0;
    /* FAKE: tail must stay a variable: as a literal, (base2 + 0x1FB0) + 0x9C40
     * folds to li 0xBBF0 + addu (0xBBF0 > 0x7FFF, out of addiu's signed range);
     * score 8. */
    base3 = (base2 + tail) + 0x9C40;
    arg1[1].unk_08 = base3 + 0x5DC0;
    arg1[1].unk_04 = base3;
    base4 = base3 + 0x6838;
    arg1[1].unk_0C = base3 + 0x61F8;
    arg1[1].unk_14 = base4 + 0x1B58;
    arg1[1].unk_18 = base4 + 0x1DB0;
    arg1[1].unk_1C = base4 + 0x1E28;
    arg1[1].unk_20 = base4 + 0x1EA0;
    arg1[1].unk_10 = (s32)base4;
    return (s32)(base4 + tail);
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
