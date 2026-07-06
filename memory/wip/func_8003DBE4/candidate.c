/* func_8003DBE4 (code6cac_c2.c) — CLEAN pure-C candidate (cheat_count 0).
 * sandbox --disable all = 21, identical with or without the committed src's
 * `s32 buf[2]` dead-array line (verified empirically this session — removing
 * the array and re-running sandbox produces the SAME score 21, proving the
 * array plays no role in the honest cheat-invisible measurement; only a real
 * build/verify-oracle full-link SHA1 check can show it "helping").
 *
 * ALL 21 diffs are the FRAME SIZE cascade: target frame = 64 bytes (0x40),
 * this clean body's natural frame = 56 bytes (0x38) — an 8-byte UNUSED gap.
 * Every instruction in the function is otherwise byte-identical; the 8-byte
 * shift moves every sw/lw stack offset and every branch/jump target by the
 * same delta, which is why a single missing frame slot inflates to a
 * 21-instruction diff count.
 *
 * PROOF the 8 bytes are genuinely UNUSED in the target (not a dropped real
 * local): objdump -d --disassemble=func_8003DBE4 on build/src/code6cac_c2.o
 * (the pristine byte-matched reference, still built from the pre-ban
 * `buf[2]` source on main) shows NO sw/lw instruction anywhere in the
 * function touching stack offsets 0x10-0x17 (the 8 bytes in question) — only
 * the callee-saved-register spill area (0x18-0x38) and the incoming
 * 5th-argument read at 0x50/0x48 (frame-relative) are ever accessed.
 * `m2c --valid-syntax` on asm/funcs/func_8003DBE4.s independently confirms
 * this: it reconstructs the entire function with ZERO stack-resident
 * locals — every value lives in a register in m2c's own output.
 *
 * This means the 8-byte gap cannot correspond to any real local variable —
 * a genuine spilled/stack-resident local would leave a load/store
 * instruction at that offset, and none exists anywhere in the target's
 * compiled bytes. The ONLY C constructs that reproduce an unused frame
 * region this way are "declared but never referenced" locals (scalar or
 * array), which `.claude/rules/dead-vars-local-array.md` FORBIDS outright
 * for the unwritten/unread case (the 2026-07-01 carve-out is explicitly
 * for WRITTEN-never-read arrays with target dead-store evidence — this
 * function's target has neither).
 *
 * This is the SAME shape as the already-escalated AddTbpOfst_80047EE8 /
 * InitHiraRmd_80047FBC cluster (memory/wip/AddTbpOfst_80047EE8, commit
 * 3d7e222d): a POLICY-grade unused-frame gap, not a single-function grind.
 * See notes.md for the full lever-exhaustion log. */
void func_8003DBE4(s32 arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4) {
    s32 limit;
    s32 step;
    s32 *colors;
    s32 i;

    colors = arg2;

    if (arg4 != 0) {
        limit = arg1;
    } else {
        limit = arg1 - 1;
    }

    if (D_800F6656 & 1) {
        step = D_80090600;
    } else {
        s32 base_val;
        if (game_GetPlayerCount() == 0) {
            base_val = 0x6590;
        } else {
            base_val = 0x55F0;
            step = base_val - arg0; /* redundant: steers base_val into v0 (matches target reg-alloc) */
        }
        step = base_val - arg0;
    }

    if (step < 0) {
        return;
    }

    if ((u32)arg3 < (u32)step) {
        step = (s32)((u32)arg3 / (u32)arg1);
    } else {
        step = step / arg1;
    }

    i = 0;
    colors = (s32 *)((u8 *)colors + (D_800A36AC & 1) * 24);

    if (limit > 0) {
        u32 rgb_mask = 0xFFFFFF;

        do {
            s32 idx = func_80052C28((u32)arg0 >> 2, 2);
            if (idx < 0x1000) {
                s32 *pal = (s32 *)((u32)idx * 4 + D_800A378C);
                s32 tmp;
                *colors = (*colors & 0xFF000000) | (*pal & rgb_mask);
                tmp = (*pal & 0xFF000000) | ((u32)colors & rgb_mask);
                *pal = tmp;
                colors = (s32 *)((u8 *)colors + 0x30);
                tmp = limit - 1; /* reuse tmp (multi-set) so limit-1 isn't a loop.c movable -> recomputed inline, not hoisted (see rule defeat-licm-hoist-var-reuse) */
                if (i == tmp) {
                    D_800905F8 = idx;
                }
            }
            arg0 += step;
            i++;
        } while (i < limit);
    }

    if (arg4 != 0) {
        func_8003DDF8((u32)colors);
    } else {
        s32 *pal = (s32 *)D_800A378C;
        *colors = (*colors & (s32)0xFF000000) | (*(s32 *)((u8 *)pal + 0x3FEC) & 0xFFFFFF);
        *(s32 *)((u8 *)pal + 0x3FEC) = (*(s32 *)((u8 *)pal + 0x3FEC) & (s32)0xFF000000) | ((u32)colors & 0xFFFFFF);
    }
}
