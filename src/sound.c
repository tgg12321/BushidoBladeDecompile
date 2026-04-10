#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

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
extern s16 g_pad_selection;

/* Externs for globals */
extern s16 g_cam_matrix;
extern s16 D_800EEDB2;
extern s16 D_800EEDBE;
extern s32 g_snd_stage_bgm;
extern s32 g_snd_fade_pos;
extern s32 g_snd_fade_amt;
extern s32 g_snd_config_tbl[];
extern s32 g_snd_fade_curve[];
extern u8 g_stage_data;
extern s16 g_game_pause;
extern u8 g_cam_bone_data;
extern u8 g_cam_bone_data2;

extern s16 g_cam_interp;
extern s16 g_cam_fov_x;
extern s16 g_cam_fov_div;
extern s16 g_cam_fov_z;
extern s16 D_800EEDB4;
extern s16 D_800EEDB6;
extern s16 D_800EEDB8;
extern s16 D_800EEDBA;
extern s16 D_800EEDBC;
extern s16 D_800EEDC0;

extern void func_800451A0(void);
extern void func_800451D0(void);
extern void func_8007E74C(void *, void *, void *);
extern void func_800418D0(s32 *);
extern void func_8004A1FC(void *);
extern void func_800420D0(void);
extern void func_8003F568(void);
extern void func_8003F5CC(void);
extern void func_8003F274(void);
extern s16 D_80101E02;
extern s16 D_80101E04;
extern s32 D_80101E3C;
extern s32 D_80101E40;
extern s32 D_80101E44;
extern s32 D_80102C00;
extern u16 D_800A38D6;
extern s32 D_800A374C;
extern s32 D_800A3808;
extern s32 D_800A378C;
extern s32 D_800F62E0;
extern s32 g_anim_func_table[];
extern void func_80042E90(void);
extern void func_80044498(void);
extern void func_80049E4C(void);
extern void func_80049F4C(void);
extern void func_8003D91C(void);
extern void func_800404D8(void);
extern void func_8003F7F4(void);
extern s16 D_800F6650;
extern s16 g_color_mode;
extern s16 g_game_p1_ctrl;
extern s16 D_800F665A;
extern s32 D_800A33BC;
extern s32 func_800486FC(void);
extern s32 *func_8004574C(s32);
extern void func_80044F80(s32, s32 *);
extern void func_80044010(s32 *, s32);
extern s16 D_800A3248;

void snd_BgmCallback(s32 a0, s32 a1);

/* --- Functions 0x80046780 - 0x80047EC8 --- */

s32 snd_GetBgmId(void) {
    return g_snd_bgm_id;
}
s32 snd_GetSeId(void) {
    return g_snd_se_id;
}

s32 stage_GetId(void) {
    return g_stage_id;
}

s32 stage_GetVariant(void) {
    return g_stage_variant;
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
    g_pad_selection = (s16)a0;
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
    g_game_pause = one;
    D_800F665A = one;
    g_color_mode = 0;
    D_800F6650 = new_var;
    g_game_p1_ctrl = 0;
    g_game_p2_ctrl = two;
    g_game_mirror_mode = 0;
    g_game_timer = 0x23;
    D_800A33BC = 0;
}
void func_80046BF4(s32 *a0, u16 *a1, s32 a2) {
    s32 result[3];
    u16 *new_var;
    u16 new_var2;
    s32 trans[3];
    s16 rot[3];
    s32 matrix_buf[8];
    s16 *rot_base;
    u8 *base;

    D_800A3820 = (s32)&D_80102C00;
    {
        u16 cnt = D_800A38D6;
        s32 old_ptr = D_800A374C;
        new_var2 = cnt - -1;
        D_800A3808 = old_ptr;
        D_800A38D6 = new_var2;
        D_800A378C = old_ptr + 0x10;
    }

    new_var = &a1[2];
    if (a0 != 0) {
        rot_base = &D_80101E00;

        *rot_base = -(s16)a1[0];
        D_80101E02 = -(s16)a1[1];
        D_80101E04 = -(s16)(*new_var);

        trans[1] = (trans[0] = 0);
        trans[2] = -a2;

        rot[0] = -(s16)a1[0];
        rot[1] = -(s16)a1[1];
        rot[2] = -(s16)(*new_var);

        ((void (*)(s16 *, s32 *))g_anim_func_table[0])(rot, matrix_buf);

        func_8007E74C(matrix_buf, trans, result);

        D_80101E3C = result[0] + a0[0];
        asm volatile("" ::: "memory");
        D_80101E40 = result[1] + a0[1];
        asm volatile("" ::: "memory");
        D_80101E44 = result[2] + a0[2];

        func_800418D0((s32 *)((char *)rot_base - 0x10));
        camera_InitBoneData();
        func_8003F274();

        g_snd_stage_bgm = a2;
    }

    base = (u8 *)&D_800F62E0;
    func_8004A1FC(base);
    func_8004A1FC(base + 0x60);
    func_8004A1FC(base + 0x180);
    func_800420D0();
    func_8003F568();
    func_8003F5CC();
}
void game_StageInit(s32 a0) {
    if (a0 & 1) {
        game_InitStageSound(g_snd_stage_bgm);
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
    g_game_pause = 0;
}

void game_SetPause(s32 a0) {
    if (a0) {
        g_game_pause = 1;
    } else {
        g_game_pause = 0;
    }
}

s32 game_GetPause(void) {
    return g_game_pause;
}

void game_ResetTimer(void) {
    g_game_timer = 0x23;
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
    return &g_stage_data;
}

void camera_InitMatrix(void) {
    s32 num = (s32)g_cam_fov_x << 12;
    s32 div = g_cam_fov_div;
    s32 v0 = num / div;
    s32 v1 = ((s32)g_cam_fov_z << 12) / div;
    D_800EEDB4 = 0;
    div = 0;
    num = v0;
    D_800EEDB6 = 0;
    v1 = -(s16)v1;
    D_800EEDB8 = 0;
    D_800EEDBA = 0;
    D_800EEDBC = div;
    g_cam_matrix = 0x1000;
    D_800EEDC0 = 0x1000;
    v0 = -(s16)num;
    D_800EEDB2 = v0;
    D_800EEDBE = v1;
}
void camera_Transform(s32 *a0, s32 *a1, s32 a2) {
    s32 new_var;
    s32 diff, prod;
    func_80052930(&g_cam_matrix, a0, a1);
    new_var = a0[5];
    diff = a0[6] - a2;
    prod = diff * D_800EEDB2;
    a1[6] = a2;
    a1[5] = new_var + (prod >> 12);
    diff = a0[6] - a2;
    prod = diff * D_800EEDBE;
    a1[7] = a0[7] + (prod >> 12);
}
void func_800470B0(s32 arg0, void *arg1, void *arg2, s32 arg3) {
    s16 sp10[9];
    s32 temp_v1;
    void *var_s0;
    register void *var_s1 asm("s1");
    register s32 var_s2 asm("s2");
    volatile s32 _sp_pad[2];

    var_s0 = arg1;
    temp_v1 = arg0 * 0x60;
    sp10[0] = 0x1000;
    sp10[1] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_x + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10[2] = 0;
    sp10[3] = 0;
    sp10[4] = 0;
    sp10[5] = 0;
    sp10[6] = 0;
    sp10[7] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_z + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10[8] = 0x1000;
    var_s1 = arg2;
    var_s2 = arg3;
    func_80052930(sp10, var_s0);
    *(s32 *)((s8 *)var_s1 + 0x14) = *(s32 *)((s8 *)var_s0 + 0x14) + (((*(s32 *)((s8 *)var_s0 + 0x18) - var_s2) * sp10[1]) >> 12);
    *(s32 *)((s8 *)var_s1 + 0x18) = var_s2;
    *(s32 *)((s8 *)var_s1 + 0x1C) = *(s32 *)((s8 *)var_s0 + 0x1C) + (((*(s32 *)((s8 *)var_s0 + 0x18) - var_s2) * sp10[7]) >> 12);
}
typedef struct {
    s32 w[8];
} Block32;
extern Block32 D_80101E08;
extern s16 D_800EEDD6;
extern s16 D_800EEDD8;
void camera_InitBoneData(void) {
    s16 *new_var;
    do { *(Block32 *)&g_cam_bone_data = D_80101E08; } while (0);
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
    return &g_cam_bone_data;
}

extern s32 g_anim_func_table[];
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
            ((void (*)(u8 *, u8 *))g_anim_func_table[idx])(a0_arg, s0 + 0x38);
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
    camera_InitRotation(&g_cam_bone_data2);
    g_cam_interp = 4;
}
extern s16 D_800EEE00;
extern s16 D_800EEE02;
extern s32 D_800EEE1C;
extern s32 D_800EEE20;
extern s32 D_800EEE24;
extern s32 D_80101E1C;
extern s32 D_80101E20;
extern s32 D_80101E24;
extern s32 D_800F66B0;
extern void func_8007E4DC(s32 *, s32 *, s32 *);
void func_800475A4(void) {
    s16 rot[3];
    s32 result[4];
    s32 buf1[8];
    s32 buf2[8];
    s16 angle;
    s32 computed;
    u8 *base;

    if (stage_GetVariant() != 0) {
        return;
    }

    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x6590;
    func_8007ED6C((s32 *)&D_80101E08, rot, result);

    angle = func_8007FD5C(result[0], result[2]);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result[2] + (s32)Judge[angle & 0xFFF] * result[0]) >> 12;
    result[2] = computed;

    {
        s16 neg = -func_8007FD5C(result[1], computed);
        base = &g_cam_bone_data2;
        D_800EEE00 = neg;
    }
    D_800EEE02 = angle;
    D_800EEE1C = D_80101E1C;
    D_800EEE20 = D_80101E20;
    D_800EEE24 = D_80101E24 + 0x6590;
    ((void (*)(u8 *, s32 *))D_800F66B0)(base + 0x10, buf1);
    ((void (*)(u8 *, s32 *))g_anim_func_table[0])((u8 *)&D_80101E08 - 8, buf2);
    func_8007E4DC(buf2, buf1, (s32 *)(base + 0x18));

    {
        s32 *temp = (s32 *)D_800A3820;
        D_800A3820 = (s32)(temp + 1);
        *temp = (s32)base;
    }
}

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
    g_snd_volume = a0;
}
INCLUDE_ASM("asm/funcs", func_800477E8);
extern s32 D_800EF558[];
extern s32 D_800EF59C[];
extern s32 D_800EF070;
void func_80047A90(void) {
    register s32 i asm("$8");
    register s32 *p558 asm("$4");
    register s32 *p59C asm("$5");
    register s16 *pJudge asm("$6");
    register s32 *pt2 asm("$10");
    register s32 *pt1 asm("$9");
    register s32 *pa1 asm("$5");
    register s32 a3 asm("$7");
    register s32 *pa2 asm("$6");
    register s32 *pt3 asm("$11");
    s32 v1;
    s32 a0;
    s32 *temp;

    i = 0;
    pJudge = Judge;
    p558 = D_800EF558;
    p59C = D_800EF59C;
    do {
        i++;
        *p59C = ((s32)pJudge[*p558 & 0xFFF] * 0x271) >> 10;
        p59C++;
        *p558 += 0x12;
        p558++;
    } while (i < 0x11);

    i = 1;
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    a3 = 0;
    pa2 = pt2;
    pt3 = pt1 + 0x11;
  inner_loop:
    {
        s32 v0 = *pa1;
        v1 = *pa2;
        v0 -= v1;
        a0 = 0x7D0 - v0;
        if (a0 < 0) {
            v1 = (a0 + 0xF) >> 4;
        } else {
            v1 = a0 / 10;
        }
    }
    *pa1 += v1;
    if (i == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
    }
    pa1++;
    a3 += 4;
    pa2++;
    if ((s32)pa1 < (s32)pt3)
        goto inner_loop;
    pt2 += 0x11;
    i++;
    pt1 += 0x11;
    if (i < 9)
        goto outer_loop;

    temp = (s32 *)D_800A3820;
    D_800A3820 = (s32)(temp + 1);
    *temp = (s32)&D_800EF070;
}


INCLUDE_ASM("asm/funcs", func_80047BE0);
extern s32 g_snd_config_tbl[];

s32 snd_CalcFade(s32 a0) {
    s32 a1 = (a0 + 0x7D00) / 3200;
    s32 a0_div = a0 / 3200;
    s32 remainder = a0 - a0_div * 3200;
    s32 odd = remainder & 1;
    g_snd_fade_pos = a1;
    g_snd_fade_amt = odd;
    if ((u32)a1 >= 18) {
        return (s32)0xFFFE7960;
    }
    {
        s32 val1 = g_snd_config_tbl[a1] * odd;
        s32 val2 = g_snd_config_tbl[a1 + 1] * (0x1000 - odd);
        return ((val1 + val2) >> 12) - 0x3F48;
    }
}
s32 snd_GetFadeCurve(void) {
    s32 v1 = g_snd_fade_pos;
    if ((u32)v1 >= 18) {
        return 0;
    }
    {
        s32 v0 = g_snd_fade_curve[v1];
        s32 a0 = g_snd_fade_amt;
        s32 val1 = v0 * a0;
        s32 v3 = g_snd_fade_curve[v1 + 1];
        s32 val2 = v3 * (0x1000 - a0);
        return (val1 + val2) >> 12;
    }
}

s32 snd_GetMaxFade(void) {
    return 0xD00;
}
