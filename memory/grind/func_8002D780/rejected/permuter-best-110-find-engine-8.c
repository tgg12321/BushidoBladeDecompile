/* REJECTED [s4, permuter modality] - engine sandbox score 8 at 202 insns, i.e. one
 * WORSE than the 7-floor control, despite being decomp-permuter's best find of the
 * campaign (permuter weighted score 110 against a base of 300).
 *
 * This is the headline measurement of s4: on this chassis the permuter's weighted
 * scorer (regs x5, reorderings x60, ins/del x100) and the engine's raw
 * differing-instruction score are ANTI-CORRELATED.  55,531 iterations descended
 * 300 -> 110 in the permuter metric while every find that was re-measured on the
 * real chassis came back at 8 or 9 against the control's 7 (output-110-1 = 8,
 * output-115-1 = 8, output-115-2 = 8, output-115-3 = 9, output-125-1 = 9,
 * output-210-1 = 7 at 201 insns).  The permuter is buying reordering credit with
 * an extra raw diff, which is free in its metric and fatal in ours.
 *
 * The body also carries two forbidden-family constructs the permuter emits freely
 * (an `if (1) { ... }`-class brace/sameline restructuring in earlier siblings, and
 * here an alias pseudo assigned inside a condition) - it is banked as EVIDENCE, not
 * as a proposal.  Do not resubmit any part of it without a family claim. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq)
{
  s32 new_var;
  if (flag == 0)
  {
    s32 *vin;
    s32 *vout;
    *((s16 *) (obj + 0xF8)) = pos[0] - (*((s32 **) (obj + 0x60)))[0];
    *((s16 *) (obj + 0xFA)) = pos[1] - (*((s32 **) (obj + 0x60)))[1];
    *((s16 *) (obj + 0xFC)) = pos[2] - (*((s32 **) (obj + 0x60)))[2];
    vin = (s32 *) (obj + 0xF8);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
    vout = (s32 *) (obj + 0x100);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
  }
  {
    s32 y = *((s32 *) (obj + 0x108));
    if ((y < (-threshold)) || (threshold < y))
    {
      return 0;
    }
  }
  {
    s32 x0 = *((s32 *) (obj + 0xA8));
    s32 x2 = *((s32 *) (obj + 0xB8));
    s32 z0 = *((s32 *) (obj + 0xAC));
    s32 z2 = *((s32 *) (obj + 0xBC));
    s32 cx = (x0 + x2) / 3;
    s32 cz = (z0 + z2) / 3;
    s32 px = *((s32 *) (obj + 0x100));
    s32 pz = *((s32 *) (obj + 0x104));
    s32 kc = (z0 * cx) - (x0 * cz);
    s32 kp = (z0 * px) - (x0 * pz);
    if ((kc ^ kp) >= 0)
    {
      kc = (z2 * cx) - (x2 * cz);
      kp = (z2 * px) - (x2 * pz);
 if ((kc ^ kp) >= 0) { s32 ax = cx - x0; s32 az = cz - z0; s32 bx = px - x0; s32 bz = pz - z0; s32 dz = z2 - z0; s32 dx = x2 - x0; kc = (dz * ax) - (dx * az);
        kp = (dz * bx) - (dx * bz);
        if ((kc ^ kp) >= 0)
        {
          return 1;
        }
      }
    }
  }
  {
    s32 y = *((s32 *) (obj + 0x108));
    s32 sp_var;
    s32 dist = r_sq - (y * y);
    s32 sqrt_val;
    s32 *p118;
    s32 *p124;
    s32 *p10C;
    if (((u32) (new_var = dist)) < 0x400)
    {
      sqrt_val = ((u32) (*((&D_8008D118) + new_var))) >> 3;
    }
    else
    {
      s32 lzcr = 0;
      if (dist >= 0)
      {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, %2, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(dist), "r"(&sp_var) : "$12", "$13", "$14", "$15");
        lzcr = sp_var;
      }
      {
        s32 shift = 0x16 - (lzcr & (~1));
        s32 tbl = *((&D_8008D118) + (((u32) new_var) >> shift));
        sqrt_val = ((u32) (tbl << 16)) >> (0x13 - (((u32) shift) >> 1));
      }
    }
    p118 = (s32 *) (obj + 0x118);
    p124 = (s32 *) (obj + 0x124);
    p118[0] = (*((s32 *) (obj + 0xA8))) - (*((s32 *) (obj + 0x100)));
    p118[1] = (*((s32 *) (obj + 0xAC))) - (*((s32 *) (obj + 0x104)));
    p124[0] = (*((s32 *) (obj + 0xB8))) - (*((s32 *) (obj + 0x100)));
    p124[1] = (*((s32 *) (obj + 0xBC))) - (*((s32 *) (obj + 0x104)));
    if (func_8002D518(sqrt_val, new_var, p118, p124) != 0)
    {
      return 1;
    }
    p10C = (s32 *) (obj + 0x10C);
    p10C[0] = -(*((s32 *) (obj + 0x100)));
    p10C[1] = -(*((s32 *) (obj + 0x104)));
    if (func_8002D518(sqrt_val, dist, p10C, p118) != 0)
    {
      return 1;
    }
    if (func_8002D518(sqrt_val, new_var, p10C, p124) != 0)
    {
      return 1;
    }
    return 0;
  }
}
