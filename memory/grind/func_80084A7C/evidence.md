# Evidence bank — func_80084A7C

## Session 1 (recon, 2026-08-17)

### Baseline
- Honest floor at session start (`sandbox func_80084A7C --disable all`): **26**,
  `target_insns` 145, `build_insns` 146, 11 regfix rules dropped, 79 cheat-asm
  lines stripped. Floor at session end: **26** (unchanged score, but a whole
  diff axis eliminated — see below).
- `canonical` verdict is **C** (from the queue record); this is a pure-C grind,
  not an asm-routing candidate.

### The function (from `asm/funcs/func_80084A7C.s`, 145 insns)
Motion/animation state advance handler. Signature in `src/main.c` is
`void func_80084A7C(s16 a0, s16 a1)`; three call sites in the same file
(`main.c:451,503,547`) cast it to a 4-arg pointer type
`(void (*)(s16, s16, u8, u8 *))` because it is one slot of the
`D_800F3340..3350` per-status-byte dispatch table — the extra args are ignored
by this entry. That prototype contradiction is pre-existing and is NOT part of
the residual (the extra args never reach a register in the body).

Structure: `base_ptr = &((s32 *)&D_80106F28)[a0]`, `offset = (s16)a1 * 0xB0`
(strength-reduced by GCC to `x*2 +x <<2 -x <<4`), `base = (u8 *)(*base_ptr +
offset)` held in `$s0` for the whole body. `a0`/`a1` are parked in `$s2`/`$s1`
for the post-call `spu_NotifyChannel((s16)(a0 | (a1 << 8)))` recomputation.
Every access to the flag word at `+0x98` **re-loads** `*base_ptr` (`lw 0($a3)`
in target) — GCC cannot CSE it because the preceding stores through
`base`/`base+0x98` may alias the table entry. That re-load-per-site shape is
already reproduced correctly by our C.

### Tooling built this session
`tmp/grind/func_80084A7C/s1/diffit.py` — disassembles the *honest* sandbox
object (`tmp/sandbox/func_80084A7C/main.o`, i.e. rules-disabled/cheat-stripped)
and normalizes both it and `asm/funcs/func_80084A7C.s` into one canonical token
stream (objdump `v0`/decimal-offset/numeric-branch syntax vs splat
`$v0`/hex/`.L` syntax; `li`→`addiu`, `move`→`addu`, `nop`→`sll`; branch and
call targets masked). Emits a difflib opcode diff. This is what turned a
26-point scalar into the exact residual below; **re-use it, do not rebuild it.**

### The residual, exactly (after this session's edit)
The ENTIRE remaining difference is one register-allocation decision plus its
one-instruction consequence:

1. **Two-register swap.** Target keeps `base_ptr` in `$a3` and `offset` in
   `$a2`. Our build keeps `base_ptr` in `$a2` and `offset` in `$a3`/`$a1`.
   Every one of the ~18 mismatching instructions is a `lw $vX, 0($a2)` where
   target has `0($a3)`, or an `addu $vX, $a1, $vX` where target has
   `addu $vX, $a2, $vX`. Same opcode, same operand order, same immediates.
2. **One extra instruction (146 vs 145): `move $a1, $a3`** in the entry block,
   emitted between `lw $v1, 0($a2)` and `addu $s0, $v1, $a3`. It is a **failed
   coalesce**: the `*0xB0` multiply chain's final `sll` writes `$a3` (a
   block-local quantity that local-alloc assigns first), while the `offset`
   *global* allocno — live across the whole body for the eight `+0x98` sites —
   was given `$a1` by global-alloc, so a copy joins them. `$a1` is available to
   global-alloc precisely because our build sinks `move $s1, $a1` early (our
   prologue order is `sw s1 / move s1,a1 / sw ra / sw s0 / lw`, target's is
   `sw ra / sw s1 / sw s0 / lw / move s1,a1` — exactly what the inherited
   `reorder 16,14,17,18,15` regfix rule was papering over). Target's `offset`
   allocno gets `$a2` and coalesces with the multiply chain's destination, so
   no copy and 145 insns.

Read that as one causal chain: **the parameter copy `s1 = a1` retiring `$a1`
early is what lets global-alloc put `offset` in `$a1`, which both costs the
copy and pushes the `$a2`/`$a3` pair into the swapped assignment.**

### The 11 inherited regfix rules confirm this reading
`regfix.txt:719-742` for this function is exactly: `$6 <-> $7` (the a2/a3 swap),
six `subst` rules fixing the `addu` operand order after the swap, one
`delete @ 19` (the extra `move`), and one prologue `reorder`. The rule set is a
mechanical description of the residual above and nothing else — there is no
hidden second problem lurking behind the cheats.

### Measured this session
| Form | sandbox `--disable all` | insns |
|---|---|---|
| inherited HEAD (`*base_ptr + offset + 0x98`) | 26 | 146 |
| **offset-first flag expressions (`offset + *base_ptr + 0x98`)** | **26** | 146 |
| + `shifted` intermediate inlined into `base_ptr` | 26 | 146 |
| `offset` declared before `base_ptr` (with `shifted`) | 32 | 146 |
| `offset` declared before `base_ptr` (no `shifted`) | 33 | 146 |

The offset-first flip is score-neutral but strictly better structurally: before
it, the eight flag-site `addu`s differed in BOTH operand order and register;
after it they differ only in register. It is the form saved in `candidate.c`.

- [s1] Honest floor (sandbox func_80084A7C --disable all) is 26, target_insns 145, build_insns 146, 11 regfix rules dropped, 79 cheat-asm lines stripped. Floor unchanged 26 -> 26 this session; the gain was structural, not scalar.

- [s1] Built tmp/grind/func_80084A7C/s1/diffit.py: disassembles the honest (rules-disabled, cheat-stripped) sandbox object and normalizes objdump syntax and splat asm syntax into one token stream (li->addiu, move->addu, nop->sll, hex/decimal offsets unified, branch and call targets masked), then difflib-diffs them. This is what converted the 26-point scalar into the exact residual. Re-use it; do not rebuild it.

- [s1] THE ENTIRE RESIDUAL IS ONE ALLOCATION DECISION AND ITS CONSEQUENCE: (1) target holds base_ptr in $a3 and offset in $a2, our build holds base_ptr in $a2 and offset in $a3/$a1 — every one of the ~18 mismatching instructions is 'lw $vX,0($a2)' where target has '0($a3)' or 'addu $vX,$a1,$vX' where target has 'addu $vX,$a2,$vX', same opcode, same operand order, same immediates; (2) the 146th instruction is 'move $a1,$a3' in the entry block, a failed coalesce between the *0xB0 multiply chain's final sll destination ($a3, a block-local quantity local-alloc assigns first) and the 'offset' GLOBAL allocno that global-alloc placed in $a1.

- [s1] The causal chain: our build sinks 'move $s1,$a1' early (our prologue order is sw s1 / move s1,a1 / sw ra / sw s0 / lw; target's is sw ra / sw s1 / sw s0 / lw / move s1,a1), which retires $a1 early and makes it available to global-alloc for the 'offset' allocno; that both costs the join copy and pushes the $a2/$a3 pair into the swapped assignment. Attack the chain, not the register names.

- [s1] The 11 inherited regfix rules (regfix.txt:719-742) are a mechanical restatement of exactly this residual and nothing else: '$6 <-> $7' (the a2/a3 swap), six 'subst' rules repairing addu operand order after the swap, 'delete @ 19' (the extra move), and one prologue 'reorder 16,14,17,18,15' (which is precisely the move-s1-a1 placement difference). There is no second hidden problem behind the cheats.

- [s1] Function shape: motion/animation state advance. base_ptr = &((s32*)&D_80106F28)[a0]; offset = (s16)a1 * 0xB0 (GCC strength-reduces to x*2 +x <<2 -x <<4); base = (u8*)(*base_ptr + offset) held in $s0 body-wide; a0/a1 parked in $s2/$s1 for the post-call spu_NotifyChannel((s16)(a0 | (a1 << 8))) recomputation. Target RE-LOADS *base_ptr at every +0x98 flag site ('lw 0($a3)') because the preceding stores through base may alias the table entry — our C already reproduces that re-load-per-site shape correctly, so it is NOT part of the gap.

- [s1] The declared signature 'void func_80084A7C(s16 a0, s16 a1)' is contradicted by three call sites in the same file (main.c:451,503,547) that cast it to (void (*)(s16,s16,u8,u8*)) because it is one slot of the D_800F3340..3350 dispatch table. This is pre-existing and NOT part of the residual — the extra args never reach a register in the body. Do not spend a session on it.

- [s1] SCORE IS A MISLEADING GRADIENT ON THIS FUNCTION: two separate probes (H1 confirmed, H4 killed) both left the score at exactly 26 while changing the emitted code materially in opposite directions. Future sessions must judge probes with diffit.py, not with the sandbox scalar alone.

## Session 2 (structural, 2026-08-17)

### Floor: 26 -> 0 (sandbox `--disable all`, 145 insns == target)
Two independent levers, found by cc1-only sweeps and confirmed by the sandbox:

1. **The sibling idiom (natural, worth 2 points: 26 -> 24).** `src/main.c:303`
   `spu_SetMotionState` is an ALREADY-MATCHED neighbour that reads the same
   `D_80106F28` table with the same 0xB0 stride, and it spells the access as
   `s32 shifted = a0 << 16; s32 *addr = (s32 *)&D_80106F28; s32 *base_ptr =
   (s32 *)((u8 *)addr + (shifted >> 14));`. Adopting that verbatim moves the
   table address load from AFTER `sra v0,v0,14` to BETWEEN `sll v0,a0,16` and
   the `sra` — target's schedule. **Every other address spelling is inert**
   (measured: index form `&((s32*)&D_80106F28)[(s16)a0]`, pointer add,
   reversed PLUS operand order, `(s16)a0 * 4`, `(s16)a0 << 2`, a named `tbl`
   alone, a named `idx` alone — all eight emit byte-identical code). Only the
   TWO-named-intermediate sibling form moves the schedule; `shifted` alone or
   `addr` alone do not (addr-alone overshoots, putting the `la` at index 1,
   before the `sll`).
2. **`offset = -((s16)a1 * -0xB0)` (the open question, worth the other 24).**
   Arithmetically identical to `* 0xB0`; combine cancels the negation pair, so
   the emitted multiply chain is byte-identical. What changes is allocation.

### The RA mechanism, read off the `.greg` dumps (the mandated F1 diagnosis, now DONE)
`tmp/grind/func_80084A7C/s2/greg.py` reproduces the pipeline's cpp+cc1 and
extracts this function's section from a `cc1 -da` dump. Comparing the plain
form against a form that fixes the allocation:

    plain:  ;; 8 regs to allocate: 91 94 76 90 82 93 74 72
            ;; 82 conflicts: ... 2 3 4 29          <- NO conflict with hard reg 5
            (no preference line for 82)            -> allocno 82 gets $a1
    fixed:  ;; 7 regs to allocate: 93 96 82 76 95 74 72
            ;; 82 conflicts: ... 2 3 4 5 29        <- conflicts with hard reg 5
            ;; 82 preferences: 6                   -> allocno 82 gets $a2

Allocno 82 is `offset`. In target, hard reg $a1 is still live at the offset
def (target's `move $s1,$a1` sits at insn 19, AFTER `lw $v1,0($a3)`), so
`offset` cannot take $a1. In our plain build the `s1 = a1` copy is scheduled
early (insn 16), $a1 dies, allocno 82 takes it, and the multiply chain's final
`sll` (a block-local quantity local-alloc placed in $a3) then needs the join
copy `move $a1,$a3` — our 146th instruction. Fixing the conflict/preference
fixes BOTH halves at once, exactly as session 1's frontier predicted.

### Measured this session (sandbox `--disable all`)
| Form | score | insns |
|---|---|---|
| s1 candidate (offset-first flag sites, no `shifted`) | 26 | 146 |
| + sibling idiom, plain `(s16)a1 * 0xB0` | 24 | 146 |
| + sibling idiom, `-((s16)a1 * -0xB0)` | **0** | **145** |
| plain base_ptr + `-((s16)a1 * -0xB0)` | 2 | 145 |
| `(s16)a1 * 0x80 + (s16)a1 * 0x30` (split multiply) | 11 | 145 |
| `u8 *base;` + separate assignment (statement before later decls) | 135 | 34 |

The last row is a C89 violation (a statement ahead of the `s32 val; u32
threshold;` declarations) — cc1 truncates the function. Any sweep row showing
~34 build_insns is that parse failure, not a codegen result; ignore it.

### Which offset spellings reach the target allocation (cc1-only fingerprint)
GOOD (offset ends in $a2, no `move $5,$7`): `-((s16)a1 * -0xB0)`,
`-(-0xB0 * (s16)a1)`, `((s16)a1 << 4) * 0xB`, `(s16)a1 * 0xAF + (s16)a1`,
`(s16)a1 * 0xB1 - (s16)a1`, `(s16)a1 * 0x80 + (s16)a1 * 0x30`,
`(s16)a1 * 0xA0 + (s16)a1 * 0x10`, `(s16)a1 * 0xC0 - (s16)a1 * 0x10`.
BAD (offset in $a3 + the join copy): `(s16)a1 * 0xB0`, `a1 * 0xB0`,
`(s32)a1 * 0xB0`, `(s16)(u16)a1 * 0xB0`, `s16 ch = a1; ch * 0xB0`,
`s32 idx = (s16)a1; idx * 0xB0`, `((s16)a1 * 0xB) * 0x10`,
`((s16)a1 * 0x58) * 2`, `((s16)a1 * 0x2C) * 4`, `((s16)a1 * 0x16) * 8`,
`(s16)a1 * 0x58 + (s16)a1 * 0x58`.
Of the GOOD set, only the two negation spellings ALSO emit target's exact
multiply chain (`sll ,1 / addu / sll ,2 / subu / sll ,4` with target's $v1/$v0
pairing); every other GOOD spelling changes the arithmetic itself and leaves
5-11 points of chain mismatch. So the negation is not one option among many —
it is the only measured spelling that buys the allocation for free.

### The disposition question this session hands to the owner
`-((s16)a1 * -0xB0)` has no observable effect (T1), no human-programmer
rationale (T2), and its mechanism is stated in GCC-internals terms (T3:
pseudo/allocno numbering feeding `global.c`'s conflict + preference tables).
It is a FIRST REACH: no sanctioned family covers an inline arithmetic no-op
used as an RA lever. The nearest sanctioned family, "opaque arithmetic
variables" (`s32 one = 1;`; SOTN's `(Random() & 3) + 1 - 1`), is about a NAMED
variable defeating a bit-test transform. Per the frozen-list non-extension
clause the session did not self-approve it; outcome = `ruling-request`.

### Tooling built this session (re-use; do not rebuild)
- `tmp/grind/func_80084A7C/s2/greg.py` — pipeline-faithful cpp+cc1 `-da` dump,
  extracts this function's `.greg`/`.lreg`/`.combine`/`.sched` sections.
  NB cc1 exits non-zero on main.c's pre-existing redeclaration warnings and
  still emits complete dumps — never gate on its exit status.
- `tmp/grind/func_80084A7C/s2/sweep2.py` / `sweep3.py` / `sweep5.py` /
  `sweep6.py` — cc1-ONLY variant sweeps (seconds each vs minutes for a full
  sandbox run) fingerprinting: which hard reg the multiply chain's last insn
  writes, presence of `move $5,$7`, and the `la`-vs-`sra` order. Screen with
  these, then sandbox only the winners.
- `sweep.py` — the full-sandbox version (slower; use for confirmation).

### TRAP that cost this session three turns (record for every future session)
`    s32 offset = (s16)a1 * 0xB0;` occurs TWICE in src/main.c — the sibling
`spu_SetMotionState` at line ~307 has the identical line. A `str.replace(..., 1)`
or a DOTALL regex anchored on `s32 *base_ptr = ...` silently edits the SIBLING
(or spans from it into our function and deletes everything between). Every edit
anchor MUST include `&D_80106F28 + ((a0 << 16) >> 14)` or equivalent unique
text, and must be non-DOTALL. A wrong-anchor edit LOOKS like a null result
(dumps identical to baseline), which is how it burned three turns.

- [s2] Honest floor reached 0 this session (sandbox --disable all: score 0, build_insns 145 == target_insns 145) with two levers in src/main.c: BB2's own sibling entry idiom (natural) plus `offset = -((s16)a1 * -0xB0)` (an arithmetic no-op with no sanctioned family). Session returned ruling-request, NOT candidate-ready.
- [s2] The mandated F1 .greg diagnosis is DONE: allocno 82 is `offset`; in the plain form it has NO conflict with hard reg 5 and no copy preference, so global-alloc gives it $a1 and the multiply chain's block-local $a3 destination needs the join copy `move $a1,$a3`; in the fixed form 82 conflicts with hard reg 5 AND carries `preferences: 6`, so it lands in $a2 and the copy vanishes. One decision, both halves of the residual.
- [s2] The matched sibling spu_SetMotionState (src/main.c:303) reads the SAME D_80106F28 table with the SAME 0xB0 stride; copying its two-named-intermediate entry idiom (`s32 shifted` + `s32 *addr`) verbatim moves the table address load into target's schedule slot (between `sll v0,a0,16` and `sra v0,v0,14`) and is worth 2 points on its own. All eight other address spellings tested are byte-identical to each other — cse/combine canonicalises them; only this idiom moves the schedule.
- [s2] Best form containing NO unsanctioned construct measures 24 (banked as rejected/natural-offset-no-negation-floor24.c). If the owner refuses the negation lever, the remaining problem is exactly: make the `offset` allocno conflict with hard reg $a1 (or acquire a copy preference for $a2) without an arithmetic no-op.
- [s2] cc1-only sweeps are the right gradient on this function: tmp/grind/func_80084A7C/s2/sweep{2,3,5,6}.py fingerprint the allocation directly (which hard reg the chain's last insn writes, presence of `move $5,$7`, `la`-vs-`sra` order) in seconds, where a sandbox run takes minutes and its scalar score misleads (s1 banked two probes that moved structure at a flat 26).
- [s2] EDIT-ANCHOR TRAP: `s32 offset = (s16)a1 * 0xB0;` appears twice in src/main.c (the sibling at ~line 307 has the identical line). Anchor every programmatic edit on `&D_80106F28 + ((a0 << 16) >> 14)` and never use a DOTALL span — a DOTALL anchor reaches from the sibling into our function and deletes the body between them.

## Session 3 (structural, 2026-08-17)

### Floor: 24 (the natural resume point) -> 0, with NO unsanctioned construct
The Judge refused session 2's negation lever and directed a resume from
`rejected/natural-offset-no-negation-floor24.c`. That resume closed the function
in natural C by DELETING a variable rather than adding a construct: the
`s32 offset` local is gone, and the stride multiply is written out where it is
needed —

    u8 *base = (u8 *)(*base_ptr + (s16)a1 * 0xB0);
    ...
    *(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98) &= ~1;    /* x8 flag sites */

`sandbox func_80084A7C --disable all` = score 0, target_insns 145,
build_insns 145, 11 regfix rules dropped, 79 cheat-asm lines stripped. The s1
normalized objdump diff shows the ONLY remaining token difference is the unlinked
sandbox object's unresolved `D_80106F28` lui/addiu relocation pair (target `REL`
vs our `0`).

### Why it works (the mechanism, now fully closed)
Sessions 1-2 localized the residual to one hard-reg decision. This session names
its cause: `offset` and `base_ptr` were two GLOBAL allocnos of EQUAL
`allocno_compare` priority — each about ten refs across the same span — so
global.c's tiebreak fell through to the raw allocno NUMBER. It reached `offset`
while $a1 was free (our early `move $s1,$a1` retires the parameter) and gave it
$a1, which both cost the failed coalesce with the multiply chain's block-local
$a3 destination (`move $a1,$a3`, our 146th instruction) and swapped the $a2/$a3
pair against target. Session 2 flipped that tiebreak by perturbing pseudo
numbering (an arithmetic no-op). Session 3 removes the tie instead: with no named
holder the multiply's value is a single-def quantity whose definition IS the
chain's final `sll`, so it lands in $a2 exactly as target does, `base_ptr` keeps
$a3, and no join copy is ever created.

**Generalisable lesson: when two same-priority global allocnos fight over an
arg-register pair, DELETING one of the two C handles is a legitimate lever, and
it should be tried before any coercion is considered.**

### Measured this session (cc1-only gold diff; 0 == byte-identical to target)
GOLD = the cc1 asm of session 2's distance-0 form (whose bytes are target's), so
`golddiff` counts differing insn lines with no sandbox run. Baseline = the
floor-24 natural form = 45. Insn counts are pre-maspsx (target 123).

| Form | golddiff | insns | notes |
|---|---|---|---|
| floor-24 natural baseline | 45 | 124 | offset in $a1, `move $5,$7`, a2/a3 swapped |
| table word read into a local before the offset chain | 45 | 124 | INERT (byte-identical to baseline) |
| base as pointer-add / offset-first / `u8 *` entry handle | 45 | 124 | INERT |
| `u32 offset` (type-narrowing axis) | 45 | 124 | INERT |
| flag sites re-deriving base_ptr from `addr` | 45 | 124 | INERT |
| sibling `_SsSeqPlay` one-expression base, offset kept | 45 | 124 | INERT |
| 4-parameter signature matching the call sites | 45 | 124 | INERT |
| separate stage variable (`s32 idx = (s16)a1;`) | 45 | 124 | INERT |
| declaration-order permutations (4, with the sibling idiom) | 45-51 | 124 | never better |
| `s16 chan = a1;` consumed by the notify sites | 41 | 124 | copy moves to $t0 |
| channel word cached in one local | 59 | 118 | wrong bytes (drops a recompute) |
| same-variable split-init (`offset = (s16)a1; offset = offset * 0xB0;`) | 19 | 124 | swap FIXED, 1 extra join copy |
| split-init with shift-pair staging (`a1 << 16`, then `(>>16)*0xB0`) | 13 | 124 | same 1-insn residual |
| `offset = (s16)a1 * 0xB; offset = offset << 4;` | 16 | 123 | insn count right, entry chains emitted in the wrong order |
| base_ptr staged in its own variable (two-def, table side) | 47 | 124 | worse |
| base from `(offset << 4)` with offset finalised after | **0** | 123 | closes, but reads backwards |
| the multiply duplicated at `base` and at the offset holder | **0** | 123 | closes (cse unifies) |
| `offset` holds `a1*0xB`, every use shifts `<< 4` | **0** | 123 | closes |
| **NO offset variable — multiply written at every site** | **0** | 123 | **SUBMITTED**; also 0 with flag sites base_ptr-first; multiply-first at `base` costs 2 |

### Tooling built this session (re-use; do not rebuild)
Session 2's `.py` scripts were NOT preserved under `tmp/` (only their outputs
survived), so the gradient was rebuilt:
- `tmp/grind/func_80084A7C/s3/sweep.py` — pipeline-faithful cpp + cc1 (-G0
  CC_FLAGS; main.c is NOT a GP file) over a list of complete function bodies,
  diffing the extracted function asm against `gold.s`. `--gold` regenerates gold
  from `gold_body.c`. A nine-variant batch costs ONE turn. Two traps are encoded
  in it: this fork emits `.L`-prefixed labels, so an extractor that stops at the
  first non-`$L` label silently truncates the function to 36 of 123 insns; and
  cc1 exits non-zero on main.c's pre-existing redeclaration warnings while still
  writing complete output — never gate on its exit status.
- `tmp/grind/func_80084A7C/s3/greg.py` — the same pipeline with `-da`; extracts
  this function's `.greg` section and prints the `;; N regs to allocate:` line
  (which IS `allocno_compare`'s post-qsort priority order), the conflict and
  preference lines, and the `;; Register dispositions:` block. This is what
  proved `80 in 6 / 82 in 5` (broken) versus `82 in 6 / 80 in 7` (fixed).
- `tmp/grind/func_80084A7C/s3/variants{1..9}.py` — the 50 measured forms.

- [s3] FLOOR 0 IN FULLY NATURAL C: deleting the `s32 offset` local and writing the stride multiply `(s16)a1 * 0xB0` at the base computation and at each of the eight +0x98 flag sites gives sandbox --disable all score 0 with build_insns 145 == target_insns 145. No construct is added anywhere (the diff is 11 insertions / 11 deletions and is a net DELETION of one local); session 2's refused negation no-op is absent. Self-vet: memory/grind/func_80084A7C/self_vet.md.
- [s3] THE RESIDUAL WAS AN ALLOCNO-PRIORITY TIE, AND DELETING A C HANDLE BREAKS TIES: `offset` and `base_ptr` were two global allocnos of equal allocno_compare priority, so global.c's tiebreak fell to the allocno number and gave `offset` $a1 (join copy `move $a1,$a3` plus the $a2/$a3 swap). With no named holder the multiply's value is single-def, its def IS the chain's final `sll`, and it takes $a2 exactly as target does while base_ptr keeps $a3. Try deleting one of two tied handles BEFORE reaching for any coercion.
- [s3] MEASURED INERT on this function (all byte-identical to the floor-24 baseline, cc1-only golddiff 45): reading the table word into a local before the offset chain; base as pointer-add or offset-first; a u8* entry handle; `u32 offset`; flag sites re-deriving base_ptr from `addr`; the sibling _SsSeqPlay one-expression base; a separate stage variable (`s32 idx`); and the 4-PARAMETER SIGNATURE matching the three casting call sites (main.c:451,503,547). The prototype contradiction is codegen-inert here — banked as rejected/four-param-signature-inert.c.
- [s3] The same-variable split-init `offset = (s16)a1; offset = offset * 0xB0;` fixes the WHOLE $a2/$a3 swap (greg dispositions 82 in 6 / 80 in 7, no `move $5,$7`) but leaves exactly one extra instruction: the chain's final sll writes a temp that is copied into the variable, and copy-prop rewrites only the two early-branch uses, so both pseudos stay live and the copy survives. Seven spellings measured, all with the same 1-insn residual; a SEPARATE stage variable is inert, so the lever is the two-def structure and not the extra name. Banked as rejected/offset-split-init-two-def-join-copy.c.
- [s3] Session 2's tmp scripts were gone (tmp keeps only their outputs), so the cc1-only gradient was rebuilt as tmp/grind/func_80084A7C/s3/sweep.py plus greg.py. Two traps encoded there: this fork emits `.L`-prefixed labels, so an asm extractor that stops at the first non-`$L` label silently truncates the function to 36 of 123 insns; and cc1 exits non-zero on main.c's pre-existing redeclaration warnings while still writing complete dumps.
- [s3] The `shifted` + `addr` entry idiom is not a one-off sibling quirk: it appears in FOUR already-matched functions in src/main.c (lines 304-305, 596-597, 615-616, 628-629). It is this translation unit's house style for a D_80106F28 table access, which is the strongest available naturalness evidence for keeping it.
