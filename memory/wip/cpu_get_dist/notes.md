# cpu_get_dist (code6cac_b.c) — pure-C wall analysis

## TL;DR

- **HEAD's body uses two forbidden coercion cheats** (do-while(0) wrapper + dead `rx = ... uninit vz ... >> 12;` read-before-assignment). Both are
  [[no-new-park-categories]] cheats-by-spelling. With them, cheat-stripped sandbox = 8 (closed by 4 regfix rules). Without them, ALL pure-C variants = 21.
- 14-variant sweep documents the rejected lever space (see `meta.json.rejected_forms`).
- The wall is sched.c:2399 `rank_for_schedule`: at the choice point after `sll cos_idx`, sched1 picks `lw vx` (class 3, indep of last_scheduled) over `lh cos_val` (class 1, data-dep on sll). This places lw before lh, prevents vx from reusing the freshly-dead $v1, and shifts the entire downstream allocation by one register position.
- `scan_hand_coded.py --single cpu_get_dist` → tier=LOW (`no strong hand-coded indicators`). Function is NOT canonical-asm material; matching C must exist.
- candidate.c is the honest pure-C baseline (score 21). Next session should resume from THIS, not from HEAD's cheated body.

## HEAD's two cheats — evidence

### 1. `do { ... } while (0);` wrapper (lines 2303–2315 in `src/code6cac_b.c` @ HEAD `fcbde9da`)

Wraps from `sin_val = ...` through the `if (v48 < 0)` block. Creates `NOTE_INSN_LOOP_BEG` between `vx = ...` and `sin_val = ...`, which per [[loop-exit-work-inside-loop-sched-fence]] acts as a sched.c fence (sched.c:2067-2095). This splits the post-call work into two basic blocks, constraining sched1's reordering window. Not in scope for [[do-while-zero-exception]] (no LABEL_OUTSIDE_LOOP_P / reorg.c interaction; no NE-invert peephole). Forbidden.

### 2. Dead `rx = ((vx * cos_val) + (vz * sin_val)) >> 12;` reading uninit `vz` (line 2297 inside the do-block)

Followed by `vz = a0[0x4C];` and then `rx = ((vx * cos_val) + (vz * sin_val)) >> 12;` again (the real one). The first `rx = ...` is dead (immediately overwritten) and uses `vz` before its proper assignment. The construct's ONLY effect is to extend `vz`'s pseudo live range upward, biasing greg's conflict graph so vz lands in `$v0` (matching target). Identical intent to forbidden `find_dead_param_assigns` / `find_dead_conditional_stores` patterns — different LHS (a local rx) and uses uninit-RHS rather than dead-store-RHS, but the cheat-by-spelling lens classifies it the same. Forbidden.

## RTL mechanism (sched.c:2399 rank_for_schedule)

In the post-call basic block, after sched1 schedules `sll $3, $3, 1` (cos_idx in $v1), the ready set includes:

| insn | class vs last(sll) | LUID |
|---|---|---|
| `lh cos_val, Judge($3)` | 1 (data-dep on sll's $3) | lower |
| `lw vx, 68($s0)` | 3 (independent) | higher |
| `andi sin_idx, $v0, 0xfff` | 3 (independent) | higher |

sched1's tie-break (sched.c:2435 `tmp_class - tmp2_class`) prefers higher class. lw vx and andi sin_idx are class 3, lh is class 1 → lh loses the class check regardless of LUID. Among the class-3 ties, lower LUID wins (sched.c:2442) — in source-order cos_val FIRST, lw vx has lower LUID than andi sin_idx → lw vx picked.

Cascade: lw vx commits to a register WHILE $v1 still holds the live cos_idx scratch. $v1 unavailable → vx → $a0. After the eventual lh cos_val, $v1 is dead — too late for vx, which is already in $a0. cos_val then takes the next available register ($a2 instead of $a1). Downstream pseudos shift: sin_val → $a1, vz → $v1, etc.

Confirmed via `-da` greg dump (`tmp/cgd_iso.c.greg`): pseudos 88 → $a0 (vx), 97 → $a2 (cos_val) for clean form; HEAD's body gives 88 → $v1, 97 → $a1.

## What HEAD's cheats DO to flip this

HEAD's `do {` between vx-load and sin_val splits the BB, isolating the lw-vs-lh choice inside the pre-loop BB. Combined with the dead `vz`-use forcing `vz` into the conflict graph EARLIER, greg lands the target's 4-out-of-5 critical-pseudo allocation (vx=$v1, cos_val=$a1, sin_val=$a0, vz=$v0) with only the cos_idx scratch misallocated (→$a0 vs target's $v1). The 4 regfix rules close that residual.

## Lever exhaustion — what was tried (negative)

See `meta.json.rejected_forms` for the 14-variant sweep. Categories:
- **Declaration order**: K&R-top decls, vx-first, cos+sin first, early-loads — all 21
- **Type/precompute**: judge-pointer local, intermediate cross-products, pre-negated -vx, scratch idx locals — all 21
- **Addressing style**: array indexing (a0[17]), base-pointer alias ((u8*)a0 once) — all 21

The wall is NOT in any of these dimensions. It's the rank_for_schedule class preference, which no decl-order or naming change perturbs.

## Next-session hypotheses (highest-ROI first)

1. **Permuter (vanilla, then directed PERM_*)** from this candidate as base. The manual sweep covered statement-level mutations; the permuter may surface a deeper structural mutation (e.g. function-level type change of a helper return).
2. **Narrow `single_game_getEnemyCharId` return type** to `s16` — would emit `sll/sra` to extend, possibly biasing sched1 differently.
3. **Investigate whether a maspsx-level adjustment** could let the natural sched1 order (lw vx → lh cos_val) match target bytes — but target's bytes have lh before lw, so this is unlikely to work without restructuring.
4. **Deeper RTL reading**: check whether INSN_REF_COUNT (the per-pseudo use-count) feeds priority in a way I missed; if cos_val has higher REF_COUNT in some structural form, lh's priority would exceed lw vx's and win regardless of class.

## Related rules

- [[register-alloc-pure-c]] (read first — the lever playbook this exhausted)
- [[no-new-park-categories]] (cheats-by-spelling — why HEAD's do-while + uninit-vz read are forbidden)
- [[do-while-zero-exception]] (do-while(0) sanctioned ONLY for LABEL_OUTSIDE_LOOP_P / reorg.c — not in scope here)
- [[loop-exit-work-inside-loop-sched-fence]] (the LOOP_BEG-as-fence mechanism HEAD exploits)
- [[no-compiler-divergence]] (the toolchain is fixed; the variable is C structure — but sched1's class preference is not a C-tunable surface)
