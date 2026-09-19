# SELF-VET - func_8002DAD0

(Session 8 re-file, ANNOTATION-FIX modality only. Session 7's body (df-free,
same C as session 6) was layer-1 FAILed 2026-09-18 22:09 on a CITATION-ONLY
ground: "the GTE islands fail the cluster rule's dated condition-3 citation
requirement and the vet's island precedent is wrong for three of six islands
(the real in-file precedent is func_8002FC80/func_8002FDB0, not
func_8002E838/func_800203B4)". This session made ZERO changes to any C
construct: the six island comments were rewritten to (a) name each island's
PsyQ macro and its exact inline_c.h/gtemac.h header line range, read
directly against the header copy at
tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/inline_c.h and
.../gtemac.h this session (condition 3, owner ruling 2026-09-02), and (b)
correct the precedent citation: the three islands that share the cop2-
addressing-preamble cluster's own `addu/move $r,%,$zero` + immediate
ctc2/mtc2 idiom (the diagonal gte_ldopv1 preamble, the LZCS/LZCR gte_Lzc
block, and gte_SetRotMatrix) now cite the cluster's own exemplar members
func_8002FC80 (.claude/rules/cop2-addressing-preamble-cluster.md:83) and
func_8002FDB0 (:84) instead of func_8002E838/func_800203B4; the other three
islands (gte_ldopv2 + OP, gte_stlvnl, and the two gte_ldlv0+MVMVA+gte_stlvnl
pairs) keep func_8002E838 (inline_asm_canonical.txt:373) / func_800203B4
(:367) as the correct same-file SDK-macro-body-spelling precedent. A second,
independent citation error was found and fixed in the same pass: two islands
had been commented "gte_ldv0" when the lhu/lhu/sll/or VX0/VY0 pack they
contain is gte_ldlv0's body (inline_c.h:101-110) — gte_ldv0's own body
(inline_c.h:16-20) is a bare lwc2 pair with no such pack, so the old name was
a plain misidentification, not a stylistic choice. Re-measured from scratch
this session with the corrected comments applied to src/code6cac_b.c:
`sandbox func_8002DAD0 --disable all` == 0 (204/204) and `verify-oracle`
build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle,
both re-run after the comment edit, confirming the fix touched no bytes.)

CONSTRUCTS: (1) six canonical GTE/cop2 `__asm__ volatile` islands (partial
gte_SetRotMatrix diagonal preamble, gte_ldv0 + cop2 OP cross product, gte_stlvnl,
LZCS/LZCR leading-zero block, full gte_SetRotMatrix, two gte_ldv0 + MVMVA
(.word 0x4A486012) + gte_stlvnl pairs); (2) reuse of the existing `dist` local
to carry the scaled Z delta before the if/else assigns it its distance value
(FAKE-annotated); (3) split load-then-compound-shift
`dist = *(s32 *)(obj + 0xD0); dist >>= 6;`; (4) `*(((u8 *)&D_8008D118) + idx)`
byte-LUT pointer arithmetic; (5) `s32 sp_tmp;` as the `"=m"` output of the LZCS
island; (6) `s32 lzcr = 0;` initialized then conditionally overwritten.

## T1 semantic purpose:
(1) GTE/cop2 islands: YES - they ARE the computation (cross product, matrix
load, vector rotate, leading-zero count). Removing them removes the function's
behaviour. GCC 2.7.2's MIPS backend emits no cop2 mnemonics at all, so there is
no C form; `canonical func_8002DAD0` (re-run this session) returns ASM-PARTIAL,
29/204 insns canonical-asm, reasons c2/ctc2/lwc2/mtc2/swc2.
(2) `dist` reuse: NO independent semantic purpose - the value carried is real
and consumed (squared into dist_sq, then stored to obj+0xD0), but a fresh
block-local would produce identical BEHAVIOUR. It is a codegen-control choice
and is declared as such, FAKE-annotated, under the frozen-list variable-reuse
family. Honest answer: this is the one construct in the diff that exists for
codegen reasons.
(3) split load/shift: the value is real and used; `dist >>= 6;` is an ordinary
compound assignment. It is not dead and not a no-op.
(4) LUT pointer arithmetic: YES - it is the table read the function performs.
(5) `sp_tmp`: YES - it is the memory the LZCS island writes LZCR into and the
next statement reads (`lzcr = sp_tmp;`). It is written AND read.
(6) `lzcr = 0`: YES - it is the value used when `dist_sq < 0` skips the island.

## T2 human-programmer:
(1) Yes - this is how PsyQ's own libgte inline macros are written, and it is
character-identical to the two already-authorized siblings in this same file
(func_8002E838, inline_asm_canonical.txt:373) and TU-group (func_800203B4, :367).
(2) Partly. A programmer writing this function from its spec could plausibly use
one local for "the Z distance component" and then "the distance" - both are
lengths - but a reader could also ask "why is dist holding dz here?". I do NOT
claim it passes T2 unaided; that is exactly why it is declared under a
sanctioned family with a FAKE annotation rather than presented as ordinary C.
(3) Yes - `x = *p; x >>= 6;` is everyday C and is explicitly ordinary per the
split-init / compound-assignment feedback entry.
(4) Yes - it is the shipped idiom for this symbol in this very file: the matched
COMPLETED-C func_800274BC reads the same LUT the same way at
src/code6cac_b.c:284, as does the matched func_8002D320. D_8008D118 has no
header declaration anywhere in include/; the TU-local `extern u8 D_8008D118;`
at src/code6cac_b.c:278 is what every sibling in this TU uses.
(5), (6) Yes - ordinary.

## T3 GCC-internals justification:
(1), (3), (4), (5), (6): NO GCC-internals reasoning is load-bearing. The islands
are justified by "GCC has no cop2 form"; the rest by program logic.
(2): YES, and it is stated openly. The mechanism is GCC 2.7.2
`global.c:1670 set_preference` + `global.c:828 expand_preferences`: a
single-basic-block temp is local-alloc'd to a hard register ($a1), which makes
set_preference stamp $a1 onto the allocno of the dying `dz*dz` product; because
that product dies in the very insn that defines dist_sq and the two allocnos do
not conflict, expand_preferences merges the preference onto dist_sq, and
find_reg's preference loop (global.c:1130) then overrides its natural ascending
pick of $a0. A variable referenced in more than one basic block is a GLOBAL
allocno, so `reg_renumber` is -1 during global_conflicts and no preference is
stamped at all. This was READ out of the tree's own instrumentation
(BB2_FINDREG_DEBUG=75 -> `own_copy_prefs:` empty, `own_full_prefs: 5`) and the
-da .lreg/.greg dumps, not inferred. Because a GCC internal IS the mechanism,
this construct is declared FAKE inside a sanctioned family rather than defended
as ordinary C.

## T4 permuter/search provenance:
No permuter and no automated search produced any part of this diff. Session 4's
permuter workspace never ran a single iteration (banked KILLED) and session 5's
enumerator found nothing. The change came from reading global.c and the
instrumented find_reg dump, and was then verified by direct measurement: a
CONTROL variant (memory/grind/func_8002DAD0/rejected/s6-fresh-blocklocal-control-score6.c
- an identical fresh local used only in that one block) measured 6, while the
multi-basic-block carriers measured 5
(rejected/s6-dist-reuse-unsplit-load-score5.c) and then 0. The mechanism, not
the spelling, is what moves the bytes.

## T5 family check:
(1) Canonical GTE/cop2 inline asm - the function is an owner-enumerated member
of the cop2-addressing-preamble cluster
(.claude/rules/cop2-addressing-preamble-cluster.md:76 names func_8002DAD0 by
name), and the islands are character-identical to the authorized func_8002E838 /
func_800203B4 spellings. No hardcoded-$N injection outside a granted island
shape, no register pins, no scheduling barrier, no `asm("sym")` alias.
(2) Variable reuse for codegen control - the FIRST entry on the frozen
SOTN-accepted list, whose text names RA explicitly; the borrow is gated by
staged-value-reused-variable's six bounds, each checked below.
(3) Not a forbidden family: no dead store (the value is read), no dead local, no
constant holder, no redundant width cast, no frame coercion.
(4) Not a declaration pun in the aggregate-merge sense: D_8008D118 is a flat
byte LUT (census g_isqrt_lut) with NO header declaration anywhere in include/;
the read spelling here is byte-for-byte the shipped spelling of the same LUT in
the already-matched COMPLETED-C functions of this same TU. Re-declaring it as
`extern u8 D_8008D118[];` is a TU-wide edit touching already-matched functions
and is flagged here for the integration step rather than done blind.
(5), (6) Ordinary C, no family.

## T6 naming-announces-intent:
No `pad`, `dummy`, `unused`, `spill`, `_buf`, `tail`, `slack` or `_frame_pad`
anywhere. `dist`, `dist_sq`, `angle1`, `angle2`, `lzcr`, `shift`, `tbl`, `mat`
all name real values. `sp_tmp` names the stack slot the LZCS island writes and
the next line reads - it is written AND read, not a declaration-only carrier.
Every local in the diff is both written and read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "- **Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with "FAKE but makes register allocation work" comments."
  PRECEDENT: .claude/rules/no-new-park-categories.md:185

  FAMILY: Staged value through a reused variable (the borrow gate for the above)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:3

  Bounds check against that rule's "exact bounds" section
  (.claude/rules/staged-value-reused-variable.md:58):
  (1) The staged value IS real and used - it is squared into dist_sq on the next
      line and stored to *(s32 *)(obj + 0xD0) on the line after.
  (2) `dist` already exists for a real job - it carries the distance passed to
      ratan2(*(s32 *)(obj + 0xCC), dist) and is assigned in BOTH arms of the
      if/else. It was NOT invented to be borrowed.
  (3) The borrow is provably safe: `dist` has no prior value at the staging
      point (it is unassigned until here), and the staged Z delta is fully
      consumed by the `*(s32 *)(obj + 0xD0) = dist;` store that precedes the
      if/else, so nothing reads it after `dist`'s next real assignment.
  (4) Annotated - see ANNOTATION-CONFORMANCE below.
  (5) Last resort with receipts - five prior sessions, 11 banked instance kills
      and an exhaustive tool sweep, all at floor 6; see
      memory/grind/func_8002DAD0/hypotheses.md s3 and s5.
  (6) Everything else still applies; nothing else in the diff leans on this rule.

  FAMILY: Canonical GTE/cop2 inline asm (cop2-addressing-preamble cluster)
  SCOPE: "The 2026-08-17 owner CLUSTER ruling for func_8002FDB0 covers a 28-function family in the 0x8001-0x8003 band sharing the `addu $t4,$aN,$zero` + cop2 hand-asm idiom. Membership list built and verified here; each member inherits the canonical-asm disposition subject to the same mechanical per-function check, applied by the Judge without re-escalation."
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:76

  Per-island precedent (corrected this session - the prior file cited
  func_8002E838/func_800203B4 for all six islands, which layer-1 FAILed as
  wrong for three of them):
  - Diagonal gte_ldopv1 preamble, LZCS/LZCR gte_Lzc block, gte_SetRotMatrix:
    these three share the cluster's OWN `addu/move $r,%,$zero` -> immediate
    ctc2/mtc2 addressing-preamble idiom (the exact shape the cluster's
    membership table exhibits). PRECEDENT: func_8002FC80
    (.claude/rules/cop2-addressing-preamble-cluster.md:83) / func_8002FDB0
    (:84).
  - gte_ldopv2 + OP, gte_stlvnl, and the two gte_ldlv0+MVMVA+gte_stlvnl
    pairs: character-identical SDK-macro-body spellings already authorized
    in this same file. PRECEDENT: inline_asm_canonical.txt:373 -
    func_8002E838; inline_asm_canonical.txt:367 - func_800203B4.

  Condition-3 (owner ruling 2026-09-02) macro+header citations, verified
  against tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/ this
  session:
  - gte_ldopv1: inline_c.h:210-219
  - gte_ldopv2: inline_c.h:221-226; OP command 0x0170000C (`.word
    0x4B70000C`), same encoding already committed at
    src/code6cac_b.c:1717-1718 / src/display.c:2657
  - gte_stlvnl: inline_c.h:1111-1117
  - gte_Lzc: gtemac.h:174-178 = ldlzc inline_c.h:228-231 + 2x nop
    :1346-1347 + stlzc :1318-1322
  - gte_SetRotMatrix: inline_c.h:297-310
  - gte_ldlv0 (corrected from the prior file's "gte_ldv0" - gte_ldv0's own
    body at inline_c.h:16-20 is a bare lwc2 pair, not this lhu/lhu/sll/or
    pack): inline_c.h:101-110
  - MVMVA sf=1/mx=rotation/v=V0 (`.word 0x4A486012`), the gte_rtv0-class
    encoding already committed at src/code6cac_b.c:1543/1902/1985 and
    src/code6cac.c:2099

ANNOTATION-CONFORMANCE: the diff carries exactly one /* FAKE */ construct, the
`dist` reuse, annotated in full in memory/grind/func_8002DAD0/candidate.c and
identically in src/code6cac_b.c:

    /* FAKE: the scaled Z delta is staged through the function's existing
     * `dist` local (whose distance value is only assigned by the if/else
     * below, so `dist` is dead at this point) instead of a fresh
     * block-local temp, mechanism: GCC 2.7.2 global.c expand_preferences
     * (tools/gcc-2.7.2/global.c:828) - a single-block fresh temp is
     * local-alloc'd to a hard reg ($a1), so global.c set_preference
     * (tools/gcc-2.7.2/global.c:1670) stamps $a1 onto the allocno of the
     * dying `dz*dz` product, and expand_preferences then merges that
     * preference onto dist_sq's allocno (the product dies in the insn that
     * defines dist_sq and the two do not conflict), overriding find_reg's
     * natural ascending pick of $a0 that the target uses; a variable
     * referenced in more than one basic block is a GLOBAL allocno
     * (reg_renumber == -1 during global_conflicts), so no preference is
     * stamped at all and dist_sq lands in $a0,
     * lever-exhaustion: memory/grind/func_8002DAD0/hypotheses.md s3 + s5 -
     * 11 banked instance kills (addition-operand order, store/compute
     * reorder, both compound-assignment splits, fresh named intermediate,
     * local-declaration order) plus the exhaustive tools/spelling_enum.py
     * sweep of both flat blocks touching dist_sq/dist, all measured 6. */

It carries all three required parts: WHAT (the Z delta staged through the
existing `dist` local instead of a fresh temp), MECHANISM (a named GCC pass -
global.c expand_preferences, fed by set_preference, consumed by find_reg's
preference loop), and LEVER-EXHAUSTION (the hypotheses.md s3/s5 ledger entries,
verifiable against the ledger rather than asserted here).

MEASUREMENTS (re-run from scratch this session, candidate re-applied to src):
`sandbox func_8002DAD0 --disable all` -> score 0, target_insns 204,
build_insns 204, cheat_asm_stripped 21. All 21 stripped constructs are OTHER
functions' constructs elsewhere in code6cac_b.c: the stripped source the sandbox
actually compiled (tmp/sandbox/func_8002DAD0/src/code6cac_b.c) still carries all
11 `__asm__` tokens of func_8002DAD0's body, so the 0 is measured WITH this
function's canonical islands intact, exactly as the canonical gate classes them.
`verify-oracle` -> ok true, build_matches true, build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked.

CONDITION-3 CONFORMANCE (this session's fix, re-verified against
src/code6cac_b.c as committed to the working tree): every one of the six
island comments now names its PsyQ macro AND its exact header line range
(inline_c.h / gtemac.h), per the corrected list above. Nothing outside each
named macro body is swallowed into any island; the addressing-preamble
prefixes (`move`/`addu $12,%0,$zero`) are disclosed as the cluster's own
widened idiom, not attributed to SDK macro text.

INTEGRATION NOTE (a heads-up for the merge step, not a blocker claim):
func_8002DAD0 has no row of its own in inline_asm_canonical.txt yet. Its
character-identical siblings (func_8002E838 :373, func_800203B4 :367) carry
operator-written rows added after their Judge PASS. That file is outside a grind
session's allowed surface, so this session did not touch it.
