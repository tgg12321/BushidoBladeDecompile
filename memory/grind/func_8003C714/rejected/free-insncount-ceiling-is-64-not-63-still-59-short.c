/* REJECTED -- func_8003C714 s18 (rederive).  WHY IT IS DEAD:
 *
 * This is the highest byte-neutral loop.c insn_count an ORDINARY-C spelling of
 * this function reached in the s18 sweep: 64 counted insns (dump: "Loop from 25
 * to 172: 64 real insns"), asm_lines 107, sandbox score 15 on the shipped chassis
 * 2026-09-05.  It corrects s15's recorded ordinary-C ceiling of 63 by exactly one
 * insn: the extra free insn is the named u8 intermediate `w` between the record's
 * leading byte and the QImode store (`w = v; dst[0x24] = w;`), a register-to-
 * register copy that local-alloc coalesces away, so it costs zero emitted bytes.
 *
 * It is still dead because the requirement is insn_count >= 123 (see s18's
 * derivation in hypotheses.md H35): the 0x91A2B3C5 movable has savings 1 and
 * lifetime 1, so loop.c:1631's `threshold * savings * lifetime >= insn_count`
 * reads `122 >= insn_count`, and 64 is 59 short.  Every free channel measured in
 * this session yields about +1 counted insn per added statement, so closing 59
 * insns in ordinary C would take ~59 invented statements.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 w;
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
        a = *(s32 *)(src + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(src + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(src + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *src;
        w = v;
        dst[0x24] = w;
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
