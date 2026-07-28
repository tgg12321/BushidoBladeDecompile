// CANDIDATE s1 — sandbox 0 on real TU (2026-07-28), PENDING RULING (unclassified construct)
// Construct: provably-dead guarded 5-arg call. GCC 2.7.2 expand_call bumps
// current_function_outgoing_args_size to 24 (5 args -> 20 -> ALIGN8 24) BEFORE
// RTL jump/cse deletes the arm (fd==-1 impossible after the early return).
// The bump is monotonic (never shrinks), so frame = 24 args + 24 regs + 0 vars = 48
// with saves at sp+24..44 — the exact target layout. Body 51 insns byte-identical;
// zero relocs / zero rodata from the deleted call (scalar args only — a string
// literal arg WOULD leak rodata even though the call is deleted).
// This resolves the old "target must have had an unused 8-byte local" inference:
// the untouched sp+16..23 bytes are OUTGOING-ARGS area, not vars.
// If the ruling refuses this family, fall back to v9 (result[2] used-array,
// score=1, see evidence.md s0) as best non-controversial form.
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    if (fd == -1) {
        debug_printf(fd, 0, 0, 0, 0);
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
