/* func_80056FE8 / ang_hosei_80056FE8 (src/text1b.c) — s7 SOLVER: BYTE MATCH.
 * sandbox --disable all = score 0, build_insns 43 == target 43;
 * FULL BUILD SHA1 == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * (verify-oracle run this session with this body in src/text1b.c).
 *
 * ZERO rules, ZERO cheat-asm, ZERO match-hack constructs: no variable reuse,
 * no dead ops, no register pins, no volatile, no FAKE annotation needed. This
 * is the plain DRY form of the function — each dispatch arm adds its own angle
 * adjustment straight into the `base` accumulator, and the return re-reads
 * *arg0 inline.
 *
 * HOW s7 GOT HERE (the six-session wall dissolved by the RA solver):
 *   s5/s6 froze at floor 9 on a "double variable-reuse tail" chassis
 *   (base += var_v0; var_v0 = *arg0;). The whole residual was one $a1<->$a2
 *   swap: our a2local (pseudo 73, the *arg0 struct pointer) was colored BEFORE
 *   base (pseudo 77) in global.c and grabbed $a1, forcing base to $a2.
 *   s6 killed the copy-preference route ($a1 has no ABI anchor in a 1-arg leaf)
 *   and the live-range route, and escalated.
 *   s7 ran the mandated solver triage:
 *     goal_from_tgt.py classify text1b func_80056FE8   -> FIRST DIVERGENCE: RA
 *     goal_from_tgt.py goal --model ...                -> goal {"73":6,"77":5}
 *     inverse.py global --goal ... --depth 2           -> REACHABLE, 12 vectors
 *   Top vectors were all reg_n_refs perturbations; vector #3 was
 *   "[refs_up] pseudo 77: refs 4->8". Spelling that honestly = stop caching the
 *   per-arm adjustment in a shared temp and instead accumulate it into `base`
 *   inside each arm, which gives `base` a def+use in every arm.
 *   MEASURED CONFIRMATION (extract.py on this body):
 *     base   (p77) refs 4->8, livelen 21->23, pri 3809 -> 10434   [colored 1st]
 *     a2local(p73) refs 6,    livelen 14,     pri 8571            [colored 2nd]
 *   base now out-prioritizes a2local, is colored first, takes $a1 (pass 0
 *   excludes only $v0/$v1/$sp and $a0 via regs_someone_prefers from pseudo 72's
 *   $a0 arg preference); a2local then takes $a2 = target's allocation exactly.
 *   The 43-insn schedule survives because the arm-local `addu`s cross-jump-merge
 *   at the join (build_insns 43 == target, no nop delta).
 *
 * Three spellings all measured score 0 this session (e1/e2/e3 in
 * tmp/grind/func_80056FE8/s7/): e1 kept the old var_v0 temp AND added the
 * per-arm `base += var_v0`; e2 dropped the temp in the arms but kept a named
 * local for the tail reload; e3 (THIS body) inlines the tail reload too. e3 is
 * the cleanest and is what is in src/text1b.c.
 */
s32 func_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((*((s16 *) (arg0 + 0x5E))) == 0) {
            base += (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            base += (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        base += (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    return base + (*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C;
}
