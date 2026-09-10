/* func_8005C6D0 - per-frame sound-request flush (grind s2; honest sandbox distance 0,
 * 118/118 instructions, full-build SHA1 == oracle).
 *
 * Sweeps the 24-entry sound-request pool at D_800EFB78 (8 bytes per entry: +0 the s32
 * request pointer written by func_8005C650, +4 / +5 the two u8 volumes).  For every live
 * entry whose VAB slot D_800EFC38[*p] holds a KSEG0 pointer (negative as s32) it scans
 * the SPU voices, starting from where the previous key-on stopped (`next` deliberately
 * persists across pool entries), for the first voice SpuGetKeyStatus reports as not
 * keyed on; remaps VAB id 6 -> 3 while D_800EFC50 == D_800EFC44; unpacks the packed
 * note word (bits 0-6 prog, 7-10 tone, 11-17 note, 18-24 fine) and SsUtKeyOnV's it.
 * Every iteration ends by clearing the pool entry's +0 word.
 *
 * Spelling notes (all measured, s1 + s2 - see memory/grind/func_8005C6D0/evidence.md):
 *  - the pool and the two volume bytes are three INDEPENDENT symbols addressed as
 *    `(u8 *)&SYM + <byte offset>`, the same spelling the already-byte-matching writer
 *    func_8005C650 uses at src/text1b.c:2686-2689.  The target emits a separate
 *    lui %hi / addu $at / {lw,lbu,sw} %lo triple per symbol per use and never shares a
 *    base register between them; declaring the pool as an array instead makes loop.c
 *    hoist the symbol address into a callee-saved register (frame 0x60, 112 insns) -
 *    banked as rejected/array-decl-licm-hoist-frame-0x60.c.
 *  - `entry_off` (the pool byte offset) and `vol_off` (the volume byte offset, computed
 *    inside the voice-scan guard) are separate locals because folding them into one, or
 *    inlining either, changes three different GCC decisions: the LICM hoist above, the
 *    cross-jump of the loop-entry guard into the bottom test, and the recomputed
 *    sll/sra at the 4-way join.  Both single-local forms were re-measured on this
 *    chassis (105 and 99 insns) and are banked as rejected forms.
 *  - the inner scan is a top-tested `for` inside an explicit `if` guard: jump.c's
 *    duplicate_loop_exit_test rotation is what gives reorg.c a prediction > 0 on the
 *    `beq $v0,$s5` branch, so the branch delay slot is filled from the loop-continue
 *    thread with `addiu $v0,$s0,0x1` (as the target does) instead of stealing the
 *    `li $v0,6` constant out of the fall-through block.
 *  - `next = (s16)(voice + 1);` sits AFTER the SsUtKeyOnV call: the scheduler hoists it
 *    back above the call into the target's slot between the $a0 sign-extension and the
 *    $a1 shift.  Written before the call it is emitted one slot too early.
 */
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern void SpuGetAllKeysStatus(u8 *);
extern s32 SpuGetKeyStatus(s32);
extern s32 SsUtKeyOnV(s16, s16, s16, s16, s16, s16, s16, s16);
void func_8005C6D0(void) {
    extern s32 *D_800EFC38[];
    u8 keys[24];
    s16 i;
    s16 voice;
    s16 next;
    u16 vab;
    u16 *p;
    s32 vol_off;
    s32 entry_off;
    u32 *ev;

    SpuGetAllKeysStatus(keys);
    next = 0;
    for (i = 0; (s16)i < 0x18; i = (s16)(i + 1)) {
        entry_off = i * 8;
        p = *(u16 **)((u8 *)&D_800EFB78 + entry_off);
        if (p != 0 && (s32)D_800EFC38[*p] < 0) {
            voice = next;
            if ((s16)voice < 0x18) {
                vol_off = i * 8;
                for (; (s16)voice < 0x18; voice = (s16)(voice + 1)) {
                    if (SpuGetKeyStatus(1 << voice) != 1) {
                        vab = *p;
                        if (vab == 6 && D_800EFC50 == D_800EFC44) {
                            vab = 3;
                        }
                        ev = &((u32 *)D_800EFC38[vab][0])[p[1]];
                        SsUtKeyOnV((s16)voice, (s16)vab,
                                   (s16)(*ev & 0x7F),
                                   (s16)((*ev >> 7) & 0xF),
                                   (s16)((*ev >> 11) & 0x7F),
                                   (s16)((*ev >> 18) & 0x7F),
                                   *((u8 *)&D_800EFB7D + vol_off),
                                   *((u8 *)&D_800EFB7C + vol_off));
                        next = (s16)(voice + 1);
                        break;
                    }
                }
            }
        }
        *(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0;
    }
}
