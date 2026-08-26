/* func_8002304C (tanren_CameraControl) — BEST BANKED FORM, honest floor 1
 * Measured 2026-08-26 (s1, recon): sandbox --disable all == 1, 216/216 insns,
 * zero pins, zero rules, zero FAKE constructs. APPLIED to src/code6cac.c at
 * session end (replaces the INCLUDE_ASM line; typedef Quad_2304C { s32 a,b,c,d; }
 * already exists in the TU just above the function).
 *
 * Sole residual (1 pt): target 0x232F4 `andi $v1,$a0,0xffff` vs our
 * `addu $v1,$a0,$zero` — same position, same registers, opcode-only.
 * Mechanism (read, not guessed): GCC 2.7.2 combine.c nonzero_bits REG case
 * (combine.c:6885-6923) proves the same-BB single-set lhu value <= 0xFFFF via
 * reg_last_set_value/get_last_value and simplifies zero_extend(subreg:HI) to a
 * copy. The reg_nonzero_bits full-mask fallback (combine.c:6920-6923) is only
 * reachable for multi-set pseudos whose last set is in a DIFFERENT label
 * region (label_tick mismatch) — impossible here: the lhu and the mask use
 * are 2 insns apart in one BB. See evidence.md [s1-E6..E8] + hypotheses.md.
 */
void func_8002304C(u8 *obj, s32 *pos1, s32 *pos2, s32 *arg3)
{
  s32 *scratch = (s32 *) 0x1F8001B0;
  s32 count = 0;
  s32 lim;
  s16 *scratch_d;
  s32 *scratch_c = (s32 *) 0x1F8001C0;
  lim = 0x1F8002B8;
  scratch_d = (s16 *) 0x1F8001D0;
  loop:
  if (((pos1[0] != pos2[0]) || (pos1[1] != pos2[1])) || (pos1[2] != pos2[2]))
  {
    if (func_8005344C(pos1, pos2, scratch, scratch_c, lim) == 0)
    {
      *(Quad_2304C *)pos1 = *(Quad_2304C *)pos2;
      goto done;
    }
    *((s8 *) (obj + 0xB1)) = (s8) func_80054434();
    *(Quad_2304C *)pos1 = *(Quad_2304C *)scratch;
    func_8002EBDC((s16 *) arg3, (s16 *) scratch_c, arg3, -0x40, 0xE6);
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      scratch[12] = pos1[0] + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      scratch[13] = pos1[1] + (vel / 1024);
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      scratch[14] = pos1[2] + (vel / 1024);
    }
    {
      s16 vel;
      vel = *((s16 *) (((u8 *) scratch) + 0x10));
      pos2[0] += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x12));
      pos2[1] += vel / 1024;
      vel = *((s16 *) (((u8 *) scratch) + 0x14));
      pos2[2] += vel / 1024;
    }
    if (func_8005344C(pos1, scratch + 12, scratch, scratch + 6, lim) == 0)
    {
      *(Quad_2304C *)pos1 = *(Quad_2304C *)(scratch + 12);
      scratch[8] = pos2[0] - pos1[0];
      scratch[9] = pos2[1] - pos1[1];
      scratch[10] = pos2[2] - pos1[2];
      {
        func_8002EBDC(scratch_d, (s16 *) scratch_c, (s32 *) scratch_d, 0,
                      (*((u16 *) (obj + 0x6A)) == 0x15) ? 0x80 : 0x100);
      }
      {
        s16 vel_y = *((s16 *) (((u8 *) scratch) + 0x12));
        if (vel_y >= (-0x7FF))
        {
          s32 mode = *((u16 *) (obj + 0x6A));
          u16 m = mode;
          if (((((m != 8) && (m != 0x22)) && (((u32) (mode - 0x17)) >= 2)) && (m != 0xA)) && ((*((s16 *) (obj + 0x72))) == 0))
          {
            scratch[9] = 0;
          }
        }
      }
      pos2[0] = pos1[0] + scratch[8];
      pos2[1] = pos1[1] + scratch[9];
      count++;
      pos2[2] = pos1[2] + scratch[10];
      if (count < 4)
      {
        goto loop;
      }
    }
  }

  done:
  ;

  ;
}
