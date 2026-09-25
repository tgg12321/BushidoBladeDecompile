/* PsyQ 4.0 LIBC2 PRNT: prnt — verbatim-linked Sony object (census 2026-07-09);
 * C ref: 4.3BSD-Reno _doprnt (vfprintf.c), FILE buffering replaced by putchar.
 * BB2's build does not count ordinary characters in the return value. The
 * digit/"(null)" strings are the named arrays above rather than literals:
 * this file also holds LIBC SPRINTF (a separate object in the original link),
 * and GCC pools identical string literals within one translation unit, which
 * would fold sprintf's two digit strings into these and drop 40 bytes of
 * .rodata. The switch table is compiler-emitted (jtbl_80015A98). */
#define PRNT_LONGINT 0x01
#define PRNT_LONGDBL 0x02
#define PRNT_SHORTINT 0x04
#define PRNT_ALT 0x08
#define PRNT_LADJUST 0x10
#define PRNT_ZEROPAD 0x20
#define PRNT_HEXPREFIX 0x40
#define PRNT_BUF 40

extern u8 _ctype__plus_0x1;
#define isascii(c) ((u32)(c) < 0x80)
#define isdigit(c) ((&_ctype__plus_0x1)[(u8)(c)] & 4)
#define todigit(c) ((c) - '0')

#define __va_rounded_size(TYPE) (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define prnt_va_arg(AP, TYPE) \
    (AP += __va_rounded_size(TYPE), *((TYPE *)(AP - __va_rounded_size(TYPE))))
#define PRNT_ARG() \
    _ulong = flags & PRNT_LONGINT ? prnt_va_arg(argp, long) : \
        flags & PRNT_SHORTINT ? (long)prnt_va_arg(argp, short) : (long)prnt_va_arg(argp, int)

s32 prnt(s32 fd, u8 *fmt0, char *argp) {
    u8 *fmt;
    s32 ch;
    s32 cnt;
    s32 precn;
    s32 widthn;
    s32 padn;
    s32 zeron;
    s32 left;
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
                precn = prnt_va_arg(argp, s32);
            } else {
                precn = 0;
                while (isascii(*fmt) && isdigit(*fmt)) {
                    precn = 10 * precn + todigit(*fmt++);
                }
                --fmt;
            }
            prec = precn < 0 ? -1 : precn;
            goto rflag;
        case '0':
            flags |= PRNT_ZEROPAD;
            goto rflag;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            widthn = 0;
            do {
                widthn = 10 * widthn + todigit(*fmt);
            } while (isascii(*++fmt) && isdigit(*fmt));
            width = widthn;
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
                for (padn = realsz; padn < width; padn++) {
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
                for (padn = realsz; padn < width; padn++) {
                    putchar('0');
                }
            }
            for (zeron = fieldsz; zeron < dprec; zeron++) {
                putchar('0');
            }
            left = size;
            while (--left >= 0) {
                putchar(*t++);
            }
            while (--fpprec >= 0) {
                putchar('0');
            }
            if (flags & PRNT_LADJUST) {
                for (padn = realsz; padn < width; padn++) {
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
