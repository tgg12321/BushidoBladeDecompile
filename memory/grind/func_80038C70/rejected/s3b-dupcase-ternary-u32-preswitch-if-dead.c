/* REJECTED — s3b structural probes (2026-07-18), all four KILLED by
 * measurement (tmp/grind/motion_SetMotion/s3/s3b_probe.py, label-normalized
 * cc1 asm diff vs m0 committed baseline 363 insns).
 *
 * These are the residual structural spelling classes NOT covered by
 * r1-r9 / m-series; measuring them dead seals the closure theorem against
 * four more escape hatches:
 *
 * r10 — duplicate case blocks (attacks "one shared C block" assumption):
 *     case 9:  sel = 0xD; goto sel_dispatch;
 *     case 11: sel = 0xD; goto sel_dispatch;
 *   RESULT: 361 insns, 86 diff lines — IDENTICAL merged signature to the
 *   honest-0xD control h0. cross-jump collapses the 9/11 duplicates first,
 *   then the survivor merges with the ==3 arm exactly as before.
 *
 * r11 — cond-expr spelling of the ==3 arm + honest 0xD:
 *     sel = (v0 != 10) ? 0xD : 0xF; goto sel_dispatch;
 *   RESULT: 361 insns, 86 diff lines — identical merged signature. The
 *   ternary canonicalizes to the same branch shape as the if/else (r1/r3/m8
 *   sibling; the cond-expr expansion surface adds nothing).
 *
 * r12 — u32 sel + honest 0xD (unsigned type class; r2 was s16):
 *   RESULT: 362 insns, 149 diff lines. Doubly dead: unsigned makes the
 *   `if (sel >= 0)` dispatch guard vacuous, restructuring the dispatch path
 *   (new [j; move $4,$16] tail), on top of the merge.
 *
 * r13 — cases 9/11 extracted to a pre-switch if (removes the jtbl
 *   CODE_LABEL — the label-bonus source — from the 13 block):
 *     if (v0 == 9 || v0 == 11) { sel = 0xD; goto sel_dispatch; }
 *   RESULT: 369 insns, 126 diff lines — two extra beq/li compare pairs
 *   before the range check and rewritten jtbl entries (9/11 -> default).
 *   The label bonus cannot be dodged without paying real bytes; same
 *   byte-count closure as F1-slack and r9-hoist.
 *
 * Conclusion: consistent with the sealed closure theorem — every honest
 * set13 spelling canonicalizes to the identical merged 361-insn shape, and
 * every merge-avoiding spelling pays bytes. No structural lever remains.
 */
