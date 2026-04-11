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
extern void coli_HitPauseKatana_2(s32, s32, s32, s32);

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
extern volatile s32 D_800A2D14;
extern s32 D_800A2CDC;
extern s32 spu_TransferData(s32, s32);

/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */

extern void func_8008AF9C(void *);
void func_80083BE4(s16 a0, s16 a1) {
    s32 buf[10];
    buf[0] = 3;
    *(s16 *)&buf[1] = (s16)(a0 * 129);
    *((s16 *)&buf[1] + 1) = (s16)(a1 * 129);
    func_8008AF9C(buf);
}
INCLUDE_ASM("asm/funcs", saTan5TakeAnim2_2);
/* kengo:MED  |  sa_tan5/saTan5TakeAnim2_2  |  154i  |  x2 size collision */
INCLUDE_ASM("asm/funcs", DispStuff);
/* kengo:LOW  |  su_menu_ending/_DispStuff  |  209i  |  PS2 UI — reverted */
INCLUDE_ASM("asm/funcs", func_800841E0);
INCLUDE_ASM("asm/funcs", func_80084500);
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
INCLUDE_ASM("asm/funcs", func_80084A7C);
INCLUDE_ASM("asm/funcs", saTan0Main);
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
INCLUDE_ASM("asm/funcs", func_80085270);
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
INCLUDE_ASM("asm/funcs", SetBloodSpot);
/* kengo:MED  |  am_rmd/SetBloodSpot  |  91i */
INCLUDE_ASM("asm/funcs", func_800856B0);
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

INCLUDE_ASM("asm/funcs", func_80085FB8);
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
INCLUDE_ASM("asm/funcs", func_800861BC);

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
INCLUDE_ASM("asm/funcs", func_800872A4);
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
INCLUDE_ASM("asm/funcs", AddTbpOfst);
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
INCLUDE_ASM("asm/funcs", func_80088740);
INCLUDE_ASM("asm/funcs", DispUpdateStatusMessage);
/* kengo:LOW  |  su_menu_home/_DispUpdateStatusMessage  |  206i  |  PS2 UI — reverted */
INCLUDE_ASM("asm/funcs", saTan0GaugeDraw);
/* kengo:MED  |  sa_tan0/saTan0GaugeDraw  |  164i */
extern void saTan0GaugeDraw(s32, ...);
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
INCLUDE_ASM("asm/funcs", saTan1SyuryoDraw);
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
INCLUDE_ASM("asm/funcs", coli_HitPauseKatana);
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

INCLUDE_ASM("asm/funcs", coli_HitPauseKatana_2);
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
INCLUDE_ASM("asm/funcs", func_80089F3C);
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
INCLUDE_ASM("asm/funcs", func_8008AAD4);
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

INCLUDE_ASM("asm/funcs", func_8008AF9C);
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
INCLUDE_ASM("asm/funcs", saTan1MainJump);
/* kengo:MED  |  sa_tan1/saTan1MainJump  |  413i  |  -10 */
INCLUDE_ASM("asm/funcs", func_8008BB24);
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
INCLUDE_ASM("asm/funcs", cpu_side_move_dir_3);
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
INCLUDE_ASM("asm/funcs", SetPacketData);
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
