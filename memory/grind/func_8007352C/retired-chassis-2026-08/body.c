s32 func_8007352C(EnvA *_env) {
    register EnvA *env asm("s4") = _env;
    register s32 *hdr asm("s6") = env->header;
    register s32 buf asm("s5") = env->out;
    s32 fp;
    register s32 i asm("s7");
    register s32 dst asm("s3");
    s32 v_tail;
    s8 *e;
    s32 a1, x, y, v1;
    s32 raw;
    s32 saved_buf;

    {
        register s32 j asm("v1");
        raw = GetClut(*((u16 *)hdr + 2), *((u16 *)hdr + 3));
        j = (s32)*((u8 *)hdr + 2) - 1;
        fp = raw & 0xFFFF;
        if (j < 0) goto exit;
        i = j;
    }
    dst = buf + 6;

loop_top:
    e = env->table + ((i << 16) >> 13);
    a1 = *(s16 *)(e + 0);
    x = a1 + env->x;
    y = *(s16 *)(e + 2) + env->y;
    {
        s32 x_check = x + (s32)*((u8 *)e + 6);
        v1 = y + (s32)*((u8 *)e + 7);
        if (x_check <= 0) goto next_compute_v_tail;
    }
    v_tail = i - 1;
    if (x >= 0x280) goto merge_v_tail;
    if (y >= 0xF0) goto next_compute_v_tail;
    v_tail = i - 1;
    if (v1 <= 0) goto merge_v_tail;
    SetSprt(buf, (s16)a1);
    *(s16 *)(dst + 8) = (s16)fp;
    *(s16 *)(dst + 2) = (s16)x;
    *(s16 *)(dst + 4) = (s16)y;
    *(s8 *)(dst + 6) = (s8)((s32)*((u8 *)e + 4) + (s32)*((u8 *)hdr + 8));
    *(s8 *)(dst + 7) = (s8)((s32)*((u8 *)e + 5) + (s32)*((u8 *)hdr + 0xA));
    *(s16 *)(dst + 0xA) = (s16)*((u8 *)e + 6);
    *(s16 *)(dst + 0xC) = (s16)*((u8 *)e + 7);
    if (env->has_color != 0) {
        SetShadeTex(buf, 0);
        *((u8 *)dst - 2) = env->col_r;
        *((u8 *)dst - 1) = env->col_g;
        *((u8 *)dst + 0) = env->col_b;
    } else {
        SetShadeTex(buf, 1);
    }
    SetSemiTrans(buf, env->semi);
    {
        register s32 sleep_arg asm("$5") = buf;
        if (env->ot_idx >= 0x1006U) {
            env->ot_idx = 1U;
            func_8003D52C(&D_800159A0, buf);
            /* INLINE_MOVE_ALIASING: target re-emits `addu $a1,$s5,$zero` after
             * DispSleepMenuTex at 0x800736b8 (lost_codegen pattern from regfix
             * line 4222: `insert "addu $5,$21,$zero" @ 93`). Plain `saved_buf
             * = buf;` is folded by CSE; this barrier forces a real re-emission.
             * Placed INSIDE the if-body so it's skipped when branch taken
             * (matches target's bnez offset of 7 not 6).
             *   - technique=plain_copy: `saved_buf = buf;` per dump-text —
             *     GCC scheduled the move into jal-delay-slot ONLY (1 less insn
             *     than target; the second emission was elided by CSE since
             *     $s5 unchanged across the call).
             *   - technique=decl_reorder: moving `saved_buf = buf;` BEFORE the
             *     if block created an extra `addu $s0,$s5,$zero` (1 extra
             *     instruction); GCC promoted saved_buf to a callee-save reg
             *     to preserve it across DispSleepMenuTex.
             *   - technique=plain_inline_after: placing `__asm__ ("move %0,%1")`
             *     AFTER if-block produced correct 508-byte function with all
             *     instructions present but bnez offset off by 1 (the move was
             *     outside the if-body, target wants it inside).
             * Per feedback_inline_move_aliasing.md, single-insn escape valve. */
            __asm__ volatile("move %0, %1" : "=r"(sleep_arg) : "r"(buf));
        }
        saved_buf = sleep_arg;
    }
    dst += 0x14;
    buf += 0x14;
    AddPrim(D_800A374C + env->ot_idx * 4, saved_buf);
next_compute_v_tail:
    v_tail = i - 1;
merge_v_tail:
    i = v_tail;
    if ((v_tail << 16) >= 0) goto loop_top;
exit:
    return buf;
}
