/* REJECTED (s1) — shift/fence placement variants for the sll cluster:
 *   A2: `ofs = sa1 << 4;` BEFORE D_801027F1 store          -> 7
 *   A3: `ofs = sa1 << 4;` AFTER D_801027F1, before fence   -> 7
 *       (named single-set temp perturbs RA: param copies move to $6/$7,
 *        v2's lw displaced below the first sb)
 *   A4: A1 order with NO do{}while(0) fence                -> 19 (tail scrambles)
 *   A5: entry load moved BEFORE the fence                  -> 20
 *   A6: fence BEFORE both sb's (one shared region)         -> 5
 *       (scheduler emits sb1, sll, addu, sb2 — target wants sll, sb1, sb2, addu)
 *   A11 (analysis, not built): `entry = sa1 << 4; entry = *(...)` reuse —
 *       dead: entry's pseudo ends in $3 (lw/sb), shift dest must be $2.
 *   Baseline in-place `sa1 = sa1 << 4;` — correct PLACEMENT (scheduler
 *       hoists it above sb1) but dest = sa1's pseudo $3; target's slt proves
 *       sa1 stays $3, so the shifted value is a FRESH pseudo in $2. In-place
 *       can never match; fresh subexpr (A1, banked candidate) has right regs
 *       but sits after the fence so it can't hoist above sb1.
 */
s32 AddTbpOfst(u16 a0, s16 a1);
