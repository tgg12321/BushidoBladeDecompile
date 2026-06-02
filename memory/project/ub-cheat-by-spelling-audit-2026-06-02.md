---
name: ub-cheat-by-spelling-audit-2026-06-02
description: Audit of UB / dead-read cheat-by-spelling patterns triggered by the C86C agent's UB-conditional-init observation. 3 COMPLETED-C functions carry load-bearing `if (...) { }` empty-body dead-read cheats (func_80072E10, func_8003D7B4, func_80049A2C). Detector gaps identified for empty-if, `(void) name;` without `&`, and `(void) array;` paths. C86C/C7A0 finding confirmed but both already parked.
metadata:
  type: project
---

# UB / Dead-Read Cheat-by-Spelling Audit — 2026-06-02

## Background

The `func_8007C86C` agent ran a ~5400-iteration clean-target permuter session
and reported (memory/project/func-8007c86c-permuter-ub-cheat-findings.md):

> "HEAD's own committed form (line 580-583, `var_a1 = arg1;` inside arg0<0
> else only) is the same UB cheat-by-spelling that produces target bytes."

The user's concern: if the C86C observation is correct, **other prior matches
may contain similar UB cheat-by-spelling patterns** that slipped through
historical detection. Current detectors (`engine/volatile_cheats.py`) catch
specific literal forms (unused arrays, `(void)&local;`, `arg0 = 0;`
dead-param assigns, dead-conditional-store, volatile-coercion) but **do not
catch UB-conditional-init** or the more general "dead read for codegen
coercion" pattern.

Per [[no-new-park-categories]] (2026-06-01): *"If it's a cheat, it will not
be accepted. Full stop."*

## Phase 1 — C86C finding: CONFIRMED

Read `src/display.c` lines 564-604 (func_8007C86C body) plus 523-562 (sibling
func_8007C7A0 — same shape). Both contain the agent's flagged pattern:

```c
s32 func_8007C86C(s16 arg0, s16 arg1) {
    s16 var_a1;           /* declared uninit */
    /* ... */
    if (arg0 >= 0) {
        /* var_a1 NOT assigned in this branch */
        ...
    } else {
        var_v0_2 = 0;
        var_a1 = arg1;    /* assigned ONLY in arg0 < 0 branch */
    }
    if (var_a1 >= 0) { ... }  /* read on all paths — UB when arg0 >= 0 */
```

A second UB-conditional-init in the same function: `int new_var2;` (line 570)
is assigned only in the first arm of the `((u32)(D_8009BE74 - 1)) >= 2U`
branch at line 593, then read in the else branch at line 600.

**Experimental verification of load-bearing-ness:**
- HEAD form (with UB): `sandbox func_8007C86C --disable all` → **20**
- Clean form (var_a1 = arg1 unconditional, new_var2 hoisted): → **22**

Removing the UB makes the cheat-free pure-C distance WORSE by 2. The UB
construct's existence shifts cc1's codegen toward what the function's 21
calibrated regfix rules expect to fix.

**Important nuance — both C86C and C7A0 are PARKED, NOT COMPLETED-C.**
Per engine/queue.json: both `status=parked`, `verdict=C`. Neither is
recorded in `inline_asm_canonical.txt`. They are INCOMPLETE work, not
"prior matches that hid the UB." Per [[no-new-park-categories]] the parked
state is still legitimate — they're being worked. But the HEAD source DOES
encode the UB-cheat-by-spelling pattern at lines 580-583 and 593-600.

## Phase 2 — Scan: UB conditional-init across src/*.c

`tmp/scan_ub_v4.py` (with v5 also tested): scans every function body for
locals declared without initializer that are assigned in EXACTLY ONE branch
of an if/else and then READ after the branch closes (the C86C pattern).

**Result: 4 findings across 1316 functions in 22 files, all in src/display.c
in the 2 parked siblings:**

| File | Func | Var | Pattern |
|---|---|---|---|
| src/display.c:539 | func_8007C7A0 | s16 var_a1 | assigned else only, read after |
| src/display.c:551 | func_8007C7A0 | int new_var2 | assigned if only, read after |
| src/display.c:580 | func_8007C86C | s16 var_a1 | assigned else only, read after |
| src/display.c:592 | func_8007C86C | int new_var2 | assigned if only, read after |

**No other committed function in the codebase carries this exact
UB-conditional-init pattern.** This is good news: the broader concern that
"prior matched functions may have slipped through with UB" is not borne out
for THIS specific syntactic shape. But the scanner has known blind spots
(see Phase 5).

## Phase 3 — Scan: sibling cheat-by-spelling patterns

The C86C agent's concern was broader: dead/empty code that DCE's to nothing
but influences cc1's analysis. The canonical sibling pattern is:

```c
if (some_cond) { }   /* empty body — does nothing observable
                        but exists as a READ of some_cond */
```

This is the same intent: a code construct with no semantic purpose, written
purely to influence GCC's flow/RA/scheduling analysis. NOT a literal
UB-conditional-init, but the same cheat-by-spelling family.

**Scanner approach:** search all src/*.c for `if (...) { }` (empty-body if),
with function-context and queue status.

**Total: 20 empty-body if statements; status breakdown:**
- 7 in COMPLETED functions (this is the concern)
- 11 in active/parked queue items (not committed yet — different concern)
- 2 outside detected function context

### COMPLETED-function findings (the cheats that slipped through)

| File:Line | Function | Pattern | Verdict |
|---|---|---|---|
| code6cac.c:1614 | camera_SetMatrix_8001DBE4 | `if (D_800A38F8 > D_800A37A0) { } else { do { ... } while (!cond); ... }` | NOT a cheat — semantically equivalent to `while (cond) { ... }`. The if-else with empty if branch is just an inverted-sense do-while loop. |
| code6cac_c2.c:1166 | **func_8003D7B4** | `if (!val) { }` between `val = func()` and use of `val` | **HIGH-confidence cheat. CONFIRMED LOAD-BEARING (sandbox 0 → 2 when removed).** |
| code6cac_c_mid.c:710 | func_800388A8 | `if (D_800A3318 != 0) { } else { result = 1; }` | NOT a cheat — equivalent to `if (D_800A3318 == 0) result = 1;`. Sense-flipped if. |
| text1b.c:1113 | **func_80049A2C** | `if (a1_val) { }` between two GPU-store statements | **HIGH-confidence cheat. Load-bearing-ness couldn't be isolated (removing it triggers sibling-rule cascade) but the function ALSO carries `s32 dummy[2]; ... (void) dummy; (void) new_var4;` — multiple cheat-by-spelling constructs in one function.** |
| text1b.c:16639,16649,16659,16669 | **func_80072E10** | Four `if (D_800A3580) { }` interspersed between blocks of field-stores | **HIGH-confidence cheat. CONFIRMED LOAD-BEARING (sandbox 0 → 44 when all 4 removed).** |

### Test results for load-bearing confirmation

**`func_8003D7B4` (code6cac_c2.c:1148):**
- HEAD baseline `sandbox --disable all`: **0** (perfect cheat-free match)
- After removing `if (!val) { }` (lines 1166-1167): **2**
- 0 regfix/asmfix rules, 0 cheat-asm, not in `inline_asm_canonical.txt`.
- Status: COMPLETED-C carrying a load-bearing dead-read UB cheat.

**`func_80072E10` (text1b.c:16635):**
- HEAD baseline `sandbox --disable all`: **0** (perfect cheat-free match)
- After removing all 4 `if (D_800A3580) { }`: **44** (massive regression)
- 0 regfix/asmfix rules, 0 cheat-asm, not in `inline_asm_canonical.txt`.
- Status: COMPLETED-C carrying 4 load-bearing dead-read UB cheats.

**`func_80049A2C` (text1b.c:1070):**
- HEAD baseline `sandbox --disable all`: **0** (perfect cheat-free match)
- After removing `if (a1_val) { }` (lines 1113-1114): build crashes with
  asmfix sibling-rule cascade (index shift breaks a later function's rules).
- Couldn't measure score in isolation, but the cascade BY ITSELF is evidence
  the empty-if is shifting bytes (load-bearing).
- Function also carries:
  - `s32 dummy[2];` (line 1086) + `(void) dummy;` (line 1148) — dead-local-array
    + (void) discard. The `find_unused_local_arrays` detector misses this
    because `dummy` IS referenced (by the `(void)` discard), defeating the
    "never referenced" check.
  - `char new_var4;` (line 1074) + `(void) new_var4;` (line 1149) —
    scalar version of the same coercion. The `find_addr_coerced_locals`
    detector requires `&` so the no-& form slips through.
- 0 regfix/asmfix rules, 0 cheat-asm, not in `inline_asm_canonical.txt`.
- Status: COMPLETED-C carrying multiple cheat-by-spelling constructs.

## Phase 4 — Other findings worth recording (informational)

### `if (a1_val) { }` in active/parked functions

These are NOT committed cheats (the functions are INCOMPLETE), but worth
noting for orchestrators:

- `func_800600C8` (text1b.c:12926): `if ((!hi) && (!hi)) {}` reads `hi`
  BEFORE it's assigned at line 12937. Pure UB read (no value at all). The
  function is INCOMPLETE (active, 6 rules). The UB read is likely a
  hand-decompiled m2c artifact rather than an intentional cheat, but the
  pattern is the same shape.

### `(void) <name>;` discard pattern (no `&`)

The current `_ADDR_COERCION_RE` requires `&`. The variant `(void) name;`
without `&` is a separate spelling not yet in the catalog. 343 instances
across src/*.c — most are in active/parked functions; ONLY 2 in COMPLETED
(both in `func_80049A2C` per above). So historically this pattern was
rarely used as a finished-match coercion. But: as a forward-looking
detector gap, agents could trivially write `(void) x;` to bypass the `&`
requirement. Worth widening the detector.

### `do { } while (0);` and `while (0)` constructs

39 occurrences (15 in COMPLETED). Most appear to be legitimate "single-
execution block-scope" idioms — typical compound-macro shape. But some are
suspicious:

- `do { idx++; idx--; } while (0);` (text1b.c, in `func_80049A2C`): self-
  cancelling expressions wrapped in do-while(0) is the same intent as
  `if (x) { }` — emit code that DCE's but affects analysis. Worth deeper
  inspection; not testable in isolation due to the same sibling-rule cascade.

These do-while(0) constructs aren't easy to triage automatically. Most
matched siblings (e.g. `func_8005344C`'s do-while(0)) appear semantically
load-bearing. Recommend: case-by-case review when the time comes, not a
broad sweep.

## Phase 5 — Recommendations (the actionable list)

### Immediate: re-evaluate the 3 confirmed-cheat COMPLETED-C functions

| Function | File | Cheat type | Action |
|---|---|---|---|
| `func_8003D7B4` | code6cac_c2.c | 1× `if (!val) { }` empty-body dead-read | **Re-route to active.** Worker needs to find a non-UB form. Honest cheat-free distance after removing the cheat is 2 (close — worth a focused attempt). |
| `func_80072E10` | text1b.c | 4× `if (D_800A3580) { }` empty-body dead-reads | **Re-route to active.** Honest cheat-free distance after removing all 4 is 44 — significant gap. Will be hard; may need canonical-asm authorization if no C lever found. |
| `func_80049A2C` | text1b.c | `if (a1_val) { }` + `s32 dummy[2]; (void) dummy;` + `char new_var4; (void) new_var4;` + `do { idx++; idx--; } while (0);` | **Re-route to active.** Function is dense with cheat-by-spelling; will need substantial re-derivation. Sibling-rule coupling means removal triggers cascading rebuilds. |

These three should be added to the queue's active bucket and have their
cheats removed before being committed again.

### Detector gaps to wire

1. **Empty-body `if (cond) { }`** — exact pattern is `if (<expr>) { }` with
   nothing inside. Has effectively zero legitimate use (the `if-else` with
   empty if-branch is a separate, semantically meaningful pattern that
   should NOT be flagged). Detector should be tight enough to skip:
   - `if (cond) { } else { ... }` (this is valid sense-flipped logic)
   - `if (cond) { /* comment */ }` (legitimate documentation placeholder
     might exist, though all 7 COMPLETED instances are empty)
   
   Trial implementation: `engine/volatile_cheats.find_empty_if_dead_reads`
   that matches `^[ \t]*if\s*\([^)]+\)\s*\{\s*\}[ \t]*$` outside
   if-else contexts. Verify zero false positives across the codebase
   before wiring as `mark_done` refuser.

2. **`(void) <name>;` without `&`** — extend `_ADDR_COERCION_RE` to match
   `\(\s*void\s*\)\s*(?:&\s*)?(?P<name>\w+)\s*;`. The 343 active/parked
   instances need triage — most are likely legitimate "discard return value"
   uses (`(void) func_call();` is fine and not the cheat form) — so the
   detector must distinguish:
   - `(void) func_call();` — discarding a side-effect-bearing call (fine)
   - `(void) local_var;` — discarding a local's value (cheat-by-spelling)
   - `(void) global;` — discarding a global's read (cheat-by-spelling)
   - `(void) &local;` — already caught (cheat)
   - `(void) array;` — discarding array-decay-to-pointer (cheat — array
     was declared just to get the frame slot)
   
   Approach: a name preceded only by `(void)` and `;` qualifies as a "bare
   discard"; if the discarded thing is a function call (contains `(...)`),
   skip. If it's a bare identifier, flag.

3. **UB-conditional-init** — the V4/V5 scanner in `tmp/scan_ub_v4.py` works
   but currently is a Python script, not wired. Could be added to
   `engine/volatile_cheats.py` as `find_ub_conditional_init`. Would refuse
   completion for any function with a local declared uninit, assigned in
   exactly one branch of an if/else, and read after.

4. **`do { ... } while (0);` with no side-effects** — harder, needs body
   analysis. Defer until a specific committed instance is identified as
   load-bearing.

### Forbidden patterns to capture in `.claude/rules/`

The findings above should be summarized in a new
`.claude/rules/empty-if-dead-read.md` (analogous to
[[inline-asm-injection]] and [[dead-vars-local-array]]) so future agents
know this construct is forbidden by intent, even before the detector lands.

### What was NOT covered this audit (recommended for next pass)

1. **Switch statements with UB-conditional-init across cases.** Built a
   scanner (`tmp/scan_switch_ub.py`) but it had false positives from
   inner-block scoping — would need careful scope tracking. The C86C
   pattern is the simple form; the switch form might exist in some
   harder-to-detect shape.

2. **Strict-aliasing punning** (`*(s32 *)&float_var` etc). 34 hits across
   src/*.c, but ALL appear to be the well-known `*(s8 *)&u16_global` lever
   from `.claude/rules/u16-global-lhu-lbu-low-byte.md` and
   `narrow-stack-param-subword-offset.md`. These are blessed pure-C
   matching techniques in the project, not cheats. Confirmed not a concern.

3. **Reading union members not last-written** — would need semantic
   analysis. Skipped.

4. **goto-into-nested-block** — looked for the pattern, no hits.
   Not present in this codebase.

5. **Signed-integer-overflow exploits** — would need value-range analysis.
   Skipped.

6. **`do { idx++; idx--; } while (0);` self-cancelling expression patterns**
   inside do-while(0). Saw one instance; deeper sweep deferred.

7. **The `(void)` discard catalog walk** — only the 2 in COMPLETED were
   analyzed. The 343 in active/parked deserve a triage pass if/when those
   functions reach the queue top.

## Phase 6 — Verification methodology used

For each suspected load-bearing UB cheat:
1. Read the source to confirm the construct's syntactic form
2. Backup the file (`tmp/<file>.bak`)
3. Remove the suspect construct
4. Run `& tools/eng.ps1 sandbox <func> --disable all`
5. Compare the score against HEAD's baseline
6. Revert from backup
7. Re-verify oracle (`verify-oracle`)

If the score WORSENS upon removal, the construct is load-bearing for the
match — confirming cheat status. If it stays the same or improves, the
construct is genuinely dead (a stale artifact, not a cheat).

For functions where the construct's removal triggers sibling-rule cascade
(asmfix index-anchored rules break), the score can't be isolated, but the
cascade ITSELF is evidence the construct is shifting bytes (otherwise
sibling rules wouldn't notice).

All experimental edits were reverted before this memory note was written.
`& tools/eng.ps1 verify-oracle` confirms `build_matches: true` end-state.

## Genuine resume avenues

1. **Wire the detectors.** Implement `find_empty_if_dead_reads`,
   `find_void_discard_locals`, `find_ub_conditional_init` in
   `engine/volatile_cheats.py`. Run `queue regen` and observe which
   functions get re-routed from `done` to `active`.

2. **Re-derive the 3 confirmed functions.** `func_8003D7B4`,
   `func_80072E10`, `func_80049A2C` will need pure-C re-attempts with
   their cheats removed. May require permuter runs or careful structural
   levers (see [[register-alloc-pure-c]] and friends).

3. **The C7A0/C86C pair stays parked.** No change in their status from
   this audit — they were already parked and remain so. The reformulated
   work on them is unchanged: find a non-UB form, OR escalate as canonical
   if no C form reaches target.

4. **The do-while(0) sweep** if/when a confirmed-load-bearing instance
   shows up. Currently inconclusive — most are legitimate scoping idioms.

## Related

- [[func-8007c86c-permuter-ub-cheat-findings]] — the agent's original
  observation that triggered this audit
- [[func-8007c86c-clean-form-floor]] — the C86C clean-form context
- [[no-new-park-categories]] — the policy this audit enforces
- [[completion-standard]] — the COMPLETED-C bar these 3 functions fail
- [[inline-asm-policy]] — the expanded cheat catalog this audit extends
- [[dead-vars-local-array]] — the frame-coercion forbidden pattern; the
  (void) variant detected here is a syntactic gap in the existing detector
- [[register-alloc-pure-c]] — the lever playbook the 3 re-derived
  functions will need
- [[difficult-is-not-impossible]] — cardinal rule for the re-derivation
