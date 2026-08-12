/* REJECTED — MoveImage, grind session 4 (permuter modality).
 *
 * WHY THIS IS DEAD, in one line: the ONLY score class the permuter found below
 * either base is a semantically-TRUNCATING narrow read of the dispatch argument,
 * and in the real build it changes exactly one opcode (`lw` -> `lhu`/`lh`) without
 * moving a single instruction's slot.  It is neither a match nor a lever.
 *
 * Provenance.  ws_a (tmp/grind/MoveImage/s4/ws_a) was a decomp-permuter random
 * campaign seeded from the floor-2 candidate (permuter base score 225).  In
 * 52,580 iterations over ~42 minutes it produced exactly two outputs below its
 * base, both at score 210, and both were the same transformation spelled two
 * ways: stage `p[6]` through a NARROW local before passing it.
 *
 *   output-210-1:  unsigned short nv;  ... return fn(nv = p[6], ...);
 *   output-210-2:  short nv;           ... nv = p[6]; ... return fn(nv, ...);
 *
 * Measurement (sweep12, tmp/grind/MoveImage/s4/sweep12_results.md, all against
 * `sandbox MoveImage --disable all` with the real whole-TU build):
 *
 *   P0 base (s32, `fn(p[6], ...)`)                     2 / 49
 *   P1 `unsigned short nv; fn(nv = p[6], ...)`         2 / 49
 *   P2 `short nv; nv = p[6]; fn(nv, ...)`              2 / 49
 *   P3 `s32 nv;  nv = p[6]; fn(nv, ...)`               2 / 49   (byte-identical to P0)
 *   P4 `fn = (s32 (*)(u16,s32,s32,s32))p[2];`          2 / 49
 *
 * Objdump diff of every narrow form against P0 is ONE line:
 *      508:  8c640018   lw   a0,24(v1)          <- P0 / P3 / target
 *      508:  94640018   lhu  a0,24(v1)          <- P1, P4
 *      508:  84640018   lh   a0,24(v1)          <- P2
 * The instruction's SLOT does not move; only its width does, and target uses the
 * full word.  So the permuter's 210 is purely an artifact of its own weighted
 * scorer (a differing opcode at the same index is cheaper than the 4-slot
 * displacement it is standing in for), not a step toward the match.
 *
 * It is also semantically wrong: `g_gpu_dev_table[6]` is a 32-bit value and the
 * narrow stage truncates it, so the form could not be committed even if it had
 * matched.  It additionally fails cheat-checklist T1/T2 (a temporary whose only
 * purpose is to change the load's width is not something a human writes from the
 * function's specification).  Recorded here so no future session re-derives it
 * from the permuter output dirs.
 *
 * The wide staging (P3) is codegen-INERT — byte-identical to the base — which
 * confirms the effect belongs entirely to the narrowing, not to the temporary.
 */
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    unsigned short nv; /* REJECTED: truncates a 32-bit dev-table word */
    s32 src;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    rect = arg0;
    src = *rect++;
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = *rect;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(nv = p[6], (s32)bf24 - 8, 0x14, 0);
}
