# SELF-VET — get_alarm (session 47, 2026-09-04; carried forward from s46 unchanged)

SESSION 47 STATUS: the `volatile_extern_allowlist.txt` scope grant that s46 handed
off now EXISTS (tools/grinder/scope_allow.txt, commit c30a5102 — "get_alarm
volatile_extern_allowlist.txt"), so this session applied the banked diff verbatim
and returns `candidate-ready`. Re-measured live this session with the edits in
place in src/: `sandbox get_alarm --disable all` = score 0 (91/91, rules_dropped 0,
cheat_asm_stripped 147 = unchanged display.c baseline) AND `verify-oracle`
build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches true).
Proof: `tmp/grind/get_alarm/s47/measurements.txt`. The vet body below is the s46
text, unmodified — the diff and the constructs are identical, so the analysis is
identical, and the submitted BODY is the one the Judge PASSed at 2026-09-04 13:48
(hash bc13a6d76f47d945).
Diff: `memory/grind/get_alarm/s46-score0-verified-diff.txt` — the exact tree that
measured `sandbox get_alarm --disable all` = 0 (91/91) AND full-build
SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa this session. Only the
`src/display.c` hunks belong in the candidate diff; the
`volatile_extern_allowlist.txt` hunk is the granted path.

Delta vs the s45 vet: the s46 diff additionally RECONCILES the two duplicate/
contradictory declaration sites the 14:08 Judge ruling ordered fixed —
`src/display.c:721` (`extern s32 D_8009BF78;` -> volatile, matching :750) and
`:733` (`extern s32 (*D_8009BF68)(s32 *, s32);` -> `extern volatile s32
D_8009BF68[];`, matching :758). Removing a wrong function-pointer spelling of a
data symbol in favour of the array spelling the use site requires is a
declaration CORRECTION, not a construct: it adds nothing to the constructs list
below and is byte-neutral outside get_alarm (proven by the full-build SHA1).

CONSTRUCTS: (1) `extern volatile s32 D_8009BF68[];` decl-level volatile on a splat
global (both decl sites, src/display.c:733 and :758); (2) `extern volatile u32
*g_gpu_dma_madr;` pointed-to volatile on an MMIO register pointer (both decl sites,
src/display.c:20 and :756); (3) `extern volatile s32 D_8009BF78;` decl-level volatile
on a splat global (both decl sites, src/display.c:721 and :750); (4) the
function body itself — plain C, one named local `temp_v0` holding the real
`SetIntrMask(0)` result that is genuinely re-read later.

## T1 semantic purpose
(1)(3) YES — `volatile` states that these libgpu queue/state words are read for
their live value and may not be folded or reordered against each other. Removing
either changes the emitted memory ordering, so the constructs are not byte-inert
decoration; they are type-level facts about the objects.
(2) YES — `*g_gpu_dma_madr` is a read of a hardware DMA register (0x1F8010A0). Without
`volatile` the compiler is free to hoist it; that is a real semantic difference on
memory-mapped I/O, independent of any byte goal.
(4) `temp_v0` is written once by `SetIntrMask(0)` and read at `D_8009BF88 = temp_v0;` —
an ordinary consumed value, not a holder.

## T2 human-programmer
Yes for all four. A programmer writing PsyQ libgpu code declares MMIO register pointers
`volatile u32 *` (Sony does; the two sibling declarations three lines away in this very
file already do) and declares interrupt/queue-shared module state `volatile` (Sony's
libgpu declares `_qin`/`_qout` this way, and D_8009BF7C — the other half of that pair —
is already declared volatile in this tree under an existing grant). Nothing here reads
as "why is this here?": the ODD-looking line in the file before this session was the
INCONSISTENCY — one of three adjacent GPU register pointers lacking `volatile` while a
use site 47 lines away cast it back on by hand (`src/display.c:709`).

## T3 GCC-internals justification
The MECHANISM I can describe (MEM_VOLATILE_P blocking combine's single-use address
substitution; volatile MEMs acting as full memory barriers in sched.c's dependence
analysis) is the EXPLANATION of why the bytes move, but it is NOT the justification for
writing the construct. The justification is the object's own nature: MMIO register /
Sony library module state. That is the distinction this test exists to draw, and the
policy that governs it is explicit that it may be measured: Ruling 4 makes "the Sony
object's code is MEASURED unreachable without volatile" the *evidence* of original
intent. No allocator/scheduler internal is being steered by a program-logic-free
construct here; there is no extra local, no extra statement, no extra read.

## T4 permuter/search provenance
None. No permuter ran this session. All three declarations were derived by reading
`tmp/grind/get_alarm/dumps/display.sched` and `.combine` and reasoning about
dependence classes, then measured one at a time (9 -> 6 -> 5 -> 0). Note the honest
history: (3) measured ALONE on the floor-6 chassis scored 12 (worse) and was banked as
killed before (2) was found; it only becomes correct in combination.

## T5 family check
(1) and (3): family = `extern volatile T G;` on census-proven Sony library module state,
Ruling 4 class (frozen-list entry "Sony census module state measured unreachable without
volatile"). Both need their `volatile_extern_allowlist.txt` grant; that is exactly what
this session hands off.
(2): family = type-level MMIO volatile, `.claude/rules/mmio-volatile-type-level.md`,
which requires no annotation and no allowlist. It is NOT "volatile-coercion by cast"
(no cast is added — one is REMOVED in spirit), NOT "by alias-rename" (no second handle),
NOT "by plain extern" on a game-state global (the object is a hardware register pointer
whose target is in the sanctioned 0x1F801000-0x1F802FFF window).
(4): no family — ordinary C.

## T6 naming-announces-intent
No construct is named `pad`, `dummy`, `slack`, `_buf`, `tmp`, `spill`, or similar.
The only local is `temp_v0`, pre-existing, once-written once-read with a real value.
No `(void)&x`, no unused array, no dead local.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: type-level MMIO volatile
  SCOPE: "OWNER RULING 2026-07-01: volatile on hardware-MMIO-range addresses (0x1F801000-0x1F802FFF I/O registers) is legitimate TYPE-LEVEL hardware semantics - all read/write shapes, including single-read probes. No use-site shape test, no FAKE annotation (it isn't fake). SOTN declares register pointers volatile once; GetIntrMask/_status/bit-test probes are volatile by default. Game-state globals keep the two-prong gate."
  PRECEDENT: .claude/rules/mmio-volatile-type-level.md:5

  FAMILY: Ruling 4 — ground-truth-codegen volatile class (census-proven Sony library module state)
  SCOPE: "For census-proven Sony library module state (symbol identity reloc-proven against the verbatim-linked SDK object), where the Sony object's code is MEASURED unreachable without `volatile` (non-volatile build demonstrably collapses ordering/re-reads), volatile is legal as ORIGINAL SEMANTICS — no in-binary IRQ-writer prong required."
  PRECEDENT: c80d976e

  FAMILY: Ruling 4 applied to THIS function's D_8009BF68 by a Judge PASS on record
  SCOPE: "Ruling 4 GRANTS `extern volatile s32 D_8009BF68[];` here, with NO IRQ prong required (prong 1 holds anyway)."
  PRECEDENT: docs/grind/decisions.md:22292

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. None of the three declarations is a
FAKE-annotated last-resort lever: (2) is sanctioned at type level with no annotation
required by its own rule, and (1)/(3) are Ruling-4 declarations of original semantics,
whose conformance surface is the `volatile_extern_allowlist.txt` entry (with its audit
comment), not a `/* FAKE */` comment. The frozen-list entries for both families are
annotation-free; the allowlist grant IS the record.

NOTE: the MMIO scope sentence above is quoted verbatim from that rule file's own
`description:` line (`.claude/rules/mmio-volatile-type-level.md:5`), re-read this
session. The Ruling-4 scope sentence is quoted verbatim from the ruling text recovered
with `git show cd19d7a2^:docs/closer/rulings.md` (lines 68-83); the file itself was
deleted as dead-era docs in cd19d7a2, which is why the PRECEDENT is given as the
granting commit hash `c80d976e` rather than a path — the frozen-family table's
`docs/closer/rulings.md:68` citation is STALE and would fail the dead-path check.
