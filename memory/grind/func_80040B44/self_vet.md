# SELF-VET — func_80040B44

Diff under review: `src/text1a_pre.c`, function body of `func_80040B44` only
(git status shows exactly one tracked source file modified plus the engine's own
`metrics/events.jsonl`). No regfix.txt / asmfix.txt / .claude/rules / engine /
tools / Makefile / *.ld touched. No inline `__asm__` anywhere in the file
(`grep -c "__asm__" src/text1a_pre.c` = 0). Honest measurement with this exact
body in place: `sandbox func_80040B44 --disable all` → `"score": 0`,
`"build_insns": 93`, `"target_insns": 93`.

CONSTRUCTS: integer-domain address arithmetic at five sites (`a3 * 0x68 + (s32)t7`,
`a3 * 4 + (s32)&seen[0]`, `(t2 << 2) + (s32)arg0 + 0x1A34`); one shared loop counter
`i` reused by the two init loops; two fresh block-scoped pointer locals `p1`/`p2`,
one per init loop; the walking-pointer advance `t3 = (s32 *)((u8 *)t3 + 2);` placed
at the top of the main loop body instead of in a preheader statement; early exit
`goto done;` to a single trailing label; else arm addressed through `t5` (`t5 + 2`,
`t5 + 0x58`) instead of through a separate `t4` walking pointer; block-scoped
`a3` / `a1` / `t2` / `slot` / `t0` / `a2` / `end` loop temporaries; pre-existing
`Copy16` / `Copy8` struct-copy typedefs (unchanged from the committed chassis).

REMOVED by this diff (net simplification): the `s32 one = 1;` constant holder that
s1 flagged as the forbidden-catalog "opaque variable to defeat single-bit transform"
spelling, and the explicit `s32 *t4` walking pointer. Both deletions were measured
byte-neutral/byte-improving, not asserted.

## T1 semantic purpose
Every construct changes what the function computes or how it addresses memory, and
removing any of them changes behaviour, not just bytes.
- Integer-domain arithmetic: computes the address that is then dereferenced/stored.
  Delete the operand and the store has no destination. Real.
- Shared counter `i` / fresh pointers `p1`,`p2`: `i` is the live trip counter of each
  init loop; `p1`/`p2` are the live cursors being stored through. Removing any makes
  the loops non-terminating or storeless. Real.
- Advance at loop top: the loop walks a halfword stream; without the advance the loop
  re-reads the same element forever. Real — and its POSITION is a real semantic
  choice (advance-then-read vs read-then-advance), not a marker.
- `goto done;`: the documented early-exit when the stream is empty. Removing it makes
  the function read past an empty list. Real.
- Addressing the else arm through `t5`: `t5 + 0x58` is literally the address written.
  Real.
There is no construct in this diff whose removal leaves the function's behaviour
byte-identical. Nothing here exists to pad, park, pin, hold, discard, or alias.

## T2 human-programmer
Yes to all of it, and in several places the diff is what a human would write and the
PRIOR body was not. A programmer writing "walk a halfword stream of (id, value)
pairs until the 0xFFFF terminator" naturally writes the advance at the top of the
body. A programmer with two independent clear-loops naturally reuses one counter and
gives each loop its own cursor. A programmer who has `t5` in hand does not invent a
second pointer that is permanently `t5 + 0x58`; the previous body's `t4` is the thing
a reader would ask "why is this here?" about, and this diff deletes it. Same for
`s32 one = 1;` — deleted in favour of `*a1 = 1;`.
The one construct a reader might query is the integer-domain address arithmetic
(`(s32)t7 + a3 * 0x68` rather than `(u8 *)t7 + a3 * 0x68`). That is idiomatic,
extremely common PS1/embedded C — computing an address in a scalar and casting — and
it is semantically identical, self-explanatory at the point of use, and not a marker
of any kind. It is a style choice among two spellings a human writes routinely.

## T3 GCC-internals justification
This is the test this vet has to answer most carefully, because the DISCOVERY path
for three of the four changes went through GCC internals (`pointer_int_sum`,
`allocno_compare`, `move_movables`, `reorg.c`) and the ledger records that reasoning
in full. Discovery mechanism is not the same thing as justification, so:

None of the constructs REQUIRES a GCC internal to explain why it is in the program.
Each one is justified by the program logic alone (T1/T2 above), and each one is a
form a human writes for readability reasons with no compiler in mind. What the GCC
internals explain is only which of several BEHAVIOURALLY IDENTICAL, EQUALLY NATURAL
spellings the original author happened to use — they are evidence about the original
source, not a mechanism the construct exploits. Concretely: `advance at loop top` vs
`advance in preheader + advance at loop bottom` are two ordinary ways to write the
same walk; internals told me which one the 1998 author wrote. Nothing in the diff is
inert-as-program-text-but-load-bearing-as-codegen, which is the actual cheat signal
this test is aimed at.

The strongest evidence that this is not a lever hunt: the constructs in this diff are
strictly FEWER and SIMPLER than the chassis they replace. A coercion pass adds
constructs; this one net-removed two (`one`, `t4`) and added no scaffolding. In
particular the single missing instruction (`addiu t3,t3,-2`) — the one place a lever
would have been reached for, and which the retired regfix @175 faked — is produced
here by reorg.c as ordinary compiler output from an ordinary loop shape, with NO
source statement corresponding to it at all. The previous body's literal
`t3 = t3 - 2;` (a dead store that GCC's flow.c deleted, verified in the dumps: the
insn is present through .cse2 and gone in .flow) is DELETED by this diff.

## T4 permuter/search provenance
No permuter, no auto-search, no randomized mutation was used at any point this
session. Zero permuter runs. Every form was hand-derived from the target
disassembly, the m2c reference partition banked in s1, and cc1 `-da` dumps read
directly (`tmp/grind/func_80040B44/dumps/`). Each of the seven forms was a stated
hypothesis with a predicted outcome, measured one at a time by
`sandbox --disable all`; the six that failed are banked in `rejected/` with the
measured floor and the reason. Nothing in the diff survives because a detector
missed it — the diff contains no construct that any detector targets.

## T5 family check
No construct in this diff matches any entry in the forbidden-family catalog, and —
this is the point — none needs a SANCTIONED family either. Checked one by one against
the catalog: no register-asm pins; no `$N` asm injection (no asm at all); no regfix
insert; no scheduling barrier; no INLINE_MOVE_ALIASING; no volatile of any spelling
(no `volatile` keyword in the body); no unused local array (`seen[18]` is written by
init loop 1 and read by `*a1 == 0` — fully live); no `&local` or `(void)local`
discards; no dead-param-assign; no dead-conditional-store; no empty-body `if`; no
`if (1)`; no dead goto label-pad (`done:` is a reached early-exit target); no DImode
chain; no accumulator-plus-shared-label trick; no param-local alias; no `s32 one = 1;`
opaque holder (DELETED by this diff); no lowercase `asm()`; no `asm("sym")` rename; no
redundant width casts; no `bb2.ld` reorder.
Nor does it match any sanctioned family, because it does not need to: variable reuse
here (`i` across the two init loops) is a plain live counter reused for its ordinary
purpose in two sequential loops, which is just C, not the "borrow a local to carry a
second unrelated value" construct that `defeat-licm-hoist-var-reuse.md` governs — `i`
carries the same kind of value, a loop counter, in both loops, and both writes and
both reads are live. The `goto done;` is an ordinary early return path, not the mixed
`goto endK` + inline return codegen device. Nothing is annotated, nothing is claimed.

## T6 naming-announces-intent
No name in the diff is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`,
`tail`, `slack`, or `_frame_pad`, and no local's only uses are discards, address-of,
or declaration. The names are register-derived (`t3`, `t5`, `t7`, `a0_val`, `a1`,
`a2`, `a3`, `t0`, `t2`) — inherited from the committed chassis and from m2c's output,
the project's normal convention for a function whose symbols are not yet recovered —
plus `seen`, `slot`, `end`, `i`, `p1`, `p2`. Every one of them is written and read.
`p1`/`p2` are cursors that are stored through; `i` is compared and decremented; `seen`
is stored and loaded. Nothing announces coercion intent because nothing coerces.

SANCTIONED-FAMILY-CLAIMS: none
  No construct in this diff belongs to a sanctioned SOTN family, so no family is
  claimed and no scope sentence or precedent is cited. The diff is ordinary C: loop
  restructuring, local-variable partitioning, integer-domain pointer arithmetic, and
  the deletion of two constructs (`s32 one = 1;` and the redundant `t4` pointer) that
  the previous chassis carried. If a reviewer believes any construct here does belong
  to a family, the honest reading is that it belongs to NONE — I am not invoking a
  carve-out to license anything, I am asserting the diff needs no carve-out.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
  There is no `/* FAKE */` construct in this diff, and none is required: no sanctioned
  family is claimed, no coercion is present, no lever was spent. The one instruction a
  FAKE would classically have been reached for — the exit-path `addiu t3,t3,-2` that
  the retired regfix @175 injected — is emitted by reorg.c from ordinary loop code,
  with no source statement behind it.
