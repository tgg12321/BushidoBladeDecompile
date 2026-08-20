// CANDIDATE — s2 (structural, 2026-08-20). *** MATCH: sandbox --disable all = 0 ***
// (score 0, target_insns 51, build_insns 51, rules_dropped 0), measured in the REAL TU
// with this body in place of the INCLUDE_ASM at src/ings.c:161.
//
// THE WHOLE FUNCTION WAS ONE LOOP SPELLING. Every prior session (s0 WIP, s1, s1b) wrote
// the read loop as an explicit `i = 0; if (count > 0) { do { ... i += 1; } while (i < count); }`,
// which compiles to frame 40 (vars=0, regs=6/0, args=16) and leaves exactly 14 frame-offset
// diffs (2x addiu sp, 6x sw, 6x lw) against the target's frame 48 — all 51 body instructions
// already matched. Those sessions then treated "the target reserves 24 bytes at sp+0..23 that
// its body never touches" as proof that the 1998 source contained a DEAD object, and split the
// remaining search space into two owner-refused families: (a) a dead 1-8 byte local (vars axis),
// (b) a deleted >=5-arg call inflating outgoing_args (args axis). Both were byte-proven, both
// REFUSED (docs/grind/decisions.md:1985, standing ruling 2026-07-28, option (c) granted).
//
// That partition was WRONG, and this is the third door it missed: writing the loop as the plain
// `for (i = 0; i < count; i++)` a human would write makes GCC 2.7.2 reserve vars=8 with NO
// declared object at all — the phantom-frame artifact of memory/project/phantom-frame-slots-gcc272.md
// (GCC allocates a stack temp during RTL expansion of the strength-reduced induction variable,
// get_frame_size() counts it, and the allocator then keeps the value in a register so no store is
// ever emitted). Measured with cc1's own `.frame` comment:
//   do-while form  : .frame $sp,40 # vars= 0, regs= 6/0, args= 16   -> sandbox 14
//   for form       : .frame $sp,48 # vars= 8, regs= 6/0, args= 16   -> sandbox 0
// The two cc1 outputs are instruction-for-instruction identical apart from the frame offsets and
// label numbering (tmp/grind/file_LoadSectors/s2/sweep/, v_base.s vs v_for_loop.s).
//
// CONSTRUCTS: none. No dead local, no dead call, no volatile, no alias, no FAKE annotation,
// no sanctioned-family claim needed. `for (i = 0; i < count; i++)` is the natural spelling of
// "read `count` sectors" and would be written by any programmer from the specification alone.
s32 file_LoadSectors(s32 a0, u8 *dest, s32 sector, s32 count) {
    s32 fd;
    s32 i;

    fd = func_80083698(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    func_800836C8(fd, sector << 11, 0);
    for (i = 0; i < count; i++) {
        if (bios_FileRead(fd, dest, 0x800) != 0x800) {
            close(fd);
            return -1;
        }
        dest += 0x800;
    }
    func_800836B8(fd);
    return count << 11;
}
