/* SN Systems runtime: PCread and _SN_read (the PC file server). .text
 * 0x8008387C..0x80083954: an unidentified region between verbatim LIBSCAN
 * modules (docs/naming/libscan/matches.json;
 * memory/closer/psyq-library-census.md), one file per gap (Q106 D3), named by
 * its ROM offset. */
#include "common.h"
#include "include_asm.h"
#include <psxsdk/libsn.h>

extern s32 _SN_read(s32, s32, s32, s32);

s32 PCread(s32 addr, s32 dest, s32 len) {
    s32 total;
    s32 chunk;
    s32 result;

    total = 0;
    if (len != 0) {
        do {
            chunk = len;
            if ((u32)0x8000 < (u32)len) {
                chunk = 0x8000;
            }
            result = _SN_read(0, addr, chunk, dest);
            total += result;
            if (result == -1) {
                return -1;
            }
            dest += result;
            len -= result;
            if (result < chunk) {
                return total;
            }
        } while (len != 0);
    }
    return total;
}

INCLUDE_ASM("asm/funcs", _SN_read);
