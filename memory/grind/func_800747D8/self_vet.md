# SELF-VET — func_800747D8   (s10, 2026-09-20)

Scope note: this session's outcome is `owner-gated` (INTEGRATION HANDOFF), not
`candidate-ready`, because the sandbox cannot reach 0 on this residual by
construction (see below) — the proof instrument was the FULL build, which
matched the oracle SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. The vet is
written anyway because the operator must run a fresh layer-2 cheat-reviewer on
this exact body before `queue done`.

CONSTRUCTS: (1) `sound = 4;` duplicated into both inner-switch arms (case 1 and
case 2 of `switch (ret & 0xFF)`) instead of one shared copy after the
`selection_sound:` label — FAKE-annotated; (2) mixed exit forms —
`goto selection_sound;` / `goto confirm;` / `goto tail;` alongside `break;`;
(3) duplicate reads of the global pointer `D_800A36A0` into the locals `base`,
`menu`, `work` and through the `MENU_800747D8` macro; (4) the named intermediate
`u8 row = work->field67;`; (5) the `S_800747D8` struct with explicit `padNN`
members; (6) [CORRECTED 2026-09-21 by the layer-2 reviewer: only `D_800159A0` is
a hand-written const; `jtbl_800159B0` / `jtbl_800159D0` do not exist as source
symbols at all -- they are the COMPILER-GENERATED ADDR_VECs of func_8006E534 and
func_8006ECF4, emitted into this TU once those functions became C. The original
wording below overstated this construct.] the rodata run relocated from
`src/text1a_b_mid_rodata.c` into `src/text1b.c`.

## T1 semantic purpose
(1) REAL on both paths: `sound` is uninitialized on entry to each case arm and
`sound = 4;` is the only definition reaching `func_8005C650(sound, ...)` down
that path. Delete either copy and the program reads an indeterminate value —
this is not a dead store, not a self-assign, not a constant holder.
(2) Each `goto` is the function's actual control flow: the target asm converges
cases 1/2 on a shared sound-playing block and all of state 0/1/2 on a shared
`confirm` block. Removing them would require duplicating those blocks.
(3) Every read of `D_800A36A0` feeds a real load/store; the target re-loads the
global at each of those points (`lw v0, %gp_rel(D_800A36A0)`), so the duplicate
reads are the observable behaviour, not decoration.
(4) `row` is written once and consumed once as the first index of
`D_8009BD20[row][D_800A35DC & 1]`; it is a readability name for a real value.
(5) The pad members are the struct's actual byte layout at those offsets.
(6) The arrays are the binary's real rodata; they were already present in the
tree in exactly this form and are byte-identical after the move.

## T2 human-programmer
(1) Yes. "each menu direction sets the cursor-move sound, then jumps to the
shared play-sound block" is a shape a human writes directly; a reader does not
ask "why is this here?" — the assignment is the case's own behaviour.
(2) Yes — this is a state machine with shared tails; `goto` to a common label is
ordinary decomp and ordinary C.
(3)(4)(5)(6) Yes — all ordinary decomp style.

## T3 GCC-internals justification
Only construct (1) has a GCC-internals component, and it is a PLACEMENT
argument, not an existence argument: the statement must exist on each path for
the program to be correct; what the compiler mechanism explains is why it sits
at the END of each arm rather than hoisted once after the label. Mechanism named
in the annotation: `reg_set_last` (tools/gcc-2.7.2/rtlanal.c:886-888) stops
scanning at a CODE_LABEL, so the store-flag gate at tools/gcc-2.7.2/jump.c:1178
sees `temp3` = a REG rather than a CONST_INT and, with BRANCH_COST == 1 on
R3000, declines the branchless `sltiu`/`sll` fold. Because a GCC pass is part of
the justification, the construct is declared under a sanctioned family with the
mandated FAKE annotation rather than asserted to be plain ordinary C.
Constructs (2)-(6) rest on program logic alone.

## T4 permuter/search provenance
None of these came from a permuter or automated search win. The s3 permuter
campaign (21,884 iterations) found nothing and is banked as a kill; the s4
spelling_enum/sweep_variants runs produced only worse forms. Construct (1) was
derived from the compiler source in s6/s7 and spelled by hand in s9, and its
byte-neutrality is now established by a full-binary SHA1 match, not by a
detector gap.

## T5 family check
(1) duplicated-statement-into-arms (sanctioned; see claim block). Checked
against the adjacent families and it is NOT any of them: it is not a dead store
(the value is read), not a constant holder (no dead local), not a named
intermediate (it is written on two paths), not a do-while(0) wrap, not a
variable-reuse borrow (`sound` is fresh and carries only this value).
(2) cross-jump / mixed exit forms — ordinary C,
`.claude/rules/cross-jump-store-tail-merge.md`.
(3) duplicate reads — ordinary C; `.claude/rules/split-read-defeats-hoist.md`
covers the codegen-motivated version, and here they are simply the target's
loads.
(4)(5)(6) no family needed; no coercion, no volatile, no asm, no pins.
Nothing in the diff is a register pin, hardcoded-`$N` asm, scheduling barrier,
volatile coercion, dead local, frame pad, alias rename, or build-time asm
rewrite.

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, `_buf` or similar
coercion-announcing names on any live local. Names present: `base, sp10, ret,
state, result, i, menu, work, row, sound`. The struct's `padNN` members are
layout holes in a reconstructed struct (standard decomp convention) and are
never referenced. `sp10` is the conventional stack-offset name for the local
whose address is passed to func_800692C0 — it is read and written.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): duplicating a REAL statement into 2+ arms (instead of label-sharing) is legitimate — incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift. SOTN duplicates assignments across arms routinely (7-arm, 11-arm instances). Prerequisites: byte-neutrality verified, lever-exhaustion, FAKE annotation when match-motivated."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:29
  Prerequisite 1 (REAL on its path): yes — see T1(1).
  Prerequisite 2 (byte-neutrality verified): yes — full build SHA1
    62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, and
    `sandbox --disable all --diff` reports 0 source-level hunks with
    build_insns 208 == target_insns 208.
  Prerequisite 3 (lever-exhaustion documented): yes —
    memory/grind/func_800747D8/hypotheses.md, sessions s1-s9, 20 measured
    kills, incl. the s9 `break`-converged single-assignment control (score
    10/205) and the s5/s6 fourteen-spelling class kill at jump.c:1178.
  Prerequisite 4 (FAKE annotation): yes — see ANNOTATION-CONFORMANCE.
  Prerequisite 5 (layer-1 + layer-2 review): outstanding; the operator runs it
    as part of the integration handoff.

ANNOTATION-CONFORMANCE: the following annotation is emitted inline on BOTH
duplicated copies (src/text1b.c during the s10 proof build, and permanently in
candidate.c and integration/text1b.c.proven):

    /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
     * shared copy after `selection_sound:`; mechanism: reg_set_last
     * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
     * `selection_sound:` CODE_LABEL, so the store-flag gate at
     * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
     * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
     * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
     * hypotheses.md s1-s9 (20 measured kills, incl. the s9
     * break-converged single-assignment control at score 10/205). */

It carries all three required elements: WHAT (the duplication and where the
shared copy would otherwise go), MECHANISM (named GCC passes — `reg_set_last`
in rtlanal.c and the store-flag gate in jump.c), and LEVER-EXHAUSTION (the
ledger file and the session range, with the decisive control named).
