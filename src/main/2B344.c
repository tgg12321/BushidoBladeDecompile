/* 92 game functions, among them bitstream_ReadBits, gpu_SetDrawMoveArray and
 * math_AlignUp4. .text 0x8003AB44 (ROM 0x2B344). Start boundary: LEGACY (a
 * tooling split); one object by the per-file gp model (Q65 merge group) and the
 * rodata-align section 11 merge. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include <psxsdk/inline_o.h>
#include <psxsdk/gtemac.h>

extern u8 g_comb_handshake_step;
extern s32 D_800A37D8;

s32 comb_HandshakeStep(void) {
    D_800A37B8++;
    switch (g_comb_handshake_step) {
    case 0:
        g_comb_handshake_step = 1;
        break;
    case 1:
        comb_ReadCtsSetRts();
        D_800A37D8 = 0;
        if (D_800A38A0 == 0) {
            SetDispMask(1);
            g_comb_handshake_step = 2;
        } else {
            g_comb_handshake_step = 3;
        }
        break;
    case 2:
        if (g_pad_state.pressed & 0x10) {
            comb_ResetClose();
            return -1;
        }
        if (_comb_control(3, 1, 0) != 0) {
            _comb_control(3, 0, 0);
            g_comb_handshake_step = 4;
        }
        break;
    case 3:
        if (_comb_control(3, 1, 0) == 0) {
            _comb_control(3, 0, 0);
            g_comb_handshake_step = 4;
            break;
        }
        D_800A37D8++;
        if (D_800A37D8 < 4) {
            break;
        }
        comb_ResetClose();
        return -1;
    case 4:
        SetDispMask(0);
        /* fall through */
    case 5:
    case 6:
        g_comb_handshake_step++;
        break;
    case 7:
        D_800A3916 = 1;
        comb_EnableEvents();
        return 1;
    }
    return 0;
}

s32 comb_Connect(void) {
    s32 temp_s0;

    func_80077AE0();
    SetDispMask(0);
    D_800A37B8 = 0;
    g_comb_handshake_step = 0;
    D_800A38D0 = 0;
    D_800A3908 = 0;
    D_800A38FC = 0;
    comb_Init();
    do {
        pad_Update(1);
        snd_FlushSoundQueue();
        temp_s0 = comb_HandshakeStep();
        VSync(2);
    } while (temp_s0 == 0);
    func_80077B00();
    pad_ResetState();
    D_800A37C4 = bits_ExtractMask3F83F8(D_80106A50.unk_00);
    comb_ExchangeEmptyPad();
    VSync(1);
    comb_ExchangeEmptyPad();
    D_800A38E4 = bits_DepositMask3F83F8(D_800A36C6);
    D_800A37C4 = func_80019488();
    VSync(1);
    comb_ExchangeEmptyPad();
    VSync(1);
    comb_ExchangeEmptyPad();
    VSync(1);
    comb_ExchangeEmptyPad();
    func_800194C0(D_800A36C6);
    D_800A37C4 = rand();
    VSync(1);
    comb_ExchangeEmptyPad();
    VSync(1);
    comb_ExchangeEmptyPad();
    VSync(1);
    comb_ExchangeEmptyPad();
    {
        s32 var_v0;
        if (D_800A38A0 == 0) {
            var_v0 = (u16)D_800A37C4;
        } else {
            var_v0 = D_800A36C6;
        }
        D_800A3904 = var_v0;
    }
    gpu_HideResetAndClear();
    gpu_SetDispMaskOn();
    ResetRCnt(0xF2000001);
    return temp_s0;
}

void func_8003AE5C(u8 *arg0) {
    s32 *addr = (s32 *)0x80190800;
    s32 result = -1;
    s32 var_2 = 0;

    do {
        s32 var_3 = *arg0++;
        switch (var_3) {
        case 1:
            result = *arg0;
        case 0:
        case 20:
            var_2 = 1;
            break;
        case 16:
            arg0 += 5;
            break;
        case 17:
            arg0 += 3;
            break;
        case 3:
            arg0 += 2;
            break;
        case 19:
            arg0 += 4;
            break;
        case 2:
            arg0 += 0x1D;
            break;
        }
    } while (!var_2);

    if (result >= 0) {
        D_800A37A8[D_800A37A0] = D_800A36A4;
        gpu_ResetGraphMode1();
        motion_InvalidateMarkedNdataPackAndClearSlot1Id();
        func_800602AC(result, addr);
    }
}

void func_8003AF40(s32 arg0) {
    if (D_80102778.unk_4[2 + arg0] == 0xFF) {
        D_80102778.unk_4[2 + arg0] = (&D_80102778.unk_4[2])[(u32)arg0 < 1u];
    }
    fighter_Init(arg0, (s8)D_80102778.unk_4[4 + arg0],
                 (s8)D_80102778.unk_4[arg0], (s8)D_80102778.unk_4[2 + arg0], 0);
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
        arg0, D_8008D578[(s8)D_80102778.unk_4[arg0]], (s32)0x80190800);
}

void func_8003AFFC(void) {
    s32 addr = (s32)0x80190800;
    s32 i;

    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    ResetEntrySelectionState();

    for (i = 0; i < 2; i++) {
        resource_RequestEntryWithLinkedEntriesAndLatchCategory(
            D_80101EC8[i].unk_12);
        resource_SetPairSelectionSlotWithBoundChecks(
            i, D_8008E5CC[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
        if (D_80101EC8[i].unk_14 != -1) {
            resource_RequestEntryWithLinkedEntriesAndLatchCategory(
                D_8008EB80[D_80101EC8[i].unk_14]);
            if (D_80101EC8[i].unk_14 == 14) {
                resource_RequestEntryWithLinkedEntriesAndLatchCategory(
                    D_8008EB80[14] + 3);
            }
        }
    }

    resource_RankRequestedEntriesAndReloadOnChange(addr);
}

void func_8003B10C(s32 arg0) {
    s32 addr = (s32)0x80190800;

    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    ResetEntrySelectionState();

    resource_RequestEntryWithLinkedEntriesAndLatchCategory(
        D_80101EC8[arg0].unk_12);

    if (D_800A38DC == 5) {
        resource_SetPairSelectionSlotWithBoundChecks(
            arg0, D_8008E6A4[D_80101EC8[arg0].unk_0A][D_80101EC8[arg0].unk_0E]);
    } else {
        resource_SetPairSelectionSlotWithBoundChecks(
            arg0, D_8008E5CC[D_80101EC8[arg0].unk_0A][D_80101EC8[arg0].unk_0E]);
    }
    resource_RankRequestedEntriesAndReloadOnChange(addr);
}

void fight_InitCommandStream(s32 arg0) {
    D_80102778.unk_4[4] = 0;
    D_80102778.unk_4[5] = 1;
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = 1;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = D_800900EC[arg0];
    eff_Init();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = D_8008D538[(s8)D_80102778.unk_4[0]];
}

void func_8003B2C8(void) {
    D_800A3836 = D_80102778.unk_4[0];
    {
        u8 *base = D_800A3894;
        u8 v1 = D_800A376A;
        u8 v0 = *base;
        D_800A36C8 = v1;
        D_800A376A = 0;
        D_80102778.unk_4[0] = v0;
    }
    slot_RequestTimGroupsAndMarkReloadOnChange(0, 0);
}

void func_8003B328(void) {
    u8 v_277C = D_80102778.unk_4[0];
    u8 v_376A = D_800A376A;
    u8 v_3836 = D_800A3836;
    u8 v_36C8 = D_800A36C8;
    D_800A3836 = 0xFF;
    D_800A3915 = v_277C;
    D_800A36F4 = v_376A;
    D_80102778.unk_4[0] = v_3836;
    D_800A376A = v_36C8;
    slot_RequestTimGroupsAndMarkReloadOnChange(0, v_36C8);
    fighter_ResetBoneGroupHitState(D_80101EC8);
}

s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = D_8008D538[(s8)D_80102778.unk_4[0]];
    a1 = D_8008D9EC[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    D_80102778.unk_4[1] = a1 != 0 ? 0xE : 0x1D;
    D_80102778.unk_4[3] = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102778.unk_A[1] = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}

s32 func_8003B484(u8 *arg0) {
    D_800A3712 = 1;
    D_80102778.unk_4[1] = arg0[0];
    D_80102778.unk_4[3] = arg0[1];
    D_80102778.unk_A[1] = arg0[2];
    func_8003AF40(1);
    func_8003AFFC();
    return 3;
}

void func_8003B4DC(void) {
    D_800A3712 = 1;
    D_80102778.unk_4[2] = 0;
    D_80102778.unk_4[1] = 0x1F;
    D_80102778.unk_4[3] = 0;
    func_8003AF40(0);
    func_8003AF40(1);
    func_8003AFFC();
}

void func_8003B534(s32 a0) {
    D_800A37B0 = a0;
    D_800A3834 = 6;
    D_800A3878 = D_800A3894 + (a0 * 4 + 1);
}

void func_8003B56C(s32 arg0) {
    D_800A390C = arg0;
    D_800A3834 = 6;
    D_800A3878 = D_800A385C + (arg0 * 4 - 4);
}

void fight_RunCommandStream(void) {
    s32 var_0;
    u8 *var_1;

    snd_ResetVoicesAndQueue();
    var_0 = 0;
    var_1 = &D_80102778.unk_4[1];

    do {
        u8 *ptr = D_800A3844;
        s32 var_3;
        D_800A3844 = ptr + 1;
        var_3 = *ptr;

        switch (var_3) {
        case 16: {
            s32 ret = func_8003B3A4(D_800A3844);
            D_800A3844 += ret;
            break;
        }

        case 17: {
            s32 ret = func_8003B484(D_800A3844);
            D_800A3844 += ret;
            break;
        }

        case 3: {
            u8 *p = D_800A3844;
            u8 byte0;
            D_800A3844 = p + 1;
            byte0 = p[0];
            D_800A3844 = p + 2;
            var_1[0] = byte0;
            var_1[2] = p[1];
            if ((s8)byte0 == D_800A3915) {
                slot_RequestTimGroupsAndMarkReloadOnChange(1, D_800A36F4);
            }
            func_8003AF40(1);
            func_8003AFFC();
            break;
        }

        case 1: {
            u8 byte;
            u8 *p = D_800A3844;
            u8 counter = D_800A37A0;
            D_800A3844 = p + 1;
            byte = *p;
            D_800A37A0 = counter + 1;
            D_800A36A4 = byte;
            D_800A37A8[counter] = byte;
            D_800A3834 = 22;
            var_0 = 1;
            fighter_ResetBoneGroupHitState(D_80101EC8);
            D_800A3907 = 0;
            break;
        }

        case 0:
            if (D_800A380C == 0) {
                D_800A38A4 = (D_8008D9EC[D_80101EC8[0].unk_0A] != 0);
                D_800A3834 = 18;
            } else {
                D_800A3834 = 10;
            }
            var_0 = 1;
            break;

        case 19:
            D_800A3834 = 6;
            D_800A3878 = D_800A3844;
            D_800A3844 += 4;
            var_0 = 1;
            break;

        case 2:
            D_800A3894 = D_800A3844;
            D_800A3844 += 0x1D;
            func_8003B2C8();
            func_8003AF40(0);
            func_8003B3A4(D_800A3894 + 1);
        case 18:
            D_800A3834 = 0;
            D_800A3907++;
            var_0 = 1;
            break;

        case 20:
            D_800A385C = D_800A3844;
            D_800A3844 += 12;
            var_0 = 1;
            func_8003B4DC();
            func_8003B56C(1);
            D_800A38BA = 0;
            break;
        }
    } while (!var_0);
}

void func_8003B870(void) {
    slot_RequestTimGroupsAndMarkReloadOnChange(0, D_800A376A);
    slot_RequestTimGroupsAndMarkReloadOnChange(1, 0);
    snd_ResetVoicesAndQueue();
    gpu_HideResetAndClear();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    gpu_SetDispMaskOn();
}

void func_8003B8E4(void) {
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 < 3) {
        g_prim_buf_cursor += func_80060544((s32)g_prim_buf_cursor, 1) / 4;
    }
    if (D_800A37B8 == 3) {
        DrawSync(0);
        func_8003AE5C(D_800A3844);
        D_800A37C0 = 500;
        D_800A38F8 = 0;
        D_800A3768 = DISP_ACTIVE;
        fight_ResetGraphAndReloadResourcesOnCachedKeyChange();
        fight_RunCommandStream();
        gpu_SetDrawEnvBg(1, 0, 0, 0);
        D_800A390D = 1;
    }
}

/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
u16 D_800A37C4;
s32 D_800A37D8;
u8 g_comb_handshake_step;
s32 D_800A38D0;
s32 D_800A38FC;
s32 D_800A3908;
u8 D_800A3916;

/* Merged section (Q65: one original file). */
extern DR_TPAGE g_font_tpage[2]; /* one per frame parity (g_font_buf_index) */

extern Unk800A4750Rec D_800A4750[];
extern Unk800A6690Rec D_800A6690[];
extern s16 D_800A7FE0[32][32];
extern u16 D_800A87E0[];
extern u8 g_gpu_cell_draw_attributes[];

extern void func_8003AF40(s32);

extern void gte_ReadFarColor(u8 *);

extern s8 D_8008EA70[][2];

extern void func_8003AFFC(void);

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 var_4;
    s32 v0;
    u8 flags;

    var_4 = 0x80190800;
    snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
    eff_StopCommands();
    if (D_800A3768 != DISP_ACTIVE)
        gpu_HideResetAndClear();
    if (D_800A3768 != DISP_DISABLED)
        gpu_SetDispMaskOn();
    gpu_DrawDisplayModeOnLocalOtAndSync();
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndClearSlot1Id();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    if (D_800A3878[3] & 0x80) {
        motion_InvalidateMarkedNdataPackAndResetLoadCaches();
        var_4 = 0x80118800;
    }
    {
        u8 var_7 = D_800A3878[3];
        if (var_7 & 0x30) {
            saved_first = D_80101EC8[0].unk_12;
            saved_44c = D_80101EC8[1].unk_12;
            if (var_7 & 0x10)
                D_80101EC8[0].unk_12 = 0x32;
            if (D_800A3878[3] & 0x20)
                D_80101EC8[1].unk_12 = 0x32;
            func_8003AFFC();
            D_80101EC8[0].unk_12 = saved_first;
            D_80101EC8[1].unk_12 = saved_44c;
        }
    }
    if (D_800A3878[3] & 0x1)
        a3_arg = D_80101EC8[0].unk_12;
    else
        a3_arg = -1;
    if (D_800A3878[3] & 0x2)
        a0_arg = D_80101EC8[1].unk_12;
    else
        a0_arg = -1;
    flags = D_800A3878[3];
    if (flags & 0x10)
        a3_arg = 0x32;
    if (flags & 0x20)
        a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, D_800A3878[0], 0, a3_arg, a0_arg, -1, -1, var_4);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    if (D_800A3878[3] & 0x40)
        resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
    if ((s8)D_80102778.unk_4[1] == 0xE || (s8)D_80102778.unk_4[1] == 0x1D) {
        gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
            D_800A37B4, D_800A37B5, D_800A37B6);
    }
    fight_WaitForCounterThenClearAndEnableDisplay();
    D_800A3768 = DISP_DISABLED;
    D_800A36A8 = 0;
    snd_SerialMixOn();
    v0 = cdrom_GetFileIndex(5, D_800A3878[1]);
    cdrom_StartAudio(v0, D_800A3878[2]);
    cdrom_WaitAudioStarted();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_SetDispMaskOn();
}

void func_8003BCB4(void) {
    D_800A37B8++;

    if (func_80054F68() != 0 && (g_pad_state.pressed & 0x400040) == 0) {
        return;
    }
    cdrom_PauseAndWait();
    func_800548DC();
    if (D_800A38DC != 0) {
        return;
    }
    if (D_800A3894 != 0) {
        D_800A3834 = 0;
        switch (D_800A37B0) {
        case 1:
        case 2:
        case 4:
        case 5:
            D_800A3907++;
            return;
        case 3:
            func_8003AF40(0);
            func_8003AFFC();
            /* fall through */
        case 6:
            D_800A3894 = 0;
            break;
        default:
            return;
        }
    } else if (D_800A385C != 0) {
        s32 val = D_800A390C;

        if (val == 1) {
            D_800A3834 = 0;
            return;
        }
        if (val == 0 || val >= 4) {
            return;
        }
        D_800A385C = 0;
    }
    fight_RunCommandStream();
}

void func_8003BE10(void) {
    gpu_ResetGraphMode1();
    gpu_HideResetAndClear();
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
    slot_ReleaseObjectAndResources(0);
    slot_ReleaseObjectAndResources(1);
    eff_ClearInitFlag();
    snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb();
    InitScrollingSpriteDisplay((s32)0x80118800);
    snd_SerialMixOn();
    {
        s32 v0 = cdrom_GetFileIndex(5, 0x20);
        cdrom_StartAudio(v0, 4);
    }
    cdrom_WaitAudioStarted();
    D_800A3834 = 0xB;
    gpu_SetDispMaskOn();
}

void func_8003BEA8(void) {
    s32 changed = 0;
    s32 go;
    s32 old;
    s32 bit;

    go = StepScrollingSpriteDisplay() != 0;
    if (g_pad_state.pressed & 0x40) {
        go = 1;
    }
    if (go) {
        cdrom_PauseAndWait();
        old = D_80106A50.unk_00;
        if ((old | D_800A37A4) != old) {
            D_80106A50.unk_00 = old | D_800A37A4;
            changed = 1;
        }
        if (D_80101EC8[0].unk_0A == 0 || D_80101EC8[0].unk_0A == 1 ||
            D_80101EC8[0].unk_0A == 2 || D_80101EC8[0].unk_0A == 0xC ||
            D_80101EC8[0].unk_0A == 0xD || D_80101EC8[0].unk_0A == 0xE) {
            bit = D_80101EC8[0].unk_0A;
            if (bit >= 0xC) {
                bit -= 9;
            }
            if (!((D_80106A50.unk_04 >> bit) & 1)) {
                D_80106A50.unk_04 |= 1 << bit;
                changed = 1;
            }
        }
        if (changed) {
            D_800A3834 = 0x1A;
        } else {
            D_800A3834 = 8;
        }
    }
}

void func_8003BFC4(void) {
    s32 v;
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
    slot_ReleaseObjectAndResources(0);
    slot_ReleaseObjectAndResources(1);
    eff_ClearInitFlag();
    v = (s32)arena_GetBase();
    sys_ExecMovieOverlay(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    func_80046B44();
    D_800A3834 = 8;
}

extern void func_8003B10C(s32);

void func_8003C040(void) {
    s32 var_1;
    s8 *p;
    gpu_HideResetAndClear();
    gpu_ResetGraphMode1();
    motion_InvalidateMarkedNdataPackAndResetLoadCaches();
    if (D_800A38A4 == 4 || D_800A38A4 == 5) {
        eff_ClearInitFlag();
    }
    if (D_800A38A4 == 6) {
        D_80102778.unk_4[0] = 8;
        D_80102778.unk_4[2] = 6;
        func_8003AF40(0);
        func_8003B10C(0);
    } else if (D_800A38A4 == 7) {
        D_80102778.unk_4[0] = 0x16;
        D_80102778.unk_4[2] = 7;
        func_8003AF40(0);
        func_8003B10C(0);
    } else if (D_800A38A4 == 8) {
        D_80102778.unk_4[0] = 0x1E;
        D_80102778.unk_4[2] = 0;
        func_8003AF40(0);
        func_8003B10C(0);
    } else if (D_800A38A4 == 9) {
        D_80102778.unk_4[0] = 0x20;
        D_80102778.unk_4[2] = 0;
        func_8003AF40(0);
        func_8003B10C(0);
    }
    if (D_800A38A4 != 9) {
        var_1 = D_800A38A4;
    } else {
        var_1 = 8;
    }
    func_8005FBC8(var_1, (u8 *)0x80118800);
    if ((D_800A38A4 == 4 && D_8008D9EC[D_80101EC8[0].unk_0A] != 0) ||
        (D_800A38A4 == 5 && D_8008D9EC[D_80101EC8[0].unk_0A] == 0)) {
        D_80102778.unk_4[0] = D_80102778.unk_4[1];
        D_80102778.unk_4[2] = D_80102778.unk_4[3];
        func_8003AF40(0);
        func_8003B10C(0);
    }
    func_80054884(0x16, D_8009016C[D_800A38A4], 0, D_80101EC8[0].unk_12, -1, -1,
                  -1, (s32)0x80118800);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    eff_StopCommands();
    p = D_8008EA70[D_800A38A4];
    if (p[0] >= 0) {
        snd_SerialMixOn();
        cdrom_StartAudio(cdrom_GetFileIndex(5, p[0]), (u8)p[1]);
        cdrom_WaitAudioStarted();
    }
    D_800A37B8 = 0;
    D_800A3834 = 0x13;
    gpu_SetDispMaskOn();
}

void func_8003C2C0(void) {
    D_800A37B8 = D_800A37B8 + 1;
    g_prim_buf_cursor +=
        gpu_DrawImageAndAdvanceSplitWipe((s32)g_prim_buf_cursor, 1);
    if (func_80054F68() == 0 || (g_pad_state.pressed & 0x400040) != 0) {
        if ((u32)(D_800A38A4 - 6) < 2u && D_800A3781 != 0) {
            D_800A38A4 = D_8008D9EC[D_80101EC8[0].unk_0A] != 0 ? 9 : 8;
            D_800A3834 = 0x12;
        } else {
            if ((u32)(D_800A38A4 - 4) < 2u) {
                D_800A3834 = 0x18;
            } else if ((u32)(D_800A38A4 - 6) < 2u) {
                D_800A3834 = 0x1A;
            } else if ((u32)(D_800A38A4 - 8) < 2u) {
                D_800A3834 = 0x1A;
            } else if (D_800A38A4 < 2u) {
                D_800A3834 = 0xA;
            } else {
                D_800A3834 = 8;
            }
        }
    }
    if (D_800A3834 != 0x13) {
        cdrom_PauseAndWait();
        if (D_800A3834 != 0x12) {
            func_800548DC();
        }
    }
}

void fight_TallyMatchResultsAndInitPresentation(void) {
    s32 counts[3];
    s32 i;
    s32 j;

    counts[2] = 0;
    counts[1] = 0;
    counts[0] = 0;
    for (i = 0; i < g_fight_match_count; i++) {
        counts[g_fight_match_results[i]]++;
    }
    if (counts[0] != counts[1]) {
        D_800A382D = counts[0] < counts[1];
    } else {
        counts[1] = 0;
        counts[0] = 0;
        for (i = 0; i < g_fight_match_count; i++) {
            for (j = 0; j < 2; j++) {
                counts[j] += g_fight_match_scores[i][j];
            }
        }
        if (counts[0] != counts[1]) {
            D_800A382D = counts[0] < counts[1];
        } else {
            D_800A382D = 2;
        }
    }
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x15;
}

void fight_UpdateMatchTallyPresentation(void) {

    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A382D == 2) {
        if (D_800A37B8 == 0x1E) {
            snd_QueueSound(0xA4, 0x7F, 0x7F);
        }
    } else {
        if (D_800A37B8 == 0x1E) {
            snd_QueueSound(
                D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0 ? 0xA8 : 0xA7,
                0x7F, 0x7F);
        }
        if (D_800A37B8 == 0x43) {
            snd_QueueSound(0xA9, 0x7F, 0x7F);
        }
    }
    g_prim_buf_cursor +=
        func_8005E54C(D_800A3784, (s32)g_prim_buf_cursor, 1) / 4;
    if ((g_pad_state.pressed & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        if (D_800A382D == 2) {
            D_800A3834 = 0x18;
        } else {
            cdrom_PauseAndWait();
            D_800A38A4 = D_8008D9EC[D_80101EC8[D_800A382D].unk_0A] != 0 ? 5 : 4;
            D_800A3834 = 0x12;
        }
    }
}

/*
 * Copies the three clock records (D_80106A50.times, frame counts at 30 fps)
 * into the record at func_80077D00(): minutes (/1800), seconds ((/30) % 60),
 * hundredths ((% 30) * 100 / 30) and the record's unk_0 byte at +0x21 + i*4;
 * then three bytes from fight_SplitFrameCounterIntoMinutesSecondsHundredths at
 * +0x2D..+0x2F and the stage at +0x30.
 */
void func_8003C714(void) {
    Unk8001CD68Rec buf;
    Unk8009BD24Block *s0;
    s32 i;
    s32 a, b, c, v;
    TimeRec *base;
    TimeRec *src;

    s0 = func_80077D00();
    cdrom_PauseAndWait();
    gpu_HideResetAndClear();
    func_80060758();
    i = 0;
    base = D_80106A50.times;
    do {
        src = &base[i];
        a = src->unk_4;
        a = a / 1800;
        s0->unk21[i].unk0 = a;
        b = src->unk_4;
        b = b / 30;
        b = b % 60;
        s0->unk21[i].unk1 = b;
        c = src->unk_4;
        c = c % 30;
        c = c * 100;
        c = c / 30;
        s0->unk21[i].unk2 = c;
        v = src->unk_0;
        s0->unk21[i].unk3 = v;
        i += 1;
        if (i >= 3) {
            break;
        }
    } while (1);
    fight_SplitFrameCounterIntoMinutesSecondsHundredths(&buf);
    s0->unk2D[0] = buf.unk_0;
    s0->unk2D[1] = buf.unk_2;
    s0->unk2D[2] = buf.unk_3;
    s0->unk30 = D_80101EC8[0].unk_0A;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_SetDispMaskOn();
}

void func_8003C8B4(void) {
    D_800A37B8 = D_800A37B8 + 1;
    g_prim_buf_cursor +=
        func_80060768((s32)g_prim_buf_cursor, 1, D_800A38E9) / 4;
    if ((g_pad_state.pressed & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        func_80033FE4();
    }
}

void func_8003C958(void) {
    gpu_HideResetAndClear();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    gpu_SetDispMaskOn();
}

void func_8003C9A4(void) {
    func_8003F1E4(0);
    D_800F6608.unk_00.x = 0;
    D_800F6608.unk_00.y = -0xBB8;
    D_800F6608.unk_00.z = 0;
    D_800F6608.unk_10.vx = 0x20;
    D_800F6608.unk_10.vz = 0;
    D_800F6608.w18 = 0x2710;
    D_800F6608.unk_10.vy = D_800A36AC << 2;
    gpu_ResetDrawQueueAndPrepareViewAndLighting(
        &D_800F6608.unk_00, &D_800F6608.unk_10, 0x2710);
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(1);

    if (D_800A3929 == 0) {
        g_prim_buf_cursor +=
            func_8005C8A8(1, D_800A3817, (s32)g_prim_buf_cursor, 0) / 4;

        if ((g_pad_state.pressed & 0x10001000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3817 != 0) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((g_pad_state.pressed & 0x40004000) != 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = 0;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }

        if ((g_pad_state.pressed & 0x400040) != 0) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
        }
        return;
    }

    if (D_800A3817 == 0) {
        g_prim_buf_cursor += func_8005FA98(0, (s32)g_prim_buf_cursor, 1) / 4;
    }
    D_800A3929 = D_800A3929 + 1;
    if (D_800A3929 < 0x3C)
        return;

    cdrom_PauseAndWait();
    if (D_800A3817 == 0) {
        if (D_800A38DC == 5) {
            gpu_ResetGraphMode1();
            motion_InvalidateMarkedNdataPackAndResetLoadCaches();
            g_fight_match_cursor = 0;
            fight_AdvanceMatch();
            return;
        }
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF =
            fight_FindNearestStartPair(&D_80101EC8[D_800A3748].world_pos);
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if (D_800A3817 == 2) {
        snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
        D_800A3834 = 8;
    }
}

void func_8003CCCC(void) {
    gpu_HideResetAndClear();
    eff_StopCommands();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    gpu_SetDispMaskOn();
}

void func_8003CD10(void) {
    func_8003F1E4(0);
    D_800F6608.unk_00.x = 0;
    D_800F6608.unk_00.y = -0xBB8;
    D_800F6608.unk_00.z = 0;
    D_800F6608.unk_10.vx = 0x20;
    D_800F6608.unk_10.vz = 0;
    D_800F6608.w18 = 0x2710;
    D_800F6608.unk_10.vy = D_800A36AC << 2;
    gpu_ResetDrawQueueAndPrepareViewAndLighting(
        &D_800F6608.unk_00, &D_800F6608.unk_10, 0x2710);
    gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(1);

    g_prim_buf_cursor += func_800600C8(D_800A391F, (s32)g_prim_buf_cursor, 1);
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 >= 0x97 || (g_pad_state.pressed & 0x400040) != 0) {
        cdrom_PauseAndWait();
        snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
        D_800A3834 = 8;
    }
}

extern s32 D_800A3818;

void func_8003CE18(void) {
    s32 s0;
    s8 var_1;

    snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
    cdrom_PauseAndWait();
    gpu_HideResetAndClear();
    gpu_ResetGraphMode1();
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    gpu_RestoreObjectClutOffsets();
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    var_1 = D_800A3748;
    {
        if ((u16)(D_80101EC8[var_1].unk_0E - 6) < 2) {
            s0 = 8;
            if (var_1 != 0) {
                s0 = 9;
            }
        } else {
            s0 = 6;
            if (var_1 != 0) {
                s0 = 7;
            }
        }
    }
    {
        Vec3i32 *addr = &D_80101EC8[0].world_pos;
        s32 result;
        if (D_800A3748 == 0) {
            addr = &D_80101EC8[1].world_pos;
        }
        result = fight_FindNearestStartPair(addr);
        D_800A3818 = result;
        func_80054884(0x16, s0, result, (s32)D_80101EC8[0].unk_12,
                      (s32)D_80101EC8[1].unk_12, -1, -1, 0);
    }
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(0, 0);
    node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(1, 0);
    eff_StopCommands();
    D_800A37B8 = 0;
    D_800A3834 = 0x1D;
    gpu_SetDispMaskOn();
}

extern void func_8003B328(void);
extern void func_8003B534(s32);
extern SVECTOR D_800A312C;

void func_8003CF84(void) {
    /* FAKE: unwritten leading pad: the original frame's untouched 16-byte
     * leading region (a compiled-out >=7-word call) (dead-vars-local-array
     * carve-out, owner rulings 2026-08-17 + 2026-08-18; SOTN precedent
     * `volatile u32 pad[4]; // FAKE` at st/sel/stream.c:80); without it:
     * score 17 */
    volatile u32 pre_pad[4];
    s32 vec[3];
    /* FAKE: unwritten trailing pad: the target frame's second untouched
     * 8-byte object above vec (owner ruling 2026-08-18, this function only);
     * without it: score 8 */
    volatile u32 pad2[2];
    s16 *a;
    s32 *b;
    s32 flag = 0;
    s8 p;
    s16 var_7;

    snd_UpdateStageSounds();
    p = D_800A3748;
    var_7 = D_80101EC8[p].unk_0A;
    if (D_800A37B8 == D_8008EAC0[var_7]) {
        snd_QueueSound(40 * p + 0x2D, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB04) {
        snd_QueueSound(40 * D_800A3748 + 0x31, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB06) {
        snd_QueueSound(40 * D_800A3748 + 0x36, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB08) {
        if (D_800A3748 == 0) {
            snd_QueueSound(0x53, 0x7F, 0x7F);
        } else {
            snd_QueueSound(0x2B, 0x7F, 0x7F);
        }
    }
    if (D_800A37B8 == D_8008EB0A) {
        snd_QueueSound(0x71, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB0C) {
        fight_GetStartPosition(0, vec, D_800A3818);
        vec[0] += D_8008EB10;
        vec[1] += D_8008EB14;
        vec[2] += D_8008EB18;
        eff_StartRecycledVectorCommandWithRedLight(vec, &D_800A312C.vx);
    }
    a = motion_GetPlaybackViewAngles();
    b = func_8005508C();
    func_80061064(a, b);
    if (func_80054F68() == 0) {
        flag = 1;
    }
    if (flag != 0 || (g_pad_state.pressed & 0x400040) != 0) {
        func_800548DC();
        if (D_800A38DC == 4 || D_800A38DC == 6) {
            /* FAKE: indexes past D_800A37D2 into D_800A37D3 by player number
             * (Q63, this byte pair only); no single array or struct gives the
             * target's per-symbol access: score 16 */
            (&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;
        }
        motion_InitStream(0, g_motion_common_sections.unk_10);
        motion_InitStream(1, g_motion_fighter_sections[0].unk_10);
        motion_InitStream(2, g_motion_fighter_sections[1].unk_10);
        if (D_800A38DC == 0 && (u8)D_800A3836 != 0xFF) {
            snd_ResetVoicesQueueReverbAndCloseVab9OrVabs8And4ByMode();
            func_8003B328();
            func_8003AF40(0);
            func_8003AFFC();
            func_8003B534(4);
        } else {
            D_800A3834 = 0x18;
        }
    }
    D_800A37B8 = D_800A37B8 + 1;
}

void font_LoadImage(void) { LoadImage(&g_font_rect, g_font_image); }

extern s32 g_font_buf_index;
extern s32 g_font_enabled;
/* Q65: this file's statics (.sbss, allocated per file by PSYLINK), in
 * address order. */
static s32 g_font_sprite_count;
static s32 g_font_col;
static s32 g_font_row;
static s32 g_font_color;
static s32 D_800A3368;
static s32 D_800A336C;
static s32 D_800A3370;
static s32 D_800A3374;

void font_BeginFrame(void) {
    s32 v0;
    s32 v1;
    g_font_color = 0xF0F0F0;
    v0 = g_font_buf_index;
    v1 = g_font_enabled;
    g_font_sprite_count = 0;
    g_font_row = 0;
    g_font_col = 0;
    g_font_buf_index = (u32)v0 < 1u;
    if (v1 == 0) {
        g_font_sprite_count = 0x20;
    }
}

void font_AddTpage(void) {
    DR_TPAGE *p = &g_font_tpage[g_font_buf_index];
    OTag *ot;
    setlen(p, 1);
    p->code[0] = 0xE100001F;
    ot = (OTag *)g_gpu_ot_ptr;
    setaddr(p, ot->addr);
    ot->addr = (u32)p;
}

extern SPRT_8 g_font_sprites[2][32];

void font_DrawChar(s32 x, s32 y, s32 arg2, s32 arg3) {
    s32 n = g_font_sprite_count;
    SPRT_8 *p;
    OTag *ot;

    if (n == 0x20)
        return;
    g_font_sprite_count = n + 1;
    p = &g_font_sprites[g_font_buf_index][n];
    setlen(p, 3);
    p->code = 0x74;
    p->x0 = x;
    p->y0 = y;
    p->u0 = (arg2 & 7) * 8 - 0x40;
    p->v0 = (arg2 >> 5) * 8 - 0x20;
    p->clut = ((arg2 >> 3) & 3) << 6 | 0x773F;
    *(u32 *)&p->r0 = (arg3 >> 1) | 0x74000000;
    ot = (OTag *)g_gpu_ot_ptr;
    setaddr(p, ot->addr);
    ot->addr = (u32)p;
}

void font_DrawString(s32 x, s32 y, u8 *str, s32 arg3) {
    s32 c;
    s32 start_x = x;

    for (c = *str++; c != 0; c = *str++) {
        if (c == ' ') {
            x += 8;
        } else if (c == '\n') {
            x = start_x;
            y += 8;
        } else {
            font_DrawChar(x, y, c, arg3);
            x += 8;
        }
    }
}

typedef char *va_list;
#define va_start(ap, last) ((ap) = (va_list)(&(last) + 1))
#define va_arg(ap, type) ((type *)(void *)(ap += 4))[-1]

void font_Printf(u8 *arg0, s32 first_arg, ...) {
    char buf[0x400];
    char var_1[0x100];
    va_list ap;
    s32 cur_arg;
    s32 var_4;
    char *p;
    s32 var_6;

    cur_arg = first_arg;
    var_4 = 0;
    p = var_1;
    va_start(ap, first_arg);
    buf[0] = 0;

    while ((var_6 = *arg0++) != 0) {
        if (var_6 == '%') {
            if (var_4 == 0) {
                var_4 = 1;
            } else {
                *p = 0;
                sprintf(buf + strlen(buf), var_1, cur_arg);
                p = var_1;
                cur_arg = va_arg(ap, s32);
            }
        }
        *p++ = var_6;
    }

    *p = 0;
    sprintf(buf + strlen(buf), var_1, cur_arg);

    p = buf;
    while ((var_6 = *p++) != 0) {
        s32 row = g_font_row;
        if (row >= 0x1A)
            break;

        if (var_6 == ' ') {
            g_font_col++;
        } else if (var_6 == '\n') {
            g_font_col = 0;
            g_font_row = row + 1;
        } else if (var_6 == '~') {
            var_6 = *p++;
            if (var_6 == 0)
                break;
            if (var_6 == 'c' || var_6 == 'C') {
                s32 d1, d2, d3;
                d1 = *p++;
                if (d1 == 0)
                    break;
                d2 = *p++;
                if (d2 == 0)
                    break;
                d3 = *p++;
                if (d3 == 0)
                    break;
                g_font_color =
                    ((d1 - '0') << 5) | ((d2 - '0') << 13) | ((d3 - '0') << 21);
            }
        } else {
            font_DrawChar(
                g_font_col * 8 + 0x10, row * 8 + 0x10, var_6, g_font_color);
            g_font_col++;
        }
        if (g_font_col >= 0x4C) {
            g_font_col = 0;
            g_font_row++;
        }
    }
}

void bitstream_InitDeltaAccumulator(s32 arg0, s32 arg1) {
    Unk800A3D40Rec *ptr = &g_bitstream_delta_accumulators[arg1];
    ptr->bits.next = (u32 *)arg0;
    ptr->bits.word = 0;
    ptr->bits.avail = 0;
    ptr->unkC[5] = 0;
    ptr->unkC[4] = 0;
    ptr->unkC[3] = 0;
    ptr->unkC[2] = 0;
    ptr->unkC[1] = 0;
    ptr->unkC[0] = 0;
}

extern s32 bitstream_ReadBits(BitStream *, s32);

s16 *bitstream_AccumulateDeltas(s32 arg0) {
    s32 i = 0;
    Unk800A3D40Rec *base = &g_bitstream_delta_accumulators[arg0];
    do {
        s32 nbits;
        s16 val;
        nbits = bitstream_ReadBits(&base->bits, 4);
        if (nbits == 0) {
            nbits = 16;
        }
        val = (s16)bitstream_ReadBits(&base->bits, nbits);
        if ((val >> (nbits - 1)) & 1) {
            val |= 0xFFFF << (nbits - 1);
        }
        base->unkC[i] += val;
        i++;
    } while (i < 6);
    return base->unkC;
}

/* Bitstream reader: returns the next `n` bits of the BitStream `s`, loading
   the next word when the current one has fewer than `n` unread bits.
   `m1 = 1 << var_0; m1 -= 1;` folds into one `addiu v0,v0,-1` (split-init
   accumulation, owner ruling 2026-06-13). */
s32 bitstream_ReadBits(BitStream *s, s32 n) {
    s32 var_0 = s->avail;
    u32 r;

    if (var_0 < n) {
        u32 m1, m2;
        s32 shift;
        u32 *var_5;
        u32 w;
        u32 hi;

        n -= var_0;
        m1 = 1 << var_0;
        m1 -= 1;
        m2 = (1 << n) - 1;
        r = s->word & m1;
        var_5 = s->next;
        s->next = var_5 + 1;
        shift = 32 - n;
        w = *var_5;
        s->avail = shift;
        s->word = w;
        hi = (w >> shift) & m2;
        r = (r << n) | hi;
    } else {
        s->avail = var_0 - n;
        r = (s->word >> (var_0 - n)) & ((1 << n) - 1);
    }
    return r;
}

void gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE (*)[2]);

void gpu_InitFogDrawMoves(void) {
    RECT buf;
    buf.x = 0;
    buf.y = 0x1E0;
    buf.w = 0x140;
    buf.h = 1;
    gpu_SetDrawMoveArray(&buf, 0x1F, D_800A3D70);
    buf.w = 0x40;
    buf.x = 0x140;
    buf.y = 0x1E0;
    buf.h = 8;
    gpu_SetDrawMoveArray(&buf, 0x13, D_800A4340);
}

void gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE (*a2)[2]) {
    s32 s4, s3;
    s32 s2;

    s4 = a0->x;
    s3 = a0->y;

    s2 = a1 - 1;
    if (s2 != -1) {
        do {
            s16 v0;
            v0 = a0->y + a0->h;
            a0->y = v0;
            if (v0 >= 0x200) {
                a0->y = s3;
                a0->x = a0->x + a0->w;
            }
            SetDrawMove(&(*a2)[0], a0, s4, s3);
            (*a2)[1] = (*a2)[0];
            a2++;
        } while (--s2 != -1);
    }
}

extern void gpu_QueueDepthSpacedMovesAndLinkTerminalPacket(
    s32, s32, DR_MOVE (*)[2], s32, s32);

void gpu_LatchFogDepthOnTransitionAndQueueMoves(s32 arg0, s32 arg1) {
    s16 *rec;
    s32 *ptr;
    s32 dist;
    s32 offset;

    g_gpu_last_fog_ot_index = 0xFFFF;
    /* FAKE: the pair read through a pointer local; a direct
       `D_800906A4[arg0][0]` read is scheduled above the g_gpu_last_fog_ot_index
       store: score 9 (Q129) */
    rec = D_800906A4[arg0];
    if (rec[0] != 0) {
        {
            s32 base = func_8003F268();
            if (base == 0) {
                base = 0x6590;
            } else {
                base = 0x55F0;
            }
            dist = base - arg1;
        }
        if (dist < 0x1770) {
            offset = 0x1770 - dist;
            dist = 0x1770;
        } else {
            offset = 0;
        }
        {
            s16 v1 = D_800F6656;
            s16 mask = D_80090608;
            if ((v1 & ~mask) & 1) {
                D_80090600 = dist;
                D_80090604 = offset;
            }
            if (v1 & 1) {
                arg1 -= D_80090604;
            } else {
                arg1 -= offset;
            }
        }
        D_80090608 = (u16)D_800F6656;
        ptr = &D_8009060C[arg0];
        {
            /* FAKE: D_800906A4[arg0][1] read as a byte offset off the array
               base; element reads are scheduled past the D_80090608 store
               above, ahead of the `sh`: score 8 */
            s32 idx = arg0 * 4;
            gpu_QueueDepthSpacedMovesAndLinkTerminalPacket(
                arg1, 0x1F, D_800A3D70, *ptr,
                *(s16 *)((u8 *)D_800906A4 + 2 + idx));
            gpu_QueueDepthSpacedMovesAndLinkTerminalPacket(
                arg1, 0x13, D_800A4340, *ptr,
                *(s16 *)((u8 *)D_800906A4 + 2 + idx));
        }
    }
}

void gpu_QueueDepthSpacedMovesAndLinkTerminalPacket(
    s32 arg0, s32 arg1, DR_MOVE (*arg2)[2], s32 arg3, s32 arg4) {
    s32 limit;
    s32 step;
    DR_MOVE *pkt;
    s32 i;

    pkt = *arg2;

    if (arg4 != 0) {
        limit = arg1;
    } else {
        limit = arg1 - 1;
    }

    if (D_800F6656 & 1) {
        step = D_80090600;
    } else {
        if (func_8003F268() == 0) {
            step = 0x6590 - arg0;
        } else {
            step = 0x55F0 - arg0;
        }
    }

    if (step < 0) {
        return;
    }

    if ((u32)arg3 < (u32)step) {
        step = (s32)((u32)arg3 / (u32)arg1);
    } else {
        step = step / arg1;
    }

    pkt += D_800A36AC & 1;

    for (i = 0; i < limit; i++) {
        s32 idx = gpu_DepthToOtIndex((u32)arg0 >> 2, 2);
        if (idx < 0x1000) {
            addPrim(&D_800A378C[idx], pkt);
            pkt += 2;
            if (i == limit - 1) {
                g_gpu_last_fog_ot_index = idx;
            }
        }
        arg0 += step;
    }

    if (arg4 != 0) {
        func_8003DDF8((u32)pkt);
    } else {
        addPrim(&D_800A378C[0xFFB], pkt);
    }
}

void func_8003DDF8(u32 arg0) {
    u32 *ptr = D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}

/* Builds count-1 progressively fogged copies of the VRAM rectangle `rect`.
 * The original is first copied one rect height up; each pass then blends
 * every non-zero pixel toward the GTE far colour by (i + 1) / count (the last
 * pass writes the far colour itself) and uploads the result one rect height
 * below the previous one, wrapping to the next column at y 0x200.
 *
 * FAKE constructs (each labelled at its site): the far-colour buffer's frame
 * size, the r_src / g_src far-colour staging, and the two combine-foldable
 * chain-extender detours (zero emitted bytes; dead-store-fake-exception). */
void gpu_BuildFogImageLevels(RECT *rect, s32 count) {
    u16 src_buf[0x200];
    u16 dst_buf[0x200];
    /* FAKE: frame layout. gte_ReadFarColor writes 3 bytes, but the target
     * frame needs 0x19-0x20 bytes here; var_2[3] through [0x18]: score 22 */
    u8 var_2[0x20];
    s32 i;
    s32 saved_y;
    s32 r;
    s32 g;
    s32 b;
    s32 var_8;

    DrawSync(0);
    count--;
    StoreImage(rect, (u32 *)src_buf);
    DrawSync(0);
    rect->y -= rect->h;
    LoadImage(rect, (u32 *)src_buf);
    saved_y = rect->y;
    rect->y = rect->h + saved_y;
    gte_ReadFarColor(var_2);

    r = var_2[0];
    g = var_2[1];
    b = var_2[2];
    var_8 =
        (((u32)r >> 3) | (s32)-0x8000) | ((g & 0xF8) << 2) | ((b & 0xF8) << 7);

    i = 0;
    if (count > 0) {
        do {
            s32 total = rect->w * rect->h;
            u16 *src = src_buf;
            u16 *dst = dst_buf;
            s32 factor = ((i + 1) << 12) / count;
            s32 j = 0;
            if (total > 0) {
                do {
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst = pixel;
                            src++;
                            dst++;
                            goto loop_check;
                        }
                        *dst++ = var_8;
                        src++;
                        goto loop_check;
                    }
                    {
                        u16 pixel = *src;
                        s32 px = pixel & 0xFFFF;
                        if (px == 0) {
                            *dst = pixel;
                            src++;
                            dst++;
                            goto loop_check;
                        }
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 sum;
                            s32 var_22;
                            s32 var_23;
                            s32 green;
                            /* the source blue, weighted, then blended in
                             * place */
                            s32 b_src;
                            src++;
                            /* FAKE: r_src / g_src reused for the far-colour
                             * term (r * factor, g * factor) between the
                             * weighted source and the blend; fresh r_far /
                             * g_far locals: score 33 */
                            var_22 = r_src * (0x1000 - factor);
                            r_src = r * factor;
                            sum = var_22 + r_src;
                            r_src = sum >> 15;
                            r_src = r_src & 0x1F;
                            var_23 = g_src * (0x1000 - factor);
                            g_src = g * factor;
                            sum = var_23 + g_src;
                            g_src = sum >> 10;
                            b_src = ((u32)px >> 7) & 0xF8;
                            b_src = b_src * (0x1000 - factor);
                            sum = b_src + b * factor;
                            b_src = sum >> 5;
                            green = g_src & 0x3E0;
                            *dst = (pixel & 0x8000) | r_src | green |
                                   (b_src & 0x7C00);
                        }
                    }
                    dst++;
                loop_check:
                    j++;
                    /* FAKE: bound routed through `+ rect->w - rect->w`, which
                     * combine folds away (zero bytes); the extra refs rank the
                     * rect->w load first in local-alloc so it takes $v0 as in
                     * the target (`lh $v0,4($s0)` / `lh $v1,6($s0)`); the
                     * natural bound gives it $v1: score 3
                     * (dead-store-fake-exception chain extender, owner ruling
                     * 2026-07-01) */
                } while (j < rect->w * rect->h + rect->w - rect->w);
            }

            rect->y += rect->h;
            if (rect->y >= 0x200) {
                rect->y = saved_y;
                rect->x += rect->w;
            }
            /* FAKE: j chain extender (+j/-j folds away); the extra refs lift j
             * above the `0x1000 - factor` value so the $t4/$t5 pair matches;
             * without it they swap: score 7 (both FAKEs reverted: score 10) */
            LoadImage(rect, (u32 *)(((s32)dst_buf + j) - j));
            DrawSync(0);
            i++;
        } while (i < count);
    }
}

void func_8003E0E0(void) {
    RECT buf;
    buf.y = 0x1E1;
    buf.w = 0x140;
    buf.x = 0;
    buf.h = 1;
    gpu_BuildFogImageLevels(&buf, 0x1F);
}

void func_8003E120(void) {
    RECT buf;
    buf.x = 0x140;
    buf.y = 0x1E8;
    buf.w = 0x40;
    buf.h = 8;
    gpu_BuildFogImageLevels(&buf, 0x13);
}

extern s32 g_gpu_fog_clut_object;

void gpu_MoveObjectClutsToFogBlock(s32 arg0) {
    RECT buf;
    Unk80045878Obj *obj;

    if (g_gpu_fog_clut_object != arg0) {
        gpu_RestoreObjectClutOffsets();
        obj = slot_GetObject(arg0);
        if (obj != 0) {
            if (arg0 != 0) {
                buf.x = 0x300;
            } else {
                buf.x = 0x280;
            }
            buf.y = 0xF8;
            buf.w = 0x40;
            buf.h = 6;
            MoveImage(&buf, 0x140, 0x1E8);
            if (arg0 == 0) {
                gpu_OffsetBlockTexPrimStreams(obj->unk_14, 0, 0, -0x140, 0xE8);
            } else {
                gpu_OffsetBlockTexPrimStreams(obj->unk_14, 0, 0, -0x1C0, 0xE8);
            }
            DrawSync(0);
            func_8003E120();
        }
    }
    g_gpu_fog_clut_object = arg0;
}

void gpu_RestoreObjectClutOffsets(void) {
    Unk80045878Obj *v1;

    if (g_gpu_fog_clut_object != -1) {
        v1 = slot_GetObject(g_gpu_fog_clut_object);
        if (v1 != 0) {
            if (g_gpu_fog_clut_object == 0) {
                gpu_OffsetBlockTexPrimStreams(v1->unk_14, 0, 0, 0x140, -0xE8);
            } else {
                gpu_OffsetBlockTexPrimStreams(v1->unk_14, 0, 0, 0x1C0, -0xE8);
            }
        }
        g_gpu_fog_clut_object = -1;
    }
}

s32 gpu_GetFogClutObject(void) { return g_gpu_fog_clut_object; }

void func_8003E2AC(void) {
    /* FAKE: the flag word's address held in p (one lui / addiu, lhu / sh off
     * it); `D_800F6656 &= ~2` scores 6 */
    s16 *p = &D_800F6656;
    *p &= ~2;
}

u32 gpu_GetLastFogOtIndex(void) { return g_gpu_last_fog_ot_index; }

extern void gpu_SetCellGateBit(s16 *arg0);

void gpu_SetSegmentCellsGateBit(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_fp;
    s32 temp_lo;
    s32 temp_s2;
    s32 temp_s7;
    s32 temp_t0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 var_s0;
    s32 var_s1;
    s32 var_s3;
    s32 var_s4;
    s32 var_s5;
    s32 var_s6;

    temp_t0 = arg0 + 0x7D00;
    temp_s2 = arg1 + 0x7D00;
    temp_a2 = arg2 + 0x7D00;
    temp_a3 = arg3 + 0x7D00;
    arg0 = temp_a2 - temp_t0;
    arg1 = temp_a3 - temp_s2;
    var_s1 = temp_t0 / 2000;
    var_s0 = temp_s2 / 2000;
    temp_fp = temp_a2 / 2000;
    temp_s7 = temp_a3 / 2000;
    if ((arg0 == 0) && (arg1 == 0)) {
        if ((var_s1 >= 0) && (var_s0 >= 0) && (var_s1 < 0x20) &&
            (var_s0 < 0x20)) {
            sp10[0] = var_s1 - 0x10;
            sp10[2] = var_s0 - 0x10;
            gpu_SetCellGateBit(sp10);
        }
    } else {
        temp_v1 = (var_s1 * 0x7D0) + 0x3E8;
        temp_t0 -= temp_v1;
        temp_v0 = (var_s0 * 0x7D0) + 0x3E8;
        temp_s2 -= temp_v0;
        temp_a2 -= temp_v1;
        temp_a3 -= temp_v0;
        if (arg0 < 0) {
            var_s6 = -1;
            arg0 = -arg0;
            temp_t0 = -temp_t0;
            temp_a2 = -temp_a2;
        } else {
            var_s6 = 1;
        }
        var_s5 = 1;
        if (arg1 < 0) {
            var_s5 = -1;
            arg1 = -arg1;
            temp_s2 = -temp_s2;
            temp_a3 = -temp_a3;
        }
        if (arg0 < arg1) {
            temp_v0_2 = arg0;
            arg0 = arg1;
            arg1 = temp_v0_2;
            temp_v0_3 = temp_t0;
            temp_t0 = temp_s2;
            temp_s2 = temp_v0_3;
            temp_a2 = temp_a3;
            var_s4 = 1;
        } else {
            var_s4 = 0;
        }
        temp_lo = (s32)(arg1 << 0xC) / arg0;
        temp_a2 += 0x3E8;
        temp_t0 += 0x3E8;
        var_s3 = (temp_a2 / 2000) - (temp_t0 / 2000);
        temp_s2 += 0x3E8;
        temp_s2 -= (s32)(temp_t0 * temp_lo) >> 0xC;
        sp20 = (s32)(temp_lo * 0x7D0) >> 0xC;
        for (; var_s3 != -1; var_s3--) {
            if (var_s1 >= 0) {
                if ((var_s0 >= 0) && (var_s1 < 0x20) && (var_s0 < 0x20)) {
                    sp10[0] = var_s1 - 0x10;
                    sp10[2] = var_s0 - 0x10;
                    gpu_SetCellGateBit(sp10);
                }
            }
            temp_s2 %= 2000;
            temp_s2 += sp20;
            if (var_s3 == 0) {
                break;
            }
            if (temp_s2 >= 0x7D1) {
                if (var_s4 != 0) {
                    if (var_s6 < 0) {
                        var_s1 -= 1;
                    } else {
                        var_s1 += 1;
                    }
                } else if (var_s5 < 0) {
                    var_s0 -= 1;
                } else {
                    var_s0 += 1;
                }
                if ((var_s1 >= 0) && (var_s0 >= 0) && (var_s1 < 0x20) &&
                    (var_s0 < 0x20)) {
                    sp18[0] = var_s1 - 0x10;
                    sp18[2] = var_s0 - 0x10;
                    gpu_SetCellGateBit(sp18);
                }
            }
            if (var_s4 != 0) {
                if (var_s5 < 0) {
                    var_s0 -= 1;
                } else {
                    var_s0 += 1;
                }
            } else if (var_s6 < 0) {
                var_s1 -= 1;
            } else {
                var_s1 += 1;
            }
        }
        if ((temp_fp >= 0) && (temp_s7 >= 0) && (temp_fp < 0x20) &&
            (temp_s7 < 0x20)) {
            sp10[0] = temp_fp - 0x10;
            sp10[2] = temp_s7 - 0x10;
            gpu_SetCellGateBit(sp10);
        }
    }
}

void gpu_SetViewOriginSegmentCellsGateBit(s32 arg0, s32 arg1) {
    gpu_SetSegmentCellsGateBit(
        D_80101DF0.work.t[0], D_80101DF0.work.t[2], arg0, arg1);
}

extern s32 D_800A322C;
extern s32 D_800927C0[65][32];
extern s32 D_80090740[65][32];
extern s32 D_80094840[];
/* gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards's
 * buffer of the queued Unk800A6690Rec */
extern Unk800A6690Rec *D_800A7EF0[];
extern Unk800A6690Rec **func_8003EB84(s32, s32, Unk800A6690Rec **);

/* Grid pass: D_800A3708's xf.rot is turned into a matrix and applied to
 * {0,0,0x1000}; ratan2 of the result (D_800A336C) picks one of 64 tables of
 * 32-bit row masks. Each set cell of the 31x31 grid window centred on
 * D_800A3708's cell emits its records, as in func_8003EB84. */
void gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards(
    s32 arg0) {
    s32 mat[8];
    s16 vec[4];
    s32 res[3];
    s16 pos[4];
    s32 cx;
    s32 cz;
    s16 x;
    s16 z;
    s32 *mask;
    Unk800A6690Rec **out;
    s32 i;
    s32 j;
    s32 bits;
    s16 row;
    s16 col;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    Unk800A4750Rec *e;
    Unk800A6690Rec *e2;
    void **list;
    s32 ang;
    s32 idx;

    cx = (D_800A3708->work.t[0] + 0x7D00) / 2000;
    cz = (D_800A3708->work.t[2] + 0x7D00) / 2000;
    camera_CalcAngles();
    math_RotMatrixYXZ((u16 *)&D_800A3708->xf.rot, (MATRIX *)mat);
    vec[2] = 0x1000;
    vec[0] = 0;
    vec[1] = 0;
    gte_ApplyMatrix(mat, vec, res);
    ang = ratan2(res[0], res[2]);
    D_800A336C = ang;
    x = cx - 0xF;
    z = cz - 0xF;
    idx = (ang >> 6) & 0x3F;
    if (D_800A322C != 0) {
        mask = D_800927C0[idx];
    } else if (D_800F6656 & 1) {
        mask = D_80090740[idx];
    } else {
        s32 *src2;
        s32 *dst;
        src2 = D_80094840;
        mask = D_80090740[idx];
        dst = (s32 *)0x1F800004;
        for (i = 0; i < 0x1F; i++) {
            *dst++ = *src2++ & *mask++;
        }
        mask = (s32 *)0x1F800004;
    }

    out = D_800A7EF0;
    for (i = 0; i < 0x1F; i++) {
        row = z + i;
        if (row < 0) {
            mask++;
            continue;
        }
        if (row >= 0x20) {
            break;
        }
        bits = *mask++;
        if (bits == 0) {
            continue;
        }
        bits <<= 1;
        for (j = 0; j < 0x1F; j++) {
            col = x + j;
            if (col < 0) {
                bits <<= 1;
                continue;
            }
            if (col >= 0x20) {
                break;
            }
            if (bits < 0) {
                vidx = D_800A7FE0[row][col];
                if (vidx >= 0) {
                    a3 = g_gpu_cell_draw_attributes[row * 0x20 + col];
                    do {
                        t0 = D_800A87E0[vidx++];
                        v1 = t0 & 0x7FFF;
                        if (v1 < D_800A3368) {
                            e = &D_800A4750[v1];
                            e->unk6 = a3 & 3;
                            if ((a3 & 8) || ((a3 & 4) && (e->unk7 & 8))) {
                                e->unk7 |= 1;
                            } else {
                                e->unk7 &= 0xFE;
                            }
                            list = g_draw_queue_cursor;
                            g_draw_queue_cursor = list + 1;
                            *list = e;
                        } else {
                            e2 = &D_800A6690[v1 - D_800A3368];
                            if (e2->unk58 == 0) {
                                *out++ = e2;
                                e2->unk58 = 1;
                            }
                        }
                    } while (!(t0 & 0x8000));
                }
            }
            bits <<= 1;
        }
    }

    if ((D_800F6656 & 1) && D_800A322C == 0) {
        out = func_8003EB84(x, z, out);
    }
    while (out != D_800A7EF0) {
        out--;
        *g_draw_queue_cursor = *out;
        ((Unk800A6690Rec *)*g_draw_queue_cursor)->unk58 = 0;
        g_draw_queue_cursor++;
    }
    if (D_800948BC[resource_GetLoadedSelector()].unk4 != 0) {
        D_800948BC[resource_GetLoadedSelector()].unk4();
    }
    pos[0] = -D_80101DF0.xf.rot.vx;
    pos[1] = -D_80101DF0.xf.rot.vy;
    pos[2] = -D_80101DF0.xf.rot.vz;
    eff_DrawBillboardList(pos, D_80101DF0.xf.mat.t);
}

Unk800A6690Rec **func_8003EB84(s32 a0, s32 a1, Unk800A6690Rec **out) {
    s32 sp[0x20];
    s32 mask;
    s32 i;
    s32 t4;
    s32 t1;
    s32 t2;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    Unk800A4750Rec *e;
    Unk800A6690Rec *e2;
    void **list;

    if (a0 >= 0) {
        mask = -1;
        if (a0 >= 0x20) {
            goto skip;
        }
        goto calc;
    } else {
        mask = -1;
        if (-a0 < 0x20) {
        calc:
            if (a0 < 0) {
                mask = -1U >> (a0 + 0x1F);
            } else if (a0 == 0) {
                mask = 0;
            } else {
                mask = -1 << (0x20 - a0);
            }
        }
    }
skip:
    for (i = 0; i < 0x20; i++) {
        if (i >= a1 && i < a1 + 0x1F) {
            sp[i] = mask;
        } else {
            sp[i] = -1;
        }
    }

    for (t4 = 0; t4 < 0x20; t4++) {
        t2 = sp[t4];
        if (t2 != 0) {
            for (t1 = 0; t1 < 0x20; t1++) {
                if (t2 < 0) {
                    vidx = D_800A7FE0[t4][t1];
                    if (vidx >= 0) {
                        a3 = g_gpu_cell_draw_attributes[t4 * 0x20 + t1];
                        do {
                            t0 = D_800A87E0[vidx++];
                            v1 = t0 & 0x7FFF;
                            if (v1 < D_800A3368) {
                                e = &D_800A4750[v1];
                                e->unk6 = a3 & 3;
                                if ((a3 & 8) || ((a3 & 4) && (e->unk7 & 8))) {
                                    e->unk7 |= 1;
                                } else {
                                    e->unk7 &= 0xFE;
                                }
                                list = g_draw_queue_cursor;
                                g_draw_queue_cursor = list + 1;
                                *list = e;
                            } else {
                                e2 = &D_800A6690[v1 - D_800A3368];
                                if (e2->unk58 == 0) {
                                    *out = e2;
                                    out += 1;
                                    e2->unk58 = 1;
                                }
                            }
                        } while (!(t0 & 0x8000));
                    }
                }
                t2 = t2 << 1;
            }
        }
    }

    return out;
}

extern s32 D_800A3230;

/* func_8003EDC0 - unpacks a stream of u16 words in sections ended by -1:
 * (cell, id) pairs into the 16-byte record table (count kept in D_800A3368),
 * the transform-node records (header word, xf.mat.t, xf.rot; each node is
 * then passed to its D_800F66A0 handler), then the 32x32 cell grid,
 * each listed cell indexing a run of record ids (bit 15 ends a run) in the
 * run table. Cells are numbered row * 32 + column. */
void func_8003EDC0(u16 *p, s32 arg1) {
    Unk800A4750Rec *r;
    Unk800A6690Rec *c;
    s32 i;
    s32 j;
    s16 n;
    s16 w; /* the stream word just read */
    s16 x;
    s16 z;
    u16 var_8;

    n = 0;
    while ((w = *p++) != -1) {
        r = &D_800A4750[n++];
        var_8 = *p++;
        r->unk4 = var_8;
        if (var_8 & 0x8000) {
            r->unk4 = var_8 & 0x7FFF;
            r->unk7 = 8;
        } else {
            r->unk7 = 0;
        }
        r->unk0 = 0xC;
        r->unk6 = 0;
        r->unk2 = arg1;
        r->unk8 = (w % 32) * 2000 - 32000;
        r->unkC = (w / 32) * 2000 - 32000;
    }
    D_800A3368 = n;
    n = 0;
    while ((w = *p++) != -1) {
        c = &D_800A6690[n++];
        c->node.unk1 = 0;
        c->node.parent_node = 0;
        c->node.rotation_order = 0;
        c->node.light_setup_index = 4;
        c->node.unk2 = w;
        c->node.unk4 = arg1;
        c->node.xf.mat.t[0] = *p++;
        c->node.xf.mat.t[0] |= *p++ << 16;
        c->node.xf.mat.t[1] = *p++;
        c->node.xf.mat.t[1] |= *p++ << 16;
        c->node.xf.mat.t[2] = *p++;
        c->node.xf.mat.t[2] |= *p++ << 16;
        c->node.xf.rot.vx = *p++;
        c->node.xf.rot.vy = *p++;
        c->node.xf.rot.vz = *p++;
        if (c->node.xf.rot.vz != (c->node.xf.rot.vx == c->node.xf.rot.vy)) {
            c->node.unk0 = 1;
        } else {
            c->node.unk0 = 0;
        }
        D_800F66A0[c->node.rotation_order](&c->node.xf.rot, &c->node.xf.mat);
        c->unk58 = 0;
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            D_800A7FE0[i][j] = -1;
        }
    }
    /* the grid row counter i, restarted as the run-table fill index (its
     * final value is the entry count), the way SOTN's DebugCaptureScreen
     * reuses its i loop to loop (Q51) */
    /* SOTN: src/dra/42398.c:75 @aa53500 */
    i = 0;
    while ((w = *p++) != -1) {
        x = w % 32;
        z = w / 32;
        D_800A7FE0[z][x] = i;
        do {
            w = *p++;
            D_800A87E0[i++] = w;
        } while (!(w & 0x8000));
    }
    D_800A3230 = (D_800A3230 >= i) ? D_800A3230 : i;
    if (D_800A3230 >= 1000) {
        /* The overflowing count is passed to the halt stub: the target loads
         * it into $a0 for this jal (no argument puts it in $v1: score 5), as
         * softbody_CreateForNode's call loads its message string into $a0.
         * The stub's body ignores its arguments. */
        func_80052C10(D_800A3230);
    }
    D_800A3678[0] = 0;
    D_800A3678[1] = 0;
    D_800A3678[2] = 0;
}

/* Merged section (Q65: one original file). */

/* Read by softbody_CreateForNode: 14 chars + NUL + 1 pad. */
const char D_80010D8C[16] = "Multipul Model";

extern void gte_SetMatrixRotTransIR(MATRIX *, SVECTOR *, VECTOR *);

extern s32 D_80094A6C[];

void func_8003F168(void) {
    if (D_800948BC[resource_GetLoadedSelector()].init != 0) {
        D_800948BC[resource_GetLoadedSelector()].init();
    }
}

s32 func_8003F1C8(void) { return D_800A336C; }

void *func_8003F1D4(void) { return D_800A6690; }

void func_8003F1E4(s32 a0) {
    if (a0) {
        D_800F6656 = 3;
        D_800F6658 = 2;
    } else {
        D_800F6656 = 0;
        D_800F6658 = 1;
    }
}

void func_8003F218(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == D_800A322C) {
        return;
    }
    D_800A322C = a0;
    if (!a0) {
        func_8003F1E4(0);
    }
    D_800F665C = (s16)D_800A322C;
}

s32 func_8003F268(void) { return D_800A322C; }

void gpu_ResetAndStampViewCenteredCellAttributes(void) {
    s32 i, j;
    s32 var_2, var_3;
    s32 data;
    s32 var_5;
    s32 var_6;
    s32 *ptr = (s32 *)g_gpu_cell_draw_attributes;

    i = 0xFF;
    do {
        *ptr = 0;
        i--;
        ptr++;
    } while (i >= 0);

    func_8003F268();

    var_2 = (D_800A3708->work.t[0] + 0x7D00) / 2000;
    var_3 = (D_800A3708->work.t[2] + 0x7D00) / 2000;

    for (i = 0; i < 16; i++) {
        var_5 = i - 8;
        {
            u32 y = (u32)(var_3 + var_5);
            if (y < 0x20) {
                data = D_80094A6C[i];
                for (j = 0; j < 16; j++) {
                    var_6 = j - 8;
                    {
                        u32 x = (u32)(var_2 + var_6);
                        if (x < 0x20) {
                            s32 bits = (data >> ((15 - j) * 2)) & 3;
                            g_gpu_cell_draw_attributes[(y << 5) + x] |= bits;
                        }
                    }
                }
            }
        }
    }
}

void gpu_SetCellGateBit(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_gpu_cell_draw_attributes[y * 32 + x] |= 0x4;
    }
}

void gpu_SetCellForceBit(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_gpu_cell_draw_attributes[y * 32 + x] |= 0x8;
    }
}

void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    s32 s0;
    a0 += 0x7D00;
    a1 += 0x7D00;
    s3 = a0 / 2000;
    s1 = a0 - s3 * 2000;
    s0 = a1 / 2000;
    s2 = s0;
    s0 = a1 - s2 * 2000;
    if (s1 < 1000) {
        s1 = -1;
    } else {
        s1 = 1;
    }
    if (s0 < 1000) {
        s0 = -1;
    } else {
        s0 = 1;
    }
    func_8003F52C(s3, s2, 2);
    func_8003F52C(s3 + s1, s2, 2);
    func_8003F52C(s3, s2 + s0, 2);
    func_8003F52C(s3 + s1, s2 + s0, 2);
}

void func_8003F52C(s32 a0, s32 a1, s32 a2) {
    g_gpu_cell_draw_attributes[a1 * 32 + a0] = a2 & 3;
}

u32 func_8003F54C(s32 a0, s32 a1) {
    return g_gpu_cell_draw_attributes[a1 * 32 + a0];
}

void ClearSavedWordPairsAndTwoWriteOnlyWords(void) {
    D_800A3374 = 0;
    D_800A3370 = 0;
    D_800A93B0[2] = 0;
    D_800A93B0[1] = 0;
    D_800A93B0[0] = 0;
    D_800A93BC[2] = 0;
    D_800A93BC[1] = 0;
    D_800A93BC[0] = 0;
}

void func_8003F5A8(s32 a0, s32 a1, s32 a2) {
    D_800A93B0[a2] = a0;
    D_800A93BC[a2] = a1;
}

void func_8003F5CC(void) {
    func_80017F98(D_800A93B0[0], D_800A93BC[0], 0);
    func_80017F98(D_800A93B0[1], D_800A93BC[1], 1);
    func_80017F98(D_800A93B0[2], D_800A93BC[2], 2);
}

typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ Unk80045878Node *objs[3];
    /* 0x10 */ Unk8003F6D8Coll coll;
    /* 0x84 */ SVECTOR f84[3][2];
} Func8003F6D8Inner; /* size 0xB4; sits at +0x1C of each 0xD0-byte record,
                        records start at arg0+8 */

typedef struct {
    /* 0x00 */ s16 slot_index;
    /* 0x02 */ u8 unk2[2];
    /* 0x04 */ Unk80017FA0Rec step_desc;
    /* 0x14 */ Unk80045878Node *obj;
    /* 0x18 */ u8 *cur;
    /* 0x1C */ Func8003F6D8Inner inner;
} Unk8003F824Rec; /* size 0xD0 */

typedef struct {
    /* 0x000 */ s16 count;
    /* 0x002 */ s16 snap_request;
    /* 0x004 */ s16 unk4;
    /* 0x006 */ s16 rebuild_request;
    /* 0x008 */ Unk8003F824Rec recs[5];
    /* 0x418 */ Unk80017FA0Rec step_descs[5];
    /* 0x468 */ u8 data[1];
} Unk8003F62CRec;

extern void softbody_UpdateColliders(Unk8003F62CRec *);
extern void softbody_SetObjectSnap(Unk8003F62CRec *);
extern void softbody_ClearObjectSnap(Unk8003F62CRec *);

void softbody_UpdateObject(Unk80045878Obj *a0) {
    Unk8003F62CRec *s0;
    s0 = a0->unk_24;
    if (s0 == 0)
        return;
    if (s0->rebuild_request) {
        softbody_FreeObjectById(a0->arena_id);
        softbody_SetupObject(a0, 0);
    }
    if (s0->snap_request) {
        softbody_SetObjectSnap(s0);
    }
    softbody_UpdateColliders(s0);
    softbody_UpdateList(s0->step_descs, s0->count);
    if (s0->snap_request) {
        softbody_ClearObjectSnap(s0);
        s0->snap_request = 0;
    }
}

void softbody_UpdateColliders(Unk8003F62CRec *arg0) {
    s32 i;
    s32 j;

    for (i = 0; i < arg0->count; i++) {
        Func8003F6D8Inner *in = &arg0->recs[i].inner;
        for (j = 0; j < in->count; j++) {
            MATRIX *mat = &in->objs[j]->node.xf.mat;
            gte_SetMatrixRotTransIR(mat, &in->f84[j][0], &in->coll.foci[j][0]);
            gte_SetMatrixRotTransIR(mat, &in->f84[j][1], &in->coll.foci[j][1]);
        }
    }
}

void func_8003F7F4(void) {
    softbody_FreeAll();
    func_80017F90();
    D_800A3370 = 0;
    D_800A3374 = 0;
}

extern u8 *softbody_CreateForNode(Unk8003F824Rec *rec, s16 *cmds, u8 *cur);
void softbody_AddColliders(Unk80045878Obj *a0, Unk8003F824Rec *rec, s16 *a2);

void softbody_SetupObject(Unk80045878Obj *arg0, s32 arg1) {
    Unk8003F62CRec *var_0;
    s16 *var_1;
    u8 *cur;
    Unk8003F824Rec *rec;
    Unk80045878Node *obj;
    s32 i;

    var_0 = arg0->unk_24;
    if (var_0 == 0)
        return;
    var_1 = (s16 *)arg0->softbody_params;
    cur = var_0->data;
    if (*var_1 == -3) {
        arg0->unk_24 = 0;
        return;
    }
    var_0->count = 0;
    var_0->snap_request = 0;
    var_0->unk4 = 0;
    var_0->rebuild_request = 0;
    for (i = 0; *var_1 != -3; i++) {
        if (*var_1 != -2) {
            if (var_0->count >= 5) {
                func_80052C10();
            }
            rec = &var_0->recs[var_0->count];
            obj = arg0->node_by_id[i];
            rec->cur = cur;
            rec->obj = obj;
            cur = softbody_CreateForNode(rec, var_1, cur);
            var_0->step_descs[var_0->count] = rec->step_desc;
            obj->node.unk0 = 0xD;
            var_0->count++;
            if (*var_1 >= 0) {
                var_1++;
                while (*var_1 >= 0) {
                    var_1++;
                }
            }
            if (*var_1 == -1) {
                var_1++;
                softbody_AddColliders(arg0, rec, var_1);
            }
        }
        while (*var_1++ != -2) {
        }
    }
    if (arg1) {
        func_80045A28(arg0->arena_id, cur - (u8 *)arg0->unk_1C);
    }
}

extern s16 D_80094AEC[];

/* The face records softbody_CreateForNode walks, one layout per mode ((flags >>
 * 3) & 3): bit 0 set is a quad (the size pass counts 6 springs per face, one
 * per pair of its 4 points; 3 for a triangle), bit 1 picks the longer record.
 * The sizes are the D_80094AEC[mode] strides in halfwords (12, 12, 18, 22).
 * vtx_ids holds the face's point indices, one byte each (4 for a quad, 3 for
 * a triangle), which softbody_CreateForNode writes as one spring group (count,
 * group id 2, the index bytes); read as two halfwords (the mode-1 quad record
 * places it off word alignment).
 */
typedef struct {
    u16 unk0[6];
    u16 vtx_ids[2];
    u16 unk8[4];
} FaceTri; /* mode 0 */

typedef struct {
    u16 unk0[7];
    u16 vtx_ids[2];
    u16 unk9[3];
} FaceQuad; /* mode 1 */

typedef struct {
    u16 unk0[6];
    u16 vtx_ids[2];
    u16 unk8[10];
} FaceTriLong; /* mode 2 */

typedef struct {
    u16 unk0[8];
    u16 vtx_ids[2];
    u16 unk10[12];
} FaceQuadLong; /* mode 3 */

s16 *softbody_SetAnchorWeights(SVECTOR *a0, s32 a1, s16 *a2);

u8 *softbody_CreateForNode(Unk8003F824Rec *rec, s16 *cmds, u8 *cur) {
    /* FAKE: frame layout (oversized live object): softbody_Create's argument
       plus an unwritten tail (two or three words are byte-identical: three
       chosen); the argument alone gives a frame 8 short (0x48 for 0x50):
       score 16. */
    struct {
        Func80017A44Input in;
        s32 tail[3];
    } init;

    Unk80045878Node *obj;
    u16 *src;
    u16 *block;
    s16 count;
    u16 flags;
    s32 mode;
    s32 point_count;
    s32 value;
    s32 n;
    s16 *packet;
    u16 *dst;

    obj = rec->obj;
    dst = (u16 *)cur;
    src = (u16 *)D_80103608[obj->node.unk4][obj->node.unk2];
    count = *src;
    init.in.count = count;
    init.in.points = (SVECTOR *)cur;
    src += 2;
    count--;
    if (count != -1) {
        do {
            dst[0] = *src++;
            dst[1] = *src++;
            dst[2] = *src++;
            dst += 4;
            count--;
        } while (count != -1);
    }
    cur = (u8 *)dst;
    if ((u32)src & 3) {
        src++;
    }

    block = src;
    point_count = 0;
    count = *src++;
    while (count != 0) {
        flags = *src++;
        if (flags & 8) {
            point_count += count * 6;
        } else {
            point_count += count * 3;
        }
        src += D_80094AEC[(flags >> 3) & 3] * count;
        count = *src++;
    }

    init.in.buf = cur;
    cur = cur + softbody_GetBufferSize(init.in.count, point_count);
    cur = ((u32)cur & 3) ? cur + 2 : cur;

    src = block;
    packet = (s16 *)0x1F800000;
    init.in.groups = packet;
    /* FAKE: n reads each group count signed (lh) and is copied to count; read
     * straight into count the load is lhu and the copy goes (score 8) */
    for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {
        flags = *src++;
        /* FAKE: mode computed once ahead of the record loop; indexing
         * D_80094AEC with the expression moves the addiu s0 / andi pair (score
         * 6). The following tests re-derive ((s16)flags >> 3) & 1 / & 2 from
         * flags rather than reading mode (Q129 blind verdict doubtful). */
        mode = ((s16)flags >> 3) & 3;
        if (((s16)flags >> 3) & 1) {
            while (--count != -1) {
                if (((s16)flags >> 3) & 2) {
                    FaceQuadLong *f = (FaceQuadLong *)src;
                    value = ((u32)f->vtx_ids[1] << 16) | f->vtx_ids[0];
                } else {
                    FaceQuad *f = (FaceQuad *)src;
                    value = ((u32)f->vtx_ids[1] << 16) | f->vtx_ids[0];
                }
                *packet++ = 4;
                *packet++ = 2;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                *packet++ = (u32)value >> 24;
                src += D_80094AEC[mode];
            }
        } else {
            while (--count != -1) {
                if (((s16)flags >> 3) & 2) {
                    FaceTriLong *f = (FaceTriLong *)src;
                    value = ((u32)f->vtx_ids[1] << 16) | f->vtx_ids[0];
                } else {
                    FaceTri *f = (FaceTri *)src;
                    value = ((u32)f->vtx_ids[1] << 16) | f->vtx_ids[0];
                }
                *packet++ = 3;
                *packet++ = 2;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                src += D_80094AEC[mode];
            }
        }
        *packet++ = 0;
    }

    cur = ((u32)cur & 3) ? cur + 2 : cur;
    arena_UpdatePeak((s32)cur);
    if (*src != 0) {
        func_80052C10(D_80010D8C);
    }
    softbody_SetAnchorWeights(init.in.points, init.in.count, cmds);

    init.in.matrix = &obj->node.xf.mat;
    init.in.flags = 0xE00;
    rec->slot_index = softbody_Create(&init.in);
    obj->softbody_nodes = init.in.buf;
    rec->inner.count = 0;
    rec->inner.coll.count = 0;
    cur = ((u32)cur & 3) ? cur + 2 : cur;
    rec->step_desc.slot_index = rec->slot_index;
    rec->step_desc.unk4 = &obj->node.xf.mat;
    rec->step_desc.unk8 = obj->softbody_nodes;
    rec->step_desc.colliders = &rec->inner.coll;
    rec->step_desc.snap = 0;
    rec->step_desc.unk3 = 0;
    return cur;
}

s16 *softbody_SetAnchorWeights(SVECTOR *a0, s32 a1, s16 *a2) {
    /* the point loop counter, then each point index read from a2, the way
     * SOTN's EntityBladeMaster runs its i as a loop counter and then reads it
     * from a table as a subscript (Q51) */
    /* SOTN: src/st/are/e_blade_master.c:485 @db41b28 */
    s32 i;
    for (i = 0; i < a1; i++) {
        a0[i].pad = -256;
    }

    {
        s32 count;
        count = a2[0];
        a2++;
        if (count >= 0) {
            do {
                int val;
                val = a2[0];
                a2++;
                count--;
                while (count != -1) {
                    i = a2[0];
                    a2++;
                    count--;
                    a0[i].pad = val;
                }
                count = a2[0];
                a2++;
            } while (count >= 0);
        }
    }
    return a2;
}

void softbody_AddColliders(Unk80045878Obj *a0, Unk8003F824Rec *rec, s16 *a2) {
    Func8003F6D8Inner *in = &rec->inner;
    s32 n = in->count;
    s16 id = a2[0];

    if (id != -2) {
        do {
            a2++;
            in->objs[n] = &a0->transform_nodes[id + 1];
            in->f84[n][0].vx = *a2++;
            in->f84[n][0].vy = *a2++;
            in->f84[n][0].vz = *a2++;
            in->f84[n][1].vx = *a2++;
            in->f84[n][1].vy = *a2++;
            in->f84[n][1].vz = *a2++;
            in->coll.unk68[n] = *a2++;
            n++;
            id = *a2;
        } while (id != -2);
    }
    in->count = n;
    in->coll.count = n;
}

s32 math_AlignUp4(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}

void softbody_RequestRebuild(Unk80045878Obj *a0) {
    Unk8003F62CRec *v1 = a0->unk_24;
    if (v1) {
        v1->rebuild_request = 1;
    }
}

void softbody_RequestSnap(s32 a0) {
    Unk80045878Obj *v0 = slot_GetObject(a0);
    if (v0) {
        Unk8003F62CRec *v1 = v0->unk_24;
        if (v1) {
            v1->snap_request = 1;
        }
    }
}

void softbody_SetObjectSnap(Unk8003F62CRec *a0) {
    s32 i;
    for (i = 0; i < a0->count; i++) {
        a0->step_descs[i].snap = 1;
        a0->recs[i].step_desc.snap = 1;
    }
}

void softbody_ClearObjectSnap(Unk8003F62CRec *a0) {
    s32 i;
    for (i = 0; i < a0->count; i++) {
        a0->step_descs[i].snap = 0;
        a0->recs[i].step_desc.snap = 0;
    }
}

void softbody_SetObjectGround(Unk80045878Obj *a0, s32 a1) {
    Unk8003F62CRec *v1 = a0->unk_24;
    if (v1) {
        s32 i;
        for (i = 0; i < v1->count; i++) {
            v1->recs[i].inner.coll.unk00 = a1;
        }
    }
}

void softbody_FreeObject(Unk80045878Obj *a0) {
    Unk8003F62CRec *s2;
    s32 s0;
    s2 = a0->unk_24;
    if (s2 != 0) {
        for (s0 = 0; s0 < s2->count; s0++) {
            softbody_Free(s2->recs[s0].slot_index);
        }
    }
}

void softbody_FreeObjectById(s32 a0) {
    Unk80045878Obj *v0 = slot_GetObject(a0);
    if (v0) {
        softbody_FreeObject(v0);
    }
}

void softbody_RebaseObject(Unk80045878Obj *a0, s32 a1) {
    Unk8003F62CRec *v1 = a0->unk_24;
    if (v1) {
        v1 = (Unk8003F62CRec *)((u8 *)v1 + a1);
        a0->unk_24 = v1;
        a0->softbody_params = a0->softbody_params + a1;
        v1->rebuild_request = 1;
    }
}

/* .data 0x8008F220..0x800905F8, the head of this file's .data (28708.c's
 * .data precedes; g_gpu_last_fog_ot_index .. D_80094AEC in the data-only
 * d_80DF8.c follow):
 * the fight command streams D_8008F220 .. D_8009000C (18 byte streams of
 * 188..224 bytes; D_8008F220 also fills the unused slots) and their pointer
 * table D_800900EC, which fight_InitCommandStream indexes. Only that table
 * points at the streams, and only this file reads it. Defined after every
 * function, so no body sees a definition where it saw an extern. */
u8 D_8008F220[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x19,
    0x0A, 0x01, 0x01, 0x0F, 0x10, 0x05, 0x10, 0x78, 0x00, 0xFF, 0x12, 0x11,
    0x1A, 0x02, 0x50, 0x13, 0x01, 0x0E, 0x03, 0x00, 0x12, 0x13, 0x02, 0x06,
    0x09, 0x00, 0x03, 0x07, 0xFF, 0x13, 0x03, 0x18, 0x07, 0x03, 0x01, 0x10,
    0x02, 0x07, 0x06, 0x10, 0xFF, 0x30, 0x03, 0x1B, 0x05, 0x60, 0x04, 0x12,
    0x00, 0x03, 0x05, 0x02, 0x0C, 0x00, 0x06, 0x00, 0x05, 0x01, 0x07, 0x0A,
    0x00, 0x01, 0x08, 0x08, 0x07, 0x00, 0x01, 0x12, 0x10, 0x05, 0x20, 0xC8,
    0xC8, 0xC8, 0x12, 0x11, 0x10, 0x00, 0x70, 0x13, 0x09, 0x12, 0x03, 0x03,
    0x12, 0x13, 0x0A, 0x14, 0x0D, 0x03, 0x01, 0x13, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x12, 0x07, 0x03, 0x12, 0x13, 0x0C, 0x14, 0x06, 0x03, 0x01,
    0x03, 0x03, 0x0C, 0xFF, 0x13, 0x0D, 0x1B, 0x05, 0x03, 0x02, 0x0C, 0x06,
    0x20, 0x89, 0x00, 0x00, 0x11, 0x01, 0xA0, 0x0E, 0x11, 0x0D, 0x03, 0x0F,
    0x05, 0x09, 0x03, 0x10, 0x00, 0x05, 0x03, 0x11, 0x08, 0x03, 0x03, 0x12,
    0x12, 0x01, 0x03, 0x01, 0x0C, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x14, 0x03, 0x03, 0x12, 0x13, 0x14,
    0x09, 0x0D, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x15,
    0x04, 0x03, 0x12, 0x03, 0x01, 0xFF, 0x13, 0x16, 0x24, 0x00, 0x83, 0x00,
};
u8 D_8008F2EC[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x18,
    0x0D, 0x01, 0x01, 0x06, 0x10, 0x05, 0x10, 0xBB, 0x59, 0x01, 0x12, 0x11,
    0x15, 0x05, 0x50, 0x13, 0x01, 0x0F, 0x02, 0x03, 0x12, 0x13, 0x02, 0x0A,
    0x0B, 0x03, 0x03, 0x0B, 0xFF, 0x13, 0x03, 0x18, 0x09, 0x00, 0x01, 0x07,
    0x02, 0x0B, 0x06, 0x10, 0x89, 0x00, 0x00, 0x11, 0x02, 0x60, 0x04, 0x0E,
    0x0D, 0x03, 0x05, 0x06, 0x03, 0x01, 0x06, 0x01, 0x05, 0x03, 0x07, 0x08,
    0x04, 0x02, 0x08, 0x07, 0x08, 0x01, 0x01, 0x09, 0x11, 0x16, 0x07, 0x90,
    0x13, 0x09, 0x12, 0x08, 0x03, 0x12, 0x13, 0x0A, 0x13, 0x0D, 0x00, 0x01,
    0x0A, 0x10, 0x05, 0x20, 0x71, 0x01, 0x96, 0x12, 0x11, 0x18, 0x01, 0x70,
    0x13, 0x0B, 0x0C, 0x0D, 0x03, 0x12, 0x13, 0x0C, 0x0B, 0x0E, 0x00, 0x01,
    0x16, 0x03, 0x06, 0xFF, 0x13, 0x0D, 0x1A, 0x0D, 0x03, 0x02, 0x06, 0x06,
    0x20, 0xC8, 0xC8, 0xC8, 0x10, 0x00, 0xA0, 0x0E, 0x14, 0x02, 0x03, 0x0F,
    0x0A, 0x0D, 0x01, 0x10, 0x01, 0x05, 0x01, 0x11, 0x04, 0x04, 0x03, 0x12,
    0x14, 0x04, 0x01, 0x01, 0x17, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x10, 0x0A, 0x03, 0x12, 0x13, 0x14,
    0x0B, 0x04, 0x01, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x15,
    0x09, 0x01, 0x12, 0x13, 0x16, 0x24, 0x01, 0x83, 0x00, 0x00, 0x00, 0x00,
};
u8 D_8008F3B8[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x18,
    0x0E, 0x03, 0x01, 0x19, 0x10, 0x05, 0x10, 0x6B, 0x66, 0x1B, 0x12, 0x11,
    0x17, 0x02, 0x50, 0x13, 0x01, 0x0F, 0x0B, 0x03, 0x12, 0x13, 0x02, 0x0A,
    0x03, 0x00, 0x03, 0x0A, 0xFF, 0x13, 0x03, 0x17, 0x0A, 0x02, 0x01, 0x1A,
    0x02, 0x0A, 0x06, 0x10, 0xC8, 0x1E, 0x1E, 0x19, 0x01, 0x60, 0x04, 0x11,
    0x09, 0x03, 0x05, 0x07, 0x07, 0x01, 0x06, 0x05, 0x01, 0x01, 0x07, 0x05,
    0x06, 0x02, 0x08, 0x05, 0x07, 0x02, 0x01, 0x14, 0x10, 0x05, 0x20, 0xC8,
    0xC8, 0xC8, 0x12, 0x11, 0x10, 0x05, 0x70, 0x13, 0x09, 0x0E, 0x01, 0x03,
    0x12, 0x13, 0x0A, 0x15, 0x03, 0x03, 0x01, 0x1C, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x11, 0x06, 0x03, 0x12, 0x13, 0x0C, 0x15, 0x05, 0x03, 0x01,
    0x06, 0x03, 0x09, 0xFF, 0x13, 0x0D, 0x19, 0x0D, 0x03, 0x02, 0x09, 0x06,
    0x20, 0x89, 0x00, 0x00, 0x11, 0x01, 0xA0, 0x0E, 0x0E, 0x05, 0x03, 0x0F,
    0x03, 0x03, 0x03, 0x10, 0x05, 0x02, 0x03, 0x11, 0x01, 0x03, 0x03, 0x12,
    0x0B, 0x03, 0x03, 0x01, 0x1F, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x12, 0x06, 0x03, 0x12, 0x13, 0x14,
    0x03, 0x06, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x16,
    0x09, 0x03, 0x12, 0x03, 0x01, 0xFF, 0x13, 0x16, 0x24, 0x02, 0x83, 0x00,
};
u8 D_8008F484[188] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x17,
    0x03, 0x03, 0x01, 0x19, 0x10, 0x05, 0x10, 0xFF, 0x30, 0x03, 0x12, 0x11,
    0x1B, 0x02, 0x50, 0x13, 0x01, 0x13, 0x00, 0x03, 0x12, 0x13, 0x02, 0x05,
    0x0A, 0x03, 0x01, 0x1A, 0x10, 0x06, 0x10, 0x89, 0x00, 0x00, 0x12, 0x11,
    0x11, 0x05, 0x60, 0x13, 0x03, 0x11, 0x05, 0x03, 0x12, 0x13, 0x04, 0x0A,
    0x01, 0x03, 0x01, 0x1C, 0x10, 0x05, 0x20, 0xC8, 0x1E, 0x1E, 0x12, 0x11,
    0x19, 0x01, 0x70, 0x13, 0x05, 0x0D, 0x02, 0x03, 0x12, 0x13, 0x06, 0x07,
    0x0A, 0x03, 0x01, 0x0B, 0x11, 0x16, 0x07, 0x80, 0x13, 0x07, 0x0C, 0x08,
    0x03, 0x12, 0x13, 0x08, 0x13, 0x0F, 0x03, 0x03, 0x01, 0xFF, 0x13, 0x09,
    0x1A, 0x05, 0x03, 0x01, 0x16, 0x02, 0x01, 0x06, 0x20, 0xC8, 0xC8, 0xC8,
    0x10, 0x00, 0xA0, 0x0A, 0x0F, 0x05, 0x03, 0x0B, 0x0A, 0x0E, 0x03, 0x0C,
    0x02, 0x0D, 0x03, 0x0D, 0x04, 0x04, 0x03, 0x0E, 0x04, 0x0D, 0x03, 0x01,
    0x17, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12, 0x11, 0x0F, 0x03, 0xB0,
    0x13, 0x0F, 0x10, 0x02, 0x03, 0x12, 0x13, 0x10, 0x0A, 0x02, 0x03, 0x01,
    0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x11, 0x15, 0x0B, 0x03, 0x12, 0x13,
    0x12, 0x23, 0x06, 0x83, 0x00, 0x00, 0x00, 0x00,
};
u8 D_8008F540[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x18,
    0x06, 0x03, 0x01, 0x19, 0x10, 0x05, 0x10, 0x6B, 0x66, 0x1B, 0x12, 0x11,
    0x17, 0x05, 0x50, 0x13, 0x01, 0x10, 0x0C, 0x03, 0x12, 0x13, 0x02, 0x01,
    0x09, 0x03, 0x03, 0x00, 0xFF, 0x13, 0x03, 0x1A, 0x01, 0x03, 0x01, 0x1A,
    0x02, 0x00, 0x06, 0x10, 0xFF, 0x30, 0x03, 0x1B, 0x02, 0x60, 0x04, 0x13,
    0x01, 0x03, 0x05, 0x03, 0x05, 0x03, 0x06, 0x03, 0x00, 0x03, 0x07, 0x03,
    0x02, 0x03, 0x08, 0x09, 0x09, 0x03, 0x01, 0x1C, 0x10, 0x05, 0x20, 0x71,
    0x01, 0x96, 0x12, 0x11, 0x18, 0x01, 0x70, 0x13, 0x09, 0x13, 0x07, 0x03,
    0x12, 0x13, 0x0A, 0x07, 0x05, 0x03, 0x01, 0x16, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x0F, 0x0E, 0x03, 0x12, 0x13, 0x0C, 0x02, 0x05, 0x03, 0x03,
    0x0B, 0xFF, 0x13, 0x0D, 0x17, 0x07, 0x03, 0x01, 0x15, 0x02, 0x0B, 0x06,
    0x20, 0xC8, 0xC8, 0xC8, 0x10, 0x00, 0xA0, 0x0E, 0x10, 0x0D, 0x03, 0x0F,
    0x07, 0x0D, 0x03, 0x10, 0x02, 0x0B, 0x03, 0x11, 0x04, 0x04, 0x03, 0x12,
    0x06, 0x0B, 0x03, 0x01, 0x0C, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x11, 0x02, 0x03, 0x12, 0x13, 0x14,
    0x06, 0x0C, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x14,
    0x00, 0x03, 0x12, 0x03, 0x06, 0xFF, 0x13, 0x16, 0x23, 0x01, 0x81, 0x00,
};
u8 D_8008F60C[188] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x18,
    0x08, 0x03, 0x01, 0x06, 0x10, 0x05, 0x10, 0xBB, 0x59, 0x01, 0x12, 0x11,
    0x15, 0x01, 0x50, 0x13, 0x01, 0x10, 0x0E, 0x03, 0x12, 0x13, 0x02, 0x07,
    0x00, 0x03, 0x01, 0x07, 0x10, 0x06, 0x10, 0xFF, 0x30, 0x03, 0x12, 0x11,
    0x1B, 0x02, 0x60, 0x13, 0x03, 0x13, 0x08, 0x03, 0x12, 0x13, 0x04, 0x07,
    0x0E, 0x03, 0x01, 0x09, 0x10, 0x05, 0x20, 0xC8, 0xC8, 0xC8, 0x12, 0x11,
    0x10, 0x05, 0x70, 0x13, 0x05, 0x0D, 0x00, 0x03, 0x12, 0x13, 0x06, 0x00,
    0x0A, 0x03, 0x01, 0x1D, 0x11, 0x16, 0x07, 0x80, 0x13, 0x07, 0x11, 0x0B,
    0x03, 0x12, 0x13, 0x08, 0x14, 0x08, 0x03, 0x01, 0x03, 0x03, 0x02, 0xFF,
    0x13, 0x09, 0x1A, 0x03, 0x03, 0x02, 0x02, 0x06, 0x20, 0x89, 0x00, 0x00,
    0x11, 0x01, 0xA0, 0x0A, 0x0D, 0x07, 0x03, 0x0B, 0x01, 0x0D, 0x03, 0x0C,
    0x03, 0x08, 0x03, 0x0D, 0x01, 0x0E, 0x03, 0x0E, 0x01, 0x04, 0x03, 0x01,
    0x17, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12, 0x11, 0x0F, 0x03, 0xB0,
    0x13, 0x0F, 0x11, 0x01, 0x03, 0x12, 0x13, 0x10, 0x02, 0x00, 0x03, 0x01,
    0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x11, 0x14, 0x0C, 0x03, 0x12, 0x03,
    0x06, 0xFF, 0x13, 0x12, 0x23, 0x05, 0x83, 0x00,
};
u8 D_8008F6C8[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x17,
    0x06, 0x03, 0x01, 0x18, 0x10, 0x05, 0x10, 0xFF, 0x30, 0x03, 0x12, 0x11,
    0x1B, 0x01, 0x50, 0x13, 0x01, 0x0D, 0x08, 0x03, 0x12, 0x13, 0x02, 0x00,
    0x0C, 0x03, 0x03, 0x0C, 0xFF, 0x13, 0x03, 0x1B, 0x0E, 0x83, 0x01, 0x0E,
    0x02, 0x0C, 0x06, 0x10, 0x71, 0x01, 0x96, 0x18, 0x05, 0x60, 0x04, 0x0E,
    0x06, 0x03, 0x05, 0x04, 0x00, 0x03, 0x06, 0x05, 0x08, 0x03, 0x07, 0x02,
    0x03, 0x03, 0x08, 0x04, 0x03, 0x03, 0x01, 0x05, 0x10, 0x05, 0x20, 0xBB,
    0x59, 0x01, 0x12, 0x11, 0x15, 0x03, 0x70, 0x13, 0x09, 0x0D, 0x09, 0x03,
    0x12, 0x13, 0x0A, 0x09, 0x0C, 0x03, 0x01, 0x0F, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x0F, 0x01, 0x03, 0x12, 0x13, 0x0C, 0x03, 0x0D, 0x03, 0x03,
    0x02, 0xFF, 0x13, 0x0D, 0x18, 0x00, 0x03, 0x01, 0x10, 0x02, 0x02, 0x06,
    0x20, 0xC8, 0x1E, 0x1E, 0x19, 0x02, 0xA0, 0x0E, 0x10, 0x01, 0x03, 0x0F,
    0x09, 0x05, 0x03, 0x10, 0x05, 0x00, 0x03, 0x11, 0x02, 0x08, 0x03, 0x12,
    0x0C, 0x04, 0x03, 0x01, 0x0C, 0x10, 0x05, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x0F, 0x0C, 0x03, 0x12, 0x13, 0x14,
    0x05, 0x0E, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x14,
    0x09, 0x03, 0x12, 0x03, 0x02, 0xFF, 0x13, 0x16, 0x22, 0x04, 0x83, 0x00,
};
u8 D_8008F794[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x18,
    0x0C, 0x03, 0x01, 0x0F, 0x10, 0x05, 0x10, 0x78, 0x00, 0xFF, 0x12, 0x11,
    0x1A, 0x05, 0x50, 0x13, 0x01, 0x0E, 0x02, 0x03, 0x12, 0x13, 0x02, 0x07,
    0x06, 0x03, 0x03, 0x01, 0xFF, 0x13, 0x03, 0x19, 0x09, 0x03, 0x01, 0x07,
    0x02, 0x01, 0x06, 0x10, 0x89, 0x00, 0x00, 0x11, 0x02, 0x60, 0x04, 0x10,
    0x07, 0x03, 0x05, 0x04, 0x02, 0x03, 0x06, 0x00, 0x09, 0x03, 0x07, 0x00,
    0x06, 0x03, 0x08, 0x04, 0x01, 0x03, 0x01, 0x10, 0x10, 0x05, 0x20, 0xC8,
    0x1E, 0x1E, 0x12, 0x11, 0x19, 0x01, 0x70, 0x13, 0x09, 0x10, 0x08, 0x03,
    0x12, 0x13, 0x0A, 0x09, 0x0E, 0x03, 0x01, 0x12, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x11, 0x0A, 0x03, 0x12, 0x13, 0x0C, 0x03, 0x07, 0x03, 0x03,
    0x07, 0xFF, 0x13, 0x0D, 0x19, 0x05, 0x03, 0x01, 0x15, 0x02, 0x07, 0x06,
    0x20, 0xC8, 0xC8, 0xC8, 0x10, 0x00, 0xA0, 0x0E, 0x0F, 0x00, 0x03, 0x0F,
    0x01, 0x0C, 0x03, 0x10, 0x00, 0x0B, 0x03, 0x11, 0x04, 0x04, 0x03, 0x12,
    0x00, 0x00, 0x03, 0x01, 0x16, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x10, 0x00, 0x03, 0x12, 0x13, 0x14,
    0x04, 0x0A, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x14,
    0x01, 0x03, 0x12, 0x03, 0x07, 0xFF, 0x13, 0x16, 0x22, 0x03, 0x83, 0x00,
};
u8 D_8008F860[204] = {
    0x01, 0x08, 0x10, 0x07, 0x10, 0x03, 0x33, 0x24, 0x12, 0x13, 0x00, 0x19,
    0x03, 0x03, 0x01, 0x18, 0x10, 0x05, 0x10, 0x6B, 0x66, 0x1B, 0x12, 0x11,
    0x17, 0x05, 0x50, 0x13, 0x01, 0x0F, 0x03, 0x03, 0x12, 0x13, 0x02, 0x02,
    0x07, 0x03, 0x03, 0x0A, 0xFF, 0x13, 0x03, 0x1C, 0x04, 0x03, 0x01, 0x05,
    0x02, 0x0A, 0x06, 0x10, 0x71, 0x01, 0x96, 0x18, 0x02, 0x60, 0x04, 0x11,
    0x07, 0x03, 0x05, 0x09, 0x01, 0x03, 0x06, 0x08, 0x0B, 0x03, 0x07, 0x02,
    0x03, 0x03, 0x08, 0x0A, 0x0C, 0x03, 0x01, 0x11, 0x10, 0x05, 0x20, 0xC8,
    0x1E, 0x1E, 0x12, 0x11, 0x19, 0x03, 0x70, 0x13, 0x09, 0x0E, 0x0E, 0x03,
    0x12, 0x13, 0x0A, 0x0A, 0x08, 0x03, 0x01, 0x0A, 0x11, 0x16, 0x07, 0x80,
    0x13, 0x0B, 0x12, 0x0D, 0x03, 0x12, 0x13, 0x0C, 0x16, 0x07, 0x03, 0x01,
    0x03, 0x03, 0x00, 0xFF, 0x13, 0x0D, 0x19, 0x0B, 0x03, 0x02, 0x00, 0x06,
    0x20, 0x89, 0x00, 0x00, 0x11, 0x01, 0xA0, 0x0E, 0x0D, 0x0E, 0x03, 0x0F,
    0x05, 0x0D, 0x03, 0x10, 0x08, 0x0C, 0x03, 0x11, 0x01, 0x01, 0x03, 0x12,
    0x09, 0x02, 0x03, 0x01, 0x1F, 0x10, 0x04, 0x30, 0x1E, 0x1E, 0x1E, 0x12,
    0x11, 0x0F, 0x03, 0xB0, 0x13, 0x13, 0x0F, 0x04, 0x03, 0x12, 0x13, 0x14,
    0x09, 0x03, 0x03, 0x01, 0x02, 0x11, 0x1C, 0x00, 0xC0, 0x13, 0x15, 0x13,
    0x02, 0x03, 0x12, 0x03, 0x00, 0xFF, 0x13, 0x16, 0x23, 0x04, 0x83, 0x00,
};
u8 D_8008F92C[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x17, 0x0B,
    0x00, 0x12, 0x13, 0x01, 0x16, 0x0B, 0x03, 0x01, 0x16, 0x10, 0x04, 0x10,
    0x4A, 0x00, 0x89, 0x12, 0x11, 0x0C, 0x03, 0x50, 0x13, 0x02, 0x15, 0x01,
    0x03, 0x12, 0x13, 0x03, 0x00, 0x03, 0x00, 0x03, 0x1B, 0xFF, 0x13, 0x04,
    0x19, 0x04, 0x00, 0x01, 0x15, 0x02, 0x1B, 0x05, 0x10, 0x71, 0x82, 0x02,
    0x01, 0x01, 0x60, 0x05, 0x11, 0x0C, 0x03, 0x06, 0x08, 0x08, 0x02, 0x07,
    0x02, 0x0E, 0x01, 0x08, 0x02, 0x04, 0x03, 0x09, 0x06, 0x06, 0x03, 0x01,
    0x11, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x10, 0x0B, 0x03, 0x12, 0x13,
    0x0B, 0x16, 0x06, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x03, 0x80, 0x13, 0x0C, 0x13, 0x0A, 0x03, 0x12, 0x13,
    0x0D, 0x0F, 0x0D, 0x13, 0x01, 0x0D, 0x03, 0x18, 0xFF, 0x13, 0x0E, 0x1C,
    0x05, 0x13, 0x02, 0x18, 0x05, 0x30, 0x89, 0x00, 0x47, 0x02, 0x04, 0xA0,
    0x0F, 0x15, 0x0A, 0x03, 0x10, 0x09, 0x0B, 0x03, 0x11, 0x02, 0x0E, 0x03,
    0x12, 0x04, 0x0B, 0x03, 0x13, 0x0A, 0x09, 0x03, 0x01, 0x0E, 0x10, 0x04,
    0x30, 0xFF, 0x82, 0x04, 0x12, 0x11, 0x06, 0x00, 0xB0, 0x13, 0x14, 0x11,
    0x08, 0x03, 0x12, 0x13, 0x15, 0x03, 0x04, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x15, 0x08, 0x43, 0x12, 0x13, 0x17, 0x0B, 0x09,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x23, 0x02, 0x13, 0x19,
    0x1A, 0x08, 0x93, 0x1A, 0x19, 0x0E, 0x93, 0x00,
};
u8 D_8008FA0C[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x18, 0x01,
    0x03, 0x12, 0x13, 0x01, 0x17, 0x05, 0x03, 0x01, 0x16, 0x10, 0x04, 0x10,
    0x2C, 0x26, 0xAD, 0x12, 0x11, 0x0A, 0x01, 0x50, 0x13, 0x02, 0x16, 0x01,
    0x03, 0x12, 0x13, 0x03, 0x00, 0x01, 0x01, 0x03, 0x17, 0xFF, 0x13, 0x04,
    0x18, 0x05, 0x01, 0x01, 0x1E, 0x02, 0x17, 0x05, 0x10, 0x71, 0x82, 0x02,
    0x01, 0x03, 0x60, 0x05, 0x0E, 0x00, 0x03, 0x06, 0x01, 0x02, 0x00, 0x07,
    0x0D, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x03, 0x09, 0x0B, 0x0C, 0x03, 0x01,
    0x1B, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x13, 0x03, 0x03, 0x12, 0x13,
    0x0B, 0x16, 0x03, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x02, 0x80, 0x13, 0x0C, 0x0C, 0x07, 0x03, 0x12, 0x13,
    0x0D, 0x0C, 0x0C, 0x13, 0x03, 0x19, 0xFF, 0x13, 0x0E, 0x1A, 0x07, 0x13,
    0x01, 0x06, 0x02, 0x19, 0x05, 0x30, 0x4A, 0x00, 0x89, 0x0C, 0x00, 0xA0,
    0x0F, 0x13, 0x05, 0x03, 0x10, 0x03, 0x0C, 0x03, 0x11, 0x0B, 0x0D, 0x03,
    0x12, 0x07, 0x0C, 0x03, 0x13, 0x0A, 0x07, 0x03, 0x01, 0x05, 0x10, 0x04,
    0x30, 0x89, 0x00, 0x47, 0x12, 0x11, 0x02, 0x04, 0xB0, 0x13, 0x14, 0x13,
    0x0B, 0x03, 0x12, 0x13, 0x15, 0x01, 0x0A, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x12, 0x05, 0x43, 0x12, 0x13, 0x17, 0x0E, 0x04,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x22, 0x02, 0x93, 0x19,
    0x1B, 0x0D, 0x93, 0x1A, 0x18, 0x0B, 0x93, 0x00,
};
u8 D_8008FAEC[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x18, 0x0A,
    0x00, 0x12, 0x13, 0x01, 0x16, 0x0E, 0x01, 0x01, 0x16, 0x10, 0x04, 0x10,
    0xFF, 0x1E, 0x00, 0x12, 0x11, 0x07, 0x03, 0x50, 0x13, 0x02, 0x11, 0x03,
    0x03, 0x12, 0x13, 0x03, 0x02, 0x02, 0x01, 0x03, 0x1A, 0xFF, 0x13, 0x04,
    0x19, 0x07, 0x01, 0x01, 0x0A, 0x02, 0x1A, 0x05, 0x10, 0x71, 0x82, 0x02,
    0x01, 0x01, 0x60, 0x05, 0x10, 0x06, 0x03, 0x06, 0x04, 0x07, 0x01, 0x07,
    0x01, 0x01, 0x01, 0x08, 0x04, 0x06, 0x03, 0x09, 0x08, 0x02, 0x00, 0x01,
    0x08, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x11, 0x04, 0x03, 0x12, 0x13,
    0x0B, 0x15, 0x0F, 0x01, 0x01, 0x01, 0x10, 0x03, 0x30, 0x89, 0x00, 0x47,
    0x12, 0x11, 0x02, 0x00, 0x80, 0x13, 0x0C, 0x12, 0x0A, 0x03, 0x12, 0x13,
    0x0D, 0x01, 0x08, 0x11, 0x03, 0x15, 0xFF, 0x13, 0x0E, 0x1B, 0x08, 0x13,
    0x01, 0x18, 0x02, 0x15, 0x05, 0x30, 0x15, 0x80, 0x22, 0x09, 0x04, 0xA0,
    0x0F, 0x0B, 0x00, 0x03, 0x10, 0x03, 0x0B, 0x01, 0x11, 0x01, 0x00, 0x01,
    0x12, 0x0B, 0x01, 0x03, 0x13, 0x0B, 0x05, 0x01, 0x01, 0x0E, 0x10, 0x04,
    0x30, 0x01, 0x01, 0xB1, 0x12, 0x11, 0x00, 0x02, 0xB0, 0x13, 0x14, 0x15,
    0x0E, 0x03, 0x12, 0x13, 0x15, 0x04, 0x08, 0x01, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x12, 0x0E, 0x43, 0x12, 0x13, 0x17, 0x0C, 0x03,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x23, 0x00, 0x91, 0x19,
    0x1C, 0x08, 0x90, 0x1A, 0x16, 0x00, 0x90, 0x00,
};
u8 D_8008FBCC[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x17, 0x0D,
    0x03, 0x12, 0x13, 0x01, 0x16, 0x0D, 0x03, 0x01, 0x16, 0x10, 0x04, 0x10,
    0x4B, 0x0B, 0x0B, 0x12, 0x11, 0x0B, 0x04, 0x50, 0x13, 0x02, 0x13, 0x09,
    0x03, 0x12, 0x13, 0x03, 0x03, 0x01, 0x03, 0x03, 0x1B, 0xFF, 0x13, 0x04,
    0x1A, 0x0B, 0x03, 0x01, 0x1D, 0x02, 0x1B, 0x05, 0x10, 0x89, 0x00, 0x47,
    0x02, 0x01, 0x60, 0x05, 0x0D, 0x0C, 0x03, 0x06, 0x03, 0x09, 0x03, 0x07,
    0x02, 0x09, 0x03, 0x08, 0x05, 0x01, 0x03, 0x09, 0x0B, 0x08, 0x03, 0x01,
    0x1A, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x11, 0x0E, 0x03, 0x12, 0x13,
    0x0B, 0x14, 0x0A, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x03, 0x80, 0x13, 0x0C, 0x12, 0x04, 0x03, 0x12, 0x13,
    0x0D, 0x06, 0x07, 0x13, 0x01, 0x0D, 0x03, 0x11, 0xFF, 0x13, 0x0E, 0x1A,
    0x09, 0x13, 0x02, 0x11, 0x05, 0x30, 0x15, 0x80, 0x22, 0x09, 0x02, 0xA0,
    0x0F, 0x0F, 0x06, 0x03, 0x10, 0x00, 0x04, 0x03, 0x11, 0x02, 0x0A, 0x03,
    0x12, 0x03, 0x08, 0x03, 0x13, 0x07, 0x0B, 0x03, 0x01, 0x18, 0x10, 0x04,
    0x30, 0x71, 0x82, 0x02, 0x12, 0x11, 0x01, 0x00, 0xB0, 0x13, 0x14, 0x0E,
    0x0A, 0x03, 0x12, 0x13, 0x15, 0x0B, 0x0B, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x16, 0x02, 0x43, 0x12, 0x13, 0x17, 0x0D, 0x04,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x22, 0x01, 0x93, 0x19,
    0x1A, 0x06, 0x93, 0x1A, 0x17, 0x00, 0x93, 0x00,
};
u8 D_8008FCAC[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x17, 0x0C,
    0x03, 0x12, 0x13, 0x01, 0x16, 0x0C, 0x03, 0x01, 0x16, 0x10, 0x04, 0x10,
    0x89, 0x00, 0x47, 0x12, 0x11, 0x02, 0x03, 0x50, 0x13, 0x02, 0x0B, 0x0A,
    0x03, 0x12, 0x13, 0x03, 0x00, 0x08, 0x03, 0x03, 0x10, 0xFF, 0x13, 0x04,
    0x19, 0x00, 0x03, 0x01, 0x1E, 0x02, 0x10, 0x05, 0x10, 0x71, 0x82, 0x02,
    0x01, 0x00, 0x60, 0x05, 0x10, 0x05, 0x03, 0x06, 0x0C, 0x01, 0x03, 0x07,
    0x03, 0x0A, 0x03, 0x08, 0x0B, 0x0F, 0x03, 0x09, 0x08, 0x01, 0x03, 0x01,
    0x08, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x0E, 0x08, 0x03, 0x12, 0x13,
    0x0B, 0x16, 0x08, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x03, 0x80, 0x13, 0x0C, 0x15, 0x06, 0x03, 0x12, 0x13,
    0x0D, 0x15, 0x0C, 0x13, 0x03, 0x1A, 0xFF, 0x13, 0x0E, 0x19, 0x02, 0x13,
    0x01, 0x05, 0x02, 0x1A, 0x05, 0x30, 0x4B, 0x0B, 0x0B, 0x0B, 0x01, 0xA0,
    0x0F, 0x0D, 0x03, 0x03, 0x10, 0x04, 0x0E, 0x03, 0x11, 0x02, 0x01, 0x03,
    0x12, 0x00, 0x09, 0x03, 0x13, 0x0A, 0x06, 0x03, 0x01, 0x06, 0x10, 0x04,
    0x30, 0xFF, 0x82, 0x04, 0x12, 0x11, 0x06, 0x02, 0xB0, 0x13, 0x14, 0x12,
    0x0B, 0x03, 0x12, 0x13, 0x15, 0x01, 0x07, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x14, 0x07, 0x43, 0x12, 0x13, 0x17, 0x0C, 0x05,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x22, 0x00, 0x93, 0x19,
    0x1B, 0x03, 0x93, 0x1A, 0x16, 0x05, 0x93, 0x00,
};
u8 D_8008FD8C[208] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x18, 0x03,
    0x03, 0x12, 0x13, 0x01, 0x17, 0x02, 0x03, 0x01, 0x0B, 0x10, 0x04, 0x10,
    0xFF, 0x1E, 0x00, 0x12, 0x11, 0x07, 0x03, 0x50, 0x13, 0x02, 0x13, 0x06,
    0x03, 0x12, 0x13, 0x03, 0x07, 0x04, 0x03, 0x01, 0x0A, 0x10, 0x05, 0x10,
    0x15, 0x80, 0x22, 0x12, 0x11, 0x09, 0x01, 0x60, 0x13, 0x04, 0x11, 0x00,
    0x03, 0x12, 0x13, 0x05, 0x00, 0x0E, 0x03, 0x01, 0x09, 0x11, 0x08, 0x06,
    0x70, 0x13, 0x06, 0x10, 0x04, 0x03, 0x12, 0x13, 0x07, 0x15, 0x00, 0x03,
    0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1, 0x12, 0x11, 0x00, 0x02,
    0x80, 0x13, 0x08, 0x10, 0x09, 0x03, 0x12, 0x13, 0x09, 0x0D, 0x0B, 0x13,
    0x03, 0x0F, 0xFF, 0x13, 0x0A, 0x1A, 0x0C, 0x13, 0x01, 0x05, 0x02, 0x0F,
    0x05, 0x30, 0x89, 0x00, 0x47, 0x02, 0x04, 0xA0, 0x0B, 0x0E, 0x07, 0x03,
    0x0C, 0x0A, 0x0A, 0x03, 0x0D, 0x02, 0x03, 0x03, 0x0E, 0x05, 0x02, 0x03,
    0x0F, 0x0C, 0x09, 0x03, 0x01, 0x06, 0x10, 0x04, 0x30, 0x71, 0x82, 0x02,
    0x12, 0x11, 0x01, 0x00, 0xB0, 0x13, 0x10, 0x15, 0x02, 0x03, 0x12, 0x13,
    0x11, 0x0F, 0x08, 0x03, 0x01, 0x03, 0x11, 0x0D, 0x00, 0xC0, 0x13, 0x12,
    0x0F, 0x09, 0x43, 0x12, 0x13, 0x13, 0x08, 0x0D, 0x01, 0x01, 0x00, 0x03,
    0x1F, 0xFF, 0x14, 0x14, 0x22, 0x06, 0x93, 0x15, 0x1B, 0x00, 0x93, 0x16,
    0x19, 0x01, 0x93, 0x00,
};
u8 D_8008FE5C[208] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x18, 0x04,
    0x03, 0x12, 0x13, 0x01, 0x17, 0x01, 0x03, 0x01, 0x1D, 0x10, 0x04, 0x10,
    0x4B, 0x0B, 0x0B, 0x12, 0x11, 0x0B, 0x04, 0x50, 0x13, 0x02, 0x08, 0x05,
    0x03, 0x12, 0x13, 0x03, 0x0A, 0x05, 0x03, 0x01, 0x1C, 0x10, 0x05, 0x10,
    0xFF, 0x1E, 0x00, 0x12, 0x11, 0x07, 0x03, 0x60, 0x13, 0x04, 0x0E, 0x0C,
    0x03, 0x12, 0x13, 0x05, 0x07, 0x09, 0x03, 0x01, 0x10, 0x11, 0x08, 0x06,
    0x70, 0x13, 0x06, 0x0F, 0x0A, 0x03, 0x12, 0x13, 0x07, 0x12, 0x0C, 0x03,
    0x01, 0x01, 0x10, 0x03, 0x30, 0x89, 0x00, 0x47, 0x12, 0x11, 0x02, 0x00,
    0x80, 0x13, 0x08, 0x10, 0x03, 0x03, 0x12, 0x13, 0x09, 0x04, 0x05, 0x13,
    0x03, 0x10, 0xFF, 0x13, 0x0A, 0x19, 0x08, 0x13, 0x01, 0x06, 0x02, 0x10,
    0x05, 0x30, 0x4A, 0x00, 0x89, 0x0C, 0x01, 0xA0, 0x0B, 0x0D, 0x01, 0x03,
    0x0C, 0x08, 0x0E, 0x03, 0x0D, 0x02, 0x08, 0x03, 0x0E, 0x08, 0x0C, 0x03,
    0x0F, 0x08, 0x06, 0x03, 0x01, 0x18, 0x10, 0x04, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x02, 0xB0, 0x13, 0x10, 0x12, 0x09, 0x03, 0x12, 0x13,
    0x11, 0x06, 0x02, 0x03, 0x01, 0x03, 0x11, 0x0D, 0x00, 0xC0, 0x13, 0x12,
    0x12, 0x02, 0x43, 0x12, 0x13, 0x13, 0x00, 0x02, 0x01, 0x01, 0x00, 0x03,
    0x1F, 0xFF, 0x14, 0x14, 0x22, 0x07, 0x93, 0x15, 0x1A, 0x04, 0x93, 0x16,
    0x16, 0x0A, 0x93, 0x00,
};
u8 D_8008FF2C[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x18, 0x02,
    0x03, 0x12, 0x13, 0x01, 0x17, 0x09, 0x03, 0x01, 0x0B, 0x10, 0x04, 0x10,
    0x2C, 0x26, 0xAD, 0x12, 0x11, 0x0A, 0x01, 0x50, 0x13, 0x02, 0x0B, 0x06,
    0x03, 0x12, 0x13, 0x03, 0x03, 0x0E, 0x03, 0x03, 0x11, 0xFF, 0x13, 0x04,
    0x19, 0x0C, 0x03, 0x01, 0x13, 0x02, 0x11, 0x05, 0x10, 0x71, 0x82, 0x02,
    0x01, 0x00, 0x60, 0x05, 0x0C, 0x0B, 0x03, 0x06, 0x0B, 0x07, 0x03, 0x07,
    0x01, 0x06, 0x03, 0x08, 0x04, 0x06, 0x03, 0x09, 0x07, 0x01, 0x03, 0x01,
    0x10, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x14, 0x05, 0x03, 0x12, 0x13,
    0x0B, 0x16, 0x04, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x89, 0x00, 0x47,
    0x12, 0x11, 0x02, 0x04, 0x80, 0x13, 0x0C, 0x0E, 0x09, 0x03, 0x12, 0x13,
    0x0D, 0x09, 0x0A, 0x13, 0x01, 0x0D, 0x03, 0x17, 0xFF, 0x13, 0x0E, 0x19,
    0x06, 0x13, 0x02, 0x17, 0x05, 0x30, 0x01, 0x01, 0xB1, 0x00, 0x03, 0xA0,
    0x0F, 0x0D, 0x0A, 0x03, 0x10, 0x06, 0x0A, 0x03, 0x11, 0x01, 0x06, 0x03,
    0x12, 0x00, 0x05, 0x03, 0x13, 0x0C, 0x0E, 0x03, 0x01, 0x05, 0x10, 0x04,
    0x30, 0xFF, 0x82, 0x04, 0x12, 0x11, 0x06, 0x02, 0xB0, 0x13, 0x14, 0x0F,
    0x07, 0x03, 0x12, 0x13, 0x15, 0x09, 0x08, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x15, 0x0D, 0x43, 0x12, 0x13, 0x17, 0x0D, 0x06,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x22, 0x05, 0x93, 0x19,
    0x1A, 0x0A, 0x93, 0x1A, 0x15, 0x07, 0x93, 0x00,
};
u8 D_8009000C[224] = {
    0x01, 0x02, 0x10, 0x05, 0x10, 0x50, 0x11, 0x2D, 0x13, 0x00, 0x17, 0x0E,
    0x03, 0x12, 0x13, 0x01, 0x17, 0x04, 0x03, 0x01, 0x16, 0x10, 0x04, 0x10,
    0x2C, 0x26, 0xAD, 0x12, 0x11, 0x0A, 0x01, 0x50, 0x13, 0x02, 0x13, 0x0E,
    0x03, 0x12, 0x13, 0x03, 0x05, 0x0B, 0x03, 0x03, 0x15, 0xFF, 0x13, 0x04,
    0x1B, 0x04, 0x03, 0x01, 0x1D, 0x02, 0x15, 0x05, 0x10, 0x89, 0x00, 0x47,
    0x02, 0x04, 0x60, 0x05, 0x08, 0x09, 0x03, 0x06, 0x02, 0x06, 0x03, 0x07,
    0x05, 0x03, 0x03, 0x08, 0x05, 0x01, 0x03, 0x09, 0x06, 0x0D, 0x03, 0x01,
    0x09, 0x11, 0x08, 0x06, 0x70, 0x13, 0x0A, 0x13, 0x04, 0x03, 0x12, 0x13,
    0x0B, 0x14, 0x0F, 0x03, 0x01, 0x01, 0x10, 0x03, 0x30, 0x01, 0x01, 0xB1,
    0x12, 0x11, 0x00, 0x03, 0x80, 0x13, 0x0C, 0x0D, 0x05, 0x03, 0x12, 0x13,
    0x0D, 0x05, 0x0C, 0x13, 0x03, 0x0F, 0xFF, 0x13, 0x0E, 0x1A, 0x02, 0x13,
    0x01, 0x05, 0x02, 0x0F, 0x05, 0x30, 0x71, 0x82, 0x02, 0x01, 0x00, 0xA0,
    0x0F, 0x0E, 0x0B, 0x03, 0x10, 0x0C, 0x06, 0x03, 0x11, 0x03, 0x02, 0x03,
    0x12, 0x04, 0x06, 0x03, 0x13, 0x06, 0x0E, 0x03, 0x01, 0x06, 0x10, 0x04,
    0x30, 0xFF, 0x82, 0x04, 0x12, 0x11, 0x06, 0x02, 0xB0, 0x13, 0x14, 0x13,
    0x0C, 0x03, 0x12, 0x13, 0x15, 0x05, 0x04, 0x03, 0x01, 0x03, 0x11, 0x0D,
    0x00, 0xC0, 0x13, 0x16, 0x14, 0x0B, 0x43, 0x12, 0x13, 0x17, 0x0C, 0x02,
    0x01, 0x01, 0x00, 0x03, 0x1F, 0xFF, 0x14, 0x18, 0x23, 0x03, 0x93, 0x19,
    0x1B, 0x02, 0x93, 0x1A, 0x17, 0x08, 0x93, 0x00,
};
u8 *D_800900EC[32] = {
    D_8008F220, D_8008F2EC, D_8008F3B8, D_8008F484, D_8008F540, D_8008F220,
    D_8008F60C, D_8008F6C8, D_8008F794, D_8008F860, D_8008F220, D_8008F220,
    D_8008F92C, D_8008FA0C, D_8008FAEC, D_8008FBCC, D_8008F220, D_8008FCAC,
    D_8008FD8C, D_8008FE5C, D_8008FF2C, D_8009000C, D_8008F220, D_8008F220,
    D_8008F220, D_8008F220, D_8008F220, D_8008F220, D_8008F220, D_8008F220,
    D_8008F220, D_8008F220,
};
/* func_8003C040's per-mode values, passed to func_80054884; D_800A38A4 is
 * only ever 0..9. The 2 zero bytes after it are g_font_image's alignment. */
u8 D_8009016C[10] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0B, 0x0A, 0x0C, 0x0D,
};
/* The font image font_LoadImage uploads to g_font_rect (16 x 36 16-bit
 * pixels, 1152 bytes). */
u32 g_font_image[288] = {
    0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000,
    0xFFFF0000, 0xFFFF0000, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
    0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000,
    0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0x898D8901, 0x09D99999, 0x880333AA, 0x00027E62,
    0x03519113, 0x03A008AB, 0x008F66CC, 0x00089008, 0x88DCB810, 0x49DBFF99,
    0x08920896, 0x0007AF26, 0x4168CC21, 0x0298889E, 0x00AD98AC, 0x00CBBA88,
    0x85472B01, 0x0D573359, 0x018A8843, 0x000FEF66, 0x49CACA89, 0x00898988,
    0x039D9D8E, 0x00FD99B8, 0x84BEBAB8, 0x0B33333F, 0x0108A621, 0x009CAA34,
    0x45CCEC81, 0x0088B88C, 0x039DB99E, 0x01BDD9F8, 0x8BAFEFAB, 0x4D77733D,
    0x01888E01, 0x001C8A14, 0x410ACA41, 0x00098148, 0x0315990E, 0x00B15DF8,
    0x00967658, 0x0913371D, 0x08900E94, 0x01048A05, 0x01A808A5, 0x0210041A,
    0x0025102C, 0x0223162A, 0x01836709, 0x0999B9D9, 0x880111C8, 0x01115D51,
    0x4FD555DF, 0x0320046F, 0x0007664C, 0x0222122A, 0x00888888, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x008EEC80, 0x008DFB80, 0x00556550, 0x00554550, 0x00CC1100, 0x05544555,
    0x00445DC0, 0x06ECDD44, 0x08C426C8, 0x08877588, 0x0671A976, 0x0551B955,
    0x005DD111, 0x01110145, 0x04418954, 0x0EE219C4, 0x0CC022CC, 0x08867188,
    0x0462AA20, 0x0451A910, 0x0044DD01, 0x00999955, 0x00099144, 0x0CE23000,
    0x04C8AAC4, 0x088EF980, 0x00442000, 0x00776330, 0x00551DD1, 0x00677764,
    0x01CC5554, 0x00CE2300, 0x0CC022CC, 0x08866000, 0x0026EE20, 0x0451A910,
    0x00549B64, 0x05D99910, 0x05599055, 0x000CE220, 0x08C426C8, 0x08867188,
    0x0220AC62, 0x0551B155, 0x045DD775, 0x05410155, 0x04518877, 0x00004622,
    0x008EEC80, 0x00CCFF80, 0x04446444, 0x00554D50, 0x00CC1122, 0x01554451,
    0x01545DF2, 0x0008C422, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0AF555FE, 0x00CF77E8,
    0x0AFFFDDD, 0x02FDDD72, 0x08D555FF, 0x077999F7, 0x07719177, 0x04733374,
    0x07E808E7, 0x04D923DC, 0x0DF20055, 0x05728877, 0x01984CB3, 0x066A8277,
    0x06688877, 0x07704077, 0x07699967, 0x05D822DD, 0x05FA0055, 0x0002A877,
    0x0118CC33, 0x066AA277, 0x06E80AF7, 0x06604077, 0x0763A367, 0x0558AAD5,
    0x047B9155, 0x0044EE73, 0x0198CCB3, 0x0679B177, 0x0EF133FF, 0x07715077,
    0x037DDD67, 0x0155FB55, 0x0176CC55, 0x0442AA33, 0x0110C433, 0x06688077,
    0x0E62207F, 0x07744477, 0x02A808E7, 0x0554AA55, 0x057608F7, 0x05728877,
    0x0198CCB3, 0x06688077, 0x02660473, 0x07740477, 0x0BB111FE, 0x0516EE71,
    0x0CDBBBFD, 0x02FDDD72, 0x0233F733, 0x037DDDF3, 0x02244433, 0x09FBBBF8,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00110022, 0x00022000, 0x00220011, 0x00888222,
    0x0112AE40, 0x00008880, 0x00111880, 0x00000000, 0x00110022, 0x00000000,
    0x00000011, 0x00008A20, 0x0112AC40, 0x00088000, 0x09909188, 0x00000000,
    0x08E764EE, 0x0CDF7FD8, 0x08FFBDDD, 0x0675DF70, 0x0157FD54, 0x007FB176,
    0x04EA3166, 0x0DFBBBFC, 0x06F80EE2, 0x09D628C4, 0x05FA4451, 0x01320AF5,
    0x0112AC51, 0x03FC2277, 0x06711377, 0x0F70C07F, 0x06688C62, 0x09D739D4,
    0x013A8451, 0x0046EE71, 0x0112AC51, 0x037DB177, 0x02641562, 0x0F70407F,
    0x02EC4CE2, 0x09DEEC51, 0x01328C51, 0x05728A31, 0x0552AC51, 0x026CA077,
    0x02245522, 0x03F555F2, 0x0AA004EA, 0x09F73310, 0x08BBDDFF, 0x02FDD776,
    0x0177FF10, 0x0671FDF2, 0x02205122, 0x013EAE20, 0x00004444, 0x04CCC880,
    0x00022220, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00111110,
};

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 g_font_buf_index = 0;
s32 g_font_enabled = 1;
RECT g_font_rect = {0x3F0, 0x1DC, 0x10, 0x24};
s32 g_gpu_fog_clut_object = -1;
s32 D_800A322C = 0;
/* reached gp-relative by func_8003EDC0: size from the blob label */
s32 D_800A3230 = 0;
/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
s32 D_800A3818;
