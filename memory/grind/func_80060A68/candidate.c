/* [s29 2026-09-05 - synthesis modality.  MATCH: `sandbox func_80060A68 --disable all` = score 0,
 * build_insns 66 / target_insns 66; `verify-oracle` = build_sha1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked, build_matches true.
 * Zero FAKE constructs, zero named intermediates, zero staged locals, zero volatile, zero inline
 * asm, and no local declared for codegen reasons at all - `result`, the dispatch call's return
 * value, is the function's only local.
 *
 * WHAT CHANGED after 28 sessions of cast-through-integer geometry: the global at 0x800A3468 is a
 * POINTER, not an integer that happens to hold an address, and the object it points at gets a
 * declared shape.  src/text1b.c's own COMMITTED, MATCHED C is the evidence, independently of any
 * codegen observation (line numbers below are against the INCLUDE_ASM tree, i.e. src/text1b.c as
 * committed at s29):
 *   - :3358 `extern s32 *D_800A3468;` -- the matched sibling func_80061064 ALREADY declares this
 *     exact global with a pointer type.  The pointer typing is not this session's invention; it is
 *     the file's existing, accepted declaration for the same symbol.
 *   - Sixteen sites assign a POINTER into it: `D_800A3468 = (s32)v1;` where v1 is a callee's
 *     returned pointer (:3406, :3423, :3457, :3472, :3492, :3506, :3521, :3562, :3599, :3628,
 *     :3659, :3694, :3709, :3722), plus :3315 `= 0x1F800000` (the scratchpad base) and :3740
 *     `= (s32)&D_800F116C`.  Nothing ever stores a non-address into it.
 *   - :3369 `*(s32 **)((s32)D_800A3468 + 0x14) = ...` and :3432 / :3530 / :3637 / :3670 / :3750 --
 *     the member at +0x14 always receives a pointer to a byte buffer; this function stores one byte
 *     through it (`sb`), which is what `s8 *p14` declares.
 *   - :3433, :3531, :3638, :3671, :3751 write the WHOLE 32-bit word at offset 0 as a single
 *     constant -- 0x210009, 0x210005, 0x210010, 0x210002, 0x210014.  In every one of the five the
 *     low halfword is the character index this function loads with `lhu`, and bit 21 (0x200000) is
 *     the flag this function tests at the tail.  :3371 writes the same word as a bare loop index.
 *     One storage location written whole at five sites and read at two widths here is what the
 *     union at offset 0 declares.
 * The three tables are declared as the arrays the naming census already documents them to be
 * (24-entry flag table; per-index offset table; per-character combo-id table), so every access in
 * the body is a member reference or an array subscript and nothing is spelled as pointer
 * arithmetic through a cast.
 *
 * ROBUSTNESS OF THE MODEL (s29, measured on today's HEAD chassis).  The object model, not a swept
 * spelling, determines the bytes: FOUR structurally distinct faithful spellings of this same model
 * all measure 0/66 -- this body; the tables spelled through the address of their first word
 * (alt-s29-score0-tables-through-address-of-first-word.c); offset 0 declared as two u16 members
 * with the flag test cast instead of a union
 * (alt-s29-score0-two-halfwords-plus-cast-flag-read.c); and a FILE-scope struct with every member
 * renamed and the unused words typed u32 (alt-s29-score0-file-scope-struct-renamed-members.c).
 * The spelling that regresses (11/66) is the one that CONTRADICTS :3358's committed pointer
 * declaration by reading offset 0 through an integer cast.
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
