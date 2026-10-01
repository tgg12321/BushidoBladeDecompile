"""Add the landing annotations to the byte-exact body (comments only).
usage: python annotate.py <in.c> <out.c>"""
import sys

src, dst = sys.argv[1:3]
s = open(src).read()


def rep(old, new, cnt=1):
    global s
    n = s.count(old)
    assert n == cnt, (n, old[:80])
    s = s.replace(old, new)


rep("extern s16 D_8008E0BC[27][4];\nextern u8 D_8008D90C[28][8];\n",
    """/* [unk_0A][min(unk_272, 3)]: 4.12 scale of unk_1E into unk_20 (func_80023F08). */
extern s16 D_8008E0BC[27][4];
/* [D_8008D9EC[unk_0A]][unk_0E]: the move command func_80023F08 hands
 * cpu_set_move_command_and_dir. */
extern u8 D_8008D90C[28][8];
""")

rep("void func_80023F08(s32 arg0, PadState *pad) {\n",
    """/* Per-frame update of character `arg0`'s record from this frame's pad input:
 * converts the pad bits, advances the move frame, runs the move script's
 * command list (unk_50 / unk_7C, func_80021424 / func_80021A98), decodes the
 * current and next motion frames and blends their root offsets into unk_E8,
 * integrates the velocities (unk_104 / unk_134) into the position, resolves
 * it (func_8002304C), then refreshes the bone points, hit flags (unk_62,
 * unk_AE, unk_288) and damping and the per-frame side effects. */
void func_80023F08(s32 arg0, PadState *pad) {
""")

rep("    s32 temp;\n",
    """    /* temp holds five values: the unk_14C clamp limit, the unk_1D8 / unk_1C8.vy
     * angle gap (folded to 0..0x800), the unk_14C turn step, the facing
     * (unk_154 or unk_1C8.vy) and the -1/0/1 stick side. One local, not five:
     * ordinary-c-judge-decidable.md Ruling 11; (D) record in
     * memory/grind/func_80023F08/r11/ (evidence.md [s3]). */
    s32 temp;
""")

rep("""            if (!(cmd & 0x8000) || ((ent[2] | ((u32)ent[3] << 16)) & (1 << rec->unk_0A))) {""",
    """            /* the entry's 32-bit class mask, high half shifted in unsigned */
            if (!(cmd & 0x8000) || ((ent[2] | ((u32)ent[3] << 16)) & (1 << rec->unk_0A))) {""")

rep("""    if (rec->unk_286 >= 0) {
        u16 *table;
""", """    if (rec->unk_286 >= 0) {
        /* the move-script record of unk_50's follow-up id is a u16 move-id
         * table indexed by event code (here unk_286; 0xD, 0x17/0x18 below) */
        u16 *table;
""")

rep("""        s32 state = rec->unk_6A;

        if (rec->unk_7A != 0 && state == 6) {""",
    """        /* FAKE: named intermediate (no-new-park-categories.md family 6): unk_6A
         * is read into `state` before the unk_7A test, as the target loads it
         * (lhu 0x6A ahead of the beqz at 0x80024C14). Spelled with two direct
         * reads, the 6A load follows the branch and jump.c thread_jumps sends
         * the 0x23 test's unk_7A == 0 jump past this test (+2 insns, score 3);
         * u16 state 1. Final .s of both: memory/grind/func_80023F08/fake/ */
        s32 state = rec->unk_6A;

        if (rec->unk_7A != 0 && state == 6) {""")

rep("""        s32 state = rec->unk_6A;

        if (state == 8 || state == 0x22) {""",
    """        /* FAKE: named intermediate (no-new-park-categories.md family 6): with
         * `state`, cse.c keeps this test's own li 8 / li 0x22 (the target
         * re-materialises them at 0x80025784..8C); with direct rec->unk_6A
         * reads cse substitutes the previous test's constant pseudos (score 11).
         * .cse of both: memory/grind/func_80023F08/fake/ */
        s32 state = rec->unk_6A;

        if (state == 8 || state == 0x22) {""")

rep("""        twist = -ratan2(m1.m[0][0], m1.m[2][0]) + ratan2(m2.m[0][0], m2.m[2][0]);""",
    """        /* new frame's heading minus the previous frame's (old one computed first) */
        twist = -ratan2(m1.m[0][0], m1.m[2][0]) + ratan2(m2.m[0][0], m2.m[2][0]);""")

open(dst, "w", newline="\n").write(s)
