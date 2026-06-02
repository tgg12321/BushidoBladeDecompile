# func_8007B844 — WIP resume notes

## TL;DR

- **Floor 6** (HEAD floor 7). Lever: named-intermediate `u32 mask;` (Lever B
  from `register-alloc-pure-c.md`). Stable across 3 sessions, 50+ structural
  variants, ~50k permuter iters.
- **Closing forms found are FORBIDDEN cheats** — preserved at `rejected/`:
  - `rejected/conditional_dead_store.c` — score 0 + SHA1 == oracle, but
    Lever D family (find_dead_conditional_stores detector catches it).
  - Function-pointer return-type lie (described in `meta.json`
    rejected_forms[1] but not preserved as a file — different cheat shape).
- **Hand-coded asm: ZERO signals.** S1 (handwritten annotations) = 0; S2
  (hardcoded register signatures) = none; S6 (custom ABI) = none; S8
  (packed-multiply pattern) = no. Every instruction has direct C analog;
  no GTE/BIOS/scratchpad. **Canonical-asm authorization is NOT
  evidence-backed** despite the worker's session-3 framing — the
  structural ceiling is a `sched.c` INSN_PRIORITY wall, same class as
  cpu_side_move_dir_4 / marionation_Exec.

## How to resume in one read

1. Read `meta.json` — note `instrumented_evidence` (the BB2_SCHED_DEBUG
   finding pins the exact mechanism), `rejected_forms` (do NOT re-derive
   these), `next_hypotheses`.
2. Read `rejected/conditional_dead_store.c` to understand the cheat
   shape (so you don't re-derive it under a different variable name like
   `_pre`, `stage`, `tmp`, etc.).
3. Apply `candidate.c`'s body to `src/display.c`. Confirm score 6 with
   `& tools/eng.ps1 sandbox func_8007B844 --disable all`.
4. Pick a `next_hypothesis`. Top candidate: fresh SOTN borderline-research
   pass focused on return-staging priority patterns (the 2026-06-02
   research found 6 new SOTN-allowed techniques; another pass may surface
   patterns specific to vtable-dispatch wrappers).

## The mechanism (instrumented, decisive)

Target asm (40 lines, src/display.c offset 199):
```
0x6C0A8: lui  $a0, 0xFF        ; mask = 0xFFFFFF
0x6C0AC: ori  $a0, $a0, 0xFFFF
0x6C0B0: addu $v0, $s0, $zero  ; <- return-staging (move v0,s0) BEFORE store
0x6C0B4: lui  $v1, %hi(D_8009BF30)
0x6C0B8: addiu $v1, $v1, %lo(D_8009BF30)
0x6C0BC: and  $v1, $v1, $a0    ; addr & mask
0x6C0C0: sw   $v1, 0($v0)      ; *ot = result, via the return-staged $v0
```

The candidate's score-6 form emits the same instructions but in a
different order: store first, return-staging after. Reason:

> cc1 sched.c gives return-staging INSN_PRIORITY = 1 (chain depth to
> `jr ra` is 1); the mask/addr chain has priority 4 (lui->addiu->and->sw,
> depth 4). `rank_for_schedule` picks higher-priority first; mask/addr
> chain emits before return-staging.

To match target, return-staging's chain depth must extend to >=4. The
only way in C is to make a later instruction consume `$v0` — but every
tested `u32 *p = ot; *p = ...; return p;` form folds to `ot` via
copy-prop. No SOTN-allowed construct prevents the fold.

## What's NOT canonical-asm

The worker's session-3 framing suggested "canonical-asm authorization OR
a fresh SOTN borderline-research pass" as next-step options. The
canonical-asm option fails the evidence threshold for THIS function:

| Signal | Threshold | B844 finding |
|---|---|---|
| S1 — `/* handwritten instruction */` annotations | grep > 0 | 0 |
| S2 — hardcoded source registers (SOTN signature) | yes | no — all ABI standard |
| S6 — custom-ABI / non-standard arg passing | yes | no — `$a0..$a3` |
| S8 — redundant low-bits mask before discarding shift | yes | no — mask is real (24-bit OT pointer) |
| no-C-form constructs | GTE/BIOS/scratchpad | none — all standard MIPS |

Per `.claude/rules/hand-coded-asm-recognition.md` strict gate, B844 fails
the canonical-asm test. Per `.claude/rules/difficult-is-not-impossible.md`,
the structural ceiling is "unfinished work," not "proven impossible."

The honest next-step menu is therefore:
1. Fresh SOTN borderline-research pass (genuine option)
2. Sibling cross-reference (look for a matched gpu_LinkList-style wrapper)
3. Instrumented BB2_SCHED_DEBUG re-probe on the candidate
4. ~~Canonical-asm authorization~~ — not evidence-backed

## Related

- `memory/project/func-8007b844-conditional-dead-store.md` — full
  session-2 ledger documenting the conditional dead-store discovery and
  its rejection
- `.claude/rules/register-alloc-pure-c.md` — Lever B (named intermediate)
  used to reach the score-6 base; Lever D (dead self-assign) is the
  forbidden closing form's family
- `.claude/rules/no-new-park-categories.md` — the cheats-by-any-spelling
  policy that catches the rejected conditional dead-store
- `.claude/rules/difficult-is-not-impossible.md` — the cardinal rule
  that says "stuck on a sched.c priority is unfinished work, never
  proven-impossible"
- `.claude/rules/hand-coded-asm-recognition.md` — the strict gate that
  this function fails (no S1/S2/S6/S8 signals; no no-C-form constructs)
