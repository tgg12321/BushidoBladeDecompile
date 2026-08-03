/* REJECTED (grind session 2, structural) — statement-order / block-scope forms.
 *
 * Representative of a family of 10 measured variants (v_rev shown; v_scope, v_argsfirst,
 * v_interleave, v_more, v_half, v_ll, v_llarr, v_dbl, v_dipress all likewise). Every one
 * of them emits its reload spill slots at offsets congruent to 4 mod 8 (4,12,20,...),
 * exactly like the shipped form; the target needs 0,8,16,...,64.
 *
 * WHY IT IS DEAD: the congruence is not a function of the C at all. From
 * reload1.c:2337-2410 and function.c:666-727, a spilled pseudo of mode M lands at
 *     STARTING_FRAME_OFFSET + 8k + (CEIL_ROUND(total_size,8) - GET_MODE_SIZE(M)),
 * and STARTING_FRAME_OFFSET is always a multiple of 8, so the offset is congruent to
 * -GET_MODE_SIZE(M) mod 8. For any 4-byte mode that is 4, unconditionally. Statement
 * order, scoping, declaration order, spill count and mixing in long long / double
 * values cannot touch it. Do not re-propose any reordering / re-scoping variant.
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
    D_800A34EC = 0x1F8000B8;
    D_800A348C = 0x1F8000B0;
    D_800A3484 = 0x1F8000AC;
    D_800A3480 = 0x1F8000A8;
    D_800A34E8 = 0x1F8000A4;
    D_800A34E4 = 0x1F8000A0;
    D_800A34E0 = 0x1F80009E;
    D_800A34DC = 0x1F80009C;
    D_800A34D8 = 0x1F80009A;
    D_800A34D4 = 0x1F800098;
    D_800A34D0 = 0x1F800090;
    D_800A34CC = 0x1F80008C;
    D_800A34C8 = 0x1F800088;
    D_800A34C4 = 0x1F800084;
    D_800A34C0 = 0x1F800082;
    D_800A34BC = 0x1F800080;
    D_800A34B8 = 0x1F800074;
    D_800A34B4 = 0x1F800070;
    D_800A34B0 = 0x1F80006C;
    D_800A34AC = 0x1F80006A;
    D_800A34A8 = 0x1F800068;
    D_800A34A4 = 0x1F800066;
    D_800A34A0 = 0x1F800064;
    D_800A349C = 0x1F800062;
    D_800A3498 = 0x1F800060;
    D_800A3494 = 0x1F80005C;
    D_800A3490 = 0x1F800058;
    D_800A3488 = 0x1F800050;
    D_800A3474 = 0x1F800030;
    D_800A3470 = 0x1F800020;
    D_800A346C = 0x1F800018;
    D_800A3468 = 0x1F800000;
    *(s32 *)0x1F800004 = arg0;
    *(s32 *)0x1F800008 = arg1;
}
