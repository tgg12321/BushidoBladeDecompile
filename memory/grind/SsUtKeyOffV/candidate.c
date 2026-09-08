/* CANDIDATE s1 (2026-09-08): sandbox --disable all = 0 (30/30 insns), applied in
   src/main.c.  Ordinary C, no FAKE constructs.  Structure mirrors psyz's matched
   PsyQ 4.0 SsUtKeyOff (tmp/psyq_prov/psyz/decomp/src/libsnd/ut_key.c:76-98):
   success arm FIRST inside `if (voice >= 0 && voice < 24)`, error fall-out last.
   Why this shape and not the flat error-first one (score 12): see evidence.md
   [s1] E4/E5 - jump1's range swap (jump.c:1826) puts whichever arm is written
   first into the tail position; only success-first yields the target's
   error-inline / success-falls-into-epilogue layout, which (a) leaves the error
   block ending in `jump Lend` so sched1 hoists `v0=-1` above the flag store and
   the two `return -1` suffixes are no longer rtx_equal for jump2's cross-jump,
   and (b) lets the success block's `jump Lend` be deleted as jump-to-next. */
s16 SsUtKeyOffV(s16 voice) {
    if (_snd_ev_flag == 1) {
        return -1;
    }
    _snd_ev_flag = 1;
    if (voice >= 0 && voice < 24) {
        _svm_cur.voice = voice;
        _SsVmKeyOffNow(0);
        _snd_ev_flag = 0;
        return 0;
    }
    _snd_ev_flag = 0;
    return -1;
}
