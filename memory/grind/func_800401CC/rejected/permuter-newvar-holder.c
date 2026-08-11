/*
 * REJECTED — permuter score-0 find, session s3-permuter, 2026-08-11.
 * Found 97s into the score2-natural-order campaign (output-0-1, iter ~1992,
 * stop-on-zero). This IS a byte-perfect closing form — and it is the BANNED
 * constant-holder construct for this function respelled: a NEW dead-scalar
 * local (`unsigned int new_var;`) invented solely to hold the low 24-bit mask
 * and set before the packet-link statements. The driver's ban covers this
 * construct in any spelling; per no-new-park-categories the permuter's output
 * is a proposal, and this one is rejected without adoption.
 *
 * Evidentiary value: within the permuter's mutation space, the ONLY zero on
 * the score-2 chassis is this holder family. No natural spelling closes the
 * 2-insn constant-emission-order residual (24k further iterations on the
 * floor-7 chassis also found nothing). The closing lever family is real; the
 * open question (filed as this session's ruling-request) is whether staging
 * the low mask through the PRE-EXISTING dead-after-call texture-U local
 * (staged-value-reused-variable family, twin of the layer-1-approved v
 * staging) is inside or outside the ban.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
extern s32 D_800A36AC;
extern s32 D_800A378C;
extern s32 D_800A3234;
extern s32 D_800A3378;
extern u8 D_800A9830;
extern u8 D_800A9920;
extern u16 D_80094AF4;
extern void SetDrawMove(s32, s16 *, s32, s32);
void func_800401CC(s32 a0, s32 a1, s32 a2)
{
  s16 buf[4];
  unsigned int new_var;
  u16 *tbl;
  s16 u;
  s32 v;
  s32 *pkt;
  s32 *ot;
  a2 = D_800A36AC & 1;
  if (a2 != D_800A3234)
  {
    D_800A3378 = (s32) ((&D_800A9830) + (a2 * 240));
    D_800A3234 = a2;
  }
  if (((s32 *) D_800A3378) != ((s32 *) ((&D_800A9920) + (D_800A3234 * 240))))
  {
    tbl = (&D_80094AF4) + (a1 * 6);
    buf[0] = *(tbl++);
    buf[1] = *(tbl++);
    buf[2] = *(tbl++);
    buf[3] = *(tbl++);
    u = *(tbl++);
    v = *tbl;
    if (a0 != 0)
    {
      buf[0] = buf[0] + 0x80;
      u = u + 0x80;
    }
    SetDrawMove((s32) ((s32 *) D_800A3378), buf, (s16) u, (s16) v);
    pkt = (s32 *) D_800A3378;
    new_var = 0xFFFFFF;
    ot = (s32 *) D_800A378C;
    v = 0xFF000000;
    *pkt = ((*pkt) & v) | (ot[0x3FFC / 4] & new_var);
    ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | (((s32) pkt) & new_var);
    D_800A3378 = (s32) (pkt + 6);
  }
}
