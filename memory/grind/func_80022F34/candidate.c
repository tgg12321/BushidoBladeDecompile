/*
 * func_80022F34 -- BANKED MATCHING FORM (s12, 2026-09-06).
 *
 * MEASURED THIS SESSION on clean HEAD with the owner's maspsx row present
 * (maspsx_label_nop_funcs.txt:22, commit d4338774):
 *   sandbox func_80022F34 --disable all -> score 0 (70/70 insns, rules_dropped 0)
 *   sandbox func_8001C444 --disable all -> score 0 (the D_80102778 decl change
 *                                          is byte-neutral for its other user)
 *   verify-oracle -> build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *
 * Declarations this form requires (include/code6cac.h, in scope):
 *   extern u8  D_80102782[];      (was: extern u8  D_80102782;)
 *   extern s32 D_801027BC[][5];   (was: extern s32 D_801027BC;)
 *   extern u16 D_80102778[2];     (was: extern s16 D_80102778;)
 * All three are DECLARATION-level corrections, matching how the target
 * addresses each symbol; no per-use cast pun remains for any of them.
 *
 * D_80101EC8 is addressed as `(u8 *)&D_80101EC8 + offset` deliberately, NOT
 * as a per-use pun: (i) that is the repo-wide idiom for this symbol, already
 * shipping in matched functions at code6cac.c:846/1052/1570/2103/2220,
 * code6cac_b.c:666/868/880, text1b.c:1520/1531; (ii) the target itself
 * re-materializes the symbol base inside the loop and adds a byte offset
 * register (asm/funcs/func_80022F34.s:12-14: lui/addiu %hi/%lo(D_80101EC8)
 * then `addu $a0, $s1, $v0`, $s1 stepping by 0x44C); (iii) correcting the
 * declaration to `u8 D_80101EC8[][0x44C]` is a project-wide refactor that
 * would have to edit src/code6cac_b.c, src/code6cac_c_ab.c and src/text1b.c
 * simultaneously -- all outside this function's scope grant -- and would
 * perturb functions that are already COMPLETED-C. It is an integration
 * handoff, not a per-function decision.
 *
 * D_8010277A remains declared as a separate s16 because its last remaining
 * user, src/code6cac_b.c:3048, is outside the scope grant; inside
 * src/code6cac.c the merged two-element object model is used everywhere.
 */
void func_80022F34(void) {
    s32 i;
    u16 *tbl;
    s32 offset;

    i = 0;
    tbl = D_80102778;
    offset = 0;

loop_22F34:
    {
        u8 *rec = (u8 *)&D_80101EC8 + offset;

        if (*(s16 *)(rec + 6) != 0) {
            s32 mode = D_800A38DC;

            switch (mode) {
                case 0:
                    *(s16 *)(rec + 8) = D_80102782[i] << 4;
                    break;
                case 1:
                case 2:
                default:
                    *(s16 *)(rec + 8) = *tbl;
                    break;
                case 3:
                    break;
            }

            {
                s16 idx1 = *(s16 *)(rec + 0x4A);
                s32 val1 = D_801027BC[idx1][0];
                rec = *(u8 **)rec;
                {
                    s16 idx2 = *(s16 *)(rec + 0x4A);
                    func_80055138(i, val1, D_801027BC[idx2][0]);
                }
            }
        }

        tbl++;
        i++;
        offset += 0x44C;
    }
    if (i < 2) goto loop_22F34;
}

