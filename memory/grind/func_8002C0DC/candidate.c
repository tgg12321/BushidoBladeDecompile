/* func_8002C0DC — src/code6cac_b.c
 *
 * STATUS: sandbox --disable all == 0 (session 1, 2026-08-13). Honest pure-C
 * floor went 23 -> 0 on the first probe. Verified NOT a masked zero: an
 * objdump of the sandbox object was compared instruction-by-instruction with
 * asm/funcs/func_8002C0DC.s and is identical across all 84 instructions —
 * same opcodes, same registers, same immediates, same field offsets, same
 * delay-slot occupants, same frame size (0x20) and same three callee-saves
 * (s0/s1/s2). The only fields that read as zero in the object are the
 * link-time relocations (lui/%lo pairs and jal targets).
 *
 * THE LEVER (single change, first try): the previous body walked the array by
 * hand (`var_s0 += 0x44C`) with a separate counter, and pinned three values
 * with `register T x asm("sN")`. That gives loop.c TWO independent induction
 * variables, so strength_reduce/combine_givs materialized a SECOND live
 * pointer for the two clustered field reads — the build emitted
 * `addiu $s1,$s0,0xE0` at loop entry plus `addiu $s1,$s1,0x44C` per iteration
 * and consumed the fields as `-8($s1)` / `0($s1)`. That surplus pointer needed
 * a FOURTH callee-save ($s3), which pushed the frame to 0x28 and added the
 * save/restore pair: +4 instructions, and it displaced the whole register
 * assignment downstream (which is what the 13 regfix rules at
 * regfix.txt:229-244 were papering over).
 *
 * Writing the loop as an ordinary indexed `for` leaves exactly ONE basic
 * induction variable (`i`, which survives because the bound test reads it),
 * so all three references off the element pointer (+0x00, +0xD8, +0xE0)
 * combine onto a single derived pointer — precisely the target's $s0. The
 * fourth callee-save disappears, the frame drops to 0x20, and the post-loop
 * register assignment falls out correct on its own: the fifth surplus
 * instruction (a load-delay `nop` before the `bnez` on D_800A376E) also
 * vanished, because with the register pressure relieved the scheduler issues
 * the `lh D_800A376E` early, ahead of the 1100x multiply-shift chain, exactly
 * as target does.
 *
 * PURITY: this diff only REMOVES coercion (three register-asm pins) and
 * replaces a hand-rolled pointer walk with the plainer index walk. It adds no
 * dead code, no dead stores, no volatile, no aliases, no wrappers, no inline
 * asm, and claims no sanctioned-exception family. See self_vet.md.
 *
 * INTEGRATION: the 13 regfix rules at regfix.txt:229-244 were written against
 * the old wrong codegen and are now actively harmful. They must be retired
 * (operator/driver surface — this session may not touch regfix.txt or run
 * `retire`).
 */

void func_8002C0DC(void) {
    s32 i;
    u8 *var_s0;
    s32 temp_s2;

    temp_s2 = func_8002BC68(D_800A371C);

    for (i = 0; i < 2; i++) {
        u8 *e = (u8 *)&D_80101EC8 + i * 0x44C;
        u8 *ptr = *(u8 **)e;
        s32 arg1 = *(s32 *)(ptr + 0xD8) - *(s32 *)(e + 0xD8);
        s32 arg2 = *(s32 *)(ptr + 0xE0) - *(s32 *)(e + 0xE0);
        func_8001F860((s16 *)e, ratan2(arg1, arg2));
    }

    {
        s32 idx;
        s32 chk;
        idx = D_800A38AE;
        chk = D_800A376E;
        var_s0 = &D_80101EC8 + idx * 0x44C;

        if (chk == 0) {
            if (D_800A3758 == 0xFF) {
                if (*(u8 *)(var_s0 + 0xAA) == *(s16 *)(var_s0 + 0x40)) {
                    func_8002AB08(1);
                }
            }
        }

        {
            s32 v1;
            v1 = *(s16 *)(var_s0 + 0x40);
            if (v1 < (s32)D_800A38E8) {
                return;
            }
            if (v1 >= *(u8 *)(var_s0 + 0xAA)) {
                return;
            }
            if (D_800A371C + 0xC8 >= temp_s2) {
                return;
            }
            {
                u8 *v1ptr;
                v1ptr = *(u8 **)var_s0;
                *(s16 *)(var_s0 + 0x286) = 4;
                *(s16 *)(v1ptr + 0x286) = 5;
            }
        }
    }
}
