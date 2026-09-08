extern u16 D_800A26E4[];

/* PsyQ LIBSND VM_N2P: note2pitch — a second exported entry point that splat
   merged into func_80086818. Split out 2026-08-10 (docs/naming/libscan/
   boundary_fixes.md); must stay immediately after its former host so the
   link order reproduces the original byte layout. C ref: psyz
   decomp/src/libsnd/vm_n2p.c (PsyQ 4.0). */
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
