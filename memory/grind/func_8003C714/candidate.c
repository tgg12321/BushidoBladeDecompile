/*
 * CANDIDATE -- func_8003C714 (src/code6cac_c2.c) -- s20 (2026-09-05, structural)
 *
 * MEASURED THIS SESSION on the shipped chassis HEAD: this exact body, dropped
 * over the `INCLUDE_ASM("asm/funcs", func_8003C714);` line at
 * src/code6cac_c2.c:629 with NO other edit anywhere in the tree, scores
 *   sandbox func_8003C714 --disable all = 0   (target_insns 104, build_insns 104)
 *
 * WHAT CHANGED FROM THE s19 CANDIDATE (which measured 15). Exactly one thing:
 * the loop is spelled as a loop-and-a-half. The s19 body ended the loop with
 *     i += 1;
 *   } while (i < 3);
 *   func_8001CD68(buf);
 * this body ends it with
 *     i += 1;
 *     if (i >= 3) {
 *         func_8001CD68(buf);
 *         break;
 *     }
 *   } while (1);
 * Nothing else differs -- same statements, same declarations, same
 * `base = (u8 *)&D_80106A58;`, same `extern s32 D_80106A58;` at line 156, no
 * padding, no dead stores, no FAKE construct, no `volatile`, no header edit.
 *
 * WHY IT MATCHES (pass-attributed from tmp/grind/func_8003C714/dumps/code6cac_c2.loop,
 * regenerated this session with this body in place):
 *   Loop from 28 to 184: 64 real insns.
 *   Insn 48: regno 87 (life 1), move-insn savings 1 not desirable     <- 0x91A2B3C5
 *   Insn 66: regno 93 (life 35), move-insn savings 1  moved to 234    <- 0x88888889
 * The whole 19-session residual was one LICM decision. loop.c:1631 hoists a
 * movable iff `threshold * savings * m->lifetime >= insn_count`, and
 * loop.c:532 sets `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`
 * = 122 with no call in the loop, 61 with one. The 0x91A2B3C5 constant sits at
 * savings 1 / lifetime 1 (both at their floor), so with no call in the loop
 * 122 >= 62 hoists it -- which is what every previous session measured, and is
 * the sole source of all 15 residual instructions.
 *
 * Moving `func_8001CD68(buf)` inside the loop's terminal `if` puts a CALL_INSN
 * between the NOTE_INSN_LOOP_BEG/END pair, so prescan_loop (loop.c:2199-2203)
 * sets loop_has_call = 1 and threshold halves to 61. Then
 *   0x91A2B3C5:  61 * 1 * 1  = 61  <  64  -> "not desirable", stays in the loop
 *   0x88888889:  61 * 1 * 35 = 2135 >= 64 -> moved to the preheader
 * which is exactly the target's split (lui/ori for 0x88888889 at 8003C740, and
 * lui/ori for 0x91A2B3C5 in-loop at 8003C754/8003C75C filling the load-delay
 * slot after `lw $v1, 4($a2)`). insn_count rises 62 -> 64 because the call and
 * its argument setup are now counted inside the loop.
 *
 * The emitted control flow is unchanged: `slti $v0, $t0, 0x3; bnez $v0, .L8003C754`
 * with `addiu $a1, $a1, 0x4` in the delay slot, falling through to
 * `jal func_8001CD68`. The loop-and-a-half is a source-level exit spelling, not
 * an emitted extra branch -- the byte count is 104 == 104 and score 0.
 *
 * ADMISSIBILITY. This body contains no construct from any forbidden family and
 * claims no sanctioned-family carve-out: no dead store, no constant holder, no
 * unused local, no padding chain, no long long / float cast, no `volatile`, no
 * declaration change, no `/* FAKE * /` annotation needed. It is a loop-and-a-half
 * -- the ordinary C idiom for "do the last thing on the way out" -- and it is
 * the ONLY byte-free carrier of loop_has_call found in 20 sessions. See
 * memory/grind/func_8003C714/self_vet.md.
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
        if (i >= 3) {
            func_8001CD68(buf);
            break;
        }
    } while (1);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
