/* =====================================================================
 * func_80056CB8 — CANDIDATE (s2, 2026-09-16; re-verified s3, 2026-09-16;
 * re-verified s4, 2026-09-16) — floor 106/204, NOT YET 0
 * ---------------------------------------------------------------------
 * s4 (permuter modality): body UNCHANGED from s2/s3. Re-confirmed floor
 * 106 before permuting. Ran a first-ever directed permuter campaign
 * (44,294 iterations, 24 min, base permuter score 5235) on this exact
 * chassis -- best find 4103/5235 (21.6% reduction), plateaued, and the
 * best form itself is a type-broken mutation, not a valid lever. KILLED
 * (instance): permuter-as-primary-lever does not close (or meaningfully
 * narrow) this residual, because the residual is PURE register
 * allocation on an already-structurally-matching body, which the
 * permuter's semantic-mutation search is poorly suited to explore. See
 * hypotheses.md [s4] + evidence.md [s4] for the full workspace/campaign
 * writeup. NEXT SESSION should use register-alloc/rederive/solver
 * modality (read the .lreg/.greg dumps for pseudo 82/83/90 live ranges),
 * NOT another permuter campaign on this chassis (2-permuter-session cap
 * per R3; this was campaign 1 of 2 max).
 * ---------------------------------------------------------------------
 * s3 (structural modality): re-applied this body + the func_80053614
 * prerequisite fix, re-confirmed score 106 (baseline WITHOUT the fix is
 * 147 — the fix is load-bearing, not optional). Tried two DECLARATION-
 * order probes (flags-before-obj; r1-declared-next-to-flags) distinct
 * from s2's already-killed STATEMENT-order probe — both measured ZERO
 * effect on the obj/flags/r1 register assignment or score. All three
 * order-permutation probes across s2+s3 are now killed; see
 * hypotheses.md's s3 entries and "Frontier for s4" for the next
 * register-alloc-specific (not order-based) levers to try. No code
 * change applied to this banked candidate.c this session — it is
 * unchanged from s2.
 * ---------------------------------------------------------------------
 * This is the WORKING FRONTIER, not a finished match. sandbox --disable
 * all reports score 106 (build_insns 201 vs target_insns 204) as of this
 * session. Applied directly from the s1 ledger's inherited
 * authored-draft-2026-08-18.c (which was NEVER applied to src/ before
 * this session — the function was still INCLUDE_ASM going into s2).
 *
 * PREREQUISITE CHANGE IN THE SAME TU (already applied to src/text1b.c,
 * verified byte-neutral this session — sandbox func_80053614 --disable
 * all still scores 0/32 after the change):
 *   func_80053614's return type was `void`; changed to `s32` with
 *   `return func_80052D00(arg2, arg3);` as its last statement (was a
 *   bare call). func_80053614's own asm (asm/funcs/func_80053614.s)
 *   ends `jal func_80052D00` immediately followed by the epilogue with
 *   no further $v0 traffic, so $v0 falls through to the caller
 *   regardless of the C-level return type — this is why the change is
 *   byte-neutral for func_80053614 itself. func_80056CB8's two calls to
 *   it DO consume the return value ($v0 feeds `move $22,$2` /
 *   `sll $2,$2,1` in the target asm), so the void declaration must be
 *   fixed for this candidate to type-check and to carry the value.
 *
 * WHAT MOVED THE FLOOR THIS SESSION (both measured, both banked in
 * hypotheses.md as CONFIRMED):
 *   1. First full-body draft applied verbatim from the s1 authored
 *      draft (see PROVENANCE below) + the func_80053614 signature fix:
 *      204 -> 126.
 *   2. `work` widened from `s32 work[2]` (8 bytes) to `s32 work[4]`
 *      (16 bytes): 126 -> 106, and the function's `.frame` size became
 *      an EXACT match to target (168/0xA8 bytes, `vars=104` — dumped
 *      and confirmed via tmp/grind/func_80056CB8/dumps/text1b.s). This
 *      resolves authored-notes-2026-08-18.md's [S7] unknown: `work` is
 *      16 bytes, not 8.
 *
 * REMAINING RESIDUAL (106) — REGISTER ALLOCATION, NOT STRUCTURE.
 * Side-by-side against tmp/grind/func_80056CB8/dumps/text1b.s vs
 * asm/funcs/func_80056CB8.s this session: the INSTRUCTION SEQUENCE
 * (mnemonics, branch structure, load/store order, the *0x7D fallback
 * expansion, the double-read of obj+0xC0, the double-read of obj+0xB8)
 * matches target line-for-line region by region. The frame layout
 * matches exactly (pt0@0x18, pt1@0x28, hit0@0x38, hit1@0x48, work@0x58,
 * spill slots @0x60/0x68/0x70/0x78, saved regs @0x80-0xA7, total 0xA8).
 * What does NOT match is WHICH hard register each pseudo lands in:
 *   target: flags/r1 share $s0, obj=$s1, x=$s2, z=$s3, cos_p=$s4,
 *           sin_p=$s5, i(loop)=$s6, arg0=$s7
 *   ours:   obj=$s0(=$16), flags/z share $s1(=$17), x=$s2(=$18, MATCHES),
 *           cos_p=$s3(=$19), i(loop)=$s4(=$20), sin_p=$s5(=$21, MATCHES),
 *           r1=$s6(=$22), arg0=$s7(=$23, MATCHES)
 * Only arg0, x and sin_p land in the same hard register as target; every
 * other local is systematically off by one-or-two slots in the s0-s7
 * ring. This is a global-allocation ordering question (global.c pseudo
 * numbering / allocno priority), NOT a source-order artifact — see the
 * KILLED hypothesis below (swapping the `obj`/`flags` statement order
 * to flip which gets the lower pseudo number had ZERO effect on either
 * the register assignment or the score; the compiler already coalesces
 * `obj = arg0;` before assigning pseudo numbers in this shape).
 *
 * NEXT SESSION: this needs the register-alloc-pure-c lever family
 * (.claude/rules/register-alloc-pure-c.md) — read the .greg dump
 * (tmp/grind/func_80056CB8/dumps/text1b.greg once regenerated) for the
 * actual conflict graph and allocno priorities driving this specific
 * assignment, rather than guessing more C-order permutations blind.
 * A `register-alloc` or `rederive` modality session, not `structural`.
 *
 * PROVENANCE: this file's C body is inherited verbatim (plus the [S7]
 * work-size fix) from memory/grind/func_80056CB8/authored-draft-2026-08-18.c,
 * itself derived from m2c + hand analysis in
 * memory/grind/func_80056CB8/authored-notes-2026-08-18.md. See that
 * file's SYMBOL / TYPE MAPPING and SUSPICIOUS SPOTS sections for the
 * full derivation of every field offset and the *0x7D fallback multiplier.
 * ===================================================================== */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820;
extern u8 D_8009A821;

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 ang;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 r1;
        s32 r2;
        s32 code;

        obj = arg0;
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            ang = flags + *(s16 *)(obj + 0x1CA);
        } else {
            ang = flags + ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                                  D_800F6608.w8 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (ang & 0xFFF);
        cos_p = &Judge + ((ang + 0x400) & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;

        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);

        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt1[2] = z;

        r1 = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (r1 != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        r2 = func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8);

        code = (r1 | (r2 << 1)) + 1;
        if (code == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                code = 0;
            }
        } else if (code == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        code = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        code = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)code;
    }
}
