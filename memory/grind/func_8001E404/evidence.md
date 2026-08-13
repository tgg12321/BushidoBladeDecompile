# EVIDENCE — func_8001E404 (src/code6cac.c:1384)

## Session 1 (recon, 2026-08-12)

### Baseline
- `canonical func_8001E404` → verdict **C**, 0 asm insns, 184 target insns,
  pure-C distance 23 ("pure-C target").
- `sandbox func_8001E404 --disable all` → **score 23**, target_insns 184,
  build_insns 183, 0 rules dropped, 139 cheat-asm insns stripped file-wide.
- 0 regfix rules, 0 asmfix rules for this function.

### The residual is ONE cause, measured — not 23 independent diffs
Normalized instruction diff (`tmp/grind/func_8001E404/s1/diff.py`, artifacts
`diff_sandbox_vs_target.txt` / `diff_realbuild_vs_target.txt`):

- **Against the REAL build object (`build/src/code6cac.o`, i.e. with the committed
  `s32 pre_pad[2];` in place): 184 vs 184 instructions, ZERO real differences.**
  (The 4 residual lines the script prints at target[41/48/54/93] are a normalizer
  artifact — `addiu $s2,$s2,%lo(SYM)` vs objdump's unrelocated `addiu s2,s2,0`.)
  So the function **byte-matches target today** and is queued INCOMPLETE purely
  because its match is held up by a forbidden construct.
- **Against the cheat-stripped sandbox object (pre_pad removed): 183 vs 184.**
  Every real difference is downstream of ONE fact — the frame is 8 bytes short:
  - `addiu $sp,$sp,-0x70` → `-0x68`; all four callee-save slots shift −8
    (0x6C/0x68/0x64/0x60 → 0x64/0x60/0x5C/0x58) in both prologue and epilogue;
  - the camera work buffer base moves `sp+0x18` → `sp+0x10`, so every buffer
    store/load offset shifts −8 (`sw 0x18/0x1C/0x20($sp)`, `sh 0x28/0x2A/0x2C($sp)`,
    `sw 0x30($sp)`, `addiu $a3,$sp,0x18`, `lw $a2,0x30($sp)`);
  - knock-on addressing choice: target materializes the buffer address ONCE
    (`addiu $s0,$sp,0x18` + `move $a0,$s0` at both consumers, 184 insns); the
    8-bytes-short build re-derives it inline (`addiu $a0,$sp,0x10` twice, 183 insns).
    This is a consequence of the frame geometry, not an independent lever — it
    disappears the moment the 8 bytes are present (real-build diff = 0).

### The 8 bytes are an ALLOCATED-BUT-NEVER-TOUCHED leading frame region
Lowest `$sp` offset any target instruction touches is 0x18. `sp+0x10..0x17` is
inside the frame and is **never read or written by any instruction in the target**.
Binary-wide census (`tools/…` = `tmp/grind/func_8001E6E4/s7/hole_census.py`,
re-run this session, output `tmp/grind/func_8001E404/s1/hole_census.txt`):

```
 hole func                  frame   lowest   saves@  in-src?
  512 sprintf                 584    0x210    0x218  (PsyQ varargs, not in src)
   16 func_8003CF84            72     0x20     0x38  code6cac_c2.c
    8 func_8001E6E4           112     0x18     0x60  code6cac.c
    8 func_8001E404           112     0x18     0x60  code6cac.c   <-- this function
```
Exactly four functions in 1434 have the shape; three are the known family.

### This function IS the sibling family member named in the 2026-08-11 escalation
`docs/grind/decisions.md:4640-4649` (the func_8001E6E4 OWNER-ESCALATION —
RESOLVED BY STANDING RULING entry) names `func_8001E404` explicitly: "same file,
same 0x70 frame, same 8-byte hole, same committed `pre_pad` construct — every s4-s7
instrument transfers unchanged … When either reaches the queue top, one
measurement … confirms it is the same single-phantom-region defect before any
spelling work is spent, and it should take this same disposition rather than
re-grinding six modalities."

Both prescribed confirmations were run THIS session and both are positive:
the hole-census row above, and the frame gradient (honest form = 0x68/vars base
0x10; committed form = 0x70/vars base 0x18, byte-identical to target).

### Inherited (do NOT re-derive) — the sibling's white-box frame partition
From `memory/grind/func_8001E6E4/{evidence,hypotheses}.md` + decisions.md:4578-4609,
8 sessions / 6 modalities / ~100k permuter iterations / 19 measured spellings:
- **vars, declaration order:** `assign_stack_local` (tools/gcc-2.7.2/function.c:669-742)
  does `frame_offset = CEIL_ROUND(frame_offset, alignment)` from `frame_offset = 0`
  with `FRAME_GROWS_DOWNWARD` undefined on MIPS ⇒ the FIRST declared slot always
  lands at vars offset 0 for every type/size/alignment. Nothing can pad beneath it.
- **pre-declaration window:** provably empty on o32 (`assign_parms` keeps `stack_parm`
  non-null because `REG_PARM_STACK_SPACE` is unconditionally 16, mips.h:1822; the only
  pre-body allocator is the nested-function static-chain slot).
- **args partition:** `args=24 / vars=72` reproduces the geometry, but every route to
  `args_size > 16` (5th scalar arg, 8-byte-aligned arg past `$a3`, struct-return hidden
  pointer) materializes a store into `sp+0x10..0x17` — and the target stores nothing there.
- **expansion-time objects:** temps/spills/inner-scope objects grow the frame at the TOP
  (measured `put_reg_into_stack` 72→74→76), never below the first declared slot.
- **pretend_args_size:** mips.c:4531 guarded by `ABI_64BIT && mips_isa >= 3` ⇒ 0 on o32.
⇒ the 8 bytes can ONLY come from an object declared before the work buffer, i.e. an
unwritten leading dead array/struct lead — the forbidden family.

### Endgame gates re-measured for THIS function
- **Gate 1 (canonical asm): FAIL.** `python3 tools/scan_hand_coded.py --single
  func_8001E404` → `tier=LOW score=1/8`, "no strong hand-coded indicators"; only S4
  (front loads) fires. S1 (multu pacing), S2 (empty branch), S6 (BIOS jumptable) —
  the only signals that can carry STRONG — are all absent.
- **Gate 2 (citable SOTN precedent for the coercion family): FAIL.** The only
  mechanically-viable construct is an UNWRITTEN leading local array/struct lead. The
  nearest sanctioned family, the written-never-read local array carve-out, excludes it
  by its own scope sentence (.claude/rules/no-new-park-categories.md:255-262):
  "sanctioned ONLY when the target bytes contain the corresponding dead stores
  (oracle-enforced), written (not merely declared) … The unwritten-array and
  `(void)&local` forms remain forbidden." Measured here: target touches nothing in
  `sp+0x10..0x17`, so the carve-out's precondition is false for this function too.

### The 8 bytes are an ARGS-partition fact, not a locals-padding fact (NEW — recon run 2)
The first run framed the residual as "8 phantom bytes below the first declared
local". Reading the frame equation end-to-end this run makes the statement much
sharper, and moves the whole search out of the locals partition:

- `tools/gcc-2.7.2/config/mips/mips.h:1651` —
  `#define STARTING_FRAME_OFFSET (current_function_outgoing_args_size + (TARGET_ABICALLS ? MIPS_STACK_ALIGN(UNITS_PER_WORD) : 0))`
  and `FRAME_GROWS_DOWNWARD` is `#undef` (mips.h:1645). TARGET_ABICALLS is off in
  this build. So the address of the FIRST slot `assign_stack_local` hands out is
  **exactly `$sp + current_function_outgoing_args_size`**, with no rounding applied
  at that point (`compute_frame_size` rounds only its own `args_size` copy).
- `compute_frame_size` (mips.c:4444-4535) — the complete frame equation is
  `total = MIPS_STACK_ALIGN(get_frame_size()) + MIPS_STACK_ALIGN(outgoing_args_size)
   + extra_size + MIPS_STACK_ALIGN(gp_reg_size) + MIPS_STACK_ALIGN(fp_reg_size)`
  with `extra_size = MIPS_STACK_ALIGN(TARGET_ABICALLS ? UNITS_PER_WORD : 0)` = **0**
  here, `fp_reg_size` = 0, and the `pretend_args_size` term guarded by
  `ABI_64BIT && mips_isa >= 3` = **0** on o32. There is no fourth term to hide 8
  bytes in.
- Target: first local at `$sp+0x18`, saves at 0x60..0x6F, frame 0x70.
  ⇒ `get_frame_size()` (vars) = 72 in BOTH forms; the differing term is
  `current_function_outgoing_args_size`: **24 in the original, 16 in our honest
  build**. The committed `pre_pad[2]` fakes the same bytes on the vars side
  (vars 80 / args 16), which is why it is byte-equivalent but mechanically wrong.

This is why no locals-side spelling can ever be honest here: the theorem that the
first declared slot lands at vars offset 0 is not an obstacle to be spelled
around, it is the *definition* of where `local` goes. The only honest lever is
`current_function_outgoing_args_size == 24`.

### KILLED: "some callee originally took a 5th (stack) argument"
`tmp/grind/func_8001E404/s1/callee_stackarg_scan.py` scans every in-EXE callee
(func_80046BF4, func_8001A538, func_80061064, func_80041688, func_8003F3D4) plus
the sibling func_8001E6E4 for any `$sp`-relative access at or above its own frame
size — i.e. any read of an incoming stack argument. **Zero hits in all six**
(`…/s1/callee_stackarg_scan.txt`). None of them takes more than 4 words. So the
ordinary route to `outgoing_args_size = 24` (a call whose 5th/6th word is stored
into the caller's `sp+0x10..0x17`) is doubly refuted: no callee reads such an
argument, and the target contains no store into that range.

### What holds the byte-match today
No regfix/asmfix rule (0 of each). The committed body carries `s32 pre_pad[2];`
declared first — the unwritten-local-array frame coercion from the forbidden-family
catalog. It is RETAINED (not sanctioned) so the oracle stays green; it is exactly the
cheat this disposition refuses to legitimize.

- [s1] canonical func_8001E404 -> verdict C, 0 asm insns, 184 target insns, pure-C distance 23; sandbox --disable all -> score 23 (target_insns 184, build_insns 183, 0 rules dropped, 139 cheat-asm insns stripped file-wide). 0 regfix and 0 asmfix rules for this function.

- [s1] The committed body is byte-identical to target today; its match is held up by `s32 pre_pad[2];` declared first — the unwritten-leading-local-array frame coercion from the forbidden-family catalog. Retained (not sanctioned) so the oracle stays green.

- [s1] Target $sp map (complete): stores/loads at 0x18,0x1C,0x20 (words), 0x28,0x2A,0x2C (halfwords), 0x30 (word), address materializations `addiu $a3,$sp,0x18` / `addiu $s0,$sp,0x18` / `addiu $s1,$sp,0x28`, callee saves 0x60/0x64/0x68/0x6C, frame 0x70. NOTHING is read or written in sp+0x10..0x17.

- [s1] Frame equation, frozen toolchain: STARTING_FRAME_OFFSET == current_function_outgoing_args_size (mips.h:1651, FRAME_GROWS_DOWNWARD undefined at mips.h:1645, TARGET_ABICALLS off) — the first stack slot is at exactly $sp+outgoing_args_size, unrounded. compute_frame_size (mips.c:4444-4535): total = MIPS_STACK_ALIGN(get_frame_size()) + MIPS_STACK_ALIGN(outgoing_args_size) + extra_size + saves, with extra_size = 0 (non-ABICALLS), fp_reg_size = 0, pretend_args_size term gated on ABI_64BIT && mips_isa >= 3 = 0 on o32.

- [s1] Therefore the honest and original builds have IDENTICAL vars (get_frame_size() == 72) and differ only in current_function_outgoing_args_size: 16 (ours) vs 24 (original). The defect is an ARGS-partition fact, not a locals-padding fact — which is the mechanical reason every pad / lead-array / declaration-order spelling on this family failed, and why they should not be re-run.

- [s1] No callee of this function reads an incoming stack argument (six-function scan, tmp/grind/func_8001E404/s1/callee_stackarg_scan.txt), so no ordinary call site here can honestly carry args=24.

- [s1] Endgame gates, re-checked for this function (context only — the driver has NOT declared exhaustion and the modality is recon): gate 1 canonical-asm FAIL (scan_hand_coded tier=LOW 1/8, S1/S2/S6 absent); gate 2 citable-precedent FAIL (the written-never-read local-array carve-out's own scope sentence requires the target bytes to contain the dead stores; this target touches nothing in the region).

- [s1] The 2026-08-12 OWNER-ESCALATION entry filed for this function by the earlier discarded session has been annotated WITHDRAWN in docs/grind/decisions.md — the disposition was taken in the wrong modality; the measured evidence in it stands, the disposition does not. func_8001E404 is an ACTIVE grind item.


## Session 2 (structural, 2026-08-12) — the args partition is REPRODUCIBLE; floor 23 -> 0

### The `# vars=/regs=/args=/extra=` instrument is now in the loop (s1 frontier item 3, DONE)
`tmp/grind/func_8001E404/s2/frame.sh <src.c> [func]` compiles any TU with the exact
canonical cc1 flags (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -mel`, cpp `-Iinclude -undef -Wall -lang-c -fno-builtin` plus the
Makefile CPP_DEFS) straight to `.s` and prints the frame comment cc1 emits above
`.frame`, which reads out compute_frame_size's partition directly. Reusable verbatim
for func_8001E6E4 and func_8003CF84.
- committed (`pre_pad[2]`) form: `.frame $sp,112,$31  # vars= 80, regs= 4/0, args= 16, extra= 0`
- target's partition (from the asm): vars 72 / args 24 — same total 112, different split.
NB `src/code6cac.c` emits two `parse error before 'GameObj'` diagnostics (lines 757 /
1072 — `GameObj` is declared nowhere the TU includes); cc1 RECOVERS and emits complete,
correct code for every other function, and the real build has always done this. Not a
tooling failure; do not chase it.

### CONFIRMED — a compiled-out call keeps the raised args partition (s1 frontier item 1)
`tmp/grind/func_8001E404/s2/probe2.c` (a 72-byte local plus one >4-word call made
unreachable five different ways) — ALL FIVE report `vars= 72, args= 24` and emit NO
call and NO stores anywhere in the args region:
  `if (0) { g6(...); }` · `s32 mode = 0; if (mode) { g6(...); }` · a zero-trip
  `for (i = 0; i < 0; i++)` · statements after `return` · a `goto` over the call.
The live control (`if (G) { g6(...); }`) also reports args=24 but emits the `jal` and
the two `sw ...,16($sp)` / `sw ...,20($sp)` stack-arg stores. Mechanism exactly as
predicted by the s1 frontier: `expand_call` raises
`current_function_outgoing_args_size` during RTL expansion; `compute_frame_size` reads
it at final, after jump.c / cse.c have deleted the insns. Deleting the call does not
lower the partition.

### The dichotomy theorem (the durable result of this session)
o32 sets `REG_PARM_STACK_SPACE` / `OUTGOING_REG_PARM_STACK_SPACE` to an unconditional
16-byte register-home floor, so a LIVE call raises the args partition above 16 only by
having a 5th-or-later argument word — and every such word is genuinely STORED into
`sp+0x10..0x17`. Measured across the whole plausible space
(`tmp/grind/func_8001E404/s2/probe1.c`): six scalar words -> args=24 with stores at
16/20; three words plus a `double` (8-byte alignment pushes the double to offset 16)
-> args=24 with stores; two words plus a `double` -> args=16; a 72-byte struct passed
BY VALUE -> args=72 with block-copy stores; `__divdi3` (64-bit divide) -> args=16;
float libcalls -> args=16. No libcall family reachable from this body exceeds 16, so
s1 frontier item 2 (the library-call route) is KILLED.

=> **args > 16 with zero stores in the args region <=> the >4-word call was expanded
and then deleted.** The original translation unit therefore CONTAINED a >=5-word call
site that compiled away. The 8-byte "hole" is that call site's fossil, not padding —
and no live C can ever reproduce it.

### Word-count calibration of the three-function family
`tmp/grind/func_8001E404/s2/probe3.c`: a dead 5-word call and a dead 6-word call both
give args=24 (`MIPS_STACK_ALIGN` rounds 20 -> 24); a dead 7-word call gives args=32 —
which is exactly the 16-byte hole of the third census family member func_8003CF84
(frame 72, lowest touched 0x20, saves 0x38). So the census family is ONE phenomenon at
two word counts: 5-6 words in func_8001E404 / func_8001E6E4, 7-8 words in
func_8003CF84 (and sprintf's 512-byte hole is the same term, varargs-sized).
Also measured: a DELETED call still emits its string literal into `.rodata`. So if the
original dead calls had been `printf`-style, their format strings would be present —
and unreferenced — in the shipped rodata. That is a checkable forensic prediction about
what the deleted call was; it has NOT been run.

### Floor 23 -> 3 -> 0, measured this session
1. Delete `s32 pre_pad[2];`; add `extern void bb2_dbg_probe();` and
   `if (0) { bb2_dbg_probe(0, 0, 0, 0, 0, 0); }` at the top of the body. cc1 then
   reports `# vars= 72, regs= 4/0, args= 24, extra= 0` — the target's partition,
   produced on the args side rather than faked on the vars side — and
   `sandbox func_8001E404 --disable all` drops **23 -> 3** (build_insns 183 vs
   target 184, 0 rules dropped).
2. The residual 3 is the address-materialization choice s1 had attributed to the frame
   shift. With `local` at vars offset 0 its address IS the frame base, so GCC
   rematerializes `addiu $a0,$sp,0x18` at each of the two consumers, where the target
   holds it in `$s0` (`addiu $s0,$sp,0x18` once + `move $a0,$s0` twice). Naming the
   pointer — `s32 *lp = (s32 *)&local;` passed to func_80046BF4 and func_8001A538 —
   restores the target's form: **`sandbox --disable all` == 0**, 184/184, and the
   normalized objdiff is clean except the four known `addiu s2,s2,%lo(SYM)`
   relocation-display artifacts. `lp` is ordinary live C (a named pointer consumed by
   two live calls); the `if (0)` call is not.
`verify-oracle --rebuild` was REFUSED with `dirty-build-inputs` — the correct guard, a
rebuild with uncommitted edits would corrupt the `build/` reference the sandbox scores
against — so the full-link proof is not in hand. Sandbox 0 is.

### Disposition: ruling-request, tree reverted
The closing construct is a reconstructed compiled-out call site: dead code with no
observable effect. That fails tests T1 (semantic purpose) and T2 (human-programmer) on
its face, and no sanctioned SOTN family covers it — it is the mirror image of the
catalog's forbidden `if (1) { ... }` always-true wrapping. Per the first-reach rule
this session did NOT self-approve it. `src/code6cac.c` was reverted to the committed
`pre_pad` form (tree clean, oracle untouched) and the closing body is banked verbatim
in `memory/grind/func_8001E404/candidate.c` for one-edit re-application if the owner
rules it acceptable. What makes the question worth the owner's time rather than a
routine rejection: this construct does not coerce GCC's analysis of live code the way
every cataloged family does. The args partition is a mechanical fossil that PROVES the
original source contained such a call, and the dichotomy theorem shows no live C can
reproduce it. One ruling settles all three family members (func_8001E404,
func_8001E6E4 which is parked on exactly this question, and func_8003CF84).

## Session 3 (structural, 2026-08-13) — s2's result independently re-measured; the vars-side alternative partition CLOSED at the compiler-source level; the family cross-check DONE

Session 2 did all of the mechanism work recorded above but ended without writing an
outcome JSON, so the driver discarded it as an invalid session and re-dispatched
`structural`. Its ledger survived (it is what this file's Session 2 section is), so this
session did NOT re-derive any of it. What follows is only what is NEW.

### Re-measured this session, with the banked candidate applied to src/code6cac.c
`sandbox func_8001E404 --disable all` -> **score 0**, target_insns 184, build_insns 184,
0 rules dropped, 138 cheat-asm insns stripped file-wide. So the closing form in
`memory/grind/func_8001E404/candidate.c` reproduces at floor 0 on a fresh session; s2's
claim is confirmed by independent measurement rather than inherited.

### NEW — the deleted call leaves NO external reference in the object
`mipsel-linux-gnu-nm` on the sandbox object (`tmp/sandbox/func_8001E404/code6cac.o`)
with the candidate applied: **zero occurrences of `bb2_dbg_probe` in the symbol table**
(neither defined nor undefined), against 60+ genuine `U` entries for the file's real
callees. Because jump.c deletes the call before final, the fabricated callee never
becomes a relocation — so the construct cannot break the link, and it is invisible to
everything downstream of cc1. (Artifact: `tmp/grind/func_8001E404/s3/family.sh` output.)
This also means the choice of callee NAME is unobservable in the bytes: any 5-or-6-word
dead call produces the identical object. The name is therefore purely a
source-plausibility question, not a codegen one.

### NEW — the "vars-side internal object" alternative partition is CLOSED at the source level
The 8-byte hole below the first local admits exactly two frame partitions:
`args=24 / vars=72` (the reconstruction) or `args=16 / vars=80` with some
compiler-internal object occupying vars offsets 0..7 ahead of the work buffer. s1
asserted the second is impossible ("the pre-declaration window is provably empty on
o32") but did not enumerate it. Enumerated this session by grepping every
`assign_stack_local` / `assign_stack_temp` call site in `tools/gcc-2.7.2/function.c`:
- lines 3596/3605, 3758, 3888 — all inside `assign_parms` (function boundaries checked:
  `assign_parms` runs to `promoted_input_arg` at 3996). This function is
  `void func_8001E404(void)`: no parameters, so none of the three can fire.
- line 4503 — `trampoline_address` (taking the address of a NESTED function);
  allocates `TRAMPOLINE_REAL_SIZE`, and fires at the point of use, i.e. after the
  outermost block's `expand_decl`s, not before them.
- line 5038 — `expand_function_start`, guarded by `current_function_needs_context`,
  commented verbatim *"If function gets a static chain arg, store it in the stack frame.
  Do this first, so it gets the first stack slot offset."* This is the ONLY allocator
  that provably precedes the first declared local — and it allocates
  `GET_MODE_SIZE (Pmode)` = **4 bytes**, not 8, and only for a nested function.
- line 1347 — `put_reg_into_stack`, i.e. a pseudo demoted to memory during expansion:
  by construction it runs after the decls whose slots are already handed out.
=> No compiler-internal object can put exactly 8 bytes below the first declared local in
a `void f(void)`. The vars-side partition is reachable ONLY by a declared object, i.e.
the forbidden unwritten-leading-array family. The args-side partition is reachable ONLY
by an expanded-then-deleted >4-word call (s2's dichotomy theorem). **Both roads end at
"the original source contained something that compiled away"** — that is the complete
statement of the defect, and it is why no live-C form exists.

### NEW — the census family's partitions, all three measured (s2 frontier item 3, DONE)
Via `tmp/grind/func_8001E404/s3/report.sh` reading cc1's `# vars=/regs=/args=/extra=`
comment out of a full-TU compile with the canonical flags:

| function | our current form | frame | target's partition (from the asm layout) |
|---|---|---|---|
| func_8001E404 (candidate applied) | `vars= 72, args= 24` | 112 | vars 72 / args 24 — **identical** |
| func_8001E6E4 (committed `pre_pad[2]`) | `vars= 80, args= 16` | 112 | vars 72 / args 24 — same total, wrong split |
| func_8001E6E4 (honest, pre_pad deleted) | `vars= 72, args= 16` | 104 | 8 bytes short, as predicted |
| func_8003CF84 (as committed) | `vars= 40, args= 16` | 72 | first local 0x20, saves 0x38 => vars 24 / args 32 |

So all three family members are the SAME defect at two word counts, now with measured
partitions rather than by analogy: 5-6 dead argument words for the two 0x70-frame
functions, 7-8 for func_8003CF84. One ruling disposes of all three.
NB `frame.sh` exits non-zero because `src/code6cac.c` provokes recoverable cc1
diagnostics (`parse error before 'GameObj'` at 757/1072, a `func_80017FA0` prototype
conflict) — cc1 still writes complete, correct assembly for every other function, which
is what the real build has always relied on. Read the `.s` it produced rather than
trusting the exit code; `report.sh` does exactly that.

### Disposition
Unchanged from s2, and now on independently re-measured ground: **ruling-request**.
`src/code6cac.c` was reverted to the committed `pre_pad` form (tree clean); the closing
body stays banked verbatim in `candidate.c` for one-edit re-application.
