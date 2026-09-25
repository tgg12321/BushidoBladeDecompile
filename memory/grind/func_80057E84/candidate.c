typedef struct {
    s16 x;
    s16 z;
    u8 poly;
} PathNode;

typedef struct {
    u8 poly;
    u8 vtx;
    u8 count;
    PathNode node[8];
} PathBuf;

/* The walker func_80058580 hands in: world x/z at +0xF4/+0xFC (the s32
 * fields func_80057ACC reads) and its route record at +0x360, laid out like
 * the local PathBuf (poly/vertex index, node count, 6-byte nodes from +0x364;
 * func_80058580 writes node 0 and count = 1 at 0x800594E0 before calling).
 * Declared as a struct rather than read through (u8 *) casts: a COMPONENT_REF
 * load is MEM_IN_STRUCT_P, which sched.c true_dependence exempts from the
 * fixed-address stores to dn_x/up_x..., so sched1 can issue both loads ahead
 * of the four stores as the target does (lhu v0 / lhu a0 back to back); the
 * cast spelling orders them load/store/load/store (26, rejected/). */
typedef struct {
    u8 pad00[0xF4];
    s32 x;
    u8 padF8[4];
    s32 z;
    u8 pad100[0x360 - 0x100];
    PathBuf path;
} PathWalker;

/* Route arg0 around the edges of its current polygon toward (goal_x, goal_z):
 * walk the vertex ring both ways from the vertex arg0 stands at, stepping to
 * the next corner while the segment to the goal is blocked by an edge, and
 * append the cheaper of the two corner chains (up to 8 corners) to arg0's
 * route. */
void func_80057E84(PathWalker *arg0, u8 *arg1, s32 goal_x, s32 goal_z) {
    PathBuf path[2];
    s16 ofs0_x;
    s16 ofs0_z;
    s16 ofs1_x;
    s16 ofs1_z;
    s32 hit_x;
    s32 hit_z;
    s16 dn_x;
    s16 dn_z;
    s16 up_x;
    s16 up_z;
    u8 go_dn;
    s8 idx_dn;
    s8 idx_up;
    s16 iter;
    s16 nedges;
    s32 dist_dn;
    s32 dist_up;
    u8 go_up;
    u8 hit_dn;
    u8 hit_up;
    s16 i;
    s16 next;
    s16 ax;
    s16 az;
    s16 bx;
    s16 bz;
    u8 *poly;
    s16 *va;
    s16 *vb;
    PathBuf *buf;

    go_dn = 1;
    go_up = 1;
    poly = (u8 *)(*(s32 *)(arg1 + 4) + arg0->path.poly * 8);
    dn_x = up_x = arg0->x;
    dn_z = up_z = arg0->z;
    path[1].count = 0;
    path[0].count = 0;
    dist_up = 0;
    dist_dn = 0;
    idx_dn = arg0->path.vtx;
    idx_up = idx_dn + 1;
    if (!(idx_up < poly[3])) {
        if (poly[0] & 0x80) {
            go_up = 0;
        } else {
            idx_up = 0;
        }
    }
    nedges = poly[3];
    if (poly[0] & 0x80) {
        nedges--;
    }
    for (iter = 0; iter < nedges; iter++) {
        hit_up = 0;
        hit_dn = 0;
        for (i = 0; i < nedges; i++) {
            next = i + 1;
            if (!(next < poly[3])) {
                next = 0;
            }
            va = *(s16 **)(poly + 4) + i * 2;
            ax = va[0];
            az = va[1];
            func_80057CC8(poly, i, &ofs0_x, &ofs0_z);
            vb = *(s16 **)(poly + 4) + next * 2;
            bx = vb[0];
            bz = vb[1];
            func_80057CC8(poly, next, &ofs1_x, &ofs1_z);
            if (go_dn && !hit_dn) {
                if ((dn_x != ofs0_x || dn_z != ofs0_z) && (dn_x != ofs1_x || dn_z != ofs1_z)) {
                    if (func_8005763C(dn_x, dn_z, goal_x, goal_z, ax, az, bx, bz, &hit_x, &hit_z)) {
                        hit_dn = 1;
                    }
                }
            }
            if (go_up && !hit_up) {
                if ((up_x != ofs0_x || up_z != ofs0_z) && (up_x != ofs1_x || up_z != ofs1_z)) {
                    if (func_8005763C(up_x, up_z, goal_x, goal_z, ax, az, bx, bz, &hit_x, &hit_z)) {
                        hit_up = 1;
                    }
                }
            }
            if (hit_dn && hit_up) {
                break;
            }
        }
        if (go_dn) {
            if (hit_dn) {
                s32 c;
                s16 *cv;
                PathNode *node;

                cv = *(s16 **)(poly + 4) + idx_dn * 2;
                dist_dn += SquareRoot0((cv[0] - dn_x) * (cv[0] - dn_x) + (cv[1] - dn_z) * (cv[1] - dn_z));
                func_80057CC8(poly, idx_dn, &dn_x, &dn_z);
                /* FAKE: the route-buffer pointer `buf` (whose real job is the
                 * pick-and-copy tail) is staged here and in the up block while
                 * dead; each staged value is consumed by the next lines.
                 * mechanism: global.c -- the shared pseudo spans the tail and
                 * both blocks and is seated in $t0 (addiu t0,sp,40 in the
                 * delay slot of the c >= 7 test) as the target; block-local
                 * pointers are seated in $a1 (4). lever-exhaustion:
                 * memory/grind/func_80057E84/hypotheses.md. */
                buf = &path[0];
                c = buf->count;
                buf->count = c + 1;
                if (c >= 7) {
                    go_dn = 0;
                }
                node = &buf->node[c];
                node->x = dn_x;
                node->z = dn_z;
                node->poly = poly[1];
                if (--idx_dn < 0) {
                    if (poly[0] & 0x80) {
                        go_dn = 0;
                        dist_dn = 100000;
                    } else {
                        idx_dn = poly[3] - 1;
                    }
                }
            } else {
                dist_dn += SquareRoot0((goal_x - dn_x) * (goal_x - dn_x) + (goal_z - dn_z) * (goal_z - dn_z));
                go_dn = 0;
            }
        }
        if (go_up) {
            if (hit_up) {
                s32 c;
                s16 *cv;
                PathNode *node;

                cv = *(s16 **)(poly + 4) + idx_up * 2;
                dist_up += SquareRoot0((cv[0] - up_x) * (cv[0] - up_x) + (cv[1] - up_z) * (cv[1] - up_z));
                func_80057CC8(poly, idx_up, &up_x, &up_z);
                buf = &path[1];
                c = buf->count;
                buf->count = c + 1;
                if (c >= 7) {
                    go_up = 0;
                }
                node = &buf->node[c];
                node->x = up_x;
                node->z = up_z;
                node->poly = poly[1];
                if (!(++idx_up < poly[3])) {
                    if (poly[0] & 0x80) {
                        go_up = 0;
                        dist_up = 100000;
                    } else {
                        idx_up = 0;
                    }
                }
            } else {
                dist_up += SquareRoot0((goal_x - up_x) * (goal_x - up_x) + (goal_z - up_z) * (goal_z - up_z));
                go_up = 0;
            }
        }
        if (!go_dn && !go_up) {
            break;
        }
    }
    if (dist_dn < dist_up) {
        buf = &path[0];
    } else {
        buf = &path[1];
    }
    /* FAKE: the copy loop reuses the edge-loop counter `i` (dead after the
     * search). mechanism: global.c -- i's pseudo crosses the search loop's
     * calls and is seated in callee-saved $s2, which the target's copy loop
     * uses; a fresh call-free counter takes $t0, which drops $t0 from the
     * reload spill set and rotates every reload register through the
     * function (79). lever-exhaustion: memory/grind/func_80057E84/
     * hypotheses.md. */
    for (i = buf->count - 1; i >= 0; i--) {
        arg0->path.node[arg0->path.count] = buf->node[i];
        arg0->path.count++;
    }
}
