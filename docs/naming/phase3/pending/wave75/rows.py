"""wave75 rows: the full-screen image pair of 596CC.c (re-mined by Codex after the wave73 FAIL of
their comb_Connect wrappers). The wrappers func_80077AE0 / func_80077B00 stay neutral.
2 RENAME. Applier note: fix the include/game.h:55 comment in the comment pass.
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1, 2/2): gpu_BeginFullscreenImageDisplay and "
                 "gpu_EndFullscreenImageDisplay CONFIRM. Begin covers the blocking CD read and LoadImage as set-up; the image "
                 "RECT equals the 640x240 display; comb_HandshakeStep unmasks it during the handshake; End pairs with Begin at "
                 "the only call chain (2B344.c:72 then :86) and clears both buffers.")

FUNCS = [
    ('0x8006E10C', 'gpu_BeginFullscreenImageDisplay', 'RENAME',
     'BODY 596CC.c:2462: picks CD file (2,0x60) or (2,7), reads it (blocking, cdrom_WaitIdle), masks the '
     'display, re-sets both g_gpu_db envs to the standard 640x240 double buffer, clears RECT D_800A32D8 = '
     '(0,0,640,240), LoadImages the file at +0x14, puts draw db[0] / disp db[1], unmasks, returns 1. USES: '
     'decl bb2.h:976; only caller func_80077AE0 (64FD8.c:1335), itself called only by comb_Connect '
     '(2B344.c:72); comment game.h:55; no data reference. WHY: sets up and shows a 640x240 image on the '
     '640x240 display. COUNTER-EVIDENCE: comb_Connect masks the display right after (2B344.c:73) but '
     'comb_HandshakeStep unmasks it (2B344.c:26), so the image is what shows during the handshake; the '
     'image subject is not claimed. FRESH: absent from src/, include/, symbol files, census, tools.'),
    ('0x8006E2A8', 'gpu_EndFullscreenImageDisplay', 'RENAME',
     'BODY 596CC.c:2498: masks the display, puts the draw/disp envs of g_gpu_db[D_800A3518&1], clears '
     'D_800A32E0 = (0,0,640,480) (both buffers, erasing the image), gpu_SetDrawEnvBg(1,0,0,0), unmasks, '
     'returns 1. USES: decl bb2.h:977; only caller func_80077B00 (64FD8.c:1337), called only by '
     'comb_Connect after the handshake loop (2B344.c:86). WHY: the counterpart of '
     'gpu_BeginFullscreenImageDisplay at its only call chain. COUNTER-EVIDENCE: it does not restore saved '
     'env definitions (Begin wrote the standard ones). FRESH: as above.'),
]

DATA = []
