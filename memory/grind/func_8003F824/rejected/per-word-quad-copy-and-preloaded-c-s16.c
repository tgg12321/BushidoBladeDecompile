/* REJECTED (s1 2026-09-08): score 70. quads copied as 4 scalar stores (address recomputed per store, 4 extra lh/sll/addu triples) and `s16 c = *(u16*)cmds` preloaded before the loop + reloaded at body end: cse merges c load with the -3 compare load (single lhu + sll/sra); walk loop `while(*cmds!=-2)cmds++;cmds++` leaves addiu -2/+2 pair. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 quad[4];
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
    /* 0x418 */ s32 quads[5][4];
    /* 0x468 */ u8 data[1];
} Scene;

extern u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur);
extern s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2);

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
    c = *(u16 *)cmds;
    for (i = 0; *cmds != -3; i++) {
        if (c != -2) {
            if (sc->count >= 5) {
                func_80052C10();
            }
            rec = &sc->recs[sc->count];
            obj = ((u8 **)(arg0 + 0x1A34))[i];
            rec->cur = cur;
            rec->obj = obj;
            cur = func_8003FA24(rec, cmds, cur);
            sc->quads[sc->count][0] = rec->quad[0];
            sc->quads[sc->count][1] = rec->quad[1];
            sc->quads[sc->count][2] = rec->quad[2];
            sc->quads[sc->count][3] = rec->quad[3];
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
        while (*cmds != -2) {
            cmds++;
        }
        cmds++;
        c = *(u16 *)cmds;
    }
    if (arg1) {
        func_80045A28(*(s16 *)(arg0 + 4), cur - *(u8 **)(arg0 + 0x1C));
    }
}
