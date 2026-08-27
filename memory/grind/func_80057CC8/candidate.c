/* BEST BAN-COMPLIANT FORM (grind s33, 2026-08-27, rederive modality).
 * MEASURED THIS SESSION: `sandbox func_80057CC8 --disable all` -> score 22,
 * target_insns 111, build_insns 108, rules_dropped 0.  The inherited s32
 * candidate re-measured at 24 on this chassis first, so the floor moved 24 -> 22.
 *
 * WHAT CHANGED vs s32: exactly one character-level edit -- the PLUS operand order
 * of the next-neighbour address, `(s16 *)(off + (s32)table)` instead of
 * `(s16 *)((s32)table + off)`.  Everything else is the s32 candidate verbatim.
 *
 * WHY THAT ONE FLIP IS WORTH TWO POINTS (mechanism, dump-verified).  In the s32
 * spelling RTL-expand emits the address as `plus(table_pseudo, off_pseudo)` with
 * the base first, which makes the allocator record a copy PREFERENCE from the base
 * pseudo to the address pseudo (`;; 87 preferences: 17` in the .greg dump under
 * tmp/grind/func_80057CC8/dumps/text1b.greg).  Because the address pseudo crosses
 * the first `ratan2` call it must live in a callee-save register, and the
 * preference drags the BASE pseudo into that same callee-save register too --
 * emitted `lw $17,0x4($19)`.  The target keeps the base in the CALLER-SAVE `$a2`
 * (`lw $a2,0x4($s2)`, asm/funcs/func_80057CC8.s:17), because in the target the
 * base dies before the call.  Writing the offset first breaks the preference: the
 * base is no longer the leading operand, it is left in `$6`, and our
 * `lw $6,0x4($19)` / `addu $2,$2,$6` now agree with the target's `lw $a2,0x4($s2)`
 * / `addu $v0,$v0,$a2` instruction-for-instruction.
 *
 * REGISTER-ASSIGNMENT ACCOUNTING (updated -- this CORRECTS s32-E4).  The .greg
 * dump prints the global allocation order directly:
 *   ;; 15 regs to allocate: 78 77 172 73 187 79 87 104 72 76 168 83 86 74 75
 * For the s32 form pseudo 87 is the vertex-table base, disposition `87 in 17`,
 * `88 in 17` -- i.e. the callee-save seat that s32 attributed to "the base pseudo
 * at four references" is actually held by the next-neighbour ADDRESS allocno, and
 * the base merely inherits it through the copy preference.  Under the flip the
 * base drops out of the callee-save competition entirely.  The surviving residual
 * is a single rank swap: we get $16 cys, $17 next-address, $18 cxs, $19 arg0,
 * where the target has $16 cys, $17 cxs, $18 arg0, $19 next-INDEX.  For the
 * next-address allocno to fall below arg0 under allocno_compare
 * (tools/gcc-2.7.2/global.c:635, floor_log2(n_refs)*n_refs/live_length) it needs
 * at most THREE references over a live range longer than ~30 insns; every
 * three-reference spelling measured this session (arm-selected index + sll16/sra14,
 * arm-selected index * 4, ternary offset) lands at 28/28/25 because the index form
 * costs instructions the offset form does not.  See evidence.md s33-E3.
 *
 * WHY IT IS STILL NOT 0: unchanged from s30b/s31/s32 -- asm/funcs/func_80057CC8.s
 * loads 0x4($s2) TWICE (:17 `lw $a2`, :50 `lw $a0`) and GCC 2.7.2 has no pass that
 * turns one RTL load into two loads at the original address.  That residual is the
 * policy question refused 2026-07-20 and standing-ruled 2026-07-27, not a spelling.
 *
 * MEASURED-INERT THIS SESSION (all still 22 at 108 insns): naming the prev
 * neighbour address in its own local; flipping the PLUS operand order of the centre
 * address as well; forming the next address after `pi` instead of inside the arms;
 * two different local-declaration orders.  The flip's value is specific to the one
 * add whose result crosses the call.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        s32 off = (s16) tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
        next_vert = (s16 *)(off + (s32)table);
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
