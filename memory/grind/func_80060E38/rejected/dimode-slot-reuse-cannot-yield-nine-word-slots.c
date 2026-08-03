/* REJECTED — grind session 3 (structural, 2026-08-03), func_80060E38.
 *
 * THE IDEA
 * Session 2's closed-form proof (reload1.c:2337-2410 + function.c:666-727) showed that
 * every freshly-allocated reload spill slot of a 4-byte mode lands at an offset
 * congruent to 4 (mod 8) under our BYTES_BIG_ENDIAN cc1, and left exactly ONE opening:
 * the slot-REUSE branch at reload1.c:2363-2367,
 *
 *     else if (spill_stack_slot[from_reg] != 0
 *              && spill_stack_slot_width[from_reg] >= total_size
 *              && GET_MODE_SIZE (GET_MODE (spill_stack_slot[from_reg])) >= inherent_size)
 *       x = spill_stack_slot[from_reg];
 *
 * which returns a previously-allocated slot's address with adjust == 0, skipping
 * assign_stack_local's bigend_correction entirely. If the pre-existing slot was created
 * for an 8-byte-mode pseudo (bigend_correction = 8 - 8 = 0, hence base at 0 mod 8), a
 * later SImode pseudo reusing it would be re-MEMed at that base by reload1.c:2410-2415
 * and land at 0 mod 8 — target's congruence.
 *
 * This file is the representative C shape that was supposed to trigger it: an 8-byte-mode
 * value (long long) live alongside SImode temps in a call-free (leaf) function, so that
 * global alloc puts both in caller-save hard registers and reload's spill-register
 * selection (spill_hard_reg, reload1.c:2253/3499) commandeers one that holds both, in
 * turn — the only way from_reg != -1 is ever reached.
 *
 * WHY IT IS DEAD — two independent measurements
 *
 * 1. IT NEVER FIRES FROM C. 23 compiled variants (tmp/grind/func_80060E38/s3/gen3.py:
 *    v_di9, v_dihalf, v_reload, v_di_reload, v_union, v_dbl9, v_maxpress;
 *    reuse_probe.py: v_scoped_*; reuse_probe2.py: v_leafA_{1,2,3}_{0,4,8} and
 *    v_leafB_{1,2,4}_{4,10}) were compiled with the real build flags and their sp
 *    offsets classified: an offset counts as a WORD slot only if a single-word sw/lw
 *    uses it and its +4 neighbour is unused. Across all 23, the number of word slots at
 *    0 mod 8 that were spills is ZERO. Every SImode spill sat at 4, 12, 20, ... — even
 *    in the shapes where the .greg dump proves reload DID commandeer the long long's
 *    register pair ("Spilling reg 8." / "9.", "24." / "25.", "16." / "17."). What
 *    lands at 0 mod 8 in those dumps is the DImode slot itself, and it is always read
 *    and written as TWO words (offsets O and O+4), never as the single sw/lw target uses.
 *
 * 2. EVEN IF IT FIRED, IT CANNOT PRODUCE TARGET. spill_stack_slot is indexed by hard
 *    register, so the reuse branch yields at most ONE 0-mod-8 slot per commandeered hard
 *    register, and each such register must ALREADY carry a spilled 8-byte-mode pseudo.
 *    func_80060E38's target needs NINE distinct slots at 0, 8, ..., 64 — i.e. nine
 *    commandeered hard registers, each with its own prior long long / double pseudo.
 *    Target's instruction stream is 139 instructions of lui/ori/sw/gp-relative stores
 *    with not one 8-byte-mode operation in it, and each such pseudo mandates at least a
 *    two-word spill store plus its own materialisation. Any C that supplies the nine
 *    8-byte-mode pseudos necessarily leaves the 139/139 instruction alignment that the
 *    current body already achieves. The route is arithmetically incompatible with the
 *    target, independently of whether it can be triggered at all.
 *
 * Do not re-propose long long / double / union mode-mixing, extra register pressure, or
 * "make reload spill a hard register" shapes for this function. See evidence.md
 * (session 3) and hypotheses.md.
 */

typedef int s32;

extern s32 D_800A3468, D_800A346C, D_800A3470, D_800A3474, D_800A3480;
extern s32 D_800A3484, D_800A3488, D_800A348C, D_800A3490, D_800A3494;
extern s32 D_800A3498, D_800A349C, D_800A34A0, D_800A34A4, D_800A34A8;
extern s32 D_800A34AC, D_800A34B0, D_800A34B4, D_800A34B8, D_800A34BC;
extern s32 D_800A34C0, D_800A34C4, D_800A34C8, D_800A34CC, D_800A34D0;
extern s32 D_800A34D4, D_800A34D8, D_800A34DC, D_800A34E0, D_800A34E4;
extern s32 D_800A34E8, D_800A34EC;

void func_80060E38(s32 arg0, s32 arg1) {
    /* the 8-byte-mode pseudo whose spill slot the SImode spills were meant to reuse */
    long long L0 = (long long)arg0 * 7 + arg1;
    long long L1 = (long long)arg0 * 8 + arg1;
    s32 x0, x1, x2, x3;
    D_800A3468 = 0x1F800000;
    D_800A346C = 0x1F800018;
    D_800A3470 = 0x1F800020;
    D_800A3474 = 0x1F800030;
    D_800A3488 = 0x1F800050;
    D_800A3490 = 0x1F800058;
    D_800A3494 = 0x1F80005C;
    D_800A3498 = 0x1F800060;
    D_800A349C = 0x1F800062;
    D_800A34A0 = 0x1F800064;
    D_800A34A4 = 0x1F800066;
    D_800A34A8 = 0x1F800068;
    D_800A34AC = 0x1F80006A;
    D_800A34B0 = 0x1F80006C;
    D_800A34B4 = 0x1F800070;
    D_800A34B8 = 0x1F800074;
    /* the long longs die here; the SImode temps below were meant to inherit their
       hard registers, and with them their 0-mod-8 spill slots */
    *(long long *)0x1F800100 = L0;
    *(long long *)0x1F800108 = L1;
    x0 = arg0 * 11 + arg1;
    x1 = arg0 * 12 + arg1;
    x2 = arg0 * 13 + arg1;
    x3 = arg0 * 14 + arg1;
    D_800A34BC = 0x1F800080;
    D_800A34C0 = 0x1F800082;
    D_800A34C4 = 0x1F800084;
    D_800A34C8 = 0x1F800088;
    D_800A34CC = 0x1F80008C;
    D_800A34D0 = 0x1F800090;
    D_800A34D4 = 0x1F800098;
    D_800A34D8 = 0x1F80009A;
    D_800A34DC = 0x1F80009C;
    D_800A34E0 = 0x1F80009E;
    D_800A34E4 = 0x1F8000A0;
    D_800A34E8 = 0x1F8000A4;
    D_800A3480 = 0x1F8000A8;
    D_800A3484 = 0x1F8000AC;
    D_800A348C = 0x1F8000B0;
    D_800A34EC = 0x1F8000B8;
    *(s32 *)0x1F800004 = arg0;
    *(s32 *)0x1F800008 = arg1;
    *(s32 *)0x1F800120 = x0;
    *(s32 *)0x1F800124 = x1;
    *(s32 *)0x1F800128 = x2;
    *(s32 *)0x1F80012C = x3;
}
