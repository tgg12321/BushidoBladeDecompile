/* memcard_Format, 30 game functions, then the link-cable wrappers comb_Init..comb_WaitRead8 and
 * math_Popcount32. .text 0x80037F08 (ROM 0x28708). Start boundary: LEGACY (a tooling split, no
 * evidence either way). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Extern data declarations */
extern u8 D_800F33D8[];




/* Extern function declarations */






extern void VSync(s32);




extern void game_Cleanup(void);




















































extern s32 g_str_sio_800A3210;
extern void AddCOMB(void);







extern s32 D_800A36C0;












/* GP-relative extern data (for decompiled functions) */
extern s32 D_800A3730;
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
extern s32 D_800A36EC;
extern u8 D_800A3782;

extern u8 D_800A31FC;


extern s32 D_800A3870;
extern s32 g_comb_recv_buf;
extern s32 g_comb_send_buf;
extern u8 D_800A37D0;



/* Extern function declarations for decompiled functions */
extern s32 TestEvent(s32);
extern void CloseEvent(s32);
extern void EnableEvent(s32);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void read(s32, s32 *, s32);
extern void close(s32);


extern void ResetRCnt(s32);
extern s32 GetRCnt(s32);

extern void DelCOMB(void);

extern void func_8003E22C(void);
extern void func_8003F218(s32);
extern s32 math_FovToScreenDist(s32);
extern void SetGeomScreen(s32);
extern void func_8001B6F4(void);



extern s32 D_800F34D8;
extern s32 D_800A31F0;
extern s32 g_memcard_fd;
extern s32 memcard_CountFiles(s32, s32);

extern s32 func_80037AA4(void);
extern s32 func_80037B00(s32);
extern s32 memcard_ReadFile(s32, s32, s32, void *, s32);
extern s32 memcard_WriteFile(s32, s32, s32, void *, s32, s32, s32);

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

s32 memcard_Format(s32 a0, s32 a1) {
    s32 buf[2];
    sprintf(buf, &D_800109C8, a0, a1);
    return format(buf);
}

void func_80037F40(u8 *a0) {
    s32 checksum;
    u8 *p;
    s32 i;

    checksum = 0;
    p = (u8 *)&D_80106A50;
    i = 0;
    do {
        checksum += *p++;
        i++;
    } while ((u32)i < 0x24);

    {
        u8 *base = a0;
        i = 0;
        do {
            ((FileRecord *)base)[i] = D_80106A50;
            *(s32 *)(base + i * 4 + 0x6C) = checksum;
            {
                s32 j = 0;
                s16 *hp = (s16 *)base;
                u8 *bp = base;
                do {
                    *(s32 *)(bp + 0x78) = 0;
                    *(s16 *)((u8 *)hp + 0xD0) = 0;
                    hp++;
                    j++;
                    bp += 4;
                } while (j < 0x16);
            }
            i++;
        } while (i < 3);
        *(s32 *)(base + 0xFC) = 0;
    }
}

s32 func_8003800C(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;
    /* FAKE: one counter 'j' serves both the per-record checksum loop and the
       0x16-entry fixup loop (C89 counter reuse), mechanism: global.c
       allocno_compare -- the merged live range lifts reg_live_length(j) so j's
       allocno priority falls below sum's and sum takes $a0 (target's seat). */
    s32 j;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;

        sum = 0;
        bp = base + offset;
        j = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;

        if (!(*(src + 0x23) & 0x80)) {
            D_80106A50 = *(FileRecord *)src;
        }

        j = 0;
        do {
            u16 *ptr = *(u16 **)(base + j * 4 + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(base + j * 2 + 0xD0);
            }
            j++;
        } while (j < 0x16);
    }

    return 1;
}

void func_80038148(void) {
    u8 *p = D_800F33D8;
    s32 i = 0;
    do {
        *p = 0;
        i++;
        p++;
    } while ((u32)i < 0x200);
}
extern u8 D_8008F1C0[];
/* Rodata moved from asm/data/101C.rodata_pre_post.s. func_80038170 (this file) is the
 * sole owner — uses these as &-addressed byte/word lookups. Declared as u32
 * arrays since the content is word-aligned. D_80010A2C is the 128 bytes func_80038170 copies
 * (0x40 halfwords); the save-file id after it is its own object, D_80010AAC (D_800A31F0 holds
 * its address); the 3 zero bytes after its terminator are rodata padding. */
const u32 D_800109EC[16] = {
    0x77DF7FFF, 0x635E6B9F, 0x56FD5B1E, 0x427C4ABD,
    0x2DFB323C, 0x199B21BB, 0x0D3A115A, 0x8000051A,
    0, 0, 0, 0, 0, 0, 0, 0,
};
const u32 D_80010A2C[32] = {
    0x00000000, 0x00055110, 0x00000000, 0x003EC5A3,
    0x38531000, 0x002958EA, 0x7DBA8400, 0x00038DDC,
    0xB7411000, 0x001CBBDD, 0xCDDB9400, 0x000111AD,
    0x668EE800, 0x0002008E, 0x6216A900, 0x0017008E,
    0x6AC63210, 0x001800AE, 0x449A8410, 0x002903DD,
    0x38500000, 0x006B08EB, 0x06D93000, 0x006B5DE7,
    0x006CE500, 0x006DCD91, 0x0009C300, 0x007EEA20,
    0x00001000, 0x008EB200, 0x00000000, 0x00020000,
};
const char D_80010AAC[] = "BASLUS-00663BUSHIDO2";
extern u8 D_800A3200;
extern u8 D_800A3201;
extern u8 *strcpy(u8 *, u8 *);

void func_80038170(u8 *out) {
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
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
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
        u16 *src = (u16 *)&D_800109EC;
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
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}

void func_800383A4(void) {
    s32 var_v1;
    s32 var_v0;
    s32 temp_s0;
    s32 var_s1;
    u16 temp_v0;

    var_v1 = D_800A31F4;
    if (var_v1 != 1) goto state_other;

    var_v1 = D_800A31F8;
    if (var_v1 == -1) goto sub_inc;

    if (var_v1 >= 0) goto positive_path;
    if (var_v1 == -2) goto neg2_handler;
    return;

positive_path:
    if (var_v1 >= 3) return;
    if (var_v1 <= 0) return;
    if (D_800A37C8 == 0) { D_800A31F4 = 5; return; }
    if (D_800A37C8 == 3) goto set_state_7;
    D_800A31F4 = 3;
    return;

neg2_handler:
    if (D_800A37C8 != 3) goto neg2_else;
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
    if ((s16)temp_v0 < 4) return;
    var_v0 = 8;
    goto finish;

state_other:
    var_v0 = 5;
    if (var_v1 == var_v0) goto state_5;
    if (var_v1 < 6) {
        var_v0 = 3;
        if (var_v1 == var_v0) goto state_3;
        return;
    }
    var_v0 = 7;
    if (var_v1 == var_v0) goto state_7;
    return;

state_3:
    D_800A379E = 1;
    memcard_CountFiles(0, 0);
    temp_s0 = func_80037AA4();
    if (func_80037B00(D_800A31F0) != 0) {
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
    func_80038148();
    func_80038170(D_800F33D8);
    func_80037F40(D_800F33D8 + 0x100);
    if (memcard_WriteFile(0, 0, D_800A31F0, D_800F33D8, 1, 0x200, var_s1) != 0) {
        close(g_memcard_fd);
        var_v0 = 3;
        goto finish;
    }
    D_800A31F4 = 4;
    return;

state_5:
    memcard_CountFiles(0, 0);
    func_80037AA4();
    if (func_80037B00(D_800A31F0) == 0) {
        var_v0 = 0xE;
        goto finish;
    }
    D_800A379E = 4;
    func_80038148();
    if (memcard_ReadFile(0, 0, D_800A31F0, D_800F33D8, 0x200) != 0) {
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

extern s32 func_8003800C(s32 *);
/* func_80038658 — CD-load/save state-machine completion handler: dispatches
 * on D_800A31F4 (state 4 = post-read, state 6 = post-write), reaps
 * func_800378A8()'s status, closes the file handle, and posts a result code
 * to D_800A379E. The ret==0 ("still pending") paths route through the shared
 * fail_store end label (the shared-end-label recipe,
 * .claude/rules/shared-end-label.md) so GCC cannot constant-fold the
 * per-state fail codes. */
void func_80038658(void) {
    s32 ret;
    s32 fail;

    switch (D_800A31F4) {
    case 4:
        ret = memcard_PollSwEvents();
        if (ret == 0) {
            fail = 1;
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
            fail = 4;
            goto fail_store;
        }
        close(g_memcard_fd);
        if (ret == 1) {
            D_800A379E = 5;
            if (func_8003800C(&D_800F34D8) == 0) {
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
    D_800A379E = fail;
}
s32 func_80038734(void) {
    if ((u32)D_800A31F4 < 2) {
        D_800A31F8 = func_80037D14(0, 0);
    }
    func_800383A4();
    func_80038658();
    return D_800A379E;
}
void func_8003877C(void) {
    D_800A379E = 4;
    D_800A3814 = 0;
    D_800A37C8 = 0;
    D_800A31F4 = 1;
}
void func_8003879C(void) {
    D_800A379E = 1;
    D_800A37C8 = 1;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387C0(void) {
    D_800A379E = 1;
    D_800A37C8 = 2;
    D_800A38CC = 0;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
void func_800387E8(void) {
    D_800A379E = 9;
    D_800A37C8 = 3;
    D_800A3814 = 0;
    D_800A31F4 = 1;
}
extern u8 D_800A3203;
extern u8 D_800A31FC;
extern void func_8003877C(void);
extern s32 func_80038734(void);
extern void func_8006BEC4(s32, s32);

s32 func_8003880C(void) {
    s32 s0;
    s32 v0;

    s0 = 0;
    if (D_800A3203) {
        D_800A3203 = 0;
        D_800A31FC = 1;
        func_8003877C();
    }
    v0 = func_80038734();
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
/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
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
    u32 buttons;
    if (D_800A3204 != 0) {
        D_800A3204 = 0;
        D_800A3318 = 0;
    }
    buttons = g_pad_state.pressed;
    if (buttons & 0x400040) {
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A3318 == 0) {
            result = 1;
        } else {
            result = -1;
        }
    } else if (buttons & (u32)0x80008000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3318 = 0;
    } else if (buttons & 0x20002000) {
        func_8005C650(0, 0x7F, 0x7F);
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
    s32 sel;

    if (D_800A3205) {
        D_800A331C = 0;
        D_800A3320 = 0;
        D_800A3324 = 0;
        D_800A3328 = 0;
        D_800A332C = 0;
        D_800A31FC = 1;
        func_8003877C();
        D_800A3205 = 0;
        D_800A3330 = 0x5A;
        D_800A3334 = 0;
        D_800A3338 = 0;
        D_800A333C = 0;
    }

    v0 = func_80038734();

    if (D_800A333C != 0) {
        func_8006BEC4(0xA, -1);
        goto timer;
    }

    sel = 0;
    if (D_800A31FC == 0) {
        if (D_800A3338 == 1) {
            switch (v0 - 4) {
            case 0: sel = 2; break;
            case 1: sel = 3; break;
            case 2:
            case 6:
            case 10:
            case 11: sel = 4; break;
            case 4: sel = -1; break;
            default: sel = 0; break;
            }
        } else {
            switch (v0 - 4) {
            case 0: sel = 2; break;
            case 1: sel = 3; break;
            case 2: sel = 4; break;
            case 10: sel = 1; break;
            case 11: sel = 0xB; break;
            case 4: sel = -1; break;
            case 6: sel = 0x14; break;
            default: sel = 0; break;
            }
        }
    }

    if (sel >= 0) {
        func_8006BEC4(sel, -1);
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
            func_8003877C();
            break;
        case 11:
            D_800A332C++;
            if ((u8)D_800A332C >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        case 6:
            D_800A3328++;
            if ((u8)D_800A3328 >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        case 10:
            D_800A3324++;
            if ((u8)D_800A3324 >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
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
            func_8003877C();
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
        func_8005C650(2, 0x7F, 0x7F);
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
    extern void func_8006BEC4(s32, s32);
    extern void func_8005C650(s32, s32, s32);
    extern void func_8003877C(void);
    extern void func_8003879C(void);
    extern void func_800387C0(void);
    extern void func_800387E8(void);
    s32 result = 0;
    s32 sel2 = -1;
    s32 v0;
    s32 sel;

    if (!D_800A3207) {
        D_800A3207 = 1;
        D_800A334C = 0x5A;
        D_800A3350 = 0;
        D_800A3354 = 0;
        D_800A31FC = 0;
    }

    v0 = func_80038734();

    if (D_800A3354 != 0) {
        func_8006BEC4(0xA, -1);
        D_800A334C--;
        if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
            func_8005C650(2, 0x7F, 0x7F);
            D_800A3207 = 1;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            D_800A3354 = 0;
            D_800A31FC = 0;
        }
        goto end;
    }

    /* FAKE: the two empty arms reproduce the target's dead `== 2` / `== 3` tests that branch straight to the
     * join; without them those 4 insns are gone (score 5; a switch scores 14) */
    if (D_800A3207 == 1) {
        v0 = 0;
    } else if (D_800A3207 == 2) {
    } else if (D_800A3207 == 3) {
    } else if (D_800A3207 == 4) {
        v0 = 0x11;
    }

    sel = 0;
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
        sel = 0x11;
        sel2 = D_800A3350; /* FAKE: duplicate of load_sel2's store — jump2 cross-jump re-merges it
                              (zero emitted bytes); the extra real def lifts sel2's reg_n_refs
                              priority above result's so RA lands sel2->$s2 / result->$s3 (target).
                              SOTN duplicate-into-arms family. */
        goto sel_dispatch;
    case 13:
    case 17:
        sel = 6;
    load_sel2:
        sel2 = D_800A3350;
        goto sel_dispatch;
    case 1:
        sel = (-(D_800A38CC != 0)) & 7;
        goto sel_dispatch;
    case 2:
        sel = 8;
        goto sel_dispatch;
    case 3:
        sel = 9;
        goto sel_dispatch;
    case 8:
    case8_sel:
        sel = -1;
        goto sel_dispatch;
    case 7:
        sel = 5;
        goto sel_dispatch;
    case 10:
        if (D_800A3206 == 0) {
            D_800A3350 = 1;
        }
        if (g_pad_state.pressed & 0x400040) {
            D_800A3206 = 0;
            func_8005C650(1, 0x7F, 0x7F);
            sel = 0xD;
            if (D_800A3350 == 0) {
                func_800387E8();
                goto sel_dispatch;
            }
            v0 = 0;
            D_800A3207 = 5;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            sel = -1;
            goto sel_dispatch;
        }
        sel = 0xC;
        if ((g_pad_state.pressed & 0xA000A000U) != 0) {
            D_800A3206 = 1;
        }
        goto load_sel2;
    case 9:
    case 11:
    case9_11_sel:
        sel = 0xD;
        goto sel_dispatch;
    case 12:
    case12_sel:
        sel = 0xF;
        goto sel_dispatch;
    default:
        sel = 0;
        goto sel_dispatch;
    }

sel_dispatch:
    if (sel >= 0) {
        func_8006BEC4(sel, sel2);
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
            func_8003877C();
            break;
        case 1: case 2: case 3:
        case 9: case 10: case 11:
            D_800A3340++;
            if (((u8)D_800A3340) >= 5) {
                D_800A31FC = 0;
                func_8003879C();
                break;
            }
            func_8003877C();
            break;
        case 6:
            D_800A3344++;
            if (((u8)D_800A3344) >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
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
        case 1: case 2: case 3:
        case 5: case 7: case 8: case 9: case 10: case 11:
        default:
            func_8003879C();
            D_800A3207 = 2;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 6:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
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
        case 2: case 3: case 7: case 12:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (g_pad_state.pressed & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
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
            func_8003877C();
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
                func_8005C650(1, 0x7F, 0x7F);
                if (v0 == 0) {
                    if (D_800A3350 != 0) {
                        result = 1;
                        break;
                    }
                    D_800A3348 = 0;
                    D_800A3340 = 0;
                    D_800A3344 = 0;
                    D_800A31FC = 1;
                    func_8003877C();
                } else {
                    if (D_800A3350 != 0) goto area_c_long;
                    func_800387C0();
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
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3350 = 0;
                break;
            }
            if (g_pad_state.pressed & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
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
s32 *func_800392B8(void) {
    return (s32 *)D_800F33D8;
}
void func_800392C8(void) {
    /* FAKE: constant holder for the 0xFF fill; with the literal, `li 255` is scheduled after `li 0x1F0` (score 2) */
    u8 fill;
    s32 i;
    s32 j;

    fill = 0xFF;
    i = 0x1F0;
    D_800A36EC = (u8 *)D_800F33D8;
    D_800A36F8 = 0;
    D_800A3782 = 0;
    do {
        *(&D_80101BF0 + i) = fill;
        i -= 0x10;
    } while (i >= 0);
    j = 0xB30;
    do {
        *((s16 *)((u8 *)D_800F68E0 + j)) = -1;
        j -= 0x10;
    } while (j >= 0);
}
void func_80039320(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    u8 *p;
    s16 *q;
    s16 val;
    s16 newval;

    i = 0;
    newval = 0xFF;
    p = &D_80101BF0;

    do {
        if (*p == D_800A36F8) {
            *p = newval;
        }
        i++;
        p += 0x10;
    } while (i < 0x20);

    q = D_800F68E0;
    i = 0;
    do {
        val = *q;
        if (val != -1) {
            newval = val + 1;
            *q = newval;
            if ((s16)newval - *(u8 *)((u8 *)q + 2) >= 0x101) {
                *q = -1;
            }
        }
        i++;
        q = (s16 *)((u8 *)q + 0x10);
    } while (i < 0xB4);

    D_800A379C = 0;
    D_800A3714 = 0;
}
void func_800393C8(s32 arg0, s32 arg1, s32 *arg2, u16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    u8 *slot;
    s32 i;
    s16 idx;
    s16 cur;
    s32 state;
    u8 age;
    s32 raw;
    s32 rot;

    slot = (u8 *)D_800F68E0;
    i = 0;
    do {
        state = *(s16 *)(slot + 0);
        if (state != -1) {
            age = *(u8 *)(slot + 2);
            if (age == state - 1 && age < 0xFF) {
                raw = *(u16 *)(slot + 0xA) << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    *(s16 *)(slot + 4) == arg2[0] &&
                    *(s16 *)(slot + 6) == arg2[1] &&
                    *(s16 *)(slot + 8) == arg2[2] &&
                    ((rot - *(s16 *)(arg3 + 0)) & 0xFFF) == 0 &&
                    ((*(s16 *)(slot + 0xC) - *(s16 *)(arg3 + 1)) & 0xFFF) == 0 &&
                    ((*(s16 *)(slot + 0xE) - *(s16 *)(arg3 + 2)) & 0xFFF) == 0) {
                    *(u8 *)(slot + 2) = age + 1;
                    return;
                }
            }
        }
        i++;
        slot += 0x10;
    } while (i < 0xB4);

    idx = D_800A3714;
    slot = (u8 *)D_800F68E0 + idx * 0x10;
    while (idx < 0xB4) {
        if (*(s16 *)slot == -1) {
            break;
        }
        cur = idx + 1;
        D_800A3714 = cur;
        slot += 0x10;
        idx = cur;
    }

    if (D_800A3714 == 0xB4) {
        D_800A3209++;
        return;
    }

    *(s16 *)(slot + 0) = 0;
    *(u8 *)(slot + 3) = arg1;
    *(u8 *)(slot + 2) = 0;
    *(s16 *)(slot + 4) = arg2[0];
    *(s16 *)(slot + 6) = arg2[1];
    *(s16 *)(slot + 8) = arg2[2];
    *(u16 *)(slot + 0xA) = (arg3[0] & 0xFFF) | (arg0 << 12);
    *(u16 *)(slot + 0xC) = arg3[1];
    *(u16 *)(slot + 0xE) = arg3[2];
}
void func_800395B4(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    u8 *slot;
    u8 idx;
    u8 sentinel;

    if (D_800A3208 == 0) {
        idx = D_800A379C;
        slot = &D_80101BF0 + (u32)(idx & 0xFF) * 0x10;
        if ((u32)(idx & 0xFF) < 0x20U) {
            sentinel = 0xFF;
loop:
            if (*slot != sentinel) {
                D_800A379C = idx + 1;
                idx = idx + 1;
                slot += 0x10;
                if ((u32)(idx & 0xFF) < 0x20U) {
                    goto loop;
                }
            }
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            slot[1] = arg0;
            slot[2] = arg1;
            slot[0] = tmp;
            *(s16 *)&slot[4] = (s16)arg2[0];
            *(s16 *)&slot[6] = (s16)arg2[1];
            *(s16 *)&slot[8] = (s16)arg2[2];
            if (arg3 != NULL) {
                *(u16 *)&slot[0xA] = arg3[0];
                *(u16 *)&slot[0xC] = arg3[1];
                *(u16 *)&slot[0xE] = arg3[2];
            }
        }
    }
}
void func_80039680(u8 *a0) {
    s16 idx;
    u8 *base;
    u8 *dest;

    idx = *(s16 *)(a0 + 4);
    base = (u8 *)(D_800A36EC + D_800A36F8 * 56);
    dest = base + idx * 28;

    *(s16 *)(dest + 4) = *(s32 *)(a0 + 0xF4);
    *(s16 *)(dest + 8) = *(s32 *)(a0 + 0xFC);
    *(s16 *)(dest + 6) = *(s32 *)(a0 + 0xF8);

    {
        u16 v = *(u16 *)(a0 + 0x1CA);
        u8 b = *(u8 *)(a0 + 0xB3);
        *(s16 *)(dest + 0xA) = (v & 0xFFF) | (b << 12);
    }

    *(s16 *)(dest + 0xC) = *(s32 *)(a0 + 0x148);
    *(u8 *)(dest + 0x14) = *(u16 *)(a0 + 0x1E6) >> 2;
    *(u8 *)(dest + 0x15) = *(u16 *)(a0 + 0x1E8) >> 2;
    *(u8 *)(dest + 0x16) = *(u16 *)(a0 + 0x1EA) >> 2;
    *(s32 *)(dest + 0) = *(s32 *)(a0 + 0x50);
    *(u8 *)(dest + 0x17) = 0;

    if (*(u8 *)(a0 + 0x60) != 0) {
        *(u8 *)(dest + 0x17) = 1;
    }
    if (*(u8 *)(a0 + 0x61) != 0) {
        *(u8 *)(dest + 0x17) |= 2;
    }

    *(s16 *)(dest + 0xE) = *(u16 *)(a0 + 0x64);
    *(s16 *)(dest + 0x10) = *(u16 *)(a0 + 0x66);
    *(s16 *)(dest + 0x12) = *(u16 *)(a0 + 0x68);
    *(u8 *)(dest + 0x18) = *(u8 *)(a0 + 0x62);
    *(u8 *)(dest + 0x19) = *(u16 *)(a0 + 0x40);
}
void func_800397A0(void) {
    u8 val = D_800A36F8;
    if (val == 0x77) {
        D_800A36F8 = 0;
        D_800A3782 = 1;
    } else {
        D_800A36F8 = val + 1;
    }
}
void func_800397D4(void) {
    gpu_ResetGraphMode1();
    func_8003E22C();
    func_8003F218(0);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    func_80041688(0, 0);
    func_80041688(1, 0);
    func_8001B6F4();
    game_Cleanup();
    D_800A37D0 = 0;
    D_800A3834 = 5;
}
extern s32 func_80053584(s32 *, s32 *, s32 *, s32 *);
extern s32 func_80054434(void);
void func_8003984C(Unk80101EC8Record *arg0, s32 *arg1, s32 *arg2) {
    s32 sp10[3];
    s32 sp20[3];
    s32 sp30[4];
    s32 sp40[2];
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
    if (func_80053584(sp10, sp20, sp30, sp40) != 0) {
        result = func_80054434();
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
extern s32 camera_GetBoneData(void);
extern u8 D_800A3208;
extern u8 *D_800A3894;

extern void func_800207C8(Unk80101EC8Record *, LeafPos *, LeafPos *, LeafPos *);
void func_8003993C(void) {
    s32 work[2][33];
    s32 sp120[34];
    s32 pos[3];
    s16 rot[3];
    s32 sp1C0[100];
    s32 out_b1;
    s32 out_b2;
    s32 idx;
    s32 prog;
    s32 i;
    u8 *p;
    Unk80101EC8Record *rob;
    u8 *e;
    /* Ruling 11 (ordinary-c-judge-decidable.md): `temp` holds two values, the 0/1 weapon-set
     * selector (flags >> 1) & 1 in the per-player loop and the replay window of the event loop. */
    s32 temp;
    u8 save40;
    u8 *save58;

    if (D_800A3782 != 0) {
        idx = (D_800A36F8 + D_800A37D0) % 120;
        prog = (D_800A37D0 << 12) / 120;
    } else {
        idx = D_800A37D0;
        prog = (idx << 12) / D_800A36F8;
    }
    D_800A3778 = camera_GetBoneData();
    /* The frame record's address is written out at each argument (compound-address duplication,
     * no-new-park-categories F3): binding it to a `rec` local does not match. */
    func_8001BAE4((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
    func_8001BBD8((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
    func_8001E6E4(prog);

    for (i = 0; i < 2; i++) {
        /* Ruling 11 (ordinary-c-judge-decidable.md): `entry` holds two values, the address of the
         * frame's 4-byte entry in the practice weapon table (if arm) and in the character's weapon
         * table (else arm). */
        s32 entry;

        p = (u8 *)(D_800A36EC + idx * 56) + i * 28;
        rob = &D_80101EC8[i];
        func_800198D0((*(s16 *)(p + 0xE) >> 14) & 3, *(s16 *)(p + 0xE) & 0x3FFF, work[0], sp1C0);
        func_800198D0((*(s16 *)(p + 0x10) >> 14) & 3, *(s16 *)(p + 0x10) & 0x3FFF, work[1], sp1C0);
        func_8001F1C4(rob, p, work[0], work[1]);
        func_80041188(i, work[0], work[1], *(s16 *)(p + 0x12), sp120);
        pos[0] = *(s16 *)(p + 4);
        pos[1] = *(s16 *)(p + 6);
        pos[2] = *(s16 *)(p + 8);
        rot[0] = 0;
        rot[1] = *(u16 *)(p + 0xA);
        rot[2] = 0;
        func_80040D48(i, 1, pos, rot, 0, *(s16 *)(p + 0xC));
        if (*(u8 *)(p + 0x18) & 1) {
            func_80049718(rob->unk_12, i * 2 + 0x8000, 0, 0);
        }
        if (*(u8 *)(p + 0x18) & 4) {
            func_80049718(D_8008EB80[rob->unk_14], i * 2 + 0x8001, 0, 0);
        }
        if (*(u8 *)(p + 0x18) & 2) {
            func_80049A2C(rob->unk_12, i * 2, (*(u8 *)(p + 0x18) >> 4) & 1);
        }
        if (*(u8 *)(p + 0x18) & 8) {
            func_80049A2C(D_8008EB80[rob->unk_14], i * 2 | 1, (*(u8 *)(p + 0x18) >> 5) & 1);
        }
        func_800207C8(rob, SPAD->unkA8[i], SPAD->unk00[i], SPAD->unk48[i]);
        func_8003984C(rob, &out_b1, &out_b2);
        if (out_b1 != 1) {
            rob->unk_B1 = out_b1;
            if (out_b2 != -1) {
                rob->unk_B2 = out_b2;
            }
        }
        save40 = rob->unk_40;
        rob->unk_40 = *(u8 *)(p + 0x19);
        save58 = rob->unk_58;
        if (*(u8 *)(p + 0x17) & 1) {
            entry = D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4;
            rob->unk_58 = (u8 *)(D_80102768 + *(u16 *)(entry + 2));
        } else {
            temp = (*(u8 *)(p + 0x17) >> 1) & 1;
            entry = D_801027B0[temp][1] + *(u16 *)(*(s32 *)p + 4) * 4;
            rob->unk_58 = (u8 *)(D_801027B0[temp][2] + *(u16 *)(entry + 2));
        }
        if (*(u8 *)(p + 0x18) & 0x40) {
            func_8003339C(rob);
        }
        rob->unk_40 = save40;
        rob->unk_58 = save58;
        func_80040304(i, (*(u16 *)(p + 0xA) >> 12) & 7);
    }

    e = &D_80101BF0;
    for (i = 0; i < 0x20; i++, e += 0x10) {
        if (e[0] == idx) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            D_800A3208 = 1;
            func_80032854(e[1], e[2], pos, rot);
            D_800A3208 = 0;
        }
    }

    if (D_800A3782 != 0) {
        temp = 0x77 - D_800A37D0;
    } else {
        /* FAKE: named intermediate (Ruling 1, once-written). Unnamed, fold-const.c `associate`
         * (split_tree) rewrites D_800A36F8 - (D_800A37D0 + 1) as (D_800A36F8 - 1) - D_800A37D0 at
         * tree level; the target adds 1 to the counter first (addiu; subu). */
        s32 next = D_800A37D0 + 1;
        temp = D_800A36F8 - next;
    }
    e = (u8 *)D_800F68E0;
    for (i = 0; i < 0xB4; i++, e += 0x10) {
        if (*(s16 *)e >= temp && *(s16 *)e - e[2] <= temp) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            func_80049718(e[3], 1, pos, rot);
        }
    }

    func_80046DA8(1);
    func_800335D8();
    D_800A37D0++;
    if ((D_800A3782 != 0 ? D_800A37D0 == 0x78 : D_800A37D0 == D_800A36F8) || (g_pad_state.pressed & 0x400040)) {
        switch (D_800A38DC) {
        case 0:
            if (D_80101EC8[0].unk_96 == 0) {
                s32 valid = D_800A3836 != 0xFF;
                if (D_800A3712 != 0) {
                    D_800A38D4 = 2;
                    if (D_800A3836 != 0xFF) {
                        D_800A37A4 |= 1 << D_8008D538[(s8)D_80102778.unk_4[0]];
                    }
                    if (valid) {
                        func_8003B328();
                    }
                }
                func_8001DA2C();
                if (D_800A3894 != 0) {
                    if (D_800A3712 != 0) {
                        func_8003B534(valid ? 3 : 6);
                    } else {
                        func_8003B484(D_800A3894 + 6);
                        func_8003B534(valid ? 2 : 5);
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
s32 comb_IsCtsDsrClear(void) {
    return (_comb_control(0, 0, 0) & 0x180) == 0;
}
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
    D_800A3730 = 0;
}
void comb_ResetClose(void) {
    D_800A320C = 0;
    D_800A3730 = 0;
    _comb_control(2, 0, 0);
    _comb_control(1, 1, 0);
    comb_Close();
    D_800A3834 = 8;
}
void func_8003A3F0(void) {
    comb_ResetClose();
    D_800A3928 = 1;
}
void func_8003A41C(void) {
    D_800A3730 = 1;
}
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
void comb_Read8(void) {
    read(g_comb_read_fd, &g_comb_recv_buf, 8);
}
extern s32 D_800A38D0;
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
    a1 = g_comb_recv_buf;
    a0 = g_comb_recv_buf_plus_0x4;
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
extern s32 D_800A38A0;
extern s16 D_800A36C2;
extern s32 D_800A36D0;
extern s16 D_800A36D2;
extern s32 D_800A36D4;
extern s32 g_comb_send_buf_plus_0x4;
extern u16 D_800A37C4;
extern u8 D_800A3916;
extern s32 D_800A3908;
extern s32 D_800A38FC;

typedef s32 (*FuncBufType)(void *);

/* func_8003A728: one link-cable exchange. Packs the vsync/state bits, the record's first
 * halfword and the low half of its word at +8 into g_comb_send_buf, appends a 16-bit xor check (with D_800A37C4 in the high
 * half), then sends/receives through comb_Write8 / comb_Read8 (order set by D_800A3916).
 * On success it folds the partner's word back into the record at a0 and clears the
 * D_800A3870 handshake state once both sides report state 2.
 * `hi16 = hi16 | packed;` updates hi16 in place so the or prints `or a0,a0,v0` as in the
 * target. FAKE: the u16 low-half loads in the tail reuse the buf8 local (variable reuse
 * for codegen control). */
void func_8003A728(s32 a0) {
    s32 buf8;
    s32 packed;
    s32 hi16;
    s32 flag;
    s32 vsync;
    s32 c0lo;
    s32 t;
    /* FAKE: constant-holder local; mechanism: the (set (reg) (const_int 0)) survives into
     * sched1's block-1 ready lists and displaces the D_800A369C store from the slot before the
     * branch, then local-alloc.c update_equiv_regs deletes it (no insn emitted). */
    s32 zero;

    if (D_800A320C != 0) {
        buf8 = *(s32 *)(a0 + 8);
        vsync = D_800A38A0;
        zero = 0;
        packed = (vsync << 31) | (D_800A3730 << 30) | ((D_800A3870 & 3) << 28)
               | (*(s16 *)a0 << 16) | (buf8 & 0xFFFF);
        g_comb_send_buf = packed;
        hi16 = D_800A37C4 << 16;
        packed = packed ^ (packed >> 16);
        packed = packed ^ (hi16 >> 16);
        packed = packed & 0xFFFF;
        hi16 = hi16 | packed;
        g_comb_send_buf_plus_0x4 = hi16;
        flag = D_800A3916;

        if (flag != 0) {
            if (vsync == 0) {
                comb_Read8();
            } else {
                if (((FuncBufType)comb_Write8)(&g_comb_send_buf) == 0) {
                    func_8003A3F0();
                    return;
                }
                D_800A3908 += math_Popcount32(buf8 & 0xFFFF);
                comb_Read8();
            }
        } else {
            if (comb_WaitRead8() == 0) {
                func_8003A3F0();
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
                func_8003A3F0();
                return;
            }
            D_800A3908 += math_Popcount32(buf8 & 0xFFFF);
            comb_Read8();
            if (D_800A38A0 == 0) {
                if (D_800A3730 != zero || (D_800A36C0 & 0x40000000)) {
                    comb_ResetClose();
                    return;
                }
            }
        }

        if (D_800A3916 == 0) {
            D_800A38FC += math_Popcount32((u16)D_800A36C0);
            c0lo = (u16)D_800A36C0;
            if (D_800A38A0 == 0) {
                buf8 = (u16)g_comb_send_buf;
                *(s32 *)(a0 + 8) = (c0lo << 16) | buf8;
                t = D_800A36C2;
                *(s16 *)(a0 + 2) = t & 0xF;
            } else {
                buf8 = (u16)D_800A36D0;
                *(s32 *)(a0 + 8) = (buf8 << 16) | c0lo;
                t = D_800A36C2;
                *(s16 *)a0 = t & 0xF;
                t = D_800A36D2;
                *(s16 *)(a0 + 2) = t & 0xF;
            }
            if (D_800A38A0 == 0) {
                if (((D_800A36C0 >> 28) & 3) == 2 && D_800A3870 == 2) {
                    D_800A3870 = 0;
                }
            } else {
                if (((D_800A36C0 >> 28) & 3) == 2 && ((D_800A36D0 >> 28) & 3) == 2) {
                    D_800A3870 = 0;
                }
            }
        }
        D_800A3916 = 0;
        D_800A36D0 = g_comb_send_buf;
        D_800A36D4 = g_comb_send_buf_plus_0x4;
    } else {
        D_800A3870 = 0;
    }
}

void func_8003AA48(void) {
    s16 buf[12];
    *(s32 *)&buf[4] = 0;
    buf[1] = 4;
    buf[0] = 4;
    func_8003A728((s32)buf);
}
void func_8003AA78(void) {
    D_800A3870 = 1;
    VSync(2);
    func_8003AA48();
    VSync(2);
}
void func_8003AAB0(void) {
    s32 val;
    D_800A3870 = 2;
    VSync(2);
    val = 2;
    do {
        func_8003AA48();
        if (D_800A320C == 0) {
            goto end;
        }
        ResetRCnt(0xF2000001);
        do {
        } while (GetRCnt(0xF2000001) < 0x100);
    } while (D_800A3870 == val);
end:
    VSync(2);
    func_8003AA48();
    VSync(2);
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A31F0 = (s32)D_80010AAC;
s32 D_800A31F4 = 0;
s32 D_800A31F8 = 0;
u8 D_800A31FC = 0;
u8 D_800A31FD = 0;  /* not named by any code or data: size from the gap */
s16 D_800A31FE = 0;  /* not named by any code or data: size from the gap */
u8 D_800A3200 = 0x81;
u8 D_800A3201 = 0x7b;
u8 D_800A3202 = 0;  /* not named by any code or data: size from the gap */
u8 D_800A3203 = 1;
u8 D_800A3204 = 1;
u8 D_800A3205 = 1;
u8 D_800A3206 = 0;
u8 D_800A3207 = 0;
u8 D_800A3208 = 0;
u8 D_800A3209 = 0;
s16 D_800A320A = 0;  /* not named by any code or data: size from the gap */
u8 D_800A320C = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
s32 g_comb_recv_buf;
s32 g_comb_send_buf;
s32 D_800A36C0;
s32 D_800A36D0;
s32 D_800A36EC;
u8 D_800A36F8;
s16 D_800A3714;
s32 D_800A3730;
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
s32 D_800A3870;
u8 D_800A38CC;
s32 D_800A38D0;
s32 D_800A38FC;
s32 D_800A3908;
u8 D_800A3916;
