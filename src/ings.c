#include "common.h"
#include "include_asm.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* Forward declarations for called functions */
extern void func_8001945C(void);
extern void func_8007B4D0(void *, s32, s32, s32);
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

extern void func_80079208();
extern void func_800164F8(void);
extern s16 Judge[];
extern s32 func_80083698(s32, s32, s32);
extern s32 ang_hosei(s32, s32, s32);
extern s32 bios_FileRead(s32, u8 *, s32);
extern void func_80078A18(s32);
extern void func_800836B8(s32);


extern u8 D_800A30E8;
extern u8 D_800A30D4;
extern u8 D_800FB524;
extern u8 *D_800A374C;
extern u32 D_800A38B4;
extern u32 D_80102794;
extern s32 D_800A30DC;
extern u8 g_str_prim_overflow;
extern u32 g_module_func_tbl[];
extern u8 D_800F33D8;
extern u8 D_800A37A8[];
extern void replay_camera_Init(s32, s32);
extern void func_80036F40(void);
extern void func_8007BC08(u8 *);
extern void func_8007B600(u8 *, u8 *);
extern void func_800828CC(s32);
extern void func_8007B844(u8 *, s32);
extern void func_80019568(s32);
extern void func_8005C8A8(s32, s32, u32, s32);
extern void func_8005C650(s32, s32, s32);
extern void func_8007B9B0(u8 *);
extern void func_8007B93C(u8 *);
extern void func_80078BA8(u32);
extern s32 func_80078B04(u32);
extern s32 func_80079154(void);
extern void func_800372C0(void);
extern void motion_Open(void);
extern void func_800789D8(u32);
extern void func_80078968(s32);
extern void func_80060E04(s32);
extern void func_8003D2F4(void);
extern void func_8003D330(void);
extern u8 *func_8005D46C(u8 *);
extern u8 *func_8005D554(u8 *, u8);
extern void func_8005E54C(s32, u8 *, s32);
extern void func_80060414(s32, u8 *, s32);
extern void func_8007EEEC(u8 *);
extern void func_8007EF4C(u8 *);
extern void func_8007F2AC(u8 *, s32 *, s32 *);

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
    total = ang_hosei(fd, 0, 2);
    remaining = total;
    ang_hosei(fd, 0, 0);
    if (total > 0) {
        do {
            chunk = 0x4000;
            if (remaining < 0x4001) {
                chunk = remaining;
            }
            if (bios_FileRead(fd, dest, chunk) != chunk) {
                func_80078A18(fd);
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
    s32 _pad[2];
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    ang_hosei(fd, sector << 11, 0);
    i = 0;
    if (count > 0) {
        do {
            if (bios_FileRead(fd, dest, 0x800) != 0x800) {
                func_80078A18(fd);
                return -1;
            }
            i += 1;
            dest += 0x800;
        } while (i < count);
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

    for (; i < 2; i++) {
        *(vu8 *)((u8 *)&g_disp_fb_flag + offset) = a0;
        *(vu8 *)(ptr + 0x19) = a1;
        *(vu8 *)(ptr + 0x1A) = a2;
        *(vu8 *)(ptr + 0x1B) = a3;
        ptr += 0x4090;
        offset += 0x4090;
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
    register s32 i asm("a0") = 0;
    register u32 c asm("a1") = 0x1A5E0;
    register u8 *tmp asm("v0");
    register u8 *p asm("v1");

    g_file_vram_timer = 0;
    g_file_flags = 0;
    tmp = (u8 *)&g_file_disc_size;
    asm("addu %0,%1,$zero" : "=r"(p) : "r"(tmp));
    *(s32 *)p = 0x7007;
    g_file_disc_type = 0;
    do {
        ((volatile u8 *)p)[8] = 0;
        ((volatile u8 *)p)[9] = 0;
        *(volatile u32 *)(p + 0xC) = c;
        p += 8;
        i++;
    } while (i < 3);
    D_80106A5C = 0x6978;
    func_8001945C();
}

void gpu_EnableDisplay(void) {
    func_8007AE7C(1);
}

void gpu_InitDisplay(void) {
    func_8007B2A0(0);
    func_8007AE7C(1);
    func_8007B4D0(&g_disp_gp_base, 0, 0, 0);
    func_8007B33C(0);
}

void gpu_DisableDisplay(void) {
    func_8007B2A0(1);
}

void sys_StubEmpty(void) {
}

void sys_InitSound(void) {
    snd_PlaySystemSe();
}

extern void func_8007B114(s32);
extern void func_8007E094(void);
extern void func_8007EFDC(s32, s32);
extern void func_8007EFFC(s32);
extern void func_8007A694(u8 *, s32, s32, s32, s32);
extern void func_8007A74C(u8 *, s32, s32, s32, s32);
void disp_Init(void) {
    u8 *base;

    func_8007AE7C(0);
    func_8007B114(0);
    func_8007B2A0(0);
    func_8007E094();
    func_8007EFDC(0x140, 0x78);
    func_8007EFFC(disp_CalcFov(0x2D));
    base = &g_disp_fb_base;
    func_8007A694(base, 0, 0, 0x280, 0xF0);
    func_8007A694(base + 0x4090, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x5C, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x40EC, 0, 0, 0x280, 0xF0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8007B4D0(&g_disp_gp_base, 0, 0, 0);
    func_8007B33C(0);
}
extern void func_80078C9C(u8 *, s32, u8 *, s32);
extern void func_80078D38(void);
extern void func_80078A58(s32);
extern void func_80035FE0(void);
extern void func_800375EC(void);
extern u8 g_pad_data;
void sys_Init(void) {
    u8 *base = &g_pad_data;
    irq_DisableInterrupts();
    func_80078C9C(base, 8, base + 0x24, 8);
    func_80078D38();
    func_80078A58(0);
    disp_Init();
    g_disp_enable = 0xFF;
    g_disp_fade = 0;
    func_80035FE0();
    func_800375EC();
    sys_InitSound();
}
INCLUDE_ASM("asm/funcs", func_80016A8C);
void sys_Panic(void) {
    func_80079208((s32)&g_str_overflow);
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
    func_80079208((s32)&g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    g_file_dma_flag = 1;
}
extern void func_8005B43C(void);
extern s32 func_8005B7C4(u32);
extern void func_80079120(u32, u32, s32);
extern void func_8005C4C0(u32, s32);
extern void func_8005C614(void);
void file_LoadSoundData(void) {
    s32 size;

    func_8005B43C();
    size = func_8005B7C4(0x801D8800);
    if (size >= 0xD01) {
        sys_Panic();
    }
    func_80079120(0x8010DB00, 0x801D8800, size);
    func_8005C4C0(0xFFF35300, 0);
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
    func_80079208((s32)&g_str_limit, 0x8010DB00);
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
    func_8007B33C(0);
}

INCLUDE_ASM("asm/funcs", func_80016E60);
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
INCLUDE_ASM("asm/funcs", cpu_set_move_command_and_dir_for_no_action_2);
/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action_2  |  189i  |  x2 size collision */
void gnd_disp_loop_ctrl(void) {
    u32 new_var;
    u8 sp18[8];
    u8 sp20[0x68];
    register s32 s0_var asm("s0");
    int new_var2;
    unsigned short s1_var;
    register s32 s2_var asm("s2");
    s32 mask;
    s2_var = (s32)(&D_800F33D8);
    if (g_disp_enable == 0xFF) {
        return;
    }
    s0_var = (s32)sp20;
    s1_var = 0xF0;
    mask = D_800A36AC & 1;
    mask = -mask;
    func_8007A694((u8 *)s0_var, 0, mask & 0xF0, 0x280, s1_var);
    new_var2 = 0;
    sp20[0x18] = new_var2;
    func_8007B9B0((u8 *)s0_var);
    D_800A374C = sp18;
    func_8007B844(sp18, 2);
    switch (g_disp_enable) {
    case 1:
    case 2:
        s2_var = (s32)func_8005D46C((u8 *)s2_var);
        if (g_disp_fade != new_var2) {
            s32 v0;
            s32 a0_temp;
            s0_var = new_var2;
            v0 = func_80079154();
            v0 &= 3;
            s1_var = v0 + 4;
            if (s1_var != new_var2) {
                a0_temp = s2_var;
                inner_loop:
                s0_var++;
                s2_var = (s32)func_8005D554((u8 *)a0_temp, g_disp_enable);
                if (s0_var >= s1_var) {
                    break;
                }
                a0_temp = s2_var;
                goto inner_loop;
            }
        } else if ((func_80079154() & 7) == new_var2) {
            func_8005D554((u8 *)s2_var, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)s2_var, new_var2);
        break;
    case 20:
    {
        u8 a2_val = D_800A38F8;
        register s32 a1_val asm("a1") = D_800A37A0;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        new_var = (u32)a0_val;
        if (((u32)a1_val) < new_var) {
            break;
        }
        div_result = s1_var / (a1_val + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == a1_val) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = new_var2;
            s1_var = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == a1_val) {
                s1_var |= 0x8000;
                if ((a1_val && a1_val) && a1_val) {
                }
            }
            func_80060414(s1_var, (u8 *)s2_var, new_var2);
        }
        break;
    }
    }
    func_8007B93C((u8 *)(D_800A374C + 4));
    new_var = new_var2;
    func_8007B33C(new_var);
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

extern s32 func_8007F0BC(s32 *, s32 *);
extern s32 func_8007E43C(s32);
s32 math_Distance3D(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 2;
    in[1] = (a0[1] - a1[1]) >> 2;
    in[2] = (a0[2] - a1[2]) >> 2;
    func_8007F0BC(in, out);
    return func_8007E43C(out[0] + out[1] + out[2]) << 2;
}
s32 math_Distance3D_16(s32 *a0, s32 *a1) {
    s32 in[3];
    s32 out[4];

    in[0] = (a0[0] - a1[0]) >> 4;
    in[1] = (a0[1] - a1[1]) >> 4;
    in[2] = (a0[2] - a1[2]) >> 4;
    func_8007F0BC(in, out);
    return func_8007E43C(out[0] + out[1] + out[2]) << 4;
}
INCLUDE_ASM("asm/funcs", func_80017848);
INCLUDE_ASM("asm/funcs", func_80017A44);
extern void func_80017A44(void *, u8 *);
s32 func_80017D84(void *a0) {
    register u8 *obj_base asm("a3");
    register void *saved_a0 asm("t0");
    s32 i;

    saved_a0 = a0;
    obj_base = &g_file_data_buf[0];
    for (i = 0; i < 8; i++) {
        if (*(s32 *)obj_base == 0) break;
        obj_base += 0x34;
    }
    if (i == 8) return -1;
    if (D_800A30E8 < i) {
        D_800A30E8 = i;
    }
    *(u16 *)(obj_base + 4) = *(u16 *)saved_a0;
    *(u32 *)obj_base = *(u32 *)((u8 *)saved_a0 + 4);
    {
        register u32 *ptr asm("v0");
        ptr = *(u32 **)((u8 *)saved_a0 + 0xC);
        {
            register s32 r0 asm("v1"), r1 asm("a0"), r2 asm("a1"), r3 asm("a2");
            r0 = ptr[0]; r1 = ptr[1]; r2 = ptr[2]; r3 = ptr[3];
            *(s32 *)(obj_base + 0x14) = r0;
            *(s32 *)(obj_base + 0x18) = r1;
            *(s32 *)(obj_base + 0x1C) = r2;
            *(s32 *)(obj_base + 0x20) = r3;
            r0 = ptr[4]; r1 = ptr[5]; r2 = ptr[6]; r3 = ptr[7];
            *(s32 *)(obj_base + 0x24) = r0;
            *(s32 *)(obj_base + 0x28) = r1;
            *(s32 *)(obj_base + 0x2C) = r2;
            *(s32 *)(obj_base + 0x30) = r3;
        }
        asm volatile("" ::: "memory");
    }
    {
        register s32 val1 asm("v1");
        register void *tmp_a0 asm("a0") = saved_a0;
        register u8 *tmp_a1 asm("a1");
        s32 val0;
        val0 = *(s16 *)((u8 *)saved_a0 + 2);
        *(s32 *)(obj_base + 8) = val0;
        asm volatile("" : "=r"(saved_a0) : "0"(saved_a0));
        val1 = *(volatile s32 *)((u8 *)saved_a0 + 0x10);
        val0 = *(s16 *)(obj_base + 4);
        asm volatile("move %0, %1
	sh $0, 6(%1)" : "=r"(tmp_a1) : "r"(obj_base), "r"(val1), "r"(val0));
        val0 <<= 6;
        *(s32 *)(obj_base + 0xC) = val1;
        val1 += val0;
        *(s32 *)(obj_base + 0x10) = val1;
        func_80017A44(tmp_a0, tmp_a1);
        return i;
    }
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
