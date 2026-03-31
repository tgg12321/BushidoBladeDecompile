#include "common.h"
#include "include_asm.h"

/* Forward declarations for called functions */
extern void func_8001945C(void);
extern void func_8007AE7C(s32);
extern void func_8007B2A0(s32);
extern void func_8007B4D0(void *, s32, s32, s32);
extern void func_8007B33C(s32);
extern void snd_PlaySystemSe(void);

/* Externs for globals */
extern u8 D_80106A73;
extern u8 D_80106A54;
extern u16 D_800A3710;
extern u32 D_80106A50;
extern u32 D_80106A5C;
extern u8 D_800A3716;
extern s32 D_800A38BC;
extern u32 D_800A30CC;
extern u8 D_800F6740[];
extern u8 D_800F7438;
extern u8 D_800F7450;
extern u32 D_800F5370;
extern u8 D_80010000;
extern u8 D_8001000C;

extern void func_80079208();
extern void func_800164F8(void);
extern s16 D_800973FC[];
extern s32 func_80083698(s32, s32, s32);
extern s32 func_800836C8(s32, s32, s32);
extern s32 bios_FileRead(s32, u8 *, s32);
extern void func_80078A18(s32);
extern void func_800836B8(s32);


extern u8 D_800A30E8;
extern u8 D_800A30D4;
extern u8 D_800FB524;
extern s16 D_800A38DC;
extern u8 D_800A389A;
extern s32 D_800A36AC;
extern u8 D_800A3788;
extern u8 *D_800A374C;
extern u32 D_800A38B4;
extern u32 D_80102794;
extern u8 D_800A31DA;
extern s16 D_800A3834;
extern s32 D_800A30DC;
extern u8 D_80010034;
extern u8 D_800A390D;
extern u8 D_800A3713;
extern u32 D_8008D090[];
extern u8 D_800F33D8;
extern u8 D_800A37A0;
extern u8 D_800A38F8;
extern s32 D_800A37C0;
extern u8 D_800A37A8[];
extern s32 func_80036EA8(s32, s32);
extern void replay_camera_Init(s32, s32);
extern void func_80036F40(void);
extern void func_8007BC08(u8 *);
extern void func_8007B600(u8 *, u8 *);
extern void func_800828CC(s32);
extern void func_8007B844(u8 *, s32);
extern void special_camera_Exec(void);
extern void func_8005C6D0(void);
extern void func_80019568(s32);
extern void func_8005C8A8(s32, s32, u32, s32);
extern void func_8005C650(s32, s32, s32);
extern void func_8007B9B0(u8 *);
extern void func_8007B93C(u8 *);
extern void func_80078BA8(u32);
extern s32 func_80078B04(u32);
extern s32 func_80079154(void);
extern void func_800372C0(void);
extern void func_80083794(void);
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
    func_800836C8(fd, sector << 11, 0);
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
    s16 cos_val = D_800973FC[(v1 + 0x400) & 0xFFF];
    s16 sin_val = D_800973FC[v1 & 0xFFF];
    return (cos_val * 320) / sin_val;
}
void disp_SetFramebufferMode(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 i;
    u8 *ptr;
    s32 offset;

    i = 0;
    ptr = (u8 *)&D_800F7438;
    offset = 0;

    for (; i < 2; i++) {
        *(vu8 *)((u8 *)&D_800F7450 + offset) = a0;
        *(vu8 *)(ptr + 0x19) = a1;
        *(vu8 *)(ptr + 0x1A) = a2;
        *(vu8 *)(ptr + 0x1B) = a3;
        ptr += 0x4090;
        offset += 0x4090;
    }
}

/* --- Decompiled functions --- */

u32 file_GetFlag0(void) {
    return D_80106A73 & 1;
}

u32 file_GetFlag1(void) {
    return (D_80106A73 >> 1) & 1;
}

u32 file_GetFlag2(void) {
    return (D_80106A73 >> 2) & 1;
}

INCLUDE_ASM("asm/funcs", func_800167EC);

void gpu_EnableDisplay(void) {
    func_8007AE7C(1);
}

void gpu_InitDisplay(void) {
    func_8007B2A0(0);
    func_8007AE7C(1);
    func_8007B4D0(&D_800A30CC, 0, 0, 0);
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
    base = &D_800F7438;
    func_8007A694(base, 0, 0, 0x280, 0xF0);
    func_8007A694(base + 0x4090, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x5C, 0, 0xF0, 0x280, 0xF0);
    func_8007A74C(base + 0x40EC, 0, 0, 0x280, 0xF0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8007B4D0(&D_800A30CC, 0, 0, 0);
    func_8007B33C(0);
}
extern void irq_DisableInterrupts(void);
extern void func_80078C9C(u8 *, s32, u8 *, s32);
extern void func_80078D38(void);
extern void func_80078A58(s32);
extern void func_80035FE0(void);
extern void func_800375EC(void);
extern u8 D_800FF580;
extern u8 D_800A3768;
extern u8 D_800A36A8;
void sys_Init(void) {
    u8 *base = &D_800FF580;
    irq_DisableInterrupts();
    func_80078C9C(base, 8, base + 0x24, 8);
    func_80078D38();
    func_80078A58(0);
    disp_Init();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FE0();
    func_800375EC();
    sys_InitSound();
}
INCLUDE_ASM("asm/funcs", func_80016A8C);
void sys_Panic(void) {
    func_80079208((s32)&D_80010000);
    while (1) {
        func_800164F8();
    }
}
void file_ResetDmaFlag(void) {
    D_800A3716 = 0;
}
extern s32 func_80060CB8(u32, u32);
void file_LoadOverlay(void) {
    s32 size;

    if (D_800A3716 != 0) {
        return;
    }
    size = func_80060CB8(0x801D8800, 0x8010E800);
    func_80079208((s32)&D_8001000C, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    D_800A3716 = 1;
}
extern void func_8005B43C(void);
extern s32 func_8005B7C4(u32);
extern void func_80079120(u32, u32, s32);
extern void func_8005C4C0(u32, s32);
extern void func_8005C614(void);
extern u8 D_800A3906;
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
extern u8 D_80010028;
extern u32 D_800A3770;
extern u32 D_800A3774;
extern u32 D_800A3798;
extern u8 D_800A36F9;
extern u8 D_800A3690;
extern u8 D_800A3744;
extern u8 D_800A3745;
extern u8 D_800A3746;
extern void func_80020D70(void);
extern void game_Init(void);
extern u8 D_800A36F1;
extern u16 D_800A38C6;
extern u8 D_800A36B0;
extern u8 D_800A3928;
extern void func_80019534(void);
extern void func_8003D2C4(void);
extern void func_8001C444(void);
void sys_GameInit(void) {
    func_80079208((s32)&D_80010028, 0x8010DB00);
    func_800167EC();
    func_80020D70();
    D_800A3770 = 0x801D8800;
    D_800A3774 = 0x801EBC00;
    D_800A3798 = 0x13400;
    D_800A3716 = 0;
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
    D_800A38BC = a0;
}
s32 rng_Next(void) {
    s32 seed = D_800A38BC;
    s32 result = seed * 5497 + 0x7FA9;
    seed = (seed >> 16) ^ result;
    D_800A38BC = seed;
    return seed & 0x7FFF;
}
INCLUDE_ASM("asm/funcs", cpu_set_move_command_and_dir_for_no_action_2);
/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action_2  |  189i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", gnd_disp_loop_ctrl);
/* kengo:MED  |  hi_gnd/gnd_disp_loop_ctrl  |  140i  |  +4 */
void obj_ClearAll(void) {
    s32 i;
    for (i = 0x16C; i >= 0; i -= 0x34) {
        *(s32 *)(D_800F6740 + i) = 0;
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
INCLUDE_ASM("asm/funcs", func_80017D84);
void obj_Clear(s32 a0) {
    *(s32 *)(D_800F6740 + a0 * 52) = 0;
}
void obj_UpdatePosition(s32 a0, s32 a1) {
    u8 *ptr = D_800F6740 + a0 * 52;
    s32 c = *(s32 *)(ptr + 0xC) + a1;
    *(s32 *)(ptr + 0xC) = c;
    *(s32 *)(ptr + 0x10) = c + (*(s16 *)(ptr + 4) << 6);
}
void obj_AddValue(s32 a0, s32 a1) {
    s32 *ptr = (s32 *)(D_800F6740 + a0 * 52);
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
