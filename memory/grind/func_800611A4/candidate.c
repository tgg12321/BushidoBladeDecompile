/* func_800611A4 — pin-free pure-C form. Honest sandbox --disable all = 6
 * (s3 lowered from 9). Emitted post-call shape: mask lui/ori/sw D_800A3464
 * ATOMICALLY hoisted to the top of the tail, then 3× {lw arg0[i]; sw ...}.
 * Because the mask pseudo is dead before load 1, GCC reuses $v0 for both
 * mask and the load-temp — no register-name swap penalty. Structurally
 * different from target (target interleaves the mask lui/ori into the load
 * gaps and keeps mask alive across the loads in $v1), but scores fewer diffs
 * than any interleaved variant we can reach without a pin.
 *
 * Target requires load-temp -> $v0 AND mask -> $v1 with mask ALIVE across
 * all three loads. Any pure-C form that keeps mask alive across the loads
 * (V5/V6/V9/V16 in s3) ends up allocating mask -> $v0 and load-temp -> $v1
 * (the swap seen in HEAD), scoring 9. The V4/V7 shape here (mask atomic-first)
 * is the only pin-free arrangement measured this session that drops the
 * score below 9. Reaching 0 needs the interleaved shape WITH the reversed
 * register assignment; no pure-C lever measured this session can flip it.
 *
 * See memory/grind/func_800611A4/hypotheses.md (s3) for the killed axes:
 * pre-call reorderings all worse; v1 alias load-bearing (drop = +1 insn);
 * no-t-local (m2c shape) = 22; u32 mask type = 9 (no effect). */
extern u8 D_800F116A;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
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
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    t = arg0[2];
    D_800F1148 = t;
}
