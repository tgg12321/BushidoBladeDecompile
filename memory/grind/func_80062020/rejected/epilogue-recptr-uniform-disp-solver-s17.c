/* REJECTED (s17, solver modality, 2026-09-03) — the defeat-combine-symbol-fold
 * family: a NAMED row pointer for the terminator row, uniform spelling, no chain.
 *
 * WHY IT IS HERE.  tools/ra_solver/inverse_compose.py classify typed this function's
 * residual PRE-RA and ranked "defeat-combine-symbol-fold — pre-compute a displaced
 * pointer so combine cannot fold the displacement into the addressing mode" as the
 * top ordinary-C vector.  This is that vector, spelled on the merged-record
 * declaration.  It is ADMISSIBLE (no chained assignment, no dual spelling, no FAKE
 * construct, no banned construct) and it is the best form in its family.
 *
 * WHY IT IS DEAD.  The vector works mechanically — the named pointer really does
 * defeat combine's symbol fold and put the stores on a base register — but it does
 * so for ALL THREE stores at once, because a pointer pseudo leaves GCC no symbol to
 * spell a LO_SUM against:
 *     this form : DISP8 | DISP4 | DISP0        (sw zero,8(v0) / 4(v0) / 0(v0))
 *     target    : DISP8 | DISP4 | LOSUM0       (asm/funcs/func_80062020.s:0x8006209C-AC)
 * Real-chassis measurement of the equivalent split-symbol spelling (S11 ==
 * rejected/epilogue-uniform-pointer-floor4-superseded.c) on 2026-09-03:
 *     sandbox func_80062020 --disable all -> score 6, target_insns 38, build_insns 35
 * The whole 3-instruction residual is the missing lui/addu/%lo re-materialisation
 * for the +0 column.
 *
 * Six sibling spellings measured identically (record pointer both orders, 2-D row
 * pointer, pointer-to-array, chained-through-pointer, split-symbol byte offset):
 * tmp/grind/func_80062020/s17/sweep_solver_results.txt rows S01-S06, S11.
 * Chain-ness alone does not rescue it — S03/S05 are chains through the pointer and
 * still emit DISP0, because expr.c:3457 only leaves the LEFTMOST link's address
 * unstabilised, and through a pointer there is no symbol left to fold into.
 *
 * Shown here as the standalone harness TU that was compiled (tmp/grind/func_80062020/
 * s17/S01_recptr_stmts_840.c); in src/text1b.c the typedefs come from include/game.h.
 */
typedef int s32;
typedef unsigned char u8;
extern s32 D_800A32B8;
typedef struct { s32 unk0; s32 unk4; s32 unk8; } Rec;
extern Rec D_800F1198[];

void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    Rec *r;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        D_800F1198[i].unk0 = *(s32 *)((u8 *)arg0 + ofs + 0);
        D_800F1198[i].unk4 = *(s32 *)((u8 *)arg0 + ofs + 4);
        D_800F1198[i].unk8 = *(s32 *)((u8 *)arg0 + ofs + 8);
        i = i + 1;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    r = &D_800F1198[i];
    r->unk8 = 0;
    r->unk4 = 0;
    r->unk0 = 0;
}
