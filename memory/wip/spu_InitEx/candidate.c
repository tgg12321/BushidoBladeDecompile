/* spu_InitEx (main.c) — body is ALREADY clean pure-C at floor 0.
 * This candidate is HEAD's body verbatim; it is NOT the blocker.
 * The blocker is the file-scope `extern volatile s32 D_800A28A0/D_800A2CD4`
 * declarations (lines 48/50) that the CONSUMER functions need for the byte
 * match. See notes.md — spu_InitEx cannot close independently; it is coupled
 * to coli_HitPauseKatana_2 + func_8008AAD4. */
void spu_InitEx(s32 arg0) {
    u16 *var_v0;
    s32 var_v1;
    s32 val;

    irq_DisableInterrupts();
    func_80088740(arg0);
    val = 0xC000;
    if (arg0 == 0) {
        var_v1 = 0x17;
        var_v0 = (u16 *)&D_800A28D2;
        do {
            *var_v0 = val;
            var_v1 -= 1;
            var_v0 -= 1;
        } while (var_v1 >= 0);
    }
    spu_InitIrq();
    D_800A287C = 0;
    D_800A2880 = 0;
    D_800A288C = 0;
    D_800A2890 = 0;
    D_800A2892 = 0;
    D_800A2894 = 0;
    D_800A2898 = 0;
    D_800A2884 = D_800A2D44;
    spu_WriteReg(0xD1, D_800A2D44, 0);
    g_spu_voice_key_a = 0;
    g_spu_voice_key_b = 0;
    g_spu_voice_key_c = 0;
    g_snd_reverb_flag = 0;
    g_spu_reverb_mode = 0;
    D_800A2874 = 0;
    D_800A28A0 = 0;
    D_800A289C = 0;
    D_800A2CD4 = 0;
}
