/* PsyQ 4.0 LIBSND MIDITIME: _SsReadDeltaValue. .text 0x80085064..0x80085114, a verbatim LIBSCAN
 * module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

s32 _SsReadDeltaValue(s16 arg0, s16 arg1) {
    s32 result;
    u8 *ptr;
    struct SeqStruct *score;
    s32 val;
    s32 byte;

    score = &_ss_score[arg0][arg1];
    ptr = score->read_pos;
    score->read_pos = ptr + 1;
    val = *ptr;
    if (val == 0) {
        return 0;
    }
    if (val & 0x80) {
        val &= 0x7F;
        do {
            ptr = score->read_pos;
            score->read_pos = ptr + 1;
            byte = *ptr;
            val = (val << 7) + (byte & 0x7F);
        } while (byte & 0x80);
    }
    result = val * 10;
    score->unk88 += result;
    return result;
}
