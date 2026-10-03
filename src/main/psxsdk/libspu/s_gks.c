/* PsyQ 4.0 LIBSPU S_GKS: SpuGetKeyStatus. .text 0x8008ACD0..0x8008AD64, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libspu_internal.h"

s32 SpuGetKeyStatus(s32 arg0) {
    s32 bit_found;
    s32 i;
    s32 one;
    s32 mask;
    s32 base;
    s32 flags;

    bit_found = -1;
    i = 0;
    one = 1;
    for (; i < 0x18; i++) {
        mask = one << i;
        if (arg0 & mask) {
            bit_found = i;
            break;
        }
    }

    if (bit_found != -1) goto work;
    return -1;
work:
    base = bit_found << 4;
    mask = _spu_RXX;
    flags = _spu_keystat;
    base = base + mask;
    mask = 1 << bit_found;
    flags = flags & mask;
    base = *(u16 *)(base + 0xC);
    if (!flags) goto no_flags;
    if (!base) goto ret3;
    goto ret1;
no_flags:
    return (s32)(base != 0) << 1;
ret3:
    return 3;
ret1:
    return 1;
}
