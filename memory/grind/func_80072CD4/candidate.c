/* func_80072CD4 - POLY_G4 vertex-colour body. `sandbox func_80072CD4 --disable all` = 0,
 * build_insns 79 == target_insns 79, rules_dropped 0.
 *
 * WHAT THE FUNCTION IS. arg1 points at a libgpu POLY_G4 primitive: the touched offsets are
 * exactly its four RGB triples - rgb0 {0x04,0x05,0x06}, rgb1 {0x0C,0x0D,0x0E},
 * rgb2 {0x14,0x15,0x16}, rgb3 {0x1C,0x1D,0x1E} - the function calls SetPolyG4/SetSemiTrans on
 * arg1 and AddPrim's it, and it returns arg1 + 0x24 == sizeof(POLY_G4) (the advance to the next
 * primitive in the packet buffer). It is a per-vertex gouraud colour setter: the arg0 < 4 case
 * picks one of two colour sets for vertices 0 and 1 from a game-state flag and then paints
 * vertices 2 and 3 unconditionally; the arg0 >= 4 case paints all four.
 *
 * The struct typedef and the per-triple ascending-field writing style are taken verbatim from the
 * COMPLETED-C func_8003553C in src/code6cac_b2_pre.c:153-178, which is the same POLY_G4 shape
 * (zero rules, absent from engine/queue.json, not in inline_asm_canonical.txt) and likewise
 * repeats the same channel constant across vertices instead of hoisting it to a temporary.
 *
 * WHY EACH ARM CARRIES ITS OWN COMPLETE rgb0+rgb1 PAIR (and why r0 == r1 == 0xFC is not a
 * hoistable duplicate): each branch selects a complete colour for vertices 0 and 1; the two
 * colours happen to share a red component. The COMPLETED-C sibling func_80072BC4 in this same
 * file writes the identical un-hoisted repeated component `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;`
 * in BOTH arms of its inner if (src/text1b.c:5840 and :5843), and target's own bytes for THIS
 * function keep an unmerged cross-arm repeat: arm A at 0x80072D28-2C and arm B at
 * 0x80072D48-4C each emit `addiu $v0,0xC3 / sb $v0,0x5` (asm/funcs/func_80072CD4.s:23-24,32-33).
 * 12 C stores -> 12 emitted stores; nothing here is dead, byte-neutral, or a holder.
 *
 * CODEGEN CONSEQUENCE (recorded, not relied on as the reason for the spelling): with the colour
 * assignments complete inside the arms, jump2 cross-jumps the arms' common tail
 * `sb v1,4 / sb v1,0xC / sb v0,0xE` to the join label, which is target's merge head. The
 * blob-model bodies of s1-s5 (which lifted the two red components out of the colour assignments
 * into a shared merge-block tail behind an `int fc_const`) are bounded away from target by the
 * L1/L2 laws recorded in hypotheses.md; the clean floor-4 blob body is preserved unchanged as
 * fallback_floor4.c.
 *
 * Contains no local of any kind, no volatile, no asm, no barrier, no dead store, no do-while,
 * no annotation.
 */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x1E;
            ((POLY_G4 *)arg1)->r1 = 0xFC; ((POLY_G4 *)arg1)->g1 = 0xC8; ((POLY_G4 *)arg1)->b1 = 0x32;
        } else {
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x50;
            ((POLY_G4 *)arg1)->r1 = 0xFC; ((POLY_G4 *)arg1)->g1 = 0xDC; ((POLY_G4 *)arg1)->b1 = 0x46;
        }
        ((POLY_G4 *)arg1)->r2 = 0xFC; ((POLY_G4 *)arg1)->g2 = 0x82; ((POLY_G4 *)arg1)->b2 = 0;
        ((POLY_G4 *)arg1)->r3 = 0x32; ((POLY_G4 *)arg1)->g3 = 0x28; ((POLY_G4 *)arg1)->b3 = 0xA;
    } else {
        ((POLY_G4 *)arg1)->r0 = 0x10; ((POLY_G4 *)arg1)->g0 = 0x30; ((POLY_G4 *)arg1)->b0 = 0x60;
        ((POLY_G4 *)arg1)->r1 = 0x18; ((POLY_G4 *)arg1)->g1 = 0; ((POLY_G4 *)arg1)->b1 = 0x40;
        ((POLY_G4 *)arg1)->r2 = 0x30; ((POLY_G4 *)arg1)->g2 = 0; ((POLY_G4 *)arg1)->b2 = 0x60;
        ((POLY_G4 *)arg1)->r3 = 0; ((POLY_G4 *)arg1)->g3 = 0; ((POLY_G4 *)arg1)->b3 = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
