/* REJECTED s4 (2026-07-18): permuter scorer FALSE ZERO — swapped jump targets.
 * perm_a output-0-1, scorer=0 with --stack-diffs, 112k-iter honest-0xD chassis.
 * Raw word diff vs target = 6 real words: the ==3 arm's beq+j target the lbu
 * block (0x164) instead of sel_dispatch (0x26c), and case-9/11's j targets
 * 0x26c instead — i.e. the two jump destinations are SWAPPED vs target
 * (plus 3 jtbl-reloc noise lws shared by every compile incl. committed).
 * Mechanism: load_sel2 (sel2 = D_800A3350) moved from case 13/17 into the
 * ==3 arm; at jump2 the arm suffix is [set13; lbu; j], the [lbu; j] tail
 * cross-jumps into the shared lbu block -> arm's jump redirected to a
 * DIFFERENT label than case-9/11's -> 13-pair unmerged (402 insns).
 * REFUTED twice over: (a) bytes wrong (jump-target words differ);
 * (b) semantics wrong (arm path dispatches sel2=lbu, target dispatches -1;
 * case-13/17 path loses its sel2 load). First measured instance of the
 * "two different JUMP_LABELs" unmerge — reachable only by rerouting through
 * a real-code label whose code is semantically forbidden on the 13-paths.
 * ALSO PROVES: the permuter scorer is metric-BLIND on this wall (label
 * normalization cannot see swapped branch targets; score 0 != bytes 0).
 */

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern u8 D_800A31FC;
extern u8 D_800A38CC;
extern s32 D_80102794;
extern s32 D_800A31F8;
extern s32 func_80038734(void);
s32 motion_SetMotion(void)
{
  extern u8 D_800A3207;
  extern u8 D_800A334C;
  extern u8 D_800A3350;
  extern u8 D_800A3354;
  extern u8 D_800A3206;
  extern u8 D_800A3340;
  extern u8 D_800A3344;
  extern u8 D_800A3348;
  extern void func_8006BEC4(s32, s32);
  extern void func_8005C650(s32, s32, s32);
  extern void motion_shift_check_m_hit_stop(void);
  extern void func_8003879C(void);
  extern void func_800387C0(void);
  extern void func_800387E8(void);
  s32 result = 0;
  s32 sel2 = -1;
  s32 v0;
  s32 sel;
  if (!D_800A3207)
  {
    D_800A3207 = 1;
    D_800A334C = 0x5A;
    D_800A3350 = 0;
    D_800A3354 = 0;
    D_800A31FC = 0;
  }
  v0 = func_80038734();
  if (D_800A3354 != 0)
  {
    func_8006BEC4(0xA, -1);
    D_800A334C--;
    if ((((u8) D_800A334C) == 0) || (D_80102794 & 0x100010))
    {
      func_8005C650(2, 0x7F, 0x7F);
      D_800A3207 = 1;
      D_800A334C = 0x5A;
      D_800A3350 = 0;
      D_800A3354 = 0;
      D_800A31FC = 0;
    }
    goto end;
  }
  if (D_800A3207 == 1)
  {
    v0 = 0;
  }
  else
    if (D_800A3207 == 2)
  {
  }
  else
    if (D_800A3207 == 3)
  {
  }
  else
    if (D_800A3207 == 4)
  {
    v0 = 0x11;
  }
  sel = 0;
  if (D_800A31FC != 0)
  {
    goto sel_dispatch;
  }
  if (D_800A3207 == 3)
  {
    if (v0 == 8)
    {
      goto case8_sel;
    }
    if (v0 != 10)
    {
      sel = 0xD;
    }
    else
    {
      sel = 0xF;
    }
    load_sel2:
    sel2 = D_800A3350;

    goto sel_dispatch;
  }
  switch (v0)
  {
    case 0:
      sel = 0x11;
      sel2 = D_800A3350;
      goto sel_dispatch;

    case 13:

    case 17:
      sel = 6;
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
      if (D_800A3206 == 0)
    {
      D_800A3350 = 1;
    }
      if (D_80102794 & 0x400040)
    {
      D_800A3206 = 0;
      func_8005C650(1, 0x7F, 0x7F);
      sel = 0xD;
      if (D_800A3350 == 0)
      {
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
      if ((D_80102794 & 0xA000A000U) != 0)
    {
      D_800A3206 = 1;
    }
      goto load_sel2;

    case 9:

    case 11:
      sel = 0xD;
      goto sel_dispatch;

    case 12:
      sel = 0xF;
      goto sel_dispatch;

    default:
      sel = 0;
      goto sel_dispatch;

  }

  sel_dispatch:
  if (sel >= 0)
  {
    func_8006BEC4(sel, sel2);
  }

  if (D_800A31FC != 0)
  {
    switch (v0 - 4)
    {
      case 0:
        break;

      case 4:
        D_800A3348++;
        if (((u8) D_800A3348) >= 5)
      {
        D_800A3354 = 1;
        D_800A334C = 0x5A;
        break;
      }
        motion_shift_check_m_hit_stop();
        break;

      case 1:

      case 2:

      case 3:

      case 9:

      case 10:

      case 11:
        D_800A3340++;
        if (((u8) D_800A3340) >= 5)
      {
        D_800A31FC = 0;
        func_8003879C();
        break;
      }
        motion_shift_check_m_hit_stop();
        break;

      case 6:
        D_800A3344++;
        if (((u8) D_800A3344) >= 5)
      {
        D_800A31FC = 0;
        break;
      }
        motion_shift_check_m_hit_stop();
        break;

      default:
        break;

    }

  }
  else
    if (D_800A3207 == 3)
  {
    switch (v0 - 4)
    {
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
        if ((((u8) D_800A334C) == 0) || (D_80102794 & 0x100010))
      {
        func_8005C650(2, 0x7F, 0x7F);
        result = 1;
        break;
      }
        break;

    }

  }
  else
  {
    switch (v0)
    {
      case 8:
        D_800A3354 = 1;
        D_800A334C = 0x5A;
        break;

      case 2:

      case 3:

      case 7:

      case 12:
        D_800A334C--;
        if ((((u8) D_800A334C) == 0) || (D_80102794 & 0x100010))
      {
        func_8005C650(2, 0x7F, 0x7F);
        if (v0 != 7)
        {
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
        motion_shift_check_m_hit_stop();
        D_800A3207 = 3;
        D_800A334C = 0x5A;
        break;

      case 13:
        D_800A3207 = 4;
        D_800A334C = 0x5A;
        D_800A3350 = 0;
        break;

      case 0:
        if (D_800A3207 == 5)
      {
        D_800A3207 = 1;
        break;
      }
        goto sw4_L2;

      case 17:
        sw4_L2:
      if (D_80102794 & 0x400040)
      {
        func_8005C650(1, 0x7F, 0x7F);
        if (v0 == 0)
        {
          if (D_800A3350 != 0)
          {
            result = 1;
            break;
          }
          D_800A3348 = 0;
          D_800A3340 = 0;
          D_800A3344 = 0;
          D_800A31FC = 1;
          motion_shift_check_m_hit_stop();
        }
        else
        {
          if (D_800A3350 != 0)
          {
            goto area_c_long;
          }
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
      if (D_80102794 & 0x80008000U)
      {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3350 = 0;
        break;
      }

        if (D_80102794 & 0x20002000)
      {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A3350 = 1;
      }
        break;

      default:
        break;

    }

  }
  if (v0 < 11)
  {
    if (v0 >= 9)
    {
      goto d_check;
    }
    if (v0 == 1)
    {
      goto d_check;
    }
    goto end;
  }
  if (v0 != 17)
  {
    goto end;
  }
  d_check:
  if (D_800A31F8 == (-1))
  {
    D_800A3354 = 1;
    D_800A334C = 0x5A;
  }

  end:
  if (result != 0)
  {
    D_800A3207 = 0;
    D_800A31FC = 0;
  }

  return result;
}
