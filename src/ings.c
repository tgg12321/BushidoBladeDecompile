#include "common.h"
#include "include_asm.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Forward declarations for called functions */
extern void func_8001945C(void);
extern void ClearImage(void *, s32, s32, s32);
extern void snd_PlaySystemSe(void);

/* Externs for globals */
extern u8 g_file_flags;
extern u32 g_file_disc_size;
extern u32 D_80106A5C;
extern u8 g_file_dma_flag;
extern s32 g_file_heap_base;
extern u32 g_disp_gp_base;
extern u8 g_file_data_buf[];
extern u8 g_disp_fb_base;
extern u8 g_disp_fb_flag;
extern u32 D_800F5370;
extern u8 g_str_overflow;
extern u8 g_str_eff_init;

extern void printf();
extern void func_800164F8(void);
extern s16 Judge[];
extern s32 func_80083698(s32, s32, s32);
extern s32 func_800836C8(s32, s32, s32);
extern s32 bios_FileRead(s32, u8 *, s32);
extern void close(s32);
extern void func_800836B8(s32);


extern u8 D_800A30E8;
extern u8 D_800A30D4;
extern u8 D_800FB524;
extern u8 *D_800A374C;
extern u32 D_800A38B4;
extern u32 D_80102794;
extern s32 D_800A30DC;


extern u8 D_800F33D8;
extern u8 D_800F7438;
extern u8 D_800A37A8[];
extern void replay_camera_Init(s32, s32);
extern void game_FrameLoop(void);
extern void PutDispEnv(u8 *);
extern void LoadImage(u8 *, u8 *);
extern void VSync(s32);
extern void ClearOTagR(u8 *, s32);

extern s32 func_8005C8A8(s32, s32, u32, s32);
extern void func_8005C650(s32, s32, s32);
extern void PutDrawEnv(u8 *);
extern void DrawOTag(u8 *);
extern void ResetRCnt(u32);
extern s32 GetRCnt(u32);
extern s32 rand(void);
extern void func_800372C0(void);
extern void func_80083794(void);
extern void SetSp(u32);
extern void SetMem(s32);
extern void func_80060E04(s32);
extern void func_8003D2F4(void);

extern void func_80019568();
extern u8 D_800A3768;
extern u32 D_8008D090;
extern u8 D_80010034;
extern void func_8003D330(void);
extern u8 *func_8005D46C(u8 *);
extern u8 *func_8005D554(u8 *, u8);
extern s32 func_8005E54C(s32, u8 *, s32);
extern void func_80060414(s32, u8 *, s32);




typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect;

/* --- Non-decompiled functions (INCLUDE_ASM) --- */
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_800164AC\n"
    "    .word 0x8008C49C\n"
    "    .word 0x8008C5B4\n"
    "    .word 0x8008C750\n"
    "    .word 0x8008C8A0\n"
    "    .word 0x8008C930\n"
    "    .word 0x8008C94C\n"
    "    .word 0x8008C4C0\n"
    "    .word 0x8008C4D8\n"
    "    .word 0x8008C504\n"
    "    .word 0x8008C518\n"
    "    .word 0x8008C928\n"
    "    .word 0x8008C560\n"
    "    .word 0x8008C590\n"
    "    .word 0x00000000\n"
    "    .word 0x8008C95C\n"
    "    .word 0x8008C5D8\n"
    "    .word 0x8008C658\n"
    "    .word 0x8008C680\n"
    "    .word 0x8008C6DC\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_800164F8\n"
    "    .word 0x2402270F\n"
    "    .word 0x0001000D\n"
    "    .word 0x2442FFFF\n"
    "    .word 0x0441FFFD\n"
    "    .word 0x00000000\n"
    "    .word 0x03E00008\n"
    "    .word 0x00000000\n"
    ".set reorder\n"
    ".set at\n"
);
s32 file_LoadAll(s32 a0, u8 *dest) {
    s32 fd;
    s32 total;
    s32 remaining;
    s32 chunk;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    total = func_800836C8(fd, 0, 2);
    remaining = total;
    func_800836C8(fd, 0, 0);
    if (total > 0) {
        do {
            chunk = 0x4000;
            if (remaining < 0x4001) {
                chunk = remaining;
            }
            if (bios_FileRead(fd, dest, chunk) != chunk) {
                close(fd);
                return -1;
            }
            remaining -= chunk;
            dest += chunk;
        } while (remaining > 0);
    }
    func_800836B8(fd);
    return total;
}
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    func_800836C8(fd, sector << 11, 0);
    for (i = 0; i < count; i++) {
        if (bios_FileRead(fd, dest, 0x800) != 0x800) {
            close(fd);
            return -1;
        }
        dest += 0x800;
    }
    func_800836B8(fd);
    return count << 11;
}
s32 disp_CalcFov(s32 a0) {
    s32 tmp = (a0 << 12) / 360;
    s32 v1 = tmp / 2;
    s16 cos_val = Judge[(v1 + 0x400) & 0xFFF];
    s16 sin_val = Judge[v1 & 0xFFF];
    return (cos_val * 320) / sin_val;
}
void disp_SetFramebufferMode(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 i;
    u8 *ptr;
    s32 offset;

    i = 0;
    ptr = (u8 *)&g_disp_fb_base;
    offset = 0;
loop:
    *((u8 *)&g_disp_fb_flag + offset) = a0;
    *(ptr + 0x19) = a1;
    *(ptr + 0x1A) = a2;
    *(ptr + 0x1B) = a3;
    ptr += 0x4090;
    offset += 0x4090;
    i++;
    if (i < 2) {
        goto loop;
    }
}

/* --- Decompiled functions --- */

u32 file_GetFlag0(void) {
    return g_file_flags & 1;
}

u32 file_GetFlag1(void) {
    return (g_file_flags >> 1) & 1;
}

u32 file_GetFlag2(void) {
    return (g_file_flags >> 2) & 1;
}

void func_800167EC(void) {
    s32 i = 0;
    u32 c = 0x1A5E0;
    u8 *p;

    g_file_vram_timer = 0;
    g_file_flags = 0;
    p = (u8 *)&g_file_disc_size;
    *(s32 *)p = 0x7007;
    g_file_disc_type = 0;
    for (i = 0; i < 3; i++) {
        p[i * 8 + 8] = 0;
        p[i * 8 + 9] = 0;
        *(u32 *)(p + i * 8 + 0xC) = c;
    }
    D_80106A5C = 0x6978;
    func_8001945C();
}

void gpu_EnableDisplay(void) {
    ResetGraph(1);
}

void gpu_InitDisplay(void) {
    SetDispMask(0);
    ResetGraph(1);
    ClearImage(&g_disp_gp_base, 0, 0, 0);
    DrawSync(0);
}

void gpu_DisableDisplay(void) {
    SetDispMask(1);
}

void sys_StubEmpty(void) {
}

void sys_InitSound(void) {
    snd_PlaySystemSe();
}

extern void SetGraphDebug(s32);
extern void InitGeom(void);
extern void SetGeomOffset(s32, s32);
extern void SetGeomScreen(s32);
extern void SetDefDrawEnv(u8 *, s32, s32, s32, s32);
extern void SetDefDispEnv(u8 *, s32, s32, s32, s32);
void disp_Init(void) {
    u8 *base;

    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    InitGeom();
    SetGeomOffset(0x140, 0x78);
    SetGeomScreen(disp_CalcFov(0x2D));
    base = &g_disp_fb_base;
    SetDefDrawEnv(base, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(base + 0x4090, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(base + 0x5C, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(base + 0x40EC, 0, 0, 0x280, 0xF0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    ClearImage(&g_disp_gp_base, 0, 0, 0);
    DrawSync(0);
}
extern void InitPAD(u8 *, s32, u8 *, s32);
extern void StartPAD(void);
extern void ChangeClearPAD(s32);
extern void cdrom_Init(void);
extern void memcard_Init(void);
extern u8 g_pad_data;
void sys_Init(void) {
    u8 *base = &g_pad_data;
    ResetCallback();
    InitPAD(base, 8, base + 0x24, 8);
    StartPAD();
    ChangeClearPAD(0);
    disp_Init();
    g_disp_enable = DISP_DISABLED;
    g_disp_fade = 0;
    cdrom_Init();
    memcard_Init();
    sys_InitSound();
}
void func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2) {
    Rect rect;
    s32 i;

    rect = *(Rect *)&D_800A30D4;

    SetDispMask(0);
    SetDefDispEnv(&D_800FB524, 0, 0, 0x140, 0xF0);
    game_FrameLoop();
    replay_camera_Init(func_80036EA8(2, 0x61), (s32)arg0);
    game_FrameLoop();
    PutDispEnv(&D_800FB524);
    DrawSync(0);
    LoadImage((u8 *)&rect, arg0 + 0x14);
    DrawSync(0);
    SetDispMask(1);

    for (i = 0; i < 0x96; i++) {
        if (i >= 0x79) {
            u16 *pixels = (u16 *)(arg0 + 0x28);
            s32 j;

            for (j = 0; j < rect.w * rect.h; j++, pixels++) {
                u32 pixel = *pixels;
                s32 value = (s32)(pixel & 0x1F) >> 1;
                u32 temp;

                pixel &= 0xFFFF;
                temp = pixel >> 1;
                temp &= 0x1E0;
                value += temp;
                pixel >>= 1;
                pixel &= 0x3C00;
                value += pixel;
                *pixels = value;
            }
            LoadImage((u8 *)&rect, arg0 + 0x14);
        }

        DrawSync(0);
        VSync(0);
    }

    SetDispMask(0);
    SetDefDispEnv(&D_800FB524, 0, 0, 0x280, 0xF0);
    SetDispMask(1);
}
void sys_Panic(void) {
    printf((s32)&g_str_overflow);
    while (1) {
        func_800164F8();
    }
}
void file_ResetDmaFlag(void) {
    g_file_dma_flag = 0;
}
extern s32 func_80060CB8(u32, u32);
void file_LoadOverlay(void) {
    s32 size;

    if (g_file_dma_flag != 0) {
        return;
    }
    size = func_80060CB8(0x801D8800, 0x8010E800);
    printf((s32)&g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    g_file_dma_flag = 1;
}
extern void snd_Init(void);
extern s32 func_8005B7C4(u32);
extern void memcpy(u32, u32, s32);
extern void snd_VabFakeOpen(u32, s32);
extern void func_8005C614(void);
void file_LoadSoundData(void) {
    s32 size;

    snd_Init();
    size = func_8005B7C4(0x801D8800);
    if (size >= 0xD01) {
        sys_Panic();
    }
    memcpy(0x8010DB00, 0x801D8800, size);
    snd_VabFakeOpen(0xFFF35300, 0);
    func_8005C614();
    D_800A3906 = 1;
}
extern u8 g_str_limit;
extern u32 D_800A3770;
extern u32 D_800A3774;
extern u32 D_800A3798;
extern u8 D_800A3744;
extern u8 D_800A3745;
extern u8 D_800A3746;
extern void func_80020D70(void);
extern void game_Init(void);
extern u8 D_800A36B0;
extern void func_80019534(void);
extern void func_8003D2C4(void);
extern void func_8001C444(void);
void sys_GameInit(void) {
    printf((s32)&g_str_limit, 0x8010DB00);
    func_800167EC();
    func_80020D70();
    D_800A3770 = 0x801D8800;
    D_800A3774 = 0x801EBC00;
    D_800A3798 = 0x13400;
    g_file_dma_flag = 0;
    D_800A3906 = 0;
    file_LoadSoundData();
    func_80019534();
    func_8003D2C4();
    func_8001C444();
    D_800A36F9 = 0;
    D_800A3690 = 0;
    D_800A3744 = 0;
    D_800A3745 = 0;
    D_800A3746 = 0;
    game_Init();
    D_800A36F1 = 2;
    D_800A38C6 = 0;
    D_800A36B0 = 0;
    D_800A3928 = 0;
}

void gpu_SetDrawMode(void) {
    DrawSync(0);
}

void func_80016E60(u8 *arg0, s32 arg1) {
    u8 *ot[2];
    u8 *env;
    s32 select;
    s32 special;
    s32 limit;
    u32 fb_base;
    s32 idx;
    u32 padbits;
    u8 *ot_base;

    select = 0;
    special = 0;
    /* FAKE: pass-through local handle on the parameter (pointer-alias family,
       .claude/rules/pointer-alias-fake-exception.md exact-scope bullet 3),
       consumed once at the DrawOTag call site; effect: the prologue's
       (save,init) group for $s5 emits third instead of first.
       mechanism: combine.c's i2/i3 merge folds `p1 = a0` into this later copy
       and leaves it at the LATER position, so sched.c:3256's parameter-copy pin
       (leading run of hard-register-source SETs) no longer applies, sched1's
       birthing_insn_p boost (sched.c:2504) emits it after the two init insns,
       and sched2's INSN_LUID tie-break (sched.c:2462) orders the groups s1,s2,s5.
       lever-exhaustion: memory/grind/func_80016E60/hypotheses.md (s1-s5 bare-
       parameter forms, [s6] E-s6-4 hard mechanism, [s6] E-s6-7 honest env route). */
    ot_base = arg0;
    if (D_800A38DC == 2) {
        special = D_800A389A < 1;
    }
    limit = 3;
    if (special != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    func_8005C650(3, 0x7F, 0x7F);
    fb_base = (&D_800A3770)[D_800A36AC & 1];

    while (1) {
        idx = D_800A36AC & 1;
        D_800A38B4 = fb_base + (idx * 0x9A00);
        D_800A374C = (u8 *)&ot[idx];
        env = &D_800F7438 + (idx * 0x4090);

        ClearOTagR(D_800A374C, 1);
        func_80019568();
        if (special != 0) {
            func_8005C8A8(2, select | (D_800A3788 << 16), D_800A38B4, 0);
        } else {
            func_8005C8A8(0, select, D_800A38B4, 0);
        }
        func_80036940();
        func_8005C6D0();
        DrawSync(0);
        VSync(2);
        /* FAKE: single-level do { } while (0) wrap around the two env
           publishes (do-while-zero family, .claude/rules/do-while-zero-exception.md);
           effect: env is seated in $s0 and select in $s1, the target's assignment.
           mechanism: the wrap's loop notes make flow.c weight env's three in-loop
           references at loop_depth 3 instead of 2, lifting its global.c
           allocno_compare priority above select's.
           lever-exhaustion: memory/grind/func_80016E60/hypotheses.md ([s2] H6 and
           [s6] E-s6-7/E-s6-8 - the honest env split-init routes measure 22 vs 21). */
        do {
            PutDispEnv(env + 0x5C);
            PutDrawEnv(env);
        } while (0);
        DrawOTag(ot_base + 0x408C);
        DrawOTag(D_800A374C);
        D_800A36AC++;

        padbits = D_80102794;
        if (padbits & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            break;
        }
        if (padbits & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            break;
        }
        if (padbits & 0x10001000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == 0) ? limit - 1 : select - 1;
        } else if (padbits & 0x40004000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == limit - 1) ? 0 : select + 1;
        }

        if ((special != 0) && (select >= 3)) {
            if (D_80102794 & 0x80008000) {
                u8 shift;
                s32 mask;
                s32 bits;
                func_8005C650(0, 0x7F, 0x7F);
                shift = select - 3;
                mask = 1;
                mask <<= shift;
                bits = D_800A3788;
                bits |= mask;
                D_800A3788 = bits;
            } else if (D_80102794 & 0x20002000) {
                u8 shift;
                s32 mask;
                s32 bits;
                func_8005C650(0, 0x7F, 0x7F);
                shift = select - 3;
                mask = 1;
                mask <<= shift;
                bits = D_800A3788;
                bits &= ~mask;
                D_800A3788 = bits;
            }
        }
    }

    if (select != 0) {
        if (select == 1) {
            D_800A31DA = 1;
            D_800A3834 = 8;
            func_800372C0();
        } else if (select == 2) {
            D_800A3834 = 8;
            func_800372C0();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}
void rng_SetSeed(s32 a0) {
    g_file_heap_base = a0;
}
s32 rng_Next(void) {
    s32 seed = g_file_heap_base;
    s32 result = seed * 5497 + 0x7FA9;
    seed = (seed >> 16) ^ result;
    g_file_heap_base = seed;
    return seed & 0x7FFF;
}
void main(void) {
    s32 idx;
    u8 *env;
    u8 *ot;
    s32 voice;
    u32 *tbl;

    func_80083794();
    SetSp(0x801FFF00);
    SetMem(2);
    sys_Init();
    sys_GameInit();
    SetDispMask(1);
    func_80016A8C((u8 *)0x80118800, env, idx);

    tbl = &D_800A3770;
    D_800A3834 = 0xF;
    D_800A390D = 0;
    D_800A36AC = 0;

loop:
    idx = D_800A36AC & 1;
    env = &D_800F7438 + idx * 0x4090;
    ot = env + 0x70;
    ClearOTagR(ot, 0x1008);
    D_800A374C = ot;
    D_800A38B4 = tbl[idx];
    func_80060E04(idx);
    func_8003D2F4();
    func_80019568(voice);
    func_80036940();
    func_8005C6D0();

    if (D_800A3928 != 0) {
        func_800372C0();
        D_800A3768 = 0xFF;
        D_800A3928 = 0;
        D_800A31DA = 0;
        D_800A3834 = 8;
    }

    ((void (*)(void))(&D_8008D090)[D_800A3834])();
    func_8003D330();

    do {
        s32 cnt = GetRCnt(0xF2000001u);
        /* FAKE: same-pseudo chain blocks combine's 2->2 split gate
           (combine.c:1836 reg_referenced_p) — fresh-variable spellings fold
           to sll;addiu -128. Owner grant 2026-08-11 (docs/grind/decisions.md,
           split-init-accumulation family extension). */
        s32 lim = D_800A36F1;
        lim = lim - 1;
        lim = lim << 8;
        lim = lim + 0x80;
        if (cnt >= lim) break;
        rand();
    } while (1);

    VSync(1);
    DrawSync(0);
    VSync(0);
    ResetRCnt(0xF2000001u);

    voice = D_800A390D;
    if (voice == 0) {
        PutDispEnv(env + 0x5C);
        PutDrawEnv(env);
    }

    {
        s32 cnt = (s32)tbl[idx];
        s32 adj = D_800A38B4 + 0xFFFECC00u;
        s32 remaining = cnt - adj;
        if (remaining < D_800A30DC) {
            D_800A30DC = remaining;
        }
        if (remaining < 0) {
            printf(&D_80010034);
            while (1) {
                func_800164F8();
            }
        }
    }

    if (D_800A390D != 0) {
        D_800A390D--;
    } else {
        DrawOTag(env + 0x408C);
        D_800A36AC++;
    }

    if (D_800A3834 != 1) goto loop;
    if (voice != 0) goto loop;
    if (D_80102794 & 0x08000800u) goto call_func;
    if (D_800A38DC != 2) goto loop;
    if (D_800A3713 == 0) goto loop;
    D_800A3713--;
    if (D_800A3713 != 0) goto loop;
call_func:
    func_80016E60(env, idx);
    goto loop;
}

/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action_2  |  189i  |  x2 size collision */
/* 100% pure C, zero rules needed: `sandbox func_800174F4 --disable all` scores
 * 0 with 136 == 136 instructions and the sole regfix rule disabled.
 * The two `do { ... } while (0);` wraps in case 1/2 are annotated at their
 * sites; the full derivation (six grind sessions) is in
 * memory/grind/func_800174F4/{evidence,hypotheses,self_vet}.md.
 */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;
    s32 mode;

    prim = (s32)(&D_800F33D8);
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    env = (s32)sp20;
    h = 0xF0;
    mask = D_800A36AC & 1;
    mask = -mask;
    SetDefDrawEnv((u8 *)env, 0, mask & 0xF0, 0x280, h);
    sp20[0x18] = 0;
    PutDrawEnv((u8 *)env);
    D_800A374C = sp18;
    ClearOTagR(sp18, 2);
    mode = g_disp_enable;
    switch (mode) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: memory/grind/
             * func_800174F4/hypotheses.md K5/K10/K11/K12/H-S4-2/H-S5-1/H-S5-2.
             * Effect: seats the loop counter in $s0 and h in $s1 while the
             * counter is initialised before rand(). */
            do { i = 0; } while (0);
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h == 0) {
                break;
            }
        inner_loop:
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: as above.
             * Effect: the companion wrap for the counter's in-loop refs; it
             * must span the whole body so no code label lands between the
             * call and `i++` (that placement costs reorg.c the jal delay
             * slot). */
            do {
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
            } while (0);
            if (i >= h) {
                break;
            }
            goto inner_loop;
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
        mode = D_800A37A0;
    {
        u8 a2_val = D_800A38F8;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)mode) < (u32)a0_val) {
            break;
        }
        div_result = h / (mode + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == mode) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == mode) {
                h |= 0x8000;
            }
            func_80060414(h, (u8 *)prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(D_800A374C + 4));
    DrawSync(0);
}
void obj_ClearAll(void) {
    s32 i;
    for (i = 0x16C; i >= 0; i -= 0x34) {
        *(s32 *)(g_file_data_buf + i) = 0;
    }
}

s32 obj_CalcOffset(s32 a0, s32 a1) {
    return (a0 << 6) + (a1 << 4);
}

extern s32 Square12(s32 *, s32 *);
extern s32 SquareRoot12(s32);
s32 math_Distance3D(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[3];

    in[0] = (a0[0] - a1[0]) >> 2;
    in[1] = (a0[1] - a1[1]) >> 2;
    in[2] = (a0[2] - a1[2]) >> 2;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 2;
}
s32 math_Distance3D_16(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 4;
    in[1] = (a0[1] - a1[1]) >> 4;
    in[2] = (a0[2] - a1[2]) >> 4;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 4;
}
INCLUDE_ASM("asm/funcs", func_80017848);
INCLUDE_ASM("asm/funcs", func_80017A44);
extern void func_80017A44(void *, u8 *);
typedef struct { s32 v[8]; } ObjBlock;
s32 func_80017D84(u8 *a0) {
    u8 *p;
    s32 i;
    s32 c;

    p = g_file_data_buf;
    for (i = 0; i < 8; i++) {
        if (*(s32 *)p == 0) break;
        p += 0x34;
    }
    if (i == 8) return -1;
    if (D_800A30E8 < i) D_800A30E8 = i;
    *(u16 *)(p + 4) = *(u16 *)a0;
    *(s32 *)p = *(s32 *)(a0 + 4);
    *(ObjBlock *)(p + 0x14) = **(ObjBlock **)(a0 + 0xC);
    *(s32 *)(p + 8) = *(s16 *)(a0 + 2);
    c = *(s32 *)(a0 + 0x10);
    *(s16 *)(p + 6) = 0;
    *(s32 *)(p + 0xC) = c;
    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);
    func_80017A44(a0, p);
    return i;
}
void obj_Clear(s32 a0) {
    *(s32 *)(g_file_data_buf + a0 * 52) = 0;
}
void obj_UpdatePosition(s32 a0, s32 a1) {
    u8 *ptr = g_file_data_buf + a0 * 52;
    s32 c = *(s32 *)(ptr + 0xC) + a1;
    *(s32 *)(ptr + 0xC) = c;
    *(s32 *)(ptr + 0x10) = c + (*(s16 *)(ptr + 4) << 6);
}
void obj_AddValue(s32 a0, s32 a1) {
    s32 *ptr = (s32 *)(g_file_data_buf + a0 * 52);
    *ptr = *ptr + a1;
}
void scratchpad_Save(void) {
    vu32 *src = (vu32 *)0x1F800000;
    u32 *dst = (u32 *)&D_800F5370;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}
void scratchpad_Restore(void) {
    u32 *src = (u32 *)&D_800F5370;
    vu32 *dst = (vu32 *)0x1F800000;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void sys_StubEmpty2(void) {
}

void sys_StubEmpty3(void) {
}
