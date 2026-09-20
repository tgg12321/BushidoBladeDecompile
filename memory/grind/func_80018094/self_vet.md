# SELF-VET — func_80018094  (s10 (chassis session-count), 2026-09-20, annotation-fix — filed WITH a `candidate-ready`)

**THIS SESSION'S FIX-UP SCOPE.** The 2026-09-20 01:23 layer-1 review FAILed the prior
submission on a DISPOSITION/CITATION-ONLY defect: every executable construct — including the
three cop2 inline-asm islands, which are owner-granted for THIS function by the
cop2-addressing-preamble-cluster census — was independently re-verified legitimate inside its
family, but the submission filed itself under the wrong disposition: the candidate.c header
headlined "MATCHED IN PURE C" and this self-vet's CONSTRUCTS list omitted the three asm islands
entirely while T5 affirmatively denied any hardcoded-`$N` asm — contradicting the dated owner
ruling docs/grind/decisions.md 2026-09-15 "OWNER RULING — the candidate-path no-progress
tripwire + a registry row for func_80018094", Ruling 3, which states in terms that
func_80018094's honest finished bucket is **COMPLETED-INLINE-ASM-CANONICAL, not COMPLETED-C**.
This session changed COMMENTS ONLY: candidate.c's header now states the correct disposition and
cites Ruling 3 by name; this self-vet's CONSTRUCTS list now enumerates the three cop2 islands as
constructs 9-11 with their own T1-T6 rows and a SANCTIONED-FAMILY-CLAIMS block citing
cop2-addressing-preamble-cluster.md; T5's summary paragraph no longer denies hardcoded-`$N` asm
outright — it now states plainly that the three islands ARE inline asm, granted under that named
family, and that no OTHER family (register pin, scheduling barrier, un-granted `$N` injection) is
present. `git diff src/code6cac.c` contains exactly one hunk, the func_80018094 body replacing
its `INCLUDE_ASM`, with zero other lines touched (verified: 226 insertions / 1 deletion, no
mojibake em-dash sequences found in the file). The executable BODY is byte-for-byte the
previously-Judge-reviewed body (hash f5e49e3dddc24d62 lineage); no construct was added, removed,
renamed or reordered, and every FAKE annotation below is unchanged. Re-measured this session:
`sandbox func_80018094 --disable all` == 0 (target_insns 153, build_insns 153, rules_dropped 0,
cheat_asm_stripped 20).

**Prior fix-up (2026-09-16 lineage, unchanged this session).** An earlier annotation-fix session
corrected two stale citations: the Set*Matrix islands' "memory"-clobber precedent now cites
src/code6cac_b.c:1116-1123 (func_8002D320's lwc2-read island, `"r"(vin) : "$12", "memory"`), the
same precedent the committed sibling func_80019310 itself cites (src/code6cac.c:356-361); the
`lw_v1`/`li_v0` precedent now cites src/code6cac.c:778-781 (func_8001A538's identical
`{ s32 lw_v1 = sp_tmp; s32 li_v0 = -2; li_v0 = lw_v1 & li_v0; shift_a = 0x16 - li_v0; }` block);
the `D_8008D118` byte-LUT-read precedent cites src/code6cac.c:757 and :784.

The body under vet is `memory/grind/func_80018094/candidate.c` (= tmp/grind/func_80018094/s10/final.c,
generated from s10/e1.c by adding the FAKE annotations only). Spliced into src/code6cac.c it
measures `sandbox func_80018094 --disable all` == **0** (target_insns 153, build_insns 153,
rules_dropped 0, cheat_asm_stripped 20 — the three inline-asm islands, stripped on BOTH sides),
measured this session on the -mel -msoft-float chassis. The LZC island's operand list is exactly
the granted honest form `: "=m"(sp_tmp[0]) : "r"(lut) : "$2", "$12"` — the Judge's binding
constraint from the 2026-09-09 23:11 ruling (docs/grind/decisions.md) is satisfied: no extra,
tied, or clobber operand was added to any `__asm__` in this function, and the pre-island
`move $a0,$a1` comes from ordinary C.

CONSTRUCTS: (1) oversized locals object `s32 sp_tmp[4]` with only element 0 written/read;
(2) `do { ... } while (0);` wrap around the whole else-arm body; (3) `do { ...; goto lzc_done;
} while (0);` around the small arm, so its `if` has NO else; (4) `do { ... } while (0);` around
the LZC arm's body (the third wrap, new this session); (5) the staged island-input copy
`lut = sum_sq;` through the LZC arm's existing `lut` local; (6) the named intermediates
`lut`, `lw_v1`, `li_v0`, `shift_a`, `shift_b`, `dx`, `dy`, `dz`, `sum_sq`, `scale`, `dst`;
(7) the `*(&D_8008D118 + i)` byte-LUT read spelling; (8) the `goto lzc_done` mixed exit;
(9) the `gte_SetRotMatrix(r0)` inline-asm island (5 `ctc2` transfers preceded by the arg-pointer
load sequence); (10) the `gte_SetTransMatrix(r0)` inline-asm island (3 `ctc2` transfers); (11) the
`gte_Lzc(r1,r2)` inline-asm island (`mtc2`/two `nop`/`swc2` plus the `addu $t4,...,$zero` +
`addiu $v0,$sp,0x10` cop2 addressing preamble). Constructs 9-11 are genuine `__asm__` blocks —
this function is NOT pure C, and the honest completion bucket is COMPLETED-INLINE-ASM-CANONICAL
(docs/grind/decisions.md 2026-09-15 Ruling 3), not COMPLETED-C.

## T1 semantic purpose (per construct)
(1) No observable effect — a frame-size device; sanctioned only under the oversized-locals
carve-out, and the target frame equation proves the original declared this object larger than the
4 bytes it writes (ALIGN8(4)+16+16 = 0x28, but the target frame is 0x30). Ablation: `s32 sp_tmp`
scalar scores 8 (tmp/grind/func_80018094/s10/f2.c).
(2) No observable effect — control flow is identical without it. Ablation: 13 (s10/f1.c).
(3) No observable effect on behavior — the arm computes and exits identically. Ablation: 10
(s10/f3.c).
(4) No observable effect — the arm's statements run identically. Ablation: 13 (s10/a0.c).
(5) Semantic: `lut` is READ by the island as its `"r"` input operand. Zero dead code — this is the
value the LZC hardware sees. Without it the island reads `sum_sq` directly and the target's
`move $a0,$a1` never exists (measured across s5-s9, hypotheses.md H26-H46).
(6) Semantic — every one is once- or twice-written and read; `lw_v1`/`li_v0` are the COMPLETED-C
sibling func_8001A538's own names for the identical statements (src/code6cac.c:778-781).
(7) Semantic — it is the actual byte load from the magnitude LUT.
(8) Semantic — it is the arm's exit.
(9)(10)(11) Full semantic effect — these ARE the function's rotation/translation-matrix load and
leading-zero-count magnitude estimate; the cop2 coprocessor has no C-level access path, so the
asm text is not "no observable effect", it is the only way these operations can be expressed at
all. Removing any of the three changes the program's behavior (the GTE state is simply never
loaded / never counted).

## T2 human-programmer
(1) No — a programmer writes `s32 sp_tmp;`. This is why it carries a FAKE annotation and the
frame-math proof.
(2)(3)(4) No — a reader would ask "why is this wrapped?" of all three do-while(0)s. That is
exactly why the do-while-zero-exception family requires the FAKE annotation, and why all three
annotations name the pass and the ablation score.
(5) Yes — staging a value into an existing local one line before it is consumed is ordinary,
readable C, and the SOTN corpus ships the shape verbatim (see the family's census below).
(6) Yes. (7) Yes — it is the existing house spelling for this symbol in this TU, shipped in
already-matched bodies (src/code6cac.c:757 and :784, inside the pure-C func_8001A538, plus
src/code6cac_b.c:284 and :747), all against the same `extern u8 D_8008D118;` declaration at
src/code6cac.c:19. Changing the declaration to an array type would break those matched siblings'
pointer arithmetic, so this is not a new pun introduced by this candidate; it is the TU's
established, oracle-verified declaration. (8) Yes.
(9)(10)(11) No — a programmer does not free-write GTE cop2 transfers as inline asm from a blank
page; they call the SDK macro (`gte_SetRotMatrix`, `gte_SetTransMatrix`, `gte_Lzc`). That is
exactly why these are governed by the canonical-asm family (a human WOULD write the macro
invocation; the macro's own body is what compiles to this asm — see cop2-addressing-preamble-
cluster.md's Ruling 2026-09-02 "condition 3 clarified" note, which holds every GPR instruction
here is the named macro's own published text, not an invented device) rather than by the
ordinary-C tests T1-T2 use for constructs (1)-(8).

## T3 GCC-internals justification
Constructs (1)-(4) ARE justified by a named pass rather than by program logic, and each sits
inside a sanctioned family that permits exactly that with a FAKE annotation:
(1) function.c assign_stack_local / mips.c compute_frame_size (get_frame_size raw 16 ->
MIPS_STACK_ALIGN keeps 16 where the scalar form rounds 4 -> 8).
(2) and (4) flow.c life analysis: NOTE_INSN_LOOP_BEG/END raise basic_block_loop_depth
(tools/gcc-2.7.2/flow.c:440-471) and every reference in the region is weighted by that depth
(`reg_n_refs[regno] += loop_depth`, flow.c:2081), which is the numerator of global.c's
`allocno_compare` priority `floor_log2(n_refs)*n_refs/live_length*10000`.
(3) cse.c `cse_end_of_basic_block`'s follow-jumps gate (tools/gcc-2.7.2/cse.c:8100-8125): the
backward walk from the jump target stops on a NOTE_INSN_LOOP_END and carries no `after_loop`
guard, so both cse runs refuse to extend the block into the LZC arm.
(5) is justified by PROGRAM logic first (it is the island's input operand); the pass reference
(reorg.c delay-slot fill parking it in the `beqz` slot, global.c find_reg seating it at $a0) is
the reason the spelling is `lut = sum_sq;` rather than `"r"(sum_sq)` — which is precisely what the
staged-value-reused-variable family sanctions, with its mandatory annotation.
(6)(7)(8) need no pass reference at all.
(9)(10)(11) need no GCC-internals justification at all — unlike (1)-(5), these are not devices
that influence what GCC's optimizer does with surrounding C; they are literal machine
instructions the compiler passes through unchanged. Their legitimacy rests on hand-coded-asm
evidence (splat's handwritten-instruction tags, the redundant `addu $tN,...,$zero` copy signature
GCC 2.7.2 never emits, unfilled cop2 load-delay slots), not on any pass mechanism.

## T4 permuter/search provenance
No permuter and no auto-search ran this session. The closing form was PREDICTED arithmetically
before it was written: this session read `allocno_n_refs` / `allocno_live_length` / `pri` for both
allocnos out of the instrumented cc1's BB2_ALLOC_DEBUG hook on the two banked bodies
(tmp/grind/func_80018094/s10/candidate.allocdbg.txt: pseudo 80 = 14/9/46666 wins;
s10/m1.allocdbg.txt: pseudo 80 = 8/7/34285 loses to pseudo 77's 19/21/36190), solved
`allocno_compare` for the thresholds, identified loop-depth weighting as the only lever that moves
n_refs without moving a seat, and predicted 11/7/47142 versus 21/21/40000. The measured dump
(s10/e1.allocdbg.txt) came out exactly those numbers. Nothing here passes a detector by spelling;
the mechanism is stated, quantified and reproducible.

## T5 family check
(1) dead-vars-local-array, oversized-locals carve-out 2026-07-13 — claimed below.
(2)(3)(4) do-while-zero-exception, owner ruling 2026-07-06 — claimed below. The nested-wrap
prerequisite is discharged by measurement: each of the three wraps was ablated INDIVIDUALLY this
session and each ablation costs (13 / 10 / 13 respectively); no single level and no pair
substitutes for the three.
(5) staged-value-reused-variable, owner ruling 2026-07-03 — claimed below. All six bounds hold:
the value is read by the island (bound 1); `lut` exists for the LZC arm's LUT byte independent of
the staging (bound 2 — it is the named intermediate for `((s32)(lut << 16)) >> (0x13 - shift_b)`,
not a variable invented to be borrowed); the borrow is safe in both directions (bound 3 — `lut`
holds nothing before the staging and the staged value is consumed by the island before `lut`'s
next write, stated in the annotation); annotated (bound 4); last resort with receipts (bound 5 —
nine sessions, hypotheses.md H26-H46, every fresh-local and every declaration-scope spelling
measured); nothing else in the catalog is opened (bound 6).
(6) ordinary C — named intermediates, once/twice-written and read, real values in the target's
bytes.
(7) ordinary C for this TU — the established declaration+use pair, oracle-verified in matched
sibling bodies (precedent cited below).
(8) ordinary C — mixed exit forms, `.claude/rules/cross-jump-store-tail-merge.md`.
(9)(10)(11) cop2-addressing-preamble-cluster, owner ruling 2026-08-17 (widened 2026-09-01,
condition 3 clarified 2026-09-02) — claimed below. func_80018094 is enumerated BY NAME in the
cluster census (.claude/rules/cop2-addressing-preamble-cluster.md:60, SetRotMatrix/long-vector
sub-family, 3 idiom sites) and satisfies the file's 4-point mechanical per-function check: (i)
`sandbox func_80018094 --disable all` == 0, measured this session; (ii) zero register pins, zero
`move %0,%1` aliasing blocks, zero scheduling barriers anywhere in the body (constructs (1)-(8)
are do-while(0) wraps / named locals / a staged copy / a LUT-read spelling / a mixed exit — none
is a pin, alias block, or barrier); (iii) every in-island GPR instruction is either the named
SDK macro's own published text (the `ctc2`/`lw`/`mtc2`/`swc2`/`nop` transfers) or the disclosed
cop2 addressing preamble (the `addu $t4,...,$zero` copies + `addiu $v0,$sp,0x10`) — nothing else
is swallowed into any of the three templates; (iv) bytes proven on main this session, and a
fresh layer-2 cheat-reviewer + `verify-oracle --rebuild` still gate the merge (outside this
session's surface — the operator registry row is the blocker, per the MERGE REFUSED constraint
below).

**THIS IS NOT A "NO CONSTRUCT MATCHES A FORBIDDEN FAMILY" BODY.** Constructs (9)-(11) ARE
`__asm__` blocks containing hardcoded machine instructions with real (non-`%0`-placeholder-only)
register names (`$12`,`$13`,`$14`,`$15`,`$t4`,`$v0`,`$2`) — the literal shape the "hardcoded-`$N`
`__asm__` injection" forbidden family describes at the syntax level. They are NOT that family
because they are the disclosed, owner-granted canonical reproduction of a named PsyQ SDK macro's
own published text (cop2-addressing-preamble-cluster.md), not an invented device to force a byte
no C source produces — the forbidden family is for asm blocks whose ONLY purpose is defeating
GCC's allocator/scheduler on GENERAL-PURPOSE opcodes with C equivalents (`move`/`addu`/`nop` used
as workarounds); `ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2` have NO C equivalent at all. Constructs
(1)-(8) contain no register pin, no un-granted hardcoded-`$N` injection, no scheduling barrier,
no volatile coercion, no dead store, no dead local, no unused array, no `(void)&x`, no alias
rename, no asm-operand device beyond the Judge-granted LZC island form, no build-time rewriting.

## T6 naming-announces-intent
No `pad` / `_pad` / `dummy` / `unused` / `spill` / `sp_*`-as-pad / `_buf` / `tail` / `slack` names.
`sp_tmp` is the inherited name for the LIVE LZC-output locals object (its element 0 is written by
the island and read on the next line). `lut` names the LUT byte it holds for most of its life.
`lw_v1` / `li_v0` are the matched sibling func_8001A538's own names for the identical statements.
Every declared local is read.
(9)(10)(11) No coercion-announcing names — the islands are unnamed statement blocks; every
comment discloses the SDK macro name, the header line range, and (for the LZC island) the
addressing-preamble disclosure the Judge's 2026-09-09 23:11 ruling required.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-vars-local-array (OVERSIZED-LOCALS carve-out 2026-07-13)
  SCOPE: "FORBIDDEN as of 2026-05-31 (expanded 2026-06-01) — unused local arrays and (void)&scalar address-coercion. NARROW CARVE-OUT 2026-07-01: a WRITTEN-never-read local array is sanctioned (SOTN dra/62DEC.c ships u8 sp70[4] written 4x/read 0x, twice, in matched core) when the TARGET bytes contain the dead stores (oracle-enforced), after exhaustion, FAKE-annotated, dual-reviewed. OVERSIZED-LOCALS CARVE-OUT 2026-07-13: a locals object with an unwritten tail (written-prefix buffer) or, fallback, a dead pad local is sanctioned when the target frame equation PROVES the original declared locals strictly larger than the bytes it writes — frame-math proof + range annotation + exhaustion + dual review required."
  PRECEDENT: .claude/rules/dead-vars-local-array.md:5

  FAMILY: do-while-zero-exception
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:6

  FAMILY: staged-value-reused-variable
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:3

  FAMILY: mixed exit forms (goto endK + inline exit) — ordinary C, no FAKE required
  SCOPE: "target has more `sw GLOBAL` stores (or more `j SAME_LABEL` error tails) than your build: GCC 2.7.2 jump2 cross_jump merged N identical `[sw GLOBAL; j END]` tails into one block. FIX: give the error paths a MIX of exit forms (distinct `goto endK; ... endK: return G;` labels + one inline `return G;`) so the block ENDINGS differ -> suffixes not rtx_equal -> no merge."
  PRECEDENT: .claude/rules/cross-jump-store-tail-merge.md:6

  FAMILY: cop2-addressing-preamble-cluster (constructs 9-11, the three GTE inline-asm islands)
  SCOPE: "The 2026-08-17 owner CLUSTER ruling for func_8002FDB0 covers a 28-function family in the 0x8001-0x8003 band sharing the `addu $t4,$aN,$zero` + cop2 hand-asm idiom. Membership list built and verified here; each member inherits the canonical-asm disposition subject to the same mechanical per-function check, applied by the Judge without re-escalation."
  SCOPE-AMENDMENTS (not part of the verbatim scope sentence above): widened 2026-09-01 to any copy-source register; condition 3 clarified 2026-09-02 -- condition 3's "template" is the body of the named Sony PsyQ GTE macro the island reproduces, so GPR instructions that are the macro's own published text are part of the template and ADMITTED.
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:60 (func_80018094 membership row, SetRotMatrix/long-vector sub-family, 3 idiom sites)

ORDINARY-C NOTE (NOT a sanctioned-family claim — no exception is invoked for it):
  construct 7, the `*(&D_8008D118 + i)` byte-LUT read, is this TU's pre-existing,
  oracle-verified spelling against the `extern u8 D_8008D118;` declaration at
  src/code6cac.c:19, shipped unchanged in the already-matched pure-C sibling
  func_8001A538 at src/code6cac.c:757 and src/code6cac.c:784, and in matched bodies in
  src/code6cac_b.c:284 and src/code6cac_b.c:747. It needs no family grant because it is
  ordinary C against the declaration this TU already ships.

ANNOTATION-CONFORMANCE:
  Constructs (9)-(11) (the three cop2 islands) carry NO `/* FAKE */` annotation and need none —
  the FAKE template is for no-semantic-purpose C devices that influence codegen; these are
  semantically-necessary machine instructions with no C form at all, disclosed instead by a
  plain comment naming the SDK macro + header line range (candidate.c lines 60-63, 77, 165-178).
  The four `/* FAKE */` blocks below are unchanged from the prior submission and cover only
  constructs (1)-(4):
  /* FAKE: unwritten tail sp_tmp[1..3] on the live LZC-output locals object, mechanism:
     function.c assign_stack_local / mips.c compute_frame_size (get_frame_size raw 16 ->
     MIPS_STACK_ALIGN keeps 16 where the scalar form rounds 4 -> 8), lever-exhaustion:
     memory/grind/func_80018094/hypotheses.md s2 H15 (declaration scope/order/hoisting), s3
     H19-H22 (HImode narrowing, named-intermediate scalar splits, live 8-byte aggregate,
     BLKmode-only FRAMEDBG census) and s4 (8,906 permuter iterations on the scalar chassis,
     0 novel finds). */
  /* FAKE: do{...}while(0) around the whole else-arm body, mechanism: flow.c life_analysis /
     basic_block_loop_depth (tools/gcc-2.7.2/flow.c:440-471) ... reg_n_refs[regno] += loop_depth
     (flow.c:2081) ... Ablation: dropping this wrap scores 13 (tmp/grind/func_80018094/s10/f1.c).
     lever-exhaustion: memory/grind/func_80018094/hypotheses.md s5 H26-H28, s6 H29-H32,
     s7 H31-H37, s8, s9 H42-H49. */
  /* FAKE: the LZC island's input operand staged through `lut`, the local the LZC arm already
     owns for its LUT byte, mechanism: cse.c cse_end_of_basic_block's follow-jumps gate ... where
     global.c find_reg seats it at $a0 and reorg.c fills the `beqz` delay slot with it ...
     lever-exhaustion: memory/grind/func_80018094/hypotheses.md s5 H26-H28, s6 H29-H32,
     s7 H31-H37, s8, s9 H42-H46. */
  /* FAKE: do{...}while(0) around the small arm's body, with the arm exited by `goto lzc_done`
     so this `if` has NO else, mechanism: cse.c cse_end_of_basic_block's follow-jumps gate --
     expand_end_loop emits NOTE_INSN_LOOP_END after the `goto`'s BARRIER and before the if's
     false label, and the gate's backward walk (tools/gcc-2.7.2/cse.c:8112-8118) stops on a
     LOOP_END note ... lever-exhaustion: memory/grind/func_80018094/hypotheses.md s5 H26-H28,
     s6 H29-H32, s7 H31-H37, s8, s9 H42 (cse class kill), s9b H44-H46. */
  /* FAKE: third do{...}while(0), around the LZC arm's body, mechanism: the same flow.c
     loop-depth weighting -- allocno_n_refs[lut] 8 -> 11 at unchanged live_length 7, pri 47142
     versus sum_sq's 40000 (measured, tmp/grind/func_80018094/s10/e1.allocdbg.txt) ...
     SINGLE LEVEL IS INSUFFICIENT (nested-wrap prerequisite, measured this session): a0.c 13,
     f1.c 13, f3.c 10 ... lever-exhaustion: hypotheses.md s9b H46-H49 and s10 H50-H52. */
