/* [s29 2026-09-05 - structural modality.  MATCH.  sandbox func_80060A68 --disable all = score 0,
 * build_insns 66, target_insns 66, and `verify-oracle` reports build_sha1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked original.  Zero FAKE constructs, zero
 * named intermediates, zero staged locals, zero volatile, zero inline asm.  The body below is
 * plain idiomatic C.
 *
 * THE MECHANISM, read out of the compiler source rather than guessed (tools/gcc-2.7.2/sched.c:826-841,
 * `true_dependence`).  For 28 sessions the whole residual was governed by one dependence: the
 * `sw $v0, %gp_rel(D_800A3478)($gp)` store and the `lw ?, 0x10($v1)` loads.  memrefs_conflict_p
 * cannot disambiguate a symbol address from a base-register address, so the scheduler created that
 * dependence unconditionally and any 0x10 read written below the gp store was stranded at slot 26
 * (the Pgp / P1A family, 3/66) while any read written above it could not be made to produce three
 * distinct loads (the Pt2 / P18 family, 2/66, because cse folded two of the three).  s27 and s28
 * between them proved there is no second cse separator available in that window - every store in
 * the function was measured as the candidate separator and every one regressed.
 *
 * `true_dependence` does not stop at memrefs_conflict_p.  It ANDs in two struct-aliasing escapes:
 *
 *     && ! (MEM_IN_STRUCT_P (x) && rtx_addr_varies_p (x)
 *           && GET_MODE (x) != QImode
 *           && ! MEM_IN_STRUCT_P (mem) && ! rtx_addr_varies_p (mem))
 *
 * The gp store is a scalar global at a SYMBOL_REF address: MEM_IN_STRUCT_P == 0 and
 * rtx_addr_varies_p == 0, so it satisfies the whole right-hand side.  All that is missing is for
 * the 0x10 read to be a STRUCT reference with a varying address.  Expressed as a cast through an
 * integer - `*(s32 *)(outer + 0x10)` - the read is a bare MEM: MEM_IN_STRUCT_P == 0, the escape
 * does not fire, and the dependence stands.  Expressed as a member of a struct object -
 * `ob->p10` - expand_expr sets MEM_IN_STRUCT_P on the COMPONENT_REF, the address is
 * `(plus (reg) (const_int 16))` so rtx_addr_varies_p == 1, the mode is SImode, and
 * true_dependence returns 0.  The dependence that shaped 28 sessions of geometry simply
 * disappears, and the three 0x10 reads can then sit in their natural source positions on both
 * sides of the gp store.
 *
 * The same escape does NOT change cse's behaviour, which is why the fold problem also evaporates
 * rather than moving: cse's invalidate_memory (tools/gcc-2.7.2/cse.c:1703-1719) removes a memory
 * entry when `all || (nonscalar && p->in_struct) || cse_rtx_addr_varies_p (p->exp)`, and the last
 * disjunct is already true for every one of these base-register reads.  So each of the three
 * halfword stores separates the read that follows it exactly as it always did, all three reads
 * survive into sched, and the geometry is target's.
 *
 * MEASUREMENTS THIS SESSION (all on today's HEAD chassis, all zero-FAKE).  Cast-based control
 * (previous candidate.c, the Pt2 body) 2/66.  Struct-typed, same Pt2 statement order (SPt2) 2/66
 * and SP18 2/66 - struct typing alone does not close it, the statement order has to move back to
 * the natural one now that the dependence is gone.  Struct-typed with the third read below the gp
 * store: SPgp 0/66 and SP1A 0/66 - the first zeroes on this function.  Struct-typed with the
 * named intermediates removed entirely: SN3 0/66, SN4 0/66, SN5 0/66, SN9 0/66.  Struct-typed with
 * the whole scratch set removed and the members typed as pointers: SN10 0/66, SN11 0/66, and SN12
 * (this body) 0/66.  Negative controls that confirm the statement order still matters under struct
 * typing: SNAT (both halfword stores above the gp store) 8/66, SN2 (gp store above both) 8/66,
 * SN6 7/66, SN7 13/67, SN8 7/66, SM1 6/65.
 *
 * WHY THIS SHAPE IS THE HONEST ONE.  The DATA MODEL for this function records that none of the
 * members of the object behind D_800A3468 has a declaration in include/*.h, so its declared shape
 * was genuinely undetermined; a struct is the ordinary way to write a game object reached through
 * a global pointer, and the resulting C reads as what the function does - copy three words and
 * three halfwords out of two sub-objects, publish two interior addresses to globals, dispatch
 * through a per-character table, store the result, and set a flag.  No local in this body exists
 * for codegen reasons: `ob` is the object, `result` is the call's return value, and there is
 * nothing else.  Notably this body also declares NO local named temp2, temp_a1, p10, src or cp -
 * every one of the five banned constructs for this function is about such a local, and none of
 * them is reachable from a body that has no scratch locals at all. */
void func_80060A68(void) {
    struct Ob {
        u16 idx;
        u16 u02;
        s32 u04;
        s32 u08;
        s32 *p0C;
        u16 *p10;
        s32 p14;
        u16 m18;
        u16 m1A;
        u16 m1C;
        u16 u1E;
        s32 m20;
        s32 m24;
        s32 m28;
    };
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    struct Ob *ob;
    s32 result;

    ob = (struct Ob *)D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + ob->idx * 4) = 0;
    ob->m20 = ob->p0C[0];
    ob->m24 = ob->p0C[1];
    ob->m28 = ob->p0C[2];
    ob->m18 = ob->p10[0];
    ob->m1A = ob->p10[1];
    D_800A3478 = (s32)&ob->m18;
    ob->m1C = ob->p10[2];
    D_800A347C = (s32)&ob->m20;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + ob->idx) + *(s32 *)((s32)&D_800F10D0 + ob->idx * 4)) * 4))();
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
