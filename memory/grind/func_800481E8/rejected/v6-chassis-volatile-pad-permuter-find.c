/* REJECTED (s4, permuter modality, 2026-08-20) — CHEAT, not a candidate.
 * Best honest --stack-diffs find of the s4 fresh-seed campaign on the V6 chassis
 * (54,167 iters / 23 min / 8 jobs): score 202 vs base 266 — i.e. it does NOT even
 * close the gap, and the ONLY thing it changes is adding a `volatile unsigned char
 * new_var;` dead pad to grow the frame. That is the volatile-coercion forbidden
 * family (frame coercion by volatile-typed scalar), score-inert under the sandbox
 * (which strips it) and a layer-1 FAIL on sight. All 3 novel finds of the campaign
 * were the same construct with a different volatile width (212=u16, 207=short,
 * 202=uchar). Identical family to s2's `volatile int new_var;` find, reproduced
 * from a structurally DIFFERENT seed chassis — so the basin is a property of the
 * function, not of the seed spelling. DO NOT re-propose. */

typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
extern void func_800482C8();
void func_800481E8(s32 arg0, s32 arg1)
{
  volatile unsigned char new_var;
  u32 *p;
  s32 a3w;
  u32 *base;
  s32 count;
  s32 a0_for_call;
  p = (u32 *) arg0;
  base = p;
  arg0 = 0;
  p = (u32 *) (((s32) p) + (((s32) (arg1 << 16)) >> 14));
  p = (u32 *) (((s32) base) + (((*p) >> 2) << 2));
  count = *(p++);
  if (count != 0)
  {
    count--;
    do
    {
      u32 word;
      s32 a1w;
      s32 a2w;
      s32 v0w;
      unsigned int new_var2;
      word = *p;
      p = (u32 *) (((s32) p) + 4);
      a1w = *((u16 *) p);
      p = (u32 *) (((s32) p) + 2);
      a2w = *((u16 *) p);
      p = (u32 *) (((s32) p) + 2);
      a3w = *((u16 *) p);
      p = (u32 *) (((s32) p) + 2);
      v0w = *((u16 *) p);
      new_var2 = word >> 2;
      a0_for_call = ((s32) base) + (new_var2 << 2);
      p = (u32 *) (((s32) p) + 2);
      if (((s32) ((s16) a3w)) < 0x280)
      {
        v0w += 1;
      }
      ;
      func_800482C8(a0_for_call, (s32) ((s16) a1w), (s32) ((s16) a2w), (s32) ((s16) a3w), (s32) ((s16) v0w));
    }
    while ((count--) != 0);
  }
}
