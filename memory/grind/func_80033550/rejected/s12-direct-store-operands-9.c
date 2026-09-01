extern u8 D_80107850;
extern u8 D_80107854;
extern u8 D_80107858;
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  idx = new_var;
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = arg0[0];
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = arg0[1];
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = arg0[2];
}
