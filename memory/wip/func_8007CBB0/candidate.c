/* func_8007CBB0 — score-52 pure-C draft, build_insns 151 == target 151.
 *
 * Sandbox: `& tools/eng.ps1 sandbox func_8007CBB0 --disable all` → 52.
 * Prior session's documented floor was 76; this session's draft reached 52.
 *
 * Levers that paid off vs the prior 76 floor:
 *   1. m2c-shape signature: `(arg0, arg1)` only — arg2/arg3 are (void)-silenced
 *      4-arg formal so cc1 sees the natural ABI; the function ignores them.
 *   2. Add the `extern s32 *D_8009BF48;` BEFORE func_8007CBB0 (the existing
 *      decl in display.c is later in the file — without forward decl, GCC
 *      assumed implicit-int and the `*D_8009BF48` deref emitted wrong codegen).
 *   3. Take address of D_800F187C ONCE into a local `u32 *p187C` and reuse
 *      it for both the chain-header calculation AND the `*p187C = 0x03FFFFFF`
 *      store. cc1 then keeps the symbol-ref pointer alive in a register ($a2
 *      in mine, matching target's $a2) saving one `lui $at; sw ..., %lo` pair.
 *      Single-source change from 67 → 53.
 *   4. `*(u16 *)arg0 & 0x3F` for the flag check (forces `lhu` to match target's
 *      `lhu $v0, 0x0($t0)` instead of `(u32)arg0->word0 & 0x3F`'s `lw`). Drops
 *      one cycle. 53 → 52.
 *   5. Restore goto-based clamps (mine had nested-if form which was 87; goto
 *      form gave 67 then 52 after subsequent levers).
 *
 * What this draft achieves vs target asm:
 *   - arg0 in $t0 (matches), arg1 in $t1 (matches)
 *   - $s0 holds 0xE3000000, $s1 holds 0xE5000000 (matches)
 *   - Frame size 0x40 = 64 (matches) — automatic with the (void)arg2/arg3 hold
 *   - 151 build_insns == 151 target_insns (matches)
 *   - Big-packet path: 13 stores in correct order F1858/F1868/F185C/F1860/
 *     F1864/F1870/F186C/F1874/F187C/F1878/F1880/F1884/F1888 (matches)
 *   - Small-packet path: 6 stores in correct order F1858/F185C/F1864/F1860/
 *     F1868/F186C (matches)
 *   - Both clamps emit `lh; nop; addu v1,v0,zero; addiu v0,v0,-1; slt; bnez`
 *     pattern (matches target shape, including the "save BE78 into $v1" trick)
 *
 * What's still off (the residual 52 score):
 *   - The big-packet path's lui/and/or/sw scheduling differs from target.
 *     Target interleaves the constant-materialization (lui $a3 0x3FFFFFF
 *     and ori $a3 0xFFFF) with the BF48 load chain. Mine emits them adjacent.
 *   - The lui $a3 0x3FFFFFF ; ori 0xFFFF pair: target splits them by ~20 insns
 *     letting BF48 load + scheduling happen in between, mine emits them
 *     together pre-stores.
 *   - The small-packet path has similar list-scheduler diffs.
 *
 * These are list-scheduler INSN_PRIORITY decisions — same shape as the
 * cpu_side_move_dir_4 / marionation_Exec scheduling-wall sessions (see
 * `[[register-alloc-pure-c]]` § session-6/7/8 BB2_SCHED_DEBUG). Closing
 * the residual 52 would need either:
 *   (a) directed permuter from a clean offset-0 target with this base
 *       (each new candidate scored honestly without the cheat-asm strip
 *       crashing — verify the sandbox doesn't try to strip non-asm cheats);
 *   (b) BB2_SCHED_DEBUG / BB2_PRIO_DEBUG instrumented cc1 dump of this
 *       function's RTL to identify which pseudo's priority needs nudging;
 *   (c) C-source restructure that changes INSN_PRIORITY of the BF48 load
 *       or the 0x03FFFFFF materialization (without adding emitted bytes).
 *
 * What is NOT applicable / would be a cheat:
 *   - Adding `s32 _pad[N]` to force frame size (frame already matches at 64).
 *   - Adding `__asm__ volatile("" ::: "memory")` between stores ([[inline-asm-policy]]).
 *   - Marking globals volatile ([[split-read-defeats-hoist]] § 2026-05-31 update).
 *   - Dead conditional stores `if (cond) { v = x; } v = x;` ([[no-new-park-categories]]).
 *
 * Sandbox baseline gotcha (file-state corruption):
 *   The committed source has `void func_8007CBB0(s32 a, s32 b, s32 c, s32 d)
 *   { (void)a;...; }` as the body and `func_8007CBB0: replace_with_asmfile
 *   "asm/funcs/func_8007CBB0.s"` in asmfix.txt. `sandbox --disable all`
 *   strips both the cheat-asm-injection (371 lines) AND the rule, producing
 *   a build_insns=2 / score=149 measurement. Once a real C body is in place,
 *   sandbox measures the honest cheat-free distance (152 → 151 → 52).
 *   This is why the prior parked-state measurement read 149 not 76.
 *
 * To resume: paste this body in place of the stub, sandbox to confirm 52,
 * then either drive the permuter from this base or instrument cc1 to find
 * the scheduling lever.
 */
typedef struct {
    u32 word0;
    s16 w;
    s16 h;
} _GpuChunk_CBB0;

extern u32 D_800F1858;
extern u32 D_800F185C;
extern u32 D_800F1860;
extern u32 D_800F1864;
extern u32 D_800F1868;
extern u32 D_800F186C;
extern u32 D_800F1870;
extern u32 D_800F1874;
extern u32 D_800F1878;
extern u32 D_800F187C;
extern u32 D_800F1880;
extern u32 D_800F1884;
extern u32 D_800F1888;
extern s32 *D_8009BF48;
extern u32 gpu_GetInfo(u32 a0);
extern void gpu_StartDmaList(u32 a0);

s32 func_8007CBB0(_GpuChunk_CBB0 *arg0, u32 arg1, s32 arg2, s32 arg3) {
    s16 temp_a0;
    s16 temp_a1;
    s16 var_v0;
    s16 var_v1;
    (void)arg2; (void)arg3;

    temp_a0 = arg0->w;
    if (temp_a0 < 0) goto w_neg;
    if (D_8009BE78 - 1 < temp_a0) {
        var_v0 = D_8009BE78 - 1;
    } else {
        var_v0 = temp_a0;
    }
    goto w_done;
w_neg:
    var_v0 = 0;
w_done:
    arg0->w = var_v0;

    temp_a1 = arg0->h;
    if (temp_a1 < 0) goto h_neg;
    if (D_8009BE7A - 1 < temp_a1) {
        var_v1 = D_8009BE7A - 1;
    } else {
        var_v1 = temp_a1;
    }
    goto h_done;
h_neg:
    var_v1 = 0;
h_done:
    arg0->h = var_v1;

    if ((*(u16 *)arg0 & 0x3F) || ((u16)arg0->w & 0x3F)) {
        /* big-packet path: 13 dwords */
        u32 *p187C = &D_800F187C;
        D_800F1858 = ((u32)p187C & 0xFFFFFF) | 0x08000000;
        D_800F1868 = 0xE6000000;
        D_800F185C = 0xE3000000;
        D_800F1860 = 0xE4FFFFFF;
        D_800F1864 = 0xE5000000;
        D_800F1870 = (arg1 & 0xFFFFFF) | 0x60000000;
        D_800F186C = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);
        D_800F1874 = (s32)arg0->word0;
        *p187C = 0x03FFFFFF;
        D_800F1878 = *((s32 *)arg0 + 1);
        D_800F1880 = gpu_GetInfo(3) | 0xE3000000;
        D_800F1884 = gpu_GetInfo(4) | 0xE4000000;
        D_800F1888 = gpu_GetInfo(5) | 0xE5000000;
    } else {
        /* small-packet path: 6 dwords */
        D_800F1858 = 0x05FFFFFF;
        D_800F185C = 0xE6000000;
        D_800F1864 = (arg1 & 0xFFFFFF) | 0x02000000;
        D_800F1860 = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);
        D_800F1868 = (s32)arg0->word0;
        D_800F186C = *((s32 *)arg0 + 1);
    }
    gpu_StartDmaList((u32)&D_800F1858);
    return 0;
}
