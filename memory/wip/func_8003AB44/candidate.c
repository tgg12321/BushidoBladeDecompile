/* Candidate body for func_8003AB44 (code6cac_c_ab.c).
 *
 * Floor: 6 (same as HEAD).
 *
 * Differences from HEAD:
 *   - Removes 3 cheat-asm scheduling barriers (__asm__ __volatile__("" ::: "memory"))
 *     in cases 0 and 1. These were confirmed SCORE-INERT (sandbox stripped them
 *     in both HEAD and this candidate; floor stays 6 either way).
 *
 * This candidate is a SOURCE CLEANUP rather than floor progress. The 6-distance
 * gap (case 2 polarity + case 5/6 register) remains unresolved.
 *
 * To resume: apply this body to src/code6cac_c_ab.c, then iterate on case 2
 * polarity and case 5/6 register issues (see notes.md). Sandbox will still
 * report 6 from this base — same as HEAD — but with 3 fewer cheat-asm
 * constructs, the function is one step closer to COMPLETED-C.
 */

s32 func_8003AB44(void) {
    D_800A37B8++;
    switch (D_800A38AC) {
    case 0:
        D_800A38AC = 1;
        return 0;
    case 1:
        func_8003A308();
        D_800A37D8 = 0;
        if (D_800A38A0 == 0) {
            gpu_SetDispMask(1);
            D_800A38AC = 2;
            return 0;
        }
        D_800A38AC = 3;
        return 0;
    case 2:
        if (D_80102794 & 0x10) {
            goto fail;
        }
        if (func_8008C464(3, 1, 0) == 0) {
            return 0;
        }
        goto done;
    case 3:
        if (func_8008C464(3, 1, 0) != 0) {
            goto retry;
        }
        /* fall through */
    done:
        func_8008C464(3, 0, 0);
        D_800A38AC = 4;
        return 0;
    retry:
        D_800A37D8++;
        if (D_800A37D8 < 4) {
            return 0;
        }
        /* fall through */
    fail:
        func_8003A39C();
        return -1;
    case 4:
        gpu_SetDispMask(0);
    case 5:
    case 6:
        D_800A38AC++;
        return 0;
    case 7:
        D_800A3916 = 1;
        func_8003A360();
        return 1;
    }
    return 0;
}
