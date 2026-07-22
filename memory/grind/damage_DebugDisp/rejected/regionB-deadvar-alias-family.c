/* REJECTED (s5) — Region B dead-variable alias family (permuter finds).
 *
 * Fresh-seed floor-6 permuter campaign (20,895 iters) reproduced EXACTLY the
 * s4 result: every sub-130 basin exit is a dead-variable alias of the Region B
 * store `*ptr = *(u16 *)(a2p + 0xD0);`, coercing GCC to read a2p through a
 * different (already-live) register. All are cheat-by-spelling (dead
 * reassignment / alias with no semantic purpose — no human would write it) or
 * semantically broken, and NONE reach floor < 6:
 *
 *   output-75-1  score 75 : `*ptr = *(u16 *)((chkptr = a2p) + 0xD0);`
 *       chkptr is s32* -> chkptr+0xD0 = a2p + 0x340 bytes. BROKEN (target reads
 *       a2p+0xD0). Same trap as regionB-chkptr-alias-wrong-offset.c.
 *   output-90-1  score 90 : `*ptr = *(u16 *)((src = a2p) + 0xD0);`
 *       src is u8* -> offset correct, but a dead reassignment of `src` purely
 *       to steer RA. cheat-by-spelling. Only partially cuts Region B.
 *   output-70-1  score 70 : `base = a2p; *ptr = *(u16 *)(base + 0xD0);`
 *       dead reassignment of the `base` param mid-loop; 79 insns; cheat.
 *   output-100-1 score 100: `u8 *new_var = (u8 *)src;` ... `end = new_var+0x20`
 *       redundant dead alias of src in the CopyBlock end computation; the name
 *       literally announces coercion intent. cheat, and worse than floor.
 *
 * Region B (the LICM moves-vs-consts scheduling order) has NO clean permuter
 * lever in the floor-6 chassis — reconfirmed with a structurally fresh seed.
 * The two range-check constants (0x80000000, 0x1FFFFF) are only referenceable
 * inside the k-loop, so LICM controls their placement; the only source forms
 * that move them are dead-alias cheats. Needs a genuine control-flow change
 * (a different loop shape that legitimately computes the range bound before the
 * ap/a2p moves) — not a local permuter mutation.
 *
 * Representative broken form (output-75-1):
 */
        do {
            u16 *ptr = *(u16 **)((u8 *)ap + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)((chkptr = a2p) + 0xD0);  /* s32* + 0xD0 = +0x340 bytes: BROKEN */
            }
            a2p += 2;
            k++;
            ap++;
        } while (k < 0x16);
