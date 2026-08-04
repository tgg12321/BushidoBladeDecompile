# Instrumented-cc1 hook additions for ra_solver (2026-08-04)

`tools/gcc-2.7.2/` is gitignored (local toolchain build), so the env-gated
diagnostic hooks that `extract.py` depends on are recorded here for
reproduction. All are print-only fprintf under env guards; rebuild with
`cd tools/gcc-2.7.2 && TMPDIR=/dev/shm make cc1` (produces `./cc1`;
`build/cc1` is untouched — verify with tmp/parity_check.sh pattern).

## 1. global.c — ALLOCDBG per-allocno line (existing hook, + func tag)

In `global_alloc`, the BB2_ALLOC_DEBUG block after the allocation loop:
add `extern char *current_function_name;` and change the fprintf to

    "ALLOCDBG func=%s ord=%d pseudo=%d hardreg=%d nrefs=%d livelen=%d pri=%d\n",
    current_function_name ? current_function_name : "?", ...

## 2. global.c — seed dump (NEW)

Immediately after the `regs_used_so_far` seeding loops (call-used +
regs_ever_live + local-alloc renumberings; right before "Establish
mappings from register number to allocation number"):

    {
      extern char *getenv ();
      extern char *current_function_name;
      if (getenv ("BB2_ALLOC_DEBUG"))
        {
          int dk;
          fprintf (stderr, "ALLOCDBG func=%s seed_used=",
                   current_function_name ? current_function_name : "?");
          for (dk = 0; dk < FIRST_PSEUDO_REGISTER; dk++)
            if (TEST_HARD_REG_BIT (regs_used_so_far, dk))
              fprintf (stderr, "%d,", dk);
          fprintf (stderr, "\n");
        }
    }

## 3. global.c — FINDREGDBG (existing hook, + func tag)

In `find_reg`'s BB2_FINDREG_DEBUG block: add the extern and change the
first fprintf to

    "FINDREGDBG func=%s pseudo=%d alt=%d acc=%d retry=%d\n",
    current_function_name ? current_function_name : "?", ...

(The block already prints conflicts / someone_prefers / used_so_far /
pass0_used / own_copy_prefs / own_full_prefs — unchanged.)
