/* func_800858D0 (title_mv_exec2) — MATCHED, sandbox --disable all = 0
 * (s2, 2026-08-20, build_insns 72 == target 72, frame 96, saves 3,
 * per-instruction-word disassembly identical to asm/funcs/func_800858D0.s
 * modulo relocations).
 *
 * THE KEY (s2 discovery, closes the whole s1 frontier in one move): the loop
 * is spelled with a LABEL + trailing `if (...) goto loop;` instead of
 * do/while. A goto-spelled loop emits NO NOTE_INSN_LOOP_BEG/END, so loop.c
 * (move_movables/LICM) never runs on it:
 *   - the single-set constant pseudos for 24 and 1 are NOT hoisted (kills the
 *     entire multi-set-carrier machinery the old floor-22/15 candidates
 *     needed) — resolving H3's paradox: target's constants ARE plain
 *     single-set spellings, they just never met loop.c;
 *   - single-set => birthing_insn_p (sched.c:2504-2526, reg_n_sets==1 gate)
 *     => adjust_priority LAUNCH inheritance (sched.c:2584-2590) => each li is
 *     scheduled ADJACENT to its consumer (li v1,24 next to its sh; li v0,1
 *     next to the sllv/jal), exactly target's clusters;
 *   - the a0 = sp+16 hard-reg set (multi-set hard reg, never launchable)
 *     becomes the lone priority-1 straggler, lands at sched1 block top, and
 *     reorg steals it into the preheader + loop-back delay slot (target's
 *     rotation identity), which also holds hard $a0 live across the
 *     sign-extend span => local-alloc puts the sign-extend in $a1 and the 24
 *     in $v1 (both parts of the ra_solver Phase 5 two-part requirement land
 *     for free).
 * The banked A/B statement moves are retained (buf[1]=0x60093 before
 * var_s0 = 0; the 0x3C store last in the init block).
 * SOTN-master precedent for goto-spelled loops in matched PSX code:
 * docs/reference/sotn-construct-index.md:1033 (src/main/main.c:40
 * main_search_loop_1), :1015 (src/dra/5F60C.c:579), :1035
 * (src/main/psxsdk/libc/sprintf.c:96).
 * Body below is exactly what is applied over the INCLUDE_ASM at
 * src/main.c:857 (externs at src/main.c:848-855 already present). */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (D_80101BCC != 0) {
        ff = 0xFF;
    loop:
        offset = var_s0 * 54;
        *(s16 *)((u8 *)&D_800F4E1A + offset) = 0x18;
        *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
        *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
        *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
        *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
        *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
        buf[0] = 1 << var_s0;
        func_8008B488(buf);
        D_8010280A = var_s0;
        func_800871D4(1);
        var_s0 = var_s0 + 1;
        if (var_s0 < D_80101BCC) {
            goto loop;
        }
    }
}
