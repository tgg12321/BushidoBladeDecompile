/* REJECTED PROBE — do NOT re-propose (grind session 2, annotation-fix,
 * 2026-08-08).
 *
 * Variant: the v4 sandbox-0 body with the PRE-EXISTING empty
 * `do { } while (0);` between the s0+0x60 and s0+0x61 byte stores REMOVED
 * (the two stores made adjacent), everything else identical.
 *
 * Measurement: `sandbox func_80021A98 --disable all` = 2 (158/158).
 * The same body WITH the wrap = 0, re-measured immediately before and after
 * this probe in the same session.
 *
 * Conclusion: the empty do-while(0) IS load-bearing. Per the Judge's fix-up
 * notice ("if removal moves the score, add the mandatory inline
 * FAKE-annotation at the construct site") the wrap is retained in the
 * candidate with its annotation. Future sessions: do not delete it as
 * "hygiene" — it costs 2 insns.
 *
 * (Contrast: the `s32 li1 = 1;` constant-holder at the s0+0x7A store was
 * probed the same way, measured NOT load-bearing — sandbox stays 0 without
 * it — and is therefore permanently deleted from the candidate per the
 * notice's preferred branch. Do not reintroduce li1.)
 *
 * The probe body itself is the candidate.c body minus the wrapped empty
 * statement; not duplicated here in full to keep the ledger lean.
 */
