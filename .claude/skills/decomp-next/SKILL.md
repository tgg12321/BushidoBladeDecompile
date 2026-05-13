---
name: decomp-next
description: End-to-end matching decompilation of one function from WORK_QUEUE.md. Invoke whenever the user asks to "decomp the next function", "do the next one", "match the next function", "work the next N functions", "run through the queue", "do N more", or similar BB2-decomp phrasings. ONE FUNCTION PER INVOCATION — for multi-function batches, re-invoke this skill after each successful commit so the guardrails reload fresh. Mandatory §2.5 pre-work (memory-check + explicit C-vs-asm determination) BEFORE pipeline launch, so prior memory/gotchas surface early and hand-coded-asm functions aren't pure-C-attacked for hours. Programmatic pipeline (attempt → diff-align → recipes → near-miss → regfix-suggest → permute-adaptive); escalates to manual technique only when automation is exhausted. Hard guardrails against inline-asm / asmfix-bridge cheats; rare canonically-asm functions go through inline_asm_canonical.txt with per-function user authorization. Resilient — runs until 100% pure C + byte-matching commit, never quits for fatigue or iteration count.
---

# /decomp-next — single-invocation, end-to-end function match

You were invoked to take the next function in the queue and decompile it to **100% pure C that builds byte-identical to the original**. One invocation, one matched function (or one already-matched session you resumed), committed.

Read this entire skill before doing anything. The rules below are the contract.

---

## §0. THE CONTRACT (re-read whenever tempted to deviate)

**You stop only when ONE of these is true:**

1. `dc.sh verify-c <func>` reports MATCH **and** the function body in `src/` contains zero non-blessed inline asm **and** the commit landed (hook auto-cleared `.bb2_active_func`).
2. `dc.sh start` reports `Build: MISMATCH` **before you've done any work** — this is a pre-existing repo problem, not a function problem; report and stop.
3. Catastrophic external state — WSL is down, disc image missing, splat refuses to run. Genuine impossibility, not difficulty.

**Every other reason to stop is forbidden.** Specifically NOT stop conditions:

- Score plateau at any level
- N attempts / N hours / N regfix rules
- "Diminishing returns" / "good place to stop" / "next session can continue"
- "Structurally hard" / "register allocation can't be coerced" / "exceeds the tool's tractable bound"
- "Fundamental wall" / "I've hit a wall" / "GCC's heuristic is sticky" — these are descriptions of *your* plateau, not the language's reachability
- "I've exhausted the toolbox" — you haven't. Build a new tool.
- "The function uses inline asm naturally" — almost always wrong; this codebase was canonically C. Even GTE/BIOS wrappers are pure-C-with-narrow-asm-primitives, not inline-asm blocks.

**THE PHYSICS CHECK (mandatory whenever you feel walled).** Before any "fundamental wall" / "can't find the C structure" / "should we stop" framing, run this check on yourself:

> **"Is this code canonically C?"**

If yes — and it almost always is for this codebase (Lightweight, GCC 2.7.2, PsyQ SDK 3.5, 1998 commercial PS1 game) — then a pure-C body that emits matching bytes **exists**. The toolchain is deterministic; the answer space is finite; you just haven't found the right structure yet.

This question is answered EXPLICITLY in §2.5.b before §3 launches — not implicitly at the wall point. By the time you reach this check, you've already classified the function as C-canonical or asm-canonical with evidence; if you're walled on a C-canonical function, the answer is "keep trying," and if the function is on `inline_asm_canonical.txt` you wouldn't be in pure-C deepening in the first place. Re-reaching for "maybe this IS asm" after §3 has been running is rarely the right move — it's almost always a signal to switch technique within the C path.

How to tell if a function is canonically C (this is the §2.5.b checklist condensed):

- The `/* handwritten instruction */` markers in `asm/funcs/*.s` only flag GTE cop2 ops (ctc2/mtc2/mfc2/swc2/lwc2/cfc2/cop2/mvmva). Those legitimately come from inline asm *inside* C source. Their presence is NOT evidence of hand-written-asm function.
- Callers passing normal arguments (`func(a, b, c)`) → callee is a regular C function.
- Stack frame with standard prologue/epilogue, saved $ra, callee-saved $s regs → C function.
- Function is in a `.c` file in `src/` and has been getting compiled (even with parse errors) → meant to be C.
- Sibling functions in the same file are already matched in C with similar shape → this one matches in C too.

Vanishingly few functions in this codebase are canonically asm. They are gated out by the classifier (`bios_or_syscall:*`, `permanently_blocked:*`) or already authorized in `inline_asm_canonical.txt`. If you got the function from a queue pull and `dc.sh memory-check` shows `CANONICAL_ASM: no`, **it's C** — keep pushing the pure-C ladder.

When you think you're walled, you are wrong about *yourself*, not the function. The avenues you haven't tried always exist; surface them and try them.

**Avenues that exist when you feel stuck** (this is not exhaustive, build new ones as needed):

- The permuter (`dc.sh permute-adaptive`, `dc.sh permute --max-time 3600`) — randomized C-structural search. The right tool when "GCC's heuristic is stuck on one choice and you can't see what perturbs it." Manual iteration on register allocation is the wrong tool there.
- Fix tooling that's blocking the permuter for your function (e.g., GTE-mnemonic preprocessing of target.s).
- Sibling matched functions in the same file — grep for the codegen pattern you need (e.g., `jal.*\n.*sh `, register pin shapes, scheduling tricks) and copy the C structure.
- Other PS1 decomps using the same compiler (SOTN, Vagrant Story, FF7, etc. — see `reference_decomp_projects.md`) — same GCC 2.7.2 + PsyQ, same idioms.
- Build a new transformation pass / regfix op / named recipe.
- Different optimization flag (`-fno-strength-reduce`, etc. — see `NO_SR_FILES` in Makefile for the per-file mechanism).
- Read other memory files for matching patterns (`feedback_matching_playbook.md`, `feedback_regfix_reference.md`, `feedback_70C70_lessons.md`).
- **Memory recipes you read EARLIER but dismissed as "not applicable" — re-evaluate.** When the cpu_check_tubazeri match was stuck at 32 diffs, the agent read `feedback_quick_reference.md:391` ("`lui+ori+sw 0(reg) → lui+sw offset($at)` | 2 substs + 1 delete") and dismissed it as "not my pattern." The eventual fix was structurally identical (delete + reorder + 2 substs). When you've plateaued, go back to recipes you skimmed and TRY them — your initial "doesn't apply" judgment was made before you understood the gap.

The list is open-ended on purpose: if you've tried every item above and the function still doesn't match, the next item is *invent the technique this function needs and write it down in a new feedback file*.

**Asking the user to release / accept partial / accept eval-mode-as-stop-condition is forbidden while avenues remain unexplored.** The user has explicitly directed: never quit while you can still try something. The "let's evaluate at the end" framing is a TRAP that previous agents fell into — they used it to declare premature stopping. Don't.

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
| `Build: OK` + `Active: <func>` | Skip §2 — resume `<func>` from current state, go to §3 |
| `Build: MISMATCH` | STOP. Report repo state. The hook can't help; this is a baseline issue requiring user investigation. |
| `Queue: <N> days old` (N > 7) | Run `dc.sh refresh-queue` before §2 |

---

## §2. PULL THE NEXT FUNCTION

```
bash tools/dc.sh next --with-context
```

This atomically:
- Sets `.bb2_active_func` to the next queue entry
- Auto-runs `dc.sh agent-brief <func>` (classification + asm + neighbor functions + kengo reference + existing regfix/asmfix rules)

If the **active decomp queue is empty** (`No entries found in active decomp queue`), fall through to the retirement queue:

```
bash tools/dc.sh next-asmfix
bash tools/dc.sh agent-brief <func>
```

If **both queues are empty**: report "all decomp work complete" and stop — there's nothing to do.

Read the brief carefully. The very top of the brief now shows a **`Function state`** line — `BRIDGED` / `RETIRING` / `NORMAL`. This decides whether you need `dc.sh retire` (BRIDGED) or which verify command to use (RETIRING → `verify-c`, NORMAL → `verify`).

Then the `recommendation` field tells you the function class:

| recommendation | Path |
|---|---|
| `easy_attempt` / `standard` | §3 normal pipeline |
| `needs_lwl_fix` | §3 pipeline + add file to `FIX_LWL_FILES` in Makefile (see `feedback_retirement_recipes.md`) |
| `gte_function` | **§3.0 GTE pre-route FIRST** (below), then §3 — `attempt` crashes on `mvmva`/raw GTE mnemonics until target.s is preprocessed. |
| `aliasing_heavy` (tag) | Use `dc.sh diff-align` early; expect asymmetric reload patterns |
| `needs_function_split` / `needs_rodata_split` / `needs_delay_slot_ra` | Structural; specific tooling exists — see `feedback_workflow_rules.md` |
| `permanently_blocked:*` / `bios_or_syscall:*` | Should never appear (queue filters them). If it does, something's wrong — investigate before continuing |

For **retirement queue items** (or any function whose `Function state` is BRIDGED):

1. Run `dc.sh retire <func>` — comments out the bridge with `# RETIRE: ` so the C body becomes the source of truth.
2. **Check the stub signature against canonical**: read `tmp/bridge_signature_audit.json` (refresh with `bash tools/dc.sh audit-bridges` if stale). 41/209 bridged stubs have a wrong arity/type. If the stub disagrees with `caller_max_arity` from the audit, fix the decl in src/ to match canonical BEFORE iterating — wrong stubs cause cascade-regressions through callers (the §6.4 caller-audit case, but for *retirement* you can catch it up front).
3. Re-run `dc.sh agent-brief <func>` so `Function state` shows RETIRING (not BRIDGED) and `gen_regfix` runs against the real C body, not the bridge.

---

## §2.5. PRE-WORK INVESTIGATION (mandatory before §3)

Two cheap, high-leverage checks before any decomp work. They catch the two failure modes that wasted the most time historically: agents re-deriving documented gotchas, and agents pure-C-attacking functions whose original source was hand-coded asm.

### 2.5.a Memory & gotcha lookup — `dc.sh memory-check <func>`

```
bash tools/dc.sh memory-check <func>
```

Surfaces:
- Whether the function is in `inline_asm_canonical.txt` (authorized hand-coded-asm list)
- Memory files mentioning the function by name (with first context line)
- Memory files mentioning the function's neighbors in the same .c file (cluster context)

**Read every memory file the tool surfaces before launching §3.** A prior incident, named recipe, register-cycle pattern, or cluster-wide gotcha is documented in there — re-deriving it from scratch is the failure mode this step exists to prevent.

If `memory-check` reports `CANONICAL_ASM: yes`: the function is already authorized as hand-coded asm. **Skip §3. Go to §2.5.c.** The programmatic pipeline is tuned for pure-C matching and will not converge on a hand-coded byte sequence.

### 2.5.b Canonical form determination (C vs asm) — explicit, mandatory

Before §3, decide explicitly: **is this function canonically C or canonically asm?**

The default — and the right answer for ~99% of queue items — is C. THE PHYSICS CHECK (§0) applies, the §3 pipeline + §5 ladder will reach 0 diffs, you just have to keep trying. **Do not abuse this step to declare "asm-canonical" as an exit from a hard pure-C function.** The user's standing rule: *"we have to confirm with extreme confidence that the source was asm not C before using inline asm in any circumstance."*

**`dc.sh memory-check` auto-runs the data-driven hand-coded-asm scan** (`tools/scan_hand_coded.py`) and reports one of four tiers. Trust the tier as your primary input:

| Tier | Meaning | Action |
|---|---|---|
| `STRONG` | S1 (uniform `multu` pacing) and/or S2 (empty-body branch) fired. Both are effectively impossible in GCC output. | Treat as hand-coded asm. If on `inline_asm_canonical.txt`: §2.5.c. If not: confirm signals by reading target.s, then surface evidence to user and request authorization. **Do not self-authorize.** |
| `POSSIBLE` | One GCC-impossible signal fired without tightness backing. | Read the target.s; could be hand-coded or an edge case (small GTE wrapper). Lean toward §3 unless additional review reveals more evidence. |
| `TIGHT_C` | Tightness signals (S3/S4/S5) but no GCC-impossible signal. | **Pure-C cluster, NOT hand-coded.** Do not use the asm-canonical path. Proceed to §3 and prioritize the §3.0.a sibling-template lookup. |
| `LOW` | Nothing suggests hand-coded. | Default: C-canonical. Proceed to §3. |

**The five signals** (computed automatically by `scan_hand_coded.py`):

1. **S1 — Uniform `multu`/`mflo` pacing.** Every multu/mflo pair separated by exactly 2 cycles, across ≥2 pairs. **GCC-impossible.** GCC's scheduler tightens to 0-cycle gap when useful work exists.
2. **S2 — Empty-body branch.** `bgez/bltz/.../bne` whose target label is the instruction immediately after the delay slot — both paths converge with no body. **GCC-impossible.** GCC elides empty `if` bodies.
3. **S3 — No callee-save spills in 40+ insn function.** GCC spills under any meaningful pressure of that size; absence indicates hand-allocated. Tightness signal — also seen in tight pure-C.
4. **S4 — Load burst.** ≥4 loads from non-$sp base within any 8-insn window. Tightness signal.
5. **S5 — Cluster behavior.** Approximate-match opcode-sequence k-mer similarity (Jaccard ≥0.5) with ≥1 sibling. Tightness signal — also seen in C function families (GTE 3x3, calc_fc_frame).

**S1 and S2 are the decisive discriminators.** S3/S4/S5 are tightness signals; they fire on tight pure-C clusters too. A 5/5 score with S1 and S2 is essentially certain. A 3/5 score from only S3/S4/S5 is a tight-C cluster.

**For the entire project landscape**: `bash tools/dc.sh scan-hand-coded --all` lists every function in each tier. As of the initial scan: 5 STRONG (the sin/cos rotation cluster), 0 POSSIBLE, 9 TIGHT_C (GTE 3x3 + calc_fc_frame + others), ~1305 LOW. The asm-canonical population is small and bounded.

Weak signals that are NOT enough on their own (require corroborating strong signals):
- A documented cc1 register-allocator divergence (could be cc1 fault, could be hand-coded — don't conclude from this alone)
- Pure-C plateau after N attempts (you haven't exhausted the §5 ladder)
- Kengo source has a function with this name (Kengo is the PS2 sequel; may have been rewritten)
- "The function feels hard"

### 2.5.c Inline-asm canonical path (only when §2.5.b authorizes)

When the function is in `inline_asm_canonical.txt` OR the user has just authorized this specific function:

1. **Replace the C stub with file-scope `__asm__`.** Pattern (display.c has reference implementations: `func_8007F87C`, `func_8007FA1C`):

   ```c
   /* <func>: hand-coded asm in original PSY-Q source.
    * Evidence: <list the strong signals you confirmed>.
    * Authorization: <user message ref / commit ref>.
    * See memory/feedback_hand_coded_asm_recognition.md. */
   __asm__(
       ".section .text\n"
       ".set\tnoreorder\n"   /* TAB, not space — see below */
       ".set\tnoat\n"
       "glabel <func>\n"
       "    <target.s body verbatim, decimal offsets>\n"
       "    jr         $ra\n"
       "     nop\n"
       "endlabel <func>\n"
       ".set\treorder\n"
       ".set\tat\n"
   );
   ```

2. **Maspsx gotchas (the two that bit prior matches in this cluster):**

   - **TAB after `.set`, not space.** Maspsx's `is_reorder` tracking requires `.set\tnoreorder` (literal `\t` in the C string). Space-separated `.set noreorder` slips through to gas but maspsx still injects `nop  # DEBUG: branch/jump` after every branch/jump in the asm — silent +N-instruction inflation that diff-aligns to an off-by-N mismatch.
   - **Decimal offsets only.** `lh $t2, 0x6($a1)` crashes maspsx at `int(operand)` base-10 parsing. Convert all hex offsets to decimal (`lh $t2, 6($a1)`). The immediates inside arithmetic ops (e.g., `andi $t9, $t7, 0xFFF`) parse fine — only the `N($reg)` offset form is affected.

3. **Skip §3 and §5.** Both are pure-C pipelines.

4. **Build with `dc.sh build-active <func>`.** A byte-exact inline asm should match on first build. If it doesn't, your asm doesn't match target.s — diff and fix the asm, not your decomp approach.

5. **After MATCH: add the function to `inline_asm_canonical.txt`** with a one-line justification:

   ```
   <func>  # <one-line summary of evidence>. User-authorized <date>. See <memory_file> and commit <sha>.
   ```

   This prevents the inline-asm-debt scanner from re-listing the function as active work.

6. **Continue to §6 (anti-cheat gate — honors the canonical list) and §7 (commit).**

This path is a **recognized exception** to §6.1's multi-instruction-asm rule, scoped to functions on `inline_asm_canonical.txt`. It is NOT a general escape hatch from hard pure-C functions.

---

## §3. PROGRAMMATIC PIPELINE (mandatory first pass)

**Precondition: §2.5 must have run.** If `memory-check` returned `CANONICAL_ASM: yes` or you confirmed ≥3 hand-coded signals in §2.5.b with user authorization, you are NOT in §3 — you are in §2.5.c. The §3 pipeline is for pure-C matching only; running it on a hand-coded function burns hours converging on nothing.

**You do not write C by hand before this pipeline runs.** Trust the automation; it's been tuned over hundreds of matches.

**The `agent-brief` you got in §2 is authoritative — re-scroll your context for it before re-grepping or re-reading.** It already contains: classification, full target.s, m2c base.c, neighbor functions in the same source file, kengo reference, existing regfix/asmfix rules, recipe suggestions. Reaching for `Read src/<file>.c` to inspect the surrounding function preamble or `Grep` for sibling templates is almost always wasted — that material was in the brief. Re-running `dc.sh agent-brief <func>` is also fine and cheap if your context has scrolled.

### 3.0 GTE pre-route (only if classification = `gte_function`)

`dc.sh attempt` crashes at the setup stage for GTE functions because `mipsel-linux-gnu-as` can't assemble raw `mvmva`/`ctc2`/`mtc2`/`mfc2`/`cop2` mnemonics — target.s is left with an empty .text section. Skip §3.1 and start here instead:

**3.0.a — First, look for an existing sibling template.** The codebase has a uniform GTE-wrapper style; replicating it is faster than re-deriving from m2c. Grep the same source file for the pattern:

```
grep -nE '^\s*register s32 t0 asm\("\$8"\)' src/<file>.c
```

In `src/text1b.c` alone there are ~9 already-matched GTE wrappers (`game_2d_CheckLifeGaugeNoDisp`, `func_80052A88`, `func_80052B00`, `func_80052B44`, …) sharing the same shape:

- 8 register-pin declarations: `register s32 t0 asm("$8"); ...; register s32 t7 asm("$15");`
- Load N matrix words into t0..tN
- Sequence of `__asm__ volatile ("ctc2 %0, $K" :: "r"(tK))` — one per ctc2/mtc2
- Mvmva via `__asm__ volatile (".word 0x4A480012")` (or use `gte_mvmva()` macro)
- Result reads via `__asm__ volatile ("mfc2 %0, $N" : "=r"(tK))` or `swc2`

Copy that shape, swap in your function's specific loads/packs/stores. If the matched function uses extra registers ($t9 mask, $v0/$v1 temp packing), add `register` pins for those too (`asm("$25")`, `asm("$2")`, `asm("$3")`).

**3.0.b — Fall back to gte-migrate scaffold only if no sibling template fits.**

1. **`dc.sh gte-migrate <func> --setup`** — emits a starter base.c using `gte_*()` macros from `include/gte.h`. Modern versions handle ctc2/mtc2/mfc2/mvmva via macros (`handled=N, leftover=0` line at top). The remaining TODOs (`/* TODO: var holding tN */`) are data-flow questions — read the asm to see which arg/word fills each register.
2. **Check `include/gte.h` for the macros you need.** Current coverage: gte_rtps/rtpt/nclip/avsz3/avsz4/dpcs/dpct/intpl/sqr/ncs/nct/ncds/ncdt/nccs/ncct/cdp/cc/dpcl/mvmva, plus generic ctc2/mtc2/cfc2/mfc2. If a needed op isn't there, add it (one-line `.word 0x<encoding>` macro) — that's expected toolbox extension.
3. **Replace TODO scaffolding with real C + macro calls.** Each placeholder becomes the natural C value (`mat[N]`, `vec[N]`, packed combination, etc.) that the source code would have written.
4. **Then** run §3.1 `dc.sh attempt <func>` — target.s now assembles, scoring works, and the rest of the pipeline applies normally.

See `feedback_gte_3x3_recipe.md` and `feedback_gte_wrapper_gotchas.md` for the recurring patterns.

### 3.1 First attempt

```
bash tools/dc.sh attempt <func>
```

Runs: classify → setup → smart_match (16 transformation strategies) → permute_capped → gen_regfix.

Verdicts:

- **MATCHED** → `auto_matches/<func>.c` has the matched body. Go to §6.
- **NEAR_MISS** (score ≤ 200) → §4 programmatic deepening
- **HARD** (score > 200) → §4 (still programmatic — do NOT jump to manual)
- **SKIPPED** → should not occur for queue items. Investigate the classifier output.

### 3.2 Inline-asm setup (if the function is currently bridged or has an inline `__asm__` block in src/)

```
bash tools/dc.sh inline-setup <func>
```

This stages `permuter/<func>/` with m2c base.c + target.s. Then re-run §3.1 attempt.

---

## §4. PROGRAMMATIC DEEPENING (still no hand-written C)

Run these tools in order. After **each** tool: rebuild (`make`), re-check (`dc.sh verify-c <func>`). On MATCH → §6. On still-mismatch → next tool.

### 4.1 `dc.sh diff-align <func>` — FIRST tool after any build mismatch

Sequence-aligned diff with relocation masking. Plain `dc.sh diff` cascades (1 missing instruction shows as 140+ "differences"); `diff-align` collapses the cascade and surfaces real structural diffs. Compute `byte_diff / 4` — if mine is 4 bytes short, you have **one** missing instruction, not 145.

Reading raw `dc.sh diff` output before `diff-align` is wasted tokens. Don't.

### 4.1.b `dc.sh iter-log show <func>` — trajectory + plateau detection

`dc.sh build-active` auto-records each round's diff count to `.bb2_iter_log/<func>.jsonl` and surfaces the recent trajectory + plateau advice. After 3 rounds at the same diff count, the advice escalates to "PLATEAU detected: switch technique." Don't keep iterating the same knob.

This was retrofitted after a previous session's 25-round push to match `calc_fc_frame_800203B4` bounced 35→39→35→40→35 across 8 rounds without realizing the plateau. Each round looked productive in isolation (diff count was reasonable); the trajectory is what surfaces "you're stuck."

### 4.2 `dc.sh diff-summary <func>` — categorized verdict

One-line-per-category classification of every differing instruction:

| Category | Next action |
|---|---|
| opcode-only | `dc.sh recipes <func>` — likely a named recipe (LICM, multu, byte-cast) |
| register-rename | `dc.sh add-regfix <func> swap $X $Y @ <idx>` — NOT permuter |
| immediate | Check signed vs unsigned cast at the source level |
| branch-offset | Label drift — `dc.sh fix-label-drift --apply` |
| structural (1–2) | Targeted fix via `dc.sh diff-align` |
| structural (3–15) | `dc.sh permute-adaptive <func>` — permuter territory |
| structural (>15) | Re-read m2c base.c (top of `agent-brief`); restructure the C shape, then re-run programmatic pipeline |
| cascade | False alarm — re-read `diff-align` |

`diff-summary` also runs a callee-save count diagnostic. If it reports `+N callee-save vs target`, that's almost always the lookup-store-via-delay-slot pattern (see `feedback_store_before_jal.md`) — fix that before anything else; the other diffs likely cascade from it.

### 4.3 `dc.sh recipes <func>` — named-recipe lookup

If the diff matches a known pattern (LICM unhoist, call-loop, early-exit alias, varargs prologue, nested-bool memcard, CU split, GTE 3x3, etc.), apply:

```
bash tools/dc.sh apply-recipe <recipe> <func>
```

### 4.4 `dc.sh near-miss <func> --apply` — auto-detected fix patterns

Detects `byte_arith_fix` / `drain_delay` / `plain_reg_substs` patterns and applies them with try-and-revert.

### 4.5 `dc.sh regfix-suggest <func> --apply` — auto-generated regfix rules

Diffs target.s vs the build pipeline output and emits `delete`/`insert_after`/`subst` rules with correct maspsx indices. Also flags gp-rel pseudos (emits `sdata_exclude.txt` hints) and label drift.

**Never write regfix rules from scratch before running this.** The indices are easy to get wrong (`lw $X, sym` is 1 maspsx instruction, not 2; `addiu $5,$zero,1` not `$0`; deletes/inserts shift indices in different phases). `regfix-suggest` knows the conventions.

**Suggest output > 15 rules → STOP, don't `--apply`.** The tool's default `--max-rules` is now 15 (lowered from 40 after the cpu_check_tubazeri retrospective: a 43-rule auto-apply regressed mine 32→44 diffs because rules operate on each other's intermediate indices and don't account for `as` delay-slot fills). When the cap fires, the right move is to RE-READ m2c base.c for shape mismatch, run `dc.sh permute-adaptive`, OR pick 3-5 KEY rules from the output by hand. Do NOT raise `--max-rules` to bypass the cap.

### 4.6 `dc.sh permute-adaptive <func>` — budget-scaled permuter

Budget scales with ins/del count: 1-2 → 90s; 3-5 → 5min; 6-10 → 15min; >10 → 30min. Run this; wait for it.

For long-tail cases:

```
bash tools/dc.sh permute <func> --max-time 600   # 10 min
bash tools/dc.sh permute <func> --max-time 1800  # 30 min
```

Long permuter runs find structural variants you wouldn't think of.

### 4.7 Build new tooling

If §4.1–4.6 don't close the gap, the project tooling needs an extension. **Build it.** Past examples:

- `prologue_fix.py` extensions for new prologue patterns
- New regfix ops (`fill_delay`, `drain_delay`)
- New named recipes captured via `dc.sh capture-recipe`

Don't ask permission. The user has explicitly authorized new tool work. Commit the tool alongside the function match.

---

## §5. MANUAL TECHNIQUE LADDER (only after §4 is exhausted)

When you have to write C by hand: read `feedback_matching_playbook.md` first. The penalty-list → technique routing table tells you the right move for the symptom.

The ladder (in order — each rung is a TECHNIQUE switch, never a TARGET switch):

1. **C structural alternatives** — goto vs if/else vs switch; intermediate variable for subtraction order (`s32 v = A - B; r = v - C;`); declaration position controls load timing; `do { } while(0)` scoping barrier; explicit cast widening for lh→lhu / lb→lbu.

2. **Register asm allocator hints** — `register T x asm("$N") = K;` constrains GCC's allocator. **This is NOT a function body in asm.** It's a 1-line hint telling the allocator to pin one value. WARNING: leaf temp regs (`$t0`–`$t9`) cause scheduling side effects; safer on callee-saves (`$s0`–`$s7`). See `feedback_70C70_lessons.md`.

3. **Long permuter runs** — `--max-time 3600` (1 hour), or overnight. Most cheap structural variants surface in the first 90s; long runs find the obscure ones. Acceptable.

4. **regfix at assembly stream** — `swap`, `subst`, `delete`, `insert`, `insert_after`, `insert_label`, `reorder`, `fill_delay`, `drain_delay`. Use `dc.sh add-regfix` (it pre-validates against live `dump_text_indices` before append). NEVER edit `regfix.txt` directly with `Edit`/`Write` — the `regfix_scope_guard.sh` hook will block you anyway, and you'll waste a tool call.

   **Maspsx idx vs binary idx** — these are NOT the same number. `dc.sh dump-text` shows MASPSX OUTPUT indices (post-cc1, pre-`as`). `dc.sh verify-c` shows BINARY indices (post-`as` expansion). A single maspsx line like `sw $X, 0x1F800360` is 1 maspsx idx but 2 binary insns (`as` expands it to `lui $at, hi; sw $X, lo($at)`). **Regfix rules (`subst`, `delete`, `insert_after`, `reorder`) operate on MASPSX indices**, not binary — get the idx by reading `dc.sh dump-text` (where `lw $X, ABS_ADDR` shows as a single line), NOT `dc.sh verify-c` (where it shows as the post-expansion lui+lw pair). Confusing them is the #1 reason regfix rules target the wrong instruction.

   **`subst > reorder` near jal/branch** — when target wants two instructions in swapped order and they're immediately before a jal, prefer two `subst` rules that swap the LINE TEXT in place over a `reorder` that moves them. The reorder produces e.g. `[lw $a0, lw $a1, jal, nop]` and `as` (with default `.set reorder`) auto-fills the jal delay slot from the immediately preceding instruction (`lw $a1`), giving `[lw $a0, jal, lw $a1 in delay]` — wrong byte sequence. The subst-swap leaves both instructions in their source positions so `as` doesn't promote either. See `feedback_scratchpad_gte_recipe.md` for the cpu_check_tubazeri reference.

   **`add-regfix insert/insert_after/subst` and `$N` shell escaping** — `wsl bash -c '... add-regfix insert "lui $7, ..."'` STRIPS `$7` (positional arg expansion). The tool now detects `lui ,` / `,,` / ` , ` patterns and aborts with an error, but the canonical safe pattern is to write a `tmp/inject_*.py` script that calls `Path("regfix.txt").write_text(... + rule + "\n", newline="\n")` — same Python idiom as the C-body injection in §5.x. NEVER pass `$N`-bearing asm text through a `bash -c` quoted string.

   **Inspecting POST-regfix output:** `dc.sh dump-text <func>` shows the maspsx output BEFORE any regfix rules apply — that's the canonical input-side view for writing rule indices. To see what regfix actually produced (post-substitution, post-fill_delay, post-insert), there are two options: (a) `python3 tools/dump_text_indices.py --post-regfix <func>` shows the maspsx output AFTER both regfix passes (this is what `regfix-suggest` reads); (b) `mipsel-linux-gnu-objdump -dz build/src/<file>.o | awk "/<func>/,/^$/"` shows the final assembled bytes after `as` runs (the byte-level view that `verify-c` compares against target). Use (a) when debugging "did my subst rule actually match" or "where did fill_delay put my instruction"; use (b) when you need to see the actual encoded bytes (`b` vs `j` opcodes, etc.) and confirm the hooked-up delay-slot wiring.

   **`regfix-suggest` j+nop ↔ b+useful_insn paired hint** — when mine has `[X, j .L<N>, nop]` and target has `[b .L<N>, X-equivalent in delay]`, the suggester now emits a paired `fill_delay @ <j_idx> <- <X_idx>` + `subst "j\\s+\\.L<N>" "b\t.L<N>" @ <j_idx>` hint. This is hint-only (not auto-applied) because the rules don't compose with the difflib delete/insert output — apply by hand via `tmp/inject_*.py` script. Recurring pattern in GTE wrappers (see `feedback_strength_reduce_defeat.md`).

5. **Compound regfix layering** — 10+ rules across multiple blocker classes for one function is normal. The `dc.sh asmfix-slice` tool can lift a small region into `asmfix.txt` if the surrounding C is correct but a sub-region is structurally unrepresentable (e.g., scratchpad pointer math GCC won't emit).

6. **Named recipes by reference** — LICM unhoist, call-loop family, early-exit alias breaker, varargs prologue, nested-bool memcard, CU split for jtbl interposition. All documented in `feedback_matching_playbook.md`.

7. **New transformation pass** — extend `prologue_fix.py`, add a pipeline stage, write a new regfix op, capture a new named recipe via `dc.sh capture-recipe`. Expected — this is how the project grows.

After each rung: rebuild + re-verify. If MATCHED → §6. Otherwise next rung.

### 5.x Writing C bodies through WSL (gotcha)

CLAUDE.md mandates all build-file edits go through WSL (CRLF rule). The naive pattern fails for any source containing `$N` literals (register names, asm strings, `$0`, `$1`, …) because **nested heredocs in `wsl bash -c '... <<EOF ... EOF'` re-evaluate `$N` as shell positional args**: `$0` becomes `/bin/bash`, `$1`–`$9` become empty strings, `$10` → `0`, etc. The corruption is silent until cc1/as errors out with `invalid operands 'ctc2 ,'`.

**Canonical pattern that works:**

1. Write a Python injection script to `tmp/inject_<func>.py` via the `Write` tool (Windows-side, with `newline="\n"` in `Path.write_text`).
2. Inside the Python: read the target file, do `text.replace(stub, body)` or `re.sub`, then `Path(target).write_text(new_text, encoding="utf-8", newline="\n")`.
3. Run via `wsl python3 tmp/inject_<func>.py` (no `bash -c`, no heredoc — Python reads the file's literal contents, no shell expansion happens).
4. (Optional) `dos2unix tmp/inject_<func>.py` first in case the `Write` tool emitted CRLF.

**What NOT to do:** `wsl bash -c 'python3 << "PYEOF" ... PYEOF'` with the body inline. The outer `bash -c '...'` and inner heredoc-with-quoted-delimiter interact unpredictably; even with `'PYEOF'` (quoted, "no expansion"), the outer bash has already consumed the `$N` tokens before the heredoc sees them.

When you suspect this trap fired: grep the cpp output of the file for the function name and look at the literal `register asm("...")` strings. If you see `asm("")`, `asm("0")`, `asm("1")` instead of `asm("$8")`, `asm("$10")`, `asm("$11")`, you've been bitten. Re-write the injection script via tmp file.

**Iterating after the first injection:** the injection regex matches the original BRIDGE STUB, so it won't match again once your body is in place. Do **not** reach for `git checkout src/<file>.c` to "reset and re-inject" — the active-marker hook blocks that on every WIP build file (it would erase your work; the block is the rule, not a bug). **Edit forward** via the `Edit` tool on src/ directly — it preserves the file's existing line endings, so the LF discipline is maintained. The hex-vs-decimal swc2 fix in the calc_fc_frame_8007EC5C run was a one-character `Edit` swap; reaching for `git checkout` first cost a tool call.

**Inline-asm offset format:** maspsx parses load/store offsets as decimal-only. `__asm__ volatile ("swc2 $11, 0x10(%0)" :: ...)` raises `invalid literal for int() with base 10: '0x10'` and breaks the build mid-pipeline. Use `__asm__ volatile ("swc2 $11, 16(%0)" :: ...)`. `dc.sh build-active` now pre-flights this and aborts before make if any `__asm__ volatile (".*<load_or_store> ..., 0x[0-9a-f]+(...")` slips through, but write decimal in the first place to avoid the round-trip.

---

## §6. ANTI-CHEAT GATE (mandatory before commit)

Before you consider the function done, **every** check below must pass.

### 6.1 Read the function body in src/

Open the file. Find the function. Look at its body. Apply this gate:

**Pure C: ALLOWED.** Including `register T x asm("$N") = expr;` allocator hints, which emit exactly the natural C operation under that register binding — not a custom asm body.

**The rule (principle):** A function body counts as pure C when the asm content is limited to single instructions whose purpose is either (a) accessing hardware the C language can't reach, or (b) steering the compiler's codegen. Multi-instruction asm blocks that *implement function logic* are forbidden.

Concretely:

**Pure C: ALLOWED.** Including `register T x asm("$N") = expr;` allocator hints, which emit exactly the natural C operation under that register binding — not a custom asm body.

**Single-instruction asm — ALLOWED** when its role is codegen control or hardware access, not function logic:

- **Empty/memory barriers** — `__asm__ volatile("" ::: "memory")`, `asm volatile("" : "=r"(x) : "0"(x))`, `__asm__ volatile("" : "+r"(x))`.
- **GTE coprocessor primitives (no C analog)** — `cop2`, `ctc2`, `mfc2`, `mtc2`, `cfc2`, `lwc2`, `swc2`, plus `.word 0x4[89A-F]......` (GTE op encoded as raw bytes).
- **Scheduling barriers that materialize a constant or hint placement** — e.g. `__asm__ volatile ("lui %0, 0x%hx" : "=r"(x))` to defeat GCC's hoist of a `lui` ahead of preceding loads; `__asm__ volatile ("nop")` to force an instruction slot. Permitted only when (i) the asm is one instruction, (ii) the C equivalent (`x = 0xFFFF0000;`) would emit the same bytes BUT in the wrong position, and (iii) you've already tried natural-C alternatives (variable declaration position, intermediate temporaries, do-while barriers — see §5 ladder rung 1). If you're using single-instruction barriers, mention them in the commit message ("technique: lui placement barrier") so reviewers can audit.

**BIOS kernel trampolines: ALLOWED** but rare and auto-categorized as `bios_or_syscall` (already filtered out of queue).

**FORBIDDEN — any of these means NOT MATCHED:**

- Top-level `__asm__("...")` block doing the function's actual computational work. This is "inline-asm-as-decomp" — the reverted-on-2026-05-12 anti-pattern.
- `register T x asm("$N") = K;` followed by a multi-instruction `asm volatile("...")` block that emits the rest of the function. The pin is fine; the body in asm is not.
- Multi-instruction `asm volatile (...)` blocks (newlines inside the asm string, or `\t...\n\t...`) that compute, branch, load, or store as part of function logic. The single-instruction rule above is *single instruction per asm block* — don't smuggle work in via concatenation.
- `replace_with_asmfile "<func>"` still ACTIVE in `asmfix.txt` (not preceded by `# RETIRE: `). The bridge is the original asm pasted in — that's not C.
- `INCLUDE_ASM("asm/funcs", <func>);` in src/.

**EXCEPTION — `inline_asm_canonical.txt`.** Functions listed in `inline_asm_canonical.txt` are explicitly authorized to use file-scope `__asm__()` with `glabel`/`endlabel` containing the function body (the §2.5.c form). The list is small, per-function-authorized, and gated on the strong-signal evidence in §2.5.b. The inline-asm-debt scanner skips these names. **If you wrote a §2.5.c inline-asm body but did NOT add the function to `inline_asm_canonical.txt` first (or get user authorization), this gate FAILS.** No silent slips: every authorized inline-asm function leaves a paper trail in `inline_asm_canonical.txt`.

**The test, when in doubt:** could a human reader figure out what the function computes by reading only the C, treating each one-instruction asm as a labeled primitive (e.g. "this `lui` materializes 0xFFFF0000 in $t9 here")? If yes, it's allowed. If the asm is doing the actual work and the C is just scaffolding, it's not — UNLESS the function is on `inline_asm_canonical.txt` (and the authorization is current).

If any forbidden pattern is present: strip it. Switch technique (§5). The function is **not done**.

### 6.2 Force a clean rebuild FIRST (clears stale .o cache)

```
bash tools/dc.sh verify --clean
```

`rm -rf build && make && verify --all`. Catches the build-cache regression trap (`feedback_retirement_recipes.md` gotcha #6). **Run this before `verify-c`** — otherwise `verify-c` may read stale bytes from a binary that doesn't reflect your latest src/ + asmfix.txt state and report a false MATCH. (`verify-c` now refuses if asmfix.txt is newer than build/bb2.exe, but it can still be fooled by stale .o files for unrelated src/ edits.)

### 6.3 `dc.sh verify-c <func>` (bridge-aware, after a clean build)

```
bash tools/dc.sh verify-c <func>
```

Refuses if any `replace_with_asmfile` line for `<func>` is still active in `asmfix.txt` (the bridge would mask your C body — false MATCH). Also refuses if asmfix.txt has been touched since the last build. Use this during retirement work — **never** plain `dc.sh verify <func>`, which is bridge-blind. See `feedback_retirement_recipes.md` gotcha #7 and G2 in `feedback_quick_reference.md`.

### 6.4 `dc.sh caller-audit <func>` (auto-runs on inline-replace)

Reports max args passed by any caller. If your decl is too narrow, GCC dead-codes the caller's extra-arg computation, cascading byte regressions through siblings. Widen the decl to the max observed arity.

### 6.5 `dc.sh post-match-validate <func>` (sibling-regression check)

After integration, scans for sibling .L<N> label drift. If anything else broke: fix the affected sibling's regfix rules (`dc.sh fix-label-drift --apply` or `dc.sh fix-asmfix-drift --apply`) before commit.

**If any of 6.1–6.5 fail: NOT MATCHED.** Go back to §4 or §5. Do not commit.

---

## §7. COMMIT

```
git add <specific files>      # NOT -A; avoid accidentally committing tmp/* or untracked transients
git commit -m "<src>.c: match <func> (<one-line technique summary>)"
```

Use the project's commit-message style (see `git log --oneline`): `<scope>.c: <verb> <func> (<technique>)`. Include `Co-Authored-By: Claude Opus 4.7 (1M context) <noreply@anthropic.com>` per CLAUDE.md.

The commit hook does a clean rebuild + bridge check. If it blocks: fix the underlying issue, **never** `--no-verify`. On success it auto-clears `.bb2_active_func`.

```
bash tools/dc.sh refresh-queue
```

Drops the matched function from WORK_QUEUE.md and runs `capture-recipe` on HEAD.

---

## §8. END THE INVOCATION (and decide whether to chain)

Report one line:

> Matched `<func>` (commit `<sha>`, technique: `<one-line>`).

**One invocation = one function. Always.** Do NOT pull a second function within this same invocation — the skill is deliberately scoped to one function so the guardrails (§0 contract, §6 anti-cheat gate, §9 banned language) reload fresh per function. Letting one invocation handle 10 functions in a row is how agents drift: by function 6 the contract has faded from active attention and an inline-asm "match" slips through.

**If the user explicitly asked for multiple functions** ("do the next 5", "work through 10", "run the queue until empty", "do N more", etc.):

1. Track progress in your own text (e.g., "3/10 done").
2. **Re-invoke this skill via the Skill tool** — `Skill({skill: "decomp-next"})`. That starts the next function with a fresh load of this entire contract.
3. Repeat until: (a) the user's count is reached, (b) the queue is empty, or (c) a `§0` stop condition fires (build mismatch / catastrophic state).

**If the user asked for one function** (or said "decomp the next one" / "do the next" / similar singular phrasing): stop here. Don't auto-continue. The user will re-invoke or move on as they choose.

**If you resumed an existing active marker that was already matched** (someone else committed it but the marker wasn't cleared, see G5 in the quick reference): report that and exit; do not re-invoke for a next function unless the user asked for multi.

**Between chained invocations**: don't put anything between the previous skill's end and the next skill's start except the one-line progress note. No "now starting the next one" preamble — the freshly-loaded skill content is the new context.

---

## §9. RESILIENCE — DO NOT QUIT

### 9.1 BANNED LANGUAGE

The Stop-event hook (`tools/hooks/grind_check.sh`) blocks these phrases when the active marker is set and the build is mismatch. They're shorthand for quitting and you must not use them:

- "next session", "future session", "let the next session", "pick up from here"
- "wrap up", "wrapping up", "to wrap", "final summary", "progress summary"
- "diminishing returns", "good place to stop", "let me leave it here", "let me wrap"
- "for now", "for this session", "this iteration", "is enough for one session"
- "I've made [substantial/significant] progress"
- "given the volume", "[N]+ more rules", "many hours", "[N]+ hours", "[N] iterations"
- "I've exhausted [angles]", "exhausted .* angle", "session ends? before", "ending this session"
- "ground hard", "honestly at this point", "one more push", "one more attempt", "final attempt"
- "tractable bound", "exceeds the tool", "cannot be coerced", "structural .* cannot be"
- "regalloc cannot", "register allocation cannot"
- "[N]+ attempts.{0,40}none", "reached -[N]", "still requires ~?[N]"
- "no improvement after", "reached the limit", "path [a-z] exhausted"

If you notice yourself reaching for any of these: that's the cue to switch technique (§5 next rung), not to wrap.

### 9.2 BANNED ACTIONS

- Asking the user for direction mid-work
- Running `dc.sh release` (user-only escape hatch)
- Manually deleting or truncating `.bb2_active_func` (the hook enforces; bypassing it = bug)
- `git checkout` / `git restore` / `git reset --` on `src/*.c`, `regfix.txt`, `asmfix.txt` to "start fresh" (hook blocks anyway; edit forward)
- `git commit --no-verify` (the verify hook is load-bearing)
- Mid-work status reports, progress dumps, summaries
- "I'm going to try X" preambles when X is just the next step

### 9.3 REQUIRED BEHAVIORS

- Commit when matched (the hook does the verification; trust it)
- Build new tools without asking
- Switch TECHNIQUE at each obstacle (target stays the same)
- Read memory files when stuck: `feedback_matching_playbook.md` (techniques + routing), `feedback_regfix_reference.md` (regfix syntax + gotchas), `feedback_retirement_recipes.md` (retirement traps), `feedback_70C70_lessons.md` (high-difficulty patterns)
- Keep working until the §0 stop condition is met

### 9.4 IF THE PERMUTER IS RUNNING

Permuter runs blocking the conversation are normal. 30-min and 1-hour runs are normal. Overnight is acceptable. Wait for them — don't kill them to "try something else."

If using `run_in_background`: you'll be notified when it finishes. Do other useful work in the meantime (read another memory file, study the asm, hypothesize the next variant).

### 9.5 EVAL MODE (user explicitly asks for skill/tool evaluation, not a match)

When the user invokes this skill with an explicit evaluation directive — "test the skill", "look for issues / inefficiencies", "walk through it and report friction", "audit the workflow", etc. — that directive overrides §0/§9.1/§9.2 **for reporting workflow observations**, not for stopping on a function:

- You may produce a structured report of issues found AT NATURAL CHECKPOINTS (between matches, or when the user explicitly asks "let's evaluate now").
- "I noticed X" / "issue: Y" / "this step takes N tool calls when it could take 1" are not banned wrap-up phrases; they are the deliverable.
- If the eval leaves `.bb2_active_func` set + asmfix.txt in retirement state + a mismatched build, **say so explicitly** and tell the user which command they'd run to clean up (`dc.sh release <func>` is user-only — agent can name it but not invoke it).

**Eval mode is NOT a license to stop on an unmatched function while avenues remain unexplored.** A previous agent (this is the documented trap) used "user asked me to evaluate" as cover for premature stopping mid-function with "fundamental wall" framing. The user pushed back: **"every time an agent believes themselves to be walled and wants to stop and check in, i want them to ask themselves 'is this code canonically C?' If it was, then we can get it decompiled and matching, we just have to try harder. Do not quit under any circumstances if we still have avenues unexplored."** (User directive, 2026-05-12.)

The rule:

- Eval directive + still-unexplored avenues → KEEP PUSHING. Do not lay out an evaluation as a way to stop. Run the next avenue.
- Eval directive + genuinely all avenues exhausted (including avenues you have to invent and write new tooling for) → that's not eval mode, that's §0/§9.1 territory and you still don't stop.
- Eval directive + match completed → report at natural checkpoint, fine.
- User explicitly says "stop and evaluate now" → fine, but make clear in the report that the function is unfinished and you'll resume.

Eval mode ends when (a) the user redirects ("ok now actually match it"), or (b) the function matches and the report is at a natural checkpoint.

### 9.6 ABORTING AN INVOCATION (rare; almost never the right move)

The skill's contract is one-function end-to-end. There is no agent-driven "abandon this function" path. The only valid termination of an unmatched invocation is **user-driven**:

```
bash tools/dc.sh release <func>     # user types the function name to confirm
make                                 # restore build to clean state (if retirement was started)
```

`release` restores the bridge if `# RETIRE: ` was active, and clears `.bb2_active_func`. The agent can name this command in a status report (eval mode) or when surfacing a catastrophic blocker (§0.3); the agent must not invoke it.

If you find yourself wanting to abort, you almost certainly are at a technique-switch point (§5 next rung), not a function-abandon point. Re-read `feedback_quick_reference.md` Part 9 (decision trees) and switch tools.

---

## §10. READING LIST (consult by symptom, not at the start)

**Primary reference: `memory/feedback_quick_reference.md`** — symptom-indexed, ~870 lines, covers every tool, every named technique, every gotcha. **Look here FIRST when stuck — before manual experimentation, before re-deriving anything.** Sections:
- Part 1: Tool catalog by workflow phase
- Part 2: Penalty → technique routing table
- Part 3: C-side techniques
- Part 4: register-asm hints (where the line is)
- Part 5: regfix operations (syntax, phases, idx counting, gotchas)
- Part 6: Named recipes (LICM, call-loop, early-exit, varargs, nested-bool, delay-slot fill, label-shift, hoist-removal, CU split, GTE 3x3, etc.)
- Part 7: Gotchas (G1 build-cache, G2 bridge false-match, G3 stub label drift, G7 GTE wrapper silent fails, G13 hand-written-asm detection, etc.)
- Part 8: Dead ends — don't try these
- Part 9: Decision trees

**Drill down to deep-reference files only when quick_reference points there:**

| Symptom | Deep file |
|---|---|
| Full toolbox order / unabridged technique discussion | `feedback_matching_playbook.md` |
| Full regfix syntax + smoke-test workflow | `feedback_regfix_reference.md` |
| Retirement gotchas with full incident context | `feedback_retirement_recipes.md` |
| Full GTE 3x3 recipe + reference matches | `feedback_gte_3x3_recipe.md` |
| Bridge signature audit data (41 mismatched stubs) | `feedback_bridge_signature_audit.md` |
| Tempted to bridge unauthorized | `feedback_bridge_is_not_decomp.md` |
| Suspected hand-coded asm — strong-signal checklist + authorization rules | `feedback_hand_coded_asm_recognition.md` |
| Stop-language patterns blocked by grind_check.sh | `feedback_voluntary_stop_forbidden.md` |
| HARD RULE / queue-driven selection / WSL discipline | `feedback_workflow_rules.md` |

Do not read everything up front. Read `feedback_quick_reference.md` once at the start of a hard problem; drill to a deep file only if the quick reference doesn't have what you need.

---

## TL;DR (one-paragraph contract)

Pre-flight → pull from queue → **`dc.sh memory-check <func>` + explicit C-vs-asm determination (§2.5)** → if asm-canonical, §2.5.c inline-asm path → otherwise `dc.sh attempt` → if not matched, run programmatic deepening (diff-align → diff-summary → recipes → near-miss → regfix-suggest → permute-adaptive) → if still not matched, manual ladder (C variants → register hints → long permuter → regfix → compound → named recipes → new tool) → before commit, anti-cheat gate (read body for forbidden asm, verify-c, verify --clean, caller-audit, post-match-validate) → commit → refresh-queue → done. One function, end-to-end, no quitting, no cheating.
