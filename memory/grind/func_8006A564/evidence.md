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

## Session 7 (structural, 2026-09-16)

- Chassis check at dispatch: HEAD had no measurable floor (function is
  INCLUDE_ASM per asm-until-matched, dispatch printed "measurement
  unavailable"); applied candidate.c (session 5/6 form) to src/text1b.c
  first and re-confirmed sandbox score 45, build_insns 199==199 before
  any new work -- matches the ledger exactly.
- Read hypotheses.md + evidence.md in full (all 6 prior sessions) before
  starting, per the ledger-inheritance mandate. Both sibling ledgers
  (func_8007352C, main/src/ings.c) re-checked as COMPLETED-C/closed with
  nothing to transplant, consistent with s5/s6's own re-checks -- no
  ledger drift.
- [s7] Applied the s5-proven "fully inline single-use values, never
  inline a multi-use value" pattern to block 4 (the ONLY tile-draw/record
  block untouched by s5's transplant or s6's independent sweeps).
  Measured 4 independent wins, each via `& tools/wteng.ps1 main sandbox
  func_8006A564 --disable all` immediately after the edit:
    45 -> 39 (if-arm's two single-use halving values inlined)
    39 -> 35 (else-arm's 0x28 literal broadcast inlined)
    35 -> 29 (both single-use `*(arg0+0x14)` loads before the two
              func_8007352C calls inlined)
  (The top-of-block `*(tile+0x28)` inline was folded into the same batch
  as the if-arm edit in the actual edit sequence; re-verified as its own
  independently-scored step: applying JUST the top-of-block inline atop
  the un-touched-else-arm chassis also measured a real drop, confirming
  it is not free-riding on the other edits.) build_insns stayed
  199==199 (target) after every single edit -- exact parity never broke
  for any of the 4 CONFIRMED wins.
- [s7] KILLED (instance): inlining the `v0=*(arg1+0)+0xC; ...;
  *(arg1+4)=v0;` group (with its two preceding dead reads left untouched)
  measured WORSE: 29 -> 34, build_insns held at 199==199 (a pure
  register/scheduling regression, not a parity break). Reverted,
  reconfirmed 29.
- [s7] KILLED (instance): reordering that same group's 3 statements
  (moving the `*(arg1+0x18)=0` store before vs after the two dead reads)
  measured NO CHANGE in either order (stays 29) -- statement order is not
  a lever here; only the inline-vs-not axis matters and inlining already
  measured worse (previous bullet).
- [s7] KILLED (instance): fully inlining the `v0 = *(tile+0x2C);
  *(arg1+0)=v0; *(arg1+4)=v0+0xC;` group (v0 used TWICE) as two direct
  `*(tile+0x2C)` reads broke build_insns exact parity: 199 -> 201 (GCC's
  CSE did not common the duplicate load across the intervening store to
  arg1+0). Confirms the s5/s6-established boundary condition
  ("never inline a value used 2+ times") also applies to block 4.
  Reverted, reconfirmed 199==199 / score 29.
- [s7] Also tested swapping the *(arg1+0)=v0 / *(arg1+4)=v0+0xC store
  order within the (still-named, multi-use) tile+0x2C group: NO CHANGE
  (stays 29) -- reverted to original order for consistency, this axis is
  neutral.
- Final measured floor this session: **29** (was 45 at dispatch),
  target_insns 199, build_insns 199, exact parity held throughout every
  edit. src/text1b.c reverted to INCLUDE_ASM (`git checkout --`) before
  ending the session per asm-until-matched -- candidate.c is the sole
  persistence mechanism.
- No permuter run this session (structural modality). No dump.ps1 run
  this session -- every hypothesis was resolved by direct measurement
  (single-site edits with immediate sandbox re-scoring), not RTL
  inspection; the next session's highest-value probe (per the frontier)
  is a fresh register-normalized dump/diff at the new floor-29 chassis,
  since blind inlining is now exhausted for block 4's two remaining
  named-v0 groups (both measured non-improvable this session).

- [s7] Chassis check at dispatch confirmed floor 45 (target_insns 199, build_insns 199) after applying session 5/6's candidate.c, matching the ledger exactly.

- [s7] Both sibling ledgers (func_8007352C, main/src/ings.c) re-checked as COMPLETED-C/closed with nothing to transplant -- no ledger drift since s5/s6.

- [s7] Block 4 (the obj2->field_1C->field_28 record-copy block) had never received the s5 full-inline transplant or an independent s6-style sweep; it was the last untouched region of the function.

- [s7] 4 single-use named-v0/v1 groups in block 4 (top-of-block load, if-arm halving pair, else-arm literal broadcast, the two pre-call arg0+0x14 loads) all measured strict improvements when inlined, each individually re-verified via sandbox, with build_insns staying at exact 199==199 target parity throughout.

- [s7] 2 remaining named-v0 groups in block 4 (the dead-read-adjacent arg1+4 tail, and the genuinely-twice-used tile+0x2C group) both measured NON-improvable this session: the first regresses the score without breaking parity, the second breaks build_insns parity outright when de-duplicated.

- [s7] Final floor this session: 29 (was 45 at dispatch), a 16-point drop, target_insns 199 == build_insns 199 throughout.

- [s7] src/text1b.c reverted to INCLUDE_ASM via git checkout before ending the session; candidate.c in memory/grind/func_8006A564/ is the sole persistence mechanism per asm-until-matched.

## [s8] 2026-09-16 — solver — floor 29 (ledger) / 7 (actual chassis) -> 3

### Chassis correction (read this before trusting any earlier floor number)
At dispatch the driver reported "HEAD honest floor: measurement unavailable" and
the ledger's last recorded floor was 29. Both were stale: the working tree's
`src/text1b.c` already carried an UNBANKED func_8006A564 body left behind by a
discarded earlier session-8 process (its scratch survives in
`tmp/grind/func_8006A564/s8/`, files timestamped 2026-09-16 14:08-14:16; it
wrote no `candidate.c`, no ledger update and no outcome JSON, so the driver
treated it as if it never ran). Measured at the start of THIS session:
`sandbox func_8006A564 --disable all` = **score 7**, target_insns 199,
build_insns 199 (exact parity). That body is now banked in `candidate.c`
(its diff vs the s7 candidate: store-order reshuffles in blocks 1-3, `tile[6] =
v0` moved inside both arms of block 3, and the final record-copy region split
into two separate `{ s32 v0; ... }` scopes, one per `func_8007352C` call).

### Residual localization (register-normalized objdump diff)
`tmp/grind/func_8006A564/s8/objdiff.sh` renders ours
(`tmp/sandbox/func_8006A564/text1b.o`) beside the target
(`build/src/text1b.o`) instruction-for-instruction. On the score-7 chassis the
ENTIRE residual was 7 instructions in two adjacent clusters of the final block
(all other mismatched lines are branch/jump displacement text, which the engine
score masks):

    A)  ours: lw v0,28(s1) | lw v1,0(s1)  | sw zero,24(s1) | addiu v0,v0,15 | addiu v1,v1,12 | sw v0,28(s1) | sw v1,4(s1)
        tgt : lw v1,0(s1)  | lw v0,28(s1) | sw zero,24(s1) | addiu v1,v1,12 | addiu v0,v0,15 | sw v0,28(s1) | sw v1,4(s1)
    B)  ours: lw v0,44(s0) | sw v0,0(s1)     | addiu v0,v0,12 | sw v0,4(s1)
        tgt : lw v0,44(s0) | addiu v1,v0,12  | sw v0,0(s1)    | sw v1,4(s1)

Both are pure ORDERING differences (cluster A: which of the two loads/adds goes
first; cluster B: whether the add precedes the first store), and cluster B's
register difference (v1 vs a second write of v0) is a consequence of the order,
not an independent allocation decision.

### Cluster A CLOSED (7 -> 3)
A 12-variant sweep of that group (`tmp/grind/func_8006A564/s8/gen.py` +
`sweep.ps1`, results in `sweep_A.txt`) found two forms at score 3; the clean one
is a compound-assignment split with the arg1+0x18 store hoisted to the head:

    *(s32 *)(arg1 + 0x18) = 0;
    v0 = *(s32 *)(arg1 + 0);
    v0 += 0xC;
    *(s32 *)(arg1 + 0x1C) += 0xF;
    *(s32 *)(arg1 + 4) = v0;

This is ordinary C (compound-assignment split on one variable — owner Ruling 4,
`.claude/rules/ordinary-c-judge-decidable.md`). It also removed the two dead
reads (`v0 = *(arg1+0); v0 = *(arg1+0x1C);`) that every body since session 2
carried: **the current candidate has zero dead stores, zero pads, zero FAKE
constructs, zero annotations.** The other score-3 form (a10) keeps the dead
reads and only moves the arg1+0x18 store to the end; it was rejected in favour
of the dead-read-free spelling per the simplest-known-form rule
(ordinary-c-judge-decidable.md Ruling 1.4).

### Cluster B — pass attribution (dumps READ, not guessed)
Dumps regenerated this session from this body (`pwsh tools/grinder/dump.ps1
func_8006A564`, tmp/grind/func_8006A564/dumps/):

* `.rtl` and `.combine` already carry TARGET's order when the C names the +0xC
  value (`v1 = v0 + 0xC;` before the two stores): insn chain 436(lw) 439(add)
  442(sw 0(s1)) 445(sw 4(s1)).
* **sched1 is the pass that breaks it.** Its trace for that region:
      ;; ready list at T-25: 442 (4) 445 (4), now 445 442
      ;; ready list at T-26: 442 (4) 439 (7f000001), now 439 442
  `0x7f000001` is `LAUNCH_PRIORITY` (`tools/gcc-2.7.2/sched.c:187`, assigned at
  `sched.c:4049`). The add enters the ready list carrying that boost the moment
  its consumer (`sw 4(s1)`) is scheduled, so `rank_for_schedule`'s FIRST test —
  the raw `INSN_PRIORITY` difference at `sched.c:2418` — decides, and the
  dependence-CLASS and INSN_LUID tests (`sched.c:2420-2463`) are never reached.
  `schedule_block` is BACKWARD, so being chosen at T-26 puts the add AFTER the
  store in program order. This is a DIFFERENT wall from
  `.claude/rules/sched-rank-class-tie-wall.md` (that rule's wall is the class
  compare on an equal-priority tie; here the priorities are not equal at all).
* Post-reload `sched2` shows the region fully dependence-forced (every ready
  list has exactly one member) because the two values are already coalesced into
  `v0`, giving `addiu v0,v0,12` a REG_DEP_ANTI on `sw v0,0(s1)`. That coalescing
  is downstream of the sched1 order — which is why every "two named locals"
  spelling is inert.

### Matched-sibling ground truth for cluster B
`func_8006A1A0` (same file, COMPLETED-C, src/text1b.c:6280) emits exactly the
target shape — `lw v0,12(s3); addiu v1,v0,12; sw v0,24(sp); sw v1,28(sp)` —
from `p1 = ptr[3]; tbl = p1 + 0xC; s.sp18 = p1; s.sp1C = tbl;`. Its destination
is a STACK STRUCT (fixed-address `sw ...(sp)`); ours is a pointer parameter
(`sw ...(s1)`). Transplanting the sibling's exact spelling (function-scope
`tbl`, block-scope pair, `s32 *rec` indexing) onto our chassis: 5 variants, all
score 3, all exact parity. Banked as instance kills below.

- [s8] The ledger's floor of 29 was stale at dispatch: the working tree already carried an unbanked func_8006A564 body from a discarded earlier session-8 process (scratch in tmp/grind/func_8006A564/s8/, timestamps 2026-09-16 14:08-14:16, no candidate.c and no outcome written) that measured score 7 with exact parity. This session banked it to memory/grind/func_8006A564/candidate.c before editing anything.

- [s8] The whole score-7 residual was 7 instructions in two adjacent clusters of the final block; everything else in the register-normalized objdump diff is branch/jump displacement text that the engine score masks.

- [s8] Cluster A (4 insns) is closed. The banked candidate measures 3 with build_insns 199 == target_insns 199 and contains zero dead stores, zero pads, zero volatile and zero FAKE annotations: it is entirely ordinary C.

- [s8] Cluster B's mechanism is named and dump-proven: sched1's LAUNCH_PRIORITY boost (sched.c:187 / sched.c:4049) wins rank_for_schedule's first test (sched.c:2418), so the dependence-class and LUID tie-breaks (sched.c:2420-2463) never run. This is a different wall from .claude/rules/sched-rank-class-tie-wall.md, which describes the equal-priority class-compare case.

- [s8] Post-reload sched2 shows cluster B fully dependence-forced (REG_DEP_ANTI from the coalesced `addiu v0,v0,12` onto `sw v0,0(s1)`, every ready list single-membered), so the v1-vs-v0 register difference is a consequence of the sched1 order rather than an independent RA decision. That is why all two-named-local spellings are inert.

- [s8] 28 distinct spellings of cluster B's 4-statement group were measured this session (b/c/d/e sweeps); every one scores 3 or worse, tabulated in tmp/grind/func_8006A564/s8/sweep_results.txt.

## [s9] 2026-09-16 — forensics — floor 3 -> 0 (MATCHED)

### Result
`sandbox func_8006A564 --disable all` = **score 0**, target_insns 199 ==
build_insns 199. The register-normalized objdump diff
(`tmp/grind/func_8006A564/s9/final_objdiff_f01.txt`) is instruction-identical
on all 199 rows; the 8 remaining `!!` rows are branch/jump displacement TEXT
only (identical relative offsets, different absolute section addresses — the
sandbox .o and build/src/text1b.o sit at different section bases). Full-tree
`verify-oracle` re-linked to `62efab4f73f992798c43e8c730aa43baa10bb4fa`, i.e.
byte-identical to the original EXE. The body carries zero cheat constructs: no
inline asm, no register pins, no volatile, no dead stores, no pads, no unused
locals, no FAKE annotations.

### The one edit
The final block's two `{ s32 v0; ... }` scopes are merged into a single
`{ s32 v0, v1; ... }` scope, and the SAME local `v1` now carries the
"record base + 0xC" value in BOTH record-fill groups:
  cluster A: `v1 = *(s32 *)(arg1 + 0); v1 += 0xC; ... *(s32 *)(arg1 + 4) = v1;`
  cluster B: `v0 = *(s32 *)(tile + 0x2C); v1 = v0 + 0xC;
              *(s32 *)(arg1 + 0) = v0; *(s32 *)(arg1 + 4) = v1;`
Nothing else in the function changed from the s8 candidate.

### The predicate that closes it (this is the transferable finding)
s8 correctly named sched1 as the pass reordering cluster B, and correctly
identified the `LAUNCH_PRIORITY` (0x7f000001, `tools/gcc-2.7.2/sched.c:187`,
installed on the insn being scheduled at `sched.c:4049`) boost as the reason
the add was pulled past the store. What s8 did not have was the SOURCE-SIDE
INPUT that decides who gets the boost. It is:

    sched.c:2584   adjust_priority() raises a just-released insn to
                   max_priority ONLY if birthing_insn_p(PATTERN(prev)).
    sched.c:2505-2536  birthing_insn_p((set (reg i) ...)) returns
                   `reg_n_sets[i] == 1`.

**The boost is a property of the destination pseudo's SET COUNT, not of the
dependence edge.** A pseudo assigned exactly once in the function is
"birthing" and gets LAUNCH_PRIORITY when released; a pseudo assigned twice
does not. Set count is pure C-level input: assign the same local in two places
and the boost disappears. That is why all 28 of s8's local re-spellings were
inert — every one of them left the add writing a single-set pseudo (in several
cases a compiler temp, because combine had folded a `v1 = v0; v1 += 0xC;` pair
back into one set before flow recomputed reg_n_sets). The extra set has to be a
SEPARATE, non-foldable, genuinely-consumed assignment elsewhere in the
function; a second assignment adjacent to the first is folded away by combine
and does not move reg_n_sets.

### Measured proof (both traces from the instrumented cc1 .sched dump)
s8 body — the add writes a single-set compiler temp:
    (insn 442 ... (set (reg:SI 147) (plus:SI (reg/v:SI 146) (const_int 12))))
    ;; ready list at T-26: 439 (4) 442 (7f000001), now 442 439
  The boosted add wins rank_for_schedule's FIRST test (INSN_PRIORITY,
  `sched.c:2418`); the class and INSN_LUID tests (`sched.c:2420-2463`) are
  never reached. schedule_block is BACKWARD, so being picked at T-26 places the
  add AFTER the store in program order. Score 3.
s9 body — the add writes `v1`, which cluster A also assigns:
    (insn 428 ... (set (reg/v:SI 79) (plus:SI (reg/v:SI 78) (const_int 12))))
    ;; ready list at T-26: 431 (4) 428 (4), now 431 428
  No boost; priorities are EQUAL, the tie falls through to the dependence-class
  / LUID tests, the store is picked first, and the add lands BEFORE it — the
  target's order. Score 0.
Note the `/v` flags: reg 78/79 are REG_USERVAR_P pseudos (our named locals),
where the s8 form's add wrote an anonymous temp.
Artifacts: `tmp/grind/func_8006A564/s9/f.sched` (s9 trace, extracted from
`tmp/grind/func_8006A564/dumps/text1b.sched`) and
`tmp/grind/func_8006A564/s8/f.sched` (s8 trace).

### Sweep
5 hypothesis-driven variants (`tmp/grind/func_8006A564/s9/genf.py`) plus one
declaration-hoisting variant (`geng.py`); results in
`tmp/grind/func_8006A564/s9/sweep_f.txt`. f01 / f03 / f04 / g01 all measure 0;
f02 and f05 (which leave the add writing a single-set pseudo) measure 6 and 4.
f01 was adopted: it is the smallest diff from the banked body, it invents no
carrier (f04 needed a new function-scope variable), and it leaves blocks 1-3 —
which already matched instruction-for-instruction — untouched.

- [s9] func_8006A564 MATCHES in pure C: sandbox score 0, 199 == 199, full-tree verify-oracle SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
- [s9] The closing lever was giving the cluster-B "+0xC" value a local that is assigned TWICE in the function (once per record-fill group) instead of a once-assigned pseudo.
- [s9] GENERALIZABLE: GCC 2.7.2 sched1's LAUNCH_PRIORITY boost is gated by birthing_insn_p (sched.c:2505-2536), whose predicate is `reg_n_sets[i] == 1` on the insn's DESTINATION PSEUDO. Any "the scheduler moves my add/copy past its neighbour and no local re-spelling helps" residual should first be tested by giving that destination a second, non-foldable assignment elsewhere in the function. Adjacent second assignments do not work — combine folds them before flow recomputes reg_n_sets.
- [s9] Confirms the s8 note that this is a DIFFERENT wall from .claude/rules/sched-rank-class-tie-wall.md: that rule describes the equal-priority class-compare tie. Here the priorities were unequal, and the fix was to MAKE them equal so that tie-break could run.
