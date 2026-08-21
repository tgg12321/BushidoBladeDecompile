/* REJECTED-as-spelled s2 (2026-08-21) — split model + duplicated-statement-
 * into-arms on the loop1 back edge for tbl (pseudo 79) refs+1:
 *
 *     offset = (*tbl) * 6;          // fall-in copy, moved above loop1 label
 *     loop1: ...
 *     if (i < 0x12) { offset = (*tbl) * 6; goto loop1; }
 *
 * MEASURED: the ref-lift DOES materialize (79: 4 -> 5 refs = pri 2083, the
 * H2 mechanism is real), but byte-neutrality FAILS: build_insns 134,
 * sandbox 30, allocation scrambled (carrier stole s6). jump2's cross_jump
 * did not re-merge the copies: sched1 interleaves the fall-in copy's insns
 * (lw/sll/addu/sll) with independent preamble insns, so the block-0 suffix
 * is not textually identical to the arm's copy and find_cross_jump has no
 * matching tail. The arm-dup +1 is the ONLY ref-lift spelling that survived
 * flow this session; if a future session can force fall-in-copy contiguity
 * (or find a merge-tolerant dup site), the atom becomes usable — but note
 * no depth<=3 solver-sufficient set uses a 79-atom without a dead 86-atom.
 */
