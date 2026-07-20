/* func_800611A4 — pin-free pure-C form. Honest sandbox --disable all = 9.
 * The 9 diffs are ENTIRELY a v0<->v1 register-name swap on the post-call
 * load-temp / mask cluster (no instruction add/del/reorder — structurally
 * byte-identical to target except register names). HEAD "matches" only via
 * register asm("$2")/asm("$3") pins (cheat-asm). This form is the cheat-free
 * floor. See notes.md: the swap is unreachable from correct pure C — confirmed
 * against BOTH cc1 ports (our GCC 2.7.2 and original cc1psx). */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
    s32 mask;
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
    D_800F1144 = t;
    mask = 0xFFFFEF;
    D_800A3464 = mask;
    t = arg0[2];
    D_800F1148 = t;
}
