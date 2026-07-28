/* REJECTED s2: score 7 (vs floor 6). disc = arg2*arg2 + dy2*arg3 —
 * mult operand order is NOT canonicalized away; emits mult $s0,$s6 where
 * target has mult $s6,$s0. arg3 * dy2 is the correct spelling. */
