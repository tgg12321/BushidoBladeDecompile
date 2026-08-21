/* REJECTED (s4, 2026-08-20): permuter campaign #1's best find (score 105/170,
 * output-105-1). The invented pointer intermediate below is the BANNED
 * base/ff invented-block-0-intermediate family's direction (layer-1 FAIL
 * 2026-08-20 15:48) — and it does not even close (105, not 0): a bare copy
 * is partially coalesced by cse; full closure needs the complete banned
 * overlapping-pair construct. Do not re-propose in any spelling. */
void func_800324D0(u8 *pad)
{
  u8 *ptr;
  u8 *new_var;
  u8 c;
  u32 cmd;
  u8 val;
  new_var = *((u8 **) (pad + 0x58));
  ptr = new_var;
  c = 0xFF;
  pad[0xA1] = c;
  pad[0xA3] = c;
  pad[0xA2] = c;
  pad[0xA4] = c;
  pad[0xAA] = 0;
  pad[0xA7] = 0;
  pad[0xA8] = 0;
  pad[0xA5] = 0;
  pad[0xA6] = c;
  pad[0xAB] = c;
  pad[0xAC] = c;
  c = ptr[4];
  cmd = c;
  ptr += 5;
  if (cmd == 0)
  {
    return;
  }
  do
  {
    cmd = c;
    if (cmd == 0xFF)
    {
      ptr += 6;
    }
    else
      if (cmd < 0x80)
    {
      ptr++;
    }
    else
    {
      cmd -= 0x80;
      val = *ptr;
      ptr++;
      if (cmd < 12)
      {
        switch (cmd)
        {
          case 0:
            pad[0xA1] = val;
            break;

          case 1:
            pad[0xA3] = val;
            break;

          case 2:
            pad[0xA7] = val;
            break;

          case 3:
            pad[0xA8] = val;
            break;

          case 4:
            pad[0xA9] = val;
            break;

          case 5:
            pad[0xA5] = val;
            break;

          case 6:
            pad[0xA6] = val;
            break;

          case 7:
            pad[0xA2] = val;
            break;

          case 8:
            pad[0xA4] = val;
            break;

          case 9:
            pad[0xAA] = val;
            break;

          case 10:
            pad[0xAB] = val;
            break;

          case 11:
            pad[0xAC] = val;
            break;

        }

      }
    }
    c = *ptr;
    ptr++;
  }
  while (c != 0);
}
