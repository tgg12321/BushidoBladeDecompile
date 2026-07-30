/* func_80040CB8 — best form as of grind session s1 (2026-07-30, modality: recon)
 *
 * Honest pure-C floor: sandbox --disable all == 13   (inherited baseline was 17)
 * build_insns == 36 == target_insns, and the emitted INSTRUCTION SEQUENCE is
 * identical to asm/funcs/func_80040CB8.s opcode-for-opcode and
 * displacement-for-displacement. Every one of the 13 residual diffs is a
 * register-NAME diff plus a one-slot rotation of the prologue. There is no
 * remaining structural gap.
 *
 * Residual, precisely:
 *   want (target)                 got (this form)
 *   ---------------------------   -----------------------------
 *   slot (0x8B4 cursor)  = $a2    $a2   OK
 *   i                    = $t1    $t1   OK
 *   none (-1)            = $t4    $t4   OK
 *   kind (3)             = $t3    $t3   OK
 *   one  (1)             = $t2    $t2   OK
 *   link (0x94 cursor)   = $t0    $t0   OK
 *   tbl  (D_80094B9E)    = $a3    $a3   OK
 *   ent  (0x90C cursor)  = $a1    $v1   WRONG  <-- swapped with...
 *   id-copy (store src)  = $v1    $a1   WRONG  <-- ...this
 *   AND: target emits `addiu $a2,$a0,0x8B4` as the FIRST prologue insn;
 *        this form emits it LAST (prologue rotated by one slot).
 *
 * Those two facts are COUPLED — see memory/grind/func_80040CB8/hypotheses.md
 * (H2/H3). `slot` must be declared LAST to win its allocno-priority race
 * against `tbl`/`link`, but declaring it last is exactly what sinks its
 * `addiu` to the bottom of the prologue. Breaking that coupling is the
 * frontier.
 *
 * NOTE ON THE THREE CONSTANT-HOLDER LOCALS (none/kind/one): they are NOT
 * decorative. Probe C (s1) replaced all three with literals and the build
 * dropped to 35 insns / score 23 — in the `goto`-loop form GCC never forms a
 * NOTE_INSN_LOOP_BEG loop, so there is no LICM to hoist a literal into a
 * loop-invariant register, and one of the three constants folds away entirely.
 * They are therefore load-bearing, but they are ALSO constant-holder locals in
 * the sense of .claude/rules/named-local-fake-exception.md and a future
 * candidate-ready session MUST either (a) find a spelling that does not need
 * them, or (b) carry the /* FAKE */ annotation + documented lever exhaustion
 * that rule requires. This session does not claim (b) is satisfied.
 */
/* kengo:MED  |  my_rob/rob_calc_2d_position  |  93i */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s32 i = 0;
    s32 none = -1;
    s32 kind = 3;
    s32 one = 1;
    s32 link = (s32)arg0 + 0x94;
    s16 *tbl = D_80094B9E;
    s32 ent = (s32)arg0 + 0x90C;
    s8 *slot = (s8 *)arg0 + 0x8B4;

loop:
    {
        s16 id = *tbl;
        if (id != none) {
            *(s16 *)(ent - 0x56) = id;
            *slot = kind;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = one;
            *(s16 *)(ent - 0x4E) = 0;
            {
                u16 w = *(u16 *)((s32)arg0 + 0x16);
                slot += 0x68;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    }
    *(s16 *)((s32)slot + 2) = -1;
}
