/* REJECTED FORM — func_80060E38, grind session 4 (permuter), 2026-08-03.
 *
 * SLUG MEANING: a randomized machine search over C spellings (decomp-permuter,
 * ~94,554 mutations across two structurally distinct chassis) cannot move the
 * reload-spill-slot congruence off 4 mod 8, so it cannot beat the floor of 18.
 *
 * WHAT WAS TRIED
 * --------------
 * The body below (chassis 2, "base-arith-chassis") is representative of the space
 * searched: the same 32 scratchpad-pointer assignments written as offsets off a
 * `spbase = 0x1F800000` local instead of 32 independent absolute constants. It was
 * chosen as a SECOND permuter chassis precisely because it gives the mutator a
 * different neighbourhood (temporaries, addends, re-association) than the s1
 * absolute-constant probe.
 *
 * MEASUREMENTS
 * ------------
 * 1. Compiled through the real pipeline it yields 139 instructions — the same count
 *    as target and as the current source — with spill offsets 4,12,20,28,36,44,52,
 *    60,68($sp). GCC folds `spbase + 0xNN` straight back into absolute constants, so
 *    the arithmetic spelling is codegen-inert. Target needs 0,8,...,64.
 * 2. Permuter campaign on this chassis (label base-arith-chassis, -j 6, base_score 72
 *    with --stack-diffs): 40,003 iterations, ZERO finds.
 * 3. Permuter campaign on the s1 absolute-constant chassis (label probe-chassis,
 *    -j 8, base_score 72): 54,551 iterations, ZERO finds.
 * Combined: ~94,554 randomized C forms, not one output below the 72 baseline.
 *
 * WHY IT IS DEAD (mechanism, not just observation)
 * ------------------------------------------------
 * The nine slots are reload spill slots allocated by alter_reg via
 * assign_stack_local(mode, total_size, -1). Session 2's closed form, re-derived in
 * session 3, is
 *     offset === -GET_MODE_SIZE(M) (mod 8)
 * for a spilled pseudo of mode M, because the align == -1 path rounds the slot size
 * to 8 and function.c:702-703 then applies bigend_correction = size - GET_MODE_SIZE(M)
 * under our fork's BYTES_BIG_ENDIAN == 1. The congruence is a function of the spilled
 * pseudo's MODE alone; no property of the C source appears in it. A search over C
 * spellings is therefore searching a space in which every point has the same value —
 * which is exactly what 94.5k samples showed.
 *
 * DO NOT re-run a permuter campaign on this function, and do NOT run raw permuter.py
 * on it at all: the default scorer normalizes sp-relative offsets away and would
 * report score 0 (a false match) on a source that is honestly 18 away. Only
 * tools/permuter_campaign.py, which passes --stack-diffs.
 *
 * Chassis and logs: tmp/grind/func_80060E38/s4/{mkws.sh,mkws2.sh,ws,ws2}.
 */
void func_80060E38(s32 arg0, s32 arg1) {
    s32 spbase = 0x1F800000;
    D_800A3468 = spbase + 0x0;
    D_800A346C = spbase + 0x18;
    D_800A3470 = spbase + 0x20;
    D_800A3474 = spbase + 0x30;
    D_800A3488 = spbase + 0x50;
    D_800A3490 = spbase + 0x58;
    D_800A3494 = spbase + 0x5C;
    D_800A3498 = spbase + 0x60;
    D_800A349C = spbase + 0x62;
    /* … the remaining assignments follow the same shape; the full generated file is
       tmp/grind/func_80060E38/s4/base2.c (and s4/ws2/base.c). Only the SHAPE matters
       for this rejection: every spelling of these 32 constants, arithmetic or literal,
       compiles to the same 139 instructions with spill slots at 4 mod 8. */
    *(s32 *)0x1F800004 = arg0;
    *(s32 *)0x1F800008 = arg1;
}
