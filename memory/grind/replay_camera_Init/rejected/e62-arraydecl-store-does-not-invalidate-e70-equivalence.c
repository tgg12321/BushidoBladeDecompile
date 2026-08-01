/* REJECTED (s9, 2026-07-31) — the honest de-FAKE of `pe70` via CSE memory
 * invalidation is DEAD.
 *
 * THE IDEA.  s8 established that declaring `extern s16 D_80101E62[];`
 * (include/code6cac.h:280) and writing `D_80101E62[0]` reproduces target's
 * `$t0` shape: `lui $t0,%hi; addiu $t0,$t0,%lo` with the guard load `lh $v0,0($t0)`
 * and the final store `sh $a0,0($t0)` both going through the REGISTER base.  The
 * s9 hypothesis was that this register-based store is exactly the case
 * `cse.c:7539-7578 note_mem_written` treats as writing an unknown address:
 * `writes_ptr->all = 1` -> `invalidate_memory` -> every memory equivalence in the
 * table is discarded, INCLUDING the `D_80101E70` store entry recorded a few
 * insns earlier.  If so, the very next statement's DIRECT read of
 * `D_80101E70` would survive store-to-load forwarding as a real
 * `lui $v1,%hi / lw $v1,%lo` pair — target's two missing instructions — and the
 * `/* FAKE *\/ s32 *pe70 = &D_80101E70;` pointer local would no longer be needed
 * at all.  That would leave the function with ZERO fake constructs.  The
 * ordering is favourable and honest: `D_80101E62[0] = 2;` sits immediately
 * before the re-read in candidate order, which is also target's source order.
 *
 * THE MEASUREMENT (sandbox --disable all, with the two-file array patch applied;
 * harness tmp/grind/replay_camera_Init/s9/probe9.py, numbers in s9_results.json):
 *
 *   n1  this body (array decl, pe70 deleted, direct read)          17 / 36
 *   n2  n1 with the re-read folded into the final expression       17 / 36
 *   n3  n1 with `D_80101E62[0] = 2;` hoisted to sit immediately
 *       after `D_80101E70 = ec_val;`                               23 / 35
 *   n4  n1 with the `D_80101E9E = 0;` store moved before it        16 / 36
 *   --- for comparison ---
 *   a1  the same array decl but WITH `s32 *pe70` (candidate)       13 / 38
 *
 * 36 instructions, not 38: the `D_80101E70` reload is ABSENT in every
 * pointer-free form.  The register-based store does not invalidate anything.
 *
 * WHY (mechanism).  The array subscript `D_80101E62[0]` is an ARRAY REFERENCE,
 * so GCC sets `MEM_IN_STRUCT_P` on the destination MEM, and its address
 * `(plus (symbol_ref "D_80101E62") (const_int 0))` is constant-folded by
 * `fold_rtx` before `note_mem_written` inspects it.  `note_mem_written` sets
 * `writes_ptr->all` only for a MEM that is NEITHER `MEM_IN_STRUCT_P` NOR a
 * PLUS-form address AND whose address satisfies `cse_rtx_addr_varies_p` — this
 * store fails all three tests.  The register base visible in the ASSEMBLY is
 * produced downstream, by the address CSE that shares the materialised
 * `%hi/%lo` address across the guard and the store; it is not the rtx cse_insn
 * sees when it decides what to invalidate.  This is the same trap s1's H2 fell
 * into with the pointer-local spelling, now confirmed for the honest
 * array-declaration spelling too.
 *
 * CONSEQUENCE FOR THE FRONTIER.  The two known register-base spellings for the
 * `D_80101E62` store (pointer local, s1 H2; array declaration, s9 H21) BOTH
 * fail to invalidate, for the same documented reason.  Combined with s8's
 * finding that an array declaration on D_80101E70 ITSELF also fails (an index-0
 * subscript folds to the same `(mem (symbol_ref))` rtx the store recorded, so
 * forwarding still matches — rejected/e70-array-decl-does-not-defeat-store-to-
 * load-forwarding.c), and with s8's NEGATIVE census over 1751 matched community
 * scratches (zero same-mode word-store/word-reload of one global without
 * volatile), there is now no known honest route to the reload.  `pe70` is the
 * function's single remaining /* FAKE *\/ and the reviewer verdict on it is the
 * whole remaining question.
 *
 * DO NOT RE-PROPOSE any variant of "make the D_80101E62 store go through a
 * register so it invalidates memory". Both spellings are measured dead.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    s32 reloaded;

    if (D_80101E62[0] != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62[0] = 2;
    reloaded = D_80101E70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
