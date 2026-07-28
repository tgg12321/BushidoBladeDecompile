/* REJECTED s2: flipping branch sense so else-arm falls through and then-arm
 * gotos ("if (cond != K) { else-body; goto complete_store; } then-body; break")
 * with per-case func_8005C650 calls scored 26 (worse than baseline 15). Reason:
 * duplicating the func_8005C650 call across the two arms of each case creates 4
 * jal sites that jump2 cross-jumps back inconsistently; the new merges disturb
 * delay-slot fills and hoist the else-arm's tail. Single shared call site
 * (H1d shape) is required. */
