/* REJECTED s1 (score 12): flat psyz-SsUtKeyOnV-style range check, error arm first.
   `voice < 0 || voice >= 24` fixes the $a0/$a1 split (fold_range_test makes the
   compare a zero_extend that cse resolves to the incoming $a0 while the store keeps
   the HI pseudo -> $a1), but with the ERROR arm written first jump1's range swap
   (jump.c:1826) still moves the error block to the tail, the two `return -1`
   suffixes become rtx_equal and jump2 cross-jumps them, and the success block ends
   in `j Lend` instead of falling into the epilogue.  Byte-identical output to the
   `else`-arm form and the nested-flag-if form (both also 12). */
s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice < 0 || voice >= 24) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.voice = voice;
    _SsVmKeyOffNow(0);
    _snd_ev_flag = 0;
    return 0;
}
