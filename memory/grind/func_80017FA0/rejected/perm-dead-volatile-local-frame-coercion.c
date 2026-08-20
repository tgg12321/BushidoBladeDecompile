/* REJECTED (s4, permuter) - dead-volatile-local frame coercion.
 *
 * The s4 permuter campaign (tmp/perm_17fa0, 15,708 iterations) produced 26
 * score-0 forms. Most of them close the target's 8-byte zero-store leaf frame
 * by declaring a DEAD volatile local of >=4 bytes (`volatile long long pad;`,
 * `volatile unsigned long pad;`, `volatile char new_var;`) whose only effect is
 * to make get_frame_size() non-zero. Every one of these is the forbidden
 * dead-vars-local-array family (.claude/rules/dead-vars-local-array.md): no
 * semantic purpose, no human would write it, the name announces the intent, and
 * the WRITTEN carve-out is inapplicable because the target contains ZERO frame
 * stores. Banked as PROPOSALS THAT WERE REFUSED, not as candidates.
 *
 * The accepted closer came from the same campaign but is a different construct:
 * output-0-3 spelled the outer loop guard against the live counter
 * (`if (i < ptr[1])` instead of `if (ptr[1] > 0)`), which reserves the same 8
 * frame bytes with an ordinary LIVE local and no dead declaration. See
 * candidate.c. */

/* --- permuter output-0-4: volatile long long pad --- */
void func_80017FA0(s32 *a0)
{
  volatile s32 *scr = (volatile s32 *) 0x1F800000;
  s32 temp;
  s32 *ptr;
  volatile long long pad;
  temp = a0[3];
  if (temp == 0)
  {
    goto end;
  }
  ptr = (s32 *) temp;
  scr[0x2E] = ptr[0] << 7;
  {
    s32 i = 0;
    if (ptr[1] > 0)
    {
      s32 *p68 = ptr;
      volatile s32 *ac_base = (volatile s32 *) 0x1F800000;
      s32 sp_off = 0;
      do
      {
        s32 j = 0;
        s32 data_off = i << 5;
        s32 sp_inner = sp_off;
        do
        {
          s32 *dp = (s32 *) (((u8 *) ptr) + data_off);
          *((volatile s32 *) (0x1F800064 + sp_inner)) = dp[2] << 2;
          data_off += 0x10;
          *((volatile s32 *) (0x1F800068 + sp_inner)) = dp[3] << 2;
          j++;
          *((volatile s32 *) (0x1F80006C + sp_inner)) = dp[4] << 2;
          sp_inner += 0xC;
        }
        while (j < 2);
        ac_base[0x2B] = (*((s32 *) (((u8 *) p68) + 0x68))) << 2;
        p68 = (s32 *) (((u8 *) p68) + 4);
        sp_off += 0x18;
        i++;
        ac_base++;
      }
      while (i < ptr[1]);
    }
  }
  scr[0x18] = ((s32 *) a0[3])[1];
  end:
  ;

  ;
}

/* --- permuter output-0-1: volatile unsigned long pad --- */
void func_80017FA0(s32 *a0)
{
  volatile s32 *scr = (volatile s32 *) 0x1F800000;
  s32 temp;
  s32 *ptr;
  volatile unsigned long pad;
  temp = a0[3];
  if (temp == 0)
  {
    goto end;
  }
  ptr = (s32 *) temp;
  scr[0x2E] = ptr[0] << 7;
  {
    s32 i = 0;
    if (ptr[1] > 0)
    {
      s32 *p68 = ptr;
      volatile s32 *ac_base = (volatile s32 *) 0x1F800000;
      s32 sp_off = 0;
      do
      {
        s32 j = 0;
        s32 data_off = i << 5;
        s32 sp_inner = sp_off;
        do
        {
          s32 *dp = (s32 *) (((u8 *) ptr) + data_off);
          *((volatile s32 *) (0x1F800064 + sp_inner)) = dp[2] << 2;
          data_off += 0x10;
          *((volatile s32 *) (0x1F800068 + sp_inner)) = dp[3] << 2;
          j++;
          *((volatile s32 *) (0x1F80006C + sp_inner)) = dp[4] << 2;
          sp_inner += 0xC;
        }
        while (j < 2);
        ac_base[0x2B] = (*((s32 *) (((u8 *) p68) + 0x68))) << 2;
        p68 = (s32 *) (((u8 *) p68) + 4);
        sp_off += 0x18;
        i++;
        ac_base++;
      }
      while (i < ptr[1]);
    }
  }
  scr[0x18] = ((s32 *) a0[3])[1];
  end:
  ;

  ;
}

/* --- permuter output-0-2: volatile char new_var --- */
void func_80017FA0(s32 *a0)
{
  volatile s32 *scr = (volatile s32 *) 0x1F800000;
  volatile char new_var;
  s32 temp;
  s32 *ptr;
  temp = a0[3];
  if (temp == 0)
  {
    goto end;
  }
  ptr = (s32 *) temp;
  scr[0x2E] = ptr[0] << 7;
  ;
  {
    s32 i = 0;
    if (ptr[1] > 0)
    {
      s32 *p68 = ptr;
      volatile s32 *ac_base = (volatile s32 *) 0x1F800000;
      s32 sp_off = 0;
      do
      {
        s32 j = 0;
        s32 data_off = i << 5;
        s32 sp_inner = sp_off;
        do
        {
          s32 *dp = (s32 *) (((u8 *) ptr) + data_off);
          *((volatile s32 *) (0x1F800064 + sp_inner)) = dp[2] << 2;
          data_off += 0x10;
          *((volatile s32 *) (0x1F800068 + sp_inner)) = dp[3] << 2;
          j++;
          *((volatile s32 *) (0x1F80006C + sp_inner)) = dp[4] << 2;
          sp_inner += 0xC;
        }
        while (j < 2);
        ac_base[0x2B] = (*((s32 *) (((u8 *) p68) + 0x68))) << 2;
        p68 = (s32 *) (((u8 *) p68) + 4);
        sp_off += 0x18;
        i++;
        ac_base++;
      }
      while (i < ptr[1]);
    }
  }
  scr[0x18] = ((s32 *) a0[3])[1];
  end:
  ;

  ;
}
