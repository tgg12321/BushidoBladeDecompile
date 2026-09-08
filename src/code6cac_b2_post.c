#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern s32 D_800F33D8;
extern u32 D_800A378C;
extern u32 D_80101E3C;
extern u32 D_80101E44;

/* Extern function declarations */
extern void func_80023CB4(s32, s32);
extern void func_800194F4(void);
extern void seq_Reset(void);
extern void func_8003A39C(void);
extern void VSync(s32);
extern void LoadImage(s32, s32);
extern void game_Cleanup(void);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern s32 D_800A3894;
extern s16 D_800A38C4;
extern s16 D_80101F32;
extern void obj_InitChars(void);
extern void obj_Reset(void);
extern void obj_InitTask(void);
extern void obj_InitPair(void);
extern void player_SetCharId(s32, s32);
extern void func_80021974(s32);
extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
extern s32 D_800A3894;
extern u8 D_80102781;


extern u8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 obj_InitTaskCamera(s32);
extern void *D_800A38B4;
extern s32 memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void func_800174F4(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 ratan2(s32, s32);

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void snd_Quit(void);
extern void func_80037774(void);
extern void StopPAD(void);
extern void StopCallback(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern s32 D_80106A50;
extern void SetPolyG4(u8 *p);
extern void AddPrim(u32 *a0, u32 *a1);
extern u8 D_800A377C;
extern u8 D_800A37A8;
extern u16 D_800A3904;
extern u8 D_800F65F8;
extern s32 g_anim_func_table;
extern s32 g_pad_data;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s16 D_80101ED6;
extern s32 g_file_disc_size;
/* Continuation of src/code6cac_b2.c (split for Phase B sec.15.1 rodata-cleanup -
 * replay_camera_rob_back_loose2 extracted to its own .c file, requiring this
 * file to be split around it to preserve text addresses). */
INCLUDE_ASM("asm/funcs", func_80035828);
void func_80035DC8(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    obj_InitAll();
    func_80077820((s32)0x80118800);
    D_800A3834 = 0x1B;
    gpu_DisableDisplay();
}
void func_80035E38(void) {
    D_800A36F1 = 1;
    func_8003553C();
    if (func_80038C70() != 0) {
        D_800A3834 = 8;
        D_800A36F1 = 2;
    }
}
s32 func_80035E88(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << i)) {
                result |= (one << j);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
s32 func_80035EDC(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << j)) {
                result |= (one << i);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
extern void CdMix(u8 *);
extern u8 D_800A3718;
extern s16 D_800A3854;
void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    D_800A3718 = (u8)arg0;
    D_800A3719 = (u8)arg1;
    D_800A371A = (u8)arg2;
    D_800A371B = (u8)arg3;
    CdMix(&D_800A3718);
    D_800A3854 = 0;
}
extern u8 D_800A36B8;
extern s16 D_800A3840;
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8 = (u8)arg1;
    D_800A36B9 = (u8)arg2;
    D_800A36BA = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36BB = (u8)arg4;
}
void snd_SerialMixOn(void) {
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x7F, 0x7F);
}
extern u8 D_800A31E4;
void cdrom_Init(void) {
    CdInit();
    CdSetDebug(0);
    cdrom_SetMix(0, 0, 0, 0);
    D_80101E60.unk02 = 0;
    if (D_800A31E4 == 0) {
        D_800A31E4 = 1;
    }
}
void func_80036034(void) {
    CdFlush();
    CdInit();
    VSync(4);
}
extern void CdGetSector(s32, s32);
extern s32 CdPosToInt(s32);
void func_80036064(u8 arg0) {
    s32 sp[4];
    if (arg0 == 1) {
        D_80101E98 = 0;
        if (D_80101E80 <= 0) {
            return;
        }
        CdGetSector((s32)sp, 3);
        {
            s32 v0 = CdPosToInt((s32)sp);
            if (v0 != D_80101EA0) {
                D_80101E80 = -2;
                goto do_stop;
            }
        }
        CdGetSector(D_80101E84, 0x200);
        D_80101E84 = D_80101E84 + 0x800;
        D_80101E80 = D_80101E80 - 1;
        D_80101EA0 = D_80101EA0 + 1;
        if (D_80101E80 == 0) {
            goto do_stop;
        }
        return;
    } else {
        D_80101E80 = -1;
    }
do_stop:
    CdReadyCallback(0);
    CdControlF(9, 0);
}
INCLUDE_ASM("asm/funcs", func_80036140);
/* kengo:MED  |  nm_special_cam/special_camera_set_win_cam  |  502i  |  -10 */
void func_80036940(void);
INCLUDE_ASM("asm/funcs", func_80036940);
/* kengo:HIGH  |  nm_special_cam/special_camera_Exec  |  274i */
s32 func_80036D88(void) {
    return D_80101E60.unk02 == 0;
}
s32 replay_camera_Init(s32 a0, s32 a1) {
    extern u8 SpecialCam;
    s32 sval;
    s32 reloaded;

    if (D_80101E60.unk02 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60.unk00 = a0;
    D_80101E60.pair = *(CamPair *)((u8 *)&SpecialCam + sval);
    D_80101E7C = a1;
    D_80101E60.unk08 = 0;
    D_80101E60.unk02 = 2;
    reloaded = D_80101E60.pair.b;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
/* kengo:HIGH  |  nm_replay_cam/replay_camera_Init  |  39i */
s32 func_80036E34(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    if (replay_camera_Init(arg0, arg1) == 0) {
        return 0;
    }
    CdIntToPos(CdPosToInt((s32)&D_80101E60.pair) + arg2, (s32)&D_80101E60.pair);
    D_80101E78 = arg3;
    return 1;
}
s32 func_80036EA8(s32 arg0, s32 arg1) {
    return (&D_8008F12C)[arg0] + arg1;
}
void game_FrameInit(void) {
    CdReadyCallback(0);
    cdrom_SetMix(0, 0, 0, 0);
    CdFlush();
    CdControlF(9, 0);
    D_80101E60.unk08 = 1;
    D_80101E60.unk02 = 0xB;
    D_80101E5C = 0;
}
u32 func_80036F28(s32 arg0) {
    return (&D_8008EC38)[arg0 * 2];
}
void game_FrameLoop(void) {
    u16 *s0;
    func_8003AA78();
    s0 = (u16 *)&D_80101E9E;
    while (1) {
        if (func_80036D88() != 0) {
            break;
        }
        func_8003AA48();
        func_80036940();
        if (D_800A3906 != 0) {
            func_8005C6D0();
        }
        func_800174F4();
        *s0 = *s0 + 2;
        VSync(2);
    }
    func_8003AAB0();
}
extern void CdControlB(s32, u8 *, s32);
s32 func_80036FD4(s32 arg0, s32 arg1) {
    s16 *s0 = &D_80101E60.unk02;

    if (*s0 != 0) {
        return 0;
    }

    {
        extern u8 SpecialCam;
        ReplayCamRec *rec = &D_80101E60;
        s32 idx;
        u8 *cam;
        CamPair *entry;

        rec->unk00 = arg0;
        /* FAKE: reading the index back out of the record (instead of reusing
         * arg0) exists to win a scheduler tie-break, not to compute anything
         * new.  sched.c rank_for_schedule() breaks equal-priority ties on
         * INSN_LUID, i.e. on original RTL order, so splitting the sign-extend
         * (this read-back -> sll16/sra16) from the scaling (idx * 8 -> sll3,
         * which combine fuses back into the sra as sra13 AT THE sll3's
         * position) and materialising the table base BETWEEN them is what puts
         * the three insns in target's sll / lui+addiu / sra order. */
        idx = rec->unk00;
        cam = &SpecialCam;
        entry = (CamPair *)(cam + idx * 8);
        rec->pair = *entry;
    }

    {
        extern u8 SpecialCam;
        D_80101E60.unk14 = CdPosToInt((s32)(&SpecialCam + D_80101E60.unk00 * 8)) + (*(u32 *)((u8 *)&D_8008EC38 + (D_80101E60.unk00 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E94 = 0;
        D_80101E90 = 5;
    } else {
        u8 *base = (u8 *)s0 - 0xA;
        D_80101E94 = 1;
        *base = 1;
        D_80101E59 = arg1;
        CdControlB(0xD, base, 0);
        D_80101E90 = 0xC8;
    }

    D_80101E60.unk04 = 0;
    D_80101E60.unk08 = 0;
    D_80101E60.unk0A = 0;
    D_80101E60.unk02 = 0x10;

    return 1;
}
s32 func_80037110(s32 arg0) {
    u8 *s0 = (u8 *)&D_8008F13C + (arg0 << 3);
    s32 v0;
    v0 = func_80036EA8(5, s0[0]);
    v0 = func_80036FD4(v0, s0[1]);
    if (v0 != 0) {
        if (*(s32 *)(s0 + 4) != -1) {
            v0 = CdPosToInt((s32)&SpecialCam + (s32)D_80101E60.unk00 * 8);
            D_80101E60.unk14 = v0 + *(s32 *)(s0 + 4);
        }
        return 1;
    }
    return 0;
}

s32 func_800371AC(void) {
    s32 ret = ((s32 (*)())func_80037110)();
    if (ret) {
        D_80101E60.unk04 = 1;
        return 1;
    }
    return 0;
}
void func_800371E8(s16 arg0) {
    D_80101E60.unk0A = arg0;
}
s32 func_800371F8(void) {
    extern s32 func_80036FD4();

    if (func_80036FD4() != 0) {
        D_80101E60.unk04 = 1;
        return 1;
    }
    return 0;
}
void func_80037234(void) {
    D_80101E60.unk04 = 0;
    D_80101E60.unk08 = 1;
}
void func_80037250(void) {
    D_80101E60.unk04 = 0;
}
void func_80037260(void) {
    while (D_80101E60.unk02 != 0x16) {
        func_8003AA48();
        func_80036940();
        VSync(2);
    }
}
void func_800372C0(void) {
    if (D_80101E60.unk02 != 0) {
        game_FrameInit();
    }
    game_FrameLoop();
}
s32 func_800372F4(s32 nbytes, s32 buf, s32 mode) {
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
/* The 0x3C-byte struct EXEC (PS-EXE header body) that sits 0x10 into the first
 * sector of a PS-EXE image on disc: pc0, gp0, t_addr, t_size, ... */
typedef struct { s32 rot[15]; } CamRot;
extern void CdControl(s32, s32, s32);
extern void CdIntToPos(s32, s32);
/* Loads a PS-EXE from disc (renamed cdrom_LoadExec 2026-09-07; was
 * special_camera_get_rot_dir - nothing camera-related). Seeks to entry
 * D_8008F12C[6] (=156, MOVOVL.EXE) of the SpecialCam CD-locator table, reads
 * one 2048-byte sector, copies the struct EXEC at +0x10 into the caller's
 * dest[], then seeks to the following sector and reads t_size (dest[3]) bytes to
 * t_addr (dest[2]). The sole caller, sys_Exec, then Exec()s dest. Any failed read
 * restarts the whole sequence from the seek. */
void cdrom_LoadExec(s32 *dest) {
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    s32 index;
    s32 cam_base;
    s32 v0;
    s32 mode;

    mode = 0x80; /* CdlModeSpeed - double-speed transfer */
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;

    for (;;) {
        CdControl(2, index + cam_base, 0);
        v0 = func_800372F4(0x800, (s32)sp_buf, mode);
        if (v0 != 0) continue;

        *(CamRot *)dest = *(CamRot *)&sp_buf[0x10];

        v0 = CdPosToInt(index + cam_base);
        CdIntToPos(v0 + 1, (s32)sp_buf2);
        CdControl(2, (s32)sp_buf2, 0);
        v0 = func_800372F4(dest[3], dest[2], mode);
        if (v0 == 0) break;
    }
}
/* kengo:MED  |  nm_special_cam/special_camera_get_rot_dir  |  66i  |  +6 9.1% */
void sys_Exec(s32 a0, s32 *a1, s32 a2) {
    s32 sp[16];
    VSync(0);
    SetDispMask(0);
    gpu_EnableDisplay();
    snd_Quit();
    func_80037774();
    ResetCallback();
    CdInit();
    cdrom_LoadExec(sp);
    DrawSync(0);
    ResetGraph(0);
    StopPAD();
    StopCallback();
    sp[8] = a2;
    sp[9] = 0;
    EnterCriticalSection();
    Exec(sp, a0, a1);
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
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    sys_Exec(6, sp, v0 + 0x7FC);
}
