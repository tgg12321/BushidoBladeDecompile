
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern s32 g_spu_voice_key_b;
extern s32 g_spu_voice_key_c;
void exec_game(void)
{
  s32 *new_var;
  s32 i;
  s32 j;
  s32 *base;
  s32 *cur;
  s32 *p;
  s32 new_var2;
  s32 tag;
  s32 *best;
  s32 saved;
  s32 best_tag;
  i = 0;
  if (g_spu_voice_key_b >= 0)
  {
    base = (s32 *) g_spu_voice_key_c;
    cur = base;
    do
    {
      if (cur[0] & 0x80000000)
      {
        j = i + 1;
        p = (s32 *) (((s32) base) + (j << 3));
        sentinel_check:
        if ((*p) == 0x2FFFFFFF)
        {
          p = (s32 *) (((s32) p) + 8);
          j++;
          goto sentinel_check;
        }

        p = (s32 *) (((s32) base) + (j << 3));
        tag = p[0];
        if ((tag & 0x80000000) && ((tag & 0x0FFFFFFF) == ((cur[0] & 0x0FFFFFFF) + cur[1])))
        {
          p[0] = 0x2FFFFFFF;
          cur[1] += p[1];
          goto phase1_check;
        }
      }
      cur = (s32 *) (((s32) cur) + 8);
      i++;
      phase1_check:
      ;

      ;
      ;
    }
    while (g_spu_voice_key_b >= i);
  }
  i = 0;
  new_var2 = g_spu_voice_key_b << 3;
  if (g_spu_voice_key_b >= 0)
  {
    saved = g_spu_voice_key_b;
    p = (s32 *) g_spu_voice_key_c;
    do
    {
      if (p[1] == 0)
      {
        p[0] = 0x2FFFFFFF;
      }
      i++;
      p = (s32 *) (((s32) p) + 8);
    }
    while (saved >= i);
  }
  i = 0;
  if (0 <= g_spu_voice_key_b)
  {
    base = (s32 *) g_spu_voice_key_c;
    cur = base;
    do
    {
      if (cur[0] & 0x40000000)
      {
        goto phase3_exit;
      }
      j = i + 1;
      if (g_spu_voice_key_b >= j)
      {
        best = cur;
        saved = g_spu_voice_key_b;
        p = (s32 *) (((s32) base) + (j << 3));
        do
        {
          tag = p[0];
          if (tag & 0x40000000)
          {
            goto phase3_next;
          }
          best_tag = best[0];
          if (((u32) (tag & 0x0FFFFFFF)) < ((u32) (best_tag & 0x0FFFFFFF)))
          {
            s32 s1;
            s32 s2;
            best[0] = tag;
            s1 = p[1];
            s2 = best[1];
            best[1] = s1;
            p[0] = best_tag;
            p[1] = s2;
          }
          j++;
          p = (s32 *) (((s32) p) + 8);
        }
        while (saved >= j);
      }
      phase3_next:
      i++;

      cur = (s32 *) (((s32) cur) + 8);
    }
    while (g_spu_voice_key_b >= i);
  }
  phase3_exit:
  i = 0;

  if (g_spu_voice_key_b >= 0)
  {
    base = (s32 *) g_spu_voice_key_c;
    cur = base;
    do
    {
      tag = cur[0];
      if (tag & 0x40000000)
      {
        goto phase4_exit;
      }
      if (tag == 0x2FFFFFFF)
      {
        p = (s32 *) (((s32) base) + new_var2);
        cur[0] = p[0];
        g_spu_voice_key_b = i;
        cur[1] = p[1];
        goto phase4_exit;
      }
      i++;
      cur = (s32 *) (((s32) cur) + 8);
    }
    while (g_spu_voice_key_b >= i);
  }
  phase4_exit:
  i = g_spu_voice_key_b - 1;

  if (i >= 0)
  {
    base = (s32 *) g_spu_voice_key_c;
    new_var = base;
    cur = (s32 *) (((s32) new_var) + (i << 3));
    do
    {
      tag = cur[0];
      if (!(tag & 0x80000000))
      {
        goto phase5_exit;
      }
      saved = g_spu_voice_key_b;
      cur[0] = (tag & 0x0FFFFFFF) | 0x40000000;
      g_spu_voice_key_b = i;
      cur[1] += ((s32 *) (((s32) new_var) + (saved << 3)))[1];
      i--;
      cur = (s32 *) (((s32) cur) - 8);
    }
    while (i >= 0);
  }
  phase5_exit:
  ;

  ;
  ;
}
