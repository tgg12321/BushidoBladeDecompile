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
