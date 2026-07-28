/* REJECTED s3: score 26 (vs floor 5). Staging both minuends through the
 * zero-arm's existing `neg` local (decl hoisted to function top):
 *   neg = arg1[0]; dx = neg - arg0[0]; ... neg = arg1[1]; dy = neg - arg0[1];
 *   zero arm keeps neg = -arg3.
 * Rationale: existing-var carrier with empty prefs (no call-arg $a0 pref like
 * disc). Measured: the merged neg allocno lands $a1, not $v1 — lw $a1/subu $a1
 * at both staging sites and negu $a1 in the zero arm (+zero-arm div cluster
 * reflow). dy DOES stay $v0 (the pref-kill works), but the carrier itself
 * mis-colors: the arg1-ptr {$a1} expand-merge leak reaches neg despite the
 * dx-segment overlap (behavior differs from the y1-relay, which scanned to
 * $v1 with identical staging segments — the zero-arm negu segment/decl
 * position changes its allocno order/conflicts). Do not re-propose without a
 * FINDREG dump explaining the $a1 landing. */
