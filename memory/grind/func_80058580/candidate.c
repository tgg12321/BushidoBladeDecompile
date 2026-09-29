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
#define CPU_TBL(o) (D_80099D88[p[0x443]].unk4[(o) - 4])
#define CPU_OARR(o) (*(s16 *)(CPU_OPP + *(s16 *)(CPU_OPP + 0x86) * 2 + (o)))
#define CPU_SARR(o) (*(s16 *)(p + *(s16 *)(p + 0x86) * 2 + (o)))
#define CPU_WP(i) (p + (i) * 6)
#define CPU_SQ(x) ((x) * (x))

#define side t3
#define n447 t3
#define ang t3
#define prod t3
#define force t3
#define idx t3
#define dist t3
#define coin t3
#define score t3
#define ok t3
#define mask t3
#define cmask t3
#define script4 t3
#define n449 t1
#define base t1
#define kind t1
#define x1 t1
#define m445 t1
#define tired t1
#define flip t1
#define w t1
#define lo t1
#define near t1
#define n445 t2
#define wtype t2
#define pace t2
#define y1 t2
#define m449 t2
#define best t2
#define cnt2 t2
#define hi t2
#define lvl t2
#define i t4
#define j t4
#define cnt t4
#define n t4
#define ok4 t4
#define far t5
#define lim t5
#define besti t5
#define et t5
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
    s32 t1, t2, t3, t4, t5;
    s32 k;
    s32 va;
    s32 vb;
    s32 vn;
    s32 vc;
    s32 adj;
    s32 sc;
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
                side = CPU_OPP[0xAF] & 1;
                f = CPU_S32(0x430);
                if (!(((f & 0x20) || ((f & 0x10) && p[0x3F3] % ((CPU_S16(0x438) >> 8) + 2) != (CPU_S16(0x438) >> 8) + 1)) &&
                      (!(CPU_FLAGS & 0xFF00) || (f & 0x40))) ||
                    (file_GetFlag1() && D_800A38DC != 3)) {
                    side = !side;
                }
                script1 = tbl[side];
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
            n447 = p[0x447] == 0;
            n445 = p[0x445] == 0;
            n449 = p[0x449] == 0;
            if (p[0x426] == 2) {
                if (CPU_S16(0x42E) * CPU_S16(0x42E) <
                    CPU_SQ(CPU_S16(0x42A) - CPU_S32(0xF4)) + CPU_SQ(CPU_S16(0x42C) - CPU_S32(0xFC))) {
                    CPU_S32(0x3CC) = 0;
                    p[0x426] = 4;
                    p[0x425] = 4;
                    p[0x3F3]++;
                } else if (n447 && (CPU_S32(0x430) & 8) &&
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
                        if (n449) {
                            script1 = D_8009A880;
                        } else if (n445) {
                            script1 = D_8009A878;
                        }
                    } else {
                        if (n445) {
                            script1 = D_8009A878;
                        } else if (n449) {
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
            ang = CPU_S16(0x43A);
            if (p[0x441] == 0) {
                ang = (ang + 0x800) & 0xFFF;
                if (ang > 0x800) {
                    ang -= 0x1000;
                }
            }
            if (!(CPU_FLAGS & 0xF00)) {
                if (ang < 0) {
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
            if (CPU_U16(0x6A) == 0x1D && (rand() & 0xFF) < CPU_TBL(4) && p[0x447] == 0) {
                CPU_S32(0x3CC) = 0x8000;
            } else {
                va = 0x80;
                if (CPU_U16(0x3E8) % (0x12 - (CPU_S16(0x438) >> 8)) == 0) {
                    va = 0x20;
                }
                CPU_S32(0x3CC) = va;
                if (p[0x447] != 0) {
                    if (p[0x446] != 0) {
                        CPU_S32(0x3CC) = va | 0x1000;
                    } else if (p[0x448] != 0) {
                        CPU_S32(0x3CC) = va | 0x4000;
                    }
                } else if (p[0x44B] == 0) {
                    CPU_S32(0x3CC) = (p[0x443] & 1) ? va | 0x1000 : va | 0x4000;
                }
            }
        } else if (st == 0x11) {
            if (CPU_S16(4) != D_800A38AE && CPU_S16(0x40) == (*(u8 **)(p + 0x50))[8] - 1) {
                s32 b, c, a;
                a = CPU_S16(0x26E);
                b = CPU_S16(0x270);
                c = CPU_S16(0x272);
                prod = 0x1000 - (((CPU_S16(0x26C) == 0 ? a + 4 + b : a + b) + c) << 8);
                prod = (CPU_S16(0x438) * prod) >> 12;
                if ((rand() & 0xFFF) < prod) {
                    CPU_S32(0x3CC) = 0x20;
                } else if (CPU_S32(0x430) & 0x20) {
                    CPU_S32(0x3CC) = 0x20;
                }
            }
        } else {
            if ((CPU_S32(0x148) - CPU_S32(0xBC) >= 0 ? CPU_S32(0x148) - CPU_S32(0xBC)
                                                     : CPU_S32(0xBC) - CPU_S32(0x148)) < 200 && !(CPU_FLAGS & 0x8C00) &&
                ((!file_GetFlag1() && D_800A38DC != 3) || D_800A38DC == 3)) {
                force = 0;
                if ((p[0x426] == 1 || p[0x425] == 2) && (CPU_S32(0x430) & 8)) {
                    force = 1;
                }
                far = 0;
                if ((rand() & 0xFFF) < (CPU_S16(0x438) >> 3)) {
                    far = CPU_U16(0x3E8) > 0x3C;
                }
                if (CPU_S16(0xE) >= 6) {
                    base = 100000;
                } else {
                    base = (&D_8009A838)[CPU_S16(0xE)] * 8;
                }
                for (i = 0; i < 8U; i++) {
                    if (!(D_8009A850[i][3] & 1) || CPU_S16(0x40) >= (*(u8 **)(p + 0x50))[8] - 2 || force) {
                        if ((D_800A387C < (t2 = D_8009A850[i][2] * 16 + base + CPU_S16(0x40A)) &&
                             (!(D_8009A850[i][3] & 8) || CPU_S16(0x43C) < 0x100) &&
                             (far || (D_8009A850[i][3] & 4))) ||
                            force) {
                            if (D_8009A850[i][0] == CPU_U16(0x6A) &&
                                ((u8)(st = D_8009A850[i][1]) == 0xFF || st == CPU_OPP[0x6A])) {
                                if (D_8009A850[i][3] & 2) {
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
                kind = p[0x44A];
                if (!(CPU_S32(0x430) & 0x800) || D_800A387C < 4000) {
                    if (p[0x442] == 2 && !(kind == 1 || kind == 2)) {
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
                    kind = p[0x44A];
                    idx = p[0x362] - 1;
                    pace = p[0x444];
                    wp = CPU_WP(idx);
                    wx = *(s16 *)(wp + 0x364);
                    wy = *(s16 *)(wp + 0x366);
                    if (!(kind == 1 || kind == 2)) {
                        script2 = 0;
                        if (wp[0x368] == 1) {
                            if ((pace == 3 && D_800A387C < CPU_OARR(0x404)) ||
                                (pace == 5 && (CPU_OARR(0x404) < D_800A387C && D_800A387C < CPU_OARR(0x404)))) {
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
                    j = idx;
                    if (idx == 0) {
                        if (p[0x368] == 1) {
                            dist = SquareRoot0(CPU_SQ(CPU_S32(0xF4) - tx) + CPU_SQ(CPU_S32(0xFC) - ty2));
                        } else {
                            dist = SquareRoot0(CPU_SQ(CPU_S32(0xF4) - wx) + CPU_SQ(CPU_S32(0xFC) - wy));
                        }
                    } else {
                        dist = SquareRoot0(CPU_SQ(wx - CPU_S32(0xF4)) + CPU_SQ(wy - CPU_S32(0xFC)));
                        if (j >= 2) {
                        leg_loop:
                            {
                                u8 *a;
                                u8 *b;
                                a = CPU_WP(j);
                                b = CPU_WP(j - 1);
                                j = j - 1;
                                dist += SquareRoot0(CPU_SQ(*(s16 *)(a + 0x364) - *(s16 *)(b + 0x364)) +
                                                    CPU_SQ(*(s16 *)(a + 0x366) - *(s16 *)(b + 0x366)));
                            }
                            if (j >= 2) {
                                goto leg_loop;
                            }
                        }
                        {
                            x1 = CPU_S16(0x36A);
                            y1 = CPU_S16(0x36C);
                            if (p[0x368] == 1) {
                                dist += SquareRoot0(CPU_SQ(x1 - tx) + CPU_SQ(y1 - ty2));
                            } else {
                                dist += SquareRoot0(CPU_SQ(x1 - CPU_S16(0x364)) + CPU_SQ(y1 - CPU_S16(0x366)));
                            }
                        }
                    }
                    dist = lim < dist;
                    CPU_S32(0x3CC) = func_80057094(p, wx, wy, dist);
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
                            ang = (ratan2(*(s16 *)(wp3 + 0x364) - CPU_S32(0xF4), *(s16 *)(wp3 + 0x366) - CPU_S32(0xFC)) -
                                   CPU_S16(0x1CA)) & 0xFFF;
                            if (ang > 0x800) {
                                ang -= 0x1000;
                            }
                            if ((ang < 0 ? -ang : ang) > 0x300) {
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
                coin = CPU_TBL(0xC) < (rand() & 0xFF);
                sel = -1;
                m445 = p[0x445] == 0;
                m449 = p[0x449] == 0;
                for (; cnt < 2; cnt++, coin++) {
                    if (coin & 1) {
                        if (*(s16 *)(CPU_OPP + 0x43A) < 0) {
                            if (m445) {
                                sel = 6;
                            } else if (m449) {
                                sel = 7;
                            }
                        } else {
                            if (m449) {
                                sel = 7;
                            } else if (m445) {
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
                t3 = CPU_S32(0x394);
            } else if (*(u16 *)(CPU_OPP + 0x6A) == 0x19) {
                u8 buf[4];
                __builtin_memcpy(buf, D_800A325C, 4);
                t3 = buf[CPU_OPP[0x441]];
            } else if (*(u16 *)(CPU_OPP + 0x6A) == 0x1A) {
                u8 buf2[4];
                __builtin_memcpy(buf2, D_800A3260, 4);
                t3 = buf2[CPU_OPP[0x441]];
            }
            switch (t3) {
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
            tired = CPU_TBL(6);
            r = rand() & 0xFF;
            if (p[0x440] == 4 ? r < (tired >> 2) : r < tired) {
                best = -1;
                besti = -1;
                j = 0;
            pick_loop:
                {
                    score = ((rand() & 0xFFF) * ((u8 *)&D_80099D88[p[0x443]] + 8)[j]) >> 12;
                    if (score != 0) {
                        flip = 0;
                        switch (j) {
                        case 0:
                        case 2:
                            if (rand() & 1) {
                                flip = 1;
                            } else if (p[0x447] != 0) {
                                goto pick_next;
                            }
                            vc = CPU_S16(0x3F0);
                            if (CPU_TBL(7) < (vc >= 0 ? vc : -vc)) {
                                score += 0x80;
                                flip = vc > 0;
                            }
                            if (CPU_S16(0xE) >= 6 && p[0x34A] == 0 && j == 2) {
                                score += 0x100;
                                flip = 0;
                            }
                            break;
                        case 1:
                        case 3:
                            if (rand() & 1) {
                                flip = 1;
                                if (p[0x449] != 0) {
                                    goto pick_next;
                                }
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
                        if ((s16)best < score) {
                            besti = j;
                            best = score;
                            bestflip = flip;
                        }
                    }
                }
            pick_next:
                if (++j < 7) {
                    goto pick_loop;
                }
                if ((t1 = (s8)besti) != -1) {
                    CPU_S32(0x394) = 0;
                    p[0x39C] = 0;
                    CPU_S16(0x398) = (((((rand() & 0xFFF) * CPU_TBL(6)) >> 12) + 0x17) << 12) / CPU_S16(0x1C);
                    switch (t1) {
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
                    n = 0;
                    while (off != 0) {
                        ep = off + *(u8 **)(p + 0x3A4);
                        e = ep;
                        ep += 4;
                        q = ep;
                        if (CPU_FLAGS & 0xFF00) {
                            switch (D_800A38DC) {
                            case 3:
                                if (D_800A38E2 < 0x5B) {
                                    mask = (u8)(D_800A38E2 / 10) * 2;
                                    if ((u8)(D_800A38E2 % 10) == 0) {
                                        mask--;
                                    }
                                } else if (D_800A38E2 < 0x5E) {
                                    mask = 0x12;
                                } else if (D_800A38E2 < 0x60) {
                                    mask = 0x13;
                                } else if (D_800A38E2 < 0x62) {
                                    mask = 0x14;
                                } else if (D_800A38E2 < 0x64) {
                                    mask = 0x15;
                                } else {
                                    mask = 0x16;
                                }
                                mask = D_8009A928[p[0x440]][mask];
                                break;
                            case 2:
                                w = D_8009A9F0[D_8009A9DC[CPU_S16(0xE)][p[0x440]]][D_800A3788];
                                mask = 0;
                                cnt2 = w & 0xF;
                                if (cnt2 != 0) {
                                    if (w >> 27) {
                                        while (cnt2 > 0) {
                                            w >>= 4;
                                            mask |= 1 << ((w & 0xF) - 1);
                                            cnt2--;
                                        }
                                    } else {
                                        w >>= ((u8)(p[0x3F2] / 3) % cnt2) * 4 + 4;
                                        mask = 1 << ((w & 0xF) - 1);
                                    }
                                }
                                break;
                            default:
                                mask = D_8009A8CA[p[0x440]][D_800A37A0 - 1][0];
                                break;
                            }
                            if ((e[3] >> 4) == 0 || !((u32)mask & (1 << ((e[3] >> 4) - 1)))) {
                                goto next;
                            }
                        }
                        if (!(CPU_FLAGS & 0x80) && *(s8 *)(p + 0x40D) == CPU_S16(0x86) && *(s8 *)(p + 0x40C) == n) {
                            goto next;
                        }
                        if (q[0] == 0x40) {
                            cmask = q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1];
                            if (!((u32)cmask & (1 << p[0x443]))) {
                                goto next;
                            }
                            ep += 5;
                        }
                        if ((e[0] & 0x80) && CPU_S16(0x26C) == 0) {
                            goto next;
                        }
                        lo = e[1] * 40;
                        hi = e[2] * 40;
                        et = (u8)(e[0] & 7);
                        ok = 0;
                        if (et == 0) {
                            if (lo < D_800A387C && D_800A387C < hi) {
                                st = *(u16 *)(CPU_OPP + 0x6A);
                                if (st == 0x15 || st == 0x2C || st == 0xE || st == 0x19) {
                                    ok = 1;
                                }
                            }
                        } else {
                            if ((CPU_FLAGS & 0xFC00) || CPU_S16(0xE) >= 6) {
                                lo = -(et < 5) & 100000;
                                hi = 100000;
                            } else {
                                adj = (((0x1000 - lv) * 625) >> 10) - 400 + CPU_S16(0x40A);
                                lo += adj;
                                hi += adj;
                            }
                            if (et < 5) {
                                if (D_800A387C < lo && !(CPU_S32(0x430) & 0x20000)) {
                                    if ((CPU_S32(0x430) & 0x200) ? CPU_S16(0x43C) < 0x800 : CPU_S16(0x43C) < 0x400) {
                                        ok = 1;
                                    } else if (CPU_S16(0xE) >= 6) {
                                        ok = 1;
                                    }
                                }
                            } else if (lo < D_800A387C && D_800A387C < hi &&
                                       CPU_S16(0x43C) < 0x200 - (CPU_S16(0x438) >> 4) &&
                                       (CPU_S32(0x430) & 0x280) != 0x280) {
                                if (et == 5) {
                                    if ((0x78 >> p[0xB1]) & 1) {
                                        ok = 1;
                                    }
                                } else if (et == 6) {
                                    if (p[0x443] == 0x15 || CPU_S16(0x330) != 0) {
                                        ok = 1;
                                    }
                                } else {
                                    ok = 1;
                                }
                            }
                        }
                        if (ok) {
                            sc = ((rand() & 0xFFF) * ((u8 *)&D_80099D88[p[0x443]] + 0xF)[et]) >> 12;
                            if (sc != 0 && pbest < sc) {
                                pbest = sc;
                                pbesti = n;
                                pscript = ep;
                                phi = hi;
                            }
                        }
                    next:
                        list++;
                        off = *list;
                        n++;
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
                script4 = 0;
                if ((rand() & 0xFF) < (CPU_TBL(0x14) >> 2) && ((0x78 >> p[0xB1]) & 1) && p[0x442] == 0 &&
                    CPU_OARR(0x404) < D_800A387C && D_800A387C < 4500) {
                    script4 = (s32)D_8009A8C0;
                } else {
                    if (p[0x443] == 0x15) {
                        lvl = p[0x34D];
                        near = CPU_OARR(0x3F8);
                    } else {
                        near = CPU_OARR(0x404) + 300;
                        if (CPU_FLAGS & 0x300) {
                            lvl = 0;
                            if (D_800A37A0 >= 6) {
                                lvl = p[0x34A];
                            }
                        } else {
                            lvl = CPU_S16(0x330);
                        }
                    }
                    ok4 = 0;
                    if ((rand() & 0xFF) < (CPU_TBL(0x15) >> 2) && lvl != 0 && near < D_800A387C &&
                        CPU_S32(0x434) == 100000 && p[0x442] == 0 && (CPU_S32(0x430) & 0xA002) &&
                        (p[0x443] != 0x15 || D_800A387C < 3000) && (CPU_S16(0x8A) == 0 || lvl >= 2)) {
                        ok4 = 1;
                    }
                    if (ok4) {
                        if ((CPU_FLAGS & 0x10) && lvl >= 2 && (rand() & 1)) {
                            script4 = (s32)D_8009A8B4;
                        } else {
                            script4 = (s32)D_8009A8AC;
                        }
                    }
                }
                if (script4 != 0) {
                    func_80055B44(p, script4, 2, 0);
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
#undef CPU_TBL
#undef CPU_OARR
#undef CPU_SARR
#undef CPU_WP
#undef CPU_SQ
#undef side
#undef n447
#undef ang
#undef prod
#undef force
#undef idx
#undef dist
#undef coin
#undef score
#undef ok
#undef mask
#undef cmask
#undef script4
#undef n449
#undef base
#undef kind
#undef x1
#undef m445
#undef tired
#undef flip
#undef w
#undef lo
#undef near
#undef n445
#undef wtype
#undef pace
#undef y1
#undef m449
#undef best
#undef cnt2
#undef hi
#undef lvl
#undef i
#undef j
#undef cnt
#undef n
#undef ok4
#undef far
#undef lim
#undef besti
#undef et
