/* func_8007CE0C - WIP candidate. sandbox --disable all == 30
 * (HEAD == 48, r6 candidate == 47). 143/143 insns AND frame 80 == target 0x50.
 * Pure C: the committed asm("s5") pin and its __asm__ barrier are gone.
 *
 * GROUP A CLOSED (r8) - all 16 frame instructions now match.
 * A widening site costs an 8-byte phantom frame slot only when the reg:HI being
 * widened has a SECOND use AS AN HImode VALUE (combine then keeps the dead
 * intermediate: combine.c:1458 added_sets_2). The y clamp had two such sites
 * because its accumulator a0_tmp is 16-bit; the x clamp had none because
 * v1_tmp was s32, so its assignments consumed the WIDENED value instead.
 * Narrowing the x accumulator to u16 and mirroring the y side's shape - the
 * symmetric spelling of the same algorithm - converts both x sites:
 * orphans 2 -> 4, vars 16 -> 32, frame 64 -> 80. 47 -> 30.
 *
 * GROUP C CLOSED (r6): func_8007DC9C() takes no argument.
 * Group B (the ~30-insn register-naming residual) is next, by ALLOCDBG sizing.
 */
s32 func_8007CE0C(_GpuChunkHdr_CE0C *arg0, s32 *arg1) {
    s32 var_s5;
    s16 coord;
    s32 half_size;
    s32 big_size;
    s32 remainder;
    u16 v1_tmp;
    u16 a0_tmp;
    s32 v0_ext;

    motion_LoadPreCalcData_8007DC68();

    coord = arg0->x;
    var_s5 = 0;
    if (coord < 0) goto x_neg;
    v1_tmp = coord;
    if (D_8009BE78 < coord) {
        v1_tmp = D_8009BE78;
    }
    goto x_done;
x_neg:
    v1_tmp = 0;
x_done:
    arg0->x = (s16)v1_tmp;

    coord = arg0->y;
    if (coord < 0) goto y_neg;
    a0_tmp = coord;
    v0_ext = a0_tmp << 16;
    if (D_8009BE7A < coord) {
        a0_tmp = D_8009BE7A;
        goto y_block_8;
    }
    goto y_done;
y_neg:
    a0_tmp = 0;
y_block_8:
    v0_ext = a0_tmp << 16;
y_done:
    arg0->y = (s16)a0_tmp;

    {
        s32 y_ext = v0_ext >> 16;
        s32 prod = (s32)arg0->x * y_ext + 1;
        s32 rounded = prod + ((u32)prod >> 31);
        half_size = rounded >> 1;
        big_size = rounded >> 5;
    }

    if (half_size <= 0) {
        return -1;
    }

    remainder = half_size - big_size * 16;

    if (!((*g_gpu_stat_reg) & 0x04000000)) {
        do {
            if (func_8007DC9C() != 0) {
                return -1;
            }
        } while (!((*g_gpu_stat_reg) & 0x04000000));
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = (var_s5 != 0) ? 0xB0000000 : 0xA0000000;
    *g_gpu_data_reg = arg0->unk0;
    *g_gpu_data_reg = *(s32 *)&arg0->x;

    while (--remainder != -1) {
        *g_gpu_data_reg = *arg1++;
    }

    if (big_size != 0) {
        *g_gpu_stat_reg = 0x04000002;
        *g_gpu_dma_madr = (u32)arg1;
        *g_gpu_dma_bcr = (big_size << 16) | 0x10;
        *g_gpu_dma_chcr = 0x01000201;
    }

    return 0;
}
