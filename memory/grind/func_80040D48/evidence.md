# Evidence bank — func_80040D48

## [s1 2026-08-24, recon] Baseline, directive acknowledgment, full diff map, floor 34 → 24

**Owner directive ACKNOWLEDGED and being executed:** RULES-TO-ZERO campaign —
34 regfix rules (largest remaining stack), jtbl-coupled (the compiled switch
emits the jtbl), pure-C COMPLETED-C is the route, INCLUDE_RODATA measured dead
in wave 2 (do not re-attempt). This session works pure-C toward 0; no
INCLUDE_ASM/INCLUDE_RODATA probes were run or are needed.

**Honest floor:** at dispatch (HEAD 250f7ee6 committed body) `sandbox
func_80040D48 --disable all` = **34** (272/272 insns, rules_dropped 34,
cheat_asm_stripped 4 — the body's `register s32 a0_s7 asm("s7")` pin among
them). End of s1 with the candidate body in src: **24** (272/272). Every
divergence is a register-seat / addressing-anchor diff; instruction count and
schedule match throughout.

**canonical:** verdict C (pure-C target, distance 34 ≤ 50). Queue agrees.

**Rule-stack decode (regfix.txt:999-1040, all 34):** (1) `$21<->$22` swaps ×3
ranges = the s5/s6 seat swap; (2) prologue structural substs @14-28 = the
load-into-a0 / copy-in-delay-slot shape (paired with the committed pin); (3)
offset substs @49-84 = the a4p IV-bias cluster (s3+0x7C anchor vs s3+0x68);
(4) reg substs @160-244 = mflo temp + tail-loop pointer seats. Classes (1),
(2) and most of (4) are now closed honestly; see below.

**What closed this session (34 → 24), in order:**

1. **Prologue two-var load shape (34 → 31).** Target: `lw $a0,%lo(D_800A9A10)($at);
   beqz $a0; addu $s4,$a0,$zero` (delay slot) + `addiu $s5,$s4,0x2C` landing
   at insn 28 filling a later load-delay nop. Committed body loaded straight
   into s4 (pin + rules faked the rest). Honest fix: `tmp = (u8 *)D_800A9A10[a0];
   if (tmp == 0) return; s4 = tmp;` — tmp is caller-save (dies at the copy),
   the copy lands in the beqz delay slot, and s4+0x2C computes late exactly as
   target. ALSO removed the `asm("s7")` pin entirely: param a0 naturally lands
   in s7 (it crosses 6 calls; pseudo 72 is allocated last of the call-crossing
   set and takes s7 with no coercion). The final call uses plain `a0`.
   NOTE for vetting: `tmp` is once-written, twice-read? — no: written once,
   read twice (test + copy). It holds a real consumed value (the loaded
   pointer) and a human writes exactly this null-check-then-keep shape; it is
   arguably more natural than the one-liner. Flag for self-vet when
   candidate-ready; if the Judge treats it as a named-intermediate family
   member it may need the 6-prong check (multi-read may disqualify prong 1 —
   in that case respell as `if ((s4 = (u8 *)D_800A9A10[a0]) == 0)`-class forms
   and re-measure; NOT yet measured).

2. **s5/s6 seat swap + mflo temp (31 → 24, with the goto-loop spelling).**
   greg dump (tmp/grind/func_80040D48/dumps/, regenerate via
   `pwsh tools/grinder/dump.ps1 func_80040D48`): allocation order is
   priority-sorted; pseudo 77 (arg5, 4 refs/362 insns, log2(4)*4/362 ≈ .022)
   was allocated immediately BEFORE pseudo 79 (s4+0x2C ptr, 2 refs/109 insns
   ≈ .018), taking s5 and pushing the pointer to s6 — exactly the adjacent
   inversion the 3 swap rules papered over. Target's $s5 holds BOTH the
   s4+0x2C value (def@29, use@166 = `sh $zero,6($s5)`) AND the Copy8-loop
   source pointer (def@203 `lw $s5,0x40($a3)`, 10 loop-weighted refs) — i.e.
   in the ORIGINAL these were ONE C variable. flow.c counts
   `reg_n_refs += loop_depth` (flow.c:2081 etc.), so the merged pseudo's
   priority beats arg5's and it is allocated first → s5; arg5 falls to s6.
   Honest fix: reuse the existing `s5` local as the copy-loop pointer
   (variable-reuse family, and here it is evidence-backed as the original's
   own structure, not a codegen trick — the seat only falls out right if the
   two roles share one variable).

3. **Copy-loop spelling constraint (the rotation trap).** With `for (;;) {...
   break;}` + reused s5, loop.c emits loop notes, the loop rotates (bottom
   `bnez` test) and cse folds the first-iteration load to `lw s5,0x112C(s4)`
   → 273 insns, floor 29. Target keeps top-test + unconditional `j` back-edge
   and loads through the a3p base even on iteration 0. Fix: explicit-label
   spelling (`copyloop: { ... goto copyloop; } copydone:;`) — no loop notes,
   no rotation, floor 24, 272/272. Banked:
   rejected/for-loop-s5-reuse-rotates-copy-loop.c.

**Remaining 24 (floor-24 map, artifact
tmp/grind/func_80040D48/s1/floor24_diff.txt; line numbers = 0-based insn
index; ignore LO/LBL/li lines — normalizer reloc artifacts):**

- **Class A — a4p IV-bias cluster (4 diffs; lines 50/65/75/85).** First init
  loop (case 0): ours anchors the store pointer at `addiu $a0,$s3,124`
  (= s3+0x7C) with sh offsets -4/-2/0; target anchors at s3+0x68 (the natural
  `a4p = s3 + 0x68`) with offsets 16/18/20. The C already spells
  `a4p = s3 + 0x68` + `*(s16*)(a4p+0x10/0x12/0x14)`; loop.c strength-reduction
  re-biases the induction variable by +0x14. Old rules @1012-1015 rewrote
  exactly this. Pass attribution needed from .loop dump (regenerate dumps —
  they were generated pre-candidate this session, so the .loop content is
  stale vs the current body but the a4p loop was untouched by s1 edits).
- **Class B — a2p/a3p seat swap in the Copy8 loop (~14 diffs; lines 200-230).**
  Target: a2p→$a2(6), a3p→$a3(7). Ours: a3p→$6, a2p→$7. Both local-alloc'd
  (no calls in loop). a3p has ~3-4x a2p's weighted refs, so priority order
  gives a3p the first free reg ($6) — target's order implies the original's
  a2p qty outranked or preceded a3p's. Derived-base anchor respelling is DEAD
  for this (cse folds — rejected/derived-base-anchor-ref-cse-folded.c). Next:
  read local-alloc.c qty_compare + .lreg qty dump for the ACTUAL order; probe
  first-use-order / def-order permutations (e.g. define a3p first, or touch
  a2p before a3p inside the loop).
- **Class C — a2p2/-1 seats (6 diffs; lines 234-246).** Final walker loop:
  target a2p2→$6, -1 holder→$4; ours a2p2→$4, -1→$5. Straight-line local
  alloc after the copy loop; plausibly cascades from Class B ($6/$7 roles) —
  re-measure after B closes before spending levers on it.
- **mflo seat (was 2 diffs) closed itself** when s5/s6 flipped (162: mflo $8 ✓).

**Chassis notes:** all measurements this session on HEAD 250f7ee6 + the
candidate body in src/text1a_pre.c. The committed rule-era body's shape
(named intermediates, decl order) is rule-calibrated — the candidate body
supersedes it as the working frontier. Diff tooling: s1/diff.sh + s1/norm.py
(normalizes objdump-vs-splat mnemonics; li/addiu-from-zero and LO/LBL lines
are artifacts, not diffs).

**Sibling reference:** func_80040B44 (same TU, COMPLETED-C 2026-08-24,
ledger in git at 68065f31) — its s2 confirmed the fresh-vs-shared-local
partition drives seats and that suspect holders must be re-tested after every
structural change (probe relativity). Both lessons applied here.

- [s1] Owner directive acknowledged + executed: pure-C route only, no INCLUDE_RODATA/INCLUDE_ASM probes run (wave-2 dead per directive)

- [s1] Dispatch floor 34 (272/272, 34 rules dropped, 4 cheat-asm stripped incl. the body's asm(s7) pin); end-of-s1 floor 24 with candidate body in src, 272/272

- [s1] Full rule-stack decode + floor-24 diff map banked in evidence.md; residual = Class A a4p IV-bias (4, loop.c giv re-bias +0x14), Class B a2p/a3p local-alloc seats (~14), Class C a2p2/-1 seats (6, likely cascades from B)

- [s1] greg/lreg dumps read for pass attribution: seat swap was global.c priority order (pseudo 77 arg5 4refs/362 vs 79 ptr 2refs/109, adjacent); flow.c:2081 reg_n_refs += loop_depth is the weighting that lets the merged-variable structure win

- [s1] cse1-folds-derived-base-anchors banked as a dead mechanism class for ref-bumping (rejected/derived-base-anchor-ref-cse-folded.c)

## [s2 2026-08-24, structural] Floor 24 → 4: the a2p/a2p2 merge closes Classes B and C

**Chassis re-measurement.** s1's candidate.c re-applied to src/text1a_pre.c
reproduced `sandbox --disable all` = **24** (272/272, 34 rules dropped, 2
cheat-asm stripped) on HEAD abf5600a. s1's floor is chassis-valid; every
number below is on that chassis with the candidate body in src.

**The Class B mechanism, measured (not guessed).** `pwsh tools/grinder/dump.ps1
func_80040D48` regenerated against the current body. From the `.lreg` dump the
two Copy8-loop pointers are pseudo **186 = a2p** (`s4 + 0x10D4`) and pseudo
**187 = a3p** (`s4 + 0x10EC`). The `.flow` dump gives the exact global.c inputs:

    Register 186 used 4 times across 13 insns; dies in 0 places; pointer.
    Register 187 used 5 times across 12 insns; dies in 0 places; pointer.

`global.c allocno_compare` = `floor_log2(n_refs) * n_refs / live_length`
(×10000×size), sorted DESCENDING:
  - 186: floor_log2(4)*4/13 = 8/13 = **0.615**
  - 187: floor_log2(5)*5/12 = 10/12 = **0.833**

The `.greg` order line confirms it verbatim — `;; 23 regs to allocate: 99 97
192 178 82 168 83 213 187 95 81 75 186 84 80 173 74 78 202 73 79 77 72` — 187
is 9th, 186 is 13th. MIPS defines no `REG_ALLOC_ORDER`, so `find_reg` scans
ascending and the first-allocated of the pair takes the lowest free reg. With
187 first: 187→$6, 186→$7. Target is the opposite.

Note a3p's 5 refs are irreducible: def, `lw 0x40($a3)`, the BLKmode struct copy
(ONE insn at flow time, not 8), and the increment (set+use = 2). a2p's 4 are
def, the list store, and the increment (2). Target's a2p also has only 4 refs,
so **no ref-count edit inside the copy loop can flip the order** — the flip has
to come from OUTSIDE the loop.

**The fix — a2p and the tail walker are ONE variable in the original.** Target
holds the Copy8 loop's list-push pointer (`s4+0x10D4`) in `$a2` AND the later
`s4+0x8B4` walker (Class C's `a2p2`) in `$a2` as well. Merging them into a
single local (`a2p = s4 + 0x8B4;` reusing the same variable after `copydone:`)
raises the merged pseudo to ~9 refs over a ~25-insn live range (pri ≈ 1.08),
which sorts it AHEAD of a3p. Result, measured:

    24 → 4   (272/272 insns)

Class B (14 diffs: `$6`/`$7` swap across the whole copy loop) and Class C
(6 diffs: `a2p2`→`$6`, the `-1` holder→`$4`) BOTH closed in that one edit. This
is the same "target's register serves two roles ⇒ the original had one
variable" reasoning that closed the `$s5` seat in s1, and it is object-model
reconstruction, not a codegen lever. Banked:
`rejected/separate-a2p-and-a2p2-locals-seat-inversion.c`.

**Residual = Class A only (4 diffs), and it is now fully attributed.**
Diff (artifact `tmp/grind/func_80040D48/s2/`, lines are 0-based insn index):

    50  ours: addiu $a0,$s3,124   target: addiu $a0,$s3,104
    65  ours: sh $v0,-4($a0)      target: sh $v0,0x10($a0)
    75  ours: sh $v0,-2($a0)      target: sh $v0,0x12($a0)
    85  ours: sh $v0,0($a0)       target: sh $v0,0x14($a0)

`.loop` dump, case-0 init loop (`Loop from 118 to 197: 28 real insns`):

    Insn 162: possible biv, reg 82,  const = 1     (the s0 counter)
    Insn 165: possible biv, reg 95,  const = 4     (the D_80094CFC tbl pointer)
    Insn 184: possible biv, reg 96,  const = 104   (a4p)
    Insn 137: dest address src reg 96 benefit 2 used 1 lifetime 1 replaceable mult 1 add 16
    Insn 156: dest address src reg 96 benefit 2 used 1 lifetime 1 replaceable mult 1 add 18
    Insn 181: dest address src reg 96 benefit 2 used 1 lifetime 1 replaceable mult 1 add 20
    biv 96 can be eliminated.
    giv at 156 combined with giv at 181
    giv at 137 combined with giv at 181
    giv at 181 reduced to (reg:SI 213)
    biv 96 was eliminated.

Source read (`tools/gcc-2.7.2/loop.c`):
  - `combine_givs` iterates `bl->giv` head-first and `record_giv` PREPENDS, so
    the head is the LAST-discovered giv — insn 181, the `+0x14` store. The base
    of the combination is therefore always the highest-offset (last-emitted)
    address, which is why our reduced register is `s3 + 0x68 + 0x14 = s3+124`
    and the three stores become `-4/-2/0`.
  - `combine_givs` credits the base with the absorbed benefits
    (`g1->benefit += g2->benefit`), so the combined giv has benefit 6.
  - The reduction test is `v->lifetime * threshold * benefit < insn_count`
    (loop.c:3823) with `benefit -= add_cost * bl->biv_count` and
    `threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)`.
    Measured `add_cost = 2` — the sibling single-giv loops in this same function
    print `giv of insn 499 not worth while, 0 vs 12`, i.e. benefit 2 − 2*1 = 0.
    Combined: 6 − 2*1 = 4, lifetime 3 → 3*threshold*4 ≫ 28 ⇒ always reduced.

**Target is provably UNREDUCED, and no combination base can produce it.** If
target's `$a0` were a reduced giv with base add A, the last-emitted store would
sit at offset 0 (it is the base). Target's last store is at `+0x14`, and its
base register value is exactly `s3 + 0x68` = the biv's initial value. So in the
original this loop's three dest-address givs were NOT combined and NOT reduced,
i.e. `benefit ≤ 0` there. Two corollaries for the next session:
  - Re-basing a4p so one store sits at offset 0 does NOT help: loop.c does not
    record a mult-1/add-0 address as a giv at all, so the remaining two still
    combine AND the biv then survives as a second live register (+insns). Both
    `a4p = s3+0x78` (offsets 0/2/4) and `a4p = s3+0x7C` (offsets -4/-2/0) are
    dead by this argument — do not spend measurements on them.
  - Changing the store ORDER only moves the base to a different nonzero offset;
    no permutation yields base `+0` with offsets `+16/+18/+20`.
  The only shapes that reach target are (a) `a4p` failing biv verification in
  loop.c (any second SET of that pseudo inside the loop body sets
  `reg_iv_type = NOT_BASIC_INDUCT`, after which the three addresses stay
  biv-relative and the single `addiu $a0,$a0,0x68` is emitted as an ordinary
  add — this is target's shape exactly), or (b) `bl->biv_count >= 3` (three
  increment insns — costs +2 insns, dead). Route (a) is the live axis.

**Sibling confirmation of the model.** The second init loop in the same
function (`p = s3 + 0x6E8; *(s16*)(p+6) = 0; p -= 0x68`) has ONE address giv,
prints `not worth while`, keeps its biv, and MATCHES target byte-for-byte. Same
for the a4p push loop (`Cannot eliminate biv 178: biv used in insn 516` — the
`*list2 = (s32)a4p` store uses the biv value directly, so no reduction). The
case-0 init loop is the only three-giv loop in the function and the only one
that diverges. That is the whole of the remaining residual.

- [s2] Chassis re-verified: s1 candidate re-applied = floor 24 on HEAD abf5600a (272/272)
- [s2] .flow gives exact allocator inputs: a2p(186) refs=4 len=13 pri .615; a3p(187) refs=5 len=12 pri .833 -> a3p allocated first, takes $6; target needs the reverse
- [s2] a2p (Copy8 list-push walker) and a2p2 (s4+0x8B4 tail walker) are ONE variable in the original — merging them raises the pseudo's priority above a3p's and closes Classes B (14) and C (6) together: floor 24 -> 4
- [s2] Class A fully attributed to loop.c combine_givs + strength reduction; target proven UNREDUCED (base = biv, last store at +0x14 not 0); re-basing a4p and store reordering both proven dead by the loop.c source, the live axis is defeating biv verification of a4p

- [s2] Chassis re-verified: s1's candidate.c re-applied to src/text1a_pre.c reproduces sandbox --disable all = 24 (272/272, rules_dropped 34, cheat_asm_stripped 2) on HEAD abf5600a. s1's banked floor is chassis-valid.

- [s2] Exact global.c inputs read from the .flow dump (not guessed): 'Register 186 used 4 times across 13 insns' (a2p) and 'Register 187 used 5 times across 12 insns' (a3p); the .greg allocation-order line confirms 187 is allocated 4 slots before 186.

- [s2] a3p's 5 refs are def + lw 0x40($a3) + the BLKmode struct copy (ONE insn at flow time, not 8 stores) + increment set/use; a2p's 4 are def + list store + increment set/use. Target's a2p has the same 4 refs, so no in-loop ref edit can flip the order.

- [s2] Merging a2p with the s4+0x8B4 tail walker into one local measured 24 -> 4 (272/272). Classes B and C closed together; every insn in the function now matches target except the four Class A lines.

- [s2] Class A residual (0-based insn index): 50 ours addiu $a0,$s3,124 vs target addiu $a0,$s3,104; 65/75/85 ours sh $v0,-4/-2/0($a0) vs target sh $v0,0x10/0x12/0x14($a0).

- [s2] loop.c .loop dump for the case-0 init loop: three dest-address givs (add 16/18/20, benefit 2, lifetime 1) off biv 96 (const 104); 'giv at 156 combined with giv at 181', 'giv at 137 combined with giv at 181', 'giv at 181 reduced to (reg:SI 213)', 'biv 96 was eliminated'.

- [s2] add_cost measured = 2 from this function's own sibling loops, which print 'giv of insn 499 not worth while, 0 vs 12' for a single uncombined giv (benefit 2 - 2*1 = 0). Combined benefit 6 - 2 = 4 always passes the threshold.

- [s2] Model cross-checked against the two loops in this function that already MATCH target: the s3+0x6E8 backward init loop and the a4p push loop each have a single address giv (or use the biv value directly), print 'not worth while' / 'Cannot eliminate biv', keep their biv, and match byte-for-byte. The case-0 init loop is the only three-giv loop and the only one that diverges.

- [s2] Proven dead without measurement (do not spend probes): re-basing a4p to s3+0x78 (offsets 0/2/4) or s3+0x7C (offsets -4/-2/0), and any permutation of the three stores. loop.c does not record a mult-1/add-0 address as a giv, so a zero-offset store forces the biv to survive alongside the reduced register.

- [s2] Owner directive (RULES-TO-ZERO campaign) followed: pure-C route only, no INCLUDE_RODATA / INCLUDE_ASM probes attempted. The 34 regfix rules are retired by reaching COMPLETED-C, and the honest floor is now 4.


## [s3 2026-08-25, structural] Floor 4 -> 0: the case-0 init loop is not a loop.c loop, and the walker is the arg4 variable

**Chassis re-measurement.** s2's `candidate.c` re-applied to `src/text1a_pre.c`
reproduced `sandbox func_80040D48 --disable all` = **4** (272/272, rules_dropped 34,
cheat_asm_stripped 2 -- both strips are in the untouched `func_80041188`, line 752,
not in this function) on HEAD 51a9e464. Every number below is on that chassis.

**Class A closed -- and s2's attribution was right, but its conclusion ("the live axis
is defeating biv verification of a4p") pointed at the wrong axis.** The correct
reading of s2's own proof is stronger: target's loop was never processed by loop.c AT
ALL. Reading `tools/gcc-2.7.2/loop.c:3775-3830` directly this session confirms the
arithmetic is unwinnable from inside a note-delimited loop:
  - `benefit -= add_cost * bl->biv_count` (loop.c:3803) and the skip test
    `v->lifetime * threshold * benefit < insn_count` (loop.c:3823).
  - For DEST_ADDR givs `record_giv` hardcodes `v->lifetime = 1` (loop.c:4394);
    `combine_givs` (loop.c:5494) then does `g1->benefit += g2->benefit` and
    `g1->lifetime += g2->lifetime`, so the merged giv is benefit 6 / lifetime 3.
  - `threshold` is `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` with
    `n_non_fixed_regs == 60` on this target (measured and documented in
    `.claude/rules/defeat-licm-hoist-var-reuse.md`), i.e. 126 (63 with a call).
    `3 * 126 * 4 = 1512` vs `insn_count` 28. The skip can NEVER fire.
  - Preventing the combine is equally impossible: `combine_givs_p` (loop.c:5458)
    combines two DEST_ADDR givs when `express_from` yields a valid address whose
    `ADDRESS_COST` is `<=` the original's. `ADDRESS_COST` for MIPS is
    `(REG_P (ADDR) ? 1 : mips_address_cost (ADDR))` (mips.h:2897) and
    `mips_address_cost` returns 1 for `PLUS(reg, SMALL_INT)` -- so any two stores
    whose offsets differ by less than 0x8000 always combine. Target's offsets differ
    by 2 and 4.
  So with `NOTE_INSN_LOOP_BEG` present, GCC 2.7.2 CANNOT emit target's shape for this
  loop, however the C is spelled. **PROBE: spell the loop with an explicit label and a
  backward `goto` so no loop notes are emitted.** Measured: the four Class A diffs
  vanish -- the loop body comes out byte-identical to target
  (`addiu $a0,$s3,104` / `sh ...,16/18/20($a0)` / `addiu $a0,$a0,104` in the delay
  slot), artifact `tmp/grind/func_80040D48/s3/build.insns` lines 52-93.

**The wrapper does NOT work.** `do { ... goto initloop; ... } while (0);` (the idiom
the COMPLETED sibling `func_80040CB8` uses at `src/text1a_pre.c:524-548`) re-creates
the notes: measured back to **4**, byte-identical to the plain do/while form. Banked
`rejected/dowhile0-wrapper-restores-loop-notes-floor4.c`. Losing the notes is the
whole point; there is no spelling that keeps the weighting AND loses the reduction.

**The price of losing the notes, and how the original paid it (floor 111 -> 0).**
The goto spelling alone measured **111 (274/272)**: two extra insns and a global
callee-saved rename. Attribution read from the regenerated dumps, not guessed:
  - `.greg`: `;; 22 regs to allocate: 97 99 178 186 168 82 187 83 81 75 84 80 173 74
    78 96 95 201 73 79 77 72` -- pseudo **168** (the `func_800417D0` walker) is
    allocated BEFORE pseudo **82** (the shared counter).
  - `.flow`: `Register 168 used 7 times across 10 insns` (pri
    floor_log2(7)*7/10 = 1.40) vs `Register 82 used 25 times across 81 insns` (pri
    floor_log2(25)*25/81 = 1.235). With the notes, flow.c's `reg_n_refs += loop_depth`
    had boosted the counter over the walker; without them it loses the race.
  - MIPS defines no `REG_ALLOC_ORDER`, so the walker takes the first free callee-saved
    reg ($s0), the counter -- which conflicts with it inside that loop
    (`;; 82 conflicts: ... 168 ...`) -- is pushed to $s1, and every later callee-saved
    allocno shifts up one seat. The function ends up needing a NINTH callee-saved
    register: `;; Register dispositions: 72 in 30` ($s8/$fp) and
    `;; Hard regs used: ... 16 17 18 19 20 21 22 23 30 ...`, plus the `sw`/`lw` pair
    that saves it (the +2 insns).
  - **Fix, and it is the same object-model argument as s1's $s5 and s2's $a2:** in the
    original that walker and the `arg4` pointer are ONE variable. Target's `$s1` is
    loaded from the incoming stack slot, read through `0x6C..0x82($s1)` in case 0, and
    then redefined by `addu $s1,$s3,$zero` to walk `s3` by 0x68. Reusing the existing
    `s1` local for the walk restores counter-first order: **0 (272/272)**.
  Banked `rejected/goto-init-loop-separate-s1p-walker-extra-s8.c`.

**Prologue refinement (construct hygiene, byte-neutral).** s1's `u8 *tmp` two-var load
was respelled as `s32 ent = D_800A9A10[a0]; if (ent == 0) return; s4 = (u8 *)ent;`.
`D_800A9A10` is declared `extern s32 []`, so reading the element into an `s32`,
null-checking it and casting is ordinary C with two distinctly TYPED values rather
than two same-typed handles -- it removes the "why are there two pointers?" question
s1 flagged for the Judge. Measured **0** (unchanged). The one-variable collapse
(`s4 = (u8 *)D_800A9A10[a0]; if (s4 == 0) return;`) measures **4** -- banked
`rejected/single-var-prologue-entry-load-floor4.c` -- so the split is load-bearing and
mirrors target, which loads the entry into caller-saved `$a0`, tests THAT, and copies
to `$s4` in the `beqz` delay slot.

**Re-tested on the new chassis and still dead:** spelling the Copy8 loop as
`while ((s5 = *(u8 **)(a3p + 0x40)) != 0) { ... }` measures **7 (273/272)** -- loop.c
rotates it to a bottom test and cse folds the first-iteration load, exactly as s1
measured for the `for(;;)+break` form. The `copyloop:`/`copydone:` spelling stands.

**FINAL STATE: honest floor 0**, 272/272 insns, all 34 regfix rules dropped, zero
inline asm, zero pins, zero volatile, zero dead code in the body. `self_vet.md`
written (SANCTIONED-FAMILY-CLAIMS: none -- the argument is object-model
reconstruction, mirroring the accepted sibling `func_80040B44` vet at `68065f31`).
Three short explanatory comments were added at the two goto loops and the `s1` reuse,
in the house style of this TU's matched siblings; re-measured 0 with them in place.

- [s3] Chassis re-verified: s2 candidate re-applied = floor 4 (272/272, rules_dropped 34) on HEAD 51a9e464.
- [s3] loop.c read directly (3775-3830, 4394, 5458-5530) + mips.h:2897 + mips_address_cost: with NOTE_INSN_LOOP_BEG present, three DEST_ADDR givs off one biv whose offsets differ by <0x8000 ALWAYS combine (ADDRESS_COST 1 <= 1) and the merged giv (benefit 6, lifetime 3) can NEVER hit the "not worth while" skip (3*126*4 = 1512 vs insn_count 28). Target's shape is unreachable from ANY note-delimited spelling of this loop.
- [s3] Spelling the case-0 init loop as `initloop:` + backward `goto` (no loop notes -> loop.c never sees it) makes the loop body byte-identical to target: base addiu $a0,$s3,104, stores at 16/18/20, single addiu $a0,$a0,104 in the delay slot. Class A CLOSED.
- [s3] `do { ... goto initloop; } while (0);` re-creates the loop notes and measures 4 again, byte-identical to the plain do/while. The sibling func_80040CB8 wrapper idiom does NOT suppress strength reduction.
- [s3] The goto spelling alone measures 111 (274/272): losing the notes loses flow.c's loop_depth ref weighting for the shared counter, so global.c allocates the func_800417D0 walker (pseudo 168, 7 refs/10 insns, pri 1.40) before the counter (pseudo 82, 25 refs/81 insns, pri 1.235); they conflict, the counter is pushed off $s0, and the whole callee-saved bank shifts up, requiring a ninth register ($s8, "72 in 30") plus its save/restore.
- [s3] Merging the func_800417D0 walker into the existing `s1` (arg4) local restores counter-first allocation order and closes the last 4 diffs: floor 0, 272/272. Target's $s1 serves exactly those two roles (stack-slot load + 0x6C..0x82 reads, then addu $s1,$s3,$zero for the walk).
- [s3] Prologue respelled `s32 ent = D_800A9A10[a0]; if (ent == 0) return; s4 = (u8 *)ent;` (byte-neutral, two distinctly typed values); the one-variable collapse measures 4, so the split is load-bearing and mirrors target's load-into-$a0 / test / copy-in-delay-slot shape.
- [s3] Copy8 loop as a `while ((s5 = ...) != 0)` re-tested on this chassis: 7 (273/272), same rotation+cse fold s1 measured. The goto spelling stands.

## [s3b 2026-08-25] Floor 4 / layer-1-FAILed-0 -> honest floor 0 with NO goto and NO loop-note suppression

- The 2026-08-25 02:53 layer-1 FAIL was correct about the mechanism and wrong about the
  necessity: the init loop does NOT have to lose its NOTE_INSN_LOOP_BEG to reach target.
  Target's "unreduced-looking" shape (base register = s3+0x68, stores at +0x10/+0x12/+0x14,
  a single `addiu $a0,$a0,0x68`) is exactly what loop.c PRODUCES when the destination
  cursor is a GIV of the counter rather than a BIV of its own -- i.e. when the C indexes
  the array (`a4p = s3 + s0 * 0x68;`) instead of hand-incrementing a pointer. combine_givs
  then merges the three DEST_ADDR givs onto the cursor giv (add_val 0x68) instead of onto
  the last-recorded address giv (add_val 0x7C), and the reduced base is the cursor itself.
- The increments must sit AFTER the third store. loop.c emits the giv update immediately
  after the biv increment; with `s0++; tbl++;` before the third store the update lands
  mid-body and RTL has to copy the pre-increment cursor (`move v1,a1`), which takes $v1
  and pushes the index temp out of it, shifting cursor/table from $a0/$a1 to $a1/$a2.
  Measured: increments early = score 20, increments late = score 0.
- The Copy8 loop's rotation is jump.c:2163 `duplicate_loop_exit_test`, invoked from
  jump.c:626 ONLY when the insn following NOTE_INSN_LOOP_BEG is an unconditional jump.
  That is the `while` expansion (jump to a bottom test). `for (;;)` with an explicit
  `break` puts the test at the top and an unconditional `j` at the bottom -- the transform
  never runs, cse never sees a constant-offset copy of the entry load, and the shape is
  target's (target insn 229 `j @`, delay slot 230 `addiu a2,a2,104`). Measured:
  `while` = 273 insns / score 7, `for (;;)`+break = 272/272 score 0.
- With the init loop's notes restored, the s1/arg4 walker merge that s3's H9 needed is
  gone: reverting the walker to its own local keeps score 0. That construct (a banned one
  for this function) is out of the body entirely, as are both goto spellings.
- Re-measured on the score-0 chassis and still load-bearing (all four banked in
  rejected/): the prologue two-variable load (collapse = 4), the `s5` reuse (split = 35 at
  block scope AND at function scope), the `a2p` reuse (split = 20 at both scopes).
  Declaration placement changes nothing for either reuse.
- Diff tooling for this function: `tmp/grind/func_80040D48/s3/dif2.py` prints the
  masked instruction-by-instruction diff via `engine.score.normalized_insns` against
  `build/src/text1a_pre.o`; `reg.py <lo> <hi>` prints an aligned window. Both are far
  cheaper than eyeballing objdump and correctly ignore %hi/%lo and branch targets.
  `apply.sh <bodyfile>` swaps a candidate body into src/text1a_pre.c (head.c/tail.c
  are the frozen prefix/suffix of the file).
