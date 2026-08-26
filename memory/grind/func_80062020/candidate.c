/* 2026-08-24 MIGRATION NOTE: HEAD is now INCLUDE_ASM — migrated in
   a7892ba2 (2026-08-24 sweep 2); all rules retired and all in-source cheat-asm removed
   from main. Statements below about "HEAD", pins, rules carried, or
   "applied to src" describe the PRE-MIGRATION tree (banked at
   retired-chassis-2026-08/body.c). This body must be pasted over the
   INCLUDE_ASM line before any sandbox re-measure. */
/* func_80062020 (text1b.c) — candidate, honest pure-C floor = 4 (sandbox --disable all)
 *
 * Clean pure C: 0 register-asm pins, 0 rules, 0 dead vars, no dual-spelling.
 * LOOP BODY matches target 100% (25/25 insns). Epilogue register allocation now
 * matches target 100% (index in v1, base &D_800F1198 in v0, cols b,c via 4/8(v0)).
 *
 * KEY LEVER 1 (s1): read source via FIXED-base indexed form
 *   *(s32*)((u8*)arg0 + ofs + K)   [NOT the walking a0[K] form]
 * so GCC strength-reduces source into ONE walking giv (0/4/8(a0), a0+=12).
 *
 * KEY LEVER 2 (s2): REUSE `ofs` (the loop's byte-offset biv, allocated to v1)
 * to hold the terminator index (12*count is also a byte offset -> semantically
 * the same value). This biases RA to keep the terminator index in v1 (target),
 * instead of v0. Dropped floor 10 -> 4. Everything now matches EXCEPT col a's
 * addressing: mine emits `sw zero,0(v0)` (reuses base pointer v0); target
 * recomputes `lui at,%hi(1198); addu at,at,v1; sw zero,%lo(1198)(at)` (keeps the
 * raw index v1 live). See hypotheses.md — the residual is the col-a partial-CSE
 * addressing-mode split.
 *
 * s3 (structural): the CSE-defeat lever is KILLED. Store-order permutations
 * (c,b,a=4, c,a,b=5, a,b,c=5) all fold col a onto the base pointer v0 — the
 * base-pointer CSE is store-order-invariant. Type/width distinction is
 * unavailable (all 3 are `sw` of 0; a differently-typed view of the same lvalue
 * is the banked dual-spelling). No intervening dependency exists in a 3-word
 * constant-zero terminator. Structural axis exhausted (s1/s2/s3); floor flat at
 * 4. Frontier -> permuter (confirm no non-cheat byte-0 form), then
 * endgame-lock-disposition OWNER-ESCALATION.
 *
 * s4 (permuter): axis KILLED. Two fresh-seed campaigns (clean offset-0
 * target.o). Chassis A (floor-4 base) hit byte-0 only via the same-lvalue
 * dual-spelling (rejected/epilogue-permuter-s4-dualspelling-chain.c). Chassis B
 * (two-object 119C-anchor, base_score 20, col a matching) plateaued at 15 over
 * 45,307 iters, no byte-0. Every sanctioned axis dead; OWNER-ESCALATION filed
 * (docs/grind/decisions.md 2026-07-24); returned owner-gated. This IS the best
 * form (clean floor-4 pure C, 0 rules) and stays on main.
 *
 * s5 (synthesis): PASS ATTRIBUTION CORRECTED. The `.rtl` post-expand dump shows
 * `p[0]` is already `(set (mem (reg 76)) 0)` AT EXPAND (insn 112) � the col-a
 * "fold" is an RTL-expansion / MIPS legitimize_address decision keyed on the C
 * TREE SHAPE, not a CSE decision. There is no fold to defeat, so every
 * CSE-defeat-style lever is a category error here. New expand-time law
 * (5 tree shapes measured, see hypotheses.md s5 table): force_reg shapes
 * (pointer var, struct COMPONENT_REF, 1-element-array member) make ALL THREE
 * stores base+disp including offset 0; symbol-folding shapes (2D array
 * `arr[i][K]`) fold the column into the symbol for ALL THREE and never share a
 * base. Target mixes both on one element; no uniform tree shape can. Aggregate/
 * tree-shape axis KILLED; solver axis measured inapplicable (residual is PRE-RA:
 * 35 insns vs 38). A struct-row declaration IS byte-free in the pointer idiom
 * (score 4, identical to this form) � so the object model is not the obstacle.
 * Frontier reset to FORENSICS: recover the original object model from sibling
 * byte evidence (the func_800651F0 ruling's standard), then re-classify.
 *
 * s6 (synthesis): FRONTIER F1 EXECUTED AND RESOLVED. The table has exactly one
 * consumer (func_800620B8); it addresses all three columns identically
 * (per-column symbol + byte-index, LO_SUM), reading all three columns of one row
 * back to back with the index register live and never forming a shared row base.
 * No flag/data object split exists — and the consumer's arithmetic refutes one
 * (col a packs x*2 | flag: bit 0 is the terminator flag, the rest is the X
 * coordinate). Whole-function 2D-array model KILLED (score 24 / 30 insns): the
 * target bumps the count MID-loop and the byte offset in the loop-end delay slot,
 * i.e. two independent bivs, so the loop source carries an explicit byte offset —
 * this form. TWO-SHAPE THEOREM established: MIPS legitimize_address accepts
 * (symbol_ref + reg) as an address (LO_SUM, symbol never entering a register) but
 * not (symbol_ref + reg + const), which it folds into the symbol and force_regs;
 * so target's mix of LO_SUM (col a) and shared base+disp (cols b,c) on ONE row
 * address requires that address to be written in TWO tree shapes. The search for
 * a uniform legitimate spelling is closed by derivation, not exhaustion. Finally,
 * s4's "no SOTN precedent" gate assertion is measured FALSE: 34 SOTN-master PSX
 * instances spell the same lvalue both via a local pointer alias and directly in
 * one function (hand-verified: src/st/cen/e_chamber.c EntityPlatform, alias at
 * :72, tilemap->height at :201, g_Tilemap.height at :240). The contested
 * alias+direct epilogue measures score 0 / 38 insns / 0 rules on this chassis but
 * is NOT proposed here — it sits in the rejected bank and its disposition is a
 * ruling question (see hypotheses.md s6 frontier item 1). THIS form remains the
 * best UNCONTESTED body: clean pure C, floor 4.
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
    p[0] = 0;
}
