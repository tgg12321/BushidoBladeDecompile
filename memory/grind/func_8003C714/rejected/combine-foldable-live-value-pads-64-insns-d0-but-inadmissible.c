/* REJECTED (s14, 2026-09-05) - INADMISSIBLE, banked as a REACHABILITY PROOF only.
 *
 * sandbox func_8003C714 --disable all = score 0, target_insns 104, build_insns 104
 * on the SHIPPED chassis (HEAD 2026-09-05). This is the SECOND independent
 * distance-0 form for this function (the first is
 * rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c, s6) and it is free
 * by a COMPLETELY DIFFERENT mechanism.
 *
 * MECHANISM (s14 H19). The 32 `v ^= C; v ^= C;` pairs are ordinary SImode
 * arithmetic on a LIVE local whose only consumer is the QImode store
 * `dst[0x24] = v;`. cse1 does not value-number them away (it performs no
 * algebraic cancellation), so all 64 insns are present in the RTL stream when
 * loop_optimize runs and count_loop_regs_set counts them: the .loop dump goes
 * from "57 real insns" (k=0) to "121 real insns" (k=32). At insn_count 121 the
 * 0x91A2B3C5 movable fails loop.c:1631 (threshold 119 >= 121 is false) and
 * prints "not desirable", so the magic stays in the loop in the target's split
 * lui / lw / ori / mult form and every register seat lands. AFTER loop_optimize,
 * combine's force_to_mode (tools/gcc-2.7.2/combine.c:5682) erases every one of
 * the 64 insns, because none of them can affect the 8 bits the store keeps -
 * emitted asm is byte-identical to the k=0 body at every k <= 31 and one
 * instruction SHORTER at k=32 (the hoist that disappeared).
 *
 * BOUNDARY (measured, s14): k=30 -> insn_count 117 (moved), k=31 -> 119 (moved),
 * k=32 -> 121 (not desirable). threshold is exactly 119, confirming s11 H17.
 *
 * BREADTH (measured, s14): the channel is not xor-specific. `v += C; v -= C;`,
 * `v &= 0xFF; v &= 0xFF;` and `v = v << 3; v = v >> 3;` all measure +2
 * insn_count per pair at +0 emitted instructions and all flip the movable at
 * k=32. This also re-scopes s3 K10 ("byte-neutral masks fold before loop"):
 * masks fold when they sit on the DIVIDED value, but not when they sit on a
 * live local ahead of a narrower store.
 *
 * WHY IT IS INADMISSIBLE. The freeness IS the inertness. force_to_mode deletes
 * exactly those computations whose results cannot reach the stored bits, so any
 * carrier this channel makes free is by construction unobservable: cheat-test T1
 * (semantic purpose) fails and T2 (human-programmer) fails. No frozen SOTN family
 * covers in-loop inert arithmetic. NOT SUBMITTED.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 v;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        v = *src;
        v ^= 0x1001;
        v ^= 0x1001;
        v ^= 0x1002;
        v ^= 0x1002;
        v ^= 0x1003;
        v ^= 0x1003;
        v ^= 0x1004;
        v ^= 0x1004;
        v ^= 0x1005;
        v ^= 0x1005;
        v ^= 0x1006;
        v ^= 0x1006;
        v ^= 0x1007;
        v ^= 0x1007;
        v ^= 0x1008;
        v ^= 0x1008;
        v ^= 0x1009;
        v ^= 0x1009;
        v ^= 0x100A;
        v ^= 0x100A;
        v ^= 0x100B;
        v ^= 0x100B;
        v ^= 0x100C;
        v ^= 0x100C;
        v ^= 0x100D;
        v ^= 0x100D;
        v ^= 0x100E;
        v ^= 0x100E;
        v ^= 0x100F;
        v ^= 0x100F;
        v ^= 0x1010;
        v ^= 0x1010;
        v ^= 0x1011;
        v ^= 0x1011;
        v ^= 0x1012;
        v ^= 0x1012;
        v ^= 0x1013;
        v ^= 0x1013;
        v ^= 0x1014;
        v ^= 0x1014;
        v ^= 0x1015;
        v ^= 0x1015;
        v ^= 0x1016;
        v ^= 0x1016;
        v ^= 0x1017;
        v ^= 0x1017;
        v ^= 0x1018;
        v ^= 0x1018;
        v ^= 0x1019;
        v ^= 0x1019;
        v ^= 0x101A;
        v ^= 0x101A;
        v ^= 0x101B;
        v ^= 0x101B;
        v ^= 0x101C;
        v ^= 0x101C;
        v ^= 0x101D;
        v ^= 0x101D;
        v ^= 0x101E;
        v ^= 0x101E;
        v ^= 0x101F;
        v ^= 0x101F;
        v ^= 0x1020;
        v ^= 0x1020;
        dst[0x24] = v;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
