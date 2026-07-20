/* s11 composite candidate — sandbox --disable all == 0 CONFIRMED THIS SESSION.
 * Discharges Judge constraint (a) of the final-call gate: the s6 arg0=0 /* FAKE */
 * form applied in-tree lets the FAKE-bypass path in engine/volatile_cheats.py:815
 * (_stmt_fake_annotated) pass the assignment through to cc1, which then emits
 * `addu $s0, $s4, $v0` matching target insn #18 — RTL-proven at s6, now
 * end-to-end verified via the sandbox integration.
 *
 * DOES NOT reach candidate-ready. Composite still carries `s32 buf[8]; (void)buf;`
 * (pre-existing s1 cheat, load-bearing for the 0x50 frame per s3 measurement).
 * buf[8] fails dead-vars-local-array WRITTEN requirement (s7 grep proved target
 * has ZERO stores in sp+0x18..sp+0x37 locals region — the shipped bytes carry
 * no dead stores that would ground the SOTN WRITTEN carve-out); no other
 * sanctioned family covers an UNWRITTEN frame carrier. The 2026-07-20
 * endgame-lock-disposition species criteria are met and both AND-gates fail
 * (no hand-coded signals for canonical-asm; no SOTN precedent for unwritten
 * frame carrier). Per s10 synthesis + s11 F1 discharge, next step is F2
 * OWNER-ESCALATION per [[endgame-lock-disposition]] protocol, following the
 * hirahira_w_frie / motion_SetMotion / func_80049A2C format.
 *
 * Sandbox measurement (tmp/grind/InitHiraRmd_80047FBC/s11/sandbox_s6form.log):
 *   {"score": 0, "target_insns": 65, "build_insns": 65, "scorable": true, ...}
 */
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 buf[8];
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: defeats cse2 canonical-reg substitution
                 that folds {reg 72 arg0, reg 78 p, reg 79 base_addr}
                 equivalence class at insn 36 — RTL-proven s6 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
    (void)buf;
}
