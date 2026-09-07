/* REJECTED -- s21 (2026-09-07, rederive).  score 21, build_insns 97, outer insn_count 99.
 * Fully ordinary C: no `lim` local at all, case 3's two 0xC8 resets written as
 * `for (k = 8; k <= 0xC; k += 4) *(u16 *)(p + k) = 0xC8;` and case 1's reset written with the
 * literal.  The compiler temp holding 0xC8 IS a movable of the inner loop and is moved
 * ("Insn 127: regno 104 ... moved to 298"), so moved_once[104] is set.  In the outer loop the
 * hoisted copy is recognised as a DUPLICATE of the case-1 literal's movable -- "Insn 298:
 * regno 104 (life 11), done move-insn matches 76" -- and a matched/duplicate movable takes
 * move_movables' m->done path, which is short-circuited BEFORE the moved_once test at
 * loop.c:1609.  No "halved since already moved" is printed and the four dispatch constants
 * are all moved.  Arming requires the armed pseudo to reach the desirability block as a
 * movable in its own right; a duplicate never does. */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 k;

    base = D_800A36A0;
    i = 0;
    do {
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                for (k = 8; k <= 0xC; k += 4) {
                    *(u16 *)(p + k) = 0xC8;
                }
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
