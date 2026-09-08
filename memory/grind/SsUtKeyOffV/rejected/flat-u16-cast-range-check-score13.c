/* REJECTED s1 (score 13): flat form with `(u16)voice >= 24`.  Two residuals:
   (1) the parm's HI pseudo takes $a0 (no `addu $a1,$a0,$zero` copy, compare reads
       the pseudo) because the range check is expanded from the HImode parm copy;
   (2) error block placed AFTER the key-off block by jump1's range swap
       (jump.c:1826 "if (foo) bar; else break;"), so the two `return -1` tails
       cross-jump-merge in jump2 and the success path ends in `j Lend`. */
s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if ((u16)voice >= 24) {
        _snd_ev_flag = 0;
        return -1;
    }
    _svm_cur.voice = voice;
    _SsVmKeyOffNow(0);
    _snd_ev_flag = 0;
    return 0;
}
