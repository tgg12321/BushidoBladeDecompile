/* The code6cac_b4_post.c functions that follow func_80036940, moved unchanged so
 * that func_80036140 and func_80036940 can sit in their own -G8 unit
 * (code6cac_b5.c) before them. The declarations below are the source file's own,
 * for the names this code uses. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern void VSync(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void func_800174F4(void);
extern void func_8003AAB0(void);
extern void snd_Quit(void);
extern void memcard_Quit(void);
extern void StopPAD(void);
extern void StopCallback(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern s32 CdPosToInt(s32);
extern void func_80036940(void);

s32 cdrom_IsIdle(void) {
    return D_80101E58.rec.unk02 == 0;
}
s32 cdrom_StartRead(s32 a0, s32 a1) {
    s32 reloaded;

    if (D_80101E58.rec.unk02 != 0) {
        return 0;
    }

    D_80101E58.rec.unk00 = a0;
    D_80101E58.rec.pair = g_cd_file_table[(s16)a0];
    D_80101E58.rec.unk1C = a1;
    D_80101E58.rec.unk08 = 0;
    D_80101E58.rec.unk02 = 2;
    reloaded = D_80101E58.rec.pair.b;
    D_80101E58.rec.unk3E = 0;
    D_80101E58.rec.unk18 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
s32 cdrom_StartReadAt(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    if (cdrom_StartRead(arg0, arg1) == 0) {
        return 0;
    }
    CdIntToPos(CdPosToInt((s32)&D_80101E58.rec.pair) + arg2, (s32)&D_80101E58.rec.pair);
    D_80101E58.rec.unk18 = arg3;
    return 1;
}
s32 func_80036EA8(s32 arg0, s32 arg1) {
    return (&D_8008F12C)[arg0] + arg1;
}
void cdrom_Pause(void) {
    CdReadyCallback(0);
    cdrom_SetMix(0, 0, 0, 0);
    CdFlush();
    CdControlF(9, 0);
    D_80101E58.rec.unk08 = 1;
    D_80101E58.rec.unk02 = 0xB;
    D_80101E58.unk04 = 0;
}
u32 cdrom_GetFileSize(s32 arg0) {
    return g_cd_file_table[arg0].b;
}
void game_FrameLoop(void) {
    u16 *p;
    func_8003AA78();
    p = &D_80101E58.rec.unk3E;
    while (1) {
        if (cdrom_IsIdle() != 0) {
            break;
        }
        func_8003AA48();
        func_80036940();
        if (D_800A3906 != 0) {
            func_8005C6D0();
        }
        func_800174F4();
        *p = *p + 2;
        VSync(2);
    }
    func_8003AAB0();
}
s32 cdrom_StartAudio(s32 arg0, s32 arg1) {
    if (D_80101E58.rec.unk02 != 0) {
        return 0;
    }

    D_80101E58.rec.unk00 = arg0;
    D_80101E58.rec.pair = g_cd_file_table[D_80101E58.rec.unk00];
    D_80101E58.rec.unk14 = CdPosToInt((s32)&g_cd_file_table[D_80101E58.rec.unk00]) +
                           ((u32)g_cd_file_table[D_80101E58.rec.unk00].b >> 11) - 0x96;

    if (arg1 < 0) {
        D_80101E58.rec.unk34 = 0;
        D_80101E58.rec.unk30 = 5;
    } else {
        D_80101E58.rec.unk34 = 1;
        D_80101E58.file = 1;
        D_80101E58.chan = arg1;
        CdControlB(0xD, &D_80101E58.file, 0);
        D_80101E58.rec.unk30 = 0xC8;
    }

    D_80101E58.rec.unk04 = 0;
    D_80101E58.rec.unk08 = 0;
    D_80101E58.rec.unk0A = 0;
    D_80101E58.rec.unk02 = 0x10;

    return 1;
}
s32 func_80037110(s32 arg0) {
    u8 *s0 = (u8 *)&D_8008F13C + (arg0 << 3);
    s32 v0;
    v0 = func_80036EA8(5, s0[0]);
    v0 = cdrom_StartAudio(v0, s0[1]);
    if (v0 != 0) {
        if (*(s32 *)(s0 + 4) != -1) {
            v0 = CdPosToInt((s32)&g_cd_file_table[D_80101E58.rec.unk00]);
            D_80101E58.rec.unk14 = v0 + *(s32 *)(s0 + 4);
        }
        return 1;
    }
    return 0;
}

s32 func_800371AC(void) {
    s32 ret = ((s32 (*)())func_80037110)();
    if (ret) {
        D_80101E58.rec.unk04 = 1;
        return 1;
    }
    return 0;
}
void func_800371E8(s16 arg0) {
    D_80101E58.rec.unk0A = arg0;
}
s32 func_800371F8(void) {
    extern s32 cdrom_StartAudio();

    if (((s32 (*)())cdrom_StartAudio)() != 0) {
        D_80101E58.rec.unk04 = 1;
        return 1;
    }
    return 0;
}
void func_80037234(void) {
    D_80101E58.rec.unk04 = 0;
    D_80101E58.rec.unk08 = 1;
}
void func_80037250(void) {
    D_80101E58.rec.unk04 = 0;
}
void func_80037260(void) {
    while (D_80101E58.rec.unk02 != 0x16) {
        func_8003AA48();
        func_80036940();
        VSync(2);
    }
}
void func_800372C0(void) {
    if (D_80101E58.rec.unk02 != 0) {
        cdrom_Pause();
    }
    game_FrameLoop();
}
s32 cdrom_ReadWait(s32 nbytes, s32 buf, s32 mode) {
    s32 v = nbytes;
    nbytes += 0x7FF;
    if (nbytes < 0) {
        nbytes = v + 0xFFE;
    }
    CdRead(nbytes >> 11, buf, mode);
    do {
        v = CdReadSync(1, 0);
        if (v > 0) {
            VSync(0);
        }
    } while (v > 0);
    return v;
}
extern void CdIntToPos(s32, s32);
/* Loads a PS-EXE from disc (renamed cdrom_LoadExec 2026-09-07; was
 * special_camera_get_rot_dir - nothing camera-related). Seeks to entry
 * D_8008F12C[6] (=156, MOVOVL.EXE) of g_cd_file_table, reads
 * one 2048-byte sector, copies the struct EXEC at +0x10 into the caller's
 * *dest, then seeks to the following sector and reads dest->t_size bytes to
 * dest->t_addr. The sole caller, sys_Exec, then Exec()s dest. Any failed read
 * restarts the whole sequence from the seek. */
void cdrom_LoadExec(EXEC *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 v0;
    s32 pos;
    s32 mode;

    mode = 0x80; /* CdlModeSpeed - double-speed transfer */
    index = func_80036EA8(6, 0);

    for (;;) {
        CdControl(2, (u8 *)&g_cd_file_table[index], 0);
        v0 = cdrom_ReadWait(0x800, (s32)sp_buf, mode);
        if (v0 != 0) continue;

        *dest = *(EXEC *)&sp_buf[0x10];

        pos = CdPosToInt((s32)&g_cd_file_table[index]);
        CdIntToPos(pos + 1, (s32)sp_buf2);
        CdControl(2, sp_buf2, 0);
        v0 = cdrom_ReadWait(dest->t_size, dest->t_addr, mode);
        if (v0 == 0) break;
    }
}
void sys_Exec(s32 a0, s32 *a1, s32 a2) {
    EXEC exec;
    VSync(0);
    SetDispMask(0);
    gpu_ResetGraphMode1();
    snd_Quit();
    memcard_Quit();
    ResetCallback();
    CdInit();
    cdrom_LoadExec(&exec);
    DrawSync(0);
    ResetGraph(0);
    StopPAD();
    StopCallback();
    exec.s_addr = a2;
    exec.s_size = 0;
    EnterCriticalSection();
    Exec(&exec, a0, a1);
    sys_Init();
    file_LoadSoundData();
    VSync(0);
    SetDispMask(1);
}
extern s32 func_800392B8(void);
extern void sys_Exec(s32, s32 *, s32);
void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    /* n.b.! needs to be 25-32 bytes (inclusive): target frame 0x48 - callee
       saves (6 regs @ 0x30-0x44 = 24) - outgoing args (16) = 32-byte locals
       region, but only sp[0..5] (24 bytes) are ever written (count=6 to the
       callee) and ALIGN8(24)+16+24 = 0x40 != 0x48 — the original provably
       declared a larger argv buffer than it fills; s32 [7] and [8] are
       byte-identical. Oversized-locals carve-out (owner ruling 2026-07-13),
       see .claude/rules/dead-vars-local-array.md. */
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&g_cd_file_table[v0];
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&g_cd_file_table[v0];
    sp[5] = a4;
    v0 = func_800392B8();
    sys_Exec(6, sp, v0 + 0x7FC);
}
