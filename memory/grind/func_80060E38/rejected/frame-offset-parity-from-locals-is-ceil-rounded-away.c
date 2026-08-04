/* REJECTED — grind session 7 (forensics, 2026-08-03), func_80060E38.
 *
 * FORM: give the function a small amount of ordinary declared, address-taken local
 * storage so that GCC's running frame cursor (`frame_offset`) is left at an ODD
 * multiple of 4 at the moment reload begins allocating spill slots, in the hope that
 * the nine spill slots inherit that parity and land at 0,8,...,64 (target's congruence)
 * instead of 4,12,...,68.
 *
 * WHY IT LOOKED LIVE: assign_stack_local's align == 0 path (ordinary locals) uses
 * alignment = GET_MODE_ALIGNMENT(mode)/8 and does NOT round the slot size, so locals
 * really do advance frame_offset at stride 4 (s32) or 1 (char) — session 2's v_locals
 * measured exactly that. The closed form sessions 2 and 3 derived,
 *     offset = frame_offset + (CEIL_ROUND(size,8) - GET_MODE_SIZE(mode)) + STARTING_FRAME_OFFSET,
 * is only congruent to -GET_MODE_SIZE(mode) (mod 8) if frame_offset is 8-aligned on
 * entry, and no session had ever measured that it is. This was the last C-controlled
 * input to the formula, and unlike every previously-tried lever it did not require
 * changing the instruction stream: 4 bytes of locals leaves 108 -> 112 == 0x70, the
 * target's exact frame size, because the callee-save block already leaves 4 bytes of
 * MIPS_STACK_ALIGN slack.
 *
 * WHY IT IS DEAD: function.c:692-698 rounds the cursor BEFORE using it —
 *     frame_offset = CEIL_ROUND (frame_offset, alignment);
 * and the spill path (align == -1) has alignment = BIGGEST_ALIGNMENT/8 = 8. Any parity
 * the locals established is erased on the very first spill allocation.
 *
 * MEASURED: an instrumented assign_stack_local (tmp/grind/func_80060E38/s7/gcc, validated
 * byte-identical to stock build/cc1) on seven leaf variants of the s1 probe carrying
 * 1, 2, 3, 4, 5, 12 and 20 bytes of address-taken locals (probes/p_loc*.c). Traces show
 * fo_in=1 -> fo_rounded=8 and fo_in=3 -> fo_rounded=8; EVERY spill in EVERY variant is
 * still at 4 mod 8 (p_loc1/p_loc2/p_loc3x1: 12,20,...,76; p_loc12: 20,...,84;
 * p_loc20: 28,...,92). The whole-tree sweep agrees: of 317 align == -1 allocations in
 * Bushido Blade 2, only 2 entered with a non-8-aligned frame_offset and both were
 * rounded to 0 mod 8, and 131/131 four-byte-mode spill slots are at 4 mod 8.
 *
 * NOTE ON POLICY: even had it worked, a local declared purely to shift frame layout is
 * the forbidden dead-vars-local-array / frame-coercion family and is stripped by the
 * cheat-invisible sandbox. It was measured because the FORENSIC fact — whether the
 * congruence has any C-controlled input at all — is what the ledger and any future
 * escalation need; the answer is no.
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
    s32 lv;                                     /* 4 bytes -> frame_offset = 4 ... */
    *(volatile s32 *)0x1F800010 = (s32)&lv;     /* ... forced to memory, still a leaf */
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
    /* MEASURED spill slots: 12,20,28,36,44,52,60,68,76 — still 4 mod 8. */
}
