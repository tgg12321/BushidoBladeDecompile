# r11/banked — evidence previously held only in tmp/ (banked 2026-09-29, author-side audit)

- bankrun.log: tmp/f187/bankrun.sh (fast3.sh, real per-file recipe) on the bodies here:
  rv3I_1000000_ff_loop_lz8 3 lines (frame 0x78); rv3_rv_wd_end_lz5 / rv3_rv_both_lz5 / rv3_nfdw_lz5 0 (the Q32
  twins of the set-aside closers); pc_noislands 425 lines, 491 insns (islands removed); pc_joined 180 lines,
  661 insns (run with GENMODE=--joined: gen.py joins each macro into one __asm__; without it, the same
  template expands to header-exact statements and scores 0, also logged); rv3_c7 0 (the v3 reviewer's base).
- alloc_c7_landing.txt: tmp/func_800187F4/dump_c7/alloc.txt, the instrumented-cc1 ALLOCDBG output for the
  landing chassis (every allocno with ord / nrefs / livelen / pri), the source of §3 idx's seating order.
- perm_*_campaign_tail.log: the last ~4 KB of each permuter campaign log (iteration counts, final scores);
  full logs stay in tmp/f187/perm_*/ (0.4-1.4 MB each).
