/*
 * REJECTED - func_8003C714 - s5 (2026-09-01, synthesis modality)
 * SLUG: dead-locals-deleted-before-loop-counts-them
 *
 * WHY IT IS DEAD (measured, not argued): dead in-loop computations do NOT
 * inflate loop.c's insn_count, because toplev.c:2866 runs
 * `delete_dead_from_cse` immediately after cse1 and BEFORE loop_optimize
 * (toplev.c:2895). Every dead insn is already a NOTE by the time
 * count_loop_regs_set (loop.c:2989-3092) counts class-'i' insns.
 *
 * Sweep k = 0, 4, 8, 16, 32 dead chained ALU ops on a loaded value
 * (tmp/grind/func_8003C714/s5/gen_dead.py + sweep.sh):
 *   k=0   Loop from 25 to 146: 56 real insns.   asm 107 lines
 *   k=4   Loop from 25 to 167: 56 real insns.   asm 107 lines
 *   k=8   Loop from 25 to 185: 56 real insns.   asm 107 lines
 *   k=16  Loop from 25 to 221: 56 real insns.   asm 107 lines
 *   k=32  Loop from 25 to 293: 56 real insns.   asm 107 lines
 * insn_count is PINNED at 56 at every k; the emitted function is byte-identical.
 * (The loop's UID range grows because deleted insns keep their UIDs as NOTEs -
 * that is the positive proof the insns were created and then deleted upstream.)
 *
 * This is the k=32 form. It is a diagnostic only - bulk dead locals to steer a
 * compiler heuristic would be a cheat (checklist T1/T2/T3/T6) even if it worked,
 * and it does not work. See hypotheses.md K15.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;
    s32 dead;
    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dead = *(s32 *)(src + 4);
        dead ^= 0x12345678;
        dead += 0x2468ACF1;
        dead ^= 0x3579BDE2;
        dead += 0x4A5B6C7D;
        dead ^= 0x5B6C7D8E;
        dead += 0x6C7D8E9F;
        dead ^= 0x7D8E9FA0;
        dead += 0x1E2D3C4B;
        dead ^= 0x2F3E4D5C;
        dead += 0x30415263;
        dead ^= 0x41526374;
        dead += 0x52637485;
        dead ^= 0x63748596;
        dead += 0x748596A7;
        dead ^= 0x8596A7B8;
        dead += 0x96A7B8C9;
        dead ^= 0xA7B8C9DA;
        dead += 0xB8C9DAEB;
        dead ^= 0xC9DAEBFC;
        dead += 0xDAEBFC0D;
        dead ^= 0x1B2A3948;
        dead += 0x2C3B4A59;
        dead ^= 0x3D4C5B6A;
        dead += 0x4E5D6C7B;
        dead ^= 0x12345678;
        dead += 0x2468ACF1;
        dead ^= 0x3579BDE2;
        dead += 0x4A5B6C7D;
        dead ^= 0x5B6C7D8E;
        dead += 0x6C7D8E9F;
        dead ^= 0x7D8E9FA0;
        dead += 0x1E2D3C4B;
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
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
