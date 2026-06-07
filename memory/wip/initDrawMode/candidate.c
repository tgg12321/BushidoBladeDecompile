/* memory/wip/initDrawMode/candidate.c
 *
 * Drop-in replacement for src/gpu.c::initDrawMode.
 * Honest pure-C floor: sandbox --disable all = 1 (HEAD floor with pins = 5).
 * The remaining diff is the OR operand-order at the function's last insn:
 *   build:  or v0, v0, v1   (00431025)  — val|cmd source order, val rs / cmd rt
 *   target: or v0, v1, v0   (00621025)  — cmd|val source order, cmd rs / val rt
 * Both forms are semantically identical (OR is commutative) but encode to
 * different bytes. Reaching target's bytes requires `cmd | val` source order
 * with val landing in $v0 and cmd in $v1; in cmd|val form GCC's combine pass
 * folds val into $a3 (a3 dies after val's andi), giving the wrong allocation.
 * See rejected/ for the cmd|val attempts.
 */
void initDrawMode(u8 *a0, s32 a1, s32 a2, u32 a3) {
    u32 cmd;
    u32 val;
    a0[3] = 1;
    cmd = GP0_DRAW_MODE;
    if (a2) {
        cmd = (GP0_DRAW_MODE | GPU_DRAW_MODE_DITHER);
    }
    val = a3 & GPU_DRAW_MODE_MASK;
    if (a1) {
        val |= GPU_DRAW_MODE_TEXOFF;
    }
    *(u32 *)(a0 + 4) = val | cmd;
}
