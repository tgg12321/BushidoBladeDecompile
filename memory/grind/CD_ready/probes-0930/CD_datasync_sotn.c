s32 CD_datasync(s32 mode) /* SOTN: src/main/psxsdk/libcd/bios.c:459 @aa53500 */
{
    /* FAKE: one return value written on each of the three exits, reused exactly
     * as SOTN's CD_datasync reuses its ret (Q51, Q53); lever-exhaustion: a direct
     * return on each exit = 4/91 (94 insns), memory/grind/CD_ready/evidence.md */
    s32 ret; /* SOTN: src/main/psxsdk/libcd/bios.c:460 @aa53500 */

    set_alarm(D_800162C0);
    while (1) {
        if (get_alarm()) {
            ret = -1;
            break;
        }
        if (!(*D_800A14C0 & 0x1000000)) {
            ret = 0;
            break;
        }
        if (mode != 0) {
            ret = 1;
            break;
        }
    }
    return ret;
}
