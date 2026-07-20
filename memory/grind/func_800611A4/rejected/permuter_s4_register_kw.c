/* s4 V13: K&R `register` keyword hint on t/mask without asm() pin.
 * Sandbox = 9 (baseline pin-free wall). GCC 2.7.2 respects `register` only
 * weakly for RA priority, and here the load-temp vs mask tiebreak is driven
 * by global.c's allocno priority formula (log2(refs)*freq*size/live_length),
 * which the `register` keyword does not influence. Not a lever.
 */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    register s32 t;
    register s32 mask;
    s32 *v1 = (s32 *) (&D_800F116C);
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    mask = 0xFFFFEF;
    D_800F1144 = t;
    D_800A3464 = mask;
    t = arg0[2];
    D_800F1148 = t;
}
