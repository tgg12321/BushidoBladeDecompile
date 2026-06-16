/* REJECTED: scope_mask — score 7
 * Reason: correct delay slot (NOP), correct mask->$v1, correct gp-relative store,
 * BUT t->$a0 (wrong; should be $v0). Both $v0 and $v1 get added to t's conflict
 * by local_alloc (tmp takes $v0, mask takes $v1), pushing t to $a0.
 * Also: store ordering D_800A3464 before D_800F1144 (wrong). */
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    { s32 tmp = arg0[1];
      { s32 mask = 0xFF00FF; D_800A3464 = mask; }
      D_800F1144 = tmp; }
    t = arg0[2]; D_800F1148 = t;
