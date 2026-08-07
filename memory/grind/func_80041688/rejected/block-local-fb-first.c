/* KILLED s2: block-local {s32 fb, fr, fg;} declared in FALSE arm with fb assigned/named FIRST.
 * Hypothesis: New per-branch locals with fb allocated as pseudo BEFORE fr/fg might
 * bias sched1 tie-break (LUID / DECL_ORDER) toward emitting b's lbu first.
 * Result: score stayed at 2, emission still [r,g,b]. Confirms s1 finding at greater
 * strength: sched1's INSN_PRIORITY (chain length 4 for r/g vs 2 for b) is a HARD
 * ordering; LUID / decl-order tie-breaks only apply among equal-priority insns.
 * Chain-length dominates regardless of pseudo/decl ordering. */
s32 fb, fr, fg;
fb = *((u8 *)player + 0x1A);
fr = *((u8 *)player + 0x18);
fg = *((u8 *)player + 0x19);
gnd_load_tex(fb | ((fr << 16) | (fg << 8)));
