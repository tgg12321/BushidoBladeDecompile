---
name: decomp-cheat-cleanup
description: End-to-end retirement of one cheat-cleanup queue item. Invoke whenever the user asks to "work the cheat queue", "do the next cheat cleanup", "retire a cheat", "next cheat-cleanup item", "work an orphan cheat", or similar BB2 cheat-retirement phrasings. ONE FUNCTION PER INVOCATION — for multi-function batches, re-invoke after each successful commit. Pulls from `dc.sh next-cheat-cleanup` (the Cheat Cleanup Queue — functions currently SHA1-matching ONLY because of a regfix/asmfix/inline-asm cheat). Starting state is matching-via-cheat; success state is matching-via-pure-C with the cheat rules removed. Different from `decomp-next` (which starts from UNMATCHED): here you remove the cheat first, watch SHA1 break by exactly N bytes (= the cheat's contribution), then coerce the C source to emit equivalent codegen via the documented techniques (inline_move_aliasing, declaration reorder, intermediate vars, register asm pins, scheduling tricks). Inherits §0 contract from decomp-next — no premature stopping, switch technique not target, build new tooling when stuck.
---

# /decomp-cheat-cleanup — single-invocation cheat retirement

You were invoked to take ONE function from the Cheat Cleanup Queue and retire its cheat(s) — meaning: remove the cheat rules from `regfix.txt`/`asmfix.txt`/`src/*.c`, coerce the C source to emit equivalent codegen naturally, end with SHA1 still byte-identical to the original, commit.

Read this entire skill before doing anything. The rules below are the contract.

---

## §0. THE CONTRACT (re-read whenever tempted to deviate)

**You stop only when ONE of these is true:**

1. The cheat rule(s) have been removed AND `dc.sh verify <func>` reports MATCH AND the commit landed (hook auto-cleared `.bb2_active_func`).
2. `dc.sh start` reports `Build: MISMATCH` **before you've done any work** — pre-existing repo problem; report and stop.
3. Catastrophic external state (WSL down, disc missing, splat refuses). Genuine impossibility, not difficulty.

**Every other reason to stop is forbidden.** Same as `decomp-next`:

- Score plateau, N attempts, N hours, N regfix rules
- "Diminishing returns" / "good place to stop" / "next session can continue"
- "The cheat can't be removed in C" — see THE PHYSICS CHECK below
- "GCC's heuristic is sticky" — that's a description of your plateau, not the language
- "I've exhausted the toolbox" — you haven't. Build a new tool.

**THE PHYSICS CHECK (mandatory whenever you feel walled).** Before any "the cheat is canonical" / "C can't emit this" framing, ask:

> **"Is this code canonically C?"**

For ~99% of queue items, yes — Lightweight, GCC 2.7.2, PsyQ SDK 3.5, 1998. A pure-C body that emits matching bytes exists. The toolchain is deterministic; you just haven't found the right structure.

**If you genuinely believe the cheat is canonical asm**, the path is NOT "give up and authorize" — it is:
1. Run `dc.sh scan-hand-coded --func <func>` to get the hand-coded-asm tier
2. If `STRONG` with S1/S2/S6 signals, surface the evidence to the user and request authorization to add to `inline_asm_canonical.txt`
3. Do NOT self-authorize. The user's standing rule: *"we have to confirm with extreme confidence that the source was asm not C before using inline asm in any circumstance."*
4. If authorized: skip §5, replace the C body with the canonical inline-asm pattern, add to `inline_asm_canonical.txt`, commit

If the function is on the cheat queue and NOT on the strong-signal hand-coded list, **it's C** — keep pushing the §5 coercion ladder.

**Asking the user to release / accept partial / accept the cheat as canonical is forbidden while §5 avenues remain unexplored.**

---

## §1. PRE-FLIGHT

Run this first:

```
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && bash tools/dc.sh start'
```

Branch on the briefing:

| Briefing line | Action |
|---|---|
| `Build: OK` + `Active: NONE` | Proceed to §2 |
| `Build: OK` + `Active: <func>` | Skip §2 — resume `<func>` from current state (it was a prior cheat cleanup); go to §3 |
| `Build: MISMATCH` | STOP. Report repo state. Hook can't help; baseline issue requires user investigation. |
| `Queue: <N> days old` (N > 7) | Run `dc.sh refresh-queue` before §2 |

**If active marker is set to a non-cheat function** (e.g., from a running decomp-next session): STOP. Don't override their work. Report and ask whether to wait or pick a different cheat orphan in a different file. Two simultaneous active markers in the same worktree are not supported.

---

## §2. PULL ONE CHEAT ORPHAN

```
bash tools/dc.sh next-cheat-cleanup --with-context
```

Sets `.bb2_active_func` to the top of the cheat queue and runs `dc.sh agent-brief <func>` for the full context dump. The brief includes: classification, full target.s, neighbor functions, kengo reference, existing regfix/asmfix rules.

**Parallel-safety check.** Before launching §3, verify your worktree isolation:

```
git rev-parse --git-dir              # should NOT be the main repo if running parallel
cat .bb2_active_func                  # the func you just pulled
```

If another agent is working a function in the same `src/*.c` file as yours, expect merge conflicts on commit. Check the active queue and consider pulling a cheat orphan from a different file — there are typically cheat orphans in 10+ different files (see `dc.sh next-cheat-cleanup 28` for the full list).

**Files with NO active decomp agent work (safest for parallel cheat cleanup as of last check):** `code6cac_b.c`, `code6cac_b2.c`, `code6cac_c_mid.c`, `config.c`, `ings.c`, `ings2.c`, `system.c`, `text1a.c`, `text1a_c.c`, `text1a_c2.c`. These have cheat orphans but the active decomp queue doesn't touch them.

**Files heavily contended with decomp agent:** `text1b.c` (54 active queue items live here). Skip cheat orphans in this file unless you specifically know the decomp agent is on a different function.

---

## §3. IDENTIFY THE CHEAT TYPE

The agent-brief shows the function's `cheat_types` tag from WORK_QUEUE.md. There are five types; each has its own retirement strategy.

| Cheat type | Where it lives | Typical fix |
|---|---|---|
| `lost_codegen(N)` | regfix.txt `insert "addu RD, RS, $zero"` rules | inline_move_aliasing trick OR coerce GCC via C-source reorder |
| `c_body_multi_insn` | `__asm__("op1\nop2")` inside C function body | Split into per-instruction `__asm__` blocks OR move work to C |
| `inline_asm` | File-scope `__asm__(.section .text\nglabel <func>...)` in src/*.c | Write pure C body (entire function) |
| `bios_inline` | File-scope `__asm__` for BIOS-style trampoline | Usually authorize (truly hand-coded); or rewrite with C+small asm primitive |
| `splice` | Large `splice K..N "..."` rule in regfix.txt | Write real C that produces target bytes |

**Surface the exact cheat rule(s):**

```
# For regfix cheats:
grep -n "^<func>:" regfix.txt regfix_stage2.txt

# For src cheats:
bash tools/dc.sh inline-locate <func>    # for file-scope inline_asm
grep -n "<func>" src/*.c                  # to find c_body multi-insn definitions

# For asmfix cheats:
grep -n "^<func>:" asmfix.txt
```

**Read each cheat rule.** You need to know exactly what it injects/replaces. The fix is to make C produce the same bytes naturally.

---

## §4. BASELINE BEFORE TOUCHING ANYTHING

**Take a snapshot of "current matching state" before iterating.** This is the reference point — every change must either preserve match or break it in a controlled way that you understand.

```
# Confirm current build matches
bash tools/dc.sh verify <func>          # expect: MATCH (since it's a cheat orphan)

# Optional: save the matched asm bytes for sibling comparison
mipsel-linux-gnu-objdump -d build/bb2.elf | awk '/<<func>>:/,/^$/' > /tmp/<func>.matched.s
```

**Identify what GCC currently emits.** Run the build pipeline through maspsx for the function and dump-text it:

```
bash tools/dc.sh dump-text <func>       # post-maspsx, pre-regfix instruction stream
```

Compare this to target.s and to the regfix rules. The regfix rules transform `dump-text` output into target. The cheat rule fills a specific gap. You need to know that gap.

---

## §5. THE COERCION LADDER

Try in this order. Each rung requires SHA1 to either stay matching or break in a way that gets you closer to a clean removal. **Revert immediately on regression.**

### 5.0 Remove the cheat rule first (controlled break)

Edit `regfix.txt` / `asmfix.txt` / src/*.c to remove the cheat. Build:

```
bash tools/dc.sh build
```

Expected: build fails with N-byte size diff, where N matches the cheat's contribution (typically 4 bytes per single-instruction insert). If the diff is much bigger (>20 bytes), the cheat was masking a larger structural mismatch — pause and re-read §3.

**This first failure tells you the size of the gap you need to fill in C.** A 4-byte gap = 1 MIPS instruction. A 12-byte gap = 3 instructions. A larger gap means multiple cheats or a structural mismatch.

### 5.1 inline_move_aliasing trick (the #1 fix for lost_codegen)

For `addu RD, RS, $zero` (a redundant register copy that GCC's optimizer ate), use the single-instruction inline asm pattern documented in `feedback_inline_move_aliasing.md`:

```c
register T dst asm("$RD");
register T src asm("$RS");
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));
```

This emits exactly one `addu RD, RS, $zero` instruction. **Single-instruction `__asm__` is NOT a cheat** (the audit only flags multi-instruction inline asm and file-scope inline-asm bodies). This is a documented and accepted technique.

Place the `__asm__` in the C source where the target wants the addu to appear. If the cheat was `insert "addu $16,$0,$zero" @ 28`, the addu needs to land at maspsx position 28 — work backward from `dump-text` output to figure out which C statement produces position 28.

### 5.2 Declaration reorder + intermediate variables

GCC's instruction scheduler is sensitive to:
- The ORDER in which locals are declared (affects when they're spilled/loaded)
- Whether an intermediate variable is introduced (forces a store/load pair)
- Whether a value is computed inline vs assigned first

If the cheat is a scheduling fix ("move s0=0 from delay slot to branch target" in the regfix comment), try:
- Moving the `s0 = 0;` assignment closer to or further from the branch
- Splitting the branch condition into an intermediate variable
- Reordering unrelated statements to perturb GCC's scheduling

Build after each variant; SHA1 either matches or shows the same N-byte diff in a different location.

### 5.3 Register asm pins (last resort for register-allocation diffs)

`register T x asm("$N")` constrains GCC's register allocator to use $N for x. Useful when the cheat is `subst "register X" "register Y"` (GCC picked the wrong register).

**Caveat: pins are HINTS, not orders.** Per `feedback_register_asm_pin_reliability.md`, GCC ignores them when they fight a strong RA preference. Verify with `dump-text`; when ignored, the natural fix is regfix swap (but that's just trading one mechanism for another — better C structure is the real fix).

### 5.4 Scheduling tricks documented in memory

Read these BEFORE rolling your own:
- `feedback_store_before_jal.md` — lookup-store-via-delay-slot
- `feedback_strength_reduce_defeat.md` — when `if (t<0) -((-t)<<N); else t<<N` collapses to one `sll`
- `feedback_dead_branch_scheduling.md` — branch-likely scheduling artifact
- `feedback_inline_move_aliasing.md` — the move trick (already covered in §5.1)
- `feedback_matching_playbook.md` — the full toolbox
- `feedback_regfix_reference.md` — every gotcha

### 5.5 Switch to the permuter

```
bash tools/dc.sh permute-adaptive <func>
```

Randomized C-structural search. Right tool when you can SEE the diff (via `dump-text`) but can't manually find the C-source pattern that produces the target. Set `--max-time 1800` (30 min) for a meaningful sweep.

### 5.6 Build new tooling

If the existing toolbox can't close the gap, write a new regfix op, new transformation pass, or new memory recipe. This is expected — the toolbox grew from prior matches.

---

## §6. VERIFY + COMMIT

When `dc.sh verify <func>` reports MATCH with the cheat rule(s) removed:

```
# Sanity: confirm the cheat is actually gone
grep -n "^<func>:" regfix.txt regfix_stage2.txt asmfix.txt   # should show no cheat lines
grep -n "<func>" src/*.c                                      # should show only the C body, no file-scope __asm__

# Full SHA1 verify (also checks no sibling function regressed)
bash tools/dc.sh verify --all                                 # expect: bb2 matches!

# Caller-audit (catches cascade-regressions where you changed the function signature)
bash tools/dc.sh caller-audit <func>
```

If `verify --all` fails on a sibling function, you've caused label-renumber drift. Run `bash tools/dc.sh fix-label-drift` and rebuild.

### 6.1 Commit

```
git add <files-you-touched>
git commit -m "cheat-cleanup: <func> — <one-line summary>

Removed cheat: <which rule(s) you removed, with regfix.txt line range>
Coercion: <which §5 technique you used>
Build: bb2 matches! (verified --all)
"
```

The active_func_guard hook will verify `dc.sh verify <func>` returns MATCH before allowing the commit and auto-clears `.bb2_active_func`.

### 6.2 Push

```
git push origin main
```

If push is rejected because main moved (another agent committed), `git pull --rebase origin main` then re-push. Your commit is small and unlikely to conflict if you stayed in a single file outside the decomp agent's text1b.c work.

### 6.3 Regenerate the queue

```
bash tools/dc.sh refresh-queue
git add WORK_QUEUE.md
git commit -m "queue: refresh after <func> cheat retirement"
git push origin main
```

The function should drop off the Cheat Cleanup Queue automatically (it's no longer cheating).

---

## §7. ANTI-PATTERNS — DO NOT

- **Don't re-add the cheat under a different name.** If you remove `insert "addu ..."` and replace with `subst ".*" "addu ..."`, you've moved the cheat, not retired it. The auditor will catch both forms.
- **Don't introduce multi-instruction inline `__asm__` to fix a lost_codegen cheat.** That's c_body_asm_debt — a different cheat. The §5.1 trick uses SINGLE-instruction `__asm__ volatile("move ...")`, which is accepted.
- **Don't authorize the function as canonical asm unless §0's strong-signal check actually fires.** Self-authorization is forbidden. Per `feedback_bridge_is_not_decomp.md` and `feedback_hand_coded_asm_recognition.md`.
- **Don't commit with SHA1 mismatch.** The hook should block this, but if you bypass somehow, you've broken the build.
- **Don't bundle unrelated changes.** Your commit should touch only the cheat-retirement files (regfix.txt, the src/*.c file, and `WORK_QUEUE.md` if regenerated). Avoid `git add -A` — the decomp agent's WIP shouldn't be in your commit (see `feedback_parallel_worktree_work.md`).
- **Don't quit because "the cheat is structural" / "GCC can't emit this" / "this would require a new pipeline pass."** See §0. Switch technique, build the tool.

---

## §8. WHEN MULTIPLE CHEATS ARE ON ONE FUNCTION

Some functions have multiple cheats (rule_count > 1 in the queue). Two options:

**Option A: Remove all at once (preferred for related cheats).** Read all the rules; they often work together (a `subst` + an `insert` are one logical transformation). Removing one but leaving the others usually breaks SHA1 in confusing ways. Remove the cluster, then coerce.

**Option B: One at a time (for clearly independent cheats).** Remove one cheat, verify SHA1 with the others still present, commit. Repeat. This gives smaller commits but doubles the iteration count.

Default to Option A unless the rules are at very different positions in the function and clearly unrelated.

---

## §9. ESCALATION TO USER

The only reasons to surface to the user mid-flight (not stop, just ask):

1. **Hand-coded-asm authorization request** (per §0). You ran `dc.sh scan-hand-coded`, got `STRONG` tier with S1/S2/S6 signals, and the function is NOT yet on `inline_asm_canonical.txt`. Show the user the evidence, request authorization.
2. **Pre-existing build mismatch** (per §1). The repo was already broken before your work; surface immediately.
3. **Catastrophic environment** — WSL down, splat broken, disc image missing.

Anything else is "switch technique." See §0.

---

## §10. END OF SKILL — RETURN VALUE

When the commit lands and the function drops off the queue, summarize in 3-5 lines:
- Function name + cheat type that was retired
- Which §5 technique closed the gap
- Net diff: `regfix.txt` lines removed, `src/*.c` lines added
- Confirmation: `bb2 matches!` + `verify --all` clean
- Next: invoke `decomp-cheat-cleanup` again for the next orphan, or stop if user said "do one."
