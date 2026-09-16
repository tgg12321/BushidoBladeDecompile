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
