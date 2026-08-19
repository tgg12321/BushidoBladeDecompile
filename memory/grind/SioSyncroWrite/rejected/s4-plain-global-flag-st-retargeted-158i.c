/* REJECTED (s4) — 158 insns, 57 differing lines.
 * Attempt to remove the last volatile-qualifier-adding construct
 * (`volatile s32 *flag = &D_800F1AEC;` over a non-volatile extern) by making
 * `flag` a plain `s32 *` and re-basing the member pointer onto the already-
 * volatile, already-allowlisted D_800F1AF0:
 *     s32 *flag = &D_800F1AEC;
 *     volatile s32 *st = &D_800F1AF0;   with st[0]/st[1]/st[2]
 * WHY IT IS DEAD: target holds ONE base register ($s3 = &D_800F1AEC, indexed
 * at 0x4/0x8/0xC), so re-basing changes every member offset and the whole
 * block diverges. The only in-scope alternative, `(volatile s32 *)flag`, DOES
 * match (159i/0) but is an explicit volatile CAST — expressly forbidden by the
 * volatile catalog — so it is strictly worse than the pointer-declaration form
 * kept in candidate.c. The clean fix is the file-scope
 * `extern volatile s32 D_800F1AEC;` (measured score-neutral, [s4-M6]), which
 * needs a volatile_extern_allowlist.txt entry that is out of grind scope.    */
    s32 *flag = &D_800F1AEC;
    volatile s32 *st = &D_800F1AF0;
