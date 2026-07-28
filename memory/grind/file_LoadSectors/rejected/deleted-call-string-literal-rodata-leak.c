typedef int s32;
typedef short s16;
typedef unsigned char u8;

extern s32 func_80083698(s32, s32, s32);
extern s32 ang_hosei(s32, s32, s32);
extern s32 bios_FileRead(s32, u8 *, s32);
extern s32 bios_FileClose_B(s32);
extern s32 md_gview_init(s32);
extern s32 printf5(char *, s32, s32, s32, s32);

s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    if (0) {
        printf5("file_LoadSectors: open error %d %d %d\n", fd, sector, count);
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
    return count << 11;
}
/* REJECTED s1b 2026-07-28: the format-string literal is emitted to .rodata even
   though the if(0) call is deleted (GCC 2.7.2 expands the arm to RTL first).
   Oracle has no such string => any deleted-call original had SCALAR-ONLY args.
   See tmp/grind/file_LoadSectors/s1/p2_if0_strlit.s. */
