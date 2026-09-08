/*
 * CANDIDATE -- func_80035280 (src/code6cac_b.c) -- s5 (2026-09-08, rederive)
 *
 * FLOOR 15 -> 0. `sandbox func_80035280 --disable all` prints score 0,
 * target_insns 108 == build_insns 108, and `verify-oracle` builds the whole EXE
 * to SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. This is the body on main.
 *
 * WHAT S1-S4 HAD WRONG, AND IT WAS THE WHOLE FUNCTION. The residual was
 * correctly attributed to loop.c:1631 hoisting the 0x91A2B3C5 (/1800) magic out
 * of loop 2, and the gate is exactly what s4 wrote down:
 *     threshold * savings * m->lifetime  >=  insn_count
 * with savings and lifetime both pinned at 1. The error was the value of
 * `threshold`. s1/s2 measured it as 122 and s4 re-derived 122 from loop.c:532
 * (`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`), concluding loop 2 needed
 * insn_count >= 123 against a natural maximum of 62 -- i.e. unreachable.
 * THAT NUMBER WAS MEASURED ON THE PRE-`-msoft-float` CHASSIS. `-msoft-float`
 * (adopted 2026-09-07, [[softfloat-adoption]]) marks the 32 FP registers fixed,
 * which halves n_non_fixed_regs and therefore halves the threshold.
 * Re-measured this session by padding loop 2 and reading the -dL dump:
 *     insn_count 55 -> "moved"          57 -> "moved"
 *     insn_count 59 -> "not desirable"  61, 63, 123 -> "not desirable"
 * so threshold = 58 on the shipped chassis and loop 2 needs insn_count >= 59.
 * The gap was never 68 insns. It was FOUR.
 * (tmp/grind/func_80035280/s5/pad*.c, dumps in tmp/grind/func_80035280/dumps/.)
 *
 * HOW THE FOUR INSNS ARE PAID FOR. Loop 2 stages each of the three clock fields
 * and the raw record byte through a fresh named intermediate before its store
 * (`mn`, `sc`, `hs`, `t`). The three u8 intermediates each contribute one QImode
 * truncation insn at loop time and `t` contributes the SImode load temp, taking
 * loop 2 from 55 real insns to 59; combine folds all four copies away, so
 * build_insns is 108 -- identical to the target -- and nothing but the LICM
 * decision changes. With the hoist refused, the lui/ori for 0x91A2B3C5
 * materialises inside the loop, $t2 goes back to the mfhi temp, the load-delay
 * `nop` the target fills with `ori` disappears, and all 15 residual points go
 * together, exactly as the s4 header predicted they would.
 *
 * INHERITED CONSTRUCTS, ALL RE-MEASURED THIS SESSION AND ALL STILL LOAD-BEARING
 * (dropping each one from THIS body, everything else unchanged):
 *   `i = 0;` hoisted above `for (; i < 3; i++)`  -> 2 diffs   (s5/m1.c)
 *   `f = &D_80106A73;` alias dropped             -> 50 diffs  (s5/m2.c)
 *   flags0/flags1/flags2 collapsed to one local  -> 44 diffs  (s5/m4.c)
 *
 * SPELLINGS MEASURED AND REJECTED THIS SESSION (all at loop-2 insn_count 58,
 * one short of the gate, all still 29 diffs): u8/u16/s8 intermediates in nested
 * blocks (s5/vK,vL,vM,vN,wA-wD,wF), a dest pointer local (s5/yB), an index local
 * (s5/yC), a source pointer local (s5/yD), `((s32 *)(base + i*8))[1]` reads
 * (s5/yF). Declaring the intermediates at the top of the loop body WITHOUT
 * interleaving the stores collapses the four loads to one and costs 120 diffs
 * (s5/vF, vG). Spelling the symbol inline at the use sites does reach the gate
 * but creates a second address movable: 39 diffs (s5/xA, xB, xD), and
 * recomputing the record pointer inside the loop destroys the $a2 giv: 35 diffs
 * (s5/xC).
 *
 * The `base = (u8 *)&D_80106A58;` byte-view is unchanged from s1 and is the same
 * spelling the COMPLETED-C sibling func_8003C714 ships at src/code6cac_c2.c:685
 * with `extern s32 D_80106A58;` untouched -- the aggregate-merge/declaration
 * question is not part of this residual.
 */
void func_80035280(void) {
    s32 *p;
    /* FAKE: `f` is a redundant second handle to D_80106A73 -- the walker `src`
     * could be spelled `&D_80106A73 - 3` directly; mechanism: cse.c materialises
     * the %hi/%lo address pair once for `f` and reuses that pseudo for both the
     * `src[3]` flag reads and the loop-1 walker, which is the base-register
     * shape the target carries ($a1 = &D_80106A73, $a2 = $a1 - 3); spelling it
     * directly measures 50 diffs (tmp/grind/func_80035280/s5/m2.c),
     * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s1-s5 */
    u8 *f;
    u8 *src;
    /* FAKE: typed re-view of the global D_80106A58 as the byte-strided base of
     * the three 8-byte clock records, hoisted above the loop rather than
     * respelled at each use; mechanism: loop.c move_movables hoists the
     * address-materialisation movable into the loop-2 preheader exactly once,
     * giving the target's single $a2 record cursor -- writing the symbol inline
     * at the use sites creates a second address movable and measures 39 diffs
     * (tmp/grind/func_80035280/s5/xB.c),
     * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s1-s5 */
    u8 *base;
    s32 i;
    s32 flags;
    /* FAKE: the flag merge is staged through one fresh named intermediate per
     * merged bit instead of re-using a single accumulator; mechanism:
     * local-alloc.c:472's `reg_n_deaths == 1` eligibility test -- one death per
     * pseudo makes each merge result eligible for the target's seat, where a
     * single re-used accumulator has three deaths and is refused (single
     * accumulator measures 44 diffs, tmp/grind/func_80035280/s5/m4.c),
     * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s3 + s5 */
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    i = 0;
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
        ((u8 *)p + i)[0x17] = *src;
        ((u8 *)p + i)[0x1D] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        /* FAKE: the three clock fields and the raw record byte are each staged
         * through a fresh named intermediate before their store; mechanism:
         * loop.c:1631's move_movables desirability test
         * `threshold * savings * m->lifetime >= insn_count`. The 0x91A2B3C5
         * (/1800) magic is a movable with savings 1 and lifetime 1, and
         * loop.c:532 fixes `threshold = (loop_has_call ? 1 : 2) *
         * (1 + n_non_fixed_regs)` = 58 on this -msoft-float configuration, so
         * the constant is hoisted into the loop-2 preheader for any
         * insn_count <= 58. These four intermediates raise loop 2's real-insn
         * count from 55 to 59 (measured in the -dL dump,
         * tmp/grind/func_80035280/s5/last.loop), which is the first count that
         * refuses the hoist and leaves the lui/ori inside the loop exactly as
         * the target carries it. Every one of them holds a real value that is
         * stored to the target's own bytes, and combine folds the copies away
         * (build_insns 108 == target_insns 108),
         * lever-exhaustion: memory/grind/func_80035280/hypotheses.md s1-s5 */
        u8 mn;
        u8 sc;
        u8 hs;
        s32 t;

        mn = *(s32 *)(base + i * 8 + 4) / 1800;
        ((u8 *)p)[i * 4 + 0x21] = mn;
        sc = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        ((u8 *)p)[i * 4 + 0x22] = sc;
        hs = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x23] = hs;
        t = base[i * 8];
        ((u8 *)p)[i * 4 + 0x24] = t;
    }
}
