/* func_80062020 (text1b.c) — CANDIDATE, sandbox distance 0 (s7, 2026-08-30).
 *
 * MEASURED THIS SESSION on the live chassis, with this body pasted over the
 * INCLUDE_ASM line at src/text1b.c:3853:
 *   sandbox func_80062020 --disable all -> score 0, build_insns 38,
 *     target_insns 38, rules_dropped 0
 *   verify-oracle -> ok true, build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *     == oracle (full clean-driver build + link)
 * (The prior floor-4 uncontested body is preserved at rejected/… nothing —
 *  it is exactly this body with the last store spelled `p[0] = 0;`; the s1-s6
 *  header history for it is in evidence.md and in git history of this file.)
 *
 * WHY THE LAST STORE IS SPELLED THROUGH THE SYMBOL, not through `p`:
 * admitted under .claude/rules/proven-spelling-class-reconstruction.md by OWNER
 * RULING 6a of the 2026-08-30 escalation batch (docs/grind/decisions.md:14836),
 * which made that rule's 4-point criterion the operative bar for this function.
 * The full adjudication (all four criteria answered, with measurements) is in
 * memory/grind/func_80062020/self_vet.md. In one line: MIPS legitimize_address
 * accepts (symbol_ref + reg) as an address (LO_SUM, symbol never enters a
 * register) but folds (symbol_ref + reg + const) into la(sym+K) and force_regs
 * it, so one expression shape can emit EITHER a shared base+disp row for all
 * three columns OR a per-column symbol-relative address for all three — never
 * target's mix of both on one row (7 tree-node classes measured, s5/s6/s7).
 * The target mixes them ⇒ the 1998 source wrote that address in two shapes.
 *
 * The rest of the body is unchanged from the s2 floor-4 form: KEY LEVER 1 (s1)
 * reads the source through a FIXED-base indexed form so GCC strength-reduces it
 * to one walking giv; KEY LEVER 2 (s2) reuses `ofs` (the loop byte-offset biv,
 * allocated to $v1) to carry the terminator index, which seats the index in the
 * target's register.
 */

void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    s32 *p;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i;
    ofs = ofs + i;
    ofs = ofs << 2;
    p = (s32 *)((u8 *)&D_800F1198 + ofs);
    p[2] = 0;
    p[1] = 0;
    *(s32 *)((u8 *)&D_800F1198 + ofs) = 0;
}
