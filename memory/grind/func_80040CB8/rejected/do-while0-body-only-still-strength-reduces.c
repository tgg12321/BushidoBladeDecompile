// REJECTED (grind s2, probe p3) — the loop notes alone are not enough.
//
// HYPOTHESIS: wrapping the goto-loop body in do { ... } while (0) supplies the
// NOTE_INSN_LOOP_BEG/END that flow.c needs for loop_depth-2 ref weighting
// (which is the mechanism that fixes the register assignment — s2 H6), while
// keeping the goto back-edge that s1 proved is required.
//
// MEASURED: 38 insns, score 25 — IDENTICAL failure mode to a real for-loop.
// loop.c accepted the do-while(0) region as a loop (its scan_start IS the
// `loop:` CODE_LABEL, so it is not phony), recognised `ent` as a biv, created
// DEST_ADDR givs for the -0x57..-0x4C displacement cluster, combined them and
// strength-reduced them into a third induction pointer based at arg0+0x8B8 —
// leaving the 0x90C cursor to serve only `sw $zero,0(...)`. Three
// `addiu rX,rX,0x68` per iteration against target's two.
//
// THE FIX (see candidate.c): move the `ent` initialiser INSIDE the wrapper,
// ahead of the `loop:` label. Then scan_loop's scan_start is an ordinary insn
// rather than a CODE_LABEL, loop.c reports "Loop from 31 to 112 is phony" and
// returns before biv analysis — notes kept, strength reduction gone.
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    s32 none = -1;
    s32 kind = 3;
    s32 one = 1;
    s32 link = (s32)arg0 + 0x94;
    s16 *tbl = D_80094B9E;
    s32 ent = (s32)arg0 + 0x90C;

    do {
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
    } while (0);
    *(s16 *)((s32)slot + 2) = -1;
}
