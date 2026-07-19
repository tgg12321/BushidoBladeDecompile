/* REJECTED: reordering `count = 3;` to the FIRST assignment (before
   idx/saved/base/elem) worsened the sandbox from 8 → 13. First-write
   order for s0 alone doesn't drive save-slot placement in the way I
   hypothesized; other locals' first-use ordering matters more. */
s32 tslPolyF4Init(s32 a0, s32 a1, s32 a2) {
    s32 count, idx, saved, *elem, *base, status;

    count = 3;                  /* was last; moved to first */
    idx = a0 & 0xFF;
    base = g_cd_sector_buf;
    saved = g_cd_callback_a;
    elem = base + idx;
    /* ...body unchanged... */
}
