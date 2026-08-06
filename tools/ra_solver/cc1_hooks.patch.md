# Instrumented-cc1 hook additions for ra_solver (2026-08-04)

`tools/gcc-2.7.2/` is gitignored (local toolchain build), so the env-gated
diagnostic hooks that `extract.py` depends on are recorded here for
reproduction.

> **CRLF footgun (2026-08-06).** Because `tools/gcc-2.7.2/` is **gitignored**,
> the `.gitattributes` line `tools/gcc-2.7.2/** text eol=lf` never applies —
> git does not normalize files it does not track. Several upstream GCC sources
> ship with CRLF (`reload1.c` had 7179 CR lines before any hook was added), and
> the Windows Edit/Write tool preserves/reintroduces them, which trips
> `tools/hooks/tooling_error_guard.py`. **After editing any file under
> `tools/gcc-2.7.2/`, run `python3 tools/normalize_lf.py <file>` before
> rebuilding**, then do the usual parity check. All are print-only fprintf under env guards; rebuild with
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

## 4. local-alloc.c — QTYDBG (existing hook, Phase 5)

See the ra_solver README's Phase-5 row; unchanged by the 2026-08-05 additions.

## 5. function.c + reload1.c — FRAMEDBG frame-slot census (NEW 2026-08-05)

Answers "which allocation produced this function's `vars=` bytes, and which of
them does no instruction ever touch" — the direct instrument for the
phantom-frame-slot class. Print-only, env-gated, parity-verified on text1b +
main after rebuild.

### function.c

Next to `int frame_offset;`:

    /* BB2 instrumentation: tag identifying which caller requested the current
       assign_stack_local.  Print-only; cleared by the hook.  */
    char *bb2_frame_ctx = 0;

At the end of `assign_stack_local`, immediately before its `return x;`:

    {
      extern char *getenv ();
      extern char *current_function_name;
      if (getenv ("BB2_FRAME_DEBUG"))
        fprintf (stderr,
                 "FRAMEDBG func=%s ctx=%s mode=%d size=%d align=%d alignment=%d frame_offset=%d\n",
                 current_function_name ? current_function_name : "?",
                 bb2_frame_ctx ? bb2_frame_ctx : "?",
                 (int) mode, size, align, alignment, frame_offset);
      bb2_frame_ctx = 0;
    }

Caller tags — set `bb2_frame_ctx` on the line before each `assign_stack_local`
call: `"stack_temp"` (in `assign_stack_temp`'s "make a new temporary" branch),
`"put_reg_into_stack"`, `"assign_parms_blk"`, `"assign_parms_reg"`.

### reload1.c

`"round_frame"` before the bare `assign_stack_local (BLKmode, 0, 0);` in
`reload`, and in `alter_reg` — where `i` is the pseudo number:

    { extern char *bb2_frame_ctx; static char bb2b[64];
      sprintf (bb2b, "spill_new_p%d", i); bb2_frame_ctx = bb2b; }

before the `from_reg == -1` call, and the same with `"spill_grow_p%d"` /
`bb2b2` before the "allocate a bigger slot" call.

### Reading it

`tmp/csz/gn_frame.sh <stem> <func>` dumps a function's allocations + its
`.frame` line; `tmp/csz/gn_census.py <framedbg.log> <asm.s> --only-phantom`
pairs every allocation against the emitted sp traffic tree-wide and reports the
byte ranges nothing touches (it treats `addiu $rX,$sp,K` as address-taken, so
locals passed by pointer are not false-flagged).

Two facts worth keeping: a spill slot from `alter_reg` is **always 8 bytes**
(`align == -1` rounds to `BIGGEST_ALIGNMENT` = 64 bits on MIPS), and an
orphaned pseudo — `reg_n_refs > 0` but no surviving RTL reference after combine —
gets one of those slots while emitting **no instruction at all**. That is the
phantom-slot class, and `tmp/csz/gn_orph2.py` names its members from a `.greg`
dump (empty conflict list + absent from the dispositions).

## 6. global.c + reload1.c — BB2_RELOAD_DEBUG (NEW 2026-08-06, Campaign 7)

The reload/retry instrument. Everything is print-only under
`getenv ("BB2_RELOAD_DEBUG")`; parity re-verified on 5 TUs after the rebuild,
and the env var itself is output-inert (`tmp/parity_check_multi.sh`).

### Shared helper (added to the top of BOTH files, after the last `#include`)

    /* BB2 instrumentation: print-only hard-reg-set dump of the 32 GPRs.  */
    #define BB2_DUMP_HRS(LABEL, SET)					\
      do {									\
        int bb2_dk_;							\
        fprintf (stderr, "%s", (LABEL));					\
        for (bb2_dk_ = 0; bb2_dk_ < 32; bb2_dk_++)				\
          if (TEST_HARD_REG_BIT ((SET), bb2_dk_))				\
    	fprintf (stderr, " %d", bb2_dk_);				\
        fprintf (stderr, "\n");						\
      } while (0)

reload1.c additionally gets, next to it, the spill-loop iteration counter

    int bb2_reload_pass = 0;

zeroed in `reload` (next to `bzero (cannot_omit_stores, max_regno);`) and
incremented right after the `something_changed = 0;` at the top of the
`while (something_changed)` body.

### global.c

* **`retry_global_alloc`** — after `int allocno = reg_allocno[regno];`, a block
  printing `RETRYDBG func= pseudo= allocno= nrefs= livelen= calls=` followed by
  `BB2_DUMP_HRS` of `forbidden_regs`, `hard_reg_conflicts[allocno]` and
  `regs_used_so_far`; and at the very end of the function a
  `RETRYDBG  result func= pseudo= got=` line reading `reg_renumber[regno]`.
* **`find_reg`** — the existing `BB2_FINDREG_DEBUG` block's gate becomes

      if ((dbg && atoi (dbg) == allocno_reg[allocno])
          || (retrying && getenv ("BB2_RELOAD_DEBUG")))

  so every retrying call dumps its sets for every pseudo. Three lines are
  appended inside it: `BB2_DUMP_HRS ("FINDREGDBG  pass1_used:", used1)`,
  `BB2_DUMP_HRS ("FINDREGDBG  used2_noconflict:", used2)`, and
  `FINDREGDBG  class=%d mode=%d size=%d` from `class`, `mode`,
  `allocno_size[allocno]`.
* **`find_reg`** — immediately before the `if (best_reg >= 0)` under the
  "Did we find a register?" comment, a
  `FINDREGDBG  best func= pseudo= alt= acc= best_reg=` line, gated on
  `retrying && getenv ("BB2_RELOAD_DEBUG")`.

### reload1.c

* **`order_regs_for_reload`** — at the end, `RELOADDBG order func= prr=` (the
  `potential_reload_regs` entries below 32, comma-separated), `RELOADDBG  uses=`
  (`regno:uses` pairs from the sorted `hard_reg_n_uses`), and
  `BB2_DUMP_HRS ("RELOADDBG  bad_spill_regs:", bad_spill_regs)`.
  This line is emitted once per function and is the stream's segmenter.
* **`new_spill_reg`** — before `potential_reload_regs[i] = -1;`:
  `RELOADDBG new_spill_reg func= pass= idx= regno= class= n_spills= need= nongroup=`.
* **`spill_hard_reg`** — after `SET_HARD_REG_BIT (forbidden_regs, regno);`:
  `RELOADDBG spill_hard_reg func= pass= regno= class= cant_eliminate= global=`
  plus `BB2_DUMP_HRS ("RELOADDBG  forbidden_after:", forbidden_regs)`.
* **`spill_hard_reg`** — immediately before `reg_renumber[i] = -1;` in the
  eviction loop (so `had` still reads the old assignment):
  `RELOADDBG kickout func= pass= spillreg= pseudo= had= nrefs= bb=` from
  `reg_renumber[i]`, `reg_n_refs[i]`, `reg_basic_block[i]`.
* **`reload`** — just before the "If all needs are met, we win." comment:
  `RELOADDBG needs func= pass=` followed by, for each class with a non-zero
  need, ` NAME(idx):n=,g=,ng=` from `max_needs` / `max_groups` /
  `max_nongroups` (using the loop-local `reg_class_names`), then
  ` new_bb_needs= changed=`.

### Reading it

`bash tools/ra_solver/reload_harvest.sh` runs the whole tree;
`python3 tools/ra_solver/reload_extract.py` turns the logs into JSON;
`python3 tools/ra_solver/reload_sim.py --check` validates the model.

### Reference-pollution note (2026-08-05)

FRAMEDBG numbers are **structurally immune to `build/` reference pollution**:
`gn_frame.sh` / `gn_census.py` / `gn_da.sh` read cc1's own stderr and its `-o`
output, and the `.frame vars=` figure is printed by `get_frame_size()` inside
cc1. None of them open `build/src/*.o`. Only the *sandbox score* and the
normalized objdump diff (`tmp/csz/d.sh`) consult `build/src/<stem>.o`, and they
consult it read-only.

Standing rule: **no script may write into `build/src/`** — scratch goes to
`tmp/` (or `/dev/shm`). A tool that overwrites the pristine reference before
scoring makes the measurement circular; the `-G8` census defect of 2026-08-05
is the cautionary case. The `build/cc1` paths in `tmp/csz/gn_*.sh` are the
*compiler binary* under `tools/gcc-2.7.2/`, a different `build/` entirely, and
are execute-only.
