/* REJECTED s1 (2026-08-18): moving `s1 = 0;` to BEFORE the loop_flag[3]
 * comparison (straight-line, unconditional):
 *
 *     if (s1 != s5) goto loop_continue;
 *     s1 = 0;
 *     if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) != loop_flag[3]) goto loop_continue;
 *
 * Measured: sandbox 26 (from 25). Mechanism: sched1 sees the store as
 * independent and hoists it UP into the lhu load-delay slot (build idx 113),
 * while target keeps a nop there (target idx 112) and has the s1=0 in the
 * bne's BRANCH delay slot (target idx 117). The unconditional spelling can
 * never land it in the branch delay slot because the scheduler grabs it
 * first. The winning spelling (see evidence.md [s1-H2]) puts `s1 = 0;`
 * INSIDE the if-arm before the goto — reorg then fills the branch delay
 * slot from the taken thread, exactly matching target (which clobbers the
 * 0 immediately on fall-through via `lui s1`). Do not re-try this form.
 */
