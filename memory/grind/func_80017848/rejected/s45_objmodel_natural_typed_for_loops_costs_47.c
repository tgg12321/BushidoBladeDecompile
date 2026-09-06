/* s45 O4 (object-model): the fully NATURAL typed form (one record pointer per loop, for-loops reading
 * rec->na / rec->nb, combined && top guard, tail through typed pointers).  47 at 127/118 on the HEAD
 * chassis: 9 instructions SHORT - the record pointer and links pointer are cse-shared across guards,
 * loops and tail, and none of target's reloads survive.  Proves the shipped code does NOT hold one
 * typed record pointer across the function; the matched sibling func_80017D84 (src/ings.c) uses the
 * same raw u8 * + literal-offset idiom on this object.  Instance kill, no FAKE.
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
/* O4 (s45 object-model): the fully NATURAL typed form a human would write
 * against this object model - one record pointer per loop, for-loops reading
 * the count through the record pointer, tail through the same pointers. */
s32 func_80017848(ObjCtx *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    ObjRec *rec;
    ObjRec *rec_b;
    ObjLink *lnk;
    ObjLink *link;
    s32 dist;
    s32 i;

    if (slot_a == slot_b) {
        return 0;
    }

    if (ctx->recs[slot_a].f18 >= 0 && ctx->recs[slot_b].f18 >= 0) {
        return 0;
    }

    rec = &ctx->recs[slot_a];
    lnk = ctx->links;
    for (i = 0; i < rec->na; i++) {
        if (lnk[rec->la[i]].b == slot_b) {
            return 0;
        }
    }

    rec = &ctx->recs[slot_a];
    lnk = ctx->links;
    for (i = 0; i < rec->nb; i++) {
        if (lnk[rec->lb[i]].a == slot_b) {
            return 0;
        }
    }

    dist = math_Distance3D(ctx->recs[slot_a].pos, ctx->recs[slot_b].pos);
    link = &ctx->links[ctx->nlink];
    link->dist = dist;
    link->dist3 = dist * 3;
    link->owner = arg1;
    *(s32 *)&link->b = (slot_a << 16) | slot_b;

    rec = &ctx->recs[slot_a];
    i = rec->na;
    rec->na = i + 1;
    rec->la[i] = ctx->nlink;

    rec_b = &ctx->recs[slot_b];
    i = rec_b->nb;
    rec_b->nb = i + 1;
    rec_b->lb[i] = ctx->nlink;

    ctx->nlink++;
    return 1;
}
