/* REJECTED — func_80036FD4, session 2.
 *
 * This is session 1's score-9 form: the aggregate (BLKmode) copy is written to
 * a DISTINCT symbol (D_80101E6C) from the halfword global it must be ordered
 * against (D_80101E60).  It is dead as a route to the last instruction, and it
 * is dead for a reason that no reshaping of the record can fix.
 *
 * WHAT IT ACHIEVES: floor 17 -> 9, build_insns 76 -> 78 (target 79).  The
 * BLKmode destination flushes cse's memory table (cse.c:7539
 * note_mem_written -> writes->all), so both of target's reloads of
 * D_80101E60 come back.  That part is CONFIRMED and is preserved in the
 * session-2 candidate.
 *
 * WHY IT CANNOT REACH 79 INSTRUCTIONS (measured, cc1 -da dumps in
 * tmp/grind/func_80036FD4/s2/dump-cur/):
 *   - The copy IS ALREADY a single movstrsi block-move insn (dump insn 53:
 *     `(set (mem:BLK (symbol_ref "D_80101E6C")) (mem:BLK (reg 78)))`,
 *     pattern 177 {movstrsi_internal}).  Session 1's H5 assumed it took the
 *     move_by_pieces path and that forcing emit_block_move/movstrsi would
 *     block the hoist.  It does not: the block move is opaque, and sched2
 *     STILL sinks it past the reload.  H5 is KILLED at its premise.
 *   - sched2's trace: ";; ready list at T-14: 53 (1), now 53" then
 *     ";; launching 62 before 53 with no stalls at T-15".  Insn 53 is the only
 *     insn ready in the reload's load-delay slot, so it fills it — killing the
 *     nop (78 vs 79 insns) and making the reload's pseudo live across the
 *     block move, which pushes the movstrsi scratches to $a1/$a2 instead of
 *     target's $a0/$a1.  The missing nop and the register residual are ONE
 *     symptom of this single scheduling decision.
 *   - The only way to keep 53 out of that slot is a real memory dependence
 *     53 -> 62.  sched.c:817 true_dependence -> memrefs_conflict_p with
 *     SIZE_FOR_MODE(BLKmode) == 0 bottoms out at
 *     `rtx_equal_for_memref_p (x, y) && (xsize == 0 || ysize == 0 || ...)`.
 *     For two constant addresses that requires the SAME base symbol.  Two
 *     distinct symbol_refs return 0 unconditionally — no member layout, no
 *     declared alignment, no record size changes that.
 *
 * So the record's SHAPE was never the variable; its BASE was.  See
 * ../candidate.c for the shared-base form (floor 2).
 */

extern void tslPolyF4Init(s32, u8 *, s32);
typedef struct {
    s32 a;
    s32 b;
} CamPair;
s32 func_80036FD4(s32 arg0, s32 arg1) {
    s16 *s0 = &D_80101E62;

    if (*s0 != 0) {
        return 0;
    }

    {
        extern u8 SpecialCam;
        s32 *entry;

        D_80101E60 = arg0;
        entry = (s32 *)(&SpecialCam + D_80101E60 * 8);
        *(CamPair *)&D_80101E6C = *(CamPair *)entry;
    }

    {
        extern u8 SpecialCam;
        D_80101E74 = cdrom_BcdToFrames((s32)(&SpecialCam + D_80101E60 * 8)) + (*(u32 *)((u8 *)&D_8008EC38 + (D_80101E60 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E94 = 0;
        D_80101E90 = 5;
    } else {
        u8 *base = (u8 *)s0 - 0xA;
        D_80101E94 = 1;
        *base = 1;
        D_80101E59 = arg1;
        tslPolyF4Init(0xD, base, 0);
        D_80101E90 = 0xC8;
    }

    D_80101E64 = 0;
    D_80101E68 = 0;
    D_80101E6A = 0;
    D_80101E62 = 0x10;

    return 1;
}
