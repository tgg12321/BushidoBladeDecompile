/* [s29 2026-09-05 - synthesis modality.  MATCH: `sandbox func_80060A68 --disable all` = score 0,
 * build_insns 66 / target_insns 66, zero FAKE constructs, zero named intermediates, zero staged
 * locals, zero volatile, zero inline asm, and no local declared for codegen reasons at all -
 * `result` (the dispatch call's return value) is the function's only local.
 *
 * The object behind D_800A3468 is a POINTER GLOBAL, not an integer global holding an address:
 * src/text1b.c writes it as `D_800A3468 = (s32)v1;` (:3452, :3469, :3503) and other functions in
 * this file already dereference it as one (:3415 stores a pointer into `+ 0x14`, :3479 writes the
 * whole word at offset 0 as 0x210009 - low halfword 9 = the index this function reads with `lhu`,
 * bit 0x200000 = the flag this function tests at the tail).  Declaring it `struct Ob *` and
 * writing every access as a member reference is what makes the target's instruction stream fall
 * out, and it does so for two independent reasons that 28 sessions of cast-through-integer
 * geometry could not reach:
 *
 *  1. The three `lw ?,0x10($v1)` loads and the reloads of the global after the call are cse's
 *     doing, not the source's.  Because the object is reached through the global pointer, the
 *     pointer load is cse'd into $v1 for the straight-line prologue, and each store made through
 *     that pointer invalidates cse's memory table (tools/gcc-2.7.2/cse.c:1703-1719), so the p10
 *     read preceding each of the 0x18 / 0x1A / 0x1C stores becomes its own load - target's three.
 *     The `jalr` and the `sb` through `p14` invalidate it again, which is why the target reloads
 *     `%gp_rel(D_800A3468)` twice in the tail.  No statement of this function is written twice and
 *     nothing is staged: the duplication is entirely cse's.
 *  2. Member references set MEM_IN_STRUCT_P, which is what lets the index read at offset 0 and the
 *     `%gp_rel(D_800A3478)` / `%gp_rel(D_800A347C)` scalar stores be disambiguated in
 *     `true_dependence` (tools/gcc-2.7.2/sched.c:826-841): its struct escape requires the read to
 *     be MEM_IN_STRUCT_P with a varying address and the store to be neither.  A base-register read
 *     spelled `*(u16 *)((s32)p + 0)` is a bare MEM and the escape does not fire, which is what
 *     stranded the read window in every previous chassis.  Measured directly this session: the
 *     identical body with offset 0 spelled `*(u16 *)&D_800A3468->status` through a cast instead of
 *     as a union member scores 11/66 (tmp/grind/func_80060A68/s29/B.c); as a member, 0/66.
 *
 * The union at offset 0 is the field as the rest of the file uses it - one 32-bit word that is
 * written whole (`= 0x210009`) and read either whole (the 0x200000 test) or as its low halfword
 * (the character index).  A plain `u16 idx; u16 u02;` pair with the flag test spelled
 * `*(s32 *)D_800A3468` also measures 0/66 (tmp/grind/func_80060A68/s29/D.c); the union is kept
 * because it needs no cast anywhere in the body. */
void func_80060A68(void) {
    struct Ob {
        union { s32 w; u16 h; } id;
        s32 u04;
        s32 u08;
        s32 *p0C;
        u16 *p10;
        s8 *p14;
        u16 m18;
        u16 m1A;
        u16 m1C;
        u16 u1E;
        s32 m20;
        s32 m24;
        s32 m28;
    };
    extern struct Ob *D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60[];
    extern s32 D_800F10D0[];
    extern s32 chractar_use_pset_combo_id_table[];
    s32 result;

    D_800F10D0[D_800A3468->id.h] = 0;
    D_800A3468->m20 = D_800A3468->p0C[0];
    D_800A3468->m24 = D_800A3468->p0C[1];
    D_800A3468->m28 = D_800A3468->p0C[2];
    D_800A3468->m18 = D_800A3468->p10[0];
    D_800A3468->m1A = D_800A3468->p10[1];
    D_800A3478 = (s32)&D_800A3468->m18;
    D_800A3468->m1C = D_800A3468->p10[2];
    D_800A347C = (s32)&D_800A3468->m20;

    result = ((s32 (*)(void)) chractar_use_pset_combo_id_table[
                  D_8009BA60[D_800A3468->id.h]
                  + D_800F10D0[D_800A3468->id.h]])();
    *D_800A3468->p14 = result;

    if (D_800A3468->id.w & 0x200000) {
        D_800A32BC = 0xA;
    }
}
