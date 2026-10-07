/* PsyQ 4.0 LIBSND VM_N2P: note2pitch and note2pitch2. .text
 * 0x80086B38..0x80086CF8, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

extern u16 D_800A26E4[];

/* Kept right after vm_init's _SsVmInit in link order (bb2.ld), reproducing
 * the original layout. */
u16 note2pitch(void) {
    s32 octave;
    s32 note;
    s32 shiftVal;
    s32 semitones;
    s16 semitone;
    u32 tableIndex;
    u16 step;
    s16 shift;
    u16 pitch;
    s32 noteIndex;

    note = _svm_cur.note + (60 - _svm_cur.tone_center);
    shiftVal = _svm_cur.tone_shift;
    step = shiftVal / 8;
    semitones = (s16)note;
    octave = semitones / 12;
    semitone = semitones - (octave * 12);
    if (step >= 16) {
        step = 15;
    }
    noteIndex = semitone * 16;
    tableIndex = noteIndex + step;
    pitch = D_800A26E4[tableIndex];
    shift = octave - 5;
    if (shift > 0) {
        pitch <<= shift;
    } else if (shift < 0) {
        pitch >>= -shift;
    }
    return pitch;
}

s32 note2pitch2(u16 arg0, u16 arg1) {
    s16 octave;
    s16 var_a2;
    s16 var_a3;
    short new_var;
    u16 var_v1;
    s32 pos;
    s32 tone;

    tone = _svm_cur.tone + (_svm_cur.field_7_fake_program * 0x10);
    var_a3 = (arg1 + _svm_tn[tone].shift) / 8;
    var_a2 = 0;
    if (var_a3 >= 16) {
        var_a2 = 1;
        var_a3 -= 16;
    }
    new_var = arg0 + 60 - _svm_tn[tone].center + var_a2;
    octave = new_var / 12;
    pos = (new_var % 12) * 16;
    var_v1 = D_800A26E4[pos + var_a3];

    octave -= 5;
    if (octave > 0) {
        var_v1 <<= octave;
    } else if (octave < 0) {
        var_v1 >>= -octave;
    }
    return var_v1;
}
