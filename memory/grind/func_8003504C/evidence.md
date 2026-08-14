# evidence — func_8003504C (src/code6cac_b.c)

## Session 1 (recon, 2026-08-13) — baseline map

Honest pure-C floor: **24** (`sandbox func_8003504C --disable all`).
`target_insns == build_insns == 141`. The residual is therefore **100%
register-assignment + intra-block instruction-ordering** — there is no missing
or extra instruction anywhere in the function, and no control-flow difference.
That is the single most important framing fact for every future session: nothing
needs to be *added* to or *removed from* the C; the C only has to steer GCC's
allocator/scheduler.

The 9 committed regfix rules corroborate exactly that shape (regfix.txt:673-685):
`$6 <-> $7 @ 4-44` (a whole-loop register swap), `reorder 4,9,10,5,6,7,8 @ 4-10`
(prologue/pre-loop block reordering), `reorder 45,52,48,53,54,46,47,49,50,51 @
45-54` plus six `subst "$3" "$2" @ 46-51` (a second reorder + a v1→v0 rename).
Two clusters, matching the two measured diff regions below.

### Instrument (reusable)
`tmp/grind/func_8003504C/s1/sidediff.py [obj]` prints a normalized
position-by-position target-vs-ours listing and a differing-position count.
It canonicalizes away the cosmetic noise between the split asm and objdump
(`$reg` prefixes, `move`/`li` aliases, `%hi`/`%lo` vs `0(` relocs, hex vs
decimal immediates, branch/jump displacements). Run from the repo root via
`bash tools/wsl.sh 'python3 tmp/grind/func_8003504C/s1/sidediff.py'`.
Baseline reports **26 differing positions** in exactly two contiguous clusters.
`tmp/grind/func_8003504C/s1/greg.sh` regenerates the full cc1 `-da` dump set
for code6cac_b.c into that same directory; `refs.py <pass>` reports the
per-pseudo `reg_n_refs` and first-set insn for the loop-1 pseudos.

### Cluster 1 — positions 4-12, 38-47: loop-1 register assignment + pre-loop order
Target: `i` (the 0..1 counter) lives in **$a3**, `src` (the walking read
pointer over p) lives in **$a2**. Ours is the exact inverse (`i`→$a2,
`src`→$a3), which is what the `$6 <-> $7 @ 4-44` regfix rule paper-overs.
Additionally target materializes the two loop-invariant constants 5 and 20
(`li $t2,5`, `li $t1,20`) EARLY — at positions 5,6, before the `move $t3,$v0`
/ `move $a2,$t3` pointer copies and before the `base`/`ptr` lui/addiu pair —
whereas ours emits them last (positions 10,11). All of positions 4..11 are one
basic block, so that ordering is a sched1 priority difference, not a source
statement-order difference.

RTL evidence (`code6cac_b.i.greg`, `.lreg`, this session's dumps):
`;; Register dispositions:` gives `72 in 11  73 in 6  74 in 7  75 in 5
76 in 8`, i.e. pseudo 72 = `p` → $t3, **73 = `i` → $a2 (6)**, **74 = `src` →
$a3 (7)**, 75 = `ptr` → $a1, 76 = `base` → $t0. Measured `reg_n_refs`:
72=19, **73=7**, 75=7, **74=5**, 76=3. GCC 2.7.2 `global.c` orders allocnos by
`reg_n_refs / live_length`, so with equal live lengths `i` (7 refs) outranks
`src` (5 refs) and takes the earlier hard reg in the MIPS allocation order —
producing our inverse assignment. **To match, `src` must outrank `i`.**

### Cluster 2 — positions 51-61: the p[5]/p[8] bitfield-extraction block
Target emits the two extractions strictly **serialized**, both through **$v0**,
and hoists the `D_80102785` read (`lui $v1` + `lb $v1`) up into the load-delay
region of `lw $v0,0x14($t3)`:

    lw v0,0x14(t3) ; lui v1,%hi(D_80102785) ; lb v1,%lo(D_80102785)(v1)
    srl v0,4 ; andi v0,0x3F ; lui at ; sb v0,%lo(D_80102784)(at)
    lw v0,0x20(t3) ; lui at ; sh zero,%lo(D_800A36F6)(at)
    srl v0,3 ; andi v0,1 ; lui at ; sb v0,%lo(D_80102786)(at)

Ours interleaves the two chains and puts the second one in **$v1**: it hoists
`lw v1,0x20(t3)` and the `sh zero` above the first chain's `srl`, finishes the
`D_80102786` store first, and emits the `D_80102785` `lb` late (position 59-60)
and the `D_80102784` store last. Note that in target the `sh zero`
(`D_800A36F6 = 0`) sits *between* `lw v0,0x20(t3)` and its consumer — i.e. it
is a sched1 delay-slot fill by an instruction that came *later* in program
order, which is a hint about the original statement order in this block.

## Facts that are NOT hypotheses (measured this session)
- Floor 24 with 141/141 instructions: register + ordering only.
- The two diff clusters are independent in position but may be coupled through
  the same allocation pass; cluster 2's register choice ($v0 vs $v1) shifted
  when cluster-1-affecting edits were made, so re-measure cluster 2 after any
  cluster-1 change rather than treating the two as separable.
- The allocation flip in cluster 1 **is reachable** — see hypotheses.md H3: a
  live-range extension of `i` moved `src` into $a2 exactly as target wants
  (positions 12/38/39 went clean). The mechanism is confirmed; only the
  *amount* of the priority shift was wrong.

- [s1] Honest pure-C floor is 24 with target_insns == build_insns == 141: the residual is 100% register-assignment plus intra-block instruction ordering. Nothing needs to be added to or removed from the C, and there is no control-flow difference anywhere in the function.

- [s1] The 9 committed regfix rules (regfix.txt:673-685) corroborate exactly that shape: `$6 <-> $7 @ 4-44` (whole-loop register swap), `reorder 4,9,10,5,6,7,8 @ 4-10`, `reorder 45,52,48,53,54,46,47,49,50,51 @ 45-54` plus six `subst "$3" "$2" @ 46-51` (a v1->v0 rename). Two rule clusters, two measured diff clusters.

- [s1] Cluster 1 (positions 4-12, 38-47): target puts the 0..1 counter `i` in $a3 and the walking read pointer `src` in $a2; ours is the exact inverse. Target also materializes the two loop-invariant constants 5 and 20 early (positions 5,6, ahead of the `move $t3,$v0` / `move $a2,$t3` pair and the base/ptr lui/addiu), ours emits them last (positions 10,11) - and all of positions 4..11 are one basic block, so that is a sched1 priority difference, not a source statement-order difference.

- [s1] RTL evidence from this session's cc1 -da dumps: `;; Register dispositions:` reads `72 in 11  73 in 6  74 in 7  75 in 5  76 in 8`, i.e. p=$t3, i=$a2(6), src=$a3(7), ptr=$a1, base=$t0. Measured reg_n_refs: p=19, i=7, ptr=7, src=5, base=3. With equal live lengths, i (7 refs) outranks src (5 refs) and takes the earlier hard reg - that is precisely the inversion. To match, src must outrank i.

- [s1] Cluster 2 (positions 51-61): target emits the two bitfield extractions strictly serialized, both through $v0, and hoists the D_80102785 read (lui $v1 + lb $v1) into the load-delay region of `lw $v0,0x14($t3)`; the `sh zero` for D_800A36F6 sits between `lw $v0,0x20($t3)` and its consumer, i.e. it is a sched1 delay-slot fill by a LATER-in-program-order instruction - a hint about the original statement order. Ours interleaves the two chains, puts the second in $v1, finishes the D_80102786 store first, and emits the D_80102785 lb late.

- [s1] The two clusters are positionally independent but coupled through the same allocation pass: cluster 2's register choice ($v0 vs $v1) shifted under every cluster-1-affecting edit, so cluster 2 must be re-measured after any cluster-1 change rather than treated as separable.

- [s1] Reusable instruments were built and are committed to the scratch dir: sidediff.py (normalized position-by-position target-vs-ours listing; canonicalizes $reg prefixes, move/li aliases, %hi/%lo vs reloc-0, hex vs decimal immediates, and branch displacements - baseline reports 26 differing positions), greg.sh (regenerates the full cc1 -da dump set for code6cac_b.c), refs.py (per-pseudo reg_n_refs + first-set insn).

## Session 2 (structural, 2026-08-13) — the cluster-1 allocation model, solved

Floor unchanged at 24 (HEAD form re-confirmed at session end, `git status src/`
clean). This session did not lower the floor; it CLOSED the cluster-1
register-inversion axis with an exact model of GCC 2.7.2 `global.c`, and built
the instruments that turn every future allocation question into one command.

### New instruments (tmp/grind/func_8003504C/s2/)
- `apply.py <variant.c>` — splice a whole-function variant into src/code6cac_b.c.
- `probe.sh` — regenerate the cc1 `-da` dumps for the CURRENT src and print, for
  func_8003504C: every pseudo's `refs`/`live_length` (parsed from the `.flow`
  dump's "Register N used R times across L insns"), the post-qsort allocno order
  (`;; N regs to allocate:` in `.greg`), and the register-disposition line.
- `sweep.sh <variant.c>...` — apply+probe a list of variants, then revert src.
- `findreg.sh <pseudo>` — run the INSTRUMENTED cc1 (`tools/gcc-2.7.2/cc1`, NOT
  `build/cc1`) with `BB2_FINDREG_DEBUG=<pseudo>` and print that pseudo's hard-reg
  conflicts / someone_prefers / used_so_far / pass0 / pass1 exclusion sets.

### The exact allocation model (read out of tools/gcc-2.7.2/global.c)
`allocno_compare` sorts by `pri = floor_log2(refs) * refs / live_length` (times
allocno_size, = 1 for all of these). `flow.c` accumulates `reg_n_refs[r] +=
loop_depth` per OCCURRENCE, with loop_depth 1 outside any loop-note region and 2
inside loop 1 — a ref inside a `do/while` counts DOUBLE. `find_reg` then walks
REG_ALLOC_ORDER and takes the first hard reg that is neither conflicting nor
(pass 0) preferred by someone else.

Measured baseline (HEAD form); pseudos 72=p, 73=i, 74=src, 75=ptr, 76=base:

    p    refs=12 len=103  pri = 3*12/103 = 0.350   -> $t3
    i    refs=11 len=37   pri = 3*11/37  = 0.892   -> $a2   (target: $a3)
    src  refs=9  len=36   pri = 3*9 /36  = 0.750   -> $a3   (target: $a2)
    ptr  refs=11 len=34   pri = 3*11/34  = 0.971   -> $a1   (matches target)
    base refs=4  len=34   pri = 2*4 /34  = 0.235   -> $t0   (matches target)

The `;; regs to allocate` order is exactly that ranking, and `findreg.sh 73` shows
i's hard-reg conflicts are exactly {2,3,4,5} = $v0,$v1,$a0,$a1 with an EMPTY
someone_prefers set — so i takes $a2 as the first free reg in REG_ALLOC_ORDER,
deterministically; src then inherits the conflict on $a2 and takes $a3.

### The target's own compile had the SAME ref counts
The occurrences are visible in target's asm and are identical to ours. $a3 (=i):
`addu $a3,$zero,$zero` (pre-loop, weight 1), `addu $at,$at,$a3` x2,
`addiu $a3,$a3,1` (2 occurrences), `slti $v0,$a3,0x2` — 5 in-loop occurrences x2
+ 1 = **11**. $a2 (=src): `move $a2,$t3` (weight 1), `lbu $v0,0($a2)`,
`lbu $v0,1($a2)`, `addiu $a2,$a2,0xA` (2 occurrences) — 4 in-loop x2 + 1 = **9**.
The hoisted `li $t2,5` / `li $t1,20` prove loop 1 carried NOTE_INSN_LOOP_BEG in
the original too (that hoist is loop.c LICM), so the depth-2 weighting applied
there as well. Hence pri_i = 33/L_i and pri_src = 27/L_src in the original, and
for src to be allocated first it needs **L_i > 1.222 * L_src**.

### Why that is unreachable (the closure)
Both i and src are live-in and live-out of every block of the loop body (a
loop-carried value is live across the back edge), so both carry the full body
length B (measured B ~= 33). They can differ ONLY by the number of pre-loop insns
between their two initializations — and the entire pre-loop block is 8 insns
(target positions 4-11), of which src's `move $a2,$t3` must follow `move $t3,$v0`.
Measured extremes: baseline L_i=37 / L_src=36; src-initialized-last L_i=37 /
L_src=34, i.e. a best achievable ratio of **1.088** against a requirement of
1.222. Session 1's H3 frontier ("one discrete position weaker live-range
extension") is therefore not mis-calibrated — the axis has no such position.

### Loop-note removal compresses the gap but not the requirement
Spelling loop 1 as an `if/goto` loop (no NOTE_INSN_LOOP_BEG) drops every in-loop
weight from 2 to 1: measured refs become i=6, src=5, ptr=6, base=3. Both i and
src stay in the same `floor_log2 = 2` bucket, so the requirement is still
L_i > 1.2 * L_src and the achievable ratio is still 37/34. It also shifts the
whole assignment DOWN one register ($a0/$a2/$a3 instead of $a1/$a2/$a3) because
p's allocno overtakes ptr's in the reordered ranking — strictly worse.

### Strength-reduction (giv) provenance does not change it either
Rewriting loop 1 with pure `i * 10` indexing so the walkers become loop.c-created
givs leaves the giv allocated AFTER i in every case (same 27/L priority shape), so
i still takes $a2. vE (both walkers as givs): 142 insns, score 32 — the
D_8010277C giv costs an extra `lui`+`addiu`, because target derives that pointer
as `addiu $a1,$t0,-0x9` from the D_80102785 address, which independently proves
the original had the source-level `base` / `ptr = base - 9` relationship HEAD
already spells. vF (only the p-walk as a giv, HEAD's base/ptr kept): 141 insns,
score 26, identical inversion.

### What this leaves for cluster 1
Every knob feeding `allocno_compare` is measured and closed: refs are pinned by
the (fixed, 141/141) instruction stream, live lengths are pinned by loop-carried
liveness, loop-note weighting scales both sides equally, and giv provenance does
not reorder them. `find_reg` cannot be steered either — i's hard-reg conflicts
come only from $v0/$v1/$a0 (in-loop temporaries) and $a1 (ptr, allocated
earlier), ANY pseudo conflicting with i inside the body also conflicts with src
(so none can take $a2 "on i's behalf"), and `regs_someone_prefers` is empty for
every pseudo because the function has no argument-register copies at all (both
calls are 0-arg), so the pass-0 preference skip can never fire. Treat cluster 1
as allocation-CLOSED under the current instruction stream; spend effort on
cluster 2 (untouched this session), or re-open cluster 1 only against a premise
this model does not cover (a source shape in which the loop-1 counter is not one
pseudo spanning the whole body).

- [s2] allocno_compare in GCC 2.7.2 global.c ranks by floor_log2(refs)*refs/live_length; flow.c weights each ref occurrence by loop_depth (1 outside a loop-note region, 2 inside loop 1). Measured baseline priorities ptr 0.971 > i 0.892 > src 0.750 > base 0.235 reproduce the observed allocation order and the observed $a1/$a2/$a3/$t0 assignment exactly.

- [s2] BB2_FINDREG_DEBUG=73 on the instrumented cc1 (tools/gcc-2.7.2/cc1) shows i's hard-reg conflicts are exactly {$v0,$v1,$a0,$a1} with an EMPTY someone_prefers set, so i takes $a2 as the first free register in REG_ALLOC_ORDER; src then conflicts with $a2 and takes $a3. The function has no argument-register copies (both calls are 0-arg), so no copy preference can ever populate someone_prefers here.

- [s2] The target's own asm pins the original compile's ref counts to the same 11 (i) and 9 (src) - 5 vs 4 in-loop occurrences at weight 2 plus one pre-loop init at weight 1 - and the hoisted li 5 / li 20 prove loop 1 had loop notes in the original. So src outranking i requires L_i > 1.222*L_src.

- [s2] i and src are both live across the whole loop body (loop-carried, live across the back edge), so their live lengths differ only by the pre-loop distance between their initializations, and the entire pre-loop block is 8 insns. Measured extremes give L_i/L_src = 37/34 = 1.088 against the 1.222 requirement: the live-range-extension axis is arithmetically closed, not merely uncalibrated.

- [s1] src/ was left byte-identical to HEAD (git diff --stat src/ is empty) and the floor was re-confirmed at 24 after reverting every probe. No cheat construct was written at any point in this session: no pins, no inline asm, no dead stores, no volatile coercion, no unused locals.

- [s2] GCC 2.7.2 global.c ranks allocnos by floor_log2(refs)*refs/live_length (allocno_compare), and flow.c adds loop_depth per ref occurrence (1 outside a loop-note region, 2 inside loop 1). Measured baseline priorities ptr 0.971 > i 0.892 > src 0.750 > base 0.235 reproduce both the observed allocno order and the observed $a1/$a2/$a3/$t0 assignment exactly.

- [s2] Measured baseline pseudo table (72=p, 73=i, 74=src, 75=ptr, 76=base): p refs=12 len=103, i refs=11 len=37, src refs=9 len=36, ptr refs=11 len=34, base refs=4 len=34.

- [s2] Target's own asm pins the ORIGINAL compile's weighted ref counts to the same values: $a3 (i) has 5 in-loop occurrences plus one pre-loop init = 11; $a2 (src) has 4 in-loop occurrences plus one pre-loop init = 9. The hoisted li 5 / li 20 prove loop 1 had NOTE_INSN_LOOP_BEG in the original, so the same depth-2 weighting applied.

- [s2] i and src are both loop-carried, hence live across every block of the loop body; their live lengths can differ only by the pre-loop distance between their initializations, and the whole pre-loop block is 8 insns. Best measured ratio 37/34 = 1.088 against a 1.222 requirement - the live-range-extension axis is arithmetically closed, not mis-calibrated.

- [s2] BB2_FINDREG_DEBUG on the instrumented cc1 shows i's hard-reg conflicts = {$v0,$v1,$a0,$a1} and an empty someone_prefers set; the function has no argument-register copies at all (both calls 0-arg), so the pass-0 preference skip can never fire here.

- [s2] Spelling loop 1 as an if/goto loop removes the loop notes and halves every in-loop weight (measured refs i=6, src=5, ptr=6, base=3) but leaves i and src in the same floor_log2 bucket, so the flip requirement is unchanged; it also shifts the group down one register ($a0/$a2/$a3), which is strictly worse.

- [s2] New reusable instruments in tmp/grind/func_8003504C/s2: apply.py (splice a variant), probe.sh (per-pseudo refs/live_length from the .flow dump + post-qsort allocno order + dispositions), sweep.sh (apply+probe a list, then revert), findreg.sh (BB2_FINDREG_DEBUG exclusion sets from the instrumented cc1).

- [s2] Session end state: src/ reverted to HEAD and re-verified at sandbox score 24, 141/141 instructions; no cheat construct was written at any point (no pins, no inline asm, no dead stores, no volatile coercion, no unused locals).
