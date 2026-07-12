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
extern s16 saTan2Main(u8 *, s16, s16, u32);
extern s32 coli_HitPauseKatana_2(s32, u32, s32, s32);

/* Externs for globals */
extern s16 D_800F66F8;
extern s16 g_memcard_busy;
extern s32 g_spu_busy;
extern s32 g_spu_base_addr;

extern void irq_AcknowledgeVblank(s32, s32);
extern s32 spu_TransferDirect(s32, s32);
/* PsyQ LIBSPU: _spu_transferCallback — Sony's own header types the SPU
   transfer callback as a volatile function pointer (sotn-decomp
   libspu_internal.h:39); volatile is original semantics, not coercion */
extern void (* volatile g_spu_init_flag)();
extern s32 D_800A2874;
extern s32 g_snd_reverb_flag;
extern s32 g_spu_reverb_mode;
extern u16 g_spu_xfer_addr;
extern s32 g_spu_addr_shift;
extern s32 EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void bios_AddDevice_B(s32 *);
extern s32 g_snd_callback;
extern s32 D_80106F28;
extern s32 bios_TestEvent(s32);
extern s32 func_80088740(s32);
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
extern volatile s32 D_800A289C; /* _spu_RQvoice — Ruling-4 grant (volatile_extern_allowlist.txt:44) */
extern volatile s32 D_800A28A0;
extern s16 D_800A28D2;
extern volatile s32 D_800A2CD4;
extern s32 g_spu_reverb_mode;
extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;
/* PsyQ libspu memList record (_spu_memList entries) */
typedef struct {
    u32 addr;
    u32 size;
} SpuMemRec;

extern s32 D_800A2D44[]; /* _spu_rev_startaddr */
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
/* Sony _spu_RQ: ONE u16[4] object (PsyQ 4.0 LIBSPU S_SK relocs: addends 0/2/4/6 —
 * key-on pending [0..1], key-off pending [2..3]); splat split it into two D_
 * symbols. Ruling-4 grant, volatile_extern_allowlist.txt:40-41. */
extern volatile u16 D_800F7420[10]; /* _spu_RQ; _spu_init clears all 10 (PsyQ 4.0 spu.c) */
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
extern void irq_SetAlarm(s32);
extern s32 irq_EnableInterrupts(s32, s32);
extern void func_80078BA8(s32);
extern void func_80078A68(s32, s32, s32);
extern s32 g_alarm_secondary_cb_ptr;
static void D_80083EDC(void); /* _SsTrapIntrVSync (ssstart.c static) */
static void D_80083F1C(void); /* _SsSeqCalledTbyT_1per2 (ssstart.c static) */

/* PsyQ 4.0 LIBSND ssstart: _SsStart + SndSeqTickEnv (_snd_seq_tick_env @
   D_800A26CC) — verbatim-linked Sony object (census 2026-07-09); C ref:
   sotn-decomp src/main/psxsdk/libsnd/ssstart.c (BB2's 4.0 rev uses 0x7F for
   the case-0 sentinel where SOTN's rev uses 0xFF) */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unk12;
    /* 0x10 */ u8 unk16;
    /* 0x11 */ u8 unk17;
    /* 0x12 */ u8 unk18;
    /* 0x13 */ u8 unk19;
} SndSeqTickEnv;
extern SndSeqTickEnv D_800A26CC;

void saTan5TakeAnim2_2(s32 arg0) {
    u16 rcnt_target;
    u32 rcnt_spec;

    s32 wait = 1000;
    while (--wait >= 0) {
    }

    D_800A26CC.unk16 = 0;
    D_800A26CC.unk18 = 6;
    D_800A26CC.unk17 = 0;
    D_800A26CC.unk12 = 0;
    rcnt_spec = 0xF2000002;
    rcnt_target = 0x44E8;
    switch (D_800A26CC.unk0) {
    case 0:
        D_800A26CC.unk18 = 0x7F;
        return;

    case 5:
        D_800A26CC.unk18 = 0;
        if (arg0 == 0) {
            D_800A26CC.unk16 = 1;
        } else {
            rcnt_spec = 0xF2000003;
            rcnt_target = 1;
        }
        break;

    case 3:
        rcnt_target = 0x89D0;
        break;

    case 2:
        break;

    default:
        if (D_800A26CC.unk4 == 0) {
            if (D_800A26CC.unk0 < 0x46) {
                rcnt_target = 0x204CC0 / D_800A26CC.unk0;
                D_800A26CC.unk17++;
            } else {
                rcnt_target = 0x409980 / D_800A26CC.unk0;
            }
        } else {
            return;
        }
        break;
    }

    if (D_800A26CC.unk16 != 0) {
        EnterCriticalSection();
        irq_SetAlarm(g_alarm_secondary_cb_ptr);
    } else {
        s32 de;
        s32 a1_val;
        EnterCriticalSection();
        func_80078BA8(rcnt_spec);
        func_80078A68(rcnt_spec, rcnt_target, 0x1000);
        de = D_800A26CC.unk18;
        if (de == 0) {
            s32 ret = irq_EnableInterrupts(0, 0);
            de = D_800A26CC.unk18;
            a1_val = (s32)&D_80083EDC;
            D_800A26CC.unk12 = ret;
        } else {
            a1_val = (s32)&D_80083F1C;
            if (D_800A26CC.unk17 == 0) {
                a1_val = g_alarm_secondary_cb_ptr;
            }
        }
        irq_EnableInterrupts(de, a1_val);
    }
    ExitCriticalSection();
}
/* kengo:MED  |  sa_tan5/saTan5TakeAnim2_2  |  154i  |  x2 size collision */
/* PsyQ 4.0 LIBSND ssstart: SsStart / SsStart2 / _SsTrapIntrVSync /
   _SsSeqCalledTbyT_1per2 + sscall: SsSeqCalledTbyT — verbatim-linked Sony
   objects (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libsnd/{ssstart.c,sscall.c}. Only SsStart (=DispStuff)
   has a glabel: SsStart2 + the tick trampolines are statics inside the
   splat extent; SsSeqCalledTbyT is address-referenced only by the
   SndSeqTickEnv .data initializer (raw .word @0x800A26D4). */
extern s32 D_800FF630;  /* _snd_ev_flag */
extern s16 D_801077A8;  /* _snd_seq_s_max */
extern s16 D_801077AA;  /* _snd_seq_t_max */
extern s32 D_801027E4;  /* _snd_openflag */
extern s32 D_80106F28;  /* _ss_score (per-SEP score-block pointer table) */
extern void action_CheckHitZangeki(void);   /* SpuVmFlush */
void spu_SetMotionCallback(s16 a0, s16 a1); /* _SsSndPlay */
void func_800841E0(s16 arg0, s16 arg1);     /* _SsSndCrescendo */
void func_80084500(s16 arg0, s16 arg1);     /* _SsSndDecrescendo */
void func_800856B0(s16 a0, s16 a1);         /* _SsSndTempo */
void spu_SetMotionState(s16 a0, s16 a1);    /* _SsSndPause */
void spu_SetMotionActive(s32 a0, s16 a1);   /* _SsSndReplay */
void func_80085270(s16 a0, s16 a1);         /* _SsSndStop */

void DispStuff(void) {
    saTan5TakeAnim2_2(1);
}
static void SsStart2(void) {
    saTan5TakeAnim2_2(0);
}
static void D_80083EDC(void) {
    if (D_800A26CC.unk12 != 0) {
        ((void (*)(void))D_800A26CC.unk12)();
    }
    ((void (*)(void))D_800A26CC.unk8)();
}
static void D_80083F1C(void) {
    /* The 1-per-2 tick toggle: a standalone word AFTER the declared
       SndSeqTickEnv block (which ends at +0x13) — its own splat symbol
       (dlabel D_800A26E0 in 7D920.data.s; named_syms.txt:
       g_alarm_pending_priority_flag), the only C handle for this memory
       in the TU. */
    extern s32 D_800A26E0;
    if (D_800A26E0 == 0) {
        D_800A26E0 = 1;
    } else {
        D_800A26E0 = 0;
        ((void (*)(void))D_800A26CC.unk8)();
    }
}
#define SS_SCORE_FLAG(i, j) \
    (*(s32 *)(((s32 *)&D_80106F28)[i] + (j) * 0xB0 + 0x98))
static void SsSeqCalledTbyT(void) {
    int i;
    int j;
    if (D_800FF630 != 1) {
        D_800FF630 = 1;

        action_CheckHitZangeki();

        for (i = 0; i < D_801077A8; i++) {
            s32 bit = 1 << i;
            if (D_801027E4 & bit) {
                for (j = 0; j < D_801077AA; j++) {
                    if (SS_SCORE_FLAG(i, j) & 1) {
                        spu_SetMotionCallback(i, j);

                        if (SS_SCORE_FLAG(i, j) & 0x10) {
                            func_800841E0(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x20) {
                            func_80084500(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x40) {
                            func_800856B0(i, j);
                        }
                        if (SS_SCORE_FLAG(i, j) & 0x80) {
                            func_800856B0(i, j);
                        }
                    }
                    if (SS_SCORE_FLAG(i, j) & 2) {
                        spu_SetMotionState(i, j);
                    }
                    if (SS_SCORE_FLAG(i, j) & 8) {
                        spu_SetMotionActive((s16)i, j);
                    }
                    if (SS_SCORE_FLAG(i, j) & 4) {
                        func_80085270(i, j);
                        SS_SCORE_FLAG(i, j) = 0;
                    }
                }
            }
        }
        D_800FF630 = 0;
    }
}
/* kengo:LOW  |  su_menu_ending/_DispStuff  |  209i  |  PS2 UI — reverted */
void func_800841E0(s16 arg0, s16 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800841E0.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
    (void)arg1;
}
void func_80084500(s16 arg0, s16 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80084500.s).
     * Inline-asm scaffolding retired; pure-C decomp pending. */
    (void)arg0;
    (void)arg1;
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
extern void (*D_800F3344)(s16, s16, u8);
extern void (*D_800F3348)(s16, s16);
extern void (*D_800F334C)(s16, s16, u8);
extern void (*D_800F3350)(s16, s16, u8);
extern s32 spu_ReadMotionFrame(s32, s16);

s32 saTan0Main(s16 a0, s16 a1)
{
  /* 100% pure C: no regfix rules, no register pins, no inline asm.
   *
   * One non-obvious choice: the 2nd switch's 0x90 case reads the sequence
   * pointer into a *block-local* `cp` instead of the shared `cmd_ptr`.
   * Sharing one variable made GCC's global allocator place `cmd_ptr` in $a2
   * (reusing the handler arg register, since the temp dies just before arg
   * setup); splitting that one use shrinks cmd_ptr's live range / conflicts
   * so the allocator gives it $v1 (its default-order preference) across the
   * 1st-switch cases — exactly target. Retires the last pin. */
  u8 *state;
  s32 cmd;
  u8 *cmd_ptr;
  u8 *ptr;
  u32 data;
  u8 b;
  u8 prev;
  u8 databyte;
  u8 next;
  s32 ret;
  state = (u8 *) (((s32) ((void **) (&D_80106F28))[a0]) + (a1 * 0xB0));
  ptr = *((u8 **) state);
  *((u8 **) state) = ptr + 1;
  b = ptr[0];
  ret = 0;
  if (((*((s32 *) (((u8 *) (((s32) ((void **) (&D_80106F28))[a0]) + (a1 * 0xB0))) + 0x98))) & 0x401) == 0x401)
  {
    if (((s32) (ptr + 1)) == ((*((s32 *) (state + 0x10))) + 1))
    {
      ((void (*)(s16, s16, u8, u8 *)) func_80084A7C)(a0, a1, ((u8 *) (*((s32 *) (state + 0x10))))[1], ptr);
      return -1;
    }
  }
  if (b & 0x80)
  {
    state[0x17] = b & 0xF;
    cmd = b & 0xF0;
    switch (cmd)
    {
      case 0x90:
        state[0x16] = 0x90;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        b = cmd_ptr[0];
        data = b;
        *((u8 **) state) = cmd_ptr + 2;
        next = cmd_ptr[1];
        *((s32 *) (state + 0x90)) = spu_ReadMotionFrame(a0, a1);
        D_800F3340(a0, a1, data, next);
        goto end;

      case 0xB0:
        state[0x16] = 0xB0;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        D_800F3350(a0, a1, databyte);
        goto end;

      case 0xC0:
        state[0x16] = 0xC0;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        D_800F3344(a0, a1, databyte);
        goto end;

      case 0xE0:
        state[0x16] = 0xE0;
        *((u8 **) state) = (*((u8 **) state)) + 1;
        D_800F3348(a0, a1);
        goto end;

      case 0xF0:
        state[0x16] = 0xFF;
        cmd_ptr = *((u8 **) state);
        *((u8 **) state) = cmd_ptr + 1;
        databyte = cmd_ptr[0];
        if (databyte == 0x2F)
      {
        ret = 1;
        ((void (*)(s16, s16, u8, u8 *)) func_80084A7C)(a0, a1, 0x2F, ptr);
        goto end;
      }
        D_800F334C(a0, a1, databyte);
        goto end;

      default:
        goto end;

    }

  }
  else
  {
    prev = state[0x16];
    switch (prev)
    {
      case 0x90:
      {
        u8 *cp = *((u8 **) state);
        *((u8 **) state) = cp + 1;
        next = cp[0];
        *((s32 *) (state + 0x90)) = spu_ReadMotionFrame(a0, a1);
        D_800F3340(a0, a1, b, next);
        goto end;
      }

      case 0xB0:
        D_800F3350(a0, a1, b);
        goto end;

      case 0xC0:
        D_800F3344(a0, a1, b);
        goto end;

      case 0xE0:
        D_800F3348(a0, a1);
        goto end;

      case 0xFF:
        next = b;
        if (next == 0x2F)
      {
        ret = 1;
        ((void (*)(s16, s16, u8, u8 *)) func_80084A7C)(a0, a1, 0x2F, ptr);
        goto end;
      }
        D_800F334C(a0, a1, b);

      default:
        goto end;

    }

  }
  end:
  return ret;

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
/* PsyQ LIBSND next.c: _SsSndNextSep — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/next.c (mixed
   score-pointer / full-index accesses are the original's spelling) */
void spu_ResetMotionEntry(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
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
/* PsyQ 4.0 LIBSND replay: _SsSndReplay — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/replay.c */
void spu_SetMotionActive(s32 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 base = *base_ptr;
    u8 *entry = (u8 *)(base + (s16)a1 * 0xB0);
    entry[0x14] = 1;
    *(s32 *)((u8 *)*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~8;
}
/* PsyQ 4.0 LIBSND SSSTOP: _SsSndStop — verbatim-linked Sony object (LIBSND
   hunt 2026-07-10: bit-verbatim vs the Jun-06-1997 4.0 build, 118 words);
   C ref: sotn-decomp src/main/psxsdk/libsnd/stop.c (interim 4.0 build adds
   the ~0x400 flag clear + NotifyChannel/ResetCounter pair). */
void func_80085270(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *p = (u8*)(*base_ptr + (s16)a1 * 0xB0);
    s32 i;
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

    do {
        ip = p + i;
        ip[0x37] = (u8)i;
        ip[0x27] = 0x40;
        *(s16 *)(p + i * 2 + 0x60) = 0x7F;
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
/* PsyQ LIBSND ssvol: SsSetSerialVol — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/scssvol.c.
   SpuCommonAttr per PsyQ libspu.h (sizeof = 0x28 — matches the frame). */
typedef struct {
    s16 left, right;
} SpuVolume;
typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ SpuVolume mvol;
    /* 0x08 */ SpuVolume mvolmode;
    /* 0x0C */ SpuVolume mvolx;
    struct {
        /* 0x10 */ SpuVolume volume;
        /* 0x14 */ s32 reverb;
        /* 0x18 */ s32 mix;
    } cd;
    struct {
        /* 0x1C */ SpuVolume volume;
        /* 0x20 */ s32 reverb;
        /* 0x24 */ s32 mix;
    } ext;
} SpuCommonAttr;

void func_80085448(s16 s_num, s16 voll, s16 volr) {
    SpuCommonAttr attr;
    if ((u8)s_num == 0) {
        attr.mask = 0xC0;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.cd.volume.left = voll * 258;
        attr.cd.volume.right = volr * 258;
    }
    if ((u8)s_num == 1) {
        attr.mask = 0xC00;
        if (voll >= 0x80) {
            voll = 0x7F;
        }
        if (volr >= 0x80) {
            volr = 0x7F;
        }
        attr.ext.volume.left = voll * 258;
        attr.ext.volume.right = volr * 258;
    }
    func_8008AF9C((s32 *)&attr);
}
extern s32 D_80104E80;
void SetBloodSpot(s32 arg) {
    s32 mode;
    s32 v;

    mode = sys_GetVideoMode();

    if (arg & 0x1000) {
        D_800A26CC.unk4 = 1;
        D_800A26CC.unk0 = arg & 0xFFF;
    } else {
        D_800A26CC.unk4 = 0;
        D_800A26CC.unk0 = arg;
    }

    v = D_800A26CC.unk0;
    if (v >= 6) goto big_v;
    switch (v) {
    case 4: {
        s32 t = 50;
        D_80104E80 = t;
        if (mode == 1) D_800A26CC.unk0 = 5;
        else D_800A26CC.unk0 = t;
        break;
    }
    case 1: {
        s32 t = 60;
        D_80104E80 = t;
        if (mode == 0) D_800A26CC.unk0 = 5;
        else D_800A26CC.unk0 = t;
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
/* PsyQ 4.0 LIBSND TEMPO: _SsSndTempo — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/tempo.c (interim
   4.0 build adds the counter<0 early clear-and-return). */
void func_800856B0(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *tbl = (s32 *)((u8 *)addr + (shifted >> 14));
    u8 *p = (u8 *)(*tbl + (s16)a1 * 0xB0);

    *(s32 *)(p + 0xA8) = *(s32 *)(p + 0xA8) - 1;
    if (*(s32 *)(p + 0xA8) < 0) {
        *(s32 *)((u8 *)(*tbl + (s16)a1 * 0xB0) + 0x98) &= ~0x40;
        *(s32 *)((u8 *)(*tbl + (s16)a1 * 0xB0) + 0x98) &= ~0x80;
        return;
    }

    if (*(s16 *)(p + 0x4E) > 0) {
        u32 new_val;
        if ((*(s32 *)(p + 0xA8) % *(s16 *)(p + 0x4E)) != 0) {
            return;
        }
        if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
            new_val = *(u32 *)(p + 0x94) - 1;
            goto tempo_store;
        }
        if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
            new_val = *(u32 *)(p + 0x94) + 1;
        tempo_store:
            *(u32 *)(p + 0x94) = new_val;
        }
    } else {
        if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
            *(u32 *)(p + 0x94) = *(u32 *)(p + 0x94) + *(s16 *)(p + 0x4E);
            if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
                *(u32 *)(p + 0x94) = *(u32 *)(p + 0xAC);
            }
        } else if (*(u32 *)(p + 0x94) < *(u32 *)(p + 0xAC)) {
            *(u32 *)(p + 0x94) = *(u32 *)(p + 0x94) - *(s16 *)(p + 0x4E);
            if (*(u32 *)(p + 0x94) > *(u32 *)(p + 0xAC)) {
                *(u32 *)(p + 0x94) = *(u32 *)(p + 0xAC);
            }
        }
    }

    *(s16 *)(p + 0x54) =
        (*(s16 *)(p + 0x50) * *(u32 *)(p + 0x94) * 10) / (u32)(D_80104E80 * 60);
    if (*(s16 *)(p + 0x54) <= 0) {
        *(s16 *)(p + 0x54) = 1;
    }
    if ((*(s32 *)(p + 0xA8) == 0) || (*(u32 *)(p + 0x94) == *(u32 *)(p + 0xAC))) {
        s32 shifted2 = a0 << 16;
        s32 *addr2 = (s32 *)&D_80106F28;
        s32 *tbl2 = (s32 *)((u8 *)addr2 + (shifted2 >> 14));
        *(s32 *)((u8 *)(*tbl2 + (s16)a1 * 0xB0) + 0x98) &= ~0x40;
        *(s32 *)((u8 *)(*tbl2 + (s16)a1 * 0xB0) + 0x98) &= ~0x80;
    }
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
void AllocBukiRmd(void) {
}
/* kengo:MED  |  am_rmd/AllocBukiRmd  |  259i */
extern s32 D_800F5750;
extern s16 D_800F5758;
extern s16 D_800F575A;
extern s32 func_80089F3C();
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
  register volatile unsigned int ra0 asm("$4");
  register s32 ra1 asm("$5");
  register s32 ra3 asm("$7");
  if (((u32) (a0 & 0xFFFF)) < 0x18)
  {
    ra3 = a1;
    v1 = (a0 << 16) >> 16;
    ra1 = v1 << 4;
    *((s16 *) (((u8 *) (&D_80102A7A)) + ra1)) = (s16) a2;
    asm volatile("" : : : "memory");
    ra0 = *(((u8 *) (&D_800F65E0)) + v1);
    v0 = 0;
    *((s16 *) (((u8 *) (&D_80102A78)) + ra1)) = ra3;
    ra0 |= 3;
    *(((u8 *) (&D_800F65E0)) + v1) = ra0;
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
/* PsyQ VagAtr (libsnd) — _svm_tn tone-attribute table pointer */
typedef struct {
    u8 prior, mode, vol, pan, center, shift, min, max;
    u8 vibW, vibT, porW, porT, pbmin, pbmax, reserved1, reserved2;
    u16 adsr1, adsr2;
    s16 prog, vag;
    s16 reserved[4];
} VagAtr;
extern VagAtr *D_80101BC8; /* _svm_tn */
extern u8 D_801027F7;
extern u8 D_801027FC;
extern u16 D_80102808;
extern s16 D_8010280C;
extern s16 D_8010280E;
extern s16 D_80102A7E;
extern s16 D_80102A80;
extern s16 D_80102A82;
extern s32 D_80107898[];
void tslGlobalMemFree_800861BC(void)
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
  new_var = (s32)D_80101BC8;
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A80)) + (*pc * 2))) = *((u16 *) (base + 0x10));
  base = (u8 *) ((((D_801027F7 << 4) + D_801027FC) << 5) + new_var);
  *((s16 *) (((u8 *) (&D_80102A82)) + (*pc * 2))) = (*((u16 *) (base + 0x12))) + D_800F66F8;
  *(((u8 *) (&D_800F65E0)) + D_8010280A) |= 0x30;
}

void spu_ResetCounter(void) {
    D_800F66F8 = 0;
}

void action_CheckHitZangeki(void) {
}
/* kengo:HIGH  |  is_action/action_CheckHitZangeki  |  271i */
void md_game_end(s32 arg0) {
}
/* kengo:HIGH  |  md_game/md_game_end  |  249i */
extern u8 D_801027F7;
extern u8 D_801027FC;
extern u16 D_800A26E4[];

/* PsyQ 4.0 LIBSND vmanager (VM_N2P): note2pitch2 — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libsnd/vmanager.c */
s32 func_80086BFC(u16 arg0, u16 arg1) {
    s16 octave;
    s16 var_a2;
    s16 var_a3;
    short new_var;
    u16 var_v1;
    s32 pos;
    s32 tone;

    tone = D_801027FC + (D_801027F7 * 0x10);
    var_a3 = (arg1 + D_80101BC8[tone].shift) / 8;
    var_a2 = 0;
    if (var_a3 >= 16) {
        var_a2 = 1;
        var_a3 -= 16;
    }
    new_var = arg0 + 60 - D_80101BC8[tone].center + var_a2;
    octave = new_var / 12;
    pos = (new_var % 12) * 16;
    var_v1 = D_800A26E4[pos + var_a3];

    octave -= 5;
    if (octave > 0) {
        var_v1 <<= octave;
    } else if (octave < 0) {
        var_v1 >>= -octave;
    }
    return var_v1;
}
void func_80086CF8(s32 arg0) {
}
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
void func_80087770(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
}
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
    D_80101BC8 = (VagAtr *)v2;
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

s16 tslCDFileRead(s16 a0) {
    if ((u16)a0 < 0x11) {
        if (g_snd_ch_status[a0] == 2) {
            ReturnVSMode(0);
            g_snd_ch_status[a0] = 1;
            return a0;
        }
    }
    return -1;
}

s16 func_80088058(s32 a0, s16 a1) {
    return saTan2Main((u8 *)a0, a1, 0, 0);
}

s16 coli_CheckBukiPreHit_80088088(s32 a0, s16 a1, s32 a2) {
    return saTan2Main((u8 *)a0, a1, 1, (u32)a2);
}

s16 coli_CheckBukiPreHit_800880B8(s32 a0, s16 a1, s32 a2) {
    return saTan2Main((u8 *)a0, a1, 1, (u32)a2);
}
/* PsyQ VabHdr (libsnd) — VAB bank header */
typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u8 vs;
    u8 vspad;
    u8 mvol;
    u8 pan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;
typedef struct {
    u8 tones;
    u8 mvol;
    u8 prior;
    u8 mode;
    u8 mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1;
    u16 reserved2;
    u16 reserved3;
} ProgAtr;
extern u16 D_80107808;
extern s32 D_80107810[];
extern s32 D_801077C8[];
extern s32 coli_HitPauseKatana(s32);
/* PsyQ 4.0 LIBSND vs_vh: SsVabOpenHeadWithMode — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libsnd/vs_vh.c */
s16 saTan2Main(u8 *addr, s16 vabid, s16 arg2, u32 sbaddr) {
    int vagLens[256];
    s32 i;
    s32 var_s0;
    s16 vabId_2;
    u16 temp_v1;
    u16 *ptr_vag_off_table;
    u32 magic;
    u32 spuAllocMem;
    u8 num_vags;
    ProgAtr *pProgTable;
    u8 *var_a2;
    VabHdr *vab_hdr_2;
    u32 sum;
    vabId_2 = 0x10;
    if (func_8008AF84() == 1) {
        return -1;
    }
    ReturnVSMode(1);
    if (vabid >= 0x10) {
        ReturnVSMode(0);
        return -1;
    }
    if (vabid == -1) {
        for (i = 0; i < 16; i++) {
            if (D_80102A68[i] == 0) {
                D_80102A68[i] = 1;
                vabId_2 = i;
                D_80107808++;
                break;
            }
        }
    } else {
        var_a2 = D_80102A68;
        if (var_a2[vabid] == 0) {
            D_80102A68[vabid] = 1;
            vabId_2 = vabid;
            D_80107808++;
        }
    }
    if (vabId_2 >= 0x10) {
        ReturnVSMode(0);
        return -1;
    }
    var_a2 = addr;
    D_800F66B8[vabId_2] = (s32)var_a2;

    var_a2 = var_a2 + 0x20;
    vab_hdr_2 = (VabHdr *)addr;
    magic = vab_hdr_2->form;
    if ((magic >> 8) != ('V' << 0x10 | 'A' << 0x8 | 'B')) {
        D_80102A68[vabId_2] = 0;
        ReturnVSMode(0);
        D_80107808 -= 1;
        return -1;
    }
    if ((magic & 0xFF) == 'p') {
        if (vab_hdr_2->ver >= 5) {
            D_800FF634 = 0x80;
        } else {
            D_800FF634 = 0x40;
        }
    } else {
        D_800FF634 = 0x40;
    }
    if (vab_hdr_2->ps <= D_800FF634) {
        D_800F6660[vabId_2] = (s32)var_a2;
        pProgTable = (ProgAtr *)var_a2;
        var_a2 = var_a2 + (D_800FF634 * 0x10);
        var_s0 = 0;
        for (i = 0; i < D_800FF634; i++) {
            pProgTable[i].reserved1 = var_s0;
            if (pProgTable[i].tones != 0) {
                var_s0++;
            }
        }
        var_s0 = 0;
        D_800F6700[vabId_2] = (s32)var_a2;
        ptr_vag_off_table = (u16 *)(var_a2 + (vab_hdr_2->ps << 9));
        num_vags = vab_hdr_2->vs;
        for (i = 0; i < 256; i++) {
            if (num_vags >= i) {
                temp_v1 = *ptr_vag_off_table;
                if (vab_hdr_2->ver >= 5) {
                    vagLens[i] = temp_v1 * 8;
                } else {
                    vagLens[i] = temp_v1 * 4;
                }
                var_s0 += vagLens[i];
            }
            ptr_vag_off_table++;
        }
        if (arg2 == 0) {
            spuAllocMem = coli_HitPauseKatana(var_s0);
            if (spuAllocMem == -1) {
                D_80102A68[vabId_2] = 0;
                ReturnVSMode(0);
                D_80107808 -= 1;
                return -1;
            }
        } else {
            spuAllocMem = sbaddr;
        }
        sum = spuAllocMem + var_s0;
        if (sum > 0x80000U) {
        end:
            D_80102A68[vabId_2] = 0;

            ReturnVSMode(0);
            D_80107808 -= 1;
            return -1;
        }
        D_80107810[vabId_2] = spuAllocMem;
        var_s0 = 0;
        for (i = 0; i <= num_vags; i++) {
            var_s0 += vagLens[i];
            if (!(i & 1)) {
                pProgTable[i / 2].reserved2 = (spuAllocMem + var_s0) >> 3;
            } else {
                pProgTable[i / 2].reserved3 = (spuAllocMem + var_s0) >> 3;
            }
        }

        D_801077C8[vabId_2] = var_s0;
        D_80102A68[vabId_2] = 2;
    } else {
        goto end;
    }
    return vabId_2;
}
/* kengo:MED  |  sa_tan2/saTan2Main  |  247i */
extern u8 g_snd_ch_status[];
extern s32 g_snd_ch_addr[];
extern s32 D_801077C8[];
extern s32 func_8008AE24(s32);
extern s32 func_8008ADC4(s32, s32);
s16 func_800884C4(s32 a0, s16 a1) {
    if ((u16)a1 >= 0x11) {
        ReturnVSMode(0);
        return -1;
    }
    if (g_snd_ch_status[a1] != 2) {
        ReturnVSMode(0);
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
    D_800A2884 = D_800A2D44[0];
    spu_WriteReg(0xD1, D_800A2D44[0], 0);
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
        v0 = bios_OpenEvent((s32)0xF0000009, 0x20, 0x2000, 0);
        g_snd_irq_handle = v0;
        bios_EnableEvent(v0);
        ExitCriticalSection();
    }
}
/* SPU-module debug strings (rodata 0x800163D8..0x80016420). Defined HERE —
 * before func_80088740 (_spu_init), their first user — so they emit into
 * .rodata AHEAD of the compiler-emitted jump tables of func_8008AF9C
 * (SpuSetCommonAttr), matching the original Sony spu.c literal order. */
const char D_800163D8[16] = "SPU:T/O [%s]\n";
const char D_800163E8[16] = "wait (reset)";
const char D_800163F8[20] = "wait (wrdy H -> L)";
const char D_8001640C[20] = "wait (dmaf clear/W)";

/* PsyQ 4.0 LIBSPU spu.c: _spu_init — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_init) */
s32 func_80088740(s32 a0) {
    u32 i;
    s32 channel;

    *D_800A2CEC |= 0xB0000;

    D_800A2CF8 = 0;
    D_800A2CFC = 0;
    D_800A2CF4 = 0;
    *(volatile u16 *)(D_800A2CDC + 0x180) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x182) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = 0;
    spu_WriteReg16();

    *(volatile u16 *)(D_800A2CDC + 0x180) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x182) = 0;

    if (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF) {
        i = 0;
        do {
            if (++i > 0xF00) {
                debug_printf(&D_800163D8, &D_800163E8);
                break;
            }
        } while (*(volatile u16 *)(D_800A2CDC + 0x1AE) & 0x7FF);
    }

    channel = 0;
    D_800A2D00 = 2;
    D_800A2D04 = 3;
    D_800A2D08 = 8;
    D_800A2D0C = 7;
    *(volatile u16 *)(D_800A2CDC + 0x1AC) = 4;
    *(volatile u16 *)(D_800A2CDC + 0x184) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x186) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x18C) = 0xFFFF;
    *(volatile u16 *)(D_800A2CDC + 0x18E) = 0xFFFF;
    *(volatile u16 *)(D_800A2CDC + 0x198) = 0;
    *(volatile u16 *)(D_800A2CDC + 0x19A) = 0;
    for (channel = 0; channel < 10; channel++) {
        D_800F7420[channel] = 0;
    }

    if (a0 == 0) {
        s32 kon;
        s32 koff;
        volatile u16 *vp;

        D_800A2CF4 = 0x200;
        *(volatile u16 *)(D_800A2CDC + 0x190) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x192) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x194) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x196) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x1B0) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x1B2) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x1B4) = 0;
        *(volatile u16 *)(D_800A2CDC + 0x1B6) = 0;
        DispUpdateStatusMessage((s32)&D_800A2D1C, 0x10);

        vp = (volatile u16 *)D_800A2CDC;
        for (channel = 0; channel < 0x18; channel++) {
            vp[channel * 8 + 0] = 0;
            vp[channel * 8 + 1] = 0;
            vp[channel * 8 + 2] = 0x3FFF;
            vp[channel * 8 + 3] = 0x200;
            vp[channel * 8 + 4] = 0;
            vp[channel * 8 + 5] = 0;
        }

        kon = 0xFFFF;
        koff = 0xFF;
        *(volatile u16 *)(D_800A2CDC + 0x188) = kon;
        *(volatile u16 *)(D_800A2CDC + 0x18A) = koff;
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();

        *(volatile u16 *)(D_800A2CDC + 0x18C) = kon;
        *(volatile u16 *)(D_800A2CDC + 0x18E) = koff;
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
        spu_WriteReg16();
    }

    D_800A2D10 = 1;
    *(volatile u16 *)(D_800A2CDC + 0x1AA) = 0xC000;
    D_800A2D14 = 0;
    D_800A2D18 = 0;
    return 0;
}
void DispUpdateStatusMessage(s32 arg0, s32 arg1) {
}
/* PsyQ 4.0 LIBSPU spu.c: _spu_t — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/spu.c (_spu_t) */
typedef char *va_list;
#define va_start(ap, parmN) ((ap) = (va_list)(&(parmN) + 1))
#define va_arg(ap, T) ((ap) += sizeof(T), *(T *)((ap) - sizeof(T)))

s32 saTan0GaugeDraw(s32 mode, ...) {
    s32 var_a2;
    u32 i;
    va_list args;
    u32 count;
    u16 ck2;
    u16 cnt;
    u16 t;

    va_start(args, mode);

    switch (mode) {
    case 2:
        count = va_arg(args, u32);
        D_800A2CF4 = count >> D_800A2D04;
        *(volatile u16 *)(D_800A2CDC + 0x1A6) = D_800A2CF4;
        break;

    case 1:
        t = *(volatile u16 *)(D_800A2CDC + 0x1A6);
        i = 0;
        D_800A2D2C = 0;
        if ((t & 0xFFFF) != D_800A2CF4) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(D_800A2CDC + 0x1A6) != D_800A2CF4);
        }
        cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x20;
        *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        break;

    case 0:
        t = *(volatile u16 *)(D_800A2CDC + 0x1A6);
        i = 0;
        D_800A2D2C = 1;
        if ((t & 0xFFFF) != D_800A2CF4) {
            do {
                if (++i > 0xF00) {
                    return -2;
                }
            } while (*(volatile u16 *)(D_800A2CDC + 0x1A6) != D_800A2CF4);
        }
        cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
        cnt &= ~0x30;
        cnt |= 0x30;
        *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        break;

    case 3:
        if (D_800A2D2C == 1) {
            ck2 = 0x30;
        } else {
            ck2 = 0x20;
        }
        i = 0;
        while ((*(volatile u16 *)(D_800A2CDC + 0x1AA) & 0x30) != ck2) {
            if (++i > 0xF00) {
                return -2;
            }
        }
        if (D_800A2D2C == 1) {
            spu_ReadReg();
        } else {
            spu_ReadStatus();
        }
        count = va_arg(args, u32);
        D_800A2D30 = count;
        count = va_arg(args, u32);
        D_800A2D34 = (count / 64);
        D_800A2D34 += ((count % 64) ? 1 : 0);
        *D_800A2CE0 = D_800A2D30;
        *D_800A2CE4 = (D_800A2D34 << 16) | 0x10;
        if (D_800A2D2C == 1) {
            var_a2 = 0x1000200;
        } else {
            var_a2 = 0x1000201;
        }
        *D_800A2CE8 = var_a2;
        break;
    }
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
    if (arg2 == 0) {
        *(volatile u16 *)(arg0 * 2 + g_spu_base_addr) = arg1;
        return;
    }
    *(volatile u16 *)(arg0 * 2 + g_spu_base_addr) = arg1 >> g_spu_addr_shift;
}
s32 saTan1SyuryoDraw(s32 mode, s32 val) {
    s32 aligned;
    if (D_800A2D00 != 0) {
        u32 step = D_800A2D08;
        if ((u32)val % step != 0) {
            val += step;
            val &= ~D_800A2D0C;
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
/* PsyQ LIBSPU s_q.c: SpuQuit — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_q.c.
   g_spu_init_flag = _spu_transferCallback, g_spu_timer = _spu_IRQCallback
   (both volatile fn ptrs per Sony's header), g_snd_init_flag =
   _spu_isCalled. */
extern void (* volatile g_spu_timer)();

void func_800892F8(void) {
    if (g_snd_init_flag == 1) {
        g_snd_init_flag = 0;
        EnterCriticalSection();
        g_spu_init_flag = 0;
        g_spu_timer = 0;
        spu_SetCallback(0);
        bios_CloseEvent(g_snd_irq_handle);
        bios_DisableEvent(g_snd_irq_handle);
        ExitCriticalSection();
    }
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_DisableEvent\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xD\n"
    "    nop\n"
    "endlabel bios_DisableEvent\n"
    ".set reorder\n"
    ".set at\n"
);
extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;

extern s32 g_spu_voice_key_a;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;

/* PsyQ LIBSPU s_m_init.c: SpuInitMalloc — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/
   s_m_init.c */
s32 spu_IrqHandler(s32 num, s32 *top) {
    s32 size;

    if (num > 0) {
        size = 0x10000 << g_spu_addr_shift;
        top[0] = 0x40001010;
        g_spu_voice_key_c = (s32)top;
        g_spu_voice_key_b = 0;
        g_spu_voice_key_a = num;
        top[1] = size - 0x1010;
        return num;
    }
    return 0;
}
extern void exec_game(void);
/* PsyQ 4.0 LIBSPU s_m_m: SpuMalloc — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_m_m.c */
#define _spu_memList ((SpuMemRec *)g_spu_voice_key_c)
s32 coli_HitPauseKatana(s32 size) {
    s32 var_s2;
    s32 var_s3;
    s32 i;

    i = 0;
    var_s2 = -1;

    if (D_800A2880 == 0) {
        var_s3 = 0;
    } else {
        var_s3 = (0x10000 - D_800A2884) << D_800A2D04;
    }

    size += (size & ~D_800A2D0C) ? D_800A2D0C : 0;
    size >>= D_800A2D04;
    size <<= D_800A2D04;

    if (_spu_memList[0].addr & 0x40000000) {
        var_s2 = 0;
    } else {
        exec_game();

        for (; i < g_spu_voice_key_a; i++) {
            if (_spu_memList[i].addr & 0x40000000 ||
                (_spu_memList[i].addr & 0x80000000 &&
                 _spu_memList[i].size >= size)) {
                var_s2 = i;
                break;
            }
        }
    }

    if (var_s2 == -1)
        return -1;

    if (_spu_memList[var_s2].addr & 0x40000000) {
        if (var_s2 < g_spu_voice_key_a &&
            _spu_memList[var_s2].size - var_s3 >= size) {
            s32 next = var_s2 + 1;

            /* Sony s_m_m.c has this volatile re-read verbatim (SOTN psxsdk
               keeps it with a "why the volatile?" note) -- original semantics */
            _spu_memList[next].addr =
                (*(volatile u32 *)&_spu_memList[var_s2].addr & 0x0FFFFFFF) +
                    size |
                0x40000000;
            _spu_memList[next].size = _spu_memList[var_s2].size - size;

            g_spu_voice_key_b = next;
            _spu_memList[var_s2].size = size;
            _spu_memList[var_s2].addr &= 0x0FFFFFFF;

            exec_game();

            return _spu_memList[var_s2].addr;
        }
    } else {
        if (size < _spu_memList[var_s2].size &&
            g_spu_voice_key_b < g_spu_voice_key_a) {
            u32 _addr = _spu_memList[var_s2].addr + size;
            u32 _size = _spu_memList[var_s2].size - size;
            SpuMemRec *kb =
                (SpuMemRec *)((g_spu_voice_key_b << 3) + (s32)_spu_memList);
            u32 swapAddr = kb->addr;
            u32 swapSize = kb->size;

            kb->addr = _addr | 0x80000000;
            kb->size = _size;
            g_spu_voice_key_b++;
            kb[1].addr = swapAddr;
            kb[1].size = swapSize;
        }

        _spu_memList[var_s2].size = size;
        _spu_memList[var_s2].addr &= 0x0FFFFFFF;
        exec_game();

        return _spu_memList[var_s2].addr;
    }
    return -1;
}
/* kengo:HIGH  |  is_coli/coli_HitPauseKatana  |  178i  |  x2 size collision */
typedef struct Entry { s32 w0; s32 w1; } Entry;
extern s32 D_800A2D3C;
extern s32 D_800A2D40;
void exec_game(void) {
    s32 i;
    s32 j;
    Entry *base;
    Entry *cur;
    Entry *p;
    Entry *outer;
    Entry *inner;
    Entry *e;
    s32 ival;
    s32 ov0;
    s32 in1;
    s32 ou1;
    s32 v;
    s32 cnt;
    s32 icnt;

    /* Phase 1: merge each flagged entry with the next matching flagged one */
    if (D_800A2D3C >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        cur = base;
        do {
            if (cur->w0 & 0x80000000) {
                j = i + 1;
                p = base + j;
            p1_scan:
                if (p->w0 == 0x2FFFFFFF) {
                    p++;
                    j++;
                    goto p1_scan;
                }
                p = base + j;
                if (p->w0 & 0x80000000) {
                    if ((p->w0 & 0x0FFFFFFF) == ((cur->w0 & 0x0FFFFFFF) + cur->w1)) {
                        p->w0 = 0x2FFFFFFF;
                        cur->w1 = cur->w1 + p->w1;
                        goto p1_cont;
                    }
                }
            }
            cur++;
            i++;
        p1_cont:
            ;
        } while (D_800A2D3C >= i);
    }

    /* Phase 2: mark zero-value entries as sentinels */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        p = (Entry *)D_800A2D40;
        do {
            if (p->w1 == 0) {
                p->w0 = 0x2FFFFFFF;
            }
            i++;
            p++;
        } while (cnt >= i);
    }

    /* Phase 3: selection sort by (w0 & 0x0FFFFFFF), skipping flagged */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        outer = base;
        do {
            if (outer->w0 & 0x40000000) {
                goto p3_done;
            }
            j = i + 1;
            if (cnt >= j) {
                icnt = D_800A2D3C;
                inner = base + j;
                do {
                    ival = inner->w0;
                    if (ival & 0x40000000) {
                        goto p3_next;
                    }
                    ov0 = outer->w0;
                    if ((u32)(ival & 0x0FFFFFFF) < (u32)(ov0 & 0x0FFFFFFF)) {
                        outer->w0 = ival;
                        in1 = inner->w1;
                        ou1 = outer->w1;
                        outer->w1 = in1;
                        inner->w0 = ov0;
                        inner->w1 = ou1;
                    }
                    j++;
                    inner++;
                } while (icnt >= j);
            }
        p3_next:
            i++;
            outer++;
        } while (D_800A2D3C >= i);
    }
p3_done:
    ;

    /* Phase 4: compact - replace first sentinel with base[count], shrink */
    cnt = D_800A2D3C;
    if (cnt >= 0) {
        i = 0;
        base = (Entry *)D_800A2D40;
        p = base;
        do {
            if (p->w0 & 0x40000000) {
                goto p4_done;
            }
            if (p->w0 == 0x2FFFFFFF) {
                e = base + cnt;
                p->w0 = e->w0;
                D_800A2D3C = i;
                p->w1 = e->w1;
                goto p4_done;
            }
            i++;
            p++;
            cnt = D_800A2D3C;
        } while (cnt >= i);
    }
p4_done:
    ;

    /* Phase 5: finalize backward - re-flag entries, fold in counts */
    i = D_800A2D3C - 1;
    if (i >= 0) {
        base = (Entry *)D_800A2D40;
        p = base + i;
        do {
            v = p->w0;
            if (!(v & 0x80000000)) {
                goto p5_done;
            }
            cnt = D_800A2D3C;
            p->w0 = (v & 0x0FFFFFFF) | 0x40000000;
            D_800A2D3C = i;
            p->w1 = p->w1 + base[cnt].w1;
            i--;
            p--;
        } while (i >= 0);
    }
p5_done:
    ;
}
/* kengo:HIGH  |  md_game/exec_game  |  194i */
extern s32 g_spu_voice_key_a;
extern void exec_game(void);
/* PsyQ 4.0 LIBSPU s_m_f: SpuFree — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_m_f.c */
void spu_DmaTransfer(u32 arg0) {
    s32 i;

    for (i = 0; i < g_spu_voice_key_a; i++) {
        if (((SpuMemRec *)g_spu_voice_key_c)[i].addr & 0x40000000) {
            break;
        }
        if (((SpuMemRec *)g_spu_voice_key_c)[i].addr == arg0) {
            ((SpuMemRec *)g_spu_voice_key_c)[i].addr |= 0x80000000;
            break;
        }
    }
    exec_game();
}

void spu_WaitReady(s32 a0, s32 a1) {
    coli_HitPauseKatana_2(a0, a1, 0xCA, 0xCB);
}

s32 coli_HitPauseKatana_2(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
    s32 mode = arg0;
    u32 mask = arg1;
    volatile u16 *base;
    u32 t2;

    if (D_800A2CD4 & 1) {
        t2 = ((D_800F7298[arg3] & 0xFF) << 16) | D_800F7298[arg2];
    } else {
        base = (volatile u16 *)D_800A2CDC;
        t2 = ((base[arg3] & 0xFF) << 16) | base[arg2];
    }
    switch (mode) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7298[arg2] |= mask;
            D_800F7298[arg3] |= (mask >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] |= mask;
            ((volatile u16 *)D_800A2CDC)[arg3] |= (mask >> 16) & 0xFF;
        }
        t2 |= mask & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7298[arg2] &= ~mask;
            D_800F7298[arg3] &= ~((mask >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] &= ~mask;
            ((volatile u16 *)D_800A2CDC)[arg3] &= ~((mask >> 16) & 0xFF);
        }
        t2 &= ~(mask & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            D_800F7298[arg2] = mask;
            D_800F7298[arg3] = (mask >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] = mask;
            ((volatile u16 *)D_800A2CDC)[arg3] = (mask >> 16) & 0xFF;
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
/* PsyQ 4.0 LIBSPU s_sr: SpuSetReverb — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_sr.c */
s32 saEft03Start2(s32 on_off) {
    u16 cnt;
    switch (on_off) {
    case 0:
        cnt = *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA);
        D_800A287C = 0;
        cnt &= ~0x80;
        *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA) = cnt;
        break;

    case 1:
        if ((D_800A2880 != on_off) && func_80089EB0(D_800A2884)) {
            cnt = *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA);
            D_800A287C = 0;
            cnt &= ~0x80;
            *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA) = cnt;
        } else {
            cnt = *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA);
            D_800A287C = on_off;
            cnt |= 0x80;
            *(volatile u16 *)((u8 *)D_800A2CDC + 0x1AA) = cnt;
        }
        break;
    }

    return D_800A287C;
}
/* PsyQ 4.0 LIBSPU s_m_util: _SpuIsInAllocateArea / _SpuIsInAllocateArea_ —
   verbatim-linked Sony object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libspu/s_m_util.c (4.0 adds the NULL list guard) */
s32 func_80089E30(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)g_spu_voice_key_c;
    s32 i;

    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}

s32 func_80089EB0(u32 arg0) {
    SpuMemRec *list = (SpuMemRec *)g_spu_voice_key_c;
    s32 i;

    arg0 <<= g_spu_addr_shift;
    if (list == 0) {
        return 0;
    }
    for (i = 0;; i++) {
        if (list[i].addr & 0x80000000) {
            continue;
        }
        if (list[i].addr & 0x40000000) {
            break;
        }
        if (arg0 <= (list[i].addr & 0x0FFFFFFF)) {
            return 1;
        }
        if (arg0 < (list[i].addr & 0x0FFFFFFF) + list[i].size) {
            return 1;
        }
    }
    return 0;
}
/* PsyQ 4.0 LIBSPU s_srmp: SpuSetReverbModeParam — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_srmp.c.
   4.0 deltas vs the SOTN revision: the DELAYTIME/FEEDBACK gates are range
   compares (mode >= ECHO && mode <= DELAY) with no default-arm clears, and
   the depth/zero split threads off the var_s4 flag. Reverb preset table:
   D_800A2D94 (= Sony rev_param table, 10 entries x 0x44). */
typedef struct {
    /* 0x00 */ u32 flags;
    /* 0x04 */ u16 dAPF1, dAPF2;
    /* 0x08 */ u16 vIIR, vCOMB1, vCOMB2, vCOMB3, vCOMB4;
    /* 0x12 */ u16 vWALL, vAPF1, vAPF2;
    /* 0x18 */ u16 mLSAME, mRSAME, mLCOMB1, mRCOMB1, mLCOMB2, mRCOMB2;
    /* 0x24 */ u16 dLSAME, dRSAME;
    /* 0x28 */ u16 mLDIFF, mRDIFF, mLCOMB3, mRCOMB3, mLCOMB4, mRCOMB4;
    /* 0x34 */ u16 dLDIFF, dRDIFF;
    /* 0x38 */ u16 mLAPF1, mRAPF1, mLAPF2, mRAPF2;
    /* 0x40 */ u16 vLIN, vRIN;
} RevParamEntry;

typedef struct {
    /* 0x00 */ u32 mask;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuReverbAttr;

extern RevParamEntry D_800A2D94[]; /* rev_param preset table */

/* Sony _spu_rev_attr — ONE struct (sotn libspu_internal.h:87 struct
   SpuRevAttr), base 0x800A2888. Members == the split splat symbols
   D_800A288C (mode) / D_800A2890/92 (depth L/R) / D_800A2894 (delay) /
   D_800A2898 (feedback), which other already-matched functions in this TU
   still reference by their per-member names (same linked bytes). */
typedef struct {
    /* 0x00 */ u32 unk0;
    /* 0x04 */ s32 mode;
    /* 0x08 */ SpuVolume depth;
    /* 0x0C */ s32 delay;
    /* 0x10 */ s32 feedback;
} SpuRevAttr;
extern SpuRevAttr D_800A2888; /* _spu_rev_attr */

void func_8008A434(s32 *arg0);
s32 md_game_check_change_main_mode_katinuki(u32 rev_mode);

static inline void _memcpy(char *dst, char *src, u32 size) {
    while (size--) {
        *dst++ = *src++;
    }
}

s32 func_80089F3C(SpuReverbAttr *attr) {
    RevParamEntry entry;
    u32 var_s0;
    u16 cnt;

    s32 var_s7 = 0;
    s32 var_s4 = 0;
    s32 var_s6 = 0;
    s32 sp58 = 0;
    s32 var_fp = 0;

    u32 mask = attr->mask;
    s32 bSetAll = attr->mask == 0;

    entry.flags = 0;
    if (bSetAll || (mask & 0x1)) {
        var_s0 = attr->mode;
        if (attr->mode & 0x100) {
            var_s0 &= ~0x100;
            sp58 = 1;
        }
        if (var_s0 >= 0xA || func_80089EB0(D_800A2D44[var_s0])) {
            return -1;
        }
        var_s4 = 1;
        D_800A2888.mode = var_s0;
        D_800A2884 = D_800A2D44[D_800A2888.mode];
        _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                sizeof(RevParamEntry));
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
            D_800A2888.feedback = 0x7F;
            D_800A2888.delay = 0x7F;
            break;
        case 8: /* SPU_REV_MODE_DELAY */
            D_800A2888.feedback = 0;
            D_800A2888.delay = 0x7F;
            break;
        default:
            D_800A2888.feedback = 0;
            D_800A2888.delay = 0;
            break;
        }
    }
    if (bSetAll || (mask & 0x8)) {
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_s6 = 1;
            if (!var_s4) {
                _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                        sizeof(RevParamEntry));
                entry.flags = 0x0C011C00;
            }
            D_800A2888.delay = attr->delay;
            entry.mLSAME = ((D_800A2888.delay << 0xD) / 0x7F) - entry.dAPF1;
            entry.mRSAME = ((D_800A2888.delay << 0xC) / 0x7F) - entry.dAPF2;
            entry.mLCOMB1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mRCOMB1;
            entry.dLSAME = ((D_800A2888.delay << 0xC) / 0x7F) + entry.dRSAME;
            entry.mLAPF1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mLAPF2;
            entry.mRAPF1 = ((D_800A2888.delay << 0xC) / 0x7F) + entry.mRAPF2;
            break;
        default:
            break;
        }
    }
    if (bSetAll || (mask & 0x10)) {
        switch (D_800A2888.mode) {
        case 7: /* SPU_REV_MODE_ECHO */
        case 8: /* SPU_REV_MODE_DELAY */
            var_fp = 1;
            if (!var_s4) {
                if (!var_s6) {
                    _memcpy((char *)&entry, (char *)&D_800A2D94[D_800A2888.mode],
                            sizeof(RevParamEntry));
                    entry.flags = 0x80;
                } else {
                    entry.flags |= 0x80;
                }
            }
            D_800A2888.feedback = attr->feedback;
            entry.vWALL = (D_800A2888.feedback * 0x8100) / 0x7F;
            break;
        default:
            break;
        }
    }
    if (var_s4) {
        var_s7 = (*(volatile u16 *)(D_800A2CDC + 0x1AA) >> 7) & 1;
        if (var_s7) {
            cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~0x80;
            *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }
    if (!var_s4) {
        if (bSetAll || (mask & 0x2)) {
            *(u16 *)(D_800A2CDC + 0x184) = attr->depth.left;
            D_800A2888.depth.left = attr->depth.left;
        }
        if (bSetAll || (mask & 0x4)) {
            *(u16 *)(D_800A2CDC + 0x186) = attr->depth.right;
            D_800A2888.depth.right = attr->depth.right;
        }
    } else {
        *(u16 *)(D_800A2CDC + 0x184) = 0;
        *(u16 *)(D_800A2CDC + 0x186) = 0;
        D_800A2888.depth.left = 0;
        D_800A2888.depth.right = 0;
    }
    if (var_s4 || var_s6 || var_fp) {
        func_8008A434((s32 *)&entry);
    }
    if (sp58) {
        md_game_check_change_main_mode_katinuki(D_800A2888.mode);
    }
    if (var_s4) {
        spu_WriteReg(0xD1, D_800A2884, 0);
        if (var_s7) {
            cnt = *(volatile u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 0x80;
            *(volatile u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }
    return 0;
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

/* PsyQ 4.0 LIBSPU s_crwa: SpuClearReverbWorkArea — verbatim-linked Sony
   object (census 2026-07-09); C ref: sotn-decomp
   src/main/psxsdk/libspu/s_crwa.c */
s32 md_game_check_change_main_mode_katinuki(u32 rev_mode) {
    volatile s32 callback;
    s32 oldTransmode;
    s32 var_s2;
    s32 var_s3;
    s32 transmodeCleared;
    u32 var_s0;
    u32 var_s1;

    callback = 0;
    transmodeCleared = 0;
    if (rev_mode >= 10 || func_80089EB0(D_800A2D44[rev_mode])) {
        return -1;
    }
    if (rev_mode == 0) {
        var_s1 = 0x10 << D_800A2D04;
        var_s2 = 0xFFF0 << D_800A2D04;
    } else {
        var_s1 = (0x10000 - D_800A2D44[rev_mode]) << D_800A2D04;
        var_s2 = D_800A2D44[rev_mode] << D_800A2D04;
    }
    oldTransmode = D_800A2CF8;
    if (D_800A2CF8 == 1) {
        D_800A2CF8 = 0;
        transmodeCleared = 1;
    }
    var_s3 = 1;
    if (D_800A2D14 != 0) {
        callback = D_800A2D14;
        D_800A2D14 = 0;
    }
    while (var_s3 != 0) {
        var_s0 = var_s1;
        if (var_s1 > 0x400) {
            var_s0 = 0x400;
        } else {
            var_s3 = 0;
        }

        saTan0GaugeDraw(2, var_s2);
        saTan0GaugeDraw(1);
        saTan0GaugeDraw(3, &D_800A28D4, var_s0);
        CheckFadeEnd(D_800A2870);
        var_s1 -= 0x400;
        var_s2 += 0x400;
    }
    if (transmodeCleared != 0) {
        D_800A2CF8 = oldTransmode;
    }
    if (callback != 0) {
        D_800A2D14 = callback;
    }
    return 0;
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel CheckFadeEnd\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0xA\n"
    "    nop\n"
    "endlabel CheckFadeEnd\n"
    ".set reorder\n"
    ".set at\n"
);
/* PsyQ 4.0 LIBSPU s_sk: SpuSetKey — verbatim-linked Sony object (census
 * 2026-07-09); C ref: sotn-decomp src/psxsdk/libspu/s_sk.c shape + PsyQ 4.0
 * S_SK object relocs (_spu_RQ = one u16[4]). Volatile decls are Ruling-4
 * ground-truth-codegen grants (volatile_extern_allowlist.txt:40-44). */
typedef struct {
    u16 pad[196];
    volatile u16 key_on[2];  /* +0x188 SPU KEY-ON (MMIO via _spu_RXX) */
    volatile u16 key_off[2]; /* +0x18C SPU KEY-OFF */
} SpuRXX;

void func_8008AAD4(s32 on_off, u32 voice_bit) {
    u16 lo;
    u16 hi;
    u32 hi2;

    voice_bit &= 0xFFFFFF;
    lo = voice_bit;
    hi2 = voice_bit >> 16;
    hi = hi2;

    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7420[0] = lo;
            D_800F7420[1] = hi;
            D_800A28A0 |= 1;
            D_800A289C |= voice_bit;
            if (D_800F7420[2] & voice_bit) {
                D_800F7420[2] &= ~voice_bit;
            }
            if (D_800F7420[3] & hi2) {
                D_800F7420[3] &= ~hi2;
            }
        } else {
            u32 stat = D_800A2874 | voice_bit;
            ((SpuRXX *)D_800A2CDC)->key_on[0] = lo;
            ((SpuRXX *)D_800A2CDC)->key_on[1] = hi;
            D_800A2874 = stat;
        }
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7420[2] = lo;
            D_800F7420[3] = hi;
            D_800A28A0 |= 1;
            D_800A289C &= ~voice_bit;
            if (D_800F7420[0] & voice_bit) {
                D_800F7420[0] &= ~voice_bit;
            }
            if (D_800F7420[1] & hi2) {
                D_800F7420[1] &= ~hi2;
            }
        } else {
            ((SpuRXX *)D_800A2CDC)->key_off[0] = lo;
            ((SpuRXX *)D_800A2CDC)->key_off[1] = hi;
            D_800A2874 &= ~voice_bit;
        }
        break;
    }
}
s32 func_8008ACD0(s32 arg0) {
    s32 bit_found;
    s32 i;
    s32 one;
    s32 mask;
    s32 base;
    s32 flags;

    bit_found = -1;
    i = 0;
    one = 1;
    for (; i < 0x18; i++) {
        mask = one << i;
        if (arg0 & mask) {
            bit_found = i;
            break;
        }
    }

    if (bit_found != -1) goto work;
    return -1;
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
/* PsyQ LIBSPU s_stm.c: SpuSetTransferMode — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_stm.c */
s32 func_8008AE7C(s32 mode) {
    s32 transMode;

    switch (mode) {
        case 0:
            transMode = 0;
            break;
        case 1:
            transMode = 1;
            break;
        default:
            transMode = 0;
    }
    g_snd_reverb_flag = mode;
    g_spu_reverb_mode = transMode;
    return transMode;
}
s32 func_8008AEB0(s32 arg0) {
    s32 var_v0;

    if ((g_snd_reverb_flag == 1) || (g_spu_busy == 1)) {
        return 1;
    }
    var_v0 = bios_TestEvent(g_snd_irq_handle);
    if (arg0 == 1) {
        if (var_v0 == 0) {
            do {
                var_v0 = bios_TestEvent(g_snd_irq_handle);
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
void ReturnVSMode(s32 a0) {
    if (a0 == 1) {
        g_spu_busy = 0;
    } else {
        g_spu_busy = 1;
    }
}

s32 func_8008AF84(void) {
    return g_spu_busy != 1;
}

/* PsyQ 4.0 LIBSPU s_sca: SpuSetCommonAttr — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_sca.c
   (4.0 block order: mvol L/R, cd vol L/R, ext vol L/R, cd rev/mix,
   ext rev/mix). */
void func_8008AF9C(void *arg0) {
    SpuCommonAttr *attr = arg0;
    u16 mvol_mode_left;
    u16 mvol_mode_right;
    u16 vol_total_left;
    u16 vol_total_right;
    u32 mask;
    s32 bSetAll;
    u16 cnt;

    vol_total_left = 0;
    vol_total_right = 0;
    mask = attr->mask;
    bSetAll = attr->mask == 0;

    if (bSetAll || (mask & 0x1)) {
        if (bSetAll || (mask & 0x4)) {
            switch ((s16)attr->mvolmode.left) {
            case 1:
                mvol_mode_left = 0x8000;
                break;
            case 2:
                mvol_mode_left = 0x9000;
                break;
            case 3:
                mvol_mode_left = 0xA000;
                break;
            case 4:
                mvol_mode_left = 0xB000;
                break;
            case 5:
                mvol_mode_left = 0xC000;
                break;
            case 6:
                mvol_mode_left = 0xD000;
                break;
            case 7:
                mvol_mode_left = 0xE000;
                break;
            case 0:
                vol_total_left = attr->mvol.left;
                mvol_mode_left = 0;
                break;
            default:
                vol_total_left = attr->mvol.left;
                mvol_mode_left = 0;
                break;
            }
        } else {
            vol_total_left = attr->mvol.left;
            mvol_mode_left = 0;
        }

        if (mvol_mode_left != 0) {
            if ((s16)attr->mvol.left >= 0x80) {
                vol_total_left = 0x7F;
            } else if ((s16)attr->mvol.left < 0) {
                vol_total_left = 0;
            } else {
                vol_total_left = attr->mvol.left;
            }
        }
        vol_total_left &= 0x7FFF;
        *(u16 *)(D_800A2CDC + 0x180) = vol_total_left | mvol_mode_left;
    }

    if (bSetAll || (mask & 0x2)) {
        if (bSetAll || (mask & 0x8)) {
            switch ((s16)attr->mvolmode.right) {
            case 1:
                mvol_mode_right = 0x8000;
                break;
            case 2:
                mvol_mode_right = 0x9000;
                break;
            case 3:
                mvol_mode_right = 0xA000;
                break;
            case 4:
                mvol_mode_right = 0xB000;
                break;
            case 5:
                mvol_mode_right = 0xC000;
                break;
            case 6:
                mvol_mode_right = 0xD000;
                break;
            case 7:
                mvol_mode_right = 0xE000;
                break;
            case 0:
                vol_total_right = attr->mvol.right;
                mvol_mode_right = 0;
                break;
            default:
                vol_total_right = attr->mvol.right;
                mvol_mode_right = 0;
                break;
            }
        } else {
            vol_total_right = attr->mvol.right;
            mvol_mode_right = 0;
        }

        if (mvol_mode_right != 0) {
            if ((s16)attr->mvol.right >= 0x80) {
                vol_total_right = 0x7F;
            } else if ((s16)attr->mvol.right < 0) {
                vol_total_right = 0;
            } else {
                vol_total_right = attr->mvol.right;
            }
        }
        vol_total_right &= 0x7FFF;
        *(u16 *)(D_800A2CDC + 0x182) = vol_total_right | mvol_mode_right;
    }

    if (bSetAll || (mask & 0x40)) {
        *(u16 *)(D_800A2CDC + 0x1B0) = attr->cd.volume.left;
    }

    if (bSetAll || (mask & 0x80)) {
        *(u16 *)(D_800A2CDC + 0x1B2) = attr->cd.volume.right;
    }

    if (bSetAll || (mask & 0x400)) {
        *(u16 *)(D_800A2CDC + 0x1B4) = attr->ext.volume.left;
    }

    if (bSetAll || (mask & 0x800)) {
        *(u16 *)(D_800A2CDC + 0x1B6) = attr->ext.volume.right;
    }

    if (bSetAll || (mask & 0x100)) {
        if (attr->cd.reverb == 0) {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~4;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 4;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x200)) {
        if (attr->cd.mix == 0) {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~1;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 1;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x1000)) {
        if (attr->ext.reverb == 0) {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~8;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 8;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }

    if (bSetAll || (mask & 0x2000)) {
        if (attr->ext.mix == 0) {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt &= ~2;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        } else {
            cnt = *(u16 *)(D_800A2CDC + 0x1AA);
            cnt |= 2;
            *(u16 *)(D_800A2CDC + 0x1AA) = cnt;
        }
    }
}
/* PsyQ 4.0 LIBSPU sr_gaks: SpuRGetAllKeysStatus — verbatim-linked Sony object
   (census 2026-07-09; module SR_GAKS spans 0x8008B330..0x8008B488). This
   entry point is UNREFERENCED in BB2 (dead code pulled in by whole-object
   linking) — no glabel exists at 0x8008B330, so it shares func_8008AF9C's
   splat extent. C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c. */
static s32 SpuRGetAllKeysStatus(s32 min, s32 max, s8 *status) {
    s32 voice;
    u16 volumex;

    if (min < 0) {
        min = 0;
    }
    if (min >= 24) {
        return -3;
    }
    if (max >= 24) {
        max = 23;
    }
    if (max < 0 || max < min) {
        return -3;
    }

    max++;
    for (voice = min; voice < max; voice++) {
        s32 off = voice << 4;
        s32 bit;
        volumex = *(u16 *)((off + D_800A2CDC) + 0xC);
        bit = D_800A2874 & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else {
            if (volumex != 0) {
                status[voice] = 2;
            } else {
                status[voice] = 0;
            }
        }
    }

    return 0;
}
/* PsyQ LIBSPU sr_gaks.c: SpuGetAllKeysStatus — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/sr_gaks.c
   (SpuRGetAllKeysStatus inlined with min=0, max=NUM_SPU_CHANNELS) */
void func_8008B400(u8 *status) {
    s32 limit = 24;
    s32 voice = 0;

    do {
        s32 off = voice << 4;
        u16 volumex;
        s32 bit;
        volumex = *((u16 *)((off + g_spu_base_addr) + 0xC));
        bit = D_800A2874 & (1 << voice);
        if (bit) {
            if (volumex != 0) {
                status[voice] = 1;
            } else {
                status[voice] = 3;
            }
        } else if (volumex != 0) {
            status[voice] = 2;
        } else {
            status[voice] = 0;
        }
        voice++;
    } while (voice < limit);
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
    register s32 saved_arg0 asm("$24");
    register s32 search asm("$6");
    register s32 saved_arg2 asm("$4");
    s32 scan_bit;
    register s32 shift asm("$10");
    s32 scan_result;

    saved_arg0 = arg0;
    search = arg2;
    saved_arg2 = search;
    search = ~search;
    scan_bit = 0;
    shift = 0xF;
    search &= 0xFFFF;
    scan_result = search >> shift;

scan_loop:
    if ((scan_result & 1) == 0) {
        do {
            scan_bit = shift;
            goto have_bit;
        } while (0);
    }

    shift--;
    scan_result = search >> shift;
    if (shift >= 0) {
        goto scan_loop;
    }

have_bit:
    {
        register s32 bit asm("$3") = scan_bit;
        register s32 bit_offset asm("$15");
        register s32 scale asm("$14");
        register u32 curve asm("$8");
        register s32 outer asm("$10");
        register u32 target asm("$6");
        register s32 result asm("$2");

        bit_offset = bit - 12;
        result = 1;
        scale = result << bit;
        curve = 0x1000;
        outer = 0;
        target = saved_arg2 & 0xFFFF;

        for (;;) {
            register s32 old_prod asm("$12");
            register s32 next_prod asm("$3");
            register u32 step asm("$9");
            register u32 next asm("$7");
            register u32 acc asm("$11");
            register s32 inner asm("$4");
            register s32 base asm("$13");

            old_prod = scale * curve;
            result = curve << 6;
            result += curve;
            result <<= 4;
            result -= curve;
            result <<= 2;
            curve = result - curve;
            curve >>= 12;
            next_prod = scale * curve;
            inner = 0;
            base = outer << 5;
            acc = 0;
            result = next_prod - old_prod;
            step = (u32)result >> 5;
            next = step;

            do {
                result = old_prod + acc;
                next_prod = old_prod + next;
                next_prod = (u32)next_prod >> 12;
                result = (u32)result >> 12;

                if (target < (u32)result) {
                    goto inner_next;
                }
                result = target < (u32)next_prod;
                if (result != 0) {
                    result = base + inner;
                    goto found;
                }

            inner_next:
                next += step;
                inner++;
                acc += step;
            } while (inner < 0x20);

            outer++;
            if (outer >= 0x30) {
                result = 0x600;
                goto found;
            }
        }

    found:
        {
            register s32 quot asm("$3") = result;
            register s32 rem asm("$4");

            if (result < 0) {
                quot = result + 0x7F;
            }
            quot >>= 7;
            rem = result - (quot << 7);
            result = (saved_arg0 & 0xFFFF) + quot;
            quot = bit_offset * 3;
            quot <<= 2;
            result += quot;
            quot = (arg1 & 0xFFFF) + rem;
            result <<= 8;
            return result | quot;
        }
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
    bios_AddDevice_B(&g_snd_callback);
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
/* PsyQ LIBCOMB comb: SioAnsyncRead (static) — verbatim-linked Sony object
   (census 2026-07-09; ground truth tmp/libscan/psyq40/LIBCOMB.LIB). The
   SIO async state (D_800F1B00/04 buf/len) is mutated at interrupt time by
   HandleSio (static @0x8008C9F4) — volatile is original semantics
   (operator-audited grant 2026-07-10, volatile_extern_allowlist.txt). */
extern s32 D_800F1AFC;
extern volatile s32 D_800F1B00;
extern volatile s32 D_800F1B04;
extern s32 D_800A3044;
s32 func_8008BEA4(int a0, int a1) {
    s32 *flag = &D_800F1AFC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1B04 = a1;
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
extern void bios_DeliverEvent(s32, s32);

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
    bios_DeliverEvent(0xF000000B, 0x8000);
    __asm__ __volatile__("# A" ::: "memory");
    goto return_val;

cleanup_B:
    spu = (void *)D_800A3044;
    *((volatile u16 *)(((s32)spu) + 0xA)) &= 0xFFDF;
    bios_DeliverEvent(0xF000000B, 0x100);
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
/* PsyQ LIBCOMB comb: SioAnsyncWrite (static) — verbatim-linked Sony object.
   D_800F1AF0/AF4/AF8 are mutated at interrupt time by HandleSio — volatile
   is original semantics (operator-audited grant 2026-07-10). */
extern s32 D_800F1AEC;
extern volatile s32 D_800F1AF0;
extern volatile s32 D_800F1AF4;
extern volatile s32 D_800F1AF8;
extern s32 D_800A3044;
s32 func_8008C184(int a0, int a1) {
    s32 *flag = &D_800F1AEC;
    if (*flag != 0) {
        return -1;
    }
    D_800F1AF4 = a1;
    D_800F1AF0 = a0;
    {
        s32 reg = D_800A3044;
        *flag = 1;
        D_800F1AF8 = *(volatile u16 *)(reg + 4) & 0x80;
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
    bios_DeliverEvent(0xF000000B, 0x100);
    goto return_val;

cleanup_B:
    bios_DeliverEvent(0xF000000B, 0x100);
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
    if (*(volatile s32 *)&D_800F1AF4 == 0) goto return_val;

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
        if (*(volatile s32 *)&D_800F1AF4 != 0) goto outer_top;
    }

return_val:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return r_arg1 - *p_af4;
    }
}
/* kengo:MED  |  am_rmd/SetPacketData  |  159i */
s32 func_8008C464(s32 arg0, s32 arg1, s32 arg2) {
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel bios_AddDevice_B\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x47\n"
    "    nop\n"
    "endlabel bios_AddDevice_B\n"
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

/* Rodata moved from asm/data/101C.rodata_main_post.s (rodata-cleanup project,
 * docs/rodata-cleanup-project.md, 2026-06-09). All 10 symbols are owned by
 * main.c functions (4 debug-printf strings + 4 switch jtbls + 2 SIO debug
 * strings); per the inventory CSV every owner has file=main. Placed at
 * end-of-file so the existing 24 bytes of main.o(.rodata) (from earlier
 * inline-asm rodata) stay at the original offsets (0..24 = 0x800163C0..
 * 0x800163D8) and the new declarations land at the retired block's slot
 * (0x800163D8..0x800164AC). Jtbl entries use literal addresses because the
 * referenced `.L<n>` labels live inside the asmfile-bridged stub function
 * bodies (saTan1MainJump, func_8008AF9C) and aren't visible to C source —
 * the linker produces identical bytes for either form. */
/* jtbl_80016420/jtbl_80016440 (the two SpuSetCommonAttr switch tables) are
 * now COMPILER-EMITTED by func_8008AF9C's switches (they emit into .rodata
 * at that function's file position, right after the four SPU debug strings
 * below, which moved up beside their owner functions for the same reason —
 * see the comment above D_800163D8). */
const u32 jtbl_80016460[8] = {
    0x8008B5CC, 0x8008B5D4, 0x8008B5DC, 0x8008B5E4,
    0x8008B5EC, 0x8008B5F4, 0x8008B5FC, 0x00000000,
};
const u32 jtbl_80016480[7] = {
    0x8008B6AC, 0x8008B6B4, 0x8008B6BC, 0x8008B6C4,
    0x8008B6CC, 0x8008B6D4, 0x8008B6DC,
};
const char D_8001649C[12] = "SIO console";
const char D_800164A8[4] = "sio";
