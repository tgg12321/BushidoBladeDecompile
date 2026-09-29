/* [s65 LANDED 2026-09-29, manual lane] COMPLETED-C body as spliced into
 * src/ings.c (types included; they replace the file's Func80017A44Record /
 * Func80017A44Output definitions).  Sandbox 0 at 127/127, full-build SHA1 ==
 * oracle.  The s1-s64 cast-arithmetic chassis (floor 3) is in this file's git
 * history; evidence.md s65 explains the closure. */
typedef struct {
    s32 pos[3];
    s32 field_C;
    s32 field_10;
    s32 field_14;
    s32 index;
    s32 field_1C;    /* entries used in field_24 */
    s32 field_20;    /* entries used in field_2C */
    u8 field_24[8];  /* edges whose node a is this record */
    u8 field_2C[8];  /* edges whose node b is this record */
    s32 distance;
    s32 field_38[2];
} Func80017A44Record;

typedef struct {
    s32 dist;
    union {
        s32 pair; /* a << 16 | b */
        struct {
            u16 b;
            s16 a;
        } node;
    } ends;
    s32 field_8;
    s32 group_id;
} Func80017848Edge;

typedef struct {
    u8 field_0[6];
    s16 edge_count;
    u8 field_8[4];
    Func80017A44Record *records;
    Func80017848Edge *edges;
} Func80017A44Output;

/* Adds an edge between records a and b unless a == b, both records have a
 * non-negative index, or an edge a->b or b->a already exists. Returns 1 when
 * an edge was added. */
s32 func_80017848(Func80017A44Output *out, s32 group_id, s32 a, s32 b) {
    s32 i;
    s32 dist;
    Func80017848Edge *edge;

    if (a == b) {
        return 0;
    }
    if (out->records[a].index >= 0 && out->records[b].index >= 0) {
        return 0;
    }
    for (i = 0; i < out->records[a].field_1C; i++) {
        if (out->edges[out->records[a].field_24[i]].ends.node.b == b) {
            return 0;
        }
    }
    for (i = 0; i < out->records[a].field_20; i++) {
        if (out->edges[out->records[a].field_2C[i]].ends.node.a == b) {
            return 0;
        }
    }
    dist = math_Distance3D(out->records[a].pos, out->records[b].pos);
    edge = &out->edges[out->edge_count];
    edge->dist = dist;
    edge->field_8 = dist * 3;
    edge->group_id = group_id;
    edge->ends.pair = (a << 16) | b;
    out->records[a].field_24[out->records[a].field_1C++] = out->edge_count;
    out->records[b].field_2C[out->records[b].field_20++] = out->edge_count;
    out->edge_count++;
    return 1;
}
