/* SOTN: src/main/psxsdk/libcd/bios.c:260 @aa53500 */
s32 CD_ready(s32 mode, u8 *result)
{
    s32 c;
    s32 ready;

    set_alarm(D_80016248);
    while (1) {
        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
        c = Intr.c;
        if (c != 0) {
            Intr.c = 0;
            _memcpy(result, &Result_plus_0x10, 8);
            return c;
        }
        ready = Intr.ready;
        if (ready != 0) {
            Intr.ready = 0;
            _memcpy(result, &Result_plus_0x8, 8);
            return ready;
        }
        if (mode != 0) {
            return 0;
        }
    }
}
