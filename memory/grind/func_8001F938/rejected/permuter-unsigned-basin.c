/* func_8001F938 — s5 permuter REJECTED: unsigned floor-6 basin is permuter-dead.
 *
 * A decomp-permuter campaign (pid 405) seeded from the STRUCTURALLY-DIFFERENT
 * unsigned-read floor-6 chassis (s3: `u32 probe=*(u16*)(a0+0x270); if(probe>=4U)
 * raw=3; else raw=probe; idx=(raw<<16)>>15;`), which s4's clean-floor-8 campaign
 * never explored. 36,031 iterations / ~19 min; base score 705 -> plateaued at 505
 * for the full observation window (5 stale samples over 7 min flat; observer loop
 * saw best=505 from t=5s to campaign death).
 *
 * KILLED. The +0x270 crux is UNTOUCHED in the best form (below): still the plain
 * unsigned read with `sltiu`, fold defeated but the SIGNED compare forfeited. The
 * 505 improvement over base 705 came entirely from unrelated branch-address / frame
 * scheduling noise, NOT from closing the +0x270 gap. The randomizer NEVER restored
 * the signed compare and NEVER generated the banned dual-typed read (the only
 * distance-0 form) — exactly as s4 found from the floor-8 basin. Confirms the
 * signedness-split dichotomy is inescapable by random search from BOTH basins.
 * This form == the already-banked rejected/unsigned-single-read.c (dominated by the
 * pre-banned signed-cast-single-read.c). No new lever. src/ stays at clean floor 8.
 */
void func_8001F938(u8 *arg0)
{
  u32 kind_full;
  u32 kind;
  s32 val;
  volatile unsigned long long pad;
  s32 a2;
  s32 idx;
  s32 factor;
  kind_full = *((u16 *) (arg0 + 0x6A));
  kind = kind_full & 0xFFFFU;
  a2 = *((s16 *) (arg0 + 0x1C));
  if (((((((((((kind == 0x11) || (kind == 0xF)) || (((u32) (((s32) kind_full) - 0x1C)) < 2U)) || (((u32) (((s32) kind_full) - 0x1E)) < 2U)) || (((u32) (((s32) kind_full) - 0x20)) < 2U)) || (kind == 0xE)) || (kind == 0x2C)) || (kind == 0xD)) || (kind == 0x7)) || (kind == 0x33)) || (kind == 0x14))
  {
    goto clamp;
  }
  if (kind == 0x2)
  {
    goto rangecheck;
  }
  if (kind == 0x1B)
  {
    goto rangecheck;
  }
  if (kind == 0x28)
  {
    goto rangecheck;
  }
  if (kind != 0x26)
  {
    goto defaultpath;
  }
  rangecheck:
  val = *((s16 *) (arg0 + 0x40));

  if (val < ((s32) (*((u8 *) (arg0 + 0xA1)))))
  {
    goto check_outer;
  }
  if (val > ((s32) (*((u8 *) (arg0 + 0xA3)))))
  {
    goto check_outer;
  }
  goto clamp;
  check_outer:
  if (val < ((s32) (*((u8 *) (arg0 + 0xA2)))))
  {
    goto multpath_start;
  }

  if (val > ((s32) (*((u8 *) (arg0 + 0xA4)))))
  {
    goto multpath_start;
  }
  clamp:
  *((s16 *) (arg0 + 0x44)) = 0x1000;

  return;
  multpath_start:
  if ((*((s16 *) (arg0 + 0x26C))) == 0)
  {
    s32 f = *((s16 *) (arg0 + 0x274));
    a2 = (a2 * f) >> 12;
  }

  {
    u32 probe = *((u16 *) (arg0 + 0x270));
    s32 raw_or_3;
    if (probe >= 4U)
    {
      raw_or_3 = 3;
    }
    else
    {
      raw_or_3 = probe;
    }
    idx = (raw_or_3 << 16) >> 15;
  }
  factor = *((s16 *) ((arg0 + 0x276) + idx));
  a2 = (a2 * factor) >> 12;
  defaultpath:
  {
    s32 vv0 = *((s16 *) (arg0 + 0x26E));
    s32 vv1 = *((s16 *) (arg0 + 0x272));
    s32 sum = vv0 + vv1;
    s32 sum_or_3 = (sum < 4) ? (sum) : (3);
    idx = sum_or_3 * 2;
  }

  factor = *((s16 *) ((arg0 + 0x27E) + idx));
  a2 = (a2 * factor) >> 12;
  *((s16 *) (arg0 + 0x44)) = (s16) a2;
}
