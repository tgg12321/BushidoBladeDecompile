/* KILLED s3 — H6 scalar-dummy variants.
 *
 * The named-local-fake-exception carve-out permits dead SCALAR locals biasing
 * RA (arrays remain forbidden by dead-vars-local-array). Hoped: `s32 dummy;`
 * scalar replaces `s32 dummy[2]` array and satisfies the sanctioned family.
 *
 * Measured — 3 spellings all sandbox=12 (SAME AS FULL REMOVAL, no frame slot):
 *  (A) `s32 dummy;` + `(void) dummy;`                      -> score 12
 *  (B) `s32 dummy = 0;` + `(void) dummy;`                  -> score 12
 *  (C) `s32 dummy; ...; dummy = temp_v1; ...(void) dummy;` -> score 12
 *
 * MECHANISM: GCC's frame-size logic reserves stack storage for AGGREGATES
 * (arrays / structs) at declaration time unconditionally — that's what
 * dummy[2] does. SCALARS are register-allocation candidates; a fully-dead or
 * DCE'd scalar leaves no frame footprint (get_frame_size doesn't reserve
 * anything for it). Confirmed here across bare/initialized/live-init forms.
 *
 * CONSEQUENCE: `s32 dummy;` cannot substitute for `s32 dummy[2]` in the
 * dead-pad role. The named-local-fake-exception family (constant-holder
 * scalars, dead scalar RA-biasing) delivers a different codegen effect
 * (RA priority tie-break, not frame reservation) — inapplicable to a
 * frame-slack-forced target. Only the oversized-locals array carve-out
 * addresses this shape (owner-granted 2026-07-13).
 */
