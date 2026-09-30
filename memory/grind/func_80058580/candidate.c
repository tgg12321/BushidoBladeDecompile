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
extern u8 D_8009A850[][4];
extern u8 D_8009A658[][12];
extern u16 D_8009A928[][23];
extern u8 D_8009A9DC[][3];
extern s32 D_8009A9F0[][8];
extern u16 D_8009A8CA[][8][2];
extern u8 D_800A325C[4];
extern u8 D_800A3260[4];
extern void func_80057E84(u8 *, u8 *, s32, s32);

#define CPU_OPP (*(u8 **)p)
#define CPU_S16(o) (*(s16 *)(p + (o)))
#define CPU_U16(o) (*(u16 *)(p + (o)))
#define CPU_S32(o) (*(s32 *)(p + (o)))
#define CPU_FLAGS (D_80099D88[p[0x443]].flags)
#define CPU_OARR(o) (*(s16 *)(CPU_OPP + *(s16 *)(CPU_OPP + 0x86) * 2 + (o)))
#define CPU_SARR(o) (*(s16 *)(p + *(s16 *)(p + 0x86) * 2 + (o)))
#define CPU_WP(i) (p + (i) * 6)
#define CPU_SQ(x) ((x) * (x))

s32 func_80058580(u8 *p) {
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
    s32 work1, work2, work3, work4;
    s32 hi;
    s32 slot;
    s16 et;
    s32 va;
    s32 vd;
    s32 vb;
    s32 vn;
    s32 vc;
    s32 adj;
    s16 sc;
    u8 *pois;
    s32 tx, ty2;
    s32 r;
    s32 sel;
    u16 *list;
    u8 *e;
    u8 *ep;
    u8 *q;
    u16 off;
    s16 pbest;
    u16 st;
    u16 st2;
    s8 flip;
    s32 near;
    s32 wtype;
    s32 cnt;
    s32 ok4;
    s32 far;
    s32 lim;
    s32 besti;
    s32 score;

    if (*(u16 *)(CPU_OPP + 0x6A) == 4 || *(u16 *)(CPU_OPP + 0x6A) == 0x14) {
        return 0;
    }
    if (p[0x443] != 0x16 && ((CPU_S32(0x430) & 0x15100) ||
                             (CPU_U16(0x6A) == 0xD && (p[0x426] == 4 || p[0x425] == 4)))) {
        script1 = 0;
        if ((p[0x426] == 1 && *(s16 *)(CPU_OPP + 0x40) + 1 >= p[0x427]) || p[0x425] == 2) {
            if (CPU_S32(0x430) & 8) {
                u8 *tbl[2];
                s32 f;
                tbl[0] = D_8009A874;
                tbl[1] = D_8009A870;
                work3 = CPU_OPP[0xAF] & 1;
                f = CPU_S32(0x430);
                if (!(((f & 0x20) || ((f & 0x10) && p[0x3F3] % ((CPU_S16(0x438) >> 8) + 2) != (CPU_S16(0x438) >> 8) + 1)) &&
                      (!(CPU_FLAGS & 0xFF00) || (f & 0x40))) ||
                    (file_GetFlag1() && D_800A38DC != 3)) {
                    work3 = !work3;
                }
                script1 = tbl[work3];
                mode = 3;
            }
            if (CPU_U16(0x6A) == 0xD) {
                script1 = 0;
                CPU_S32(0x3CC) = 0;
                p[0x426] = 4;
                p[0x425] = 4;
                p[0x3F3]++;
            }
        } else if (CPU_U16(0x6A) == 0x15) {
            mode = 4;
            work3 = p[0x447] == 0;
            work1 = p[0x449] == 0;
            work2 = p[0x445] == 0;
            if (p[0x426] == 2) {
                if (CPU_S16(0x42E) * CPU_S16(0x42E) <
                    CPU_SQ(CPU_S16(0x42A) - CPU_S32(0xF4)) + CPU_SQ(CPU_S16(0x42C) - CPU_S32(0xFC))) {
                    CPU_S32(0x3CC) = 0;
                    p[0x426] = 4;
                    p[0x425] = 4;
                    p[0x3F3]++;
                } else if (work3 && (CPU_S32(0x430) & 8) &&
                           (!(CPU_FLAGS & 0x8F00) || ((CPU_FLAGS & 0x300) && D_800A37A0 >= 6))) {
                    script1 = D_8009A890;
                }
            }
            if (p[0x425] == 1 ||
                (p[0x426] == 1 && CPU_OPP[0x441] == 2 && p[0x427] - *(s16 *)(CPU_OPP + 0x40) >= 6) ||
                (p[0x426] == 2 && script1 == 0)) {
                if ((CPU_S32(0x430) & 8) &&
                    (!(CPU_FLAGS & 0xBF00) || ((CPU_FLAGS & 0x300) && D_800A37A0 >= 7))) {
                    if (*(s16 *)(CPU_OPP + 0x43A) > 0) {
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
                        p[0x426] = 1;
                        p[0x425] = 2;
                        return -1;
                    }
                    p[0x426] = 4;
                    p[0x425] = 4;
                    p[0x3F3]++;
                }
            }
        }
        if (script1 != 0) {
            func_80055B44(p, (s32)script1, mode & 0xFF, 0);
            return CPU_S32(0x3CC);
        }
    }

    if ((CPU_S32(0x430) & 0x400) && p[0x441] != 2) {
        if (CPU_FLAGS & 0x8000) {
            CPU_S32(0x3CC) = 0x2000;
        } else if (CPU_S32(0x430) & 8) {
            work3 = CPU_S16(0x43A);
            if (p[0x441] == 0) {
                work3 = (work3 + 0x800) & 0xFFF;
                if (work3 > 0x800) {
                    work3 -= 0x1000;
                }
            }
            if (!(CPU_FLAGS & 0xF00)) {
                if (work3 < 0) {
                    if (p[0x445] == 0 || p[0x445] == 3) {
                        CPU_S32(0x3CC) = 0x4000;
                    }
                } else {
                    if (p[0x449] == 0 || p[0x449] == 3) {
                        CPU_S32(0x3CC) = 0x1000;
                    }
                }
            }
            if (!(p[0x425] == 1 || p[0x425] == 2) && p[0x426] != 2 &&
                (CPU_S32(0x3CC) == 0 || p[0x442] != 0 || *(u16 *)(CPU_OPP + 0x6A) == 2 ||
                 *(u16 *)(CPU_OPP + 0x6A) == 0x29 || *(u16 *)(CPU_OPP + 0x6A) == 0x13 ||
                 *(u16 *)(CPU_OPP + 0x6A) == 6 || CPU_OARR(0x404) < D_800A387C)) {
                if (CPU_S16(0x43C) < 0x400 && p[0x447] == 0) {
                    CPU_S32(0x3CC) = 0x8000;
                } else {
                    CPU_S32(0x3CC) = 0x2000;
                }
            }
        }
    } else {
        st = CPU_U16(0x6A);
        if (st == 0xF || st == 0x1C || st == 0x1D || st == 0x1E || st == 0x1F || st == 0x20 || st == 0x21) {
            if (CPU_U16(0x6A) == 0x1D && (rand() & 0xFF) < D_80099D88[p[0x443]].unk4 && p[0x447] == 0) {
                CPU_S32(0x3CC) = 0x8000;
            } else {
                vd = 0x80;
                if (CPU_U16(0x3E8) % (0x12 - (CPU_S16(0x438) >> 8)) == 0) {
                    vd = 0x20;
                }
                CPU_S32(0x3CC) = vd;
                if (p[0x447] != 0) {
                    if (p[0x446] != 0) {
                        CPU_S32(0x3CC) = vd | 0x1000;
                    } else if (p[0x448] != 0) {
                        CPU_S32(0x3CC) = vd | 0x4000;
                    }
                } else if (p[0x44B] == 0) {
                    CPU_S32(0x3CC) = (p[0x443] & 1) ? vd | 0x1000 : vd | 0x4000;
                }
            }
        } else if (st == 0x11) {
            if (CPU_S16(4) != D_800A38AE && CPU_S16(0x40) == (*(u8 **)(p + 0x50))[8] - 1) {
                s32 b, c, a;
                a = CPU_S16(0x26E);
                b = CPU_S16(0x270);
                c = CPU_S16(0x272);
                work3 = 0x1000 - (((CPU_S16(0x26C) == 0 ? a + 4 + b : a + b) + c) << 8);
                work3 = (CPU_S16(0x438) * work3) >> 12;
                if ((rand() & 0xFFF) < work3) {
                    CPU_S32(0x3CC) = 0x20;
                } else if (CPU_S32(0x430) & 0x20) {
                    CPU_S32(0x3CC) = 0x20;
                }
            }
        } else {
            if ((CPU_S32(0x148) - CPU_S32(0xBC) >= 0 ? CPU_S32(0x148) - CPU_S32(0xBC)
                                                     : CPU_S32(0xBC) - CPU_S32(0x148)) < 200 && !(CPU_FLAGS & 0x8C00) &&
                ((!file_GetFlag1() && D_800A38DC != 3) || D_800A38DC == 3)) {
                work3 = 0;
                if ((p[0x426] == 1 || p[0x425] == 2) && (CPU_S32(0x430) & 8)) {
                    work3 = 1;
                }
                far = 0;
                if ((rand() & 0xFFF) < (CPU_S16(0x438) >> 3)) {
                    far = CPU_U16(0x3E8) > 0x3C;
                }
                if (CPU_S16(0xE) >= 6) {
                    work1 = 100000;
                } else {
                    work1 = (&D_8009A838)[CPU_S16(0xE)] * 8;
                }
                for (work4 = 0; work4 < 8U; work4++) {
                    if (!(D_8009A850[work4][3] & 1) || CPU_S16(0x40) >= (*(u8 **)(p + 0x50))[8] - 2 || work3) {
                        if ((D_800A387C < (work2 = D_8009A850[work4][2] * 16 + work1 + CPU_S16(0x40A)) &&
                             (!(D_8009A850[work4][3] & 8) || CPU_S16(0x43C) < 0x100) &&
                             (far || (D_8009A850[work4][3] & 4))) ||
                            work3) {
                            if (D_8009A850[work4][0] == CPU_U16(0x6A) &&
                                ((u8)(st2 = D_8009A850[work4][1]) == 0xFF || st2 == CPU_OPP[0x6A])) {
                                if (D_8009A850[work4][3] & 2) {
                                    vb = 0x40;
                                    if (CPU_U16(0x3E8) & 1) {
                                        vb = 0x20;
                                    }
                                    CPU_S32(0x3CC) = vb;
                                } else if ((CPU_FLAGS & 0x20) && !(p[0x440] == 3 || p[0x440] == 4)) {
                                    CPU_S32(0x3CC) = (CPU_U16(0x3E8) & 1) * 8;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (CPU_S32(0x3CC) != 0) {
        return CPU_S32(0x3CC);
    }
    if ((CPU_S32(0x430) & 0x4108) == 8) {
        st = CPU_U16(0x6A);
        if (st == 3 || st == 0x2C || st == 7 || (p[0x426] == 1 || p[0x426] == 2) ||
            (p[0x425] == 1 || p[0x425] == 2)) {
            p[0x39D] = 0;
            p[0x362] = 0;
            CPU_S16(0x398) = 0;
            return 0;
        }
    }

    if (CPU_S32(0x430) & 0x5100) {
        pois = D_8009A658[D_800A36A4];
        if (p[0x39D] == 0) {
            tx = *(s32 *)(CPU_OPP + 0xF4);
            ty2 = *(s32 *)(CPU_OPP + 0xFC);
            wtype = 1;
        } else {
            tx = CPU_S16(0x3A0);
            ty2 = CPU_S16(0x3A2);
            wtype = CPU_S16(0x39E);
        }
        if (!(wtype == 1 &&
              (*(u16 *)(CPU_OPP + 0x6A) == 0xA || p[0x443] == 0xA || (CPU_S16(0xE) >= 6 && p[0x34A] == 0)) &&
              wtype == 1)) {
            if (!(CPU_U16(0x3E8) & 7)) {
                CPU_S32(0x434) = func_80057ACC((s32)p, pois, tx, ty2);
            }
            lim = 2000;
            if (wtype == 1) {
                work1 = p[0x44A];
                if (!(CPU_S32(0x430) & 0x800) || D_800A387C < 4000) {
                    if (p[0x442] == 2 && !(work1 == 1 || work1 == 2)) {
                        lim = CPU_OARR(0x3F8);
                    } else if ((p[0x442] == 1 || p[0x442] == 2) || p[0x442] == 3) {
                        if (!(CPU_FLAGS & 0x4000)) {
                            lim = CPU_OARR(0x3FE);
                        } else {
                            lim = CPU_OARR(0x404);
                        }
                    } else if (CPU_S32(0x434) != 100000) {
                        if (CPU_S32(0x430) & 0x200) {
                            lim = CPU_OARR(0x404);
                        } else {
                            lim = CPU_SARR(0x404) + CPU_OARR(0x404);
                        }
                    } else {
                        s16 ob;
                        lim = CPU_SARR(0x3FE) + (ob = CPU_OARR(0x3FE));
                        if (CPU_S32(0x430) & 0x200) {
                            if (*(s16 *)(CPU_OPP + 0x43C) > 0x600) {
                                lim = CPU_OARR(0x3F8);
                            } else if (*(s16 *)(CPU_OPP + 0x43C) > 0x300) {
                                lim = CPU_OARR(0x404);
                            } else if (*(s16 *)(CPU_OPP + 0x43C) > 0x100) {
                                lim = ob;
                            }
                        }
                    }
                }
            }
            if (wtype == 1) {
                if (p[0x362] < 2 && (CPU_S32(0x434) != 100000 || D_800A387C >= lim)) {
                    goto record;
                }
            } else if (p[0x362] == 0) {
                if (CPU_S32(0x434) != 100000 || CPU_SQ(CPU_S32(0xF4) - tx) + CPU_SQ(CPU_S32(0xFC) - ty2) > 0x15F8F) {
                record:
                    CPU_S16(0x364) = tx;
                    CPU_S16(0x366) = ty2;
                    p[0x368] = wtype;
                    p[0x362] = 1;
                    if (CPU_S32(0x434) != 100000 && !(CPU_U16(0x3E8) & 7)) {
                        func_80057E84(p, pois, tx, ty2);
                    }
                }
            }
            if (p[0x362] != 0) {
                if (p[0x368] == 1 ? (CPU_S32(0x434) == 100000 && D_800A387C < lim)
                                  : SquareRoot0(CPU_SQ(CPU_S32(0xF4) - tx) + CPU_SQ(CPU_S32(0xFC) - ty2)) < 2000) {
                    p[0x362] = 0;
                    p[0x39D] = 0;
                    goto after_nav;
                }
                {
                    u8 *wp;
                    u8 *wp2;
                    u8 *wp3;
                    work1 = p[0x44A];
                    work3 = p[0x362] - 1;
                    work2 = p[0x444];
                    wp = CPU_WP(work3);
                    wx = *(s16 *)(wp + 0x364);
                    wy = *(s16 *)(wp + 0x366);
                    if (!(work1 == 1 || work1 == 2)) {
                        script2 = 0;
                        if (wp[0x368] == 1) {
                            if ((work2 == 3 && D_800A387C < CPU_OARR(0x404)) ||
                                (work2 == 5 && (CPU_OARR(0x404) < D_800A387C && D_800A387C < CPU_OARR(0x404)))) {
                                if (CPU_U16(0x6A) == 0x13) {
                                    script2 = D_8009A8A4;
                                } else if (p[0x440] != 4) {
                                    script2 = D_8009A89C;
                                }
                            }
                            if (script2 != 0) {
                                func_80055B44(p, (s32)script2, 4, 0);
                            }
                        }
                    }
                    if (CPU_S32(0x3CC) != 0) {
                        return CPU_S32(0x3CC);
                    }
                    work4 = work3;
                    if (work3 == 0) {
                        if (p[0x368] == 1) {
                            work3 = SquareRoot0(CPU_SQ(CPU_S32(0xF4) - tx) + CPU_SQ(CPU_S32(0xFC) - ty2));
                        } else {
                            work3 = SquareRoot0(CPU_SQ(CPU_S32(0xF4) - wx) + CPU_SQ(CPU_S32(0xFC) - wy));
                        }
                    } else {
                        work3 = SquareRoot0(CPU_SQ(wx - CPU_S32(0xF4)) + CPU_SQ(wy - CPU_S32(0xFC)));
                        while (work4 >= 2) {
                            u8 *a;
                            u8 *b;
                            a = CPU_WP(work4);
                            b = CPU_WP(work4 - 1);
                            work4 = work4 - 1;
                            work3 += SquareRoot0(CPU_SQ(*(s16 *)(a + 0x364) - *(s16 *)(b + 0x364)) +
                                                CPU_SQ(*(s16 *)(a + 0x366) - *(s16 *)(b + 0x366)));
                        }
                        {
                            work1 = CPU_S16(0x36A);
                            work2 = CPU_S16(0x36C);
                            if (p[0x368] == 1) {
                                work3 += SquareRoot0(CPU_SQ(work1 - tx) + CPU_SQ(work2 - ty2));
                            } else {
                                work3 += SquareRoot0(CPU_SQ(work1 - CPU_S16(0x364)) + CPU_SQ(work2 - CPU_S16(0x366)));
                            }
                        }
                    }
                    work3 = lim < work3;
                    CPU_S32(0x3CC) = func_80057094(p, wx, wy, work3);
                    va = 300;
                    vn = CPU_S32(0x3CC) & 4;
                    if (vn) {
                        va = 1000;
                    }
                    wp2 = CPU_WP(p[0x362] - 1);
                    if (CPU_SQ(*(s16 *)(wp2 + 0x364) - CPU_S32(0xF4)) + CPU_SQ(*(s16 *)(wp2 + 0x366) - CPU_S32(0xFC)) <
                        va * (vn ? 1000 : 300)) {
                        if ((u8)--p[0x362] != 0) {
                            wp3 = CPU_WP(p[0x362] - 1);
                            work3 = (ratan2(*(s16 *)(wp3 + 0x364) - CPU_S32(0xF4), *(s16 *)(wp3 + 0x366) - CPU_S32(0xFC)) -
                                   CPU_S16(0x1CA)) & 0xFFF;
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
            if (CPU_S32(0x3CC) != 0) {
                return CPU_S32(0x3CC);
            }
        }

        if ((!(CPU_S32(0x430) & 0x80) && D_800A387C < CPU_OARR(0x404) &&
             ((p[0x447] >= 2 && (p[0x449] >= 2 || p[0x445] >= 2)) || p[0x447] == 1 || p[0x448] == 1 ||
              p[0x446] == 1)) ||
            ((CPU_FLAGS & 0x4000 || p[0x443] == 0x18 || p[0x443] == 0x1A) && D_800A387C < CPU_OARR(0x404) &&
             *(s16 *)(CPU_OPP + 0x43C) < 0x10) ||
            (CPU_S16(0xE) >= 7 && D_800A387C < CPU_OARR(0x404) && *(s16 *)(CPU_OPP + 0x43C) < 0x10 &&
             p[0x34A] != 0)) {
            if (!(CPU_FLAGS & 0x8F00)) {
                if ((p[0x362] = func_800571C0((s32)p)) != 0) {
                    p[0x39D] = 2;
                    return -1;
                }
            }
            if (!(CPU_FLAGS & 0x8C00)) {
                cnt = 0;
                work3 = D_80099D88[p[0x443]].pick_weight[4] < (rand() & 0xFF);
                sel = -1;
                work1 = p[0x445] == 0;
                work2 = p[0x449] == 0;
                for (; cnt < 2; cnt++, work3++) {
                    if (work3 & 1) {
                        if (*(s16 *)(CPU_OPP + 0x43A) < 0) {
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
                    } else if (p[0x440] != 4 && D_800A387C > 3000 && D_800A387C < 5000) {
                        sel = 8;
                        break;
                    }
                }
                if (sel != -1) {
                    CPU_S32(0x394) = sel;
                    p[0x39C] = 0;
                    CPU_S16(0x398) = CPU_U16(0x39A);
                }
            }
        }

        if (CPU_S16(0x398) >= CPU_S16(0x39A)) {
            script3 = 0;
            if (p[0x39C] != 1) {
                work3 = CPU_S32(0x394);
            } else if (*(u16 *)(CPU_OPP + 0x6A) == 0x19) {
                u8 buf[4];
                __builtin_memcpy(buf, D_800A325C, 4);
                work3 = buf[CPU_OPP[0x441]];
            } else if (*(u16 *)(CPU_OPP + 0x6A) == 0x1A) {
                u8 buf2[4];
                __builtin_memcpy(buf2, D_800A3260, 4);
                work3 = buf2[CPU_OPP[0x441]];
            }
            switch (work3) {
            case 0:
                if (p[0x447] == 0) {
                    CPU_S32(0x3CC) = 0x8000;
                }
                break;
            case 1:
                if (p[0x444] == 0) {
                    CPU_S32(0x3CC) = 0x2000;
                }
                break;
            case 2:
                if (p[0x445] == 0) {
                    CPU_S32(0x3CC) = 0x4000;
                }
                break;
            case 3:
                if (p[0x449] == 0) {
                    CPU_S32(0x3CC) = 0x1000;
                }
                break;
            case 4:
                if (p[0x447] == 0 && D_800A387C < CPU_SARR(0x3FE)) {
                    script3 = D_8009A890;
                }
                break;
            case 5:
                if (p[0x444] == 0 && CPU_SARR(0x3F8) < D_800A387C) {
                    script3 = D_8009A888;
                }
                break;
            case 6:
                if (p[0x445] == 0 && D_800A387C < CPU_SARR(0x3FE)) {
                    script3 = D_8009A878;
                }
                break;
            case 7:
                if (p[0x449] == 0 && D_800A387C < CPU_SARR(0x3FE)) {
                    script3 = D_8009A880;
                }
                break;
            case 8:
                if (!(p[0x44A] == 1 || p[0x44A] == 2) && D_800A387C > 3000 && D_800A387C < 5000 &&
                    CPU_S16(0x43C) < 0x200) {
                    script3 = D_8009A89C;
                }
                break;
            }
            if ((p[0x426] == 1 || p[0x426] == 2) || (p[0x425] == 1 || p[0x425] == 2)) {
                if (CPU_S32(0x3CC) != 0) {
                    CPU_S16(0x398) = CPU_U16(0x39A) + 1;
                } else {
                    CPU_S16(0x398) = 0;
                    script3 = 0;
                }
            } else if ((CPU_S32(0x430) & 0x800) || p[0x442] != 0 ||
                       (CPU_S32(0x3CC) == 0x2000 && D_800A387C < CPU_OARR(0x3F8))) {
                script3 = 0;
                CPU_S16(0x398) = 0;
                CPU_S32(0x3CC) = 0;
            }
            if (script3 != 0) {
                CPU_S16(0x398) = 0;
                if (CPU_S32(0x430) & 0xA000) {
                    func_80055B44(p, (s32)script3, 4, 0);
                }
            }
        } else if (CPU_S16(0x398) == 0) {
            s32 tired = D_80099D88[p[0x443]].unk6;
            r = rand() & 0xFF;
            if (p[0x440] == 4 ? r < (tired >> 2) : r < tired) {
                work2 = -1;
                besti = -1;
                work4 = 0;
            pick_loop:
                {
                    s32 rnd;
                    u8 *row;
                    rnd = rand() & 0xFFF;
                    row = D_80099D88[p[0x443]].pick_weight;
                    score = (rnd * row[work4]) >> 12;
                    if (score != 0) {
                        flip = 0;
                        switch (work4) {
                        case 0:
                        case 2:
                            if (rand() & 1) {
                                flip = 1;
                            } else if (p[0x447] != 0) {
                                goto pick_next;
                            }
                            vc = CPU_S16(0x3F0);
                            if (D_80099D88[p[0x443]].unk7 < (vc >= 0 ? vc : -vc)) {
                                score += 0x80;
                                flip = vc > 0;
                            }
                            if (CPU_S16(0xE) >= 6 && p[0x34A] == 0 && work4 == 2) {
                                score += 0x100;
                                flip = 0;
                            }
                            break;
                        case 1:
                        case 3:
                            if (rand() & 1) {
                                if (p[0x449] != 0) {
                                    goto pick_next;
                                }
                                flip = 1;
                            } else if (p[0x445] != 0) {
                                goto pick_next;
                            }
                            if (CPU_S32(0x430) & 0x20000) {
                                score += 0x80;
                                flip = CPU_S16(0x43A) > 0;
                                if (*(flip ? p + 0x449 : p + 0x445) != 0) {
                                    flip ^= 1;
                                }
                            }
                            break;
                        case 4:
                            if ((p[0x44A] == 1 || p[0x44A] == 2) || !(D_800A387C >= 3000 && D_800A387C <= 5000) ||
                                CPU_S16(0x43C) >= 0x201 || p[0x440] == 4 || *(u16 *)(CPU_OPP + 0x6A) == 0x18 ||
                                *(u16 *)(CPU_OPP + 0x6A) == 0x2A) {
                                goto pick_next;
                            }
                            break;
                        }
                        if ((s16)work2 < score) {
                            besti = work4;
                            work2 = score;
                            bestflip = flip;
                        }
                    }
                }
            pick_next:
                if (++work4 < 7) {
                    goto pick_loop;
                }
                if ((work1 = (s8)besti) != -1) {
                    CPU_S32(0x394) = 0;
                    p[0x39C] = 0;
                    CPU_S16(0x398) = (((((rand() & 0xFFF) * D_80099D88[p[0x443]].unk6) >> 12) + 0x17) << 12) / CPU_S16(0x1C);
                    switch (work1) {
                    case 0:
                        CPU_S32(0x394) = bestflip != 0;
                        break;
                    case 1:
                        if (bestflip) {
                            CPU_S32(0x394) = 3;
                        } else {
                            CPU_S32(0x394) = 2;
                        }
                        break;
                    case 2:
                        if (bestflip) {
                            CPU_S32(0x394) = 5;
                        } else {
                            CPU_S32(0x394) = 4;
                        }
                        break;
                    case 3:
                        if (bestflip) {
                            CPU_S32(0x394) = 7;
                        } else {
                            CPU_S32(0x394) = 6;
                        }
                        break;
                    case 4:
                        CPU_S32(0x394) = 8;
                        break;
                    case 5:
                        p[0x39C] = 1;
                        break;
                    }
                }
            }
        }
        if (CPU_S16(0x398) != 0) {
            CPU_S16(0x398)--;
        }
    }

    if (CPU_S32(0x3CC) != 0 || CPU_S16(0x398) != 0) {
        return CPU_S32(0x3CC);
    }
    if ((CPU_S32(0x430) & 6) && !(CPU_S32(0x430) & 0x800)) {
        st = CPU_U16(0x6A);
        if (st == 0x15 || (st == 0x19 && p[0x441] >= 2)) {
            if ((CPU_U16(0x3E8) & 1) || CPU_S16(0xE) >= 6) {
                if (p[0x442] == 0 && p[0x447] != 1 && (CPU_S16(0xE) < 6 || p[0x34A] != 0)) {
                    pbest = -1;
                    if (p[0x3F2] % ((CPU_S16(0x438) >> 8) + 2) == (CPU_S16(0x438) >> 8) + 1) {
                        lv = CPU_S16(0x438) >> 1;
                    } else {
                        lv = CPU_S16(0x438);
                    }
                    list = *(u16 **)(p + *(s16 *)(p + 0x86) * 4 + 0x3A8);
                    off = *list;
                    work4 = 0;
                    while (off != 0) {
                        ep = off + *(u8 **)(p + 0x3A4);
                        e = ep;
                        ep += 4;
                        q = e + 4;
                        if (CPU_FLAGS & 0xFF00) {
                            switch (D_800A38DC) {
                            case 3:
                                if (D_800A38E2 < 0x5B) {
                                    slot = (u8)(D_800A38E2 / 10) * 2;
                                    if ((u8)(D_800A38E2 % 10) == 0) {
                                        slot--;
                                    }
                                } else if (D_800A38E2 < 0x5E) {
                                    slot = 0x12;
                                } else if (D_800A38E2 < 0x60) {
                                    slot = 0x13;
                                } else if (D_800A38E2 < 0x62) {
                                    slot = 0x14;
                                } else if (D_800A38E2 < 0x64) {
                                    slot = 0x15;
                                } else {
                                    slot = 0x16;
                                }
                                work3 = D_8009A928[p[0x440]][slot];
                                break;
                            case 2:
                                work1 = D_8009A9F0[D_8009A9DC[CPU_S16(0xE)][p[0x440]]][D_800A3788];
                                work3 = 0;
                                work2 = work1 & 0xF;
                                if (work2 != 0) {
                                    if (work1 >> 27) {
                                        while (work2 > 0) {
                                            work1 >>= 4;
                                            work3 |= 1 << ((work1 & 0xF) - 1);
                                            work2--;
                                        }
                                    } else {
                                        work1 >>= ((u8)(p[0x3F2] / 3) % work2) * 4 + 4;
                                        work3 = 1 << ((work1 & 0xF) - 1);
                                    }
                                }
                                break;
                            default:
                                work3 = D_8009A8CA[p[0x440]][D_800A37A0 - 1][0];
                                break;
                            }
                            if ((e[3] >> 4) == 0 || !((u32)work3 & (1 << ((e[3] >> 4) - 1)))) {
                                goto next;
                            }
                        }
                        if (!(CPU_FLAGS & 0x80) && *(s8 *)(p + 0x40D) == CPU_S16(0x86) && *(s8 *)(p + 0x40C) == work4) {
                            goto next;
                        }
                        if (q[0] == 0x40) {
                            work3 = q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1];
                            if (!((u32)work3 & (1 << p[0x443]))) {
                                goto next;
                            }
                            ep += 5;
                        }
                        if ((e[0] & 0x80) && CPU_S16(0x26C) == 0) {
                            goto next;
                        }
                        work1 = e[1] * 40;
                        hi = e[2] * 40;
                        et = e[0] & 7;
                        work3 = 0;
                        if (et == 0) {
                            if (work1 < D_800A387C && D_800A387C < hi) {
                                st = *(u16 *)(CPU_OPP + 0x6A);
                                if (st == 0x15 || st == 0x2C || st == 0xE || st == 0x19) {
                                    work3 = 1;
                                }
                            }
                        } else {
                            if ((CPU_FLAGS & 0xFC00) || CPU_S16(0xE) >= 6) {
                                work1 = -(et < 5) & 100000;
                                hi = 100000;
                            } else {
                                adj = (((0x1000 - lv) * 625) >> 10) - 400;
                                adj += CPU_S16(0x40A);
                                work1 += adj;
                                hi += adj;
                            }
                            if (et < 5) {
                                if (D_800A387C < work1 && !(CPU_S32(0x430) & 0x20000)) {
                                    if ((CPU_S32(0x430) & 0x200) ? CPU_S16(0x43C) < 0x800 : CPU_S16(0x43C) < 0x400) {
                                        work3 = 1;
                                    } else if (CPU_S16(0xE) >= 6) {
                                        work3 = 1;
                                    }
                                }
                            } else if (work1 < D_800A387C && D_800A387C < hi &&
                                       CPU_S16(0x43C) < 0x200 - (CPU_S16(0x438) >> 4) &&
                                       (CPU_S32(0x430) & 0x280) != 0x280) {
                                switch (et) {
                                case 5:
                                    if ((0x78 >> p[0xB1]) & 1) {
                                        work3 = 1;
                                    }
                                    break;
                                case 6:
                                    if (p[0x443] == 0x15 || CPU_S16(0x330) != 0) {
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
                            s32 rnd2;
                            u8 *row2;

                            rnd2 = rand() & 0xFFF;
                            row2 = D_80099D88[p[0x443]].script_weight;
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
                        func_80055B44(p, (s32)pscript, 0, 0);
                        p[0x40C] = pbesti;
                        p[0x40D] = CPU_S16(0x86);
                        CPU_S16(0x40E) = CPU_S32(0xF4);
                        CPU_S16(0x410) = CPU_S32(0xFC);
                        p[0x3F2]++;
                        CPU_S16(0x412) = phi;
                    }
                }
            } else if (st == 0x15 && CPU_S16(0x26C) != 0 && p[0x440] != 4 &&
                       CPU_S16(0x43C) < 0x200 - (CPU_S16(0x438) >> 4) &&
                       !(D_800A38DC == 2 || D_800A38DC == 3)) {
                work3 = 0;
                if ((rand() & 0xFF) < (D_80099D88[p[0x443]].script_weight[5] >> 2) && ((0x78 >> p[0xB1]) & 1) && p[0x442] == 0 &&
                    CPU_OARR(0x404) < D_800A387C && D_800A387C < 4500) {
                    work3 = (s32)D_8009A8C0;
                } else {
                    if (p[0x443] == 0x15) {
                        work2 = p[0x34D];
                        near = CPU_OARR(0x3F8);
                    } else {
                        near = CPU_OARR(0x404) + 300;
                        if (CPU_FLAGS & 0x300) {
                            work2 = 0;
                            if (D_800A37A0 >= 6) {
                                work2 = p[0x34A];
                            }
                        } else {
                            work2 = CPU_S16(0x330);
                        }
                    }
                    ok4 = 0;
                    if ((rand() & 0xFF) < (D_80099D88[p[0x443]].script_weight[6] >> 2) && work2 != 0 && near < D_800A387C &&
                        CPU_S32(0x434) == 100000 && p[0x442] == 0 && (CPU_S32(0x430) & 0xA002) &&
                        (p[0x443] != 0x15 || D_800A387C < 3000) && (CPU_S16(0x8A) == 0 || work2 >= 2)) {
                        ok4 = 1;
                    }
                    if (ok4) {
                        if ((CPU_FLAGS & 0x10) && work2 >= 2 && (rand() & 1)) {
                            work3 = (s32)D_8009A8B4;
                        } else {
                            work3 = (s32)D_8009A8AC;
                        }
                    }
                }
                if (work3 != 0) {
                    func_80055B44(p, work3, 2, 0);
                }
            }
        }
    }

    if (CPU_S32(0x3CC) != 0) {
        return CPU_S32(0x3CC);
    }
    if (CPU_U16(0x6A) == 0x15) {
        if (CPU_S16(0x43C) > 0x100 && !(CPU_U16(0x6C) == 0x19 || CPU_U16(0x6C) == 0x1A) && CPU_S16(0xE) < 7 &&
            p[0x443] != 0x16) {
            p[0x39C] = 0;
            CPU_S16(0x398) = 0x17000 / CPU_S16(0x1C);
            if (p[0x449] == 0) {
                CPU_S32(0x394) = 3;
            } else if (p[0x445] == 0) {
                CPU_S32(0x394) = 2;
            } else if (p[0x444] == 0) {
                CPU_S32(0x394) = 1;
            } else {
                CPU_S32(0x394) = 0;
            }
        } else if (CPU_S16(0xE) >= 6) {
            if (p[0x34A] == 0 && p[0x34B] != 0 && CPU_S16(0x26C) != 0 && CPU_OARR(0x404) < D_800A387C) {
                CPU_S32(0x3CC) = 0x80;
            }
        } else if ((CPU_S32(0x430) & 0x800) && (CPU_FLAGS & 1) && D_800A387C < 4000 && p[0x440] != 4 &&
                   (p[0x442] == 0 || p[0x442] == 2)) {
            func_80055B44(p, (s32)D_8009A898, 1, p[0x3BD]);
            p[0x3F2]++;
        } else if ((CPU_S32(0x430) & 0xA801) || *(u16 *)(CPU_OPP + 0x6A) == 0x18 ||
                   *(u16 *)(CPU_OPP + 0x6A) == 0x25 || *(u16 *)(CPU_OPP + 0x6A) == 8 ||
                   *(u16 *)(CPU_OPP + 0x6A) == 0xA || (*(u16 *)(CPU_OPP + 0x6A) == 0x1A && p[0x441] == 1)) {
            if (D_80099D88[p[0x443]].unk3 != 0 &&
                ((!(CPU_FLAGS & 0xFF00) && CPU_OARR(0x404) < D_800A387C && p[0x3F4] >= D_80099D88[p[0x443]].unk3) ||
                 ((CPU_FLAGS & 0x100) && CPU_OARR(0x3F8) < D_800A387C && p[0x3F4] >= D_80099D88[p[0x443]].unk3 &&
                  p[0x440] != 2) ||
                 ((CPU_FLAGS & 0x7C00) && CPU_OARR(0x3F8) < D_800A387C && p[0x3F4] >= D_80099D88[p[0x443]].unk3 &&
                  D_800A38E2 >= 0x5B) ||
                 (CPU_S32(0x430) & 0x40000))) {
                p[0x3F4] = 0;
                CPU_S32(0x3CC) = 0x80;
                CPU_S32(0x430) &= ~0x40000;
            }
        }
    }
    return CPU_S32(0x3CC);
}

#undef CPU_OPP
#undef CPU_S16
#undef CPU_U16
#undef CPU_S32
#undef CPU_FLAGS
#undef CPU_OARR
#undef CPU_SARR
#undef CPU_WP
#undef CPU_SQ
