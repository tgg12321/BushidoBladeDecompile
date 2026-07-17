/* REJECTED s2: 3-temp one-per-swap form scores 13, build emits 13 insns vs
   target 14 — the scheduler fills target's unfilled load-delay nop at
   80042EF0. Same failure for the mem-to-mem variant (x=a0[2]; y=a0[1];
   z=a0[5]; a0[2]=a0[6]; a0[6]=x; a0[1]=a0[3]; a0[3]=y; a0[5]=a0[7];
   a0[7]=z;) — also 13/13. The explicit reused scratch for the second-loads
   is required to reproduce the $a2 triple-reuse + the kept nop. */
void func_80042ED8(u16 *a0) {
    u16 t1, t2, t3;
    t1 = a0[2];
    a0[2] = a0[6];
    a0[6] = t1;
    t2 = a0[1];
    a0[1] = a0[3];
    a0[3] = t2;
    t3 = a0[5];
    a0[5] = a0[7];
    a0[7] = t3;
}
