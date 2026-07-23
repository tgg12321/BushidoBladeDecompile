/* func_80061710 s4 PERMUTER re-find — score 0, REJECTED as cheat by any spelling.
 *
 * Directed permuter on the pin-free floor-9 chassis (tmp/perm_710_s4) reached
 * sandbox/permuter score 0 in 4 independent finds (output-0-1..0-4, ~635 iters
 * to first find), EVERY one the constant-staging coercion below. This is the
 * IDENTICAL construct Judge-FAILed for sibling func_80061658 (route-to-sign-off,
 * docs/grind/decisions.md 2026-07-22 22:50): staging the mask constant through
 * the reused live local `val` to steer GCC local-alloc.c:472 copy-preference off
 * $v0 for the mask. No semantic purpose; register-CHOICE coercion outside every
 * sanctioned family. Vetted per no-new-park-categories § auto-search-output ->
 * REJECTED, not surfaced. The deny-copy-source campaign (tmp/perm_710_s4b, ~19k
 * iters) proves this is the UNIQUE score-0 basin: block-scoping `val` so it is
 * not a live copy source yields NO score-0 (best 50). See s4/permuter_summary.md.
 */
void func_80061710(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0: val = 0x21000E; p = &D_800F115C + 2; *p = 0; D_800F1180 = (s32)p; *v1 = val; break;
    case 1: val = 0x21000F; p = &D_800F115C + 3; *p = 0; D_800F1180 = (s32)p; *v1 = val; break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; val = 0x10FF10; D_800F1144 = t;   /* CHEAT: val staged as copy source */
    mask = val;                                     /* CHEAT: copy-preference steers RA off $v0 */
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
