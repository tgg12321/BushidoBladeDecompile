/* func_80017FA0 (code6cac.c) - MATCHED. sandbox --disable all = 0 and full
 * build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (s4, 2026-08-20).
 * ZERO regfix/asmfix rules, ZERO cheat-asm, ZERO FAKE-annotated constructs.
 *
 * Two levers, both ordinary C:
 *  1. (s1) The two FIXED scratchpad writes go through a pointer-typed lvalue
 *     (scr[0x2E] / scr[0x18]) rather than a raw *(volatile s32*)0xCONST cast.
 *     The cast form makes cc1 synthesise the address into a register
 *     (lui;ori;sw 0(r) = 3 insns); the pointer lvalue yields a (mem (const_int))
 *     the assembler folds to lui at,0x1f80; sw x,disp(at) = target's 2 insns.
 *     That also removed the v0/v1 cascade the early lui had forced. 13 -> 2.
 *  2. (s4) The outer loop guard is spelled against the LIVE counter,
 *     `if (i < ptr[1])`, not against the literal `ptr[1] > 0`. Semantically the
 *     same test (i is 0 there) and the shape GCC's loop rotation produces from
 *     `for (i = 0; i < ptr[1]; i++)`. Using i in the guard makes i a local that
 *     survives to frame layout, so get_frame_size() reports vars=8 and
 *     mips.c:compute_frame_size emits `addiu sp,sp,-8` (in the beqz delay slot)
 *     / `addiu sp,sp,8` - the target's zero-store 8-byte leaf frame - while i
 *     still lives entirely in a register, so no frame store is ever emitted.
 *     This is the ordinary phantom-frame artifact of GCC 2.7.2 documented in
 *     memory/project/phantom-frame-slots-gcc272.md, NOT a dead local: sessions
 *     s1-s3 concluded only a forbidden dead local could reserve those 8 bytes
 *     and escalated on that basis; the conclusion was wrong. 2 -> 0.
 *
 * The inner double-loop keeps FULL-constant computed addresses
 * *(volatile s32*)(0x1F800064 + sp_inner) so cc1 re-materialises lui 0x1f80
 * every iteration (target does not LICM-hoist it). Routing the inner writes
 * through `scr` hoists the lui and regresses (old idx_base form = 33). */
/* Copies scaled fields out of the block at a0[3] into scratchpad RAM
 * (0x1F800000). ptr[0] is written scaled by 128; ptr[1] is the group count,
 * and each group writes three words scaled by 4 at a 0x18 stride plus one
 * word taken from the 0x68 array. Nothing happens when a0[3] is null. */
void func_80017FA0(s32 *a0) {
    volatile s32 *scr = (volatile s32 *)0x1F800000;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    scr[0x2E] = ptr[0] << 7;   /* 0x1F8000B8 */

    {
        s32 i = 0;
        if (i < ptr[1]) {
            s32 *p68 = ptr;
            volatile s32 *ac_base = (volatile s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j = 0;
                s32 data_off = i << 5;
                s32 sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(volatile s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(volatile s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(volatile s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                } while (j < 2);
                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }

    scr[0x18] = ((s32 *)a0[3])[1];   /* 0x1F800060 */
end:
    ;
}
