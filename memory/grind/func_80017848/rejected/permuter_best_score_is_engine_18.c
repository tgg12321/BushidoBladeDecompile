/* REJECTED - engine distance 18 (worse than the 14 it was derived from).
 * decomp-permuter's OWN best find of the s4 campaign A: permuter score 405 vs
 * base 855, i.e. the permuter called this a 53%% improvement.  Re-scored with
 * `sandbox --disable all` it is 18.  Banked as the concrete proof of the s4
 * headline finding: on a function whose residual is a small instruction
 * DELETION near the top of the body, the permuter's weighted metric charges the
 * whole byte-exact tail as displaced (855 ~= 14 reorderings x 60), so its
 * gradient rewards destroying the tail.  NEVER trust a permuter score ranking
 * on this function; re-score every find with the engine.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b)
{
  u8 *link;
  u8 *slots;
  u8 *p;
  u8 *base;
  u8 *rec_a;
  u8 *rec_b;
  s32 i;
  s32 dist;
  if (slot_a == slot_b)
  {
    return 0;
  }
  slots = *((u8 **) (ctx + 0xC));
  if ((*((s32 *) (((slot_a << 6) + ((s32) (*((u8 **) (ctx + 0xC))))) + 0x18))) >= 0)
  {
    if ((*((s32 *) (((slot_b << 6) + ((s32) (*((u8 **) (ctx + 0xC))))) + 0x18))) >= 0)
    {
      return 0;
    }
  }
  i = 0;
  if (i < (*((s32 *) ((((s32) (*((u8 **) (ctx + 0xC)))) + (slot_a << 6)) + 0x1C))))
  {
    p = *((u8 **) (ctx + 0xC));
    base = (u8 *) ((slot_a << 6) + ((s32) p));
    do
    {
      if ((*((u16 *) ((((*((u8 *) ((((u8 *) ((slot_a << 6) + ((s32) p))) + i) + 0x24))) << 4) + ((s32) (*((u8 **) (ctx + 0x10))))) + 0x4))) == slot_b)
      {
        return 0;
      }
      i++;
    }
    while (i < (*((s32 *) (base + 0x1C))));
  }
  i = 0;
  if (i < (*((s32 *) ((((s32) (*((u8 **) (ctx + 0xC)))) + (slot_a << 6)) + 0x20))))
  {
    p = *((u8 **) (ctx + 0xC));
    base = (u8 *) ((slot_a << 6) + ((s32) p));
    do
    {
      if ((*((s16 *) ((((*((u8 *) ((base + i) + 0x2C))) << 4) + ((s32) (*((u8 **) (ctx + 0x10))))) + 0x6))) == slot_b)
      {
        return 0;
      }
      i++;
    }
    while (i < (*((s32 *) (base + 0x20))));
  }
  dist = math_Distance3D((s32 *) ((*((u8 **) (ctx + 0xC))) + (slot_a << 6)), (s32 *) ((*((u8 **) (ctx + 0xC))) + (slot_b << 6)));
  link = (*((u8 **) (ctx + 0x10))) + ((*((s16 *) (ctx + 0x6))) << 4);
  *((s32 *) (link + 0x0)) = dist;
  *((s32 *) (link + 0x8)) = dist * 3;
  *((s32 *) (link + 0xC)) = arg1;
  *((s32 *) (link + 0x4)) = (slot_a << 16) | slot_b;
  rec_a = (u8 *) ((slot_a << 6) + ((s32) (*((u8 **) (ctx + 0xC)))));
  i = *((s32 *) (rec_a + 0x1C));
  *((s32 *) (rec_a + 0x1C)) = i + 1;
  *((u8 *) ((rec_a + i) + 0x24)) = *((u16 *) (ctx + 0x6));
  rec_b = (u8 *) ((slot_b << 6) + ((s32) (*((u8 **) (ctx + 0xC)))));
  i = *((s32 *) (rec_b + 0x20));
  *((s32 *) (rec_b + 0x20)) = i + 1;
  *((u8 *) ((rec_b + i) + 0x2C)) = *((u16 *) (ctx + 0x6));
  *((s16 *) (ctx + 0x6)) = (*((u16 *) (ctx + 0x6))) + 1;
  return 1;
}
