/* s31 rejected form: PERM_LINESWAP over the 5 fn-scope prologue init statements
 * (tbl_125c/idx_1494/idx_1495/D_800F19BC/D_800F19C0) - permuter enumerated
 * 5!=120 orderings, base_score=60 (h5 basin preserved), 0 novel finds beyond
 * the 14 pre-existing outputs (s22 + s23 legacy).
 *
 * Mechanism attempted: prologue statement C-order sets expand-time LUID for
 * fn-scope pseudo initializations (tbl_125c, idx_1494, D_800F19C0); a distinct
 * ordering could shift flow.c reg_n_sets birth luid for the s-reg web that
 * feeds block=3's residual pair-swap at clock=13 without disturbing the
 * multi-set t0 chain that h5 basin depends on.
 *
 * Empirical result: NONE of the 120 valid orderings surfaced a distinct-score
 * output; the fn-scope prologue init statements are qty-invariant with
 * respect to the block=3 sched2 pair-swap outcome. Consistent with s3 V12
 * (block-local decl reversal INERT) — extends the finding to prologue-init
 * scope: C-statement-order of prologue globals-init does NOT propagate to
 * block=3 qty birth-order or LUID tiebreaks.
 *
 * See tmp/grind/cpu_side_move_dir_4/s31/ for campaign.log + campaign_meta.json.
 */
