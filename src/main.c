#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "system.h"
#include "psx.h"

/* Forward declarations */
extern void func_80089D60(s32);
extern void spu_InitEx(s32);
extern s32 func_8008AEB0(s32);
extern void saTan4GaugeInit(s16, s16);
extern s16 saTan2Main(s32, s16, s32, s32);
extern s32 coli_HitPauseKatana_2(s32, u32, s32, s32);

/* Externs for globals */
extern s16 D_800F66F8;
extern s16 g_memcard_busy;
extern s32 g_spu_busy;
extern s32 g_spu_base_addr;

extern void irq_AcknowledgeVblank(s32, s32);
extern s32 spu_TransferDirect(s32, s32);
extern volatile s32 g_spu_init_flag;
extern s32 D_800A2874;
extern s32 g_snd_reverb_flag;
extern s32 g_spu_reverb_mode;
extern u16 g_spu_xfer_addr;
extern s32 g_spu_addr_shift;
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void func_8008D050(s32 *);
extern s32 g_snd_callback;
extern s32 D_80106F28;
extern s32 func_80078998(s32);
extern void func_80088740(s32);
extern void func_80087770(s32, s32, s32, s32);
extern s16 func_80087CAC(s32, s16 *, s16 *);
extern void spu_WriteReg(s32, u32, s32);
extern s32 D_800A287C;
extern s32 D_800A2880;
extern s32 D_800A2884;
extern s32 D_800A288C;
extern s16 D_800A2890;
extern s16 D_800A2892;
extern s32 D_800A2894;
extern s32 D_800A2898;
extern s32 D_800A289C;
extern s32 D_800A28A0;
extern s16 D_800A28D2;
extern s32 D_800A2CD4;
extern s32 g_spu_reverb_mode;
extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;
extern s32 D_800A2D44;
extern s32 D_800A2870;
extern s32 D_800A28D4;
extern s32 D_800A2CF8;
extern s32 D_800A2D04;
extern s32 D_800A2D00;
extern s32 D_800A2D08;
extern s32 D_800A2D0C;
extern volatile u32 *D_800A2CEC;
extern volatile s32 D_800A2D14;
extern s32 D_800A2CDC;
extern volatile u16 D_800F7298[];
extern s32 D_800A2CFC;
extern u16 D_800A2CF4;
extern s32 D_800A2D10;
extern s32 D_800A2D18;
extern s32 D_800A2D1C;
extern s32 D_800163D8;
extern s32 D_800163E8;
extern void debug_printf(s32 *, s32 *);
extern void spu_WriteReg16(void);
extern volatile u16 D_800F7420[2];
extern volatile u16 D_800F7424[2];
extern s32 spu_TransferData(s32, s32);
extern s32 D_800A2D2C;
extern s32 D_800A2D30;
extern s32 D_800A2D34;
extern s32 *D_800A2CE0;
extern s32 *D_800A2CE4;
extern s32 *D_800A2CE8;

/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */

extern void func_8008AF9C(void *);
void func_80083BE4(s16 a0, s16 a1) {
    s32 buf[10];
    buf[0] = 3;
    *(s16 *)&buf[1] = (s16)(a0 * 129);
    *((s16 *)&buf[1] + 1) = (s16)(a1 * 129);
    func_8008AF9C(buf);
}
extern s32 D_800A26CC;
extern s32 D_800A26D0;
extern void irq_SetAlarm(s32);
extern s32 irq_EnableInterrupts(s32, s32);
extern void func_80078BA8(s32);
extern void func_80078A68(s32, s32, s32);
extern s32 D_800A26D4;
extern s32 D_800A26D8;
extern u8 D_800A26DC;
extern u8 D_800A26DD;
extern u8 D_800A26DE;
extern s32 D_80083EDC;
extern s32 D_80083F1C;
void saTan5TakeAnim2_2(s32 arg0) {
    s32 i = 0x3E8;
    s32 s1;
    s32 s0;
    s32 cc;
    u8 *dc_ptr;

    while (--i >= 0) ;

    s1 = 0xF2000002;
    dc_ptr = &D_800A26DC;
    *dc_ptr = 0;
    cc = D_800A26CC;
    D_800A26DE = 6;
    D_800A26DD = 0;
    D_800A26D8 = 0;
    s0 = 0x44E8;

    switch (cc) {
    case 0:
        D_800A26DE = 0x7F;
        return;
    case 5:
        D_800A26DE = 0;
        if (arg0 == 0) {
            *dc_ptr = 1;
        } else {
            s1 = 0xF2000003;
            s0 = 1;
        }
        break;
    case 2:
        break;
    case 3:
        s0 = 0x89D0;
        break;
    default: {
        register s32 *ptr asm("a1") = &D_800A26D0;
        s32 val;
        __asm__("" : "=r"(ptr) : "0"(ptr));
        if (*ptr != 0) return;
        val = ptr[-1];
        if (val < 0x46) {
            s32 quotient = 0x204CC0 / val;
            *(u8 *)((u8 *)ptr + 0xD) += 1;
            s0 = quotient;
        } else {
            s32 quotient = 0x409980 / val;
            __asm__("" : "=r"(quotient) : "0"(quotient));
            s0 = quotient;
        }
        break;
    }
    }

    if (D_800A26DC != 0) {
        EnterCriticalSection();
        irq_SetAlarm(D_800A26D4);
    } else {
        s32 de;
        s32 a1_val;
        EnterCriticalSection();
        func_80078BA8(s1);
        func_80078A68(s1, (u16)s0, 0x1000);
        de = D_800A26DE;
        if (de == 0) {
            s32 ret = irq_EnableInterrupts(0, 0);
            de = D_800A26DE;
            a1_val = (s32)&D_80083EDC;
            D_800A26D8 = ret;
        } else {
            a1_val = (s32)&D_80083F1C;
            if (D_800A26DD == 0) {
                a1_val = D_800A26D4;
            }
        }
        irq_EnableInterrupts(de, a1_val);
    }
    ExitCriticalSection();
}
/* kengo:MED  |  sa_tan5/saTan5TakeAnim2_2  |  154i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", DispStuff);
/* kengo:LOW  |  su_menu_ending/_DispStuff  |  209i  |  PS2 UI — reverted */
void func_800841E0(s32 arg0, s32 arg1) {
    struct {
        s16 sp10;
        s16 sp12;
        s32 pad[5];
    } stack;
    register s32 *base_ptr asm("s3");
    register s32 offset asm("s2");
    register u8 *entry asm("s0");
    register s32 packed asm("s1");
    register s32 ch asm("s5");
    register s32 slot asm("s4");
    register s32 a3_arg asm("a3");
    register u8 *clear_entry asm("v0");
    register s32 slot_se asm("v1");
    register s32 offcalc asm("v0");
    s32 timer;
    register s32 base asm("v1");

    a3_arg = arg0;
    {
        register s32 shift asm("v0") = (s32)(a3_arg << 16) >> 14;
        register s32 tbl asm("v1") = (s32)&D_80106F28;
        __asm__ volatile("addu %0, %1, %2" : "=r"(base_ptr) : "r"(shift), "r"(tbl));
    }
    slot_se = (s16)arg1;
    offcalc = (slot_se << 1) + slot_se;
    offcalc = (offcalc << 2) - slot_se;
    offset = offcalc << 4;
    base = *base_ptr;
    entry = (u8 *)(base + offset);

    ch = a3_arg;
    timer = *(s32 *)(entry + 0xA0);
    slot = arg1;
    timer = timer - 1;
    *(s32 *)(entry + 0xA0) = timer;

    if (timer < 0) {
        clear_entry = (u8 *)(*base_ptr + offset);
        goto clear_flag_10;
    }

    {
    register s32 step asm("a2");
    register s32 step_copy asm("v1");
    step = *(s16 *)(entry + 0x4C);
    step_copy = step;
    if (step_copy > 0) {
        if ((timer % step) != 0) {
            goto update_position;
        }
        *(u16 *)(entry + 0x4A) = *(u16 *)(entry + 0x4A) - 1;
        if (*(s16 *)(entry + 0x4A) < 0) {
            goto send_center_and_clear;
        }

        packed = (s16)(ch | (slot << 8));
        func_80087CAC(packed, &stack.sp10, &stack.sp12);
        if (((u16)stack.sp10 + *(s16 *)(entry + 0x4A)) < ((u16)stack.sp10 + 1)) {
            goto post_send_check;
        }
        func_80087770(packed, (u16)(stack.sp10 + 1), (u16)(stack.sp12 + 1), 1);
        goto post_send_check;
    }

    if (step_copy < 0) {
        *(u16 *)(entry + 0x4A) = *(u16 *)(entry + 0x4A) + step;
        if (*(s16 *)(entry + 0x4A) < 0) {
            goto send_center_and_clear;
        }

        packed = (s16)(ch | (slot << 8));
        func_80087CAC(packed, &stack.sp10, &stack.sp12);

        if ((((u16)stack.sp10 - step) >= 0x7F) && (((u16)stack.sp12 - step) >= 0x7F)) {
            func_80087770(packed, 0x7F, 0x7F, 1);
            clear_entry = (u8 *)(*base_ptr + offset);
            *(s32 *)(clear_entry + 0x98) &= ~0x10;
        }

        step = *(s16 *)(entry + 0x4C);
        if (((*(s32 *)(entry + 0x9C) - *(s32 *)(entry + 0xA0)) * -step) < *(s16 *)(entry + 0x48)) {
            packed = (s16)(ch | (slot << 8));
            func_80087770(packed, (u16)(stack.sp10 - step), (u16)(stack.sp12 - step), 1);
        }
        goto post_send_check;
    }
    }
    goto update_position;

send_center_and_clear:
    func_80087770((s16)(ch | (slot << 8)), 0x7F, 0x7F, 1);
    clear_entry = (u8 *)(*base_ptr + offset);
    *(s32 *)(clear_entry + 0x98) &= ~0x10;

post_send_check:
    if (*(s32 *)(entry + 0xA0) == 0) {
        goto recompute_clear_flag_10;
    }
    if (*(s16 *)(entry + 0x4A) > 0) {
        goto update_position;
    }

recompute_clear_flag_10:
    {
        s32 clear_shift = (s32)(ch << 16) >> 14;
        s32 clear_offset = (s16)slot * 0xB0;
        clear_entry = (u8 *)(*(s32 *)((u8 *)&D_80106F28 + clear_shift) + clear_offset);

    }
clear_flag_10:
    *(s32 *)(clear_entry + 0x98) &= ~0x10;

update_position:
    func_80087CAC((s16)(ch | (slot << 8)), (s16 *)(entry + 0x5C), (s16 *)(entry + 0x5E));
    return;
}
void func_80084500(s32 arg0, s32 arg1) {
    struct {
        s16 sp10;
        s16 sp12;
        s32 pad[5];
    } stack;
    register s32 *base_ptr asm("s3");
    register s32 offset asm("s2");
    register u8 *entry asm("s0");
    register s32 packed asm("s1");
    register s32 ch asm("s5");
    register s32 slot asm("s4");
    register s32 a3_arg asm("a3");
    register u8 *clear_entry asm("v0");
    register s32 slot_se asm("v1");
    register s32 offcalc asm("v0");
    s32 timer;
    register s32 base asm("v1");

    a3_arg = arg0;
    {
        register s32 shift asm("v0") = (s32)(a3_arg << 16) >> 14;
        register s32 tbl asm("v1") = (s32)&D_80106F28;
        __asm__ volatile("addu %0, %1, %2" : "=r"(base_ptr) : "r"(shift), "r"(tbl));
    }
    slot_se = (s16)arg1;
    offcalc = (slot_se << 1) + slot_se;
    offcalc = (offcalc << 2) - slot_se;
    offset = offcalc << 4;
    base = *base_ptr;
    entry = (u8 *)(base + offset);

    ch = a3_arg;
    timer = *(s32 *)(entry + 0xA0);
    slot = arg1;
    timer = timer - 1;
    *(s32 *)(entry + 0xA0) = timer;

    if (timer < 0) {
        clear_entry = (u8 *)(*base_ptr + offset);
        goto clear_flag_20;
    }

    {
    register s32 step asm("a2");
    register s32 step_copy asm("v1");
    step = *(s16 *)(entry + 0x4C);
    step_copy = step;
    if (step_copy > 0) {
        if ((timer % step) != 0) {
            goto update_position;
        }
        *(u16 *)(entry + 0x4A) = *(u16 *)(entry + 0x4A) - 1;
        if (*(s16 *)(entry + 0x4A) < 0) {
            goto send_zero_and_clear;
        }

        packed = (s16)(ch | (slot << 8));
        func_80087CAC(packed, &stack.sp10, &stack.sp12);
        if ((((u16)stack.sp10 - 1) < ((u16)stack.sp10 - *(s16 *)(entry + 0x4A))) &&
            (((u16)stack.sp12 - 1) < ((u16)stack.sp12 - *(s16 *)(entry + 0x4A)))) {
            goto post_send_check;
        }
            if ((u16)stack.sp10 == 0) {
                goto recompute_post_send_clear_flag_20;
            }
            if ((u16)stack.sp12 != 0) {
                goto send_step_down;
            }
            goto recompute_post_send_clear_flag_20;
        send_step_down:
            func_80087770((s16)(ch | (slot << 8)), (u16)(stack.sp10 - 1), (u16)(stack.sp12 - 1), 1);
            goto post_send_check;
    }

    if (step_copy < 0) {
        *(u16 *)(entry + 0x4A) = *(u16 *)(entry + 0x4A) + step;
        if (*(s16 *)(entry + 0x4A) < 0) {
            goto send_zero_and_clear;
        }

        packed = (s16)(ch | (slot << 8));
        func_80087CAC(packed, &stack.sp10, &stack.sp12);

        if ((((u16)stack.sp10 + *(s16 *)(entry + 0x4C)) <= 0) &&
            (((u16)stack.sp12 + *(s16 *)(entry + 0x4C)) <= 0)) {
            func_80087770(packed, 0, 0, 1);
            clear_entry = (u8 *)(*base_ptr + offset);
            *(s32 *)(clear_entry + 0x98) &= ~0x20;
        }

        step = *(s16 *)(entry + 0x4C);
            if (((*(s32 *)(entry + 0x9C) - *(s32 *)(entry + 0xA0)) * -step) < *(s16 *)(entry + 0x48)) {
                if ((u16)stack.sp10 == 0) {
                    goto recompute_post_send_clear_flag_20;
                }
                if ((u16)stack.sp12 != 0) {
                    goto send_step_up;
                }
                goto recompute_post_send_clear_flag_20;
            send_step_up:
                func_80087770((s16)(ch | (slot << 8)), (u16)(stack.sp10 + step), (u16)(stack.sp12 + step), 1);
            }
            goto post_send_check;
        }
    }
    goto update_position;

recompute_post_send_clear_flag_20:
    {
        s32 clear_shift = (s32)(ch << 16) >> 14;
        s32 clear_offset = (s16)slot * 0xB0;
        clear_entry = (u8 *)(*(s32 *)((u8 *)&D_80106F28 + clear_shift) + clear_offset);
    }
    *(s32 *)(clear_entry + 0x98) &= ~0x20;
    goto post_send_check;

send_zero_and_clear:
    func_80087770((s16)(ch | (slot << 8)), 0, 0, 1);
    clear_entry = (u8 *)(*base_ptr + offset);
    *(s32 *)(clear_entry + 0x98) &= ~0x20;

post_send_check:
    if (*(s32 *)(entry + 0xA0) == 0) {
        goto recompute_clear_flag_20;
    }
    if (*(s16 *)(entry + 0x4A) > 0) {
        goto update_position;
    }

  recompute_clear_flag_20:
      {
          s32 clear_shift = (s32)(ch << 16) >> 14;
          s32 clear_offset = (s16)slot * 0xB0;
          clear_entry = (u8 *)(*(s32 *)((u8 *)&D_80106F28 + clear_shift) + clear_offset);
    }
clear_flag_20:
    *(s32 *)(clear_entry + 0x98) &= ~0x20;

update_position:
    func_80087CAC((s16)(ch | (slot << 8)), (s16 *)(entry + 0x5C), (s16 *)(entry + 0x5E));
    return;
}
void spu_SetMotionState(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    u8 *entry;
    entry = (u8 *)(*base_ptr + offset);
    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    entry[0x14] = 0;
    offset += *base_ptr;
    *(s32 *)(offset + 0x98) &= ~2;
}

void spu_SetMotionCallback(s16 a0, s16 a1) {
    saTan4GaugeInit(a0, a1);
}

void saTan4GaugeInit(s16 a0, s16 a1) {
    u8 *base;
    s32 gauge;
    s32 diff;
    s32 accum;
    s32 result;

    base = (u8 *)(*(s32 *)((u8 *)&D_80106F28 + ((s32)(a0 << 16) >> 14)) + (s16)a1 * 0xB0);
    gauge = *(s32 *)(base + 0x90);
    diff = gauge - *(s16 *)(base + 0x54);
    if (diff > 0) {
        s16 timer = *(s16 *)(base + 0x52);
        if (timer > 0) {
            *(s16 *)(base + 0x52) = timer - 1;
            return;
        }
        if (timer == 0) {
            *(s16 *)(base + 0x52) = *(s16 *)(base + 0x54);
            result = *(s32 *)(base + 0x90) - 1;
            goto store_result;
        }
        *(s32 *)(base + 0x90) = diff;
        return;
    }
    if (*(s16 *)(base + 0x54) < gauge) {
        return;
    }
    accum = gauge;
    {
        s32 sa0 = a0 << 16;
        s32 sa1 = a1 << 16;
    loop:
        saTan0Main(sa0 >> 16, sa1 >> 16);
        gauge = *(s32 *)(base + 0x90);
        if (gauge == 0) {
            goto loop;
        }
        accum += gauge;
        if (accum < *(s16 *)(base + 0x54)) {
            goto loop;
        }
        result = accum - *(s16 *)(base + 0x54);
    }
store_result:
    *(s32 *)(base + 0x90) = result;
}
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *base_ptr = (s32 *)((u8 *)&D_80106F28 + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    u8 *base = (u8 *)(*base_ptr + offset);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + offset + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(*base_ptr + offset + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(*base_ptr + offset + 0x98) &= ~1;
    *(s32 *)(*base_ptr + offset + 0x98) &= ~8;
    *(s32 *)(*base_ptr + offset + 0x98) &= ~2;
    *(s32 *)(*base_ptr + offset + 0x98) |= 0x200;
    *(s32 *)(*base_ptr + offset + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(*base_ptr + offset + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
        base[0x14] = 0;
        spu_ResetMotionEntry(base[0x22], base[0x23]);
        spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    }
    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    *(s32 *)(base + 0x90) = *(s16 *)(base + 0x54);
}
extern void (*D_800F3340)(s16, s16, u8, u8);
extern void (*D_800F3344)(s16, s16, u8, u8 *);
extern void (*D_800F3348)(s16, s16, u8, u8 *);
extern void (*D_800F334C)(s16, s16, u8, u8 *);
extern void (*D_800F3350)(s16, s16, u8, u8 *);

s32 saTan0Main(s16 a0, s16 a1) {
    u8 *state;
    unsigned char new_var2;
    register s32 cmd asm("a2");
    u8 *cmd_ptr;
    u8 *ptr;
    register u8 b asm("s2");
    u8 prev;
    u8 **new_var;
    u8 *new_var3;
    register u8 next asm("s4");
    register s32 ret asm("s5");

    state = (u8 *)(((s32)((void **)&D_80106F28)[a0]) + (a1 * 0xB0));
    ptr = *(u8 **)state;
    *(u8 **)((u8 *)(((s32)((void **)&D_80106F28)[a0]) + (a1 * 0xB0))) = ptr + 1;
    b = ptr[0];
    ret = 0;
    if (((*(s32 *)((u8 *)(((s32)((void **)&D_80106F28)[a0]) + (a1 * 0xB0)) + 0x98)) & 0x401) == 0x401) {
        if ((s32)(ptr + 1) == (*(s32 *)(state + 0x10) + 1)) {
            ((void (*)(s16, s16, u8, u8 *))func_80084A7C)(a0, a1, ((u8 *)(*(s32 *)(state + 0x10)))[1], ptr);
            ptr = *(u8 **)state;
            return -1;
        }
    }
    if (b & 0x80) {
        state[0x17] = b & 0xF;
        cmd = b & 0xF0;
        switch (cmd) {
        case 0x90:
            state[0x16] = 0x90;
            cmd_ptr = *(u8 **)state;
            *(u8 **)state = cmd_ptr + 1;
            prev = cmd_ptr[0];
            *(u8 **)state = cmd_ptr + 2;
            next = cmd_ptr[1];
            new_var3 = state + 0x90;
            *(s32 *)new_var3 = spu_ReadMotionFrame(a0, a1);
            D_800F3340(a0, a1, prev, next);
            return 0;

        case 0xB0:
            state[0x16] = 0xB0;
            cmd_ptr = *(u8 **)state;
            *(u8 **)state = cmd_ptr + 1;
            next = cmd_ptr[0];
            D_800F3350(a0, a1, next, cmd_ptr);
            new_var2 = !a0;
            if ((new_var2 && new_var2) && new_var2) {
            }
            return 0;

        case 0xC0:
            state[0x16] = 0xC0;
            cmd_ptr = *(u8 **)state;
            *(u8 **)state = cmd_ptr + 1;
            next = cmd_ptr[0];
            D_800F3344(a0, a1, next, cmd_ptr);
            return 0;

        case 0xE0:
            state[0x16] = 0xE0;
            *(u8 **)state = (*(u8 **)state) + 1;
            D_800F3348(a0, a1, cmd, ptr);
            return 0;

        case 0xF0:
            state[0x16] = 0xFF;
            cmd_ptr = ptr;
            *(u8 **)state = cmd_ptr + 1;
            next = cmd_ptr[0];
            if (next == 0x2F) {
                ret = 1;
                ((void (*)(s16, s16, u8, u8 *))func_80084A7C)(a0, a1, 0x2F, cmd_ptr);
                return ret;
            }
            D_800F334C(a0, a1, next, cmd_ptr);
            return 0;

        default:
            return 0;
            if ((!a1) && (!a1)) {
            }
            return 0;
        }
    } else {
        prev = state[0x16];
        switch (prev) {
        case 0x90:
            cmd_ptr = *(u8 **)state;
            *(u8 **)state = cmd_ptr + 1;
            next = cmd_ptr[0];
            *(s32 *)new_var3 = spu_ReadMotionFrame(a0, a1);
            D_800F3340(a0, a1, b, next);
            return 0;
            if (!state) {
            }

        case 0xB0:
            D_800F3350(a0, a1, b, ptr);
            return 0;

        case 0xC0:
            D_800F3344(a0, a1, b, ptr);
            return 0;

        case 0xE0:
            D_800F3348(a0, a1, b & 0xFF, ptr);
            return 0;

        case 0xFF:
            if ((b & 0xFF) == 0x2F) {
                ret = 1;
                ((void (*)(s16, s16, u8, u8 *))func_80084A7C)(a0, a1, 0x2F, ptr);
                return ret;
            }
            D_800F334C(a0, a1, b & 0xFF, ptr);

        default:
            return 0;
        }
    }
}
/* kengo:MED  |  sa_tan0/saTan0Main  |  233i */
s32 spu_ReadMotionFrame(s32 arg0, s16 arg1) {
    s32 result;
    u8 *ptr;
    u8 **base;
    s32 val;
    s32 byte;

    base = (u8 **)((*(s32 *)((u8 *)&D_80106F28 + ((s32)(arg0 << 16) >> 14))) + (arg1 * 0xB0));
    ptr = *base;
    *base = ptr + 1;
    val = *ptr;
    if (val == 0) {
        return 0;
    }
    result = val << 2;
    if (val & 0x80) {
        val &= 0x7F;
        do {
            ptr = *base;
            *base = ptr + 1;
            byte = *ptr;
            val = (val << 7) + (byte & 0x7F);
        } while (byte & 0x80);
    }
    result = val << 2;
    result = (result + val) << 1;
    *(s32 *)((u8 *)base + 0x88) += result;
    return result;
}
void spu_ResetMotionEntry(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = (s16)a1 * 0xB0;
    s32 base = *base_ptr;
    u8 *entry = (u8 *)(base + (s16)a1 * 0xB0);
    entry[0x20] = 1;
    entry[0x21] = 0;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x100;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~2;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~4;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~0x200;
    *(s32 *)entry = *(s32 *)(entry + 4);
    entry[0x14] = 1;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) |= 1;
}
void spu_SetMotionActive(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 new_var;
    s32 offset = a1 * 0xB0;
    s32 base = *base_ptr;
    if (1) {
        *(u8 *)((base + offset) + 0x14) = 1;
        base = (new_var = *base_ptr);
        *(s32 *)((offset + base) + 0x98) &= ~8;
    }
}
void func_80085270(s16 a0, s16 a1) {
    s32 *base_ptr = (s32*)((u8*)&D_80106F28 + ((s32)(a0 << 16) >> 14));
    u8 *p = (u8*)(*base_ptr + (s16)a1 * 0xB0);
    s32 i;
    s16 *hp;
    u8 *ip;

    *(s32*)(p + 0x98) &= ~1;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~2;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~8;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) &= ~0x400;
    *(s32*)((*base_ptr + (s16)a1 * 0xB0) + 0x98) |= 4;

    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    spu_ResetCounter();

    i = 0;
    p[0x14] = 0;
    *(s32*)(p + 0x88) = 0;
    p[0x1C] = 0;
    p[0x18] = 0;
    p[0x19] = 0;
    p[0x1E] = 0;
    p[0x1A] = 0;
    p[0x1B] = 0;
    p[0x1F] = 0;
    p[0x17] = 0;
    p[0x21] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x15] = 0;
    p[0x16] = 0;

    *(s32*)(p + 0x90) = *(s32*)(p + 0x84);
    *(s32*)(p + 0x94) = *(s32*)(p + 0x8C);
    *(s16*)(p + 0x54) = *(u16*)(p + 0x56);
    *(s32*)(p + 0x0) = *(s32*)(p + 0x4);
    *(s32*)(p + 0x8) = *(s32*)(p + 0x4);

    hp = (s16*)p;
    do {
        ip = p + i;
        ip[0x37] = (u8)i;
        ip[0x27] = 0x40;
        *(s16*)((u8*)hp + 0x60) = 0x7F;
        hp++;
        i++;
    } while (i < 0x10);
    *(s16*)(p + 0x5C) = 0x7F;
    *(s16*)(p + 0x5E) = 0x7F;
}

void func_800853F4(s16 a0) {
    func_80085270(a0, 0);
}

void func_8008541C(s16 a0, s16 a1) {
    func_80085270(a0, a1);
}
void func_80085448(s16 a0, s16 a1, s16 a2) {
    s32 buf[10];
    if ((u8)a0 == 0) {
        buf[0] = 0xC0;
        if ((s16)a1 >= 0x80) a1 = 0x7F;
        if ((s16)a2 >= 0x80) a2 = 0x7F;
        *(s16 *)((u8 *)buf + 0x10) = (s16)a1 * 129 * 2;
        *(s16 *)((u8 *)buf + 0x12) = (s16)a2 * 129 * 2;
    }
    if ((u8)a0 == 1) {
        buf[0] = 0xC00;
        if ((s16)a1 >= 0x80) a1 = 0x7F;
        if ((s16)a2 >= 0x80) a2 = 0x7F;
        *(s16 *)((u8 *)buf + 0x1C) = (s16)a1 * 129 * 2;
        *(s16 *)((u8 *)buf + 0x1E) = (s16)a2 * 129 * 2;
    }
    func_8008AF9C(buf);
}
extern s32 D_800A26CC;
extern s32 D_800A26D0;
extern s32 D_80104E80;
void SetBloodSpot(s32 arg) {
    s32 mode;
    s32 v;

    mode = sys_GetVideoMode();

    if (arg & 0x1000) {
        D_800A26D0 = 1;
        D_800A26CC = arg & 0xFFF;
    } else {
        D_800A26D0 = 0;
        D_800A26CC = arg;
    }

    v = D_800A26CC;
    if (v >= 6) goto big_v;
    switch (v) {
    case 4: {
        s32 t = 50;
        D_80104E80 = t;
        if (mode == 1) D_800A26CC = 5;
        else D_800A26CC = t;
        break;
    }
    case 1: {
        s32 t = 60;
        D_80104E80 = t;
        if (mode == 0) D_800A26CC = 5;
        else D_800A26CC = t;
        break;
    }
    case 3:
        D_80104E80 = 120;
        break;
    case 2:
        D_80104E80 = 240;
        break;
    case 5:
        if (mode == 0) D_80104E80 = 60;
        else if (mode == 1) D_80104E80 = 50;
        else D_80104E80 = 60;
        break;
    case 0:
        if (mode == 0) D_80104E80 = 60;
        else if (mode == 1) D_80104E80 = 50;
        else D_80104E80 = 60;
        break;
    default:
        D_80104E80 = 60;
        break;
    }
    return;
big_v:
    D_80104E80 = v;
}
/* kengo:MED  |  am_rmd/SetBloodSpot  |  91i */
void func_800856B0(s16 a0, s16 a1) {
    s32 *tbl_ptr;
    u8 *p;
    s32 countdown;
    s16 duration;
    u32 current;
    u32 target;
    u32 new_val;
    s32 product;
    s32 numer;
    s32 denom;
    s16 display;
    s32 *tbl2;
    u8 *q;

    tbl_ptr = (s32 *)((u8 *)&D_80106F28 + ((s32)(a0 << 16) >> 14));
    p = (u8 *)(*tbl_ptr + (s16)a1 * 0xB0);

    countdown = *(s32 *)(p + 0xA8) - 1;
    *(s32 *)(p + 0xA8) = countdown;

    if (countdown < 0) {
        q = (u8 *)(*tbl_ptr + (s16)a1 * 0xB0);
        *(s32 *)(q + 0x98) &= ~0x40;
        q = (u8 *)(*tbl_ptr + (s16)a1 * 0xB0);
        goto clear_80;
    }

    duration = *(s16 *)(p + 0x4E);
    if (duration > 0) {
        if (countdown % duration != 0) {
            return;
        }
        current = *(u32 *)(p + 0x94);
        target = *(u32 *)(p + 0xAC);
        if (target < current) {
            new_val = current - 1;
            goto pos_store;
        }
        if (current < target) {
            new_val = current + 1;
        pos_store:
            *(u32 *)(p + 0x94) = new_val;
        }
    } else {
        current = *(u32 *)(p + 0x94);
        target = *(u32 *)(p + 0xAC);
        if (target < current) {
            new_val = current + duration;
            *(u32 *)(p + 0x94) = new_val;
            if (new_val < target) {
                *(u32 *)(p + 0x94) = target;
            }
        } else if (current < target) {
            new_val = current - duration;
            target = *(u32 *)(p + 0xAC);
            *(u32 *)(p + 0x94) = new_val;
            if (target < new_val) {
                *(u32 *)(p + 0x94) = target;
            }
        }
    }

    product = *(s16 *)(p + 0x50) * *(s32 *)(p + 0x94);
    numer = product * 10;
    denom = D_80104E80 * 60;
    display = (u32)numer / (u32)denom;
    *(s16 *)(p + 0x54) = display;
    if ((s16)display <= 0) {
        *(s16 *)(p + 0x54) = 1;
    }

    if (*(s32 *)(p + 0xA8) == 0) {
        goto recompute;
    }
    if (*(s32 *)(p + 0x94) == *(s32 *)(p + 0xAC)) {
        goto recompute;
    }
    return;

recompute:
    tbl2 = (s32 *)((u8 *)&D_80106F28 + ((s32)(a0 << 16) >> 14));
    q = (u8 *)(*tbl2 + (s16)a1 * 0xB0);
    *(s32 *)(q + 0x98) &= ~0x40;
    q = (u8 *)(*tbl2 + (s16)a1 * 0xB0);
clear_80:
    *(s32 *)(q + 0x98) &= ~0x80;
}
extern u8 D_80101BCC;
extern s16 D_800F4E1A;
extern s16 D_800F4E1E;
extern s16 D_800F4E28[];
extern s16 D_800F4E2A;
extern s16 D_800F4E2C;
extern s16 D_800F4E2E;
extern u16 D_8010280A;

void title_mv_exec2(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;

    var_s0 = 0;
    buf[1] = 0x60093;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            offset = (s16)var_s0 * 54;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = 0x18;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            buf[0] = 1 << (s16)var_s0;
            saTan1MainJump(buf);
            D_8010280A = var_s0;
            motutil_GetAngTableNum(1);
            var_s0 = var_s0 + 1;
        } while ((s16)var_s0 < (s32)D_80101BCC);
    }
}
extern u8 g_snd_ch_status[];
extern s32 g_snd_ch_addr[];
s32 func_800859F0(s16 a0) {
    if ((u16)a0 >= 0x11) {
        return -1;
    }
    if (g_snd_ch_status[a0] != 1) {
        return -1;
    }
    return g_snd_ch_addr[a0];
}
INCLUDE_ASM("asm/funcs", AllocBukiRmd);
/* kengo:MED  |  am_rmd/AllocBukiRmd  |  259i */
extern s32 D_800F5750;
extern s16 D_800F5758;
extern s16 D_800F575A;
extern void func_80089F3C(s32 *);
void func_80085E4C(s16 a0, s16 a1) {
    s32 x = (s16)a0 * 32767 / 127;
    s32 y = (s16)a1 * 32767 / 127;
    s32 *buf = &D_800F5750;
    *buf = 6;
    D_800F5758 = x;
    D_800F575A = y;
    func_80089F3C(buf);
}
extern s32 D_800F5750;
extern s16 D_800F5758;
extern s16 D_800F575A;
extern s32 D_800F5754;
s16 func_80085EE4(s16 a0) {
    s32 neg = 0;
    s16 v1 = a0;
    s32 s0;
    if ((s32)(a0 << 16) < 0) {
        neg = 1;
        v1 = -a0;
    }
    if ((u16)v1 < 0xA) {
        D_800F5750 = 1;
        if (neg) {
            D_800F5754 = (s16)((v1 | 0x100) << 16 >> 16);
        } else {
            D_800F5754 = (s16)(v1 << 16 >> 16);
        }
        s0 = (s16)(v1 << 16 >> 16);
        if (s0 == 0) {
            func_80089D60(0);
        }
        func_80089F3C(&D_800F5750);
        return s0;
    }
    return -1;
}
s16 spu_GetReverbMode(void) {
    return *(s16 *)&D_800F5754;
}

void func_80085F98(void) {
    func_80089D60(0);
}

extern s16 D_80102A78[];
extern s16 D_80102A7A[];
extern u8 D_800F65E0[];

void func_80085FB8(void) {
    func_80089D60(1);
}

s32 func_80085FD8(s16 a0) {
    if ((u16)a0 < 0x18) {
        func_8008BD88((s16)(a0 << 16 >> 16));
        return 0;
    }
    return -1;
}

s32 func_80086014(s32 a0, s32 a1, s32 a2)
{
  register s32 v0 asm("$2");
  register s32 v1 asm("$3");
  register s32 ra0 asm("$4");
  register s32 ra1 asm("$5");
  register s32 ra3 asm("$7");

  if (((u32)(a0 & 0xFFFF)) < 0x18) {
    ra3 = a1;
    v1 = (a0 << 16) >> 16;
    ra1 = v1 << 4;
    *((s16 *)((u8 *)&D_80102A7A + ra1)) = (s16)a2;
    asm volatile("" ::: "memory");
    ra0 = *((u8 *)&D_800F65E0 + v1);
    v0 = 0;
    *((s16 *)((u8 *)&D_80102A78 + ra1)) = ra3;
    ra0 |= 3;
    *((u8 *)&D_800F65E0 + v1) = ra0;
    return 0;
  }
  return -1;
}

s32 func_80086080(s16 a0, s16 *a1, s16 *a2) {
    u16 raw1, raw2;

    if ((u16)a0 < 0x18) {
        func_8008BD88((s16)(a0 << 16 >> 16), &raw1, &raw2);
        *a1 = (s16)raw1 / 129;
        *a2 = (s16)raw2 / 129;
        return 0;
    }
    return -1;
}
extern s16 D_80102A78[];
extern s16 D_80102A7A[];
extern u8 D_800F65E0[];

extern s16 D_80102A78[];
extern s16 D_80102A7A[];
extern u8 D_800F65E0[];

s32 func_80086130(s32 a0, s32 a1, s32 a2)
{
  register s32 v0 asm("$2");
  register s32 v1 asm("$3");
  register volatile int ra2 asm("$6");
  register s32 ra1 asm("$5");
  register s32 ra0 asm("$4");

  if (((u32)(a0 & 0xFFFF)) < 0x18) {
    v1 = a2;
    v0 = (a1 << 16) >> 16;
    ra2 = v0 << 7;
    ra2 += v0;
    v0 = (v1 << 16) >> 16;
    v1 = (v0 << 7) + v0;
    ra0 = (a0 << 16) >> 16;
    ra1 = ra0 << 4;
    *((s16 *)((u8 *)&D_80102A7A + ra1)) = v1;
    v1 = *((u8 *)&D_800F65E0 + ra0);
    v0 = 0;
    *((s16 *)((u8 *)&D_80102A78 + ra1)) = ra2;
    v1 |= 3;
    *((u8 *)&D_800F65E0 + ra0) = v1;
    return 0;
  }
  return -1;
}
extern s32 D_800FF6A0;
extern s32 D_80101BC8;
extern u8 D_801027F7;
extern u8 D_801027FC;
extern u16 D_80102808;
extern s16 D_8010280C;
extern s16 D_8010280E;
extern s16 D_80102A7E;
extern s16 D_80102A80;
extern s16 D_80102A82;
extern s32 D_80107898[];
void func_800861BC(void)
{
  s32 new_var;
  s32 i;
  s32 *p;
  s16 v;
  s16 idx;
  s16 idx2;
  s32 ofs;
  u8 *base;
  s16 *pc;
  pc = &D_8010280C;
  p = &D_80107898[0];
  idx = D_8010280A;
  *pc = idx * 8;
  D_8010280E = (s16) (D_801027FC + (D_801027F7 << 4));
  *((s16 *) (((u8 *) (&D_800F4E1E)) + ((s32) (idx * 54)))) = 0x7FFF;
  i = 0;
  do
  {
    i += 1;
    *p &= ~(1 << (*((s16 *) (((u8 *) pc) - 2))));
    p += 1;
  }
  while (i < 16);
  if (D_80102808 & 1)
  {
    v = (s16) D_80102808;
    ofs = (((s32) (v - 1)) / 2) << 4;
    *((s16 *) (((u8 *) (&D_80102A7E)) + (D_8010280C * 2))) = *((u16 *) ((((u8 *) D_800FF6A0) + ofs) + 0xC));
  }
  else
  {
    v = (s16) D_80102808;
    ofs = (((s32) (v - 1)) / 2) << 4;
    *((s16 *) (((u8 *) (&D_80102A7E)) + (D_8010280C * 2))) = *((u16 *) ((((u8 *) D_800FF6A0) + ofs) + 0xE));
  }
  idx2 = D_8010280A;
  *(((u8 *) (&D_800F65E0)) + idx2) |= 8;
  new_var = D_80101BC8;
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A80)) + (*pc * 2))) = *((u16 *) (base + 0x10));
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A82)) + (*pc * 2))) = (*((u16 *) (base + 0x12))) + D_800F66F8;
  *(((u8 *) (&D_800F65E0)) + D_8010280A) |= 0x30;
}

void spu_ResetCounter(void) {
    D_800F66F8 = 0;
}

INCLUDE_ASM("asm/funcs", action_CheckHitZangeki);
/* kengo:HIGH  |  is_action/action_CheckHitZangeki  |  271i */
INCLUDE_ASM("asm/funcs", md_game_end);
/* kengo:HIGH  |  md_game/md_game_end  |  249i */
extern u8 D_801027F7;
extern u8 D_801027FC;
extern s32 D_80101BC8;
extern u16 D_800A26E4[];

u16 func_80086BFC(s32 a0, s16 a1) {
    u8 *entry;
    s32 val, div8, col, overflow, total, row, rem, shift;
    u16 result;

    {
        u8 idx1 = D_801027F7;
        u8 idx2 = D_801027FC;
        s32 combined = idx2 + (idx1 << 4);
        entry = (u8 *)((combined << 5) + D_80101BC8);
    }

    val = (u16)a1 + entry[5];
    if (val < 0) {
        val += 7;
    }

    div8 = val >> 3;
    col = div8;
    overflow = 0;
    if (div8 >= 0x10) {
        overflow = 1;
        col = div8 - 0x10;
    }

    {
        s32 e4 = entry[4];
        s32 base_val = a0 + 0x3C;
        s32 diff = base_val - e4;
        total = (s16)(overflow + diff);
    }
    row = total / 12;
    rem = total - row * 12;

    {
        s32 index = ((s16)rem << 4) + (s16)col;
        shift = (s16)(row - 5);
        result = D_800A26E4[index];
    }

    if (shift > 0) {
        result <<= shift;
    } else if (shift < 0) {
        result = (u16)result >> (-shift);
    }
    return result;
}
INCLUDE_ASM("asm/funcs", func_80086CF8);
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern s16 D_800F4E18;
extern s16 D_800F4E1C;
extern s8 D_800F4E35;
extern u16 D_8010280A;
extern u16 D_801078D8;
extern u16 D_801078DA;
void func_800871D4(s32 a0_arg)
{
  u32 temp_a0;
  u32 var_v1;
  s32 var_a2;
  s32 var_a1;
  s32 temp_v0;
  u16 temp_v1_d8;
  u16 temp_a0_da;
  u16 temp_v0_1b10;
  u16 temp_v0_1b12;

  temp_a0 = D_8010280A;
  __asm__("andi %0, %1, 0xffff" : "=r"(var_v1) : "r"(temp_a0));
  if (var_v1 < 0x10U)
  {
    var_a2 = 1 << var_v1;
    var_a1 = 0;
  }
  else
  {
    var_a2 = 0;
    var_a1 = 1 << (var_v1 - 0x10);
    __asm__("andi %0, %1, 0xffff" : "=r"(var_v1) : "r"(temp_a0));
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  *((s8 *)((u8 *)&D_800F4E35 + temp_v0)) = 0;
  temp_v1_d8 = D_801078D8;
  temp_a0_da = D_801078DA;
  *((s16 *)((u8 *)&D_800F4E1C + temp_v0)) = 0;
  *((s16 *)((u8 *)&D_800F4E18 + temp_v0)) = 0;
  temp_v0_1b10 = D_800F1B10;
  __asm__("" : : "r"(var_a1));
  temp_v1_d8 = temp_v1_d8 | var_a2;
  D_801078D8 = temp_v1_d8;
  D_800F1B10 = temp_v0_1b10 & ~temp_v1_d8;
  temp_v0_1b12 = D_800F1B12;
  temp_a0_da = temp_a0_da | var_a1;
  D_801078DA = temp_a0_da;
  D_800F1B12 = temp_v0_1b12 & ~temp_a0_da;
}
void func_800872A4(void) {
}
INCLUDE_ASM("asm/funcs", func_80087770);
extern s16 D_80102806;
s16 func_80087CAC(s32 a0, s16 *a1, s16 *a2) {
    u8 *base;
    s16 *ptr;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    ptr = &D_80102806;
    *ptr = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    *a1 = *(u16 *)(p + 0x58);
    *a2 = *(u16 *)(p + 0x5A);
    return *ptr;
}

s16 func_80087D10(s32 a0) {
    u8 *base;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    __asm__ volatile("" ::: "memory");
    D_80102806 = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    return *(s16 *)(p + 0x58);
}

s16 func_80087D58(s32 a0) {
    u8 *base;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    __asm__ volatile("" ::: "memory");
    D_80102806 = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    return *(s16 *)(p + 0x5A);
}
extern u8 g_memcard_slot;
extern s16 D_800F4E28[];
void spu_NotifyChannel(s16 a0) {
    s32 s0 = 0;
    s16 s1;
    if (g_memcard_slot == 0) {
        return;
    }
    s1 = (s16)a0;
    do {
        s32 idx = (u8)s0;
        s32 off = ((idx * 8 - idx) * 4 - idx) * 2;
        if (*(s16 *)((u8 *)D_800F4E28 + off) == s1) {
            D_8010280A = (u8)s0;
            func_800871D4(0);
        }
        s0++;
    } while ((u8)s0 < g_memcard_slot);
}
extern u8 D_80102A68[];
extern s16 D_800FF634;
extern s32 D_800F66B8[];
extern s32 D_800F6660[];
extern s32 D_800F6700[];
extern u8 D_801027F1;
extern u8 D_801027F6;
extern s32 D_80101BC4;
extern s32 D_800FF6A0;
s32 AddTbpOfst(u16 a0, s16 a1) {
    s32 idx;
    s32 sa1;
    s32 v0;
    int v1;
    s32 v2;
    s32 entry;
    if ((a0 & 0xFFFF) >= 0x10) goto fail;
    idx = (s16)a0;
    if (D_80102A68[idx] != 1) return -1;
    sa1 = (s16)a1;
    if (sa1 < D_800FF634) goto ok;
fail:
    return -1;
ok:
    v0 = D_800F66B8[idx];

    v1 = D_800F6660[idx];
    v2 = D_800F6700[idx];
    D_801027F1 = (u8) a0;
    sa1 = sa1 << 4;
    do { } while (0);
    D_801027F6 = (u8) a1;
    entry = *((s32 *) ((sa1 + v1) + 8));
    D_80101BC4 = v0;
    D_800FF6A0 = v1;
    D_80101BC8 = v2;
    D_801027F7 = (u8)entry;
    return 0;
}
/* kengo:MED  |  am_rmd/AddTbpOfst  |  49i */
extern u8 g_memcard_data;
void memcard_SetData(u8 a0) {
    g_memcard_data = a0;
}
void memcard_SetBusy(void) {
    g_memcard_busy = 1;
}

void memcard_ClearBusy(void) {
    g_memcard_busy = 0;
}

extern u8 g_memcard_slot;
s32 memcard_SetSlot(s32 a0) {
    u8 v = (u8)a0;
    if (v >= 0x19 || v == 0) {
        return 0xFF;
    }
    g_memcard_slot = a0;
    return v;
}
extern u8 g_snd_ch_status[];
extern s32 g_snd_ch_addr[];
extern u16 g_snd_ch_count;
extern void spu_DmaTransfer(s32);
void func_80087F64(s16 a0) {
    if ((u16)a0 < 0x10) {
        s16 idx = a0;
        if (g_snd_ch_status[idx] == 1) {
            spu_DmaTransfer(g_snd_ch_addr[idx]);
            g_snd_ch_status[idx] = 0;
            g_snd_ch_count--;
        }
    }
}
extern u8 g_snd_ch_status[];

s16 func_80087FE8(s16 a0) {
    if ((u16)a0 < 0x11) {
        if (g_snd_ch_status[a0] == 2) {
            func_8008AF58(0);
            g_snd_ch_status[a0] = 1;
            return a0;
        }
    }
    return -1;
}

s16 func_80088058(s32 a0, s16 a1) {
    return saTan2Main(a0, a1, 0, 0);
}

s16 func_80088088(s32 a0, s16 a1, s32 a2) {
    return saTan2Main(a0, a1, 1, a2);
}

s16 func_800880B8(s32 a0, s16 a1, s32 a2) {
    return saTan2Main(a0, a1, 1, a2);
}
INCLUDE_ASM("asm/funcs", saTan2Main);
/* kengo:MED  |  sa_tan2/saTan2Main  |  247i */
extern u8 g_snd_ch_status[];
extern s32 g_snd_ch_addr[];
extern s32 D_801077C8[];
extern s32 func_8008AE24(s32);
extern s32 func_8008ADC4(s32, s32);
s16 func_800884C4(s32 a0, s16 a1) {
    if ((u16)a1 >= 0x11) {
        func_8008AF58(0);
        return -1;
    }
    if (g_snd_ch_status[a1] != 2) {
        func_8008AF58(0);
        return -1;
    }
    {
        s32 s0 = g_snd_ch_addr[a1];
        func_8008AE7C(0);
        func_8008AE24(s0);
        func_8008ADC4(a0, D_801077C8[a1]);
        g_snd_ch_status[a1] = 1;
    }
    return a1;
}

s16 func_80088584(s16 a0) {
    return func_8008AEB0(a0);
}

void spu_Init(void) {
    spu_InitEx(0);
}

void spu_InitEx(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    irq_DisableInterrupts();
    func_80088740(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = (u16 *)&D_800A28D2;
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    spu_InitIrq();
    D_800A287C = 0;
    D_800A2880 = 0;
    D_800A288C = 0;
    D_800A2890 = 0;
    D_800A2892 = 0;
    D_800A2894 = 0;
    D_800A2898 = 0;
    D_800A2884 = D_800A2D44;
    spu_WriteReg(0xD1, D_800A2D44, 0);
    g_spu_voice_key_a = 0;
    g_spu_voice_key_b = 0;
    g_spu_voice_key_c = 0;
    g_snd_reverb_flag = 0;
    g_spu_reverb_mode = 0;
    D_800A2874 = 0;
    D_800A28A0 = 0;
    D_800A289C = 0;
    D_800A2CD4 = 0;
}
extern s32 g_snd_init_flag;
extern s32 g_snd_irq_handle;
extern s32 g_snd_irq_data;

void spu_InitIrq(void) {
    s32 v0;
    if (g_snd_init_flag == 0) {
        g_snd_init_flag = 1;
        EnterCriticalSection();
        spu_SetCallback((s32)&g_snd_irq_data);
        v0 = func_80078978((s32)0xF0000009, 0x20, 0x2000, 0);
        g_snd_irq_handle = v0;
        func_800789A8(v0);
        ExitCriticalSection();
    }
}
void func_80088740(s32 a0) {
    s32 base;
    s32 count;
    s32 i;
    s16 *a1_ptr;

    *D_800A2CEC |= 0xB0000;

    base = D_800A2CDC;
    D_800A2CF8 = 0;
    D_800A2CFC = 0;
    D_800A2CF4 = 0;
    *(s16 *)(base + 0x180) = 0;
    *(s16 *)(base + 0x182) = 0;
    *(s16 *)(base + 0x1AA) = 0;
    spu_WriteReg16();

    base = D_800A2CDC;
    *(volatile s16 *)(base + 0x180) = 0;
    *(volatile s16 *)(base + 0x182) = 0;

    if (*(volatile u16 *)(base + 0x1AE) & 0x7FF) {
        count = 0;
        do {
            count++;
            if ((u32)count >= 0xF01) {
                debug_printf(&D_800163D8, &D_800163E8);
                goto common_init;
            }
        } while (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF);
    }

common_init:
    i = 0;
    a1_ptr = (s16 *)&D_800F7420;
    D_800A2D00 = 2;
    D_800A2D04 = 3;
    D_800A2D08 = 8;
    D_800A2D0C = 7;
    __asm__ __volatile__("" : : : "memory");

    base = D_800A2CDC;
    *(s16 *)(base + 0x1AC) = 4;
    *(s16 *)(base + 0x184) = 0;
    *(s16 *)(base + 0x186) = 0;
    *(s16 *)(base + 0x18C) = (s16)0xFFFF;
    *(s16 *)(base + 0x18E) = (s16)0xFFFF;
    *(s16 *)(base + 0x198) = 0;
    *(s16 *)(base + 0x19A) = 0;

loop_10:
    *a1_ptr = 0;
    a1_ptr++;
    i++;
    if (i < 10) goto loop_10;

    if (a0 != 0) {
        goto end_init;
    }

    {
        s32 addr;
        s32 s1_val;
        s32 s0_val;
        s16 *p;
        s16 c_3fff;
        s16 c_200;

        addr = (s32)&D_800A2D1C;
        base = D_800A2CDC;
        D_800A2CF4 = 0x200;
        *(s16 *)(base + 0x190) = 0;
        *(s16 *)(base + 0x192) = 0;
        *(s16 *)(base + 0x194) = 0;
        *(s16 *)(base + 0x196) = 0;
        *(s16 *)(base + 0x1B0) = 0;
        *(s16 *)(base + 0x1B2) = 0;
        *(s16 *)(base + 0x1B4) = 0;
        *(s16 *)(base + 0x1B6) = 0;
        DispUpdateStatusMessage(addr, 0x10);

        i = 0;
        c_3fff = (s16)0x3FFF;
        c_200 = (s16)0x200;
        p = (s16 *)D_800A2CDC;
loop_18:
        p[0] = 0;
        p[1] = 0;
        p[2] = c_3fff;
        p[3] = c_200;
        p[4] = 0;
        p[5] = 0;
        p += 8;
        i++;
        if (i < 0x18) goto loop_18;

        s1_val = 0xFFFF;
        s0_val = 0xFF;
        base = D_800A2CDC;
        *(s16 *)(base + 0x188) = s1_val;
        *(s16 *)(base + 0x18A) = s0_val;
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();

        base = D_800A2CDC;
        *(s16 *)(base + 0x18C) = s1_val;
        *(s16 *)(base + 0x18E) = s0_val;
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
    }

end_init:
    base = D_800A2CDC;
    D_800A2D10 = 1;
    *(s16 *)(base + 0x1AA) = (s16)0xC000;
    D_800A2D14 = 0;
    D_800A2D18 = 0;
}
INCLUDE_ASM("asm/funcs", DispUpdateStatusMessage);
/* kengo:LOW  |  su_menu_home/_DispUpdateStatusMessage  |  206i  |  PS2 UI — reverted */
s32 saTan0GaugeDraw(s32 mode, ...) {
    u32 i;
    s32 *args = (s32 *)__builtin_next_arg(mode);
    s32 second;
    s32 ctrl;
    s32 mask;

    if (mode == 1) goto m1;
    if (mode >= 2) goto ge2;
    if (mode == 0) goto m0;
    return 0;

ge2:
    if (mode == 2) goto m2;
    if (mode == 3) goto m3;
    return 0;

m2:
    {
        u32 shifted = (u32)args[0] >> D_800A2D04;
        D_800A2CF4 = (u16)shifted;
        *(u16 *)(D_800A2CDC + 0x1A6) = (u16)shifted;
    }
    return 0;

m1:
    D_800A2D2C = 0;
    if ((*(volatile u16 *)(D_800A2CDC + 0x1A6) & 0xFFFFu) != D_800A2CF4) {
        i = 0;
        do {
            i++;
            if (i >= 0xF01) return -2;
        } while (*(volatile u16 *)(D_800A2CDC + 0x1A6) != D_800A2CF4);
    }
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = (u16)((*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0xFFCFu) | 0x20u);
    return 0;

m0:
    D_800A2D2C = 1;
    if ((*(volatile u16 *)(D_800A2CDC + 0x1A6) & 0xFFFFu) != D_800A2CF4) {
        i = 0;
        do {
            i++;
            if (i >= 0xF01) return -2;
        } while (*(volatile u16 *)(D_800A2CDC + 0x1A6) != D_800A2CF4);
    }
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = (u16)(*(volatile u16 *)(D_800A2CDC + 0x1AA) | 0x30u);
    return 0;

m3:
    mask = 0x20;
    if (D_800A2D2C == 1) mask = 0x30;
    if ((*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0x30u) != (u32)mask) {
        i = 0;
        do {
            i++;
            if (i >= 0xF01) return -2;
        } while ((*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0x30u) != ((((u32)mask) & 0xFFFF) & 0xFFFF));
    }
    if (D_800A2D2C == 1) {
        args++;
        spu_ReadReg();
    } else {
        args++;
        spu_ReadStatus();
    }
    D_800A2D30 = args[-1];
    second = (u32)args[0];
    D_800A2D34 = (second >> 6) + ((second & 0x3F) != 0);
    *D_800A2CE0 = D_800A2D30;
    *D_800A2CE4 = (D_800A2D34 << 16) | 0x10;
    ctrl = 0x01000201;
    if (D_800A2D2C == 1) ctrl = 0x01000200;
    *D_800A2CE8 = ctrl;
    return 0;
}
/* kengo:MED  |  sa_tan0/saTan0GaugeDraw  |  164i */
extern void DispUpdateStatusMessage(s32, s32);
s32 spu_TransferData(s32 a0, s32 a1) {
    if (g_spu_reverb_mode == 0) {
        saTan0GaugeDraw(2, g_spu_xfer_addr << g_spu_addr_shift);
        saTan0GaugeDraw(1);
        saTan0GaugeDraw(3, a0, a1);
    } else {
        DispUpdateStatusMessage(a0, a1);
    }
    return a1;
}
s32 spu_TransferDirect(s32 a0, s32 a1) {
    saTan0GaugeDraw(2, g_spu_xfer_addr << g_spu_addr_shift);
    saTan0GaugeDraw(0);
    saTan0GaugeDraw(3, a0, a1);
    return a1;
}
void spu_WriteReg(s32 arg0, u32 arg1, s32 arg2) {
    register s32 temp_v0 asm("v0");
    register s32 temp_a0 asm("a0");
    s32 temp_v1;

    temp_v0 = arg0 * 2;
    if (arg2 == 0) {
        *(u16 *)(temp_v0 + g_spu_base_addr) = arg1;
        __asm__("" ::: "memory");
        return;
    }
    temp_a0 = g_spu_base_addr;
    temp_v1 = g_spu_addr_shift;
    *(u16 *)(temp_v0 + temp_a0) = arg1 >> temp_v1;
}
s32 saTan1SyuryoDraw(s32 mode, s32 val) {
    s32 aligned;
    if (D_800A2D00 != 0) {
        u32 step = D_800A2D08;
        if ((u32)val % step != 0) {
            val = (val + step) & ~D_800A2D0C;
        }
    }
    aligned = (s32)((u32)val >> D_800A2D04);
    if (mode == -2) goto ret_val_m2;
    if (mode != -1) goto store;
    return aligned & 0xFFFF;
ret_val_m2:
    return val;
store:
    ((s16 *)D_800A2CDC)[mode] = (s16)aligned;
    return val;
}
s32 func_80089178(s32 index, s32 mode) {
    u16 val = ((u16 *)D_800A2CDC)[index];
    if (mode == -1) {
        return val;
    }
    return val << D_800A2D04;
}
void func_800891B4(s32 arg0) {
    *D_800A2CEC &= 0xFFF8FFFF;
    if (arg0 != 0) {
        *D_800A2CEC |= 0x30000;
    } else {
        *D_800A2CEC |= 0x50000;
    }
}
extern volatile u32 *g_spu_dma_ctrl;
void spu_ReadStatus(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_FROM_RAM;
}
void spu_ReadReg(void) {
    *g_spu_dma_ctrl = (*g_spu_dma_ctrl & DMA_CHAN_MASK) | DMA_SPU_TO_RAM;
}
void spu_WriteReg16(void) {
    volatile s32 i;
    volatile s32 v = 0xD;
    for (i = 0; i < 0x3C; i++) {
        v = v * 13;
    }
}
void spu_SetCallback(s32 a0) {
    irq_AcknowledgeVblank(4, a0);
}
extern s32 g_spu_timer;

extern s32 g_spu_timer;

extern s32 g_spu_timer;

void func_800892F8(void) {
    if (g_snd_init_flag == 1) {
        g_snd_init_flag = 0;
        EnterCriticalSection();
        g_spu_init_flag = 0;
        g_spu_timer = 0;
        spu_SetCallback(0);
        func_80078988(g_snd_irq_handle);
        func_80089374(g_snd_irq_handle);
        ExitCriticalSection();
    }
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_80089374\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xD\n"
    "    nop\n"
    "endlabel func_80089374\n"
    ".set reorder\n"
    ".set at\n"
);
extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;

extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;

s32 spu_IrqHandler(s32 a0, s32 *a1) {
    int new_var2;
    int new_var;
    s32 v0 = a0;
    if (v0 <= 0) {
        new_var2 = 1;
        if (new_var2) {
            return 0;
        }
    }
    new_var = 0x10000 << g_spu_addr_shift;
    *a1 = 0x40001010;
    g_spu_voice_key_c = (s32)a1;
    g_spu_voice_key_b = 0;
    g_spu_voice_key_a = v0;
    a1[1] = new_var - 0x1010;
    return v0;
}
extern void exec_game(void);
s32 coli_HitPauseKatana(s32 arg0) {
    s32 i = 0;
    s32 found = -1;
    s32 shift_val;
    unsigned char new_var;

    if (D_800A2880 == 0) {
        shift_val = 0;
    } else {
        shift_val = (0x10000 - D_800A2884) << D_800A2D04;
    }

    {
        s32 a0_val = arg0;
        if (arg0 & ~D_800A2D0C) {
            a0_val = arg0 + D_800A2D0C;
        }
        arg0 = (a0_val >> D_800A2D04) << D_800A2D04;
    }

    if (*(s32 *)g_spu_voice_key_c & 0x40000000) {
        found = 0;
    } else {
        exec_game();
        if (i < g_spu_voice_key_a) {
            u32 stopbit = 0x40000000;
            u32 highbit = 0x80000000;
            s32 count = g_spu_voice_key_a;
            s32 *ptr = (s32 *)g_spu_voice_key_c;
            do {
                s32 val = *ptr;
                if (val & stopbit) goto found_match;
                if (!(val & highbit)) goto next_iter;
                if ((u32)ptr[1] < (u32)arg0) goto next_iter;
            found_match:
                found = i;
                goto after_search;
            next_iter:
                i++;
                ptr = (s32 *)((u8 *)ptr + 8);
            } while (i < count);
        }
    }

after_search:
    if (found == -1) {
        return -1;
    }

    {
        s32 idx = found << 3;
        s32 *tbl = (s32 *)g_spu_voice_key_c;
        s32 *entry = (s32 *)((u8 *)tbl + idx);
        s32 field0 = *entry;

        if (field0 & 0x40000000) {
            s32 f4;
            if (found >= g_spu_voice_key_a) {
                return -1;
            }
            f4 = entry[1];
            if ((u32)(f4 - shift_val) < (u32)arg0) {
                return -1;
            }
            {
                s32 next_idx = found + 1;
                s32 *next = (s32 *)((u8 *)tbl + (next_idx << 3));
                *next = ((*entry & 0x0FFFFFFF) + arg0) | 0x40000000;
                next[1] = f4 - arg0;
                g_spu_voice_key_b = next_idx;
                entry[1] = arg0;
                *entry = *entry & 0x0FFFFFFF;
                exec_game();
                return *(s32 *)((u8 *)(s32 *)g_spu_voice_key_c + idx);
            }
        }

        {
            s32 f4 = entry[1];
            if ((u32)arg0 < (u32)f4 && g_spu_voice_key_b < g_spu_voice_key_a) {
                s32 *kb = (s32 *)((u8 *)tbl + (g_spu_voice_key_b << 3));
                s32 old0 = kb[new_var = 0];
                s32 old1 = kb[1];
                kb[0] = (field0 + arg0) | 0x80000000;
                kb[1] = f4 - arg0;
                g_spu_voice_key_b++;
                kb[2] = old0;
                kb[3] = old1;
            }

            {
                s32 idx2 = found << 3;
                s32 *e2 = (s32 *)((u8 *)(s32 *)g_spu_voice_key_c + idx2);
                e2[1] = arg0;
                e2[0] = e2[new_var] & 0x0FFFFFFF;
                exec_game();
                return *(s32 *)((u8 *)(s32 *)g_spu_voice_key_c + idx2);
            }
        }
    }
}
/* kengo:HIGH  |  is_coli/coli_HitPauseKatana  |  178i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", exec_game);
/* kengo:HIGH  |  md_game/exec_game  |  194i */
extern s32 g_spu_voice_key_a;
extern void exec_game(void);
void spu_DmaTransfer(s32 a0) {
    s32 count;
    s32 i;
    volatile s32 pad;
    count = g_spu_voice_key_a;
    i = 0;
    if (count > 0) {
        u32 stopbit = 0x40000000;
        u32 highbit = 0x80000000;
        s32 marked = a0 | highbit;
        s32 n = count;
        s32 *ptr = g_spu_voice_key_c;
        do {
            s32 val;
            __asm__("nop");
            val = *ptr;
            if (val & stopbit) {
                break;
            }
            if (val == a0) {
                *ptr = marked;
                break;
            }
            i++;
            ptr = (s32 *)((u8 *)ptr + 8);
        } while (i < n);
    }
    exec_game();
}

void spu_WaitReady(s32 a0, s32 a1) {
    coli_HitPauseKatana_2(a0, a1, 0xCA, 0xCB);
}

s32 coli_HitPauseKatana_2(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
  register s32 mode asm("t1") = arg0;
  volatile u16 *base;
  register u32 t2 asm("t2");
  s32 new_var;
  s32 new_var3;
  s32 new_var2 = arg3;
  register u32 mask asm("t0") = arg1;
  if (D_800A2CD4 & 1)
  {
    base = D_800F7298;
  }
  else
  {
    base = (volatile u16 *) D_800A2CDC;
  }
  new_var3 = new_var2;
  t2 = (((u32) (base[new_var3] & 0xFF)) << 16) | base[arg2];
  switch (mode)
  {
    new_var = arg2;
    case 1:
      if (D_800A2CD4 & 1)
    {
      D_800F7298[new_var] |= mask;
      D_800F7298[arg3] |= (mask >> 16) & 0xFF;
      __asm__ volatile("" ::: "memory");
      D_800A28A0 |= 1 << ((new_var - 0xC6) >> 1);
    }
    else
    {
      ((volatile u16 *) D_800A2CDC)[new_var] |= mask;
      ((volatile u16 *) D_800A2CDC)[arg3] |= (mask >> 16) & 0xFF;
    }
      t2 |= (mask & 0xFFFFFF);
      break;

    case 0:
      if (D_800A2CD4 & 1)
    {
      D_800F7298[new_var] &= ~mask;
      do
      {
        D_800F7298[arg3] &= ~((mask >> 16) & 0xFF);
        D_800A28A0 |= 1 << ((new_var - 0xC6) >> 1);
      }
      while (0);
    }
    else
    {
      ((volatile u16 *) D_800A2CDC)[new_var] &= ~mask;
      ((volatile u16 *) D_800A2CDC)[arg3] &= ~((mask >> 16) & 0xFF);
    }
      t2 &= ~(mask & 0xFFFFFF);
      break;

    case 8:
      if (D_800A2CD4 & 1)
    {
      D_800F7298[new_var] = mask;
      D_800F7298[arg3] = (mask >> 16) & 0xFF;
      D_800A28A0 |= 1 << ((new_var - 0xC6) >> 1);
    }
    else
    {
      ((volatile u16 *) D_800A2CDC)[new_var] = mask;
      ((volatile u16 *) D_800A2CDC)[arg3] = (mask >> 16) & 0xFF;
    }
      t2 = mask & 0xFFFFFF;
      break;

  }

  return t2 & 0xFFFFFF;
}
/* kengo:HIGH  |  is_coli/coli_HitPauseKatana_2  |  178i  |  x2 size collision */
s32 func_80089D10(s32 a0) {
    s32 val;
    if (a0 < 0) {
        val = 0;
    } else if (a0 >= 0x40) {
        val = 0x3F;
    } else {
        val = a0;
    }
    {
        volatile u16 *ptr = (volatile u16 *)(g_spu_base_addr + 0x1AA);
        u16 tmp = *ptr;
        *ptr = (tmp & 0xC0FF) | ((val & 0x3F) << 8);
    }
    return val;
}
s32 saEft03Start2(s32 a0) {
    u8 *p;
    u16 v1;

    if (a0 == 0) goto case_0;
    if (a0 == 1) goto case_1;
    goto exit_load;

case_0:
    D_800A287C = 0;
    p = (u8 *)D_800A2CDC;
    v1 = *(u16 *)(p + 0x1AA);
    v1 &= 0xFF7F;
    goto store_v1;

case_1:
    if (D_800A2880 == a0) goto set_flag;
    if (func_80089EB0(D_800A2884) == 0) goto set_flag;
    p = (u8 *)D_800A2CDC;
    v1 = *(u16 *)(p + 0x1AA);
    D_800A287C = 0;
    v1 &= 0xFF7F;
    goto store_v1;

set_flag:
    p = (u8 *)D_800A2CDC;
    v1 = *(u16 *)(p + 0x1AA);
    D_800A287C = a0;
    v1 |= 0x80;

store_v1:
    *(u16 *)(p + 0x1AA) = v1;

exit_load:
    return D_800A287C;
}
s32 func_80089E30(u32 a0) {
    register s32 shift asm("v0");
    register s32 entry asm("v1");
    register u32 *p asm("a1");
    register u32 t0 asm("t0");
    register u32 a3reg asm("a3");
    register u32 a2 asm("a2");

    entry = g_spu_voice_key_c;
    if (entry != 0) goto body;
    shift = 0;
    goto end;
body:
    t0 = 0x80000000U;
    a3reg = 0x40000000U;
    a2 = 0x0FFFFFFFU;
    p = (u32 *)entry;
loop:
    entry = p[0];
    if (entry & t0) goto iter;
    if (entry & a3reg) goto ret0;
    entry &= a2;
    if ((u32)entry >= a0) {
        shift = 1;
        goto end;
    }
    shift = p[1];
    shift = (s32)((u32)entry + (u32)shift);
    if (a0 < (u32)shift) {
        shift = 1;
        goto end;
    }
iter:
    p += 2;
    goto loop;
ret0:
    __asm__ volatile("move $2,$0" : "=r"(shift));
end:
    return shift;
}
s32 func_80089EB0(u32 a0) {
    register s32 shift asm("v0");
    register s32 entry asm("v1");
    register u32 *p asm("a1");
    register u32 t0 asm("t0");
    register u32 a3reg asm("a3");
    register u32 a2 asm("a2");

    shift = g_spu_addr_shift;
    entry = g_spu_voice_key_c;
    a0 <<= shift;
    if (entry != 0) goto body;
    __asm__ volatile("" ::: "v1");
    shift = 0;
    goto end;
body:
    t0 = 0x80000000U;
    a3reg = 0x40000000U;
    a2 = 0x0FFFFFFFU;
    p = (u32 *)entry;
loop:
    entry = p[0];
    if (entry & t0) goto iter;
    if (entry & a3reg) goto ret0;
    entry &= a2;
    if ((u32)entry >= a0) {
        shift = 1;
        goto end;
    }
    shift = p[1];
    shift = (s32)((u32)entry + (u32)shift);
    if (a0 < (u32)shift) {
        shift = 1;
        goto end;
    }
iter:
    p += 2;
    goto loop;
ret0:
    __asm__ volatile("move $2,$0" : "=r"(shift));
end:
    return shift;
}
void func_80089F3C(s32 *arg0) {
    (void)arg0;
}
void func_8008A434(s32 *arg0) {
    s32 flags = arg0[0];
    s32 zero = flags == 0;

    if (zero || (flags & 0x1)) { *(u16 *)(g_spu_base_addr + 0x1C0) = *(u16 *)((s32)arg0 + 0x4); }
    if (zero || (flags & 0x2)) { *(u16 *)(g_spu_base_addr + 0x1C2) = *(u16 *)((s32)arg0 + 0x6); }
    if (zero || (flags & 0x4)) { *(u16 *)(g_spu_base_addr + 0x1C4) = *(u16 *)((s32)arg0 + 0x8); }
    if (zero || (flags & 0x8)) { *(u16 *)(g_spu_base_addr + 0x1C6) = *(u16 *)((s32)arg0 + 0xA); }
    if (zero || (flags & 0x10)) { *(u16 *)(g_spu_base_addr + 0x1C8) = *(u16 *)((s32)arg0 + 0xC); }
    if (zero || (flags & 0x20)) { *(u16 *)(g_spu_base_addr + 0x1CA) = *(u16 *)((s32)arg0 + 0xE); }
    if (zero || (flags & 0x40)) { *(u16 *)(g_spu_base_addr + 0x1CC) = *(u16 *)((s32)arg0 + 0x10); }
    if (zero || (flags & 0x80)) { *(u16 *)(g_spu_base_addr + 0x1CE) = *(u16 *)((s32)arg0 + 0x12); }
    if (zero || (flags & 0x100)) { *(u16 *)(g_spu_base_addr + 0x1D0) = *(u16 *)((s32)arg0 + 0x14); }
    if (zero || (flags & 0x200)) { *(u16 *)(g_spu_base_addr + 0x1D2) = *(u16 *)((s32)arg0 + 0x16); }
    if (zero || (flags & 0x400)) { *(u16 *)(g_spu_base_addr + 0x1D4) = *(u16 *)((s32)arg0 + 0x18); }
    if (zero || (flags & 0x800)) { *(u16 *)(g_spu_base_addr + 0x1D6) = *(u16 *)((s32)arg0 + 0x1A); }
    if (zero || (flags & 0x1000)) { *(u16 *)(g_spu_base_addr + 0x1D8) = *(u16 *)((s32)arg0 + 0x1C); }
    if (zero || (flags & 0x2000)) { *(u16 *)(g_spu_base_addr + 0x1DA) = *(u16 *)((s32)arg0 + 0x1E); }
    if (zero || (flags & 0x4000)) { *(u16 *)(g_spu_base_addr + 0x1DC) = *(u16 *)((s32)arg0 + 0x20); }
    if (zero || (flags & 0x8000)) { *(u16 *)(g_spu_base_addr + 0x1DE) = *(u16 *)((s32)arg0 + 0x22); }
    if (zero || (flags & 0x10000)) { *(u16 *)(g_spu_base_addr + 0x1E0) = *(u16 *)((s32)arg0 + 0x24); }
    if (zero || (flags & 0x20000)) { *(u16 *)(g_spu_base_addr + 0x1E2) = *(u16 *)((s32)arg0 + 0x26); }
    if (zero || (flags & 0x40000)) { *(u16 *)(g_spu_base_addr + 0x1E4) = *(u16 *)((s32)arg0 + 0x28); }
    if (zero || (flags & 0x80000)) { *(u16 *)(g_spu_base_addr + 0x1E6) = *(u16 *)((s32)arg0 + 0x2A); }
    if (zero || (flags & 0x100000)) { *(u16 *)(g_spu_base_addr + 0x1E8) = *(u16 *)((s32)arg0 + 0x2C); }
    if (zero || (flags & 0x200000)) { *(u16 *)(g_spu_base_addr + 0x1EA) = *(u16 *)((s32)arg0 + 0x2E); }
    if (zero || (flags & 0x400000)) { *(u16 *)(g_spu_base_addr + 0x1EC) = *(u16 *)((s32)arg0 + 0x30); }
    if (zero || (flags & 0x800000)) { *(u16 *)(g_spu_base_addr + 0x1EE) = *(u16 *)((s32)arg0 + 0x32); }
    if (zero || (flags & 0x1000000)) { *(u16 *)(g_spu_base_addr + 0x1F0) = *(u16 *)((s32)arg0 + 0x34); }
    if (zero || (flags & 0x2000000)) { *(u16 *)(g_spu_base_addr + 0x1F2) = *(u16 *)((s32)arg0 + 0x36); }
    if (zero || (flags & 0x4000000)) { *(u16 *)(g_spu_base_addr + 0x1F4) = *(u16 *)((s32)arg0 + 0x38); }
    if (zero || (flags & 0x8000000)) { *(u16 *)(g_spu_base_addr + 0x1F6) = *(u16 *)((s32)arg0 + 0x3A); }
    if (zero || (flags & 0x10000000)) { *(u16 *)(g_spu_base_addr + 0x1F8) = *(u16 *)((s32)arg0 + 0x3C); }
    if (zero || (flags & 0x20000000)) { *(u16 *)(g_spu_base_addr + 0x1FA) = *(u16 *)((s32)arg0 + 0x3E); }
    if (zero || (flags & 0x40000000)) { *(u16 *)(g_spu_base_addr + 0x1FC) = *(u16 *)((s32)arg0 + 0x40); }
    if (zero || (flags < 0)) { *(u16 *)(g_spu_base_addr + 0x1FE) = *(u16 *)((s32)arg0 + 0x42); }
}

void func_8008A904(s32 a0, s32 a1) {
    coli_HitPauseKatana_2(a0, a1, 0xCC, 0xCD);
}

s32 md_game_check_change_main_mode_katinuki(s32 arg0) {
    volatile s32 sp10;
    s32 s1_val;
    s32 s2_val;
    s32 s3_flag;
    s32 s4_saved;
    s32 s5_flag;
    s32 chunk;
    s32 v0_cmp;
    s32 *table_ptr;
    s32 *base;

    sp10 = 0;
    s5_flag = 0;
    if ((u32)arg0 >= 10u) {
        return -1;
    }
    base = &D_800A2D44;
    table_ptr = (s32 *)((arg0 << 2) + (s32)base);
    if (func_80089EB0(*table_ptr) != 0) {
        return -1;
    }
    if (arg0 == 0) {
        s1_val = 0x10 << D_800A2D04;
        s2_val = (s32)0xFFF0 << D_800A2D04;
    } else {
        s32 entry_val = *table_ptr;
        s1_val = (0x10000 - entry_val) << D_800A2D04;
        s2_val = entry_val << D_800A2D04;
    }
    s4_saved = D_800A2CF8;
    if (s4_saved == 1) {
        D_800A2CF8 = 0;
        s5_flag = 1;
    }
    s3_flag = 1;
    if (D_800A2D14 != 0) {
        sp10 = D_800A2D14;
        D_800A2D14 = 0;
    }
    ;
loop:
    if (!((u32)s1_val < 0x401u)) {
        chunk = 0x400;
    } else {
        do { chunk = s1_val; } while (0);
        s3_flag = 0;
    }
    saTan0GaugeDraw(2, s2_val);
    saTan0GaugeDraw(1);
    saTan0GaugeDraw(3, &D_800A28D4, chunk);
    func_8008AAC4(D_800A2870);
    s1_val -= 0x400;
    s2_val += 0x400;
    if (s3_flag != 0) {
        v0_cmp = (u32)s1_val < 0x401u;
        goto loop;
    }
    if (s5_flag != 0) {
        D_800A2CF8 = s4_saved;
    }
    if (sp10 != 0) {
        D_800A2D14 = sp10;
    }
    return 0;
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008AAC4\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel func_8008AAC4\n"
    ".set reorder\n"
    ".set at\n"
);
void func_8008AAD4(s32 arg0, u32 arg1) {
    register u32 mask asm("a1");
    register u32 lo asm("a2");
    register u32 hi asm("a3");
    register u32 hi32 asm("t0");

    mask = arg1 & 0xFFFFFF;
    lo = mask;
    hi32 = mask >> 16;
    hi = hi32;

    if (arg0 == 0) goto case0;
    if (arg0 != 1) return;

    if (D_800A2CD4 & 1) {
        D_800F7420[0] = (u16)lo;
        D_800F7420[1] = (u16)hi;
        *(volatile s32 *)&D_800A28A0 = *(volatile s32 *)&D_800A28A0 | 1;
        *(volatile s32 *)&D_800A289C = *(volatile s32 *)&D_800A289C | mask;
        if (D_800F7424[0] & mask) {
            D_800F7424[0] = (u16)(D_800F7424[0] & ~mask);
        }
        if (D_800F7424[1] & hi32) {
            D_800F7424[1] = (u16)(D_800F7424[1] & ~hi32);
        }
        return;
    }
    *(s16 *)(D_800A2CDC + 0x188) = (s16)lo;
    *(s16 *)(D_800A2CDC + 0x18A) = (s16)hi;
    D_800A2874 = D_800A2874 | mask;
    return;

case0:
    if (D_800A2CD4 & 1) {
        u32 notmask = ~mask;
        D_800F7424[0] = (u16)lo;
        D_800F7424[1] = (u16)hi;
        D_800A28A0 |= 1;
        D_800A289C &= notmask;
        if (D_800F7420[0] & mask) {
            D_800F7420[0] = (u16)(D_800F7420[0] & notmask);
        }
        if (D_800F7420[1] & hi32) {
            D_800F7420[1] = (u16)(D_800F7420[1] & ~hi32);
        }
        return;
    }
    *(s16 *)(D_800A2CDC + 0x18C) = (s16)lo;
    *(s16 *)(D_800A2CDC + 0x18E) = (s16)hi;
    D_800A2874 = D_800A2874 & ~mask;
}
s32 func_8008ACD0(s32 arg0) {
    register s32 bit_found asm("a1");
    register s32 i asm("v1");
    register s32 one asm("a2");
    register s32 mask asm("v0");
    register s32 base asm("a0");
    s32 flags;

    bit_found = -1;
    i = 0;
    one = 1;
    __asm__ volatile("sllv %0, %1, %2" : "=r"(mask) : "r"(one), "r"(i));

loop:
    if (arg0 & mask) {
        goto found;
    }
    i++;
    if (i < 0x18) {
        mask = one << i;
        goto loop;
    }

check:
    mask = -1;
    if (bit_found != mask) goto work;
    return mask;

found:
    bit_found = i;
    goto check;

work:
    base = bit_found << 4;
    mask = g_spu_base_addr;
    flags = D_800A2874;
    base = base + mask;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
    if (!flags) goto no_flags;
    if (!base) goto ret3;
    goto ret1;
no_flags:
    return (s32)(base != 0) << 1;
ret3:
    return 3;
ret1:
    return 1;
}
s32 func_8008AD64(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    spu_TransferDirect(a0, a1);
    if (g_spu_init_flag == 0) {
        g_spu_busy = 0;
    }
    return a1;
}
s32 func_8008ADC4(s32 a0, s32 a1) {
    if ((u32)a1 > 0x7EFF0u) {
        a1 = 0x7EFF0;
    }
    spu_TransferData(a0, a1);
    if (g_spu_init_flag == 0) {
        g_spu_busy = 0;
    }
    return a1;
}
extern s32 func_800890D4(s32, s32);
s32 func_8008AE24(s32 a0) {
    s32 v0;
    if ((u32)(a0 - 0x1010) > (u32)0x7EFE8) {
        return 0;
    }
    v0 = func_800890D4(-1, a0);
    g_spu_xfer_addr = (u16)v0;
    return (u32)(u16)v0 << g_spu_addr_shift;
}
void func_8008AE7C(s32 a0) {
    int new_var;
    s32 v0;
    new_var = 1;
    if (a0 == 0) {
        v0 = 0;
    } else if (new_var == a0) {
        v0 = new_var;
    } else {
        v0 = 0;
        v0++;
        v0--;
    }
    g_snd_reverb_flag = a0;
    g_spu_reverb_mode = v0;
}
s32 func_8008AEB0(s32 arg0) {
    s32 var_v0;

    if ((g_snd_reverb_flag == 1) || (g_spu_busy == 1)) {
        return 1;
    }
    var_v0 = func_80078998(g_snd_irq_handle);
    if (arg0 == 1) {
        if (var_v0 == 0) {
            do {
                var_v0 = func_80078998(g_snd_irq_handle);
            } while (var_v0 == 0);
        }
        var_v0 = 1;
        goto block_8;
    }
    if (var_v0 == 1) {
block_8:
        g_spu_busy = var_v0;
    }
    return var_v0;
}
void func_8008AF58(s32 a0) {
    if (a0 == 1) {
        g_spu_busy = 0;
    } else {
        g_spu_busy = 1;
    }
}

s32 func_8008AF84(void) {
    return g_spu_busy != 1;
}

void func_8008AF9C(void *arg0) {
    (void)arg0;
}
void func_8008B400(u8 *a0) {
    s32 limit = 24;
    s32 i = 0;
    s32 one = 1;
    s32 three = 3;
    s32 two = 2;
    u8 *buf = a0;
    do {
        s32 off = i << 4;
        u16 data;
        s32 bit;
        data = *((u16 *)((off + g_spu_base_addr) + 0xC));
        bit = D_800A2874 & (one << i);
        if (bit) {
            if (data != 0) {
                *buf = one;
            } else {
                *buf = three;
            }
        } else if (data != 0) {
            *buf = two;
        } else {
            *buf = 0;
        }
        i++;
        buf++;
    } while (i < limit);
}
void saTan1MainJump(void *a0) {
    (void)a0;
}
/* kengo:MED  |  sa_tan1/saTan1MainJump  |  413i  |  -10 */
s32 func_8008BB24(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    volatile s32 sp0;
    volatile s32 sp4;
    volatile s32 sp8;
    s32 diff;
    s32 abs_diff;
    s32 quot;
    s32 rem;
    s32 base;

    arg0 = ((arg0 & 0xFFFF) << 7) + (arg1 & 0xFFFF);
    arg2 = ((arg2 & 0xFFFF) << 7) + (arg3 & 0xFFFF);
    diff = arg2 - arg0;
    abs_diff = diff;
    if (diff < 0) {
        abs_diff = -diff;
    }

    quot = abs_diff / 1536;
    rem = abs_diff - (quot * 1536);

    if (diff >= 0) {
        base = 0x1000 << quot;
    } else {
        if (rem != 0) {
            quot += 1;
            rem = 0x600 - rem;
        }
        base = 0x1000 >> quot;
    }

    {
        s32 atten = base & 0xFFFF;
        s32 mult_const = 0x103B;
        s32 a3_val = (u32)atten << 12;
        s32 counter = 0;
        s32 abs_rem;
        u32 upper;
        s32 frac;

        abs_rem = rem;
        if (rem < 0) {
            abs_rem = -abs_rem;
        }
        upper = (u32)abs_rem >> 5;
        frac = abs_rem & 0x1F;
        sp8 = atten * mult_const;

        if (upper != 0) {
            do {
                a3_val = atten * mult_const;
                mult_const = ((u32)mult_const * 0x103B) >> 12;
                sp8 = atten * mult_const;
                counter += 1;
            } while (counter < (s32)upper);
        }

        {
            u32 result = (u32)(a3_val + (((u32)(sp8 - a3_val) >> 5) * frac)) >> 12;
            if (result >= 0x4000U) {
                result = 0x3FFF;
            }
            return result & 0xFFFF;
        }
    }
}
s32 func_8008BC60(s32 arg0, s32 arg1, s32 arg2) {
    s32 bit_pos;
    s32 i;
    s32 v0;
    s32 v1;
    u32 t0;
    s32 old_prod;
    s32 t6;
    s32 t7;
    u32 step;
    u32 a3_acc;
    s32 t3_acc;
    s32 a0_inner;
    u32 a2_search;

    a2_search = ~arg2 & 0xFFFF;
    bit_pos = 0;
    i = 15;
    v0 = a2_search >> i;

    do {
        if ((v0 & 1) == 0) {
            bit_pos = i;
            goto found_bit;
        }
        i--;
        v0 = a2_search >> i;
    } while (i >= 0);

found_bit:
    t7 = bit_pos - 12;
    t6 = 1 << bit_pos;
    t0 = 0x1000;
    i = 0;
    a2_search = arg2 & 0xFFFF;

    do {
        old_prod = t6 * t0;
        t0 = (t0 * 4155) >> 12;
        a0_inner = 0;
        t3_acc = 0;
        step = (u32)(t6 * t0 - old_prod) >> 5;
        a3_acc = step;

        do {
            v0 = (u32)(old_prod + t3_acc) >> 12;
            if (a2_search < (u32)v0) {
                goto inner_inc;
            }
            v1 = (u32)(old_prod + a3_acc) >> 12;
            if (a2_search < (u32)v1) {
                v0 = (i << 5) + a0_inner;
                goto found;
            }
        inner_inc:
            a3_acc += step;
            a0_inner++;
            t3_acc += step;
        } while (a0_inner < 0x20);

        i++;
    } while (i < 0x30);

    v0 = 0x600;

found:
    v1 = v0;
    if (v0 < 0) {
        v1 = v0 + 0x7F;
    }
    v1 >>= 7;
    {
        s32 rem = v0 - (v1 << 7);
        v0 = (arg0 & 0xFFFF) + v1;
        v1 = t7 * 3;
        v1 <<= 2;
        v0 += v1;
        v1 = (arg1 & 0xFFFF) + rem;
        v0 <<= 8;
        return v0 | v1;
    }
}
void func_8008BD88(s32 arg0, u16 *arg1, u16 *arg2) {
    u16 temp_v1;
    u16 temp_a0_2;
    u32 temp_a3;
    u32 temp_v1_2;
    s32 temp;

    temp = (arg0 << 4) + g_spu_base_addr;
    temp_v1 = *(u16 *)(temp);
    temp_a0_2 = *(u16 *)(temp + 2);
    temp_a3 = temp_v1 & 0xFFFF;
    if (temp_a3 >= 0x4000U) {
        u32 sub = 0x8000;
        *arg1 = temp_a3 - sub;
    } else {
        *arg1 = temp_v1;
    }
    temp_v1_2 = temp_a0_2 & 0xFFFF;
    if (temp_v1_2 >= 0x4000U) {
        u32 sub = 0x8000;
        *arg2 = temp_v1_2 - sub;
        return;
    }
    *arg2 = temp_a0_2;
}

void func_8008BDE8(s32 a0, u16 *a1) {
    a0 = (a0 << 4) + g_spu_base_addr;
    *a1 = *(u16 *)(a0 + 0xC);
}

void func_8008BE04(void) {
    s32 v0;
    v0 = EnterCriticalSection();
    func_8008D050(&g_snd_callback);
    if (v0 == 1) {
        ExitCriticalSection();
    }
}
extern s32 g_str_sio;

void func_8008BE4C(void) {
    s32 v0;
    v0 = EnterCriticalSection();
    func_8008D060(&g_str_sio);
    func_80078FF0();
    if (v0 == 1) {
        ExitCriticalSection();
    }
}

void func_8008BE9C(void) {
}
extern s32 D_800F1AFC;
extern s32 D_800F1B00;
extern s32 D_800F1B04;
extern s32 D_800A3044;
s32 func_8008BEA4(int a0, int a1) {
    s32 *flag = &D_800F1AFC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1B04 = a1;
    if ((flag && flag) && flag) {
        do { } while (0);
    }
    D_800F1B00 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        {
            volatile u16 *ptr = (volatile u16 *)(reg + 0xA);
            *ptr |= 0x800;
            *ptr |= 0x20;
        }
    }
    return 0;
}
extern u16 D_800F1AE0;
extern volatile u16 D_800F1AE2;
extern u16 D_800F1AE6;
extern s32 (*D_800F1AE8)(s32, s32);
extern s16 D_800A3074[4];
extern void func_8008008C(s32, s32);

s32 cpu_side_move_dir_3(u8 *arg0, s32 arg1) {
    s32 r_arg1 = arg1;
    volatile s32 *flag = &D_800F1AFC;
    s32 s0;
    s32 s4;
    void *spu;
    s32 (*fn)(s32, s32);
    u16 saved_a;
    volatile u16 *p_ae0;
    s32 s2 = 0;

    if (*flag != 0) return -1;
    goto main_work;

cleanup_A:
    __asm__ ("la %0, D_800F1AE0" : "=r"(p_ae0));
    saved_a = *((volatile u16 *)(((s32)spu) + 0xA));
    *((volatile u16 *)(((s32)spu) + 0xA)) = 0x50;
    *((volatile u16 *)(((s32)spu) + 8)) = *p_ae0;
    *((volatile u16 *)(((s32)spu) + 0xE)) = D_800F1AE6;
    *((volatile u16 *)(((s32)spu) + 0xA)) |= 0x10;
    *((volatile u16 *)(((s32)spu) + 0xA)) = saved_a;
    *((volatile u16 *)(((s32)spu) + 0xA)) &= 0xFFDF;
    func_8008008C(0xF000000B, 0x8000);
    __asm__ __volatile__("# A" ::: "memory");
    goto return_val;

cleanup_B:
    spu = (void *)D_800A3044;
    *((volatile u16 *)(((s32)spu) + 0xA)) &= 0xFFDF;
    func_8008008C(0xF000000B, 0x100);
    __asm__ __volatile__("# B" ::: "memory");
    goto return_val;

main_work:
    {
        volatile u16 *p_ae2;
        u32 ae2_val;
        __asm__ ("la %0, D_800F1AE2" : "=r"(p_ae2));
        ae2_val = *p_ae2;
        s4 = *(s16 *)((s32)D_800A3074 + ((ae2_val & 0x300) >> 7));
    }
    flag[2] = r_arg1;
    flag[1] = (s32)arg0;
    *flag = 0;
    *((volatile u16 *)(((s32)D_800A3044) + 0xA)) |= 0x20;

    if (flag[2] == 0) goto final_cleanup;
    s0 = 0;
    {
        volatile s32 *loop_flag = flag;

loop_top:
    spu = (void *)D_800A3044;
    if ((*((volatile u16 *)(((s32)spu) + 4))) & 0x38) goto cleanup_A;
    if ((*((volatile u16 *)(((s32)spu) + 4))) & 2) goto copy_byte;

inner:
    fn = D_800F1AE8;
    if (fn != 0) {
        s32 prev = s2;
        s2 += 1;
        if (fn(1, prev) == 0) goto cleanup_B;
    }
    if (!((*((volatile u16 *)(((s32)D_800A3044) + 4))) & 2)) goto inner;

copy_byte:
    *((u8 *)D_800F1B00) = *((u8 *)D_800A3044);
    loop_flag[1] += 1;
    loop_flag[1];
    loop_flag[2] -= 1;
    loop_flag[2];
    s0 += 1;
    if (s0 == s4) {
        s0 = 0;
        *((volatile u16 *)(((s32)D_800A3044) + 0xA)) ^= 2;
    }
    if (loop_flag[2] != 0) goto loop_top;
    }

final_cleanup:
    *((volatile u16 *)(((s32)D_800A3044) + 0xA)) &= 0xFFDF;

return_val:
    {
        volatile s32 *p_b04 = &D_800F1B04;
        return r_arg1 - *p_b04;
    }
}

/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_3  |  160i  |  x4 size collision */
extern s32 D_800F1AEC;
extern s32 D_800F1AF0;
extern s32 D_800F1AF4;
extern s32 D_800F1AF8;
extern s32 D_800A3044;
s32 func_8008C184(int a0, int a1) {
    s32 *flag = &D_800F1AEC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1AF4 = a1;
    if ((flag && flag) && flag) {
        do { } while (0);
    }
    D_800F1AF0 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        D_800F1AF8 = *(u16 *)(reg + 4) & 0x80;
        *(volatile u16 *)(reg + 0xA) |= 0x400;
    }
    return 0;
}
s32 SetPacketData(u8 *arg0, s32 arg1) {
    register s32 r_arg1 asm("s4") = arg1;
    volatile s32 *flag = &D_800F1AEC;
    s32 s0;
    s32 s5;
    s32 s1;
    void *spu;
    s32 (*fn)(s32, s32);
    s32 wait_val;

    if (*flag != 0) return -1;
    s0 = 0;
    goto main_work;

cleanup_A:
    func_8008008C(0xF000000B, 0x100);
    goto return_val;

cleanup_B:
    func_8008008C(0xF000000B, 0x100);
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return (r_arg1 - *p_af4) - 1;
    }

main_work:
    {
        volatile u16 *p_ae2;
        u32 ae2_val;
        __asm__ ("la %0, D_800F1AE2" : "=r"(p_ae2));
        ae2_val = *p_ae2;
        s5 = *(s16 *)((s32)D_800A3074 + ((ae2_val & 0x300) >> 7));
    }
    D_800F1AF4 = r_arg1;
    D_800F1AF0 = (s32)arg0;
    s1 = 0;
    if (D_800F1AF4 == 0) goto return_val;

    {
        volatile s32 *loop_flag = flag;

outer_top:
        spu = (void *)D_800A3044;
        if ((*((volatile u16 *)(((s32)spu) + 4)) & 5) == 5) goto check_first;
        wait_val = 5;

inner_wait:
        fn = D_800F1AE8;
        if (fn != 0) {
            s32 prev = s0;
            s0 += 1;
            if (fn(2, prev) == 0) goto cleanup_A;
        }
        if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != wait_val) goto inner_wait;

check_first:
        if (s1 != 0) goto send_byte;
        D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;

send_byte:
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        loop_flag[1] += 1;
        loop_flag[1];
        s1 += 1;
        loop_flag[2] -= 1;
        loop_flag[2];
        if (s1 != s5) goto loop_continue;
        if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) != loop_flag[3]) goto loop_continue;
        s1 = 0;
        {
            volatile s32 *p_af8 = &D_800F1AF8;
inner_wait2:
            fn = D_800F1AE8;
            if (fn != 0) {
                s32 prev = s0;
                s0 += 1;
                if (fn(2, prev) == 0) goto cleanup_B;
            }
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8) goto inner_wait2;
        }
        s1 = 0;

loop_continue:
        if (loop_flag[2] != 0) goto outer_top;
    }

return_val:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return r_arg1 - *p_af4;
    }
}
/* kengo:MED  |  am_rmd/SetPacketData  |  159i */
INCLUDE_ASM("asm/funcs", func_8008C464);
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008D050\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x47\n"
    "    nop\n"
    "endlabel func_8008D050\n"
    ".set reorder\n"
    ".set at\n"
);

__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8008D060\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x48\n"
    "    nop\n"
    ".global g_data_start\n"
    ".type g_data_start, @function\n"
    "g_data_start:\n"
    "    .aent g_data_start\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    "    nop\n"
    ".global g_module_func_tbl\n"
    ".type g_module_func_tbl, @function\n"
    "g_module_func_tbl:\n"
    "    .aent g_module_func_tbl\n"
    "    .word 0x8001DCB0\n"
    "    .word 0x8001E878\n"
    "    .word 0x80033898\n"
    "    .word 0x80034708\n"
    "    .word 0x800397D4\n"
    "    .word 0x8003993C\n"
    "    .word 0x8003B9D0\n"
    "    .word 0x8003BCB4\n"
    "    .word 0x80035480\n"
    "    .word 0x80035828\n"
    "    .word 0x8003BE10\n"
    "    .word 0x8003BEA8\n"
    "    .word 0x8001EA04\n"
    "    .word 0x8001EA84\n"
    "    .word 0x80035430\n"
    "    .word 0x8003BFC4\n"
    "    .word 0x8001EEB4\n"
    "    .word 0x8001EFA0\n"
    "    .word 0x8003C040\n"
    "    .word 0x8003C2C0\n"
    "    .word 0x8003C42C\n"
    "    .word 0x8003C560\n"
    "    .word 0x8003B870\n"
    "    .word 0x8003B8E4\n"
    "    .word 0x8003C958\n"
    "    .word 0x8003C9A4\n"
    "    .word 0x80035DC8\n"
    "    .word 0x80035E38\n"
    "    .word 0x8003CE18\n"
    "    .word 0x8003CF84\n"
    "    .word 0x8003C714\n"
    "    .word 0x8003C8B4\n"
    "    .word 0x8003CCCC\n"
    "    .word 0x8003CD10\n"
    ".global g_module_type_tbl\n"
    ".type g_module_type_tbl, @function\n"
    "g_module_type_tbl:\n"
    "    .aent g_module_type_tbl\n"
    "    .word 0x0D0B0800\n"
    "    .word 0x15131110\n"
    "endlabel func_8008D060\n"
    ".set reorder\n"
    ".set at\n"
);
