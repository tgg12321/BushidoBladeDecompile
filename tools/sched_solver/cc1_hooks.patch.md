# Instrumented-cc1 hook additions for sched_solver (2026-08-05)

`tools/gcc-2.7.2/` is gitignored (local toolchain build), so the env-gated
diagnostic hooks that `extract.py` depends on are recorded here for
reproduction — same practice as `tools/ra_solver/cc1_hooks.patch.md`.
All are print-only `fprintf` under `getenv ("BB2_SCHED_DEBUG")`. Rebuild with
`cd tools/gcc-2.7.2 && TMPDIR=/dev/shm make cc1` (produces `./cc1`;
`build/cc1`, the compiler the build actually uses, is untouched).

**Always parity-check after a rebuild.** `extract.py` does it per TU
automatically and records the result in the model's `parity` field; a model
built from a non-parity TU describes a different compiler than the one whose
bytes we are matching.

Hooks 1 and 2 (`SCHEDDBG block=` / `SCHEDDBG PICK`) predate this campaign;
3–6 were added by it.

## 1. sched.c — per-block header + priorities + initial ready list (pre-existing)

In `schedule_block`, right after the `if (file)` dump of the initial ready
list: `SCHEDDBG block=%d n_insns=%d n_ready=%d`, then one
`SCHEDDBG   insn=%d kind=%s pri=%d refcnt=%d` per insn with
`INSN_PRIORITY > 0`, then one `SCHEDDBG   pos=%d insn=%d pri=%d luid=%d`
per initial ready-list slot.

## 2. sched.c — per-pick trace (pre-existing)

In `schedule_block`'s main loop, after `schedule_select` and before
`ready += 1`: `SCHEDDBG PICK clock=%d picked=%d (pri=%d luid=%d)`,
`SCHEDDBG   ready was: [ uid(p=,l=) ... ]`, `SCHEDDBG   last_scheduled=%d`.

## 3. sched.c — function + pass banner (NEW)

At the top of `schedule_insns`, after the `rtx insn;` declaration and before
the `n_basic_blocks == 0` early return. Without this the stream cannot be
segmented, and sched1 (pre-RA) and sched2 (post-reload) are
indistinguishable — both passes run at `-O2` on this build.

    {
      extern char *getenv ();
      extern char *current_function_name;
      if (getenv ("BB2_SCHED_DEBUG"))
        fprintf (stderr, "SCHEDDBG FUNC func=%s pass=%d nbb=%d\n",
                 current_function_name ? current_function_name : "?",
                 reload_completed ? 2 : 1, n_basic_blocks);
    }

## 4. sched.c — dependence graph + per-insn statics + unit census (NEW)

Inside the existing `BB2_SCHED_DEBUG` block in `schedule_block`, immediately
before the `"SCHEDDBG initial ready list"` line. This is the model's whole
input side: `LOG_LINKS` with `REG_NOTE_KIND` (0 = true data dependence,
non-zero = anti/output, which MIPS `ADJUST_COST` collapses to cost 1),
`INSN_LUID`, `insn_unit`, `INSN_COST`, `INSN_CODE`, `INSN_PRIORITY`,
`INSN_REF_COUNT`, `SCHED_GROUP_P`, and `blockage_range`'s min/max.

    {
      int ui;
      fprintf (stderr, "SCHEDDBG units");
      for (ui = 0; ui < FUNCTION_UNITS_SIZE; ui++)
        fprintf (stderr, " n%d=%d", ui, unit_n_insns[ui]);
      fprintf (stderr, "\n");
    }
    fprintf (stderr, "SCHEDDBG deps:\n");
    for (sched_insn = head; sched_insn != next_tail;
         sched_insn = NEXT_INSN (sched_insn))
      if (GET_RTX_CLASS (GET_CODE (sched_insn)) == 'i')
        {
          rtx dlink;
          fprintf (stderr,
                   "SCHEDDBG   node insn=%d luid=%d unit=%d icost=%d code=%d pri=%d ref=%d grp=%d bmin=%d bmax=%d\n",
                   INSN_UID (sched_insn), INSN_LUID (sched_insn),
                   insn_unit (sched_insn), INSN_COST (sched_insn),
                   INSN_CODE (sched_insn), INSN_PRIORITY (sched_insn),
                   INSN_REF_COUNT (sched_insn),
                   SCHED_GROUP_P (sched_insn) ? 1 : 0,
                   (insn_unit (sched_insn) >= 0
                    && function_units[insn_unit (sched_insn)].blockage_range_function)
                   ? (int) MIN_BLOCKAGE_COST (blockage_range (insn_unit (sched_insn), sched_insn))
                   : -1,
                   (insn_unit (sched_insn) >= 0
                    && function_units[insn_unit (sched_insn)].blockage_range_function)
                   ? (int) MAX_BLOCKAGE_COST (blockage_range (insn_unit (sched_insn), sched_insn))
                   : -1);
          for (dlink = LOG_LINKS (sched_insn); dlink; dlink = XEXP (dlink, 1))
            fprintf (stderr, "SCHEDDBG     dep insn=%d pred=%d kind=%d\n",
                     INSN_UID (sched_insn), INSN_UID (XEXP (dlink, 0)),
                     (int) REG_NOTE_KIND (dlink));
        }

`insn_unit` and `blockage_range` memoize into `INSN_UNIT` / `INSN_BLOCKAGE`,
which the scheduler would populate anyway with identical values; the parity
check confirms this is inert.

## 5. sched.c — schedule_select diagnostics (NEW)

In `schedule_select`, after `queue_insn (insn, cost);` in the blocking loop:

    {
      extern char *getenv ();
      if (getenv ("BB2_SCHED_DEBUG"))
        fprintf (stderr, "SCHEDDBG SELBLOCK clock=%d insn=%d unit=%d cost=%d\n",
                 clock, INSN_UID (insn), insn_unit (insn), cost);
    }

and at the top of the `if (best_insn != 0)` body:

    {
      extern char *getenv ();
      if (getenv ("BB2_SCHED_DEBUG"))
        fprintf (stderr, "SCHEDDBG SELBEST clock=%d insn=%d pos=%d\n",
                 clock, INSN_UID (ready[best_insn]), best_insn);
    }

## 6. sched.c — unit blockage measurement (NEW)

In `actual_hazard_this_instance`, immediately before the closing
`if (tick - clock > cost) cost = tick - clock;`. This is what let the
generated `*_unit_blockage` functions be fitted without reading
`insn-attrtab.c`: the compiler's own blockage value is recoverable as
`adj_tick + maxb - raw_tick`.

    {
      extern char *getenv ();
      if (getenv ("BB2_SCHED_DEBUG"))
        fprintf (stderr,
                 "SCHEDDBG BLOCKAGE unit=%d clock=%d raw_tick=%d adj_tick=%d maxb=%d exec=%d last=%d\n",
                 unit, clock, unit_tick[instance], tick,
                 function_units[unit].max_blockage, INSN_UID (insn),
                 unit_last_insn[instance] ? INSN_UID (unit_last_insn[instance]) : -1);
    }

## 7. sched.c — adjust_priority observation (NEW)

In `adjust_priority`, at the head of `case 0:` (before the
`birthing_insn_p` test that may raise the priority). `birthing_insn_p`
reads `bb_live_regs` / `reg_n_sets`, which no dump carries, so the flag is
harvested rather than derived — the same treatment `ra_solver` gives
`seed_used`. It fires only in pass 1.

    {
      extern char *getenv ();
      if (getenv ("BB2_SCHED_DEBUG"))
        fprintf (stderr, "SCHEDDBG ADJPRI insn=%d deaths=%d birth=%d maxpri=%d pri=%d\n",
                 INSN_UID (prev), n_deaths,
                 birthing_insn_p (PATTERN (prev)) ? 1 : 0,
                 max_priority, INSN_PRIORITY (prev));
    }

## Parity status (2026-08-05)

Instrumented `cc1` vs `build/cc1` over all 29 `src/*.c` TUs with no BB2_*
env set: **28 byte-identical, 1 differs — `src/ings.c`.**

That divergence **pre-dates these hooks** (verified by rebuilding `cc1` from
the pre-hook `sched.c`: byte-identical to the hooked build on every TU, and
still different from `build/cc1` on `ings`). `build/cc1` was built 2026-05-18
and the source tree has since had `jump.c`, `global.c`, `local-alloc.c`,
`reorg.c` and the regenerated `insn-*.c` touched. It does not affect the
build (the Makefile uses `build/cc1`), but any `ings.c` diagnostic taken from
the instrumented `cc1` describes a different compiler. `extract.py` flags it.
Worth resolving separately — it also silently affects `ra_solver` extractions
on that TU.
