/* memcard_Format, 30 game functions, then the link-cable wrappers
 * comb_Init..comb_WaitRead8 and math_Popcount32. .text 0x80037F08 (ROM
 * 0x28708). Start boundary: LEGACY (a tooling split, no evidence either way).
 */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

extern s32 g_str_sio_800A3210;

extern s32 D_800A36C0;

extern s32 g_comb_close_request;
extern s16 D_800A379E;
extern s16 D_800A3814;
extern s16 D_800A37C8;
extern s32 D_800A31F4;
extern u8 D_800A38CC;
extern u8 D_800A382C;
extern s32 g_comb_read_fd;
extern s32 g_comb_write_fd;
extern s32 g_comb_event_error;
extern s32 g_comb_event_ioer;
extern u8 D_800A320C;

extern s32 D_800A31F8;
extern u8 D_800A36F8;
extern Rec1C (*D_800A36EC)[2];
extern u8 D_800A3782;

extern u8 D_800A31FC;

extern s32 g_comb_wait_state;
extern s32 g_comb_recv_buf[2];
extern s32 g_comb_send_buf;
extern u8 D_800A37D0;

extern Unk800F34D8Save D_800F34D8; /* = D_800F33D8 + 0x100 */
extern s32 D_800A31F0;

s32 memcard_Format(s32 a0, s32 a1) {
    s32 buf[2];
    sprintf(buf, (char *)D_800109C8, a0, a1);
    return format(buf);
}

void memcard_BuildSaveBlock(u8 *a0) {
    s32 var_0;
    u8 *p;
    s32 i;

    var_0 = 0;
    p = (u8 *)&D_80106A50;
    i = 0;
    do {
        var_0 += *p++;
        i++;
    } while ((u32)i < 0x24);

    {
        /* FAKE: the save-block view in its own local; a cast of a0 at each
           use moves the copy into the loop's delay slot (score 2) */
        Unk800F34D8Save *base = (Unk800F34D8Save *)a0;
        i = 0;
        do {
            base->rec[i] = D_80106A50;
            base->sum[i] = var_0;
            {
                s32 j = 0;
                do {
                    base->ptr[j] = 0;
                    base->val[j] = 0;
                    j++;
                } while (j < 0x16);
            }
            i++;
        } while (i < 3);
        base->unk_FC = 0;
    }
}

s32 memcard_RestoreSaveBlock(Unk800F34D8Save *arg0) {
    s32 i;
    /* FAKE: one counter j for both loops lengthens its live range so sum,
       not j, gets $a0; two counters: score 16 */
    s32 j;

    i = 0;
    do {
        s32 sum;
        u8 *var_3;

        sum = 0;
        var_3 = (u8 *)&arg0->rec[i];
        j = 0;
        do {
            sum += *var_3;
            var_3++;
            j++;
        } while (j < 0x24U);
        if (sum == arg0->sum[i]) {
            break;
        }
        i++;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        FileRecord *src = &arg0->rec[i];

        if (!(src->flags & 0x80)) {
            D_80106A50 = *src;
        }

        j = 0;
        do {
            u16 *ptr = arg0->ptr[j];
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = arg0->val[j];
            }
            j++;
        } while (j < 0x16);
    }

    return 1;
}

void memcard_ClearBuffer(void) {
    u8 *p = D_800F33D8;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}

extern u8 D_8008F1C0[];
/* memcard_BuildFileHeader copies D_800109EC's first 16 halfwords to +0x60 of
 * its buffer and D_80010A2C's 128 bytes to +0x80: the memory-card file header's
 * icon CLUT (16 RGB555 colours; the 16 halfwords after them are zero) and its
 * 16x16 4-bit icon image. D_80010AAC is the save-file id (D_800A31F0 holds its
 * address), then 3 bytes of rodata padding. */
const u16 D_800109EC[32] = {
    0x7FFF, 0x77DF, 0x6B9F, 0x635E, 0x5B1E, 0x56FD, 0x4ABD, 0x427C,
    0x323C, 0x2DFB, 0x21BB, 0x199B, 0x115A, 0x0D3A, 0x051A, 0x8000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
const u8 D_80010A2C[128] = {
    0x00, 0x00, 0x00, 0x00, 0x10, 0x51, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xA3, 0xC5, 0x3E, 0x00, 0x00, 0x10, 0x53, 0x38, 0xEA, 0x58, 0x29, 0x00,
    0x00, 0x84, 0xBA, 0x7D, 0xDC, 0x8D, 0x03, 0x00, 0x00, 0x10, 0x41, 0xB7,
    0xDD, 0xBB, 0x1C, 0x00, 0x00, 0x94, 0xDB, 0xCD, 0xAD, 0x11, 0x01, 0x00,
    0x00, 0xE8, 0x8E, 0x66, 0x8E, 0x00, 0x02, 0x00, 0x00, 0xA9, 0x16, 0x62,
    0x8E, 0x00, 0x17, 0x00, 0x10, 0x32, 0xC6, 0x6A, 0xAE, 0x00, 0x18, 0x00,
    0x10, 0x84, 0x9A, 0x44, 0xDD, 0x03, 0x29, 0x00, 0x00, 0x00, 0x50, 0x38,
    0xEB, 0x08, 0x6B, 0x00, 0x00, 0x30, 0xD9, 0x06, 0xE7, 0x5D, 0x6B, 0x00,
    0x00, 0xE5, 0x6C, 0x00, 0x91, 0xCD, 0x6D, 0x00, 0x00, 0xC3, 0x09, 0x00,
    0x20, 0xEA, 0x7E, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xB2, 0x8E, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
};
const char D_80010AAC[] = "BASLUS-00663BUSHIDO2";
extern u8 D_800A3200;
extern u8 D_800A3201;

void memcard_BuildFileHeader(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    mask = D_80106A50.unk_00;

    for (i = 0; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = D_8008F204[i];
            switch (v) {
            case 0:
                s1++;
                break;
            case 1:
                s2++;
                break;
            case 2:
                s3++;
                break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    strcpy(out + 4, D_8008F1C0);

    out[0x22] = D_8008F1A8[s1 * 2 + 0];
    out[0x23] = D_8008F1A8[s1 * 2 + 1];
    out[0x3C] = D_8008F1A8[s2 * 2 + 0];
    out[0x3D] = D_8008F1A8[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = D_8008F19C[s3 * 2 + 0];
        out[0x43] = D_8008F19C[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        const u16 *src = D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        const u8 *var_11 = D_80010A2C;
        u8 *var_12 = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(var_12 + 0x80);
            const u16 *src = (const u16 *)var_11;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            var_11 += 0x80;
            i++;
            var_12 += 0x80;
        } while (i <= 0);
    }
}

void memcard_StepRequest(void) {
    s32 var_v1;
    s32 var_v0;
    s32 temp_s0;
    s32 var_s1;
    u16 temp_v0;

    var_v1 = D_800A31F4;
    if (var_v1 != 1)
        goto state_other;

    var_v1 = D_800A31F8;
    if (var_v1 == -1)
        goto sub_inc;

    if (var_v1 >= 0)
        goto positive_path;
    if (var_v1 == -2)
        goto neg2_handler;
    return;

positive_path:
    if (var_v1 >= 3)
        return;
    if (var_v1 <= 0)
        return;
    if (D_800A37C8 == 0) {
        D_800A31F4 = 5;
        return;
    }
    if (D_800A37C8 == 3)
        goto set_state_7;
    D_800A31F4 = 3;
    return;

neg2_handler:
    if (D_800A37C8 != 3)
        goto neg2_else;
set_state_7:
    D_800A31F4 = 7;
    return;
neg2_else:
    D_800A31F4 = 0;
    D_800A379E = 0xA;
    return;

sub_inc:
    temp_v0 = (u16)D_800A3814 + 1;
    D_800A3814 = temp_v0;
    if ((s16)temp_v0 < 4)
        return;
    var_v0 = 8;
    goto finish;

state_other:
    var_v0 = 5;
    if (var_v1 == var_v0)
        goto state_5;
    if (var_v1 < 6) {
        var_v0 = 3;
        if (var_v1 == var_v0)
            goto state_3;
        return;
    }
    var_v0 = 7;
    if (var_v1 == var_v0)
        goto state_7;
    return;

state_3:
    D_800A379E = 1;
    memcard_CountFiles(0, 0);
    temp_s0 = memcard_CountFreeBlocks();
    if (memcard_HasFile((u8 *)D_800A31F0) != 0) {
        var_s1 = 0;
        if (D_800A37C8 == 1) {
            var_v0 = 0xD;
            goto finish;
        }
        goto setup_load;
    }
    var_s1 = 1;
    if (temp_s0 == 0) {
        var_v0 = 7;
        goto finish;
    }
setup_load:
    D_800A38CC = 1;
    memcard_ClearBuffer();
    memcard_BuildFileHeader(D_800F33D8);
    memcard_BuildSaveBlock(D_800F33D8 + 0x100);
    if (memcard_WriteFile(
            0, 0, D_800A31F0, (s32)D_800F33D8, 1, 0x200, var_s1) != 0) {
        close(g_memcard_fd);
        var_v0 = 3;
        goto finish;
    }
    D_800A31F4 = 4;
    return;

state_5:
    memcard_CountFiles(0, 0);
    memcard_CountFreeBlocks();
    if (memcard_HasFile((u8 *)D_800A31F0) == 0) {
        var_v0 = 0xE;
        goto finish;
    }
    D_800A379E = 4;
    memcard_ClearBuffer();
    if (memcard_ReadFile(0, 0, D_800A31F0, (s32)D_800F33D8, 0x200) != 0) {
        close(g_memcard_fd);
        var_v0 = 6;
        goto finish;
    }
    D_800A31F4 = 6;
    return;

state_7:
    var_v0 = memcard_Format(0, 0);
    if (var_v0 != 0) {
        var_v0 = 0xB;
        goto finish;
    }
    var_v0 = 0xC;
finish:
    D_800A379E = var_v0;
    D_800A31F4 = 0;
}

extern s32 memcard_RestoreSaveBlock(Unk800F34D8Save *);

/* Load/save state-machine completion handler: dispatches on D_800A31F4
 * (4 = after a write, 6 = after a read), reaps memcard_PollSwEvents()'s status,
 * closes the file and posts a result code to D_800A379E. The still-pending
 * paths share the fail_store end label (the shared-end-label recipe,
 * .claude/rules/shared-end-label.md). */
void memcard_PollFileCompletion(void) {
    s32 ret;
    s32 var_1;

    switch (D_800A31F4) {
    case 4:
        ret = memcard_PollSwEvents();
        if (ret == 0) {
            var_1 = 1;
            goto fail_store;
        }
        close(g_memcard_fd);
        if (ret == 1) {
            D_800A379E = 2;
        } else {
            D_800A379E = 3;
        }
        D_800A31F4 = 0;
        return;
    case 6:
        ret = memcard_PollSwEvents();
        if (ret == 0) {
            var_1 = 4;
            goto fail_store;
        }
        close(g_memcard_fd);
        if (ret == 1) {
            D_800A379E = 5;
            if (memcard_RestoreSaveBlock(&D_800F34D8) == 0) {
                D_800A379E = 0xF;
            }
        } else {
            D_800A379E = 6;
        }
        D_800A31F4 = 0;
        return;
    }
    return;

fail_store:
    D_800A379E = var_1;
}

s32 memcard_Update(void) {
    if ((u32)D_800A31F4 < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    memcard_StepRequest();
    memcard_PollFileCompletion();
    return D_800A379E;
}

void memcard_RequestLoad(void) {
    D_800A379E = 4;
    D_800A3814 = 0;
    D_800A37C8 = 0;
    D_800A31F4 = 1;
}

void memcard_RequestSave(void) {
    D_800A379E = 1;
    D_800A37C8 = 1;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}

void memcard_RequestOverwrite(void) {
    D_800A379E = 1;
    D_800A37C8 = 2;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}

void memcard_RequestFormat(void) {
    D_800A379E = 9;
    D_800A37C8 = 3;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}

extern u8 D_800A3203;
extern u8 D_800A31FC;
extern void memcard_RequestLoad(void);
extern s32 memcard_Update(void);

s32 func_8003880C(void) {
    s32 s0;
    s32 v0;

    s0 = 0;
    if (D_800A3203) {
        D_800A3203 = 0;
        D_800A31FC = 1;
        memcard_RequestLoad();
    }
    v0 = memcard_Update();
    switch (v0 - 4) {
    case 0:
        break;
    case 1:
        s0 = 1;
        break;
    case 2:
        s0 = -1;
        break;
    case 3:
        s0 = -1;
        break;
    case 4:
        s0 = -1;
        break;
    case 5:
        s0 = -1;
        break;
    case 6:
        s0 = -1;
        break;
    case 7:
        s0 = -1;
        break;
    case 8:
        s0 = -1;
        break;
    case 9:
        s0 = -1;
        break;
    case 10:
        s0 = -1;
        break;
    case 11:
        s0 = -1;
        break;
    default:
        s0 = -1;
        break;
    }
    func_8006BEC4(0, -1);
    if (s0) {
        D_800A3203 = 1;
        D_800A31FC = 0;
    }
    return s0;
}

/* Q65: this file's statics (.sbss), in address order. */
static u8 D_800A3318;
static u8 D_800A331C;
static u8 D_800A3320;
static u8 D_800A3324;
static u8 D_800A3328;
static u8 D_800A332C;
static u8 D_800A3330;
static u8 D_800A3334;
static u8 D_800A3338;
static u8 D_800A333C;
static u8 D_800A3340;
static u8 D_800A3344;
static u8 D_800A3348;
static u8 D_800A334C;
static u8 D_800A3350;
static u8 D_800A3354;

s32 func_800388A8(void) {
    extern u8 D_800A3204;
    s32 result = 0;
    u32 var_1;
    if (D_800A3204 != 0) {
        D_800A3204 = 0;
        D_800A3318 = 0;
    }
    var_1 = g_pad_state.pressed;
    if (var_1 & 0x400040) {
        snd_QueueSound(1, 0x7F, 0x7F);
        if (D_800A3318 == 0) {
            result = 1;
        } else {
            result = -1;
        }
    } else if (var_1 & (u32)0x80008000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A3318 = 0;
    } else if (var_1 & 0x20002000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A3318 = 1;
    }
    func_8006BEC4(0x13, D_800A3318);
    if (result != 0) {
        D_800A3204 = 1;
    }
    return result;
}

s32 func_80038988(void) {
    extern u8 D_800A3205;
    s32 result = 0;
    s32 v0;
    s32 var_2;

    if (D_800A3205) {
        D_800A331C = 0;
        D_800A3320 = 0;
        D_800A3324 = 0;
        D_800A3328 = 0;
        D_800A332C = 0;
        D_800A31FC = 1;
        memcard_RequestLoad();
        D_800A3205 = 0;
        D_800A3330 = 0x5A;
        D_800A3334 = 0;
        D_800A3338 = 0;
        D_800A333C = 0;
    }

    v0 = memcard_Update();

    if (D_800A333C != 0) {
        func_8006BEC4(0xA, -1);
        goto timer;
    }

    var_2 = 0;
    if (D_800A31FC == 0) {
        if (D_800A3338 == 1) {
            switch (v0 - 4) {
            case 0:
                var_2 = 2;
                break;
            case 1:
                var_2 = 3;
                break;
            case 2:
            case 6:
            case 10:
            case 11:
                var_2 = 4;
                break;
            case 4:
                var_2 = -1;
                break;
            default:
                var_2 = 0;
                break;
            }
        } else {
            switch (v0 - 4) {
            case 0:
                var_2 = 2;
                break;
            case 1:
                var_2 = 3;
                break;
            case 2:
                var_2 = 4;
                break;
            case 10:
                var_2 = 1;
                break;
            case 11:
                var_2 = 0xB;
                break;
            case 4:
                var_2 = -1;
                break;
            case 6:
                var_2 = 0x14;
                break;
            default:
                var_2 = 0;
                break;
            }
        }
    }

    if (var_2 >= 0) {
        func_8006BEC4(var_2, -1);
    }

    if (D_800A31FC != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            D_800A331C++;
            if ((u8)D_800A331C >= 5) {
                D_800A333C = 1;
                D_800A3330 = 0x5A;
                break;
            }
            memcard_RequestLoad();
            break;
        case 11:
            D_800A332C++;
            if ((u8)D_800A332C >= 5) {
                D_800A31FC = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 6:
            D_800A3328++;
            if ((u8)D_800A3328 >= 5) {
                D_800A31FC = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 10:
            D_800A3324++;
            if ((u8)D_800A3324 >= 5) {
                D_800A31FC = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        case 2:
            D_800A31FC = 0;
            break;
        case 1:
            D_800A3320++;
            if ((u8)D_800A3320 >= 5) {
                D_800A31FC = 0;
                D_800A3338 = 1;
            }
            memcard_RequestLoad();
            break;
        default:
            break;
        }
    } else {
        switch (v0 - 5) {
        case 0:
        case 1:
        case 5:
        case 9:
        case 10:
            goto timer;
        case 3:
            D_800A333C = 1;
            D_800A3330 = 0x5A;
            break;
        default:
            break;
        }
    }

    goto end;

timer:
    D_800A3330--;
    if ((u8)D_800A3330 == 0 || (g_pad_state.pressed & 0x100010)) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = 1;
    }

end:
    if (result != 0) {
        D_800A3205 = 1;
    }
    return result;
}

s32 func_80038C70(void) {
    extern u8 D_800A3207;
    extern u8 D_800A3206;
    extern void memcard_RequestLoad(void);
    extern void memcard_RequestSave(void);
    extern void memcard_RequestOverwrite(void);
    extern void memcard_RequestFormat(void);
    s32 result = 0;
    s32 var_1 = -1;
    s32 v0;
    s32 var_3;

    if (!D_800A3207) {
        D_800A3207 = 1;
        D_800A334C = 0x5A;
        D_800A3350 = 0;
        D_800A3354 = 0;
        D_800A31FC = 0;
    }

    v0 = memcard_Update();

    if (D_800A3354 != 0) {
        func_8006BEC4(0xA, -1);
        D_800A334C--;
        if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
            snd_QueueSound(2, 0x7F, 0x7F);
            D_800A3207 = 1;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            D_800A3354 = 0;
            D_800A31FC = 0;
        }
        goto end;
    }

    /* FAKE: the two empty arms reproduce the target's dead `== 2` / `== 3`
     * tests that branch straight to the join; without them those 4 insns are
     * gone (score 5; a switch scores 14) */
    if (D_800A3207 == 1) {
        v0 = 0;
    } else if (D_800A3207 == 2) {
    } else if (D_800A3207 == 3) {
    } else if (D_800A3207 == 4) {
        v0 = 0x11;
    }

    var_3 = 0;
    if (D_800A31FC != 0) {
        goto sel_dispatch;
    }

    if (D_800A3207 == 3) {
        if (v0 == 8) {
            goto case8_sel;
        }
        if (v0 == 10) {
            goto case12_sel;
        }
        goto case9_11_sel;
    }

    switch (v0) {
    case 0:
        var_3 = 0x11;
        /* FAKE: duplicate of load_sel2's store — jump2 cross-jump re-merges it
           (zero emitted bytes); the extra real def lifts var_1's reg_n_refs
           priority above result's so RA lands var_1->$s2 / result->$s3
           (target). SOTN duplicate-into-arms family; `goto load_sel2`
           scores 13. */
        var_1 = D_800A3350;
        goto sel_dispatch;
    case 13:
    case 17:
        var_3 = 6;
    load_sel2:
        var_1 = D_800A3350;
        goto sel_dispatch;
    case 1:
        var_3 = (-(D_800A38CC != 0)) & 7;
        goto sel_dispatch;
    case 2:
        var_3 = 8;
        goto sel_dispatch;
    case 3:
        var_3 = 9;
        goto sel_dispatch;
    case 8:
    case8_sel:
        var_3 = -1;
        goto sel_dispatch;
    case 7:
        var_3 = 5;
        goto sel_dispatch;
    case 10:
        if (D_800A3206 == 0) {
            D_800A3350 = 1;
        }
        if (g_pad_state.pressed & 0x400040) {
            D_800A3206 = 0;
            snd_QueueSound(1, 0x7F, 0x7F);
            var_3 = 0xD;
            if (D_800A3350 == 0) {
                memcard_RequestFormat();
                goto sel_dispatch;
            }
            v0 = 0;
            D_800A3207 = 5;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            var_3 = -1;
            goto sel_dispatch;
        }
        var_3 = 0xC;
        if ((g_pad_state.pressed & 0xA000A000U) != 0) {
            D_800A3206 = 1;
        }
        goto load_sel2;
    case 9:
    case 11:
    case9_11_sel:
        var_3 = 0xD;
        goto sel_dispatch;
    case 12:
    case12_sel:
        var_3 = 0xF;
        goto sel_dispatch;
    default:
        var_3 = 0;
        goto sel_dispatch;
    }

sel_dispatch:
    if (var_3 >= 0) {
        func_8006BEC4(var_3, var_1);
    }

    if (D_800A31FC != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            D_800A3348++;
            if (((u8)D_800A3348) >= 5) {
                D_800A3354 = 1;
                D_800A334C = 0x5A;
                break;
            }
            memcard_RequestLoad();
            break;
        case 1:
        case 2:
        case 3:
        case 9:
        case 10:
        case 11:
            D_800A3340++;
            if (((u8)D_800A3340) >= 5) {
                D_800A31FC = 0;
                memcard_RequestSave();
                break;
            }
            memcard_RequestLoad();
            break;
        case 6:
            D_800A3344++;
            if (((u8)D_800A3344) >= 5) {
                D_800A31FC = 0;
                break;
            }
            memcard_RequestLoad();
            break;
        default:
            break;
        }
    } else if (D_800A3207 == 3) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            result = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        default:
            memcard_RequestSave();
            D_800A3207 = 2;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 6:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
                snd_QueueSound(2, 0x7F, 0x7F);
                result = 1;
                break;
            }
            break;
        }
    } else {
        switch (v0) {
        case 8:
            D_800A3354 = 1;
            D_800A334C = 0x5A;
            break;
        case 2:
        case 3:
        case 7:
        case 12:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
                snd_QueueSound(2, 0x7F, 0x7F);
                if (v0 != 7) {
                    result = 1;
                    break;
                }
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                D_800A3354 = 0;
                D_800A31FC = 0;
            }
            break;
        case 11:
            D_800A31FC = 1;
            memcard_RequestLoad();
            D_800A3207 = 3;
            D_800A334C = 0x5A;
            break;
        case 13:
            D_800A3207 = 4;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 0:
            if (D_800A3207 == 5) {
                D_800A3207 = 1;
                break;
            }
            goto sw4_L2;
        case 17:
        sw4_L2:
            if (g_pad_state.pressed & 0x400040) {
                snd_QueueSound(1, 0x7F, 0x7F);
                if (v0 == 0) {
                    if (D_800A3350 != 0) {
                        result = 1;
                        break;
                    }
                    D_800A3348 = 0;
                    D_800A3340 = 0;
                    D_800A3344 = 0;
                    D_800A31FC = 1;
                    memcard_RequestLoad();
                } else {
                    if (D_800A3350 != 0)
                        goto area_c_long;
                    memcard_RequestOverwrite();
                }
                D_800A3207 = 2;
                break;
            area_c_long:
                v0 = 0;
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                break;
            }
            goto sw4_buttons;
        case 10:
        sw4_buttons:
            if (g_pad_state.pressed & 0x80008000U) {
                snd_QueueSound(0, 0x7F, 0x7F);
                D_800A3350 = 0;
                break;
            }
            if (g_pad_state.pressed & 0x20002000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                D_800A3350 = 1;
            }
            break;
        default:
            break;
        }
    }

    if (v0 < 11) {
        if (v0 >= 9) {
            goto d_check;
        }
        if (v0 == 1) {
            goto d_check;
        }
        goto end;
    }
    if (v0 != 17) {
        goto end;
    }
d_check:
    if (D_800A31F8 == -1) {
        D_800A3354 = 1;
        D_800A334C = 0x5A;
    }

end:
    if (result != 0) {
        D_800A3207 = 0;
        D_800A31FC = 0;
    }
    return result;
}

s32 *func_800392B8(void) { return (s32 *)D_800F33D8; }

void replay_Reset(void) {
    s32 i;
    s32 j;

    D_800A36EC = (Rec1C(*)[2])D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
    for (i = 0x1F; i >= 0; i--) {
        D_80101BF0[i].unk_0 = 0xFF;
    }
    for (j = 0xB3; j >= 0; j--) {
        D_800F68E0[j].unk_0 = -1;
    }
}

void replay_BeginFrame(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    Unk800F68E0Rec *q;
    s16 val;
    s16 newval;
    for (i = 0; i < 0x20; i++) {
        if (D_80101BF0[i].unk_0 == D_800A36F8) {
            D_80101BF0[i].unk_0 = 0xFF;
        }
    }

    q = D_800F68E0;
    i = 0;
    do {
        val = q->unk_0;
        if (val != -1) {
            newval = val + 1;
            q->unk_0 = newval;
            if (newval - q->unk_2 >= 0x101) {
                q->unk_0 = -1;
            }
        }
        i++;
        q++;
    } while (i < 0xB4);

    D_800A379C = 0;
    D_800A3714 = 0;
}

void replay_RecordModelDraw(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    Unk800F68E0Rec *var_0;
    s32 i;
    s16 idx;
    s16 cur;
    s32 state;
    u8 seen;
    s32 raw;
    s32 rot;

    var_0 = D_800F68E0;
    i = 0;
    do {
        state = var_0->unk_0;
        if (state != -1) {
            seen = var_0->unk_2;
            if (seen == state - 1 && seen < 0xFF) {
                raw = var_0->unk_A << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    var_0->unk_4[0] == arg2[0] && var_0->unk_4[1] == arg2[1] &&
                    var_0->unk_4[2] == arg2[2] &&
                    ((rot - arg3[0]) & 0xFFF) == 0 &&
                    ((var_0->unk_C - arg3[1]) & 0xFFF) == 0 &&
                    ((var_0->unk_E - arg3[2]) & 0xFFF) == 0) {
                    var_0->unk_2 = seen + 1;
                    return;
                }
            }
        }
        i++;
        var_0++;
    } while (i < 0xB4);

    idx = D_800A3714;
    var_0 = &D_800F68E0[idx];
    while (idx < 0xB4) {
        if (var_0->unk_0 == -1) {
            break;
        }
        cur = idx + 1;
        D_800A3714 = cur;
        var_0++;
        idx = cur;
    }

    if (D_800A3714 == 0xB4) {
        D_800A3209++;
        return;
    }

    var_0->unk_0 = 0;
    var_0->unk_3 = arg1;
    var_0->unk_2 = 0;
    var_0->unk_4[0] = arg2[0];
    var_0->unk_4[1] = arg2[1];
    var_0->unk_4[2] = arg2[2];
    var_0->unk_A = (arg3[0] & 0xFFF) | (arg0 << 12);
    var_0->unk_C = arg3[1];
    var_0->unk_E = arg3[2];
}

void replay_RecordEvent(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    Unk80101BF0Rec *var_0;
    u8 idx;
    if (D_800A3208 == 0) {
        idx = D_800A379C;
        var_0 = &D_80101BF0[idx];
        for (; idx < 0x20; idx++, var_0++) {
            if (var_0->unk_0 == 0xFF) {
                break;
            }
            D_800A379C = idx + 1;
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            var_0->unk_1 = arg0;
            var_0->unk_2 = arg1;
            var_0->unk_0 = tmp;
            var_0->unk_4[0] = arg2[0];
            var_0->unk_4[1] = arg2[1];
            var_0->unk_4[2] = arg2[2];
            if (arg3 != NULL) {
                var_0->unk_A[0] = arg3[0];
                var_0->unk_A[1] = arg3[1];
                var_0->unk_A[2] = arg3[2];
            }
        }
    }
}

void replay_RecordFighter(Unk80101EC8Record *a0) {
    s16 idx;
    Rec1C *dest;

    idx = a0->index;
    dest = &D_800A36EC[D_800A36F8][idx];

    dest->h4 = a0->unk_F4.x;
    dest->h8 = a0->unk_F4.z;
    dest->h6 = a0->unk_F4.y;

    {
        u16 v = a0->unk_1C8.vy;
        u8 b = a0->unk_B3;
        dest->hA = (v & 0xFFF) | (b << 12);
    }

    dest->hC = a0->unk_148;
    dest->b14 = a0->unk_1E6 >> 2;
    dest->b15 = a0->unk_1E8 >> 2;
    dest->b16 = a0->unk_1EA >> 2;
    dest->w0 = a0->unk_50;
    dest->b17 = 0;

    if (a0->unk_60 != 0) {
        dest->b17 = 1;
    }
    if (a0->unk_61 != 0) {
        dest->b17 |= 2;
    }

    dest->hE = a0->unk_64;
    dest->h10 = a0->unk_66;
    dest->h12 = a0->unk_68;
    dest->b18 = a0->unk_62;
    dest->b19 = a0->unk_40;
}

void replay_AdvanceFrame(void) {
    u8 val = D_800A36F8;
    if (val == 0x77) {
        D_800A36F8 = 0;
        D_800A3782 = 1;
    } else {
        D_800A36F8 = val + 1;
    }
}

void replay_Start(void) {
    gpu_ResetGraphMode1();
    func_8003E22C();
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_8001B6F4();
    eff_StopCommands();
    D_800A37D0 = 0;
    D_800A3834 = 5;
}

extern s32 collision_GetHitTag(void);

void func_8003984C(Unk80101EC8Record *arg0, s32 *arg1, s32 *arg2) {
    s32 sp10[3];
    s32 sp20[3];
    s32 sp30[4];
    s16 sp40[4];
    s32 mid_x, mid_y, mid_z;
    s32 result;

    mid_x = (s32)(arg0->unk_198[0].x + arg0->unk_198[1].x) / 2;
    sp10[0] = mid_x;
    mid_y = (s32)(arg0->unk_198[0].y + arg0->unk_198[1].y) / 2;
    sp10[1] = mid_y - 0x190;
    mid_z = (s32)(arg0->unk_198[0].z + arg0->unk_198[1].z) / 2;
    sp20[0] = mid_x;
    sp20[1] = mid_y + 0x190;
    sp10[2] = mid_z;
    sp20[2] = mid_z;
    if (collision_TestSegment(sp10, sp20, sp30, sp40) != 0) {
        result = collision_GetHitTag();
        *arg1 = result;
        if (result == 7) {
            goto neg;
        }
        if (result == 0) {
            goto neg;
        }
        *arg2 = ((0x2A >> result) ^ 1) & 1;
    } else {
        *arg1 = -1;
    neg:
        *arg2 = -1;
    }
}

extern u8 D_800A3208;

void replay_PlayFrame(void) {
    MotionFrame work[2];
    /* FAKE: frame layout - func_80041188 writes two MATRIXes (0x40 bytes); 0x88
     * keeps the frame (MATRIX[2]: score 58; MATRIX[4] also differs) */
    s32 sp120[34];
    s32 pos[3];
    s16 rot[3];
    u16 sp1C0[200];
    s32 out_b1;
    s32 out_b2;
    s32 idx;
    s32 var_8;
    s32 i;
    Rec1C *p;
    Unk80101EC8Record *var_11;
    Unk80101BF0Rec *e;
    Unk800F68E0Rec *s;
    /* temp: the 0/1 weapon-set selector in the per-player loop, then the
     * replay window of the event loop (Ruling 11, ordinary-c-judge-decidable)
     */
    s32 temp;
    u8 var_15;
    u8 *var_16;

    if (D_800A3782 != 0) {
        idx = (D_800A36F8 + D_800A37D0) % 120;
        var_8 = (D_800A37D0 << 12) / 120;
    } else {
        idx = D_800A37D0;
        var_8 = (idx << 12) / D_800A36F8;
    }
    D_800A3778 = func_800472B0();
    /* The frame record's address is written out at each argument (F3
     * compound-address duplication, no-new-park-categories); a `rec` local:
     * score 20. */
    func_8001BAE4(
        &D_800A36EC[idx][D_800A3748],
        D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], var_8);
    func_8001BBD8(
        &D_800A36EC[idx][D_800A3748],
        D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], var_8);
    camera_ApplyReplayView(var_8);

    for (i = 0; i < 2; i++) {
        /* entry: the frame's entry in the practice weapon table (if arm) or
         * the character's weapon table (else arm) (Ruling 11,
         * ordinary-c-judge-decidable) */
        u16 *entry;

        p = &D_800A36EC[idx][i];
        var_11 = &D_80101EC8[i];
        func_800198D0((p->hE >> 14) & 3, p->hE & 0x3FFF, &work[0], sp1C0);
        func_800198D0((p->h10 >> 14) & 3, p->h10 & 0x3FFF, &work[1], sp1C0);
        motion_ApplyRecordedPoseAdjustments(var_11, p, &work[0], &work[1]);
        func_80041188(
            i, (u8 *)&work[0], (u8 *)&work[1], p->h12, (MATRIX *)sp120);
        pos[0] = p->h4;
        pos[1] = p->h6;
        pos[2] = p->h8;
        rot[0] = 0;
        rot[1] = p->hA;
        rot[2] = 0;
        func_80040D48(i, 1, pos, rot, 0, p->hC);
        if (p->b18 & 1) {
            func_80049718(var_11->unk_12, i * 2 + 0x8000, 0, 0);
        }
        if (p->b18 & 4) {
            func_80049718(D_8008EB80[var_11->unk_14], i * 2 + 0x8001, 0, 0);
        }
        if (p->b18 & 2) {
            func_80049A2C(var_11->unk_12, i * 2, (p->b18 >> 4) & 1);
        }
        if (p->b18 & 8) {
            func_80049A2C(
                D_8008EB80[var_11->unk_14], i * 2 | 1, (p->b18 >> 5) & 1);
        }
        fighter_UpdateBoneWorldData(
            var_11, SPAD->unkA8[i], SPAD->unk00[i], SPAD->unk48[i]);
        func_8003984C(var_11, &out_b1, &out_b2);
        if (out_b1 != 1) {
            var_11->unk_B1 = out_b1;
            if (out_b2 != -1) {
                var_11->unk_B2 = out_b2;
            }
        }
        var_15 = var_11->unk_40;
        var_11->unk_40 = p->b19;
        var_16 = var_11->unk_58;
        if (p->b17 & 1) {
            entry = &D_80102760.unk_04[p->w0->unk_04 * 2];
            var_11->unk_58 = D_80102760.unk_08 + entry[1];
        } else {
            temp = (p->b17 >> 1) & 1;
            entry = &D_801027B0[temp].unk_04[p->w0->unk_04 * 2];
            var_11->unk_58 = D_801027B0[temp].unk_08 + entry[1];
        }
        if (p->b18 & 0x40) {
            fighter_ProcessFrameEvents(var_11);
        }
        var_11->unk_40 = var_15;
        var_11->unk_58 = var_16;
        func_80040304(i, (p->hA >> 12) & 7);
    }

    e = D_80101BF0;
    for (i = 0; i < 0x20; i++, e++) {
        if (e->unk_0 == idx) {
            pos[0] = e->unk_4[0];
            pos[1] = e->unk_4[1];
            pos[2] = e->unk_4[2];
            rot[0] = e->unk_A[0];
            rot[1] = e->unk_A[1];
            rot[2] = e->unk_A[2];
            D_800A3208 = 1;
            func_80032854(e->unk_1, e->unk_2, pos, rot);
            D_800A3208 = 0;
        }
    }

    if (D_800A3782 != 0) {
        temp = 0x77 - D_800A37D0;
    } else {
        s32 next_count = D_800A37D0 + 1;
        temp = D_800A36F8 - next_count;
    }
    s = D_800F68E0;
    for (i = 0; i < 0xB4; i++, s++) {
        if (s->unk_0 >= temp && s->unk_0 - s->unk_2 <= temp) {
            pos[0] = s->unk_4[0];
            pos[1] = s->unk_4[1];
            pos[2] = s->unk_4[2];
            rot[0] = s->unk_A;
            rot[1] = s->unk_C;
            rot[2] = s->unk_E;
            func_80049718(s->unk_3, 1, pos, rot);
        }
    }

    func_80046DA8(1);
    snd_UpdateStageSounds();
    D_800A37D0++;
    if ((D_800A3782 != 0 ? D_800A37D0 == 0x78 : D_800A37D0 == D_800A36F8) ||
        (g_pad_state.pressed & 0x400040)) {
        switch (D_800A38DC) {
        case 0:
            if (D_80101EC8[0].unk_96 == 0) {
                s32 var_19 = D_800A3836 != 0xFF;
                if (D_800A3712 != 0) {
                    D_800A38D4 = 2;
                    if (D_800A3836 != 0xFF) {
                        D_800A37A4 |= 1 << D_8008D538[(s8)D_80102778.unk_4[0]];
                    }
                    if (var_19) {
                        func_8003B328();
                    }
                }
                func_8001DA2C();
                if (D_800A3894 != 0) {
                    if (D_800A3712 != 0) {
                        func_8003B534(var_19 ? 3 : 6);
                    } else {
                        func_8003B484(D_800A3894 + 6);
                        func_8003B534(var_19 ? 2 : 5);
                    }
                } else {
                    func_8003B5A4();
                }
                return;
            }
            D_800A3748 = 1;
            if (D_800A3836 == 0xFF) {
                D_800A3834 = 0xC;
                break;
            }
            func_8001DA2C();
            func_8003B328();
            func_8003AF40(0);
            func_8003AFFC();
            if (D_800A3712 != 0 && D_80101EC8[1].unk_96 != 0) {
                func_8003B534(6);
            } else {
                func_8003B534(4);
            }
            return;
        case 3:
            if (D_80101EC8[0].unk_96 == 0) {
                D_800A3834 = 0x1E;
            } else {
                D_800A3834 = 0xC;
            }
            break;
        default:
            if (D_80101EC8[0].unk_96 == 0 || D_80101EC8[1].unk_96 == 0) {
                D_800A3834 = 0x10;
            } else {
                D_800A3834 = 0xC;
            }
            break;
        }
    }
}

void comb_Init(void) {
    s32 neg1;
    EnterCriticalSection();
    neg1 = -1;
    do {
        g_comb_event_ioer = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    } while (g_comb_event_ioer == neg1);
    neg1 = -1;
    do {
        g_comb_event_error = OpenEvent(0xF000000B, 0x8000, 0x2000, 0);
    } while (g_comb_event_error == neg1);
    ExitCriticalSection();
    neg1 = -1;
    VSync(2);
    AddCOMB();
    do {
        g_comb_write_fd = open(&g_str_sio_800A3210, 2);
    } while (g_comb_write_fd == neg1);
    neg1 = -1;
    do {
        g_comb_read_fd = open(&g_str_sio_800A3210, 0x8001);
    } while (g_comb_read_fd == neg1);
    _comb_control(2, 0, 0);
    _comb_control(1, 3, 0xE100);
    _comb_control(1, 4, 1);
}

void comb_Close(void) {
    close(g_comb_read_fd);
    close(g_comb_write_fd);
    EnterCriticalSection();
    CloseEvent(g_comb_event_ioer);
    CloseEvent(g_comb_event_error);
    ExitCriticalSection();
    VSync(2);
    DelCOMB();
    _comb_control(1, 1, 0);
}

s32 comb_IsCtsDsrClear(void) { return (_comb_control(0, 0, 0) & 0x180) == 0; }

void comb_ReadCtsSetRts(void) {
    if (_comb_control(3, 1, 0) != 0) {
        D_800A38A0 = 1;
    } else {
        D_800A38A0 = 0;
    }
    _comb_control(3, 0, 1);
}

void comb_EnableEvents(void) {
    EnableEvent(g_comb_event_error);
    EnableEvent(g_comb_event_ioer);
    D_800A320C = 1;
    g_comb_close_request = 0;
}

void comb_ResetClose(void) {
    D_800A320C = 0;
    g_comb_close_request = 0;
    _comb_control(2, 0, 0);
    _comb_control(1, 1, 0);
    comb_Close();
    D_800A3834 = 8;
}

void comb_Abort(void) {
    comb_ResetClose();
    g_comb_abort_request = 1;
}

void comb_RequestClose(void) { g_comb_close_request = 1; }

s32 comb_WriteWaitCallback(s32 a0, u32 a1) {
    if (a1 > 0x10000) {
        D_800A382C = 0;
        return 0;
    }
    return 1;
}

s32 comb_Write8(void) {
    s32 s1;
    s32 s0;

    s1 = GetRCnt(0xF2000001);
    if (s1 >= 0x401) {
        ResetRCnt(0xF2000001);
        s1 = 0;
    }

    while (1) {
        while (1) {
            if (_comb_control(3, 1, 0) != 0) {
                break;
            }
            if (GetRCnt(0xF2000001) - s1 >= 0x7801) {
                return 0;
            }
        }

        s0 = 0;
        do {
            if (_comb_control(3, 1, 0) == 0) {
                break;
            }
            s0++;
        } while (s0 < 1000);

        if (s0 >= 1000) {
            break;
        }
    }

    _comb_control(1, 1, 1);
    D_800A382C = 1;
    _comb_control(4, 0, (s32)&comb_WriteWaitCallback);
    write(g_comb_write_fd, &g_comb_send_buf, 8);
    _comb_control(4, 0, 0);
    _comb_control(1, 1, 0);
    return D_800A382C;
}

void comb_Read8(void) { read(g_comb_read_fd, g_comb_recv_buf, 8); }

s32 comb_WaitRead8(void) {
    s32 s0;
    s32 s1;
    s32 a1;
    s32 a0;
    s32 v0;

    s1 = 0;
    s0 = GetRCnt(0xF2000001);
    if (s0 >= 0x401) {
        goto overflow;
    }
    goto loop_check;
overflow:
    ResetRCnt(0xF2000001);
    s0 = 0;
loop_check:
    if (TestEvent(g_comb_event_ioer) != 0) {
        goto success;
    }
    if (TestEvent(g_comb_event_error) == 0) {
        goto poll;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    _comb_control(2, 0, 0);
    s0 = 0;
    comb_Read8();
    ResetRCnt(0xF2000001);
poll:
    if (((_comb_control(0, 0, 0) >> 7) & 3) == 1) {
        goto loop_check;
    }
    if (GetRCnt(0xF2000001) - s0 < 0x3C01) {
        goto loop_check;
    }
    s1 += 1;
    if (s1 >= 5) {
        goto ret0_tramp;
    }
    goto overflow;
success:
    a1 = g_comb_recv_buf[0];
    a0 = g_comb_recv_buf[1];
    v0 = a1 ^ (a1 >> 16);
    v0 = v0 ^ (a0 >> 16);
    v0 = v0 & 0xFFFF;
    if ((a0 & 0xFFFF) == v0) {
        goto match;
    }
    D_800A38D0 += 1;
    return 0;
ret0_tramp:
    return 0;
match:
    D_800A36C0 = a1;
    D_800A36C4 = a0;
    return 1;
}

s32 math_Popcount32(u32 arg0) {
    s32 count = 0;
    s32 i;
    for (i = 0; i < 32; i++) {
        count += (arg0 >> i) & 1;
    }
    return count;
}

extern s32 D_800A36D0;

typedef s32 (*FuncBufType)(void *);

/* Clears g_comb_wait_state while D_800A320C is 0. Otherwise packs bits of
 * D_800A38A0, g_comb_close_request and g_comb_wait_state, a0->type[0] and the
 * low half of a0->held into g_comb_send_buf, puts D_800A37C4 and a 16-bit xor
 * check in g_comb_send_buf_plus_0x4, and calls comb_Write8, comb_Read8 or
 * comb_WaitRead8 as D_800A3916 and D_800A38A0 select, returning early when
 * a transfer fails or after comb_ResetClose. If D_800A3916 is then 0, it
 * stores new values to a0->held and a0->type. FAKE: the tail's u16 loads
 * reuse the buf8 local; own locals: score 33. */
void comb_ExchangePad(PadState *a0) {
    s32 buf8;
    s32 var_1;
    s32 hi16;
    s32 flag;
    s32 var_4;
    s32 c0lo;
    s32 t;
    /* FAKE: constant-holder displaces the g_comb_send_buf_plus_0x4 store from
     * the slot before the branch (emits nothing); the literal: score 3 */
    s32 zero;

    if (D_800A320C != 0) {
        buf8 = a0->held;
        var_4 = D_800A38A0;
        zero = 0;
        var_1 = (var_4 << 31) | (g_comb_close_request << 30) |
                ((g_comb_wait_state & 3) << 28) | (a0->type[0] << 16) |
                (buf8 & 0xFFFF);
        g_comb_send_buf = var_1;
        hi16 = D_800A37C4 << 16;
        var_1 = var_1 ^ (var_1 >> 16);
        var_1 = var_1 ^ (hi16 >> 16);
        var_1 = var_1 & 0xFFFF;
        hi16 = hi16 | var_1;
        g_comb_send_buf_plus_0x4 = hi16;
        flag = D_800A3916;

        if (flag != 0) {
            if (var_4 == 0) {
                comb_Read8();
            } else {
                if (((FuncBufType)comb_Write8)(&g_comb_send_buf) == 0) {
                    comb_Abort();
                    return;
                }
                D_800A3908 += math_Popcount32(buf8 & 0xFFFF);
                comb_Read8();
            }
        } else {
            if (comb_WaitRead8() == 0) {
                comb_Abort();
                return;
            }
            if (D_800A38A0 == 1) {
                if (D_800A36C0 & 0x40000000) {
                    comb_ResetClose();
                    return;
                }
                if (D_800A36D0 & 0x40000000) {
                    comb_ResetClose();
                    return;
                }
            }
            if (((FuncBufType)comb_Write8)(&g_comb_send_buf) == 0) {
                comb_Abort();
                return;
            }
            D_800A3908 += math_Popcount32(buf8 & 0xFFFF);
            comb_Read8();
            if (D_800A38A0 == 0) {
                if (g_comb_close_request != zero || (D_800A36C0 & 0x40000000)) {
                    comb_ResetClose();
                    return;
                }
            }
        }

        if (D_800A3916 == 0) {
            /* FAKE: the record's held / type stores go through pointers; stored
               as members (in-struct memory) sched moves the D_800A36C2 /
               D_800A36D2 loads above them (score 22). */
            u32 *var_8 = &a0->held;
            s16 *type = a0->type;

            D_800A38FC += math_Popcount32((u16)D_800A36C0);
            c0lo = (u16)D_800A36C0;
            if (D_800A38A0 == 0) {
                buf8 = (u16)g_comb_send_buf;
                *var_8 = (c0lo << 16) | buf8;
                t = D_800A36C2;
                type[1] = t & 0xF;
            } else {
                buf8 = (u16)D_800A36D0;
                *var_8 = (buf8 << 16) | c0lo;
                t = D_800A36C2;
                type[0] = t & 0xF;
                t = D_800A36D2;
                type[1] = t & 0xF;
            }
            if (D_800A38A0 == 0) {
                if (((D_800A36C0 >> 28) & 3) == 2 && g_comb_wait_state == 2) {
                    g_comb_wait_state = 0;
                }
            } else {
                if (((D_800A36C0 >> 28) & 3) == 2 &&
                    ((D_800A36D0 >> 28) & 3) == 2) {
                    g_comb_wait_state = 0;
                }
            }
        }
        D_800A3916 = 0;
        D_800A36D0 = g_comb_send_buf;
        D_800A36D4 = g_comb_send_buf_plus_0x4;
    } else {
        g_comb_wait_state = 0;
    }
}

void comb_ExchangeEmptyPad(void) {
    PadState pad;
    pad.held = 0;
    pad.type[1] = 4;
    pad.type[0] = 4;
    comb_ExchangePad(&pad);
}

void comb_BeginWait(void) {
    g_comb_wait_state = 1;
    VSync(2);
    comb_ExchangeEmptyPad();
    VSync(2);
}

void comb_EndWait(void) {
    s32 val;
    g_comb_wait_state = 2;
    VSync(2);
    val = 2;
    do {
        comb_ExchangeEmptyPad();
        if (D_800A320C == 0) {
            goto end;
        }
        ResetRCnt(0xF2000001);
        do {
        } while (GetRCnt(0xF2000001) < 0x100);
    } while (g_comb_wait_state == val);
end:
    VSync(2);
    comb_ExchangeEmptyPad();
    VSync(2);
}

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A31F0 = (s32)D_80010AAC;
s32 D_800A31F4 = 0;
s32 D_800A31F8 = 0;
u8 D_800A31FC = 0;
u8 D_800A31FD = 0;  /* not named by any code or data: size from the gap */
s16 D_800A31FE = 0; /* not named by any code or data: size from the gap */
u8 D_800A3200 = 0x81;
u8 D_800A3201 = 0x7b;
u8 D_800A3202 = 0; /* not named by any code or data: size from the gap */
u8 D_800A3203 = 1;
u8 D_800A3204 = 1;
u8 D_800A3205 = 1;
u8 D_800A3206 = 0;
u8 D_800A3207 = 0;
u8 D_800A3208 = 0;
u8 D_800A3209 = 0;
s16 D_800A320A = 0; /* not named by any code or data: size from the gap */
u8 D_800A320C = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
s32 g_comb_recv_buf[2];
s32 g_comb_send_buf;
s32 D_800A36C0;
s32 D_800A36D0;
Rec1C (*D_800A36EC)[2];
u8 D_800A36F8;
s16 D_800A3714;
s32 g_comb_close_request;
s32 g_comb_read_fd;
s32 g_comb_event_ioer;
s32 g_comb_write_fd;
u8 D_800A3782;
u8 D_800A379C;
s16 D_800A379E;
u16 D_800A37C4;
s16 D_800A37C8;
u8 D_800A37D0;
s32 g_comb_event_error;
s16 D_800A3814;
u8 D_800A382C;
s32 g_comb_wait_state;
u8 D_800A38CC;
s32 D_800A38D0;
s32 D_800A38FC;
s32 D_800A3908;
u8 D_800A3916;
