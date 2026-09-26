---
name: compiler-flags-canonical
paths: [".claude/rules/compiler-flags-canonical.md"]
description: "Compiler FLAGS are a dead avenue for unmatched functions. -O2 (+ per-file GP_FILES and NO_SR_FILES exceptions) is PROVEN canonical for every file with remaining work. Don't flag-hunt; the walls are source-structure."
metadata:
  type: reference
---

# Compiler flags are canonical — do not flag-hunt

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

When a function won't match, it is tempting to wonder "did the original build use
different compiler flags than ours?" **It did not.** This was settled with a
project-wide proof (2026-05-20). Do not spend time toggling `-O` levels or
`-f*` flags hoping a remaining function will fall out — the divergence is always
in the C **source structure** (scheduling, register allocation, cross-jump
merging — see [[cross-jump-call-merge]]), never in flags.

## The proof (closed loop)

1. **GCC 2.7.2 has per-translation-unit flags only.** No `#pragma optimize`, no
   per-function flag mechanism. A function's flags are its `.c` file's flags.
2. **Every `.c` file containing remaining work also contains many byte-exact
   `-O2` matches.** Coverage scan (`tmp/flag_coverage.py`): main.c = 95 compiled
   matches + 17 bridged; text1b.c = 277 + 71; code6cac.c = 89 + 17; etc. **Zero**
   files have remaining work without compiled matches.
3. If a file held a sub-group compiled with *different* flags, those functions
   would NOT match at our `-O2` — **but all non-bridged functions in every file
   match.** So each file is provably **flag-uniform at `-O2`**.
4. Therefore the bridged/cheated functions share their file's flags (`-O2`); no
   alternative flag can change their canonical output.

## `-mel` (2026-08-04) — part of the canonical flag set, not a flag-hunt precedent

`-mel` was added to `CC_FLAGS`/`CC_FLAGS_GP` (Makefile + engine/buildconfig.py)
by owner election on 2026-08-04. It is a CONFIGURATION-FIDELITY correction, not
a tuning flag: the cc1 build's `mips-mips-gnu` triple defaulted to big-endian for
a little-endian game, which (a) shifted every 4-byte reload spill slot to
4 mod 8 where the original compiler provably emitted 0 mod 8 (func_80060E38
nine-session evidence chain, docs/grind/decisions.md 2026-08-03), (b) reversed
struct bitfield allocation ([[bitfield-direction-divergence]] — now resolved),
and (c) emitted big-endian lwl/lwr offsets (the reason the now-retired
`fix_lwl` pipeline stage existed). A 4-build controlled experiment validated
the switch: full-build SHA1 == oracle, func_80060E38 COMPLETED-C with zero
rules, zero regressions after the OTag field-order restore + a 2-rule decBs0
re-fit. This does NOT reopen flag-hunting: the flag set (now including -mel)
remains frozen and the walls remain source-structure.

## `-msoft-float` (2026-09-07) — the second target-triple default corrected

Adopted by delegated owner ruling 2026-09-07 (docs/grind/decisions.md, same-day
entry). The same `mips-mips-gnu` cc1 build defaults to HARD float; the PS1 has
no FPU and PsyQ's original `cc1psx` prints `# Cc1 defaults: -mgas -msoft-float`
in every asm file it emits (verified directly: `float a*b` → `jal __mulsf3`
under cc1psx, `mul.s` under our default). The integer-code consequence:
`CONDITIONAL_REGISTER_USAGE` fixes the 32 FP registers only under soft float,
and `loop.c:532` sizes the invariant-hoist threshold from `n_non_fixed_regs`
(122 hard vs 58 soft, tested at `loop.c:1631` as
`threshold * savings * lifetime >= insn_count`). Our build had been hoisting
loop invariants the original compiler left in-loop for every loop with
`insn_count` in (58, 122]. Project-wide blast radius measured at exactly two
functions: func_8007526C (ordinary do-while body becomes byte-exact) and
func_800324D0 (its `/* FAKE */` completion was an artifact of the wrong
threshold — reopened). cc1psx reproduces the target's loop shape on the
ordinary body instruction-for-instruction. Like `-mel`, this is
configuration fidelity, NOT a flag-hunting precedent — the flag set (now
`-mel -msoft-float`) stays frozen. cc1psx's printed `Cc1 defaults` header is
the authoritative record of the original configuration.

## The only real per-file flag variation (already encoded)

| Mechanism (Makefile) | Flag | Why |
|---|---|---|
| `GP_FILES` | `-G8` instead of `-G0` | small-data threshold for GP-relative files. **First member: text1a (owner-approved 2026-08-05** — target %gp_rel loads + MEM_IN_STRUCT_P dependence + cost-model proof; PsyQ's ccpsx defaulted to -G8 and the maspsx sdata_syms machinery had been compensating). Census screening rule for further adoptions: a file is G8-safe only if every <=8-byte extern whose compiled instructions -G8 changes is in-gp-range (sdata_syms.txt) or honestly non-small-typed (scope narrowed by owner ruling 2026-09-26, third batch: see § "Screening scope" below), and any file-scope `__asm__` is extracted to asm/funcs first (-G8 defers function bodies; top-level asm floats to .text 0). |
| `NO_SR_FILES` | `-fno-strength-reduce` | files where strength-reduction diverges |
| `FIX_LWL_FILES` | RETIRED 2026-08-04 (empty) | fix_lwl XOR-corrected big-endian lwl/lwr offsets; obsolete under -mel |

These ARE the per-file flag variation the original build used. Nothing else.
If a file isn't on these lists, it is plain `-O2 -G0`. The one route for adding
a file to `GP_FILES` is the owner ruling of 2026-09-26 below.

### Screening scope (owner ruling 2026-09-26, third batch)

**Question and answer.** Filed question: docs/grind/borderline.md 2026-09-26
"func_80034708 — -G8 screening rule vs plain small externs". The question put
to the owner, verbatim (record: docs/grind/owner-rulings-2026-09-26.md, batch
3): "The -G8 screening rule says every small variable a -G8 file mentions must
be on the small-data list. func_80034708's file mentions one 4-byte counter
(D_800A37B8) that can't go on that list — other functions access it the
normal way — and its code is identical at -G0 and -G8. The already-approved
text1a -G8 files have 29 such variables. Should screening only require
listing the variables whose compiled code actually changes under -G8?" Owner
(Trenton) chose, verbatim: **"Only if code changes (Recommended)"**, whose
text is: "A small variable must be listed only when -G8 changes its compiled
instructions; proven by building it both ways (bytes identical). Unblocks
func_80034708; matches existing text1a practice."

**Rule text** (the author's narrowing, not the owner's words). The screening
rule's "every <=8-byte extern" is narrowed to the externs whose compiled
instructions `-G8` changes. An extern of 8 bytes or less that a `-G8` TU
references, that is not in `sdata_syms.txt` and is not honestly typed larger
than 8 bytes, is exempt from the listing requirement ONLY when all of the
following hold:
1. **Built both ways.** The TU is compiled through the full per-file pipeline
   (cc1, maspsx and the assembler, with every other flag and per-file list
   exactly as in the build) once as a `-G8` TU and once as a `-G0` TU.
2. **Every access identical.** In the two objects, every instruction that
   accesses the extern or forms its address has identical bytes and an
   identical relocation (type, symbol and addend), and the two objects have
   the same number of such instructions. One differing access, or an access
   present in only one object, means `-G8` changes its compiled instructions,
   and the extern must be listed in `sdata_syms.txt` (in gp range) or be
   honestly typed larger than 8 bytes, exactly as before.
3. **Banked.** The function's ledger (`memory/grind/<func>/`) records, for
   each exempt extern: its size, why it is not in `sdata_syms.txt`, the two
   build command lines, and the side-by-side listing of every access (offset,
   bytes, relocation) from both objects.

Nothing else changes. The file-scope `__asm__` half of the screening rule, the
rest of the Per-file -G8 ruling below and the full-build oracle SHA1 all apply
unchanged. The exemption covers only the listing requirement; it never admits
an access whose `-G8` bytes differ. As before, screening applies to further
adoptions only: the two text1a `-G8` files approved on 2026-08-05 are not
re-screened by this ruling. Record:
docs/grind/decisions.md 2026-09-26 OWNER RULING — -G8 screening scope.

## Per-file -G8 by proof (owner ruling 2026-09-26)

**Question and answer.** Filed questions: docs/grind/borderline.md
2026-09-26 func_80036140 (its build-model question, which the first 2026-09-26
batch left undecided) and func_80034708. The context given to the owner
(verbatim record: docs/grind/owner-rulings-2026-09-26.md (batch 2)): func_80034708 (and
func_80036140) need a new per-file -G8 translation unit; for func_80034708
the target reads D_800A3174 gp-relative 16 times, the neighbouring functions
none, and the original PsyQ cc1psx emits those gp reads at -G8 and none at
-G0. The question put to the owner, verbatim: "Allow giving a function its own
source file compiled at -G8 (small-data setting) when the shipped code proves
it — gp-relative reads in the original bytes that neighbours lack, confirmed
by the original PsyQ compiler producing them only at -G8?" Owner (Trenton)
chose, verbatim: **"Allow with that proof (Recommended)"**, whose text is:
"Requires gp-relative accesses in the original bytes + cc1psx confirmation +
neighbours moved unchanged; layer-2 still reviews. Unblocks func_80034708 (and
part of func_80036140)."

**Rule text.** This is the author's narrowing of that answer, not the owner's
words. A function may be moved out of its splat `.c` file into a new
translation unit (TU) that joins `GP_FILES` (compiled `-G8`) ONLY when ALL of
(i)-(vi) hold, including (iv-a). The flag set itself is unchanged:
`CC_FLAGS_GP` is the existing text1a flag line, and no other flag, list or
pipeline stage is added.

- **(i) gp-relative accesses in the original bytes, which the neighbours
  lack.** The function's ledger (`memory/grind/<func>/`) lists every
  gp-relative access in the function's ORIGINAL bytes (`asm/funcs/<func>.s`
  and the original EXE): address, instruction and symbol. It also lists, for
  each function that stays outside the new TU and is adjacent to it in
  address order, that function's gp-relative accesses in its original bytes.
  No such neighbour has a gp-relative access to any symbol in the function's
  listed set. The ledger also records the best `-G0` score of the
  function's body under the existing build, with the reason `-G0` cannot
  produce the listed accesses.
- **(ii) The original compiler agrees** (a calibration use). The original
  PsyQ compiler, run through `tools/cc1psx_wrapper.sh` on the new TU's
  preprocessed source with the build's cc1 flags, is run twice: at `-G8` it
  emits the listed gp-relative accesses, and at `-G0` it emits none of them.
  The ledger banks both outputs and the counts. If cc1psx emits them at `-G0`
  too, or does not emit them at `-G8`, this ruling does not apply. This is a
  calibration use under [[cc1psx-calibration-only]] and
  [[no-compiler-divergence]]: cc1psx is never a build path, the committed
  build compiles with the project's cc1, and the oracle SHA1 decides the
  match.
- **(iii) The new -G8 TU holds only proven functions.** Each function in it
  meets (i) and (ii) on its own. The functions are contiguous in the original
  address order. The TU has no file-scope `__asm__`, `INCLUDE_ASM` or
  `INCLUDE_RODATA` (under `-G8` cc1 buffers function bodies, so those float to
  the top of the TU). The existing screening rule in the table above applies,
  with its 2026-09-26 scope (§ "Screening scope" above): every extern of 8
  bytes or less that the TU references is in gp range (`sdata_syms.txt`) or
  is honestly typed larger than 8 bytes, unless that section's both-ways
  build proves `-G8` leaves every access to it unchanged. Its only
  compile-flag difference from the file it came from is `GP_FILES`
  membership: it keeps that file's `NO_SR_FILES`, `EXPAND_LB_FILES` and
  `EXPAND_LH_FILES` membership exactly.
- **(iv) The neighbours move unchanged.** Every other function of the
  original file moves to the remaining original file or to a new adjacent
  `-G0` TU, in its original order. That includes `INCLUDE_ASM` lines,
  `INCLUDE_RODATA` lines and file-scope declarations. The move is a
  textually identical diff: every moved line appears in the new file exactly
  as it stood in the source file at the commit before the split. A
  respelling that falls under another rule (e.g. an aggregate merge under
  the aggregate-merge entry in [[no-new-park-categories]]) is not part of the
  move: it lands FIRST, in its own earlier commit under its own rule, or is
  proven byte-neutral (full-build SHA1 == oracle) on the unsplit tree before
  the split. The ledger shows the move diff. Every `-G0` TU produced by the
  split has exactly the source file's compile flags: the same cc1 flags and
  the same maspsx flags, so the same memberships in `GP_FILES` (none),
  `NO_SR_FILES`, `EXPAND_LB_FILES` and `EXPAND_LH_FILES`. Every moved
  function's bytes are unchanged in the full build.
- **(iv-a) RODATA_ALIGN2 membership is mechanical.** For EVERY new TU,
  including the `-G8` one, membership in `RODATA_ALIGN2_FILES` is decided by
  one test, from the jump-table census in docs/grind/decisions.md (2026-09-20
  func_800747D8 entry: ASPSX/psylink did not 8-align jump tables, and a file
  whose table sits at an address that is 4 mod 8 needs the list). The TU is
  on the list exactly when it emits at least one jump table whose own
  address in the shipped binary is 4 mod 8, and off it otherwise. The ledger
  records each such table's address in the shipped binary, or that the TU
  emits none.
- **(v) Build files updated verbatim, bb2.ld stays hand-maintained.** The
  Makefile change is limited to the new TU names: `GP_FILES` gains the `-G8`
  TU, and the per-file lists gain the memberships (iii), (iv) and (iv-a)
  require.
  `engine/buildconfig.py` mirrors those lists verbatim in the same commit
  ([[buildconfig-mirror-drift-false-mismatch]]). `bb2.ld` stays
  hand-maintained (never `make setup`). Its only change is object lines for
  the new TUs, each at its place in the original address order within the
  source file's section runs. No other linker-script change (alignment,
  section moves, new sections) is admitted by this ruling. No
  `LINKED_ASM_FUNCS` entry is admitted by this ruling either; a function that
  cannot sit in a C TU needs its own ruling. All build files keep LF line
  endings.
- **(vi) Bytes and review.** `verify-oracle --rebuild` passes (full-build
  SHA1 == oracle) and `engine test` stays green. A fresh layer-2
  `cheat-reviewer` PASS covers the split diff, the build-file diff and the
  (i)-(iv-a) ledger, together with the function's own completion review. The
  Grinder cannot apply this ruling: Makefile, `bb2.ld` and `engine/` are
  outside session scope ([[integration-handoff-self-serve]] denylist), so it
  lands on the manual path.

**What this does not decide.** func_80036140's other build-model change, the
maspsx COMMON-no-gp model (a maspsx behaviour change under
[[no-compiler-divergence]] item 2), is NOT decided; the owner left it for
separate investigation. A `-G8` TU whose match depends on that model is not
admitted by this ruling alone. This ruling does not reopen flag-hunting:
the flag set stays frozen, and `-G8` is admitted only when every condition
(i)-(vi) holds, including (iv-a), never by a measured score improvement. It
pre-decides no landing: func_80034708's split
(memory/grind/func_80034708/integration/) is judged fresh against (i)-(vi),
including (iv-a). Record: docs/grind/decisions.md 2026-09-26 OWNER
RULING — per-file -G8 by proof.

## The 24-flag sweep (empirical, on the hardest case)

saTan0Main was swept across `-O0/-O1/-O2/-O3` + scheduling (`-fno-schedule-insns`,
`-insns2`), `-fno-thread-jumps`, `-fno-delayed-branch`, all four CSE flags,
`-fno-function-cse`, `-fcaller-saves`/`-fno-caller-saves`, `-fno-omit-frame-pointer`,
`-fno-defer-pop`, `-fno-force-mem`, `-fsigned-char`, `-funroll-loops`. Result:
**`-O0` → 9 jalr (cross-jumping off), every `-O1/-O2/-O3` combination → 1 jalr.**
The target's 3 jalr is a *partial* merge that no flag produces — it comes from
source structure. (`tmp/flag_sweep.sh` is the reusable ~10s harness.)

## When flag-hunting is warranted

Effectively never, at our file granularity. The proof above covers all 22 src
files. The only theoretical hole is if the ORIGINAL had finer TU boundaries than
splat's `.c` files AND a bridged function sat in a sub-TU with different flags —
but step 3 rules this out per file (mixed-flag sub-TUs would leave non-bridged
functions unmatched, and none are). Treat "maybe it's the flags" as answered: no.
(Amendment, owner ruling 2026-09-26: this note amends the "answered: no"
conclusion just above. The finer-TU case is exactly what the per-file -G8
ruling above covers, and a finer `-G8` TU is admitted only when every
condition (i)-(vi) of that ruling holds. Outside that ruling the conclusion
stands.)

## Related
- [[no-compiler-divergence]] — the standing HARD RULE: no cc1/maspsx patches, no cc1psx-switch, no fork. The compiler is frozen; this rule (flags) is a corollary.
- [[cross-jump-call-merge]] — the real wall for multi-jalr dispatch functions
- [[compiler-patch-low-roi]] — patching cc1 itself is also low-ROI (measured) — superseded as POLICY by [[no-compiler-divergence]]
- `no-new-regfix-rules` (retired rule, deleted 2026-08-30) — the remaining walls are closed with C structure, not flags
