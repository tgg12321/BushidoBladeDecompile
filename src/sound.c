#include "common.h"
#include "include_asm.h"

/* Forward declarations for called functions */
extern void func_800453E0(s32);
extern s32 *func_800455AC(s32);
extern void func_80045510(s32, s32);
extern void func_80045230(s32);
extern void func_80054FDC(s32);
extern void func_80078A68(u32, s32, s32);
extern void func_80078B04(u32);
extern void func_80078B3C(u32);
extern void func_8004473C(void);
extern void efc_rob_set_type_flash(void);
extern void func_80048F58(s32, s32);
extern void func_80048FFC(s32);
extern void *func_8004153C(void);
extern void func_800477E8(void);
extern void func_80047A90(void);
extern void func_80048B8C(s32);
extern void func_80044100(s32, s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, void (*)(void));
extern void func_800460E4(s32);
extern void func_800421C8(s32);
extern void func_8003E0E0(void);
extern void DispHira(s32);
extern void func_8003DA8C(s32, s32);
extern void func_800415C4(s32);
extern void func_8004668C(void);
extern void func_80046020(void);
extern void func_80049E1C(void);
extern void camera_InitRotation(u8 *);
extern void func_80042A88(s32 *, s32 *);
extern void func_8007ED6C(s32 *, s16 *, s32 *);
extern s16 func_8007FD5C(s32, s32);
extern s16 Judge[];
extern s16 D_800A33C8;
extern s16 D_800A33CA;
extern s32 D_800A3708;
extern void func_8004211C(void);
extern void func_800444BC(void);
extern void func_80052930(void *, void *, void *);
extern void func_80044FA0(s32, s32 *);
extern s16 D_800A324A;

/* Externs for globals */
extern s32 D_800A33B0;
extern s16 D_800EEDB0;
extern s16 D_800EEDB2;
extern s16 D_800EEDBE;
extern s32 D_800A33B4;
extern s32 D_800A33D0;
extern s32 D_800A33C0;
extern s32 D_800A33D4;
extern s32 D_800A33D8;
extern s32 D_800EF7BC[];
extern s32 D_800EF800[];
extern s16 D_80099478;
extern s16 D_8009947A;
extern u8 D_8009947C;
extern s16 D_800F6654;
extern s32 D_800A3790;
extern u8 D_800EEDD0;
extern u8 D_800EEDF0;

extern s16 D_800EEDF8;
extern s16 D_800F62F8;
extern s16 D_800F62FA;
extern s16 D_800F62FC;
extern s16 D_800EEDB4;
extern s16 D_800EEDB6;
extern s16 D_800EEDB8;
extern s16 D_800EEDBA;
extern s16 D_800EEDBC;
extern s16 D_800EEDC0;

extern void func_800451A0(void);
extern void func_800451D0(void);
extern void func_80042E90(void);
extern void func_80044498(void);
extern void func_80049E4C(void);
extern void func_80049F4C(void);
extern void func_8003D91C(void);
extern void func_800404D8(void);
extern void func_8003F7F4(void);
extern s16 D_800F6650;
extern s16 D_800F6652;
extern s16 D_800F6656;
extern s16 D_800F6658;
extern s16 D_800F665A;
extern s16 D_800F665C;
extern s32 D_800A33BC;
extern s32 func_800486FC(void);
extern s32 *func_8004574C(s32);
extern void func_80044F80(s32, s32 *);
extern void func_80044010(s32 *, s32);
extern s16 D_800A3248;

void snd_BgmCallback(s32 a0, s32 a1);

/* --- Functions 0x80046780 - 0x80047EC8 --- */

s32 snd_GetBgmId(void) {
    return D_800A33B0;
}
s32 snd_GetSeId(void) {
    return D_800A33B4;
}

s32 stage_GetId(void) {
    return D_80099478;
}

s32 stage_GetVariant(void) {
    return D_8009947A;
}

s32 *snd_LoadBgm(s32 a0) {
    s32 arg = a0;
    s32 chan = 8;
    s32 *s2;

    if (func_800486FC()) {
        arg = arg + 0x1B;
    }
    {
        s32 *a1 = func_8004574C(8);
        if (a1 && D_800A3248 == arg) {
            s2 = (s32 *)a1[1];
        } else {
            s32 *s1;
            s32 *s0;
            s2 = func_800455AC(chan);
            func_80044F80(arg, s2);
            {
                s32 off1 = (u32)s2[2] >> 2 << 2;
                s32 off0 = (u32)s2[1] >> 2 << 2;
                s1 = (s32 *)((u8 *)s2 + off1);
                s0 = (s32 *)((u8 *)s2 + off0);
            }
            func_80045230((s32)s1);
            func_80044010(s0, 9);
            func_80045600(chan, (s32)s1);
            D_800A3248 = arg;
        }
    }
    func_80045694(chan, snd_BgmCallback);
    return s2;
}

void snd_PlayBgm(s32 a0) {
    func_80045510(8, a0);
    func_80045230(0);
}

void snd_BgmCallback(s32 a0, s32 a1) {
    func_80048B8C(a1);
    func_80044100(9, a1);
}

void snd_StopBgm(void) {
    func_800453E0(8);
}

void snd_AllocSe(void) {
    func_800455AC(9);
}

void snd_SeNullCallback(void) {
}

void snd_LoadSe(s32 a0) {
    func_80045230(a0);
    func_80045600(9, a0);
    func_80045694(9, snd_SeNullCallback);
}

void snd_PlaySe(s32 a0) {
    func_80045510(9, a0);
}

void snd_SelectionCallback(s32, s32);

s32 *snd_LoadSelection(s32 a0) {
    s32 *v0;
    s32 offset;

    v0 = func_800455AC(0xA);
    func_80044FA0(a0, v0);
    offset = (u32)v0[v0[0] + 1] >> 2 << 2;
    {
        s32 *s0 = (s32 *)((u8 *)v0 + offset);
        func_80045230((s32)s0);
        func_80045600(0xA, (s32)s0);
    }
    D_800A324A = (s16)a0;
    func_80045694(0xA, snd_SelectionCallback);
    return v0;
}

void snd_StopSelection(void) {
    func_800453E0(0xA);
}

void snd_SelectionCallback(s32 a0, s32 a1) {
    func_80054FDC(a1);
}

void snd_StopAll(void) {
    func_800415C4(0);
    func_800415C4(1);
    func_8004668C();
    func_80046020();
    func_80049E1C();
    snd_StopBgm();
}

void snd_PlaySystemSe(void) {
    func_80078A68(0xF2000001, -1, 0x2000);
    func_80078B3C(0xF2000001);
}

void snd_StopSystemSe(void) {
    func_80078B04(0xF2000001);
}

void game_Init(void) {
    s16 one;
    unsigned int new_var;
    s16 two;

    func_800451A0();
    func_800451D0();
    func_80042E90();
    func_80044498();
    new_var = 0;
    func_80049E4C();
    func_80049F4C();
    func_8003D91C();
    func_800404D8();
    func_8003F7F4();
    one = 1;
    two = 2;
    do { } while (0);
    D_800F6654 = one;
    D_800F665A = one;
    D_800F6652 = 0;
    D_800F6650 = new_var;
    D_800F6656 = 0;
    D_800F6658 = two;
    D_800F665C = 0;
    D_800A3790 = 0x23;
    D_800A33BC = 0;
}
INCLUDE_ASM("asm/funcs", func_80046BF4);
void game_StageInit(s32 a0) {
    if (a0 & 1) {
        game_InitStageSound(D_800A33C0);
    }
    func_8004211C();
    func_800444BC();
}

s32 game_GetDummyFlag(void) {
    return 0;
}

void *game_GetPlayerData(void) {
    void *v0 = func_8004153C();
    if (v0) {
        return (u8 *)v0 + 0x1994;
    }
    return NULL;
}

void *game_GetPlayerBase(void) {
    void *v0 = func_8004153C();
    if (v0) {
        return (u8 *)v0 + 0x2C;
    }
    return NULL;
}

void game_DisablePause(void) {
    D_800F6654 = 0;
}

void game_SetPause(s32 a0) {
    if (a0) {
        D_800F6654 = 1;
    } else {
        D_800F6654 = 0;
    }
}

s32 game_GetPause(void) {
    return D_800F6654;
}

void game_ResetTimer(void) {
    D_800A3790 = 0x23;
}

void game_InitStageSound(s32 a0) {
    DispHira(a0);
    {
        s32 v0 = stage_GetId();
        func_8003DA8C(v0, a0);
    }
}

void game_StageCleanup(s32 a0) {
    func_800460E4(a0);
    func_800421C8(a0);
    func_8003E0E0();
}

void *stage_GetDataPtr(void) {
    return &D_8009947C;
}

void camera_InitMatrix(void) {
    s32 num = (s32)D_800F62F8 << 12;
    s32 div = D_800F62FA;
    s32 v0 = num / div;
    s32 v1 = ((s32)D_800F62FC << 12) / div;
    D_800EEDB4 = 0;
    div = 0;
    num = v0;
    D_800EEDB6 = 0;
    v1 = -(s16)v1;
    D_800EEDB8 = 0;
    D_800EEDBA = 0;
    D_800EEDBC = div;
    D_800EEDB0 = 0x1000;
    D_800EEDC0 = 0x1000;
    v0 = -(s16)num;
    D_800EEDB2 = v0;
    D_800EEDBE = v1;
}
void camera_Transform(s32 *a0, s32 *a1, s32 a2) {
    s32 new_var;
    s32 diff, prod;
    func_80052930(&D_800EEDB0, a0, a1);
    new_var = a0[5];
    diff = a0[6] - a2;
    prod = diff * D_800EEDB2;
    a1[6] = a2;
    a1[5] = new_var + (prod >> 12);
    diff = a0[6] - a2;
    prod = diff * D_800EEDBE;
    a1[7] = a0[7] + (prod >> 12);
}
INCLUDE_ASM("asm/funcs", func_800470B0);
typedef struct {
    s32 w[8];
} Block32;
extern Block32 D_80101E08;
extern s16 D_800EEDD6;
extern s16 D_800EEDD8;
void camera_InitBoneData(void) {
    s16 *new_var;
    do { *(Block32 *)&D_800EEDD0 = D_80101E08; } while (0);
    new_var = &D_800EEDD8;
    {
        s16 h0 = D_800EEDD6;
        s16 h1 = D_800EEDD8;
        D_800EEDD6 = h0 >> 1;
        {
            s16 h2;
            D_800EEDD8 = h1 >> 1;
            *(new_var + 1) = (*((&D_800EEDD8) + 1)) >> 1;
        }
    }
}

void *camera_GetBoneData(void) {
    return &D_800EEDD0;
}

extern s32 D_800F66A0[];
void camera_InitRotation(u8 *a0) {
    u8 *s0 = a0;
    *(s16 *)(s0 + 4) = 8;
    {
        s16 v0 = 4;
        *(volatile s16 *)(s0 + 8) = 0;
        {
            s32 idx = *(s16 *)(s0 + 8);
            u8 *a0_arg = s0 + 0x10;
            *(s16 *)(s0 + 2) = 0;
            s0[0] = 0;
            s0[1] = 0;
            *(s32 *)(s0 + 0xC) = 0;
            *(s16 *)(s0 + 0xA) = v0;
            *(s16 *)(s0 + 0x10) = 0;
            *(s16 *)(s0 + 0x12) = 0;
            *(s16 *)(s0 + 0x14) = 0;
            ((void (*)(u8 *, u8 *))D_800F66A0[idx])(a0_arg, s0 + 0x38);
        }
    }
    *(volatile s32 *)(s0 + 0x54) = 0;
    *(volatile s32 *)(s0 + 0x50) = 0;
    *(volatile s32 *)(s0 + 0x4C) = 0;
    {
        s32 v0 = *(volatile s32 *)(s0 + 0x38);
        s32 v1 = *(volatile s32 *)(s0 + 0x3C);
        s32 a0t = *(volatile s32 *)(s0 + 0x40);
        s32 a1t = *(volatile s32 *)(s0 + 0x44);
        *(volatile s32 *)(s0 + 0x18) = v0;
        *(volatile s32 *)(s0 + 0x1C) = v1;
        *(volatile s32 *)(s0 + 0x20) = a0t;
        *(volatile s32 *)(s0 + 0x24) = a1t;
    }
    {
        s32 v0 = *(volatile s32 *)(s0 + 0x48);
        s32 v1 = *(volatile s32 *)(s0 + 0x4C);
        s32 a0t = *(volatile s32 *)(s0 + 0x50);
        s32 a1t = *(volatile s32 *)(s0 + 0x54);
        *(volatile s32 *)(s0 + 0x28) = v0;
        *(volatile s32 *)(s0 + 0x2C) = v1;
        *(volatile s32 *)(s0 + 0x30) = a0t;
        *(volatile s32 *)(s0 + 0x34) = a1t;
    }
}

s16 *camera_CalcAngles(void) {
    s16 rot[3];
    s32 sp18[3];
    s32 pos[8];
    s16 s0;
    s32 a1;

    func_80042A88((s32 *)((u8 *)*(s32 **)&D_800A3708 + 0x10), pos);
    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x1000;
    func_8007ED6C(pos, rot, sp18);
    s0 = func_8007FD5C(sp18[0], sp18[2]);
    a1 = ((s32)Judge[((s16)s0 + 0x400) & 0xFFF] * sp18[2]
         + (s32)Judge[s0 & 0xFFF] * sp18[0]) >> 12;
    sp18[2] = a1;
    D_800A33C8 = -func_8007FD5C(sp18[1], a1);
    D_800A33CA = s0;
    return &D_800A33C8;
}

void game_EffInit(void) {
    func_8004473C();
}

void game_EffCleanup(void) {
    efc_rob_set_type_flash();
}

void game_AnimInit(void) {
    func_80048F58(0, 0);
}

void game_AnimCleanup(void) {
    func_80048FFC(0);
}

void game_EffInit2(void) {
    func_8004473C();
}

void game_EffCleanup2(void) {
    efc_rob_set_type_flash();
}

void game_SndInit(void) {
    func_800477E8();
}

void game_SndCleanup(void) {
    func_80047A90();
}

void camera_InitBone2(void) {
    camera_InitRotation(&D_800EEDF0);
    D_800EEDF8 = 4;
}
INCLUDE_ASM("asm/funcs", func_800475A4);

void game_AnimStart(void) {
    func_80048F58(1, 0);
}

void game_AnimStop(void) {
    func_80048FFC(0);
}

void game_EffStart(void) {
    func_8004473C();
}

void game_EffStop(void) {
    efc_rob_set_type_flash();
}

void game_Stub1(void) {
}

void game_Stub2(void) {
}

void game_Stub3(void) {
}

void game_Stub4(void) {
}

void snd_SetVolume(s32 a0) {
    D_800A33D0 = a0;
}
INCLUDE_ASM("asm/funcs", func_800477E8);
INCLUDE_ASM("asm/funcs", func_80047A90);

INCLUDE_ASM("asm/funcs", func_80047BE0);
extern s32 D_800EF7BC[];

s32 snd_CalcFade(s32 a0) {
    s32 a1 = (a0 + 0x7D00) / 3200;
    s32 a0_div = a0 / 3200;
    s32 remainder = a0 - a0_div * 3200;
    s32 odd = remainder & 1;
    D_800A33D4 = a1;
    D_800A33D8 = odd;
    if ((u32)a1 >= 18) {
        return (s32)0xFFFE7960;
    }
    {
        s32 val1 = D_800EF7BC[a1] * odd;
        s32 val2 = D_800EF7BC[a1 + 1] * (0x1000 - odd);
        return ((val1 + val2) >> 12) - 0x3F48;
    }
}
s32 snd_GetFadeCurve(void) {
    s32 v1 = D_800A33D4;
    if ((u32)v1 >= 18) {
        return 0;
    }
    {
        s32 v0 = D_800EF800[v1];
        s32 a0 = D_800A33D8;
        s32 val1 = v0 * a0;
        s32 v3 = D_800EF800[v1 + 1];
        s32 val2 = v3 * (0x1000 - a0);
        return (val1 + val2) >> 12;
    }
}

s32 snd_GetMaxFade(void) {
    return 0xD00;
}
