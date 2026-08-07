/* REJECTED (s2, 2026-07-17): the ENTIRE do-while(0) wrap family is inert on
 * this cross-jump wall — measured, closes frontier F3.
 *
 * All six spellings compiled to the MERGED shape (361 insns, identical to the
 * plain honest-0xD control h1; case-9/11 block deleted by find_cross_jump):
 *   w1: sel = 0xD; do { } while (0); goto sel_dispatch;      (empty, between)
 *   w2: do { sel = 0xD; } while (0); goto sel_dispatch;
 *   w3: do { sel = 0xD; goto sel_dispatch; } while (0);
 *   w4: ==3 arm: if (v0 != 10) { do { sel = 0xD; } while (0); } else ...
 *   w5: ==3 arm: do { if (v0 != 10) sel = 0xD; else sel = 0xF; } while (0);
 *   w6: wraps on BOTH arms (w2 + w4)
 * Mechanism (predicted by s1 source reading, now measured): loop notes are
 * skipped by every comparison surface in find_cross_jump/mark_jump_label, and
 * the wraps changed neither emission order nor sched2 adjacency here.
 * Transfers cluster-wide: wrap family is DEAD for cross-jump walls.
 *
 * Also killed:
 *   v1: sel = 0xD; sel = sel;   (self-assign deleted as no-op pre-jump2: 361)
 *   u2: { union { s32 a; } u; u.a = 0; }  (field assign, no constructor ->
 *       no CLOBBER emitted; dead store deleted: 361, merged)
 *
 * Probe harness: tmp/grind/motion_SetMotion/s2/probe_s2.py (judge = label-
 * normalized full-function cc1 asm diff vs committed baseline m0). */
