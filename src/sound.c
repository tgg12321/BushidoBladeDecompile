#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"
#include "gte.h"

/* Forward declarations for called functions */
extern void func_800453E0(s32);
extern s32 *func_800455AC(s32);
extern void func_80045510(s32, s32);
extern void func_80045230(s32);
extern void func_80054FDC(s32);
extern void SetRCnt(u32, s32, s32);
extern void GetRCnt(u32);
extern void StartRCnt(u32);
extern void func_8004473C(void);
extern void func_80044800(void);
extern void func_80048F58(s32, s32);
extern void func_80048FFC(s32);
extern void *func_8004153C(void);
extern s32 func_800477E8(void);
extern void func_80047A90(void);
extern void func_80048B8C(s32);
extern void func_80044100(s32, s32);
extern void func_80045600(s32, s32);
extern void func_80045694(s32, void (*)(void));
extern void func_800460E4(s32);
extern void func_800421C8(s32);
extern void func_8003E0E0(void);
extern void func_8003E6D8(s32);
extern void func_8003DA8C(s32, s32);
extern void player_Destroy(s32);
extern void func_8004668C(void);
extern void func_80046020(void);
extern void func_80049E1C(void);
extern void camera_InitRotation(u8 *);
extern void func_80042A88(s32 *, s32 *);
extern void ApplyMatrix(s32 *, s16 *, s32 *);
extern s16 ratan2(s32, s32);
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
extern void ApplyMatrixLV(void *, void *, void *);
extern void func_800418D0(s32 *);
extern void func_8004A1FC(void *);
extern void func_800420D0(void);
extern void stage_ClearLighting(void);
extern void stage_ApplyLighting(void);
extern void stage_InitCollision(void);
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

void func_800469A0(s32 a0) {
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
    player_Destroy(0);
    player_Destroy(1);
    func_8004668C();
    func_80046020();
    func_80049E1C();
    snd_StopBgm();
}

void snd_PlaySystemSe(void) {
    SetRCnt(0xF2000001, -1, 0x2000);
    StartRCnt(0xF2000001);
}

void snd_StopSystemSe(void) {
    GetRCnt(0xF2000001);
}

void game_Init(void) {
    /* FAKE: constant-holder locals — set in source order ahead of the fence
       below so 1 seats in $v0 and 2 in $v1 before the store tail, with $v0
       freed for reuse by 0x23 mid-tail */
    s16 one;
    s16 two;

    func_800451A0();
    func_800451D0();
    func_80042E90();
    func_80044498();
    func_80049E4C();
    func_80049F4C();
    func_8003D91C();
    func_800404D8();
    func_8003F7F4();
    one = 1;
    two = 2;
    /* FAKE: sched.c mid-block loop-note fence — keeps the two pre-fence
       constant sets from being folded into the store tail (measured: without
       it CSE/sched collapse 1/2/0x23 into a single serialized $v0) */
    do { } while (0);
    g_game_pause = one;
    D_800F665A = one;
    g_color_mode = 0;
    D_800F6650 = 0;
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

        ApplyMatrixLV(matrix_buf, trans, result);

        {
            s32 *rp = result;
            s32 *ap = a0;
            D_80101E3C = *rp++ + *ap++;
            D_80101E40 = *rp++ + *ap++;
            D_80101E44 = *rp++ + *ap++;
        }

        func_800418D0((s32 *)((char *)rot_base - 0x10));
        camera_InitBoneData();
        stage_InitCollision();

        g_snd_stage_bgm = a2;
    }

    base = (u8 *)&D_800F62E0;
    func_8004A1FC(base);
    func_8004A1FC(base + 0x60);
    func_8004A1FC(base + 0x180);
    func_800420D0();
    stage_ClearLighting();
    stage_ApplyLighting();
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
    func_8003E6D8(a0);
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
void func_800470B0(s32 arg0, s32 *arg1, s32 *arg2, s32 arg3) {
    MATRIX sp10;
    s32 temp_v1;
    s32 *var_s0;

    var_s0 = arg1;
    temp_v1 = arg0 * 0x60;
    sp10.m[0][0] = 0x1000;
    sp10.m[0][1] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_x + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10.m[0][2] = 0;
    sp10.m[1][0] = 0;
    sp10.m[1][1] = 0;
    sp10.m[1][2] = 0;
    sp10.m[2][0] = 0;
    sp10.m[2][1] = (s16) -((s32)(*(s16 *)((s8 *)&g_cam_fov_z + temp_v1) << 12) / *(s16 *)((s8 *)&g_cam_fov_div + temp_v1));
    sp10.m[2][2] = 0x1000;
    func_80052930(&sp10, var_s0, arg2);
    arg2[5] = var_s0[5] + (((var_s0[6] - arg3) * sp10.m[0][1]) >> 12);
    arg2[6] = arg3;
    arg2[7] = var_s0[7] + (((var_s0[6] - arg3) * sp10.m[2][1]) >> 12);
}
typedef struct {
    s32 w[8];
} Block32;
extern Block32 D_80101E08;
typedef struct { s16 lo; s16 hi; } CamHalves;
extern s16 D_800EEDD6;
extern s16 D_800EEDD8;
void camera_InitBoneData(void) {
    /* FAKE: sched fence — without it sched1 hoists the lhu of D_800EEDD6
       above the block copy (renaming the copy's regs). D_800EEDD6/D_800EEDD8
       physically live INSIDE g_cam_bone_data (+6/+8), so the dependency is
       real, but the split extern symbols hide it from GCC's alias analysis;
       no distinct-symbol spelling can express it (measured s2). */
    do { *(Block32 *)&g_cam_bone_data = D_80101E08; } while (0);
    {
        s16 h0 = D_800EEDD6;
        s16 h1 = D_800EEDD8;
        D_800EEDD6 = h0 >> 1;
        D_800EEDD8 = h1 >> 1;
        /* struct-view of the two halfwords at D_800EEDD8 (target relocation
           D_800EEDD8+0x2 proves the object spans 4 bytes); the direct
           *((&D_800EEDD8)+1) spelling makes cse common the +2 address into a
           register (la) where target keeps both accesses symbolic */
        ((CamHalves *)&D_800EEDD8)->hi = ((CamHalves *)&D_800EEDD8)->hi >> 1;
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
        *(s16 *)(s0 + 8) = 0;
        {
            u8 *a0_arg = s0 + 0x10;
            *(s16 *)(s0 + 2) = 0;
            s0[0] = 0;
            s0[1] = 0;
            *(s32 *)(s0 + 0xC) = 0;
            *(s16 *)(s0 + 0xA) = v0;
            *(s16 *)(s0 + 0x10) = 0;
            *(s16 *)(s0 + 0x12) = 0;
            *(s16 *)(s0 + 0x14) = 0;
            ((void (*)(u8 *, u8 *))g_anim_func_table[*(s16 *)(s0 + 8)])(a0_arg, s0 + 0x38);
        }
    }
    *(s32 *)(s0 + 0x54) = 0;
    *(s32 *)(s0 + 0x50) = 0;
    *(s32 *)(s0 + 0x4C) = 0;
    *(Block32 *)(s0 + 0x18) = *(Block32 *)(s0 + 0x38);
}

s16 *camera_CalcAngles(void) {
    s16 rot[3];
    s32 sp18[3];
    s32 pos[8];
    s16 s0;

    func_80042A88((s32 *)((u8 *)*(s32 **)&D_800A3708 + 0x10), pos);
    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x1000;
    ApplyMatrix(pos, rot, sp18);
    s0 = ratan2(sp18[0], sp18[2]);
    sp18[2] = ((s32)Judge[((s16)s0 + 0x400) & 0xFFF] * sp18[2]
              + (s32)Judge[s0 & 0xFFF] * sp18[0]) >> 12;
    D_800A33C8 = -ratan2(sp18[1], sp18[2]);
    D_800A33CA = s0;
    return &D_800A33C8;
}

void game_EffInit(void) {
    func_8004473C();
}

void game_EffCleanup(void) {
    func_80044800();
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
    func_80044800();
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
extern void MulMatrix0(s32 *, s32 *, s32 *);
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
    ApplyMatrix((s32 *)&D_80101E08, rot, result);

    angle = ratan2(result[0], result[2]);

    computed = ((s32)Judge[(angle + 0x400) & 0xFFF] * result[2] + (s32)Judge[angle & 0xFFF] * result[0]) >> 12;
    result[2] = computed;

    {
        s16 neg = -ratan2(result[1], computed);
        base = &g_cam_bone_data2;
        D_800EEE00 = neg;
    }
    D_800EEE02 = angle;
    D_800EEE1C = D_80101E1C;
    D_800EEE20 = D_80101E20;
    D_800EEE24 = D_80101E24 + 0x6590;
    ((void (*)(u8 *, s32 *))D_800F66B0)(base + 0x10, buf1);
    ((void (*)(u8 *, s32 *))g_anim_func_table[0])((u8 *)&D_80101E08 - 8, buf2);
    MulMatrix0(buf2, buf1, (s32 *)(base + 0x18));

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
    func_80044800();
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
extern u32 GetTPage(s32, s32, s32, s32);
extern u32 GetClut(s32, s32);
extern void func_800417D0(s32 *);
extern s16 *D_800A33D0;
extern s8 D_800EF070;
extern s8 D_800EF071;
extern s16 D_800EF076;
extern s16 D_800EF078;
extern s16 D_800EF07A;
extern s32 D_800EF07C;
extern s16 D_800EF080;
extern s16 D_800EF082;
extern s16 D_800EF084;
extern s32 D_800EF0BC;
extern s32 D_800EF0C0;
extern s32 D_800EF0C4;
extern s32 D_800EF558[];
extern s32 D_800EF59C[];
s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    s32 s1val;
    s32 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 val;

    s0 = D_800A33D0;
    s3val = GetTPage(0, 0, 0x2C0, 0x1C0);
    s2val = GetTPage(0, 0, 0x2C0, 0x180);
    s1val = GetClut(0x10, 0x1E0);
    t1val = GetClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        /* FAKE: single-level do-while(0) wrap. Its loop note adds one unit of
         * loop_depth reference weight to everything in this body, which seats
         * t1val in $t1 and the 0x2C00 constant in $t2 as target has them
         * (without it: t1val 3 refs/76 insns loses to t2's 5/150). */
        do {
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            v0 = v1 | a1;
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        } while (0);
        a0 = 0;
    } while (a3 < 8);

    {
        s32 *a0p;
        a0p = (s32 *)&D_800EF070;
        *(s8 *)a0p = 0xE;
        D_800EF07A = 4;
        D_800EF0BC = -0x2EE0;
        D_800EF071 = 0;
        D_800EF0C0 = 0;
        D_800EF0C4 = -0xFA0;
        D_800EF080 = 0;
        D_800EF082 = 0;
        D_800EF084 = 0;
        D_800EF078 = 0;
        D_800EF07C = 0;
        D_800EF076 = 0;
        func_800417D0(a0p);
    }

    a3 = 0;
    w = 0;
    do {
        val = w;
        p = &D_800EF59C[a3 * 0x11];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        a3 += 1;
    } while (a3 < 9);

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}

void func_80047A90(void) {
    s32 i;
    s32 a3;
    s32 v1;
    s32 a0;
    s16 *jb;
    s32 *p558;
    s32 *p59C;
    s32 *pt2;
    s32 *pt1;
    s32 *pa1;
    s32 *pa2;
    s32 *pt3;
    s32 *temp;

    i = 0;
    jb = Judge;
    p558 = D_800EF558;
    p59C = D_800EF59C;
  loop1:
    i++;
    *p59C = ((s32)jb[*p558 & 0xFFF] * 0x271) >> 10;
    p59C++;
    *p558 += 0x12;
    p558++;
    if (i < 0x11)
        goto loop1;

    i = 1;
    pt2 = D_800EF59C;
    pt1 = D_800EF59C + 0x11;
  outer_loop:
    pa1 = pt1;
    do {
        /* FAKE: loop-note ref weighting lifts a3's allocno priority above
         * the shared counter i, seating a3 in $a3 and i in $t0 */
        a3 = 0;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting lifts pa2 above the shared counter
         * i, seating pa2 in $a2 (shared with the loop-1 Judge base) */
        pa2 = pt2;
    } while (0);
    do {
        /* FAKE: loop-note ref weighting keeps pt1 ahead of pt2 in
         * allocation order (pt1->$t1, pt2->$t2) after the pa2 wrap's
         * weighted pt2 use lifted pt2 */
        pt3 = pt1 + 0x11;
    } while (0);
  inner_loop:
    a0 = 0x7D0 - (*pa1 - *pa2);
    if (a0 < 0) {
        v1 = (a0 + 0xF) >> 4;
    } else {
        v1 = a0 / 10;
    }
    *pa1 += v1;
    if (i == 8) {
        *(s32 *)((s8 *)g_snd_fade_curve + a3) = v1;
        pa1++;
        a3 += 4;
        pa2++;
    } else {
        /* FAKE: loop tail duplicated into both arms (cross-jump re-merges,
         * byte-neutral); reg_n_refs lift lands pa2->$a2, a3->$a3 */
        pa1++;
        a3 += 4;
        pa2++;
    }
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


extern void RotTransPers3(SVECTOR *, SVECTOR *, SVECTOR *, s32 *, s32 *, s32 *, s32 *, s32 *);
extern void ReadSZfifo3(s32 *, s32 *, s32 *);
extern s16 *func_8004BCC0(s32, s16 *, s16 *, s32);
extern SVECTOR D_800EF0D8[17];
extern SVECTOR D_800EF168[17];
void func_80047BE0(void) {
    s32 sxy0, sxy1, sxy2, pflag, flag;
    s32 sz0, sz1, sz2;
    s16 *s7val;
    s32 i, j;
    s32 *src;
    SVECTOR *base;
    SVECTOR *v;
    s32 *dst32;
    s16 *dst16;
    s16 z;

    s7val = D_800A33D0;
    i = 0;
    src = D_800EF59C;
    while (i < 9) {
        if (i & 1) {
            base = D_800EF168;
            dst32 = (s32 *)0x1F800068;
            dst16 = (s16 *)0x1F800134;
        } else {
            base = D_800EF0D8;
            dst32 = (s32 *)0x1F800020;
            dst16 = (s16 *)0x1F800110;
        }
        v = base;
        j = 0;
        z = -0x2EE0;
        while (j < 17) {
            v->vx = src[j] - 0xFA0;
            v->vy = 0;
            v->vz = z;
            v++;
            z += 0x7D0;
            j++;
        }
        v = base;
        j = 0;
        while (j < 6) {
            RotTransPers3(&v[0], &v[1], &v[2], &sxy0, &sxy1, &sxy2, &pflag, &flag);
            v += 3;
            ReadSZfifo3(&sz0, &sz1, &sz2);
            *dst32++ = sxy0;
            *dst32++ = sxy1;
            *dst32++ = sxy2;
            *dst16++ = sz0;
            *dst16++ = sz1;
            *dst16++ = sz2;
            j++;
        }
        if (i != 0) {
            s7val = func_8004BCC0(0x10, (s16 *)base, s7val, 0);
        }
        i++;
        src += 17;
    }
}
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
