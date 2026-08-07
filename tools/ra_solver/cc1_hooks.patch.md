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
> rebuilding**, then do the usual parity check.

Rebuild with `cd tools/gcc-2.7.2 && TMPDIR=/dev/shm make cc1` (produces `./cc1`;
`build/cc1` is untouched — verify with tmp/parity_check.sh pattern).

## What the hooks are — and the ONE exception (corrected 2026-08-06)

This file used to say "All are print-only fprintf under env guards." **That was
an overstatement.** Almost all of them are, but `reorg.c` carries two
**BEHAVIOURAL what-if knobs** that change codegen when enabled:

| knob | effect when set |
|---|---|
| `BB2_NO_FT_STEAL` | makes `fill_eager_delay_slots` skip the fall-through fill entirely |
| `BB2_ALLLIVE_LABEL=<uid>[,<uid>…]` | forces `mark_target_live_regs` to its conservative everything-live answer for the listed target uids (simulating `find_basic_block () == -1`) |

Both are **inert unless their env var is set**, both live only in the
diagnostic binary (`tools/gcc-2.7.2/cc1`), and neither is reachable from the
shipped compiler. But they are experiments, not observations: **never enable
them while gathering evidence without saying so in the record**, because any
dump produced under them describes a compiler that does not exist.

### Inertness proof for the WHOLE hook set (2026-08-06)

Not an assertion — measured. A cc1 built from **pristine reverted sources**
(every local modification backed out) emits **byte-identical output to the
fully instrumented cc1 on all 32 TUs**. That covers every hook in this
document, the two knobs above included, and is stronger than the per-hook A/B
in section 7 because it removes all of them at once rather than one family.

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

## 7. local-alloc.c — BB2_SUGG_DEBUG, the suggested-register pass (NEW 2026-08-06)

The last unmodeled local-alloc pass. Print-only under `getenv ("BB2_SUGG_DEBUG")`.
Both hooks live in `local-alloc.c`.

### `block_alloc` — the per-qty input table

Immediately after the first `qty_order[i] = i;` loop and **before** the
`qty_sugg_compare` sort, one line per qty:

    SUGGDBG-QTY func= blk= qty= reg1= birth= death= refs= size= mode= minclass=
                altclass= calls= chgsize= ncopysugg= nsugg= copysugg=a,b, sugg=c,d,

**Placement is load-bearing.** It must be dumped before the suggested-register
pass runs, because `find_free_reg`'s retry path (local-alloc.c:2216) *clears*
`qty_phys_num_copy_sugg[qty]` as a side effect — a dump taken afterwards
under-reports the copy suggestions on exactly the qtys that are most
interesting.

`size` is `qty_size`, previously the known Phase-5 hook gap: the Python model
hardcoded 1, which feeds `qty_compare`'s priority directly.

### `find_free_reg` — the scanned hard-reg sets

Immediately before the `GO_IF_HARD_REG_SUBSET (..., fail)`, i.e. after
`first_used` is final:

    SUGGDBG-FFR qty= class= mode= jts= acc= born= dead= used=… first_used=…

`used` is the conflict set; `first_used` is it restricted to the suggestion set
when `jts` (`just_try_suggested`) is on — that restriction *is* the whole of the
suggested-register preference. One line per call, so the copy-sugg→sugg retry
and the caller-saves retry each appear as their own line.

### Reading it

    python3 tools/ra_solver/local_extract.py <stem> --suggest   # -> <stem>.sugg.json
    python3 tools/ra_solver/local_alloc.py   <stem> --suggest   # scores the pass
    python3 tools/ra_solver/inverse.py local … --sugg <stem>.sugg.json

`--suggest` is additive: `<stem>.local.json` is byte-identical with and without
it (verified on 5 stems against the pre-change extractor), so every existing
consumer is unaffected.

**Model status: EXACT.** Corpus-wide (all 32 TUs): preference 1578/1578,
assignment 947/947. Two source details the first cut missed, both now modelled:
the retry at 2216 scans the *plain* suggestion set (the pre-pass `ncopysugg` no
longer applies), and a qty's several `jts=1` calls are one retry chain with a
single outcome, not several assignments.

### Neutrality evidence

A hooks-free cc1 was built from the same tree and compared against the
instrumented one on all 32 TUs: **identical output on every TU**, and
`BB2_SUGG_DEBUG=1` is itself output-inert. Script: `tmp/sugg_ab3.sh`.
The project build uses `tools/gcc-2.7.2/build/cc1` (Makefile:12), which these
hooks do not touch at all.

### Fidelity caveat found while validating (2026-08-06) — CLOSED 2026-08-07

**Historical, kept for the reasoning.** The instrumented `tools/gcc-2.7.2/cc1`
and the build compiler `tools/gcc-2.7.2/build/cc1` were different builds and
**disagreed on two TUs**: `ings` (5 diff lines) and `code6cac_b` (2 diff
lines) — constant-folding and `ori`-vs-`addu` differences, unrelated to any BB2
hook. Dumps for those TUs did not describe what the project builds, so
`local_extract.py` carried the pair in `UNFAITHFUL_STEMS` and warned on stderr
at the point of use.

**Resolution (2026-08-07).** The cause was found: only `build/cc1` carried a
removal of `combine.c`'s PLUS→IOR conversion — an undocumented 2026-05-18
compiler-patch experiment that had silently become the project compiler. That
patch is now committed (`tools/cc1-no-plus-to-ior.patch`), the oracle compiler
is rebuilt from it by `tools/build_oracle_cc1.sh`, and the instrumented cc1 is
rebuilt from the same hooked sources **plus the same patch** by
`tools/build_diagnostic_cc1.sh`. The two now agree on **all 32 TUs**, so
`UNFAITHFUL_STEMS` is empty — the divergence was removed, not waived, and
`saTan2KabutoWareMove` (the consumer this blocked, in `code6cac_b`) has
faithful solver evidence available again.

Re-verify at any time with `bash tools/build_diagnostic_cc1.sh` (no
`--install`): it names the divergent stems, and `none` is the contract. Full
background, including the still-open question of whether the ORIGINAL PsyQ
compiler performed that conversion, is in `docs/ORACLE-COMPILER.md`.

### What this pass did NOT explain (2026-08-06)

The hook was built for two functions and cleared neither. Recorded so nobody
re-runs the avenue:

* **`camera_set_zoom`** (banked floor 3, block 41) — with `candidate.diff`
  applied the function has 52 qtys and **none** carries a suggestion. The pass
  never runs here, so the banked "a `$v0` suggestion on the pointer would
  pre-empt the main pass" hypothesis is refuted structurally: `combine_regs`
  only creates a suggestion when one side of a copy is a HARD register, and the
  contested block (`lw $v1,0($s0)` / `li $v0,2` / `sh $v0,646($v1)`) has no
  hard-reg copy. The other named gap, `qty_size`, measures 1 for both contested
  qtys — the model's hardcoded 1 was already right. The 12 `alloc_order` vectors
  survive unchanged and the necessary-not-sufficient caveat still stands.
* **`DispPracticeMenuTex_A`** — 2 of 72 qtys carry a suggestion (block 13,
  `reg1=217` and `reg1=225`, both suggested `$a0`, both placed and honored).
  No contested pseudo is among them, and not merely because it is absent from
  the `reg1` column: pseudos 100/83/84/98/85/99 are all **global allocnos**
  (present in `DispPracticeMenuTex_A.model.json`'s `order`), and `global_alloc`
  builds allocnos only for pseudos local-alloc left unassigned — so they cannot
  be a member of any qty, first or otherwise. Pseudo 132 is local-allocated and
  carries no suggestion. The validated-UNREACHABLE verdict stands.

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
