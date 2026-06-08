# WIP: func_800484A0 (src/text1b.c)

## TL;DR
HEAD honest pure-C distance: **5** (5 regfix rules). Session-1 floor: **4**.
**Session-2 floor: 2**, build_insns 35 vs target 36 (1 short). The new lever
is the **sibling-pattern two-call form** with an early `return` on the
decompress path — the same idiom already used (and matched) at lines 280-296
of the same file. GCC's cross-jump pass (jump.c `find_cross_jump`) merges
the two `gpu_LoadImage` call sites back into one, AND emits target's exact
addressing pattern: `addiu $s0, $sp, 24` directly (no addiu+move pair) plus
`addu $a1, $s0, $0` in the call's delay slot. The candidate's ONLY remaining
diff vs target is the combine fold of `arg0 += 4; arg0 += 8;` into a single
`addiu $s0, $s0, 12`.

## Resume

1. Apply `candidate.c` to `src/text1b.c` (replace the body of `func_800484A0`
   starting around line 359). The body is shorter and cleaner than HEAD's.
2. `& tools/eng.ps1 sandbox func_800484A0 --disable all` → expect score 2,
   build_insns 35.
3. Confirm the lone diff is the +4/+8 vs +12 fold by disassembling
   `tmp/sandbox/func_800484A0/text1b.o` vs `build/src/text1b.o`
   (`mipsel-linux-gnu-objdump -d`). See `tmp/dump484a0.sh` for the dump
   helper (gitignored — may need to re-create).
4. Pick a hypothesis from `meta.json.next_hypotheses` and execute. Permuter
   from the session-2 base is the highest-leverage next move (combine-fold
   defeat may be reachable via declaration-order mutations the permuter
   excels at finding).

## Why the candidate is not a cheat

The sibling-pattern two-call form with early return is the project-standard
"decompress-or-direct upload" idiom — see `src/text1b.c:289-295` for an
already-matched sibling using IDENTICAL structure. The 6-test vetting:

- **Semantic purpose: YES.** Two distinct upload code paths — decompress
  into buf then upload buf, OR upload arg0 directly. The early `return`
  after the decompress branch lets the code structure mirror the semantic
  distinction.
- **Human-programmer test: YES.** The sibling at line 280-296 proves a
  real programmer wrote this exact pattern in this exact file. It's
  idiomatic for decompression-with-fallback upload routines.
- **GCC-internals justification: NO.** The cross-jump merge IS a
  downstream optimization that collapses the two calls, but the C source's
  structural choice (two calls vs one) stands on its own program-logic
  rationale. The lever is "structure the C the way a programmer naturally
  would for this control flow", not "structure the C to bend RTL".
- **Permuter-derived: NO.** Derived by reading the matched sibling and
  recognizing the same pattern applies.
- **Family check: NOT IN ANY FORBIDDEN FAMILY.** No dead stores, no DImode
  chain, no register-asm pin, no volatile coercion, no lost-codegen-insert,
  no inline-asm-injection, no OR-tree shuffle.
- **Naming announces intent: NO.** All identifiers are original
  (`arg0`, `buf`, `rect`, `dim`) — no `_pad`/`_dummy`/`_spill`.

The candidate is SAFE TO COMMIT as a partial-progress wip checkpoint. The
cheat-reviewer agent should be invoked before any future close-to-COMPLETED-C
attempt that builds on this form.

## What remains (the 2-diff residual)

Disassembly comparison (build 35 vs target 36, candidate is 1 insn short):

| idx | candidate (session-2) | target | issue |
|---|---|---|---|
| 12 (beqz delay) | `addiu s0,s0,12` | `addiu s0,s0,4` | GCC's combine merged the two `+= ` into one |
| 13 | `sh a1,16(sp)` | `addiu s0,s0,8` | (target has the missing +8 here) |
| 14+ | (shifted -1 from target) | (shifted +1 from candidate) | cascade from the missing insn |

So there is ONE residual structural issue: GCC's combine pass merges
`(set s0 (plus s0 4))(set s0 (plus s0 8))` into `(set s0 (plus s0 12))`.
The intervening `sh $a1,16(sp); sh $a2,18(sp)` stores don't reference `arg0`,
so combine sees no use-conflict.

Seven C-level variants tried so far (session 1's 4 + session 2's 3) — none
defeat the fold. The remaining derivable avenues (see `meta.json.next_hypotheses`)
are the permuter + a sibling-deep cross-reference (why does the sibling's
`arg0 += 4; arg0 += 8;` NOT trigger the fold while ours does — that gap
matters and may reveal a pure-C lever).

## Verified rejected forms (do NOT re-derive)

See `meta.json.rejected_forms` — 7 forms tried across 2 sessions. Notably:

- Introducing a separate `s16 *p_buf` local goes to `$s1` (+1 callee-save,
  worse than HEAD).
- Assigning `arg0 = buf` in both branches emits the assignment twice (+3).
- Goto-form is bytewise identical to if-form (jump-threading collapses).
- The +4/+8 reorder doesn't change combine.
- Single-statement interleave: combine still merges (session 2).
- Wrapping body in `if(...!=0){...}`: cc1 jump-threads back to early-return.
- Offset-based dim load + collapsed `+= 12`: +1 addressing insn,
  combine still merges the remaining adds.

## Related rules

- [[register-alloc-pure-c]] — the parent RA playbook. The new lever (two-call
  cross-jump merge) is a *control-flow* application, not strictly an RA lever.
- [[cross-jump-call-merge]] — the GCC pass this lever leverages
  (find_cross_jump in jump.c). The session-2 form satisfies its requirements
  (same arg count + same ABI usage + shared 2-insn suffix `[j ; jr ra]`).
- [[store-before-jal]] — related delay-slot scheduling family; here we hit
  target's `addu $a1,$s0,$0` in the jal delay slot naturally via the
  cross-jump merge.
- [[do-while-zero-exception]] — explicitly NOT applicable to defeating
  the +12 combine fold (scoped narrowly to LABEL_OUTSIDE_LOOP_P / reorg.c).
- [[no-new-park-categories]] — the cheats-by-spelling policy the candidate
  was vetted against.
