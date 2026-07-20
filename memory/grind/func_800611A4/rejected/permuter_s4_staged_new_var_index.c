/* Permuter s4 iter-473 basin: reuse `new_var` (u16 halfword, dead after sp[2]=new_var)
 * as a staging index for arg0[0] via `new_var = 0; t = arg0[new_var];`.
 * Permuter score 50 (base 610) -- a stable basin, hit repeatedly (iters 473,
 * 709, 1053, 1122, 1126, 2178, 2279, 2489, 2493).
 *
 * SANDBOX = 9 (WORSE than V7 floor 6). Permuter's weighted metric ranks it low
 * because it collapses several scheduling reorderings, but the raw diff count
 * grows: the staging insn adds an extra addu/li that sandbox counts against.
 *
 * Also a CHEAT by any spelling: `new_var = 0; t = arg0[new_var];` has NO
 * semantic difference from `t = arg0[0];` -- the staged 0 exists solely to
 * bias GCC's RA allocation of the load-temp pseudo. The staged-value-reused-
 * variable rule is sanctioned only when (a) the mechanism is scheduler
 * load-late LAUNCH priority, (b) no dead stores, (c) FAKE annotation, (d)
 * documented lever exhaustion. Here the mechanism is RA priority (not
 * scheduling), which the rule explicitly does not sanction. See
 * .claude/rules/no-new-park-categories.md -- coercion cheats by any spelling.
 */
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 t;
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
    D_800A3464 = 0xFFFFEF;
    new_var = 0;
    t = arg0[new_var];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    t = arg0[2];
    D_800F1148 = t;
}
