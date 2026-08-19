void func_8007C4B8(s32 *out, Rect *r)
{
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  out[1] = func_8007C7A0(r->x, r->y);
  out[2] = func_8007C86C((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));
  out[3] = func_8007C938(r->u, r->v);
  out[4] = func_8007C748(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));
  out[5] = func_8007C97C(((u8 *) r) + 12);
  out[6] = (s32) 0xE6000000;
  var_a3 = 7;
  if (!(r->flag != 0))
  {
    goto end;
  }
  buf[0] = (u16) r->x;
  buf[1] = (u16) r->y;
  new_var = (s16) r->w;
  buf[2] = (u16) r->w;
  buf[3] = (u16) r->h;
  if (new_var >= 0)
  {
    if ((D_8009BE78 - 1) < new_var)
    {
      var_v0 = D_8009BE78 - 1;
    }
    else
    {
      var_v0 = new_var;
    }
  }
  else
  {
    var_v0 = 0;
  }
  buf[2] = (u16) var_v0;
  if (((s16) buf[3]) >= 0)
  {
    if ((D_8009BE7A - 1) < ((s16) buf[3]))
    {
      var_v0_2 = D_8009BE7A - 1;
    }
    else
    {
      var_v0_2 = (s16) buf[3];
    }
  }
  else
  {
    var_v0_2 = 0;
  }
  buf[3] = (u16) var_v0_2;
  if ((buf[0] & 0x3F) || (buf[2] & 0x3F))
  {
    buf[0] -= (u16) r->u;
    buf[1] -= (u16) r->v;
    out[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
    out[var_a3++] = ((u32 *) buf)[0];
    out[var_a3++] = ((u32 *) buf)[1];
    buf[0] += (u16) r->u;
    buf[1] += (u16) r->v;
  }
  else
  {
    out[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x02000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
    out[var_a3++] = ((u32 *) buf)[0];
    out[var_a3++] = ((u32 *) buf)[1];
  }
end:
  *(((s8 *) out) + 3) = (s8) (var_a3 - 1);
}
