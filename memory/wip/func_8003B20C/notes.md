# func_8003B20C (code6cac_c_ab.c) — WIP

**HEAD:** Carries `register s32 one asm("v1") = 1;` pin AND a dead
`u8 *new_var; ...(new_var = &D_800900EC)[arg0]` alias.  Both cheats.
Removing both gives an honest pure-C floor of **6** (down from 0 with
cheats present — the cheats make HEAD oracle-clean, NOT pure-C-clean).

**Candidate (this entry):** pure-C, sandbox `--disable all` = **6**,
build_insns = target_insns = 47.

## TL;DR for resume

1. Apply `candidate.c` verbatim into `src/code6cac_c_ab.c` at lines
   412–430 (replacing the existing `func_8003B20C` body).
2. Confirm `& tools/eng.ps1 sandbox func_8003B20C --disable all` reports
   `score: 6`.
3. The 6 residual diffs are **pure scheduling**: target emits the
   `D_80102780 = 0; D_80102781 = 1` stores BEFORE the array load; mine
   emits the array load first (GCC hoists it).  All registers match
   target.  Diff (target left, mine right):

```
   target            mine
   sb zero (780)     addu at,at,a0
   lui at, (781)     lw a0, 0(at)     ← array load
   sb v1, 0(at)      li v0, 0xFF
   lui at, (900EC)   lui at, hi(780)
   addu at, at, a0   sb zero, 0(at)
   lw a0, 0(at)      lui at, hi(781)
   li v0, 0xFF       sb v1, 0(at)
```

## Why the cheats appear to be load-bearing

The `register s32 one asm("v1") = 1` pin creates a hard-coded register
binding for the constant 1.  Hard-coded regs allocate before priority
sort and indirectly anchor scheduling (each consumer creates an explicit
data dep on `$v1`).

The dead `new_var = &D_800900EC` assignment creates a pseudo for the
address-of expression that gets used in `(s32 *)new_var [arg0]`.  Even
after combine folds the pointer back to the symbolic form, the pseudo's
SSA position pins the load near its source position.

Both effects are textbook cheats-by-spelling per
[[no-new-park-categories]] / [[inline-asm-injection]] / [[inline-asm-policy]]:
no semantic purpose, justified only by GCC-internals.

## Why no pure-C lever closes it (tried sessions 1 + 2)

Session 1 (2026-06-07):
- **Source order matches target, no cheats:** array load hoists past
  D_80102780/D_80102781 stores.  Score 6.
- **Explicit `arg0 = arr[arg0]` reassignment:** creates extra pseudo
  for arg0_new that competes for $v1 — constant 1 displaced to $a1.
  Score 9.
- **`arg0` = arr[arg0] hoisted to function start:** same as above plus
  same scheduling hoist.  Score 9.
- **`func_8003AE5C(arg0)` instead of memory reload:** arg0 needs to
  cross the file_LoadOverlay call — requires callee-save reservation +
  prologue change.  Score 14.

Session 2 (2026-06-08, NO_PROGRESS — 4 NEW variants + 5000-iter permuter):
- **Source reorder: array-load statement moved to source-position 3
  (right after `D_80102781 = 1`):** sandbox HELD at 6, same swap
  pattern.  GCC's scheduler ignores source-statement position for this
  reorder.
- **Fresh local `s32 v = arr[arg0]; ... D_800A3844 = v;`:** GCC's
  combine/CSE folds the local — emitted asm is byte-for-byte identical
  to the inline form.  Floor 6, unchanged.
- **Const local pointer `s32 *arr = (s32 *)&D_800900EC;`:** combine
  folds the pointer back to the symbol-ref.  Floor 6.
- **Byte-pointer math `u8 *base = &D_800900EC; *(s32 *)(base +
  arg0 * 4);`:** REGRESSES to score 9 (explicit byte arithmetic doesn't
  fold back to lui+addu+lw).  Strictly worse.
- **Permuter random-mode 5000+ iter (j=8, ~25min):** best weighted
  score 200 (down from 385 baseline) but all candidates are cheat-class
  by inspection: do-while-zero misapplied to sched.c (not the
  LABEL_OUTSIDE_LOOP_P / reorg.c interaction the
  [[do-while-zero-exception]] is scoped to), `unsigned char` truncation
  of the array element, `(long long)0` DImode cast chain-extender,
  `D_800A3844 ^ 0` no-op XOR, signature change to `unsigned short
  arg0` (caller-breaking).  See meta.json rejected_forms for full
  enumeration.  Random mode is exhausted — DIRECTED PERM_GENERAL is
  the un-mined surface.

## Resume avenues (NOT exhausted)

1. ~~**Directed permuter** from `candidate.c` base — random restructure
   may surface a non-cheating C form that pins the load.~~
   **PARTIALLY DONE 2026-06-08** — random-mode permuter (5000+ iter,
   ~25 min wallclock, 8 threads) returned best weighted score 200 but
   all candidates are cheat-class (do-while-zero misapplied to sched.c,
   u8/short type truncations, no-op XOR, DImode `(long long)0` chain,
   signature changes). Random mode is exhausted on this function.
   **DIRECTED PERM_GENERAL is the un-mined surface** — hand-author
   statement-permutation alternatives in the base.c (the 9 byte-store
   statements + array-load statement are all SEMANTICALLY commutative
   since they touch disjoint globals) and let permuter explore the
   structurally legitimate space.
2. **Instrument `sched.c`** (BB2_SCHED_DEBUG hook in `tmp/gccdbg/cc1`
   from saEft00Add work — see `[[cross-jump-store-tail-merge]]`) to read
   the exact INSN_PRIORITY of the load vs. the stores and find what
   would equalize them.  **CONCRETE QUESTION TO ANSWER:** in mine,
   INSN_PRIORITY(lw $a0 array-load) vs INSN_PRIORITY(sb $zero D_80102780)?
   If the byte-store wins, the question is why — is it the LUID
   tiebreaker (source order) or genuinely the chain-cost calculation?
3. **Const file-scope alias** for `&D_800900EC` (`const s32 *D_900EC_tbl =
   (const s32 *)&D_800900EC;` if it survives combine without being folded
   to the symbol_ref).  Not yet attempted in any session.
4. **SOTN/community cross-reference** — search SOTN's overlay-load
   functions (src/main/, src/dra/, src/st/) for similar
   `byte-stores-then-array-load-then-call` shapes. Mechanism is generic
   (GCC sched.c INSN_PRIORITY hoist).

## Cheat-reviewer status

NOT invoked — the candidate has no cheats so reviewer would PASS, but
the entry isn't being committed to `src/` (the candidate breaks oracle
by 6 instructions).  Reviewer to be invoked at close-out if and when
a sandbox-0 form is found.

## Related

- [[no-new-park-categories]] — cheats-by-spelling policy this candidate
  honors
- [[inline-asm-policy]] / [[inline-asm-injection]] — the HEAD pin + dead
  alias fall under these forbidden categories
- [[register-alloc-pure-c]] — RA levers tried (block-local var split
  doesn't apply; no var split to make)
- [[store-before-jal]] — confirms `D_800A3844 = arg0; ... func_X(D_800A3844)`
  via memory reload is the correct target pattern (don't use C-level
  arg0 across the call)
- [[difficult-is-not-impossible]] — the standing rule; pure-C floor
  WAS lowered (cheats stripped), but the residual 6 is genuinely
  unfinished work, not "impossible"
