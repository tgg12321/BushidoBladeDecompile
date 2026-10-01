extern u8 D_8009A870[];
extern u8 D_8009A874[];
extern u8 D_8009A878[];
extern u8 D_8009A880[];
extern u8 D_8009A888[];
extern u8 D_8009A890[];
extern u8 D_8009A898[];
extern u8 D_8009A89C[];
extern u8 D_8009A8A4[];
extern u8 D_8009A8AC[];
extern u8 D_8009A8B4[];
extern u8 D_8009A8C0[];
extern u8 D_8009A850[8][4];
extern u8 D_8009A658[][12];
extern u16 D_8009A928[][23];
extern u8 D_8009A9DC[][3];
extern s32 D_8009A9F0[][8];
extern u8 D_800A325C[4];
extern u8 D_800A3260[4];
extern void func_80057E84(PracticeMenuRec *, u8 *, s32, s32);

#define CPU_SQ(x) ((x) * (x))

s32 func_80058580(PracticeMenuRec *p) {
    s32 wx;
    u8 *pscript;
    s32 wy;
    s8 bestflip;
    s16 pbesti;
    s32 phi;
    s32 lv;
    u8 *script1;
    u8 *script2;
    u8 *script3;
    u8 mode;
    /* work1 holds nine values in turn, each read before work1 is written again: the
     * unk_444[5] == 0 flag of the state-0x15 script choice; the stage distance base (100000,
     * or D_8009A838[stage] * 8) of the D_8009A850 scan; unk_444[6] for the lim chain; unk_444[6]
     * again for the waypoint script; the x of waypoint 1; the unk_444[1] == 0 flag of the 0x394
     * action pick; the chosen pick (besti sign-extended, `sll; sra 24` at 0x8005A364); case 2's
     * D_8009A9F0 pattern word, shifted in place; a script entry's low distance bound (e[1] * 40,
     * then adjusted).
     * Ruling 11 (.claude/rules/reused-local-necessity.md); proof: memory/grind/func_80058580/r11/README.md. */
    s32 work1;
    /* work2 holds eight values in turn, each read before work2 is written again: the
     * unk_444[1] == 0 flag of the state-0x15 script choice; a D_8009A850 entry's distance; the
     * pace byte unk_444[0]; the z of waypoint 1; the unk_444[5] == 0 flag of the 0x394 action
     * pick; the best random pick score so far (Q75 constant start + copy, rules 30a3e2d2d:
     * `li $s2,-1` at 0x8005A108 / 0x8005A118, `addu $s2,$s3,$zero` at 0x8005A350; compared as
     * an s16, `sll; sra 16` at 0x8005A338); case 2's nibble count, counted down; the state-0x15
     * script's counter value (unk_34D, unk_34A, 0, or unk_330).
     * Ruling 11 (.claude/rules/reused-local-necessity.md); proof: memory/grind/func_80058580/r11/README.md. */
    s32 work2;
    /* work3 holds sixteen values in turn, each read before work3 is written again: the
     * script side bit (opponent unk_AF & 1, possibly inverted); unk_444[3] == 0; the unk_43A
     * angle, wrapped to +-0x800; the state-0x11 threshold (0x1000 - (stance sum << 8)), scaled
     * by unk_438 >> 12; the forced-scan flag (0 or 1) of the D_8009A850 scan; the waypoint index
     * unk_362 - 1; the path length to the target; the "longer than lim" flag; the bearing to the
     * next waypoint, wrapped; the 0x394 action pick's coin bit, stepped per try; the 0x394 slot
     * (unk_394, or a D_800A325C / D_800A3260 entry); case 3's column in D_8009A928; the
     * entry-type mask (case 3 / case 2 / default, tested as unsigned); a script entry's
     * character mask (tested as unsigned); the entry's accept flag (0 or 1); the state-0x15
     * script address (0, or D_8009A8C0 / D_8009A8B4 / D_8009A8AC, held as an integer).
     * Read-before-write kept from the original (owner ruling Q74, rules 30a3e2d2d): the 0x394
     * slot switch below reads work3 with no write on the path unk_39C == 1, opponent state
     * neither 0x19 nor 0x1A (target 0x80059D18 -> 0x80059D6C -> 0x80059DB0; $s3 read by the
     * `sltiu` at 0x80059D70 and the `sll` at 0x80059DB4), i.e. it switches on whatever value
     * earlier work left in work3. That read is excluded from the value grouping above.
     * Ruling 11 (.claude/rules/reused-local-necessity.md); proof: memory/grind/func_80058580/r11/README.md. */
    s32 work3;
    /* work4 holds three values in turn, each read before work4 is written again: the
     * D_8009A850 scan index; the waypoint walk index (a copy of work3's waypoint index, Q34:
     * `addu $s4,$s3,$zero` at 0x800596DC, counted down); the script-list entry index.
     * Ruling 11 (.claude/rules/reused-local-necessity.md); proof: memory/grind/func_80058580/r11/README.md. */
    s32 work4;
    s32 pick;
    s32 hi;
    s16 et;
    s32 va;
    s32 vd;
    s32 vb;
    s32 vn;
    s32 vc;
    s16 sc;
    u8 *pois;
    s32 tx, ty2;
    s32 r;
    s32 sel;
    u16 *list;
    u8 *e;
    u8 *ep;
    u8 *q; /* FAKE: second handle to the script start, see `q = ep;` */
    u16 off;
    s16 pbest;
    u8 st2;
    s8 flip;
    s32 near;
    s32 wtype;
    s32 cnt;
    s32 ok4;
    s32 far;
    s32 lim;
    s8 besti;
    s32 score;

    if (p->unk_00->unk_6A == 4 || p->unk_00->unk_6A == 0x14) {
        return 0;
    }
    if (p->unk_443 != 0x16 && ((p->unk_430 & 0x15100) ||
                             (p->unk_6A == 0xD && (p->unk_426 == 4 || p->unk_425 == 4)))) {
        script1 = 0;
        if ((p->unk_426 == 1 && p->unk_00->unk_40 + 1 >= p->unk_427) || p->unk_425 == 2) {
            if (p->unk_430 & 8) {
                u8 *tbl[2];
                s32 f;
                tbl[0] = D_8009A874;
                tbl[1] = D_8009A870;
                work3 = p->unk_00->unk_AF & 1;
                f = p->unk_430;
                if (!(((f & 0x20) || ((f & 0x10) && p->unk_3F3 % ((p->unk_438 >> 8) + 2) != (p->unk_438 >> 8) + 1)) &&
                      (!(D_80099D88[p->unk_443].flags & 0xFF00) || (f & 0x40))) ||
                    (file_GetFlag1() && D_800A38DC != 3)) {
                    work3 = !work3;
                }
                script1 = tbl[work3];
                mode = 3;
            }
            if (p->unk_6A == 0xD) {
                script1 = 0;
                p->unk_3CC = 0;
                p->unk_426 = 4;
                p->unk_425 = 4;
                p->unk_3F3++;
            }
        } else if (p->unk_6A == 0x15) {
            mode = 4;
            work3 = p->unk_444[3] == 0;
            work1 = p->unk_444[5] == 0;
            work2 = p->unk_444[1] == 0;
            if (p->unk_426 == 2) {
                if (p->unk_42E * p->unk_42E <
                    CPU_SQ(p->unk_42A - p->unk_F4.x) + CPU_SQ(p->unk_42C - p->unk_F4.z)) {
                    p->unk_3CC = 0;
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                } else if (work3 && (p->unk_430 & 8) &&
                           (!(D_80099D88[p->unk_443].flags & 0x8F00) || ((D_80099D88[p->unk_443].flags & 0x300) && D_800A37A0 >= 6))) {
                    script1 = D_8009A890;
                }
            }
            if (p->unk_425 == 1 ||
                (p->unk_426 == 1 && p->unk_00->unk_441 == 2 && p->unk_427 - p->unk_00->unk_40 >= 6) ||
                (p->unk_426 == 2 && script1 == 0)) {
                if ((p->unk_430 & 8) &&
                    (!(D_80099D88[p->unk_443].flags & 0xBF00) || ((D_80099D88[p->unk_443].flags & 0x300) && D_800A37A0 >= 7))) {
                    if (p->unk_00->unk_43A > 0) {
                        if (work1) {
                            script1 = D_8009A880;
                        } else if (work2) {
                            script1 = D_8009A878;
                        }
                    } else {
                        if (work2) {
                            script1 = D_8009A878;
                        } else if (work1) {
                            script1 = D_8009A880;
                        }
                    }
                    if (script1 == 0) {
                        p->unk_426 = 1;
                        p->unk_425 = 2;
                        return -1;
                    }
                    p->unk_426 = 4;
                    p->unk_425 = 4;
                    p->unk_3F3++;
                }
            }
        }
        if (script1 != 0) {
            func_80055B44(p, script1, mode, 0);
            return p->unk_3CC;
        }
    }

    if ((p->unk_430 & 0x400) && p->unk_441 != 2) {
        if (D_80099D88[p->unk_443].flags & 0x8000) {
            p->unk_3CC = 0x2000;
        } else if (p->unk_430 & 8) {
            work3 = p->unk_43A;
            if (p->unk_441 == 0) {
                work3 = (work3 + 0x800) & 0xFFF;
                if (work3 > 0x800) {
                    work3 -= 0x1000;
                }
            }
            if (!(D_80099D88[p->unk_443].flags & 0xF00)) {
                if (work3 < 0) {
                    if (p->unk_444[1] == 0 || p->unk_444[1] == 3) {
                        p->unk_3CC = 0x4000;
                    }
                } else {
                    if (p->unk_444[5] == 0 || p->unk_444[5] == 3) {
                        p->unk_3CC = 0x1000;
                    }
                }
            }
            if (!(p->unk_425 == 1 || p->unk_425 == 2) && p->unk_426 != 2 &&
                (p->unk_3CC == 0 || p->unk_442 != 0 || p->unk_00->unk_6A == 2 ||
                 p->unk_00->unk_6A == 0x29 || p->unk_00->unk_6A == 0x13 ||
                 p->unk_00->unk_6A == 6 || p->unk_00->unk_404[p->unk_00->unk_86] < D_800A387C)) {
                if (p->unk_43C < 0x400 && p->unk_444[3] == 0) {
                    p->unk_3CC = 0x8000;
                } else {
                    p->unk_3CC = 0x2000;
                }
            }
        }
    } else {
        u16 state;

        state = p->unk_6A;
        if (state == 0xF || state == 0x1C || state == 0x1D || state == 0x1E || state == 0x1F || state == 0x20 || state == 0x21) {
            if (p->unk_6A == 0x1D && (rand() & 0xFF) < D_80099D88[p->unk_443].unk4 && p->unk_444[3] == 0) {
                p->unk_3CC = 0x8000;
            } else {
                vd = 0x80;
                if (p->unk_3E8 % (0x12 - (p->unk_438 >> 8)) == 0) {
                    vd = 0x20;
                }
                p->unk_3CC = vd;
                if (p->unk_444[3] != 0) {
                    if (p->unk_444[2] != 0) {
                        p->unk_3CC = vd | 0x1000;
                    } else if (p->unk_444[4] != 0) {
                        p->unk_3CC = vd | 0x4000;
                    }
                } else if (p->unk_444[7] == 0) {
                    p->unk_3CC = (p->unk_443 & 1) ? vd | 0x1000 : vd | 0x4000;
                }
            }
        } else if (state == 0x11 && p->unk_04 != D_800A38AE && p->unk_40 == p->unk_50[8] - 1) {
            {
                s32 b, c, a;
                a = p->unk_26E;
                b = p->unk_270;
                c = p->unk_272;
                work3 = 0x1000 - (((p->unk_26C == 0 ? a + 4 + b : a + b) + c) << 8);
                work3 = (p->unk_438 * work3) >> 12;
                if ((rand() & 0xFFF) < work3) {
                    p->unk_3CC = 0x20;
                } else if (p->unk_430 & 0x20) {
                    p->unk_3CC = 0x20;
                }
            }
        } else {
            if ((p->unk_148 - p->unk_B8.vy >= 0 ? p->unk_148 - p->unk_B8.vy
                                                     : p->unk_B8.vy - p->unk_148) < 200 && !(D_80099D88[p->unk_443].flags & 0x8C00) &&
                ((!file_GetFlag1() && D_800A38DC != 3) || D_800A38DC == 3)) {
                work3 = 0;
                if ((p->unk_426 == 1 || p->unk_425 == 2) && (p->unk_430 & 8)) {
                    work3 = 1;
                }
                far = 0;
                if ((rand() & 0xFFF) < (p->unk_438 >> 3)) {
                    far = p->unk_3E8 > 0x3C;
                }
                if (p->unk_0E >= 6) {
                    work1 = 100000;
                } else {
                    work1 = D_8009A838[p->unk_0E] * 8;
                }
                for (work4 = 0; work4 < sizeof(D_8009A850) / sizeof(D_8009A850[0]); work4++) {
                    if (!(D_8009A850[work4][3] & 1) || p->unk_40 >= p->unk_50[8] - 2 || work3) {
                        work2 = D_8009A850[work4][2] * 16 + work1 + p->unk_40A;
                        if ((D_800A387C < work2 &&
                             (!(D_8009A850[work4][3] & 8) || p->unk_43C < 0x100) &&
                             (far || (D_8009A850[work4][3] & 4))) ||
                            work3) {
                            if (D_8009A850[work4][0] == p->unk_6A &&
                                ((st2 = D_8009A850[work4][1]) == 0xFF || st2 == p->unk_00->unk_6A)) {
                                if (D_8009A850[work4][3] & 2) {
                                    vb = 0x40;
                                    if (p->unk_3E8 & 1) {
                                        vb = 0x20;
                                    }
                                    p->unk_3CC = vb;
                                } else if ((D_80099D88[p->unk_443].flags & 0x20) && !(p->unk_440 == 3 || p->unk_440 == 4)) {
                                    p->unk_3CC = (p->unk_3E8 & 1) * 8;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 0x4108) == 8) {
        u16 state;

        state = p->unk_6A;
        if (state == 3 || state == 0x2C || state == 7 || (p->unk_426 == 1 || p->unk_426 == 2) ||
            (p->unk_425 == 1 || p->unk_425 == 2)) {
            p->unk_39D = 0;
            p->unk_362 = 0;
            p->unk_398 = 0;
            return 0;
        }
    }

    if (p->unk_430 & 0x5100) {
        pois = D_8009A658[D_800A36A4];
        if (p->unk_39D == 0) {
            tx = p->unk_00->unk_F4.x;
            ty2 = p->unk_00->unk_F4.z;
            wtype = 1;
        } else {
            tx = p->unk_3A0;
            ty2 = p->unk_3A2;
            wtype = p->unk_39E;
        }
        /* !FAKE: the trailing `&& wtype == 1` repeats the first test (redundant condition,
         * .claude/rules/no-new-park-categories.md entry 16). The target re-tests $s2 after the
         * || chain (`beq $s2,$v0` at 0x80059210); without it the chain is two instructions
         * shorter. Measurements: memory/grind/func_80058580/evidence.md [s5e]. */
        if (!(wtype == 1 &&
              (p->unk_00->unk_6A == 0xA || p->unk_443 == 0xA || (p->unk_0E >= 6 && p->unk_34A == 0)) &&
              wtype == 1)) {
            if (!(p->unk_3E8 & 7)) {
                p->unk_434 = func_80057ACC((s32)p, pois, tx, ty2);
            }
            if (wtype == 1) {
                work1 = p->unk_444[6];
                if (!(p->unk_430 & 0x800) || D_800A387C < 4000) {
                    if (p->unk_442 == 2 && !(work1 == 1 || work1 == 2)) {
                        lim = p->unk_00->unk_3F8[p->unk_00->unk_86];
                    } else if ((p->unk_442 == 1 || p->unk_442 == 2) || p->unk_442 == 3) {
                        if (!(D_80099D88[p->unk_443].flags & 0x4000)) {
                            lim = p->unk_00->unk_3FE[p->unk_00->unk_86];
                        } else {
                            lim = p->unk_00->unk_404[p->unk_00->unk_86];
                        }
                    } else if (p->unk_434 != 100000) {
                        if (p->unk_430 & 0x200) {
                            lim = p->unk_00->unk_404[p->unk_00->unk_86];
                        } else {
                            lim = p->unk_404[p->unk_86] + p->unk_00->unk_404[p->unk_00->unk_86];
                        }
                    } else {
                        lim = p->unk_3FE[p->unk_86] + p->unk_00->unk_3FE[p->unk_00->unk_86];
                        if (p->unk_430 & 0x200) {
                            if (p->unk_00->unk_43C > 0x600) {
                                lim = p->unk_00->unk_3F8[p->unk_00->unk_86];
                            } else if (p->unk_00->unk_43C > 0x300) {
                                lim = p->unk_00->unk_404[p->unk_00->unk_86];
                            } else if (p->unk_00->unk_43C > 0x100) {
                                lim = p->unk_00->unk_3FE[p->unk_00->unk_86];
                            }
                        }
                    }
                } else {
                    lim = 2000;
                }
            } else {
                lim = 2000;
            }
            if (wtype == 1) {
                if (p->unk_362 < 2 && (p->unk_434 != 100000 || D_800A387C >= lim)) {
                    goto record;
                }
            } else if (p->unk_362 == 0) {
                if (p->unk_434 != 100000 || CPU_SQ(p->unk_F4.x - tx) + CPU_SQ(p->unk_F4.z - ty2) > 0x15F8F) {
                record:
                    p->unk_364[0].x = tx;
                    p->unk_364[0].z = ty2;
                    p->unk_364[0].kind = wtype;
                    p->unk_362 = 1;
                    if (p->unk_434 != 100000 && !(p->unk_3E8 & 7)) {
                        func_80057E84(p, pois, tx, ty2);
                    }
                }
            }
            if (p->unk_362 != 0) {
                if (p->unk_364[0].kind == 1 ? (p->unk_434 == 100000 && D_800A387C < lim)
                                  : SquareRoot0(CPU_SQ(p->unk_F4.x - tx) + CPU_SQ(p->unk_F4.z - ty2)) < 2000) {
                    p->unk_362 = 0;
                    p->unk_39D = 0;
                    goto after_nav;
                }
                {
                    work1 = p->unk_444[6];
                    work3 = p->unk_362 - 1;
                    work2 = p->unk_444[0];
                    wx = p->unk_364[work3].x;
                    wy = p->unk_364[work3].z;
                    if (!(work1 == 1 || work1 == 2)) {
                        script2 = 0;
                        if (p->unk_364[work3].kind == 1) {
                            if (work2 == 3 && D_800A387C < p->unk_00->unk_404[p->unk_00->unk_86]) {
                                goto pick2;
                            }
                            /* !FAKE: the inner test contradicts the outer one, so pick2's body runs only by the
                             * goto above; the target compares twice (0x80059638 / 0x8005966C), and failing
                             * either compare here still reaches the script2 call test (redundant condition,
                             * .claude/rules/no-new-park-categories.md entry 16; memory/grind/func_80058580/evidence.md [s5]). */
                            if (work2 == 5 && p->unk_00->unk_404[p->unk_00->unk_86] < D_800A387C) {
                                if (D_800A387C < p->unk_00->unk_404[p->unk_00->unk_86]) {
                                pick2:
                                    if (p->unk_6A == 0x13) {
                                        script2 = D_8009A8A4;
                                    } else if (p->unk_440 != 4) {
                                        script2 = D_8009A89C;
                                    }
                                }
                                if (script2 != 0) {
                                    func_80055B44(p, script2, 4, 0);
                                }
                            }
                        }
                    }
                    if (p->unk_3CC != 0) {
                        return p->unk_3CC;
                    }
                    work4 = work3;
                    if (work3 == 0) {
                        if (p->unk_364[0].kind == 1) {
                            work3 = SquareRoot0(CPU_SQ(p->unk_F4.x - tx) + CPU_SQ(p->unk_F4.z - ty2));
                        } else {
                            work3 = SquareRoot0(CPU_SQ(p->unk_F4.x - wx) + CPU_SQ(p->unk_F4.z - wy));
                        }
                    } else {
                        work3 = SquareRoot0(CPU_SQ(wx - p->unk_F4.x) + CPU_SQ(wy - p->unk_F4.z));
                        while (work4 >= 2) {
                            work3 += SquareRoot0(CPU_SQ(p->unk_364[work4].x - p->unk_364[work4 - 1].x) +
                                                CPU_SQ(p->unk_364[work4].z - p->unk_364[work4 - 1].z));
                            work4--;
                        }
                        {
                            work1 = p->unk_364[1].x;
                            work2 = p->unk_364[1].z;
                            if (p->unk_364[0].kind == 1) {
                                work3 += SquareRoot0(CPU_SQ(work1 - tx) + CPU_SQ(work2 - ty2));
                            } else {
                                work3 += SquareRoot0(CPU_SQ(work1 - p->unk_364[0].x) + CPU_SQ(work2 - p->unk_364[0].z));
                            }
                        }
                    }
                    work3 = lim < work3;
                    p->unk_3CC = func_80057094(p, wx, wy, work3);
                    va = 300;
                    vn = p->unk_3CC & 4;
                    if (vn) {
                        va = 1000;
                    }
                    if (CPU_SQ(p->unk_364[p->unk_362 - 1].x - p->unk_F4.x) + CPU_SQ(p->unk_364[p->unk_362 - 1].z - p->unk_F4.z) <
                        va * (vn ? 1000 : 300)) {
                        if (--p->unk_362 != 0) {
                            work3 = (ratan2(p->unk_364[p->unk_362 - 1].x - p->unk_F4.x, p->unk_364[p->unk_362 - 1].z - p->unk_F4.z) -
                                   p->unk_1C8.vy) & 0xFFF;
                            if (work3 > 0x800) {
                                work3 -= 0x1000;
                            }
                            if ((work3 < 0 ? -work3 : work3) > 0x300) {
                                return 0;
                            }
                        }
                    }
                }
            }
        after_nav:
            if (p->unk_3CC != 0) {
                return p->unk_3CC;
            }
        }

        if ((!(p->unk_430 & 0x80) && D_800A387C < p->unk_00->unk_404[p->unk_00->unk_86] &&
             ((p->unk_444[3] >= 2 && (p->unk_444[5] >= 2 || p->unk_444[1] >= 2)) || p->unk_444[3] == 1 || p->unk_444[4] == 1 ||
              p->unk_444[2] == 1)) ||
            ((D_80099D88[p->unk_443].flags & 0x4000 || p->unk_443 == 0x18 || p->unk_443 == 0x1A) && D_800A387C < p->unk_00->unk_404[p->unk_00->unk_86] &&
             p->unk_00->unk_43C < 0x10) ||
            (p->unk_0E >= 7 && D_800A387C < p->unk_00->unk_404[p->unk_00->unk_86] && p->unk_00->unk_43C < 0x10 &&
             p->unk_34A != 0)) {
            if (!(D_80099D88[p->unk_443].flags & 0x8F00)) {
                if ((p->unk_362 = func_800571C0((s32)p)) != 0) {
                    p->unk_39D = 2;
                    return -1;
                }
            }
            if (!(D_80099D88[p->unk_443].flags & 0x8C00)) {
                cnt = 0;
                work3 = D_80099D88[p->unk_443].pick_weight[4] < (rand() & 0xFF);
                sel = -1;
                work1 = p->unk_444[1] == 0;
                work2 = p->unk_444[5] == 0;
                for (; cnt < 2; cnt++, work3++) {
                    if (work3 & 1) {
                        if (p->unk_00->unk_43A < 0) {
                            if (work1) {
                                sel = 6;
                            } else if (work2) {
                                sel = 7;
                            }
                        } else {
                            if (work2) {
                                sel = 7;
                            } else if (work1) {
                                sel = 6;
                            }
                        }
                        if (sel != -1) {
                            break;
                        }
                    } else if (p->unk_440 != 4 && D_800A387C > 3000 && D_800A387C < 5000) {
                        sel = 8;
                        break;
                    }
                }
                if (sel != -1) {
                    p->unk_394 = sel;
                    p->unk_39C = 0;
                    p->unk_398 = p->unk_39A;
                }
            }
        }

        if (p->unk_398 >= p->unk_39A) {
            script3 = 0;
            if (p->unk_39C != 1) {
                work3 = p->unk_394;
            } else if (p->unk_00->unk_6A == 0x19) {
                u8 slots0[4];
                __builtin_memcpy(slots0, D_800A325C, 4);
                work3 = slots0[p->unk_00->unk_441];
            } else if (p->unk_00->unk_6A == 0x1A) {
                u8 slots1[4];
                __builtin_memcpy(slots1, D_800A3260, 4);
                work3 = slots1[p->unk_00->unk_441];
            }
            switch (work3) {
            case 0:
                if (p->unk_444[3] == 0) {
                    p->unk_3CC = 0x8000;
                }
                break;
            case 1:
                if (p->unk_444[0] == 0) {
                    p->unk_3CC = 0x2000;
                }
                break;
            case 2:
                if (p->unk_444[1] == 0) {
                    p->unk_3CC = 0x4000;
                }
                break;
            case 3:
                if (p->unk_444[5] == 0) {
                    p->unk_3CC = 0x1000;
                }
                break;
            case 4:
                if (p->unk_444[3] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A890;
                }
                break;
            case 5:
                if (p->unk_444[0] == 0 && p->unk_3F8[p->unk_86] < D_800A387C) {
                    script3 = D_8009A888;
                }
                break;
            case 6:
                if (p->unk_444[1] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A878;
                }
                break;
            case 7:
                if (p->unk_444[5] == 0 && D_800A387C < p->unk_3FE[p->unk_86]) {
                    script3 = D_8009A880;
                }
                break;
            case 8:
                if (!(p->unk_444[6] == 1 || p->unk_444[6] == 2) && D_800A387C > 3000 && D_800A387C < 5000 &&
                    p->unk_43C < 0x200) {
                    script3 = D_8009A89C;
                }
                break;
            }
            if ((p->unk_426 == 1 || p->unk_426 == 2) || (p->unk_425 == 1 || p->unk_425 == 2)) {
                if (p->unk_3CC != 0) {
                    p->unk_398 = p->unk_39A + 1;
                } else {
                    p->unk_398 = 0;
                    script3 = 0;
                }
            } else if ((p->unk_430 & 0x800) || p->unk_442 != 0 ||
                       (p->unk_3CC == 0x2000 && D_800A387C < p->unk_00->unk_3F8[p->unk_00->unk_86])) {
                script3 = 0;
                p->unk_398 = 0;
                p->unk_3CC = 0;
            }
            if (script3 != 0) {
                p->unk_398 = 0;
                if (p->unk_430 & 0xA000) {
                    func_80055B44(p, script3, 4, 0);
                }
            }
        } else if (p->unk_398 == 0) {
            s32 tired = D_80099D88[p->unk_443].unk6;
            r = rand() & 0xFF;
            if (p->unk_440 == 4 ? r < (tired >> 2) : r < tired) {
                work2 = -1;
                besti = -1;
                pick = 0;
            pick_loop:
                {
                    /* FAKE: named intermediates (.claude/rules/no-new-park-categories.md entry 6): with the
                     * row named, the symbol is added to the row offset before the pick index, so combine
                     * cannot fold `la D_80099D90` into the lbu offset (target `la; addu row; addu pick;
                     * lbu 0`); 13 expression-level spellings fold (evidence.md [s3] 50 -> 40). */
                    s32 rnd;
                    u8 *row;
                    rnd = rand() & 0xFFF;
                    row = D_80099D88[p->unk_443].pick_weight;
                    score = (rnd * row[pick]) >> 12;
                    if (score != 0) {
                        flip = 0;
                        switch (pick) {
                        case 0:
                        case 2:
                            if (rand() & 1) {
                                flip = 1;
                            } else if (p->unk_444[3] != 0) {
                                goto pick_next;
                            }
                            vc = p->unk_3F0;
                            if (D_80099D88[p->unk_443].unk7 < (vc >= 0 ? vc : -vc)) {
                                score += 0x80;
                                flip = vc > 0;
                            }
                            if (p->unk_0E >= 6 && p->unk_34A == 0 && pick == 2) {
                                score += 0x100;
                                flip = 0;
                            }
                            break;
                        case 1:
                        case 3:
                            if (rand() & 1) {
                                if (p->unk_444[5] != 0) {
                                    goto pick_next;
                                }
                                flip = 1;
                            } else if (p->unk_444[1] != 0) {
                                goto pick_next;
                            }
                            if (p->unk_430 & 0x20000) {
                                score += 0x80;
                                flip = p->unk_43A > 0;
                                if (*(flip ? &p->unk_444[5] : &p->unk_444[1]) != 0) {
                                    flip ^= 1;
                                }
                            }
                            break;
                        case 4:
                            if ((p->unk_444[6] == 1 || p->unk_444[6] == 2) || !(D_800A387C >= 3000 && D_800A387C <= 5000) ||
                                p->unk_43C >= 0x201 || p->unk_440 == 4 || p->unk_00->unk_6A == 0x18 ||
                                p->unk_00->unk_6A == 0x2A) {
                                goto pick_next;
                            }
                            break;
                        }
                        if ((s16)work2 < score) {
                            besti = pick;
                            work2 = score;
                            bestflip = flip;
                        }
                    }
                }
            pick_next:
                if (++pick < 7) {
                    goto pick_loop;
                }
                if ((work1 = besti) != -1) {
                    p->unk_394 = 0;
                    p->unk_39C = 0;
                    p->unk_398 = (((((rand() & 0xFFF) * D_80099D88[p->unk_443].unk6) >> 12) + 0x17) << 12) / p->unk_1C;
                    switch (work1) {
                    case 0:
                        p->unk_394 = bestflip != 0;
                        break;
                    case 1:
                        if (bestflip) {
                            p->unk_394 = 3;
                        } else {
                            p->unk_394 = 2;
                        }
                        break;
                    case 2:
                        if (bestflip) {
                            p->unk_394 = 5;
                        } else {
                            p->unk_394 = 4;
                        }
                        break;
                    case 3:
                        if (bestflip) {
                            p->unk_394 = 7;
                        } else {
                            p->unk_394 = 6;
                        }
                        break;
                    case 4:
                        p->unk_394 = 8;
                        break;
                    case 5:
                        p->unk_39C = 1;
                        break;
                    }
                }
            }
        }
        if (p->unk_398 != 0) {
            p->unk_398--;
        }
    }

    if (p->unk_3CC != 0 || p->unk_398 != 0) {
        return p->unk_3CC;
    }
    if ((p->unk_430 & 6) && !(p->unk_430 & 0x800)) {
        u16 state;

        state = p->unk_6A;
        if (state == 0x15 || (state == 0x19 && p->unk_441 >= 2)) {
            if ((p->unk_3E8 & 1) || p->unk_0E >= 6) {
                if (p->unk_442 == 0 && p->unk_444[3] != 1 && (p->unk_0E < 6 || p->unk_34A != 0)) {
                    pbest = -1;
                    if (p->unk_3F2 % ((p->unk_438 >> 8) + 2) == (p->unk_438 >> 8) + 1) {
                        lv = p->unk_438 >> 1;
                    } else {
                        lv = p->unk_438;
                    }
                    list = p->unk_3A8[p->unk_86];
                    off = *list;
                    work4 = 0;
                    while (off != 0) {
                        /* work5 holds three values in turn, each read before work5 is written again: case 2's
                         * pattern-word top bits (work1 >> 27); the skill offset ((0x1000 - lv) * 625 >> 10) - 400;
                         * a copy of the entry type et for the et < 5 and et == 5 / 6 tests (Q34: `addu $a1,$s5,$zero` at 0x8005AA94).
                         * Ruling 11 (.claude/rules/reused-local-necessity.md); proof: memory/grind/func_80058580/r11/README.md. */
                        s32 work5;
                        ep = off + (u8 *)p->unk_3A4;
                        e = ep;
                        ep += 4;
                        /* FAKE: pass-through pointer alias (.claude/rules/pointer-alias-fake-exception.md;
                         * SOTN src/st/no0/e_stone_rose.c:611 `fakeEntity = self; // !FAKE` @aa53500). The
                         * target copies the script start into its own register (`addu $a2,$s6,$zero` at
                         * 0x8005A67C) and reads the 0x40 character-mask header through it while ep stays in
                         * $s6; read through ep the header loads use $s6 and global.c's allocno order shifts
                         * (58 words off, memory/grind/func_80058580/review-2026-10-01/dm/results.txt
                         * vM_noq; respellings q = e + 4, e-first, `q = ep += 4`: evidence.md [s3] / [s5]). */
                        q = ep;
                        if (D_80099D88[p->unk_443].flags & 0xFF00) {
                            switch (D_800A38DC) {
                            case 3:
                                if (D_800A38E2 < 0x5B) {
                                    work3 = D_800A38E2 / 10 * 2;
                                    if (D_800A38E2 % 10 == 0) {
                                        work3--;
                                    }
                                } else if (D_800A38E2 < 0x5E) {
                                    work3 = 0x12;
                                } else if (D_800A38E2 < 0x60) {
                                    work3 = 0x13;
                                } else if (D_800A38E2 < 0x62) {
                                    work3 = 0x14;
                                } else if (D_800A38E2 < 0x64) {
                                    work3 = 0x15;
                                } else {
                                    work3 = 0x16;
                                }
                                work3 = D_8009A928[p->unk_440][work3];
                                break;
                            case 2:
                                work1 = D_8009A9F0[D_8009A9DC[p->unk_0E][p->unk_440]][D_800A3788];
                                work3 = 0;
                                work5 = work1 >> 27;
                                work2 = work1 & 0xF;
                                if (work2 != 0) {
                                    if (work5) {
                                        while (work2 > 0) {
                                            work1 >>= 4;
                                            work3 |= 1 << ((work1 & 0xF) - 1);
                                            work2--;
                                        }
                                    } else {
                                        work1 >>= (p->unk_3F2 / 3 % work2) * 4 + 4;
                                        work3 = 1 << ((work1 & 0xF) - 1);
                                    }
                                }
                                break;
                            default:
                                work3 = D_8009A8C8[p->unk_440][D_800A37A0 - 1].mask;
                                break;
                            }
                            if ((e[3] >> 4) == 0 || !((u32)work3 & (1 << ((e[3] >> 4) - 1)))) {
                                goto next;
                            }
                        }
                        if (!(D_80099D88[p->unk_443].flags & 0x80) && p->unk_40D == p->unk_86 && p->unk_40C == work4) {
                            goto next;
                        }
                        if (q[0] == 0x40) {
                            work3 = q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1];
                            if (!((u32)work3 & (1 << p->unk_443))) {
                                goto next;
                            }
                            ep += 5;
                        }
                        if ((e[0] & 0x80) && p->unk_26C == 0) {
                            goto next;
                        }
                        work1 = e[1] * 40;
                        hi = e[2] * 40;
                        /* FAKE: do-while(0) (.claude/rules/do-while-zero-exception.md). Its loop notes
                         * make flow.c weight et's defining reference by loop depth 3 instead of 2
                         * (reg_n_refs 8 -> 9), so global.c allocno_compare orders et (priority 2177)
                         * ahead of ep (2147): et takes $s5 and ep $s6, as in the target. Unwrapped,
                         * ep is allocated first and the two swap. Measurements and the plain
                         * spellings tried: memory/grind/func_80058580/evidence.md [s5]. */
                        do {
                            et = e[0] & 7;
                        } while (0);
                        work3 = 0;
                        if (et == 0) {
                            if (work1 < D_800A387C && D_800A387C < hi) {
                                if (p->unk_00->unk_6A == 0x15 || p->unk_00->unk_6A == 0x2C ||
                                    p->unk_00->unk_6A == 0xE || p->unk_00->unk_6A == 0x19) {
                                    work3 = 1;
                                }
                            }
                        } else {
                            if ((D_80099D88[p->unk_443].flags & 0xFC00) || p->unk_0E >= 6) {
                                work1 = et < 5 ? 100000 : 0;
                                hi = 100000;
                            } else {
                                work5 = (((0x1000 - lv) * 625) >> 10) - 400;
                                work1 += work5 + p->unk_40A;
                                hi += work5 + p->unk_40A;
                            }
                            work5 = et;
                            if (work5 < 5) {
                                if (D_800A387C < work1 && !(p->unk_430 & 0x20000)) {
                                    if ((p->unk_430 & 0x200) ? p->unk_43C < 0x800 : p->unk_43C < 0x400) {
                                        work3 = 1;
                                    } else if (p->unk_0E >= 6) {
                                        work3 = 1;
                                    }
                                }
                            } else if (work1 < D_800A387C && D_800A387C < hi &&
                                       p->unk_43C < 0x200 - ((p->unk_438 * 0x100) >> 12) &&
                                       /* Q76 (rules 30a3e2d2d): unk_438 / 16 as the 4.12 multiply by 0x100 (1/16); `>> 4`
                                        * lets cse.c fold_rtx merge the shift into the halfword sign extension
                                        * (lhu; sll 16; sra 20), the target has lh; sra 4 at 0x8005AB34. */
                                       (p->unk_430 & 0x280) != 0x280) {
                                switch (work5) {
                                case 5:
                                    if ((0x78 >> p->unk_B1) & 1) {
                                        work3 = 1;
                                    }
                                    break;
                                case 6:
                                    if (p->unk_443 == 0x15 || p->unk_330 != 0) {
                                        work3 = 1;
                                    }
                                    break;
                                default:
                                    work3 = 1;
                                    break;
                                }
                            }
                        }
                        if (work3) {
                            /* FAKE: named intermediates, as rnd / row above (script_weight row). */
                            s32 rnd2;
                            u8 *row2;

                            rnd2 = rand() & 0xFFF;
                            row2 = D_80099D88[p->unk_443].script_weight;
                            sc = (rnd2 * row2[et]) >> 12;
                            if (sc != 0 && pbest < sc) {
                                pbest = sc;
                                pbesti = work4;
                                pscript = ep;
                                phi = hi;
                            }
                        }
                    next:
                        list++;
                        off = *list;
                        work4++;
                    }
                    if (pbest != -1) {
                        func_80055B44(p, pscript, 0, 0);
                        p->unk_40C = pbesti;
                        p->unk_40D = p->unk_86;
                        p->unk_40E = p->unk_F4.x;
                        p->unk_410 = p->unk_F4.z;
                        p->unk_3F2++;
                        p->unk_412 = phi;
                    }
                }
            } else if (state == 0x15 && p->unk_26C != 0 && p->unk_440 != 4 &&
                       /* Q76 (rules 30a3e2d2d): as above, 4.12 factor 0x100; target lh; sra 4 at 0x8005ACCC. */
                       p->unk_43C < 0x200 - ((p->unk_438 * 0x100) >> 12) &&
                       !(D_800A38DC == 2 || D_800A38DC == 3)) {
                work3 = 0;
                if ((rand() & 0xFF) < (D_80099D88[p->unk_443].script_weight[5] >> 2) && ((0x78 >> p->unk_B1) & 1) && p->unk_442 == 0 &&
                    p->unk_00->unk_404[p->unk_00->unk_86] < D_800A387C && D_800A387C < 4500) {
                    work3 = (s32)D_8009A8C0;
                } else {
                    if (p->unk_443 == 0x15) {
                        work2 = p->unk_34D;
                        near = p->unk_00->unk_3F8[p->unk_00->unk_86];
                    } else {
                        near = p->unk_00->unk_404[p->unk_00->unk_86] + 300;
                        if (D_80099D88[p->unk_443].flags & 0x300) {
                            work2 = 0;
                            if (D_800A37A0 >= 6) {
                                work2 = p->unk_34A;
                            }
                        } else {
                            work2 = p->unk_330;
                        }
                    }
                    ok4 = 0;
                    if ((rand() & 0xFF) < (D_80099D88[p->unk_443].script_weight[6] >> 2) && work2 != 0 && near < D_800A387C &&
                        p->unk_434 == 100000 && p->unk_442 == 0 && (p->unk_430 & 0xA002) &&
                        (p->unk_443 != 0x15 || D_800A387C < 3000) && (p->unk_8A == 0 || work2 >= 2)) {
                        ok4 = 1;
                    }
                    if (ok4) {
                        if ((D_80099D88[p->unk_443].flags & 0x10) && work2 >= 2 && (rand() & 1)) {
                            work3 = (s32)D_8009A8B4;
                        } else {
                            work3 = (s32)D_8009A8AC;
                        }
                    }
                }
                if (work3 != 0) {
                    func_80055B44(p, (u8 *)work3, 2, 0);
                }
            }
        }
    }

    if (p->unk_3CC != 0) {
        return p->unk_3CC;
    }
    if (p->unk_6A == 0x15) {
        if (p->unk_43C > 0x100 && !(p->unk_6C == 0x19 || p->unk_6C == 0x1A) && p->unk_0E < 7 &&
            p->unk_443 != 0x16) {
            p->unk_39C = 0;
            p->unk_398 = 0x17000 / p->unk_1C;
            if (p->unk_444[5] == 0) {
                p->unk_394 = 3;
            } else if (p->unk_444[1] == 0) {
                p->unk_394 = 2;
            } else if (p->unk_444[0] == 0) {
                p->unk_394 = 1;
            } else {
                p->unk_394 = 0;
            }
        } else if (p->unk_0E >= 6) {
            if (p->unk_34A == 0 && p->unk_34B != 0 && p->unk_26C != 0 && p->unk_00->unk_404[p->unk_00->unk_86] < D_800A387C) {
                p->unk_3CC = 0x80;
            }
        } else if ((p->unk_430 & 0x800) && (D_80099D88[p->unk_443].flags & 1) && D_800A387C < 4000 && p->unk_440 != 4 &&
                   (p->unk_442 == 0 || p->unk_442 == 2)) {
            func_80055B44(p, D_8009A898, 1, p->unk_3BD);
            p->unk_3F2++;
        } else if ((p->unk_430 & 0xA801) || p->unk_00->unk_6A == 0x18 ||
                   p->unk_00->unk_6A == 0x25 || p->unk_00->unk_6A == 8 ||
                   p->unk_00->unk_6A == 0xA || (p->unk_00->unk_6A == 0x1A && p->unk_441 == 1)) {
            if (D_80099D88[p->unk_443].unk3 != 0 &&
                ((!(D_80099D88[p->unk_443].flags & 0xFF00) && p->unk_00->unk_404[p->unk_00->unk_86] < D_800A387C && p->unk_3F4 >= D_80099D88[p->unk_443].unk3) ||
                 ((D_80099D88[p->unk_443].flags & 0x100) && p->unk_00->unk_3F8[p->unk_00->unk_86] < D_800A387C && p->unk_3F4 >= D_80099D88[p->unk_443].unk3 &&
                  p->unk_440 != 2) ||
                 ((D_80099D88[p->unk_443].flags & 0x7C00) && p->unk_00->unk_3F8[p->unk_00->unk_86] < D_800A387C && p->unk_3F4 >= D_80099D88[p->unk_443].unk3 &&
                  D_800A38E2 >= 0x5B) ||
                 (p->unk_430 & 0x40000))) {
                p->unk_3F4 = 0;
                p->unk_3CC = 0x80;
                p->unk_430 &= ~0x40000;
            }
        }
    }
    return p->unk_3CC;
}


#undef CPU_SQ
