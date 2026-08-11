/* candidate — main (src/ings.c) — session 1 end state, sandbox floor 2.
 * ALREADY APPLIED to src/ings.c. Three coordinated edits vs the pre-session
 * tree (do not revert):
 *   1. line ~311: void func_80016A8C(u8 *arg0, u8 *arg1, s32 arg2) {   (was 1-arg)
 *   2. line ~441: void func_80016E60(u8 *arg0, s32 arg1) {             (was 1-arg)
 *      (unused extra params; main is the only caller of both — verified.
 *       Target bytes show main passing env/idx in a1/a2 resp. a1.)
 *   3. main body below.
 * Remaining diff vs target (score 2): our build folds ((D_800A36F1-1)<<8)+0x80
 * to (x<<8)-128 in RTL combine (combine.c:8196); target keeps addiu -1 + addiu
 * 0x80. See hypotheses.md frontier.
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
        s32 lim = D_800A36F1 - 1;
        if (cnt >= (lim << 8) + 0x80) break;
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
