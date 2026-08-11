# Evidence bank — func_8001E6E4

## Session 1 (grinder, recon, 2026-08-11)

- **Baselines re-measured on main.** canonical = C (71 insns, distance 21 at
  HEAD form). Sandbox `--disable all`: HEAD src form = 21; applying the
  candidate.c `p0` named-intermediate form = **19** (reproduced; left in
  src/code6cac.c as the session's final state, so HEAD-of-session floor is 19).
- **CORRECTION to the prior session's DCE claim.** The WIP note "GCC fully
  dead-code-eliminates a truly-unreferenced local regardless of declared size /
  pre_pad is mechanically inert" is **WRONG**. The frame gradient
  (`tmp/frame_probe.sh func_8001E6E4 code6cac`, cc1 `.frame` line) proves:
  WITH `s32 pre_pad[2];` → frame 112 / vars=80 (== target); WITHOUT →
  frame 104 / vars=72. GCC 2.7.2 allocates unreferenced local arrays
  (consistent with [[phantom-frame-slots-gcc272]]). The prior session's
  "inert" measurement was an artifact of the SANDBOX's cheat-stripper deleting
  the unused array from the source before compiling (139 constructs stripped
  file-wide). Policy verdict unchanged — pre_pad stays a forbidden-family
  cheat — but the mechanical reasoning in the earlier note must not be reused.
- **How HEAD stays oracle-green:** the committed `pre_pad[2]` IS load-bearing
  in the real (non-stripped) build — it produces the target's 112-byte frame.
  The function byte-matches at HEAD only via this cheat construct; the honest
  (stripped) floor is 19. Same situation in sibling func_8001E404 (its
  committed `pre_pad[2]` at src/code6cac.c:1385, honest distance 23).
- **Gap fully characterized:** target frame 112 with CamWork at sp+0x18, callee
  saves s0/s1/s2/ra at 0x60-0x6C; honest build frame 104, struct at sp+0x10,
  saves at 0x58-0x64. The phantom region is sp+0x10..0x17 (8 bytes, zero
  loads/stores). All 19 differing insns are the uniform +8 sp-offset shift;
  build_insns == target_insns == 71; zero codegen divergence. So the ONLY
  problem is producing vars=80 instead of 72 with codegen-neutral C.
- **Three honest frame-producer spellings measured DEAD** (details in
  rejected/frame-producer-probes-s1.c + tmp/grind/func_8001E6E4/s1/
  frame_probes.md): named range-compare local (no loop → compare pseudo
  consumed by sltiu, never orphans); named u16 component temps (loads widen
  to SImode instantly, no HImode expression exists → no HImode stack temp);
  named `s32 dist` scalar (fully register-allocated to $a2, no stack home).
- **Table-type lead is DEAD:** D_800F5328 / D_800F6608 are declared bare
  `extern s32` (include/code6cac.h:236,249); written by func_8001B3C0 via raw
  offsets; no struct type exists anywhere in-tree that reveals leading fields.
  No duplicate leads for 8001E6E4/8001E404 in tmp/duplicates_leads.txt.
- **m2c reference shape** (tmp: m2c --valid-syntax on asm/funcs/func_8001E6E4.s):
  flat sp18..sp30 locals, `temp_a2` for dist (computed → stored to sp30 →
  passed in a2), `temp_s0 = var_s2 + 0x20`. No structural hints beyond what we
  have; the named-dist shape it suggests was probe 4 (frame-neutral).

- WIP rejected_form: {'slug': 'dead-vars-local-array_pad_lead', 'file': 'rejected/dead-vars-local-array_pad_lead.diff', 'verdict': 'FAIL (cheat-reviewer)', 'summary': 'Added `s32 pad_lead[2];` as the first member of CamWork (address-taken only via &local.vx offset trick) to reserve the missing 8 leading frame bytes. Mechanically closed sandbox distance to 0 and full-build SHA1 == oracle, but reviewer ruled it a spelling-variant of the forbidden unused-local-array frame-coercion cheat (fails tests 1/3/5/6 of the 6-test checklist) since there is no target-side dead-store evidence for what pad_lead represents, distinguishing it from the cited func_8001A538/MATRIX precedent (commit 7f37c94c) which used a real, externally-verified multiply-reused PsyQ struct.'}

- == imported from memory/wip notes.md ==
# func_8001E6E4 — TL;DR

Distance 21 -> 19 (reviewer-approved, `candidate.c` applied). Remaining 19 is a
SINGLE root cause: target's stack frame is 112 bytes with the `CamWork local`
struct starting at sp+24; the honest pure-C reconstruction only produces a
104-byte frame with `local` at sp+16 (8-byte / 2-word gap). Every differing
instruction is just this same +8 sp-offset shift on the exact same opcodes —
there is no other structural divergence anywhere in the function.

## What's confirmed

- **No corresponding dead store exists in target.** Checked byte-by-byte via
  objdump AND via a fresh m2c re-decompile of `asm/funcs/func_8001E6E4.s` —
  zero loads/stores touch sp+0..sp+23 anywhere in the function. The
  `dead-vars-local-array` written-never-read carve-out requires target-side
  dead stores as prerequisite #1; that prerequisite is not met here, so that
  carve-out does not apply.
- **Unreferenced padding arrays are mechanically inert here, not just
  forbidden.** Tested `s32 pre_pad[N];` for N = 2, 4, 20 words (8/16/80
  bytes) — ALL produced byte-identical output with ZERO effect on frame size
  or sandbox score. GCC fully dead-code-eliminates a truly-unreferenced local
  regardless of declared size. So this isn't just a policy violation to
  avoid — it doesn't even work as a mechanical trick for this case.
- **Struct-field address-taking DOES mechanically work** (confirmed: folding
  a `s32 pad_lead[2]` as the FIRST member of `CamWork` and re-pointing the
  call-site pointer to `&local.vx` closed the gap to sandbox 0 / SHA1 ==
  oracle). This mirrors the already-committed `func_8001A538` MATRIX fix
  (`7f37c94c`) mechanically, but **cheat-reviewer FAILed it** because unlike
  MATRIX (a real, externally-verified, multiply-reused PsyQ SDK struct),
  `CamWork.pad_lead` has no such grounding — it's a single-use invented
  struct with no evidence for what those bytes represent. Saved as
  `rejected/dead-vars-local-array_pad_lead.diff`. Do not re-propose this
  exact form without NEW evidence (e.g. discovering the true shared struct
  elsewhere in the codebase with externally-verifiable field semantics for
  those leading 8 bytes).

## Sibling clue (not this function, but relevant)

`func_8001E404` (same file, same CamBuf/CamWork idiom, still in the queue at
distance 23 despite a 2026-05-26 commit message claiming full SHA1 match) has
the IDENTICAL unresolved 8-byte-short frame with an unreferenced
`s32 pre_pad[2];` already sitting in the committed tree. Since both siblings
need the same +8 leading reservation, whatever the TRUE answer is (a real
shared struct? a different original decomposition entirely?) likely closes
both at once. Worth a joint investigation session across both functions
rather than treating them independently. Flagged to orchestrator, not
resolved here (out of this worker's assigned scope).

## Next lever to try (not yet attempted)

- Search the decomp.me corpus (`tools/decomp_me_scrape.py`) for scratches
  matching this exact camera-struct-pass-by-address-to-3-callees shape —
  might surface the real original struct layout used by another PSX/PsyQ
  title targeting the same toolchain class.
- Check whether `D_800F5328`/`D_800F6608` (the two source tables `s2` can
  point to) are themselves declared with a known struct type elsewhere that
  reveals genuine leading fields (haven't checked their OWN declared type/
  size beyond raw pointer arithmetic).


- [s1] canonical: verdict C, 71 insns; sandbox HEAD form 21, candidate p0 form 19 (reproduced this session, left in src)

- [s1] Gap fully characterized: target frame 112 (CamWork at sp+0x18, saves 0x60-0x6C) vs honest 104 (struct sp+0x10, saves 0x58-0x64); phantom region sp+0x10..0x17 untouched by any insn; all 19 diffs are the uniform +8 sp shift, zero codegen divergence

- [s1] CORRECTION: GCC 2.7.2 does NOT DCE unused local arrays — pre_pad[2] produces frame 112/vars=80; prior session's 'inert' finding was the sandbox stripping the array pre-compile (139 constructs stripped file-wide). HEAD byte-matches only via this committed cheat; sibling func_8001E404 (honest 23) carries the identical committed pre_pad at src/code6cac.c:1385

- [s1] Frame gradient instrument validated both directions for this function: probe pipeline reproduces real-build 112 and stripped-build 104

- [s1] Table-type lead DEAD: D_800F5328/D_800F6608 are bare extern s32 (include/code6cac.h:236,249), no revealing struct type in-tree; no duplicate-scan leads

- [s1] m2c reference shape: flat locals, temp_a2 dist (store+pass), temp_s0 = s2+0x20; suggests named-dist original which is frame-neutral (probe 4)

## Session 2 (grinder, structural, 2026-08-11)

- **Structural modality measured DEAD: 15 honest semantic-preserving spellings,
  ALL vars=72 (frame 104).** Instrument: `tmp/frame_probe.sh` (cc1 `.frame`
  gradient), harnesses `tmp/grind/func_8001E6E4/s2/sweep.py` + `sweep2.py`,
  raw results in `sweep_results.txt` + `sweep2_results.txt`. Variants swept:
  honest baseline; select-pointer named intermediate (`sel` copied to `s2`
  after the join — the combine-orphan-USE candidate shape); sel used for all
  loads with late `s2` bind; second `u8 *base` handle for the halfword reads;
  named `out` intermediate for the final global store; two-sided compare
  re-association (`arg0 < 0x555 || arg0 >= 0xAAB`); p0/p20 pointers declared
  at function top; named `u32 range` guard intermediate; reversed declaration
  order (pointer before aggregate); split-init accumulation on dist
  (`local.dist = load; local.dist += G;`); ternary select; separate locals
  (pos/rot/dist/tail structs — declaration granularity, both declaration
  orders; diagnostic-only, dead `tail` never a candidate); `u8 *` as primary
  handle; `register` storage class on s2. NONE moved `vars` off 72.
- **Interpretation.** No named-intermediate / copy-coalesce / declaration-order
  / granularity / handle-type / guard-respelling shape orphans a pseudo in this
  function: every pseudo in every spelling is either consumed (guard chain:
  addiu→sltiu→bnez all survive in target bytes, so the guard is NOT the
  producer) or fully register-allocated. Separate-locals packing has no
  per-local padding (16+8+4+44 packs to exactly 72), so declaration
  granularity cannot supply the 8 leading bytes either.
- **Args-area alternative ruled out by elimination (recorded reasoning, no new
  measurement):** target frame 112 could in principle be vars=72+args=24, but
  args=24 requires a >4-arg call, which would emit an arg store into
  sp+0x10..0x17 — and the target has zero touches there. So target is
  vars=80, and the producer is vars-side, as H1 states.
- **Frontier unchanged**: H1 (-da greg/combine dump forensics to find which
  expansion temp / unallocated pseudo cc1psx's input shape created), H2
  (callee real argument types → true camera struct evidence), H3 (Kengo
  source cross-reference). Structural blind-spelling search on this function
  is exhausted — do not re-sweep spellings of these families.

- [s2] 15 structural spellings swept with the cc1 .frame gradient; every one reports vars=72 regs=4/0 args=16 (frame 104) - raw lines in tmp/grind/func_8001E6E4/s2/sweep_results.txt and sweep2_results.txt

- [s2] Target guard chain (addiu a0,-0x555; sltiu; bnez) survives in target bytes, so the phantom producer is NOT the guard pseudo chain; any orphan must come from elsewhere

- [s2] Separate-locals declaration granularity is frame-neutral: pos(16)+rot(8)+dist(4)+tail(44) packs to exactly 72 with no per-local padding in GCC 2.7.2

- [s2] args=24 alternative for target frame 112 ruled out by elimination: a >4-arg call would store into sp+0x10..0x17 which target never touches, so target is vars=80 and the producer is vars-side

- [s2] src/code6cac.c left byte-identical to HEAD (sweep harness restored it; git diff clean); candidate.c p0 form (floor 19) unchanged as banked best

## Session 3 (grinder, structural, 2026-08-11)

- **H2 (callee-type / true-camera-struct evidence) is DEAD — the callees
  affirmatively contradict a struct with 8 leading bytes.** All three callee
  bodies read this session:
  - `func_80046BF4(s32 *a0, u16 *a1, s32 a2)` (src/sound.c:274, matched) takes
    DISCRETE pos-pointer / rot-pointer / dist args; it reads exactly a0[0..2]
    (via walking `s32 *ap`) and a1[0], a1[1], a1[2] — no aggregate view, no
    negative offsets, nothing before the passed base.
  - `func_8001A538(s32 *arg0, s32 *arg1)` (src/code6cac.c:687, matched) reads
    arg0 at offsets 0x0/0x4/0x8 (pos), 0x10/0x12/0x14 (s16 rot), 0x18
    (arg0[6] = dist) — i.e. it treats the passed pointer as a base with **vx at
    offset 0**. If the original struct had 8 leading bytes before vx, this
    callee's own layout would not show vx at +0 of its arg. No leading fields
    exist from the callee's perspective.
  - `func_80061064` is defined `void func_80061064(void)` (src/text1b.c:3638,
    matched) — it IGNORES its arguments entirely (iterates D_800F1150 flags);
    the caller-side args are vestigial. Zero struct evidence.
  - The table writer `func_8001B3C0` (src/code6cac.c:796) writes
    D_800F5328/D_800F532C/D_800F5330 as individual bare word globals copied
    from a0+0x180/0x184/0x188 — no struct type, consistent with s1's
    bare-extern finding.
  - `func_8001E6E4` has NO in-tree C callers (asm-only callers); no caller-side
    aggregate evidence source exists.
  Conclusion: the pad_lead rejection's "NEW evidence" condition is not just
  unmet — the reachable evidence points the other way. Do not revisit the
  struct-lead layout on the evidence path; only a forensic (H1) producer can
  ground the 8 bytes.
- **H3 (Kengo cross-reference) is DEAD as an evidence source.** There is NO
  Kengo source tree in-repo — Kengo/ holds ELF symbol dumps + an asm-only
  line-info dump (kengo_debug_full.txt covers only .dsm/.vsm hand-asm files;
  zero C-local/stab info). Disassembly probes (artifacts in
  tmp/grind/func_8001E6E4/s3/): the region's `kengo:MED
  nm_mario_test/mario_test_Exec` annotation (src/code6cac.c:1374) is a
  Pad_Rpt/fnt_print DEBUG MENU function, not the camera pair — the annotation
  is a false lead for structure. The real PS2 camera layer
  (`marionation_camera_Exec` @0x135560, `mottest/robtest/tanren_camera_control`
  families) was REWRITTEN for PS2: floats + sceVu0 matrix calls + gp-relative
  global camera structs + no-arg helper calls; the PS1 "fill local
  pos/rot/dist work buffer from table row, pass (&pos,&rot,dist) to three
  callees" idiom does not survive anywhere reachable. Kengo cannot reveal the
  original BB2 local declarations for this pair.
- **New structural spelling measured and KILLED — genuinely-used pointer-array
  local `s32 *ps[2]` (the one honest addressable-8-byte shape the s1/s2 sweeps
  did not cover).** Declared before the struct, ps[0]=&local, ps[1]=s2+0x20,
  both consumed by the three calls. Frame gradient: **vars=80, frame 112 — the
  first honest spelling to reach the target frame size** — BUT GCC 2.7.2 keeps
  the array in memory (no scalarization): emits `sw $4,16($sp)` /
  `sw $2,20($sp)` / `lw $4,16($sp)` AND drops a callee-save (regs 4/0 → 3/0,
  s2's pointer now lives in the frame), so total frame is 80+12+16=108→112 by
  alignment with the save block at the WRONG offsets. Not codegen-neutral;
  rejected form banked at rejected/ptr-array-ps2-materializes-stores.c.
  Probe harness: tmp/grind/func_8001E6E4/s3/probe_ps_array.py; asm:
  variant_ps_array.s. This closes the structural spelling space: every honest
  addressable-8-byte local costs instructions; every register-allocatable
  spelling stays vars=72.
- **Table-row extent side-note (evidence-grounding for the EXISTING CamWork
  tail, not the lead):** the sibling reads row+0x30/+0x38 (func_8003F3D4) and
  both functions write row+0x20..0x2B via func_8001A538 — the row type extends
  to ≥0x40 and CamWork's 72-byte total mirrors the row stride, so pad2[11] is
  row-shape-grounded. The missing 8 bytes are NOT part of the row type from
  any observed access.
- **Frontier after s3: H1 forensics ONLY.** -da greg/combine dump census of
  every pseudo + assign_stack_local/assign_stack_temp call sites, per the s1
  probe spec. Structural (s1: 3, s2: 15, s3: 1 forms) and both external
  evidence axes (callees, Kengo) are measured dead. Next session should run
  the forensics modality; nothing else remains on this function's map.

- [s3] func_8001A538 (matched, src/code6cac.c:687) reads its arg0 at offsets 0x0/0x4/0x8 (pos), 0x10/0x12/0x14 (s16 rot), 0x18 (dist) - the passed pointer IS the struct base with vx at +0; no leading fields exist from the callee's view

- [s3] func_80046BF4 (matched, src/sound.c:274) takes discrete (s32 *pos, u16 *rot, s32 dist) pointers and reads exactly 3 words / 3 halfwords from them - no aggregate view

- [s3] func_80061064 (matched, src/text1b.c:3638) is defined void(void) and ignores the two args the camera pair passes - they are vestigial; zero struct evidence

- [s3] The kengo:MED annotation for this region (mario_test_Exec) is a false structural lead: it is a debug-menu function; Kengo's actual camera layer (marionation_camera_Exec, *_camera_control) was rewritten for PS2 (float/sceVu0/gp-global, no-arg helpers) and cannot reveal BB2's local declarations

- [s3] s32 *ps[2] genuinely-used pointer-array spelling: vars=80/frame 112 but +3 memory ops and one fewer callee-save (regs 3/0) - structural spelling space now closed at 19 measured forms (s1:3, s2:15, s3:1): every honest addressable-8-byte local costs instructions, every register-allocatable spelling stays vars=72

- [s3] Table-row extent: rows are >=0x40 (sibling reads +0x30/+0x38; both write +0x20..0x2B via func_8001A538), grounding CamWork's 72-byte tail as row-shape - the missing 8 bytes are NOT part of the row type from any observed access

- [s3] src/code6cac.c restored byte-identical after probes (git status clean); candidate.c p0 form (floor 19) unchanged as banked best

## Session 4 (grinder, permuter, 2026-08-11)

- **Workspace** (tmp/grind/func_8001E6E4/s4/perm/): full-TU base.c
  (preprocessed src/code6cac.c, pre_pad removed = honest form, plus a
  `typedef struct GameObj GameObj;` prepend for pycparser), compile.sh =
  Makefile-mirror pipeline (CC_FLAGS incl. -mel, prologue_fix, full
  MASPSX_FLAGS, multu_pad, NO regfix/asmfix), per-function extraction,
  target.o from asm/funcs/func_8001E6E4.s at offset 0. Validated: 71/71
  insns; honest base raw diff vs target = 21 pairs (18 pure sp-shift + 3
  structural — see wp finding below); campaigns via permuter_campaign.py
  with --stack-diffs (default) + --stop-on-zero.
- **Campaign 1 (s4-honest-random, default weights): score-0 at iteration
  461 = `volatile unsigned short pad;` — REJECTED per policy, never
  submitted** (forbidden family: unused-local frame coercion via
  volatile-typed scalar; banked at
  rejected/permuter-volatile-dead-scalar.c with full analysis). The
  permuter ships `perm_pad_var_decl`, a mutation whose docstring says it
  inserts unused variables "to adjust stack offsets" — a cheat generator
  under project policy; weight-zeroed for all later campaigns.
- **NEW MECHANICAL FACT (refines s1's phantom-frame note):** GCC 2.7.2
  gives an UNWRITTEN `volatile` scalar local an unconditional stack home
  (frame gradient: vars=80/frame 112, zero emitted instructions — the only
  zero-byte-cost vars=80 spelling found). Non-volatile DEAD scalars and
  pointers (`CamWork *new_var;`, `s32 *new_var;`) do NOT move the frame
  (vars=72 measured) — unused non-volatile scalars get no stack home
  (contrast: unused ARRAYS do, s1). A WRITTEN volatile short
  (`s2[new_var3 = 0]`, find output-100) reaches vars=80 but materializes
  the volatile store — permuter score 100, not codegen-neutral. So within
  the local-declaration space the ONLY inert vars=80 producer is the
  volatile dead scalar = a forbidden-family construct. Honest producers do
  not exist in this space — consistent with s2/s3's 19 dead spellings.
- **WP FINDING (permuter output-140, HONEST, banked into src +
  candidate.c): a staged work-pointer removes ALL residual structural
  divergence.** `CamWork *wp;` declared before `local`, `wp = &local;`
  placed between the rx and ry stores, first call takes `(s32 *)wp`.
  Effect at honest frame 104: GCC stages &local into callee-save $s0 in
  the former nop slot (`addiu s0,sp,16` where our build had `nop`) and
  emits `move a0,s0` for the first call's a0 setup (our build had two
  `addiu a0,sp,16`) — exactly target's shape; the remaining diff vs
  target is EXACTLY the 19-insn uniform +8 sp shift, nothing else.
  At frame 112 (pre_pad in place): BYTE-IDENTICAL to target — 71/71,
  diff 0, via the full pipeline (tmp/grind/func_8001E6E4/s4/
  test_wp_prepad.sh) — i.e. oracle-safe; the real build already emitted
  this staging naturally at 112, wp makes the honest 104 build emit it
  too. Applied to src/code6cac.c; engine sandbox --disable all = 19
  (unchanged number — the engine metric counted 19 before and after —
  but the honest gap is now PURELY the phantom 8 bytes). wp is a
  genuinely-used named pointer intermediate (sanctioned
  named-intermediate family); assignment placement is load-bearing.
- **Random-mutation space measured DEAD for honest frame producers:**
  3 seeds (honest base with default weights; honest base with
  perm_pad_var_decl=0; wp chassis with perm_pad_var_decl=0), 43k+
  iterations on the wp chassis alone, 8 jobs. Every frame-moving find
  was a volatile spelling; no mutation class (temp_for_expr, reorders,
  ins_block, type randomization, struct_ref, condition, expand_expr,
  add_sub, ...) produced an honest vars=80. Campaigns harvested +
  stopped with telemetry (metrics/events.jsonl).
- **Interpretation for H1:** the frame-112 producer is NOT any local
  declaration/spelling effect reachable by mutation of this function
  body. The s1-s3 conclusion stands strengthened: only an
  expansion-time temp / unallocated-pseudo mechanism (forensics -da
  census) can ground the 8 bytes honestly.
- src/code6cac.c left with pre_pad + wp form (byte-identical real build,
  sandbox 19); candidate.c updated to match; git tree otherwise clean.

- [s4] GCC 2.7.2 allocates a frame slot for an UNWRITTEN volatile scalar local at zero instruction cost (vars 72->80) — the only codegen-inert vars=80 local-declaration spelling; it is a forbidden-family cheat, so the local-declaration space contains NO honest producer

- [s4] Non-volatile dead scalar/pointer locals do NOT move the frame in GCC 2.7.2 (vars=72 measured for CamWork*/s32* dead decls) — refines s1's unused-ARRAY finding; scalars and arrays behave differently

- [s4] The permuter's perm_pad_var_decl mutation is a purpose-built stack-offset padder (its docstring says so) — a cheat generator under project policy; weight-zero it in settings.toml for any BB2 campaign on a frame-gap function

- [s4] wp staged-pointer form: honest build divergence vs target is now EXACTLY the 19-insn +8 sp shift (previously 18 shift + nop + 2x addiu a0,sp,16 structural); with pre_pad at 112 the form is byte-identical via the full pipeline — the frame size itself gates the $s0 staging/scheduling (vars=80 stages naturally, vars=72 does not, wp forces parity)

- [s4] 43k+ permuter iterations across 3 seeds (8 jobs, stack-diffs scorer, stop-on-zero) found no honest vars=80 producer — the phantom-slot producer is not mutation-reachable from the function body; H1 forensics (-da expansion-temp census) remains the only live axis

- [s4] Full-TU permuter workspace recipe for code6cac functions banked (s4/setup.sh): preprocessed TU + GameObj opaque typedef for pycparser, Makefile-mirror compile.sh with -mel, per-function extraction, offset-0 target.o

## Session 5 (grinder, permuter, 2026-08-11)

- **THE FRAME CONSTRAINT IS TWO-SIDED, and only s5 measured the second side.**
  Every prior session tracked one observable - `vars=80` / frame 112. That is
  NOT sufficient. The target additionally puts `CamWork local` at **sp+0x18**
  (callee-saves at 0x60-0x6C), i.e. the 8 phantom bytes sit BELOW `local`.
  Four different constructs reach vars=80 while leaving `local` at sp+0x10 and
  are therefore 9-21 insn pairs off target regardless of frame size. The new
  screening instrument reports BOTH observables (cc1 .frame line + objdump
  diff-line count vs target): `tmp/grind/func_8001E6E4/s5/screen.py` (11
  variants, sources + dumps banked as `v_<name>.c` / `v_<name>.dump`).
- **MECHANISM (measured, kills H1): GCC 2.7.2 hands out the vars area in
  ALLOCATION ORDER, lowest sp offset first, and the outer block's declared
  locals are allocated before any statement is expanded.** Controls:
  `arr_first_dead` (dead `s32 t[2]` declared FIRST = the committed pre_pad
  shape) -> frame 112, 71 insns, **objdump diff 0**; `arr_last_dead` (same dead
  array declared LAST) -> frame 112 but `local` unmoved at sp+0x10, 9 differing
  pairs; `scope_pair` (8-byte aggregate in a disjoint inner block) -> its slot
  lands at **sp+0x58**, above `local`. Therefore ANY object created after the
  outer-block declarations - inner-scope local, expansion-time aggregate temp,
  reload/alter_reg spill slot - grows the frame at the TOP and can never push
  `local` up. **H1 (the -da expansion-temp / unallocated-pseudo census, the
  ledger's sole live axis since s1) is dead on structural grounds: no
  expansion-temp mechanism can produce the target layout.**
- **What the target layout therefore requires:** an object DECLARED BEFORE the
  work buffer, 8 bytes wide, emitting ZERO instructions. Across s1-s5 exactly
  three constructs in GCC 2.7.2 satisfy "8 bytes, zero instructions": an unused
  local array (forbidden family), an unwritten `volatile` scalar (forbidden
  family, s4), and widening the declared TYPE of the work buffer itself so its
  leading 8 bytes are simply untouched (the s1 `pad_lead` form - mechanically
  perfect, cheat-reviewer FAILed for want of evidence about what those bytes
  are). Every genuinely-used 8-byte first-declared object emits memory traffic.
- **Best honest form by raw diff this session (still not a match):**
  `pair_first` / `arr_first_used` - a first-declared genuinely-used 8-byte
  object (struct or 2-word array) staged into `local.vx/vy`. Frame 112, `local`
  at 0x18, saves at 0x60-0x6C, 71 insns, **14 differing insn pairs vs target
  (the honest wp chassis is 19)** - the whole +8 shift is gone; what remains is
  the staging traffic itself (2 stores + 1 reload replacing target's 2
  scheduling nops and one lui/lw pair) plus a 3-insn s0/s1/a0 staging order
  rotation. Banked at rejected/s5-first-declared-staging-object.c. It is not a
  candidate: target never touches sp+0x10..0x17, so any construct that WRITES
  there diverges by construction.
- **Two fresh-seed permuter campaigns on these NEW chassis (s4 only ever seeded
  the honest base and the wp chassis).** Workspaces
  tmp/grind/func_8001E6E4/s5/perm_pairfirst and .../perm_arrfirst (base.c,
  compile.sh, target.o, settings.toml with `perm_pad_var_decl = 0.0` +
  `perm_inline = 0.0`; telemetry in metrics/events.jsonl).
  * s5-pair-first: base score 1156, ~25 min, 462+ outputs, best score **16** -
    and that find is SEMANTICALLY INVALID: the permuter repointed `wp` at `q`
    (`wp = &q; local.vx = (*wp).a;`) so the first callee receives the wrong
    buffer. No honest sub-100 basin. Harvested + stopped.
  * s5-arr-first-used: base score 1156, 40409 iterations / ~18 min, best score
    **108, flat across both 9-minute windows**. Harvested + stopped.
  Combined with s4's 43k iterations this closes random search from four
  distinct seeds.
- **Sibling / third-party evidence re-checked and still negative:** sibling
  func_8001E404 has the identical frame (0x70, locals from 0x18, saves
  0x60-0x68) and likewise never touches sp+0x10..0x17 - corroborating a shared
  wider type but supplying no field semantics. The only other callers of
  func_8001A538 / func_80046BF4 are func_8001A820 (frame 0x80; its sp+0x10 slot
  is an ordinary 4-byte spill of a func_80053614 return value - unrelated) and
  func_8003C9A4 / func_8003CD10 (frame 0x18, args-area only). No function in
  the tree writes an 8-byte lead before a camera work buffer.
- src/code6cac.c: HEAD did NOT carry s4's `wp` edit (HEAD sandbox = 21); the
  banked candidate.c form was re-applied this session and re-measured at
  **sandbox --disable all = 19**. Left in src as the session's final state.

- [s5] Reaching vars=80 is necessary but NOT sufficient: the target also needs `local` at sp+0x18. Measured constructs that hit vars=80 with `local` still at sp+0x10: dead array declared last (9 pairs off), inner-scope aggregate (21 pairs off), aggregate copy temps (frame 112/120, +5 insns)
- [s5] GCC 2.7.2 allocates frame slots in declaration/allocation order, lowest sp offset first, outer-block decls before any statement expansion - so expansion temps, inner-scope locals and spill slots all land ABOVE the first-declared aggregate and can never shift it up. H1 (expansion-temp census) is structurally impossible, not merely unfound
- [s5] The target layout requires an 8-byte object DECLARED BEFORE the work buffer that emits ZERO instructions; the only three such constructs in GCC 2.7.2 are unused array (forbidden), unwritten volatile scalar (forbidden), and a wider declared type for the buffer itself (s1 pad_lead, FAILed for lack of evidence)
- [s5] First-declared genuinely-used 8-byte object (Pair2 / s32 t[2] staged into local.vx/vy) is the closest honest form ever measured on this function: frame 112, local at 0x18, 71 insns, 14 differing insn pairs (vs 19 for the wp chassis) - divergence is exactly the staging traffic into sp+0x10/0x14, which target never writes
- [s5] Two fresh-seed campaigns on the new first-declared chassis: s5-pair-first best 16 but semantically invalid (wp repointed at q, wrong buffer passed); s5-arr-first-used 40409 iters best 108 flat. Random search now closed from four distinct seeds (s4: 2, s5: 2)
- [s5] Sibling func_8001E404 has the identical frame and identical untouched sp+0x10..0x17 region; func_8001A820's sp+0x10 is an unrelated 4-byte spill - no in-tree function writes an 8-byte lead before a camera work buffer

- [s5] The frame constraint is TWO-SIDED and only s5 measured the second side: vars=80 / frame 112 is necessary but NOT sufficient — the target also puts CamWork local at sp+0x18 with callee-saves at 0x60-0x6C. Four separate constructs reach vars=80 while leaving local at sp+0x10 and are 9-21 insn pairs off target regardless of frame size.

- [s5] GCC 2.7.2 hands out the vars area in allocation order, lowest sp offset first, and expands the outer block's declared locals before any statement — measured via arr_first_dead (declared FIRST -> objdump diff 0) vs arr_last_dead (declared LAST -> vars=80 but local unmoved, 9 pairs off) and scope_pair (inner-block aggregate allocated at sp+0x58).

- [s5] Therefore the target's layout requires an 8-byte object DECLARED BEFORE the work buffer that emits ZERO instructions. Across s1-s5 exactly three GCC 2.7.2 constructs satisfy that: unused local array (forbidden family), unwritten volatile scalar (forbidden family, s4), and a wider declared TYPE for the work buffer itself whose leading 8 bytes are simply never touched (the s1 pad_lead form — mechanically byte-perfect, cheat-reviewer FAILed only for want of evidence about what those bytes are).

- [s5] Closest honest form ever measured: first-declared genuinely-used 8-byte object (Pair2 / s32 t[2]) staged into local.vx/vy — frame 112, buffer at 0x18, 71 insns, 14 differing insn pairs vs the wp chassis's 19; rejected because it writes the phantom region target never touches.

- [s5] Random permuter search is now closed from FOUR distinct seeds (s4: honest base + wp chassis, 43k iters; s5: pair_first + arr_first_used, ~14k and 40409 iters). Both s5 campaigns harvested and STOPPED with telemetry in metrics/events.jsonl; no campaign outlives the session.

- [s5] Permuter finds must be semantically vetted, not just scored: s5's best find (score 16) repointed wp at q so the first callee would receive the wrong buffer — a scorer improvement that is not a program.

- [s5] Sibling/third-party evidence still negative: func_8001E404 has the identical frame (0x70, locals from 0x18, saves 0x60-0x68) and also never touches sp+0x10..0x17; func_8001A820's sp+0x10 is an unrelated 4-byte spill of a func_80053614 return; func_8003C9A4/func_8003CD10 have 0x18 frames (args only). No in-tree function writes an 8-byte lead before a camera work buffer.

- [s5] HEAD did NOT carry s4's wp edit (HEAD sandbox --disable all = 21). The banked candidate.c form was re-applied to src/code6cac.c this session and re-measured at 19; left in src as the session's final state.

- [s5] New reusable instrument: tmp/grind/func_8001E6E4/s5/screen.py splices any variant body into the s4 permuter TU, runs the Makefile-mirror pipeline and reports frame line + insn count + objdump diff-line count in one line per variant. NB it must pass REPO-RELATIVE paths to compile.sh and prepend .venv/bin to PATH — absolute paths containing spaces silently yield an empty .o.

## Session 6 (grinder, forensics, 2026-08-11)

- **Instrument used: the instrumented cc1 (`tools/gcc-2.7.2/cc1`, NOT
  `build/cc1`) with `BB2_FRAME_DEBUG=1`.** Its hook in `assign_stack_local`
  (tools/gcc-2.7.2/function.c:731-742) prints one
  `FRAMEDBG func=... ctx=... mode=... size=... frame_offset=...` line per
  frame-slot allocation, with a context tag naming the caller. Wrappers banked:
  `tmp/grind/func_8001E6E4/s6/fdbg.sh` (census for one function of a
  preprocessed TU) and `s6/run_probe.sh` (census + per-function insn count +
  `.frame` line for a whole probe TU). NB cc1 does NOT run cpp - a probe TU
  must be preprocessed first or every comment is a parse error.
- **Census of the honest (wp-chassis) form**: exactly ONE real allocation,
  `ctx=stack_temp mode=BLKmode size=72 frame_offset=0->72`, then
  `ctx=round_frame size=0`. So `CamWork local` is allocated through
  `assign_stack_temp` (expand_decl's path for a BLKmode local), it is the
  FIRST slot handed out, and nothing whatsoever precedes it. `.frame` =
  vars 72 / regs 4-0 / args 16 / extra 0 -> 104.
- **THE FRAME EQUATION IS NOW FULLY PARTITIONED** (mips.c compute_frame_size,
  line 4444ff): `total = args_size + var_size + gp_reg_rounded + fp_reg_size +
  extra_size`, where `extra_size = MIPS_STACK_ALIGN(TARGET_ABICALLS ?
  UNITS_PER_WORD : 0)` = **0 for this build** (no -mabicalls), `fp_reg_size` =
  0, `gp_reg_rounded` = 16 (s0/s1/s2/ra, matching target's saves at
  0x60-0x6C), and the local base is `STARTING_FRAME_OFFSET` =
  `current_function_outgoing_args_size` (mips.h:1651). Target's frame 112 with
  the buffer at sp+0x18 therefore admits EXACTLY TWO partitions:
  (A) args=24, vars=72 - the buffer sits at the args boundary and there is no
  phantom slot at all; (B) args=16, vars=80 - 8 bytes of vars allocated before
  the buffer.
- **Partition (A) is now KILLED BY MEASUREMENT, not by elimination** (s2 had
  only an argument). Probe `p_call5` (a 5-word outgoing call): `.frame` =
  vars 72 / args 24 / frame 104, and the local really does move to sp+0x18 -
  the ONLY mechanism ever measured on this function that moves the work buffer
  up without allocating anything below it - but the 5th argument word is
  materialized as `sw $5,16($sp)` (it even lands in the jal delay slot). Every
  route to args_size>16 (a 5th scalar arg, an 8-byte-aligned arg forced past
  $a3, a struct-returning callee's hidden pointer shifting the list - probe
  `p_structret`: vars 80 / args 24) writes at least one word into
  sp+0x10..0x17. The target touches nothing in that region, so args=24 is
  dead, definitively.
- **Partition (B): the pre-declaration allocation window on MIPS o32 is
  PROVABLY EMPTY.** Source census of every `assign_stack_local` /
  `assign_stack_temp` call site reachable before the first `expand_decl`:
  * `assign_parms` (function.c:3605 `assign_parms_blk`, :3888
    `assign_parms_reg`) can only fire when `stack_parm == 0`, and
    function.c:3489-3500 keeps `stack_parm` non-null whenever
    `REG_PARM_STACK_SPACE (fndecl) > 0` - which on this target is ALWAYS
    (mips.h:1822, `MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD - FIRST_PARM_OFFSET`
    = 16). Measured: probes `p_parm_int_addr`, `p_parm_struct8_unused`,
    `p_parm_struct8_used`, `p_parm_double_unused` ALL report vars=72 with a
    single size=72 slot - no parameter shape (extra params, by-value
    aggregates, doubles, addressable ints) allocates ANY vars byte.
  * A promoted sub-word parameter whose address is taken DOES get a vars slot,
    but through `put_reg_into_stack` DURING expansion, i.e. AFTER the decls:
    measured `p_parm_short_addr` / `p_parm_2short_addr` /
    `p_parm_short_addr_dead` -> `ctx=put_reg_into_stack mode=HImode size=2` at
    frame_offset 72->74 (and 74->76); vars rounds to 80 but the 72-byte buffer
    STAYS at the base. Same "grows at the top" law s5 measured for expansion
    temps, now confirmed at the compiler-entry-point level.
  * The only allocator in `expand_function_start` that runs before the body is
    the static-chain slot (function.c:5036-5045, comment verbatim: "If function
    gets a static chain arg, store it in the stack frame. Do this first, so it
    gets the first stack slot offset."). It needs `current_function_needs_context`
    (a GNU nested function referencing this frame), is Pmode = 4 bytes not 8,
    and is followed unconditionally by `emit_move_insn (last_ptr,
    static_chain_incoming_rtx)` - a store. Dead on all three counts.
  * stmt.c:669 (nonlocal-goto handler slot) likewise needs nested functions and
    emits `emit_stack_save`; expr.c:8208/8275 are `__builtin_apply`;
    explow.c:879 is the alloca/VLA save (and per mips.c's own frame diagram
    alloca space DOES sit below the locals - but it is a runtime `sp`
    adjustment, incompatible with the target's fixed-offset addressing).
  Conclusion: the 8 bytes can only come from a DECLARATION preceding the work
  buffer. s5's black-box law is now a white-box proof, and the whole "maybe the
  original prototype/signature was different" family - extra parameters,
  aggregate parameters, doubles, varargs, struct-returning calls, extra call
  arguments - is measured dead in one pass.
- **s5's frontier probe (a) RUN AND NEGATIVE: binary-wide census by IDIOM.**
  `tmp/grind/func_8001E6E4/s6/lead_census.py` scans all 1434 `asm/funcs/*.s`,
  collecting each function's sp-relative store offsets, `addiu $r,$sp,K`
  address-taken record bases, and jal targets (full output
  `s6/lead_census.txt`). Results: 52 functions take the address of a stack
  record with >=5 consecutive stored words at that base; **14** exhibit the
  exact pad_lead shape (stores at K and K+4 = a WRITTEN 8-byte lead, plus a
  triple stored at K+8/K+0xc/K+0x10 whose address K+8 is handed to a callee);
  and **NONE of those 14 hands the pointer to func_8001A538 or func_80046BF4**
  (the camera consumers). The nearest analogues, func_80061C00 / func_80061D74,
  feed `RotMatrix` / `RotTrans` / `SetRotMatrix` / `SetTransMatrix` - i.e. the
  ordinary PsyQ `SVECTOR` (8 bytes) + record adjacent-locals idiom, two
  INDEPENDENT locals, not one wider record with named leading fields. The
  pad_lead rejection's reopen condition therefore fails on the by-idiom search
  exactly as it failed on s5's by-callee search: a FAILED gate, not an open
  question.
- **Canonical-asm gate measured for the first time on this function:**
  `python3 tools/scan_hand_coded.py --single func_8001E6E4` ->
  `HAND_CODED: tier=LOW score=0/8`, with S1/S2/S6 (the three signals the
  endgame-lock gate requires) all absent.
- src/code6cac.c: HEAD again did NOT carry the wp edit; the banked candidate.c
  form was re-applied and re-measured this session at **sandbox --disable all =
  19** (71/71 insns, 139 cheat-asm constructs stripped file-wide), and left in
  src as the session's final state.

- [s6] The instrumented cc1's BB2_FRAME_DEBUG hook lives in assign_stack_local (function.c:731) and tags each slot with a context (stack_temp / put_reg_into_stack / assign_parms_reg / assign_parms_blk / round_frame); wrappers banked at tmp/grind/func_8001E6E4/s6/fdbg.sh and run_probe.sh. cc1 does not run cpp - preprocess probe TUs first.
- [s6] Honest-form census: exactly ONE allocation, ctx=stack_temp BLKmode size=72 at frame_offset 0 (CamWork local goes through assign_stack_temp, expand_decl's BLKmode path) - nothing precedes it.
- [s6] Frame equation partitioned from mips.c compute_frame_size: extra_size is 0 without -mabicalls, fp=0, gp_rounded=16, and the local base is STARTING_FRAME_OFFSET = current_function_outgoing_args_size (mips.h:1651). Target 112 with the buffer at 0x18 admits only (args=24,vars=72) or (args=16,vars=80).
- [s6] args=24 partition KILLED BY MEASUREMENT (previously only an elimination argument): probe p_call5 reaches vars=72/args=24 and DOES place the local at sp+0x18, but the 5th argument word is materialized as sw $5,16($sp); every route to args>16 (5th scalar, 8-byte-aligned arg past $a3, struct-return hidden pointer - probe p_structret vars=80/args=24) writes into sp+0x10..0x17, which the target never touches.
- [s6] The pre-declaration allocation window is provably EMPTY on MIPS o32: assign_parms can never call assign_stack_local because REG_PARM_STACK_SPACE(fndecl)=16>0 keeps stack_parm non-null (function.c:3489-3500, mips.h:1822) - measured over int/aggregate/double/addressable parameter shapes, all vars=72.
- [s6] An addressable sub-word parameter gets its slot from put_reg_into_stack DURING expansion (measured HImode size=2 at frame_offset 72->74), i.e. ABOVE the first local - the same grows-at-the-top law s5 measured for expansion temps, now confirmed at the entry-point level.
- [s6] The only pre-body allocator in expand_function_start is the static-chain slot (function.c:5036, "Do this first, so it gets the first stack slot offset") - GNU nested functions only, Pmode=4 bytes, and followed unconditionally by a store. Dead on all three counts.
- [s6] Binary-wide idiom census (1434 funcs, s6/lead_census.py + .txt): 52 address-taken stack records with >=5 consecutive stored words; 14 with the exact pad_lead shape (written 8-byte lead + pointer handed at base+8); ZERO of them hand that pointer to func_8001A538/func_80046BF4. The closest analogues (func_80061C00/func_80061D74) are the ordinary PsyQ SVECTOR+record adjacent-locals idiom - two independent locals, not a wider type. The pad_lead reopen condition FAILS on the by-idiom search too.
- [s6] scan_hand_coded --single func_8001E6E4 = tier LOW, score 0/8, S1/S2/S6 all absent - endgame-lock gate (1) (canonical-asm) is a documented FAIL for this function.
- [s6] Floor re-measured this session with the banked candidate applied: sandbox --disable all = 19, 71/71 insns.

- [s6] The instrumented cc1 is tools/gcc-2.7.2/cc1 (NOT build/cc1); its BB2_FRAME_DEBUG hook sits in assign_stack_local (function.c:731-742) and tags every slot with a context: stack_temp / put_reg_into_stack / assign_parms_reg / assign_parms_blk / round_frame. Wrappers banked at tmp/grind/func_8001E6E4/s6/fdbg.sh and s6/run_probe.sh. cc1 does not run cpp - probe TUs must be preprocessed or every comment is a parse error.

- [s6] Census of the honest wp-chassis form: exactly ONE real allocation, ctx=stack_temp mode=BLKmode size=72 frame_offset 0->72 (CamWork local goes through assign_stack_temp, expand_decl's BLKmode path), then ctx=round_frame size=0. Nothing precedes it. .frame = vars 72 / regs 4-0 / args 16 / extra 0 -> 104.

- [s6] Frame equation partitioned from mips.c compute_frame_size (line 4444ff): total = args_size + var_size + gp_reg_rounded + fp_reg_size + extra_size, with extra_size = MIPS_STACK_ALIGN(TARGET_ABICALLS ? UNITS_PER_WORD : 0) = 0 for this build, fp_reg_size = 0, gp_reg_rounded = 16 (s0/s1/s2/ra = target's saves at 0x60-0x6C), and local base = STARTING_FRAME_OFFSET = current_function_outgoing_args_size (mips.h:1651). Target frame 112 with the buffer at sp+0x18 admits exactly two partitions: (args=24, vars=72) or (args=16, vars=80).

- [s6] args=24 is dead by measurement: p_call5 reaches vars=72/args=24 and does move the local to sp+0x18, but emits sw $5,16($sp); p_structret (struct-returning callee) reaches vars=80/args=24 the same way. Any args_size>16 writes into sp+0x10..0x17, a region the target never touches.

- [s6] The pre-declaration allocation window is provably empty on MIPS o32: assign_parms can never call assign_stack_local because REG_PARM_STACK_SPACE(fndecl)=16>0 keeps stack_parm non-null (function.c:3489-3500 + mips.h:1822). Measured across int / by-value-aggregate / double / addressable-int parameter shapes: all vars=72.

- [s6] An addressable sub-word (promoted-mode) parameter does get a vars slot, but from put_reg_into_stack during expansion - measured HImode size=2 at frame_offset 72->74 - i.e. ABOVE the first local, which never moves. Same grows-at-the-top law s5 measured for expansion temps, now confirmed at the compiler entry point.

- [s6] The only allocator in expand_function_start that precedes the body is the static-chain slot (function.c:5036-5045, comment: 'Do this first, so it gets the first stack slot offset'): GNU nested functions only, Pmode = 4 bytes not 8, and followed unconditionally by emit_move_insn - dead on all three counts. stmt.c:669 (nonlocal goto) needs nested functions too; expr.c:8208/8275 are __builtin_apply; explow.c:879 is the alloca/VLA save, which is a runtime sp adjustment incompatible with the target's fixed-offset addressing.

- [s6] Binary-wide idiom census (1434 asm/funcs scanned, s6/lead_census.py, output s6/lead_census.txt): 52 address-taken stack records with >=5 consecutive stored words; 14 with the exact pad_lead shape (written 8-byte lead plus pointer handed at base+8); zero of them hand that pointer to func_8001A538 or func_80046BF4. Closest analogues func_80061C00/func_80061D74 are the ordinary PsyQ SVECTOR+record adjacent-locals idiom.

- [s6] tools/scan_hand_coded.py --single func_8001E6E4 = tier LOW, score 0/8, with S1 (multu pacing), S2 (empty branch) and S6 (BIOS jumptable) all absent - endgame-lock gate (1), canonical-asm authorization, is a documented FAIL for this function whenever the driver declares exhaustion.

- [s6] HEAD again did not carry the wp edit; the banked candidate.c form was re-applied to src/code6cac.c and re-measured this session at sandbox --disable all = 19, build_insns == target_insns == 71, 139 cheat-asm constructs stripped file-wide. Left in src as the session's final state.
