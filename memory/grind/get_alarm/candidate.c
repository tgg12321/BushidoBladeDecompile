/* Candidate body for get_alarm (formerly func_8007DC9C) - HONEST SANDBOX SCORE 0
 * (measured s45, 2026-09-04: `& tools/wteng.ps1 main sandbox get_alarm --disable all`
 * = score 0, target_insns 91, build_insns 91, rules_dropped 0, cheat_asm_stripped 147
 * - the 147 is display.c's pre-existing unrelated INCLUDE_ASM/canonical material,
 * unchanged from the floor-9 baseline, i.e. NOTHING in this candidate was stripped).
 *
 * THE FUNCTION IS SOLVED IN PURE C.  It is blocked ONLY by a surface a grind
 * session may not touch: two lines in volatile_extern_allowlist.txt.  See the
 * INTEGRATION HANDOFF entry filed in docs/grind/decisions.md (2026-09-04, s45).
 *
 * ============================ THE COMPLETE RECIPE ============================
 * 1. src/display.c - the body below, replacing INCLUDE_ASM("asm/funcs", get_alarm);
 * 2. src/display.c - THREE declaration changes (all three load-bearing; each measured
 *    independently this session):
 *      a. line ~758  extern s32 D_8009BF68[];   ->  extern volatile s32 D_8009BF68[];
 *                    floor 9 -> 6   (axis A; Judge PASS ruling 2026-09-04 13:48,
 *                    docs/grind/decisions.md:22292, granting Ruling 4)
 *      b. lines 20 AND 756  extern u32 *g_gpu_dma_madr;
 *                                                ->  extern volatile u32 *g_gpu_dma_madr;
 *                    floor 6 -> 5   (type-level MMIO volatile; ORDINARY C - see below)
 *      c. line ~750  extern s32 D_8009BF78;     ->  extern volatile s32 D_8009BF78;
 *                    floor 5 -> 0   (Ruling 4, same class as (a); NEEDS A GRANT)
 * 3. volatile_extern_allowlist.txt - one entry each for D_8009BF68 and D_8009BF78
 *    (exact text reproduced in the decisions.md entry and in
 *    tmp/grind/get_alarm/s45/).
 * 4. tools/grinder/scope_allow.txt - "get_alarm volatile_extern_allowlist.txt"
 *    (driver-written on a Judge ESCALATE(integration-handoff); exact precedent is the
 *    "SioSyncroRead volatile_extern_allowlist.txt" line already in that file).
 *
 * WHY (b) IS ORDINARY C, NOT A COERCION.  g_gpu_dma_madr holds the address of the PS1
 * DMA channel-2 MADR hardware register (0x1F8010A0), inside the MMIO window
 * 0x1F801000-0x1F802FFF that .claude/rules/mmio-volatile-type-level.md sanctions at
 * TYPE level with no annotation.  Its two siblings in the SAME declaration block
 * already carry exactly this type (extern volatile u32 *g_gpu_stat_reg; line 18,
 * extern volatile u32 *g_gpu_dma_chcr; line 22), and src/display.c:709 already writes
 * through this very pointer as *(volatile u32 *)g_gpu_dma_madr = a0; - the cast at the
 * use site is the same claim spelled worse.  Moving the qualifier onto the declaration
 * makes the header self-consistent; it is a correction, not a lever.  The volatile
 * lives on the POINTED-TO type, not on the pointer object, so
 * engine/volatile_cheats.py pattern 3 does not fire and NO allowlist entry is needed
 * (confirmed: cheat_asm_stripped stayed 147 across the 6 -> 5 measurement).
 *
 * WHY (a) AND (c) NEED THE ALLOWLIST.  Both are `extern volatile T G;` on a splat
 * global = volatile_cheats.py pattern 3.  Un-allowlisted, the cheat-invisible sandbox
 * STRIPS them (measured: adding (a) alone with no allowlist entry moved
 * cheat_asm_stripped 147 -> 148 and left the score at 9).  Allowlisted, they stand and
 * the score is honest.  Both qualify under Ruling 4 - the ground-truth-codegen volatile
 * class, granted 2026-07-10 (commit c80d976e; text recoverable at
 * `git show cd19d7a2^:docs/closer/rulings.md` lines 68-83, the file having been deleted
 * as dead-era docs in cd19d7a2; live policy, applied at docs/grind/decisions.md:6891
 * and :11004): "For census-proven Sony library module state (symbol identity
 * reloc-proven against the verbatim-linked SDK object), where the Sony object's code is
 * MEASURED unreachable without `volatile` (non-volatile build demonstrably collapses
 * ordering/re-reads), volatile is legal as ORIGINAL SEMANTICS - no in-binary IRQ-writer
 * prong required."
 *   - Census identity: get_alarm @0x8007DC9C is a member of a VERBATIM-matched LIBGPU/SYS
 *     module placement (memory/closer/psyq-library-census.md:246).  D_8009BF68 and
 *     D_8009BF78 are module-local state of that same Sony libgpu sys.c object: their only
 *     program-wide consumers are get_alarm, _addque2, _exeque and _sync - every one a
 *     verbatim LIBGPU/SYS member.  D_8009BF78/D_8009BF7C are libgpu's _qin/_qout queue
 *     indices (the (_qin - _qout) & 0x3F argument below is verbatim SOTN libgpu sys.c),
 *     and D_8009BF7C ALREADY holds a volatile grant in volatile_extern_allowlist.txt -
 *     (c) unifies the pair.  Independently, D_8009BF78 also satisfies the ORIGINAL
 *     two-prong IRQ carve-out on the use-site side: src/display.c:796
 *     `while (D_8009BF78 != D_8009BF7C) { _exeque(); ... }` is a cataloged spin-wait.
 *   - Measured unreachable without volatile: 9 -> 6 -> 5 -> 0, each step one declaration
 *     and one sandbox measurement, all this session, chassis-current.
 *
 * ======================= WHAT THE THREE LEVERS ACTUALLY DO =======================
 * Attribution is dump-proven, not inferred (tmp/grind/get_alarm/dumps/, regenerated
 * this session on the post-volatile-BF68 chassis).
 *
 * (a) D_8009BF68 volatile - AXIS A, worth 2.  display.combine insn 73 keeps
 *     (set (reg 92) (symbol_ref "D_8009BF68")) alive with its single consumer at insn
 *     81 flagged mem/s/v: MEM_VOLATILE_P blocks combine's address substitution AT A
 *     SINGLE USE, emitting target's lui/addiu %lo pair.  The 44-session added_sets_2 /
 *     "needs a second use" story was WRONG and is retired; every multi-use device
 *     (s44's F1 chain-extender) measured 13 precisely because a second use makes
 *     combine keep the la AND fold the first use.
 *
 * (b) g_gpu_dma_madr pointed-to volatile - AXIS B PART 1, worth 1, and it is what moves
 *     the DEAD READ'S SEAT.  In GCC 2.7.2's sched.c a volatile MEM is a full memory
 *     barrier: it depends on every prior memory ref and every later one depends on it.
 *     Making *g_gpu_dma_madr (RTL insn 56) volatile forbids sched1 from hoisting it -
 *     and with it the madr POINTER load (insn 54) - above the volatile dead read (insn
 *     38).  Consequence measured in the bytes: the discarded *g_gpu_stat_reg; read
 *     stops being seated in $a0 and lands in $v0, exactly as target does at
 *     0x8007DCFC.  This is the $v0 seat that 44 sessions of statement-order
 *     permutation, pointer aliases and F1 chain-extenders could not produce - because
 *     the seat was never a local-alloc quantity-table problem at all, it was a
 *     scheduler memory-dependence problem, one pass earlier.
 *
 * (c) D_8009BF78 volatile - AXIS B PART 2, worth the last 5.  With (a)+(b) in place the
 *     residual was a 4-slot window: mine emitted [fmt, statptr, BF78, deadread] where
 *     target has [statptr, fmt, deadread, BF78].  Marking _qin volatile chains its load
 *     (RTL insn 43) into the volatile ordering set with the dead read (38) and
 *     D_8009BF7C (45, already volatile), pinning 38 before 43 and freeing insn 60 (the
 *     format-string symbol_ref, dependence-free) to settle where target has it.
 *     NOTE: measured ALONE on the floor-6 chassis (without (b)) this same declaration
 *     scored 12 - strictly WORSE.  The three levers are ORDER-DEPENDENT and only the
 *     full triple is 0; that is why 44 sessions of one-lever-at-a-time probing missed it.
 *
 * The body itself is unchanged from the s44/s45 reference-faithful spelling: it is the
 * SOTN-libgpu sys.c int get_alarm(void) (tmp/sotn/src/main/psxsdk/libgpu/sys.c:937-946)
 * with BB2's v1.129 deltas (inlined _reset(1), the second debug printf, 0xF0000 draw
 * limit, pointer globals for the GPU registers).  Zero FAKE constructs, zero named
 * intermediates beyond the one real SetIntrMask result, zero coercion locals.
 */
s32 get_alarm(void) {
    s32 temp_v0;
    if (g_gpu_vcount < VSync(-1) || g_gpu_draw_count++ > 0xF0000) {
        *g_gpu_stat_reg;
        printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = D_8009BF7C;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}

/* ===================== s46 UPDATE (2026-09-04) — READ THIS FIRST =====================
 * The recipe above is UNCHANGED and is now proven at the full-build level:
 *   sandbox get_alarm --disable all      -> score 0, 91/91, rules_dropped 0,
 *                                           cheat_asm_stripped 147 (baseline value)
 *   verify-oracle --rebuild --allow-dirty -> ok true, build_sha1 ==
 *                                            62efab4f73f992798c43e8c730aa43baa10bb4fa
 *   build (full clean-driver build)       -> MATCH
 * (`verify-oracle --rebuild` alone REFUSES on dirty build inputs; the flag you need to
 *  prove a candidate tree is `--rebuild --allow-dirty`.)
 *
 * The authoritative diff is now memory/grind/get_alarm/s46-score0-verified-diff.txt,
 * NOT the s45 one: it additionally reconciles the two duplicate/contradictory
 * declaration sites the 2026-09-04 14:08 Judge ruling ordered fixed —
 *   src/display.c:721  extern s32 D_8009BF78;                  -> volatile (matches :750)
 *   src/display.c:733  extern s32 (*D_8009BF68)(s32*, s32);    -> extern volatile s32
 *                                                                 D_8009BF68[];
 *                                                                 (matches :758)
 * With that, all three symbols are declared consistently at both of their decl sites.
 *
 * STILL BLOCKED ON EXACTLY ONE LINE a grind session may not write:
 *   tools/grinder/scope_allow.txt  ->  `get_alarm volatile_extern_allowlist.txt`
 * Packet: docs/grind/decisions.md, "## 2026-09-04 (s46) — get_alarm ... OWNER-ESCALATION
 * — INTEGRATION HANDOFF (RE-FILED, all three 14:08 Judge defects cured)".
 * Once the grant exists: apply s46-score0-verified-diff.txt, confirm sandbox 0, and
 * submit candidate-ready with memory/grind/get_alarm/self_vet.md (already rewritten for
 * this exact diff).
 * ==================================================================================== */
