# SELF-VET — func_800324D0  [s22, 2026-09-01, supersedes the s5/s20 vet]

CONSTRUCTS: (1) the loop tail `c = *ptr; ptr++;` together with its back-transfer
(`continue`) duplicated into all twelve command arms of the switch instead of
being reached by falling out of the switch. THAT IS THE ONLY NON-ORDINARY
CONSTRUCT IN THE DIFF. Everything else is plain C, and the diff DELETES
constructs relative to the previous candidate: the s5 staged-tail FAKE
(`cmd = *ptr; c = cmd;`), the borrowed `u32 cmd` local, and the four
`register ... asm("...")` pins that main was still carrying are all gone.

## T1 semantic purpose
The duplicated tail is REAL on every path it appears on. Each command arm must
fetch the next command byte and advance the stream walker; delete the copy from
an arm and the parser reads the same byte forever. It is not a dead store, not a
self-assign, and not a discard: `c` is read by the loop test on the very next
edge and `ptr` is read by the next iteration. The construct's only *redundancy*
is with the shared tail below the switch, i.e. it is a code-layout choice
between two semantically identical spellings, which is exactly the situation the
family governs.

## T2 human-programmer
Yes. "Each case, having consumed its operand byte, reads the next command byte
and continues" is a normal hand-written command-stream-parser idiom. A reader
does not ask "why is this here?" — it is the loop's own advance. SOTN ships the
same shape routinely at larger arm counts (7-arm and 11-arm duplications,
`src/boss/bo4/doppleganger.c`) and specifically duplicates transfer-terminated
tails into arms (`e_shop.c:986-1009`, `vs_vh.c:69-128`).

## T3 GCC-internals justification
Stated honestly: the CHOICE of the duplicated spelling over the shared-tail
spelling is match-motivated, and the mechanism is named — flow.c's `reg_n_refs`
census counts the duplicated walker references before global.c's
`allocno_compare` ranks allocnos, so the walker pseudo's reg_n_refs goes from 24 to
96 while the operand carrier stays at 26 (measured: base
tmp/grind/func_800324D0/s22/lreg_seg.txt `Register 73 used 24 times` /
`Register 76 used 26 times`; this body s22/lreg_seg_p3.txt `Register 73 used 96
times` / `Register 75 used 26 times`), so the walker outranks the carrier and
takes $v1; jump2's cross-jump pass runs after reload and
re-merges the thirteen identical tails, so no duplicated reference materializes
(68 == 68, byte-identical, full build SHA1 == oracle). This does NOT make it a
Test-3 cheat: the sanctioned family exists precisely for this mechanism and
names it in its own scope text ("the duplication's surviving effect is the extra
`reg_n_refs` count flow.c records (allocno-priority lift for global RA)"), which
is why it mandates a FAKE annotation rather than forbidding the construct. The
annotation is present and names the passes.

## T4 permuter/search provenance
Not a search find. No permuter ran this session. The construct was derived
by hand from the ledger: s18's exact find_reg enumeration established that the
target disposition needs the walker's allocno to outrank the carrier's, and
s20's foreclosure of that channel rested on the claim that promoting the walker
to ~50 refs "costs >= +6 instructions on a budget that is exact at 68". That
claim is what this session falsified — cross-jump-remerged references are free —
and the falsification predicted the construct before it was measured.

## T5 family check
The claimed family is duplicated-statement-into-arms, sanctioned by owner
rulings 2026-07-01 and 2026-08-06 (both blocks cited below). Checked against the forbidden catalog line by line: no register-asm
pins (four DELETED here), no `__asm__` of any kind, no scheduling barrier, no
volatile in any spelling, no alias rename, no unused local array or frame
coercion, no dead-param-assign, no dead conditional store, no empty-body `if`,
no `if (1)`, no dead goto label pad, no DImode chain, no invented local at all
(the diff has one FEWER local than the previous body), no opaque constant
holder, no width casts, no linker/rodata reordering. It is specifically NOT the
BANNED base/ff overlapping-live-range construct or any respelling of it: nothing
is declared to overlap anything, no variable is invented to carry a second copy
of a value, and the two banned constructs' distinguishing feature (a second
carrier for the operand byte) is absent — `val` is still the single carrier.

## T6 naming-announces-intent
Locals are `ptr`, `c`, `val` — semantic names for the stream walker, the command
byte, and the operand byte. Every one is written and read on a live path. No
`pad`-style, `dummy`, `unused`, `spill`, `tail`, or `slack` naming, and the one
parameter keeps its semantic name `pad` (it is the pad struct, not padding).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "The family DOES cover duplicating a multi-statement tail that ends in a control transfer — including a loop tail with its conditional branch — when every prerequisite holds (byte-neutral via cross-jump re-merge, exhaustion documented, FAKE-annotated, dual review)."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:82
  FAMILY: duplicated-statement-into-arms (reg_n_refs-lift leg)
  SCOPE: "the duplication's surviving effect is the extra `reg_n_refs` count flow.c records (allocno-priority lift for global RA), and"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:20

Prerequisite audit (rule lines 47-59):
  1. statement REAL on its path — yes (T1 above).
  2. byte-neutrality verified — sandbox --disable all = 0 with build_insns 68 ==
     target_insns 68 and rules_dropped 0 against a reference object rebuilt from
     pristine HEAD (tmp/grind/func_800324D0/s22/build_head_reference2.log then
     s22/sandbox_final.log), plus full build SHA1 ==
     62efab4f73f992798c43e8c730aa43baa10bb4fa (s22/build_sha1_final.log).
  3. lever-exhaustion documented — memory/grind/func_800324D0/hypotheses.md
     s1-s21 and evidence.md; twenty-one prior sessions, six modalities, four
     permuter campaigns, the whole demote-the-carrier and RA-seat channels.
  4. FAKE annotation present — see ANNOTATION-CONFORMANCE below.
  5. layer-1 + layer-2 review — this vet is the layer-1 input; layer 2 is the
     operator's fresh cheat-reviewer before the commit lands.

ANNOTATION-CONFORMANCE: the single FAKE comment sits immediately above the
switch that carries the twelve duplicated tails and reads, verbatim from
src/code6cac_b.c:

    /* FAKE: the loop tail (`c = *ptr; ptr++;` + its back-transfer) is
     * duplicated into all twelve command arms instead of being reached
     * by falling out of the switch, mechanism: flow.c's reg_n_refs
     * census counts the duplicated walker references before global.c's
     * allocno_compare ranks the allocnos: the walker pseudo's
     * reg_n_refs goes 24 -> 96 while the operand carrier stays at 26
     * (measured, .lreg dumps), so the walker now outranks it and takes
     * $v1 instead of $a2, while jump2's
     * cross-jump pass runs after reload and re-merges the thirteen
     * identical tails, so not one duplicated reference materializes
     * (68 == 68, byte-identical, full SHA1 == oracle),
     * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md s1-s21
     * (the whole demote-the-carrier channel, the RA-seat channel, four
     * permuter campaigns) and evidence.md [s22] */

It carries all three required parts: WHAT (the duplicated loop tail into twelve
arms), MECHANISM (named passes: flow.c reg_n_refs -> global.c allocno_compare;
jump2 cross-jump after reload), LEVER-EXHAUSTION (the s1-s21 ledger pointer).
