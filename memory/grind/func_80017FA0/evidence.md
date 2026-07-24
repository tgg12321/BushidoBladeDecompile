# Evidence bank — func_80017FA0

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
