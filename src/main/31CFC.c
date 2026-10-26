/* node_RebaseParents alone. .text 0x800414FC (ROM 0x31CFC). Start boundary:
 * LEGACY (a tooling split inside the -G8 run, no evidence either way). Its
 * untouched 8-byte frame is cc1's stack slot for the folded `i != -1` loop
 * guard (phantom-slot-frame-lever). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "game.h"

/* Adds delta to the parent pointer (node.parent_node) of each of the n nodes
 * that is not null: slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers
 * moved the model object, and the three node arrays with it, by delta bytes. */
void node_RebaseParents(s32 delta, Unk80045878Node *rec, s32 n) {
    s32 i;

    for (i = n - 1; i != -1; i--) {
        if (rec->node.parent_node != 0) {
            rec->node.parent_node =
                (Unk80101DF0Record *)((u8 *)rec->node.parent_node + delta);
        }
        rec++;
    }
}
