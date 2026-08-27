/* BEST BAN-COMPLIANT FORM (grind s34, 2026-08-27, structural modality).
 * MEASURED THIS SESSION: `sandbox func_80057CC8 --disable all` -> score 20,
 * target_insns 111, build_insns 108, rules_dropped 0.  The inherited s33 candidate
 * re-measured at 22 on this chassis first, so the floor moved 22 -> 20.
 *
 * WHAT CHANGED vs s33: exactly one expression -- the next-neighbour BYTE OFFSET is
 * now `tmp * 4` instead of `(s16) tmp * 4`.  Everything else is the s33 candidate
 * verbatim (including s33's PLUS-operand flip `off + (s32)table`, which is still
 * load-bearing: the base-first control z2b re-measures 21/23).
 *
 * WHY DROPPING THE REDUNDANT NARROWING IS WORTH TWO POINTS (dump-verified).  The
 * wrap test `(s16) tmp >= arg0[3]` already forces the sll16/sra16 pair, so the extra
 * `(s16)` in the offset expression bought no instruction -- it only changed WHERE in
 * the block the offset's defining insn sat.  With the cast, RTL-expand emits
 * sll16, sra16, then `sll off,sra,2` (offset defined 3rd); without it, the offset is
 * `sll off,tmp,2` defined FIRST and the sign-extension pair follows.  Two consequences,
 * both measured:
 *   (1) the offset insn lands at the same position in the block as the target's
 *       `addu $s3,$v0,$zero` (asm/funcs/func_80057CC8.s:29), which is the whole
 *       two-point gain -- the emitted registers are IDENTICAL to the s33 form;
 *   (2) the offset allocno's live range grows from 16 to 19 insns
 *       (tmp/grind/func_80057CC8/dumps/text1b.lreg: "Register 104 used 3 times across
 *       19 insns"), which flips the global allocation ORDER from `... 87 104 72 ...`
 *       to `... 87 72 104 ...` in text1b.greg -- arg0 is now allocated BEFORE the
 *       offset.  The dispositions do NOT change (72 in 19, 104 in 17), because arg0
 *       carries hard-reg conflicts on 16/17/18 (text1b.greg "72 conflicts: ... 16 17 18")
 *       and can only take $19 whatever its rank.  See evidence.md s34-E3.
 *
 * THE REGISTER RESIDUAL IS NOW FULLY ATTRIBUTED (s34-E4, new this session).  Ours is
 * $16 cys, $17 next-address, $18 cxs, $19 arg0; the target is $16 cys, $17 cxs,
 * $18 arg0, $19 next-index.  The cause is LOCAL-alloc, not global-alloc: our
 * next-neighbour address is a block-local call-crossing quantity (text1b.lreg
 * "Register 88 used 3 times across 4 insns in block 4; crosses 1 call"), so local-alloc
 * hands it a callee-save seat ($17) BEFORE the two centre twins are placed, pushing
 * cxs to $18 and blocking arg0 out of 16/17/18.  The target has no such block-local
 * quantity: its call-crossing value is the next INDEX, defined in two blocks and used
 * in a third, so it is a GLOBAL allocno and the twins take $16/$17 uncontested.
 * Making our address global (z2: select the ADDRESS in the wrap arm, no `off` pseudo)
 * does reproduce the target's $16 cys / $17 cxs exactly at no instruction cost -- but
 * the address then outranks arg0 under allocno_compare (4 refs, floor_log2(4)=2) and
 * takes $18, and the vertex-table base moves out of $6 into $4; net 21, one worse.
 *
 * WHY IT IS STILL NOT 0: unchanged from s30b/s31/s32/s33 -- asm/funcs/func_80057CC8.s
 * loads 0x4($s2) TWICE (:17 `lw $a2`, :50 `lw $a0`), which is what gives the target's
 * arg0 SIX references and lets it outrank the neighbour allocno.  A ban-compliant form
 * emits one load and five references.  That residual is the policy question refused
 * 2026-07-20 and standing-ruled 2026-07-27, not a spelling.
 *
 * MEASURED-INERT THIS SESSION (all still 20 at 108 insns): `tmp << 2` instead of
 * `tmp * 4`; centre coordinates read as `((u16 *)table)[arg1*2]`; a named
 * prev-neighbour address local; the prev reads spelled as byte-offset pointer
 * arithmetic; `s16 prev_idx` instead of `unsigned short`; forming the address after
 * `pi`; `tmp`/`off` at function scope.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        s32 off = tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
        next_vert = (s16 *)(off + (s32)table);
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
