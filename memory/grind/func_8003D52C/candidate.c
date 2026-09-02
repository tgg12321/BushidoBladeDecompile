/* func_8003D52C — session 1 (recon), 2026-09-02. sandbox --disable all = 0 (146/146 insns).
 * Load-bearing spellings (both measured this session):
 *  1. va_start MUST be the address form ((ap) = (va_list)(&(last) + 1)) — the same macro
 *     src/main.c:1707 already uses. Taking &first_arg makes GCC 2.7.2 put_var_into_stack the
 *     parm pseudo, which emits the `sw $s5,0x53C($sp)` store of first_arg into its home slot
 *     (the exact insn the retired regfix rule inserted). __builtin_next_arg form measures d1.
 *  2. ONE pointer local `p` serves both loops (segment writer, then buf reader). Two separate
 *     pointer locals measure d38: the segment pointer does not cross a call by itself, so
 *     global.c gives it $v0; merged with the second-loop pointer (which crosses
 *     func_8003D39C) the shared pseudo is call-crossing and takes $s1 in priority order.
 * Ordinary C only — no FAKE constructs.
 */
void func_8003D52C(u8 *fmt, s32 first_arg, ...) {
    u8 buf[0x400];
    u8 seg[0x100];
    va_list ap;
    s32 cur_arg;
    s32 seen_pct;
    u8 *p;
    s32 ch;

    cur_arg = first_arg;
    seen_pct = 0;
    p = seg;
    va_start(ap, first_arg);
    buf[0] = 0;

    while ((ch = *fmt++) != 0) {
        if (ch == '%') {
            if (seen_pct == 0) {
                seen_pct = 1;
            } else {
                *p = 0;
                sprintf(buf + strlen(buf), seg, cur_arg);
                p = seg;
                cur_arg = va_arg(ap, s32);
            }
        }
        *p++ = ch;
    }

    *p = 0;
    sprintf(buf + strlen(buf), seg, cur_arg);

    p = buf;
    while ((ch = *p++) != 0) {
        s32 row = D_800A3360;
        if (row >= 0x1A) break;

        if (ch == ' ') {
            D_800A335C++;
        } else if (ch == '\n') {
            D_800A335C = 0;
            D_800A3360 = row + 1;
        } else if (ch == '~') {
            ch = *p++;
            if (ch == 0) break;
            if (ch == 'c' || ch == 'C') {
                s32 d1, d2, d3;
                d1 = *p++;
                if (d1 == 0) break;
                d2 = *p++;
                if (d2 == 0) break;
                d3 = *p++;
                if (d3 == 0) break;
                D_800A3364 = ((d1 - '0') << 5) | ((d2 - '0') << 13) | ((d3 - '0') << 21);
            }
        } else {
            func_8003D39C(D_800A335C * 8 + 0x10, row * 8 + 0x10, ch, D_800A3364);
            D_800A335C++;
        }
        if (D_800A335C >= 0x4C) {
            D_800A335C = 0;
            D_800A3360++;
        }
    }
}
