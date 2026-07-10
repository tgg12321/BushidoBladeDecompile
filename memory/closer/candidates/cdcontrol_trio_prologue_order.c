/* Closer session 2026-07-09 (phase 3): CdControl/CdControlF/CdControlB honest
   candidates. Floors measured with sandbox --disable all:
     func_80080258 (CdControl)  = 8   (HEAD honest floor was 25, s7 pin)
     func_80080390 (CdControlF) = 6   (HEAD honest floor was 23, s6 pin)
     tslPolyF4Init (CdControlB) = 8   (HEAD floor 8)
   Residual (ALL three, identical family): prologue def order � target homes
   a1/a2 before a0 and places li s0,3 (count=3) right after the homes, before
   the andi; our sched2 launch heuristic drifts li s0,3 to block end and puts
   the a0 home first (its consumer chain andi->sll->addu). HEAD papers over
   this with whole-prologue overrides in tools/prologue_config.json (all three
   funcs) + regfix reorder/rotation rules for 258.
   KEY LEVERS THAT WORKED (keep for the finish):
   - v1.86 body is an accumulator form: ret=0; ... ret=-1; return ret+1
     (target has move s7,zero prologue + addiu v0,s7,1 twice) � NOT v1.77's
     `return cd_cw(...)==0`.
   - ret lands s7 ONLY with a real loop (while(1), loop-weighted refs);
     goto-loop form allocates ret=s2. CdControlB has NO accumulator; its
     goto-form cd_cw (static inline, count=3, bottom `count != -1`) matches.
   - LICM hoist of the in-loop constants 1 / -1 (li s8,...) killed via
     defeat-licm-hoist-var-reuse: route both through multi-set local t that
     also captures the main CD_cw call result (target reuses v0).
   - decl order idx, base, old, elem, count reproduces target init sequence
     (base la BEFORE old lw; count decl elsewhere flips count to s1).
   - staged-value (t=3; count=t) does NOT move the li s0,3 (coalesced).
   NEXT PROBE for the residual: sched2 dump shows all-flat priorities; the
   li s0,3 (insn 19) gets blocking/launch treatment each cycle; the a1/a2 vs
   a0 home tie breaks by LUID desc backward => a0 first forward. Need a C
   shape that (a) removes the launch classification of the count init or
   (b) gives a1/a2 homes an earlier consumer. Same family as the
   func_8007C2A0/C4B8 prologue twin wall.
*/
/* PsyQ 4.0 LIBCD sys: cd_cw — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
static inline s32 cd_cw(s32 com, u8 *param, u8 *result, s32 arg3) {
    s32 idx = com & 0xFF;
    s32 *base = g_cd_sector_buf;
    s32 old = g_cd_callback_a;
    s32 *elem = base + idx;
    s32 count = 3;

loop:
    g_cd_callback_a = 0;
    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            tslTm2LoadImage(1, 0, 0, 0);
        }
    }
    if (param != 0) {
        if (*elem != 0) {
            if (tslTm2LoadImage(2, param, result, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = old;
    if (tslTm2LoadImage(com & 0xFF, param, result, arg3) == 0) {
        return 0;
    }
next:
    count--;
    if (count != -1) {
        goto loop;
    }
    g_cd_callback_a = old;
    return -1;
}

/* PsyQ 4.0 LIBCD sys: CdControl — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c (v1.86 body
   carries an explicit ret accumulator, unlike the v1.77 == 0 form) */
s32 func_80080258(s32 com, u8 *param, u8 *result) {
    s32 idx = com & 0xFF;
    s32 *base = g_cd_sector_buf;
    s32 old = g_cd_callback_a;
    s32 *elem = base + idx;
    s32 count = 3;
    s32 ret = 0;
    s32 t;

    while (1) {
        g_cd_callback_a = 0;
        t = 1;
        if (idx != t) {
            if (g_cd_mode & 0x10) {
                tslTm2LoadImage(1, 0, 0, 0);
            }
        }
        if (param != 0) {
            if (*elem != 0) {
                if (tslTm2LoadImage(2, param, result, 0) != 0) {
                    goto next;
                }
            }
        }
        g_cd_callback_a = old;
        t = tslTm2LoadImage(com & 0xFF, param, result, 0);
        if (t == 0) {
            goto done;
        }
    next:
        count--;
        t = -1;
        if (count == t) {
            break;
        }
    }
    g_cd_callback_a = old;
    ret = -1;
done:
    return ret + 1;
}

/* PsyQ 4.0 LIBCD sys: CdControlF — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c (v1.86 body
   carries an explicit ret accumulator, unlike the v1.77 == 0 form) */
s32 func_80080390(s32 com, u8 *param) {
    s32 idx = com & 0xFF;
    s32 *base = g_cd_sector_buf;
    s32 old = g_cd_callback_a;
    s32 *elem = base + idx;
    s32 count = 3;
    s32 ret = 0;
    s32 t;

    while (1) {
        g_cd_callback_a = 0;
        t = 1;
        if (idx != t) {
            if (g_cd_mode & 0x10) {
                tslTm2LoadImage(1, 0, 0, 0);
            }
        }
        if (param != 0) {
            if (*elem != 0) {
                if (tslTm2LoadImage(2, param, 0, 0) != 0) {
                    goto next;
                }
            }
        }
        g_cd_callback_a = old;
        t = tslTm2LoadImage(com & 0xFF, param, 0, 1);
        if (t == 0) {
            goto done;
        }
    next:
        count--;
        t = -1;
        if (count == t) {
            break;
        }
    }
    g_cd_callback_a = old;
    ret = -1;
done:
    return ret + 1;
}

/* PsyQ 4.0 LIBCD sys: CdControlB — verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libcd/sys.c */
s32 tslPolyF4Init(s32 com, u8 *param, u8 *result) {
    if (cd_cw(com, param, result, 0)) {
        return 0;
    }
    return cpu_side_move_dir_4(0, result) == 2;
}
