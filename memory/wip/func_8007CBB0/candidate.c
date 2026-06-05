/* func_8007CBB0 — score-41 pure-C draft, build_insns 151 == target 151.
 *
 * Sandbox: `& tools/eng.ps1 sandbox func_8007CBB0 --disable all --keep-cheat-asm` → 41.
 * Prior documented floor (sessions 2–5): 52. Round 6 (2026-06-04) lowered it
 * by 11 via the SOTN-converged indexed-array store form.
 *
 * THE NEW LEVER (round 6) — SOTN-converged indexed-array stores:
 *   Replace per-symbol stores (`D_800F1858 = …; D_800F1868 = …;`) with literal
 *   indexed access off the first symbol (`(&D_800F1858)[0] = …; (&D_800F1858)[1] = …;`),
 *   AND write them in source-order = index-order rather than target's emitted
 *   order. The direct `(&D_800F1858)[N]` form (no intermediate `u32 *base`
 *   variable) measured -11 (52 → 41) on big-packet alone; combined with the
 *   small-packet path's same conversion, the final floor is 41.
 *
 *   Why it works: cc1's allocator picks ONE base register + immediate-offset
 *   addressing for the multi-store block, collapsing what was a per-store
 *   `lui %hi(D_xxx); sw $val, %lo(D_xxx)` cascade into a single base
 *   materialization + 13 register+offset stores. Same mechanism as SOTN's
 *   `D_80037E20[N]` form in `_clr` (src/main/psxsdk/libgpu/sys.c) — direct
 *   structural precedent.
 *
 *   The `u32 *base = &D_800F1858; base[N] = …;` intermediate-pointer variant
 *   measured 45 (vs the direct form's 41) — the intermediate var seemed to
 *   cost cc1 4 points worth of scheduling. The direct `(&D_800F1858)[N]` form
 *   is the SOTN-converged shape and the right one to commit.
 *
 * Lineage of levers that paid off vs the original 149 head_floor:
 *   1. (round 2) m2c-shape 2-arg signature; (void)arg2/arg3 to honor ABI.
 *   2. (round 2) `extern s32 *D_8009BF48;` forward decl before the function
 *      (existing decl at line 837 of display.c is AFTER deployment site).
 *   3. (round 2) goto-based clamps mirroring sibling func_8007CE0C shape.
 *   4. (round 2) `*(u16 *)arg0 & 0x3F` for the flag check (matches target's lhu).
 *   5. (round 6) SOTN-converged `(&D_800F1858)[N]` indexed-array stores
 *      with source-order = index-order for BOTH packet paths (149 → 41 total).
 *
 * What this draft achieves vs target asm:
 *   - arg0 in $t0 (matches), arg1 in $t1 (matches)
 *   - Frame size 0x40 = 64 (matches)
 *   - 151 build_insns == 151 target_insns (matches)
 *
 * What's still off (the residual 41 score):
 *   - List-scheduler INSN_PRIORITY decisions remain in the GPU packet store
 *     sequence (the 0x03FFFFFF constant materialization split + BF48 load
 *     interleave). Same class as cpu_side_move_dir_4 sched.c walls.
 *
 * Move-the-0x03FFFFFF-store-early did NOT help (round 6 measurement):
 *   Moving `(&D_800F1858)[9] = 0x03FFFFFF;` BEFORE `(&D_800F1858)[0] = …;`
 *   regressed 41 → 61. Source store order should match target's emitted order
 *   for the constant-materialization chain.
 *
 * To resume: paste this body in place of the stub at src/display.c:710,
 * confirm 41 via sandbox --disable all --keep-cheat-asm, then either pursue
 * the directed permuter from a clean offset-0 target or BB2_SCHED_DEBUG
 * instrumented cc1 dump.
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
        /* big-packet path: 13 dwords — SOTN-converged indexed-array form */
        (&D_800F1858)[0] = ((u32)(&D_800F1858 + 9) & 0xFFFFFF) | 0x08000000;
        (&D_800F1858)[1] = 0xE3000000;
        (&D_800F1858)[2] = 0xE4FFFFFF;
        (&D_800F1858)[3] = 0xE5000000;
        (&D_800F1858)[4] = 0xE6000000;
        (&D_800F1858)[5] = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);
        (&D_800F1858)[6] = (arg1 & 0xFFFFFF) | 0x60000000;
        (&D_800F1858)[7] = (s32)arg0->word0;
        (&D_800F1858)[8] = *((s32 *)arg0 + 1);
        (&D_800F1858)[9] = 0x03FFFFFF;
        (&D_800F1858)[10] = gpu_GetInfo(3) | 0xE3000000;
        (&D_800F1858)[11] = gpu_GetInfo(4) | 0xE4000000;
        (&D_800F1858)[12] = gpu_GetInfo(5) | 0xE5000000;
    } else {
        /* small-packet path: 6 dwords — SOTN-converged indexed-array form */
        (&D_800F1858)[0] = 0x05FFFFFF;
        (&D_800F1858)[1] = 0xE6000000;
        (&D_800F1858)[2] = (*D_8009BF48 & 0x7FF) | (((arg1 >> 31) << 10) | 0xE1000000);
        (&D_800F1858)[3] = (arg1 & 0xFFFFFF) | 0x02000000;
        (&D_800F1858)[4] = (s32)arg0->word0;
        (&D_800F1858)[5] = *((s32 *)arg0 + 1);
    }
    gpu_StartDmaList((u32)&D_800F1858);
    return 0;
}
