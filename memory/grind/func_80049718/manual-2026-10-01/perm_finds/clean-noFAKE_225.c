void func_80049718(s32 arg0, s32 flags, s32 *arg2, s16 *arg3)
{
  SVECTOR rot;
  s16 *anim;
  s32 val58;
  u8 *vehicle;
  u8 *obj;
  u8 *part;
  ;
  if ((*(&D_800EF980[arg0])) < 0)
  {
    if (1)
    {
      func_80052C10();
    }
  }
  obj = D_800A38B4;
  val58 = 0;
  obj[0] = 0;
  obj[1] = 0;
  *((s16 *) (obj + 2)) = (*(&D_800EF980[arg0])) * 2;
  *((s16 *) (obj + 4)) = 6;
  *((s16 *) (obj + 8)) = 0;
  *((s32 *) (obj + 0xC)) = 0;
  *((s16 *) (obj + 0xA)) = 4;
  if (flags != 0)
  {
    if (flags == 1)
    {
      *((u16 *) (obj + 0x10)) = arg3[0];
      *((u16 *) (obj + 0x12)) = arg3[1];
      *((u16 *) (obj + 0x14)) = arg3[2];
      ((void (*)(SVECTOR *, MATRIX *)) g_anim_func_table[0])((SVECTOR *) (obj + 0x10), (MATRIX *) (obj + 0x18));
      *((s32 *) (obj + 0x2C)) = arg2[0];
      *((s32 *) (obj + 0x30)) = arg2[1];
      *((s32 *) (obj + 0x34)) = arg2[2];
    }
    else
    {
      flags &= 0x7FFF;
      vehicle = (u8 *) func_8004153C(flags >> 1);
      part = vehicle + (((flags & 1) * 0x68) + 0x7E4);
      *((s32 *) (part + 0x4C)) = ((*((s32 *) (part + 0x4C))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
      *((s32 *) (part + 0x50)) = ((*((s32 *) (part + 0x50))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
      *((s32 *) (part + 0x54)) = ((*((s32 *) (part + 0x54))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
      MulMatrix0((MATRIX *) (vehicle + 0x44), (MATRIX *) (part + 0x38), (MATRIX *) (obj + 0x18));
      rot.vx = *((s32 *) (part + 0x4C));
      rot.vy = *((s32 *) (part + 0x50));
      rot.vz = *((s32 *) (part + 0x54));
      ApplyMatrix((MATRIX *) ((*((u8 **) (part + 0xC))) + 0x18), &rot, (VECTOR *) (obj + 0x2C));
      *((s32 *) (obj + 0x2C)) = (*((s32 *) (obj + 0x2C))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x2C)));
      *((s32 *) (obj + 0x30)) = (*((s32 *) (obj + 0x30))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x30)));
      *((s32 *) (obj + 0x34)) = (*((s32 *) (obj + 0x34))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x34)));
      flags |= 0x8000;
      *((MATRIX *) (part + 0x18)) = *((MATRIX *) (obj + 0x18));
      val58 = *((s16 *) (vehicle + 0x1A84));
    }
    {
      u8 *ot = (u8 *) D_800A3820;
      D_800A3820 = (s32) (ot + 4);
      *((u8 **) ot) = obj;
    }
    obj += 0x68;
    if (flags != 1)
    {
      s32 frame = D_800EF980[arg0];
      u8 *ot;
      obj[0] = 3;
      obj[1] = 0;
      *((s32 *) (obj + 0x58)) = val58;
      ot = (u8 *) D_800A3820;
      *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
      *((s16 *) (obj + 6)) = 1;
      *((s16 *) (obj + 8)) = 0;
      *((s16 *) (obj + 0xA)) = 0;
      *((s16 *) (obj + 4)) = 6;
      *((s16 *) (obj + 2)) = (frame * 2) + 1;
      D_800A3820 = (s32) (ot + 4);
      *((u8 **) ot) = obj;
      obj += 0x68;
    }
    D_800A38B4 = obj;
  }
}
