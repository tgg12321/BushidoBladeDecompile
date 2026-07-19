/* KILLED s2 (H-s3): dropped `base` intermediate, `elem = &g_cd_sector_buf[idx];`
   sandbox 8 -> 13 (regressed by 5). Fewer pseudos worsens save-slot placement:
   sibling shape without its cheat pin/dummy does not reach match. */
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count;
    s32 idx;
    s32 saved;
    s32 *elem;
    s32 status;

    idx = a0 & 0xFF;
    saved = g_cd_callback_a;
    elem = &g_cd_sector_buf[idx];
    count = 3;
    /* ...body identical... */
}
