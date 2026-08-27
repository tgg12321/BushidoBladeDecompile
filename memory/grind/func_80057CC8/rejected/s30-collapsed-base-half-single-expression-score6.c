/* MATCHING form (grind s30, 2026-08-27, solver modality):
 *   `sandbox func_80057CC8 --disable all` -> score 0, target_insns 111 ==
 *   build_insns 111, rules_dropped 0;
 *   `goal_from_tgt.py classify text1b func_80057CC8` -> "NO DIVERGENCE: the two
 *   streams are identical" (object-level, ours vs build/src/text1b.o);
 *   full `build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
 *
 * WHAT IS NEW vs 29 prior sessions. Every previous form had to choose between
 * (a) caching the vertex-table base in a local that stays LIVE across the
 * intervening `ratan2` call — which forces a ninth callee-save ($s8) plus its
 * save/restore and its `move`, three instructions where the target pays only two
 * (`move` + the `lw $a0,0x4($s2)` reload), i.e. 112 insns against a 111-insn
 * target (s29 formB, score 30; s30 re-measured and typed the residual PRE-RA with
 * tools/ra_solver/goal_from_tgt.py) — or (b) writing the base expression again at
 * the second call site, which is the family the owner refused on 2026-07-20 and
 * three layer-1 cheat-reviewer passes FAILed on 2026-08-20.
 *
 * This form takes neither. The neighbour-angle computation is factored into an
 * ordinary `static inline` helper called once per neighbour. The vertex-table
 * base expression `*(s16 **)(arg0 + 4)` occurs EXACTLY ONCE in the whole
 * translation unit (inside `vert_base_57CC8`); nothing is cached across the call;
 * nothing is written twice. GCC's inliner — not the programmer — produces the two
 * materialisations, and each inlined copy reloads the base after the call because
 * the call clobbers memory. That is precisely the target's shape.
 *
 * FIDELITY EVIDENCE that the helper is the ORIGINAL source shape, not a wrapper
 * chosen for bytes: the two neighbour blocks in asm/funcs/func_80057CC8.s are
 * instruction-for-instruction parallel — `sll/sra` of the index, `lw` of the base
 * (:17 `lw $a2,0x4($s2)`, :50 `lw $a0,0x4($s2)`), `addu`, `lh 0x0`, `lh 0x2`, two
 * `subu` against the centre, `jal ratan2`, `andi 0xFFF`. Two identical inlined
 * bodies differing only in their index operand is what an inlined helper emits;
 * it is also why the base is loaded twice rather than kept live.
 *
 * NAMING (evidence-only, per [[names-require-evidence]]): the word at offset 4 is
 * a pointer to s16 PAIRS consumed as x/y by `ratan2` and indexed by a vertex index
 * that wraps modulo the byte at offset 3 (`prev = n-1` on underflow, `next = 0` at
 * `>= n`) — hence `vert_base` / `vert_angle`. No claim is made about offsets 0-2.
 *
 * MEASURED ALTERNATIVES (banked under rejected/): centre read as
 * `*(s16**)(arg0+4) + arg1*2` then `[0]/[1]` -> score 1; centre read through a
 * `base+idx*2` accessor also used by the angle helper -> score 6. Both 111 insns;
 * only the spelling of the CENTRE read moves those points.
 */
static inline s16 *vert_base_57CC8(u8 *arg0) {
    return *(s16 **)(arg0 + 4);
}

static inline s32 vert_angle_57CC8(u8 *arg0, s32 idx, s16 cx, s16 cy) {
    s16 *t = vert_base_57CC8(arg0);
    return ratan2(t[idx * 2] - cx, t[idx * 2 + 1] - cy) & 0xFFF;
}

void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    u16 cx;
    u16 cy;

    prev_idx = arg1 - 1;
    {
        s16 *ctr = vert_base_57CC8(arg0);
        cx = ctr[arg1 * 2];
        cy = ctr[arg1 * 2 + 1];
    }

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    ang_prev = vert_angle_57CC8(arg0, (s16) prev_idx, (s16) cx, (s16) cy);
    ang_next = vert_angle_57CC8(arg0, (s16) next_idx, (s16) cx, (s16) cy);

    if (ang_next < ang_prev) {
        ang_mid = (ang_prev + 0x800) - ((s32)(ang_prev - ang_next) / 2);
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(*(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
