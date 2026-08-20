s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 _pad[2];
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    func_800836C8(fd, sector << 11, 0);
    i = 0;
    if (count > 0) {
        do {
            if (bios_FileRead(fd, dest, 0x800) != 0x800) {
                close(fd);
                return -1;
            }
            i += 1;
            dest += 0x800;
        } while (i < count);
    }
    func_800836B8(fd);
    return count << 11;
}
