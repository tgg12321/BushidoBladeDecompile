s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3)
{
  struct V { s32 x; s32 y; s32 z; };
  struct V *a = (struct V *)arg0;
  struct V *b = (struct V *)arg1;
  struct V *c = (struct V *)arg2;
  struct V *p = (struct V *)arg3;
  s32 center_x = ((a->x + b->x) + c->x) / 3;
  s32 center_z = ((a->z + b->z) + c->z) / 3;
  s32 cross_center;
  s32 cross_point;
  s32 ret = 0;
  {
    s32 dz = b->z - a->z;
    s32 dx = b->x - a->x;
    cross_center = (dz * (center_x - a->x)) - (dx * (center_z - a->z));
    cross_point = (dz * (p->x - a->x)) - (dx * (p->z - a->z));
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = c->z - a->z;
    s32 dx = c->x - a->x;
    cross_center = ((ret = dz) * (center_x - a->x)) - (dx * (center_z - a->z));
    cross_point = (dz * (p->x - a->x)) - (dx * (p->z - a->z));
    ret = 0;
  }
  if ((cross_center ^ cross_point) < 0)
  {
    goto end;
  }
  {
    s32 dz = c->z - b->z;
    s32 dx = c->x - b->x;
    cross_center = (dz * (center_x - b->x)) - (dx * (center_z - b->z));
    cross_point = (dz * (p->x - b->x)) - (dx * (p->z - b->z));
  }
  ret = (cross_center ^ cross_point) >= 0;
  end:
  return ret;
}
