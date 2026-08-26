/* func_8002304C (tanren_CameraControl) - MATCHED, honest distance 0.
 * Measured 2026-08-26 (s2, structural): `sandbox func_8002304C --disable all`
 * == 0 (216/216 insns) and full-build `verify-oracle` SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa. Zero regfix/asmfix rules, zero
 * register pins, zero inline asm, zero FAKE constructs.
 *
 * The s1 residual (target 0x232F4 `andi $v1,$a0,0xffff` vs our
 * `addu $v1,$a0,$zero`) closed by spelling the masked state id the way the
 * ORIGINAL AUTHOR spelled it in the sibling function 250 lines below in this
 * same TU: func_80023E40 (COMPLETED-C since 6d255e79, src/code6cac.c:2545-2546)
 * reads the identical field with the identical two-line idiom
 *     s32 a0 = *(u16 *)(arg0 + 0x6A);
 *     s32 v1 = a0 & 0xFFFF;
 * and runs the identical comparison cascade (== 8, == 0x22,
 * (u32)(a0 - 0x17) < 2, == 0xA). The two functions are copy-paste siblings in
 * the original source; reconstructing the same idiom here is source fidelity,
 * not coercion. Both operands stay s32: `mode` is the raw widened load that
 * feeds `mode - 0x17`, `m` is the masked state id that feeds the three
 * equality tests.
 *
 * Why the mask survives to bytes (mechanism, dump-read not guessed):
 * combine sees (insn A) reg74 = zero_extend:SI(mem:HI) and (insn B)
 * reg75 = and:SI(reg74, 65535). It cannot substitute A into B because reg74 is
 * still live afterwards (`mode - 0x17`), so the AND is never brought into a
 * combination where nonzero_bits() could prove it redundant, and the standalone
 * andsi3 insn reaches the assembler as `andi $v1,$a0,0xffff`. Every s1
 * spelling that typed either side narrow (u16 mode, or u16 m) let combine fold
 * the truncate/extend pair into a copy or delete it outright - see
 * rejected/u16-mode-with-masked-or-copied-m.c.
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
      func_8002EBDC(scratch_d, (s16 *) scratch_c, (s32 *) scratch_d, 0,
                    (*((u16 *) (obj + 0x6A)) == 0x15) ? 0x80 : 0x100);
      {
        s16 vel_y = *((s16 *) (((u8 *) scratch) + 0x12));
        if (vel_y >= (-0x7FF))
        {
          s32 mode = *((u16 *) (obj + 0x6A));
          s32 m = mode & 0xFFFF;
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
}
