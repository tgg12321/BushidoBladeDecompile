/* Route arg0 around the edges of its current polygon toward (goal_x, goal_z):
 * walk the vertex ring both ways from the vertex arg0 stands at, stepping to
 * the next corner while the segment to the goal is blocked by an edge, and
 * append the cheaper of the two corner chains (up to 8 corners) to arg0's
 * route. */
void func_80057E84(PracticeMenuRec *arg0, NavPolySet *arg1, s32 goal_x, s32 goal_z) {
    CpuRoute path[2];
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
    /* FAKE: one counter for the edge scan and the route copy, reused the way
     * SOTN's DebugCaptureScreen reuses its i for its file-search loop and its
     * row countdown (Q51, Q53); lever-exhaustion: a separate copy counter,
     * memory/grind/func_80057E84/r11/README.md */
    s16 i; /* SOTN: src/dra/42398.c:75 @aa53500 */
    s16 next;
    s16 ax;
    s16 az;
    s16 bx;
    s16 bz;
    NavPoly *poly;
    /* route holds three values: the down route (&path[0]) in the down corner
     * block, the up route (&path[1]) in the up corner block, and the cheaper of
     * the two for the copy loop. One local, not three: Ruling 11
     * (.claude/rules/reused-local-necessity.md); proof:
     * memory/grind/func_80057E84/r11/README.md. */
    CpuRoute *route;

    go_dn = 1;
    go_up = 1;
    poly = &arg1->polys[arg0->cpu_route.poly];
    dn_x = up_x = arg0->unk_F4.x;
    dn_z = up_z = arg0->unk_F4.z;
    path[1].count = 0;
    path[0].count = 0;
    dist_up = 0;
    dist_dn = 0;
    idx_dn = arg0->cpu_route.vtx;
    idx_up = idx_dn + 1;
    if (!(idx_up < poly->nvtx)) {
        if (poly->flags & 0x80) {
            go_up = 0;
        } else {
            idx_up = 0;
        }
    }
    nedges = poly->nvtx;
    if (poly->flags & 0x80) {
        nedges--;
    }
    for (iter = 0; iter < nedges; iter++) {
        /* vtx holds four values, each the address of one vertex's x/z pair:
         * the edge's start (i) and end (next) in the edge scan, then the down
         * corner (idx_dn) and the up corner (idx_up). One local, not four:
         * Ruling 11 (.claude/rules/reused-local-necessity.md); proof:
         * memory/grind/func_80057E84/r11/README.md. */
        s16 *vtx;
        /* node holds two values: the waypoint appended to the down route,
         * then the one appended to the up route. One local, not two: Ruling 11
         * (.claude/rules/reused-local-necessity.md); proof:
         * memory/grind/func_80057E84/r11/README.md. */
        CpuWaypoint *node;

        hit_up = 0;
        hit_dn = 0;
        for (i = 0; i < nedges; i++) {
            next = i + 1;
            if (!(next < poly->nvtx)) {
                next = 0;
            }
            vtx = poly->vtx[i];
            ax = vtx[0];
            az = vtx[1];
            func_80057CC8(poly, i, &ofs0_x, &ofs0_z);
            vtx = poly->vtx[next];
            bx = vtx[0];
            bz = vtx[1];
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

                vtx = poly->vtx[idx_dn];
                dist_dn += SquareRoot0((vtx[0] - dn_x) * (vtx[0] - dn_x) + (vtx[1] - dn_z) * (vtx[1] - dn_z));
                func_80057CC8(poly, idx_dn, &dn_x, &dn_z);
                route = &path[0];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    go_dn = 0;
                }
                node = &route->node[c];
                node->x = dn_x;
                node->z = dn_z;
                node->kind = poly->kind;
                if (--idx_dn < 0) {
                    if (poly->flags & 0x80) {
                        go_dn = 0;
                        dist_dn = 100000;
                    } else {
                        idx_dn = poly->nvtx - 1;
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

                vtx = poly->vtx[idx_up];
                dist_up += SquareRoot0((vtx[0] - up_x) * (vtx[0] - up_x) + (vtx[1] - up_z) * (vtx[1] - up_z));
                func_80057CC8(poly, idx_up, &up_x, &up_z);
                route = &path[1];
                c = route->count;
                route->count = c + 1;
                if (c >= 7) {
                    go_up = 0;
                }
                node = &route->node[c];
                node->x = up_x;
                node->z = up_z;
                node->kind = poly->kind;
                if (!(++idx_up < poly->nvtx)) {
                    if (poly->flags & 0x80) {
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
        route = &path[0];
    } else {
        route = &path[1];
    }
    for (i = route->count - 1; i >= 0; i--) {
        arg0->cpu_route.node[arg0->cpu_route.count] = route->node[i];
        arg0->cpu_route.count++;
    }
}
