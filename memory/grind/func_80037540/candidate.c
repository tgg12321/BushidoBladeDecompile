/* func_80037540 — BEST FORM (byte-correct, sandbox --disable all == 0)
 *
 * STATUS: byte-correct but NOT cleared, and NOT the answer. This is the form on
 * main. It carries the flagged unwritten-tail frame reservation (sp[6]/sp[7]
 * never written). Do NOT treat as COMPLETED-C. Do NOT commit it as done.
 *
 * ############################################################################
 * # s5 (2026-07-13): THE SEARCH SPACE IS **NOT** CLOSED. The prior "proof"    #
 * # that this function REQUIRES an unwritten-tail array is FALSE. Do not      #
 * # trust the s2/s3/s4 header that used to sit here (it is preserved, with    #
 * # its refutation, in evidence.md + hypotheses.md).                          #
 * ############################################################################
 *
 * WHAT WAS BELIEVED (s3/s4, and adopted by the Judge as BINDING):
 *   The frame equation (verified — mips.c:compute_frame_size) gives
 *       frame = ALIGN8(get_frame_size) + ALIGN8(args=16) + ALIGN8(gp=24)
 *       => frame 0x48  <=>  get_frame_size in [25,32].            [STILL TRUE]
 *   The target writes exactly 24 bytes (six `sw`, 0x10..0x24).    [STILL TRUE]
 *   s4 then argued a "trichotomy": to hold frame bytes an object must defeat
 *   scalar-promotion => >=2 elements => >=2 EXTRA STORES; so any 0x48 form
 *   either emits stores the target lacks (+2, measured) or declares an
 *   unwritten tail. Hence "the original itself declared a dead tail", hence
 *   the item is a pure policy question for the owner.          [** FALSE **]
 *
 * WHAT IS ACTUALLY TRUE (s5, measured):
 *   GCC 2.7.2 reserves locals frame bytes for ORDINARY LIVE LOCALS that no
 *   instruction ever touches — a PHANTOM SLOT. There is no dead declaration,
 *   no unwritten-tail array, and no cheat involved. The third branch s4's
 *   trichotomy omitted is simply: **get_frame_size > bytes-written, from
 *   ordinary live code.**
 *
 *   WITNESS (in-tree, byte-verified, no cheat): `tslLineG5Init`
 *   (src/code6cac_c2.c:1267) — COMPLETED-C, `sandbox --disable all` = 0,
 *   rules_dropped = 0, not in the queue. cc1 reports for it:
 *       .frame $sp,48,$31   # vars= 8, regs= 4/0, args= 24, extra= 0
 *   get_frame_size() == 8, and the ONLY sp-relative accesses in the whole
 *   function are the 4 register saves (32..44) and `sw $2,16($sp)` (a 5th
 *   outgoing arg, inside the args region). NOTHING touches [24,32). Eight
 *   reserved bytes, never written, never read — from five plain scalar locals.
 *   The original binary has the same frame (asm/funcs/tslLineG5Init.s: -0x30).
 *
 *   MECHANISM (minimal reproducer: tmp/grind/func_80037540/s2/minrepro.py):
 *   two HImode (`s16`) locals feeding an HImode BITWISE expression
 *   (`(a & ~b) & 1`) make cc1 allocate a stack temp for the HImode computation
 *   which it then never uses => vars=8, untouched. (A call is not required.)
 *
 * CONSEQUENCE: this function does NOT need an owner carve-out for the
 * unwritten-tail family, and it is NOT blocked. It needs the LIVE-LOCALS form
 * whose get_frame_size lands in [25,32] while emitting only the target's six
 * stores. That form has not been found yet — but the space is open and now has
 * a direct gradient (below), not a policy wall.
 *
 * THE INSTRUMENT (use this, not the sandbox, to search):
 *   cc1 prints get_frame_size() itself. Compile any candidate with the project
 *   flags and read the `.frame` comment:
 *     bash tmp/grind/func_80037540/s2/build.sh <file.c>
 *     -> .frame $sp,N,$31  # vars= V, regs= R, args= A, extra= E
 *   V IS get_frame_size(). Search for V in [25,32] with no added stores. This is
 *   a far better gradient than the sandbox score for this function, which only
 *   reports the aggregate distance.
 *
 * ALREADY RULED OUT for the live-locals hunt (measured, all vars=24):
 *   s16 / u16 / u8 index locals (incl. an s16-returning func_80036EA8, which
 *   makes the sext free), three s16 locals, both indices live across the 2nd
 *   call, a long long pair. See rejected/s16-index-locals-do-not-port-phantom-slot.c
 *   — func_80037540 has no HImode bitwise semantics, so no temp is created.
 *
 * ALSO NOTE (s5): src/code6cac_c2.c:1320, the sibling func_8003DBE4, ships
 *   `s32 buf[2]; /* dead local: reserves the target's 8 extra frame bytes */`
 *   on main. That is the forbidden unwritten-array construct, in a function
 *   still ACTIVE in the queue at distance 21 — so it is NOT precedent for
 *   acceptance; it is an open cheat that the phantom-slot mechanism may well
 *   dissolve. Flagged, not exploited.
 */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
