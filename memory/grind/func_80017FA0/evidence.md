# Evidence bank — func_80017FA0

## s4 (permuter, 2026-08-20) — **SOLVED. floor 2/3 -> 0. Full-build SHA1 == oracle.**

- [s4] **THE CLOSER: `if (i < ptr[1])` instead of `if (ptr[1] > 0)` for the outer
  loop's entry guard.** One token-level change to candidate.c. Spelling the guard
  against the LIVE loop counter (which is 0 at that point, so the test is
  identical) makes `i` a local that survives to frame layout: cc1 prints
  `.frame $sp,8,$31 # vars= 8, regs= 0/0, args= 0, extra= 0` and mips.c's
  compute_frame_size emits `addiu sp,sp,-8` (scheduled into the beqz delay slot)
  and `addiu sp,sp,8` — the target's zero-store 8-byte leaf frame — while `i`
  itself lives entirely in a register, so NO frame store is emitted. This is the
  ordinary GCC-2.7.2 phantom-frame artifact recorded in
  memory/project/phantom-frame-slots-gcc272.md (byte-verified in-tree on the
  COMPLETED-C function tslLineG5Init), not a dead local.
  Measured: tmp/grind/func_80017FA0/s4/vB.c -> 61 insns, `diff` against the
  assembled asm/funcs/func_80017FA0.s = EMPTY (`IDENTICAL TO TARGET`).

- [s4] **s1/s2/s3's central conclusion was WRONG, and this is the correction the
  ledger must carry.** Three sessions concluded "the only pure-C producer of a
  zero-store 8-byte frame is a dead <=8-byte local (array/struct/union/addr-taken)
  = forbidden dead-vars-local-array", killed H-F1 and H-F2 on that basis, and filed
  an OWNER-ESCALATION that the owner resolved 2026-07-27 as REFUSED /
  OWNER-ACCEPTED INCOMPLETE. The error was in the frame probes, not the reasoning
  over them: s1's F0/F1 probes only ever tested locals that were DEAD (`s32 x;`
  unused -> vars=0 because DCE removes them before frame layout). No session tested
  a LIVE ordinary scalar. A live scalar DOES get counted by get_frame_size()
  (ALIGN8(4) = 8 bytes on MIPS_STACK_ALIGN) and still costs zero instructions when
  the allocator keeps it in a register. The dichotomy "zero-store vars=8 <=> the
  local is dead" is false.

- [s4] Oracle proof: `verify-oracle` after a full clean rebuild with the C body in
  src/code6cac.c => `"build_sha1": "62efab4f73f992798c43e8c730aa43baa10bb4fa"`,
  `"build_matches": true`, equal to `original_sha1_locked`. ZERO regfix/asmfix rules
  (`rules_dropped: 0`), ZERO cheat-asm in the function, ZERO FAKE-annotated
  constructs, no pins. `sandbox func_80017FA0 --disable all` = **0** (61/61).

- [s4] CHASSIS DRIFT explained (ledger said floor 2, dispatch could not measure, s4
  measured 3 on the inherited candidate): the extra 1 is a scorer artifact, not a
  codegen gap. `engine/score.py` masks section-relative HI16/LO16 addends but
  deliberately NOT named-symbol ones, and splat symbolised the loop's literal
  `addiu $t3,$t3,4` as `addiu $t3,$t3,%lo(D_1F800004)`. While the function shipped
  as INCLUDE_ASM the reference object carried that named reloc and our compiled
  literal `4` scored as one substitution. `undefined_syms_auto.txt:2` defines
  `D_1F800004 = 0x1F800004`, so %lo == 4 and the linked bytes were always equal —
  which is why the mid-session score of 1 (61/61, objdump diff EMPTY) was
  immediately followed by an oracle SHA1 match. Same class as
  memory/sandbox-lo16-text-addend-false-distance.md. Anyone re-measuring this
  function before the C lands should expect the +1 and check the oracle, not the
  scorer.

- [s4] Permuter campaign (the mandated modality) — tmp/perm_17fa0, label s4-base,
  -j 8, --stack-diffs (default), base_score 300, **15,708 iterations, 26 score-0
  finds**, harvested and STOPPED in-session. Workspace recipe (reusable for any
  code6cac function): tmp/grind/func_80017FA0/s4/mkws.sh — full-TU cpp of
  src/code6cac.c as base.c, per-function extraction of the maspsx output in
  compile.sh, `D_1F800004 = 0x1F800004` defined in both the target prelude and the
  compile prelude so the fold assembles, and NO `.set noat` prelude (the folded
  `sw x,0x1F8000B8` expands through $at and dies under noat). Two gotchas cost a
  cycle each: maspsx emits `.ent`/`.end` with no leading tab (the mar3 awk pattern
  requires one), and pycparser cannot parse the TU because `GameObj` is used in two
  prototypes but never declared in it — a codegen-neutral
  `typedef struct GameObj GameObj;` prepended to base.c fixes it.

- [s4] What the permuter actually found: 26 score-0 forms, of which the large
  majority close the frame with a DEAD volatile local (`volatile long long pad;`,
  `volatile unsigned long pad;`, `volatile char new_var;`) = the forbidden
  dead-vars-local-array family, banked in
  rejected/perm-dead-volatile-local-frame-coercion.c. Exactly ONE find
  (output-0-3) used a live variable in the guard (`int new_var = 0;
  if (ptr[1] > new_var)`), which is the constant-holder shape; the ACCEPTED form is
  the strictly better re-derivation of it that needs no extra variable at all,
  because the function already owns a live counter with the value 0 at that point.
  Findings are proposals: the winning one was re-derived and re-measured by hand
  before it was applied.

- [s4] Rejected variant vD (tmp/grind/func_80017FA0/s4/vD.c): spelling the loop as
  a plain `while (i < ptr[1]) { ... }` with the walking-pointer inits hoisted above
  it does NOT match — GCC emits the inits before the guard (`blez` region reordered,
  3 insns adrift). The manual-rotation shape (inits INSIDE the guarded block,
  `do { } while` back-edge) is required and is the shape GCC's own loop rotation
  produces.

- [s4] Also measured, for the record: variant vC (`s32 zero = 0;` +
  `if (ptr[1] > zero)`) is likewise vars=8 and byte-identical to target. It was NOT
  used — it needs a constant-holder local (a sanctioned-but-FAKE-annotated family)
  where vB needs nothing at all. Recorded so no future session mistakes the
  constant-holder route for the only one available.

## s3 (structural, 2026-07-24) — FLOOR HELD 2. NEW axis (OVERSIZED-LOCALS carve-out) measured DEAD → OWNER-ESCALATION filed.

- [s3] Floor=2 REPRODUCED: candidate.c (scr[] fold form) applied to src, `sandbox --disable all` = {"score":2,"target_insns":61,"build_insns":60,"rules_dropped":6,"cheat_asm_stripped":182}. src reverted clean to HEAD cheat-form. (tmp/grind/func_80017FA0/s3/sandbox_candidate.log)

- [s3] Frame decoded DIRECTLY from asm/funcs/func_80017FA0.s (not inherited): `addiu $sp,-0x8` (beqz delay slot) / `addiu $sp,0x8` then `jr $ra`. LEAF — zero `sw $ra`/`sw $s?` (all regs caller-saved temps), zero `jal` ⇒ callee-saves=0, outgoing-args=0. Locals region 100% dead: every `sw` targets ($at)/($t3) scratchpad, every `lw` sources ($a0)/($t1)/($v1)/($t4)/($v0); ZERO `sw/lw ...($sp)`. `.frame`=8 bytes, fully unwritten/unread. Confirms s1/s2's zero-frame-store claim from primary source.

- [s3] **NEW AXIS s2 never checked**: the 2026-07-13 OVERSIZED-LOCALS carve-out (.claude/rules/dead-vars-local-array.md). s2 evaluated only the 2026-07-01 WRITTEN-never-read carve-out. Frame-math prerequisite 1 for OVERSIZED-LOCALS: frame(8)−saves(0)−args(0)=8 > written(0), a fully-written form ⇒ 0 locals ⇒ 0 frame < 8, so target "proves" 8 dead bytes ⇒ **prerequisite 1 SATISFIED but only TRIVIALLY** — for ANY zero-store phantom leaf frame 0<N holds by construction, so it does NOT distinguish a genuine oversized-locals object (granted case func_80037540: 24B written prefix + live callee buffer, ALIGN8(24)+16+24=0x40≠0x48 non-trivially forced) from plain frame coercion. No written prefix, no live object here ⇒ fully-dead-pad FALLBACK (`s32 pad[2]`/`pad[1]`, ALIGN8 makes 1-8B a range). Carve-out's own NOTE: fully-dead pad still flagged by find_unused_local_arrays + refused by mark_done; sanctioning requires wiring a prerequisite-aware engine allowlist = engine/ work (forbidden grind surface) + owner ruling. func_80017FA0 is a fully-dead-pad family member alongside func_8003DBE4 (`s32 buf[2]`), file_LoadSectors/func_800165F8 (`s32 _pad[2]`). Banked rejected/fully-dead-pad-oversized-locals.c. (tmp/grind/func_80017FA0/s3/framemath.md)

- [s3] DISPOSITION: every sanctioned pure-C axis (structural shapes, BOTH dead-vars carve-outs) AND canonical-asm (scan_hand_coded LOW 1/8) measured dead. Filed OWNER-ESCALATION in docs/grind/decisions.md (2026-07-24) naming func_80017FA0, presenting options (a) sanction fully-dead-pad family [no distinguishing SOTN precedent; engine wiring req'd] vs (b) refuse + INCOMPLETE-owner-accepted per endgame-lock-disposition. Mirrors AddTbpOfst_80047EE8 / InitHiraRmd_80047FBC (both REFUSED / OWNER-ACCEPTED INCOMPLETE 2026-07-22). Returned owner-gated.

## s2 (structural, 2026-07-24) — FLOOR HELD 2. Last structural axis (frame shape) measured DEAD.

- [s2] Floor=2 REPRODUCED this session: applied candidate.c (scr[] fold form) to src/code6cac.c, `sandbox --disable all` = {"score":2,"target_insns":61,"build_insns":60,"rules_dropped":6,"cheat_asm_stripped":182}. src reverted clean. Sole residual remains the empty 8-byte frame. (tmp/grind/func_80017FA0/s2/sandbox_candidate.log)

- [s2] frameprobe2 extension (tmp/grind/func_80017FA0/s2/frameprobe2.{c,s,sh}, cc1 -O2 -G0 -funsigned-char -mcpu=3000) — the mandated remaining small-local shapes, question: does any LEGITIMATE shape reserve vars=8 with ZERO frame stores (target's residual)?
  - g0 baseline no-local -> vars=0 (control).
  - g1 DEAD `struct{s32 a,b}` unused -> vars=8, `subu sp,-8`/`addu sp,8`, ZERO stores. Reproduces target EXACTLY but is a dead unused aggregate = FORBIDDEN dead-vars-local-array (aggregate variant).
  - g2 DEAD `union{s32 w[2];s32 v}` unused -> vars=8, ZERO stores. Same forbidden dead aggregate.
  - g3 two address-taken scalars `s32 x,y; (&x==&y)?...` -> vars=8, ZERO stores. x,y semantically dead, addr-compare folds at compile time; contrived dead-var frame coercion = forbidden.
  - g4 `struct pair pp = *(struct pair*)ptr;` genuinely READ once -> vars=8 but WITH `sw $2,0($sp); sw $3,4($sp)` + reloads = emits frame stores the target LACKS. Wrong (same failure mode as s1's F3 written array).

- [s2] DICHOTOMY COMPLETE: every zero-store 8-byte shape (dead scalar rounds to vars=0 per s1 F1; dead array F2 / dead struct g1 / dead union g2 / addr-taken g3 all vars=8 zero-store) is a DEAD local whose only effect is reserving frame bytes = forbidden dead-vars-local-array. The WRITTEN carve-out (.claude/rules/dead-vars-local-array.md 2026-07-01) is INAPPLICABLE — it requires target to contain the corresponding dead stores, and target has ZERO frame stores (s1 confirmed). Every genuinely-USED aggregate (g4) emits stores target lacks. No legitimate structural shape reproduces target's zero-store 8-byte frame. The structural axis for the frame residual is measured DEAD.

- [s2] DISPOSITION (structural modality): H-F1 KILLED. This is the endgame-lock condition the s1 frontier named — func_80017FA0 is now RA/frame-locked at honest floor 2, byte-matchable only via forbidden dead-vars, hand-coded signal NEGATIVE (s1 scan_hand_coded LOW 1/8). Identical zero-store phantom-frame species to siblings InitHiraRmd_80047FBC (OWNER-ESCALATION filed 2026-07-20, awaiting ruling) and AddTbpOfst_80047EE8. The mandated next step is the ESCALATION modality: file OWNER-ESCALATION in docs/grind/decisions.md and return owner-gated. Not filed this session (modality=structural, not escalation).

## s1 (recon, 2026-07-24) — FLOOR 13 -> 2. WIP "no pure-C fold" claim FALSIFIED.

- [s1] Baseline honest floor = 13 (sandbox --disable all on HEAD cheat-form; build 59 vs target 61 insns). Old WIP candidate.c (indexed `((volatile s32*)0x1F800000)[idx]` form) = 16, WORSE. NB the queue/WIP floor of 13 was HEAD's cheat-form honest distance, not a pure-C candidate.

- [s1] The 13 diffs decompose (full objdump alignment, tmp/grind/func_80017FA0/s1/build_disasm.txt): the inner double-loop (21 insns), outer tail (11), loop-setup (4), blez region — all BYTE-IDENTICAL to target already. The 13 differ split into: (a) 0x1F800060 fixed store = 3-insn `lui;ori;sw 0(r)` vs target 2-insn folded `lui;sw 0x60(r)` [3 diffs, bottom, isolated — no reg cascade]; (b) 0x1F8000B8 fixed store same 3-vs-2 fold [top]; (c) v0<->v1 register cascade on temp/ptr[0]/sll [5 diffs] — DOWNSTREAM of (b): the early delay-slot lui materializing the B8 address in v0 forces temp into v1; (d) the empty 8-byte frame [2 diffs].

- [s1] DECISIVE fold mechanism (probe tmp/grind/func_80017FA0/s1/foldprobe.*, cc1 -O2 -G0 -funsigned-char -mcpu=3000 then maspsx 2.34 + as + objdump): a raw `*(volatile s32*)0xCONST = x` (P1) synthesizes the address into a reg (`lui;ori`) then `sw x,0(reg)` = 3 insns NO fold. Routing the SAME store through a pointer-typed lvalue — local pointer var `scr[idx]` (P4) OR struct-field cast (P5) — makes cc1 emit `sw x,<const>` which the assembler folds to `lui at,0x1f80; sw x,0x60(at)` = 2-insn folded = TARGET bytes. P2/P3 (extern symbol) also fold but leave R_MIPS_HI16/LO16 relocs (needs a linker symbol; unnecessary).

- [s1] APPLIED to real fn: replaced the two fixed writes `*(volatile s32*)0x1F8000B8` / `0x1F800060` with `scr[0x2E]` / `scr[0x18]` where `volatile s32 *scr = (volatile s32*)0x1F800000;` (removed the two cheat asm barriers). sandbox --disable all = 2 (build 60 vs target 61). Closed 11 of 13 diffs — BOTH folds AND the entire v0<->v1 cascade AND one delay slot — in one pure-C edit. Saved to memory/grind/func_80017FA0/candidate.c. src reverted clean (not candidate-ready).

- [s1] canonical-asm signal NEGATIVE: scan_hand_coded --single = tier=LOW score=1/8 (only S3 no-spills fires; ordinary GCC-scheduled fn). So the 8-byte frame came from a C-source local decl, NOT hand-written asm.

- [s1] RESIDUAL (distance 2) = the empty 8-byte leaf frame: target `addiu sp,-8` (in the beqz delay slot) ... `addiu sp,8`, ZERO frame stores. Frame-trigger probe (tmp/grind/func_80017FA0/s1/frameprobe.*): F0 no-local=vars0; F1 dead SCALAR=vars0 (scalar->reg/DCE); F2 dead 8-byte array `s32 buf[2]` unused=vars8 with ZERO sw/lw to $sp (EXACTLY target's residual); F3 written array=vars8 but emits sw stores target lacks. => the ONLY pure-C producer of target's zero-store 8-byte frame is a dead <=8-byte local ARRAY = forbidden dead-vars-local-array. WRITTEN-array carve-out inapplicable (target has ZERO frame stores). Same class + shape-of-argument as sibling AddTbpOfst_80047EE8 (owner-escalated, awaiting ruling) and endgame-lock-disposition.

- WIP rejected_form: {'form': 'idx_base: all scratchpad writes via ((volatile s32*)0x1F800000)[idx], early `return`', 'score': 33, 'reason': 'LICM-hoists the inner-loop lui 0x1f80 out of the loop (target re-materializes it each iteration); also drops the frame. Worse than hybrid.'}

- WIP rejected_form: {'form': 'HEAD committed form (6 regfix rules + asm barriers)', 'score': 0, 'reason': 'byte-matches but ONLY via cheat-regfix (scratchpad fold subst/delete) + cheat-asm (_frame dead-frame barrier, val coercion barrier). inline-asm-policy + scratchpad-gte ARCHIVED forbid it.'}

- == imported from memory/wip notes.md ==
# func_80017FA0 (code6cac.c) — WIP / BLOCKED (canonical-asm candidate)

## TL;DR
HEAD byte-matches but ONLY via **6 regfix subst/delete rules** + **asm
barriers** (a `_frame` dead-frame coercion and a `val` `"0"(val)` barrier). Both
are cheats (inline-asm-policy expanded catalog; the ARCHIVED [[scratchpad-gte]]
rule forbids this exact recipe). Honest pure-C floor measured at **16** (full
build, rules neutralized) — and the residual 16 is **two irreducible non-pure-C
gaps**, so this is a **canonical-asm authorization** function, not pure-C-closable.

## What matches in pure C (the inner loop — the bulk)
Keep the inner scratchpad writes as **full-constant addresses**
`*(volatile s32 *)(0x1F800064 + sp_inner) = ...`. That makes GCC re-materialize
`lui 0x1f80` every iteration, which is what the target does (the target does NOT
LICM-hoist the scratchpad base out of the loop). The whole inner double-loop then
matches byte-for-byte. See candidate.c.

## The two irreducible gaps (why it's not pure-C)
1. **Constant-store-address fold.** The two FIXED writes `0x1F8000B8` (=`ptr[0]<<7`)
   and `0x1F800060` (=`a0[3][1]`): target emits `lui at,0x1f80; sw v0,offset(at)`
   (2 insns, low 16 bits folded into the sw displacement). GCC 2.7.2 emits
   `lui r,0x1f80; ori r,r,0xb8; sw v0,0(r)` (3 insns, no fold). `((volatile s32*)
   0x1F800000)[idx]` folds *inconsistently* (folded in the all-index variant,
   unfolded in the hybrid — it's register-pressure-sensitive, not reliable). The 6
   regfix `subst`/`delete` rules manufacture the fold. No pure-C idiom reliably
   produces it (confirmed across this + the cpu_check_tubazeri scratchpad family).
2. **Unused 8-byte frame.** Target does `addiu sp,sp,-8` … `addiu sp,sp,8` with
   **nothing stored to the frame**. GCC emits no frame for this body. HEAD forces
   it with `s32 _frame; asm volatile("" :: "m"(_frame))` — a dead-var + barrier,
   forbidden by the expanded catalog.

Both are hallmarks of hand-written asm (or a different cc1 fold behavior).

## Endpoint
Per [[scratchpad-gte]] path (b): authorize as **canonical-asm** (user decision —
outside worker scope). Blocked on the board with this reason. 24 prior commits
(2026-03/04) never reached a clean pure-C match — consistent with no pure-C path.


- [s1] Baseline honest floor 13 (HEAD cheat-form, sandbox --disable all); old WIP candidate.c indexed form scored 16 (worse).

- [s1] The inner double-loop (21 insns), outer tail (11), loop-setup (4) already byte-match target; all 13 diffs were the 2 fixed-store folds + the fold-induced v0/v1 cascade + the 8-byte frame.

- [s1] Pure-C pointer-lvalue store (scr[idx], volatile s32 *scr = (volatile s32*)0x1F800000) reproduces the target's 2-insn folded lui;sw disp form for BOTH fixed writes; sandbox --disable all = 2, no rules, no cheat asm.

- [s1] Sole residual (distance 2) is the empty 8-byte frame; only a dead <=8-byte local array reproduces it (F2), which is forbidden dead-vars-local-array with no applicable carve-out.

- [s1] canonical-asm signal NEGATIVE (scan_hand_coded tier=LOW 1/8) — ordinary compiled C, so a C source produced the frame; not a hand-asm authorization case.

- [s1] Same zero-store phantom-frame residual class as sibling AddTbpOfst_80047EE8 (OWNER-ESCALATION filed 2026-07-20, awaiting ruling) and InitHiraRmd_80047FBC.

- [s2] Floor=2 reproduced this session: candidate.c (scr[] fold form) applied to src -> sandbox --disable all = {score:2, target_insns:61, build_insns:60, rules_dropped:6, cheat_asm_stripped:182}; src reverted clean.

- [s2] frameprobe2 (cc1 -O2 -G0 -funsigned-char -mcpu=3000): dead struct g1 / dead union g2 / two address-taken scalars g3 all give vars=8 with ZERO frame stores = reproduce target's residual exactly, but each is a dead local (forbidden dead-vars-local-array, aggregate/addr-taken variant).

- [s2] A genuinely-USED 8-byte aggregate (g4, struct read once) gives vars=8 but emits sw $2,0($sp)/sw $3,4($sp) + reloads = frame stores the target LACKS (target has zero frame stores).

- [s2] Dichotomy complete: zero-store vars=8 <=> local is DEAD <=> forbidden; genuine use of a >=5-byte local => emits stores target lacks. No legitimate C frame shape exists. Structural axis for the frame residual is dead.

- [s2] dead-vars-local-array WRITTEN carve-out (2026-07-01) is inapplicable: it requires target to contain the corresponding dead stores; s1 confirmed target has ZERO frame stores in the locals region.

- [s2] hand-coded signal NEGATIVE (s1 scan_hand_coded LOW 1/8) — ordinary compiled C, so a C source produced the frame; not a hand-asm authorization case.

- [s2] Same zero-store phantom-frame species as siblings InitHiraRmd_80047FBC (OWNER-ESCALATION filed 2026-07-20, awaiting ruling) and AddTbpOfst_80047EE8.

- [s3] s3 reproduced honest floor 2: candidate.c (scr[] pointer-lvalue fold form) applied to src, sandbox --disable all = {score:2, target_insns:61, build_insns:60, rules_dropped:6, cheat_asm_stripped:182}; src reverted clean to HEAD cheat-form.

- [s3] Target frame decoded directly from asm/funcs/func_80017FA0.s: addiu sp,-0x8 (beqz delay slot) / addiu sp,0x8 / jr ra. LEAF — zero sw$ra/sw$s?, zero jal => callee-saves=0, outgoing-args=0. Every sw targets ($at)/($t3) scratchpad, every lw sources ($a0)/($t1)/($v1)/($t4)/($v0); ZERO sw/lw ($sp) => .frame=8 bytes, 100% dead (unwritten, unread).

- [s3] NEW s3 finding: s2 evaluated only the 2026-07-01 WRITTEN-never-read carve-out. The 2026-07-13 OVERSIZED-LOCALS carve-out (contemplating a fully-dead pad) was never checked. Its frame-math prerequisite is satisfied only trivially here (0<N for any zero-store phantom frame), landing func_80017FA0 in the fully-dead-pad family (siblings func_8003DBE4 s32 buf[2], file_LoadSectors/func_800165F8 s32 _pad[2]) with no distinguishing SOTN precedent and engine-allowlist wiring required.

- [s3] canonical-asm NEGATIVE (s1 scan_hand_coded LOW 1/8, only S3 fires) — ordinary GCC-scheduled leaf, so a C-source local produced the frame; not a hand-asm authorization case. Fails endgame-lock AND-gate 1.

- [s3] Every sanctioned pure-C axis (s1 fold+cascade lever closed 11/13 diffs; s2 structural frame-shape dichotomy g1-g4; s3 OVERSIZED-LOCALS) and canonical-asm are measured dead. Identical zero-store phantom-frame species to AddTbpOfst_80047EE8 / InitHiraRmd_80047FBC (both REFUSED / OWNER-ACCEPTED INCOMPLETE 2026-07-22).

- [s3] OWNER-ESCALATION filed this session in docs/grind/decisions.md (2026-07-24) naming func_80017FA0, presenting option (a) sanction fully-dead-pad family [no distinguishing SOTN precedent; requires forbidden engine surface] vs (b) refuse + INCOMPLETE-owner-accepted per endgame-lock-disposition (2026-07-20 standing policy).

## s5 (2026-08-20) — annotation-fix dispatch; premise void, volatile axis re-opened and largely solved

**The dispatch premise was wrong, and that is the first durable fact.** s5 was
sent in `annotation-fix` modality against the layer-1 note of 02:40 ("citation
only"). But the LATER Judge call at 02:54 FAILed the s4 candidate on a
load-bearing construct — the scratchpad `volatile` — and the driver has since
BANNED it. Restoring candidate.c and correcting a citation would therefore have
produced a diff re-declaring a banned construct, which the driver rejects before
the Judge is spawned. No src edit was made; src/code6cac.c stays INCLUDE_ASM.
The citation itself WAS corrected in self_vet.md (the s4 vet cited tslLineG5Init,
which is producer #2 / combine orphan-USE; the correct exhibit for the rotated
guard is func_8003DBE4, .claude/rules/phantom-slot-frame-lever.md:37-41, plus
the in-tree instance at src/code6cac_c2.c:1325).

**The frame lever is independent of the volatile.** Every non-volatile variant
measured this session still reports `.frame $sp,8 ... # vars= 8`. The s4 frame
finding survives the ban intact.

**The volatile was doing exactly one job, now named.** With a NUMERIC constant
address, cc1's loop.c treats `(mem (plus (reg sp_inner) (const_int
0x1F800064)))` as a general induction variable; it combines the three inner
address givs and hoists one biased base out of the inner loop —
`lui v0,0x1f80 ; ori v0,v0,0x6c ; addu a1,t2,v0` — rewriting the stores as
`sw v0,-8(a1) / -4(a1) / 0(a1)`. Result 57 insns vs target 61. Target keeps
`move a1,t2` and re-materialises per store (`lui at,0x1f80 ; addu at,a1,at ;
sw v0,100(at)`). `volatile` blocked the giv. Six numeric-address spellings were
measured and ALL are strength-reduced (harness tmp/grind/func_80017FA0/s4/):
vNV plain 57 · vb1 `u32 sp_inner` 57 · vb2 `(u8 *)0x1F800064 + sp_inner` 57 ·
va2 operand order `sp_inner + 0x1F800064` 57 · va1 `sp_off = sp_inner` so the
unbiased value is consumed after the inner loop 57 · vc1 named intermediate
`s32 ad = 0x1F800064 + sp_inner` with ad/ad+4/ad+8 60 · vc4 no sp_inner,
`0x1F800064 + i*0x18 + j*0xC` 63 (three separate hoisted bases). The
numeric-constant address family is dead for this residual.

**An EXTERN SYMBOL address blocks the giv with no volatile anywhere.** Writing
the inner stores as `*(s32 *)((u8 *)D_1F800000 + 0x64 + sp_inner)` with
`extern s32 D_1F800000[];` yields a `symbol_ref` address, which loop.c does not
treat as an induction variable. cc1 then emits `sw $2,D_1F800000+100($5)` per
store. Variant vb4: **61 insns (target 61), 58/61 byte-identical, vars= 8, zero
`volatile` in the function.** Banked as the new memory/grind/func_80017FA0/candidate.c.

**The residual is 3 instructions and it is a GNU as expansion-path artifact.**
The only diffs are the three inner stores' middle instruction: target
`addu at,a1,at`, vb4 `addu at,at,a1`. maspsx does NOT expand these — verified
with tmp/grind/func_80017FA0/s4/pipe.sh, whose maspsx output still reads
`sw $2,D_1F800000+108($5)` — so GNU as performs the expansion and chooses
`addu at,base,at` for a NUMERIC address expression and `addu at,at,base` for a
SYMBOL expression. Closing the last 3 insns needs a cc1-level spelling that
emits a numeric absolute address while still defeating loop.c strength
reduction. Neither maspsx nor as is a surface a grind session may edit.

**Integration dependency of the symbol form:** it needs `D_1F800000 = 0x1F800000`
to exist for the linker (the scratch harness prepends it in
tmp/perm_17fa0/compile.sh). A real build would need it in named_syms.txt /
symbol_addrs.txt / undefined_syms_auto.txt — outside a grind session's surface.

**Two-prong volatile gate for scratchpad: negative census.**
`docs/reference/sotn-construct-index.md` contains ZERO entries matching
`0x1F800`, `1f8000` or `scratchpad` (grep, case-insensitive). There is no SOTN
precedent to cite for volatile on 0x1F800000-0x1F8003FF, so that gate FAILS and
the ban should be treated as final rather than re-litigated.

- [s4] The fix-up dispatch was based on the 02:40 layer-1 note; the 02:54 Judge FAIL superseded it and the construct it named (volatile on 0x1F800000-0x1F8003FF) is now on the BANNED list, so an annotation/citation-only fix cannot make the s4 candidate submittable. src/code6cac.c was left untouched (INCLUDE_ASM), no scope violation.

- [s4] Citation defect corrected in memory/grind/func_80017FA0/self_vet.md: the rotated-guard construct's family is phantom-slot-frame-lever producer #1 ('Folded loop-guard compare'), scope sentence at .claude/rules/phantom-slot-frame-lever.md:37, exhibit func_8003DBE4 named at lines 40-41, in-tree instance src/code6cac_c2.c:1325. The s4 vet's tslLineG5Init citation was producer #2 (combine orphan-USE) and was wrong. That rule is a DIAGNOSIS RECIPE, explicitly 'NOT a sanction', so no /* FAKE */ annotation is owed - which the Judge stated independently.

- [s4] The 8-byte phantom leaf frame is independent of the volatile: all seven non-volatile variants compile to '.frame $sp,8,$31 # vars= 8', matching target.

- [s4] GCC 2.7.2 loop.c strength reduction is the whole non-volatile shortfall: it combines the three inner scratchpad address givs into one biased base (lui 0x1f80; ori 0x6c; addu a1,t2,v0) hoisted to the outer loop, turning the stores into sw v0,-8(a1)/-4(a1)/0(a1) and the body into 57 insns vs target's 61.

- [s4] Seven numeric-address spellings measured dead (57/57/57/57/57/60/63 insns) - banked with their per-variant results in memory/grind/func_80017FA0/rejected/nonvolatile-numeric-addr-strength-reduced.c.

- [s4] An extern-symbol address (extern s32 D_1F800000[]; stores as *(s32 *)((u8 *)D_1F800000 + 0x64 + sp_inner)) defeats the giv with zero volatile: 61 insns, 58/61 byte-identical to asm/funcs/func_80017FA0.s, vars= 8. This is the new candidate.c.

- [s4] The 3 residual instructions are the same shape three times: target 'addu at,a1,at', candidate 'addu at,at,a1'. maspsx emits the store unexpanded ('sw $2,D_1F800000+108($5)'), so GNU as picks the expansion: 'addu at,base,at' for numeric address expressions, 'addu at,at,base' for symbol expressions. Neither maspsx nor as is an editable surface for a grind session.

- [s4] The symbol form carries an integration dependency: D_1F800000 = 0x1F800000 must exist for the linker (the scratch harness prepends it in tmp/perm_17fa0/compile.sh). In a real build that means named_syms.txt / symbol_addrs.txt / undefined_syms_auto.txt, outside a grind session's allowed surface.

- [s4] docs/reference/sotn-construct-index.md has zero entries for 0x1F800 / 1f8000 / scratchpad - the two-prong volatile route for the scratchpad range is a failed gate, not an open question.

- [s4] Floor reported as the ledger's 2 because no src edit was made this session; note that floor 2 was measured on a form whose volatile is now banned, so the honest floor of any PERMISSIBLE form is the candidate's 3-instruction residual (58/61 identical) until the as-expansion question is closed.


## s5 (2026-08-20, permuter modality) - CLOSED IN PURE C

- **The function byte-matches from pure C.** `sandbox func_80017FA0 --disable all`
  = 0 and a full `build` gives SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  the oracle, with the candidate in src/code6cac.c. Zero regfix/asmfix rules,
  zero inline asm, zero volatile, zero dead locals, zero FAKE constructs.
  Self-vet: memory/grind/func_80017FA0/self_vet.md (no sanctioned-family claim is
  made or needed).

- **The closer is the inner loop's FORM, not its address expression.** Writing the
  inner loop as `inner: { ... } if (j < 2) goto inner;` instead of
  `do { ... } while (j < 2);` removes the front end's NOTE_INSN_LOOP_BEG /
  NOTE_INSN_LOOP_END, and loop.c only analyses note-delimited loops. The three
  scratchpad stores therefore keep their full numeric absolute addresses, and
  maspsx expands each of them (numeric operand > 32767) into the target's exact
  `lui $at,%hi ; addu $at,$a1,$at ; sw $2,%lo($at)` triple. The OUTER loop must
  stay a real do-while: target's ac_base store IS the reduced form
  (`sw v0,0xAC(t3)` with `addiu t3,t3,4`).

- **Pass attribution was READ, not guessed** (tmp/grind/func_80017FA0/s5/vNV.loop,
  produced by `cc1 -dL` via tmp/grind/func_80017FA0/s5/dumpvar.sh). The do-while
  spelling's dump for the inner loop reads: three `dest address src reg 86 ...
  mult 1 add 528482404/528482408/528482412` givs, `giv at 85 combined with giv at
  94`, `giv at 76 combined with giv at 94`, `giv at 94 reduced to (reg:SI 102)`.
  The same dump shows the data-pointer giv being LEFT alone ("giv of insn 70 not
  worth while, 0 vs 15"), which pins add_cost = 2 for this target.

- **Why every numeric spelling s4 tried had to fail** (tools/gcc-2.7.2/loop.c,
  read directly): strength_reduce:3823 leaves a giv alone only if
  `lifetime * threshold * benefit < insn_count`; threshold = 2*(3 +
  n_non_fixed_regs) and lifetime = 1 for DEST_ADDR givs, so the test is really
  "benefit <= 0". One address giv alone is benefit 2 - add_cost 2 = 0 (left
  alone), but combine_givs:5494 merges the three into benefit 6 first, and
  combine_givs_p:5457's MIPS gate always passes (ADDRESS_COST(reg+small) = 1 <=
  ADDRESS_COST(reg+0x1F800068) = 2, mips.c:1653). The only structural escape is
  express_from:5417's `GET_CODE (g1->add_val) != CONST_INT` bail - the reason the
  extern-symbol form worked - and that one is unusable because a symbol operand
  reaches GNU as, not maspsx's expander.

- **s4's assembler attribution was wrong and is corrected.** Measured with
  tmp/grind/func_80017FA0/s5/asmorder.sh: GNU as emits `addu at,at,a1` for BOTH
  `sw $2, 0x1F800064($5)` and `sw $2, D_1F800000+0x64($5)` (with D_1F800000
  defined absolute in the same file). The target's `addu at,a1,at` comes from
  maspsx (tools/maspsx/maspsx/__init__.py:1183), which expands numeric >32767
  store offsets itself and passes symbol-addend stores through untouched. There
  was never an integration handoff here and no symbol registration is needed.

- **In-tree precedent for the goto-formed loop** (same TU, both COMPLETED-C):
  func_800206B0 at src/code6cac.c:2125 + :2143 (`loop:` ... `if (t0 < 0x16) goto
  loop;`, including the same bare-block-around-a-temp shape) and func_80021280 at
  src/code6cac.c:2220 + :2224. SOTN master precedent:
  docs/reference/sotn-construct-index.md:1015 (src/dra/5F60C.c:579 -
  `loop_check_equip_id_1:` with its back edge at line 582).

- The s4 extern-symbol form (58/61) is banked at
  memory/grind/func_80017FA0/rejected/extern-symbol-addr-wrong-as-expansion-order.c;
  it is superseded, not merely rejected - its integration dependency
  (D_1F800000 in named_syms.txt) is moot.


## s6 (2026-08-20, synthesis modality) - the match is REAL and REPRODUCIBLE at the current chassis; the blocker is a mis-banked tripwire, not the C

- **RE-MEASURED THIS SESSION, current chassis.** The s5 form (candidate.c) applied
  to `src/code6cac.c` gives `sandbox func_80017FA0 --disable all` =
  `{"score": 0, "target_insns": 61, "build_insns": 61, "scorable": true,
  "rules_dropped": 0}` and a full `build` prints
  `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa / want ... / MATCH` - the oracle.
  The honest floor for this function is therefore **0**, not the 2 the ledger
  carried from s1-s3. Both measurements were re-run AFTER adding the inline
  `/* FAKE */` annotation, so the annotated form is the measured form.
- **The 2026-08-20 03:31 layer-1 FAIL is paperwork-only.** Its own text disputes
  nothing about the construct's honesty: "Rotated outer-loop guard is fine, but
  the goto-formed inner loop was session-documented as chosen SOLELY to suppress
  loop.c's NOTE_INSN_LOOP_BEG-gated induction-variable analysis, which ...
  requires a `/* FAKE: ... */` annotation for ... - the annotation is absent and
  the self-vet wrongly asserts none is owed." Its `next action` field (visible in
  state.json `judge_constraints`, truncated by the driver's 400-char cap) begins:
  **"Next action: Do not treat this as a construct ban - the goto-lo..."**
- **But the driver banked it as a mechanical ban anyway.** `grind.ps1:495-501`
  banks `$v.banned_construct` (or the constraint text when `ground -eq
  'CONSTRUCT'`) unconditionally, and `state.json.banned_constructs` now carries
  "Inner loop rewritten from `do { ... } while (j < 2);` to `inner: { ... }
  if (j < 2) goto inner;` (src/code6cac.c, the new func_80017FA0 body)".
  `grindlib.check_banned_constructs` is a content-word tripwire over the vet's
  CONSTRUCTS: block (`_ban_trips`, >=50% of significant terms), so ANY honest
  CONSTRUCTS: line for this form trips it and the candidate-ready session is
  discarded before layer-1 or the Judge ever runs. That is the exact deadlock
  `unban_construct` was added for on 2026-08-19 (func_8002D518 burned two
  sessions on it), and the only lawful exit is a ruling that clears it.
- **The annotation layer-1 asked for now exists**, inline at the construct site
  (the `inner:` label), carrying what + mechanism (GCC 2.7.2 loop.c
  strength_reduce / find_mem_givs / combine_givs, gated on NOTE_INSN_LOOP_BEG)
  + lever-exhaustion (hypotheses.md H1-H14 / evidence.md). `self_vet.md` was
  rewritten to match: it declares the goto spelling, claims the
  do-while-zero-exception ALLOWED-spelling family with the scope sentence quoted
  verbatim from `.claude/rules/do-while-zero-exception.md:46`, and its
  ANNOTATION-CONFORMANCE line reproduces the FAKE comment instead of saying
  "n/a". It is staged, not spent - see the STATUS header in that file.
- **Citations re-verified in the APPLIED tree this session** (the 02:40 layer-1
  FAIL was a fabricated-citation FAIL, so every cite was re-checked by reading
  the file): `.claude/rules/do-while-zero-exception.md:46` is the ALLOWED
  sentence verbatim; `src/code6cac.c:2125`/`:2143` is `func_800206B0`, a
  goto-formed counted loop with the same bare-block-around-a-temp shape;
  `src/code6cac.c:2220`/`:2224` is `func_80021280`; neither appears in
  `engine/queue.json` (258 items), i.e. both are COMPLETED-C and both ship the
  construct with NO annotation at all. `docs/reference/sotn-construct-index.md:1015`
  is `src/dra/5F60C.c:579 - loop_check_equip_id_1: ... goto from line(s) 582`
  (SOTN master, PSX/GCC). `.claude/rules/phantom-slot-frame-lever.md:37` is
  producer #1 and names exhibit `func_8003DBE4`; the in-tree rotated-guard
  instance is `src/code6cac_c2.c:1327` (`if (i < limit) {`) - NOT :1325 as the
  s5 vet said, which is the `colors` adjustment two lines above. Corrected.
- **Target-asm re-read corroborates the mechanism claim** (asm/funcs/func_80017FA0.s):
  the OUTER loop IS strength-reduced (`addiu $t3,$t3,%lo(D_1F800004)` with
  `sw $v0,(0x1F8000AC & 0xFFFF)($t3)`), while the INNER loop's three stores are
  NOT (`lui $at,%hi ; addu $at,$a1,$at ; sw $v0,%lo($at)` three times, with
  `addiu $a1,$a1,0xC` in the exit branch's delay slot). So the original object
  code was produced by a compiler that ran loop.c over the outer loop and NOT
  over the inner one - exactly the note-delimitation asymmetry a goto-formed
  inner loop inside a do-while outer loop produces. That is independent,
  target-side corroboration that the s5 form is the ORIGINAL shape rather than a
  coercion that happens to land.
