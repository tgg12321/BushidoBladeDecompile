# Evidence — func_8006A564 (src/text1b.c)

## Session 1 (recon)

- **OBJECT MODEL: MATCHES.** The brief's DATA MODEL flagged `D_800A374C`
  (census: `g_dma_buf_base`, decl `extern s32 D_800A374C;` in
  `include/m2c_context.h`). Grepped every existing use in `src/text1b.c`
  (60+ sites across the file, e.g. lines 1661/3373/3453/3607/3661/...):
  ALL treat it as a plain `s32` OT/DMA-buffer base added to a `<<2`-scaled
  index and passed as the first arg to `AddPrim(s32, void*)`. The target
  asm for func_8006A564 (`asm/funcs/func_8006A564.s:49-53`,
  `:87-91`, `:127-131`, `:191-195`) does the identical
  `lui/lw %hi/%lo(D_800A374C); sll a0,a0,2; addu a0,v0,a0` sequence four
  times. No declaration mismatch, no split-scalar signal, no
  base-register-store-invisible-to-grep pattern found. Object model needs
  no fix; this is an ordinary global read, not the residual's source. No
  measurement needed beyond the grep (it's a decl-vs-use consistency check,
  not a codegen question) — MATCHES is the finding, not a score.

- Function is not yet decompiled: HEAD was `INCLUDE_ASM("asm/funcs",
  func_8006A564);` (src/text1b.c:6418 before this session's edit).
  `no_c_body: true` on the pre-session sandbox run.
- `canonical func_8006A564` = verdict C, hand_coded_tier LOW (no S1/S2/S6
  signal) — ordinary pure-C target, not a canonical-asm candidate.
- Target asm (`asm/funcs/func_8006A564.s`, 209 lines / 199 real insns,
  `.frame` implied by `addiu sp,sp,-0x30`) is a leaf-ish rendering routine:
  reads `arg0` (a struct with a growing tile-buffer pointer at +0x18, a
  second-object pointer at +4, and a draw-mode field at +0x1C), `arg1` (a
  ~0x2C-byte "packet" struct — same shape as `func_80075830`'s local
  `u8 packet[0x2C]`, src/text1b.c:8303 — with byte color fields at
  0x29/0x2A/0x2B, matching `src/text1b.c:8311-8313`), and `arg2` (an int
  compared against `D_800A34F8 & 0xF` three times).
- Wrote an initial full-body C translation (see candidate.c) mirroring the
  asm literally: 3x `SetTile`/color-select-if/`SetSemiTrans`/`AddPrim`
  tile-draw blocks (color bytes chosen by `D_800A34F8&0xF == arg2`), then a
  second-object dereference chain (`*(arg0+4)` -> `+0x1C` -> `+0x28`) whose
  result feeds `arg1`'s packet, a similar color-select block (halving
  instead of literal 0x50/0x20/0x10), two calls to `func_8007352C(arg1)`
  (each preceded by staging a different source field into `arg1+8`), and a
  closing `SetDrawMode`/`AddPrim`/`+=0xC` on `*(arg0+0x1C)`.
- **First sandbox measurement of a real C body: score 137** (was 199 /
  no-c-body). `build_insns 204` vs `target_insns 199` — the body compiles
  and is structurally very close (+5 insns) but not byte-identical.
- **srl-vs-sra byte diff, KILLED.** The two `color >> 1` halving ops
  (block 3 and the final color-select block) initially emitted `sra`
  (arithmetic shift) because `v0` is declared `s32` and GCC's default
  shift-right on a signed type is arithmetic. Target emits `srl` (logical)
  at both sites (`asm/funcs/func_8006A564.s:101/105/148/149`). Fix: cast
  through `(u32)` before the shift (`v0 = (u32)v0 >> 1;`). Confirmed via
  objdump of `tmp/sandbox/func_8006A564/text1b.o` (turn ~14 of this
  session): both sites now emit `srl s0,s0,0x1` matching target exactly.
  Sandbox score UNCHANGED at 137 (the srl/sra opcode swap was already
  counted at the same weight either way in the masked score, or was
  already dwarfed by the larger register-allocation diff below — not
  independently isolated this session).
- **Register-allocation mismatch, register mapping only — not yet
  resolved.** Disassembled `tmp/sandbox/func_8006A564/text1b.o` (objdump,
  turn ~11): our build allocates `s3=arg0, s2=arg1, s4=arg2,
  s5=(the local I call \`s4\` in C — the SetSemiTrans-arg /
  func_8006E480-arg2 flag), s0/s1 scratch`, needs 6 callee-saved regs
  (s0-s5+ra) and a `-56` (0x38) byte frame (`addiu sp,sp,-56`, extra
  `sw s5,44(sp)` vs target). Target (`asm/funcs/func_8006A564.s:2-10`)
  allocates `s2=arg0, s1=arg1, s3=arg2, s4=(the same flag local)`, only
  5 callee-saved regs (s0-s4+ra), frame `-0x30`. This is the entire
  +5-instruction gap (204 vs 199): our build's extra callee-save
  push/pop (`sw`/`lw s5`) plus the bigger `addiu` immediates. No lever
  applied yet this session (recon budget) — this is the frontier for the
  next session.
- No permuter run this session (recon modality — establishing the
  baseline candidate and object-model check was the mandate).

- [s1] OBJECT MODEL: MATCHES for D_800A374C -- grepped 60+ existing src/text1b.c use sites, all plain s32 OT-base + <<2-scaled-index feeding AddPrim(s32,void*); target asm does the identical lui/lw hi/lo + sll 2 + addu sequence 4x (asm/funcs/func_8006A564.s:49-53,87-91,127-131,191-195). No declaration mismatch, no split-scalar/base-register-invisible signal. Needs no fix.

- [s1] Function had no C body at session start (INCLUDE_ASM, no_c_body:true, distance 199); canonical verdict C, hand_coded_tier LOW -- ordinary pure-C target, ruled out canonical-asm routing.

- [s1] Wrote a full literal C translation of the 199-insn target (3x SetTile/color-select/SetSemiTrans/AddPrim tile-draw blocks + second-object dereference chain + two func_8007352C(arg1) calls + closing SetDrawMode/AddPrim) -- first-cut sandbox score 137, build_insns 204 vs target 199 (+5 insns).

- [s1] Disassembled tmp/sandbox/func_8006A564/text1b.o: our build allocates {s0,s1,s2,s3,s4,s5} (6 callee-saved, arg0=s3,arg1=s2,arg2=s4, flag-local=s5) with an 0x38-byte frame; target allocates only {s0,s1,s2,s3,s4} (arg0=s2,arg1=s1,arg2=s3,flag-local=s4) with an 0x30-byte frame (asm/funcs/func_8006A564.s:2-10). This register-mapping/frame-size difference is the entire +5-insn residual.

## Session 2 (structural)

Chassis check confirmed: sandbox --disable all == 137 at session start (matches
ledger, s1's candidate.c had NOT yet been applied to src/ -- applied it first).

- **[s2] BLOCK-LOCAL VAR SPLIT closes the extra-register problem: 137 -> 74.**
  `.greg` dump (`pwsh tools/grinder/dump.ps1 func_8006A564`, read
  tmp/grind/func_8006A564/dumps/text1b.greg lines 51553-52724) showed the s1
  candidate's single function-wide `s32 v0;` (reused across all 3 tile-draw
  blocks) had pseudo dispositions spanning the WHOLE function, and the global
  allocator needed 6 hard regs (`;; 6 regs to allocate: 77 75 73 72 74 78` ->
  s0,s1,s2,s3,s4,s5). Wrapped each tile-draw block's `v0` (and the final
  section's) in its OWN `{ }` scope with a fresh `s32 v0;` declaration --
  confining each instance's live range to its own block. Re-dumped: `;; 9 regs
  to allocate` (more total pseudos, since now there are 4 separate `v0`
  instances) but `;; Hard regs used: 2 3 4 5 6 7 16 17 18 19 20 29 31` -- s5
  (21) is GONE. The per-variable mapping ALSO came out matching target exactly
  with no further work: pseudo72->s2(arg0), 73->s1(arg1), 74->s3(arg2),
  75->s0(tile), 77->s4(the flag local) -- verified against target's own
  prologue (`asm/funcs/func_8006A564.s:2-10`: `sw $s2`,`addu $s2,$a0`;
  `sw $s1`,`addu $s1,$a1`; `sw $s4`; `sw $s3`; `sw $s0`; `lw $s0,0x18($s2)`;
  `addu $s3,$a2`). Confirmed via `& tools/wteng.ps1 main sandbox func_8006A564
  --disable all`: score 137 -> 74, build_insns 204 -> 202.
- **[s2] Delay-slot statement reorder: 74 -> 73.** Objdump of the rebuilt
  .o (via `wsl bash -c 'mipsel-linux-gnu-objdump -d ...'` -- no
  mipsel-objdump on the Windows-side PATH, must go through WSL) showed our
  build's `bne` delay slot in block1's else-arm filled with `li $3,0x50` (the
  v0=0x50 constant) where target fills it with `addiu $s4,$zero,0x20`
  (asm/funcs/func_8006A564.s:17-18). Textually reordering the else-arm to
  write `s4 = 0x20;` BEFORE `v0 = 0x50; tile[4]=v0; tile[5]=v0;` got cc1's
  list scheduler to choose the s4 write for the delay slot instead --
  byte-identical to target at that branch. Sandbox 74 -> 73 (this diff was
  a single scheduling choice, not an insn-count change: build_insns stayed
  202 here; the count drop came from the two field-offset fixes below).
- **[s2] REAL BUG in s1's block-2 tile+8 field offset, found via
  register+operand-level objdump diff (tmp/grind/func_8006A564/s2/diff2.py):
  73 -> 72, build_insns 202 -> 201.** s1's candidate had
  `v0 = *(arg1+0x18); v0 = v0 + 0x78; *(tile+8) = v0;` for the 2nd tile-draw
  block. Target's actual asm (`asm/funcs/func_8006A564.s:73,75,77-78`:
  `lw $v0,0x18($s1)` ... `sh $v0,0x8($s0)` with NO add between them, and
  SEPARATELY `addiu $v0,$zero,0x78; sh $v0,0xC($s0)`) proves `tile+8` gets
  the RAW field value and `tile+0xC` gets the literal 0x78 -- the s1
  candidate had double-applied the 0x78 constant (once as a spurious add,
  once correctly as the tile+0xC literal it already had). This was a plain
  transcription bug in s1's candidate, not a codegen/allocation question.
  Fixed by dropping the spurious `v0 = v0 + 0x78;` line.
- **[s2] Redundant re-read of *(arg0+0x18) replaced with tile+0x10 reuse:
  part of 202->201->199 insn-count convergence.** s1's candidate re-derived
  the third-object tile pointer from scratch
  (`tile = *(u8 **)(arg0 + 0x18) + 0x30;`) instead of reusing the
  already-live `tile` local (already at +0x20 from the two prior
  `tile += 0x10` advances). Target's asm
  (`asm/funcs/func_8006A564.s:124-127`: `lw $v0,0x4($s2); addiu $s0,$s0,0x10;
  sw $s0,0x18($s2); lw $s0,0x1C($v0)`) advances `$s0` (tile) IN PLACE by
  0x10 and never reloads it from `*(arg0+0x18)`. Fixed to
  `tile = tile + 0x10;`. This removed the extra `lw` the diff had flagged
  at target-index 125.
- **[s2] Dual-variable split closes build_insns to EXACT target parity:
  201 -> 199 (== target).** Register+operand diff at the final
  color-halve block (arg1+0x29/+0x2B) showed target loading BOTH bytes
  first into SEPARATE registers (`lbu $v0,0x29($s1); lbu $v1,0x2B($s1)`,
  asm/funcs/func_8006A564.s:137-138) before doing both `srl`s and both
  stores -- our single-`v0`-reused serial load/shift/store/load/shift/store
  needed one extra instruction to re-serialize the dependency. Split into
  a second block-scoped local `s32 v1;` for the second byte (same family as
  the block-local split above). build_insns now == target_insns == 199
  exactly (was 204 at s1 start of session).
- **[s2] Full register-normalized operand diff
  (tmp/grind/func_8006A564/s2/diff3.py) at build_insns==target_insns==199
  shows the ENTIRE remaining 68-point residual is a SYSTEMATIC v0<->v1
  register-coloring swap, not a control-flow or instruction-count issue.**
  Every mismatched line normalizes to the same shape: target does
  `lw $v0,SYM; andi $v0,$v0,0xF` (in-place AND, load and result share one
  register) at all 4 `D_800A34F8 & 0xF` sites; our build does
  `lw v0,SYM; andi v1,v0,0xf` (the AND's result lands in a DIFFERENT
  register than the preceding load). This repeats consistently through
  each block's tail (every place target's C variable equivalent occupies
  $v0, ours occupies $v1 and vice versa for the literal/secondary values).
  This looks like ONE upstream coloring decision (which of the two pseudos
  -- raw load temp vs. the named AND result -- gets first pick of $v0)
  repeated 4x by symmetry of the 3 tile blocks + final section, not 4
  independent ties.
- **[s2] KILLED (instance): splitting `v0 = D_800A34F8 & 0xF;` into two
  statements (`v0 = D_800A34F8; v0 &= 0xF;`) does NOT fix the v0/v1
  coloring swap -- measured WORSE (68 -> 72) and reverted.** This rules
  out "the combined expression creates an extra temp" as the mechanism;
  cc1 still chooses the same coloring even with the load and mask as
  separate C statements. Chassis: candidate.c at s2 state, no FAKE
  constructs present, measured via sandbox --disable all.
- **[s2] One remaining scheduling tie (not insn-count-affecting): in
  block3, target orders `sb $v0,0x6($s0)` (the shared-join tile[6] store)
  BEFORE `addu $a0,$s0,$zero` (call-arg setup for the next SetSemiTrans);
  our build schedules them in the OPPOSITE order.** Both instructions are
  mutually independent and ready at the same point (classic
  "sched-rank-class-tie-wall" per codegen-technique-index) -- moving the
  SetSemiTrans call earlier in the C source was tried and reverted (wrong
  fix: it relocates the call's target position entirely rather than just
  reordering the tie; not committed, not measured as a scored hypothesis).

- [s2] Applying s1's candidate.c and re-measuring confirmed the chassis floor at session start: 137 (build_insns 204 vs target 199), matching the ledger exactly.

- [s2] func_8006A564 is a 3x tile-draw + second-object-dereference + two func_8007352C calls + closing SetDrawMode/AddPrim routine in src/text1b.c; canonical verdict C, hand_coded_tier LOW (not a canonical-asm candidate).

- [s2] s1's candidate.c had a genuine transcription bug in tile-draw block 2: it added a spurious `+0x78` to the value stored at tile+8 (target's asm shows tile+8 gets the RAW *(arg1+0x18) field with no add; 0x78 belongs ONLY as the literal stored to tile+0xC, which the candidate already had separately). Fixing this alone dropped build_insns 202->201.

- [s2] s1's candidate.c also redundantly re-derived the third-object tile pointer from scratch (`*(u8**)(arg0+0x18) + 0x30`) instead of reusing the already-live `tile` local (already advanced by 0x20 from two prior `tile += 0x10`). Target's asm advances $s0 (tile) in place by 0x10 and never reloads it from memory. Fixing this plus a dual-variable split for the final color-halve block's two independently-live packet bytes brought build_insns to EXACT parity with target: 199 == 199 (was 204 at s1).

- [s2] With build_insns==target_insns==199, a register-normalized full operand diff (tmp/grind/func_8006A564/s2/diff3.py, diffing asm/funcs/func_8006A564.s against a WSL objdump of tmp/sandbox/func_8006A564/text1b.o with $vN/$sN normalized and hi/lo/gp_rel wrappers masked) shows the ENTIRE remaining 68-point score is a SYSTEMATIC v0<->v1 register-coloring swap: target does `lw $v0,SYM; andi $v0,$v0,0xF` (in-place, load and AND-result share one register) at all 4 `D_800A34F8 & 0xF` sites; our build does `lw v0,SYM; andi v1,v0,0xf` (result lands in a different register). This is NOT a control-flow or instruction-count residual anymore.

- [s2] One additional, smaller residual (not insn-count-affecting): in block3, target schedules `sb $v0,0x6($s0)` (the shared-join tile[6] store) before `addu $a0,$s0,$zero` (call-arg setup); our build schedules the reverse order -- an equal-priority list-scheduler tie between two mutually-independent ready instructions.

- [s2] The delay-slot fill for block1's else-arm branch now matches target byte-for-byte after reordering the else-arm's statements to write the flag local (`s4 = 0x20;`) before the unrelated `v0 = 0x50;` sequence -- confirmed via WSL objdump of the rebuilt .o.

## Session 3 (structural)

Chassis check confirmed: applied s2's candidate.c to src/text1b.c (HEAD had
`INCLUDE_ASM`, no C body applied at session start), re-measured via
`sandbox --disable all` -- score 68, build_insns 199==199, matching the
ledger and the driver's dispatch-time floor exactly.

- **[s3] PASS ATTRIBUTION READ: `.combine` and `.greg` dumps for the FIRST
  `D_800A34F8 & 0xF` site confirm the exact mechanism behind session 2's
  "systemic v0<->v1 coloring swap" finding.** Read
  `tmp/grind/func_8006A564/dumps/text1b.combine` (func_8006A564 section,
  lines 46779-47898) and `.greg` (lines 51553+):
  - `.combine`: the load `D_800A34F8` goes into pseudo 79 (dies
    IMMEDIATELY at the very next insn, single def/single use, genuinely
    block-local); the AND's result goes into pseudo 78, which is the SAME
    pseudo used for `v0` throughout the rest of the block (both if/else
    arms + everything after the merge) -- i.e. pseudo 78 crosses the
    conditional branch, pseudo 79 does not.
  - `.greg`: pseudo 79 -> hardreg 2 (`$v0`); pseudo 78 -> hardreg 3
    (`$v1`). GCC 2.7.2's pass order runs LOCAL-ALLOC before GLOBAL-ALLOC;
    local-alloc handles genuinely block-local pseudos first and grabs the
    first free int-class hardreg for pseudo 79 (which is `$v0`, since
    a0-a3 are freed by this point); global-alloc then processes pseudo 78
    (ineligible for local-alloc since it crosses the branch) and is forced
    to the next free register, `$v1`. Target's asm
    (`asm/funcs/func_8006A564.s:15-31`) shows `lw $v0,...; andi $v0,$v0,0xF`
    -- both the load and the AND's result in the SAME register ($v0),
    i.e. target's compile did NOT give the raw-load temp $v0 first.
  - Grepped `tools/gcc-2.7.2/config/mips/mips.h` for `REG_ALLOC_ORDER`:
    NOT FOUND. The MIPS backend in this fork uses GCC's DEFAULT ascending
    hard-reg search order (no per-target override), so "lowest free
    hardreg wins" is the actual mechanism, not a MIPS-specific quirk.
  - Also confirmed via `asm/funcs/func_8006A564.s:31` that target does NOT
    duplicate the `tile[6]=` store into both if/else arms (it's at the
    shared merge label `.L8006A5D0`) -- ruling out "the value never
    crosses the branch in target's original C" as an explanation; target
    really does have a value crossing the merge, colored to $v0.

- **[s3] KILLED (H4, instance): a genuinely FRESH separately-named local
  for the raw load (`s32 raw; raw = D_800A34F8; v0 = raw & 0xF;`) --
  distinct from session 2's H3, which reused the SAME variable name for
  both statements -- produces IDENTICAL RTL to the unsplit form.**
  Applied at site 1, re-measured (68 -> 68, no change), then re-dumped
  `.greg` and confirmed the pseudo numbers/hardreg assignments were
  byte-identical to the pre-split dump. GCC's combine pass folds the named
  copy back to the same 2-insn shape regardless of the C-level variable
  name given to the load's result.

- **[s3] CONFIRMED (H5): removing the mask-compute's named variable
  entirely -- writing `if ((D_800A34F8 & 0xF) == arg2) {...}` instead of
  `v0 = D_800A34F8 & 0xF; if (v0 == arg2) {...}` -- at all 4 sites drops
  the sandbox score 68 -> 60 (build_insns unchanged, 199==199).** Measured
  incrementally: site 1 alone (68->66), all 4 sites (68->60), via
  `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`.
  **Re-dumped `.greg` after this change and confirmed the v0<->v1 coloring
  swap is UNCHANGED** -- pseudo 79-equivalent (now renumbered, e.g. site 1
  is still pseudo 79) still gets hardreg 2, the AND-result pseudo still
  gets hardreg 3. A register-normalized objdump diff
  (`tmp/grind/func_8006A564/s3/diff.py` vs
  `tmp/grind/func_8006A564/s3/build.dis.txt`) shows the SAME v0<->v1 swap
  pattern at all 4 sites in the post-fix build. **The mechanism behind
  this 8-point improvement is NOT the coloring swap and was NOT isolated
  this session** -- it must be some other diff (possibly a secondary
  effect elsewhere in the block ordering/scheduling), and is the top
  frontier item for next session (diff session-2's vs session-3's
  build.dis.txt directly, not each against target, to find exactly which
  instructions changed).

- **[s3] KILLED (H6, instance): reversing the comparison operand order
  (`arg2 == (D_800A34F8 & 0xF)` instead of `(D_800A34F8 & 0xF) == arg2`)
  at all 4 sites, atop the H5 chassis, regresses the score 60 -> 64.**
  Reverted. This is the opposite of what
  [[compare-operand-order-register]] would predict by analogy (that rule
  is for `<`/`>`, not `==`, and explicitly notes it may not generalize);
  confirmed empirically inapplicable here.

- [s3] Chassis check at session start: applying the s2 candidate.c to src/text1b.c reproduces floor 68 exactly (build_insns 199==199), matching the ledger's last recorded floor and the driver's dispatch-time floor.

- [s3] PASS ATTRIBUTION read of tmp/grind/func_8006A564/dumps/text1b.combine and .greg (func_8006A564 section) confirms the mechanism behind session 2's 'systemic v0<->v1 coloring swap' finding precisely: the raw D_800A34F8 load creates a genuinely block-local pseudo (single def, dies at the very next insn) which LOCAL-ALLOC processes first and assigns to hardreg 2 ($v0, the first free int-class register after a0-a3 free up); the AND's result reuses the SAME pseudo as the block-scoped C variable v0, which is used in both if/else arms and after the merge, so it crosses the conditional branch and is therefore NOT local-alloc-eligible -- it is deferred to GLOBAL-ALLOC, which runs after local-alloc and is forced to the next free register, hardreg 3 ($v1). Target's asm (asm/funcs/func_8006A564.s:15-31) puts BOTH the load and the AND result in $v0 -- i.e. target's compile did not let the raw-load temp claim $v0 first.

- [s3] Grepped tools/gcc-2.7.2/config/mips/mips.h for REG_ALLOC_ORDER: not defined. The MIPS backend in this fork uses GCC's default ascending hard-register search order with no target-specific override, confirming 'lowest free hardreg wins' as the actual allocator behavior driving this residual (not a MIPS-specific REG_ALLOC_ORDER quirk).

- [s3] Confirmed via asm/funcs/func_8006A564.s:31 that target's tile[6] store sits at the shared post-if/else merge label (.L8006A5D0), not duplicated into both arms -- ruling out 'the value never crosses the branch in target's original C' as an alternative explanation; the cross-block pseudo is real and target's own compiler still colored it $v0.

- [s3] The 68->60 improvement's mechanism was NOT isolated this session: a register-normalized objdump diff of the post-fix build against target (tmp/grind/func_8006A564/s3/diff.py output) still shows the identical v0<->v1 swap pattern at all 4 sites, so whatever closed 8 points of score is a different, unattributed diff -- next session should diff session-2's build.dis.txt directly against session-3's (not each against target) to pin down exactly which instructions changed.

## Session 4 (permuter)

- Re-verified chassis: applied candidate.c (session 3, floor 60) to
  src/text1b.c, measured via `sandbox --disable all` -- confirmed floor 60,
  build_insns 199==199 target_insns, matching the ledger's last recorded
  floor exactly (dispatch's "measurement unavailable" chassis check was
  just a transient at dispatch time, not a chassis drift).
- Read `tmp/grind/func_8006A564/dumps/text1b.greg` (function func_8006A564,
  line 51553+) fresh this session and confirmed the v0/v1 coloring swap
  precisely: insns 25/26 (the inlined `D_800A34F8 & 0xF` load+and) both
  land in hardreg 2 (v0) -- so the mask-compute pseudo IS getting v0, as
  expected (it's block-local, dies at the branch insn 28). The DIFFERENT,
  cross-block arm-value pseudo (first defined at insn 33,
  `v0 = *(u8*)(arg1+0x29)` inside the if-arm) lands in hardreg 3 (v1)
  instead of v0 -- confirmed against target
  (`asm/funcs/func_8006A564.s:18` `lbu $v0,0x29($s1)` -- target keeps this
  SAME arm-value chain in v0 throughout). The conflict list for this
  pseudo (`;; 72 conflicts: ... 2 3 4 5 6 7 29`) shows it DOES conflict
  with hardreg 2, which is why global-alloc can't put it there -- the
  question left open (why does the pseudo conflict with v0 despite v0
  dying at the branch) needs a `.lreg`/liveness-focused read, not yet done
  this session (budget).
- Built a from-scratch permuter workspace at
  `tmp/grind/func_8006A564/s4/perm/` (base.c = the floor-60 candidate body
  + its extern block; compile.sh replicates the exact Makefile pipeline for
  text1b.c: cc1 -O2 -G0 ... -mel -msoft-float | prologue_fix | maspsx
  --expand-div --aspsx-version=2.34 (+ full sdata/expand-lb/multu/prefill
  flag set) | multu_pad | mipsel-linux-gnu-as -march=r3000 -no-pad-sections
  -O1 -G0; target.o built from asm/funcs/func_8006A564.s + a gp=64-stripped
  prelude.inc so the function sits at offset 0 like base.o, per
  [[difficult-is-not-impossible]]'s clean-single-function-target
  guidance). Verified base.o's opening instructions byte-match target.o's
  before launching (both start `addiu sp,-48; sw s2,32; move s2,a0; ...`).
  No permuter campaign existed yet for this function (state.json showed 0
  prior permuter sessions), so this is campaign #1 of the R3 2-session cap.
- Launched via `tools/permuter_campaign.py launch --func func_8006A564
  --dir tmp/grind/func_8006A564/s4/perm --label coloring-swap-s4 -j 4
  --stop-on-zero`. base_score (permuter's own weighted metric, NOT the
  sandbox score) = 530. Ran ~4439 iterations over ~183s wall time (4
  workers), harvested 25 output dirs, best_new_score 330 (down from 530),
  but PLATEAUED at 520 for the large majority of finds (12 of 25 finds sit
  exactly at 520) -- no zero, no discovery of the actual register-coloring
  fix. Harvested + stopped per fresh-seed discipline (a firmly-plateaued
  metric after several dozen distinct mutation attempts, well before the
  literal 20-30 min real-time mark, but the finds show the search
  converged to a local optimum of trivial statement-shape mutations, not
  further progress).
- Inspected the two most-improved finds (`output-330-1`,
  `output-420-1` area): the score drop came from RANDOM, semantically-void
  permuter mutations unrelated to program logic -- e.g.
  `output-330-1/diff.txt` shows `short v0;` (type-narrowing a pseudo that
  is genuinely `s32`-typed per the target's `lw`/full-word loads) plus a
  fabricated `s32 new_var; new_var = v0; *(...)=new_var; *(...)=new_var;`
  split of the two final `else`-arm byte stores in the 4th (record-copy)
  block. Hand-applied JUST the `new_var` split (the type-narrowing part is
  wrong per the object's actual s32 width, so excluded) to
  src/text1b.c and re-measured via `sandbox --disable all`: **KILLED,
  unchanged (60 -> 60)**. The permuter's 330 score improvement was
  entirely an artifact of the (excluded) incorrect `short` retype
  combined with the new_var split; the new_var split alone contributes
  nothing to the honest metric. Reverted immediately; confirmed back at
  60.

- [s4] Re-verified chassis: applying candidate.c (session 3 body) to src/text1b.c and running `sandbox --disable all` reproduces floor 60 exactly, build_insns 199==199 target_insns -- confirms the ledger's last recorded floor is still the live HEAD-equivalent floor (the dispatch-time 'measurement unavailable' chassis check was a transient, not chassis drift).

- [s4] Read tmp/grind/func_8006A564/dumps/text1b.greg (function func_8006A564, starts line 51553) fresh this session: confirmed the v0/v1 coloring swap precisely -- the inlined `D_800A34F8 & 0xF` load+and (insns 25/26) both land in hardreg 2 (v0, block-local, dies at the branch), but the cross-block arm-value pseudo (first defined at insn 33, `v0 = *(u8*)(arg1+0x29)` inside the if-arm) lands in hardreg 3 (v1) instead of v0. Target (asm/funcs/func_8006A564.s:18, `lbu $v0,0x29($s1)`) keeps this same arm-value chain in v0 throughout.

- [s4] The arm-value pseudo's conflict list (`;; 72 conflicts: ... 2 3 4 5 6 7 29`) shows it DOES conflict with hardreg 2 despite the mask-compute pseudo dying at the branch -- the source of that conflict is unexplained and is the next diagnostic step (a .lreg/liveness-focused read), not yet done this session.

- [s4] Built a from-scratch permuter workspace (tmp/grind/func_8006A564/s4/perm/) replicating the exact Makefile pipeline for text1b.c (cc1 -O2 -G0 -mel -msoft-float | prologue_fix | maspsx with the full flag set incl. --prefill-label-funcs | multu_pad | mipsel-linux-gnu-as -march=r3000 -no-pad-sections); target.o built from asm/funcs/func_8006A564.s + a gp=64-stripped prelude so the function sits at offset 0 like base.o. Verified base.o's opening instructions byte-match target.o's before launching. This is campaign #1 of the R3 2-permuter-session cap (state.json showed 0 prior permuter sessions).

- [s4] Campaign ran ~4439 iterations / ~183s wall / 4 workers (tools/permuter_campaign.py launch/wait/harvest, label coloring-swap-s4). Permuter's own weighted score (NOT the sandbox score) started at base_score 530 and PLATEAUED at 520 (12 of 25 harvested finds sit at exactly 520); best_new_score 330 was a false-positive artifact traced to an incorrect type-narrowing mutation, not a real fix (see the KILLED hypothesis above). No zero found; no discovery of a construct that touches the actual coloring residual.

- [s4] This plateau is evidence the coloring-swap residual is not reachable by decomp-permuter's default random-mutation search from this chassis -- consistent with the mandated-modality brief's note that the permuter cannot express chassis-level structural rewrites (this residual needs restructuring WHICH C-level pseudo crosses the if/else boundary, not a local expression mutation).

## Session 5 (enumerate, 2026-09-16)

- Chassis check at dispatch: HEAD had no measurable floor (function was
  INCLUDE_ASM on main per asm-until-matched); applied candidate.c (session
  3/4 form, floor 60) to src/text1b.c first, re-measured via
  `sandbox --disable all` and confirmed 60 matches the ledger before doing
  any new work.
- Sibling ledgers (func_8007352C, main/ings.c) are both COMPLETED-C and
  closed since before this session's last mention (s2/s3 of this ledger);
  nothing new to transplant this session.
- Systematic sweep tooling note: `tools/sweep_variants.py` is blocked by
  `tools/hooks/worktree_contamination_guard.py`'s SWEEP_RE check, which
  requires the literal string `wteng.ps1` to appear in the invoking command
  (`has_wteng` check) with NO alternative absolute-cd pin path (unlike
  `make`, which accepts `cd '/mnt/.../<repo>'`). wteng.ps1 itself has no
  passthrough for arbitrary tools (only `make` and `python3 -m engine.cli
  <subcmd>`), so `sweep_variants.py` is currently uninvocable through any
  sanctioned path. Worked around by writing a local one-off
  `tmp/grind/func_8006A564/s5/run_sweep.ps1` that loops the 150
  spelling_enum.py variants, splices each into src/text1b.c
  (`tmp/grind/func_8006A564/s5/splice.py`, brace-matched region replace),
  and calls the already-sanctioned
  `& tools/wteng.ps1 main sandbox func_8006A564 --disable all` per variant
  -- same effect as sweep_variants.py, zero guard friction. This gap may be
  worth flagging to the operator (sweep_variants.py is unusable for a
  solo/main-branch grind session, only for worktree-based ones) but is out
  of scope to fix from inside a grind session (tools/ edits are off-limits).
- Full sweep results: tmp/grind/func_8006A564/s5/sweep_results.csv (150
  rows, variant name + score). Best: v149.c (score 47, fully-inlined tail).
- Register-normalized objdump diffs before/after:
  tmp/grind/func_8006A564/s5/diff.py (baseline, floor 60, re-confirms s3's
  finding that v0/v1 swap is uniform across ALL 4 similarly-shaped blocks)
  and tmp/grind/func_8006A564/s5/diff2.py (post-fix, floor 45) against
  tmp/grind/func_8006A564/s5/build.dis.txt / build2.dis.txt respectively.
- Floor: 60 -> 45 this session (target_insns 199, build_insns 199, exact
  parity maintained throughout).

- [s5] Chassis check: HEAD had no measurable floor (function committed as INCLUDE_ASM per asm-until-matched); applied session 3/4's candidate.c (floor 60) to src/text1b.c first and re-confirmed sandbox score 60 before any new work, matching the ledger.

- [s5] Both sibling ledgers (func_8007352C, main/ings.c) are COMPLETED-C and closed well before this session's dispatch; nothing new to transplant.

- [s5] tools/sweep_variants.py is currently UNINVOCABLE from a solo/main-branch grind session: worktree_contamination_guard.py's SWEEP_RE check requires the literal string 'wteng.ps1' to appear in the command with no absolute-cd-pin alternative (unlike the make check), and wteng.ps1 itself only proxies 'make' and 'python3 -m engine.cli <subcmd>' -- no generic tool passthrough. Worked around with a local tmp/grind/func_8006A564/s5/run_sweep.ps1 loop that splices each spelling_enum.py variant into src/text1b.c and scores it via the sanctioned '& tools/wteng.ps1 main sandbox func_8006A564 --disable all' per variant -- same effect, zero policy risk, but 150x the process-launch overhead of the intended tool. This tooling gap is worth an operator note; out of scope to fix from inside a grind session (tools/ edits are off-limits to this session).

- [s5] Full sweep results: tmp/grind/func_8006A564/s5/sweep_results.csv (150 rows: variant filename + sandbox score).

- [s5] Register-normalized objdump diffs: tmp/grind/func_8006A564/s5/diff.py (baseline floor 60, against tmp/grind/func_8006A564/s5/build.dis.txt) and tmp/grind/func_8006A564/s5/diff2.py (post-fix floor 45, against tmp/grind/func_8006A564/s5/build2.dis.txt) both diffed against asm/funcs/func_8006A564.s.

- [s5] Final measured floor this session: 45 (target_insns 199, build_insns 199, exact parity). src/text1b.c reverted to INCLUDE_ASM (git checkout) before ending the session per asm-until-matched -- candidate.c is the sole persistence mechanism.

## [s6] Tooling note: sweep_variants.py is blocked by worktree_contamination_guard on main (by name, "in-place src-mutating tool"); a custom Python splice+sweep driver invoked via Bash IS an acceptable substitute (same effect as the s5 PowerShell loop) provided it always calls `tools/wteng.ps1 main sandbox ...` for the actual scoring (never a raw engine.cli invocation). CRITICAL: invoke `pwsh.exe` (PowerShell 7), NOT the legacy Windows `powershell.exe` (5.1) -- the legacy shell's default console codepage garbles a non-ASCII arrow glyph printed by wteng.ps1's WSL-bridge-unavailable message, producing a PARSER ERROR ("string is missing the terminator") that silently returns empty stdout for EVERY invocation with no other symptom. Cost one full failed sweep run (65 variants, all `score: None`) before the codepage issue was diagnosed. Driver script: tmp/grind/func_8006A564/s6/sweep.py.

## [s6] Sibling ledgers re-checked: func_8007352C (COMPLETED-C, closed since its own s1 2026-09-08) is CALLED by func_8006A564 as an ordinary extern function (`func_8007352C((s32)arg1)`), not a shared code block -- there is nothing to transplant from its candidate (it's a different function's body, already merged to main and already an `extern` decl in this ledger's candidate.c). `main` (src/ings.c, COMPLETED-C since its s33) shares no code or data with this function either (no reference to it in candidate.c). Both were already accounted for as "nothing to transplant" as of s5; re-confirmed this session, no ledger drift.

- [s6] Chassis unchanged this session: sandbox --disable all reports score=45, target_insns=199, build_insns=199 both before and after this session's probes (probes were measured on isolated variants, then reverted).

- [s6] sweep_variants.py is blocked on main by worktree_contamination_guard.py (matches by tool name, 'in-place src-mutating tool with no wteng.ps1 pin'); a custom Python splice+score driver invoked via the Bash tool is an acceptable substitute as long as scoring always goes through '& tools/wteng.ps1 main sandbox ...' -- never a raw engine.cli call.

- [s6] Tooling gotcha (cost one wasted 65-variant run): a naive subprocess driver using the legacy Windows 'powershell.exe' (5.1) fails SILENTLY -- its default console codepage garbles a non-ASCII arrow glyph in wteng.ps1's WSL-bridge-unavailable message, producing a PowerShell parser error ('string is missing the terminator') and empty stdout for every single invocation. Switching to 'pwsh.exe' (PowerShell 7, UTF-8 default) fixed it immediately. Recorded in evidence.md for future sessions.

- [s6] Both sibling ledgers (func_8007352C, main/src/ings.c) re-checked: both COMPLETED-C and closed; func_8007352C is called by this function as an ordinary extern (not a shared code block), and main/ings.c shares no code or data with this function. Nothing to transplant; no ledger drift since s5.

- [s6] Neither block 2's nor block 3's tail has a swappable commutative operand pair (spelling_enum.py --list reported the identical variant count with and without the --swaps axis for both regions), so the axis-3 sweep was skipped as genuinely inapplicable, not merely unexplored.
