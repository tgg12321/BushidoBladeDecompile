// REJECTED (grind s2, probe p1) — kills frontier F1's premise.
//
// F1 wanted a SEMANTIC sixth reference to the 0x8B4 cursor so its allocno
// priority (2*5/31 = 0.323) would clear tbl (0.348) while its initialiser
// stayed FIRST in the prologue. The most natural candidate is to express the
// 0x90C cursor as the 0x8B4 cursor plus the +0x58 field offset, which is what
// the two cursors actually are semantically.
//
// MEASURED: score 21, 36 insns (identical to the plain slot-first form) and the
// .lreg dump still reports "Register 73 used 5 times across 31 insns" — the
// reference is NOT gained. cse folds (arg0 + 0x8B4) + 0x58 back to
// arg0 + 0x90C before flow.c counts refs, and the emitted insn is
// `addiu $v1,$a0,2316`, not `addiu $v1,$a2,0x58`. Any cursor-relative
// respelling in the PROLOGUE is foldable for the same reason; only a reference
// inside the loop body (where the cursor's value is not a compile-time
// constant) could survive, and every in-loop use of it changes a displacement.
//
// SUPERSEDED ANYWAY: s2's H6 showed the whole F1 line was unnecessary — the
// cursor does not need a sixth unweighted ref, it needs loop-note ref weighting
// (loop_depth 2), which raises it to 8 refs / 0.774 and clears tbl outright.
// See candidate.c.
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    s32 none = -1;
    s32 kind = 3;
    s32 one = 1;
    s32 link = (s32)arg0 + 0x94;
    s16 *tbl = D_80094B9E;
    s32 ent = (s32)slot + 0x58;   /* folds to arg0 + 0x90C in cse */

loop:
    {
        s16 id = *tbl;
        if (id != none) {
            *(s16 *)(ent - 0x56) = id;
            *slot = kind;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = one;
            *(s16 *)(ent - 0x4E) = 0;
            {
                u16 w = *(u16 *)((s32)arg0 + 0x16);
                slot += 0x68;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    }
    *(s16 *)((s32)slot + 2) = -1;
}
