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

**If you genuinely believe the cheat is canonical asm**, the path is:
1. Run `dc.sh scan-hand-coded --func <func>` to get the hand-coded-asm tier
2. **Autonomous canonical-asm authorization (per user authorization 2026-05-17):** You may self-authorize when ALL of these hold:
   - Hand-coded scanner returns `STRONG` (S1/S2/S6 GCC-impossible signal fired), OR
   - The function has been documented as intractable across multiple sessions (e.g., a `feedback_*_intractable.md` memory file exists with ≥10 documented C-source variants all plateauing well above 0 diffs), AND
   - You have read the target.s yourself and confirmed the divergence pattern is not a known C-decomposable pattern, AND
   - The §5 coercion ladder has been genuinely exhausted (not just "I'm tired")
3. **When in doubt, don't self-authorize.** The user's standing rule: *"we have to confirm with extreme confidence that the source was asm not C before using inline asm in any circumstance."* "Extreme confidence" means the evidence is unambiguous — a single STRONG signal alone is not sufficient unless the function's cluster context corroborates.
4. If authorizing: replace the C body with the canonical retirement form (see §5.7), add to `inline_asm_canonical.txt` with one-line evidence, build, verify, commit. Document the authorization in a memory file.

If the function is on the cheat queue and NOT on the strong-signal hand-coded list AND has fewer than 5 documented intractability attempts, **it's C** — keep pushing the §5 coercion ladder.

**Asking the user to release / accept partial / accept the cheat as canonical is forbidden while §5 avenues remain unexplored.** Self-authorizing for routine difficulty (no STRONG signal, no documented intractability) is also forbidden.

---

## §0.1. SOTN-GRADE EVIDENCE GATES (read before §1; affects what you commit)

User directive 2026-05-17: **"Everything should be to the SOTN standard unless there is a very compelling evidence-driven reason to break convention. If it can be C, it should be C."**

`audit_asm_cheats.py --check-new` (Rule 1a of the commit hook) enforces these programmatic gates. If your commit hits any of them, the commit blocks. **Plan your work to satisfy them up-front; don't try to commit and discover them after the fact.**

| Gate | Triggered by | What you must provide |
|---|---|---|
| **G1 same-commit self-auth** | function newly added to `inline_asm_canonical.txt` AND newly has inline asm in same commit | Split into 2 commits: (1) canonical entry + tag + attempt log, then (2) AFTER the first lands, the inline asm + Pure-C attempts log |
| **G2 evidence tag** | any new entry in `inline_asm_canonical.txt` | A valid tag in the `# justification` comment: `gcc-cannot-emit:<reason>`, `custom-abi:<descriptor>`, `hand-coded-signal:STRONG/Sn+Sm`, `cluster-sibling:<func>,jaccard=N.NN`, or `bios-trampoline` |
| **G3 cheat delta** | new inline `__asm__(glabel)`, new lost-codegen insert, new c-body multi-insn, etc. | Either retire the cheat (preferred) or provide evidence tag + attempt log |
| **G4 aliasing-barrier doc** | new `__asm__ volatile("move %0, %1" ...)` instance | `INLINE_MOVE_ALIASING:` comment within 8 lines above, listing ≥2 `- technique=NAME: <specific failure>` bullets |
| **G5 regfix accretion** | new regfix rule for a function with ZERO rules at HEAD | Pure-C attempts log (≥3) showing the C-side fix was tried and failed |
| **G6 attempt log** | any of G2-G5 | `Pure-C attempts:` block in commit message with ≥3 entries: `[N] technique=<name> score=<N> outcome=<observable evidence>`. Vague outcomes (`didn't work`, `no improvement`) are rejected. |

The LLM auditor (Rule 1b) runs after the programmatic gate and judges EVIDENCE QUALITY: it checks that outcomes reference observable state (dump-text register names, permuter score plateaus, regalloc-dump lines, specific GCC option behavior), that listed techniques are plausible for the diff, and that the escalation order makes sense. Generic plausible-sounding evidence is REJECTED.

**Translation for daily work:** if you find yourself reaching for `__asm__ volatile("move %0, %1")` or considering an authorization, you owe the commit message a structured account of what you tried, with specific observable outcomes. The format is enforced by the gate; the quality is enforced by the auditor.

See `feedback_evidence_driven_authorization.md` for the canonical reference on tag grammar, attempt-log format, and the aliasing-barrier comment template.

---

## §1. PRE-FLIGHT

Run this first (use your worktree path if you're in one — the SessionStart hook may have briefed the main repo, not your worktree):

```
wsl bash -c 'cd <your-worktree-or-repo-path> && bash tools/dc.sh start'
```

Branch on the briefing:

| Briefing line | Action |
|---|---|
| `Build: OK` + `Active: NONE` | Proceed to §2 |
| `Build: OK` + `Active: <func>` | Skip §2 — resume `<func>` from current state (it was a prior cheat cleanup); go to §3 |
| `Build: not built yet` | **Normal in a fresh worktree.** Proceed to §2. Do NOT run `dc.sh build` to "check" — your first real build happens in §4/§5 and will surface anything wrong. |
| `Build: MISMATCH` | STOP. Report repo state. Hook can't help; baseline issue requires user investigation. |
| `Queue: <N> days old` (N > 7) | Run `dc.sh refresh-queue` before §2 |

**The ONLY build-status line that stops you is the literal `Build: MISMATCH`** — which means the briefing actually ran a build and got SHA1 inequality. A linker error you discover by *manually* running `dc.sh build` during pre-flight is NOT in this category — that's a partial / pre-existing build infrastructure issue you should not be chasing here. Pre-flight is for reading the briefing, not auditing the repo.

**DO NOT run `dc.sh build`, `dc.sh verify --all`, or any "let me just confirm the baseline" command during pre-flight.** The queue's existence IS the baseline confirmation — every entry in the cheat queue was produced from a SHA1-matching build at refresh time. The first real build in this skill happens in §4 (after you have an active function and know what symptoms to expect). If it fails there for reasons unrelated to your cheat, THAT is when you investigate or surface — not pre-emptively.

**Fresh-worktree note.** If you just entered a new worktree (no `build/` dir, no `.venv/`), the briefing will say `Build: not built yet`. This is fine. The shared venv lives at the main repo path and is invoked through `tools/dc.sh` which handles activation. You don't need to set up anything; just pull the cheat.

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

**This is where the first full build happens** (if pre-flight said `Build: not built yet`). Take a snapshot of "current matching state" before iterating — every change must either preserve match or break it in a controlled way you understand.

```
# Confirm current build matches
bash tools/dc.sh verify <func>          # expect: MATCH (since it's a cheat orphan)

# Optional: save the matched asm bytes for sibling comparison
mipsel-linux-gnu-objdump -d build/bb2.elf | awk '/<<func>>:/,/^$/' > /tmp/<func>.matched.s
```

**Interpreting the baseline build outcome:**

- **`bb2 matches!` + per-func verify MATCH** → expected. Proceed.
- **Per-func verify NO MATCH but `bb2 matches!`** → impossible (would mean SHA1 collision); re-run.
- **Linker errors referencing src symbols (e.g. `undefined reference to debug_printf` from `.data` section)** → pre-existing build infrastructure issue, NOT caused by anything you've done. Surface to user *briefly* with the exact error and stop. Do not try to fix the link errors as part of this skill — that's outside cheat-cleanup scope and likely needs a different worktree / repo state.
- **SHA1 mismatch on bb2 (build succeeds but bytes differ)** → another agent's WIP is in the tree, or a regfix rule is stale. Run `dc.sh verify --all 2>&1 | grep -v MATCH | head -20` to see which functions diverge. If it's a *different* function from your cheat target, you've inherited unfinished work — surface and stop. If it's *your* cheat target, the cheat rule itself may have rotted (rare); inspect the rule and proceed to §5.

**Identify what GCC currently emits.** Run the build pipeline through maspsx for the function and dump-text it:

```
bash tools/dc.sh dump-text <func>       # post-maspsx, pre-regfix instruction stream
```

Compare this to target.s and to the regfix rules. The regfix rules transform `dump-text` output into target. The cheat rule fills a specific gap. You need to know that gap.

---

## §5. THE COERCION LADDER

Try in this order. Each rung requires SHA1 to either stay matching or break in a way that gets you closer to a clean removal. **Revert immediately on regression.**

### 5.-1 ALWAYS TRY FIRST: remove ALL the function's regfix rules (free if stale)

**Per `feedback_cheat_cleanup_techniques.md` §0** — on the 2026-05-16 session, 22 of 28 cheat orphans (79%) had ALL their regfix rules stale. The C source had been refactored over time; rules were no-ops but still in regfix.txt. Deleting them was free.

```bash
sed -i "/^<func>:/d" regfix.txt
rm -f build/src/<file>.o
bash tools/dc.sh build && bash tools/dc.sh verify <func>
```

If `bb2 matches!` AND per-function verify clean → commit the deletion. Done.

If broken → restore (`cp /tmp/regfix.bak regfix.txt`) and proceed to §5.0.

**Caveat:** doesn't apply to `c_body_multi_insn` cheats (cheat is in src/, not regfix). Doesn't apply to file-scope `__asm__(glabel)` cheats (different category). For those, jump to §5.1+.

### 5.0 Remove just the cheat rule (controlled break — diagnostic only)

Edit `regfix.txt` / `asmfix.txt` / src/*.c to remove the cheat. Build:

```
bash tools/dc.sh build
```

Expected: build fails with N-byte size diff, where N matches the cheat's contribution (typically 4 bytes per single-instruction insert). If the diff is much bigger (>20 bytes), the cheat was masking a larger structural mismatch — pause and re-read §3.

**This first failure tells you the size of the gap you need to fill in C.** A 4-byte gap = 1 MIPS instruction. A 12-byte gap = 3 instructions. A larger gap means multiple cheats or a structural mismatch.

### 5.1 inline_move_aliasing trick (the #1 fix for lost_codegen)

For `addu RD, RS, $zero` (a redundant register copy that GCC's optimizer ate), use the single-instruction inline asm pattern documented in `feedback_inline_move_aliasing.md`. **G4 (per §0.1) requires an attached `INLINE_MOVE_ALIASING:` comment with ≥2 documented failed-alternative bullets.** Required form:

```c
/* INLINE_MOVE_ALIASING: pure-C alternatives failed.
 *   - technique=plain_copy: GCC's CSE collapsed `dst = src;` per regalloc dump
 *   - technique=volatile_copy: `volatile T dst = src;` introduced sw+lw, wrong shape
 *   - technique=pin_to_reg: `register T x asm("$RD")` pin ignored per dump-text
 * Per feedback_inline_move_aliasing.md, single-insn escape valve.
 */
register T dst asm("$RD");
register T src asm("$RS");
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));
```

The bullets are NOT optional and must reference observable state — vague bullets fail the LLM auditor (Rule 1b). At commit time, the same techniques and outcomes must also appear in the commit message's `Pure-C attempts:` block (G6).

This emits exactly one `addu RD, RS, $zero` instruction.

**The constraint detail is load-bearing — read [[feedback-inline-asm-lost-codegen-injection]] before reaching for this.** The template must use `%N` operand placeholders bound via `=r` / `r` constraints to `register T x asm("$N")` C variables. GCC then chooses the registers (steered by the pins) and tracks the operation. **If you write `__asm__ volatile("addu $8, $3, $zero")` with hardcoded `$N` registers and no `%N` placeholders, that is the lost_codegen regfix cheat re-spelled in C — same bytes, no GCC tracking — and `audit_asm_cheats.py --check-new` will block your commit (the `asm_injection` cheat type, added 2026-05-17 after two sessions of this skill hit the trap 4 hours apart: `c5e11ae` kept-and-flagged, `9118925` reverted).** Single-insn `__asm__` is allowed for codegen *control*, not for byte-for-byte injection.

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

### 5.7 Canonical-asm retirement (after §0 authorization gate is met)

If you've cleared the §0 autonomous-authorization gate, this is the retirement form. **Default for most files: inline `__asm__` block.** **Exception for `code6cac.c`: the cheat-supported C body has the structured-loop bug** ([[project_build_and_internals]] §code6cac) — inline `__asm__` blocks cause cc1 to silently drop later functions. Use the bridge form for code6cac instead.

**Inline `__asm__` block form** (most files — works for text1b.c, sound.c, etc.):

```c
__asm__(
    ".set noat\n"
    ".set noreorder\n"
    "glabel <func_name>\n"
    "    <insn>\n"
    "    <insn>\n"
    ...
);
```

Source the asm content from `asm/funcs/<func>.s`, stripping the `/* HEX HEX BYTES */` comments. The inline block goes at file scope, replacing the C function body entirely.

**Bridge form** (code6cac.c, or when the function emits rodata jtbls that cc1 generated from a switch):

1. **Stub the C body:** `void <func>(s32 arg0) { (void)arg0; }`
2. **Add asmfix bridge:** `<func>: replace_with_asmfile "asm/funcs/<func>.s"` in `asmfix.txt`
3. **Handle rodata jtbls if present.** If the original function had a `switch` that cc1 compiled to a jumptable, the jtbl bytes were in `code6cac.o(.rodata)`. Removing the switch removes them. Create a new asm rodata file:

```
# asm/data/<addr>.rodata_<func>_jtbls.s
.include "macro.inc"
.section .rodata, "a"

.align 2
nonmatching jtbl_<addr>

dlabel jtbl_<addr>
    /* offset addr */ .word .L<target_addr>
    ...
.align 2
enddlabel jtbl_<addr>
```

   Extract jtbl entry addresses from the disc directly:
   ```python
   with open('disc/SLUS_006.63', 'rb') as f:
       f.seek(jtbl_file_offset)  # = jtbl_addr - 0x80010000 + 0x800
       data = f.read(num_entries * 4)
   # struct.unpack each 4-byte little-endian word
   ```

4. **Insert the new rodata file into `bb2.ld`** at the slot where the original jtbls landed (e.g., BEFORE `code6cac.o(.rodata)` if the jtbls were at the start of that section).

5. **Strip cheats:** `sed -i '/^<func>:/d' regfix.txt regfix_stage2.txt asmfix.txt` (excluding your new replace_with_asmfile line — re-add it after).

6. **Build + verify.** A few sibling regfix rules may break due to `.L<N>` label drift (file-wide GCC numbering shifts when you remove a function's labels). Auto-fix via `dc.sh fix-label-drift` or manually update the rules to use `\.L\d+` regex + the current target labels (read `dc.sh dump-text <sibling_func>` to find them).

7. **Add to `inline_asm_canonical.txt`** with one-line evidence note.

**Reference example:** `single_game_VoiceContorol` (commit c0375a5, 2026-05-17) — code6cac.c bridge form with separate `asm/data/10068.rodata_voice_jtbls.s` for the two switch jumptables.

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

**Commit-message template (required structure):**

```
cheat-cleanup: <func> — <one-line summary>

Removed cheat: <which rule(s) you removed, with regfix.txt line range>
Coercion: <which §5 technique you used>

Pure-C attempts:
  [1] technique=<name> score=<N> outcome=<observable evidence>
  [2] technique=<name> score=<N> outcome=<observable evidence>
  [3] technique=<name> score=<N> outcome=<observable evidence>

Build: bb2 matches! (verified --all)
```

The `Pure-C attempts:` block is **mandatory** for any commit that:
- adds inline asm (any form, including `move %0, %1` barriers)
- adds an entry to `inline_asm_canonical.txt`
- adds a new regfix rule for a function with no rules at HEAD
(see §0.1 G6)

**Outcomes must reference observable state.** Specific examples:
- `dump-text shows pin to $v1 ignored; allocator chose $a0 instead`
- `permuter plateau at score=14 after 600s, no structural variant found`
- `decl reorder: same score, scheduling pattern unchanged per /tmp/dump`
- `CSE collapsed s32 v_copy = v; per regalloc dump line 487`

**Outcomes that will be REJECTED by the LLM auditor:**
- `didn't work`
- `no improvement`
- `tried everything`
- `GCC didn't cooperate`
- `same score` (without context)

If your cheat retirement is purely "remove stale regfix rules" with no new inline asm, no new aliasing barrier, and no canonical entry, the attempt-log block is NOT required (the gates don't fire). A simple `cheat-cleanup: <func> — strip N stale regfix rules` commit message is fine.

The active_func_guard hook will (1) run audit_asm_cheats.py --check-new with the commit message, then (2) run the LLM auditor, then (3) verify `dc.sh verify <func>` returns MATCH. All three must pass.

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
- **Don't write `__asm__ volatile("addu $X, $Y, $0/$zero")` with hardcoded `$N` registers and no `%N` placeholders** to replace a lost_codegen regfix cheat. That is the cheat under a different name — same MIPS bytes, no GCC tracking, just moved from regfix.txt into the C source. The audit detects this as `asm_injection` and blocks at commit (added 2026-05-17). The §5.1 trick is the **placeholder-bound** form with `=r`/`r` constraints; see [[feedback-inline-asm-lost-codegen-injection]] for the trap and [[feedback-inline-move-aliasing]] for the legitimate pattern.
- **Don't introduce multi-instruction inline `__asm__` to fix a lost_codegen cheat.** That's c_body_asm_debt — a different cheat. The §5.1 trick uses SINGLE-instruction `__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src))` with operand placeholders, NOT hardcoded `$N` literals.
- **Don't authorize the function as canonical asm unless §0's autonomous-authorization gate is met** (STRONG signal OR documented intractability + target.s confirmation + exhausted §5 ladder). Per `feedback_bridge_is_not_decomp.md`, `feedback_hand_coded_asm_recognition.md`, and `feedback_canonical_asm_retirement.md`.
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
