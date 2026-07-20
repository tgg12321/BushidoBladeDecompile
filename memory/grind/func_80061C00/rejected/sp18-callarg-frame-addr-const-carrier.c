/* s3 P1 — KILLED, score 7 (probe-1 leak shape).
 * val = {(s32)sp18 as motutil_GetWalkDir arg, 0x10016 in arm 1}, repack natural.
 * Mechanism: cse folds the call-arg copy `a0 = val` into a direct
 * `addiu a0,sp,0x18` (frame addiu as cheap as reg move), leaving val's first
 * set dead -> flow deletes it -> val collapses to const-only local ->
 * local_alloc leak (const->$v1, arg1->$a3).
 * NEW KILL-LAW: frame-address second sets survive only at STORE-OPERAND uses
 * (variant H), never at CALL-ARG uses. Same fate expected for (s32)sp10/(s32)sp30
 * call-arg hosts; additionally (s32)sp10 at the call would invite cse to
 * substitute the later +0x10 store's addiu (v0) — doubly dead.
 */
void func_80061C00(s32 arg0, s32 arg1, s32 arg2) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20[3];
    u8 sp30[32];
    s32 sp50;
    s32 val;

    D_800A3468 = (s32)&D_800F116C;
    if (arg2 != 1) {
        arg2 = 0;
    }
    sp10[1] = -0xA00;
    sp10[0] = 0;
    sp10[2] = 0xA00;
    sp20[2] = 0;
    sp20[1] = 0;
    sp20[0] = 0;
    sp18[1] = arg1;
    sp18[2] = 0;
    sp18[0] = 0;
    val = (s32)sp18;
    motutil_GetWalkDir((s16 *)val, sp30);
    /* ... rest identical to candidate.c minus the val repack carrier;
       arm 1: arg1 = D_800A3468; ... val = 0x10016; *(s32 *)arg1 = val; */
}
