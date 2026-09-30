s32 CD_sync(s32 mode, u8 *result) /* SOTN: src/main/psxsdk/libcd/bios.c:232 @aa53500 */
{
    s32 sync;

    set_alarm(D_80016240);
    while (1) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
        sync = Intr.sync;
        if (sync == 2 || sync == 5) {
            Intr.sync = 2;
            _memcpy(result, &Result, 8);
            return sync;
        }
        if (mode != 0) {
            return 0;
        }
    }
}
