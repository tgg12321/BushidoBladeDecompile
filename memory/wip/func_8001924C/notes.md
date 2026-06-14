# func_8001924C (code6cac.c) — BLOCKED: phantom 8-byte frame local (forbidden pad cheat)

## TL;DR
HEAD byte-matches ONLY via a forbidden frame-coercion cheat: `s32 pad[2];`
(an unused local array, flagged by `find_unused_local_arrays` /
[[dead-vars-local-array]]). The honest pure-C distance (sandbox, pad stripped) is
**12, and ALL 12 diffs are a single +8-byte stack-frame-size difference** — the
loop body is otherwise byte-identical to target. Removing the pad confirms it is
inert to the body and only inflates the frame 40→48. There is NO recoverable
semantic local: the target reserves 8 bytes at `sp+16..23` but NEVER reads or
writes them. Per dead-vars-local-array this needs a semantically-named real local
(unrecoverable here) or canonical-asm authorization — a worker cannot resolve it.

## The gap (precise)
sandbox build (no pad): `addiu sp,sp,-40`, saves at 16/20/24/28/32.
target:                  `addiu sp,sp,-48`, saves at 24/28/32/36/40, with an
unused 8-byte hole at sp+16..23. The 12 diffs are exactly the 6 prologue saves +
6 epilogue restores at shifted offsets + the two `addiu sp` — i.e. one frame-size
delta, nothing in the body.

## Why not pure-C-closable
- The 8 bytes are a genuine local the original C declared, but every USE of it
  was optimized away — the body never touches the slot, so there's no clue to its
  semantics (no callee takes its address; both calls pass only `s0` + the computed
  `val*52+base` pointer; no spill; 40 is already 8-aligned so it's not alignment).
- `s32 pad[2]` reproduces the size but is the FORBIDDEN frame-coercion cheat
  (dead-vars-local-array, 2026-05-31). A named guess (`MatrixScratch[2]` etc.)
  with no body reference is the same cheat under a nicer name.
- Per the rule: when the frame can't be matched with a semantically-meaningful
  local, the function is not COMPLETED-C-matchable in pure C → park / canonical-asm.

## Important: HEAD currently relies on the cheat
HEAD's `pad[2]` DOES produce the 48-byte frame in the full build (canonical
build/src/code6cac.o = `-48`), so the oracle is green at HEAD — but via the
forbidden dead-array. This function is INCOMPLETE: its "match" is a frame cheat.
`queue done` / check_completion_integrity would flag the pad.

## Resume guidance
1. Confirm: remove pad → sandbox 12 (pure frame-size diff). Don't re-derive.
2. Recover the semantic local if possible: re-examine callers of func_8001924C
   and the data layout of `g_file_data_buf` / the `s0` 16-byte stride records —
   is there a stack struct the loop fills then passes? (Body shows none, but a
   caller-level view might.) If a genuine named local with real (even if folded)
   use is found, that's the COMPLETED-C path.
3. Otherwise this is canonical-asm / park territory (unrecoverable phantom local).
   Surface for user authorization.

Source left at HEAD (oracle green via the pad cheat). Card BLOCKED.
