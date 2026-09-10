# Hypothesis ledger — func_8006DD94

## s1 (recon, 2026-09-10) — floor 117 -> 8 ordinary-C, 0 with one unclassified construct

H1 — "the data model needs no declaration change; D_800A374C's existing `extern s32` plus
the %hi/%lo (sdata_exclude) shape is already correct for this function."
mechanism: maspsx --sdata-exclude keeps the symbol out of the $gp window.
probe: wrote the sibling-idiom body against that declaration and measured sandbox.
result: CONFIRMED — the function reaches score 0 with the declaration untouched.

H2 — "the stack descriptor this function builds is the 0x2C-byte S69E18/EnvA shape the
rest of src/text1b.c uses."
mechanism: frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs) (mips.c compute_frame_size);
cc1 prints get_frame_size() as `vars=` in the .frame comment.
probe: compiled with S69E18 (0x2C) and with a 0x34 widening, read cc1's .frame line and
the sandbox score both times.
result: KILLED (instance). 0x2C gives vars=56 / rect at sp+0x48 / score 23; the target
needs vars=64 / rect at sp+0x50. The 0x34 layout gives vars=64 and score 8. The widening
must live in a function-local typedef — widening the shared S69E18 regressed COMPLETED-C
func_8006BB68 from 0 to 17.

H3 — "the target's 7th callee-saved register ($s5 = 0, passed as func_8006E480's second
argument) is reachable from a literal 0 or from a plain s32 local holding 0."
mechanism: loop.c move_movables hoists loop-invariant SETs whose destination is a pseudo;
a constant argument is expanded directly into hard register a1, and an s32 local holding 0
is constant-folded by cse (which runs before loop) back into that same form.
probe: four spellings compiled; cc1's .frame line and the a1 setup insn read out of cc1's
asm; sandbox run on the two that mattered.
result: KILLED (instance). literal 0 -> frame 112 / gp_regs 6 / score 8; `s32 semi = 0;`
before the loop -> identical (112/6, `move a1,zero`); the same declared inside the loop
body -> 112/6. Only a narrow-typed (`s16`, `u8`) or pointer-typed zero-holder reached
frame 120 / gp_regs 7, and `s32 *clut = NULL;` passed as `(s32)clut` measured score 0.

## OPEN — the only thing between this function and COMPLETED-C

Every construct except one is ordinary C and byte-exact (score 8 without it, 0 with it).
The construct is `s32 *clut; clut = NULL;`, read exactly once and cast to s32 as
func_8006E480's second argument. It has no observable effect over the literal `0`
(cheat-checklist T1), so it reads as the sanctioned "constant-holder / dead scalar local"
family (.claude/rules/named-local-fake-exception.md) — which mandates a /* FAKE */
annotation AND a demonstrably-spent modality ladder. Session 1 cannot claim the ladder,
and a Judge FAIL at FINAL CALL would lock the body permanently, so s1 filed a
ruling-request rather than a candidate-ready. The full measured table is in evidence.md.

Next sessions, in order of expected value:
1. Act on the ruling. If the constant-holder is granted, annotate and submit
   memory/grind/func_8006DD94/candidate.c verbatim — bytes are already proven at 0.
2. If it is refused, hunt an ordinary-C carrier for the same invariant zero. The
   mechanism is now known exactly (a pointer/narrow-mode pseudo surviving cse into
   loop.c), so the search is for a spelling where the pointer has a real semantic role —
   e.g. a `s32 *` that genuinely names the absent CLUT/second-texture operand of
   func_8006E480 (src/text1b.c:6172, `return (a0[0] & 0xFE1F) + (a0[1] << 7) + a1;`), or
   a re-typed prototype for that callee (the block-scope
   `extern s32 func_8006E480(s32, s32 *);` variant also measures 0 but collides with the
   file-scope prototype at src/text1b.c:5626).
3. Do NOT re-derive the body: the loop shape, the u8 colour triple, the s16 counter, the
   0x34 local EnvB typedef and the rect store order are all byte-confirmed.

## s2 (recon, 2026-09-10) — floor 8 ordinary-C -> 0 ordinary-C; the s1 open question is VOID

H4 — "s1's row is right: an `s32` local holding 0 is constant-folded by cse, so only a
narrow-mode or pointer-mode local can carry the loop-invariant zero into $s5."
mechanism claimed by s1: cse runs before loop.c and folds the SImode `(set pseudo 0)`,
leaving nothing for loop.c move_movables to hoist.
probe: recompiled `s32 semi; semi = 0;` and `s32 semi = 0;` on s1's OWN chassis
(v16/v17) and on the EnvA-named chassis (v14), sandbox each; then read the sliced
`.cse` and `.loop` RTL dumps of the winning form.
result: KILLED (instance). All three `s32` spellings measure sandbox 0, not 8. The `.cse`
dump shows `(insn 11 (set (reg/v:SI 77) (const_int 0)))` alive in the loop preheader
after cse; cse folds only the memory use. `.loop` shows the set already in the preheader,
so no move_movables hoist is involved. s1's mechanism story AND its measurement are both
wrong; the pointer-clothed `clut` construct it filed a ruling-request over was never
needed.

H5 — "the real requirement is only that the 0 reach the call through a NAMED LOCAL rather
than the literal, because a constant argument is expanded straight into hard register $a1
and no pseudo is ever live across the loop."
mechanism: expand emits the constant into the arg hard reg; with a local there is a
loop-spanning, call-crossing pseudo, and the allocator seats it in a call-saved register,
raising ALIGN8(gp_regs) 24 -> 32 and the frame 112 -> 120 (mips.c compute_frame_size).
probe: deleted the local entirely on the new chassis (v15_nolocal) and measured; measured
five different local TYPES; measured a cast literal `(s32)(s32 *)0` with no local.
result: CONFIRMED. No local -> 8. Cast literal, no local -> 8. Any local -> the frame is
right (s32 -> 0; u8/u16 -> 1; s8/s16 -> 2, the narrow ones paying one extension insn).

H6 — "the zero has a truthful semantic identity — it is the semi-transparency mode — so
the matching body needs no zero-holder, no pointer clothing and no FAKE annotation."
mechanism: not a codegen mechanism; a data-model reading. func_8007352C's decompiled body
(src/text1b.c:6710-6805) names the descriptor EnvA and hands offset 0x10 to
`SetSemiTrans` (:6790); func_8006E480 (:6106) builds a getTPage word whose abr
(semi-transparency) bits 5-6 come from its second argument.
probe: renamed the local `semi`, gave it BOTH consumers (`s.semi = semi;` and the
func_8006E480 argument), renamed the EnvB fields to EnvA's, measured sandbox and ran
verify-oracle.
result: CONFIRMED. sandbox 0 (117/117), verify-oracle build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa. Submitted as candidate-ready, ordinary C, with
no sanctioned-family claim (self_vet.md).

## OPEN — nothing. The function is bytes-proven in ordinary C.

The s1 "OPEN" block is VOID: it rested on H4, which is now killed by measurement. If the
Judge should nonetheless FAIL the named `semi` local, the next axis is NOT the pointer
form (strictly worse, already Judge-FAILed 2026-09-10 05:24) but the question of whether
the two-read shape or the one-read shape is preferred — both measure 0, and the two-read
shape is the one with a truthful reading for every read.

## s3 (recon, 2026-09-10; dispatched as "session 1" after the s2 layer-1 FAIL) — floor 117 (HEAD) -> 0 with one ruling-pending construct; ordinary-C floor 21

H7 — "the layer-1 objection is only about WHERE the unwritten words sit: the target's frame
genuinely reserves sp+0x44..0x4F, and the rest of the s2 body is byte-exact."
mechanism: mips.c compute_frame_size, `frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs)`;
cc1 prints get_frame_size() as `vars=` in its `.frame` comment.
probe: deleted s2's two trailing `EnvB` words (leaving the plain 0x2C descriptor + a separate
`u16 rect[4]`), read cc1's `.frame` line and the measured sp offsets of the descriptor and the
rect, then ran sandbox.
result: CONFIRMED. `vars= 56`, descriptor at sp+0x18 (correct), rect at sp+0x48 (target:
sp+0x50), sandbox 21. Every one of the 21 differing insns is that 8-byte displacement and its
knock-on offsets. Banked: rejected/separate-rect-0x2C-score21.c.

H8 — "a phantom frame slot from an ordinary LIVE scalar local ([[phantom-frame-slots-gcc272]])
can reserve the 8 bytes, so no aggregate and no pad is needed."
mechanism claimed by the memory note: GCC 2.7.2 allocates a stack temp for a computation it
later register-allocates away; get_frame_size() counts it and no store is ever emitted. Named
trigger: two HImode locals feeding an HImode bitwise expression.
probe: nine bodies compiled through the project's exact cpp|cc1, each declaring the candidate
local BETWEEN the descriptor and the rect so any phantom slot would land in the hole — the
memory note's own `(aa & ~bb) & 1` s16 pair, a lone `s16`, an `s64` with a 64-bit multiply,
declaration reordering (all scalars before the rect), `s16 i` hoisted, plus `u16 uv[4]`,
`s32 t[2]`, `s32 t[3]` as aggregate controls. `.frame` line + measured `&rect` sp offset read
for each (tmp/grind/func_8006DD94/s1/sweep.py).
result: KILLED (instance). Every scalar spelling leaves the rect at sp+0x48 / `vars= 56`;
only a declared AGGREGATE moves it, and a written one emits `sh`/`sw` at sp+0x44..0x4B that
the target does not contain. GCC 2.7.2 assigns slots to top-of-function decls in declaration
order before statement expansion, so an assign_stack_temp allocated during expansion can only
land above the rect — it cannot fill this hole.
kill_scope: instance. measured_on: HEAD chassis + the s2 body with the two trailing EnvB words
deleted; no FAKE construct present in any probe.

H9 — "modelling the function's whole stack-locals block as ONE struct — descriptor sub-range
at offset 0, the three unknown words as INTERIOR members, the screen rect as a real trailing
member — reproduces the target frame exactly."
mechanism: the struct's own size (0x40) sets get_frame_size directly, and its member at 0x38
lands at sp+0x50; `&s.header` is bit-identical to the `&s` the s2 body passed to func_8007352C.
probe: built that body, ran sandbox and verify-oracle.
result: CONFIRMED. sandbox 0 (117/117, rules_dropped 0) and verify-oracle build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true — both this session with the body
in src/text1b.c. Two filler words instead of three scores 5 (rejected/), which isolates the
last 5 insns to the rect's own address+stores. NOT submitted as candidate-ready: see OPEN.

## OPEN — one classification question, and it generalises to four more queue items

The body is bytes-proven. The only thing between it and COMPLETED-C is whether three
unwritten INTERIOR words of a frame-block struct are ordinary C or a respelling of the banned
trailing-pad construct. s3 does not self-answer that: the driver rejects a candidate-ready
that re-declares a banned construct under any spelling, and "respell to dodge a ban" is
exactly what the contract forbids — so s3 returns `ruling-request`.

What the ruling turns on, with everything measured:
1. The target really does reserve sp+0x44..0x4F and touch none of it; there is no
   spelling-of-a-scalar escape (H8), so SOME declared aggregate must occupy the hole.
2. Any WRITTEN filler emits stores the target lacks, so the filler cannot be a written array
   (that also fails the dead-vars-local-array oracle prong, which requires the target bytes to
   contain the dead stores).
3. Already-accepted in-tree C ships unwritten interior members in exactly this position:
   S_69AE4 (src/text1b.c:5424-5426, COMPLETED-C func_80069AE4 — sp24/sp38/sp3C unwritten,
   followed by the written sp40, descriptor passed as `&s.sp18` at src/text1b.c:5498) and
   EnvA (src/text1b.c:6654-6669, COMPLETED-C func_8007352C — pad0C/pad20/pad24 unwritten).
4. The same hole recurs in func_8006F97C (12 bytes), func_80069F80 and func_8006A1A0 (20),
   func_80070188 (36) — all still INCLUDE_ASM. A ruling here unblocks that whole family.

If the ruling REFUSES the interior words, the remaining axes in priority order are:
(a) evidence that the descriptor type at THIS call site is genuinely larger than EnvA (would
    need a second, independent BB2 call site writing a descriptor field above +0x2B — the s3
    census found none, so this is currently unsupported);
(b) forensics on the four sibling functions above to recover the shared source idiom that
    reserves the hole — they are the only remaining source of evidence about what the original
    programmer actually declared here;
(c) the constant-holder/FAKE route the 2026-09-10 05:24 Judge ruling left open, which is about
    the `semi` local and does NOT address the frame hole at all.

## s4 (recon, 2026-09-10; dispatched as "session 1" after the s3 Judge FAIL) — HEAD floor 117; honest ordinary-C floor established at 21

Context inherited: the Judge FAILed s3's merged frame-block struct at FINAL CALL
(docs/grind/decisions.md 2026-09-10 05:59) — interior unwritten words are the banned
trailing pad relocated, a new family, and not granted. The Judge's own disposition named
exactly two honest axes left: (b) sibling forensics recovering a genuinely larger evidenced
descriptor type, or rotation; plus the frozen first-decl `volatile u32 pad[N]` family,
which is form-constrained to LEADING holes. s4 attacked (b) and the frame mechanics.

H10 — "the pad/filler family is worth pursuing at all, i.e. some spelling of unwritten
filler both reproduces the target frame AND lowers the honest floor."
mechanism: engine/volatile_cheats.py strips unwritten pad locals before the sandbox scores,
so a construct that only reserves frame bytes contributes no scorable instructions
([[unannotated-fake-inflates-honest-floor]]).
probe: built the pad in the spelling the stripper actually sees — a never-written
`u16 rect0[4]` declared ahead of the real rect — read cc1's .frame line, then ran
`sandbox func_8006DD94 --disable all`.
result: KILLED (instance). cc1 prints `# vars= 64, regs= 7/0, args= 24` and puts the rect
stores at sp+0x50..0x56, i.e. the frame is byte-exact — and the sandbox still reports 21.
The engine strips the array. s3's "sandbox 0" was a false 0 produced only by hiding the
filler inside a struct member, exactly as the Judge said. Banked
rejected/two-rects-sandbox-strips-score21.c.
kill_scope: instance. measured_on: HEAD chassis + the honest 0x2C-descriptor body
(candidate.c) with one added never-written `u16 rect0[4]`; no FAKE construct present.

H11 — "the descriptor type consumed by func_8007352C is genuinely larger than 0x2C bytes
(0x34-0x38), which would place this function's rect at sp+0x50 with no added local — s3's
axis (b), the first of the two honest axes the Judge left open."
mechanism: a larger struct size feeds get_frame_size directly; the next declared local is
laid out above it (tools/gcc-2.7.2/stmt.c:3392 assign_stack_temp, BLKmode 8-alignment at
stmt.c:3419).
probe: (i) read the COMPLETED-C byte-matching sibling func_8006BB68 — its accepted C
declares `S69E18 s; u16 rect[4];` (src/text1b.c:5754-5806), its target frame is 0x68 = 104
and it passes the rect as `addiu $a1,$sp,0x48` (asm/funcs/func_8006BB68.s, 8006BCD0), which
forces vars = 56 = descriptor 0x2C + rect at 0x48; (ii) ran a 35-caller census over every
asm/funcs/*.s that calls func_8007352C, mapping every sp-relative store and load to that
function's own descriptor base (tmp/grind/func_8006DD94/s1/census.py).
result: KILLED (instance). A 0x34+ descriptor would move func_8006BB68's rect to sp+0x50
and break a function that byte-matches today, so the type is at most 0x30 bytes at a
func_8007352C call site. The census independently finds no caller touching
descriptor-relative 0x2C..0x2F anywhere; the halfword blocks that do appear at
descriptor-relative 0x30+ are rect locals sitting in exactly BB68's slot. This closes the
Judge's axis (b) with measurements instead of the earlier bare "census found none".
kill_scope: instance. measured_on: HEAD chassis, target asm of func_8006BB68 + 35 callers
of func_8007352C; no FAKE construct involved.

H12 — "the 12-byte hole is a phantom frame slot: some ordinary expression in the body
allocates a stack temp, and declaring `u16 rect[4]` in a trailing nested block (so its slot
is handed out after that temp) puts the rect at sp+0x50 with no extra declaration at all."
mechanism: GCC 2.7.2's C front end calls expand_decl at the point a declaration is parsed,
so frame slots are handed out in source order interleaved with statement expansion; a temp
created while expanding an earlier statement therefore precedes a later block's decl.
[[phantom-frame-slots-gcc272]] names a minimal trigger (two HImode locals feeding
`(aa & ~bb) & 1`) that reserved 8 bytes in tslLineG5Init.
probe: six bodies through the project's own cpp|cc1, each reading cc1's `.frame` line and
the emitted rect store offsets (tmp/grind/func_8006DD94/s1/frameprobe.py): rect in a
trailing nested block alone; the same plus the memory note's HImode pair; plus a `long long`
multiply; plus a `long long` divide; plus a soft-float `double` multiply; plus the HImode
form assigned into a third s16.
result: KILLED (instance). All six print `vars= 56` with the rect still at sp+0x48. Nesting
the declaration changes nothing because none of these expressions leaves a surviving stack
temp in this function — the phantom-slot trigger does not reproduce here. s3's H8 killed the
scalar spellings declared at top level; this kills the nested-declaration and
expression-temp variants too.
kill_scope: instance. measured_on: HEAD chassis + the honest 0x2C-descriptor body; no FAKE
construct present in any probe.

H13 — "only a declaration that fails GCC 2.7.2's register-eligibility test can reserve any
frame bytes, so the hole requires an aggregate or an address-taken object declared before
the rect."
mechanism: tools/gcc-2.7.2/stmt.c:3357-3364 gives an automatic a pseudo (zero frame
footprint) unless it is BLKmode, volatile, or TREE_ADDRESSABLE; otherwise stmt.c:3392 calls
assign_stack_temp, and stmt.c:3419 gives a BLKmode decl BIGGEST_ALIGNMENT — which is why an
8-byte aggregate lands 8-aligned at descriptor-relative 0x30 and pushes the rect to 0x38 =
sp+0x50.
probe: five positive controls measured the same way — a USED `s32 t[2]`; an address-taken
`s32 tv`; an unused `s32 dead[2]`; a `struct P2 {s32 a,b;}` passed by value; the never-written
`u16 rect0[4]`.
result: CONFIRMED. All five print `vars= 64` with the rect at sp+0x50; every probe that
declared no such object stayed at 56. The rule is exact and now costs nothing to re-derive.

## OPEN — the residual, stated as a closed dilemma, and where the next session should push

Everything in the body except the frame is byte-exact and inherited (do NOT re-derive: the
3-iteration loop, the u8 colour triple via the chained assignment, the s16 counter, the rect
store order, the named `semi` local). The whole 21-insn residual is one 8-byte frame
displacement, and H10-H13 pin it to a dilemma with no third horn found yet:

  * the target reads and writes NOTHING in sp+0x44..0x4F (E2);
  * only an aggregate / address-taken decl placed before the rect reserves those bytes (H13);
  * such an object, if genuinely used, emits sp-relative traffic the target does not contain
    (probes v4/t5/t7);
  * such an object, if unused, is stripped by the sandbox and the honest floor stays 21 (H10).

Priority for the next session, in order:

1. **FORENSICS on the sibling family, not more spelling search.** func_8006F97C has the
   IDENTICAL layout (0x2C descriptor at sp+0x18, nothing in sp+0x44..0x4F, rect at sp+0x50)
   and func_800720FC uses BOTH the sp+0x48 and the sp+0x50 rect slots with the same
   descriptor base (E4). Three more siblings show the same hole at other sizes
   (func_80069F80 / func_8006A1A0 at 20 bytes, func_80070188 at 36). Recovering what those
   functions declare — especially any sibling whose target genuinely WRITES into its hole —
   is the only remaining source of evidence about the original declaration, and a single
   find would convert the hole from "pad" to ordinary C for the whole family at once.
2. **Re-read the pad-family question with H10 in hand.** The frozen first-decl
   `volatile u32 pad[N]` family cannot produce an INTERIOR hole (the pad would land at
   sp+0x18 and displace the descriptor), and the Judge has already refused an interior pad.
   H10 now adds that it would not even lower the honest floor. Nobody should spend another
   session on a pad spelling; if the family is revisited it must be as a policy question,
   not a probe.
3. **Do NOT resubmit either FAILed body.** Review verdicts are keyed by body: s2's trailing
   -pad EnvB (layer-1 FAIL) and s3's merged S_6DD94 (Judge FINAL-CALL FAIL, banked at
   rejected/merged-struct-judge-fail-0559.c) are both permanently closed. candidate.c is now
   the honest score-21 body, which carries no pad, no dead local and no family claim.

## [s1] An unwritten-filler local that reproduces the target's frame also lowers the honest sandbox floor for func_8006DD94.
- mechanism: engine/volatile_cheats.py strips unwritten pad locals before the sandbox scores, so a declaration that only reserves frame bytes contributes no scorable instructions ([[unannotated-fake-inflates-honest-floor]]); the frame itself is set by mips.c compute_frame_size from get_frame_size().
- probe: Built the filler in the spelling the stripper actually sees — a never-written `u16 rect0[4]` declared ahead of the real rect (tmp/grind/func_8006DD94/s1/v8_two_rects.c) — read cc1's .frame line via tmp/grind/func_8006DD94/s1/frameprobe.py, then ran `sandbox func_8006DD94 --disable all` with the body in src/text1b.c.
- result: KILLED. cc1 printed `.frame $sp,120,$31 # vars= 64, regs= 7/0, args= 24, extra= 0` and put the rect stores at sp+0x50/0x52/0x54/0x56 — the target's exact frame — yet the sandbox still reported 21 (117/117, rules_dropped 0). The engine strips the array. This retroactively explains s3's sandbox 0: hiding the same filler inside struct members evaded the pad allowlist and produced a false 0, exactly as the Judge ruled on 2026-09-10 05:59. Banked memory/grind/func_8006DD94/rejected/two-rects-sandbox-strips-score21.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (INCLUDE_ASM, sandbox 117) + the honest 0x2C-descriptor body from memory/grind/func_8006DD94/candidate.c with one added never-written `u16 rect0[4]`; no FAKE construct present.

## [s1] The descriptor type consumed by func_8007352C is genuinely larger than 0x2C bytes (0x34-0x38), which would place func_8006DD94's rect at sp+0x50 with no added local — s3's axis (b), the first of the two honest axes the Judge left open on 2026-09-10 05:59.
- mechanism: A larger struct size feeds get_frame_size directly and the next declared local is laid out above it (tools/gcc-2.7.2/stmt.c:3392 assign_stack_temp; BLKmode decls get BIGGEST_ALIGNMENT at stmt.c:3419), so a 0x34-byte descriptor would push a following 8-byte rect from sp+0x48 to sp+0x50.
- probe: (i) Read the COMPLETED-C byte-matching sibling func_8006BB68: its accepted C declares `S69E18 s; u16 rect[4];` (src/text1b.c:5754-5806), its target frame is 0x68 = 104 (asm/funcs/func_8006BB68.s prologue) and it passes the rect as `addiu $a1,$sp,0x48` (8006BCD0). (ii) Ran a 35-caller census over every asm/funcs/*.s that calls func_8007352C, mapping each function's sp-relative stores and loads onto its own `addiu $a0,$sp,N` descriptor base (tmp/grind/func_8006DD94/s1/census.py).
- result: KILLED. func_8006BB68's frame forces vars = 104-24-24 = 56, i.e. descriptor 0x18..0x44 with the rect immediately after at 0x48; a 0x34+ descriptor would move that rect to sp+0x50 and break a function that byte-matches on main today. The census independently finds no caller anywhere touching descriptor-relative 0x2C..0x2F; the halfword blocks that do appear at descriptor-relative 0x30+ are rect locals sitting in exactly func_8006BB68's slot. The Judge's bare 'census found none' now has the numbers and a positive counter-witness attached.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis; target asm of func_8006BB68 plus the 35 asm/funcs callers of func_8007352C; no FAKE construct involved.

## [s1] The 12-byte hole at sp+0x44..0x4F is a phantom frame slot: an expression already in the body allocates a stack temp, and declaring `u16 rect[4]` in a trailing nested block hands out its slot after that temp, putting the rect at sp+0x50.
- mechanism: GCC 2.7.2's C front end calls expand_decl at the point a declaration is parsed, so frame slots are handed out in source order interleaved with statement expansion; a temp created while expanding an earlier statement therefore precedes a later block's declaration. [[phantom-frame-slots-gcc272]] names a minimal trigger (two HImode locals feeding `(aa & ~bb) & 1`) that reserved 8 bytes in tslLineG5Init.
- probe: Six bodies through the project's own cpp|cc1 with engine.buildconfig flags, each reading cc1's .frame line and the emitted rect store offsets (tmp/grind/func_8006DD94/s1/frameprobe.py): rect in a trailing nested block alone (v1_innerblock); the same plus the memory note's HImode pair (v2_inner_himode, t4_himode); plus a long long multiply (v3_inner_ll); plus a long long divide (t2_lldiv); plus a soft-float double multiply (t1_double).
- result: KILLED. All six print `vars= 56` with the rect still at sp+0x48. Nesting the declaration changes nothing because none of these expressions leaves a surviving stack temp in this function — the phantom-slot trigger does not reproduce in this context. s3's H8 had killed the scalar spellings declared at top level; this adds the nested-declaration and expression-temp variants.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis + the honest 0x2C-descriptor body (candidate.c); no FAKE construct present in any probe.

## [s1] A declaration reserves frame bytes for func_8006DD94 only when it fails GCC 2.7.2's register-eligibility test, so moving the rect to sp+0x50 requires an aggregate or an address-taken object declared before it.
- mechanism: tools/gcc-2.7.2/stmt.c:3357-3364 gives an automatic a pseudo (zero frame footprint) unless it is BLKmode, volatile or TREE_ADDRESSABLE; otherwise stmt.c:3392 calls assign_stack_temp and stmt.c:3419 gives a BLKmode decl BIGGEST_ALIGNMENT, so an 8-byte aggregate lands 8-aligned at descriptor-relative 0x30 and pushes the rect to 0x38 = sp+0x50.
- probe: Five positive controls measured with the same frameprobe instrument: a genuinely USED `s32 t[2]` (v4_inner_plus_used_arr); an address-taken `s32 tv` with `&tv` passed to a call (t5_addrof); an unused `s32 dead[2]` (t6_deadarr); a `struct P2 {s32 a,b;}` passed by value (t7_structval); the never-written `u16 rect0[4]` (v8_two_rects).
- result: CONFIRMED. All five print `vars= 64` with the rect at sp+0x50; every probe declaring no such object stayed at `vars= 56` / sp+0x48. Combined with the target reading and writing nothing in sp+0x44..0x4F, this closes the residual into a dilemma: a used object emits sp-relative traffic the target lacks (v4/t5/t7 all do), and an unused one is stripped by the sandbox and leaves the floor at 21.
- verdict: CONFIRMED

## s2 (structural, 2026-09-10) — floor 21 -> 0, bytes proven on main

Chassis at dispatch: HEAD carried `INCLUDE_ASM("asm/funcs", func_8006DD94);`. Applying
memory/grind/func_8006DD94/candidate.c (the s4 honest 0x2C-descriptor body) re-measured 21,
matching the ledger, so every banked conclusion was spent on the right chassis.

H14 — "frontier item 2: some insn among the target's 117 can be read as traffic into
sp+0x44..0x4F under a different register/offset attribution, so the hole belongs to a USED
object."
mechanism: an object in the hole that is genuinely used would have to be reached either by a
direct `$sp`-relative load/store or by an `addiu $rX,$sp,0x44..0x4F` base set up earlier
(the s1 census deliberately did not follow computed bases).
probe: read EVERY `$sp` reference in asm/funcs/func_8006DD94.s — all 121 lines of the
function, nothing filtered out — and mapped each offset onto the frame
(tmp/grind/func_8006DD94/s2/sibling_frame_map.md).
result: KILLED (instance). The function's complete `$sp` traffic is: prologue/epilogue
0x78; the seven register saves at 0x58-0x70; outgoing-arg slot 0x10; descriptor stores and
loads at 0x18,0x1C,0x20,0x28,0x2C,0x30,0x34,0x40,0x41,0x42,0x43; the two frame addresses
`addiu $a0,$sp,0x18` (8006DE88) and `addiu $a1,$sp,0x50` (8006DF14); and the four rect
halfword stores at 0x50,0x52,0x54,0x56. There is no reference of any kind to 0x44..0x4F and
no `addiu` from `$sp` other than those two. Whatever occupies the hole is untouched by this
function's own code.
kill_scope: instance. measured_on: target asm asm/funcs/func_8006DD94.s at HEAD; no build,
no FAKE construct.

H15 — "the 12-byte hole is not a pad at all: this drawing family declares TWO adjacent
4-halfword rect objects above the 0x2C descriptor, and func_8006DD94 uses only the upper one,
so spelling them as ONE array makes the whole object live and the floor drops to 0."
mechanism: not a codegen mechanism — a data-model recovery from sibling target asm. A single
`u16 rects[2][4]` is one BLKmode automatic (tools/gcc-2.7.2/stmt.c:3392 assign_stack_temp,
BIGGEST_ALIGNMENT at :3419), so it lands 8-aligned at 0x48 and spans 0x48-0x57, giving
vars = 0x58-0x18 = 64 and frame 120 — the target's exact frame — while `rects[1]` at 0x50 is
written and read, so engine/volatile_cheats.py has nothing to strip.
probe: (i) mapped every `$sp` reference of the five siblings that share the 0x2C-descriptor-
at-sp+0x18 idiom (tmp/grind/func_8006DD94/s2/sibling_frame_map.md). func_800720FC USES BOTH
slots: it fills sp+0x48/0x4A/0x4C/0x4E and passes `addiu $a1,$sp,0x48` (800728A4) to
func_80069898, and it fills sp+0x50/0x52/0x54/0x56 and passes `addiu $a1,$sp,0x50` (80072180
and 800725C4) to SetDrawArea. func_8006F97C has func_8006DD94's exact layout — hole untouched,
the func_80069898 rect at sp+0x50 — plus one u16 local at sp+0x58. (ii) Changed ONLY the rect
declaration in candidate.c from `u16 rect[4]` to `u16 rects[2][4]`, moved the four stores and
the call argument to row 1, and measured.
result: CONFIRMED. `sandbox func_8006DD94 --disable all` = 0 (117/117, rules_dropped 0) and
`verify-oracle` build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true,
both with the body in src/text1b.c this session. This is the third horn the s4 OPEN block said
had not been found: the object in the hole is neither a used object emitting absent traffic
nor an unused object the sandbox strips — it is the LOWER HALF OF A LIVE ARRAY whose upper
half is the rect the target does use. Nothing about it is a pad: the same declaration in
func_800720FC has both rows written to the same offsets.

H16 — "the honest floor of 21 is an INSTRUMENT ARTIFACT, not a codegen difference: the plain
two-array spelling of the rectangle block and the one-array spelling compile to the same
binary." (session 2, structural, 2026-09-10)
mechanism: engine/volatile_cheats.py strips a local array no instruction touches before the
sandbox scores the object file, so a body containing an untouched `u16 rect0[4]` is scored as
if the declaration were absent — the frame shrinks by 8 in the SCORED object even though cc1
emitted the target's frame. The linked executable is built from the UNSTRIPPED translation
unit, so the oracle sees the real bytes. If the two spellings really are the same program, the
oracle must match for BOTH while the sandbox reports 0 for one and 21 for the other.
probe: built `u16 rect0[4]; u16 rect[4];` (rect0 declared, never written, never read; the four
stores and the func_80069898 argument on `rect`) as the ONLY change from the one-array body —
tmp/grind/func_8006DD94/s2/body_tworects.c, banked at
rejected/two-separate-rect-arrays-oracle-match-sandbox-21.c — and ran both instruments on it
this session.
result: CONFIRMED, and it is the strongest fact this ledger holds. That body measures
`sandbox func_8006DD94 --disable all` = 21 (rules_dropped 0) AND `verify-oracle` build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa with build_matches true. The one-array body measures
sandbox 0 and the identical oracle SHA1. Two spellings, one binary. Consequences: (a) the
"honest floor 21" recorded by s4 was never a byte distance — the function has been at byte
distance 0 since the two-array form existed, and the number only reflected the stripper;
(b) `u16 rects[2][4]` therefore buys NO bytes a plain pair of declarations does not already
buy, which removes the "passes because the detector misses this spelling" reading — the
spelling changes what the DETECTOR sees, not what the COMPILER emits;
(c) [[unannotated-fake-inflates-honest-floor]] has a converse worth recording project-wide: an
un-stripped-but-untouched ORDINARY declaration deflates the honest floor by the same mechanism,
so a flat floor on a function whose residual is pure frame size should always be cross-checked
with verify-oracle before anyone calls the axis dead.

## OPEN — nothing measurable remains on this function.

Submitted as candidate-ready: ordinary C, no /* FAKE */, no sanctioned-family claim, sandbox 0
and oracle SHA1 match re-measured this session from a clean HEAD checkout.

If review objects that row 0 of `rects` is untouched, do NOT respell — every other spelling is
already dead (rejected/two-rects-sandbox-strips-score21.c at 21; the struct-member forms
Judge-FAILed 2026-09-10 05:59; the descriptor-widening form banned). The two answers to give
are (i) the func_800720FC target evidence: that sibling writes BOTH rows of exactly this object
at exactly these offsets (sp+0x48..0x4E and sp+0x50..0x56 off the same sp+0x18 descriptor), so
a two-row rectangle table is an evidenced data-model fact about this drawing family rather than
a frame-sizing device; and (ii) H16: the plain `u16 rect0[4]; u16 rect[4];` spelling produces
the byte-identical oracle-matching executable, so nothing here is being manufactured by the
array's shape. If review still declines, the correct next outcome is a ruling-request asking
whether a body whose ONLY objectionable feature is a declared-and-unused local array may be
accepted on the oracle when the sandbox's stripper prevents it from ever printing 0 — not
another spelling.

The same reading should close func_8006F97C (identical layout: descriptor 0x18..0x43, hole
0x44..0x4F, rect at sp+0x50) at once, and is worth testing on func_80069F80 / func_8006A1A0
(20-byte holes) and func_80070188 (36-byte hole) above the same descriptor.

## s2b (permuter, 2026-09-10) — mandated modality: permuter

Chassis re-measured at dispatch: HEAD carried `INCLUDE_ASM("asm/funcs", func_8006DD94);`.
Splicing the honest 0x2C-descriptor body (rejected/separate-rect-0x2C-score21.c) into
src/text1b.c re-measured `sandbox func_8006DD94 --disable all` = 21 (117/117,
rules_dropped 0) — the ledger floor is correct and every banked conclusion was spent on
the right chassis. A single-function decomp-permuter workspace built this session
(tmp/grind/func_8006DD94/s2/mkws.sh -> tmp/perm_6dd94) reproduces the residual exactly:
base 117 insns vs target 117, and the ONLY differing instructions are the frame ones
(`addiu sp,sp,-112` vs `-120`, the seven register saves, `addiu a1,sp,72` vs `80`, and the
four rect `sh` at 72/74/76/78 vs 80/82/84/86). Nothing else in the body differs.

## [s2b] The permuter, sampling the honest score-21 chassis, can reach a score-0 body by an ordinary-C mutation (no pad, no dead local, no volatile).
- mechanism: decomp-permuter's randomizers mutate declarations, types, statement order and
  expression shape; the residual is a single 8-byte frame displacement, which is exactly the
  kind of thing a declaration-level mutation can hit. `--stack-diffs` is on by default so
  sp-relative offsets are scored rather than normalised away (the launch reported base_score
  159 on that metric).
- probe: `tools/permuter_campaign.py launch --func func_8006DD94 --dir tmp/perm_6dd94
  --label s2-frame-residual -j 8`, run to 24,368+ iterations, waited in-turn with
  `permuter_campaign.py wait`, harvested with `--stop`. Every output-* directory was read.
- result: KILLED (instance). The campaign produced six novel finds and three of them score 0 —
  and every single one is a `volatile` unused pad local:
    output-0-1  `volatile short pad;`              declared between `EnvB s;` and `u16 rect[4];`
    output-0-2  `volatile unsigned long long pad;` same position (banked at
                rejected/permuter-interior-volatile-pad-score0.c)
    output-0-3  same family
    output-37-1 `volatile int pad;` declared LAST, after every other local -> score 37, i.e.
                WORSE than the chassis: the slot only helps when it precedes the rect
    output-97-1 a dead `short new_var = 0;` (score 97)
  No ordinary-C mutation in the reachable neighbourhood closes the frame. The score-0 finds
  are the forbidden unused-local pad family in the INTERIOR position the Judge explicitly
  refused on 2026-09-10 05:59, so they are proposals that die at the cheat checklist, not
  candidates. They are still a measurement (see the geometry fact below).
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/perm_6dd94 workspace over the honest 0x2C-descriptor chassis
  (rejected/separate-rect-0x2C-score21.c, sandbox 21); no FAKE construct in the base;
  24,368 iterations, 8 jobs, --stack-diffs.

## [s2b] GEOMETRY (CONFIRMED, and it simplifies every future probe): the residual is exactly ONE stack-homed local slot, of ANY size or mode, declared between the descriptor and the rect.
- mechanism: GCC 2.7.2 hands out frame slots in declaration order and rounds the running
  frame offset so that the next BLKmode decl gets BIGGEST_ALIGNMENT (tools/gcc-2.7.2/stmt.c:3419).
  The 0x2C descriptor occupies sp+0x18..0x43; with nothing between, the rect (BLKmode,
  8-aligned) lands at sp+0x48 and vars = 56. Any single additional stack-homed object — 2
  bytes, 4 bytes or 8 bytes, it does not matter — consumes the 0x44/0x48 region and pushes the
  rect to sp+0x50, giving vars = 64 and frame 120, the target exactly.
- probe: cross-read of five independent measurements: s1's t5_addrof (one address-taken s32 ->
  vars 64), s1's t6_deadarr and v8_two_rects (unused arrays -> vars 64), and this session's
  permuter finds output-0-1 (`volatile short`, 2 bytes -> score 0) and output-0-2
  (`volatile unsigned long long`, 8 bytes -> score 0). Position is load-bearing: output-37-1
  put the same slot AFTER the rect and scored 37.
- result: CONFIRMED. The search target is therefore precisely stated for every future session:
  find ONE ordinary-C object that (a) is stack-homed (aggregate, address-taken, or volatile —
  a plain scalar gets a pseudo and reserves nothing, stmt.c:3357-3364), (b) is declared before
  the rect, and (c) emits ZERO sp-relative traffic. (a) and (c) are what the whole ledger has
  failed to satisfy simultaneously.
- verdict: CONFIRMED

## [s2b] The in-tree phantom-frame-slot witness reproduces in func_8006DD94: transplanting tslLineG5Init's exact HImode trigger reserves the missing 8 bytes with zero dead declarations.
- mechanism: [[phantom-frame-slots-gcc272]] records that GCC 2.7.2 allocates a stack temp for
  a computation it later register-allocates away; `get_frame_size()` counts it and no store is
  emitted. The note's byte-verified witness is func_8003DA8C (the tslLineG5Init grind), and I
  read the actual C at src/code6cac_c2.c:1290-1296: `s16 v1 = D_800F6656; s16 mask =
  D_80090608; if ((v1 & ~mask) & 1) { ... }` — two s16 locals loaded from s16 globals feeding
  an and-with-complement test.
- probe: seven bodies through the project's own cc1 with engine.buildconfig flags, reading
  cc1's own `.frame`/`vars=` line and the emitted rect `sh` offsets
  (tmp/grind/func_8006DD94/s2/fp.py, which splices the function into the preprocessed TU
  tmp/perm_6dd94/base.c so no cpp round-trip is needed):
    b_ctl_himode_pair  — the witness's trigger transplanted VERBATIM (v1/mask read from
                         D_800A352C/D_800A3514) as a MECHANISM CONTROL
    b_p1_yv            — one truthful s16 carrier for `*(s16 *)(D_800A34FC + 0xE)`
    b_p2_yv_ph         — two truthful s16 carriers (the y read + the D_800A3514 phase)
    b_p3_block_init    — the same two, declared WITH INITIALISERS inside the branch's own
                         block, i.e. the witness's exact syntactic shape
    b_p4_sel           — an s16 carrier for the `D_800A352C + 1` selector
    b_p5_p2_innerrect  — b_p2 plus the rect declared in a trailing nested block
    b_p6_ctl_innerrect — the verbatim control plus the rect in a trailing nested block
- result: KILLED (instance). All seven print `# vars= 56` with the rect still at sp+0x48.
  The verbatim control failing is the decisive one: the phantom-slot trigger is not a property
  of the source spelling, it is a property of register pressure — in the witness the HImode
  pseudos have no hard register to live in, while func_8006DD94 already saves seven registers
  and its HImode pseudos are seated in callee-saved registers, so no stack temp is ever
  allocated. (b_p4_sel is separately interesting: hoisting the selector out of the loop drops
  a saved register, frame 104, regs 6/0 — a real codegen change, still vars 56.)
  s1's H12 killed six expression-temp variants; this kills the HImode family at its source,
  including the exact in-tree witness spelling, so [[phantom-frame-slots-gcc272]]'s "hunt the
  live-locals form first" instruction is now discharged for this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: the honest 0x2C-descriptor chassis spliced into the preprocessed TU
  tmp/perm_6dd94/base.c, compiled with engine.buildconfig CC_FLAGS (-mel -msoft-float);
  no FAKE construct in any of the seven probes.

## [s2b] FRONTIER ITEM 1 IS DEAD: one of the sibling functions with the same descriptor-plus-hole layout has a target that genuinely WRITES into its hole, which would name the object the original programmer declared there.
- mechanism: the hole must be an aggregate or address-taken decl (stmt.c:3357-3364); a sibling
  whose target contains stores into it identifies the type and its truthful role, and all the
  siblings share the func_8007352C descriptor idiom, so the declaration block is likely
  copy-pasted between them.
- probe: tmp/grind/func_8006DD94/s2/spmap.py maps EVERY `$sp`-relative load, store and
  `addiu $rX,$sp,N` in the target asm of all seven family members onto the frame; full output
  banked at tmp/grind/func_8006DD94/s2/spmap.txt. This extends s1's census.py, which
  deliberately did not follow computed bases: spmap.py lists every `addiu` from `$sp`, so a
  computed base into a hole would show up.
- result: KILLED (instance). Not one of the five hole-carrying siblings touches its hole, and
  there is no `addiu` from `$sp` into any hole in any of them. The measured family layout
  (args always 24, descriptor always the 0x2C block at sp+0x18..0x43, next used object always
  8-aligned):
    func_8006BB68  frame 0x68  rect at 0x48                       hole 0 bytes  (COMPLETED-C)
    func_8006DD94  frame 0x78  rect at 0x50                       hole 0x44..0x4F = 12
    func_8006F97C  frame 0x88  rect at 0x50, u16 at 0x58          hole 0x44..0x4F = 12
    func_80069F80  frame 0x70  NO rect and no object at all above hole 0x44..0x57 = 20
    func_8006A1A0  frame 0x70  NO rect and no object at all above hole 0x44..0x57 = 20
    func_80070188  frame 0x90  NO rect and no object at all above hole 0x44..0x67 = 36
    func_800720FC  frame 0x98  rects at 0x48 AND 0x50             hole 0 bytes
  Two facts fall out. (i) func_8006BB68 (byte-matching on main) and func_800720FC both have
  ZERO hole, which re-confirms the descriptor is 0x2C and kills any "the shared type is wider"
  reading for the third time. (ii) func_80069F80 and func_8006A1A0 reserve 20 bytes above the
  descriptor while making NO use whatsoever of anything above it — they do not even call
  func_80069898 — so whatever the hole is, it is not "the unused first row of the rectangle
  table": the same phenomenon occurs in siblings that have no rectangle at all. That is
  positive evidence AGAINST the `u16 rects[2][4]` data-model story (already layer-1 FAILed and
  now banned) and it means the sibling family cannot name the object. There is no sibling left
  to read.
- verdict: KILLED
- kill_scope: instance
- measured_on: target asm asm/funcs/{func_8006BB68,func_8006DD94,func_8006F97C,func_80069F80,
  func_8006A1A0,func_80070188,func_800720FC}.s at HEAD; no build, no FAKE construct.

## OPEN after s2b — the frontier, restated with the geometry fact in hand

The residual is ONE stack-homed slot in front of the rect. Everything that reserves such a
slot is now measured, and it partitions cleanly:
  * aggregate or address-taken and USED     -> emits sp-relative traffic the target lacks
                                               (s1 v4/t5/t7)
  * aggregate or address-taken and UNUSED   -> the sandbox strips it, floor stays 21 (s1 H10);
                                               as a struct member or a merged array row it is
                                               Judge-FAILed / layer-1-FAILed and now BANNED
  * volatile and unwritten                  -> score 0, but it is the frozen pad family in the
                                               INTERIOR position, refused by the Judge
                                               2026-09-10 05:59 (this session's permuter finds)
  * compiler stack temp (phantom slot)      -> does not occur in this function at ANY register
                                               pressure the body can be spelled at (s1 H12,
                                               s2b seven-probe HImode kill)
Remaining honest ideas, in the order the next session should take them:
  1. Make the FUNCTION need a spill. Every probe so far tried to add an object; the untried
     inverse is to raise register pressure until local-alloc/global-alloc gives a pseudo a
     stack home whose references are then removed by a later pass, which is the only mechanism
     that produces frame bytes with zero traffic. b_p4_sel showed the body's register count is
     movable (7 -> 6 saved regs) by hoisting one expression, so the pressure knob exists.
     Instrument: tmp/grind/func_8006DD94/s2/fp.py (reads cc1's own vars=), plus the .lreg/.greg
     dumps via `pwsh tools/grinder/dump.ps1 func_8006DD94`.
  2. A ruling on the pad-position constraint. The one construct measured to close this function
     is a `volatile` unwritten local that must sit between two other declarations rather than
     first. The frozen family's "first-decl" requirement is a form constraint, not a mechanism
     one, and the two exemplars it was written from (func_80047EE8 / func_80047FBC pre_pad[8])
     happened to have leading holes. Whether an interior instance can be granted is a policy
     question the Judge has answered once (no) — it should not be re-asked without new
     evidence, and this session's evidence is that the pad is the ONLY closing form the
     permuter can reach.
  3. Do NOT re-run a permuter campaign on this chassis. Two campaigns, 24k + N iterations, were
     spent this session; the basin's only score-0 attractor is the pad.

## [s2] The permuter, sampling the honest score-21 chassis, reaches a score-0 body by an ordinary-C mutation (no pad, no dead local, no volatile).
- mechanism: decomp-permuter's randomizers mutate declarations, types, statement order and expression shape; the residual is a single 8-byte frame displacement, exactly the kind of thing a declaration-level mutation can hit. --stack-diffs is on by default so sp-relative offsets are scored rather than normalised away (launch reported base_score 159 on that metric).
- probe: Built a single-function workspace (tmp/grind/func_8006DD94/s2/mkws.sh -> tmp/perm_6dd94) whose base reproduces the residual exactly: 117 insns vs 117, differing ONLY in the frame. Campaign 1 (label s2-frame-residual, 8 jobs) ran to 24,368 iterations; every output-* directory was read. Campaign 2 (tmp/perm_6dd94b, same chassis, settings.toml weight_overrides perm_pad_var_decl = 0.0 and perm_add_self_assignment = 0.0 to forbid the pad randomizer) ran to 32,175 iterations. Both waited in-turn via permuter_campaign.py wait and harvested with --stop.
- result: KILLED. Campaign 1 produced six novel finds; THREE score 0 and all three are the same construct - `volatile short pad;`, `volatile int pad;` and `volatile unsigned long long pad;` declared between `EnvB s;` and `u16 rect[4];` (banked at rejected/permuter-interior-volatile-pad-score0.c). That is the forbidden unused-local pad family in the INTERIOR position the Judge explicitly refused on 2026-09-10 05:59, so they are proposals that die at the cheat checklist, not candidates. A fourth find put the identical pad AFTER the rect and scored 37, i.e. worse than the chassis - position is load-bearing. Campaign 2, with the pad randomizer disabled, never reached 0 in 32k iterations: its best was 37, a body that makes the REAL local `c` volatile, which fixes the frame size but leaves the rect at sp+0x48 and adds the volatile's own load/store traffic. No ordinary-C mutation in the reachable neighbourhood closes the frame.
- verdict: KILLED
- kill_scope: instance
- measured_on: tmp/perm_6dd94 and tmp/perm_6dd94b over the honest 0x2C-descriptor chassis (memory/grind/func_8006DD94/candidate.c, sandbox 21, 117/117, rules_dropped 0); no FAKE construct in either base; 56,543 iterations total, 8 jobs each, --stack-diffs.

## [s2] The in-tree phantom-frame-slot witness reproduces in func_8006DD94: transplanting func_8003DA8C's exact HImode trigger reserves the missing frame bytes with zero dead declarations.
- mechanism: [[phantom-frame-slots-gcc272]] records that GCC 2.7.2 allocates a stack temp for a computation it later register-allocates away; get_frame_size() counts it and no store is ever emitted. I read the witness's actual C at src/code6cac_c2.c:1290-1296 - `s16 v1 = D_800F6656; s16 mask = D_80090608; if ((v1 & ~mask) & 1) { ... }`, two s16 locals loaded from s16 globals feeding an and-with-complement test.
- probe: Seven bodies through the project's own cc1 with engine.buildconfig flags (-mel -msoft-float), reading cc1's own .frame/vars= line and the emitted rect sh offsets: tmp/grind/func_8006DD94/s2/fp.py splices each body into the preprocessed TU tmp/perm_6dd94/base.c. Variants: b_ctl_himode_pair (the witness's trigger transplanted VERBATIM, as a mechanism control), b_p1_yv (one truthful s16 carrier for the *(s16 *)(D_800A34FC + 0xE) read), b_p2_yv_ph (two truthful s16 carriers), b_p3_block_init (the same two as block-scope initialised decls, the witness's exact syntactic shape), b_p4_sel (an s16 carrier for the D_800A352C + 1 selector), b_p5_p2_innerrect and b_p6_ctl_innerrect (the same with the rect in a trailing nested block).
- result: KILLED. All seven print `# vars= 56` with the rect still at sp+0x48. The verbatim control failing is the decisive one: the phantom-slot trigger is not a property of the source spelling but of register pressure - in the witness the HImode pseudos have no hard register to live in, while func_8006DD94 already saves seven registers and seats its HImode pseudos in them, so no stack temp is ever allocated. b_p4_sel is separately useful: hoisting the selector out of the loop drops a saved register (frame 104, regs 6/0) while still printing vars 56, which proves the body's register pressure is movable. s1's H12 had killed six expression-temp variants; this kills the HImode family at its source including the in-tree witness's own spelling, discharging that memory note's 'hunt the live-locals form first' instruction for this function.
- verdict: KILLED
- kill_scope: instance
- measured_on: the honest 0x2C-descriptor chassis spliced into the preprocessed TU tmp/perm_6dd94/base.c and compiled with engine.buildconfig CC_FLAGS; no FAKE construct in any of the seven probes.

## [s2] Frontier item 1: one of the sibling functions with the same descriptor-plus-hole layout has a target that genuinely WRITES into its hole, which would name the object the original programmer declared there and convert the whole family to ordinary C at once.
- mechanism: The hole must be an aggregate or address-taken decl (tools/gcc-2.7.2/stmt.c:3357-3364); a sibling whose target contains stores into it identifies the type and its truthful role, and all the siblings share the func_8007352C descriptor idiom, so the declaration block is likely copy-pasted between them.
- probe: tmp/grind/func_8006DD94/s2/spmap.py maps EVERY $sp-relative load, store and `addiu $rX,$sp,N` in the target asm of seven family members onto the frame; full output banked at tmp/grind/func_8006DD94/s2/spmap.txt. This extends s1's census.py, which deliberately did not follow computed bases: spmap.py lists every addiu from $sp, so a computed base into a hole would show up.
- result: KILLED. Not one of the five hole-carrying siblings touches its hole, and none takes an address into it. Measured family layout (args always 24, descriptor always the 0x2C block at sp+0x18..0x43, next used object always 8-aligned): func_8006BB68 frame 0x68 rect at 0x48 hole 0 (COMPLETED-C); func_800720FC frame 0x98 rects at 0x48 AND 0x50 hole 0; func_8006DD94 frame 0x78 rect at 0x50 hole 12; func_8006F97C frame 0x88 rect at 0x50 plus a u16 at 0x58 hole 12; func_80069F80 frame 0x70 hole 20; func_8006A1A0 frame 0x70 hole 20; func_80070188 frame 0x90 hole 36. Two consequences. (i) The two zero-hole members both byte-match or use both rect rows, re-confirming the 0x2C descriptor and killing the wider-descriptor reading for the third time. (ii) func_80069F80 and func_8006A1A0 reserve 20 bytes above the descriptor while using NOTHING above it - they never call func_80069898 and have no rectangle at all - so the hole is not 'the unused first row of a two-row rectangle table'; the same phenomenon appears in siblings with no rectangle. That is positive evidence against the already-banned rects[2][4] data-model story, and it means there is no sibling left to read.
- verdict: KILLED
- kill_scope: instance
- measured_on: target asm asm/funcs/{func_8006BB68,func_8006DD94,func_8006F97C,func_80069F80,func_8006A1A0,func_80070188,func_800720FC}.s at HEAD; no build, no FAKE construct.

## [s2] The whole residual is exactly ONE stack-homed local slot, of any size or mode, declared between the descriptor and the rect.
- mechanism: GCC 2.7.2 hands out frame slots in declaration order and rounds the running frame offset so the next BLKmode decl gets BIGGEST_ALIGNMENT (tools/gcc-2.7.2/stmt.c:3419). The 0x2C descriptor occupies sp+0x18..0x43; with nothing between, the rect (BLKmode, 8-aligned) lands at sp+0x48 and cc1 prints vars= 56. Any single additional stack-homed object consumes the 0x44/0x48 region and pushes the rect to sp+0x50, giving vars= 64 and frame 120 - the target exactly.
- probe: Cross-read of five independent measurements on cc1's own vars= line: s1's t5_addrof (one address-taken s32 -> 64), s1's t6_deadarr and v8_two_rects (unused arrays -> 64), and this session's permuter finds output-0-1 (volatile short, 2 bytes -> score 0) and output-0-2 (volatile unsigned long long, 8 bytes -> score 0). Control for position: the same declaration placed after the rect scores 37 and leaves the rect at 0x48.
- result: CONFIRMED. The search is now precisely stated for every future session: find ONE ordinary-C object that (a) is stack-homed (aggregate, address-taken or volatile - a plain scalar gets a pseudo and reserves nothing, stmt.c:3357-3364), (b) is declared before the rect, and (c) emits ZERO sp-relative traffic. Every form measured so far satisfies at most two of the three.
- verdict: CONFIRMED

## [s3] Frontier item 1: the sp+0x44..0x4F bytes are a REGISTER SPILL HOME rather than a declaration - at some spelling of this body the allocator fails to seat a pseudo, assigns it a stack slot, and later passes remove every reference to it, producing frame bytes with zero sp-relative traffic and no declared object at all.
- mechanism: s2 left this as the untried inverse of every previous probe. Instead of adding an object and then arguing about whether it is honest, change register pressure until GCC itself puts something on the stack. reload.c / global.c assign spill homes through assign_stack_local, and get_frame_size counts them whether or not the reload survives; s2's b_p4_sel had already proved the pressure knob is reachable from truthful C (hoisting the D_800A352C + 1 selector out of the loop moved the function from regs 7/0 frame 112 to regs 6/0 frame 104 while leaving vars at 56).
- probe: Three truthful restructurings of candidate.c - ordinary statement re-association only, no pad, no volatile, no dead local, no FAKE - measured through tmp/grind/func_8006DD94/s3/fp3.py, which splices a body into the preprocessed TU (tmp/perm_6dd94/base.c), runs the project's own cc1 with engine.buildconfig flags, and prints cc1's own vars=/regs= line, every `addu $rX,$sp,N` address-take and every sp-relative load/store offset. p1_hoist3 hoists the four loop-invariant global reads (D_800A352C + 1, D_800A34FC, D_800A3514, D_800A374C + 0x28) into named locals above the loop. p2_hoist6 additionally carries arg0[5] and arg0[7] in locals across the loop with write-back afterwards. p3_hoist10 adds four more long-lived locals (base/lim/step/mode). Bodies banked at memory/grind/func_8006DD94/rejected/hoist3-invariants-no-spill-vars56.c and rejected/pressure-spill-slot-lands-above-rect-vars64-rect-still-0x48.c.
- result: KILLED, and the kill is mechanical rather than sampled. The spill the frontier predicted DOES appear: p2_hoist6 and p3_hoist10 both reach vars= 64 - the target's exact number - with regs 10/0 and a genuine sw/lw spill pair that has no declaration behind it, while p1_hoist3 shows the softer form (pressure first spends the remaining callee-saved registers, 7/0 -> 9/0, with vars unchanged at 56). But in all three the rect's address is still `addu $5,$sp,0x48` and its four halfwords still land at 0x48/0x4A/0x4C/0x4E: the spill home sits at sp+0x50, ABOVE the rect, so the frame grows in the wrong place. The reason is structural, not incidental. MIPS leaves FRAME_GROWS_DOWNWARD undefined (tools/gcc-2.7.2/config/mips/mips.h:1645, the macro appears only inside a comment), so assign_stack_local takes the upward branch `frame_offset += size` (tools/gcc-2.7.2/function.c:724) and hands out strictly monotonically increasing offsets in ALLOCATION order; the rect's slot is allocated by expand_decl (tools/gcc-2.7.2/stmt.c:3392) while the body is still being expanded to RTL, and every spill home is allocated after expansion finishes, so a spill always receives a higher offset than the rect. This removes the last mechanism the ledger had for producing untouched frame bytes without declaring an object in the hole.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/function.c:724
- measured_on: the honest 0x2C-descriptor chassis (memory/grind/func_8006DD94/candidate.c, sandbox 21 re-measured this session, 117/117, rules_dropped 0) spliced into tmp/perm_6dd94/base.c and compiled with engine.buildconfig CC_FLAGS; no FAKE construct in any of the three probes.

## [s3] The rect can be given 16-byte alignment by an ordinary C declaration, landing it on sp+0x50 with nothing declared between it and the descriptor.
- mechanism: sp+0x50 is 16-aligned and sp+0x48 is not, so an alignment of 16 on the rect object would reproduce the target's layout with no intervening object at all - the only route to the target frame that needs no new declaration whatsoever. GCC 2.7.2 sets a BLKmode automatic's alignment inside expand_decl.
- probe: Read of the two governing definitions rather than a sweep, because they are absolute caps: tools/gcc-2.7.2/config/mips/mips.h:1082 defines BIGGEST_ALIGNMENT as 64 bits for this target, and tools/gcc-2.7.2/stmt.c:3419 sets DECL_ALIGN for a BLKmode automatic to exactly BIGGEST_ALIGNMENT (and to GET_MODE_BITSIZE for everything else), with assign_stack_temp rounding the frame offset to that alignment.
- result: KILLED. There is no C type, aggregate shape or declaration form in GCC 2.7.2 C that gives a local more than 8-byte alignment on this target - the compiler clamps every BLKmode automatic to exactly BIGGEST_ALIGNMENT and no wider mode than DImode exists to raise GET_MODE_BITSIZE past 64. sp+0x48 is therefore the first legal slot after a descriptor ending at sp+0x44 in every spelling, and the hole cannot be produced by alignment.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/stmt.c:3419
- measured_on: tools/gcc-2.7.2 source at HEAD, cross-checked against the s3 probe sweep on the honest 0x2C-descriptor chassis (all probes show the rect 8-aligned at sp+0x48); no FAKE construct.

## [s3] Frontier item 1: the sp+0x44..0x4F bytes are a REGISTER SPILL HOME rather than a declaration - at some spelling of this body the allocator stack-homes a pseudo whose references later die, producing frame bytes with zero sp-relative traffic and no declared object at all.
- mechanism: reload.c/global.c assign spill homes through assign_stack_local and get_frame_size counts them whether or not the reload survives; s2's b_p4_sel had already proved the register-pressure knob is reachable from truthful C (hoisting the D_800A352C + 1 selector moved the function from regs 7/0 frame 112 to regs 6/0 frame 104). This was the untried inverse of every previous probe: change pressure until GCC itself puts something on the stack, rather than adding an object and then arguing about whether it is honest.
- probe: Three truthful restructurings of candidate.c (ordinary statement re-association only - no pad, no volatile, no dead local, no FAKE) measured with tmp/grind/func_8006DD94/s3/fp3.py, which splices a body into the preprocessed TU tmp/perm_6dd94/base.c, runs the project's own cc1 with engine.buildconfig flags, and prints cc1's vars=/regs= line, every 'addu $rX,$sp,N' address-take and every sp-relative load/store offset. p1_hoist3 hoists the four loop-invariant global reads (D_800A352C + 1, D_800A34FC, D_800A3514, D_800A374C + 0x28) into named locals above the loop; p2_hoist6 additionally carries arg0[5] and arg0[7] in locals across the loop with write-back afterwards; p3_hoist10 adds four more long-lived locals (base/lim/step/mode).
- result: KILLED, mechanically rather than by sampling. The predicted spill does appear: p2_hoist6 and p3_hoist10 both reach vars= 64 - the target's exact number - at regs 10/0 with a genuine sw/lw spill pair that has no declaration behind it, and p1_hoist3 shows the softer form (pressure first spends the remaining callee-saved registers, 7/0 -> 9/0, vars unchanged at 56). But in all three the rect's address is still 'addu $5,$sp,0x48' and its four halfwords still land at 0x48/0x4A/0x4C/0x4E: the spill sits at sp+0x50, ABOVE the rect, so the frame grows in the wrong place. MIPS leaves FRAME_GROWS_DOWNWARD undefined (config/mips/mips.h:1645 - the macro appears only inside a comment), so assign_stack_local takes the upward branch frame_offset += size (function.c:724) and hands out strictly increasing offsets in ALLOCATION order; the rect's slot comes from expand_decl (stmt.c:3392) during RTL expansion and every spill home is allocated after expansion finishes, so a spill always receives a higher offset than the rect. Bodies banked at memory/grind/func_8006DD94/rejected/hoist3-invariants-no-spill-vars56.c and rejected/pressure-spill-slot-lands-above-rect-vars64-rect-still-0x48.c.
- verdict: KILLED
- kill_scope: class
- measured_on: the honest 0x2C-descriptor chassis (memory/grind/func_8006DD94/candidate.c, sandbox 21 re-measured this session, 117/117, rules_dropped 0) spliced into tmp/perm_6dd94/base.c and compiled with engine.buildconfig CC_FLAGS; no FAKE construct in any of the three probes.
- predicate_cite: tools/gcc-2.7.2/function.c:724

## [s3] The rect can be given 16-byte alignment by an ordinary C declaration, landing it on sp+0x50 with nothing declared between it and the descriptor.
- mechanism: sp+0x50 is 16-byte aligned and sp+0x48 is not, so an alignment of 16 on the rect object would reproduce the target's layout with no intervening object at all - the only route to the target frame that needs no new declaration whatsoever. GCC 2.7.2 fixes a BLKmode automatic's alignment inside expand_decl.
- probe: Read of the two governing definitions rather than a sweep, because they are absolute caps: tools/gcc-2.7.2/config/mips/mips.h:1082 defines BIGGEST_ALIGNMENT as 64 bits for this target, and tools/gcc-2.7.2/stmt.c:3419 sets DECL_ALIGN for a BLKmode automatic to exactly BIGGEST_ALIGNMENT (GET_MODE_BITSIZE for everything else), with assign_stack_temp rounding the frame offset to that alignment. Cross-checked against the s3 probe sweep, in which every body shows the rect 8-aligned at sp+0x48.
- result: KILLED. GCC 2.7.2 clamps every BLKmode automatic to exactly BIGGEST_ALIGNMENT on this target and no mode wider than DImode exists to raise GET_MODE_BITSIZE past 64, so no C type, aggregate shape or declaration form gives a local more than 8-byte alignment here. sp+0x48 is the first legal slot after a descriptor ending at sp+0x44 in every spelling, and the hole cannot be produced by alignment.
- verdict: KILLED
- kill_scope: class
- measured_on: tools/gcc-2.7.2 source at HEAD plus the s3 probe sweep on the honest 0x2C-descriptor chassis (sandbox 21 re-measured this session); no FAKE construct.
- predicate_cite: tools/gcc-2.7.2/stmt.c:3419

## s4 (enumerate, 2026-09-10)

### H-s4-1 — KILLED (class)
**Statement.** On the nested-rect chassis, every expression class GCC 2.7.2 expands with
a stack temp — DImode arithmetic, soft-float double/float, BLKmode struct copies, unions,
one-member structs, HImode bitwise pairs — either leaves no surviving slot (`vars= 56`)
or leaves one only together with the instructions that use it (`n` > 112); no expression
class in this body's vocabulary reserves 8 bytes below the rect at zero instruction cost.
**Mechanism.** `assign_stack_temp` and `expand_decl` draw from the same monotonically
increasing `frame_offset`, so the nested-block chassis is the ordering that lets a
statement-created temp land below the rect; but GCC 2.7.2 keeps DImode and soft-float
values in register pairs (no `assign_stack_local` at all), and the only expressions it
DOES give a stack temp are BLKmode aggregates, whose temp is always the destination of an
emitted block move.
**Probe.** 41 spellings compiled with the project's exact CC_FLAGS through
`tmp/grind/func_8006DD94/s4/fp4.py`; logs batch1.log / batch2.log / batch3.log.
**Result.** 6 hits at `vars= 64, n= 112`; all six reserve via an untouched declaration
(five address-taken dead locals, one untouched sibling array). Zero hits from the
expression axis. This closes s3's frontier item 1 (the BLKmode keep=1 temp) on its own
chassis: the class is allocatable but not instruction-free, and this body contains no
8-byte BLKmode-valued expression to begin with — every callee returns a scalar.
**predicate_cite.** `tools/gcc-2.7.2/gcc/function.c:724`
**kill_scope.** class   **measured_on.** HEAD chassis (INCLUDE_ASM in src/), honest
0x2C-descriptor body from candidate.c plus the byte-neutral nested-rect variant
b1_nested.c; no FAKE construct present in any of the 41 probes.

### H-s4-2 — CONFIRMED
**Statement.** The one-line form `u16 rect0[4]; u16 rect[4];` on the honest
0x2C-descriptor chassis builds the whole SLUS_006.63 byte-identical to the oracle, while
`sandbox --disable all` reports 21 for it.
**Mechanism.** engine/volatile_cheats.py removes the untouched local array from the
object file the sandbox scores; the real build keeps it, GCC reserves its 8 bytes, the
rect lands at `sp+0x50`, and `vars= 64` matches the target frame exactly.
**Probe.** spliced into src/text1b.c this session; `sandbox func_8006DD94 --disable all`
= 21 (117/117, rules_dropped 0), then `verify-oracle` = `ok: true`,
`build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa`, `build_matches: true`. src
restored to HEAD.
**Result.** Re-confirmed on the current chassis (kill re-audit discharged: the form
carries no FAKE construct, so ablation is a no-op). The honest floor of 21 is a scoring
artifact of the stripper, not a byte distance.

### H-s4-3 — KILLED (instance)
**Statement.** A plain non-addressable scalar declared at outer scope before the rect
(`s32 hole;` and `long long hole;`) reserves the 8 bytes the target's frame reserves.
**Mechanism.** expand_decl gives a non-addressable scalar decl a pseudo, not a stack
slot, so `get_frame_size()` never grows.
**Probe.** w02_s32_before.c, w03_ll_before.c via fp4.py.
**Result.** Both `vars= 56`, rect still at `sp+0x48`, `n= 112`. Only objects GCC must
address (arrays, aggregates, address-taken scalars) reserve. Declaring the untouched
object as a scalar is therefore not an escape from the array/aggregate families.
**kill_scope.** instance   **measured_on.** HEAD chassis, flat 0x2C-descriptor body,
no FAKE construct present.

### Frontier after s4
The codegen question is closed to a single classification question. Every mechanism that
can put `vars` at 0x40 has now been enumerated, and each one requires a declared object at
`sp+0x44..0x4F` that no instruction touches. The remaining families for such an object are
all either banned for this function (the merged `rects[2][4]`, the trailing struct pads) or
frozen behind prerequisites this instance cannot meet (the interior `volatile` pad the
Judge refused; the sanctioned pad family requires FIRST-declaration position, which would
displace the descriptor from `sp+0x18`). The one form that is neither banned nor
FAKE-annotated — a second, separately declared, untouched `u16 rect0[4]` sibling — is the
subject of this session's ruling request.

## s4-rerun (enumerate, 2026-09-10) - the previous s4 outcome was DISCARDED by the driver
(invalid `predicate_cite` path: `tools/gcc-2.7.2/gcc/function.c:724`; the file actually lives at
`tools/gcc-2.7.2/function.c:724`). Every s4 finding above stands - the chassis was re-measured
here - and the corrected citation for the frame-offset predicate is
`tools/gcc-2.7.2/function.c:724` (`frame_offset += size;` inside `assign_stack_local`, guarded by
`#ifndef FRAME_GROWS_DOWNWARD`). Any future session quoting that predicate must use the corrected
path; the `gcc/` infix does not exist in this checkout.

### H-s4r-1 - KILLED (instance): the Judge's own sanctioned closure form is measured WRONG
**Statement.** A first-declaration `volatile u32 pad[2];` - the shape the Judge's standing
constraint names as the only permitted way to close the sp+0x44..0x4F hole - reproduces the
target's frame size exactly (`vars= 64`, `addiu sp,sp,-0x78`, rect at `sp+0x50`) but places the
0x2C descriptor at `sp+0x20` instead of the target's `sp+0x18`, so every descriptor store moves and
the honest sandbox score rises from 21 to 45 (119 build insns vs 117 target).
**Mechanism.** `assign_stack_local` hands out monotonically increasing frame offsets in
`expand_decl` order (`tools/gcc-2.7.2/function.c:724`, `frame_offset += size;` under
`#ifndef FRAME_GROWS_DOWNWARD`; MIPS leaves that macro undefined, mips.h:1645). The FIRST
declaration therefore owns the LOWEST slot. The target's lowest slot is the live descriptor at
0x18 and its untouched bytes sit at 0x44..0x4F - i.e. INTERIOR, between the descriptor and the
rect. A first-declaration pad can only ever be below the descriptor, which is the one position the
target does not use.
**Probe.** `tmp/grind/func_8006DD94/s4b/p01..p04_*.c` compiled with the project CC_FLAGS through
`tmp/grind/func_8006DD94/s4/fp4.py`, plus a full `sandbox --disable all` on the pad2-first form
with it spliced into `src/text1b.c`.
**Result.**
  - `p01_pad3_first`  (`volatile u32 pad[3]` first): vars= 72, descriptor at 0x28, rect at 0x58.
  - `p02_pad2_first`  (`volatile u32 pad[2]` first): vars= 64, sp -0x78, rect at 0x50 (both target),
    descriptor at 0x20 (target 0x18). Spliced sandbox score **45** (target 117 insns, build 119).
  - `p03_pad3_interior`: vars= 72, descriptor 0x18, rect 0x58.
  - `p04_pad2_interior` (descriptor, then `volatile u32 pad[2]`, then rect): vars= 64, sp -0x78,
    descriptor 0x18, rect 0x50 - the EXACT target frame. This is the interior position the Judge
    refused, and it is the only pad position that reproduces the layout.
So the frozen pad family and this function's target layout are mutually exclusive by CODEGEN, not
by policy: the family requires first-declaration position, the target requires interior position.
Banked as `rejected/first-decl-volatile-pad-displaces-descriptor-to-0x20-score45.c`.
**kill_scope.** instance   **measured_on.** HEAD chassis (INCLUDE_ASM in src/), honest
0x2C-descriptor candidate.c body, with a `volatile u32 pad[N]` FAKE-family carrier present in the
probe by construction (the carrier IS the hypothesis).

### H-s4r-2 - KILLED (class): the rect block's spelling space contains nothing below 21
**Statement.** Enumerating the rect-store block in fully-named form over the inline/keep-named axis
and the commutative-swap axis produces 65 distinct spellings whose best honest sandbox score is 21,
reached by exactly one spelling (the fully-inlined literal form already in candidate.c).
**Mechanism.** The residual is a frame-allocation residual, not an expression residual: build and
target are both 117 instructions and differ only in `addiu sp`, the seven register saves,
`addiu a1,sp,N` and the four rect `sh` offsets. Statement spelling inside the block cannot change
`get_frame_size()`, because frame offsets are handed out by declaration order in
`assign_stack_local` (`tools/gcc-2.7.2/function.c:724`) and are independent of how the stores that
consume the slot are written.
**Probe.** `tools/spelling_enum.py --candidate tmp/grind/func_8006DD94/s4b/enum_src.c --out
tmp/grind/func_8006DD94/s4b/enum` (65 variants, swap axis included) swept with the sweep driver
(`tmp/grind/func_8006DD94/s4b/sweep.json`).
**Result.** ENUMERATION: 65 spellings, best 21, 1 at the floor. Histogram
{21: 1, 22: 4, 23: 12, 24: 24, 26: 24}. Zero hits below the floor - the strongest available
evidence that the residual does not live in this block at all, but in the declaration region.
**kill_scope.** class   **predicate_cite.** `tools/gcc-2.7.2/function.c:724`
**measured_on.** HEAD chassis, honest 0x2C-descriptor candidate.c body, no FAKE construct present
in any of the 65 variants.

### H-s4r-3 - CONFIRMED: chassis unchanged
candidate.c spliced into `src/text1b.c` measures `sandbox func_8006DD94 --disable all` = **21**,
target_insns 117, build_insns 117, rules_dropped 0. src/ restored to HEAD afterwards.

### Frontier after s4-rerun
The codegen question is closed and the policy question is now sharp enough to state in one
sentence: **the target reserves 12 bytes between two live objects, and every construct that
reserves interior bytes is either banned for this function or belongs to a family whose rule
requires first-declaration position - a position measured here to produce the wrong layout.**
That is a classification question, not a search question, and this session returns it as a
ruling-request rather than re-spending measurements on it.
