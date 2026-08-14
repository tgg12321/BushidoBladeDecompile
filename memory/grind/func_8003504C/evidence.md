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

## Session 3 (structural, 2026-08-13) — cluster 2 closed on statement order

Floor unchanged at 24 (HEAD form re-confirmed; `git status src/` clean at session
end). This session did not lower the floor; it CLOSED the cluster-2
statement-order axis by exhaustive sweep, pinned two source-spelling facts with
positive evidence, and localized the pre-loop residual to a single unexplained
sched1 decision.

### New instruments (tmp/grind/func_8003504C/s3/)
- `gen_perm.py` — emits the 24 permutations of the cluster-2 statement block.
- `sweep_score.sh <v>...` — apply + `sandbox --disable all` for a list of
  variants, printing score / target_insns / build_insns, then reverts src.
- `sweep_diff.sh` / `sweep_bad.sh` / `one_diff.sh` — same, but printing the
  normalized position diff (windowed, differing-only, or single-variant).
- `head_diff.sh <first> <last>` — position diff of the CURRENT src over a window.
- `rtlblock.py <dump>` — one compact line per RTL insn of func_8003504C for any
  cc1 `-da` dump (uid, code, squashed body); this is how the sched1 in/out order
  was read without dumping raw RTL into context.
- `rank.sh` — runs the instrumented cc1 with `BB2_RANK_DEBUG=1` (the hook lives
  in `rank_for_schedule`, tools/gcc-2.7.2/sched.c). CAVEAT: its output is not
  yet scoped to func_8003504C, and insn uids collide across the functions in
  code6cac_b.c — a next session must add a function filter before trusting it.

### The exhaustive cluster-2 sweep (the closure)
The block is four mutually independent statements — nothing in the function
writes D_80102785 and the three destinations are distinct objects — so all 24
orderings are the same program:
  A `D_80102784 = ((u32)p[5] >> 4) & 0x3F;`   B `D_800A36F6 = 0;`
  C `D_80102786 = ((u32)p[8] >> 3) & 1;`      D `val = D_80102785;`

All 24 measured. The final asm takes exactly TWO shapes:
- The 6 orders with D last (ABCD ACBD BACD BCAD CABD CBAD): 141 insns, score 24,
  and the normalized diff is BYTE-IDENTICAL to HEAD's — cluster 2 included.
  Permuting A/B/C among themselves is completely codegen-neutral.
- Every order with D not last: 143 insns, score 24 or 27.

So cluster 2 is invariant under source statement order. Whatever sets its final
order is downstream of sched1's block schedule.

### Why moving the `val` read costs 2 insns (a source-spelling pin)
With `s8 val`, a read adjacent to its first use is folded by combine into one
`(set (reg) (sign_extend:SI (mem:QI ...)))` = `extendqisi2_insn` = `lb`. Moving
the read away from the compare leaves val as a QImode pseudo and cc1 emits
`lbu` + `sll 24` + `sra 24`. Declaring `s32 val` removes that penalty (all four
probed orders came back at 141 insns) but makes the load `lbu` where target has
`lb`: with val's only uses being equality compares against 2 and 5, combine
narrows the sign_extend to a zero_extend. **`s8 val`, read LAST, is therefore
positively corroborated by target** — both the `lb` and the +0-insn cost pin it.

### What actually shapes cluster 2 (read out of the RTL dumps)
sched1's output order for the block is `122(lw p[5]) 131(sh zero) 123(srl)
134(lw p[8]) 126(andi) 128(sb D_80102784) 135 138 140(sb D_80102786) 146(lb val)`
— i.e. sched1 INTERLEAVES the two extraction chains. That interleave gives the
two chains overlapping live ranges, so lreg must give the second chain its own
hard reg ($v1), and sched2 then produces the emitted order. Target's block is
strictly serialized through $v0 (`122 146 123 126 128 134 131 135 138 140`),
which is what a NON-interleaved sched1 output would allow. The residual is thus
one sched1 decision (hoisting insn 134, the p[8] load, above insn 128, the
D_80102784 store) plus its downstream consequences — and that decision does not
respond to statement order.

`rank_for_schedule` (tools/gcc-2.7.2/sched.c) is: INSN_PRIORITY first; then a
3-way class on dependence w.r.t. the last scheduled insn; then INSN_LUID
(original program order) as the stable tie-break. Since every source order gives
the same schedule, the priorities — not the LUIDs — decide here.

### Pre-loop block: the LICM hoist-order mechanism, half confirmed
Target's pre-loop block is `move $a3,zero / li $t2,5 / li $t1,20 / move $t3,$v0 /
move $a2,$t3 / lui $t0 / addiu $t0 / addiu $a1,$t0,-9`; ours is
`move $a2,zero / move $t3,$v0 / move $a3,$t3 / lui $t0 / addiu $t0 /
addiu $a1,$t0,-9 / li $t2,5 / li $t1,20`. Probe vP3 (drop the `base`/`ptr`
locals; spell those accesses as direct i-indexed globals inside the loop) DID
move the two hoisted constants ahead of the address setup (positions 7,8 with
lui/addiu at 9,10), confirming that constants-vs-address order in that block is
decided by loop.c's hoist order — an address computed inside the loop is hoisted
into the preheader AFTER the constants hoisted from the loop condition. But vP3
scores 30 / 44 differing positions, because it loses target's
`addiu $a1,$t0,-9` derivation (independent evidence, matching session 2's giv
result, that the original had the `base` / `ptr = base - 9` relationship).

What remains UNEXPLAINED in that block: target also has `move $t3,$v0` and
`move $a2,$t3` AFTER the constants. `move $t3,$v0` has the lowest LUID in the
block and feeds `move $a2,$t3`, so its INSN_PRIORITY strictly exceeds a
constant's — under the same RTL it can never be scheduled after them. Target's
RTL for the block must therefore differ structurally from ours in a way no
probe so far reproduces. That is the sharpest open question on the function.

- [s3] All 24 orderings of the four independent cluster-2 statements (D_80102784 store, D_800A36F6=0, D_80102786 store, val=D_80102785) were measured with sandbox --disable all. The final asm takes exactly two shapes: the six orders with the val read LAST are byte-identical to HEAD (141 insns, score 24, same 26 differing positions including all of cluster 2), and every order with the val read earlier is 143 insns at score 24-27. Cluster 2 is invariant under source statement order.

- [s3] Moving the `val = D_80102785;` read away from its first use costs exactly 2 insns because combine only folds the read into a single extendqisi2_insn (`lb`) when it is adjacent to the compare; otherwise val stays a QImode pseudo and cc1 emits lbu + sll 24 + sra 24.

- [s3] Declaring `s32 val` instead of `s8 val` removes that 2-insn penalty (141 insns in all four probed orders) but emits `lbu` where target has `lb`: with val's only uses being equality compares against 2 and 5, combine narrows the sign_extend to a zero_extend. `s8 val` read last is positively corroborated by target.

- [s3] Read out of the cc1 -da dumps with the new rtlblock.py instrument: sched1's output for the cluster-2 block is 122(lw p[5]) 131(sh zero) 123(srl) 134(lw p[8]) 126(andi) 128(sb D_80102784) 135 138 140(sb D_80102786) 146(lb val) - the two extraction chains are INTERLEAVED at sched1, which forces overlapping live ranges, which is why lreg gives the second chain $v1 and sched2 emits the interleaved form. Target's block (122 146 123 126 128 134 131 135 138 140) is strictly serialized, which is what an uninterleaved sched1 output permits. The residual is one sched1 decision - hoisting insn 134 (the p[8] load) above insn 128 (the D_80102784 store) - and it does not respond to statement order.

- [s3] rank_for_schedule (tools/gcc-2.7.2/sched.c) orders by INSN_PRIORITY, then by a 3-way dependence class relative to the last scheduled insn, then by INSN_LUID (original program order) as a stable tie-break. Because every source permutation yields the identical schedule, cluster 2's residual is a PRIORITY effect, not a LUID/program-order effect.

- [s3] Probe vP3 (drop the base/ptr locals, spell the D_8010277C-area accesses as direct i-indexed globals inside loop 1) moved the two LICM-hoisted constants li 5 / li 20 AHEAD of the address lui/addiu in the pre-loop block, confirming that the constants-vs-address order there is set by loop.c's hoist order (an address computed inside the loop is hoisted after the constants hoisted from the loop condition). It scores 30 / 44 differing positions because it loses target's `addiu $a1,$t0,-9` derivation - a second, independent corroboration of the `base` / `ptr = base - 9` source relationship.

- [s3] The pre-loop block's remaining puzzle is sharply localized: target emits `move $t3,$v0` and `move $a2,$t3` AFTER the two hoisted constants, but `move $t3,$v0` has the lowest LUID in the block and feeds `move $a2,$t3`, so its INSN_PRIORITY strictly exceeds a constant's and it can never be scheduled after them under our RTL. Target's RTL for that block must differ structurally in a way no probe so far reproduces.

- [s3] Session end state: src/ reverted to HEAD and re-verified at sandbox score 24, 141/141 instructions. No cheat construct was written at any point in this session - no pins, no inline asm, no dead stores, no volatile coercion, no unused locals; every probed variant was a plain re-spelling of live, semantically-necessary statements.

- [s3] All 24 orderings of the four independent cluster-2 statements were measured. Six orders (the ones with the val read last) produce output byte-identical to HEAD at 141 insns / score 24; the other 18 produce 143 insns at score 24-27. Cluster 2 is invariant under source statement order — session 2's top frontier item is closed.

- [s3] Moving the `val = D_80102785;` read away from its first use costs exactly 2 insns: combine only folds the read into a single extendqisi2_insn (`lb`) when it is adjacent to the compare; otherwise val stays a QImode pseudo and cc1 emits lbu + sll 24 + sra 24.

- [s3] Declaring `s32 val` removes that 2-insn penalty (141 insns in all four probed orders) but emits `lbu` where target has `lb` — with val's only uses being equality compares against 2 and 5, combine narrows the sign_extend to a zero_extend. `s8 val`, read LAST, is therefore positively corroborated by target.

- [s3] sched1's output order for the cluster-2 block (read with the new rtlblock.py instrument) is 122(lw p[5]) 131(sh zero) 123(srl) 134(lw p[8]) 126(andi) 128(sb D_80102784) 135 138 140(sb D_80102786) 146(lb val): the two extraction chains are INTERLEAVED at sched1, which forces overlapping live ranges, which is why lreg gives the second chain $v1 and sched2 emits the interleaved form. Target's block (122 146 123 126 128 134 131 135 138 140) is strictly serialized through $v0. The whole residual is one sched1 decision — hoisting insn 134 (the p[8] load) above insn 128 (the D_80102784 store).

- [s3] rank_for_schedule (tools/gcc-2.7.2/sched.c) orders by INSN_PRIORITY, then by a 3-way dependence class relative to the last scheduled insn, then by INSN_LUID as a stable tie-break. Because every source permutation yields the identical schedule, cluster 2's residual is a priority effect, not a program-order effect.

- [s3] The pre-loop residual is now sharply localized: target emits `move $t3,$v0` and `move $a2,$t3` AFTER the two LICM-hoisted constants, but `move $t3,$v0` has the lowest LUID in the block and feeds `move $a2,$t3`, so its INSN_PRIORITY strictly exceeds a constant's and it can never be scheduled after them under our RTL. Target's RTL for that block must differ structurally from ours in a way no probe so far reproduces.

- [s3] The instrumented cc1 (tools/gcc-2.7.2/cc1) already carries a BB2_RANK_DEBUG hook inside rank_for_schedule that prints every priority-TIE decision (last scheduled insn, both uids, both classes). It is usable but NOT yet scoped to one function — insn uids collide across the functions in code6cac_b.c, so tmp/grind/func_8003504C/s3/rank.sh needs a function filter before its output can be trusted.

- [s3] Session end state: src/ reverted to HEAD and re-verified at score 24 / 141 of 141 instructions / 26 differing positions. No cheat construct was written at any point: no pins, no inline asm, no dead stores, no volatile coercion, no unused locals — every probed variant was a plain re-spelling of live, semantically necessary statements.

## Session 4 (permuter, 2026-08-13) — floor 24 -> 4; both clusters essentially closed

The first session to LOWER the floor. `sandbox func_8003504C --disable all`
went **24 -> 17 -> 13 -> 4**, and the normalized position diff went **26 -> 7**.
The remaining 7 positions are a pure ROTATION of a single 7-instruction
pre-loop block — every instruction is present with the correct register.
The winning form is `memory/grind/func_8003504C/candidate.c` and it is spliced
into `src/code6cac_b.c` as of session end.

### Instruments (tmp/grind/func_8003504C/s4/)
- `mkws.sh <wsdir>` — build a decomp-permuter workspace for func_8003504C that
  mirrors the REAL pipeline for code6cac_b.c (cpp | cc1 -mel -O2 -G0 |
  prologue_fix | maspsx --expand-lb (twice, per EXPAND_LB_FILES) |
  `sed .align 3 -> .align 2` (RODATA_ALIGN2_FILES) | multu_pad | as), with
  regfix/asmfix DELIBERATELY OMITTED so the permuter's metric is the honest,
  cheat-free one. Extracts the func's `.ent`..`.end` region and assembles it
  against `asm/funcs/func_8003504C.s` + the r3000-trimmed permuter prelude.
  NOTE: maspsx emits its directives at column 0, so the extraction regex must
  be `^[ \t]*\.ent[ \t]+<func>$`, not `^\t\.ent\t<func>$`.
- `mkbase.sh <wsdir> [srcfile]` — preprocess src/code6cac_b.c into
  `<wsdir>/base.c` (cpp line markers stripped), compile it through the
  workspace and print insn counts + the raw diff. A FULL preprocessed TU is
  the right base.c: decomp-permuter's randomizer only mutates the function
  named in settings.toml (`randomizer.py: extract_fn(ast, fn_name)`), so the
  rest of the TU costs only parse/print time and keeps the codegen context real.
- `showfinds.sh <wsdir>` — print the source diff of every `output-*` find,
  best score first. `dd.sh [variant.c]` — apply a variant, sandbox it, and
  print ONLY the differing normalized positions (target vs ours, side by side).
- `gen4.py` — emits the 24 cluster-2 variants (4 intermediate spellings x 6
  statement orders). `mkreuse.py` — builds the `src`-reuse variant.

### Campaign telemetry (all three campaigns harvested + stopped in-session)
- ws1 `head-chassis-random`: base permuter score 1120; **first novel find at
  15 s**; 9742 iterations / 300 s; best new 315. Stopped once the lever was
  identified.
- ws2 `acbv-ptrtemp-chassis`: base 175; first novel at 132 s; 9742 iterations /
  300 s; best new 120 (the `src`-reuse find). Stopped when it superseded.
- ws3 `reuse-src-chassis-floor4`: base 120; **20373 iterations / 610 s with NO
  find below 120** (one score-120 tie). The pre-loop rotation does not fall out
  of random search on this chassis — that is the session's negative datum.

### Lever 1 (cluster 2, 24 -> 13): a pointer intermediate for the p[8] read
Staging the eighth-word read through a named POINTER local —
`q = &p[8]; D_80102786 = ((u32)*q >> 3) & 1;` — keeps a live pointer pseudo
across the first extraction chain, so sched1 can no longer hoist the p[8] load
above the `D_80102784` store. The two chains stop interleaving, their live
ranges stop overlapping, and lreg reuses $v0 for the second chain instead of
allocating $v1 — exactly target's strictly-serialized block.
The spelling matters and was measured across all four candidates the permuter
surfaced: `s32 v8 = p[8];` -> 24, `u32 v8 = p[8];` -> 24 (both folded straight
back into the shift), `i = p[8];` (reusing the dead counter) -> 20,
`q = &p[8];` -> **17**.

### Lever 2 (cluster 2, 17 -> 13): D_800A36F6 = 0 BETWEEN the two extractions
With the pointer intermediate in place, statement order comes ALIVE again —
which retires session 3's exhaustive-sweep closure as chassis-specific. All
six orders measured on the pointer chassis: A-B-C-val 17, A-C-B-val **13**,
A-val-C-B 16, A-C-val-B 16, A-val-B-C 19 (143 insns), A-B-val-C 19 (143 insns).
The winner puts the `D_800A36F6 = 0;` store between the two bitfield
extractions and keeps the `val = D_80102785;` read LAST (session 3's `s8 val`
read-last finding still holds). At 13, cluster 2 has ZERO differing positions.

### Lever 3 (cluster 1, 13 -> 4): reuse the dead `src` for loop 2's destination
`src` is dead after loop 1. Reusing it to carry loop 2's D_801027D8
destination pointer (`src = &D_801027D8;` then `u8 *dst_d = src;`) extends its
live range into the `val == 5` branch and FLIPS the allocation to target's:
measured dispositions `72 in 11 (p=$t3), 73 in 7 (i=$a3), 74 in 6 (src=$a2),
75 in 5 (ptr=$a1), 76 in 8 (base=$t0)` — identical to target. Positions 4, 12,
38, 39, 41, 44, 46, 47 all went clean in one step.

### The allocation model from session 2 is EMPIRICALLY DISPROVEN
Post-flip measurements (tmp/grind/func_8003504C/s2/probe.sh on the winning
form): `i` refs=11 len=37 ratio=0.297, `src` refs=11 len=43 ratio=0.256 — by
the `floor_log2(refs)*refs/live_length` model session 2 built, `i` (0.892)
still outranks `src` (0.767), yet the post-qsort allocno order printed by
`.greg` is `... 75 74 73 ...`, i.e. src BEFORE i, and src gets $a2. So either
`allocno_live_length` differs from the `.flow` "used R times across L insns"
figure the probe parses, or another allocno_compare term dominates. Session
2's conclusion ("the axis has no reachable position") is therefore WRONG as a
closure — the flip is reachable and has been reached. Treat the probe's ratio
column as a heuristic, not as a decision procedure, and never close an axis on
that model alone again.

### What remains (the whole residual)
7 positions, all in the pre-loop block, all present-with-correct-register:
target `li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 /
addiu a1,t0,-9`; ours `move t3,v0 / move a2,t3 / lui t0 / addiu t0 /
addiu a1,t0,-9 / li t2,5 / li t1,20`. The two loop-1 comparison constants (5
and 20, consumed by `beq a0,t2` and `bne a0,t1` inside loop 1) are LICM-hoisted
and land at the END of the preheader in our compile, where sched1 leaves them
because they have no in-block dependents and therefore the lowest INSN_PRIORITY
in the block, while `move t3,v0 -> move a2,t3` is a 2-deep chain. For target to
emit them FIRST they must either carry a higher priority (an in-block
dependent our RTL does not give them) or have a LOWER LUID than the p-copy —
i.e. exist in the entry block BEFORE the call's return copy rather than being
appended to the preheader by loop.c.

- [s4] FLOOR LOWERED 24 -> 4 (141/141 instructions, 26 -> 7 differing normalized positions). Three edits, all originating as decomp-permuter proposals and each re-measured with `sandbox --disable all`: (1) stage the p[8] read through a named pointer intermediate `q = &p[8]`; (2) order the post-loop block as D_80102784-store / p[8]-extraction / `D_800A36F6 = 0` / `val = D_80102785`; (3) reuse the dead `src` local to carry loop 2's D_801027D8 destination pointer.

- [s4] Cluster 2 is CLOSED (zero differing positions). Mechanism: taking the ADDRESS of the eighth word keeps a live pointer pseudo across the first extraction chain, so sched1 can no longer hoist the p[8] load above the D_80102784 store; the two chains stop interleaving, their live ranges stop overlapping, and lreg reuses $v0 for the second chain instead of allocating $v1 — target's strictly-$v0-serialized block.

- [s4] The intermediate's SPELLING is load-bearing and all four permuter-surfaced candidates were measured: `s32 v8 = p[8];` -> score 24 and `u32 v8 = p[8];` -> 24 (combine/CSE folds the value temp straight back into the shift), `i = p[8];` (dead-counter reuse) -> 20, `q = &p[8];` -> 17. Only the POINTER form survives the fold.

- [s4] Session 3's exhaustive cluster-2 statement-order sweep is CHASSIS-SPECIFIC, not a closure: with the pointer intermediate present, order comes alive again. Measured on the pointer chassis — A-B-C-val 17, A-C-B-val 13, A-val-C-B 16, A-C-val-B 16, A-val-B-C 19 (143 insns), A-B-val-C 19 (143 insns). The winner puts `D_800A36F6 = 0;` BETWEEN the two extractions; `s8 val` read LAST is unchanged from session 3.

- [s4] Reusing the (dead after loop 1) `src` local to carry loop 2's D_801027D8 destination pointer flips cluster 1's register allocation to target's exactly: measured dispositions p=$t3, i=$a3, src=$a2, ptr=$a1, base=$t0. Positions 4, 12, 38, 39, 41, 44, 46 and 47 all went clean in a single step.

- [s4] Session 2's arithmetic closure of the cluster-1 allocation axis is EMPIRICALLY DISPROVEN. On the winning form the probe reports i refs=11 len=37 (ratio 0.297) and src refs=11 len=43 (ratio 0.256), so the `floor_log2(refs)*refs/live_length` model still predicts i first — yet `.greg`'s post-qsort allocno order is `... 75 74 73 ...` (src before i) and src takes $a2. The probe's ratio column is a heuristic, not a decision procedure; do not close an allocation axis on it again.

- [s4] Permuter campaign telemetry, all three campaigns harvested and stopped in-session: ws1 head-chassis-random (base score 1120, first novel find at 15 s, 9742 iters / 300 s, best new 315); ws2 acbv-ptrtemp-chassis (base 175, first novel at 132 s, 9742 iters / 300 s, best new 120); ws3 reuse-src-chassis-floor4 (base 120, 20373 iters / 610 s, NO find below 120). Basins yielded within seconds on the first two chassis and not at all on the third — the fresh-seed discipline's "yields early or not at all" prediction held exactly.

- [s4] A reusable, pipeline-faithful permuter workspace builder now exists: tmp/grind/func_8003504C/s4/mkws.sh + mkbase.sh. It mirrors the real per-file build (cc1 -mel, maspsx --expand-lb twice per EXPAND_LB_FILES, the RODATA_ALIGN2 `.align 3 -> .align 2` sed, multu_pad) and OMITS regfix/asmfix so the permuter scores the honest distance. Two gotchas worth reusing: maspsx emits `.ent`/`.end` at column 0 (so the extraction regex must not require a leading tab), and a FULL preprocessed TU is the correct base.c because decomp-permuter's randomizer only mutates the function named in settings.toml.

- [s4] The entire residual is 7 positions and is a pure ROTATION of the pre-loop block — every instruction present with the correct register. Target: `li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`. Ours: the same seven with the two `li`s moved to the END. The constants are the loop-1 comparison operands (consumed by `beq a0,t2` / `bne a0,t1`), LICM-hoisted into the preheader; sched1 leaves them last because they have no in-block dependents (lowest INSN_PRIORITY) while `move t3,v0 -> move a2,t3` is a 2-deep chain.

- [s4] Session end state: src/code6cac_b.c carries the score-4 form (identical to memory/grind/func_8003504C/candidate.c) and nothing else in the tree was modified outside src/, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s4/. No cheat construct was written at any point: no pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every local in the winning form is assigned and then read.

- [s4] Honest pure-C floor is now 4 (sandbox func_8003504C --disable all), 141/141 instructions, 7 differing normalized positions - down from the 24 / 26 positions that stood unchanged through sessions 1, 2 and 3. src/code6cac_b.c carries the winning form and it is saved verbatim to memory/grind/func_8003504C/candidate.c.

- [s4] The winning form differs from the session-3 form by exactly three edits: (1) the p[8] read is staged through a named pointer intermediate `q = &p[8]`; (2) the post-loop block is ordered D_80102784-store / p[8]-extraction / `D_800A36F6 = 0` / `val = D_80102785`; (3) the dead-after-loop-1 `src` local is reused to carry loop 2's D_801027D8 destination pointer.

- [s4] Cluster 2 (the p[5]/p[8] bitfield-extraction block) is CLOSED - zero differing positions. Target's strictly-$v0-serialized form, with the D_80102785 `lb` hoisted into the `lw 0x14($t3)` load-delay region and the `sh zero` filling the `lw 0x20($t3)` delay region, is reproduced exactly.

- [s4] The intermediate's spelling is load-bearing: `s32 v8 = p[8];` -> 24, `u32 v8 = p[8];` -> 24 (both folded straight back into the shift by combine/CSE), `i = p[8];` -> 20, `q = &p[8];` -> 17. Only the pointer form survives the fold.

- [s4] Cluster 1's register assignment is now target's exactly: measured dispositions p=$t3, i=$a3, src=$a2, ptr=$a1, base=$t0.

- [s4] Session 2's ratio model (floor_log2(refs)*refs/live_length as parsed from the .flow dump) mis-predicts the allocno order on the very form where the flip happened - it still ranks i above src while .greg ranks src above i. The model is a heuristic, not a decision procedure, and no allocation axis should be closed on it again.

- [s4] The entire residual is 7 positions and is a pure ROTATION of one 7-instruction pre-loop block, every instruction present with the correct register. Target: `li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`. Ours: the same seven with the two `li`s moved to the END.

- [s4] Those two constants are loop 1's comparison operands (consumed by `beq a0,t2` and `bne a0,t1` inside the loop), LICM-hoisted into the preheader. sched1 leaves them last because they have no in-block dependents and therefore carry the lowest INSN_PRIORITY in a block where `move t3,v0 -> move a2,t3` is a 2-deep chain.

- [s4] Permuter campaign telemetry, all three campaigns harvested and stopped in-session (no orphans; `permuter_campaign.py status` shows alive=false for all three): ws1 head-chassis-random base 1120, first novel find at 15 s, 9742 iters / 300 s, best new 315; ws2 acbv-ptrtemp-chassis base 175, first novel at 132 s, 9742 iters / 300 s, best new 120; ws3 reuse-src-chassis-floor4 base 120, 20373 iters / 610 s, no find below base. The fresh-seed discipline's "a basin yields early or not at all" prediction held exactly.

- [s4] A reusable, pipeline-faithful permuter workspace builder now exists (tmp/grind/func_8003504C/s4/mkws.sh + mkbase.sh). It mirrors the real per-file build for code6cac_b.c (cc1 -mel, maspsx --expand-lb twice per EXPAND_LB_FILES, the RODATA_ALIGN2 `.align 3 -> .align 2` sed, multu_pad) and OMITS regfix/asmfix so the permuter scores the honest distance. Two gotchas worth reusing: maspsx emits `.ent`/`.end` at column 0 (the extraction regex must not require a leading tab), and a FULL preprocessed TU is the correct base.c because decomp-permuter's randomizer only mutates the function named in settings.toml.

- [s4] No cheat construct was written at any point this session: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits, no touched surface outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s4/. Every local in the winning form is assigned and then read.

## Session 5 (permuter, 2026-08-13) - the pre-loop order MECHANISM is solved

Floor unchanged at 4 (session-4 candidate re-applied and re-confirmed at
`sandbox func_8003504C --disable all` = 4, 141/141 instructions, at session
start and again at session end). This session did not lower the floor. It
(a) KILLED both of session 4's frontier levers with an exhaustive directed
permuter sweep, and (b) found, by structural chassis variation, the FIRST form
in five sessions whose pre-loop block is ordered exactly like target's - which
converts the residual from "an unexplained sched1 decision" into a concrete,
named RTL-emission-order requirement.

### Instruments (tmp/grind/func_8003504C/s5/)
- `inject.py <ws>/base.c [--randomize]` - injects DIRECTED `PERM_*` directives
  into a permuter workspace base.c: `PERM_LINESWAP` over the pre-loop init
  statements (two sites) and a `PERM_GENERAL` over all 24 orders of loop 1's
  four condition disjuncts. Optional `--randomize` wraps loop 1 in
  `PERM_RANDOMIZE` so random mutation runs alongside the directed sites.
- `gen5.py` - generates the walker-PROVENANCE chassis cross-product
  (dwalk = pointer-local vs i-indexed, pwalk = pointer-local vs i-indexed,
  loop-2 dst = reuse-`src` vs fresh local); six variants.
- `sweep5.sh <v>...` - apply + `sandbox --disable all` each variant, print
  score / build_insns, then restore the candidate form.
- `bank.py` - copies disproven forms into rejected/ with a why-header.

### Campaign 1 (ws5) - the directed cross-product, EXHAUSTED, zero finds
`preloop-directed-lineswap-disjuncts` on the floor-4 chassis, base permuter
score 120. The perm space is finite (6 init orders x 2 src/ptr orders x 24
disjunct orders = 288) and the permuter enumerated **all 288 in 14 s with not
one find below base**. Both of session 4's top frontier levers are therefore
dead as stated: neither the pre-loop init statement order nor loop 1's
disjunct order can move the two LICM-hoisted constants.

### Campaign 2 (ws6) - random on the giv chassis, harvested + stopped
`giv-chassis-both-walkers-random`, base 495 (the giv chassis's own base),
10255+ iterations / ~500 s, four novel finds at 495 / 450 / 495 / 465 - i.e.
the basin yields but its best is 450 against a floor-4 chassis base of 120, so
it is not competitive as a closing chassis. Its proposals were constant-holder
reuses of the dead `i`/`tmp` locals (`i = 5;` then `p[i]`), which is a
different lever entirely and was not adopted. Harvested with --stop; no orphans.

### The mechanism, now positively identified
Reading GCC 2.7.2 `sched.c` settles the scheduling half: the priority
computation subtracts one so that "when all instructions have a latency of 1 ...
all instructions will end up with a priority of one, and hence no scheduling
will be done" - every insn in the pre-loop block is a reg-reg move / lui /
addiu of latency 1, so they all tie at priority 1 and `rank_for_schedule` falls
through class to `INSN_LUID`. **The pre-loop block is emitted in pure RTL
order.** There is nothing to steer in the scheduler; the question is entirely
where each insn is EMITTED.

The three emission classes, measured one at a time:
1. **Pre-loop source statements** get the lowest LUIDs and are emitted first.
2. **loop.c movables** (the `li 5` / `li 20` the condition's compares need in
   registers) are emitted by `move_movables` immediately before
   NOTE_INSN_LOOP_BEG, i.e. after ALL pre-loop statements, in discovery order.
3. **loop.c strength-reduction giv initial values** - and, with them, the
   call-result copy `move t3,v0` - are emitted at loop_start AFTER the
   movables.

Target's block is `i=0 / li 5 / li 20 / move t3,v0 / move a2,t3 / lui t0 /
addiu t0 / addiu a1,t0,-9`: exactly ONE class-1 statement (`i = 0`), then the
movables, then the giv inits. **In the original source, loop 1's two walking
pointers were not pre-loop locals at all - they were loop.c givs**, and the
only pre-loop statement was the counter initialization.

Measured, all at 141/141 instructions (`sandbox --disable all`):
  - `v_ptr_src_reuse` (= the session-4 candidate)            score **4**
  - `v_ptr_src_plain` (candidate minus the src-reuse lever)  score 13
  - `v_ptr_idx_plain` (p walker as a giv)                    score 15
  - `v_idx_src_reuse` (D_8010277C walker as a giv)           score 10
  - `v_idx_src_plain`                                        score 19
  - `v_idx_idx_plain` (BOTH walkers as givs)                 score 18
  - `v_idx85_idx_plain` (both givs, walker based on &D_80102785) score 18

`v_ptr_idx_plain` proves class 3: making the p walker a giv moves BOTH
`move t3,v0` and `move <src>,t3` from the front of the block to behind the two
constants. `v_idx_src_reuse` proves class 2 in isolation: making only the
D_8010277C walker a giv lifts the constants ahead of the address setup but not
ahead of the p-copies. `v_idx_idx_plain` combines them and reproduces target's
pre-loop ORDER exactly - the first form in five sessions to do so.

### Why the giv chassis still scores 18 (the two remaining gaps)
1. **The `-9` derivation collapses.** Target's walker giv init is three insns,
   `lui t0,%hi(D_80102785) / addiu t0,t0,%lo(D_80102785) / addiu a1,t0,-9`,
   leaving `t0` live so the in-loop guard reads `lb v0,0(t0)`. Ours is two,
   `lui a1 / addiu a1` - cse folds the walker's base straight into a single
   %lo, and the `D_80102785 == 0` guard then gets its OWN lui inside the loop.
   Writing the walker as `(&D_80102785)[i - 9]` to force the derivation does
   not help: cse folds the -9 into the %lo instead (`addiu a1,a1,-9`), same 2
   insns. Target needs &D_80102785 to be a REGISTER that the walker's init
   subtracts 9 from AND the guard dereferences at offset 0.
2. **Cluster 1's register inversion returns.** With the p walker as a giv there
   is no `src` local left, so session 4's live-range-extension lever (reusing
   the dead `src` for loop 2's D_801027D8 destination) has nothing to attach
   to, and `i`/`src` invert again ($a2/$a3 instead of target's $a3/$a2).

- [s5] Session 4's TOP frontier lever is KILLED exhaustively: a DIRECTED permuter campaign on the floor-4 chassis (PERM_LINESWAP over the pre-loop init statements x PERM_GENERAL over all 24 orders of loop 1's four condition disjuncts) enumerated its ENTIRE 288-combination space in 14 s with not one find below the base score of 120. Neither pre-loop statement order nor loop-condition disjunct order can move the two LICM-hoisted constants.

- [s5] GCC 2.7.2 sched.c's priority computation subtracts one specifically so that "when all instructions have a latency of 1 ... all instructions will end up with a priority of one, and hence no scheduling will be done". Every insn of the pre-loop block is a latency-1 move / lui / addiu, so they all tie at priority 1 and rank_for_schedule falls through the dependence-class test to INSN_LUID. The pre-loop block is emitted in PURE RTL ORDER - there is nothing to steer in the scheduler, and the entire residual is an RTL EMISSION-ORDER question.

- [s5] Three RTL emission classes decide the pre-loop block's order, and each was isolated by measurement: (1) pre-loop source statements get the lowest LUIDs and come first; (2) loop.c movables (the `li 5` / `li 20` the compares need in registers) are emitted by move_movables immediately before NOTE_INSN_LOOP_BEG, i.e. after every pre-loop statement, in discovery order; (3) loop.c strength-reduction giv INITIAL VALUES - and with them the call-result copy `move t3,v0` - are emitted at loop_start AFTER the movables.

- [s5] Target's pre-loop block (`i=0 / li 5 / li 20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`) has exactly ONE class-1 statement followed by the movables followed by the giv inits. That is positive evidence that in the ORIGINAL SOURCE loop 1's two walking pointers were NOT pre-loop pointer locals - they were i-indexed expressions that loop.c strength-reduced into givs - and the only pre-loop statement was the counter initialization.

- [s5] v_ptr_idx_plain (only the p walker made i-indexed, score 15) proves emission class 3 in isolation: both `move t3,v0` and `move <src>,t3` move from the FRONT of the pre-loop block to BEHIND the two hoisted constants. v_idx_src_reuse (only the D_8010277C walker made i-indexed, score 10) proves class 2 in isolation: the constants lift ahead of the address setup but stay behind the p-copies.

- [s5] v_idx_idx_plain (BOTH walkers as givs, score 18, 141/141 insns) reproduces target's pre-loop block ORDER exactly - `i=0 / li / li / move t3,v0 / move t3-copy / lui / addiu` - the first form in five sessions to do so. It is banked at rejected/giv-both-walkers-preloop-order-matches-score18.c.

- [s5] Two gaps keep the giv chassis at 18. (a) Target's walker giv init is THREE insns (`lui t0,%hi(D_80102785) / addiu t0,t0,%lo(D_80102785) / addiu a1,t0,-9`) leaving t0 live so the in-loop guard reads `lb v0,0(t0)`; ours is TWO (`lui a1 / addiu a1`) because cse folds the walker's base into one %lo and the `D_80102785 == 0` guard then gets its own lui inside the loop. Spelling the walker as `(&D_80102785)[i - 9]` does not force the derivation - cse folds the -9 into the %lo instead (`addiu a1,a1,-9`), still 2 insns. (b) With the p walker a giv there is no `src` local left, so session 4's src-reuse allocation lever has nothing to attach to and cluster 1's i/src register inversion returns.

- [s5] Walker-provenance chassis sweep, all at 141/141 instructions: v_ptr_src_reuse (the session-4 candidate) 4, v_ptr_src_plain 13, v_ptr_idx_plain 15, v_idx_src_reuse 10, v_idx_src_plain 19, v_idx_idx_plain 18, v_idx85_idx_plain 18. The src-reuse allocation lever alone is worth 9 points on the pointer chassis (13 -> 4).

- [s5] Permuter campaign telemetry, both campaigns harvested and stopped in-session: ws5 preloop-directed-lineswap-disjuncts, base 120, perm space 288, ENUMERATED 288/288 in 14 s, zero finds; ws6 giv-chassis-both-walkers-random, base 495, 10255+ iterations over ~500 s, four novel finds (495 / 450 / 495 / 465) whose best is far above the floor-4 chassis's base of 120. ws6's proposals were constant-holder reuses of the dead `i` / `tmp` locals (`i = 5;` then `p[i]`), a different lever that was not adopted.

- [s5] Session end state: src/code6cac_b.c carries the session-4 candidate form (re-verified at score 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s5/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every probed variant was a plain re-spelling of live, semantically necessary statements.

- [s5] Floor unchanged at 4: the session-4 candidate was re-applied to src/code6cac_b.c at session start and re-confirmed at score 4 / 141 of 141 instructions, and again at session end.

- [s5] A DIRECTED permuter campaign (PERM_LINESWAP over the pre-loop init statements x PERM_GENERAL over all 24 orders of loop 1's four condition disjuncts) has a finite 288-combination space and the permuter ENUMERATED 288/288 in 14 s with zero finds below the base score of 120. Session 4's two top frontier levers are exhaustively dead.

- [s5] GCC 2.7.2 sched.c subtracts one in its priority computation specifically so that 'when all instructions have a latency of 1 ... all instructions will end up with a priority of one, and hence no scheduling will be done'. Every insn of the pre-loop block is a latency-1 move / lui / addiu, so all priorities tie at 1, the dependence-class test ties, and rank_for_schedule falls through to INSN_LUID. The pre-loop block is emitted in PURE RTL ORDER - the residual is an emission-order problem, not a scheduling problem.

- [s5] Three RTL emission classes decide that block's order and each was isolated by measurement: (1) pre-loop source statements get the lowest LUIDs and come first; (2) loop.c movables (the `li 5` / `li 20` the compares need in registers) are emitted by move_movables immediately before NOTE_INSN_LOOP_BEG, i.e. after every pre-loop statement, in discovery order; (3) loop.c strength-reduction giv INITIAL VALUES - and with them the call-result copy `move t3,v0` - are emitted at loop_start AFTER the movables.

- [s5] Target's pre-loop block (`i=0 / li 5 / li 20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`) contains exactly ONE class-1 statement followed by the movables followed by the giv inits. That is positive evidence that in the ORIGINAL source loop 1's two walking pointers were i-indexed expressions strength-reduced into givs by loop.c, and the only pre-loop statement was the counter initialization.

- [s5] Walker-provenance chassis sweep, all at 141/141 instructions: v_ptr_src_reuse (the session-4 candidate) 4, v_ptr_src_plain 13, v_ptr_idx_plain 15, v_idx_src_reuse 10, v_idx_src_plain 19, v_idx_idx_plain 18, v_idx85_idx_plain 18. Session 4's src-reuse allocation lever alone is worth 9 points on the pointer chassis (13 -> 4).

- [s5] v_ptr_idx_plain (only the p walker made i-indexed) proves emission class 3 in isolation: both `move t3,v0` and `move <src>,t3` move from the FRONT of the pre-loop block to BEHIND the two hoisted constants. v_idx_src_reuse (only the D_8010277C walker made i-indexed) proves class 2 in isolation: the constants lift ahead of the address setup but stay behind the p-copies.

- [s5] v_idx_idx_plain (BOTH walkers as givs, score 18) reproduces target's pre-loop block ORDER exactly - the first form in five sessions to do so - and is banked at memory/grind/func_8003504C/rejected/giv-both-walkers-preloop-order-matches-score18.c.

- [s5] Two gaps keep the giv chassis at 18: (a) target's walker giv init is THREE insns (`lui t0,%hi(D_80102785) / addiu t0,t0,%lo(D_80102785) / addiu a1,t0,-9`) leaving t0 live so the in-loop guard reads `lb v0,0(t0)`, while ours is TWO (`lui a1 / addiu a1`) because cse folds the walker's base into one %lo and the `D_80102785 == 0` guard then gets its own lui inside the loop; spelling the walker as `(&D_80102785)[i - 9]` does not force the derivation because cse folds the -9 into the %lo instead; (b) with the p walker a giv there is no `src` local left, so session 4's src-reuse allocation lever has nothing to attach to and cluster 1's i/src register inversion returns.

- [s5] Permuter campaign telemetry, both campaigns harvested and stopped in-session with no orphans (harvest reported procs_killed 9 for ws6 and pid_alive false for ws5): ws5 preloop-directed-lineswap-disjuncts base 120, 288/288 iterations in 14 s, zero finds; ws6 giv-chassis-both-walkers-random base 495, 10255+ iterations over ~500 s, four novel finds at 495 / 450 / 495 / 465.

- [s5] Session end state: src/code6cac_b.c carries the session-4 candidate form (re-verified at score 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s5/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every probed variant was a plain re-spelling of live, semantically necessary statements.

## Session 6 (forensics, 2026-08-13) - sched1 rediscovered, and a new score-9 chassis

The engine floor is unchanged at 4 (the session-4 pointer chassis was
re-applied and re-confirmed at `sandbox func_8003504C --disable all` = 4,
141/141, at session start and again at session end).  But the function is much
closer than that number suggests: this session built a NEW chassis
(`memory/grind/func_8003504C/chassis_h1_score9.c`, score 9, 141/141) whose
ENTIRE residual is a single register swap - `i` in $a2 and the loop-1 p-walker
in $a3, where target wants the inverse - with every other instruction in the
function, including the whole pre-loop block, matching target byte for byte.

### Instruments (tmp/grind/func_8003504C/s6/)
- `dump.sh <outdir>` - cc1 `-da` dump set for the CURRENT src/code6cac_b.c.
- `solo.py` / `solo.sh <tag>` - build a SOLO preprocessed TU that keeps only
  func_8003504C's body (every other definition is reduced to a prototype by a
  brace-depth parse) and compile it with the INSTRUMENTED cc1.  This fixes what
  session 3 flagged: insn uids restart per function, so BB2_*_DEBUG output from
  a full-TU compile is ambiguous.  VERIFIED: the solo TU emits the identical
  pre-loop block to the full-TU compile.
- `rank.sh <tag>` - re-run the solo TU with `BB2_RANK_DEBUG=1` and print the
  `rank_for_schedule` tie decisions, now unambiguous.
- `gen6.py` / `gen6b.py` - the in-loop-invariant-pointer variant families.

### FORENSIC CORRECTION: sched1 DOES reorder the pre-loop block
Session 5 concluded "the pre-loop block is emitted in PURE RTL ORDER - there is
nothing to steer in the scheduler".  The RTL dumps disprove that.  On the
floor-4 chassis the `.loop` dump has the block as
`9(call) 11(p=v0) 14(i=0) 17(src=p) 20(base=sym) 23(ptr=base-9) 342(li 5)
344(li 20)` and the `.sched` dump has `9 14 11 17 20 23 342 344` - sched1
hoisted insn 14 ABOVE insn 11.  On the giv chassis the `.loop` order is
`9 11 14 347(li 5) 349(li 20) 355 379` and the `.sched` order is
`9 14 347 349 11 355 379` - sched1 SANK the call-result copy below BOTH
constants.  The discriminator is READINESS, not priority and not LUID: insn 11
carries a true dependence on the call (`(insn_list 9 ...)` in its LOG_LINKS)
while `i = 0` and the hoisted constants carry only `REG_DEP_ANTI 9`, so the
ready list issues the independent insns while the call's result is in flight.
Session 5's reading of sched.c is right as far as it goes (all these insns are
latency-1, so `rank_for_schedule` falls through to INSN_LUID) - what it missed
is that the ready SET differs by cycle.  `BB2_RANK_DEBUG` on the solo TU
confirms every comparison in this block is `cls=3 x cls2=3 val=0`, i.e. decided
purely by LUID among the insns that are ready.

The practical consequence is unchanged and session 5's frontier item 3 still
stands: the movables are appended after every pre-loop source statement, so no
floor-4-chassis spelling can lift `li 5` / `li 20` ahead of the base address
setup.  The route to target's block order is the giv chassis, and only it.

### The two giv-chassis gaps, both CLOSED this session (18 -> 13 -> 9)
1. **The &D_80102785 base register (18 -> 13).**  Assigning the guard pointer
   INSIDE loop 1's body - `s8 *b = &D_80102785;` as the body's first statement
   - makes it an in-loop loop INVARIANT, so loop.c hoists it as a MOVABLE into
   the preheader instead of it being a pre-loop source statement.  Target's
   `lui t0 / addiu t0` then lands in the right place, $t0 stays live across the
   loop, and the guard reads `lb v0,0(t0)` exactly like target.  Measured
   placements: b first in the body 13, b after the `lv` load 13, b inside the
   guarded arm 14 (142 insns), b as a pre-loop local 13.  Session 5's
   `(&D_80102785)[i - 9]` attempt failed because there was no pointer pseudo at
   all for cse to keep - the fix is a source-level pointer, not an array base.
2. **The `-9` derivation (13 -> 9).**  With `b` in a register, a second in-loop
   invariant `u8 *w = (u8 *)b - 9;` walked as `w[i]` gives the D_8010277C
   walker giv the initial value `(plus (reg b) (const_int -9))`, so loop.c's
   `emit_iv_add_mult` emits target's third preheader insn `addiu a1,t0,-9` and
   every in-loop access is at offset 0.  Without it the giv init is
   `move a1,t0` and the -9 rides in the memory offsets (`sb v1,-9(a1)`,
   `lbu v0,-9(a1)`).

### What is left on the h1 chassis: one allocation flip
All 9 residual positions are `i` <-> p-walker.  `.greg` on h1 gives the allocno
order `79 92 162 161 160 159 180 73 156 176 124 158 157 72 139 77 99 94`: the
SOURCE pseudo 73 (`i`) is ranked ahead of every loop.c-created giv pseudo, so
it takes $a2 as the first free register in REG_ALLOC_ORDER and the p-walker giv
inherits $a3.  Dispositions: 72 (p) = $t3, 73 (i) = $a2, 77 (b) = $t0.

Three live-range extensions of `i` were measured on this chassis and ALL move
the wrong way: reuse as loop 2's outer counter 33 (136 insns), as the val==5
table index 26 (141 insns), as loop 2's inner counter 58 (122 insns).  So the
lever is not lowering `i` - it is raising a giv, or removing `i` as a source
pseudo.  Also killed: hoisting `b` / `w` to function scope so one can carry
loop 2's D_801027D8 destination (session 4's src-reuse lever) COLLAPSES the
function - 121 / 118 / 116 instructions against target's 141 - because a
function-scope pointer assigned inside loop 1 stops being a loop-invariant
movable and loop 2's code folds.

- [s6] FORENSIC CORRECTION to session 5: sched1 DOES reorder the pre-loop block. On the floor-4 chassis the .loop order is `11(p=v0) 14(i=0) 17 20 23 342(li 5) 344(li 20)` and the .sched order is `14 11 17 20 23 342 344`; on the giv chassis the .loop order is `11 14 347 349 355 379` and the .sched order is `14 347 349 11 355 379`. The discriminator is READINESS: insn 11 carries a true dependence on the call (insn_list 9 in LOG_LINKS) while `i = 0` and the hoisted constants carry only REG_DEP_ANTI 9, so the ready list issues the independent insns while the call result is in flight. BB2_RANK_DEBUG on a solo TU confirms every comparison in this block is cls=3 vs cls2=3 val=0, i.e. decided purely by INSN_LUID among the insns that are READY that cycle.

- [s6] A SOLO-TU instrument now exists (tmp/grind/func_8003504C/s6/solo.py + solo.sh + rank.sh): it reduces every other function in the preprocessed code6cac_b.i to a prototype by a brace-depth parse, so the instrumented cc1's BB2_*_DEBUG output is unambiguous (insn uids restart per function, which is what made session 3 distrust rank.sh). The solo TU was verified to emit the identical pre-loop block to the full-TU compile.

- [s6] NEW CHASSIS at score 9 / 141 of 141 instructions, banked at memory/grind/func_8003504C/chassis_h1_score9.c: the session-5 both-walkers-as-givs chassis plus two in-loop loop-invariant pointers. ALL NINE differing positions are one register swap (i in $a2 / p-walker in $a3, target wants the inverse); every other instruction in the function, including the entire pre-loop block and both bitfield-extraction clusters, matches target byte for byte and register for register.

- [s6] Giv-chassis gap (a) is CLOSED: assigning the guard pointer INSIDE loop 1's body (`s8 *b = &D_80102785;`) makes it an in-loop loop-invariant, so loop.c hoists it as a MOVABLE into the preheader rather than it being a pre-loop source statement. Target's `lui t0 / addiu t0` lands correctly, $t0 stays live across the loop, and the guard reads `lb v0,0(t0)` exactly like target. Score 18 -> 13. Measured placements: b first in the body 13, b after the lv load 13, b inside the guarded arm 14 (142 insns), b as a pre-loop local 13.

- [s6] Giv-chassis gap (b) is CLOSED: a second in-loop invariant `u8 *w = (u8 *)b - 9;` walked as `w[i]` gives the D_8010277C walker giv the initial value `(plus (reg b) (const_int -9))`, so loop.c's emit_iv_add_mult emits target's third preheader insn `addiu a1,t0,-9` and every in-loop access is at offset 0. Score 13 -> 9. Without it the giv init is `move a1,t0` and the -9 rides in the memory offsets (`sb v1,-9(a1)`, `lbu v0,-9(a1)`).

- [s6] On the h1 chassis .greg's allocno order is `79 92 162 161 160 159 180 73 156 176 124 158 157 72 139 77 99 94` - the source pseudo 73 (`i`) outranks EVERY loop.c-created giv pseudo, so `i` takes $a2 as the first free register in REG_ALLOC_ORDER and the p-walker giv inherits $a3. Dispositions: 72 (p) = $t3, 73 (i) = $a2, 77 (b) = $t0.

- [s6] Three live-range extensions of `i` measured on the h1 chassis all move the WRONG way: reuse i as loop 2's outer counter 33 (136 insns), reuse i as the val==5 table index 26 (141 insns), reuse i as loop 2's inner counter 58 (122 insns). Lowering `i` is not the lever on this chassis; raising a giv (or removing `i` as a source pseudo) is.

- [s6] Session 4's src-reuse allocation lever CANNOT be transplanted onto the giv chassis by hoisting `b` / `w` to function scope: measured 35 / 50 / 51 at 121 / 118 / 116 instructions against target's 141, because a function-scope pointer assigned inside loop 1 stops being a loop-invariant movable and the loop-2 code folds away.

- [s6] Session end state: src/code6cac_b.c carries the session-4 candidate form (re-verified at score 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s6/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every local in every probed form is assigned and then read.

- [s6] [s6] Engine floor unchanged at 4: the session-4 candidate was re-applied to src/code6cac_b.c and re-confirmed at sandbox func_8003504C --disable all = 4, 141/141 instructions, at session start and again at session end.

- [s6] [s6] NEW CHASSIS at score 9 / 141 of 141 instructions, banked at memory/grind/func_8003504C/chassis_h1_score9.c: the session-5 both-walkers-as-givs chassis plus two in-loop loop-invariant pointers. ALL NINE differing positions are one register swap (i in $a2 / p-walker in $a3; target wants the inverse). Every other instruction in the function - the entire pre-loop block and both bitfield-extraction clusters included - matches target byte for byte and register for register.

- [s6] [s6] FORENSIC CORRECTION to session 5: sched1 DOES reorder the pre-loop block. Floor-4 chassis .loop order 11(p=v0) 14(i=0) 17 20 23 342(li 5) 344(li 20) becomes .sched order 14 11 17 20 23 342 344; giv chassis .loop order 11 14 347 349 355 379 becomes .sched order 14 347 349 11 355 379. The discriminator is READINESS: insn 11 carries a true dependence on the call (insn_list 9 in LOG_LINKS) while i=0 and the hoisted constants carry only REG_DEP_ANTI 9, so independent insns issue while the call result is in flight. BB2_RANK_DEBUG confirms every comparison in this block is cls=3 vs cls2=3 val=0 - decided purely by INSN_LUID among the READY insns.

- [s6] [s6] Giv-chassis gap (a) CLOSED: assigning the guard pointer INSIDE loop 1's body (s8 *b = &D_80102785;) makes it an in-loop loop-invariant, so loop.c hoists it as a MOVABLE into the preheader instead of it being a pre-loop source statement. Target's lui t0 / addiu t0 lands correctly, $t0 stays live across the loop, and the guard reads lb v0,0(t0). Score 18 -> 13. Placements measured: b first in the body 13, b after the lv load 13, b inside the guarded arm 14 (142 insns), b as a pre-loop local 13.

- [s6] [s6] Giv-chassis gap (b) CLOSED: a second in-loop invariant u8 *w = (u8 *)b - 9; walked as w[i] gives the D_8010277C walker giv the initial value (plus (reg b) (const_int -9)), so loop.c's emit_iv_add_mult emits target's third preheader insn addiu a1,t0,-9 and every in-loop access is at offset 0. Score 13 -> 9. Without it the giv init is move a1,t0 and the -9 rides in the memory offsets (sb v1,-9(a1), lbu v0,-9(a1)).

- [s6] [s6] On the h1 chassis .greg's allocno order is 79 92 162 161 160 159 180 73 156 176 124 158 157 72 139 77 99 94 - the SOURCE pseudo 73 (i) outranks EVERY loop.c-created giv pseudo, so i takes $a2 as the first free register in REG_ALLOC_ORDER and the p-walker giv inherits $a3. Dispositions: 72 (p) = $t3, 73 (i) = $a2, 77 (b) = $t0.

- [s6] [s6] Three live-range extensions of i on the h1 chassis all move the WRONG way: loop 2 outer counter 33 (136 insns), val==5 table index 26 (141 insns), loop 2 inner counter 58 (122 insns).

- [s6] [s6] Session 4's src-reuse allocation lever cannot be transplanted onto the giv chassis by hoisting b / w to function scope: 35 / 50 / 51 at 121 / 118 / 116 instructions against target's 141, because the pointer stops being a loop-invariant movable and loop 2's code folds away.

- [s6] [s6] New reusable instrument: tmp/grind/func_8003504C/s6/solo.py + solo.sh + rank.sh build a SOLO preprocessed TU containing only func_8003504C's body (every other definition reduced to a prototype by a brace-depth parse) and compile it with the INSTRUMENTED cc1, so BB2_*_DEBUG output is unambiguous - insn uids restart per function, which is what made session 3 distrust rank.sh. The solo TU was verified to emit the identical pre-loop block to the full-TU compile.

- [s6] [s6] Session end state: src/code6cac_b.c carries the session-4 candidate form (re-verified at score 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s6/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every local in every probed form is assigned and then read.

## Session 7 (forensics, 2026-08-13) - the allocation question is now ARITHMETIC

Engine floor unchanged at 4, but `memory/grind/func_8003504C/candidate.c` was
REPLACED: the new candidate scores the same 4 with only **FOUR** differing
normalized positions instead of seven, and its entire residual is the pre-loop
rotation `li t2,5 / li t1,20` vs `move t3,v0 / move a2,t3`.  Everything else in
the function - loop-1's register assignment included - matches target.

### The instrument this session was built on
GCC 2.7.2's `global.c` in this tree already carries an env-gated hook
(`BB2_ALLOC_DEBUG=1`) that prints, for every allocno in post-qsort order,
`ord / pseudo / hardreg / nrefs / livelen / pri`, where `pri` is exactly
`allocno_compare`'s `floor_log2(nrefs)*nrefs/livelen*10000`.  That retires the
`.flow`-parsed ratio column session 2 built and session 4 disproved: these are
cc1's OWN numbers, taken from the same array `qsort` sorts.  Driver:
`tmp/grind/func_8003504C/s7/alloc.sh [tag]` (full TU, dumps + the table for
func_8003504C only) and `sweep7.sh <variant.c>...` (apply + sandbox + table).
Verified: a solo TU (s6/solo.py) reproduces the table pseudo-for-pseudo.

### Session 4's flip lever, finally quantified
On the session-4 chassis the table reads `75 (ptr) 11/29 pri 11379 -> $a1`,
`74 (src) 11/31 pri 10645 -> $a2`, `73 (i) 11/33 pri 10000 -> $a3`.  The
src-reuse lever works because reusing the dead loop-1 walker for loop 2's
destination adds TWO depth-1 references (the `src = &D_801027D8;` store and the
`dst_d = src;` read) for only TWO insns of extra live range: 9/29 (pri 9310)
becomes 11/31 (pri 10645), which crosses the counter's 10000.  Without the
reuse the walker is 9 refs and always loses.

### The new candidate: h1's in-loop invariants + a SOURCE-pointer walker
Crossing session 6's h1 chassis (b/w assigned inside loop 1, so loop.c hoists
them as movables and emits `lui t0 / addiu t0 / addiu a1,t0,-9`) with a
source-pointer p-walker carrying session 4's reuse gives score 4 at 141/141
with four differing positions.  Measured siblings: the same chassis WITHOUT the
reuse (v_s1) scores 12 with the walker at 9/31 pri 8709.

### The closure: a giv walker can NEVER outrank the counter here
On the h1 chassis the table is `180 (D_8010277C walker giv) 11/27 pri 12222 ->
$a1`, `73 (i) 11/33 pri 10000 -> $a2`, `176 (p-walker giv) 9/29 pri 9310 ->
$a3`.  Live lengths are measured on the SCHEDULED order and every one of them
is `B + n`, where B = 27 is loop 1's body length and n is the number of
preheader insns that follow that pseudo's initialization.  Target's preheader
is seven RTL insns (the `lui/addiu` pair for `&D_80102785` is ONE RTL insn),
and the counter's init is first, so n_i <= 6 and n_walker >= 0.  The flip needs
`27/(B+n_w) > 33/(B+n_i)`, i.e. `27*n_i - 33*n_w > 6B = 162`; with n_i <= 6 the
left side is at most 162.  Equality is the best case, and `allocno_compare`
breaks an exact priority tie by ALLOCNO NUMBER (`return *v1 - *v2`), which
always favours the low-numbered source pseudo `i` over a loop.c-created giv.
So on any chassis where the p-walker is a 9-ref giv the counter takes $a2 -
which is why h1 sits at 9 and why the route to zero must give the walker more
references, i.e. keep it a SOURCE pseudo.

### And that is exactly what the rotation costs
`move_movables` inserts loop.c's movables immediately before
NOTE_INSN_LOOP_BEG - after EVERY pre-loop source statement - while giv initial
values are emitted at loop_start, i.e. after the movables.  A source-pointer
walker's init is a pre-loop statement, so `move t3,v0 / move a2,t3` always
carry lower LUIDs than `li 5 / li 20` and sched1 (all latency-1, so
rank_for_schedule falls through to INSN_LUID among ready insns) keeps them
ahead.  The two requirements - walker with post-loop refs, and walker init
after the movables - are in direct tension, and that tension IS the remaining
4 positions.

- [s7] NEW INSTRUMENT: GCC 2.7.2's global.c in this tree has an env-gated hook (BB2_ALLOC_DEBUG=1) that prints every allocno in post-qsort order as `ord / pseudo / hardreg / nrefs / livelen / pri`, with pri computed by allocno_compare's own formula. tmp/grind/func_8003504C/s7/alloc.sh runs it over the current src and filters to func_8003504C; sweep7.sh does apply + sandbox + table per variant. This supersedes the .flow-parsed ratio column from session 2 that session 4 disproved - these are cc1's own numbers.

- [s7] Session 4's src-reuse lever is now quantified with cc1's numbers: on the session-4 chassis the table is 75 (ptr) 11 refs/29 len pri 11379 -> $a1, 74 (src) 11/31 pri 10645 -> $a2, 73 (i) 11/33 pri 10000 -> $a3. The reuse adds TWO depth-1 references for only TWO insns of live range, taking the walker from 9/29 (pri 9310) to 11/31 (pri 10645) and across the counter's 10000.

- [s7] NEW CANDIDATE at score 4 with only FOUR differing positions (the session-4 candidate scored the same 4 with seven): session 6's h1 in-loop invariants (b/w hoisted as movables, giving target's lui t0 / addiu t0 / addiu a1,t0,-9) crossed with a SOURCE-pointer p-walker carrying session 4's post-loop reuse. The whole residual is the pre-loop rotation li 5 / li 20 versus move t3,v0 / move a2,t3.

- [s7] The same chassis WITHOUT the reuse (v_s1) scores 12 and the walker measures 9 refs/31 len pri 8709 against the counter's 10000 - the isolation measurement for the lever.

- [s7] CLOSURE, with cc1's numbers plus arithmetic: on any chassis where loop 1's p-walker is a loop.c giv it has 9 weighted refs, and live lengths are B + n where B = 27 is the body length and n counts the preheader insns after that init. Target's preheader is 7 RTL insns and the counter's init is first, so n_i <= 6; the flip needs 27*n_i - 33*n_w > 6B = 162, whose maximum is exactly 162. Equality is the best reachable case and allocno_compare breaks priority ties by ALLOCNO NUMBER (`return *v1 - *v2`), which always favours the low-numbered source pseudo i over a giv. The h1 chassis therefore cannot reach target's assignment by any spelling that leaves the walker a 9-ref giv.

- [s7] Three attempts to give a giv walker post-loop references all fail: a pre-loop pointer indexed s[i*10] and reused post-loop (v_b1) and an in-loop `s = (u8 *)p + i*10` at function scope reused post-loop (v_p1) both score 9 with an allocno table identical to plain h1, because the store to the source pointer is dead (the giv reads p directly) and is eliminated. A loop.c-created giv cannot inherit source-level references.

- [s7] The counter cannot be demoted on the h1 chassis either, and the target is now numeric: pri(i) must be < 9310. Reuses whose def and use are adjacent (i as the D_800A38E1 value, i as idx) are copy-propagated away entirely - score stays 9 and the table still reads 11 refs/33 len/10000. The `sel` reuse (two defs joining, so it survives) reaches 14 refs/43 len/9767 - still above 9310 - and costs 3 points. The requirement table is (refs 12, len >= 39), (13, >= 42), (14, >= 46), (15, >= 49).

- [s7] Moving `i = 0;` ahead of the call does NOT lengthen the counter's live range (still 33): live lengths are measured on the SCHEDULED order and sched1 puts the counter init back immediately after the call.

- [s7] The rotation's mechanism is confirmed by the RTL dumps of the new candidate: loop.c's movables (345 `b = &D_80102785`, 347 `b - 9`, 349 `li 5`, 351 `li 20`) are inserted immediately before NOTE_INSN_LOOP_BEG - after every pre-loop source statement - while the p-copies 11 (`p = v0`) and 17 (`s = p`) are pre-loop statements with lower LUIDs. sched1 emits 14, 11, 17, 349, 351, 345, 357. Giv initial values are the only class emitted AFTER the movables, so a walker whose init sits after the constants must be a giv - the exact class the allocation arithmetic forbids.

- [s7] Holding loop 1's two compare constants in pre-loop locals (so their `li`s would carry low LUIDs) is measured DEAD in both placements - constants initialised after the call and before it, score 9 in both, allocation wrecked - independently of it being a constant-holder spelling.

- [s7] Session end state: src/code6cac_b.c carries the new candidate (re-verified at sandbox func_8003504C --disable all = 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s7/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits. Every local in the candidate is assigned and then read.

- [s7] GCC 2.7.2's global.c in this tree carries an env-gated hook (BB2_ALLOC_DEBUG=1) printing every allocno in post-qsort order as ord / pseudo / hardreg / nrefs / livelen / pri, with pri computed by allocno_compare's own formula - cc1's own numbers, superseding the .flow-parsed ratio column session 2 built and session 4 disproved. Drivers: tmp/grind/func_8003504C/s7/alloc.sh and sweep7.sh.

- [s7] Session 4's src-reuse lever quantified on its own chassis: ptr 11 refs/29 len pri 11379 -> $a1, src 11/31 pri 10645 -> $a2, i 11/33 pri 10000 -> $a3. The reuse adds TWO depth-1 references for only TWO insns of live range, taking the walker from 9/29 (pri 9310) across the counter's 10000.

- [s7] NEW CANDIDATE at score 4 / 141 of 141 with only FOUR differing normalized positions (the session-4 candidate scored the same 4 with seven): h1's in-loop invariants b/w crossed with a SOURCE-pointer p-walker carrying session 4's post-loop reuse. The entire residual is the pre-loop rotation `li t2,5 / li t1,20` versus `move t3,v0 / move a2,t3`; every other instruction, loop-1's registers included, matches target.

- [s7] CLOSURE: on any chassis where loop 1's p-walker is a loop.c giv it has 9 weighted refs, live lengths are B + n (B = 27 body insns, n = preheader insns after the init, measured on the scheduled order), target's preheader is 7 RTL insns and the counter's init is first, so the flip needs 27*n_i - 33*n_w > 6B = 162 with n_i <= 6 - maximum exactly 162, a tie at best - and allocno_compare breaks ties by allocno NUMBER, always favouring the low-numbered source pseudo i. The h1 chassis cannot reach target's assignment by any spelling that leaves the walker a giv.

- [s7] A loop.c-created giv cannot inherit source-level references: a pre-loop pointer indexed s[i*10] and reused post-loop (v_b1) and an in-loop `s = (u8 *)p + i*10` at function scope reused post-loop (v_p1) both score 9 with an allocno table identical to plain h1, because the store to the source pointer is dead and is eliminated.

- [s7] Counter demotion is calibrated: pri(i) must fall below 9310. Adjacent def/use reuses are copy-propagated away entirely (v_c1, v_c2: score 9, table unchanged at 11/33/10000); only a reuse with two joining definitions survives (v_a1 sel: 14 refs / 43 len / pri 9767, score 12). Requirement table: (12, >= 39), (13, >= 42), (14, >= 46), (15, >= 49).

- [s7] Live lengths are measured on the SCHEDULED order: moving `i = 0;` ahead of the call does not lengthen the counter's range (still 33) because sched1 puts the init back immediately after the call.

- [s7] RTL dumps of the new candidate confirm the rotation's mechanism: loop.c's movables (345 b = &D_80102785, 347 b - 9, 349 li 5, 351 li 20) are inserted immediately before NOTE_INSN_LOOP_BEG - after every pre-loop source statement - while the p-copies 11 (p = v0) and 17 (s = p) are pre-loop statements with lower LUIDs; sched1 emits 14, 11, 17, 349, 351, 345, 357. Giv initial values are the ONLY class emitted after the movables, which is exactly the class the allocation arithmetic forbids.

- [s7] The solo-TU instrument from session 6 was re-verified: a solo TU reproduces the full-TU allocno table pseudo-for-pseudo, so BB2_* debug output on it is trustworthy.

- [s7] Session end state: src/code6cac_b.c carries the new candidate (re-verified at sandbox --disable all = 4, 141/141) and nothing outside src/code6cac_b.c, memory/grind/func_8003504C/ and tmp/grind/func_8003504C/s7/ was modified. No cheat construct was written at any point: no register pins, no inline asm, no dead stores, no volatile coercion, no unused locals, no regfix/asmfix edits.

## Session 8 (rederive, 2026-08-13) — the sched1 model that explains the rotation, and the arithmetic closure of the giv chassis

Floor unchanged at **4** (the session-7 candidate re-applied to src and
re-measured: score 4, 141/141). This session did not lower the floor. It
produced the first EXACT model of how GCC 2.7.2's sched1 orders the loop-1
preheader — a model that fits both chassis instruction-for-instruction — and
used it to convert the remaining 4-point residual from "an RTL emission-order
mystery" into a single, precisely stated, falsifiable requirement.

### The fresh re-derivation (modality deliverable)
`tools/m2c/m2c.py --target mipsel-gcc-c -f func_8003504C` (run this session)
reconstructs the function as: `temp_v0 = func_80077D00(); var_a3 = 0; var_a2 =
temp_v0; var_a1 = &D_80102785 - 9; do { ... } while (var_a3 < 2);` — i.e. the
p-walker as a **source pointer copied from the call result**, the D_8010277C
walker as a pre-loop `&D_80102785 - 9`, and the guard as a single `&&`
condition rather than nested ifs. That is structurally the session-7 candidate,
so m2c contributes no new chassis. The sibling `func_80035280` (the next
function in the file, same `p = func_80077D00()` idiom) is itself unmatched and
carries `volatile`/`new_var` debt, so it is not a transplant source.

### The sched1 model (NEW — supersedes the partial models of s5 and s6)
GCC 2.7.2's `sched.c` schedules each basic block **backwards**: it repeatedly
takes, from the set of insns whose in-block successors have all been placed,
the one with the greatest (INSN_PRIORITY, INSN_LUID), and places it at the
current LAST free position. Practical consequences for this preheader:
  * an insn with a dependent still unplaced is NOT a candidate;
  * among candidates, the one whose dependence chain to the block end is
    longest wins; ties go to the **higher LUID** (later in RTL order).

Read off the s8 dumps (`tmp/grind/func_8003504C/s8/solo_cand.i.{loop,sched}`),
the candidate chassis's preheader RTL is
`9 call / 11 p=v0 / 14 i=0 / 17 s=p / 345 b=&D_80102785 / 347 w=b-9 /
349 li 5 / 351 li 20 / 357 giv-init(D_8010277C walker) / 19 LOOP_BEG`
(345+347 later fold so 357 becomes `(plus b -9)`), and sched1 emits
`14, 11, 17, 349, 351, 345, 357`. The model reproduces that exactly:
357 (highest LUID, no successor) is placed last, 345 becomes a candidate with
priority 2 and is placed 6th, then the LUID ties 351, 349, 17, then 11
(priority 2 via 17), then 14. Applying the same model to the h1/giv chassis
(where `s = p` is not a source statement but the p-walker giv's initial value,
emitted AFTER the movables) yields `i=0, li 5, li 20, p=v0, giv-init, b,
w-giv-init` = **target's preheader exactly**, which is what h1 measures. Two
independent chassis, zero free parameters.

### What that makes the residual
The 4-point residual is exactly: `s = p` must be placed 5th, but at that step
it is tied at priority 1 with `li 5` / `li 20` and has the LOWEST LUID.
Because the preheader's insn set and its dependence graph are byte-forced
(i=0, li 5, li 20, p=v0, s=p, b=&D_80102785, w=b-9; the only possible
successors of `s=p` are b and w, whose values are unrelated addresses), the
priority of `s = p` can never exceed 1. Therefore:

> **On any chassis where the p-walker is a source pseudo, target's preheader
> order requires INSN_LUID(`s = p`) > INSN_LUID(`li 20`) — i.e. the walker's
> initializing insn must be emitted by loop.c AFTER `move_movables` has
> appended the two hoisted constants.**

`move_movables` inserts every movable immediately before `NOTE_INSN_LOOP_BEG`,
after every pre-loop source statement, so no source statement can qualify and
no permutation of the pre-loop statements or of the movable discovery order can
help (both re-verified against the model, and the 288-combination enumeration
of session 5 is the empirical counterpart). The classes loop.c emits at
`loop_start` are enumerated in `tools/gcc-2.7.2/loop.c`: the movables
themselves (lines 1652-1854), the giv initial values from `emit_iv_add_mult`
(5561, 6098), and `check_dbra_loop`'s reversed-counter initialisation (5868 —
unreachable here, the counter is used for addressing so the loop cannot be
reversed). Only the giv class can carry a pointer.

### The giv chassis is now arithmetically CLOSED (strengthens s7)
On the giv chassis the walker's initial value IS emitted after the movables
(target's order), but the pseudo is loop.c-created and its weighted
`reg_n_refs` is byte-forced at 9: init 1 (depth 1) + `s[0]` and `s[1]` (2 refs
x depth 2) + the `addiu a2,a2,0xA` increment (2 refs x depth 2). A third
in-loop reference would need a third occurrence of the walker in the body, and
target's body dereferences it exactly twice; a post-loop reference is
impossible because the walker's final value is `(u8 *)p + 20 = &p[5]` and
target reads that memory as `lw $v0,0x14($t3)` (base `p`, offset 0x14), not
through the walker — any post-loop use of the walker changes the emitted base
register. Its live length is bounded below by loop 1's 27-insn body (it is
loop-carried), measured 29. So pri(walker) = floor_log2(9)*9/29 = 0.931 at
best 27/27 = 1.000.

The counter's numbers are equally byte-forced: 11 weighted refs (init 1;
`(&D_80102780)[i]` and `(&D_8010277E)[i]` 2x2 — target keeps both as
`lui $at / addu $at,$at,$a3`, so neither is strength-reduced and neither can
be re-based on the D_8010277C walker without deleting four instructions;
`addiu $a3,$a3,1` 2x2; `slti $v0,$a3,2` 1x2) and live length exactly 33
(def is the first preheader insn — it cannot move above the call, the .sched
dump shows `insn_list:REG_DEP_ANTI 9` on it because $a3 is call-clobbered —
plus 6 preheader insns and 27 body insns), giving pri(i) = 3*11/33 = 1.000.
Since `allocno_compare` breaks priority ties by allocno number and the counter
is a low-numbered source pseudo, **a giv walker can never outrank the counter
without demoting the counter**. That is the same conclusion session 7 reached
from a different direction, now with both sides of the inequality pinned to
target's bytes rather than to one measurement.

### Measurements taken this session (all at 141 target insns)
  * candidate re-measured: **4**.
  * `p / s / i` init order (s before i): **12**, 141 insns.
  * `i = 0` before the call: **20**, **142** insns (the extra insn is real).
  * m2c's single `&&` guard instead of the nested ifs: **4**, byte-identical.
  * loop 1 spelled `for (; i < 2; i++)` instead of `do/while`: **4**,
    byte-identical (GCC folds the entry test, the loop notes are the same).
  * `s` reused as the `&p[8]` pointer intermediate (dropping the `q` local):
    **4**, byte-identical — so `q` is a free variable, not load-bearing.
The last three are recorded as codegen-NEUTRAL spellings: they are available
to a future session as free structural variation that costs nothing.

### Instruments added (tmp/grind/func_8003504C/s8/)
  * `gen8.py` — variant generator over the session-7 candidate.
  * `sweep8.sh` — apply + `sandbox --disable all`, score line only.
  * `solo8.sh <tag> <variant.c>` — full `-da` dump set into `s8/d_<tag>`, a
    SOLO one-function TU, the instrumented-cc1 compile, and the pre-loop RTL
    region of the `.loop` (pre-sched1) and `.sched` (post-sched1) dumps side by
    side. This is the instrument that produced the scheduling model; it is the
    fastest way to check any future chassis's preheader emission order.

- [s8] Floor re-measured at 4 with the session-7 candidate applied to src/code6cac_b.c: score 4, target_insns 141, build_insns 141. src is left holding that form.

- [s8] sched1 model (NEW, exact, fits both chassis with zero free parameters): GCC 2.7.2 schedules each block BACKWARDS, repeatedly taking the insn with the greatest (INSN_PRIORITY, INSN_LUID) among those whose in-block successors are all already placed, and putting it in the last free slot. Candidate preheader RTL `11 p=v0 / 14 i=0 / 17 s=p / 345 b=&D_80102785 / 347 w=b-9 / 349 li 5 / 351 li 20 / 357 giv-init` -> emitted `14, 11, 17, 349, 351, 345, 357`, predicted exactly, including `b` being demoted to position 6 because it carries priority 2 through the giv init.

- [s8] The candidate's whole 4-point residual is that `s = p` is emitted 4th instead of 5th (and the two `li`s 5th/6th instead of 2nd/3rd). Target's preheader is `move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`; ours swaps the copies with the constants. Everything else in the function - both bitfield clusters, loop 2, the whole loop-1 body and its register assignment - is byte-identical.

- [s8] EXACT REQUIREMENT for the source-pointer chassis: INSN_LUID(`s = p`) > INSN_LUID(`li 20`). Its priority cannot be raised (its only conceivable in-block successors, `b` and `w`, hold unrelated symbol addresses), so the LUID is the only lever, and move_movables inserts every movable immediately before NOTE_INSN_LOOP_BEG - after every pre-loop source statement. No source statement can therefore qualify.

- [s8] loop.c's insertion points at loop_start, enumerated (tools/gcc-2.7.2/loop.c): move_movables lines 1652 / 1708 / 1795-1854; emit_iv_add_mult giv initial values lines 5561 and 6098; check_dbra_loop's reversed-counter initialisation line 5868. The dbra class is unreachable here (the counter is used for addressing, so the loop cannot be reversed). Only the giv class can carry a pointer, which is why the giv chassis gets target's preheader.

- [s8] The giv chassis's allocno inequality is now pinned to target's bytes on BOTH sides: p-walker giv = 9 weighted refs (init 1 + two dereferences at depth 2 + the increment's 2 refs at depth 2), live length >= 27 (loop-carried across a 27-insn body), measured 29 -> pri 0.931; counter i = 11 weighted refs (init 1 + two i-indexed stores at depth 2, both of which target keeps as `lui $at / addu $at,$at,$a3` + the increment + the `slti` compare) and live length exactly 33 -> pri 1.000. allocno_compare breaks the tie by allocno number, favouring the low-numbered source pseudo.

- [s8] A post-loop reference cannot be attached to the walker on the giv chassis: the walker's final value is (u8 *)p + 20 = &p[5], and target reads that memory as `lw $v0,0x14($t3)` (base p, offset 0x14), so any post-loop use of the walker changes the emitted base register.

- [s8] `i = 0` cannot be scheduled above the call: $a3 is call-clobbered and the .sched dump carries `insn_list:REG_DEP_ANTI 9` on that insn, so the counter's live length is capped at 33 (6 preheader insns + 27 body insns) on every chassis.

- [s8] Three codegen-NEUTRAL spellings measured on the current chassis (all 4 at 141/141, byte-identical diffs): m2c's single `&&` guard instead of the nested ifs; loop 1 written `for (; i < 2; i++)` instead of do/while; and `s` reused to carry the `&p[8]` pointer intermediate, which drops the `q` local entirely.

- [s8] m2c's fresh reconstruction of the target reproduces the current chassis (source-pointer p-walker copied from the call result, `&D_80102785 - 9` for the D_8010277C walker), so the chassis is corroborated by the decompiler and is not an artefact of five sessions of incremental edits.

- [s8] The immediate sibling func_80035280 (same `p = func_80077D00()` idiom) is itself an unmatched queue item carrying volatile / `new_var` debt - not usable as a transplant source.

## Session 9 (rederive) - the three necessary conditions are jointly contradictory

Session 9 measured the last untested half of the preheader problem and closed the
giv family. Combining today's measurements with sessions 7 and 8, target's
preheader plus target's loop-1 register assignment impose THREE conditions that
cannot all be met by any chassis built so far:

 (1) The p-walker's allocno must outrank the counter's.  Measured, on cc1's own
     allocno table: a SOURCE-pseudo walker does this only by carrying the
     post-loop D_801027D8 pointer (9 refs -> 11 refs, pri 8709 -> 10645 against
     the counter's 10000).  A loop.c GIV walker can never do it: it is pinned at
     9 weighted refs, both allocnos are in floor_log2 bucket 3, so the flip needs
     L_i > 1.222 * L_giv while the giv's init is always emitted LATER in the
     preheader than the counter's - the maximum is 33/27 = 1.222 exactly, a tie,
     and ties go to the low-numbered source pseudo.  [s9 H3]

 (2) The walker's init insn must satisfy INSN_LUID(init) > INSN_LUID(li 20), or
     sched1's backward list schedule places the two hoisted constants after the
     two p-copies instead of before them.  [s8, unchanged]

 (3) The two constants must be loop.c movables (target holds 5 and 20 in $t2/$t1
     across loop 1), and move_movables inserts every movable immediately before
     NOTE_INSN_LOOP_BEG - i.e. after EVERY pre-loop source statement.  Writing
     them as pre-loop source locals does not evade this: cse propagates them back
     into the loop, the source `li`s die, and loop.c re-hoists them (v_p2a is
     byte-identical to the candidate).  [s9 H4]

(2) + (3) force the walker's init to be a loop.c emission at loop_start; (1)
forces the walker to be a source pseudo with the post-loop reuse.  The only known
loop.c class that emits at loop_start and can carry a pointer is the giv init,
whose destination is a loop.c-created pseudo - so the two requirements collide.
One of these three, or the model behind it, must be wrong; every one of them is
now backed by a measurement rather than a model, which is why the remaining
frontier is loop.c's OTHER loop_start emission classes (combine_movables'
`m->match` register copies and move_movables' `m->partial` path), whose
destination IS a source pseudo.  That is the fourth chassis, and it is now the
only structural route left.

Supporting measurements banked this session (all with cc1's BB2_ALLOC_DEBUG
table, tmp/grind/func_8003504C/s7/alloc.sh):

 - w4 (h1 chassis, counter demoted by carrying loop 2's sel, def block hoisted to
   the top of the val==5 arm): 141/141 instructions; pseudo 176 (giv) -> $a2,
   pseudo 73 (i) -> $a3, pri 8400 vs 9310; normalized positions 1..80 BYTE-CLEAN.
   The preheader and the loop-1 allocation are simultaneously correct for the
   first time in nine sessions.  All 29 residual positions are inside the val==5
   arm, and one of them (`sb $a3` vs target's `sb $a0`) can never go clean,
   because the demotion vehicle forces the counter into the register the carried
   value occupies in target.
 - w1: refs 14 / len 45 -> pri 9333 (misses 9310 by 23) and 140 instructions.
   Session 7's requirement table is exact: refs 14 needs live length >= 46.
 - w3: refs 14 / len 66 -> pri 6363; overshoots, and the counter then conflicts
   with loop 2's inner counter and lands in $t1 rather than $a3.
 - m1: an in-loop invariant base copy costs 142 instructions, because loop.c
   emits the movable copy AND the giv's own initial value.
 - Target's bytes carry NO reference to $a3 between loop 1's exit and
   `.L8003520C: addu $a3,$zero,$zero`, so the original compile had no post-loop
   use of the loop-1 counter at all.

- [s9] w4 (h1 chassis, counter demoted by carrying loop 2's sel with the def block hoisted to the top of the val==5 arm) builds 141/141 instructions and is the FIRST form in nine sessions whose preheader AND loop-1 register assignment are both target's: cc1's allocno table gives pseudo 176 (p-walker giv) -> $a2 and pseudo 73 (i) -> $a3, and normalized positions 1..80 are byte-clean. Its 29 differing positions are all inside the val==5 arm.

- [s9] The counter-demotion requirement table from session 7 is now exact rather than interpolated: w1 = refs 14 / live length 45 -> pri 9333 (fails the 9310 threshold by 23 units); w4 = refs 14 / len 50 -> pri 8400 (succeeds); w3 = refs 14 / len 66 -> pri 6363 (overshoots, and the counter then conflicts with loop 2's inner counter and lands in $t1 instead of $a3).

- [s9] Target's asm contains NO reference to $a3 anywhere between loop 1's exit (lw $v0,0x14($t3) at 0x80035114) and .L8003520C's addu $a3,$zero,$zero, so the original compile had no post-loop use of the loop-1 counter. Any demotion vehicle therefore pins the counter into the hard register the carried value occupies in target ($a0 for sel), which is why w4 emits sb $a3 where target has sb $a0.

- [s9] Target's loop-1 body confirms the p-walker is $a2 itself (lbu $v0,0x0($a2), lbu $v0,0x1($a2), addiu $a2,$a2,0xA), so target's move $a2,$t3 is the walker's own initialisation and there is no room for a separate invariant base register - which is why the in-loop-invariant-copy chassis (m1) costs 142 instructions.

- [s9] Pre-loop constant locals are a no-op spelling: v_p2a measures score 4 at 141/141 with a diff byte-identical to the candidate's, because cse propagates the constants back into the loop, the source li's die, and loop.c re-hoists them as movables. (Session 7 recorded this variant as scoring 9; on the current chassis it is 4.)

- [s9] The three necessary conditions for target's preheader are now each measured and jointly contradictory: (1) the walker's allocno must outrank the counter's, which only a SOURCE pseudo carrying the post-loop D_801027D8 reuse achieves (11 refs / pri 10645 vs 10000); (2) INSN_LUID(walker init) > INSN_LUID(li 20); (3) the two constants are loop.c movables inserted after every pre-loop source statement. (2)+(3) force the walker init to be a loop.c emission, (1) forces it to be a source pseudo - so the fourth chassis (a loop.c loop_start emission whose destination IS a source pseudo) is the only remaining structural route.

- [s9] The floor is unchanged at 4 (141/141 instructions, four differing positions, all the pre-loop rotation li t2,5 / li t1,20 versus move t3,v0 / move a2,t3). src/code6cac_b.c is left carrying memory/grind/func_8003504C/candidate.c.
