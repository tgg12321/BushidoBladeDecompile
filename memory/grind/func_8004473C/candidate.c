/* s4 LANDED (2026-09-06, synthesis modality - the driver's session 4; an earlier discarded
 * session numbered itself s5, so ignore the s5 labels below as session numbering, the
 * measurements they report are real and were reproduced this session).
 *
 * THIS SESSION landed the whole thing through the normal gates. The pipeline had already
 * executed the integration handoff before dispatch: tools/grinder/scope_allow.txt:61 grants
 * `func_8004473C include/game.h undefined_syms_auto.txt named_syms.txt`, and state.json
 * banned_constructs is empty (the Unk800A9CF8Header tripwire was cleared by the Judge's
 * ESCALATE ruling of 2026-09-06). So the blocker the previous two sessions filed against no
 * longer existed and there was nothing left to research - the correct move was to apply,
 * measure, vet and submit, which is what happened.
 *
 * What landed: candidate_merge.patch (include/game.h + src/text1a_c.c) applied to a clean
 * HEAD, PLUS the prong-(c) housekeeping the handoff specified - each of the seven per-word
 * rows at undefined_syms_auto.txt:302-308 suffixed
 * `/* alias of D_800A9CF8+0xN; retire with func_80044800 *SLASH` (the base row records that it
 * is the base). named_syms.txt needed no edit: it has no rows for these symbols, contrary to
 * the handoff's claim of rows at 1742-1743/2135-2136/2269-2270/2488 - grep returns nothing.
 *
 * Measured this session, in this configuration:
 *   sandbox func_8004473C --disable all -> {"score": 0, "target_insns": 49, "build_insns": 49,
 *     "scorable": true, "rules_dropped": 0}
 *   verify-oracle --rebuild --allow-dirty -> ok=true, build_sha1 =
 *     62efab4f73f992798c43e8c730aa43baa10bb4fa = expected = original_sha1_now =
 *     original_sha1_locked
 *
 * One inherited claim CORRECTED. The prong-(a) evidence in the discarded handoff and in the
 * Judge packet asserted that asm/funcs/func_80044800.s "forms &D_800A9D04 in $v1 and reads
 * D_800A9CFA as $v1-0xA". It does not: :6-7 form the address and :17 does a plain
 * `lw $s2, 0x0($v1)`; there is no negative displacement off $v1 anywhere in that function.
 * The claim is withdrawn from self_vet.md and from the include/game.h comment. Prong (a) does
 * not need it - asm/funcs/func_8004473C.s:11-12/15/24/43 alone shows ONE base register
 * ($a3 = &D_800A9D08 - 0x10) reaching 0x800A9CF8, 0x800A9CFE and 0x800A9D08 by signed
 * displacement, which is base+offset addressing of a single object, and func_80044C70
 * corroborates on the C side by bumping the unk8/unkC pointer pair together.
 */

/* CANDIDATE - func_8004473C (s4, 2026-09-06). MATCHED: sandbox --disable all = 0 at 49/49
 * instructions, and a full `verify-oracle` rebuild of the whole tree returns
 * build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. Pure C, no FAKE
 * construct, no inline asm, no register pins (HEAD carried a register-pinned m2c body for
 * this function; this replaces it).
 *
 * HOW THE FLOOR MOVED 13 -> 0 THIS SESSION (three independent ordinary-C levers, each
 * measured with the engine metric against tmp/perm_4473C_s4a/target.o):
 *   1. LOOP-BODY STATEMENT ORDER. s1-s3 recorded the loop body as "byte-exact" and stopped
 *      permuting it. It is not: moving `dst->unk6 = 0;` to the second-to-last position and
 *      swapping `dst->unkC = 0;` ahead of `dst->unkA = 4;` takes form C from 13 to 9.
 *   2. FOR-INCREMENT ORDER. Spelling the comma-increment `dst++, src++, i++` (dst first)
 *      instead of `i++, dst++, src++` takes 9 -> 7, and later fixes the last two-instruction
 *      swap (the `addiu a1,a1,104` / `addiu a2,a2,1` pair) in the loop body schedule.
 *   3. THE CHAINED ASSIGNMENT. `D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());`
 *      is the block-0 fix. s3 attributed the whole residual to sched1 boosting the standalone
 *      copy insn `src = <call temp>` (birthing_insn_p, sched.c:2505) to LAUNCH_PRIORITY, so it
 *      won the T-2 ready-list slot and pushed the count load next to `blez` (forcing a
 *      load-delay nop, 50 instructions). Writing the store and the pointer as ONE assignment
 *      expression means expand emits the store from the call-result pseudo directly and there
 *      is no separate user-copy insn in block 0 to be boosted: block 0 then emits
 *      `move a2,zero / lui a1; lw a1,12 / lui v1; lh v1,6 / lui a0; addiu a0,a0,16 / blez v1 /
 *      sw v0,0(a0) / addiu a3,a0,-16` - the target verbatim, store in the delay slot, no nop.
 *
 * s5 UPDATE (2026-09-06): the declaration placement question is CLOSED. With
 * candidate_merge.patch applied (typedef + `extern Unk800A9CF8Header D_800A9CF8;` in
 * include/game.h, per-word externs deleted from src/text1a_c.c, three C siblings rewritten to
 * members) and this body in place, `sandbox --disable all` = 0 at 49/49 and a full-tree
 * `verify-oracle --rebuild` returns build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
 * original_sha1_locked. So the HEADER-CANONICAL spelling -- prong (d) of the aggregate-merge
 * family, and the sole ground of the 2026-09-06 04:14 layer-1 FAIL -- is byte-identical at zero,
 * not merely at the old floor of 13. The two typedefs below are reproduced here only so this
 * file stands alone; in the landed form Unk800A9CF8Header and the extern live in include/game.h
 * and ONLY Rec4473C stays TU-local (it is a local record type, not a merged splat symbol).
 * Do NOT submit the TU-local declaration again: it is a driver-enforced banned construct for
 * this function. Apply candidate_merge.patch instead -- it is the complete byte-verified diff.
 *
 * CHASSIS: still requires the D_800A9CF8 aggregate merge (per-word splat symbols ->
 * Unk800A9CF8Header) established in s1 from base+offset evidence. The declaration is TU-LOCAL
 * here ONLY because a grind session's edit surface is src/text1a_c.c + memory/ + tmp/;
 * memory/grind/func_8004473C/candidate_merge.patch is the header-canonical variant
 * (include/game.h) that s3 measured byte-identical. See self_vet.md for prongs (c)/(d).
 */

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;   /* stage id (stage_GetId) */
    s16 unk6;   /* entry count */
    s32 unk8;
    s32 unkC;   /* entry table (stride 0x68) */
    s32 unk10;  /* game_GetCharData() table (stride 0x68) */
    s32 unk14;
} Unk800A9CF8Header;

extern Unk800A9CF8Header D_800A9CF8;

extern void *game_GetCharData(void);

/* Per-entry record (stride 0x68) shared by the D_800A9CF8.unkC table and the
 * game_GetCharData() table; the sibling func_80044B30 walks both with the
 * same stride and field offsets. */
typedef struct {
    s8 unk0;
    s8 unk1;
    s16 unk2;
    u16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    s8 pad18[0x14];
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s8 pad38[0x14];
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s8 pad60[8];
} Rec4473C;

void func_8004473C(void)
{
    Rec4473C *src;
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());
    dst = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; dst++, src++, i++) {
        dst->unk0 = 0;
        dst->unk1 = 0;
        dst->unk2 = 0;
        dst->unk4 = D_800A9CF8.unk0;
        dst->unk8 = 0;
        dst->unkC = 0;
        dst->unkA = 4;
        dst->unk10 = 0;
        dst->unk12 = 0;
        dst->unk14 = 0;
        dst->unk4C = src->unk2C;
        dst->unk50 = src->unk30;
        dst->unk54 = src->unk34;
        dst->unk6 = 0;
        dst->unk58 = -1;
    }
}
