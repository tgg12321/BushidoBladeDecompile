

## Session 15 (2026-09-01, forensics) -- the Ruling-A named probe executed; cse.c is the reason the amended once-written family cannot close this function

- **Chassis re-measured this session.** SB body (`candidate.c`) applied over the
  `INCLUDE_ASM` line in `src/text1b.c`: `sandbox func_800645B0 --disable all` =
  **score 1, target_insns 78, build_insns 78, rules_dropped 0**.  The ledger
  floor of 1 is current on today's tree.

- **The owner-directed probe (2026-09-01 Ruling A, reopen note).**  "Build and
  measure the two-distinct-once-written-locals spelling -- each fresh local
  written exactly once, textually and semantically distinct from every banned
  entry."  Executed in both directions:
  - **P1** `wid = i + j; idx = wid;` at the inner-loop top, `wid2 = idx2 + idx;`
    for the *3 sum, the three word stores reading `wid2` = **3 / 78**, 78 build
    insns.
  - **P2** (mirror) `idx = i + j;` at the loop top, `wid = idx;` +
    `wid2 = idx2 + wid;` inside the if-arm = **3 / 78**, 78 build insns.
  Both residuals are the identical three objdump positions and are the WD
  fresh-destination residual verbatim: 11 (ours `addiu v1,zero,1` / target
  `addu s0,s3,a0`), 12 (the reverse), 65 (the inner loop's back-edge delay
  slot -- the target steals the `addu`, we steal the `li`).  The operand-order
  half of the 1-vs-3 lock is won by the fresh destination; the loop-head half
  is lost.

- **NEW MECHANISM, dump-proven -- `cse.c` deletes the copy before any pass that
  counts sets.**  This is the general finding of the session and it is what
  makes the amended family unusable HERE.  A once-written named intermediate
  at the inner-loop top can only hold a COPY of the carrier (the only real
  value at that point is `i + j` itself), and GCC 2.7.2's first post-RTL pass
  removes it:
  - P1: `tmp/grind/func_800645B0/s15/p1.rtl.txt` carries
    `(insn 41 38 44 (set (reg/v:SI 74) (reg/v:SI 79)))` -- the `idx = wid` copy.
    `p1.cse.txt` has **0** occurrences of `insn 41` and **0** references to
    `reg/v:SI 74`; `cse_insn` substituted reg79 into both `1 << idx` and
    `idx << 1` and deleted the copy insn.
  - P2: `p2.rtl.txt` carries `(insn 60 57 63 (set (reg/v:SI 79) (reg/v:SI 74)))`
    -- the `wid = idx` copy.  `p2.cse.txt` has **0** occurrences of `insn 60`
    and **0** references to `reg/v:SI 79`, and insn 63 reads
    `(plus:SI (reg/v:SI 75) (reg/v:SI 74))` directly.
  Consequence: `loop.c`'s `count_loop_regs_set` and `sched.c`'s
  `birthing_insn_p` (H61/H62) see reg_n_sets == 1 for the surviving carrier in
  BOTH probes, the max-priority lift is applied to the loop-top `addu`, and the
  loop-head placement is lost.  **Raising reg_n_sets above 1 in this function
  requires a second, differently-valued write to the same carrier -- a
  multi-WRITE carrier -- which is precisely this function's standing banned
  construct and is explicitly outside the amended named-intermediate family
  (`.claude/rules/no-new-park-categories.md:227-228`, "Multi-WRITE carriers
  remain NOT this entry").**  The reopen's own rule applies: the ban wins and
  the probe FAILs.

- **Gate (b) re-censused against the AMENDED class** (per the reopen note), on
  the uncapped index rebuilt 2026-09-01 at pin `aa535002`.  Still FAILS.  The
  PSX reuse evidence in the index is the frozen variable-reuse family
  (`// fake reuse of i?` at `src/boss/mar/cutscene.c:172`,
  `src/st/cen/cutscene.c:211`, `src/st/lib/cutscene.c:153`,
  `src/st/no3/cutscene.c:360`, `src/st/top/cutscene.c:143` -- all BORROW an
  existing loop index, which `staged-value-reused-variable.md` bound 2 bars us
  from inventing), and `src/weapon/w_037.c:300` is the once-written shape just
  measured at 3 / 78.  The `new_var_temp` class
  (`docs/reference/sotn-construct-index.md:1425`) carries declaration lines
  only and cannot evidence a write count.  No PSX-tagged precedent for an
  invented multi-write carrier exists in the index.

- **Artifacts.** `tmp/grind/func_800645B0/s15/p1.c`, `p2.c`, `apply.py`,
  `p1.rtl.txt`, `p1.cse.txt`, `p2.rtl.txt`, `p2.cse.txt`, `p2.combine.txt`;
  full pass dumps in `tmp/grind/func_800645B0/dumps/` (last written for P1).
  Banked forms: `rejected/two-once-written-locals-copy-deleted-by-cse.c`,
  `rejected/two-once-written-locals-mirror-copy-deleted-by-cse.c`.
