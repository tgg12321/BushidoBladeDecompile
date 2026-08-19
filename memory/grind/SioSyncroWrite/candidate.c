/* candidate.c - SioSyncroWrite (formerly func_8008C1E8), grind session s5 [forensics], 2026-08-19
 *
 * RE-VERIFIED on the live chassis by the following [forensics] session
 * (2026-08-19, the one that filed the OWNER-ESCALATION - INTEGRATION HANDOFF
 * entry in docs/grind/decisions.md): identical numbers, and the expand_increment
 * attribution below was re-derived from that session own dumps
 * (tmp/grind/SioSyncroWrite/s4/s4.SioSyncroWrite.{rtl,flow,combine,greg}.slice;
 * volatile post-increment insns 161/163/165/167 and 173/175/177/179, both dead
 * reloads still present after combine and flow). cheat_asm_stripped read 67 on
 * the re-measurement (66 previously) - a whole-file count, not a property of
 * this body.
 *
 * MEASURED on the live chassis, with this body + the file-scope
 * declaration change below in place in src/main.c:
 *     sandbox SioSyncroWrite --disable all -> "score": 0
 *         (target_insns 159, build_insns 159, rules_dropped 1, cheat_asm_stripped 66)
 *     sandbox SioAnsyncWrite --disable all -> "score": 0
 *         (target_insns 25, build_insns 25)   <- the struct-sharing sibling is unharmed
 * Zero register pins, zero inline asm, zero rules touched, src/main.c only.
 *
 * ===== THE ONE CHANGE s5 MADE TO THE s4 BODY ==========================
 * s4's body reached 0 but was BOUNCED by the layer-1 cheat-reviewer
 * (docs/grind/decisions.md, 2026-08-19 00:25) for declaring
 *     volatile s32 *flag = &D_800F1AEC;      (+ its loop copy `st`)
 * over a non-volatile `extern s32 D_800F1AEC;` - a volatile qualifier added
 * by pointer-type coercion, which the legitimate-volatile-interrupt-touched
 * carve-out explicitly excludes. That construct is now BANNED for this
 * function.
 *
 * s5 removes the coercion entirely by declaring the symbol honestly:
 *
 *     src/main.c:3431   - extern s32 D_800F1AEC;
 *                       + extern volatile s32 D_800F1AEC;
 *     src/main.c:3437   SioAnsyncWrite's local becomes
 *                       volatile s32 *flag = &D_800F1AEC;   (type now FOLLOWS
 *                       the declaration; nothing is added to it)
 *
 * With that decl in place, `volatile s32 *flag = &D_800F1AEC;` in this body
 * adds NO qualifier - it is simply the type of `&D_800F1AEC`. Both consumers
 * were re-measured at 0 (above). The C is entirely inside src/main.c.
 *
 * ===== WHY VOLATILE IS ORIGINAL SEMANTICS, NOT A COERCION (s5 forensics) =
 * Dump-grounded, from tmp/grind/SioSyncroWrite/s4/{vol,nonvol}.rtl.slice
 * (cc1 -da, project flags via tools/grinder/dump.ps1):
 *
 * `expand_increment` (gcc/expr.c) emits a DIFFERENT insn sequence for a
 * post-increment whose value is discarded, depending on MEM_VOLATILE_P:
 *
 *   non-volatile MEM   insn 161 (set r104 (mem/s   (plus r89 4)))   load
 *                      insn 163 (set r105 (plus r104 1))            add
 *                      insn 165 (set r106 r105)                     copy
 *                      insn 167 (set (mem/s (plus r89 4)) r106)     store
 *                      -> the expression's value is the PRE-increment temp;
 *                         the copy dies in cse/flow. Machine: lw/addiu/sw.
 *
 *   volatile MEM       insn 161 (set r104 (mem/s/v (plus r89 4)))   load
 *                      insn 163 (set r105 (plus r104 1))            add
 *                      insn 165 (set (mem/s/v (plus r89 4)) r105)   store
 *                      insn 167 (set r106 (mem/s/v (plus r89 4)))   RELOAD
 *                      -> a volatile lvalue's post-value cannot be taken from
 *                         the temp, so expansion re-reads memory. r106 is dead
 *                         immediately, but flow.c's insn_dead_p refuses to
 *                         delete an insn whose source has side_effects_p
 *                         (MEM_VOLATILE_P), so the dead `lw` survives to final.
 *                         Machine: lw/addiu/sw/lw.
 *
 * TARGET HAS THE VOLATILE SEQUENCE, twice:
 *     asm/funcs/SioSyncroWrite.s:106-111
 *       lw $v0,0x4($s3); nop; addiu $v0,$v0,0x1; sw $v0,0x4($s3); lw $v0,0x4($s3)
 *     asm/funcs/SioSyncroWrite.s:112-117
 *       lw $v0,0x8($s3); addiu $v0,$v0,-0x1;     sw $v0,0x8($s3); lw $v0,0x8($s3)
 *
 * The whole-body non-volatile spelling was measured this session:
 *     rejected/s4b-nonvolatile-block-pointer-154i.c -> score 10, 154 insns
 *     (5 short: the two dead reloads, plus the $s3 base pointer collapsing
 *      back into lui/%lo forms once its MEMs are foldable).
 * So no non-volatile spelling of this block can reach the target bytes. The
 * volatile qualifier is a byte-visible property of Sony's original LIBCOMB
 * source, not a codegen coercion.
 *
 * ===== THE ONE REMAINING GATE (out of a grind session's reach) =========
 * `extern volatile s32 D_800F1AEC;` is pattern-3 of engine/volatile_cheats.py
 * and is only legal for symbols listed in volatile_extern_allowlist.txt.
 * D_800F1AEC is not listed; its three block-mates ARE
 * (volatile_extern_allowlist.txt:43-45, D_800F1AF0/AF4/AF8, "Ruling-4 class
 * grant"). A grind session cannot land the entry: tools/grinder/grind.ps1:685
 * stages only src/<stem>.c + scope_allow paths into the Match commit, so an
 * allowlist edit is silently dropped and `queue done` then refuses the
 * function. Operator steps + the verified two-prong evidence are in
 * memory/grind/SioSyncroWrite/evidence.md and in the INTEGRATION HANDOFF
 * entry in docs/grind/decisions.md (2026-08-19, s5).
 *
 * Depends on file-scope decls in src/main.c (~3427-3435):
 *   extern volatile u16 D_800F1AE2; extern s32 (*D_800F1AE8)(s32, s32);
 *   extern s16 D_800A3074[4];
 *   extern volatile s32 D_800F1AEC;      <-- CHANGED by s5 (was non-volatile)
 *   extern volatile s32 D_800F1AF0, D_800F1AF4, D_800F1AF8;
 *   extern s32 D_800A3044;
 */
s32 SioSyncroWrite(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 retries;
    s32 pkt_len;
    s32 i;
    s32 (*cb)(s32, s32);

    retries = 0;
    if (*flag != 0) return -1;
    {
        /* FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: memory/grind/SioSyncroWrite/hypotheses.md [s4-M1] (direct-global form measured 158i/mismatch) */
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 mode;
        mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    i = 0;
    if (D_800F1AF4 == 0) goto done;
    for (;;) {
        volatile s32 *st = flag;

        while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
            cb = D_800F1AE8;
            if (cb != 0) {
                s32 prev = retries;
                retries += 1;
                if (cb(2, prev) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    goto done;
                }
            }
        }
        if (i == 0) {
            D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
        }
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        st[1]++;
        i += 1;
        st[2]--;
        if (i == pkt_len) {
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == st[3]) {
                /* FAKE: redundant second handle to D_800F1AF8, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M2] (direct-global form measured 158i/mismatch) */
                volatile s32 *p_af8 = &D_800F1AF8;
                do {
                    cb = D_800F1AE8;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(2, prev) == 0) {
                            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M3] (direct-global form measured 158i/mismatch) */
                            volatile s32 *p_af4b = &D_800F1AF4;
                            DeliverEvent(0xF000000B, 0x100);
                            return (arg1 - *p_af4b) - 1;
                        }
                    }
                } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
            }
            i = 0;
        }
        {
            /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-H1/H2] (plain-global and function-scope-pointer forms both measured negative) */
            volatile s32 *remaining = &D_800F1AF4;
            if (*remaining == 0) break;
        }
    }

done:
    {
        /* FAKE: redundant second handle to D_800F1AF4, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: hypotheses.md [s4-M4] (direct-global form measured 158i/mismatch) */
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
