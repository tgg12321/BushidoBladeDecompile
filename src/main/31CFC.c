/* save_vc_ctrl alone. .text 0x800414FC (ROM 0x31CFC). Start boundary: LEGACY (a
 * tooling split inside the -G8 run, no evidence either way). One function per
 * TU, so the -G8 TARGET_FILE_SWITCHING float cannot reorder anything here.
 *
 * save_vc_ctrl is plain C: its 8-byte frame that no instruction touches is
 * cc1's stack slot for the folded `i != -1` loop guard
 * (.claude/rules/phantom-slot-frame-lever.md, producer 1), as in
 * gpu_SetDrawMoveArray (src/main/2B344.c).
 */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "game.h"

/* Adds delta to the parent pointer (node.unkC) of each of the n nodes that is
 * not null: func_80041430 moved the model object, and the three node arrays
 * with it, by delta bytes. */
void save_vc_ctrl(s32 delta, Unk80045878Node *rec, s32 n) {
    s32 i;

    for (i = n - 1; i != -1; i--) {
        if (rec->node.unkC != 0) {
            rec->node.unkC =
                (Unk80101DF0Record *)((u8 *)rec->node.unkC + delta);
        }
        rec++;
    }
}
