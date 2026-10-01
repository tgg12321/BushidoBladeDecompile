/* [unk_0A][min(unk_272, 3)]: 4.12 scale of unk_1E into unk_20 (func_80023F08). */
extern s16 D_8008E0BC[27][4];
/* [D_8008D9EC[unk_0A]][unk_0E]: the move command func_80023F08 hands
 * cpu_set_move_command_and_dir. */
extern u8 D_8008D90C[28][8];
extern void cpu_check_same_dir_timer(s32 *);
extern void cpu_set_move_command_and_dir(PracticeMenuRec *, s32, Vec3i32 *);
extern s32 func_8002798C(u8 *);
extern s32 func_8002FDB0(s32 *);
extern s32 func_800307D0(PracticeMenuRec *);
extern s32 func_80030BA8(PracticeMenuRec *);
extern u8 *func_80032064(u8 *, s32);
extern void func_80039680(u8 *);
extern void func_80040304(s32, s32);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80041188(s32, u8 *, u8 *, s32, s32 *);
extern void func_80049718(s32, s32, s32 *, s16 *);
extern void func_80049A2C(s32, s32, s32);
extern void scratchpad_Save(void);
extern void scratchpad_Restore(void);
extern void func_800204C0(PracticeMenuRec *);
extern void func_800207C8(PracticeMenuRec *, LeafPos *, LeafPos *, LeafPos *);

/* Per-frame update of character `arg0`'s record from this frame's pad input:
 * converts the pad bits, advances the move frame, runs the move script's
 * command list (unk_50 / unk_7C, func_80021424 / func_80021A98), decodes the
 * current and next motion frames and blends their root offsets into unk_E8,
 * integrates the velocities (unk_104 / unk_134) into the position, resolves
 * it (func_8002304C), then refreshes the bone points, hit flags (unk_62,
 * unk_AE, unk_288) and damping and the per-frame side effects. */
void func_80023F08(s32 arg0, PadState *pad) {
    MotionFrame pose[2];
    Vec3i32 v[2];
    s32 pos[3];
    s16 ang[2];
    s32 dir;
    s16 alt;
    PracticeMenuRec *rec;
    u16 cmd;
    u16 keys;
    u16 *ent;
    MoveScript *move;
    /* temp holds four values: the unk_14C clamp limit, the unk_1D8 / unk_1C8.vy
     * angle gap (folded to 0..0x800), the unk_14C turn step and the -1/0/1
     * stick side. One local, not four:
     * ordinary-c-judge-decidable.md Ruling 11; (D) record in
     * memory/grind/func_80023F08/r11/ (evidence.md [s3]). */
    s32 temp;
    s32 face;
    s32 face90;
    s32 blend;
    s32 cur_frame;
    s32 next;
    s32 first;
    s32 next_frame;
    s32 motion;
    s32 extra;
    s32 hit;
    s16 frac;
    s32 rest;
    s32 perp;

    rec = &g_practice_menu_table[arg0];
    rec->unk_3C++;
    rec->unk_24 = *pad;
    if (D_800A38BA != 0 && (arg0 == 0 || (arg0 == 1 && rec->unk_06 == 0))) {
        rec->unk_24.held = (rec->unk_24.held & 0xFFF) | ((rec->unk_24.held & 0x8000) >> 3) | ((rec->unk_24.held & 0x7000) << 1);
        rec->unk_24.pressed = (rec->unk_24.pressed & 0xFFF) | ((rec->unk_24.pressed & 0x8000) >> 3) | ((rec->unk_24.pressed & 0x7000) << 1);
        rec->unk_24.released = (rec->unk_24.released & 0xFFF) | ((rec->unk_24.released & 0x8000) >> 3) | ((rec->unk_24.released & 0x7000) << 1);
        rec->unk_24.unheld = (rec->unk_24.unheld & 0xFFF) | ((rec->unk_24.unheld & 0x8000) >> 3) | ((rec->unk_24.unheld & 0x7000) << 1);
    }
    if (D_800A38DC == 2 || D_800A38DC == 5 || D_800A38DC == 3 || (D_800A38DC == 0 && D_800A385C != 0)) {
        rec->unk_24.pressed &= ~0x100;
    }
    if (rec->unk_3C < 2) {
        rec->unk_24.held = rec->unk_24.pressed = rec->unk_24.released = 0;
        rec->unk_24.unheld = 0xFFFF;
    }
    rec->unk_20 = (rec->unk_1E * D_8008E0BC[rec->unk_0A][rec->unk_272 >= 4 ? 3 : rec->unk_272]) >> 12;
    if (rec->unk_7A == 1) {
        rec->unk_7A = 0;
    }
    func_8001F938((u8 *)rec);
    frac = rec->unk_42 + rec->unk_44;
    rec->unk_42 = frac;
    if (frac >= 0x1000) {
        rec->unk_46 = 0;
        rec->unk_40 += frac >> 12;
        rec->unk_42 &= 0xFFF;
    } else {
        rec->unk_46 = 1;
    }
    if (rec->unk_286 >= 0) {
        /* the move-script record of unk_50's follow-up id is a u16 move-id
         * table indexed by event code (here unk_286; 0xD, 0x17/0x18 below) */
        u16 *table;

        if (rec->unk_6A == 0xA || rec->unk_6A == 0x17 || rec->unk_6A == 0x18) {
            rec->unk_72 = 1;
        }
        rec->unk_31A = 0;
        table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
        func_80021A98(arg0, func_80021424((u8 *)rec, table[rec->unk_286], (u8 *)&rec->unk_5E), rec->unk_5E);
        rec->unk_286 = -1;
    }
    if ((rec->unk_24.held & 8) && (rec->unk_24.held & 0xF000)) {
        if ((rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x13 || rec->unk_6A == 0x30 || rec->unk_6A == 0x31) && func_800233AC((u8 *)rec, &dir) != 0) {
            u16 *table;

            func_8001F860((s16 *)rec, dir);
            rec->unk_31A = 0;
            table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
            func_80021A98(arg0, func_80021424((u8 *)rec, table[0xD], (u8 *)&rec->unk_5E), rec->unk_5E);
            rec->unk_104.vx = 0;
            rec->unk_104.vy = 0;
            rec->unk_104.vz = 0;
        }
    }
    if (rec->unk_50->unk_07 < rec->unk_40) {
        if (rec->unk_7C != NULL) {
            rec->unk_5E = rec->unk_80;
            if (rec->unk_82 & 0x1000) {
                rec->unk_4C = 1;
            }
            func_80021A98(arg0, (u8 *)rec->unk_7C, rec->unk_80);
        } else {
            if ((rec->unk_50->unk_0A[0] & 0x2000) && D_800A38AE != arg0) {
                if (rec->unk_50->unk_0A[0] & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unk_0A[1], (u8 *)&rec->unk_5E), rec->unk_5E);
            } else {
                if (rec->unk_50->unk_09 & 0x20) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unk_02, (u8 *)&rec->unk_5E), rec->unk_5E);
            }
        }
    }
    if (rec->unk_7C != NULL) {
        if (rec->unk_50->unk_08 < rec->unk_40) {
            if (rec->unk_6A == 0x11) {
                D_800A376E = 1;
                D_800A36D8 = rec->unk_7C;
                D_800A381C = rec->unk_80;
                D_800A36CA = rec->unk_82;
            } else {
                rec->unk_5E = rec->unk_80;
                if (rec->unk_82 & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, (u8 *)rec->unk_7C, rec->unk_80);
            }
        }
    } else if (rec->unk_50->unk_08 >= rec->unk_40) {
        ent = rec->unk_50->unk_0A;
        while ((cmd = ent[0]) != 0) {
            /* FAKE: named intermediate (no-new-park-categories.md family 6): the
             * entry's class mask, a 32-bit set with one bit per character class
             * (unk_0A, 27 classes; bit set = the entry applies), held as a u32
             * bitset as func_8002AB08 holds its mask_a / mask_b / mask_c (layer-2
             * rev-2AB08-dm PASS). Tested directly as an int,
             * fold-const.c:4437 turns (mask & (1 << cls)) != 0 into
             * ((mask >> cls) & 1) != 0 (srav / andi; score 3); against an unsigned
             * mask the int (1 << cls) is converted, the rewrite does not apply and
             * the target's li 1 / sllv / and stays (0x800244E0..E8). Receipts:
             * memory/grind/func_80023F08/casts/receipts.txt */
            u32 mask;

            if (!(cmd & 0x8000) || ((mask = ent[2] | (ent[3] << 16)) & (1 << rec->unk_0A))) {
                switch (cmd & 0x30) {
                case 0x00:
                    keys = rec->unk_24.held;
                    break;
                case 0x10:
                    keys = rec->unk_24.pressed;
                    break;
                case 0x20:
                    keys = rec->unk_24.unheld;
                    break;
                case 0x30:
                    keys = rec->unk_24.released;
                    break;
                }
                if (cmd & 0x2000) {
                    keys = 0;
                }
                if (rec->unk_6A == 0x11 && D_800A38AE == arg0) {
                    keys = 0;
                }
                if ((cmd & 0xF) == 9) {
                    cmd = (cmd & 0xFFF0) | 3;
                    if (rec->unk_00->unk_6A != 6 && rec->unk_00->unk_40 < rec->unk_00->unk_AB) {
                        keys = 0;
                    }
                } else if ((cmd & 0xF) == 0xA) {
                    cmd = (cmd & 0xFFF0) | 5;
                    if (rec->unk_26C == 0) {
                        keys = 0;
                    }
                }
                if ((keys >> (cmd & 0xF)) & 1) {
                    rec->unk_B0 = cmd;
                    if (cmd & 0x1000) {
                        rec->unk_4C = 1;
                    }
                    move = func_80021424((u8 *)rec, ent[1], (u8 *)&alt);
                    if (rec->unk_50->unk_09 & 0x80) {
                        rec->unk_7C = move;
                        rec->unk_82 = cmd;
                        rec->unk_4C = 0;
                        rec->unk_80 = alt;
                    } else if (rec->unk_6A == 0x11) {
                        D_800A376E = 1;
                        D_800A36D8 = move;
                        D_800A36CA = cmd;
                        D_800A381C = alt;
                    } else {
                        rec->unk_5E = alt;
                        func_80021A98(arg0, (u8 *)move, alt);
                    }
                    break;
                }
            }
            if (cmd & 0xC000) {
                ent += 4;
            } else {
                ent += 2;
            }
        }
    }
    if (rec->unk_6A == 0xA || rec->unk_72 != 0) {
        if (func_80023DB8((u8 *)rec) != 0) {
            if (rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
                u16 *table;

                table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
                func_80021A98(arg0, func_80021424((u8 *)rec, table[rec->unk_94 != 0 ? 0x17 : 0x18], (u8 *)&rec->unk_5E), rec->unk_5E);
            } else if (rec->unk_72 == 0) {
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unk_02, (u8 *)&rec->unk_5E), rec->unk_5E);
            }
            rec->unk_72 = 0;
        }
    } else if (rec->unk_7A != 0 && (rec->unk_6C == 0x17 || rec->unk_6C == 0x18) && !(rec->unk_6A == 0x17 || rec->unk_6A == 0x18) && rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
        u16 *table;

        table = func_80021424((u8 *)rec, rec->unk_50->unk_00, (u8 *)&rec->unk_5E);
        func_80021A98(arg0, func_80021424((u8 *)rec, table[0x18], (u8 *)&rec->unk_5E), rec->unk_5E);
    }
    if (rec->unk_70 == 1) {
        func_8001F860((s16 *)rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26 || rec->unk_6A == 0x24) {
            func_8001F860((s16 *)rec, rec->unk_1D8);
            if (rec->unk_6A == 2 || rec->unk_6A == 0x24) {
                temp = (rec->unk_58[2] >> 4) * 0x88;
                if (rec->unk_14C > temp) {
                    rec->unk_14C = temp;
                } else if (rec->unk_14C < -temp) {
                    rec->unk_14C = -temp;
                }
            }
        }
    }
    if (rec->unk_46 == 0 && rec->unk_6A == 9 && rec->unk_A9 == rec->unk_40) {
        func_8001F860((s16 *)rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x18 && rec->unk_6A == 0x18) {
        rec->unk_104.vx += (Judge[rec->unk_1C8.vy & 0xFFF] * D_8008DA94[rec->unk_0A]) >> 12;
        rec->unk_104.vz += (Judge[(rec->unk_1C8.vy + 0x400) & 0xFFF] * D_8008DA94[rec->unk_0A]) >> 12;
        rec->unk_104.vy += D_8008DA50[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x17 && rec->unk_6A == 0x17) {
        rec->unk_104.vy += D_8008DAD8[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x28) {
        s32 dv[3];
        s32 tgt[3];

        temp = (rec->unk_1D8 - rec->unk_1C8.vy) & 0xFFF;
        if (temp >= 0x800) {
            temp = 0x1000 - temp;
        }
        tgt[0] = rec->unk_00->unk_18C.x - ((Judge[rec->unk_1D8 & 0xFFF] * temp) >> 14);
        tgt[1] = rec->unk_00->unk_18C.y;
        tgt[2] = rec->unk_00->unk_18C.z - ((Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * temp) >> 14);
        rec->unk_104.vy -= 0xFA;
        func_800200DC((s32 *)&rec->unk_180, tgt, rec->unk_104.vy, 0x1F, dv);
        rec->unk_104.vx += dv[0];
        rec->unk_104.vz += dv[2];
        rec->unk_72 = 1;
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_6A == 0x17 || rec->unk_6A == 0x18 || rec->unk_6A == 0x28 || rec->unk_6A == 0xA) {
        if (rec->unk_40 == rec->unk_50->unk_07 && rec->unk_104.vy > 0) {
            rec->unk_40 = rec->unk_50->unk_07 - 1;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x23) {
        rec->unk_B8.vy -= 0x898;
        rec->unk_D8.y -= 0x898;
        rec->unk_1F8.y += 0x898;
    }
    {
        /* FAKE: named intermediate (no-new-park-categories.md family 6): unk_6A
         * is read into `state` before the unk_7A test, as the target loads it
         * (lhu 0x6A ahead of the beqz at 0x80024C14). Spelled with two direct
         * reads, the 6A load follows the branch and jump.c thread_jumps sends
         * the 0x23 test's unk_7A == 0 jump past this test (+2 insns, score 3);
         * u16 state 1. Final .s of both: memory/grind/func_80023F08/fake/ */
        s32 state = rec->unk_6A;

        if (rec->unk_7A != 0 && state == 6) {
            rec->unk_86 = rec->unk_84;
        }
    }
    temp = (rec->unk_14C * rec->unk_44) / 24576;
    rec->unk_1C8.vy += temp;
    rec->unk_14C -= temp;
    if (rec->unk_7A != 0) {
        if (rec->unk_78 != 0) {
            func_80023D08((s32)rec);
        } else if (rec->unk_6A == 0x1C) {
            func_80023CB4((s16 *)rec, 0x400);
        } else {
            if (rec->unk_6C == 0x1C || rec->unk_6A == 0x11 || rec->unk_6C == 0x11 || rec->unk_6A == 0x15 || rec->unk_6A == 0xA || rec->unk_6A == 0x2D) {
                func_80023D08((s32)rec);
            } else {
                switch ((rec->unk_6C << 8) | rec->unk_6A) {
                case 0x1525:
                case 0x1502:
                case 0x1322:
                case 0x1519:
                case 0x3022:
                case 0x1913:
                case 0x3122:
                    func_80023D08((s32)rec);
                    break;
                }
            }
        }
    }
    next = rec->unk_40 + 1;
    if (next >= rec->unk_58[1]) {
        next = rec->unk_58[1] - 1;
    }
    if (rec->unk_50->unk_09 & 0x40) {
        motion = rec->unk_5E == 0 ? g_practice_menu_table[D_800A38AE].unk_4A + 1 : 0;
        if (rec->unk_6A == 0x11 && D_800A38AE != arg0) {
            extra = rec->unk_58[1];
        } else {
            extra = 0;
        }
    } else {
        motion = rec->unk_5E == 0 ? rec->unk_4A + 1 : 0;
        if (rec->unk_6A == 0x15 && D_8008D9EC[rec->unk_0A] != 0) {
            extra = rec->unk_58[1];
        } else {
            extra = 0;
        }
    }
    first = extra + *rec->unk_54;
    cur_frame = first + rec->unk_40;
    next_frame = first + next;
    rec->unk_64 = (motion << 14) | cur_frame;
    rec->unk_66 = (motion << 14) | next_frame;
    if (rec->unk_6A == 0x33) {
        pose[1] = D_800A3888[arg0][rec->unk_40];
        pose[0] = pose[1];
    } else {
        func_800198D0(motion, cur_frame, (u32 *)&pose[0], (u16 *)0x1F8001B0);
        func_800198D0(motion, next_frame, (u32 *)&pose[1], (u16 *)0x1F8001B0);
    }
    if (rec->unk_7A != 0 && (rec->unk_6A == 6 || rec->unk_6A == 0x14) && rec->unk_6C == 6) {
        MATRIX m1;
        MATRIX m2;
        s32 r;
        s32 twist;

        math_RotMatrixZYXAngles(rec->unk_290.unk_06, rec->unk_290.unk_08, rec->unk_290.unk_0A, m1.m[0]);
        math_RotMatrixZYXAngles(pose[0].unk_06, pose[0].unk_08, pose[0].unk_0A, m2.m[0]);
        /* new frame's heading minus the previous frame's (old one computed first) */
        twist = -ratan2(m1.m[0][0], m1.m[2][0]) + ratan2(m2.m[0][0], m2.m[2][0]);

        rec->unk_1C8.vy += twist;
        func_8001B690(arg0, twist);
    }
    func_8001F2E4((u8 *)rec, (u8 *)&pose[0], (u8 *)&pose[1]);
    if (rec->unk_6A == 0x11) {
        rec->unk_154 = rec->unk_1C8.vy;
    }
    if (rec->unk_152 != 0) {
        face = rec->unk_154;
    } else {
        face = rec->unk_1C8.vy;
    }
    ang[0] = ang[1] = face;
    face90 = face + 0x400;
    v[0].x = (pose[0].unk_04 * Judge[(pose[0].unk_02 - face90 + 0x400) & 0xFFF]) >> 12;
    v[0].y = -pose[0].unk_00;
    v[0].z = (pose[0].unk_04 * Judge[(pose[0].unk_02 - face90) & 0xFFF]) >> 12;
    v[1].x = (pose[1].unk_04 * Judge[(pose[1].unk_02 - face90 + 0x400) & 0xFFF]) >> 12;
    v[1].y = -pose[1].unk_00;
    v[1].z = (pose[1].unk_04 * Judge[(pose[1].unk_02 - face90) & 0xFFF]) >> 12;
    v[0].x = (v[0].x * rec->unk_1A) >> 12;
    v[0].z = (v[0].z * rec->unk_1A) >> 12;
    v[1].x = (v[1].x * rec->unk_1A) >> 12;
    v[1].z = (v[1].z * rec->unk_1A) >> 12;
    if (rec->unk_6A != 8) {
        v[0].y = (v[0].y * rec->unk_1A) >> 12;
        v[1].y = (v[1].y * rec->unk_1A) >> 12;
    }
    if (rec->unk_31A != 0) {
        pose[1] = pose[0];
        ang[1] = ang[0];
        pose[0] = rec->unk_290;
        ang[0] = rec->unk_314;
        blend = rec->unk_318;
        rec->unk_66 = rec->unk_64;
        rec->unk_64 = rec->unk_316;
    } else {
        blend = rec->unk_42;
    }
    rest = 0x1000 - blend;
    rec->unk_68 = blend;
    if (rec->unk_31A != 0) {
        Vec3i32 vc;

        if (rec->unk_7A != 0) {
            rec->unk_D8.x += rec->unk_1F8.x - v[0].x;
            rec->unk_D8.z += rec->unk_1F8.z - v[0].z;
            rec->unk_320.x += rec->unk_1F8.x - v[0].x;
            rec->unk_320.z += rec->unk_1F8.z - v[0].z;
        }
        perp = ang[0] + 0x400;
        vc.x = (pose[0].unk_04 * Judge[(pose[0].unk_02 - perp + 0x400) & 0xFFF]) >> 12;
        vc.y = -pose[0].unk_00;
        vc.z = (pose[0].unk_04 * Judge[(pose[0].unk_02 - perp) & 0xFFF]) >> 12;
        vc.x = (vc.x * rec->unk_1A) >> 12;
        vc.y = (vc.y * rec->unk_1A) >> 12;
        vc.z = (vc.z * rec->unk_1A) >> 12;
        vc.x -= rec->unk_320.x;
        vc.z -= rec->unk_320.z;
        rec->unk_E8.x = (vc.x * rest + v[0].x * blend) >> 12;
        rec->unk_E8.y = (vc.y * rest + v[0].y * blend) >> 12;
        rec->unk_E8.z = (vc.z * rest + v[0].z * blend) >> 12;
    } else {
        rec->unk_E8.x = (v[0].x * rest + v[1].x * blend) >> 12;
        rec->unk_E8.y = (v[0].y * rest + v[1].y * blend) >> 12;
        rec->unk_E8.z = (v[0].z * rest + v[1].z * blend) >> 12;
    }
    if (rec->unk_7A != 0 && rec->unk_31A == 0) {
        rec->unk_D8.x += rec->unk_1F8.x - rec->unk_E8.x;
        rec->unk_D8.z += rec->unk_1F8.z - rec->unk_E8.z;
        if (rec->unk_6A == 0x23 || rec->unk_6A == 0xA) {
            rec->unk_D8.y += rec->unk_1F8.y - rec->unk_E8.y;
        }
    }
    func_80023648((u8 *)rec);
    func_800238C4((u8 *)rec);
    if (rec->unk_6A != 8 && rec->unk_6A != 0x22) {
        rec->unk_104.vy += 0x15;
    }
    {
        /* FAKE: named intermediate (no-new-park-categories.md family 6): with
         * `state`, cse.c keeps this test's own li 8 / li 0x22 (the target
         * re-materialises them at 0x80025780 / 0x80025788); with direct rec->unk_6A
         * reads cse substitutes the previous test's constant pseudos (score 11).
         * .cse of both: memory/grind/func_80023F08/fake/ */
        s32 state = rec->unk_6A;

        if (state == 8 || state == 0x22) {
            rec->unk_134.vx -= rec->unk_98.vx / 256;
            rec->unk_134.vz -= rec->unk_98.vz / 256;
        }
    }
    if (rec->unk_7A == 0 || rec->unk_6A != 0x28) {
        rec->unk_104.vx = (rec->unk_104.vx * rec->unk_156) >> 12;
        rec->unk_104.vy = (rec->unk_104.vy * rec->unk_158) >> 12;
        rec->unk_104.vz = (rec->unk_104.vz * rec->unk_15A) >> 12;
    }
    if (rec->unk_104.vx >= -15 && rec->unk_104.vx <= 15) {
        rec->unk_104.vx /= 2;
    }
    if (rec->unk_104.vy >= -15 && rec->unk_104.vy <= 15) {
        rec->unk_104.vy /= 2;
    }
    if (rec->unk_104.vz >= -15 && rec->unk_104.vz <= 15) {
        rec->unk_104.vz /= 2;
    }
    rec->unk_D8.x += rec->unk_104.vx;
    rec->unk_D8.y += rec->unk_104.vy;
    rec->unk_D8.z += rec->unk_104.vz;
    if ((rec->unk_6A == 7 || rec->unk_6A == 0xD) && rec->unk_B4 == 0) {
        temp = (rec->unk_24.held & 0x1000) ? 1 : -((rec->unk_24.held & 0x4000) != 0);
        if (temp != 0) {
            rec->unk_134.vx /= 2;
            rec->unk_134.vz /= 2;
            rec->unk_134.vx += (Judge[rec->unk_1D8 & 0xFFF] - Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * temp * 3) / 128;
            rec->unk_134.vz += (Judge[(rec->unk_1D8 + 0x400) & 0xFFF] + Judge[rec->unk_1D8 & 0xFFF] * temp * 3) / 128;
        }
    }
    rec->unk_134.vx = (rec->unk_134.vx * rec->unk_15E) >> 12;
    if (rec->unk_134.vx >= -3 && rec->unk_134.vx <= 3) {
        rec->unk_134.vx = 0;
    }
    rec->unk_134.vy = (rec->unk_134.vy * rec->unk_160) >> 12;
    if (rec->unk_134.vy >= -3 && rec->unk_134.vy <= 3) {
        rec->unk_134.vy = 0;
    }
    rec->unk_134.vz = (rec->unk_134.vz * rec->unk_162) >> 12;
    if (rec->unk_134.vz >= -3 && rec->unk_134.vz <= 3) {
        rec->unk_134.vz = 0;
    }
    if (rec->unk_6A == 4 || rec->unk_6A == 0x14 || rec->unk_0C == 0x1F) {
        rec->unk_134.vx = 0;
        rec->unk_134.vy = 0;
        rec->unk_134.vz = 0;
    } else {
        rec->unk_D8.x += rec->unk_134.vx;
        rec->unk_D8.y += rec->unk_134.vy;
        rec->unk_D8.z += rec->unk_134.vz;
    }
    pos[0] = rec->unk_D8.x + rec->unk_E8.x;
    pos[1] = rec->unk_D8.y;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        pos[1] += rec->unk_E8.y;
    }
    pos[2] = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_C8 = rec->unk_B8;
    func_8002304C((u8 *)rec, (s32 *)&rec->unk_B8, pos, (s32 *)&rec->unk_104);
    if (rec->unk_B1 != 7 && rec->unk_B1 != 0) {
        rec->unk_B2 = ((0x2A >> rec->unk_B1) ^ 1) & 1;
    }
    rec->unk_D8.x = rec->unk_B8.vx - rec->unk_E8.x;
    rec->unk_D8.y = rec->unk_B8.vy;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        rec->unk_D8.y -= rec->unk_E8.y;
    }
    rec->unk_D8.z = rec->unk_B8.vz - rec->unk_E8.z;
    rec->unk_F4.x = rec->unk_D8.x + rec->unk_E8.x;
    rec->unk_F4.y = rec->unk_D8.y + rec->unk_E8.y;
    rec->unk_F4.z = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_1F8 = rec->unk_E8;
    rec->unk_168.x = rec->unk_F4.x;
    rec->unk_168.y = rec->unk_D8.y - 0x384;
    rec->unk_168.z = rec->unk_F4.z;
    func_80023E40((u8 *)rec);
    func_80041188(arg0, (u8 *)&pose[0], (u8 *)&pose[1], rec->unk_68, (s32 *)0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, (s32 *)&rec->unk_F4, (s16 *)&rec->unk_1C8, 0, rec->unk_148);
    scratchpad_Restore();
    if (rec->unk_86 == rec->unk_8E && (rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x16 || rec->unk_6A == 0x30 || rec->unk_6A == 0x13 || rec->unk_6A == 0x31)) {
        rec->unk_92 = 0;
    } else {
        rec->unk_92 = rec->unk_92 != 0 ? 1 : 2;
    }
    rec->unk_62 = 0;
    if (rec->unk_0C != 0x1F && rec->unk_96 == 0 && rec->unk_92 != 0) {
        rec->unk_62 = 1;
    }
    if ((D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) && (rec->unk_0E == 0 || rec->unk_0E == 1) && (D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
        rec->unk_62 |= 2;
    }
    if (rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) {
        if (func_8002798C((u8 *)rec) == 0) {
            goto clear_8c;
        }
        goto set_8c;
    }
    if (((rec->unk_0C == 0x1D || rec->unk_0C == 0xE) && (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_A1[1] < 0xFF && rec->unk_26C != 0 && rec->unk_40 >= rec->unk_A5 && rec->unk_40 <= rec->unk_A6)
        || (rec->unk_0A == 0xE && rec->unk_6A == 0x11 && D_800A38AE == arg0 && rec->unk_40 >= rec->unk_A5 && rec->unk_A6 >= rec->unk_40)) {
    set_8c:
        rec->unk_8C = rec->unk_8C != 0 ? 1 : 2;
        rec->unk_62 |= 4;
    } else {
    clear_8c:
        rec->unk_8C = 0;
    }
    if (D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) {
        if (D_800A38DC == 3 && arg0 == 1 && D_800A384C == 4) {
            if (rec->unk_00->unk_0A == 1 || rec->unk_00->unk_0A == 3 || rec->unk_00->unk_0A == 4 || rec->unk_00->unk_0A == 9 || rec->unk_00->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else if ((D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
            if (rec->unk_0A == 1 || rec->unk_0A == 3 || rec->unk_0A == 4 || rec->unk_0A == 9 || rec->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
        } else {
            goto skip_62;
        }
        if (rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 |= 0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
            rec->unk_62 |= 0x20;
        }
    }
skip_62:
    if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
        if ((rec->unk_0E == 0 || rec->unk_0E == 1) && rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 = (rec->unk_62 | 2) & ~0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
            rec->unk_62 = (rec->unk_62 | 8) & ~0x20;
            func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, 0);
        }
    }
    if (rec->unk_62 & 1) {
        func_80049718(rec->unk_12, arg0 * 2 + 0x8000, 0, 0);
    }
    if (rec->unk_62 & 4) {
        func_80049718(D_8008EB80[rec->unk_14], arg0 * 2 + 0x8001, 0, 0);
    }
    if (rec->unk_62 & 2) {
        func_80049A2C(rec->unk_12, arg0 * 2, (rec->unk_62 >> 4) & 1);
    }
    if (rec->unk_62 & 8) {
        func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, (rec->unk_62 >> 5) & 1);
    }
    rec->unk_290 = pose[0];
    rec->unk_314 = ang[0];
    rec->unk_316 = rec->unk_64;
    if (rec->unk_31A != 0) {
        if (rec->unk_31A >= 2) {
            rec->unk_31A = 1;
        }
        rec->unk_318 += rec->unk_31C;
        if (rec->unk_318 >= 0x1000) {
            rec->unk_31A = 0;
            rec->unk_290 = pose[1];
            rec->unk_314 = ang[1];
            rec->unk_316 = rec->unk_66;
            rec->unk_1F8 = v[1];
        }
    }
    rec->unk_1D8 = ratan2(rec->unk_00->unk_F4.x - rec->unk_F4.x, rec->unk_00->unk_F4.z - rec->unk_F4.z);
    rec->unk_24C = rec->unk_104;
    func_80023D28((u8 *)rec);
    func_800207C8(rec, SPAD->unkA8[arg0], SPAD->unk00[arg0], SPAD->unk48[arg0]);
    if (rec->unk_3C == 1) {
        rec->unk_210[0] = SPAD->unk00[arg0][0];
    }
    if (rec->unk_8C == 2) {
        rec->unk_8C = 1;
        rec->unk_234[0] = SPAD->unk48[arg0][0];
        rec->unk_234[1] = SPAD->unk48[arg0][1];
    }
    if (rec->unk_92 == 2) {
        rec->unk_92 = 1;
        rec->unk_234[0] = SPAD->unk48[arg0][0];
        rec->unk_234[1] = SPAD->unk48[arg0][1];
    }
    if ((rec->unk_0E == 6 || rec->unk_0E == 7) && (rec->unk_62 & 1)) {
        rec->unk_25C = SPAD->unk00[arg0][0];
        rec->unk_268 = 1;
    } else {
        rec->unk_268 = 0;
    }
    rec->unk_114[0].vx = SPAD->unk00[arg0][0].x - rec->unk_210[0].x;
    rec->unk_114[0].vy = SPAD->unk00[arg0][0].y - rec->unk_210[0].y;
    rec->unk_114[0].vz = SPAD->unk00[arg0][0].z - rec->unk_210[0].z;
    rec->unk_114[1].vx = SPAD->unk48[arg0][0].x - rec->unk_234[0].x;
    rec->unk_114[1].vy = SPAD->unk48[arg0][0].y - rec->unk_234[0].y;
    rec->unk_114[1].vz = SPAD->unk48[arg0][0].z - rec->unk_234[0].z;
    rec->unk_1DA = func_8002FDB0((s32 *)rec);
    hit = 0;
    if ((rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_AD != 0) {
        if ((rec->unk_40 >= rec->unk_A1[0] && rec->unk_40 <= rec->unk_A3[0]) || (rec->unk_40 >= rec->unk_A1[1] && rec->unk_40 <= rec->unk_A3[1])) {
            hit = 1;
        }
    }
    rec->unk_AE = hit;
    rec->unk_288[0] = 0;
    rec->unk_288[1] = 0;
    if (rec->unk_AD != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) {
            rec->unk_288[0] += 1;
            rec->unk_288[1] += 1;
            if (rec->unk_40 < rec->unk_A1[0]) {
                rec->unk_288[0] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[0]) {
                rec->unk_288[0] += 4;
            }
            if (rec->unk_40 < rec->unk_A1[1]) {
                rec->unk_288[1] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[1]) {
                rec->unk_288[1] += 4;
            }
        }
    }
    if (rec->unk_6A == 6) {
        rec->unk_15E = 0x600;
        rec->unk_160 = 0x600;
        rec->unk_162 = 0x600;
    } else if (rec->unk_6A == 8) {
        rec->unk_15E = 0x100;
        rec->unk_160 = 0x100;
        rec->unk_162 = 0x100;
    } else {
        rec->unk_15E = 0xC00;
        rec->unk_160 = 0xC00;
        rec->unk_162 = 0xC00;
    }
    if (rec->unk_1DC != 0) {
        rec->unk_156 = 0xD00;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xD00;
    } else {
        rec->unk_156 = 0xFC0;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xFC0;
    }
    if (rec->unk_96 == 0
        && ((rec->unk_7A != 0 && rec->unk_6C != 4 && rec->unk_6C != 0x14 && (rec->unk_6A == 4 || rec->unk_6A == 0x14))
            || (rec->unk_6A == 0x11 && arg0 != D_800A38AE && rec->unk_AA == rec->unk_40))) {
        MATRIX **bones = (MATRIX **)game_GetPlayerData(arg0);
        if (D_800A38DC != 0 || D_8008D9EC[g_practice_menu_table[0].unk_0A] == 0 || D_800A37A0 != 1 || arg0 != D_800A37A0) {
            func_80032854(arg0, 0x2E, (s32 *)&rec->unk_F4, 0);
        }
        if (rec->unk_0C != 0x1F) {
            cpu_set_move_command_and_dir(rec, D_8008D90C[D_8008D9EC[rec->unk_0A]][rec->unk_0E], (Vec3i32 *)bones[0x12]->t);
        }
        rec->unk_96 = 1;
        if (D_800A3748 == -1) {
            D_800A3748 = arg0 == 0;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_0C == 0x1B && rec->unk_6A == 0xB && rec->unk_34D == 0) {
        rec->unk_286 = 0x1F;
    }
    if (rec->unk_46 == 0) {
        if (D_800A38DC != 5 && D_800A38DC != 2 && D_800A38DC != 3 && (D_800A38DC != 0 || D_800A385C == 0)) {
            if (rec->unk_6A == 0xB && rec->unk_40 == rec->unk_A7) {
                if (rec->unk_0C == 0x1B) {
                    if (rec->unk_34D != 0) {
                        rec->unk_34D--;
                    }
                } else if (rec->unk_26C != 0 && func_800307D0(rec) == rec->unk_14) {
                    rec->unk_8A = 0;
                }
            }
        }
        if (rec->unk_6A == 6) {
            if (func_80030BA8(rec) == rec->unk_14) {
                rec->unk_8A = rec->unk_26C;
            }
        } else if (rec->unk_26C != 0 && (rec->unk_6A == 0xC || rec->unk_6A == 0x2A) && rec->unk_40 == rec->unk_A8 && func_80030BA8(rec) == rec->unk_14) {
            rec->unk_8A = 1;
        }
        if (rec->unk_6A == 0x12 && rec->unk_40 == rec->unk_A7 && ((0x78 >> rec->unk_B1) & 1) && rec->unk_26C != 0) {
            func_80032064((u8 *)rec, ((0x18 >> rec->unk_B1) & 1) ? 1 : 2);
        }
        if (rec->unk_6A == 0x10 && rec->unk_40 == rec->unk_AC && rec->unk_26C != 0 && rec->unk_34B != 0) {
            rec->unk_34A = 0xA;
            rec->unk_34B--;
            func_80032854(arg0, 0x31, (s32 *)&rec->unk_F4, 0);
        }
        if (rec->unk_46 == 0 && (rec->unk_6A != 0x11 || arg0 == D_800A38AE)) {
            rec->unk_62 |= 0x40;
            cpu_check_same_dir_timer((s32 *)rec);
        }
    }
    if (rec->unk_26C != 0) {
        rec->unk_62 |= 0x80;
    }
    if (rec->unk_96 != 0) {
        if (D_800A3834 != 0xD) {
            rec->unk_B3 = 4;
        }
    } else {
        rec->unk_B3 = 0;
    }
    if (rec->unk_0C == 0x1F) {
        rec->unk_B3 = 4;
    }
    func_80040304(arg0, rec->unk_B3);
    func_800204C0(rec);
    if (rec->unk_7A == 2) {
        rec->unk_7A = 0;
    }
    func_80039680((u8 *)rec);
}
