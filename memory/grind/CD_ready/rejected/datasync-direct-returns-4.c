s32 CD_datasync(s32 mode)
{
    set_alarm(D_800162C0);
    while (1) {
        if (get_alarm()) {
            return -1;
        }
        if (!(*D_800A14C0 & 0x1000000)) {
            return 0;
        }
        if (mode != 0) {
            return 1;
        }
    }
}
