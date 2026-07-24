/* REJECTED (measured KILL, s1 2026-07-23) — naive HImode-bitwise phantom-slot
 * injection. Reformulated the flag test as `s16 fa = *(s16*)(s0+2); s16 fb = 1;
 * if ((fa & fb) & 1)` to try to trigger the phantom-frame-slot mechanism
 * (two HImode locals feeding an HImode bitwise expr -> cc1 reserves an untouched
 * HImode stack temp; see func_80037540 s5 mechanism).
 *
 * MEASURED: minrepro -> `.frame $sp,40 # vars= 0` — NO phantom slot fired.
 * Because fb=1 is constant, `(fa & 1) & 1` folds to `fa & 1` at the tree level
 * and the sign-ext is combined away before any stack temp is reserved. Also
 * changes lbu->lh (diverges from target bytes). Same negative the twin
 * func_80049A2C measured (s2). The witness form (tslLineG5Init) needs TWO
 * non-constant s16 GLOBAL loads AND a real emitted store gated by the bitwise —
 * func_8001924C's body has no store to gate, so the witness shape cannot be
 * replicated without emitting instructions the 49-insn target lacks.
 */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0;
    s32 new_var;
    i = 0;
    if (arg1 <= 0) return;
    new_var = (s32)&g_file_data_buf;
    s0 = arg0;
    do {
        s16 fa = *(s16 *)((u8 *)s0 + 2);   /* CHANGES lbu->lh; folds anyway */
        s16 fb = 1;
        if ((fa & fb) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            single_game_setModeRequest(s0, (s32 *)(val * 52 + new_var));
        }
        i++;
        s0 = (s16 *)((u8 *)s0 + 16);
    } while (i < arg1);
}
