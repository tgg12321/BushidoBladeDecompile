/* func_80017FA0 (code6cac.c) — BEST honest pure-C candidate. Honest sandbox
 * distance = 2 (s1 recon, 2026-07-24). Was floor 13 (WIP) / 16 (old candidate).
 *
 * KEY LEVER (s1): route the two FIXED scratchpad writes through a pointer-typed
 * lvalue (`scr[idx]`) instead of a raw `*(volatile s32*)0xCONST` cast. A raw
 * integer-constant cast makes GCC synthesize the address into a register
 * (lui+ori) then `sw ...,0(reg)` (3 insns, NO displacement fold). Routing the
 * same store through a pointer variable / struct field makes GCC emit a
 * `(mem (const_int))` that the assembler folds to `lui at,0x1f80; sw ...,0x60(at)`
 * — the 2-insn folded form the target uses. Probe: tmp/grind/func_80017FA0/s1/
 * foldprobe.* (P1 int-cast=3insn; P4 ptr-var / P5 struct-cast = 2-insn folded).
 * This closed 11 of 13 diffs: the fold (both fixed writes) AND the downstream
 * v0<->v1 register cascade (temp was forced to v1 by the early delay-slot lui).
 *
 * RESIDUAL (distance 2): the empty 8-byte stack frame. Target does
 * `addiu sp,-8` (in the beqz delay slot) ... `addiu sp,8` with NOTHING stored
 * to the frame. Our build emits no frame (nop in the delay slot, no teardown).
 * Sole remaining mechanism — see hypotheses.md frontier F1.
 *
 * The inner double-loop keeps FULL-constant computed addresses
 * `*(volatile s32*)(0x1F800064 + sp_inner)` so GCC re-materializes lui 0x1f80
 * each iteration (target does NOT LICM-hoist it). Do NOT route the inner writes
 * through `scr` — that hoists the lui and regresses (old idx_base form = 33). */
void func_80017FA0(s32 *a0) {
    volatile s32 *scr = (volatile s32 *)0x1F800000;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    scr[0x2E] = ptr[0] << 7;   /* 0x1F8000B8 — folds to lui;sw 0xB8 via scr[] */

    {
        s32 i = 0;
        if (ptr[1] > 0) {
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

    scr[0x18] = ((s32 *)a0[3])[1];   /* 0x1F800060 — folds to lui;sw 0x60 via scr[] */
end:
    ;
}
