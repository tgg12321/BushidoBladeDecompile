/* func_800238C4 - grind candidate, session 2 (2026-09-07). MATCHED: sandbox
 * --disable all = 0, 219/219 instructions, 0 rules; full-build SHA1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle ok this session).
 *
 * Derivation of the body is in the s1 header (kept in evidence.md): void return
 * type, the ((u16 *)arg0)[2] sub-word read, and the natural /8 and /64 divisions
 * with per-arm dx/dz reads took the honest floor from 48 to 3.
 *
 * The last 3 instructions were a $v0/$v1 seat swap on {lw parent, li 2} feeding
 * `sh 2, 0x286(parent)`. Closed in s2 by duplicating the common tail
 * `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);` plus its control transfer into
 * the first arm (sanctioned duplicated-statement-into-arms family, FAKE-annotated
 * in the body). Mechanism, measured with the instrumented cc1 rather than inferred:
 * local-alloc block_alloc sorts the block's quantities with a hand-rolled sort
 * (tools/gcc-2.7.2/local-alloc.c:1539-1563). With only two quantities the
 * next_qty==2 path ranks by qty_compare alone: `li 2` (refs 2, span 2) scores
 * 10000, the parent pointer (refs 2, span 4) scores 5000, so the constant is
 * allocated first and takes $v0. The duplicated tail puts a THIRD quantity (the
 * 0xBC load, refs 2, span 2, pri 10000) in the same block after the store; the
 * next_qty==3 path compares literal quantity NUMBERS rather than qty_order slots,
 * and its third comparison undoes the first exchange, leaving the pointer at
 * qty_order[0]. The pointer takes $v0, the constant $v1 - the target seating.
 * jump2 cross-jump then re-merges the duplicated tail with the shared copy, so the
 * emitted instruction count is unchanged at 219.
 */
void func_800238C4(u8 *arg0)
{
    s32 src[4];
    s32 dst[4];
    s32 out[4];
    s16 offsets[4];
    s16 offsets2[4];
    s32 s1;
    s32 dx_delta;
    s32 dz_delta;
    s32 scratchpad;
    s32 ok;
    if ((*((s32 *) (arg0 + 0x108))) <= 0) {
        return;
    }
    {
        s32 kind = *((u16 *) (arg0 + 0x6A));
        if (((u32) (kind - 0x17)) < 2u) {
            return;
        }
        if ((kind & 0xFFFF) == 0xA) {
            return;
        }
        if ((*((s16 *) (arg0 + 0x72))) != 0) {
            return;
        }
        if ((kind & 0xFFFF) == 1) {
            return;
        }
        if ((kind & 0xFFFF) == 0x28) {
            return;
        }
    }
    scratchpad = 0x1F8002B8;
    src[0] = *((s32 *) (arg0 + 0xB8));
    src[1] = (*((s32 *) (arg0 + 0xBC))) - 0xC8;
    src[2] = *((s32 *) (arg0 + 0xC0));
    dst[0] = *((s32 *) (arg0 + 0xB8));
    dst[1] = (*((s32 *) (arg0 + 0xBC))) + 0x514;
    dst[2] = *((s32 *) (arg0 + 0xC0));
    s1 = func_80053614(src, dst, out, (s32 *) offsets, scratchpad);
    ok = 1;
    if (s1 != 0)
    {
        if ((out[1] - src[1]) >= 0x191)
        {
            ok = (*((u16 *) (arg0 + 0x6A))) != 0x22;
            goto ok_check;
        }
        if (offsets[1] < (-0x7FF))
        {
            goto ok_zero;
        }
        dst[0] += offsets[0] / 8;
        dst[2] += offsets[2] / 8;
        ok = func_80053614(src, dst, out, (s32 *) offsets2, scratchpad) == 0;
        goto ok_check;
    }
    goto ok_check;
    ok_zero:
    ok = 0;
    ok_check:
    if (!ok) {
        return;
    }
    if (s1 == 0)
    {
        offsets[0] = *(s32 *)(arg0 + 0xB8) - *(s32 *)(arg0 + 0xC8);
        offsets[2] = *(s32 *)(arg0 + 0xC0) - *(s32 *)(arg0 + 0xD0);
        if (offsets[0] < -0x40) {
            offsets[0] = -0x40;
        } else if (offsets[0] > 0x40) {
            offsets[0] = 0x40;
        }
        if (offsets[2] < -0x40) {
            offsets[2] = -0x40;
        } else if (offsets[2] > 0x40) {
            offsets[2] = 0x40;
        }
        dx_delta = offsets[0];
        dz_delta = offsets[2];
    }
    else
    {
        dx_delta = offsets[0] / 64;
        dz_delta = offsets[2] / 64;
    }
    {
        s32 kind;
        s1 = ((*((s16 *) (arg0 + 0x1CA))) - ratan2(offsets[0], offsets[2])) & 0xFFF;
        if (s1 >= 0x800) {
            s1 = 0x1000 - s1;
        }
        kind = *((u16 *) (arg0 + 0x6A));
        if (((((kind & 0xFFFF) == 0xF) || (((u32) (kind - 0x1C)) < 2u)) || (((u32) (kind - 0x1E)) < 2u)) || (((u32) (kind - 0x20)) < 2u))
        {
            if (s1 < 0x400) {
                *((s16 *) (arg0 + 0x286)) = 0;
                *((s16 *) (arg0 + 0x94)) = 0;
            }
            else {
                *((s16 *) (arg0 + 0x286)) = 1;
                *((s16 *) (arg0 + 0x94)) = 1;
            }
            *((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;
            /* FAKE: the common tail `0x74 = 0xBC` + its control transfer duplicated into
             * this arm instead of falling through to the shared copy below,
             * mechanism: local-alloc block_alloc's hand-rolled quantity sort
             * (tools/gcc-2.7.2/local-alloc.c:1539-1563). With only the two quantities of
             * `lw parent` (refs 2, span 4, pri 5000) and `li 2` (refs 2, span 2, pri 10000)
             * the next_qty==2 path ranks the constant first and hands it $v0; the
             * duplicated tail puts a third quantity (the 0xBC load, pri 10000) in the same
             * block, and the next_qty==3 path's third comparison restores the parent
             * pointer to qty_order[0], so it takes $v0 and the constant takes $v1 - the
             * target seating. jump2 cross-jump re-merges the two copies, so the emitted
             * function is unchanged at 219/219 instructions.
             * lever-exhaustion: memory/grind/func_800238C4/hypotheses.md K0-K3 + the s2
             * quantity-arithmetic derivation in evidence.md. */
            *((s32 *) (arg0 + 0x74)) = *((s32 *) (arg0 + 0xBC));
            goto skip_74;
        }
        else if ((kind & 0xFFFF) == 0x11)
        {
            D_800A3769 = s1 < 0x400;
            D_800A3758 = ((u16 *)arg0)[2];
            goto skip_74;
        }
        else if (s1 < 0x400)
        {
            *((s16 *) (arg0 + 0x286)) = 0x12;
            *((s16 *) (arg0 + 0x94)) = 0;
        }
        else
        {
            *((s16 *) (arg0 + 0x286)) = 0x11;
            *((s16 *) (arg0 + 0x94)) = 1;
        }
        *((s32 *) (arg0 + 0x74)) = *((s32 *) (arg0 + 0xBC));
    }
    skip_74:
    *((s32 *) (arg0 + 0x104)) += dx_delta;
    *((s32 *) (arg0 + 0x10C)) += dz_delta;
}
