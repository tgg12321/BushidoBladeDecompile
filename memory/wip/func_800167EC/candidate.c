/* memory/wip/kgm_clamp_patch_init/candidate.c
 *
 * Drop-in replacement for src/ings.c::kgm_clamp_patch_init.
 *
 * CLEAN pure-C form: ZERO register-asm pins, ZERO inline asm, ZERO volatile
 * casts (HEAD carried 4 pins + 1 `addu` asm-move + 3 volatile byte/word casts).
 *
 * KEY FINDING (2026-06-14, worker orch0614b-3): the volatile casts and the asm
 * move were ALL UNNECESSARY for the store SHAPE:
 *   - GCC 2.7.2 does NOT combine adjacent byte stores `p[8]=0; p[9]=0;` into an
 *     `sh` — it emits two separate `sb` naturally. So the `(volatile u8*)` casts
 *     (added to force separate stores) are not needed.
 *   - The `addu %0,%1,$zero` asm move of tmp->p is also unnecessary.
 * This clean form produces the TARGET INSTRUCTION COUNT (31, vs HEAD-stripped 28)
 * and the correct separate-`sb` store shape.
 *
 * REMAINING GAP (full-build SHA1 mismatch — masked sandbox shows 0 but registers
 * differ): pure register-allocation + induction-variable base.
 *   target:  c in $a1, i in $a0, p in $v1; loop iterates $v1 with offsets 8,9,0xC
 *   mine:    c in $a2, i in $a1; GCC re-bases the induction var to (p+0xC) in $a0
 *            and stores at -4($a0), -3($a0), 0($a0)
 * The 4 HEAD pins (i=$a0, c=$a1, p=$v1) were doing exactly this RA work; the
 * `addu` move pinned p's base so the induction var stayed at p (offsets 8,9,0xC)
 * instead of being hoisted to p+0xC.
 *
 * Declaration order (c before i) did NOT change the full-build codegen (it only
 * moved the masked score to 0 by coincidence of normalization). The residual is
 * a genuine RA/induction-base wall — needs a legitimate Lever A/B/C that:
 *   (1) lands c in $a1 not $a2, and
 *   (2) keeps the loop induction variable based at p (offsets 8,9,0xC) rather
 *       than GCC hoisting it to p+0xC.
 * Do NOT reach for the volatile casts / asm move / pins again — they are cheats.
 */
void kgm_clamp_patch_init(void) {
    u32 c = 0x1A5E0;
    s32 i = 0;
    u8 *p;
    g_file_vram_timer = 0;
    g_file_flags = 0;
    p = (u8 *)&g_file_disc_size;
    *(s32 *)p = 0x7007;
    g_file_disc_type = 0;
    do {
        p[8] = 0;
        p[9] = 0;
        *(u32 *)(p + 0xC) = c;
        p += 8;
        i++;
    } while (i < 3);
    D_80106A5C = 0x6978;
    func_8001945C();
}
