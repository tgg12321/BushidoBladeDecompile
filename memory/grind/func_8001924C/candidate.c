/* func_8001924C (src/code6cac.c) — best HONEST form, floor 12 (recon s1, 2026-07-23)
 *
 * This is the CHEAT-FREE floor form: pad[2] REMOVED. sandbox --disable all = 12,
 * build_insns 49 == target_insns 49 — ALL 12 diffs are sp-relative offset shifts
 * from a single +8-byte frame-size delta (frame 0x28 here vs target 0x30).
 * The loop body is byte-identical to target.
 *
 * HEAD's src/code6cac.c currently ships `s32 pad[2];` (FORBIDDEN unused-local-array
 * frame-coercion cheat) which makes vars=8 -> frame 0x30 -> oracle green via the cheat.
 * Measured this session (minrepro): pad IN -> vars=8/frame48; pad OUT -> vars=0/frame40.
 *
 * This function is the near-exact structural TWIN of func_80049A2C (owner-gated
 * 2026-07-20): identical target frame 0x30, saves s0/s1/s2/s3/ra at 0x18..0x28,
 * 8-byte zero-store phantom hole at sp+0x10..0x17. The twin exhausted every pure-C
 * axis (cc1 -da proof: phantom-slot mechanism does NOT fire; aggregate-only) and
 * filed OWNER-ESCALATION. scan_hand_coded func_8001924C = LOW 0/8 (no canonical-asm
 * evidence), same as the twin.
 *
 * NEXT (grind modality): run func_8001924C's OWN cc1 -da greg dump (pad-in vs
 * pad-out) to confirm the phantom mechanism doesn't fire here either before any
 * owner-gated disposition. See hypotheses.md frontier.
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
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
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
