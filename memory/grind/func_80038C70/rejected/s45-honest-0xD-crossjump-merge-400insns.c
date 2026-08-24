/* REJECTED (s45, 2026-08-24) — the HONEST 0xD form: `case 9: case 11: sel = 0xD;`
 * (the semantic truth that regfix.txt:1095 fakes with subst 12->13 @ 149).
 *
 * WHY IT IS DEAD, re-measured on the CURRENT chassis (post cc1 fork-crash fix,
 * decisions.md:10340):  sandbox func_80038C70 --disable all  =>  score 2,
 * target_insns 402, build_insns 400.  jump2 cross-jumping merges the two identical
 * tails [ (set s0 13) ; (jump -> label 444) ] at arm A (if-chain, target 0x80038DA8)
 * and arm B (switch case 9/11, target 0x80038EC8), deleting BOTH insns from one arm.
 *
 * Mechanism named at GCC-source level (tools/gcc-2.7.2/jump.c:1996 gate
 * `if (cross_jump && simplejump_p (insn))`, and find_cross_jump at jump.c:2403ff).
 * The 0xF pair survives in the target ONLY because its if-chain arm ends in a
 * conditional {branch_equality} (not simplejump_p); both 0xD arms are unconditional
 * `j .L80038EDC` (0803E3B7) in the shipped bytes, so that protection is unavailable.
 * Full enumeration of the breaker set: evidence.md [s45].
 */
s32 func_80038C70(void) {
    extern u8 D_800A3207;
    extern u8 D_800A334C;
    extern u8 D_800A3350;
    extern u8 D_800A3354;
    extern u8 D_800A3206;
    extern u8 D_800A3340;
    extern u8 D_800A3344;
    extern u8 D_800A3348;
    extern void func_8006BEC4(s32, s32);
    extern void func_8005C650(s32, s32, s32);
    extern void func_8003877C(void);
    extern void func_8003879C(void);
    extern void func_800387C0(void);
    extern void func_800387E8(void);
    s32 result = 0;
    s32 sel2 = -1;
    s32 v0;
    s32 sel;

    if (!D_800A3207) {
        D_800A3207 = 1;
        D_800A334C = 0x5A;
        D_800A3350 = 0;
        D_800A3354 = 0;
        D_800A31FC = 0;
    }

    v0 = func_80038734();

    if (D_800A3354 != 0) {
        func_8006BEC4(0xA, -1);
        D_800A334C--;
        if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
            func_8005C650(2, 0x7F, 0x7F);
            D_800A3207 = 1;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            D_800A3354 = 0;
            D_800A31FC = 0;
        }
        goto end;
    }

    if (D_800A3207 == 1) {
        v0 = 0;
    } else if (D_800A3207 == 2) {
    } else if (D_800A3207 == 3) {
    } else if (D_800A3207 == 4) {
        v0 = 0x11;
    }

    sel = 0;
    if (D_800A31FC != 0) {
        goto sel_dispatch;
    }

    if (D_800A3207 == 3) {
        if (v0 == 8) {
            goto case8_sel;
        }
        if (v0 != 10) {
            sel = 0xD;
        } else {
            sel = 0xF;
        }
        goto sel_dispatch;
    }

    switch (v0) {
    case 0:
        sel = 0x11;
        sel2 = D_800A3350; /* FAKE: duplicate of load_sel2's store — jump2 cross-jump re-merges it
                              (zero emitted bytes); the extra real def lifts sel2's reg_n_refs
                              priority above result's so RA lands sel2->$s2 / result->$s3 (target).
                              SOTN duplicate-into-arms family. */
        goto sel_dispatch;
    case 13:
    case 17:
        sel = 6;
    load_sel2:
        sel2 = D_800A3350;
        goto sel_dispatch;
    case 1:
        sel = (-(D_800A38CC != 0)) & 7;
        goto sel_dispatch;
    case 2:
        sel = 8;
        goto sel_dispatch;
    case 3:
        sel = 9;
        goto sel_dispatch;
    case 8:
    case8_sel:
        sel = -1;
        goto sel_dispatch;
    case 7:
        sel = 5;
        goto sel_dispatch;
    case 10:
        if (D_800A3206 == 0) {
            D_800A3350 = 1;
        }
        if (D_80102794 & 0x400040) {
            D_800A3206 = 0;
            func_8005C650(1, 0x7F, 0x7F);
            sel = 0xD;
            if (D_800A3350 == 0) {
                func_800387E8();
                goto sel_dispatch;
            }
            v0 = 0;
            D_800A3207 = 5;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            sel = -1;
            goto sel_dispatch;
        }
        sel = 0xC;
        if ((D_80102794 & 0xA000A000U) != 0) {
            D_800A3206 = 1;
        }
        goto load_sel2;
    case 9:
    case 11:
        sel = 0xD;  /* honest value - merges, 400 insns */
        goto sel_dispatch;
    case 12:
        sel = 0xF;
        goto sel_dispatch;
    default:
        sel = 0;
        goto sel_dispatch;
    }

sel_dispatch:
    if (sel >= 0) {
        func_8006BEC4(sel, sel2);
    }

    if (D_800A31FC != 0) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            D_800A3348++;
            if (((u8)D_800A3348) >= 5) {
                D_800A3354 = 1;
                D_800A334C = 0x5A;
                break;
            }
            func_8003877C();
            break;
        case 1: case 2: case 3:
        case 9: case 10: case 11:
            D_800A3340++;
            if (((u8)D_800A3340) >= 5) {
                D_800A31FC = 0;
                func_8003879C();
                break;
            }
            func_8003877C();
            break;
        case 6:
            D_800A3344++;
            if (((u8)D_800A3344) >= 5) {
                D_800A31FC = 0;
                break;
            }
            func_8003877C();
            break;
        default:
            break;
        }
    } else if (D_800A3207 == 3) {
        switch (v0 - 4) {
        case 0:
            break;
        case 4:
            result = 1;
            break;
        case 1: case 2: case 3:
        case 5: case 7: case 8: case 9: case 10: case 11:
        default:
            func_8003879C();
            D_800A3207 = 2;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 6:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
                result = 1;
                break;
            }
            break;
        }
    } else {
        switch (v0) {
        case 8:
            D_800A3354 = 1;
            D_800A334C = 0x5A;
            break;
        case 2: case 3: case 7: case 12:
            D_800A334C--;
            if (((u8)D_800A334C) == 0 || (D_80102794 & 0x100010)) {
                func_8005C650(2, 0x7F, 0x7F);
                if (v0 != 7) {
                    result = 1;
                    break;
                }
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                D_800A3354 = 0;
                D_800A31FC = 0;
            }
            break;
        case 11:
            D_800A31FC = 1;
            func_8003877C();
            D_800A3207 = 3;
            D_800A334C = 0x5A;
            break;
        case 13:
            D_800A3207 = 4;
            D_800A334C = 0x5A;
            D_800A3350 = 0;
            break;
        case 0:
            if (D_800A3207 == 5) {
                D_800A3207 = 1;
                break;
            }
            goto sw4_L2;
        case 17:
        sw4_L2:
            if (D_80102794 & 0x400040) {
                func_8005C650(1, 0x7F, 0x7F);
                if (v0 == 0) {
                    if (D_800A3350 != 0) {
                        result = 1;
                        break;
                    }
                    D_800A3348 = 0;
                    D_800A3340 = 0;
                    D_800A3344 = 0;
                    D_800A31FC = 1;
                    func_8003877C();
                } else {
                    if (D_800A3350 != 0) goto area_c_long;
                    func_800387C0();
                }
                D_800A3207 = 2;
                break;
            area_c_long:
                v0 = 0;
                D_800A3207 = 1;
                D_800A334C = 0x5A;
                D_800A3350 = 0;
                break;
            }
            goto sw4_buttons;
        case 10:
        sw4_buttons:
            if (D_80102794 & 0x80008000U) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3350 = 0;
                break;
            }
            if (D_80102794 & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3350 = 1;
            }
            break;
        default:
            break;
        }
    }

    if (v0 < 11) {
        if (v0 >= 9) {
            goto d_check;
        }
        if (v0 == 1) {
            goto d_check;
        }
        goto end;
    }
    if (v0 != 17) {
        goto end;
    }
d_check:
    if (D_800A31F8 == -1) {
        D_800A3354 = 1;
        D_800A334C = 0x5A;
    }

end:
    if (result != 0) {
        D_800A3207 = 0;
        D_800A31FC = 0;
    }
    return result;
}
