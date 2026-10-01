#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

/* Declarations from the file this TU was split from (text1b_b.c). */
#define NULL ((void *)0)
extern s32 column;
s32 strlen(u8 *a0);

/* D_80015A68: 1 string(s), 20B @ 0x80015A68 */
const char D_80015A68[20] =
    "0123456789abcdef\0\0\0\0"
    ;

/* D_80015A7C: 1 string(s), 8B @ 0x80015A7C */
const char D_80015A7C[8] =
    "(null)\0\0"
    ;

/* D_80015A84: 1 string(s), 20B @ 0x80015A84 */
const char D_80015A84[20] =
    "0123456789ABCDEF\0\0\0\0"
    ;

/* PsyQ 4.0 LIBC2 PRNT: prnt — verbatim-linked Sony object (census 2026-07-09);
 * C ref: 4.3BSD-Tahoe _doprnt, lib/libc/stdio/doprnt.c
 * "@(#)doprnt.c 5.35 (Berkeley) 6/27/88", with the FILE buffering replaced by
 * putchar and the floating-point conversions removed. The version is fixed by
 * ARG(): the target sign-extends %h for o/u/x (lh at all four ARG sites), which
 * is 5.35's va_arg(argp, short); 5.36 (1988-10-24) changed that to
 * (short unsigned)va_arg(argp, int). Transcription diff and provenance:
 * pre-slim-2026-10-01:memory/grind/prnt/evidence.md. BB2's build does not count ordinary
 * characters in the return value. The digit/"(null)" strings are the named
 * arrays above rather than literals: this file also holds LIBC SPRINTF (a
 * separate object in the original link), and GCC pools identical string
 * literals within one translation unit, which would fold sprintf's two digit
 * strings into these and drop 40 bytes of .rodata. The switch table is
 * compiler-emitted. */
#define PRNT_LONGINT 0x01
#define PRNT_LONGDBL 0x02
#define PRNT_SHORTINT 0x04
#define PRNT_ALT 0x08
#define PRNT_LADJUST 0x10
#define PRNT_ZEROPAD 0x20
#define PRNT_HEXPREFIX 0x40
#define PRNT_BUF 40

extern u8 _ctype__plus_0x1;
#define isascii(c) ((u32)(c) <= 0177)
#define isdigit(c) ((&_ctype__plus_0x1)[c] & 4)
#define todigit(c) ((c) - '0')

#define __va_rounded_size(TYPE) (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define prnt_va_arg(AP, TYPE) \
    (AP += __va_rounded_size(TYPE), *((TYPE *)(AP - __va_rounded_size(TYPE))))
#define PRNT_ARG() \
    _ulong = flags & PRNT_LONGINT ? prnt_va_arg(argp, long) : \
        flags & PRNT_SHORTINT ? prnt_va_arg(argp, short) : prnt_va_arg(argp, int)

s32 prnt(s32 fd, u8 *fmt0, char *argp) {
    u8 *fmt;
    s32 ch;
    s32 cnt;
    /* n is the original source's own variable, reused verbatim (owner
     * Ruling 10, .claude/rules/ordinary-c-judge-decidable.md, c3e7a0b9e):
     * 4.3BSD-Tahoe doprnt.c 5.35 :64 "register int n; random handy integer",
     * https://github.com/dspinellis/unix-history-repo/blob/b98826995697c37ced684813f008619460bd7ff8/usr/src/lib/libc/stdio/doprnt.c
     * Written/read at :148 :150 :152 :155 (precision digits), :167 :169 :171
     * (width digits), :359 :370 :373 :390 (padding loops), :377 :383 (string
     * length); its FILE-buffer uses (:92-107, :379-381) are absent with the
     * FILE code. Evidence: pre-slim-2026-10-01:memory/grind/prnt/evidence.md. */
    s32 n;
    char *t;
    u32 _ulong;
    s32 base;
    s32 dprec;
    s32 fieldsz;
    s32 flags;
    s32 fpprec;
    s32 prec;
    s32 realsz;
    s32 size;
    s32 width;
    char sign;
    const char *digs;
    char buf[PRNT_BUF];

    if (fmt0 == NULL) {
        return 0;
    }
    fmt = fmt0;
    digs = D_80015A68;
    for (cnt = 0;; ++fmt) {
        if (!(ch = *fmt)) {
            return cnt;
        }
        if (ch != '%') {
            putchar(ch);
            continue;
        }

        flags = 0; dprec = 0; fpprec = 0; width = 0;
        prec = -1;
        sign = '\0';

rflag:
        switch (*++fmt) {
        case ' ':
            if (!sign) {
                sign = ' ';
            }
            goto rflag;
        case '#':
            flags |= PRNT_ALT;
            goto rflag;
        case '*':
            if ((width = prnt_va_arg(argp, s32)) >= 0) {
                goto rflag;
            }
            width = -width;
            /* FALLTHROUGH */
        case '-':
            flags |= PRNT_LADJUST;
            goto rflag;
        case '+':
            sign = '+';
            goto rflag;
        case '.':
            if (*++fmt == '*') {
                n = prnt_va_arg(argp, s32);
            } else {
                n = 0;
                while (isascii(*fmt) && isdigit(*fmt)) {
                    n = 10 * n + todigit(*fmt++);
                }
                --fmt;
            }
            prec = n < 0 ? -1 : n;
            goto rflag;
        case '0':
            flags |= PRNT_ZEROPAD;
            goto rflag;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            n = 0;
            do {
                n = 10 * n + todigit(*fmt);
            } while (isascii(*++fmt) && isdigit(*fmt));
            width = n;
            --fmt;
            goto rflag;
        case 'L':
            flags |= PRNT_LONGDBL;
            goto rflag;
        case 'h':
            flags |= PRNT_SHORTINT;
            goto rflag;
        case 'l':
            flags |= PRNT_LONGINT;
            goto rflag;
        case 'c':
            *(t = buf) = prnt_va_arg(argp, s32);
            size = 1;
            sign = '\0';
            goto pforw;
        case 'D':
            flags |= PRNT_LONGINT;
            /* FALLTHROUGH */
        case 'd':
        case 'i':
            PRNT_ARG();
            if ((long)_ulong < 0) {
                _ulong = -_ulong;
                sign = '-';
            }
            base = 10;
            goto number;
        case 'n':
            if (flags & PRNT_LONGINT) {
                *prnt_va_arg(argp, long *) = cnt;
            } else if (flags & PRNT_SHORTINT) {
                *prnt_va_arg(argp, short *) = cnt;
            } else {
                *prnt_va_arg(argp, s32 *) = cnt;
            }
            break;
        case 'O':
            flags |= PRNT_LONGINT;
            /* FALLTHROUGH */
        case 'o':
            PRNT_ARG();
            base = 8;
            goto nosign;
        case 'p':
            _ulong = (u32)prnt_va_arg(argp, void *);
            base = 16;
            goto nosign;
        case 's':
            if (!(t = prnt_va_arg(argp, char *))) {
                t = (char *)D_80015A7C;
            }
            if (prec >= 0) {
                char *p;

                if (p = memchr(t, 0, prec)) {
                    size = p - t;
                    if (size > prec) {
                        size = prec;
                    }
                } else {
                    size = prec;
                }
            } else {
                size = strlen(t);
            }
            sign = '\0';
            goto pforw;
        case 'U':
            flags |= PRNT_LONGINT;
            /* FALLTHROUGH */
        case 'u':
            PRNT_ARG();
            base = 10;
            goto nosign;
        case 'X':
            digs = D_80015A84;
            /* FALLTHROUGH */
        case 'x':
            PRNT_ARG();
            base = 16;
            if (flags & PRNT_ALT && _ulong != 0) {
                flags |= PRNT_HEXPREFIX;
            }
nosign:
            sign = '\0';
number:
            if ((dprec = prec) >= 0) {
                flags &= ~PRNT_ZEROPAD;
            }
            t = buf + PRNT_BUF;
            if (_ulong != 0 || prec != 0) {
                do {
                    *--t = digs[_ulong % base];
                    _ulong /= base;
                } while (_ulong);
                digs = D_80015A68;
                if (flags & PRNT_ALT && base == 8 && *t != '0') {
                    *--t = '0';
                }
            }
            size = buf + PRNT_BUF - t;
pforw:
            fieldsz = size + fpprec;
            if (sign) {
                fieldsz++;
            }
            if (flags & PRNT_HEXPREFIX) {
                fieldsz += 2;
            }
            realsz = dprec > fieldsz ? dprec : fieldsz;

            if ((flags & (PRNT_LADJUST | PRNT_ZEROPAD)) == 0 && width) {
                for (n = realsz; n < width; n++) {
                    putchar(' ');
                }
            }
            if (sign) {
                putchar(sign);
            }
            if (flags & PRNT_HEXPREFIX) {
                putchar('0');
                putchar((char)*fmt);
            }
            if ((flags & (PRNT_LADJUST | PRNT_ZEROPAD)) == PRNT_ZEROPAD) {
                for (n = realsz; n < width; n++) {
                    putchar('0');
                }
            }
            for (n = fieldsz; n < dprec; n++) {
                putchar('0');
            }
            n = size;
            while (--n >= 0) {
                putchar(*t++);
            }
            while (--fpprec >= 0) {
                putchar('0');
            }
            if (flags & PRNT_LADJUST) {
                for (n = realsz; n < width; n++) {
                    putchar(' ');
                }
            }
            cnt += width > realsz ? width : realsz;
            break;
        case '\0':
            return cnt;
        default:
            putchar((char)*fmt);
            cnt++;
        }
    }
}
extern u8 _ctype__plus_0x1;
u8 toupper(u8 a0) {
    u8 c = a0;
    if ((&_ctype__plus_0x1)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}
extern u8 _ctype__plus_0x1;
u8 tolower(u8 a0) {
    u8 c = a0;
    if ((&_ctype__plus_0x1)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
u8 *memchr(u8 *buf, s32 ch, s32 len) {
    if (buf == 0) return 0;
    if (len <= 0) return 0;
    len--;
    goto check;
found:
    return buf - 1;
check:
    if (len < 0) return 0;
    ch &= 0xFF;
loop:
    if (*buf++ == ch) goto found;
    --len;
    if (len >= 0) goto loop;
    return 0;
}
void write(s32, u8 *, s32);
void putchar(s8 arg0) {
    u8 sp10;
    s32 temp_a0;

    sp10 = arg0;
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 == 9) goto loop;
    if (temp_a0 == 0xA) {
        putchar(0xD);
        column = 0;
        goto tail;
    }
    goto def;
loop:
    putchar(0x20);
    if ((column & 7) == 0) return;
    goto loop;
def:
    if ((&_ctype__plus_0x1)[temp_a0] & 0x97) {
        column += 1;
    }
tail:
    write(1, &sp10, 1);
}
