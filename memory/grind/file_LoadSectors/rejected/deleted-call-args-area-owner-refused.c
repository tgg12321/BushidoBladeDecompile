// CANDIDATE s1b — OWNER-GATED (escalation filed docs/grind/decisions.md 2026-07-28).
// Judge ruling 2026-07-28 08:12 FAILed the s1 duplicate-guard spelling and barred the
// whole deleted-call/args-area family absent owner sanction. s1b completed the mandated
// SOTN census (NEGATIVE on mechanism, genre if(0) precedent only) and proved the
// human-plausible spellings below are byte-identical to the sandbox-0 form:
//   if (0) { debug_printf(fd, sector, count, 0, 0); }        <- macro-shaped, this file
//   s32 dbg = 0; ... if (dbg) { debug_printf(...); }         <- debug-flag local variant
// Mechanism: expand_call bumps outgoing_args_size to 24 (5 args) before RTL jump-opt
// deletes the arm; bump is monotonic -> frame = 24 args + 24 regs + 0 vars = 48, saves
// at sp+24..44, body 51/51 byte-identical, zero relocs/rodata (SCALAR args only — a
// string-literal arg leaks .rodata even from the deleted arm; measured s1b).
// The ONLY other frame-48 mechanism is a dead 1-8-byte local (_pad[2] class, currently
// holding bytes on main; oversized-locals carve-out prerequisite-1 is ambiguous here
// because nothing in the bytes pins args=16 vs 24). Owner picks: (a) dead-local under
// carve-out, (b) this family narrowly, or (c) endgame-lock. DO NOT APPLY TO src/ WITHOUT
// THE OWNER RULING. Fallback clean form: v9 result[2] used-array, score 1 (evidence.md s0).
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    if (0) {
        debug_printf(fd, sector, count, 0, 0); /* FAKE: pending owner ruling — deleted
            5-arg call sets outgoing-args area to 24 (frame 48); see escalation entry */
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
