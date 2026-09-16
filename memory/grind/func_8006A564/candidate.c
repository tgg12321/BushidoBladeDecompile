/* func_8006A564 -- src/text1b.c -- MATCHED (session 9, forensics).
 *
 * sandbox func_8006A564 --disable all = score 0, target_insns 199 ==
 * build_insns 199; register-normalized objdump diff
 * (tmp/grind/func_8006A564/s9/final_objdiff_f01.txt) is instruction-identical
 * to build/src/text1b.o on all 199 rows (the 8 "!!" rows are branch/jump
 * displacement TEXT only -- same relative offsets, different absolute section
 * addresses). Full-tree `verify-oracle` re-linked to
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == the oracle.
 *
 * ZERO cheat constructs: no inline asm, no register pins, no volatile, no
 * dead stores, no pads, no unused locals, no FAKE annotations. Every local is
 * written and read; every statement stores a value the target stores.
 *
 * ================= WHAT SESSION 9 CHANGED (one edit) =================
 * Inherited from s8: floor 3. The entire residual was "cluster B", the second
 * record-fill group in the final block:
 *     ours  lw v0,44(s0) | sw v0,0(s1)    | addiu v0,v0,12 | sw v0,4(s1)
 *     tgt   lw v0,44(s0) | addiu v1,v0,12 | sw v0,0(s1)    | sw v1,4(s1)
 * s8 measured 28 distinct spellings of that 4-statement group (fresh
 * block-scope local, compound split, copy-then-compound, memory re-read,
 * declaration reordering, matched-sibling transplant, MEM_IN_STRUCT_P pointer
 * views); every one scored 3.
 *
 * The edit: the final block's two `{ s32 v0; ... }` scopes are merged into one
 * `{ s32 v0, v1; ... }` scope, and the SAME local `v1` now carries the
 * "record base + 0xC" value in BOTH record-fill groups (cluster A's
 * `v1 = *(arg1+0); v1 += 0xC; *(arg1+4) = v1;` and cluster B's
 * `v1 = v0 + 0xC; *(arg1+4) = v1;`). 3 -> 0.
 *
 * ================= WHY IT WORKS (pass + predicate, READ not guessed) =======
 * sched1 is the pass that was reordering cluster B (s8 established this).
 * s9 identified the exact source-side INPUT that flips its decision:
 *
 *   sched.c:2584   adjust_priority() raises a just-released insn's priority to
 *                  max_priority (== LAUNCH_PRIORITY, 0x7f000001, sched.c:187,
 *                  installed on the insn being scheduled at sched.c:4049) ONLY
 *                  when birthing_insn_p(PATTERN(prev)) is true.
 *   sched.c:2505-2536  birthing_insn_p returns `reg_n_sets[i] == 1` for the
 *                  pseudo i that the insn sets. A pseudo assigned EXACTLY ONCE
 *                  in the function is "birthing"; one assigned twice is not.
 *
 * So the boost is not a property of "being released by your consumer" -- it is
 * a property of the DESTINATION PSEUDO'S SET COUNT, which is pure C-level
 * input. Measured, both traces extracted from the instrumented cc1 .sched dump:
 *
 *   s8 body (add writes a single-set temp, `(set (reg:SI 147) (plus (reg 146) 12))`):
 *       ;; ready list at T-26: 439 (4) 442 (7f000001), now 442 439
 *     -> boosted add wins rank_for_schedule's FIRST test (INSN_PRIORITY,
 *        sched.c:2418); schedule_block is BACKWARD, so being picked at T-26
 *        places the add AFTER the store in program order.  MISMATCH.
 *
 *   s9 body (add writes `v1`, `(set (reg/v:SI 79) (plus (reg/v:SI 78) 12))`,
 *   and v1 is also set by cluster A, so reg_n_sets[79] == 2):
 *       ;; ready list at T-26: 431 (4) 428 (4), now 431 428
 *     -> no boost; the priorities are EQUAL, the tie falls through to the
 *        dependence-class / INSN_LUID tests (sched.c:2420-2463), the store is
 *        picked first, and the add lands BEFORE it.  TARGET ORDER.
 *
 * Note the `/v` flag on reg 79/78 in the s9 dump: they are REG_USERVAR_P
 * pseudos (our named locals), where the s8 form's add wrote a compiler temp.
 * Artifacts: tmp/grind/func_8006A564/s9/f.sched (s9 trace),
 * tmp/grind/func_8006A564/s8/f.sched (s8 trace), .../s9/sweep_f.txt.
 *
 * ================= WHY THIS C IS ORDINARY =================
 * Both of v1's assignments carry a REAL value that the target itself computes
 * and stores: cluster A's `*(arg1+4) = v1` and cluster B's `*(arg1+4) = v1`
 * are both in the target's bytes, and cluster B's v1 materializes as the
 * target's own second register (`addiu v1,v0,12`). Nothing here is written and
 * not read, and nothing is byte-neutral filler. The matched sibling
 * func_8006A1A0 in this same file emits the identical two-register shape
 * (`lw v0,12(s3); addiu v1,v0,12; sw v0,24(sp); sw v1,28(sp)`) from the same
 * named-intermediate C. Declaring one pair of locals for the two consecutive
 * record fills, rather than a fresh pair per fill, is ordinary period C.
 * Full checklist: memory/grind/func_8006A564/self_vet.md.
 */
extern s32 D_800A34F8;
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        *(s16 *)(tile + 8) = (0x5F);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xF);
        *(s16 *)(tile + 0xC) = ((*(s32 *)(arg1 + 0x18)) + 0x19);
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            tile[6] = v0;
        } else {
            v0 = 0x20;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = (*(s32 *)(arg1 + 0x18));
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xE);
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            v0 = (u32)v0 >> 1;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            v0 = (u32)v0 >> 1;
            tile[6] = v0;
        } else {
            v0 = 0x10;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = ((*(s32 *)(arg1 + 0x18)) + 0x40);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xD);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = tile + 0x10;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    {
        s32 v0, v1;
        *(s32 *)(arg1 + 0) = *(s32 *)(tile + 0x28);
        if ((D_800A34F8 & 0xF) == arg2) {
            *(u8 *)(arg1 + 0x2A) = 0;
            *(u8 *)(arg1 + 0x29) = (u32)(*(u8 *)(arg1 + 0x29)) >> 1;
            *(u8 *)(arg1 + 0x2B) = (u32)(*(u8 *)(arg1 + 0x2B)) >> 1;
        } else {
            *(u8 *)(arg1 + 0x2B) = 0x28;
            *(u8 *)(arg1 + 0x2A) = 0x28;
            *(u8 *)(arg1 + 0x29) = 0x28;
        }

        *(s32 *)(arg1 + 0x18) = 0;
        v1 = *(s32 *)(arg1 + 0);
        v1 += 0xC;
        *(s32 *)(arg1 + 0x1C) += 0xF;
        *(s32 *)(arg1 + 4) = v1;

        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);

        v0 = *(s32 *)(tile + 0x2C);
        v1 = v0 + 0xC;
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v1;
        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
