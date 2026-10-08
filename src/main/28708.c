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
extern Rec1C (*D_800A36EC)[2];
extern u8 D_800A3782;

extern u8 D_800A31FC;

extern s32 D_800A3870;
extern s32 g_comb_recv_buf;
extern s32 g_comb_send_buf;
extern u8 D_800A37D0;

extern Unk800F34D8Save D_800F34D8; /* = D_800F33D8 + 0x100 */
extern s32 D_800A31F0;

s32 memcard_Format(s32 a0, s32 a1) {
    s32 buf[2];
    sprintf(buf, (char *)D_800109C8, a0, a1);
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
        /* FAKE: the save-block view in its own local; a cast of a0 at each
           use moves the copy into the loop's delay slot (score 2) */
        Unk800F34D8Save *base = (Unk800F34D8Save *)a0;
        i = 0;
        do {
            base->rec[i] = D_80106A50;
            base->sum[i] = checksum;
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

s32 func_8003800C(Unk800F34D8Save *arg0) {
    s32 i;
    /* FAKE: one counter j for both loops lengthens its live range so sum,
       not j, gets $a0; two counters: score 16 */
    s32 j;

    i = 0;
    do {
        s32 sum;
        u8 *bp;

        sum = 0;
        bp = (u8 *)&arg0->rec[i];
        j = 0;
        do {
            sum += *bp;
            bp++;
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
/* func_80038170's lookup tables (word-aligned, so u32 arrays). D_80010A2C is
 * the 128 bytes func_80038170 copies; D_80010AAC is the save-file id
 * (D_800A31F0 holds its address), then 3 bytes of rodata padding. */
const u32 D_800109EC[16] = {
    0x77DF7FFF, 0x635E6B9F, 0x56FD5B1E, 0x427C4ABD, 0x2DFB323C, 0x199B21BB,
    0x0D3A115A, 0x8000051A, 0,          0,          0,          0,
    0,          0,          0,          0,
};
const u32 D_80010A2C[32] = {
    0x00000000, 0x00055110, 0x00000000, 0x003EC5A3, 0x38531000, 0x002958EA,
    0x7DBA8400, 0x00038DDC, 0xB7411000, 0x001CBBDD, 0xCDDB9400, 0x000111AD,
    0x668EE800, 0x0002008E, 0x6216A900, 0x0017008E, 0x6AC63210, 0x001800AE,
    0x449A8410, 0x002903DD, 0x38500000, 0x006B08EB, 0x06D93000, 0x006B5DE7,
    0x006CE500, 0x006DCD91, 0x0009C300, 0x007EEA20, 0x00001000, 0x008EB200,
    0x00000000, 0x00020000,
};
const char D_80010AAC[] = "BASLUS-00663BUSHIDO2";
extern u8 D_800A3200;
extern u8 D_800A3201;

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
    temp_s0 = func_80037AA4();
    if (func_80037B00((u8 *)D_800A31F0) != 0) {
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
    func_80037AA4();
    if (func_80037B00((u8 *)D_800A31F0) == 0) {
        var_v0 = 0xE;
        goto finish;
    }
    D_800A379E = 4;
    func_80038148();
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

extern s32 func_8003800C(Unk800F34D8Save *);

/* Load/save state-machine completion handler: dispatches on D_800A31F4
 * (4 = post-read, 6 = post-write), reaps memcard_PollSwEvents()'s status,
 * closes the file and posts a result code to D_800A379E. The still-pending
 * paths share the fail_store end label (the shared-end-label recipe,
 * .claude/rules/shared-end-label.md). */
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
            case 0:
                sel = 2;
                break;
            case 1:
                sel = 3;
                break;
            case 2:
            case 6:
            case 10:
            case 11:
                sel = 4;
                break;
            case 4:
                sel = -1;
                break;
            default:
                sel = 0;
                break;
            }
        } else {
            switch (v0 - 4) {
            case 0:
                sel = 2;
                break;
            case 1:
                sel = 3;
                break;
            case 2:
                sel = 4;
                break;
            case 10:
                sel = 1;
                break;
            case 11:
                sel = 0xB;
                break;
            case 4:
                sel = -1;
                break;
            case 6:
                sel = 0x14;
                break;
            default:
                sel = 0;
                break;
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
        /* FAKE: duplicate of load_sel2's store — jump2 cross-jump re-merges it
           (zero emitted bytes); the extra real def lifts sel2's reg_n_refs
           priority above result's so RA lands sel2->$s2 / result->$s3 (target).
           SOTN duplicate-into-arms family; `goto load_sel2` scores 13. */
        sel2 = D_800A3350;
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
        case 1:
        case 2:
        case 3:
        case 9:
        case 10:
        case 11:
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
        case 2:
        case 3:
        case 7:
        case 12:
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
                    if (D_800A3350 != 0)
                        goto area_c_long;
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

s32 *func_800392B8(void) { return (s32 *)D_800F33D8; }

void func_800392C8(void) {
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

void func_80039320(void) {
    extern u8 D_800A379C;
    extern s16 D_800A3714;
    s32 i;
    Unk80101BF0Rec *p;
    Unk800F68E0Rec *q;
    s16 val;
    s16 newval;
    /* FAKE: constant holder for the 0xFF free marker; the literal moves li 255
       below the table address (score 2) */
    u8 fill;

    i = 0;
    fill = 0xFF;
    p = D_80101BF0;

    do {
        if (p->unk_0 == D_800A36F8) {
            p->unk_0 = fill;
        }
        i++;
        p++;
    } while (i < 0x20);

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

void func_800393C8(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    Unk800F68E0Rec *slot;
    s32 i;
    s16 idx;
    s16 cur;
    s32 state;
    u8 seen;
    s32 raw;
    s32 rot;

    slot = D_800F68E0;
    i = 0;
    do {
        state = slot->unk_0;
        if (state != -1) {
            seen = slot->unk_2;
            if (seen == state - 1 && seen < 0xFF) {
                raw = slot->unk_A << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    slot->unk_4[0] == arg2[0] && slot->unk_4[1] == arg2[1] &&
                    slot->unk_4[2] == arg2[2] &&
                    ((rot - arg3[0]) & 0xFFF) == 0 &&
                    ((slot->unk_C - arg3[1]) & 0xFFF) == 0 &&
                    ((slot->unk_E - arg3[2]) & 0xFFF) == 0) {
                    slot->unk_2 = seen + 1;
                    return;
                }
            }
        }
        i++;
        slot++;
    } while (i < 0xB4);

    idx = D_800A3714;
    slot = &D_800F68E0[idx];
    while (idx < 0xB4) {
        if (slot->unk_0 == -1) {
            break;
        }
        cur = idx + 1;
        D_800A3714 = cur;
        slot++;
        idx = cur;
    }

    if (D_800A3714 == 0xB4) {
        D_800A3209++;
        return;
    }

    slot->unk_0 = 0;
    slot->unk_3 = arg1;
    slot->unk_2 = 0;
    slot->unk_4[0] = arg2[0];
    slot->unk_4[1] = arg2[1];
    slot->unk_4[2] = arg2[2];
    slot->unk_A = (arg3[0] & 0xFFF) | (arg0 << 12);
    slot->unk_C = arg3[1];
    slot->unk_E = arg3[2];
}

void func_800395B4(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern u8 D_800A3208;
    extern u8 D_800A379C;
    Unk80101BF0Rec *slot;
    u8 idx;
    /* FAKE: constant holder for the 0xFF free marker; with the literal the li
       is scheduled after the lbu and the load-delay nop is lost (score 4) */
    u8 sentinel;

    if (D_800A3208 == 0) {
        idx = D_800A379C;
        slot = &D_80101BF0[idx];
        if (idx < 0x20) {
            sentinel = 0xFF;
        loop:
            if (slot->unk_0 != sentinel) {
                D_800A379C = idx + 1;
                idx = idx + 1;
                slot++;
                if (idx < 0x20) {
                    goto loop;
                }
            }
        }
        if (D_800A379C != 0x20) {
            u8 tmp = D_800A36F8;
            slot->unk_1 = arg0;
            slot->unk_2 = arg1;
            slot->unk_0 = tmp;
            slot->unk_4[0] = arg2[0];
            slot->unk_4[1] = arg2[1];
            slot->unk_4[2] = arg2[2];
            if (arg3 != NULL) {
                slot->unk_A[0] = arg3[0];
                slot->unk_A[1] = arg3[1];
                slot->unk_A[2] = arg3[2];
            }
        }
    }
}

void func_80039680(Unk80101EC8Record *a0) {
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

extern s32 func_80054434(void);

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

extern u8 D_800A3208;

void func_8003993C(void) {
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
    s32 prog;
    s32 i;
    Rec1C *p;
    Unk80101EC8Record *rob;
    Unk80101BF0Rec *e;
    Unk800F68E0Rec *s;
    /* temp: the 0/1 weapon-set selector in the per-player loop, then the
     * replay window of the event loop (Ruling 11, ordinary-c-judge-decidable)
     */
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
    D_800A3778 = (s32)func_800472B0();
    /* The frame record's address is written out at each argument (F3
     * compound-address duplication, no-new-park-categories); a `rec` local:
     * score 20. */
    func_8001BAE4(
        &D_800A36EC[idx][D_800A3748],
        D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
    func_8001BBD8(
        &D_800A36EC[idx][D_800A3748],
        D_800A3748 == 0 ? &D_800A36EC[idx][1] : &D_800A36EC[idx][0], prog);
    func_8001E6E4(prog);

    for (i = 0; i < 2; i++) {
        /* entry: the frame's entry in the practice weapon table (if arm) or
         * the character's weapon table (else arm) (Ruling 11,
         * ordinary-c-judge-decidable) */
        u16 *entry;

        p = &D_800A36EC[idx][i];
        rob = &D_80101EC8[i];
        func_800198D0((p->hE >> 14) & 3, p->hE & 0x3FFF, &work[0], sp1C0);
        func_800198D0((p->h10 >> 14) & 3, p->h10 & 0x3FFF, &work[1], sp1C0);
        func_8001F1C4(rob, p, &work[0], &work[1]);
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
            func_80049718(rob->unk_12, i * 2 + 0x8000, 0, 0);
        }
        if (p->b18 & 4) {
            func_80049718(D_8008EB80[rob->unk_14], i * 2 + 0x8001, 0, 0);
        }
        if (p->b18 & 2) {
            func_80049A2C(rob->unk_12, i * 2, (p->b18 >> 4) & 1);
        }
        if (p->b18 & 8) {
            func_80049A2C(
                D_8008EB80[rob->unk_14], i * 2 | 1, (p->b18 >> 5) & 1);
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
        rob->unk_40 = p->b19;
        save58 = rob->unk_58;
        if (p->b17 & 1) {
            entry = &D_80102760.unk_04[p->w0->unk_04 * 2];
            rob->unk_58 = D_80102760.unk_08 + entry[1];
        } else {
            temp = (p->b17 >> 1) & 1;
            entry = &D_801027B0[temp].unk_04[p->w0->unk_04 * 2];
            rob->unk_58 = D_801027B0[temp].unk_08 + entry[1];
        }
        if (p->b18 & 0x40) {
            func_8003339C(rob);
        }
        rob->unk_40 = save40;
        rob->unk_58 = save58;
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
        /* FAKE: named intermediate keeps the counter + 1 computed first
         * (fold reassociates it unnamed: score 2) (Ruling 1) */
        s32 next = D_800A37D0 + 1;
        temp = D_800A36F8 - next;
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
    func_800335D8();
    D_800A37D0++;
    if ((D_800A3782 != 0 ? D_800A37D0 == 0x78 : D_800A37D0 == D_800A36F8) ||
        (g_pad_state.pressed & 0x400040)) {
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

void func_8003A41C(void) { D_800A3730 = 1; }

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

void comb_Read8(void) { read(g_comb_read_fd, &g_comb_recv_buf, 8); }

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

extern s32 D_800A36D0;

typedef s32 (*FuncBufType)(void *);

/* One link-cable exchange: packs the vsync/state bits and the record's data
 * into g_comb_send_buf with a 16-bit xor check, sends/receives through
 * comb_Write8 / comb_Read8 (order set by D_800A3916), and on success folds the
 * partner's word back into the record at a0. FAKE: the tail's u16 loads
 * reuse the buf8 local; own locals: score 33. */
void func_8003A728(PadState *a0) {
    s32 buf8;
    s32 packed;
    s32 hi16;
    s32 flag;
    s32 vsync;
    s32 c0lo;
    s32 t;
    /* FAKE: constant-holder displaces the g_comb_send_buf_plus_0x4 store from
     * the slot before the branch (emits nothing); the literal: score 3 */
    s32 zero;

    if (D_800A320C != 0) {
        buf8 = a0->held;
        vsync = D_800A38A0;
        zero = 0;
        packed = (vsync << 31) | (D_800A3730 << 30) | ((D_800A3870 & 3) << 28) |
                 (a0->type[0] << 16) | (buf8 & 0xFFFF);
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
            /* FAKE: the record's held / type stores go through pointers; stored
               as members (in-struct memory) sched moves the D_800A36C2 /
               D_800A36D2 loads above them (score 22). */
            u32 *held = &a0->held;
            s16 *type = a0->type;

            D_800A38FC += math_Popcount32((u16)D_800A36C0);
            c0lo = (u16)D_800A36C0;
            if (D_800A38A0 == 0) {
                buf8 = (u16)g_comb_send_buf;
                *held = (c0lo << 16) | buf8;
                t = D_800A36C2;
                type[1] = t & 0xF;
            } else {
                buf8 = (u16)D_800A36D0;
                *held = (buf8 << 16) | c0lo;
                t = D_800A36C2;
                type[0] = t & 0xF;
                t = D_800A36D2;
                type[1] = t & 0xF;
            }
            if (D_800A38A0 == 0) {
                if (((D_800A36C0 >> 28) & 3) == 2 && D_800A3870 == 2) {
                    D_800A3870 = 0;
                }
            } else {
                if (((D_800A36C0 >> 28) & 3) == 2 &&
                    ((D_800A36D0 >> 28) & 3) == 2) {
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
    PadState pad;
    pad.held = 0;
    pad.type[1] = 4;
    pad.type[0] = 4;
    func_8003A728(&pad);
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
s32 g_comb_recv_buf;
s32 g_comb_send_buf;
s32 D_800A36C0;
s32 D_800A36D0;
Rec1C (*D_800A36EC)[2];
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
