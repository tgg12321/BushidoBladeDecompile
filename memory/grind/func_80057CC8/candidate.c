/* BEST BAN-COMPLIANT FORM (grind s32, 2026-08-27, rederive modality).
 * MEASURED THIS SESSION: `sandbox func_80057CC8 --disable all` -> score 24,
 * target_insns 111, build_insns 108, rules_dropped 0.  The inherited s31
 * candidate (next-neighbour ADDRESS hoisted above the first call, 110 insns)
 * re-measured at 26 on this chassis first, so the floor moved 26 -> 24.
 *
 * WHAT CHANGED vs s31: the next-neighbour BYTE OFFSET is selected in the two arms
 * of the next-index test (`off = (s16)tmp * 4;` / `off = 0;`) and the address is
 * formed once from the single `table` value afterwards.  The vertex-table base is
 * still read EXACTLY ONCE (`table = *(s16 **)(arg0 + 4);`); `off` and `next_vert`
 * are ordinary distinct values, not second materialisations of the base.  Two
 * measured consequences: (a) the arm re-uses the sign-extension the range compare
 * already needed, so the offset costs ONE instruction instead of the sll16/sra14
 * pair, and (b) GCC coalesces `next_vert` onto `table`'s hard register (the emitted
 * `addu $17,$17,$6`), which puts the sign-extended centre-Y twin on the target's $16.
 *
 * REGISTER-ASSIGNMENT ACCOUNTING (the residual, quantified this session).  The
 * eight values that cross the calls are assigned $16..$23 in descending
 * allocno_compare priority, which GCC 2.7.2 computes at global.c:635 as
 *   floor_log2(n_refs) * n_refs / live_length * 10000 * size.
 * Target ranks: cys, cxs, arg0, next_idx  ->  $16,$17,$18,$19.
 * This form ranks: cys, table/next_vert, cxs, arg0.  The base pseudo is rank 2
 * because it carries FOUR references (def + centre address + prev address + next
 * address); at four references floor_log2 jumps to 2 and its priority (~0.24)
 * beats both twins and arg0.  Dropping it to THREE references would put it at
 * ~0.09 -- below arg0 -- and reproduce the target's $16..$19 assignment exactly.
 * There is no ban-compliant way to do that: the three uses are the three distinct
 * vertex addresses the function must form, and removing one means either a second
 * materialisation of the base (the banned family) or deriving one neighbour
 * address from another, which costs the instructions it saves.  See evidence.md
 * s32-E4.
 *
 * WHY IT IS STILL NOT 0: unchanged from s30b/s31 -- asm/funcs/func_80057CC8.s
 * loads 0x4($s2) TWICE (:17 `lw $a2`, :50 `lw $a0`) and GCC 2.7.2 has no pass that
 * turns one RTL load into two loads at the original address.  That residual is the
 * policy question refused 2026-07-20 and standing-ruled 2026-07-27, not a spelling.
 *
 * SPELLING-INVARIANCE MEASURED THIS SESSION (all in the 108-insn regime): declaring
 * the centre coordinates `s16` and reading them through `*(s16 *)` scores the same
 * 24; using the sign-extended twins instead of the raw coordinates in the two final
 * adds emits BYTE-IDENTICAL text; naming the prev-neighbour address in its own local
 * changes only one addu's PLUS operand order and also scores 24.
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
        next_vert = (s16 *)((s32)table + off);
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
