/* candidate — main (src/ings.c) — session 2 end state, SANDBOX DISTANCE 0
 * (189/189 insns, all 25 regfix rules dropped, measured 2026-08-11 s2).
 * APPLIED to src/ings.c. Four coordinated edits vs the pre-grind tree:
 *   1. line ~311: void func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2) {  (was 1-arg)
 *   2. line ~441: void func_80016E60(u8 *arg0, s32 arg1) {            (was 1-arg)
 *      (unused extra params; main is the only caller of both — verified by
 *       repo-wide grep in s1. Target bytes show main passing env/idx in
 *       a1/a2 resp. a1. The widening reproduces the original call ABI.)
 *   3. main body below (named tbl[idx] load first in the 0xFFFECC00 block).
 *   4. THE s2 CLOSER — poll-loop threshold as chained same-variable
 *      accumulation:
 *          s32 lim = D_800A36F1;
 *          lim = lim - 1;
 *          lim = lim << 8;
 *          lim = lim + 0x80;
 *      Mechanism (pinned in GCC source): every statement re-uses ONE pseudo,
 *      so when try_combine merges the addiu(-1) into the sll and the
 *      distribution (combine.c:8196) produces
 *      (set rLIM (plus (ashift rLIM 8) -256)), the 2->2 split that would
 *      accept the fold is REFUSED by the guard at combine.c:1836
 *      ("We can't overwrite I2DEST if its value is still used by NEWPAT"
 *      — reg_referenced_p (i2dest, newpat) is true because the chain root
 *      is the same pseudo). The 3-insn path (addiu,sll,addiu 0x80) is
 *      blocked by the same guard (i2dest = rLIM self-referencing sll).
 *      Result: the chain stays unfolded — lbu; addiu -1; sll 8; addiu 0x80;
 *      slt — exactly the target bytes. Verified in micro-harness
 *      (tmp/grind/main/s2/foldM4.c/.s) then whole-file sandbox = 0.
 *      Every statement is live (each value read by the next); no dead
 *      stores, no volatile, no pins.
 * POLICY STATUS: ruling-request pending. The spelling is same-variable
 * split-init accumulation, but the sanctioned precedent (user 2026-06-13,
 * commit ad11a8c8, func_80049C24) scopes to "split a real a+b into init +
 * += on the same var, combine folds it back" (byte-NEUTRAL refs-lift);
 * this form is a 3-step chain whose effect is byte-MATERIALIZING (combine
 * does NOT fold back). The sanctioning memory says adjacent spellings need
 * their own user ruling — asked via the s2 outcome's ruling_question.
 */
void main(void) {
    s32 idx;
    u8 *env;
    u8 *ot;
    s32 voice;
    u32 *tbl;

    func_80083794();
    SetSp(0x801FFF00);
    SetMem(2);
    sys_Init();
    sys_GameInit();
    SetDispMask(1);
    func_80016A8C((u8 *)0x80118800, env, idx);

    tbl = &D_800A3770;
    D_800A3834 = 0xF;
    D_800A390D = 0;
    D_800A36AC = 0;

loop:
    idx = D_800A36AC & 1;
    env = &D_800F7438 + idx * 0x4090;
    ot = env + 0x70;
    ClearOTagR(ot, 0x1008);
    D_800A374C = ot;
    D_800A38B4 = tbl[idx];
    func_80060E04(idx);
    func_8003D2F4();
    func_80019568(voice);
    func_80036940();
    func_8005C6D0();

    if (D_800A3928 != 0) {
        func_800372C0();
        D_800A3768 = 0xFF;
        D_800A3928 = 0;
        D_800A31DA = 0;
        D_800A3834 = 8;
    }

    ((void (*)(void))(&D_8008D090)[D_800A3834])();
    func_8003D330();

    do {
        s32 cnt = GetRCnt(0xF2000001u);
        s32 lim = D_800A36F1;
        lim = lim - 1;
        lim = lim << 8;
        lim = lim + 0x80;
        if (cnt >= lim) break;
        rand();
    } while (1);

    VSync(1);
    DrawSync(0);
    VSync(0);
    ResetRCnt(0xF2000001u);

    voice = D_800A390D;
    if (voice == 0) {
        PutDispEnv(env + 0x5C);
        PutDrawEnv(env);
    }

    {
        s32 cnt = (s32)tbl[idx];
        s32 adj = D_800A38B4 + 0xFFFECC00u;
        s32 remaining = cnt - adj;
        if (remaining < D_800A30DC) {
            D_800A30DC = remaining;
        }
        if (remaining < 0) {
            printf(&D_80010034);
            while (1) {
                func_800164F8();
            }
        }
    }

    if (D_800A390D != 0) {
        D_800A390D--;
    } else {
        DrawOTag(env + 0x408C);
        D_800A36AC++;
    }

    if (D_800A3834 != 1) goto loop;
    if (voice != 0) goto loop;
    if (D_80102794 & 0x08000800u) goto call_func;
    if (D_800A38DC != 2) goto loop;
    if (D_800A3713 == 0) goto loop;
    D_800A3713--;
    if (D_800A3713 != 0) goto loop;
call_func:
    func_80016E60(env, idx);
    goto loop;
}
