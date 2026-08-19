/* s8 REJECTED: cc1 SIGSEGV, same loop.c strength-reduction crash as the u16-stride
 * form. This was frontier item 1's flagship probe ('D_800A9A24 declared as an array
 * type and indexed, so the address is a single (plus (symbol) (mult)) tree'). It
 * cannot be compiled at all on the frozen chassis. */
    LoadImage((s32)rect, (s32)((u8 (*)[32])&D_800A9A24)[idx - 1]);
