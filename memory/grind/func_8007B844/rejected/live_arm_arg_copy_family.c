/* func_8007B844 — REJECTED FAMILY (s6 forensics, 2026-07-21):
 * "live" arm-use copy spellings — `g_gpu_debug_func(&D_80015F98, p = ot, n);`
 * in the debug arm + unconditional `p = ot;` re-set in the tail.
 *
 * MOTIVE: s6 named the exact mechanism behind the banked sandbox-0
 * conditional-dead-store form (rejected/conditional_dead_store.c):
 * cse.c make_regs_eqv (tools/gcc-2.7.2/cse.c:853-858) keeps a reg-reg
 * copy's DEST as the quantity's canonical register ONLY when the dest's
 * live range extends beyond the current cse basic block:
 *
 *   (uid_cuid[regno_last_uid[new]] > cse_basic_block_end
 *    || uid_cuid[regno_first_uid[new]] < cse_basic_block_start)
 *   && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]
 *
 * Every single-set tail rebind (15 sessions of them) fails the first
 * conjunct by construction, so `ot` stays canonical, uses are rewritten,
 * and the copy is deleted (delete_trivially_dead_insns). An arm-block
 * set of p gives p a first-use cuid BEFORE the tail block start, so the
 *tail copy survives. This family tried to satisfy that condition with a
 * LIVE arm use (assignment-expression as the debug call's 2nd arg)
 * instead of the dead conditional store.
 *
 * MEASURED (all sandbox --disable all, target_insns 38):
 *   Z : live arm arg + Lever-B 3-stmt tail (p copy last)     = 6, build 38
 *   Z2: live arm arg + fused single-stmt store (X's tail)    = 8, build 39
 *   Z3: live arm arg + two-local AND-into-addr               = 8, build 39
 *   Z5: live arm arg + Lever-B with p copy between const/AND = 6, build 38
 *
 * Z/Z5 DO hold the copy topology (emitted `move $2,$16` + `sw $3,0($2)`
 * — store base = the copy, like target) but the residual becomes a
 * 6-diff REGISTER ROTATION: mask lands v1 (target a0), addr lands v0
 * (target v1). Z2/Z3 lose $v0 entirely (extra `move $2,$4` return copy,
 * +1 insn). Root cause, verified in dumps (tmp/grind/func_8007B844/s6/):
 * the live arm use necessarily extends p's live length across the arm,
 * deflating p's global.c allocno priority density below the tail
 * pseudos; target's rotation (p=v0, addr=v1, mask=a0, and-dest=v1)
 * requires p to be allocated FIRST with a short dense tail-only range —
 * which only exists when the arm store is DEAD and flow.c deletes it
 * before RA (the conditional_dead_store form X: cse uses the arm set for
 * canonicalization, then flow removes it byte-neutrally, then global.c
 * gives r74 $v0 via return preference; greg dump: 74 in 2, 82 in 3,
 * 84 in 4 — exactly target).
 *
 * VERDICT: the live-spelling axis around the make_regs_eqv gate is DEAD.
 * The only byte-exact pure-C closer is the dead-conditional-store form,
 * which is exactly the construct no-new-park-categories.md marks as
 * "would TODAY be reviewable under dead-store-fake-exception" — s6 filed
 * a ruling-request. ALSO NOTE: the arm assignment here is itself
 * semantically dead in its STORE effect (p unread before the tail
 * re-set); this family is the same intent spelled as a live arg, so it
 * would face the same carve-out review anyway — it is NOT a cleaner
 * alternative, and it cannot reach the bytes.
 */

u32 *func_8007B844(u32 *ot, s32 n) {          /* Z5 spelling, score 6 */
    u32 *p;
    u32 mask;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, p = ot, n);
    }
    { u32 *v0 = g_gpu_dev_table; ((void(*)(u32*,s32))v0[11])(ot, n); }
    mask = 0xFFFFFF;
    p = ot;
    mask = ((u32)&g_gpu_ot_end) & mask;
    *p = mask;
    return p;
}
