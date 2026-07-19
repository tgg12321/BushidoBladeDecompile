/* KILLED s2 (H-s4): rewrote loop as `do { ... } while (count != -1);`
   sandbox 8 -> 17 AND insns 81 -> 83. do-while emits NOTE_INSN_LOOP_BEG which
   adds an extra loop-preservation cycle; not a save-order lever, just adds
   instructions. Basic-block reshaping doesn't affect prologue save ordering
   here because the save-restore_insns pass runs BEFORE the loop notes bias
   scheduling. */
