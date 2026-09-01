/*
 * REJECTED — do{}while(0) wrap around the `dst[0x21] = v/1800;` statement.
 * s2b (2026-09-01, structural). MEASURED: sandbox func_8003C714 --disable all
 * == 15 (105 insns) — identical to the unwrapped candidate. NOT an improvement,
 * and the mechanism it was supposed to exploit does not exist.
 *
 * WHY IT IS DEAD (mechanism, read out of the .loop dump, not guessed):
 * the wrap produces `Loop from 43 to 68 is phony.` (loop.c:573). A degenerate
 * do{}while(0) has no back edge, so find_and_verify_loops/loop_optimize marks it
 * PHONY and NEVER calls scan_loop on it. Therefore the 0x91A2B3C5 movable is
 * never hoisted into an inner preheader, `moved_once[regno]` (loop.c:1912) is
 * never set for it, and the `insn_count *= 2` "halved since already moved"
 * branch (loop.c:1609-1611) — the entire point of the construct — never fires.
 * The outer loop's movable table is bit-identical to the unwrapped form:
 *   Loop from 25 to 160: 56 real insns.
 *   Insn 33: regno 78 (life 1), move-insn savings 1  moved to 217
 *   Insn 50: regno 84 (life 1), move-insn savings 1  moved to 219
 *   Insn 74: regno 91 (life 31), move-insn savings 1  moved to 221
 * Every degenerate loop spelling (do/while/for with a constant-false test) is
 * phony by the same rule, so the whole family is dead — not just this spelling.
 * A REAL inner loop (a live back edge) would set moved_once, but it emits a real
 * counter+branch that the 104-instruction target does not contain.
 * Corollary: hypotheses.md K7. Frontier item "do{}while(0) -> moved_once
 * doubling" is KILLED; no ruling-request is needed for it.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
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
        do {
            dst[0x21] = *(s32 *)(src + 4) / 1800;
        } while (0);
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
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
