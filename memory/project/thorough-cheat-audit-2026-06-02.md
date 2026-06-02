---
name: thorough-cheat-audit-2026-06-02
description: Comprehensive cheat-by-spelling audit 2026-06-02 — 25 affected COMPLETED-C functions across 8 cheat families (vs prior 5). 4 detector gaps identified — lowercase asm, volatile-typed unused local, empty do-while(0), if(1). Confidence ~85%.
metadata:
  type: project
---

# Thorough cheat-by-spelling audit — 2026-06-02

## Why this audit exists

User concern: "I don't want to keep working through functions just to find out
a week later that we are much farther out than we thought." Pattern observed:

- 2026-06-02 manual audit ([[ub-cheat-by-spelling-audit-2026-06-02]]): found 3
  affected COMPLETED-C functions
- 2026-06-02 detector landing (commit `7c8a8fe`,
  `find_empty_if_dead_reads` + `find_void_discard_unused_locals`): caught the
  same 3 plus 2 NEW ones in COMPLETED-C; commit message also named ~10 more
  but those were already-INCOMPLETE (in the queue) so not actually historical
  leakage.

Each iteration found substantially more cheats. This audit is the comprehensive
sweep to determine TRUE scope before allocating Phase 2 retry budget.

## Methodology

Phase A — ran every wired detector in `engine/volatile_cheats.py` +
`engine/inlineasm.py` against every function in `src/*.c`, filtering to
COMPLETED-C only (i.e. NOT in `engine/queue.json` AND NOT in
`inline_asm_canonical.txt`). Probe script: `tmp/audit_phase_a2.py`.

Phase B — brainstormed 24 candidate cheat-by-spelling patterns, regex-scanned
each one across the codebase, attributed hits to owning functions, filtered to
COMPLETED-C. Probe: `tmp/audit_phase_b.py`. Additional targeted scans:
`tmp/audit_asm_lowercase.py` (lowercase `asm` keyword), `tmp/check_dowhile_owners.py`
(do-while(0) variants).

Phase C — manual sample review of ~30 COMPLETED-C functions stratified across
22 source files and sizes (small/medium/large). Probe: `tmp/sample_completed.py`.

Phase D — aggregation: `tmp/aggregate_audit.py` → `tmp/aggregate_audit.json`.

## Phase A — comprehensive detector run

Detectors wired post-`7c8a8fe`:
`find_volatile_externs`, `find_volatile_casts`, `find_alias_renames`,
`find_nonvolatile_alias_renames`, `find_macro_asm_defs`,
`find_unused_local_arrays`, `find_addr_coerced_locals`,
`find_void_discard_unused_locals`, `find_empty_if_dead_reads`,
`find_dead_conditional_stores`, `find_dead_param_assigns`, plus
`engine.inlineasm._strip_spans` for `__asm__` blocks + register pins.

**Total COMPLETED-C functions scanned: 809.**

**Detector hit counts (Phase A, COMPLETED-C only):**

| Detector | Hits | Unique funcs |
|---|---|---|
| `empty_if_dead_read` | 9 | 5 |
| `void_discard_unused_local` | 2 | 1 |
| all others | 0 | 0 |

**5 COMPLETED-C functions caught by existing detectors:**

| Function | File | Pattern | Notes |
|---|---|---|---|
| `func_800224E0` | code6cac.c | `if ((!i) && (!i) && (!i)) { }` + `if ((val && val) && val) { }` | Triple-redundant self-test, empty body. NEW (not in 2026-06-02 audit). |
| `func_80023DB8` | code6cac.c | `if ((arg0 && arg0) && arg0) { }` | NEW. |
| `func_8003D7B4` | code6cac_c2.c | `if (!val) { }` between `val = func()` and use of `val` | KNOWN (audit). Load-bearing confirmed (sandbox 0→2). |
| `func_80049A2C` | text1b.c | `if (a1_val) { }` + `(void) dummy;` + `(void) new_var4;` | KNOWN. |
| `func_80072E10` | text1b.c | 4× `if (D_800A3580) { }` | KNOWN. Load-bearing (sandbox 0→44). |

(The commit message for `7c8a8fe` listed ~10-12 "new affected COMPLETED-C"
including `func_80049718`, `func_80073060`, `func_800600C8`, `gnd_disp_loop_ctrl`,
`AddTbpOfst_80047EE8`, `func_80060A68`, `func_80034200`, `gnd_init_80041688`,
`cpu_check_tubazeri` — but ALL of these are in the queue (INCOMPLETE), so they
are not historical-leakage COMPLETED-C carriers. The commit's claim of
"underreported by 3-4×" is incorrect; the actual Phase A floor over the prior
manual audit's 3 is 5.)

## Phase B — brainstormed patterns

24 brainstormed patterns scanned. **The 4 with significant COMPLETED-C hits:**

### B1: lowercase `asm(...)` cheat-blocks bypass `ASM_KEYWORD_RE` — 5 NEW funcs

`tools/classify_inline_asm.ASM_KEYWORD_RE = \b(__asm__|__asm)\s*(?:volatile|__volatile__)?\s*\(`.
Lowercase `asm volatile(...)` is NOT matched — neither for detection nor for
cheat-asm stripping. There are **878 lowercase-asm blocks codebase-wide; 876
classify as cheat (scheduling barriers, inline-move-aliasing, register pin
equivalents), 5 of those sit in COMPLETED-C functions:**

| Function | File | Lowercase-asm cheat |
|---|---|---|
| `func_80046BF4` | sound.c | 2× `asm volatile("" ::: "memory")` between sequential global stores (cross-jump barrier) |
| `func_8005B8B8` | text1b.c | `asm volatile("" : "=r"(t0) : "0"(t0))` — inline-move-aliasing |
| `func_8007CAC8` | display.c | `asm volatile("")` — bare scheduling barrier between global store and call |
| `func_80082A14` | ings2.c | `asm volatile("" ::: "memory")` after `volatile s32 counter` (scheduling barrier on top of legit volatile counter) |
| `func_80082C58` | ings2.c | 2× `asm volatile("" : "=r"(s0b) : "0"(s0b))` — inline-move-aliasing |

Same intent as forbidden `__asm__ volatile(...)` per [[inline-asm-policy]];
just spelled with lowercase `asm`. The detector + cheat-asm strip BOTH need to
treat lowercase `asm` as equivalent to `__asm__`.

### B2: `volatile T <local>;` unused-local frame coercion — 4 NEW funcs

Existing `find_unused_local_arrays` catches `s32 buf[N];` and
`find_addr_coerced_locals` catches `(void)&local;`. Neither catches the SCALAR
volatile-typed equivalent `volatile s32 pad;` (declared, never referenced, but
GCC reserves frame for it because the volatile qualifier defeats DCE).

| Function | File | Local | Verdict |
|---|---|---|---|
| `spu_DmaTransfer` | main.c | `volatile s32 pad;` | CHEAT — pad never referenced, pure frame coercion |
| `save_vc_ctrl` | text1a.c | `volatile s32 sp;` | CHEAT — sp never referenced |
| `func_80044010` | text1a_c.c | `volatile s32 sp_pad;` | CHEAT — name announces intent |
| `saSeMain_80045510` | text1a_c.c | `volatile s32 sp_pad;` | CHEAT — name announces intent |

**Borderline (volatile-typed locals that ARE legitimate):**

| Function | File | Local | Verdict |
|---|---|---|---|
| `spu_WriteReg16` | main.c | `volatile s32 i, v;` | LEGIT — calibration timing loop, volatile defeats GCC fold of `for(i;i<0x3C;i++) v=v*13;` |
| `cdrom_DmaChain` | system.c | `volatile s32 tmp; tmp = *g_cd_dma_ctrl;` | LEGIT — hardware register fence read; value discarded but the read must occur |
| `func_80082A14` | ings2.c | `volatile s32 counter = a1 << 15; do { if (--counter == -1) ...` | LEGIT volatile counter — but the function ALSO has a separate `asm volatile("" ::: "memory")` (B1) that IS a cheat |

A scalar-volatile-local detector must distinguish "never referenced" (cheat)
from "used as a calibration counter / hardware fence read" (legit). The
existing `find_void_discard_unused_locals` filter logic (check whether name has
non-discard references) extends naturally.

### B3: `if (1) { ... }` always-true dead-conditional — 3 NEW funcs

Pure scaffold — wraps real code in a non-conditional. No semantic purpose.
Same family as empty-if (`find_empty_if_dead_reads` already wired).

| Function | File | Construct |
|---|---|---|
| `spu_SetMotionActive` | main.c | `if (1) { *(...)..; base = (new_var = *base_ptr); ... }` wraps function-body work |
| `func_80047BE0` | sound.c | `if (1) { ... main body of 1686-byte function ... }` |
| `func_80044100` | text1a_c.c | `if (1) { do { } while (0); }` nested dead scaffolding |

### B4: Empty `do { } while (0);` body — 8 NEW funcs (5 unique funcs not flagged elsewhere; 3 shared with other patterns)

Same intent as empty-if: a syntactic construct GCC's DCE removes from the
emitted output, but its existence steers analysis upstream of DCE.

| Function | File | Notes |
|---|---|---|
| `func_8003C42C` | code6cac_c2.c | `do { } while (0);` |
| `game_Init` | sound.c | `do { } while (0);` |
| `func_80044100` | text1a_c.c | nested inside `if (1)` (already B3) |
| `func_80044670` | text1a_c.c | `do { } while (0);` |
| `func_800493E4` | text1b.c | `do { } while (0);` |
| `func_80068F70` | text1b.c | TWO empty `do { } while (0);` + ONE non-trivial `do { mask = ...; cache = ...; } while (0);` wrapping init (UB: `const u32 mask;` declared without initializer) |
| `func_8008BEA4` | main.c | `do { } while (0);` (inside the `(flag && flag) && flag` redundant-test scaffold from B5) |
| `func_8008C184` | main.c | Same as 8008BEA4 |

`func_80068F70` is the most concerning: it has THREE distinct cheat constructs
in one block — empty do-while, init-wrapped-in-do-while-zero, AND `const u32
mask;` declared without an initializer (undefined behavior). Likely needs heavy
re-derivation.

### B5: `x && x` / `x || x` redundant self-test — 4 funcs (2 NEW)

`func_800224E0` and `func_80023DB8` are already caught (the empty-if surrounds
the redundancy). Two MORE NEW funcs:

| Function | File | Construct |
|---|---|---|
| `func_8008BEA4` | main.c | `if ((flag && flag) && flag) { do { } while (0); }` |
| `func_8008C184` | main.c | `if ((flag && flag) && flag) { do { } while (0); }` |

The triple-test `(x && x) && x` is a syntactic shape with zero plausible
programmer purpose. Combined with the empty `do-while(0)` body, this is a
two-cheat composite (B4 + B5 in one statement). The function's commit history
should be checked — if introduced as a workaround for a specific codegen miss,
identify whose register allocation the cheats are coercing.

### B-other (negative results — patterns scanned but found NO new COMPLETED-C funcs)

| Pattern | Hits codebase-wide | COMPLETED-C hits |
|---|---|---|
| `x = x + 0;` / `x = x - 0;` / `x = x \| 0;` / `x = x ^ 0;` / `x = x * 1;` etc | 0 | 0 |
| `(x, expr);` comma-operator | 1051 | 412 — but all are function-call arg lists (false positive of the regex; manual inspection shows zero genuine comma-operator dead-reads) |
| `(void)(x == y);` comparison-as-statement | 0 | 0 |
| `y = cond ? x : x;` ternary same-value | 0 | 0 |
| `sizeof(x);` as statement | 0 | 0 |
| `(s32)(s32)x` self-cast chain | 0 | 0 |
| `&local;` address-of-only statement | 0 | 0 |
| `"literal";` as statement | 0 | 0 |
| `a = b = c = x;` multi-assign chain | 0 | 0 |
| `if (0) { }` | 1 (`func_80049718`, IN QUEUE — INCOMPLETE) | 0 |
| `(void) <number>;` | 0 | 0 |
| `x = x;` self-assign | 3 | 0 (all in queue items) |
| `if (x == x)` self-compare | 0 | 0 |
| `x += 0;` / `x \|= 0;` / `x <<= 0;` compound-assign no-op | 0 | 0 |
| Empty `else { }` block | 0 | 0 |
| `idx++; idx--;` self-cancelling in `do { } while (0)` | 1 (`func_80062FEC`, IN QUEUE) | 0 |
| `const T x;` without initializer (B5-related but standalone) | 1 (`func_80068F70`, already in B5) | 1 |

Pattern coverage is broad — the empty-if + lowercase-asm + volatile-local +
if(1) + empty-do-while family captures essentially all forms of "construct
written purely to influence GCC; DCE'd before emit."

## Phase C — manual sample review

30 COMPLETED-C functions reviewed across 22 source files, stratified by size.
Most are clean (genuine pure-C decomp). ONE non-detector-caught finding:

### tslPolyF4Init (system.c)

This is COMPLETED-C (commit message in [[global-label-drift-sibling-cheat]]
documents it as a model fix). But its body contains constructs the standing
policy now classifies as cheats:

```c
s32 count;
unsigned long long new_var2;  /* DImode chain */
int new_var;
...
new_var = 3;
new_var2 = new_var;   /* DImode chain: u32 -> u64 */
count = new_var2;     /* DImode chain: u64 -> s32 */
```

The comment in the global-label-drift rule says these declarations are
"load-bearing for scheduling" — i.e., the DImode chain exists specifically
because removing it shifts the prologue scheduling. Per [[no-new-park-categories]]:
*"Does the form contain any code with no semantic purpose? Would a human
programmer naturally write this code? Does the closing form's justification
reference GCC internals instead of program logic?"* — this fails all three.
Same family as [[register-alloc-pure-c]] Lever D (dead-param-assign, forbidden
2026-05-31) and [[dead-vars-local-array]] (frame coercion, forbidden 2026-05-31).

ALSO carries `goto done_label_pad; done_label_pad:` dead label for global `.L`
counter compensation — same family as the inline-asm injection cheat
(`global-label-drift-sibling-cheat` calls it "the general lever for negative
label-drift"). The rule itself documents this as a deliberate construct, but
it has zero semantic purpose — it exists ONLY to bump cc1's `label_num` counter.

Total: 1 NEW affected COMPLETED-C function found by Phase C manual review.

**Other Phase C observations (all clean):**

- `func_80019534`, `func_8001C4C0`, `func_80023CB4`, `cpu_get_move_pattern_table_number`
  (code6cac.c): clean. `sp10[6]` and `buf[4]` are referenced in function bodies — legit locals.
- `func_80035430`, `func_80033498`, `func_80027438`, `func_80032854` (code6cac_b.c): clean.
- `gpu_LoadImage`, `ot_SetEnd`, `initPolyGT3`, `func_8007A3C8`, `gpu_SetMode` (gpu.c): clean.
- `sys_StubEmpty2`, `file_GetFlag1`, `disp_CalcFov`, `file_LoadAll` (ings.c): clean.
- `sys_GetVideoMode`, `irq_DisableInterrupts`, `sys_MemClear2` (ings2.c): clean.
- `spu_Init`, `func_8008BDE8`, `func_800892F8`, `saTan0Main` (main.c): clean. (saTan0Main has the
  documented `char b` narrow-type lever + block-local `cp` split — both pure C levers per
  [[register-alloc-pure-c]] Levers A & B, NOT cheats.)
- `game_Stub2`, `game_AnimCleanup`, `game_SetPause`, `func_80047BE0` (sound.c): MOSTLY clean,
  but `func_80047BE0` has the `if (1) { ... }` already in Phase B.
- `func_80080148`, `Vu0SetLightColMatrix_80080208`, `cdrom_CheckReady` (system.c): clean.
- `leaf_muki_awase_800421A4`, `saTan5TakeGetCnt`, `func_8004211C`, `gnd_land_hit_char_die_main`
  (text1a.c): clean.
- `func_800466C0` (text1a_b.c): clean.
- `func_80044E64`, `seq_Start`, `func_80042ED8`, `func_80044504` (text1a_c.c): clean.
- `func_8004668C`, `func_8004659C` (text1a_c2.c): clean.
- `func_8004A938`, `func_800676C8`, `func_80069120`, `func_80068F70` (text1b.c): `func_80068F70`
  is dirty (Phase B). Others clean.
- `func_80078BF0`, `pad_Init`, `bb2_memcpy`, `func_8007855C` (text1b_b.c): clean.

## Phase D — Aggregate findings

**25 COMPLETED-C functions affected by at least one cheat-by-spelling
construct.** Distribution by cheat-family count per function:

| Cheats per function | # functions |
|---|---|
| 1 family | 18 |
| 2 families | 6 |
| 3+ families | 1 (`func_80068F70`) |

### Full deduplicated list of affected COMPLETED-C functions

| # | Function | File | Cheat families | Notes |
|---|---|---|---|---|
| 1 | `func_800224E0` | code6cac.c | empty_if(2× `(!i)&&(!i)&&(!i)`, `(val && val) && val`) | Caught by detector |
| 2 | `func_80023DB8` | code6cac.c | empty_if `(arg0 && arg0) && arg0` | Caught |
| 3 | `func_8003C42C` | code6cac_c2.c | empty_do_while_0 | NEW Phase B |
| 4 | `func_8003D7B4` | code6cac_c2.c | empty_if `(!val)` | Caught + audit-known |
| 5 | `func_80044010` | text1a_c.c | volatile_local_unused `sp_pad` | NEW Phase B |
| 6 | `func_80044100` | text1a_c.c | empty_do_while_0 nested in if(1) | NEW Phase B |
| 7 | `func_80044670` | text1a_c.c | empty_do_while_0 | NEW Phase B |
| 8 | `func_80046BF4` | sound.c | lowercase_asm cheat block (2× memory barrier) | NEW Phase B |
| 9 | `func_80047BE0` | sound.c | if(1) wrapping function body | NEW Phase B |
| 10 | `func_800493E4` | text1b.c | empty_do_while_0 | NEW Phase B |
| 11 | `func_80049A2C` | text1b.c | empty_if + 2× void_discard | Caught + audit-known |
| 12 | `func_8005B8B8` | text1b.c | lowercase_asm cheat (inline-move-aliasing) | NEW Phase B |
| 13 | `func_80068F70` | text1b.c | empty_do_while_0 (×2) + const-decl-without-init + init-wrapped-in-do-while-0 | NEW Phase B — 3 distinct cheats |
| 14 | `func_80072E10` | text1b.c | empty_if (4×) | Caught + audit-known |
| 15 | `func_8007CAC8` | display.c | lowercase_asm bare barrier | NEW Phase B |
| 16 | `func_80082A14` | ings2.c | lowercase_asm memory barrier (`volatile s32 counter` is legit) | NEW Phase B |
| 17 | `func_80082C58` | ings2.c | lowercase_asm (2× inline-move-aliasing) | NEW Phase B |
| 18 | `func_8008BEA4` | main.c | empty_do_while_0 + redundant_self_AND | NEW Phase B |
| 19 | `func_8008C184` | main.c | empty_do_while_0 + redundant_self_AND | NEW Phase B |
| 20 | `game_Init` | sound.c | empty_do_while_0 | NEW Phase B |
| 21 | `saSeMain_80045510` | text1a_c.c | volatile_local_unused `sp_pad` | NEW Phase B |
| 22 | `save_vc_ctrl` | text1a.c | volatile_local_unused `sp` | NEW Phase B |
| 23 | `spu_DmaTransfer` | main.c | volatile_local_unused `pad` | NEW Phase B |
| 24 | `spu_SetMotionActive` | main.c | if(1) wrapping body | NEW Phase B |
| 25 | `tslPolyF4Init` | system.c | DImode chain for scheduling + dead-goto label pad | NEW Phase C |

**Vs prior counts:**
- 2026-06-02 manual audit: 3 affected
- 2026-06-02 detector landing (`7c8a8fe`): caught 5 (true COMPLETED-C subset of its claim)
- THIS AUDIT: **25 affected** → ~5x the post-`7c8a8fe` figure, ~8x the manual-audit figure

## Recommendations

### Priority 1 — implement lowercase-`asm` detector (high ROI)

**Surfaces 5 NEW COMPLETED-C functions in one detector landing.** The fix is
trivial: `engine.inlineasm` / `tools.classify_inline_asm.ASM_KEYWORD_RE`
extends from `\b(__asm__|__asm)\s*(?:volatile|__volatile__)?\s*\(` to
`\b(__asm__|__asm|asm)\s*(?:volatile|__volatile__)?\s*\(`. Regression-test
that the existing 11 GTE wrapper functions etc. still classify correctly.

This MUST cascade: the cheat-asm stripper in `_strip_spans` already iterates
`ASM_KEYWORD_RE.finditer`, so a fix there immediately strips lowercase-asm
cheats in the sandbox. Affected functions' sandbox distances will REVEAL the
honest pure-C gap once they re-route to active.

### Priority 2 — implement volatile-typed unused-local detector (clear win)

Catches 4 NEW COMPLETED-C functions. Logic: same scaffolding as
`find_void_discard_unused_locals` (parameter-vs-local check + reference
count). Pattern: `volatile <type> <name>;` (optionally with an initializer)
where the name has only zero or one references in the body. Carve out the
calibration-loop case (where the volatile is used in a loop counter) — that
naturally falls out of the "reference count > 1" guard.

Risk: the 3 borderline cases (`spu_WriteReg16`, `cdrom_DmaChain`,
`func_80082A14`) genuinely use the volatile. The reference-count carve-out
should handle all three correctly.

### Priority 3 — implement empty `do { } while (0);` detector

Catches 5 NEW COMPLETED-C functions outright + one composite (`func_80068F70`)
with multiple instances. Detection is symmetric to `find_empty_if_dead_reads`:
match `do\s*\{[ \t\n]*\}\s*while\s*\(\s*0\s*\)\s*;` outside macro-expansion
contexts. (False positive risk: legitimate `do { single_stmt; } while (0);`
compound-macro idioms — those have a non-empty body, so the empty-body filter
handles it.)

### Priority 4 — implement `if (1) { ... }` detector

Catches 3 NEW COMPLETED-C functions. Pattern: `if\s*\(\s*1\s*\)\s*\{` —
trivial regex, near-zero false positive risk (it's not a pattern any human
writes intentionally).

### Lower priority / future work

- **Redundant self-AND/OR** (`(x && x) && x`): only 2 COMPLETED-C functions
  affected, but both share the same construct so a single detector pattern
  addresses them. Worth a detector. Risk: a few-percent false positives
  (e.g. `if (ptr && ptr[0])` is legitimate). Tight regex on identical
  operand identifiers only.
- **`const` declared without initializer**: 1 function. UB by C standard
  (cannot assign to const). Worth a detector if/when the scaffolding cheat
  family expands. Trivial regex.
- **DImode chain for scheduling**: 1 function (`tslPolyF4Init`). Difficult
  to detect generically (the chain LOOKS like normal C); requires AST-level
  analysis or pattern-match on the specific `unsigned long long X; X = Y; Z = X;`
  shape where Z is then used as a counter. Defer.
- **Dead `goto X; X:` label pad**: 1 function (`tslPolyF4Init`). The
  `global-label-drift-sibling-cheat` rule documents this as a deliberate
  technique. Whether it's a cheat depends on whether `.L` global-counter
  preservation is an accepted matching mechanism (per current policy, it
  ISN'T pure-C — same as inline_asm_injection of bytes). Mark as deferred /
  escalate to user policy.
- **Comma-operator dead reads** (P02 in Phase B): the regex hit 412 in
  COMPLETED-C but ALL are false positives (function call arg lists). A
  semantically-correct comma-operator detector requires an AST. Defer.

### Decision: should Phase 2 (function retries) expand?

**YES, decisively, to ~25 functions.** The leakage from the prior detector
set was substantial:

- Original concern: ~3 functions
- Post-7c8a8fe detectors: ~5 functions
- This audit: 25 functions

The new ~20 are unsurprising in shape — they extend the existing cheat
catalog by syntactic spelling, not by new conceptual cheats. The lowercase-`asm`
gap alone is the kind of "1-character spelling bypass" that proves the
catalog-is-open posture.

**The 25 should be added to the active queue.** Each will need its own
investigation — most are likely 1-2-cheat removals with pure-C levers in the
existing rule catalog (Lever A/B/C, narrow types, register-alloc-pure-c
playbook). `func_80068F70` will need substantial re-derivation. `tslPolyF4Init`
needs the user's policy call on the dead-goto label pad (the
`global-label-drift-sibling-cheat` rule has it as deliberate; this audit's
finding is that it now reads as cheat-by-spelling).

### Confidence assessment

**85% confidence that the major historical cheat-by-spelling families are now
characterized.** Reasoning:

- Phase B scanned 24 brainstormed patterns. The 4 productive ones are all
  syntactic-spelling variants of patterns the existing detector set already
  knows about (empty-if, void-discard, unused array, frame coercion). New
  CONCEPTUAL cheats (a totally novel mechanism) seem unlikely given the
  catalog's open-classness.
- Phase C manually reviewed 30 functions spread across all 22 source files.
  Found ONE additional construct (the DImode chain in `tslPolyF4Init`) that
  isn't a known cheat family. The signal-to-noise of manual review is
  diminishing — at 30 functions reviewed, we're finding ~1 new family per
  hour of careful reading.
- The remaining 15% uncertainty: there could be:
  - A handful more `tslPolyF4Init`-style "deliberate scaffolding for codegen
    that's documented in a rule as load-bearing but doesn't survive the
    `no-new-park-categories` policy lens." Re-reading every `.claude/rules/`
    entry against today's policy might surface 1-3 more.
  - Patterns specific to certain types of code (display.c GTE wrappers,
    text1b.c animation logic) that didn't show up in this sample.
  - Detector edge cases — e.g. `(void) (volatile int)0;` or other obscure
    spellings.

**Concrete next-step hypothesis: if Phase 2 surfaces MORE cheats:**

If during Phase 2 a worker discovers a NEW cheat-by-spelling pattern in
a function they're working on, that pattern should be:
1. Added to this memory as "Phase 2 follow-up addition"
2. Cross-checked against all other COMPLETED-C functions via a one-shot scan
3. If 3+ NEW affected functions surface, run a Phase 5 of this audit.

If <3 affected functions surface per new pattern across the whole Phase 2
campaign, the floor probably IS 25 + a small additive (5-10) factor — the
search has converged.

## Action items (concrete)

1. **Detector implementation pass** — Land in `engine/volatile_cheats.py`
   (4 new detectors per Priorities 1-4 above) + `tools/classify_inline_asm`
   (lowercase-asm coverage). Add regression tests in `engine/test_engine.py`
   covering each new pattern. Run `queue regen` after landing to re-route
   the 25 affected COMPLETED-C functions to `active`.

2. **Phase 2 re-derivation** — work through the queue starting from
   easiest-distance items (typical decomp loop) but with full awareness that
   the 25 will surface. The audit's Phase 1 confirmed-load-bearing
   measurements ([[ub-cheat-by-spelling-audit-2026-06-02]] Phase 3) suggest
   most are within 0-44 honest pure-C distance of close.

3. **User policy decisions** needed for:
   - `tslPolyF4Init`'s dead-goto label pad (is `.L`-counter-preservation
     a sanctioned pure-C mechanism, or a cheat?).
   - The 13 functions where the cheat is "1-line addition" (likely single-
     lever re-derivable) vs the 12 functions where the cheat is structural
     (likely need full re-investigation). Triage at Phase 2 kickoff.

## Probe scripts (preserved in tmp/, gitignored)

| Script | Purpose |
|---|---|
| `tmp/audit_phase_a.py`, `tmp/audit_phase_a2.py` | Phase A — comprehensive detector run, per-function attribution |
| `tmp/audit_phase_b.py` | Phase B — 24-pattern brainstorm scan |
| `tmp/show_phase_b_hits.py` | Phase B drill-down (P08, P17, P23) |
| `tmp/audit_asm_lowercase.py` | Phase B - lowercase asm specifically |
| `tmp/check_array_candidates.py` | Validation: each `audit_project` "unknown" hit → owning function + queue status |
| `tmp/check_dowhile_owners.py` | Phase B/C — do-while(0) variants by owning function |
| `tmp/sample_completed.py`, `tmp/sample_output.txt` | Phase C — stratified COMPLETED-C sample |
| `tmp/aggregate_audit.py`, `tmp/aggregate_audit.json` | Phase D — deduplicated aggregate |
| `tmp/probe_asm_kw.py` | One-off probe for ASM_KEYWORD_RE coverage |

## Related

- [[ub-cheat-by-spelling-audit-2026-06-02]] — the 3-finding prior audit this audit expands
- [[no-new-park-categories]] — the standing policy this audit enforces ("cheats by any spelling")
- [[inline-asm-policy]] — the expanded cheat catalog this audit cross-checks against
- [[completion-standard]] — the COMPLETED-C bar the 25 affected functions currently violate
- [[inline-asm-injection]] — the regex-spelled-spelling sibling cheat family
- [[dead-vars-local-array]] — the frame-coercion family the volatile-local case extends
- [[register-alloc-pure-c]] — the playbook the 25 Phase-2 re-derivations will draw on
- [[global-label-drift-sibling-cheat]] — documents `tslPolyF4Init`'s dead-goto pad as
  deliberate; this audit reads it as cheat-by-spelling under the 2026-06-01 policy
