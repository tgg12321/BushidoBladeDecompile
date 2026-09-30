extern s16 D_800A3540[];
extern s16 D_800A3544[];
extern u8 D_8009BC38[];
extern s16 D_800A3594[];
extern u8 D_800A3562;
void func_80070F78(s32 arg0, DescF97C *s) {
    s32 *sheets; /* several values of one kind, a sprite-sheet header table:
                  * *(D_800A35A8 + 0x74) (loaded at entry and again in the
                  * selected-slot arm) and *(D_800A35A8 + 0x60) (the table the
                  * last loop draws from). Ruling 11
                  * (ordinary-c-judge-decidable.md), proof in
                  * memory/grind/func_80070F78/r11/README.md. */
    s32 flag;
    s32 cells; /* several values of one kind: the cell table following a sheet
                * header (s->header + 0xC in the two loop-2 draws, s->header +
                * 0x24 for the last loop). Ruling 11, proof in
                * memory/grind/func_80070F78/r11/README.md. */
    s16 i;
    s16 port;
    s16 port_ofs;
    s32 c;

    flag = 0;
    sheets = *(s32 **)(D_800A35A8 + 0x74);
    s->header = sheets[0];
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s->header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x20, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;
    s->has_color = 1;
    s->scale_x = 0x100;
    s->scale_y = 0x100;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3578 == 0) {
            /* FAKE: named intermediate for player i's 3-byte D_800A3560 record
             * offset (named-intermediate entry, no-new-park-categories.md). This
             * and the per-site offsets below (rec, ofs, idx, other) share
             * func_80070188's mechanism: an inline `D_800A3560[i * 3 + k]` is
             * expanded as *(&D_800A3560 + (i * 3 + k)) (expr.c); its EXPAND_SUM
             * address (plus (mult i 3) sym+k) is not a legitimate address, so
             * explow.c memory_address -> force_operand copies sym+k into its own
             * pseudo, which cse shares and loop.c hoists (`lui/addiu` + `addu`,
             * 0() addressing). With the offset already in a pseudo, (plus off
             * sym+k) is legitimate as it stands, giving the target's `lui $at;
             * addu $at,$at,off; lbu %lo(D_800A356k)($at)`. Measured (this body 1
             * = one GPREL-name artifact): inlining this rec 52, the loop-2 rec 14,
             * ofs 22, the ==3 idx 32, the locked idx 33, other 24;
             * memory/grind/func_80070F78/evidence.md [s2]. */
            s32 rec = i * 3;

            if (D_800A3560[rec] == 5) {
                D_800A3560[rec + 2] = 6;
                flag = 1;
                ((s16 *)D_800A35C4)[i + 2] = 0;
            } else if (D_800A3560[rec] == 0x10) {
                D_800A3560[rec + 2] = 7;
                flag = 1;
                ((s16 *)D_800A35C4)[i + 2] = 0;
            }
        }
    }
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3558); i++) {
        u8 *vram; /* several values of one kind: player i's VRAM rect row,
                   * *(D_800A35A8 + 0x7C) + (i << 6), computed at the top of the
                   * loop and again in the locked-slot arm. Ruling 11, proof in
                   * memory/grind/func_80070F78/r11/README.md. */
        s32 rec; /* FAKE: record offset, mechanism at the loop-1 `rec` */

        vram = *(u8 **)(D_800A35A8 + 0x7C);
        vram += i << 6;
        port = i - port_ofs;
        rec = i * 3;
        if (D_800A3560[rec] != 5 && D_800A3560[rec] != 0x10) {
            s32 max;
            s32 min;
            s32 ofs; /* FAKE: record offset, mechanism at the loop-1 `rec` */

            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3561] & 2) {
                    max = 5;
                    min = 1;
                } else {
                    max = 4;
                    min = 0;
                }
            } else if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) {
                max = 4;
                min = 0;
            } else {
                max = 5;
                min = 1;
            }
            ofs = i * 3;
            if (D_800A3560[ofs + 2] == 0xFF) {
                D_800A3544[i] = 7;
                D_800A3540[i] = 0;
                ((s16 *)D_800A35C4)[i + 2] = 0x1E;
                if ((D_800A354C & (0x2000 << (port * 16))) && D_800A3590[i] < max) {
                    func_8005C650(0, 0x7F, 0x7F);
                    D_800A3590[i]++;
                } else if ((D_800A354C & (0x8000 << (port * 16))) && min < D_800A3590[i]) {
                    func_8005C650(0, 0x7F, 0x7F);
                    D_800A3590[i]--;
                }
                D_800A3594[i] = i;
                if (D_800A35BC == 3) {
                    s32 idx; /* FAKE: record offset, mechanism at the loop-1 `rec` */
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: named intermediate for the address of the image
                               * pointer LoadImage reads (named-intermediate entry,
                               * no-new-park-categories.md). Under `*(...)` the sum is an
                               * address and expands with EXPAND_SUM, where expr.c's
                               * PLUS_EXPR both_summands moves the constant 0x14 outside
                               * (the load becomes `lw 0x14(reg)`); assigned to a local
                               * it expands as a value (expr.c binop), computing
                               * id * 8 + 0x14 first as the target does (`addiu
                               * $v0,$a0,0x14; addu $v1,$v1,$v0; ...; lw $a1,0($v1)`).
                               * Inlined: this site 9, the confirm site 10, the locked
                               * site 15; memory/grind/func_80070F78/evidence.md [s2]. */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3565 = 0;
                    D_800A3562 = 0;
                    idx = i * 3;
                    if (D_8009BC7C[D_800A3560[idx + 1]] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560[idx + 2];
                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x40 << (port * 16))) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3560[i * 3 + 2] = D_8009BC38[D_800A3590[i]];
                    sel = 1;
                    if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                        if (!(D_8009BC7C[D_800A3561] & 2)) {
                            sel = 0;
                        }
                    } else if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) {
                        sel = 0;
                    }
                    id = D_800A3560[i * 3 + 2];
                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x10 << (port * 16))) {
                    func_8005C650(2, 0x7F, 0x7F);
                    if (D_800A3558 == 1) {
                        D_800A3558 = 0;
                        D_800A3562 = 0xFF;
                    } else {
                        D_800A3565 = 0xFF;
                        D_800A3562 = 0xFF;
                        D_800A3578 = 3;
                        D_800A3584 = 0;
                        func_8005C650(6, 0x7F, 0x7F);
                        if (D_800A3554 == 1) {
                            D_800A3563 = 0xFF;
                        } else {
                            D_800A3560[i * 3] = 0xFF;
                        }
                        break;
                    }
                }
            } else {
                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {
                    func_8005C650(2, 0x7F, 0x7F);
                    D_800A35C8[0] = 0xF;
                    D_800A35C8[1] = 0x14;
                    D_800A3560[ofs + 2] = 0xFF;
                }
                D_800A3544[i] = 9;
                if (((s16 *)D_800A35C4)[i + 2] != 0) {
                    ((s16 *)D_800A35C4)[i + 2]--;
                }
                sheets = *(s32 **)(D_800A35A8 + 0x74);
                s->header = sheets[3];
                if (D_800A3540[i] < 12) {
                    D_800A3540[i]++;
                }
                *(u8 *)(s->header + 2) = D_800A3540[i];
                s->has_color = 0;
                cells = s->header + 0xC;
                s->table = cells;
                s->x = (D_800A3590[i] << 6) + 0x85;
                s->y = 0xB6 - (D_800A3594[i] << 5);
                s->ot_idx = 7;
                s->out = ((s32 *)arg0)[4];
                ((s32 *)arg0)[4] = func_8007352C((s32)s);
            }
            if (D_800A3578 != 3) {
                s->scale_x = 0x100;
                s->scale_y = 0x100;
                if (((s16 *)D_800A35C4)[i + 2] != 0) {
                    s->has_color = 1;
                } else {
                    s->has_color = 0;
                }
                if (D_800A3558 != 0 && i != 0) {
                    s->header = sheets[i + 1];
                } else {
                    s->header = sheets[i];
                }
                cells = s->header + 0xC;
                s->table = cells;
                c = ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << D_800A3544[i]) + i * 511) * 63) >> 12) - 0x40;
                s->col_b = c;
                s->col_g = c;
                s->col_r = c;
                s->x = (D_800A3590[i] << 6) + 0x80;
                s->y = 0xAC - (D_800A3594[i] << 5);
                s->pad0C = ((s32 *)arg0)[1];
                s->ot_idx = 7;
                ((s32 *)arg0)[1] = func_80073728((s32)s, 0);
            }
        } else {
            if (D_800A3578 == 0) {
                s32 other = i == 0 ? 3 : 0; /* FAKE: the other player's record offset,
                                             * mechanism at the loop-1 `rec` */

                if (D_800A3560[other + 2] != 0xFF && ((s16 *)D_800A35C4 + 2)[i == 0 ? 1 : 0] == 0) {
                    s32 idx; /* FAKE: record offset, mechanism at the loop-1 `rec` */
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    flag = 2;
                    vram = *(u8 **)(D_800A35A8 + 0x7C);
                    vram += i << 6;
                    idx = i * 3;
                    if (D_8009BC7C[D_800A3560[idx + 1]] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560[idx + 2];
                    LoadImage(vram + id * 8, *(s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4));
                    DrawSync(0);
                }
            }
            if (D_800A354C & (0x10 << (i * 16))) {
                func_8005C650(2, 0x7F, 0x7F);
                D_800A3578 = 3;
                func_8005C650(6, 0x7F, 0x7F);
                D_800A3584 = 0;
                D_800A3565 = 0xFF;
                D_800A3562 = 0xFF;
                if (D_800A3558 == 1) {
                    D_800A3558 = 0;
                    D_800A3563 = 0xFF;
                } else {
                    D_800A3560[i * 3] = 0xFF;
                }
                ((s16 *)D_800A35C4)[i + 2] = 0x1E;
            }
        }
    }
    sheets = *(s32 **)(D_800A35A8 + 0x60);
    s->header = sheets[0];
    s->scale_x = 0x80;
    s->scale_y = 0x80;
    s->has_color = 0;
    s->ot_idx = 0xC;
    cells = s->header + 0x24;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560[i * 3] != 5 && D_800A3560[i * 3] != 0x10 && D_800A3578 != 3) {
            s->table = cells;
            s->x = (D_800A3590[i] << 6) + 0x80;
            s->y = 0xAC - (D_800A3594[i] << 5);
            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3561] & 2) {
                    s->table += *(u8 *)(s->header + 2) * 8;
                }
            } else if (!(D_8009BC7C[D_800A3560[i * 3 + 1]] & 2)) {
                s->table += *(u8 *)(s->header + 2) * 8;
            }
            s->pad0C = ((s32 *)arg0)[1];
            ((s32 *)arg0)[1] = func_80073728((s32)s, 0);
        }
    }
    if (D_800A3562 != 0xFF && ((s16 *)D_800A35C4)[2] == 0) {
        if (D_800A35BC == 2) {
            D_800A3558 = 1;
        }
        if (((D_800A3565 != 0xFF && ((s16 *)D_800A35C4)[3] == 0) || D_800A35B0 + D_800A3558 == 0) && flag != 1) {
            D_800A3578 = 0x101;
            D_800A3584 = 2;
            D_800A3550 = 1;
            D_800A355C = 0;
        }
    }
}
