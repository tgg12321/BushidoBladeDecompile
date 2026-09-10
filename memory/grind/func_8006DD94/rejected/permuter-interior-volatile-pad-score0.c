/* REJECTED (s2, permuter): decomp-permuter campaign s2-frame-residual found this at
 * iteration ~24k (tmp/perm_6dd94/output-0-2) — permuter score 0 on the --stack-diffs
 * metric.  The ONLY change from the honest score-21 chassis is an INTERIOR
 * `volatile unsigned long long pad;` declared between `EnvB s;` and `u16 rect[4];`.
 * It is the forbidden unused-local pad family, in the interior position the Judge
 * explicitly refused on 2026-09-10 05:59 ("An INTERIOR pad is outside the form
 * constraint and I do not grant it; proposing one is a new family, i.e. another FAIL").
 * Banked ONLY as the measurement that the residual is one stack-homed slot wide.
 * Sibling finds in the same campaign: output-0-1 `volatile short pad;` (same slot,
 * also score 0), output-37-1 `volatile int pad;` declared LAST (score 37 — position
 * matters: the pad must precede the rect), output-97-1 a dead `short new_var = 0;`.
 * NOT SUBMITTABLE.  Do not respell. */
typedef struct EnvB
{
  s32 *header;
  s8 *table;
  s32 out;
  s32 pad0C;
  s32 semi;
  u32 ot_idx;
  s32 x;
  s32 y;
  s32 pad20;
  s32 pad24;
  u8 has_color;
  u8 col_r;
  u8 col_g;
  u8 col_b;
} EnvB;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
void func_8006DD94(s32 *arg0)
{
  EnvB s;
  volatile unsigned long long pad;
  u16 rect[4];
  s16 i;
  s32 *q;
  s32 c;
  s32 hdr;
  s32 semi = 0;
  s.ot_idx = 0xA;
  q = *((s32 **) (arg0[1] + 0x3C));
  s.x = 0;
  s.semi = semi;
  for (i = 0; i < 3; i++)
  {
    s.has_color = 1;
    if (i == (D_800A352C + 1))
    {
      s.y = *((s16 *) (D_800A34FC + 0xE));
      c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
      s.col_r = (s.col_g = (s.col_b = c));
    }
    else
    {
      if (i == 0)
      {
        s.col_r = (s.col_g = (s.col_b = 0x80));
      }
      else
      {
        s.col_r = (s.col_g = (s.col_b = 0x40));
      }
      s.y = 0;
    }
    hdr = q[i + 8];
    s.header = (s32 *) hdr;
    s.table = (s8 *) (hdr + 0xC);
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32) (&s));
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32) s.header, semi), 0);
    AddPrim(D_800A374C + 0x28, arg0[7]);
    arg0[7] += 0xC;
  }

  func_8006D808(&arg0[5], &arg0[7], q, s.ot_idx, -1);
  rect[2] = 0x96;
  rect[0] = 0xF5;
  rect[1] = 0x25;
  rect[3] = 1;
  func_80069898((GameObj *) arg0, rect, 0x11);
}
