/* s45 O2 (object-model): FULL typing - 52-byte ObjCtx (s16 nlink at +6, ObjRec *recs at +0xC,
 * ObjLink *links at +0x10, per the matched sibling func_80017D84 in src/ings.c) + 0x40-byte ObjRec +
 * 0x10-byte ObjLink over the BASE preheader/exit-tail shape, tail written through typed pointers.
 * 37 at 127/126 on the HEAD chassis (BASE = 3; s30 cell B re-audited = 7).  Dumps (s45/iO2): every
 * ctx->recs load survives every RTL pass exactly as in s30 B (insns 29/64/167/229/283/308 through
 * .rtl .. .lreg); the difference is global.c SEATS - the guard load (insn 29) and the preheader load
 * (insn 64) both land in v1, so reorg.c redundant_insn deletes the preheader load (present in .sched2,
 * ABSENT in .dbr) and copies sll v0,s4,6 into the second guard's delay slot.  Typed indexing also
 * cse-merges the loop base into the guard's address sum (addu a0,a1,zero) and the typed tail flips the
 * record add operand order (addu v0,v0,s0).  Instance kill, no FAKE constructs.
 */
/* O2 (s45 object-model): FULL typing - ObjCtx (as O1) + 0x40-byte record +
 * 0x10-byte link - over the BASE preheader/exit-tail shape (s30 cell B's
 * record/link typedefs, now with the ctx typed too and the tail natural). */
typedef struct {
    s32 pos[3];
    s32 unkC[3];
    s32 f18;
    s32 na;
    s32 nb;
    u8 la[8];
    u8 lb[8];
    s32 unk34[3];
} ObjRec;

typedef struct {
    s32 dist;
    u16 b;
    s16 a;
    s32 dist3;
    s32 owner;
} ObjLink;

typedef struct {
    s32 handle;
    u16 nrec;
    s16 nlink;
    s32 unk8;
    ObjRec *recs;
    ObjLink *links;
} ObjCtx;

s32 func_80017848(ObjCtx *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    ObjRec *slots;
    ObjRec *p;
    ObjRec *q;
    ObjRec *rec;
    ObjRec *rec_a;
    ObjRec *rec_b;
    ObjLink *lnk;
    ObjLink *link;
    s32 dist;
    s32 t;
    s32 i;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = ctx->recs;
    if (slots[slot_a].f18 >= 0) {
        if (slots[slot_b].f18 >= 0) {
            return 0;
        }
    }

    p = ctx->recs;
    i = 0;
    t = p[slot_a].na;
    if (i < t) {
        q = ctx->recs;
        lnk = ctx->links;
        rec = &q[slot_a];
        do {
            if (lnk[rec->la[i]].b == slot_b) {
                return 0;
            }
            i++;
        } while (i < rec->na);
        p = q;
    }

    i = 0;
    if (i < p[slot_a].nb) {
        rec = &ctx->recs[slot_a];
        do {
            if (ctx->links[rec->lb[i]].a == slot_b) {
                return 0;
            }
            i++;
        } while (i < rec->nb);
    }

    dist = math_Distance3D(ctx->recs[slot_a].pos, ctx->recs[slot_b].pos);
    link = &ctx->links[ctx->nlink];
    link->dist = dist;
    link->dist3 = dist * 3;
    link->owner = arg1;
    *(s32 *)&link->b = (slot_a << 16) | slot_b;

    rec_a = &ctx->recs[slot_a];
    i = rec_a->na;
    rec_a->na = i + 1;
    rec_a->la[i] = ctx->nlink;

    rec_b = &ctx->recs[slot_b];
    i = rec_b->nb;
    rec_b->nb = i + 1;
    rec_b->lb[i] = ctx->nlink;

    ctx->nlink++;
    return 1;
}
