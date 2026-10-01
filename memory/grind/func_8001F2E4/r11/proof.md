# func_8001F2E4 — Ruling 11 submission for `temp`, `temp2`, `dx`, `dz`; the `tgt_x` constant-holder (manual laneE, 2026-09-30)

Rulings: `.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (owner, 2026-09-26) with its (D)(2)
"Any named compiler pass" clause (owner Q58, 2026-09-30, committed b1c4be4e2), the Q30 set-aside and the
Q31 mechanism + search standard; `.claude/rules/named-local-fake-exception.md` for `tgt_x`. This is a FRESH
submission answering the 2026-09-29 retro-audit FAIL of c7a9e4c6a (tmp/audit-2026-09-29/review/batch_03.md;
evidence.md "Reopen 2026-09-30"): (1) `tgt_x` an un-annotated constant-holder, (2) `ang`/`tgt_y` reuse
admitted only on allocator effect, (3, weaker) dx/dz/dist_sq/dist reuse never measured.

What changed against the retro-audit body (rejected/retro-audit-2026-09-29.c), every change sandbox 0:
- `dist_sq`/`dist` are no longer shared: each square-root block declares its own (block-1 else arm,
  block 4). Measured unnecessary (r11-2026-09-30 only_dsq_dist_b4 = 0; here F.c = 0).
- `ang` -> `temp`, `tgt_y` -> `temp2` (Ruling 11 (E)(i) generic names); `dx`/`dz` keep their names
  ((E)(ii)); `sp_tmp`/`sp_tmp2` -> `lzc_out`/`lzc_out2` (the island output slots, as func_8001A820 in the
  same file names its slot).
- The two GTE islands are now the header-exact inline_o.h class form (six separate statements each,
  character-identical to func_8001A820's island in the same file), not the joined cop2-preamble text.
- The no-op `(u8 *)` cast on `&g_sqrt_table_u8` is dropped (`*(&g_sqrt_table_u8 + i)`, the file's form).
- The three old in-body "FAKE: variable reuse" comments are gone; each Ruling 11 variable carries the
  (F) declaration comment instead; `tgt_x` carries a FAKE constant-holder annotation.

Files:
- `final.c` — the submitted body (== ../candidate.c; the splice into src/code6cac_tu2.c).
- `one-var-per-value-form.c` — the (C)(1) one-variable-per-value spelling (== variants/split_all.c).
- `variants/` — every measured spelling (`r11v_*` ablations, `r11s_*` structural, `r11f_*` FAKE-family,
  `r11t_*` tgt_x alternatives); `scores.txt` their `sandbox --disable all` lines.
- `dumps.txt` — the (D)(1) excerpts; `tools/` the scripts: gen2.py (ablations from final.c),
  gen_struct.py, gen_fake.py, gen_tgtx.py, dump.sh / dumpall.sh / runall.sh (dumps), conf.py +
  r11table.py (pseudo naming), collect.py (dumps.txt), mkperm.sh (permuter workspace).

Scores: `sandbox func_8001F2E4 --disable all` (347 target insns), build compiler (stock cc1, PLUS->IOR
patch removed), main at b1c4be4e2 + the other lanes' uncommitted work (none in code6cac_tu2.c).

## The variables and their values (target register, target address)

`temp` — six values, all in $a0 in the target:
- V1 `temp = (tgt_z - obj.1E6) & 0xFFF; if (>= 0x800) -= 0x1000;` read by `obj.1E6 += temp / 8`
  (`andi $a0,$v1,0xFFF` 0x8001F4A8; `bgez $a0` / `addiu $v0,$a0,7` 0x8001F4C0/C8).
- V2 the same for obj+0x1E8 from `temp2` (`andi $a0,$v0,0xFFF` 0x8001F4E0; `bgez $a0` 0x8001F4F4).
- V3 the clamped twist `(ratan2(D_800A387C, ..) - 0x400) & 0xFFF`, wrap, clamp ±0x1FF, stored to obj+0x1EA
  and added to a/b+0x7E (`andi $a0,$v0,0xFFF` 0x8001F580, `sh $a0,0x1EA($s0)` 0x8001F5B4).
- V4 the wrapped obj+0x1EA easing delta from `temp2`, read by `obj.1EA += temp / 8`
  (`andi $a0,$v1,0xFFF` 0x8001F734; `bgez $a0` / `addiu $v1,$a0,7` 0x8001F74C/54).
- V5, V6 the jitters `(rng_Next() & 0x3F) - 0x20`, each read by four `+=`/`-=` stores
  (`addiu $a0,$v0,-0x20` 0x8001F7B8 and 0x8001F800).
No write of one value reaches a read of another (each value's first write overwrites the whole variable
on every path to its reads), so these are six values in Ruling 11's sense.

`temp2` — two values, $a1 in the target:
- W1 the elevation target: `= 0x100` (state 0x1F), `= 0x400 - ratan2(dist, dy)` + clamp ±0xFF, `= 0`
  (outside 0x15/0x25); all three reach `temp = (temp2 - obj.1E8)` (V2's write), so one value
  (`addiu $a1,$zero,0x100` 0x8001F34C, `subu $a1,$v1,$v0` 0x8001F468, `addu $a1,$zero,$zero` 0x8001F494).
- W2 the twist target in block 4: `(ratan2(dist, ..) - 0x400) & 0xFFF`, wrap, clamp ±0x1FF, read by V4's
  write (`andi $a1,$v0,0xFFF` 0x8001F6F4).

`dx` / `dz` — two values each, $v1 in the target: block 1 `partner.0x180 - obj.0x180` (`.0x188`), read
by `dist_sq = dx*dx + dz*dz` (`subu $v1,$v1,$v0` 0x8001F3AC / 0x8001F3C0); block 4
`partner.0xF4 - obj.0x25C` (`.0xFC - .0x264`; 0x25C/0x264 are the saved copy of obj+0xF4/0xFC), read the
same way (0x8001F638 / 0x8001F64C).

## (A) Fresh local, not a borrow
All four are locals of func_8001F2E4, declared once, not parameters/globals/static/register, and no
`&temp`, `&temp2`, `&dx`, `&dz` anywhere (the only `&` in the body are `&lzc_out`, `&lzc_out2` and
`&g_sqrt_table_u8`). Innermost enclosing scope = function body for all four: `temp` has writes at
function level (V1, V2) and in blocks 3/4/5; `temp2` in the block-1 arms and block 4; `dx`/`dz` in the
block-1 else arm and block 4. No other declaration is moved or re-scoped relative to the
one-variable-per-value form (`diff final.c one-var-per-value-form.c`: added per-value declarations,
changed identifiers, and the dropped (F) comments only).

## (B) Every write is live; no re-store
(1) Every write is read before the next write on some path: V1/V2/V4 writes and their `-= 0x1000` by the
`/ 8`; V3's write, `-= 0x1000` and clamps by the obj+0x1EA store; V5/V6 by their stores; W1's three
writes and clamp stores by V2's write; W2's writes by V4's write; dx/dz writes by `dist_sq`.
(2) Writes that may store a value the variable holds on some path, with a feasible path where it holds a
different value (Ruling 5 2(c) as clarified 2026-09-26):
- `temp` V1: the variable has no earlier write on any path (first write of the function).
- V2 (held: V1). Path: obj+0x6A = 0x15, obj+0xC = 0x1F (temp2 = 0x100, tgt_z = 0), obj.1E6 = 0,
  obj.1E8 = 0: V1 = 0, V2 stores 0x100.
- V3 (held: V2). Path: obj+0x6A = 0x25, obj+0xC = 0x1D (so W1 is the computed elevation), obj+0x8C != 0,
  obj.1E8 equal to W1: V2 = 0; the twist `(ratan2(D_800A387C, dy) - 0x400) & 0xFFF` for a ratan2 result
  of 0x500 is 0x100, stored by V3's first write. V3's `-= 0x1000` changes the value; its clamp writes
  `= 0x1FF` / `= -0x1FF` run only when temp >= 0x200 / < -0x1FF, so they never re-store.
- V4 (held: V2 or V3). Path: block 3 skipped (obj+0xC not 0x1D/0xE), V2 = 0 (obj.1E8 == temp2),
  W2 - obj.1EA = 0x40: V4 stores 0x40.
- V5 (held: V2, V3 or V4), V6 (held: V5): rng_Next results; V6 = V5 only when the two random numbers
  agree mod 64, not on every path.
- `temp2` W1 writes: no earlier write on any path; the clamps run only outside ±0xFF. W2 (held: W1):
  plainly feasible path obj+0x6A = 2, obj+0xE = 6 (block 4 runs; block 1 takes its else arm, so W1 = 0),
  and the clamped twist nonzero, e.g. ratan2(..) = 0x600 gives 0x200, clamped to W2 = 0x1FF != 0.
  W2's clamps run only outside ±0x1FF. (Corrected 2026-09-30 after layer-2 rev-1f2e4: the earlier path
  W1 = 0x100 needed obj+0x6A to be 0x15/0x25 at block 1 and 2 at block 4, feasible only if a callee
  rewrote obj+0x6A in between.)
- `dx`/`dz` block 4 (held: block-1 value, or no value when block 1 did not run): plainly feasible path
  obj+0x6A = 2, obj+0xE = 6 — block 1 does not write dx/dz, so there is no earlier write on that path.

## (C) Same statements; real computations
(1) `one-var-per-value-form.c`: `temp` V1..V6 -> `d1e6`, `d1e8` (function scope: their writes are at
function level), `twist` (block-3 body), `delta` (block 4), `jit1`, `jit2` (block-5 body); `temp2`
W1 -> `elev` (function scope: writes in the block-1 arms), W2 -> `twist_tgt` (block 4); `dx`/`dz` block
1 -> `dx`/`dz` declared in the block-1 else arm, block 4 -> `dx`/`dz` declared in block 4 (same names,
different scopes). Each at the innermost scope enclosing that value's writes.
(2) `diff final.c one-var-per-value-form.c`: declarations and identifiers only (and the (F) comments).
(3) Every value has a load / arithmetic / call-result write whose instructions are in the target (the
addresses above). No value is a bare copy or all-constant: W1's constant writes (0x100, 0) sit beside
its computed `0x400 - ratan2(..)` write in the same value.

## (D)(1) Dumps — `dumps.txt`
Command lines (`tools/dump.sh`, recorded in dumps.txt): the body is substituted into a copy of
src/code6cac_tu2.c (engine.inlineasm.substitute_body), preprocessed with engine/buildconfig.py
CPP_FLAGS + CPP_DEFS, and compiled by the INSTRUMENTED `tools/gcc-2.7.2/cc1` with the build's CC_FLAGS
(`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float`,
code6cac_tu2 is a -G0 file) plus `-dr -ds -dt -df -dc -dl -dg`, BB2_ALLOC_DEBUG=1, and
BB2_FINDREG_DEBUG=<pseudo> reruns. The build cc1 (`tools/gcc-2.7.2/build/cc1`) is run on the same .i:
every variant's instrumented .s is identical to the build .s (instcheck, all "identical").
Dumped: final (R_base), the one-var form (split_all), every single-value ablation (only_V1..V6, only_V56,
only_W2, only_X2/Z2/XZ2) and the tgt_x alternatives (T1-T4). Pseudo naming: tools/r11table.py (user
variables are expanded in declaration order; the count is checked against the reg/v pseudos).
Final: temp = 82, temp2 = 78, tgt_x = 79, dx = 80, dz = 81. One-var form: elev 78, d1e6 80, d1e8 81,
twist 188, twist_tgt 214, delta 215, block-1 dx/dz 98/99, block-4 dx/dz 216/217, jit1/jit2 281/282.

## (D)(2) Mechanisms (pass and source location; the dumps show each going the target's way in final.c and
the other way in the one-variable-per-value form)

**M1 — local-alloc versus global allocation (flow.c, local-alloc.c).** flow.c records the basic block of
every reference; a pseudo referenced in two blocks becomes REG_BLOCK_GLOBAL (flow.c:2072-2075,
2508-2511). local-alloc.c:472 takes only pseudos with `reg_basic_block[i] >= 0` (one block) and one
death; the rest go to global.c.
- `dx`/`dz`: final — pseudos 80/81 are referenced in block 1 and block 4 (.lreg `used 6 times across 4
  insns; dies in 2 places`, no "in block"), so global.c allocates them (ALLOCDBG ord 1/2). find_reg's
  first-free scan would give $v0 (not in pass-0 `used`); its preferred-register step
  (global.c:1133-1165, preferences recorded by set_preference, global.c:1671-1755) replaces it with the
  preferred $v1 (FINDREG 80: conflicts {a0}, own_full_prefs {3}; 81: conflicts {}, own_full_prefs {3})
  = the target's `subu $v1` at all four sites. One-var form — 98/99/216/217 are `used 3 times across 2 insns in block
  12` / `block 45`, local-alloc's (`;; Register 98 in 2.`) = $v0. only_X2 / only_Z2 (one value split):
  8 each; only_XZ2 16; split both variables 16.
- `temp` V5/V6: final — 82 spans blocks 1-5, global; seated $a0 (M2). One-var form — jit1/jit2 281/282
  `used 5 times across 11 insns in block 63`, local-alloc'd to $v0 (the target has `addiu $a0,$v0,-0x20`).
  only_V5 / only_V6: 11 each.

**M2 — global.c find_reg over the union of the values' conflicts (global.c:952, first-free scan
global.c:1052-1083; MIPS defines no REG_ALLOC_ORDER, so numeric order v0, v1, a0, a1 ...).** A shared
variable is one allocno; its hard-register conflict set is the union of every value's live range.
- `temp`: final — 82 (ALLOCDBG ord 0) FINDREG conflicts {v0, v1}: $v1 from V2's range, which overlaps
  the local-allocated obj+0x1E8 load held in $v1 (`lh $v1,0x1E8($s0)` 0x8001F4D0, reused by
  `addu $v0,$v1,$v0` 0x8001F508); find_reg returns $a0 for all six values, the target's register.
  One-var form — d1e6 (80), twist (188), delta (215): FINDREG conflicts {v0} only -> $v1; d1e8 (81)
  {v0, v1} -> $a0 (the target's, V2 alone reproduces it); jitters local-alloc'd (M1).
  Ablations: only_V1 27 (d1e6 $v1 + M3), only_V3 11 (twist $v1), only_V4 11 (delta $v1 + M3).
- `temp2`: final — 78 (ord 4) FINDREG conflicts {v0, v1, a0} -> $a1 for both values (the target's $a1 at
  0x8001F468 and 0x8001F6F4). $a0 enters because W1's range contains V1 (temp, $a0, allocated first).
  One-var form — twist_tgt (214) conflicts {v0} -> $v1; only_W2: twist_tgt 212 -> $v1 while W1 keeps
  $a1 (score 9); elev (78) in the full form: conflicts {v0, v1, s0} -> $a0 (d1e6 now holds $v1, not $a0).

**M3 — cse.c make_regs_eqv (cse.c:826-880; the head test cse.c:840-855), the Q58 named pass.**
expmed.c expands each signed `x / 8` as `q = x; if (q >= 0) goto L; q = q + 7; L: r = q >> 3`. When
cse1 records `q = x`, make_regs_eqv makes the new register q the canonical head of {x, q} only if it
lives beyond the cse block AND `uid_cuid[regno_last_uid[q]] > uid_cuid[regno_last_uid[x]]`
(regno_last_uid ignores REG_NOTES: regclass.c:1762-1764). Final: `temp` (82) is referenced long after
every division (its next value's write, finally the jitter stores), so it stays the head and cse1
rewrites the test and the `+7` to read it: .rtl `(ge (reg 160) 0)` / `(plus (reg 160) 7)` become
.cse `(ge (reg/v 82) 0)` / `(plus (reg/v 82) 7)` at all three divisions (insns 339/341 for obj+0x1E8,
the same for 0x1E6 and 0x1EA). The copy then fills the branch delay slot: target `bgez $a0` /
`addu $v0,$a0,$zero` / `addiu $v0,$a0,7`. One-var form: each delta's variable is last referenced by the
copy `q = d` itself (nothing reads it after `d / 8`), q outlives it, q becomes the head, and the test
and `+7` keep reading q (.cse `(ge (reg 151|161|265) 0)`): `move v0,a0` before the branch, one extra
insn per division. only_V2 = 4 is exactly this decision alone (d1e8 keeps $a0 by M2; the only hunks
are the copy/branch at the obj+0x1E8 division). only_V56 = 6: both jitters in one other local, so
`temp` is last referenced at the obj+0x1EA division and M3 flips there alone.
This is the Q58 case: the decision is cse's, named by pass and source line, dumped (.cse, `-ds`) for
both spellings.

## (D)(3) Necessity (Q31 mechanism + search)
(a) Mechanisms M1-M3 above, from banked dumps of both spellings. The property of the reuse spelling each
depends on, and why each per-value spelling lacks it BECAUSE the value has its own variable:
- M1: the variable is referenced in more than one basic block. A per-value `dx`/`dz` or jitter variable
  is referenced only by its value's statements, which (C)(2) fixes: a straight-line write and its reads
  with no branch between them (the `dist_sq` sum; the four jitter stores after rng_Next), so one block.
- M2: the allocno's conflict set includes $v1 (temp) / $a0 and $v1 (temp2) contributed by ANOTHER value's
  range (V2's; W1's). A per-value variable's conflict set is its own value's range only; for d1e6,
  twist, delta and twist_tgt that range holds no $v1 (or $a0) occupant (FINDREG, one-var form).
- M3: the variable is referenced after the quotient copy. A per-value easing delta has no statement
  after `d / 8` that names it.
(b) Every per-value spelling proposed so far is banked in variants/ and measured (scores.txt): the full
form 91; the single-value ablations; the structural respellings; the permuter finds (below). No reviewer
spelling yet (fresh submission). (c) No counting spelling reaches the target: all > 0 (table).

## (D)(4) Measured alternatives (sandbox --disable all; target 347)
| spelling (variants/) | score | insns |
|---|---|---|
| final (r11v_R_base) | **0** | 347 |
| full one-var-per-value (r11v_split_all = one-var-per-value-form.c) | 91 | 349 |
| `temp` all six split, rest shared (r11v_split_temp) | 75 | 349 |
| `temp2` split, rest shared (r11v_split_temp2 = only_W2) | 9 | 347 |
| `dx` and `dz` split, rest shared (r11v_split_dxdz) | 16 | 347 |
| keep only `temp` shared (r11v_keep_temp) | 25 | 347 |
| keep only `temp2` shared (r11v_keep_temp2) | 91 | 349 |
| keep only `dx`/`dz` shared (r11v_keep_dxdz) | 75 | 349 |
| ablation, `temp` V1 alone split (r11v_only_V1) | 27 | 348 |
| V2 alone (r11v_only_V2) | 4 | 347 |
| V3 alone (r11v_only_V3) | 11 | 347 |
| V4 alone (r11v_only_V4) | 11 | 348 |
| V5 alone (r11v_only_V5) | 11 | 347 |
| V6 alone (r11v_only_V6) | 11 | 347 |
| V5+V6 in one other local, not a per-value form (r11v_only_V56) | 6 | 348 |
| `dx` block-4 alone (r11v_only_X2) / `dz` (only_Z2) / both (only_XZ2) | 8 / 8 / 16 | 347 |
| structural: every per-value local at function scope (r11s_S1_funcscope) | 91 | 349 |
| structural: `+= d / 8` compound easing stores (r11s_S2_compound) | 91 | 349 |
| structural: no dx/dz locals, sums written from the loads (r11s_S3_no_dxdz) | 91 | 349 |
| structural: static inline `wrap12()` helper for the three wraps (r11s_S4_inline_wrap) | 92 | 349 |
| structural: function-scope declarations reversed (r11s_S5_decl_reverse) | 91 | 349 |
| structural: each jitter in its own `{ s32 jitter; }` block (r11s_S6_jitter_blocks) | 91 | 349 |
| permuter from the one-var form (tmp/func_8001F2E4/perm1, 2 jobs) | see § Permuter | |

### Q30 set-aside: per-value spellings that need a FAKE construct (measured for the record)
| spelling (variants/) | family | score |
|---|---|---|
| per-value + self-assign `d = d;` after each division (r11f_M1_selfassign_all) | dead store / self-assign | 91 |
| only V2 split + `d1e8 = d1e8;` (r11f_M1b_selfassign_onlyV2) | dead store / self-assign | 4 |
| per-value + `d++; d--;` after each division (r11f_M2_cancelpair_all) | cancellation pair | 83 |
| only V2 split + `d1e8++; d1e8--;` (r11f_M2b_cancelpair_onlyV2) | cancellation pair | **0** |
| per-value + do { easing steps } while (0) (r11f_M3_dowhile_ease) | do-while(0) | 94 |
Family sentences requiring the annotation: dead-store-fake-exception.md prerequisite 3, "**Mandatory
annotation:** `/* FAKE: <one-line reason> */`"; no-new-park-categories.md 2026-08-18 F6
cancellation-pair entry, "Prerequisites: `!FAKE`-style annotation; exhaustion ledger"; do-while-zero-
exception.md, "Inline `/* FAKE: ... */` or `// FAKE` annotation at the construct site".
r11f_M2b reaches the target, but it is not a one-variable-per-value spelling (five values stay in
`temp`) and it carries a FAKE construct the reuse body does not; under Q30 it is set aside and does not
defeat the reuse (the reuse body carries strictly fewer no-purpose constructs). Banked because it exists.

### Permuter
Workspace tools/mkperm.sh (reduced TU: the body's own declarations + the body; compile.sh = the build's
cpp | build cc1 (CC_FLAGS) | prologue_fix | maspsx (MASPSX_FLAGS) | multu_pad | as recipe; the final body
built there differs from target.o in 0 instructions). Campaign from one-var-per-value-form.c
(tools/permuter_campaign.py, 2 jobs, --stack-diffs): 9,658 iterations in 1,300 s, 149 finds, permuter
score 2285 -> best 1570 (found at 184-285 s, then ~1,000 s with no better find; stopped). Nothing reached
the target. What the finds reuse (permuter.txt, the 12 best as changed lines): every one re-introduces a
second value into a per-value variable — the first jitter into `d1e6` (1570), the second into `d1e8`
(1710), the twist into `d1e8` (1795), the twist target's ratan2 term into `d1e6` (1750), `delta`/`d1e6`
/`tgt_z`/`t` reused as constant or load holders (1600-1760). That is the reuse this submission declares,
rediscovered by search; none is a one-variable-per-value spelling.

## (E) Names
`temp`, `temp2`: (E)(i) generic scratch words. `dx`, `dz`: (E)(ii) — every write of `dx` is a partner-
minus-obj difference of the x component of a position triple (0x180/0x184/0x188 in block 1,
0xF4/0xF8/0xFC with the saved 0x25C/0x260/0x264 in block 4; the middle word of each triple is the height
passed to ratan2 beside the ground distance, so the outer words are x and z), and every write of `dz`
the z component. The same file names these differences `dx`/`dy` for the 0xF4/0xFC pair
(func_8001B294).

## (F) Annotation
Each declaration in final.c carries a comment naming the values (or, for dx/dz, their kind), citing
Ruling 11 (and, for `temp`, whose M3 is a cse decision, owner ruling Q58) and this file.

## `tgt_x` — constant-holder (named-local-fake-exception.md; retro-audit objection 1)
`tgt_x` is 0 on every path and exists to carry that 0 to func_8002F770's fourth argument. It holds ONE
value in Ruling 11's sense (its three writes, one per arm of the 0x15/0x25 × 0x1F branch, all reach the
same two reads), so it is not a reused variable; Ruling 11 (C)(3) and Ruling 5 1(e) route
constant-holders to named-local-fake-exception, whose prerequisites:
1. **Exhaustion** (r11t_*): literal 0 at both calls, no local (T1) 46 / 341 insns; one `s32 tgt_x = 0;`
   initializer (T2) 43 / 344; one write after the join (T3) 46 / 341; one write just before the calls
   (T4) 46 / 341. The register-alloc levers do not apply (the value never reaches a register without a
   local).
2. **Named pass interaction**: the target materializes the 0 in $s2 on each arm (`addu $s2,$zero,$zero`
   at 0x8001F33C, 0x8001F354, and in both delay slots 0x8001F484 / 0x8001F490) and copies it
   `addu $a3,$s2,$zero` before each call (0x8001F518, 0x8001F534). cse works per extended basic block:
   the arm writes reach the calls only through the join, so cse1 leaves the argument moves reading the
   pseudo (final .cse insns 370/391 `(set (reg a3) (reg/v 79))`); written after the join (T3) cse folds
   them to `(const_int 0)` (T3 .cse insns 364/385) and the local vanishes. The pseudo is live across the
   first func_8002F770 call (.lreg `crosses 1 call`), so global.c find_reg (FINDREG 79: pass-1 `used`
   covers every call-clobbered register) seats it in the first free call-saved register, $s2
   (ALLOCDBG ord 27). Initialized once at entry (T2) it lives through the whole function and lands in
   another callee-saved register with one write instead of four (344).
3. **Annotation**: `/* FAKE: constant-holder ... */` at the declaration, naming both passes and this file.
4. **Review**: layer-2 (this submission).
The x-axis argument really is 0 in every state (the function never tilts on x); the local is the
FAKE-annotated device, not a claim of a varying value.

## Everything else (H)
GTE islands: two gte_Lzc(dist_sq, &lzc_out / &lzc_out2) units, inline_o.h class (inline-asm-policy.md §
Owner ruling 2026-09-26), character-identical to func_8001A820's (same file, COMPLETED 87450b3e7). canonical
gate `--fast`: ASM-PARTIAL, 4/347, regions [71,71] [74,74] [232,232] [235,235]. `lzcr = 0; if (dist_sq >= 0)`
is the target's `addu $v1,$zero,$zero` in the `bltz` delay slot. `func_80027334` is called undeclared as
in the file's func_8001F1C4 (pre-existing in the TU).
