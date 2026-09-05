/* REJECTED (s16, 2026-09-05) - mixed-operator loop-invariant chain.
 *
 * WHAT IT PROVES (this is why it is banked): s15's K24 recorded the
 * movable-ORDER dial as "capped at -6 because there are only two other
 * movables".  That cap is FALSE.  Extra movables can be manufactured at will:
 * a chain of loop-invariant locals joined by DISTINCT operators (* ^ + - )
 * is not constant-folded by cse1 the way s15's pure-xor chain (K44) was, so
 * each link becomes its own movable.  Measured on the shipped chassis
 * (HEAD 2026-09-05, tmp/grind/func_8003C714/s16/sweep.sh):
 *
 *   links  insn_count  moved  notdesirable  asm_lines
 *     0        56        3        0            107     (= candidate.c)
 *     1        58        5        0            111
 *     2        59        6        0            112
 *     4        62        9        0            115
 *     8        67       14        0            120
 *    10        70       17        0            123
 *    11        71       18        0            124
 *    12        73       18        2            126
 *    13        74       19        2            127
 *    14        75       20        2            128
 *    15        77       22        2            130
 *    16        78       23        2            131
 *
 * At 12 links the 0x91A2B3C5 magic (regno 112 in that dump, life 1,
 * move-insn, savings 1) prints "not desirable" - i.e. the target's in-loop
 * split lui/lw/ori/mult form IS reachable through the order dial alone,
 * with no inert padding at all.  The dial is therefore not capped; it is
 * PRICED.
 *
 * WHY IT IS REJECTED: every manufactured movable is hoisted into the
 * preheader and its insn stays there.  The measured price is 1.0-1.4 emitted
 * instructions per moved movable (asm_lines 107 -> 126 for 15 extra moved
 * movables), and the target's whole function is 104 instructions.  The
 * carrier is also inert arithmetic consumed by a rewritten
 * `D_800A37B8 = w11;` store, so it fails cheat-tests T1/T2 exactly like the
 * s6 biv noise and the s14 force_to_mode pad.  Diagnostic only, never
 * submittable.
 *
 * Also note the overshoot: at 12 links BOTH the &D_80106A58 symbol_ref and
 * the magic flip (notdesirable=2), but the target hoists the symbol_ref.
 * The one-flip window is 3 units wide and is reachable by pairing 11 links
 * (both still moved, insn_count 71 = threshold at the magic exactly) with
 * ONE unit of the free split-init channel (H23) - so the shape of a
 * distance-0 order-dial body is known; only its byte cost is fatal.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        w0 = (s32)s0 * 3;
        w1 = w0 ^ 0x1001;
        w2 = w1 + 7;
        w3 = w2 * 5;
        w4 = w3 ^ 0x2002;
        w5 = w4 - 11;
        w6 = w5 * 9;
        w7 = w6 ^ 0x4004;
        w8 = w7 * 3;
        w9 = w8 ^ 0x1001;
        w10 = w9 + 7;
        w11 = w10 * 5;
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = w11;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
