/* func_80032314 — best form as of grind session 4 (permuter).
 * *** THIS FORM MEASURED sandbox --disable all == 0 (109/109 insns, 0 rules)
 * on 2026-08-20, WITH the FAKE annotation in place, edits applied to
 * src/code6cac_b.c (candidate-ready submission of s4). ***
 * Artifacts: tmp/grind/func_80032314/s4/{allocdbg.txt,permB/output-0-1/}.
 *
 * THE CONSTRUCT: one single-level `do { ... } while (0);` wrap around the
 * post-(a0==4) body, FAKE-annotated at the construct site. Family:
 * .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06 FINAL):
 * "do { <any body> } while (0); — including empty bodies — is a sanctioned
 * pure-C match device for ANY codegen effect, including register
 * allocation." Single-level wraps carry NO exhaustion hard-gate (rule
 * prereq 2); nested-only gating does not apply here. Confirmed RA-purposed
 * application precedent: marionation_Exec (rule file line 99).
 *
 * MECHANISM (measured, ALLOCDBG s4): the wrap emits NOTE_INSN_LOOP notes;
 * flow.c weights reg_n_refs by loop_depth for refs inside them. Weighted
 * refs: walker p74 = 12 (len 82, pri 4390), ent p75 = 10 (len 63, pri
 * 4761), mult-temp p116 = 4 (len 10, pri 8000). Allocation order mult-temp
 * > ent > walker seats them $a1/$a2/$a3 = target exactly; t0->$t0, t1->$t1.
 * Same end allocation as the s3 F4 chain-extender, reached via loop-note
 * weighting instead of combine-stale refs. Contrast the s2 do-while form
 * that FAILED (score 59): that one wrapped the REAL outer loop, arming
 * loop.c LICM/giv on a genuine back-edge; this wrap is a straight-line
 * once-through region — loop.c finds nothing to hoist (byte-identical
 * emission, permuter score 0 and sandbox 0).
 *
 * PROVENANCE: found by the s4 permuter campaign (basin B, s4_a3rotation_
 * random, iter ~3284, tmp/grind/func_80032314/s4/permB/output-0-1/), vetted
 * against the cheat catalog + the do-while-zero-exception prerequisites,
 * self-vet in memory/grind/func_80032314/self_vet.md. Basin A (the floor-1
 * natural-geometry chassis) ran 32,797 iterations / 27 min with ZERO finds
 * — the pure-C no-FAKE case stays closed (s2 arithmetic + s3 premise-hole
 * measurements + this null).
 *
 * SESSION-2 WIN preserved: radius read via *a3 (walker at its byte-forced
 * 8 real refs). SESSION-1 WIN preserved: GTE island as ONE mnemonic block
 * (mtc2/swc2), never .word (evidence.md FINDING 2).
 *
 * ALTERNATIVE ENDGAME (superseded but valid): the s3 F4 chain-extender
 * form (ent+dxs/-dxs detour) also measures 0 — spec + spelling in
 * evidence.md [s3] and this file's git history. It needs the driver's
 * full-ladder exhaustion gate (dead-store-fake-exception prong a); the
 * do-while single-level wrap does not, which is why s4 submits the wrap.
 */
void func_80032314(void) {
    u8 *t0 = &D_80104E88;
    s32 t1 = 0;
    u8 *a3 = &D_80104E88 + 2;
    u8 *ent;
    s32 state;
    s32 a0;

loop:
    if (*t0 == 0) goto next;
    {
        s32 v1_v = (*(u8 *)(a3 + 1) == 0);
        v1_v = v1_v * 0x44C;
        ent = v1_v + &D_80101EC8;
    }
    state = *(u16 *)(ent + 0x6A);
    a0 = state & 0xFFFF;
    if (a0 == 4) goto next;
    /* FAKE: single-level do-while(0) wrap (body executes once), mechanism:
     * the wrap's NOTE_INSN_LOOP notes make flow.c weight in-wrap reg_n_refs
     * by loop_depth, re-ranking global.c allocno priorities (walker 4390 <
     * ent 4761 < mult-temp 8000) into the target $a1/$a2/$a3 seating —
     * ALLOCDBG trace tmp/grind/func_80032314/s4/allocdbg.txt.
     * lever-exhaustion: memory/grind/func_80032314/hypotheses.md (s2
     * arithmetic closure of the pure-C rotation + s3 premise-hole
     * measurements + s4 permuter nulls on the natural-geometry chassis). */
    do {
    if (a0 == 0x14) goto next;
    if (a0 == 0xF) goto next;
    if ((u32)(state - 0x1C) < 2) goto next;
    if ((u32)(state - 0x1E) < 2) goto next;
    if ((u32)(state - 0x20) < 2) goto next;
    if (a0 == 0x11) goto next;
    {
        s32 dx = *(s32 *)(ent + 0xF4) - *(s32 *)(a3 + 2);
        s32 dy = *(s32 *)(ent + 0xF8) - *(s32 *)(a3 + 6);
        s32 dz = *(s32 *)(ent + 0xFC) - *(s32 *)(a3 + 0xA);
        u32 dist_sq;
        u32 log2_val;
        dist_sq = (u32)(dx * dx + dy * dy + dz * dz);
        if (dist_sq < 0x400) {
            log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
        } else {
            s32 clz = 0;
            s32 sp_tmp;
            if ((s32)dist_sq >= 0) {
                /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
                 * canonical inline asm, identical to the user-authorized block in
                 * the matched sibling func_800274BC (src/code6cac_b.c:292). */
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
                    "nop\n"
                    "nop\n"
                    "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
                    "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
                    : "=m"(sp_tmp)
                    : "r"(dist_sq)
                    : "$12");
                clz = sp_tmp;
            }
            {
                u32 v0_m = (u32)-2;
                u32 v1_m;
                u32 idx;
                u32 hi;
                v0_m &= clz;
                v1_m = 0x16 - v0_m;
                idx = dist_sq >> v1_m;
                v1_m = v1_m >> 1;
                hi = (u32)((u8)(*((&D_8008D118) + idx)));
                log2_val = (hi << 16) >> (0x13 - v1_m);
            }
        }
        {
            s32 v1 = *a3;
            s32 v0 = v1 << 4;
            v0 = v0 - v1;
            v0 = v0 << 1;
            v0 = v0 + 0x1F4;
            if (log2_val < (u32)v0) {
                *(s16 *)(ent + 0x286) = 5;
                *t0 = 0;
            }
        }
    }
    } while (0);
next:
    t1 += 1;
    a3 += 0x2C;
    t0 += 0x2C;
    if (t1 < 4) goto loop;
}
