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
