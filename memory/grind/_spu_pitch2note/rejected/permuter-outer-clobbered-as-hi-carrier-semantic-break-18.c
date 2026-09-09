s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch)
{
  s32 bit;
  s32 shift;
  s32 oct;
  s32 scale;
  u32 curve;
  u32 target;
  u32 lower;
  u32 upper;
  u32 step;
  u32 acc;
  u32 next;
  u32 lo;
  u32 hi;
  s32 outer;
  s32 inner;
  s32 result;
  s32 quot;
  s32 rem;
  s32 note;
  s32 fine;
  u16 search;
  search = ~pitch;
  bit = 0;
  for (shift = 15; shift >= 0; shift--)
  {
    if (!((search >> shift) & 1))
    {
      bit = shift;
      break;
    }
  }

  target = pitch;
  scale = 1 << bit;
  curve = 0x1000;
  oct = bit - 12;
  outer = 0;
  while (outer < 0x30)
  {
    lower = scale * curve;
    curve *= 0x103B;
    curve >>= 12;
    inner = curve;
    upper = scale * inner;
    step = (upper - lower) >> 5;
    for (inner = 0, acc = 0, next = step; inner < 0x20; inner++)
    {
      lo = (lower + acc) >> 12;
      hi = lower + next;
      hi >>= 12;
      if ((pitch >= lo) && (pitch < hi))
      {
        result = (outer << 5) + inner;
        goto found;
      }
      next += step;
      acc += step;
    }

    outer++;
  }

  result = 0x600;
  found:
  quot = result / 128;

  rem = result % 128;
  note = (cen_note + quot) + (oct * 12);
  fine = cen_fine + rem;
  return (note << 8) | fine;
}
