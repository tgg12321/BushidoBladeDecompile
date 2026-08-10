/* func_8007C7A0 — BYTE-EXACT FORM (s8, 2026-08-10, rederive):
 * sandbox --disable all == 0, build_insns 51/51, 21 rules dropped,
 * cheat-asm stripped 153. Verified three times this session on this exact
 * text, edits in place in src/display.c (candidate-ready contract).
 *
 * THIS IS THE PUBLISHED SONY SOURCE TEXT — the sotn-decomp matched get_cs
 * (src/main/psxsdk/libgpu/sys.c, CLAMP house style), transliterated ONLY in
 * the build-specific limits (BB2's library build clamps against halfword
 * globals D_8009BE78/7A and dispatches on the D_8009BE74 range check; SOTN's
 * build uses constant limits + a boolean global). NO construct of any kind:
 * no temp, no alias, no writeback statement — the banned x/tx family is
 * entirely absent. cc1psx emits instruction-identical code from this text
 * (s8 psx_ternary_probe.sh), killing the toolchain-divergence reading.
 *
 * WHY FIVE SESSIONS MISSED IT: the join-temp + writeback (move a3,v0) that
 * every clean if/else spelling collapses is exactly how GCC 2.7.2 expands
 * THIS ternary nesting — outer condition `v < 0` with the self-read in the
 * innermost else arm. s7's ternary forms 3/5 used the INVERTED outer
 * condition (`v >= 0`), which takes a different expand path and folds.
 * s1's "SOTN reference killed" verdict was measured on the round-6 chassis
 * (score 28) whose tail was wrong; on the correct per-arm-return tail the
 * reference is byte-exact.
 *
 * TWIN func_8007C86C: adopt SOTN's get_ce the same way (0xE4000000; NOTE
 * SOTN get_ce's wide arm masks y with 0x1FF not 0x3FF — check the twin's
 * target bytes for which mask its build uses before assuming symmetry). */

/* PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libgpu/sys.c
 * get_cs (CLAMP house style), transliterated only in the limits: this
 * library build clamps against the halfword globals D_8009BE78/D_8009BE7A
 * and dispatches on the D_8009BE74 range check (SOTN's build uses constant
 * limits + a boolean global). */
s32 func_8007C7A0(s16 x, s16 y)
{
    x = x < 0 ? 0 : (x > D_8009BE78 - 1 ? D_8009BE78 - 1 : x);
    y = y < 0 ? 0 : (y > D_8009BE7A - 1 ? D_8009BE7A - 1 : y);
    if ((u32)(D_8009BE74 - 1) < 2U) {
        return 0xE3000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE3000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}
