/* _exeque -- s5 rejected form: do-while(0) wrap around the final
 * "clear D_8009BE7C, invoke D_8009BE80 callback" block's two statements.
 *
 * Measured (session 5, manual A/B on the s4 floor-2 chassis, sandbox
 * --disable all): sandbox score STAYED 2/187, build_insns unchanged (186).
 * No effect at all -- unlike the H9/H10 triple-store wraps (which each
 * dropped the floor), wrapping this two-statement block in do-while(0)
 * does not perturb the scheduler's decision to fill the jalr delay slot
 * with the `*p = 0;` store. Confirms the residual is NOT a compound-
 * statement-boundary/RTL-boundary issue at this site the way H9/H10's
 * residual was -- it is specifically the jalr-vs-preceding-store
 * adjacency reorg.c's fill_simple_delay_slots resolves via ordinary
 * resource-conflict analysis, immune to this lever.
 *
 * Not applied to src/display.c (reverted same session, no improvement to
 * keep).
 */
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        s32 *p = &D_8009BE7C;
        if (*p != 0 && D_8009BE80 != 0) {
            do {
                *p = 0;
                ((s32 (*)(void))D_8009BE80)();
            } while (0);
        }
    }
