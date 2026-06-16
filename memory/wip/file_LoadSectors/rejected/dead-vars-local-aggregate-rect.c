// REJECTED: dead-vars-local-array cheat in struct-aggregate spelling
// Ruled FAIL by cheat-reviewer (2026-06-16), all 6 checklist tests failed.
//
// The only score=0 form found after exhausting all 5 Adjudicator-directed candidates.
// `Rect r;` is declared, never read, never written, never address-taken.
// Sole effect: GCC 2.7.2 unconditionally allocates 8 bytes for struct aggregates,
// inflating frame from 40 to 48 (matching target). This is the dead-vars-local-array
// cheat in struct-aggregate spelling -- same intent as rejected s32 _pad[2].
// The mechanical detector (volatile_cheats.find_unused_local_arrays) covers T name[N]
// array forms but not struct types; the INTENT standard (cheats-by-any-spelling) applies.
//
// Cheat-reviewer verdict: FAIL -- no semantic purpose, no human-programmer test pass,
// justification is entirely GCC-internal (struct aggregate slot allocation behavior),
// found by search not program logic, same family as dead-vars-local-array.
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    Rect r;       /* 8-byte struct: never read, never written */
    s32 i;

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
    return count << 11;
}
