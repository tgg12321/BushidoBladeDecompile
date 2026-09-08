s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag != 1) {
        _snd_ev_flag = 1;
        if (voice >= 0 && voice < 24) {
            _svm_cur.voice = voice;
            _SsVmKeyOffNow(0);
            _snd_ev_flag = 0;
            return 0;
        }
        _snd_ev_flag = 0;
    }
    return -1;
}
