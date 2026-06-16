// CANDIDATE v9 — score=1, frame=48
// ONE extra instruction: sw v0,16(sp) before epilogue
// GCC 2.7.2 NEVER DCEs dead stores to local arrays → score cannot reach 0 with any USED array form
// Only score=0 form is an unused aggregate (cheat-reviewer FAIL, see rejected/)
// Cheat-reviewer NOT run on this candidate (score=1; needs-decision pending)
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;
    s32 result[2];

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    ang_hosei(fd, sector << 11, 0);
    i = 0;
    if (count > 0) {
        do {
            if (bios_FileRead(fd, dest, 0x800) != 0x800) {
                bios_FileClose_B(fd);
                return -1;
            }
            i += 1;
            dest += 0x800;
        } while (i < count);
    }
    md_gview_init(fd);
    result[0] = count << 11;
    return result[0];
}
