/* REJECTED (grind session 8, rederive modality, 2026-08-03)
 *
 * WHY THIS IS DEAD: a fresh m2c re-derivation of asm/funcs/func_80060E38.s produces
 * this body. It is NOT a new C shape with a new codegen lever - it is the
 * declared-dead-locals shape, twice over:
 *
 *   1. m2c models the target's nine spill slots literally, as nine dead s32 locals
 *      named for the TARGET offsets it read (sp0, sp8, sp10, sp18, sp20, sp28, sp30,
 *      sp38, sp40 - i.e. 0,8,16,24,32,40,48,56,64). Those locals are written once and
 *      never read. That is exactly the forbidden dead-vars / frame-coercion family
 *      ([[dead-vars-local-array]]), and the cheat-invisible sandbox strips it before
 *      scoring, so it could not move the floor even if it were allowed.
 *   2. Even taken as an honest C proposal, session 2 already MEASURED this shape:
 *      rejected/declared-locals-give-stride-4-not-target-stride-8.c. Declared s32
 *      locals take assign_stack_local's align == 0 path, which drops the +4
 *      bigend_correction but ALSO drops the size rounding, so they land at stride 4,
 *      not target's stride 8. The two properties target needs (stride 8 AND
 *      congruence 0) are mutually exclusive for a 4-byte value in this fork.
 *
 * The only other difference from the accepted body is statement order: m2c emits
 * `D_800A34A0 = 0x1F800064;` late, between the two indirect scratchpad stores, rather
 * than in table order. That is a pure reorder, and reordering was measured dead on
 * this function in session 2 (11 variants) and again in sessions 4-5 (~159.5k permuter
 * mutations): rejected/structural-reorder-and-scope-cannot-move-spill-congruence.c.
 * The accepted body already reproduces target's instruction stream 139/139 exactly,
 * so there is nothing for a reorder to buy.
 *
 * Command that produced it (artifact: tmp/grind/func_80060E38/s8/m2c_out.c):
 *   cd tools/m2c && python3 -m m2c.main --target mipsel-gcc-c --valid-syntax \
 *       ../../asm/funcs/func_80060E38.s
 */
void func_80060E38(s32 arg0, s32 arg1) {
    s32 sp0;
    s32 sp8;
    s32 sp10;
    s32 sp18;
    s32 sp20;
    s32 sp28;
    s32 sp30;
    s32 sp38;
    s32 sp40;

    sp0 = 0x1F800018;
    sp8 = 0x1F800020;
    sp10 = 0x1F800030;
    sp18 = 0x1F800050;
    sp20 = 0x1F800058;
    sp28 = 0x1F80005C;
    sp30 = 0x1F800060;
    sp38 = 0x1F800062;
    sp40 = 0x1F800064;
    D_800A3468 = 0x1F800000;
    D_800A346C = 0x1F800018;
    D_800A3470 = 0x1F800020;
    D_800A3474 = 0x1F800030;
    D_800A3488 = 0x1F800050;
    D_800A3490 = 0x1F800058;
    D_800A3494 = 0x1F80005C;
    D_800A3498 = 0x1F800060;
    D_800A349C = 0x1F800062;
    D_800A34A4 = 0x1F800066;
    D_800A34A8 = 0x1F800068;
    D_800A34AC = 0x1F80006A;
    D_800A34B0 = 0x1F80006C;
    D_800A34B4 = 0x1F800070;
    D_800A34B8 = 0x1F800074;
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
    D_800A34A0 = 0x1F800064;
    *(s32 *)0x1F800008 = arg1;
}
