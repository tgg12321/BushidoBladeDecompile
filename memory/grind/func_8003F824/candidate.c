/* func_8003F824 — candidate (s1, 2026-09-08). sandbox --disable all: distance 0 (128/128 insns).
 * Ordinary C, no FAKE constructs. Load-bearing shapes (each measured this session):
 *  - SceneQuad struct assignment `sc->quads[sc->count] = rec->quad;` -> one address compute +
 *    lw a0..a3 / sw block move (per-word copies re-derive the address 4x: +12 insns).
 *  - `s16 c = *cmds;` as the FIRST statement of the loop body (NOT preloaded before the loop /
 *    at the body end): cse.c cse_set_around_loop (cse.c:7909) then rewrites the top-of-loop load
 *    into a copy of the exit test's load temp (REG_LOOP_TEST_P), and combine's 3-insn PARALLEL
 *    split (combine.c:1989) re-materializes the compare as `lh` while keeping `lhu a0` for c.
 *    Preloading c in the same block as the `*cmds != -3` test lets cse merge the two loads
 *    (one lhu + sll/sra) and hoists the top's `sll` into the loop-test reg: +2 insns/regs.
 *  - `while (*cmds++ != -2) {}` (post-increment in the condition): `while (*cmds != -2) cmds++;
 *    cmds++;` leaves reorg's delay-slot peel compensation (`addiu s1,-2; addiu s1,2`).
 */
typedef struct {
    /* 0x00 */ s32 v[4];
} SceneQuad;

typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ SceneQuad quad;
    /* 0x14 */ u8 *obj;
    /* 0x18 */ u8 *cur;
    /* 0x1C */ Func8003F6D8Inner inner;
} SceneRec; /* size 0xD0 */

typedef struct {
    /* 0x000 */ s16 count;
    /* 0x002 */ s16 unk2;
    /* 0x004 */ s16 unk4;
    /* 0x006 */ s16 unk6;
    /* 0x008 */ SceneRec recs[5];
    /* 0x418 */ SceneQuad quads[5];
    /* 0x468 */ u8 data[1];
} Scene;

extern u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur);
void func_8003FECC(s32 *a0, s32 *a1, s16 *a2);

void func_8003F824(u8 *arg0, s32 arg1) {
    Scene *sc;
    s16 *cmds;
    u8 *cur;
    SceneRec *rec;
    u8 *obj;
    s32 i;
    s16 c;

    sc = *(Scene **)(arg0 + 0x24);
    if (sc == 0) return;
    cmds = *(s16 **)(arg0 + 0x28);
    cur = sc->data;
    if (*cmds == -3) {
        *(Scene **)(arg0 + 0x24) = 0;
        return;
    }
    sc->count = 0;
    sc->unk2 = 0;
    sc->unk4 = 0;
    sc->unk6 = 0;
    for (i = 0; *cmds != -3; i++) {
        c = *cmds;
        if (c != -2) {
            if (sc->count >= 5) {
                func_80052C10();
            }
            rec = &sc->recs[sc->count];
            obj = ((u8 **)(arg0 + 0x1A34))[i];
            rec->cur = cur;
            rec->obj = obj;
            cur = func_8003FA24(rec, cmds, cur);
            sc->quads[sc->count] = rec->quad;
            *obj = 0xD;
            sc->count++;
            if (*cmds >= 0) {
                cmds++;
                while (*cmds >= 0) {
                    cmds++;
                }
            }
            if (*cmds == -1) {
                cmds++;
                func_8003FECC((s32 *)arg0, (s32 *)rec, cmds);
            }
        }
        while (*cmds++ != -2) {
        }
    }
    if (arg1) {
        func_80045A28(*(s16 *)(arg0 + 4), cur - *(u8 **)(arg0 + 0x1C));
    }
}
