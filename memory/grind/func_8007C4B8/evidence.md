# Evidence bank — func_8007C4B8

- WIP rejected_form: {'form': "param-local-alias literal-rename (`Rect *_r = r; s32 *_out = out;` declared in target's pair order)", 'first_tried': 'af10dc8d (2026-06-01)', 'reverted_by': 'a7302887', 'verdict': "FORBIDDEN per [[param-local-alias-prologue-pair-flip]] + [[no-new-park-categories]] — literal renames with no semantic purpose, declaration order manipulates cc1's expand_function_start LUID assignment"}

- WIP rejected_form: {'form': 'migrate regfix `reorder 3,4,1,2 @ 1-4` to a per-function entry in tools/prologue_config.json', 'first_tried': '91fc0d90 (2026-06-15, orch0614b)', 'reverted_by': 'd925b85c (2026-06-15)', 'verdict': 'FORBIDDEN — mechanically identical cheat relocated to a different stage; prologue_fix is per-function allowlisted, not a general toolchain-faithful stage; cheats-by-any-spelling. User ruled today (2026-06-15).'}

- WIP rejected_form: {'form': 'cache r->flag into a local at function entry (s32 flag = r->flag; ...; if (flag != 0))', 'first_tried': 'fleet-blk session 2026-06-15 (variant tmp/c4b8_variants/v1_flag_cache.c)', 'measured_score': 52, 'build_insns_delta': '+1 (165 vs target 164)', 'verdict': "DISPROVEN — adding any body-level early reference to r adds an extra `lbu r->flag` load instruction (target loads it lazily right before the if). Body codegen sensitivity rules out cache-early-then-use-later as a lever. Also, structurally, this doesn't shift the move-sN-aN LUIDs because those are emitted at expand_function_start BEFORE body RTL — so even if it didn't add an instruction, it wouldn't flip the prologue pair order. Confirms the wall is purely at the prologue surface."}

- == imported from memory/wip notes.md ==
# func_8007C4B8 — TL;DR

**State:** INCOMPLETE, honest floor = distance 4 (HEAD, confirmed 2026-06-15).
**Wall:** prologue save+def pair-flip (twin of func_8007C2A0). Confirmed
purely at the prologue surface via objdump.

## The 4-insn gap (precise)

Sandbox prologue (from `tmp/sandbox/func_8007C4B8/display.o`):
```
sw   s1, 52(sp)    ; save s1 first
move s1, a0        ; out -> s1
sw   s0, 48(sp)    ; save s0 second
move s0, a1        ; r -> s0
sw   ra, 56(sp)
```
Target prologue (from `asm/funcs/func_8007C4B8.s`):
```
sw   s0, 48(sp)    ; save s0 first
move s0, a1        ; r -> s0
sw   s1, 52(sp)    ; save s1 second
move s1, a0        ; out -> s1
sw   ra, 56(sp)
```

Identical RA on both sides — `out` -> $s1 (a0), `r` -> $s0 (a1) — only
the EMIT ORDER of the two save+def pairs differs. The `regfix.txt:3619
reorder 3,4,1,2 @ 1-4` rule patches the four emitted insns post-cc1 to
target order; that rule is the cheat keeping the function in the queue.

## Why this is hard

cc1's `save_restore_insns` (gcc-2.7.2/config/mips/mips.c) emits each `sw $sN`
paired with that reg's first SET, in LUID order. Param LUIDs come from
`expand_function_start`, which walks param decls left-to-right: arg0 (`out`,
into $s1) gets lower LUID than arg1 (`r`, into $s0). So cc1 emits the $s1
pair first. Target wants the $s0 pair first. The only known "fixes" are
LUID-manipulation tricks that have all been ruled cheats.

**KEY (fleet-blk 2026-06-15):** the `move sN, aN` LUIDs are assigned
DURING expand_function_start, BEFORE body code's RTL is generated. So
body-level structural variants (early-cache, reorder, restructure) cannot
shift those LUIDs. Confirmed by measurement.

## Forms ALREADY tried and ruled cheats / disproven — DO NOT re-derive

1. **param-local-alias** (`Rect *_r = r; s32 *_out = out;` declared in
   target's pair order) — af10dc8d, reverted by a7302887. Archived as
   FORBIDDEN in `.claude/rules/param-local-alias-prologue-pair-flip.md`.
2. **prologue_fix migration** (move the regfix `reorder` to a per-function
   entry in `tools/prologue_config.json`) — 91fc0d90, reverted TODAY
   (2026-06-15) by d925b85c with user ruling "prologue_fix-migration is a
   cheat (cheats-by-any-spelling)".
3. **early-cache of r->flag** (`s32 flag = r->flag;` at function entry)
   — fleet-blk 2026-06-15. Measured score 52 vs baseline 4; build_insns
   165 vs target 164 (+1 extra `lbu`). DISPROVEN: body-level early
   references to r add a load target lacks AND cannot shift the move-sN-aN
   LUIDs anyway. See `rejected/v1_flag_cache.c`.

Forms (1) and (2) are explicitly enumerated in
`.claude/rules/no-new-park-categories.md` as the C2A0/C4B8 twin wall.

## What an honest next agent should try (status at 2026-06-15)

- **Sibling cross-check in display.c** — DONE 2026-06-15 by fleet-bw1.
  Enumerated 2-ptr-param fns: B3A8 (matched, no prologue rule), C2A0 +
  C4B8 (twins, same wall), CE0C (heavily reordered prologue). Inspected
  B3A8 target asm: cc1 saves only ONE callee-save reg ($s0 for `rect`);
  arg0 `str` stays in $t0 because str's lifetime is two narrow
  error-path calls. C4B8 cannot use this template: both `out` and `r`
  are used across 6+ straight-line JALs, so both args are forced to
  callee-saved regs — two save+def pairs are mandatory, pair-flip
  surface is structurally unavoidable in this allocation class.
- **Instrumented cc1 RTL dump** (`-da` + dump LUIDs) to confirm whether
  ANY non-rename structural change can shift the LUID ordering. Not
  actionable in the worker scope (cc1 is read-only per
  [[no-compiler-divergence]]; even if a pass were found, triggering it
  from C source would itself have to clear the cheat catalog).
- **Directed permuter** restricted to body-internal mutations. Low-yield
  by mechanism: fleet-blk's flag-cache measurement proved body-level
  early references can't shift expand_function_start LUIDs and tend to
  add loads target lacks.

## Path-forward decision (per [[no-new-park-categories]])

The policy explicitly enumerates only three options for this wall:

(a) novel pure-C lever (none identified across multiple agents/orchestrators)
(b) canonical-asm authorization — user declined on 2026-06-15 (d925b85c)
(c) keep parked / search continues — this is the current posture

This worker's outcome is `blocked` with floor=4 unchanged. The 2026-06-15
fleet-blk session disproved body-level early-cache; the 2026-06-15 fleet-bw1
session completed the sibling cross-check (the most promising untried lever)
and confirmed no analogous matched sibling exists in display.c — the
allocation class for C4B8/C2A0 (two heavily-used pointer params, both forced
to callee-saved regs across 6+ straight-line JALs) is structurally unique
in the file, and the only pure-C lever space (parameter-declaration changes)
is fully cataloged as cheats. This is NOT a fresh `needs-decision` because
the user just ruled (2026-06-15) and the answer was "search continues, not
canonical-asm yet."


- [s1] sandbox --disable all: score=4, target_insns=164, build_insns=164 — 4-insn prologue pair-flip only diff

- [s1] regfix.txt:3619 `func_8007C4B8: reorder 3,4,1,2 @ 1-4` is the sole rule; retirement blocks completion

- [s1] Target prologue (asm/funcs/func_8007C4B8.s:3-6): sw s0,0x30 / move s0,a1 / sw s1,0x34 / move s1,a0 — arg1 (r) pair FIRST

- [s1] Sandbox prologue (tmp/sandbox/func_8007C4B8/display.o): sw s1,0x34 / move s1,a0 / sw s0,0x30 / move s0,a1 — arg0 (out) pair FIRST (natural cc1 LUID order)

- [s1] Owner ruling 2026-07-17 10:35 (decisions.md:646) explicitly scopes the narrowed tombstone to include the twins — sanctioned form is a single forward-order FAKE-annotated alias with per-use dossier

- [s1] hirahira_w_frie (src/text1a_c.c:955-1007) is the analog application: `s32 *b = base; /* FAKE ... */` — final PASS 2026-07-17 11:25, sandbox 0

- [s1] For C4B8, arg0=out and arg1=r; target wants arg1's pair FIRST — so the aliased param must be arg0 (out), matching hirahira's aliased-arg0 shape

- [s1] Rejected forms banked: v1_buf_reorder.c, v1_flag_cache.c (measured score 52, +1 lbu), v2_flag_else_short.c

- [s1] Body-level restructures cannot shift prologue LUIDs (expand_function_start runs before body RTL) — dead lever class

- [s1] Sibling cross-check in display.c: only B3A8 matches without a prologue rule; B3A8 avoids the wall by keeping arg0 in $t0 (short lifetime), impossible for C4B8 where both args span 6+ JALs
