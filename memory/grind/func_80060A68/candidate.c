/* [s29 2026-09-05 - synthesis modality.  MATCH: `sandbox func_80060A68 --disable all` = score 0,
 * build_insns 66 / target_insns 66; `verify-oracle` = build_sha1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked, build_matches true.
 * Zero FAKE constructs, zero named intermediates, zero staged locals, zero volatile, zero inline
 * asm, and no local declared for codegen reasons at all - `result`, the dispatch call's return
 * value, is the function's only local.
 *
 * WHAT CHANGED after 28 sessions of cast-through-integer geometry: the global at 0x800A3468 is a
 * POINTER, not an integer that happens to hold an address, and the object it points at gets a
 * declared shape.  src/text1b.c itself is the evidence: :3452, :3469 and :3503 assign a callee's
 * returned pointer into it, :3361 assigns the scratchpad base, :3415 stores a pointer into the
 * field at +0x14, and :3479 writes the whole word at offset 0 as the single constant 0x210009 -
 * whose low halfword 9 is the index this function loads with `lhu` and whose bit 21 is the flag
 * this function tests at the tail.  One storage location written whole and read at two widths is
 * what the union at offset 0 declares.  The three tables are declared as the arrays the census
 * already documents them to be (24-entry flag table; per-index offset table; per-character
 * combo-id table), so every access in the body is a member reference or an array subscript and
 * nothing is spelled as pointer arithmetic through a cast.
 *
 * WHY THAT REACHES THE TARGET STREAM (observation, recorded for the next reader - not the reason
 * any construct is here):
 *  1. The three `lw ?,0x10($v1)` loads, and the two reloads of the object pointer after the call,
 *     are cse's doing rather than the source's.  Each store made through the pointer invalidates
 *     cse's memory table (tools/gcc-2.7.2/cse.c:1703-1719), so the read preceding each of the
 *     0x18 / 0x1A / 0x1C stores becomes its own load; the `jalr` and the byte store invalidate it
 *     again in the tail.  The source writes each of those statements exactly once.
 *  2. Member references set MEM_IN_STRUCT_P, which is what lets the offset-0 read and the two
 *     scalar stores at 0x800A3478 / 0x800A347C be disambiguated in `true_dependence`
 *     (tools/gcc-2.7.2/sched.c:826-841): that escape needs the read to be MEM_IN_STRUCT_P with a
 *     varying address and the store to be neither.  A bare-MEM spelling of the same read does not
 *     fire it, which is what stranded the read window on every previous chassis.  Measured this
 *     session on otherwise identical bodies: bare-MEM offset-0 read = 11/66
 *     (tmp/grind/func_80060A68/s29/B.c), member = 0/66.
 *
 * Also measured 0/66 with the tables spelled through the address of their first word rather than
 * as array declarations; the array declarations are kept because they put the object model at the
 * declaration instead of at each use site. */
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
