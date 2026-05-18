---
name: exec-game-plateau
paths: ["src/main.c"]
description: "exec_game (194-insn in-place SPU voice list compaction in main.c) — the canonical cheat orphan that resists pure-C retirement. Pin chain matches target's register allocation; D_800A2D3C cache-defeat is the unyielding limit. 104 regfix rules + 1 lost_codegen cheat. Also documents the tools/lessons that emerged from matching it."
metadata:
  type: recipe
---

# exec_game — pin-chain limit + cache-defeat impossibility

`exec_game` is a 194-instruction 5-phase in-place selection-sort/compaction routine in `src/main.c`. It is the **first BB2 cheat orphan demonstrated to genuinely resist pure-C retirement** via the standard techniques (pin chain + inline-move-aliasing). Useful as a calibration point: not every lost_codegen cheat is retirable, and when one is not, the cause is identifiable (compiler cost-model divergence vs. structural C issue).

## Current state

- 104 regfix rules active
- 1 lost_codegen cheat: `insert "addu $8,$10,$zero" @ 82` (= `move t0, t2`)
- Function body in `src/main.c:1805`
- Leaf, no calls, no stack frame

## Function shape

Five phases, all in-place operations on a global array (`D_800A2D40`) governed by a global count (`D_800A2D3C`):

1. **Phase 1** — merge flagged entries with next matching flagged
2. **Phase 2** — mark zero-value entries as sentinels
3. **Phase 3** — selection sort (has the lost_codegen)
4. **Phase 4** — compact one sentinel, update count
5. **Phase 5** — finalize backward, fold counts

## What works — V40 pin chain

The V40 variant (saved at `tmp/v40_phase1_pin_chain.c`) reproduces target's full Phase-1 register allocation:

```c
if (D_800A2D3C >= 0) {
    register Entry *p1_base asm("t0") = (Entry *)D_800A2D40;
    register Entry *p1_cur  asm("a3") = p1_base;
    register u32 p1_sent    asm("t2") = 0x2FFFFFFFU;
    register u32 p1_maskl   asm("t3") = 0x0FFFFFFFU;
    register u32 p1_maskh   asm("t4") = 0x80000000U;
    i = 0;
    do {
        p1_cur++; p1_cur--;          /* LICM defeat for cur+4 */
        ...
        p1_base = (Entry *)D_800A2D40;  /* forces re-load each iter */
    p1_cont: ;
    } while (D_800A2D3C >= i);
}
```

GCC emits `lui t4, 0x8000` (MASKH), `lui t2, 0x2fff` (SENT), `lui t3, 0xfff` (MASKL), `lw t0, D_800A2D40` (base), `move a3, t0` (cur) — **all matching target's exact registers.**

## What doesn't work — the D_800A2D3C cache defeat

Even with the pin chain, GCC inserts `move t5, v0` (or `t6, v0` depending on what's free) at function entry to cache `D_800A2D3C`'s value across the Phase 1 loop. Target does NOT cache — it re-reads via `lui+lw` at the loop back-edge.

Defeat techniques tried, all failed:

| Technique | Result |
|-----------|--------|
| `volatile s32 D_800A2D3C` global | Forces re-read EVERYWHERE (over-aggressive) |
| `*(volatile s32*)&D_800A2D3C` per-access cast | Same effect |
| `__asm__("":::"memory")` barrier | Cache shifts position but persists |
| `__asm__("":::"$12")` register clobber | Cache shifts to different register but persists |
| Scope-isolating the initial check | Optimizer sees across scopes |
| V41: `__asm__ volatile("" ::: "$2")` clobber after initial check | BROKEN — confused branch scheduling; bltz delay slot filled with `j 5acc` (self-jump infinite loop). Reported "133 diffs" but non-functional. |
| V42: clobber moved to loop bottom (`p1_cont`), `"$2", "memory"` | 185 diffs. Cache maybe defeated but register cascade re-cascaded badly. |
| V43: dummy pin (`register s32 p1_consume_t5 asm("t5")` + `__asm__ volatile("" : "+r"(p1_consume_t5))`) to consume `$t5` | 172 diffs. Dummy pin caused GCC to silently UN-HONOR the other pins — base ended up in `$a3`, SENT in `$t3`, MASKL in `$t2`. Pin chain collapsed. |

**Why it's stuck:** GCC's cost model decides caching (one `v0 → $tN move`) is cheaper than re-reading (`lui+lw+nop` each iteration). PsyQ's cc1psx made the opposite decision. This is a compiler-internal cost-model divergence that pure C cannot reach without rebuilding GCC with PsyQ's specific cost-model patches.

## The hard limit

Pure C with the pin chain reaches ~189 diffs (no rules) or 0 diffs (with the 104 regfix rules). No middle ground in pure C — the cache insertion is a SINGLE instruction whose presence cascades to 189+ apparent diffs.

The 104 regfix rules effectively encode "shift everything by 1 + rename registers in the cascade" — they work around the cache by accepting it.

## What the next attempt should try

1. **Start from V40 base** (`tmp/v40_phase1_pin_chain.c`) — already has the working pin chain
2. **Reorder Phase 1 prologue** — even with pins, GCC schedules `lui`/`ori`/`lw` in a different order than target. Target's order: `i, MASKH, SENT, MASKL, base, cur`. Mine's order: `MASKL, base, SENT, MASKH, cur`. May respond to declaration reordering or `volatile` hint manipulation.
3. **Apply the same pin pattern to Phase 3** (the lost_codegen `move t0, t2` location) — V33 showed the move emits correctly with `register Entry *outer asm("t2")` + inline-move-aliasing, just at the wrong source register. Combined with Phase 1's correct cascade, may align.
4. **Iterate per-phase** with `dc.sh verify` after each change, observing exactly which diffs vanish.

Realistic remaining options to reach community-standard zero-rules:

- Rebuild GCC with PsyQ's specific cost-model patches (out of scope for the BB2 project — see [[cc1psx-calibration-only]])
- A C source structure no agent has discovered in 5000+ permuter iterations + 40+ manual variants (low probability)
- Accept that this function is at the boundary of what's reachable with the current toolchain

## Saved artifacts

| Path | Description |
|------|-------------|
| `tmp/v40_phase1_pin_chain.c` | Full main.c with Phase 1 pin chain applied (semantic-clean; doesn't reduce rule count alone) |
| `tmp/v29_best_clean.c` | Minimal pin (just outer to `$t2`), permuter score 2805 |
| `tmp/v21_clean_2675.c` | Simple tricks only (no pins), permuter score 3255 |
| `tmp/exec_game_findings.md` | Full analysis document |
| `permuter/exec_game/` | Permuter workspace (kept as one of two reference dirs after Phase-G cleanup) |

## Lessons from matching exec_game — tools and gotchas

The exec_game match consumed substantial agent time. Most was avoidable; the resulting tooling now lives in `tools/`:

### Five time-sinks the match exposed

1. **Reading `verify-c` as a diff count when it's a cascade count.** Single missing instruction can look like 140 diffs; the trajectory log read this as wild oscillation and triggered a spurious REGRESSION advice. Fixed by cascade-immune metrics (see below).
2. **Writing register-rename substs by hand.** ~50 rules per function, each derived from a maspsx-index dump + escaped for regfix's regex engine + validated. ~4 hours of mechanical work per function. Fixed by `dc.sh gen-substs` auto-emit.
3. **Manual regfix syntax discovery.** The `swap` keyword typo (real syntax has no keyword: `func: $X <-> $Y`), `insert @ N` vs `insert_after @ N`, `insert_label`'s no-renumber quirk — all learned by reading parser source after the build silently ignored malformed rules. Fixed by explicit swap-keyword warning + see semantics below.
4. **Label-drift end-game.** Last 3 diffs were branches whose opcode + registers matched but whose immediate (offset) didn't — neither register rename nor structural change closes the gap. Eventual fix: `.word`-encode exact target bytes via `dc.sh fix-branch-drift`.
5. **Lossy maspsx pseudos.** A `regfix-suggest` output of 250+ rules was a phantom — comparing 1-line `lw $X, SYM` maspsx pseudos against 2-line `lui+lw` target expansions. Off-by-one alignment cascade.

### Tools that resulted (all in `tools/`)

- **`dc.sh side-by-side <func>`** — cascade-immune shape-aligned diff. Classifies each row as MATCH / REG-RENAME / BRANCH-OFFSET / STRUCTURAL. **Use this FIRST whenever `verify-c` reports a high diff count** — the breakdown tells you what kind of fix you need.
- **`dc.sh gen-substs <func> [--apply]`** — auto-emit `subst` rules for every REG-RENAME row. Maps binary diff → maspsx-stream index via `dc.sh dump-text --post-regfix`.
- **`dc.sh fix-branch-drift <func> [--apply]`** — end-game escape hatch. When `side-by-side` reports `STRUCTURAL=0 REG-RENAME=0 BRANCH-OFFSET>0`, emit `.word\t0xXXXXXXXX` subst rules that hard-code the exact target encoding.
- **`dc.sh binary-diff-count <func>`** — JSON `{structural, rename, branch_offset, total}` consumed by `build_active.py`.
- **Cascade-immune iter_log** — `record()` now accepts `structural`, `rename`, `branch_offset` alongside legacy `diffs`. `plateau_check()` prefers `structural` when present; `render_plateau_advice()` surfaces `S=N R=N B=N` and routes to the right tool.
- **regfix.py swap-keyword typo warning** — explicit function-named ERROR for the malformed `swap $X <-> $Y` form.

### Regfix operation semantics (the hard-won ones — also in [[regfix-reference]])

| Op | Renumber | Notes |
|----|----------|-------|
| `insert @ N` | `idx >= N` by +1 | N is the maspsx index BEFORE the insert applies, not after |
| `insert_after @ N` | `idx > N` by +1 | Line at N stays; new line is at N+1 |
| `insert_label @ N` | none | Labels aren't instructions; appears AFTER idx N but doesn't shift indices. The next instruction (still at N+1) gets the label attached |
| `subst @ N` | none | Pattern is regex, replacement is literal-with-`\t`. **idx is POST-regfix stream** (after deletes) — use `dc.sh dump-text <func> --post-regfix`, not the pre-regfix dump |
| `delete @ N` | `idx > N` by -1 (phase 2) | Operates BEFORE inserts (phase 3) and reorders (phase 8). Delete + insert at same position: delete happens first |

### Subst-in-place vs reorder — pick subst-in-place

When you need to "move" content from one position to another within a small range, prefer transforming each line's content directly via `subst @ N` over the `reorder` op. Reasons:

1. Reorder's index counting is fragile — after phase-2 deletes, indices shift; computing post-everything indices for a 6-line reorder in your head is error-prone.
2. `as` auto-fills the jal delay slot when reorder leaves it empty — subverting intended ordering.
3. Subst is order-independent: each rule anchored to `@ N` so reading the regfix block top-to-bottom is straightforward.

If a `reorder` rule needs more than a 3-instruction range, switch to a sequence of `subst` rules transforming each idx's content to the target's content at that idx.

## See also

- [[community-standard]] — the community bar that exec_game does not yet clear
- [[regfix-reference]] — full regfix.txt syntax
- [[matching-playbook]] — Part 9 "label drift end-game" decision tree
- [[search-tools]] — `tools/c_directed_search.py`, `tools/divergence_analyzer.py`
- [[register-asm-pins]] — pin reliability caveats (V43 collapse documents one)
