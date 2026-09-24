/* MEASUREMENT: SOTN src/main/psxsdk/libc/sprintf.c (tmp/sotn @8bd7c77) adapted to BB2 names */
#define __va_rounded_size(TYPE) (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(AP, LASTARG) (AP = ((char*)&(LASTARG) + __va_rounded_size(LASTARG)))
#define va_arg(AP, TYPE) (AP = ((char*)(AP)) += __va_rounded_size(TYPE), *((TYPE*)((char*)(AP) - __va_rounded_size(TYPE))))
typedef void *va_list_s;
#define va_list va_list_s
#define LOH(x) (*(s16*)&(x))
#define LOW(x) (*(s32*)&(x))
extern const char D_80015C7C[], D_80015C90[];
extern void *memmove();
typedef struct {
    u32 leftJustified : 1;
    u32 prependPlus : 1;
    u32 alternativeForm : 1;
    u32 leadingZeros : 1;
    u32 usePrecision : 1;
    u32 isHalf : 1;
    u32 isLong : 1;
    u32 isLongLong : 1;
    char leadingChar;
    s32 width;
    s32 precision;
} printf_info; /* size = 0xC */

extern printf_info D_8009BE10;

s32 sprintf(char* out, char* f, ...) {
    char buf[0x200];
    printf_info info;
    va_list args;
    char* hexChars;
    s32 written;
    s32 num;
    s32 len;
    char* bufPtr;
    u32 ch;

    va_start(args, f);
    ch = *f;
    written = 0;
    for (; ch = *f, ch != 0; ++f) {
        if (ch != '%') {
            out[written++] = ch;
            continue;
        }
        info = D_8009BE10;

        while (1) {
            ch = *++f;
            if (ch == '-') {
                info.leftJustified = 1;
            } else if (ch == '+') {
                info.prependPlus = 1;
            } else if (ch == ' ') {
                info.leadingChar = ' ';
            } else if (ch == '#') {
                info.alternativeForm = 1;
            } else if (ch == '0') {
                info.leadingZeros = 1;
            } else {
                break;
            }
        }

        if (ch == '*') {
            info.width = va_arg(args, s32);
            if (info.width < 0) {
                info.width = -info.width;
                info.leftJustified = 1;
            }
            ch = *++f;
        } else {
            while (ch >= '0' && ch <= '9') {
                info.width = (info.width * 10) + (ch - '0');
                ch = *++f;
            }
        }
        if (ch == '.') {
            ch = *++f;
            if (ch == '*') {
                info.precision = va_arg(args, s32);
                ch = *++f;
            } else {
                while (ch >= '0' && ch <= '9') {
                    info.precision = (info.precision * 10) + (ch - '0');
                    ch = *++f;
                }
            }
            if (info.precision >= 0) {
                info.usePrecision = 1;
            }
        }

        // This points to &buf[0x200 - 4]. Need to use args
        // to force args on the stack
        bufPtr = (char*)&args - sizeof(printf_info) - 4;

        if (info.leftJustified) {
            info.leadingZeros = 0;
        }

    loop_30:
        switch (ch) {
        case 'h':
            info.isHalf = 1;
            ch = *++f;
            goto loop_30;

        case 'l':
            info.isLong = 1;
            ch = *++f;
            goto loop_30;

        case 'L':
            info.isLongLong = 1;
            ch = *++f;
            goto loop_30;

        case 'd':
        case 'i':
            num = va_arg(args, s32);
            do {
                if (info.isHalf) {
                    num = (s16)num;
                }
            } while (0);
            if (num < 0) {
                num = -num;
                info.leadingChar = '-';
            } else if (info.prependPlus) {
                info.leadingChar = '+';
            }
            goto printDec;

        case 'u':
            num = va_arg(args, u32);
            do {
                if (info.isHalf) {
                    num = (u16)num;
                }
            } while (0);
            info.leadingChar = '\0';
        printDec:
            if (!info.usePrecision) {
                if (info.leadingZeros) {
                    info.precision = info.width;
                    if (info.leadingChar != '\0') {
                        info.precision = info.width - 1;
                    }
                }
                if (info.precision <= 0) {
                    info.precision = 1;
                }
            }
            len = 0;
            while (num != 0) {
                *--bufPtr = (num % 10U) + '0';
                num /= 10U;
                len++;
            }
            while (len < info.precision) {
                *--bufPtr = '0';
                len++;
            }
            if (info.leadingChar != '\0') {
                *--bufPtr = info.leadingChar;
                len++;
            }
            break;

        case 'o':
            num = va_arg(args, u32);
            do {
                if (info.isHalf) {
                    num = (u16)num;
                }
            } while (0);
            if (!info.usePrecision) {
                if (info.leadingZeros) {
                    info.precision = info.width;
                }
                if (info.precision <= 0) {
                    info.precision = 1;
                }
            }
            len = 0;
            while (num != 0) {
                *--bufPtr = (num % 8U) + '0';
                num /= 8U;
                len++;
            }
            if (info.alternativeForm && (len != 0) && (*bufPtr != '0')) {
                *--bufPtr = '0';
                len++;
            }
            while (len < info.precision) {
                *--bufPtr = '0';
                len++;
            }
            break;

        case 'p':
            info.precision = 8;
            info.usePrecision = 1;
            info.isLong = 1;
            /* fallthrough */
        case 'X':
            hexChars = (char *)D_80015C7C;
            goto printHex;
        case 'x':
            hexChars = (char *)D_80015C90;
        printHex:
            num = va_arg(args, u32);
            do {
                if (info.isHalf) {
                    num = (u16)num;
                }
            } while (0);
            if (!info.usePrecision) {
                if (info.leadingZeros) {
                    info.precision = info.width;
                    if (info.alternativeForm) {
                        info.precision = info.width - 2;
                    }
                }
                if (info.precision <= 0) {
                    info.precision = 1;
                }
            }
            len = 0;
            while (num != 0) {
                *--bufPtr = hexChars[num % 16U];
                num /= 16U;
                len++;
            }
            while (len < info.precision) {
                *--bufPtr = '0';
                len++;
            }
            if (info.alternativeForm) {
                *--bufPtr = ch;
                *--bufPtr = '0';
                len += 2;
            }
            break;

        case 'c':
            *--bufPtr = va_arg(args, s32);
            len = 1;
            break;

        case 's':
            bufPtr = va_arg(args, char*);
            if (info.alternativeForm) {
                len = *bufPtr++;
                if (info.usePrecision) {
                    if (info.precision < len) {
                        len = info.precision;
                    }
                }
            } else if (!info.usePrecision) {
                len = strlen(bufPtr);
            } else {
                char* ptr = memchr(bufPtr, 0, info.precision);
                len = ptr - bufPtr;
                if (ptr == 0) {
                    len = info.precision;
                }
            }
            break;

        case 'n':
            bufPtr = va_arg(args, s32*);
            if (info.isHalf) {
                LOH(*bufPtr) = written;
            } else if (info.isLong) {
                LOW(*bufPtr) = written;
            } else if (info.isLongLong) {
                LOW(*bufPtr) = written;
            } else {
                LOW(*bufPtr) = written;
            }
            continue;

        default:
            if (ch == '%') {
                out[written++] = ch;
                continue;
            } else {
                goto end;
            }
        }
        if (len < info.width && !info.leftJustified) {
            while (len < info.width) {
                out[written++] = ' ';
                info.width--;
            }
        }
        memmove(&out[written], bufPtr, len);
        written += len;
        while (len < info.width) {
            out[written++] = ' ';
            len++;
        }
    }
end:
    out[written] = 0;
    return written;
}
