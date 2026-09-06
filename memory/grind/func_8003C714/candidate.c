/*
 * CANDIDATE -- func_8003C714 (src/code6cac_c2.c) -- s19 (2026-09-05, rederive)
 *
 * MEASURED THIS SESSION on the shipped chassis: this exact body, dropped over
 * the `INCLUDE_ASM("asm/funcs", func_8003C714);` line at src/code6cac_c2.c:629
 * with NO other edit anywhere in the tree, scores
 *   sandbox func_8003C714 --disable all = 15  (target_insns 104, build_insns 105)
 *
 * WHY THIS BODY REPLACES THE s18b CANDIDATE. The s18b candidate reached
 * distance 0 but only by carrying (a) a FAKE-annotated DImode dead store whose
 * sole purpose was to summon __divdi3, and (b) a retype of D_80106A58 to
 * `extern u8 D_80106A58[24]`. BOTH are on this function's banned_constructs
 * list, so that body can never be submitted. This session proved that NEITHER
 * is load-bearing: the same distance 0 is reachable from THIS body plus pure
 * insn_count padding, with `base = (u8 *)&D_80106A58;` unchanged and the
 * `extern s32 D_80106A58;` declaration at src/code6cac_c2.c:156 untouched.
 * The two "declaration puns" the brief listed as a hard submission blocker are
 * therefore NOT part of the residual at all -- they were artefacts of the s18b
 * carrier, not of the match. See hypotheses.md s19 (H35/K57) and evidence.md s19.
 *
 * WHAT THE RESIDUAL IS, EXACTLY (all numbers measured this session from the
 * -dL loop dump, tmp/grind/func_8003C714/dumps/code6cac_c2.loop):
 *   Loop from 28 to 170: 62 real insns.
 *   Insn 48: regno 87 (life 1), move-insn savings 1  moved to 225   <- 0x91A2B3C5
 *   Insn 66: regno 93 (life 35), move-insn savings 1 moved to 227   <- 0x88888889
 * The target hoists reg 93 (lui/ori in the preheader at 8003C740) and does NOT
 * hoist reg 87 (lui/ori materialised in-loop at 8003C754/8003C75C). All 15
 * residual instructions are that one difference. loop.c:1631 moves a movable
 * iff `threshold * savings * m->lifetime >= insn_count`, and loop.c:532 sets
 * `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` = 122 with no
 * call in the loop, 61 with one. reg 87 sits at savings 1 / lifetime 1, both
 * already at their floor, so the ONLY two ways to leave it in the loop are
 *   (A) loop_has_call = 1  -> need insn_count >= 62, and the baseline is
 *       ALREADY 62, so a single byte-free CALL_INSN in the loop is sufficient
 *       on its own (margin is exactly one insn); or
 *   (B) no call             -> need insn_count >= 123, i.e. +61 RTL insns in
 *       the loop that emit no bytes.
 * Route (B) was believed capped at insn_count 64 by s18. It is NOT: see the two
 * new rejected/ forms, which reach 123 and measure sandbox 0. What blocks (B)
 * is admissibility, not reachability -- the padding has to be something a
 * programmer would actually write.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    s32 a, b, c, v;
    u8 *base;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    base = (u8 *)&D_80106A58;
    do {
        src = base + i * 8;
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
        dst[0x24] = v;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
