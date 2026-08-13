# CdControlF — evidence ledger

## Session 1 (recon, 2026-08-13)

### Baseline and shape
- `canonical CdControlF` → verdict **C**, `asm_insns 0`, `total 75`, distance 23.
  Pure-C target; no canonical-asm question exists for this function.
- Honest floor at session start: **23** (`sandbox CdControlF --disable all`).
- `src/system.c` at HEAD carried a **register pin** — `register s32 result asm("s6");`
  — inside CdControlF. The sandbox strips it, so the pin was buying nothing:
  the pin-free form (`s32 result;`, everything else identical) also scores 23.
  The session's candidate form deletes the pin outright.

### The residual is 100% register assignment — the structure is already exact
Target and our build both emit **75 instructions**, in the same order, with the
same branches, the same jal sites, the same frame size (0x30) and the same eight
saved registers (s0–s6, ra). Every one of the 23 differing instructions differed
only in which callee-saved register a value lives in. Confirmed by disassembling
`tmp/sandbox/CdControlF/system.o` and comparing to `asm/funcs/CdControlF.s`
(`tmp/grind/CdControlF/s1/build_CdControlF.txt`).

Target's assignment (from `asm/funcs/CdControlF.s`):

| value | target |
|---|---|
| `count` (=3, loop counter) | s0 |
| `a1` (param copy) | s1 |
| `idx` (`a0 & 0xFF`) | s2 |
| `a0` (raw param copy) | s3 |
| `saved` (old `g_cd_callback_a`) | s4 |
| `elem` (`&g_cd_sector_buf[idx]`) | s5 |
| `result` (0 / -1, returned as `result + 1`) | s6 |

Session-start build: s0=count, s1=a1, s2=result, s3=idx, s4=a0, s5=saved,
s6=elem — i.e. `result` was allocated too EARLY, displacing the other four
callee-saved values one register up.

### Measured levers (all pure C, no constructs added)
1. **Init-statement order is the live lever.** Full 60-permutation sweep of the
   five initialiser statements (constraint: `idx` before `elem`) moved the score
   over the range **14 … 31**. Eight distinct orders reach 14; every one of them
   has `result` initialised first-or-early and `elem` initialised LAST.
   Artifact: `tmp/grind/CdControlF/s1/sweep_results.json`.
2. **Declaration order is INERT.** With the init order pinned to a 14-scoring
   winner, all **120** declaration permutations scored exactly 14. GCC 2.7.2
   creates the pseudo at first *use*, not at the declaration, so declaration
   order carries no signal for these scalars.
   Artifact: `tmp/grind/CdControlF/s1/sweep2_results.json`.
3. **The `base` two-step is worth 3 more points.** Replacing
   `elem = &g_cd_sector_buf[idx];` with the matched sibling CdControlB's
   spelling — `base = g_cd_sector_buf; elem = base + idx;` — drops 14 → **11**.
   Mechanism (verified in the disassembly): the one-step form emits
   `sll v1,s2,2 / lui v0 / addiu v0,v0,0 / addu elem,v1,v0`; target emits
   `lui v1 / addiu v1,v1,0 / sll v0,s2,2 / addu elem,v0,v1`. The two-step makes
   the symbol materialise into v1 and the scaled index into v0, matching target.
   Artifact: `tmp/grind/CdControlF/s1/sweep3_results.json` (variant
   `A_base_two_step`).
4. **Re-sweeping init order over the 6-local two-step form** (240 legal
   permutations) confirms 11 is that form's floor; the winning orders all have
   `result` first-or-second and `base`,`elem` as the last two inits.
   Artifact: `tmp/grind/CdControlF/s1/sweep4_results.json`.
5. **Neutral / dead spellings** (all still 14 at the one-step floor):
   `elem[0]` instead of `*elem`; byte-offset pointer math
   `(s32 *)((u8 *)g_cd_sector_buf + idx * 4)`; `(u8)a0` instead of `a0 & 0xFF`
   for both `idx` and the CD_cw argument.
6. **The inherited `new_var` / `new_var2` DImode chain**
   (`int new_var = 3; unsigned long long new_var2 = new_var; count = new_var2;`)
   is NOT load-bearing at the new optimum — it exists only in the 23-scoring
   HEAD form, and the 11-scoring candidate reaches its floor with a plain
   `count = 3;`. That construct is gone from the candidate.

### Where the 11-point residual sits (exact)
Best form (`memory/grind/CdControlF/candidate.c`, applied to src/system.c):
s0=count ✓, s1=a1 ✓, s2=idx ✓, s3=a0 ✓, s4=saved ✓, **s5=result (target: s6)**,
**s6=elem (target: s5)**. v0/v1 scratch usage now matches target exactly. The
entire remaining distance is that ONE pairwise swap: `result` and `elem` are
allocated in the wrong order. Artifact:
`tmp/grind/CdControlF/s1/build11_CdControlF.txt`.

### Mechanism for the remaining swap (GCC 2.7.2 `global.c`)
`global_alloc` sorts allocnos with `allocno_compare` (global.c:635):

    pri = (floor_log2(n_refs) * n_refs / live_length) * 10000 * size

and then assigns hard registers in that order, each allocno taking its first
acceptable register. `result` has 2 sets (`= 0`, `= -1`) plus its read in
`return result + 1` → n_refs 3 → `floor_log2(3)*3 = 3`. `elem` has 1 set plus 1
read (`*elem` in the loop) → n_refs 2 → `floor_log2(2)*2 = 2`. So `result`
outranks `elem` and grabs the lower register (s5) — target has it the other way,
so in the original source `elem` must outrank `result`, i.e. `elem` carries more
references and/or `result` a longer live range than our spelling produces.

### Sibling context
- `CdControlB` (src/system.c:228) is the **matched, COMPLETED-C** near-twin of
  this function (not in engine/queue.json). It is the provenance for the `base`
  two-step spelling used in the candidate — this is not a construct invented to
  move bytes, it is the shape the already-matched sibling in the same file uses.
- `CdControl` (src/system.c:134) is the other near-twin: still ACTIVE in the
  queue at distance 25 and still carrying its own `register s32 result asm("s7")`
  pin. It is the same three-way family (CdControl / CdControlF / CdControlB) and
  will almost certainly close by the same lever — worth re-testing there once
  CdControlF lands.

- [s1] canonical CdControlF -> verdict C, asm_insns 0, total 75, distance 23. No canonical-asm question exists for this function.

- [s1] Honest floor moved 23 -> 11 this session with pure statement ordering and a spelling borrowed from an already-matched sibling; no construct was added to the source (the candidate has strictly FEWER constructs than HEAD: the register pin and the DImode chain are both gone).

- [s1] Target register map (from asm/funcs/CdControlF.s): s0=count(3), s1=a1, s2=idx(a0&0xFF), s3=a0 raw copy, s4=saved g_cd_callback_a, s5=elem, s6=result.

- [s1] Candidate register map at floor 11: s0=count, s1=a1, s2=idx, s3=a0, s4=saved all MATCH; only s5=result / s6=elem are swapped relative to target. All 11 remaining differing instructions mention one of those two values.

- [s1] v0/v1 scratch usage in the address computation now matches target exactly, thanks to the base two-step.

- [s1] GCC 2.7.2 global.c:635 allocno_compare ranks allocnos by floor_log2(n_refs)*n_refs/live_length. Hand-count: result has 3 refs (two sets + one read of `result + 1`) -> rank 3/L; elem has 2 refs -> rank 2/L. That ordering is why result takes the lower register; the original source must give elem the higher rank.

- [s1] CdControlB (src/system.c:228) is a COMPLETED-C near-twin, absent from engine/queue.json — it is the provenance for the base two-step spelling, which is therefore an already-matched in-file idiom rather than an invented construct.

- [s1] CdControl (src/system.c:134) is the third member of the family: queue-active at distance 25 and still carrying its own register s32 result asm("s7") pin. It is likely to fall to the same lever once CdControlF closes.

## Session 2 (structural, 2026-08-13) — floor 11 → 9, and the residual is now
## quantified exactly

### The floor moved on the parameter-type axis (the ledger's F2)
Typing the first parameter `u8` — the Sony libcd `int CdControlF(u_char com,
u_char *param)` shape the s1 frontier flagged — drops the honest floor
**11 → 9**. It is the only lever in this session that moved the floor.
The masking spelling is then irrelevant: `idx = a0;` / `CD_cw(a0, ...)`,
`idx = a0 & 0xFF;` / `CD_cw(a0 & 0xFF, ...)` and both mixtures all score 9
(the mask is a no-op on a `u8`, and GCC re-materialises the two `andi` that
target has at exactly target's two sites). Typing `a1` as `u8 *` is also
neutral at 9. `s8 com` and `u16 com` regress to 16; a `u8 idx` LOCAL (as
opposed to a u8 param) regresses to 27.
Artifacts: `tmp/grind/CdControlF/s2/variants_a.json`, `variants_b.json`.

### The forward prototype was updated and measured codegen-neutral
`src/system.c:1012` carried `extern s32 CdControlF(s32, s32);`, contradicting
the new `u8` definition. It is now `(u8, s32)`. All five callers
(`D_80082050`, `D_80082320`, `func_8008241C`, `CdReadBreak`, `CdRead`) score
sandbox distance **0 both before and after** — every call site passes an
integer literal, so the argument setup is `li aN, K` either way.
Artifact: `tmp/grind/CdControlF/s2/proto_check.py`.

### What the remaining 9 is, instruction by instruction
`tmp/grind/CdControlF/s2/diff2.py` (normalized through the engine's own
`engine.score.normalized_insns`, so both sides are formatted identically —
the raw objdump-vs-asm-source diff is pure syntax noise) reports 75 vs 75
instructions and exactly nine edits:

    insert  5  ours has  sw s5,36(sp)          (result's save, emitted early)
    insert  6  ours has  move s5,zero          (result = 0, emitted early)
    delete 15  target has sw s5,36(sp)
    delete 16  target has addu s5,v0,v1        (elem, emitted late)
    replace 18 T: move s6,zero      O: addu s6,v0,v1
    replace 37 T: lw v0,0(s5)       O: lw v0,0(s6)
    replace 55 T: addiu v0,s6,1     O: addiu v0,s5,1
    replace 62 T: li s6,-1          O: li s5,-1
    replace 63 T: addiu v0,s6,1     O: addiu v0,s5,1

i.e. `result` and `elem` are in each other's register and nothing else differs.

### The allocno priority arithmetic, MEASURED (not hand-counted)
The prebuilt `tools/gcc-2.7.2/cc1` is instrumented; `BB2_ALLOC_DEBUG=1` prints
`ALLOCDBG func=%s ord=%d pseudo=%d hardreg=%d nrefs=%d livelen=%d pri=%d`
(driver: `tmp/grind/CdControlF/s2/allocdbg.sh`). On the floor-9 form:

| ord | pseudo | value  | hardreg | nrefs | livelen | pri  |
|-----|--------|--------|---------|-------|---------|------|
| 0   | 78     | count  | s0      | 4     | 35      | 2285 |
| 1   | 74     | a1     | s1      | 4     | 37      | 2162 |
| 2   | 76     | idx    | s2      | 3     | 34      | 882  |
| 3   | 72     | a0     | s3      | 3     | 37      | 810  |
| 4   | 77     | saved  | s4      | 3     | 37      | 810  |
| 5   | 75     | result | s5      | 3     | 39      | 769  |
| 6   | 80     | elem   | s6      | 2     | 31      | 645  |

`pri = floor_log2(nrefs)*nrefs / livelen * 10000 * size` (global.c:635,
`allocno_compare`), size is 1 for every one of these. Ties break on the
allocno index (`return *v1 - *v2;`), i.e. on the **pseudo number**.

**Therefore the closing condition is arithmetic and exact:** the first five
rows already match target. To swap the last two, `elem`'s priority must land
strictly inside **(769, 810)** — above `result`, below `saved`/`a0` — or
`result`'s must drop strictly below **645**.

### Pseudo numbers follow DECLARATION order (correction to s1's finding)
s1 recorded "declaration order is INERT" (true of the score) and inferred
"GCC creates the pseudo at first use". The inference is wrong: `expand_decl`
assigns each local its pseudo at its DECLARATION, and reordering the
declarations demonstrably renumbers them (variant `D4_decl_elem_first`:
`elem` becomes pseudo 76 and `result` 77, with every nrefs/livelen/pri
unchanged, score still 9). Declaration order is inert only because there is
currently no TIE for it to break. It is the available lever the moment a
spelling makes `pri(elem) == pri(result)`.

### The (nrefs, livelen) pair for result/elem is invariant under the whole
### structural catalog
Measured with ALLOCDBG, all still `result 3/39` and `elem 2/31`, all still 9:
inner-block scoping of the loop locals; inner-block scoping of `base`/`elem`
alone; flattening the `idx != 1` / `g_cd_mode` nested ifs into one `&&`;
flattening the `a1` / `*elem` / `CD_cw` chain into one `&&`; `if (--count !=
-1)` instead of `count--; if (count != -1)`; `elem[0]` instead of `*elem`;
dereferencing into a named local first; `a1 != 0 && *elem != 0` combined.
Artifacts: `variants_c.json`, `variants_d.json`, `variants_e.json`.

### The init-order axis is re-measured and flat at 9 on the u8 base
All 240 legal init orders re-run over the u8 form: floor 9, reached by every
order with `result` first-or-second and `base`,`elem` last; `result` last
scores 20-24. Artifact: `sweep_c.json`. Init order changes livelens (initialise
`elem` early and its livelen rises 31 → 34 while `result`'s falls 39 → 35,
which moves both the WRONG way), but it can never produce the (769, 810)
window with `elem` at 2 refs.

### Two hard bounds that close off the "just shrink/stretch a live range" idea
- `elem` at nrefs=2 needs livelen 25 for pri ∈ (769, 810). `elem` is live
  across the loop back edge, so its live range can never be shorter than the
  loop body (~28 RTL insns); the smallest livelen observed in any spelling is
  31. **Unreachable.**
- `result` at nrefs=3 needs livelen ≥ 47 to fall below 645. The whole function
  is ~40 RTL insns (the longest live range of any pseudo, `result`'s own, is
  39). **Unreachable.**

So the only two live doors are `result` at 2 references, or `elem` at exactly
3 references with livelen ~38.

### Both doors were probed and the mechanism is confirmed
- `D1_probe_literal_zero_tail` (`return 0;` in the failure tail instead of
  `result = -1; goto done;`) drops `result` to 2 references. `result` then
  disappears from the allocno list entirely (GCC const-props it) and **`elem`
  takes s5 — target's register**. Score stays 9 because the tail loses the
  `li s6,-1 / addiu v0,s6,1` pair. Same for `D2` (`return 1;` on the success
  exit), score 12. These are PROBES, not candidates: they confirm the model
  predicts the swap, and they show why a 2-reference `result` is hard —
  `result = 0`, `result = -1` and `return result + 1` are all three
  semantically required to emit target's tail.
- `D3_probe_second_elem_read` (a second genuine `*elem` read in the other arm
  of the `a1` test) does give `elem` nrefs=3, but it also adds instructions, so
  every livelen inflates together and `elem` lands at pri 810 / ord 3 (s3).
  Score 26. A ref lift only works if it is BYTE-NEUTRAL.

- [s1] Tooling note for future sessions: Windows python3 on this machine is 3.12, whose pathlib.Path.read_text() rejects the newline= kwarg. A patch helper that used it failed silently under a PowerShell Out-Null pipeline and produced a full batch of bogus all-identical scores. The banked helpers (tmp/grind/CdControlF/s1/patch.py, sweep*.py) use open(..., newline='') and are correct; the sweeps run inside one WSL session via sweep*.sh, which is both faster and avoids the wsl.exe job-object leak.

- [s2] [s2] Honest floor 11 -> 9. The lever is the parameter type: s32 CdControlF(u8 a0, s32 a1) instead of (s32, s32). This is the ledger's own F2 hypothesis (the Sony libcd `int CdControlF(u_char com, u_char *param)` prototype), now confirmed as worth 2 points.

- [s2] [s2] Once a0 is u8 the masking spelling is completely neutral: idx = a0 and idx = a0 & 0xFF, CD_cw(a0,...) and CD_cw(a0 & 0xFF,...), and typing a1 as u8 * all score exactly 9. s8 com and u16 com regress to 16. A u8 LOCAL idx (as opposed to a u8 param) regresses to 27.

- [s2] [s2] The forward declaration at src/system.c:1012 was updated from `extern s32 CdControlF(s32, s32);` to `(u8, s32)` so it no longer contradicts the definition. Measured codegen-neutral: D_80082050, D_80082320, func_8008241C, CdReadBreak and CdRead all score sandbox distance 0 both before and after (every call site passes an integer literal, so argument setup is `li aN, K` either way). Artifact tmp/grind/CdControlF/s2/proto_check.py.

- [s2] [s2] The residual at floor 9 is 75 vs 75 instructions with exactly nine edits, all of them `result` and `elem` occupying each other's callee-saved register (ours s5=result/s6=elem, target s5=elem/s6=result) plus the resulting shift of result's save slot and its `move sN,zero` to the head of the prologue. Diff produced through engine.score.normalized_insns so both sides format identically (tmp/grind/CdControlF/s2/diff2.py).

- [s2] [s2] TOOLING: the prebuilt tools/gcc-2.7.2/cc1 is the INSTRUMENTED build. `BB2_ALLOC_DEBUG=1` makes it print `ALLOCDBG func=%s ord=%d pseudo=%d hardreg=%d nrefs=%d livelen=%d pri=%d` for every allocno of every function — i.e. global.c's allocno_compare inputs and output, directly readable. Driver: tmp/grind/CdControlF/s2/allocdbg.sh. Other hooks present in the binary: BB2_FRAME_DEBUG, BB2_XJUMP_DEBUG, BB2_FLOW_DEBUG, BB2_SUGG_DEBUG, BB2_QTY_DEBUG, BB2_FINDREG_DEBUG, BB2_RELOAD_DEBUG, BB2_DBR_DEBUG, BB2_SCHED_DEBUG, BB2_PRIO_DEBUG, BB2_RANK_DEBUG, BB2_SLL_DEBUG. This replaces hand-counting references for any register-allocation grind.

- [s2] [s2] MEASURED allocno table at floor 9: count nrefs=4 livelen=35 pri=2285 -> s0; a1 4/37 2162 -> s1; idx 3/34 882 -> s2; a0 3/37 810 -> s3; saved 3/37 810 -> s4; result 3/39 769 -> s5; elem 2/31 645 -> s6. Target wants elem at s5 and result at s6, and every other row already matches.

- [s2] [s2] Therefore the closing condition is exact and arithmetic: pri(elem) must land strictly inside (769, 810), or pri(result) strictly below 645, where pri = floor_log2(nrefs)*nrefs / livelen * 10000 (size is 1 for all seven). Ties break on the allocno index, i.e. on the pseudo number.

- [s2] [s2] elem at 2 references can NEVER win: it would need livelen 25, but it is live across the loop back edge so its live range is bounded below by the loop body; 31 is the minimum observed across every spelling measured, giving a hard ceiling of pri 714 < 769.

- [s2] [s2] result at 3 references can NEVER lose: it would need livelen 47, but the whole function is ~40 RTL insns and result's 39 is already the longest live range in it.

- [s2] [s2] So exactly two doors remain: (a) elem with EXACTLY 3 references and livelen ~38 (4 references overshoots to pri 2352 and takes s0), or (b) result with 2 references (pri 512, well clear of elem's 645).

- [s2] [s2] Door (b) is confirmed to produce target's register layout: with `return 0;` in the failure tail, result const-props away entirely and elem takes s5. It is not usable as written because it also deletes target's `li s6,-1 / addiu v0,s6,1` tail pair. The next session's job on this door is a spelling where one of the two constant stores belongs to a different, non-overlapping pseudo that local_alloc handles and that never becomes an allocno — `base` already demonstrates that shape (a real local that never appears in the global allocno list; local_alloc gives it v1).

- [s2] [s2] Door (a) needs the extra reference to be BYTE-NEUTRAL: adding a genuine second *elem read (probe D3) does give nrefs=3, but it also adds instructions, inflating every livelen together so elem lands at pri 810 / s3 and the score goes to 26.

- [s2] [s2] CORRECTION to the s1 ledger: pseudo numbers follow DECLARATION order (expand_decl), not first-use order. Reordering the declarations renumbers the pseudos with every nrefs/livelen/pri unchanged. Declaration order is score-inert only for want of a tie to break; it is the ready-made lever the moment a spelling equalises pri(elem) and pri(result).

- [s2] [s2] The (nrefs, livelen) pair for result and elem is invariant under the entire structural catalog measured this session: inner-block scoping of the loop locals, inner-block scoping of base/elem alone, `idx != 1 && (g_cd_mode & 0x10)`, `a1 != 0 && *elem != 0 && CD_cw(...) != 0`, `if (--count != -1)`, `elem[0]` instead of `*elem`, and dereferencing into a named local first. All seven still score 9.

- [s2] [s2] The goto-chain loop body is now fixed evidence, not an assumption: every real-loop spelling regresses (do/while 18, for 22, while(1)+break 18 against a base of 11).

- [s2] [s2] Harness for the next session: tmp/grind/CdControlF/s2/harness.py provides splice()/score()/sweep() against src/system.c with guaranteed restore, and variants_c.py provides allocdbg() returning the parsed ALLOCDBG table. A new probe batch is just a list of (name, body) pairs; measure the ALLOCDBG table, not only the score, because most spellings are score-flat at 9 while still being informative about nrefs/livelen.

## Session 3 (structural, 2026-08-13) — floor 9 → 4 → **0**. CLOSED.

### The missing term in the s2 model: `reg_n_refs` is loop-depth WEIGHTED
Sessions 1 and 2 modelled `nrefs` as a plain count of references. It is not.
`tools/gcc-2.7.2/flow.c` accumulates **`reg_n_refs[regno] += loop_depth`** at
four sites (flow.c:2081, 2329, 2515, 2725), and `loop_depth` is derived from
`NOTE_INSN_LOOP_BEG` / `NOTE_INSN_LOOP_END` notes (flow.c:1385-1453, filling
`basic_block_loop_depth`). So **every reference that sits inside a loop-note
region counts TWICE**, while `reg_live_length` is untouched by depth.

`tools/gcc-2.7.2/toplev.c` pins the pass order this depends on:
`cse_main` (2865) → `loop_optimize` (2895) → `cse2` (2926) → **`flow_analysis`
(2983)** → `combine_instructions` (3004) → `schedule_insns` (3033) →
`local_alloc` (3052) → `global_alloc` (3080). `reg_n_refs` / `reg_live_length`
are fixed at 2983 and never recomputed, so anything combine or sched later
folds STILL COUNTS, while anything cse/loop folds does not.

This is a general instrument, not a CdControlF fact: a loop-note region is a
per-REGION multiplier on `global.c:635 allocno_compare`'s numerator, with no
effect on its denominator.

### Why the s2 frontier (a byte-neutral third `elem` reference) was the wrong door
Measured directly (batch A, `tmp/grind/CdControlF/s3/variants_a.json`): lifting
`elem` ALONE to 3 references via a minimal loop-note region gives
`nrefs=3 livelen=31 pri=967`, which is ABOVE `idx` (882) — `elem` takes s2, and
the score regresses 9 → 19. The s2 frontier's target window "livelen ~38" is
unreachable for a different reason than s2 recorded: livelen is not moved by
ref-lifting at all, and `elem`'s live range cannot reach 37-39 without a
reference in the post-loop TAIL — and `asm/funcs/CdControlF.s` shows target's
tail never touches s5, so no such reference exists in the original either.
The door was mis-specified because the model was missing the depth weight.

### The right door: size the loop-note region to the WHOLE loop body
Predicted by hand before measuring (see `tmp/grind/CdControlF/s3/variants_b.py`
docstring) and reproduced exactly. Wrapping the retry-loop body — everything
from `g_cd_callback_a = 0;` through the `if (count != -1) goto loop;` back
branch — in a `do { ... } while (0);` region doubles exactly the references that
need doubling:

| value  | refs in/out of region | nrefs | livelen | pri  | hardreg |
|--------|-----------------------|-------|---------|------|---------|
| count  | 1 out + 2×2 + 2       | 7     | 35      | 4000 | s0 ✓ |
| a1     | 1 out + 2 + 2 + 2     | 7     | 37      | 3783 | s1 ✓ |
| idx    | 1 out + 2 + 1 out     | 4     | 34      | 2352 | s2 ✓ |
| a0     | 1 + 1 out + 2         | 4     | 37      | 2162 | s3 ✓ |
| saved  | 1 out + 2 + 1 out     | 4     | 37      | 2162 | s4 ✓ |
| elem   | 1 out + 2             | 3     | 31      |  967 | s5 ✓ |
| result | 3 refs, ALL outside   | 3     | 39      |  769 | s6 ✓ |

= target's s0..s6 in one step. `result` is the only value whose three references
all lie outside the region (`result = 0` before the loop, `result = -1` and
`return result + 1` after it), which is precisely why it alone stays at the
bottom of the order. Floor **9 → 4**.

**Region sizing is load-bearing** (batch B, `variants_b.json`): whole loop body
including the decrement and back branch → 4; body only, decrement outside → 16
(count keeps 4 refs and falls below a1); region opened before the `loop:` label
so the label sits inside → 16 plus two spilled extra allocnos.

### The last 4 was the prologue schedule, and init order was free again
At floor 4 the normalized diff was 75 vs 75 with a single displaced pair:
ours emitted `sw s6,40(sp)` / `move s6,zero` at prologue positions 5-6, target
emits them at 17-18. That is `result = 0;` being the FIRST init statement in our
C and the LAST in the original. Session 2 had measured "result first-or-early,
result-last scores 20-24" — true, but only on the OLD basis where init order WAS
the allocation lever. With the allocation pinned by the region the axis is free,
so all 240 legal init orders were re-swept (`sweep_c_d_result_first.json`).

**`idx, saved, count, base, elem, result`** (i.e. `result = 0;` last) → **0**.
Reached at permutation 58 of 240; the sweep stops on zero.

### Final state, verified this session
`sandbox CdControlF --disable all` → `{"score": 0, "target_insns": 75,
"build_insns": 75, "rules_dropped": 0}` with the edits in place in `src/system.c`.
The four in-file neighbours re-measured with the edits applied: `CdRead` 0,
`CdReadBreak` 0, `CdControlB` 0. `src/system.c` remains LF.

The committed body carries strictly FEWER constructs than HEAD did: the
`register s32 result asm("s6")` pin and the `new_var`/`new_var2` DImode chain are
both gone; what is added is one FAKE-annotated single-level `do { } while (0);`
region, the `u8` parameter type, the `base` named intermediate (the matched
sibling `CdControlB`'s own spelling at src/system.c:237-238) and a statement
order. Self-vet: `memory/grind/CdControlF/self_vet.md`.

### Ported lever for the sibling `CdControl` (queue-active, distance 25)
`CdControl` is the same function with a third argument and still carries a
`register s32 result asm("s7")` pin. Its measured allocno table (from the same
dump, `tmp/grind/CdControlF/s3/allocdbg_all.txt`) is
count 4/31=2580, a1 4/38=2105, 3/37=810, 3/38=789, 3/38=789, 3/39=769, 2/34=588
— the same shape as CdControlF's pre-close table, with `result` again third from
last and `elem` last. The whole CdControlF recipe (u8 `com`, `base`/`elem`
two-step, whole-loop-body do-while(0) region, then a 240-order init sweep) is
the obvious first attack there.
