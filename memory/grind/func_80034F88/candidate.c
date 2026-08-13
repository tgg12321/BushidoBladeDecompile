/*
 * func_80034F88 — MATCHED FORM, grind session s12 (structural).
 * Honest sandbox (`sandbox func_80034F88 --disable all`): **score 0**,
 *   49 build insns vs 49 target insns, rules_dropped 30, cheat_asm_stripped 317.
 * Installed in src/code6cac_b.c.  Vetted in self_vet.md.
 * (Floor history: s1 23 -> s3 18 -> s10 9 -> s11 13-with-block-1-right -> s12 0.)
 *
 * =====================================================================
 * THE FINDING — the byte local, not the pointer structure, was the wall
 * =====================================================================
 * s1-s11 modelled the residual as a base-register allocation tie ("blocks 2/3
 * take v1 for the base and a0 for the byte; the target does the opposite") and
 * spent eleven sessions moving the POINTER structure around: one handle, two,
 * three, four, copies, re-assignments, and every placement of each assignment
 * relative to the condition and the store.  That axis really was flat — but the
 * tie was never about the pointers.
 *
 * MECHANISM (live-range conflict, read off the s12 side-by-side).  On the s12
 * `d3` chassis every INSTRUCTION was already in the target's position and all
 * 12 residual points were register naming.  The target's stream is
 *     ori v0,a0,1 / move v0,a0 / lui a0 / addiu a0 / sb v0,0(v1)
 * — i.e. block 1's LOADED BYTE lives in a0, dies at the `move`, and block 2's
 * BASE is then materialised into that same a0 one instruction later, before
 * block 1's store.  Block 2's byte is consequently pushed off a0 onto v1 (base
 * 1's register, freed by the `sb`), and block 3 reuses a0 for its base.  That
 * whole cascade is legal ONLY if block 1's byte and blocks 2/3's byte are
 * DIFFERENT PSEUDOS.  With one C local for the loaded byte across all three
 * blocks, GCC 2.7.2's global allocator sees one allocno whose live range spans
 * block 2's base; they conflict; the base is evicted from a0 onto a2; and the
 * byte is stuck in a0 for every block.  Every one of the eleven sessions' forms
 * used a single byte local, so every one of them hit the same conflict and
 * every pointer permutation was measured against a fixed 12-13 point floor it
 * could not cross.
 *
 * Splitting the byte local is what closes it — and the split point matters:
 * mask+block 1 share one local, blocks 2/3 share another (or one local per
 * block; both reach 0).  One local for everything is 13; the "wrong" split
 * (mask alone vs the three blocks) is also 13.
 *
 * =====================================================================
 * WHY THIS SPELLING
 * =====================================================================
 * Once the split is in place the pointer structure stops being delicate: SEVEN
 * structurally different bodies reach 0 (s12 waves F/G/H/I/J).  This one was
 * chosen because it carries the fewest constructs that need vetting — four
 * self-contained blocks, each declaring its own handle and its own scratch,
 * every declaration read and written, no pointer copy, no re-assignment of a
 * variable to a value it already holds, and none of the three
 * `asm volatile("" ::: "memory")` scheduling barriers the previous src body
 * carried.  The one construct that is load-bearing rather than incidental is
 * `c` carrying the tested flag bit and then the byte value: a separate result
 * local collapses the if/else diamond and scores 30 at 44 insns (j1.c).  That
 * is the frozen SOTN "variable reuse for codegen control" family; see
 * self_vet.md for the claim, scope quote and precedent.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    {
        u8 *q = &D_80106A73;

        *q &= 0xF8;
    }

    {
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
