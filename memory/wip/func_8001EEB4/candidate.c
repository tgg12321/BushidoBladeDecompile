/* WIP candidate: sandbox --disable all = 3 (HEAD floor 6)
 *
 * Lever applied: hoist-call-arg-local for second jal (func_80021A98).
 *   `s32 idx2 = D_800A3748;` declared FIRST in the inner block before
 *   the *(entry+0x5E)=1 store and the call. Per
 *   [[hoist-call-arg-local-flips-jal-delay]] this gives idx2's RTL
 *   pseudo a low LUID, scheduling its `lb $a0,...` setup EARLIER.
 *   reorg then picks `sh $s0, 94($s1)` (the store) as the natural
 *   delay-slot fill for the jal — exactly target's pattern.
 *   Effect: retires the `reorder 36,38,40,39,37,41 @ 36-41` regfix
 *   rule (3 of 6 score diffs eliminated). Insn count 60→58
 *   matches target's 59 ±1.
 *
 * NOT a cheat-by-spelling: idx2 has semantic content (the index
 * passed as the call's arg0), human-programmer-naturally-written,
 * matches sanctioned named-intermediate-declaration-order family
 * per [[no-new-park-categories]] / [[hoist-call-arg-local-flips-jal-delay]].
 *
 * Residual gap = 3 diffs, ALL in the comparison block at idx 14-22:
 *   - lhu uses $v1; target uses $a1
 *   - missing `andi $v1, $a1, 0xFFFF` instruction at idx 16
 *   - addiu in delay slot uses $v1; target uses $a1
 *
 * The 3 diffs are coupled: forcing GCC to emit the andi would
 * naturally allocate $v1 to the andi result and push the lhu into
 * $a1 (since $v0=const, $v1=mask, lhu goes to next-free $a1).
 *
 * Forms tested (all kept score=3 — combine folds the redundant mask):
 *   - u16 a1 (HEAD form)
 *   - s32 a1
 *   - u32 a1
 *   - register u32 a1
 *   - s32 a1_raw = *(s16*)..; u16 a1 = (u16)a1_raw;  (lh+cast → combined back to lhu)
 *   - u32 a1_word + u32 a1_masked = a1_word & 0xFFFF;  (combine folds mask)
 *   - (u32) cast on the comparison
 *   - goto-end early-exit form (no codegen change)
 *
 * Sibling functions with the same `lhu/andi/beq` pattern in target:
 *   - func_8001A820 (INCOMPLETE, body empty — no source clue)
 *   - func_8001F938 (INCOMPLETE, score 11, 13 rules)
 *   - calc_loc_mat_fw_80055B60 (ASM-STRUCTURAL park bucket)
 *
 * NO sibling has matched this pattern in pure C yet. The andi
 * appears in target asm where the natural cc1 emission elides it
 * (lhu already zero-extends; combine pass folds the redundant
 * mask away).
 *
 * Resume hypotheses for the andi materialization:
 *   1. Find a source pattern that defeats combine's
 *      `(and (mem:HI ...) 0xFFFF)` → `(mem:HI ...)` fold.
 *      Tested fold-defeats (all collapsed): opaque-mask-var
 *      (cheat-by-spelling concern), DImode round-trip
 *      (forbidden), explicit (u16)/(s16) re-cast.
 *   2. Investigate whether the function's variable is read via a
 *      pattern that maintains SImode through combine (e.g. struct
 *      field of int type with cast to u16 at READ site).
 *   3. Read GCC 2.7.2 combine.c source for the specific simplify
 *      rule that folds the andi here — find what RTL shape escapes
 *      it. Use instrumented cc1 (BB2_ALLOC_DEBUG, etc.) to dump
 *      combine's input/output for the lhu region.
 */
void func_8001EEB4(void) {
    s8 idx = D_800A3748;
    u8 *entry = (u8 *)&D_80101EC8 + idx * 0x44C;
    u16 a1 = *(u16 *)(entry + 0x6A);

    if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
        (u32)((s32)a1 - 0x17) >= 2 && *(s16 *)(entry + 0x96) == 0) {
        func_800218C8(D_800A3748);
        {
            s32 ret = func_80021A3C(D_800A3748, *(s16 *)(entry + 0xA));
            s32 idx2 = D_800A3748;
            *(s16 *)(entry + 0x5E) = 1;
            func_80021A98(idx2, ret, 1);
        }
        *(s16 *)(entry + 0x26C) = 1;
    }

    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x11;
}
