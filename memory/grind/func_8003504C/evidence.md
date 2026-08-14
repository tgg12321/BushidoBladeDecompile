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

- [s1] src/ was left byte-identical to HEAD (git diff --stat src/ is empty) and the floor was re-confirmed at 24 after reverting every probe. No cheat construct was written at any point in this session: no pins, no inline asm, no dead stores, no volatile coercion, no unused locals.
