/* s6 forensics-derived form. RTL-proven to defeat cse2 canonical-reg
 * substitution at insn 36 — greg emits `(reg 16 s0) = (plus (reg 20 s4)
 * (reg 2 v0))` matching target's `addu $s0, $s4, $v0` exactly.
 *
 * Sandbox `--disable all` reports score=1 with this form applied because
 * engine.volatile_cheats.find_dead_param_assigns detects `arg0 = 0;` and
 * strips it before compilation (see engine/volatile_cheats.py:791). The
 * sandbox score is thus INVARIANT to this lever — measuring the lever via
 * sandbox is meaningless. Only raw cc1 dumps + full-build oracle can
 * validate it.
 *
 * FORBIDDEN without owner sanction: this is Lever D (dead-param-assign)
 * per [[inline-asm-policy]] expanded catalog. The [[dead-store-fake-
 * exception]] 2026-07-01 carve-out MIGHT apply — see s6 FINDINGS.md
 * ruling-request. Not to be committed without the layer-2 cheat-reviewer
 * + owner ruling. Banked here as evidence of the RTL-proven mechanism,
 * NOT as a proposal to commit. */
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
