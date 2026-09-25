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

typedef struct {
    u8 pad00[0xF4];
    s32 x;
    u8 padF8[4];
    s32 z;
    u8 pad100[0x360 - 0x100];
    PathBuf path;
} PathWalker;

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
    PathNode *node;
    s16 *vtx;
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
                    if (func_8005763C(dn_x, dn_z, goal_x, goal_z, ax, az, bx, bz,
                                      &hit_x, &hit_z)) {
                        hit_dn = 1;
                    }
                }
            }
            if (go_up && !hit_up) {
                if ((up_x != ofs0_x || up_z != ofs0_z) && (up_x != ofs1_x || up_z != ofs1_z)) {
                    if (func_8005763C(up_x, up_z, goal_x, goal_z, ax, az, bx, bz,
                                      &hit_x, &hit_z)) {
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

                vtx = *(s16 **)(poly + 4) + idx_dn * 2;
                dist_dn += SquareRoot0((vtx[0] - dn_x) * (vtx[0] - dn_x) + (vtx[1] - dn_z) * (vtx[1] - dn_z));
                func_80057CC8(poly, idx_dn, &dn_x, &dn_z);
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

                vtx = *(s16 **)(poly + 4) + idx_up * 2;
                dist_up += SquareRoot0((vtx[0] - up_x) * (vtx[0] - up_x) + (vtx[1] - up_z) * (vtx[1] - up_z));
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
    for (i = buf->count - 1; i >= 0; i--) {
        arg0->path.node[arg0->path.count] = buf->node[i];
        arg0->path.count++;
    }
}
