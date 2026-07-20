/* s2 variant G — REJECTED, score 7.
 * val second sets = { arm-2: val = (s32)(D_800F1164 + 3), arm-1: val = 0x10016 }
 * (goal: keep all three post-call repack loads single-set/boosted).
 * cse killed it: D+3 is DERIVABLE from pseudo 91 (D+2 pointer, live in arm 2)
 * -> cse rewired the store to the derived temp (addiu a0,v1,1), val's arm-2
 * set went dead -> flow deleted it -> reg_n_sets(val) collapsed to 1 -> val
 * became arm-1-local again -> probe-1 const-v1-leak regression.
 * RTL evidence: sched2 dump insn 211 stores plain (reg:SI 4 a0), no reg/v set
 * of val anywhere in arm 2.
 * Kill lesson (general law for this function): a staged second set survives
 * flow ONLY if its value is NOT cse-derivable from a live pseudo/const —
 * memory loads and fresh constants survive; symbol arithmetic and values
 * reachable from live temps are substituted away.
 */
