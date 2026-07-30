# Hypothesis ledger — replay_camera_rob_back_loose3

Floor: engine metric **17** on the HEAD (cheat-carrying) form; **26** cheat-free
(build_insns 113 vs target 114). Work the 26.

## KILLED (session 1 — do NOT re-propose)

1. **Local declaration order of `angC` steers its hard register.**
   Mechanism: GCC assigns pseudos in `expand_decl` order, so decl position could flip a
   local-alloc quantity tie-break. Probe: `angC` declared (a) first, (b) in the
   `s32 angA, angB, angC;` group, (c) on its own line after angA/angB, (d) last.
   Result: **all four measured exactly 26 / build_insns 113** — inert.
   Banked as `rejected/declorder-angC-inert.c`.

2. **A narrower-type view of the `Judge` element reproduces target's `lhu; sll 16; sra 16`.**
   Mechanism: read the element unsigned and sign-extend explicitly (the sanctioned
   `u16-global-lhu-lbu-low-byte` narrow-pointer-view family). Probes, all 26 / 113:
   `u16 raw = Judge[i]; cosA = (s16)raw;` · `cosA = (s16)*(u16 *)&Judge[i];` ·
   `u16 cosA` with `(s16)cosA` at all four multiply sites · the last one repeated with all
   four `extern s16 Judge[]` decls in text1a_c.c retyped `extern u16 Judge[]`.
   Cause (confirmed): GCC 2.7.2 combine folds `ashiftrt(ashift(zero_extend(mem)))` into
   `sign_extend(mem)` = `lh` because the zero-extended value has exactly ONE use.
   Banked as `rejected/u16-retype-and-cast-at-uses-folds-to-lh.c` and
   `rejected/u16-ptr-view-cosA-folds-to-lh.c`.

## SESSION 2 (structural) — floor 26 -> 13

### CONFIRMED — mechanism A is closed in pure C by an intervening STORE
**Statement:** the `lhu; sll 16; sra 16` shape survives if a memory WRITE sits
between the `u16` staging load and its `(s16)` cast.
**Mechanism:** combine's `can_combine_p` refuses to combine a MEM load into a later
user across an insn that may write memory, so `simplify_shift_const` never sees the
`ashiftrt(ashift(zero_extend(mem)))` chain. It is free: sched1 runs after combine
and hoists the store back out (target fills that load-delay slot with `mflo t9`).
**Probe/result:** `u16 rawA = Judge[jA]; a1[5] = -sinA; cosA = (s16)rawA;` ->
**13 / build_insns 114** (target 114). Store after the cast -> 26 (inert). Any store
works (`a1[0]` in the slot -> 13). The same interleave without the `u16` staging
local -> 26, because that load is already a `sign_extend` MEM. Banked in candidate.c.

### KILLED — "the raw halfword needs a second, flow-LIVE use" (s1's frontier A)
**Probe:** three second uses of `rawA` on top of the staging spelling — `a1[9] = rawA`
(low half), `a1[9] = rawA + 1` (SI add), `a1[9] = rawA >> 8` (needs the zero-extended
high bits). **Result:** all three still emit a single `lh` (35/114, 38/115, 42/116);
for the last, GCC kept the `lh` and re-derived the unsigned value with `andi`/`srl`.
Combine rewrites the LOAD and patches every other user from the sign-extended
register, so no width-of-use argument can block it. Banked as
`rejected/u16-second-use-does-not-block-lh-fold.c`.

### KILLED — mechanism B is not reachable by ANY structural lever
**Probe:** 15 forms (named local for the angC cos index / sin index / both / both
hoisted; angC read hoisted next to angA/angB; angC+sinC moved ahead of sinA/sinB;
named locals for all six Judge indices; angC reused as its own index holder;
two-variable and split-init `sinAxsinB`; multiply-operand-order swaps on `prod_*` and
on all four `cosA_*`; walking `s16 *J = Judge` pointer; `cosA` narrowed to `s16`),
each measured with `sandbox --disable all`. **Result:** every one scored EXACTLY
26 / 113 on the s1 baseline — bit-identical, i.e. GCC canonicalises the spelling away
before allocation. Two were worse (cos index hoisted 43/112; `sinAxsinB_12 >>= 12`
36/113). Re-measured on the new score-13 baseline the survivors are all still exactly
13 / 114 (only the cosA operand swap moved, to 17). **Root cause measured:** the
`.lreg` flow numbers that feed `qty_compare_1` are untouched by every one of these
spellings — angC is pseudo 97 "used 3 times across 12 insns" (priority 0.25, so it is
allocated LATE and takes the leftover register) and its index temp is pseudo 111
"used 2 times across 2 insns" (priority 1.00); `greg` confirms neither is in its
"20 regs to allocate" list, so both are LOCAL-alloc decisions. Banked as
`rejected/named-index-locals-all-inert.c`.

## FRONTIER

**A. The raw `cosA` halfword needs a SECOND use of the zero-extended value, so combine
cannot fold `lhu+sll+sra` into `lh`.**
Mechanism: `simplify_shift_const` only rewrites the chain when the `zero_extend` dest is
single-use and dead after the shifts (`dead_or_set_p`); the matched precedent
`func_80065344` (text1b.c:14641) keeps its `lhu` precisely because the u16 value is also
stored back. Note the second use must survive to combine — GCC 2.7.2 runs `flow` (dead-code
deletion) BEFORE `combine`, so a purely dead second use is deleted first and cannot help;
it has to be a use that appears in the emitted output or is otherwise flow-live.
Next probe: find what that live second use IS in the target rather than inventing one —
re-derive the block with m2c (`asm/funcs/replay_camera_rob_back_loose3.s`) and check the
two rule-carrying siblings `_SelectSection` and `hirahira_w_ctrl_2`, whose targets show the
same `lhu`+sext signature (hirahira ×2), for a shared shape that consumes the raw u16.
Also worth checking: whether the sign-extension in target belongs to a value that reaches
the multiplies through a DIFFERENT C variable than the load's destination (two variables,
not one), which gives the load's dest a second reader naturally.

**B. `angC` must be allocated `$v1` (freeing `$v0` for the index temp and then for
`sinAxsinB_12`) by a local-alloc quantity-priority change — 13 of the 26 diffs.**
Mechanism: local-alloc (not global) already hard-assigns `angC` → `$v0` at greg insn 44;
priority comes from `reg_n_refs` / live-range length per quantity. Decl order is dead
(above), so the lever must change the ref count or live length of `angC` vs the index
temp — e.g. splitting the cos-index chain into its own named local (shortening `angC`'s
range), or computing the sin index through a separately-named local so the temp's quantity
becomes the longer-lived one. `.claude/rules/register-alloc-pure-c` Levers A–C are the
playbook; use the `BB2_ALLOC_DEBUG` / `BB2_PRIO_DEBUG`-instrumented cc1 if the plain
`.lreg` "used N times across M insns" numbers do not explain the tie.

**C. Cross-check against the two family siblings before more single-function probing.**
`_SelectSection` (10 rules) and `hirahira_w_ctrl_2` (63 rules) are the SAME 3x3
rotation-matrix shape in the same file, and their regfix comments describe the SAME two
mechanisms (`# Fix sinAxsinB_12 register: GCC puts in $4 (a0), target has $2 (v0)` and the
`la`→`$at` volatile-addressing block). A form that closes any one of the three is very
likely to close all three; conversely a mechanism present in all three is a property of the
original *source idiom*, not of this function's local details.

## [s1] The local declaration order of angC steers which hard register local-alloc gives it ($v0 vs target's $v1).
- mechanism: GCC creates pseudos in expand_decl order at function start, so declaration position can flip a local-alloc quantity priority tie-break.
- probe: Cheat-free baseline (pin + volatile removed) with angC declared four ways: first; inside 's32 angA, angB, angC;'; on its own line after angA/angB; last. sandbox --disable all on each.
- result: All four measured exactly score 26 / build_insns 113 — bit-identical outcome. Declaration order is inert for this allocation.
- verdict: KILLED

## [s1] Reading the cosA Judge element through a narrower unsigned view reproduces target's 'lhu; sll 16; sra 16' split shape without volatile.
- mechanism: Sanctioned narrow-view family (u16-global-lhu-lbu-low-byte): a u16-typed read forces lhu and an explicit (s16) cast forces the sign-extend pair, which is the 3-insn shape target has (and the reason target is 114 insns vs our 113).
- probe: Four spellings, each sandboxed: (a) 'u16 raw = Judge[i]; cosA = (s16)raw;'  (b) 'cosA = (s16)*(u16 *)&Judge[i];'  (c) cosA typed u16 with (s16)cosA at all four multiply sites  (d) spelling (c) repeated with all four 'extern s16 Judge[]' decls in src/text1a_c.c retyped 'extern u16 Judge[]'. objdump of each sandbox .o to inspect the emitted load.
- result: All four emit a single 'lh' — score 26 / build_insns 113 in every case. Confirmed cause: GCC 2.7.2 combine (simplify_shift_const) folds ashiftrt(ashift(zero_extend(mem))) into sign_extend(mem) whenever the zero-extended value is single-use and dead after the shift chain. Only 'volatile' (an un-combinable MEM) escapes the fold — and that is the cheat currently in HEAD.
- verdict: KILLED

## [s1] The cosA load shape (mechanism A) and the angC/sinAxsinB_12 register assignment (mechanism B) are one coupled root cause.
- mechanism: If the load shape drove the register cadence, fixing A would resolve B's 13 diffs as a cascade.
- probe: Compared the diff of the with-volatile build (which HAS target's correct lhu+sll/sra shape, score 17) against the no-volatile build (score 26).
- result: Both builds show the identical 13 B-diffs (angC $v0 vs $v1, index temp $v1 vs $v0, sinAxsinB_12 $a0 vs $v0). A and B are independent axes and must be closed separately.
- verdict: CONFIRMED

## [s2] A memory WRITE placed between the u16 staging load of cosA and its (s16) cast preserves target's lhu + sll 16 + sra 16 shape, at zero instruction cost.
- mechanism: GCC 2.7.2 combine's can_combine_p refuses to combine a MEM load into a later user across an insn that may write memory, so simplify_shift_const never sees the ashiftrt(ashift(zero_extend(mem))) chain it would otherwise rewrite to sign_extend(mem) = lh. It costs nothing because sched1 runs AFTER combine and hoists the store back out; target's own schedule fills that load-delay slot with mflo t9.
- probe: u16 rawA = Judge[((s16)angA + 0x400) & 0xFFF]; a1[5] = -sinA; cosA = (s16)rawA;  (the a1[5] store MOVED from the tail of the function into that slot), measured with sandbox --disable all; plus controls: store after the cast, a different store (a1[0]) in the slot, and the same interleave without the u16 staging local.
- result: 13 / build_insns 114 (target_insns 114) vs the 26 / 113 baseline. Store after the cast -> 26 (inert). a1[0] in the slot -> 13 (any store works). Same interleave on the plain s16 baseline -> 26 (that load is already a sign_extend MEM, so there is no zero_extend to protect). diffasm on the score-13 object shows the tgt[62:67] cosA region matching exactly and no instruction-shape difference left anywhere in the function.
- verdict: CONFIRMED

## [s2] (s1's frontier A) The raw cosA halfword needs a second, flow-LIVE use of the zero-extended value to stop combine folding lhu+sll+sra into lh.
- mechanism: simplify_shift_const was believed to rewrite the chain only when the zero_extend dest is single-use and dead after the shifts (dead_or_set_p), as in the matched precedent func_80065344 whose u16 is stored back.
- probe: Three real second uses stacked on the u16 staging spelling: a1[9] = rawA (low-half-only store); a1[9] = rawA + 1 (SI add, then truncated by the store); a1[9] = rawA >> 8 (genuinely needs the zero-extended HIGH bits). Each applied to src/text1a_c.c and scored with sandbox --disable all, then objdumped to read the emitted load.
- result: All three still emit a single lh (35 / 114, 38 / 115, 42 / 116 insns). For the >> 8 case GCC kept the lh and re-derived the unsigned value with andi/srl rather than changing the load. Combine rewrites the LOAD itself and patches every other user from the sign-extended register, so no width-of-use argument can block it.
- verdict: KILLED

## [s2] (s1's frontier B) angC can be steered from $v0 into target's $v1 by a structural change to the relative local-alloc quantity priority of angC vs its index temp (var splits, declaration/statement order, type narrowing, re-association).
- mechanism: local-alloc assigns by qty_compare_1 priority = floor_log2(n_refs) * n_refs * size / live_length, so a spelling that changes either term should flip the tie-break.
- probe: 15 structural forms measured with sandbox --disable all: named local for angC's cos index / sin index / both / both hoisted to just after the angC load; angC read hoisted next to angA/angB; the angC+sinC pair moved ahead of sinA/sinB; named locals for all six Judge indices; angC reused as its own index holder; two-variable sinAxsinB; split-init sinAxsinB_12 >>= 12; multiply-operand-order swaps on prod_sinC/prod_cosC and on all four cosA_* products; a walking s16 *J = Judge pointer; cosA narrowed to s16. The survivors were re-measured on top of the new score-13 form. cc1 -da .lreg/.greg dumps read for the priority inputs.
- result: Every form scored EXACTLY 26 / build_insns 113 on the s1 baseline (bit-identical: GCC canonicalises the spelling away before allocation); two were worse (cos index hoisted 43/112, split-init 36/113). On the score-13 baseline the survivors are all still exactly 13 / 114 (the cosA operand swap moved the wrong way, to 17). The .lreg numbers explain it: angC is pseudo 97 'used 3 times across 12 insns' (priority 0.25 -> allocated late, takes the leftover register) and its index temp is pseudo 111 'used 2 times across 2 insns' (priority 1.00); no structural spelling changes either term. greg's '20 regs to allocate' list contains neither, confirming both are LOCAL-alloc decisions.
- verdict: KILLED

## SESSION 3 (structural) — floor 13 -> 12

### CONFIRMED — delaying the LAST USE of the parameter pointer a0 gives angC target's $v1
**Statement:** moving `angC = a0[2]; sinC = Judge[angC & 0xFFF];` to AFTER the
`sinAxsinB_12 = (sinA * sinB) >> 12;` statement flips angC from $v0 to target's
$v1 and its `& 0xFFF`/`<< 1` cos-index temp from $v1 to target's $v0.
**Mechanism:** `a0[2]` is the last use of the parameter pointer, so its position
is a LIVENESS fact about a hard register, not a spelling that sched1 can
normalise away (pass order confirmed in tools/gcc-2.7.2/toplev.c: combine ->
sched1 -> local_alloc). Delaying it keeps $a0 live across the multiply and puts
the angC load after the `mult` in the stream local-alloc sees.
**Probe/result:** `sandbox --disable all` **12 / build_insns 114** (target 114),
down from 13. The build reproduces target's `lhu v1,0x4(a0)` at insn 18 verbatim.
Controls: delayed past `cosB` as well -> also 12; delayed past the whole cosA
block -> 90; the same delay applied to `a0[1]` instead of `a0[2]` -> 13 (inert).
Banked as candidate.c. **This is the direct refutation of s2's blanket KILL of
mechanism B by structural means — s2 only ever moved that read EARLIER.**

### KILLED — eager a1[] stores (s2's frontier probe 3)
**Probe:** every a1[] element stored as soon as it is computable, plus the three
sub-groups separately, plus four tail-store-order permutations, plus `a1[0]` in
the combine-blocking slot. **Result:** fully eager 73/109 (combine/CSE eats five
instructions); sums eager 79/110; sinAxcosB pair eager 53/112; cosA_* singles
eager 13 (inert); tail order index/sums-first 13 (inert), reversed 65/107,
a1[8]-first 42/112; a1[0] in the blocking slot 12 (equivalent). Only the ONE
combine-blocking store belongs out of the tail; moving any other store changes
the insn count and always for the worse. Banked as
`rejected/eager-a1-stores-collapse-insn-count.c`.

### KILLED — declaration order of the WHOLE local block
**Probe:** the entire decl block reversed, collapsed into one group, and reordered
to first-use order (s2 had only permuted angC's own line). **Result:** all three
13/114, bit-identical. The `.lreg` dumps confirm the pseudo NUMBERS do change
(188 vs 192 for the same quantity) and the final assignment does not, so
"qty creation order breaks the priority-1.00 tie" is not a source-reachable
lever. Banked as `rejected/decl-order-whole-block-inert.c`.

### KILLED — type narrowing / sign-extend splitting of the angle locals
**Probe:** angC declared u16; declared s16; all three ang* as u16; `(s16)angC`
split into its own s32 local (for A alone, C alone, and all three).
**Result:** every one 13/114 — CSE canonicalises the width away before
allocation. Only hoisting that sign-extend local ahead of the sinC read moved it,
to 30/113. Banked as `rejected/type-narrow-angC-inert.c`.

### KILLED — reducing the local/pseudo population
**Probe:** fold every single-use intermediate into its consumer; fold just the
four cosA_* products. **Result:** 87/116 and 71/111. Fewer pseudos makes the
schedule worse, not the allocation better. Banked as
`rejected/fold-locals-into-stores-worse.c`.

### KILLED — shortening sinAxsinB_12's live range to raise its qty priority
**Probe:** prod_sinC and prod_cosC made adjacent (live length 17 -> ~4, priority
0.176 -> ~0.75, so it would be allocated early enough to claim $v0 rather than
take the leftover $a0), with and without the shift written inline at both uses.
**Result:** 25/115 both ways — it costs an instruction. Writing the shift inline
at both uses WITHOUT making the consumers adjacent is 13/114. Banked as
`rejected/sasb-consumers-adjacent-adds-insn.c`.

### The remaining gap, stated precisely (12 diffs = ONE value)
`sinAxsinB_12`: target $v0, ours $a0. Scheduling-coupled: target delays
`mflo`/`sra` past the cosC index chain (mult @17, mflo t0 @22, sra v0,t0,12 @26),
so $v0 — held by the index temp at 20-24 — is free from 26; ours emits `mflo v0`
@19 / `sra a0,v0` @20 before the chain, the ranges overlap, and local-alloc takes
the leftover $a0. Splitting the multiply from the shift reproduces target's
schedule exactly but flips angC back to $v0 (13). Registers XOR schedule; nothing
measured has both.

## FRONTIER (rewritten for session 4)

**A. Get target's mflo/sra placement WITHOUT losing the delayed-a0[2] register
win.** Mechanism: the candidate has target's registers and an early
`mflo`/`sra`; the split-multiply form has target's exact schedule and the wrong
registers. Both effects are decided in the same window (insns 17-26), so a form
that delays the shift by something OTHER than a source-level split — or that
delays a0[2] by something other than statement position — should be able to hold
both. Next probe: enumerate the window systematically. Vary independently (i) how
the sinA*sinB value is spelled (one statement / two / inline at each use / via a
third variable), (ii) where a0[2] is read relative to each of those, and (iii)
whether `idxB`/`idxC` locals are present, and read the resulting insn 17-26
window with `s3/sbs.py 15 32` for every form rather than only its score — the
score alone hid the fact that the two effects are separable.

**B. Instrumented-cc1 forensics on the sinAxsinB_12 quantity, now that it is the
ONLY residual.** Mechanism: the quantity is `used 3 times across 17 insns`
(priority 0.176) and takes whatever is left; the question is exactly which
register local-alloc considers and rejects for it at `find_free_reg` time and
whether $v0 is rejected for conflict or never reached. Next probe: build cc1 with
the BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG instrumentation named in the s1 ledger (or a
printf in `block_alloc`) and dump the qty order plus each `find_free_reg`
decision for BOTH the candidate (score 12) and the split form (score 13) — the
pair is a controlled A/B, which is far more informative than either alone.

**C. Port the score-12 form to the two family siblings.** Mechanism: `_SelectSection`
(10 rules) and `hirahira_w_ctrl_2` (63 rules, the signature twice) are the same
3x3 rotation-matrix idiom in the same file and their regfix comments name the same
two mechanisms; the delayed-last-parameter-use trick is a property of the original
source idiom, not of this function. One of them may have no sinAxsinB_12 residual
at all. Next probe: apply the candidate spelling to each and score with
`sandbox --disable all` (a measurement on a sibling, not a change of target).

## [s3] Delaying the read of a0[2] -- the LAST use of the parameter pointer a0 -- until after the 'sinAxsinB_12 = (sinA * sinB) >> 12;' statement flips angC from $v0 to target's $v1 and its '& 0xFFF'/'<< 1' cos-index temp from $v1 to target's $v0.
- mechanism: Statement position of a0[2] is not a spelling sched1 can normalise away: it is a LIVENESS fact about a hard register (where $a0 dies) plus it places the angC load after the mult in the stream local-alloc actually sees. Pass order confirmed from tools/gcc-2.7.2/toplev.c: combine (l.3004) -> sched1 (l.3028, enabled by -O2) -> local_alloc (l.3052) -> global_alloc -> sched2. Every s2/s3 spelling that only permuted arithmetic statement order was bit-identical because sched1 normalised it; this one is not normalisable.
- probe: On top of s2's score-13 candidate, moved 'angC = a0[2]; sinC = Judge[angC & 0xFFF];' from before to after the sinAxsinB_12 statement; sandbox --disable all; plus controls delaying it further (past cosB; past the whole cosA block), the same delay applied to a0[1] instead, and a side-by-side target-vs-build insn listing (s3/sbs.py).
- result: 12 / build_insns 114 (target 114), down from 13. The build now emits target's 'lhu v1,0x4(a0)' at insn 18 verbatim and the andi/sll cos-index chain on $v0, i.e. the mirror-image angC/temp swap is gone. Controls: delayed past cosB too -> also 12; delayed past the whole cosA block -> 90; same delay on a0[1] -> 13 (inert). Banked as candidate.c.
- verdict: CONFIRMED

## [s3] Emitting the a1[] output stores EAGERLY (each element stored as soon as it is computable), or permuting the tail store order, rewrites the live ranges local-alloc sees and can move mechanism B. (s2's frontier probe 3.)
- mechanism: Only two store placements had ever been tried in the combine-blocking slot; moving stores changes the pre-combine insn stream that sched1 and then local-alloc see.
- probe: Fully eager stores; the three sub-groups eager separately (cosA_* singles / the two sinAxsinB sums / the sinAxcosB pair); a1[0] instead of a1[5] in the blocking slot; four tail store-order permutations (reversed, index order, sums first, a1[8] first). Each scored with sandbox --disable all.
- result: Fully eager 73 / 109 insns (combine+CSE eat five instructions); sums eager 79/110; sinAxcosB pair eager 53/112; cosA_* singles eager 13 (inert); tail order index/sums-first 13 (inert), reversed 65/107, a1[8]-first 42/112; a1[0] in the blocking slot 12 (equivalent to a1[5]). Only the ONE combine-blocking store belongs out of the tail; every other store move either is inert or changes the insn count for the worse.
- verdict: KILLED

## [s3] Declaration order of the WHOLE local block steers local-alloc, because pseudo numbering is qty creation order and that is the tie-break among the large priority-1.00 quantity cluster.
- mechanism: qty_compare_1 breaks exact priority ties by qty number, and pseudos are created in expand_decl order, so renumbering the locals should flip the tie. (s2 had only permuted angC's own declaration line.)
- probe: Whole decl block reversed; all locals collapsed into one declaration group; declaration order set to first-use order. Scored with sandbox --disable all, and the .lreg dumps read to confirm the pseudo numbers actually changed.
- result: All three 13 / 114, bit-identical. The .lreg dumps DO show different pseudo numbers for the same quantity (188 vs 192) with an identical final assignment, so the tie-break is not reachable from declaration order.
- verdict: KILLED

## [s3] Type narrowing of the angle locals, or splitting the (s16) sign-extension into its own local, changes the quantity shape enough to move the allocation.
- mechanism: A narrower mode changes qty_size and the number of pseudos in the index chain, both inputs to qty_compare_1.
- probe: angC declared u16; angC declared s16; all three ang* declared u16; '(s16)angX' split into its own s32 local for A alone, for C alone, and for all three; plus that sign-extend local hoisted ahead of the sinC read. sandbox --disable all on each.
- result: All 13 / 114 -- CSE canonicalises the width away before allocation. Only the hoisted variant moved, and the wrong way: 30 / 113.
- verdict: KILLED

## [s3] Reducing the pseudo population (folding single-use intermediates into their consumers) or shortening sinAxsinB_12's live range (making its two consumers adjacent, raising its qty priority from 0.176) gets it out of the leftover register $a0 and into target's $v0.
- mechanism: priority = floor_log2(n_refs)*n_refs*size/live_length; sinAxsinB_12 is 'used 3 times across 17 insns' = 0.176, so it is allocated last and takes whatever is left. Shortening its range to ~4 insns would raise it to ~0.75 and let it claim a register instead of inheriting one.
- probe: Fold every single-use intermediate into its consumer; fold just the four cosA_* products; make prod_sinC and prod_cosC adjacent (with and without the shift written inline at both uses); write the shift inline at both uses without making them adjacent. sandbox --disable all on each.
- result: Folding all single-use locals 87 / 116; folding the cosA_* four 71 / 111; consumers adjacent 25 / 115 both ways (it costs an instruction); shift inline at both uses without adjacency 13 / 114. Every way of raising that priority pays an instruction, so the axis is dead at 114 insns.
- verdict: KILLED

## [s3] Splitting the multiply from the >>12 shift (and placing the shift after the sinC read) fixes sinAxsinB_12's register by delaying mflo/sra past the point where the cos-index temp dies.
- mechanism: Target's stream is mult @17, mflo t0 @22, sra v0,t0,12 @26 -- the shift lands AFTER the cosC index temp (which holds $v0 at 20-24) has died, so $v0 is free for sinAxsinB_12. Our build emits mflo v0 @19 / sra a0,v0 @20, before the chain, so the ranges overlap and local-alloc gives sinAxsinB_12 the leftover $a0.
- probe: 'sinAxsinB = sinA * sinB;' before the a0[2] read, then 'sinAxsinB_12 = sinAxsinB >> 12;' after the sinC read (and three further placements of the shift: after prod_sinC, after the cos index locals, after the cosC load), each scored and each read insn-by-insn with s3/sbs.py.
- result: The split DOES reproduce target's schedule exactly -- mflo t0 @22 and sra @26, matching target insn-for-insn through that window -- but it flips angC back to $v0 and the cos-index temp to $v1, scoring 13, i.e. it trades the win back. Other placements: 13, 16/116, 13. So the register fix and the schedule fix are individually reachable and, in every spelling measured, mutually exclusive.
- verdict: KILLED

## Session 4 (permuter) - RESOLVED

- H (s3 frontier 1): "the delayed-a0[2] register win and target's mflo/sra
  placement are separately reachable and can be held together." **CONFIRMED, and
  the join is in hand.** The lever is not a respelling of the multiply at all -
  it is adding an independent ready quantity (the hoisted cosB index add) to the
  scheduling window, which lets sched1 delay mflo/sra without disturbing the
  a0-last-use liveness fact that fixes angC/$v1. Score 12 -> 0.
- H (s3 frontier 2): "sinAxsinB_12 is refused $v0 for an identifiable reason at
  find_free_reg time." **CONFIRMED without needing the instrumented-cc1 forensics
  probe** - the reason was live-range overlap with the cos-index temp, and it was
  removable by scheduling rather than by allocation-order surgery. The
  BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG rebuild is no longer needed for this function.
- H (s3 frontier 3): "the closing idiom is a property of the shared 3x3
  rotation-matrix source idiom and will move the two siblings." **OPEN and now
  much more testable** - the full closing recipe (u16 staging read + interleaved
  blocking store + delayed a0[2] + hoisted index add) is known; apply it to
  _SelectSection and hirahira_w_ctrl_2.
