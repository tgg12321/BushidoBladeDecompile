/* REJECTED - func_800283D0 grind s4 (permuter modality), 2026-08-27
 * Permuter find (tmp/grind/func_800283D0/s4/perm_c/output-440-1), sandbox
 * --disable all = 22 / 215 insns, the lowest number this session produced.
 *
 * REJECTED FOR TWO INDEPENDENT REASONS:
 *
 * 1. SEMANTICALLY INVALID.  It reuses the live local `temp_s3` (which holds
 *    `arg0 + temp_a1_2 * 2`) as a store pointer `temp_s3 = arg0 + 0x286;`
 *    inside the `temp_v1_3 < temp_v0_3 && var_s1 != 0` arm, and then spells
 *    the block_48 store as `*(s16 *)temp_s3 = var_v0_2;`.  block_48 is ALSO
 *    reached from the `temp_v1_3 >= temp_v0_3` path (the var_v0_2 = 0x1A
 *    chain), where temp_s3 still holds `arg0 + temp_a1_2 * 2` - so on that
 *    path the store lands at the WRONG ADDRESS.  decomp-permuter's randomizer
 *    does not guarantee semantic equivalence; every find must be read.
 *
 * 2. DIRECTIONALLY WRONG.  The target emits `sh $v0, 0x286($s0)` at ALL THREE
 *    store sites (asm/funcs/func_800283D0.s:92, 162, 222), i.e. base+offset
 *    addressing off the arg0 pointer - never a register-indirect store.  A
 *    pointer local for the store address can therefore never be the answer,
 *    whatever it does to the allocator.
 *
 * The body also carries the `do { ... } while (0);` wrap (see
 * dowhile0-refweight-out-of-scope.c) and a `if (temp_a0_2 > (var_s1 = 0))`
 * dead-store-in-condition which was measured to contribute EXACTLY ZERO
 * (tmp/grind/func_800283D0/s4/v_nodeadstore.c also scores 22).
 */

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
extern s16 D_800A3824;
extern s16 D_800A3876;
extern s16 D_800A38A8;
extern s16 Judge;
extern s32 func_80032854(s32, s32, u8 *, s16 *);
s32 func_800283D0(u8 *arg0, u8 *arg1)
{
  s32 temp_a1;
  u8 *temp_s4;
  s32 temp_v1;
  s32 var_s1;
  s16 var_v0;
  s32 ret;
  temp_s4 = *((u8 **) arg0);
  ret = 1;
  temp_a1 = *((u16 *) (arg0 + 0x6A));
  temp_v1 = temp_a1 & 0xFFFF;
  if (temp_v1 != 4)
  {
    if (temp_v1 != 0x14)
    {
      u16 temp_v0 = *((u16 *) (temp_s4 + 0x6A));
      if ((temp_v0 != 4) && (temp_v0 != 0x14))
      {
        s32 d_val;
        s32 temp_a1_2;
        s32 temp_s5;
        if (temp_v1 != 0x13)
        {
          if ((((((((((u32) (temp_a1 - 0x19)) >= 2U) && (temp_v1 != 2)) && (temp_v1 != 0x26)) && (temp_v1 != 0x1B)) && (temp_v1 != 0x15)) && (temp_v1 != 0x25)) && (temp_v1 != 0x2C)) && (temp_v1 != 0xC))
          {
            return 1;
          }
          var_s1 = 0;
          goto block_15;
        }
        var_s1 = 0;
        block_15:
        d_val = D_800A3824;

        temp_a1_2 = (d_val >> (*((s16 *) (arg0 + 4)))) & 1;
        temp_s5 = (d_val >> (*((s16 *) (temp_s4 + 4)))) & 1;
        if ((*((s16 *) (arg0 + 0x8C))) != 0)
        {
          var_s1 = temp_a1_2 == 0;
        }
        if (var_s1 != 0)
        {
          s16 temp_v1_2 = *((s16 *) (arg0 + 0xC));
          if (temp_v1_2 != 0x1D)
          {
            if (temp_v1_2 != 0xE)
            {
              goto block_20;
            }
            return ret;
          }
          goto block_49;
        }
        block_20:
        {
          s16 temp_v1_3 = *((s16 *) ((temp_s4 + (temp_s5 * 2)) + 0x288));
          if (temp_v1_3 == 0)
          {
            if ((*((s16 *) ((arg0 + (temp_a1_2 * 2)) + 0x288))) > 0)
            {
              var_v0 = 0x19;
              if (var_s1 == 0)
              {
                set_0xB:
                var_v0 = 0xB;

              }
              do_store_calls:
              *((s16 *) (arg0 + 0x286)) = var_v0;

              do_calls:
              func_80032854(*((s16 *) (arg0 + 4)), 1, arg1, (s16 *) 0);

              func_80032854(*((s16 *) (arg0 + 4)), 0x25, arg1, (s16 *) 0);
              return ret;
            }
            goto block_49;
          }
          {
            u8 *temp_s3 = arg0 + (temp_a1_2 * 2);
            s16 temp_v0_3 = *((s16 *) (temp_s3 + 0x288));
            s16 var_v0_2;
            if (temp_v0_3 == temp_v1_3)
            {
              func_80032854(*((s16 *) (arg0 + 4)), 1, arg1, (s16 *) 0);
              func_80032854(*((s16 *) (arg0 + 4)), 0x25, arg1, (s16 *) 0);
              if ((*((s16 *) (temp_s3 + 0x288))) == 5)
              {
                if ((((u32) ((*((u16 *) (arg0 + 0xE))) - 6)) < 2U) || (((u32) ((*((u16 *) (temp_s4 + 0xE))) - 6)) < 2U))
                {
                  var_v0_2 = 0x19;
                  if (var_s1 == 0)
                  {
                    var_v0_2 = 0xB;
                  }
                  goto block_48;
                }
                D_800A38A8 = 1;
                D_800A3876 = -1;
                return ret;
              }
              if (var_s1 != 0)
              {
                var_v0_2 = 0x19;
              }
              else
              {
                var_v0_2 = 0xB;
              }
              goto block_48;
            }
            do
            {
              if (temp_v1_3 < temp_v0_3)
              {
                if (var_s1 != 0)
                {
                  temp_s3 = arg0 + 0x286;
                  *((s16 *) temp_s3) = 0x19;
                }
                else
                {
                  goto set_0xB;
                }
                goto do_calls;
              }
              func_80032854(*((s16 *) (arg0 + 4)), 0x26, arg1, (s16 *) 0);
              func_80032854(*((s16 *) (arg0 + 4)), 0x2D, arg1, (s16 *) 0);
              var_v0_2 = 0x1A;
              if (var_s1 == 0)
              {
                u8 *temp_a0 = temp_s4 + (temp_s5 * 0x10);
                s32 temp_v1_4 = -(*((s16 *) (arg0 + 0x1CA)));
                s32 temp_a0_2 = *((s32 *) (temp_a0 + 0x118));
                s32 var_a1 = temp_a0_2;
                s32 temp_v1_5 = ((s32) (((&Judge)[(temp_v1_4 + 0x400) & 0xFFF] * (*((s32 *) (temp_a0 + 0x114)))) + ((&Judge)[temp_v1_4 & 0xFFF] * (*((s32 *) (temp_a0 + 0x11C)))))) >> 0xC;
                s32 var_v0_3;
                if (temp_a0_2 < 0)
                {
                  var_a1 = -temp_a0_2;
                }
                var_v0_3 = temp_v1_5;
                if (temp_v1_5 < 0)
                {
                  var_v0_3 = -temp_v1_5;
                }
                if (var_v0_3 < var_a1)
                {
                  var_v0_2 = 0x14;
                  if (temp_a0_2 > (var_s1 = 0))
                  {
                    var_v0_2 = 0x13;
                  }
                }
                else
                {
                  var_v0_2 = 0x15;
                  if (temp_v1_5 <= 0)
                  {
                    var_v0_2 = 0x16;
                  }
                }
              }
              block_48:
              *((s16 *) temp_s3) = var_v0_2;

            }
            while (0);
          }
        }

        block_49:
        return ret;

      }
      goto block_13;
    }
    return ret;
  }
  block_13:
  return ret;

}
