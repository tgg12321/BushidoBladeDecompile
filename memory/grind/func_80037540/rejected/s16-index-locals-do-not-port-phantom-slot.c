/* REJECTED (s5, 2026-07-13) — the phantom-slot mechanism does NOT port via
 * HImode index locals.
 *
 * CONTEXT: s5 proved (see evidence.md) that GCC 2.7.2 CAN reserve locals frame
 * bytes that no instruction touches, from ORDINARY LIVE LOCALS — witness
 * tslLineG5Init (COMPLETED-C, sandbox 0, 0 rules): get_frame_size()==8 with
 * zero accesses to its locals region. That kills s4's "trichotomy". So the hunt
 * became: find a LIVE-LOCALS form for func_80037540 with get_frame_size in
 * [25,32] and only the target's six stores.
 *
 * THIS FAMILY WAS THE FIRST GUESS AND IT FAILS. The two func_80036EA8() results
 * are camera-table indices, so declaring them s16/u16/u8 is semantically natural
 * and would add 4 bytes to get_frame_size (24 -> 28 -> ALIGN8 -> 32 = frame 0x48).
 * MEASURED with cc1's own `.frame ... # vars=` readout (== get_frame_size()):
 *
 *   form                                          vars   frame
 *   s32 sp[6]  (baseline)                          24    0x40
 *   s16 i0,i1  (+ s16-returning func_80036EA8)     24    0x40   <- NO frame bytes
 *   u16 i0,i1  (+ u16-returning func_80036EA8)     24    0x40   <- NO frame bytes
 *   u8  i0,i1                                      24    0x40   <- NO frame bytes
 *   s16 i0,i1 both live across the 2nd call        24    0x40   <- NO frame bytes
 *   three s16 (indices + the func_800392B8 offset) 24    0x40   <- NO frame bytes
 *   long long pair                                 24    0x40   <- NO frame bytes
 *
 * WHY: the phantom slot is NOT "two shorts exist". The minimal reproducer
 * (tmp/grind/func_80037540/s2/minrepro.py) shows the trigger is two HImode
 * locals feeding an HImode BITWISE expression (`(a & ~b) & 1`) — GCC 2.7.2
 * allocates a stack temp for the HImode computation and then never uses it
 * (vars=8, untouched; a call is not even required). func_80037540 has no HImode
 * bitwise semantics — its indices are only shifted (`idx * 8`), so no temp is
 * created and the locals stay in registers.
 *
 * => The mechanism is REAL and REOPENS the search space, but this particular
 *    port of it is dead. Do not re-propose s16/u16/u8 index locals.
 */
extern s16 func_80036EA8(s32, s32);   /* index-returning; s16 return costs 0 sext */
extern s32 func_800392B8(void);
extern void marionation_camera_Init_80037468(s32, s32 *, s32);

void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[6];
    s16 i0;
    s16 i1;
    s32 v0;

    i0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + i0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    i1 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + i1 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    marionation_camera_Init_80037468(6, sp, v0 + 0x7FC);
}
