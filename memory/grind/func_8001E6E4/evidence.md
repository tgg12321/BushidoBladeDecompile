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
