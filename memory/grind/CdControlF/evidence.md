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

- [s1] Tooling note for future sessions: Windows python3 on this machine is 3.12, whose pathlib.Path.read_text() rejects the newline= kwarg. A patch helper that used it failed silently under a PowerShell Out-Null pipeline and produced a full batch of bogus all-identical scores. The banked helpers (tmp/grind/CdControlF/s1/patch.py, sweep*.py) use open(..., newline='') and are correct; the sweeps run inside one WSL session via sweep*.sh, which is both faster and avoids the wsl.exe job-object leak.
