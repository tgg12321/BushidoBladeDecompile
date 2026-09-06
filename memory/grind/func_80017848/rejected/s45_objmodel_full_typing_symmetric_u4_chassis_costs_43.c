/* s45 O3 (object-model): full typing (ObjCtx + ObjRec + ObjLink) over the SYMMETRIC U4 chassis (no
 * exit tail, unconditional reload, two-step guard per loop, q = p in loop 2).  43 at 127/122 on the
 * HEAD chassis: typed indexing lets cse fold the per-loop record-pointer re-reads and the base sums
 * into the guard temps, so the build is 5 instructions SHORT of target.  Instance kill, no FAKE.
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
/* O3 (s45 object-model): full typing over the SYMMETRIC U4 chassis (no exit
 * tail, unconditional reload, two-step guard per loop, q = p in loop 2). */
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
    s32 t2;
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
    }

    p = ctx->recs;
    i = 0;
    t2 = p[slot_a].nb;
    if (i < t2) {
        q = p;
        rec = &q[slot_a];
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
