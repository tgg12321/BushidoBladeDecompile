# Hypothesis ledger — AddTbpOfst_80047EE8

## s1 (2026-07-21, recon) — floor 15 -> 10

- H1 CONFIRMED — single-walker merge: the old candidate split the walker into
  `cached` (s32) + `p` (u32*); target carries ONE register ($s0) end-to-end.
  Merging into one pointer variable: 16 -> 14. (Baseline re-measure of the
  wip-imported form was 16, not 15 — minor drift.)
- H2 CONFIRMED — live `first = saved + (new_var2 << 2)` precompute inside the
  loop (sibling InitHiraRmd_80047FBC committed lever 1) flips whole-function
  RA so the prologue stages arg0 through $s0 (sw s0; move s0,a0; sw s2;
  move s2,s0; addu s0,s0,a1 — exact target shape): 14 -> 11, insns 52 -> 53/53.
- H3 KILLED — FAKE arg0=0 WITHOUT the H2 staging is INERT at both prologue
  positions (score 14, prologue byte-identical). See
  rejected/fake-arg0-zero-without-staging.c.
- H4 CONFIRMED — FAKE arg0=0 ON TOP of H1+H2 flips the second-pointer binding
  (addu s0,a0,v0 -> addu s0,s2,v0): 11 -> 10. Same cse2 canonical-register
  mechanism + same lever as sibling s6 (Judge-PASSed there). Exhaustion
  prerequisite for shipping it NOT yet discharged on this function.
- Residual at 10 = EXACTLY the 32-byte unused frame (2x addiu sp + 8 save/
  restore offsets; every other insn identical — tmp/grind/AddTbpOfst_80047EE8/
  s1/diff_p6.out). Identical species to sibling's pending owner-escalation.
- Probe note: `s32 buf[8]` unused array is STRIPPED by the sandbox
  (cheat_asm_stripped 381 -> 382, score unchanged) — the frame gap cannot even
  be measured via the cheat form, let alone closed by it.

## [s1] Merging the split cached/p variables into one walking pointer matches target's single-register ($s0) carry chain
- mechanism: target allocates arg0-copy -> add -> reload -> loop walker to ONE pseudo; two source variables split the live range and let the short first range land in $a1
- probe: rewrote prologue as p=(u32*)arg0; saved=(s32)p; p=p+off; sandbox --disable all
- result: 16 -> 14
- verdict: CONFIRMED

## [s1] A live precompute of the call's first arg (first = saved + (new_var2<<2)) inside the loop flips whole-function RA to stage arg0 through $s0 in the prologue
- mechanism: sibling InitHiraRmd_80047FBC committed lever 1: the extra loop-body pseudo referencing saved shifts allocation so GCC emits sw s0; move s0,a0; sw s2; move s2,s0; addu s0,s0,a1 instead of copy-propping to move s2,a0
- probe: added s32 first local + used as call arg; sandbox
- result: 14 -> 11, build insns 52 -> 53/53, prologue shape == target
- verdict: CONFIRMED

## [s1] FAKE arg0=0 dead store alone (without the staging lever) breaks the cse2 class and materializes the $s0 staging
- mechanism: dead-store-fake-exception family; expected to break {arg0,p,saved} canonical-register equivalence
- probe: arg0=0 /* FAKE */ at both prologue positions on the single-walker form, no first-precompute; sandbox + prologue disasm
- result: INERT: score 14 both positions, prologue byte-identical; flow deletes the dead set before it matters when no staged allocation exists
- verdict: KILLED

## [s1] FAKE arg0=0 on top of the staging levers closes the second-pointer binding diff (addu s0,a0,v0 -> addu s0,s2,v0)
- mechanism: cse2 canonical-register substitution folds base==arg0 and picks $a0; the annotated dead store kills the $a0 association (sibling s6 mechanism, Judge-PASSed there)
- probe: arg0=0 /* FAKE */ after saved=(s32)p on the H1+H2 form; sandbox + full-stream diff
- result: 11 -> 10; instruction stream now identical to target EXCEPT the 10 frame-offset insns
- verdict: CONFIRMED

## [s2] A pure spelling of the saved/p init chain (const, decl-order, split-init, retype, rebind) can flip the second-pointer binding to addu s0,s2,v0 without the FAKE arg0=0 store
- mechanism: cse2 canonical-register substitution puts {arg0, p, saved} in one equivalence class and picks $a0 for the add; the question was whether any C-level spelling changes the class or the canonical pick
- probe: On the floor-10 chassis, swept: V2 const decl-init saved, V3 decl-order swap, V4 split-init reversal (saved=arg0; p=(u32*)saved), V5 u32-typed saved, V6 two-statement rebind from saved; each measured with sandbox --disable all; control V1 = FAKE removed
- result: V1 control 11; V2=V3=V4=V5=V6 all 11 (53/53 insns) — byte-identical to the no-FAKE control; the FAKE form remains uniquely 10. Matches sibling s8 (const discarded at RTL) and s9 (pseudo order is first-USE LUID) exactly
- verdict: KILLED

## [s2] Re-associating the second-pointer offset as a mask (saved + (v_off & ~3)) instead of the srl/sll shift pair reaches the target binding
- mechanism: different RTL for the offset computation could break the cse2 substitution site or change combine's shape
- probe: V7: p = (u32*)(saved + (s32)(v_off & ~3)) on the floor-10 chassis; sandbox --disable all
- result: 14, build insns 52 vs target 53 — WORSE and byte-diverging: target carries the two-shift pair, the mask spelling emits andi and cascades
- verdict: KILLED

## [s3] A pure-C LIVE-locals shape reserves the target's phantom 32-byte frame (vars=32) without emitting stores or changing the 53-insn stream
- mechanism: phantom-frame-slots-gcc272 — GCC 2.7.2 reload reserves spill slots (counted by get_frame_size) for pseudos it keeps in registers; witnessed at vars=8 in tslLineG5Init from an s16 pair feeding `(a&~b)&1`. Frontier F2 claimed this body's s16->SImode call-arg widening + u32 word local would trigger a comparable (32-byte) reservation.
- probe: cc1 .frame instrument (framedump.sh) + 9-variant grid (sweep.py, frame_grid.md): base, fn-scope hoist, u64 word, struct record, staged temps, folded himode-pair, written array, u64 pair, and a genuinely-live tslLineG5Init-clone guard (v08).
- result: EVERY stream-preserving variant = vars=0 (frame 40, distance 10). v08 (exact tslLineG5Init trigger, live guard on a real global store) = vars=0 — the mechanism does NOT fire here. Only a WRITTEN s32 rec[6] reaches vars=24, at +6 diverging stores the target lacks (forbidden dead-array, no carve-out).
- verdict: KILLED. The 32-byte phantom reservation is not reproducible by any pure-C live-locals shape in this body; it requires the forbidden unwritten/written dead array. Structural axis exhausted. Same endgame-lock species as sibling InitHiraRmd_80047FBC.

## [s3] A pure-C LIVE-locals shape reserves the target's phantom 32-byte frame (vars=32) without emitting stores or diverging from the 53-insn stream.
- mechanism: phantom-frame-slots-gcc272: GCC 2.7.2 reload reserves spill slots (counted by get_frame_size) for pseudos it keeps in registers; witnessed vars=8 in tslLineG5Init from an s16 pair feeding (a&~b)&1. Frontier F2 claimed this body's s16->SImode call-arg widening + u32 word local would trigger a ~32-byte reservation.
- probe: Built the cc1 .frame instrument (framedump.sh); ran a 9-variant grid (sweep.py) reading vars= and sp-store count: base, fn-scope hoist, u64 word, struct record, staged temps, folded himode-pair, written array, u64 pair, and v08 = the exact tslLineG5Init trigger (live s16 pair (hv&~hm)&1 guarding a real global store).
- result: Every stream-preserving variant = vars=0 (frame 40, distance 10). v08 (exact tslLineG5Init trigger) = vars=0 — the mechanism does NOT fire in this body. Only a written s32 rec[6] reaches vars=24, at +6 diverging sp-stores the target lacks (forbidden dead-array; no carve-out, matching sibling s7: target has ZERO sw in the vars region).
- verdict: KILLED

## [s3] The candidate floor-10 form re-measures distance 10 this session.
- mechanism: single-walker + first-arg precompute + FAKE arg0=0 staging (s1/s2 levers); residual is purely the frame-offset insns.
- probe: Applied candidate.c to src/text1b.c; `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all`.
- result: score 10, target_insns 53, build_insns 53, scorable true. Confirmed floor=10 this session.
- verdict: CONFIRMED

## [s4] A whole-function permuter search over statement orderings finds a pressure profile that reserves the target's 32 phantom frame bytes with no divergent stores (frontier F2 permuter avenue)
- mechanism: reload spill-slot reservation is emergent from register pressure; a random search over C forms might surface a pressure profile reserving 8 phantom words that structural sweeps missed
- probe: built a full-TU permuter workspace on the floor-10 clean chassis (--stack-diffs; base_score 266), ran a fresh-seed campaign 9451 iterations; harvested + stopped
- result: 3 novel finds, best score 202, NEVER 0. Descent came ONLY from inserting a volatile dead-frame local (`volatile long`/`unsigned long long pad`) — a forbidden volatile-coercion/dead-vars cheat reserving ~8 bytes; plateau at 202, next novel WORSE (207). No legitimate pure-C frame-growth form exists in the searched space.
- verdict: KILLED. The permuter modality independently corroborates s3: the 32-byte phantom frame is reachable only via forbidden dead/volatile-local cheats. All sanctioned axes (F1 arg0 lever-exhaustion s2, F2 structural s3 + permuter s4) now measured dead; endgame-lock exhaustion state, same species as sibling InitHiraRmd_80047FBC.

## [s4] A whole-function permuter search over statement orderings finds a register-pressure profile that reserves the target's 32 phantom frame bytes with no divergent stores (frontier F2 permuter avenue).
- mechanism: Reload spill-slot reservation is emergent from register pressure; a random search over C forms might surface a pressure profile reserving 8 phantom words that the structural sweeps (s3) could not construct by hand.
- probe: Built a full-TU permuter workspace on the floor-10 clean chassis (tmp/grind/AddTbpOfst_80047EE8/s4/ws; text1b cc1|prologue_fix|maspsx|fix_lwl|multu_pad pipeline; --stack-diffs so frame offsets score; base_score 266). Ran a fresh-seed campaign 9451 iterations via permuter_campaign.py, waited in-turn, harvested + stopped.
- result: 3 novel finds, best score 202, NEVER 0. Descent 266->242->202 came from ONE lever only: a volatile dead-frame local (`volatile long pad;` widened to `volatile unsigned long long pad;`) reserving ~8 phantom bytes. That is a forbidden volatile-coercion/dead-vars frame cheat. It plateaus at 202 (8 bytes reserved, not the needed 32); the next novel emitted was WORSE (207 at 444s). No legitimate pure-C frame-growth form appeared in the searched space.
- verdict: KILLED

## [s5] A whole-function permuter search over a STRUCTURALLY DIFFERENT chassis (the no-FAKE floor-11 form, a distinct RA basin) finds a pressure profile that reserves the target's 32 phantom frame bytes with no divergent stores
- mechanism: fresh-seed discipline (permuter-directives §Campaign discipline) — s4 exhausted the floor-10 FAKE-chassis basin; a structurally-different chassis with scrambled register allocation (base_score 5200 vs s4's 266) presents a different search landscape that might surface a frame-reserving form the s4 basin could not reach
- probe: rebuilt the full-TU permuter workspace on the no-FAKE floor-11 chassis (tmp/grind/AddTbpOfst_80047EE8/s5/ws; --stack-diffs; base_score 5200); ran a fresh-seed campaign 17057 iters via permuter_campaign.py, drove a real wall-clock window (drive_window.py), harvested + stopped
- result: descended 5200 -> 207 and PLATEAUED FLAT at 207 from iter ~12753 to 17057, NEVER 0. The sole frame-growth lever was again a forbidden volatile dead-frame local (`volatile unsigned int pad;`, ~8 bytes) — the same cheat family as s4. No legitimate pure-C frame-growth form in this distinct basin either.
- verdict: KILLED. The permuter modality is now measured DEAD across BOTH chassis basins (floor-10 FAKE s4 + floor-11 no-FAKE s5, ~26500 combined iters). Independently triple-corroborates s3's structural DEAD. All sanctioned axes (F1 arg0 lever-exhaustion s2, F2 structural s3 + permuter-two-basin s4/s5) now measured dead; endgame-lock exhaustion state, same species as sibling InitHiraRmd_80047FBC. Remaining action = mirroring owner-escalation once the sibling family ruling lands.

## [s5] A whole-function permuter search over a STRUCTURALLY DIFFERENT chassis (the no-FAKE floor-11 form, a distinct RA basin from s4's floor-10 FAKE chassis) finds a pressure profile that reserves the target's 32 phantom frame bytes with no divergent stores.
- mechanism: Fresh-seed discipline: s4 exhausted the floor-10 FAKE-chassis basin; a structurally-different chassis with scrambled register allocation (permuter base_score 5200 vs s4's 266) presents a different search landscape that might surface a frame-reserving form the s4 basin could not reach.
- probe: Rebuilt the full-TU permuter workspace on the no-FAKE floor-11 chassis (tmp/grind/AddTbpOfst_80047EE8/s5/ws; text1b cc1|prologue_fix|maspsx|fix_lwl|multu_pad pipeline; --stack-diffs so frame offsets score; base_score 5200). Ran a fresh-seed campaign 17057 iters via permuter_campaign.py; drove a real wall-clock window (drive_window.py) tracking best score; harvested + stopped.
- result: Descended 5200 -> 207 and PLATEAUED FLAT at 207 from iter ~12753 through 17057 (thousands of iters, zero novel improvement), never 0. The sole frame-growth lever was again a forbidden volatile dead-frame local (`volatile unsigned int pad;`, ~8 bytes) — the same cheat family s4 found. No legitimate pure-C frame-growth form in this distinct basin.
- verdict: KILLED

## [s6] The target's 32-byte phantom vars region is a reload/register-pressure spill-slot reservation (the phantom-frame-slots-gcc272 / tslLineG5Init theory s3-s5 operated under)
- mechanism: s3 framed the residual as a GCC 2.7.2 reload spill-slot reservation (get_frame_size counting slots for pseudos kept in registers); s4/s5 searched register-pressure profiles to trigger it. Forensics tests whether the origin is really reload (route-b) or function.c expand (route-a).
- probe: cc1 -da full-pass RTL dump of the floor-10 candidate (.rtl post-expand + .greg post-reload); decode target frame from asm/funcs; positive control compiling `int buf[8]` locals (addressed ctlA + unused ctlB). tmp/grind/AddTbpOfst_80047EE8/s6/{dump.sh,control.sh,dumps/,control/}.
- result: FALSIFIED. (1) Candidate get_frame_size()=0 at BOTH expand and post-reload; .greg shows all 22 pseudos in hard regs, zero spills. (2) Target's vars region 0x18-0x37 has ZERO sw/lw — a reload spill slot always emits store+reload, so it is NOT a reload artifact. (3) Positive control: a declared local array `int buf[8]` (ctlA addressed OR ctlB unused) reserves vars=32 with zero stores, byte-shape-identical to target — GCC function.c allocates the slot at RTL-expand from the DECL and never reclaims it post-DCE.
- verdict: KILLED (theory falsified). The 32-byte divergence is produced by function.c stack-frame layout (assign_stack_local at RTL-EXPAND from a source-level local-aggregate DECL), NOT global.c RA / reload / sched. It is UPSTREAM of register allocation, so the entire s3/s4/s5 RA+scheduling search space provably cannot reach it. The only pure-C reproduction is declaring a >=32-byte dead local aggregate = the forbidden dead-vars-local-array (WRITTEN carve-out inapplicable: target has zero region stores). Endgame-lock confirmed with a precisely named mechanism; same species as sibling InitHiraRmd_80047FBC.

## [s6] The target's 32-byte phantom vars region (0x18-0x37) is a reload/register-pressure spill-slot reservation (the phantom-frame-slots-gcc272 / tslLineG5Init theory s3/s4/s5 operated under).
- mechanism: s3 framed the residual as GCC 2.7.2 reload reserving spill slots (get_frame_size) for pseudos kept in registers; s4/s5 searched register-pressure profiles to trigger it. Forensics tests reload-origin (route-b) vs function.c expand-origin (route-a).
- probe: cc1 -da full-pass RTL dump of the floor-10 candidate (.rtl post-expand + .greg post-reload); decoded target frame from asm/funcs/AddTbpOfst_80047EE8.s; positive control compiling declared `int buf[8]` locals (addressed ctlA + unused ctlB).
- result: FALSIFIED. (1) Candidate get_frame_size()=0 at BOTH expand and post-reload; .greg shows all 22 pseudos (74-99) in hard regs (2 4 5 6 7 16 17 18 29 31), zero spills, no frame pointer. (2) Target vars region 0x18-0x37 has ZERO sw/lw; a reload spill slot ALWAYS emits store+reload, so it is not a reload artifact. (3) Positive control: declared `int buf[8]` (addressed ctlA=.frame $sp,56 vars=32 OR unused ctlB=vars=32) reserves the region with ZERO stores, byte-shape-identical to target.
- verdict: KILLED

## [s7] The target's 32-byte phantom vars region is produced by a per-function register-allocation accident, so it varies with each function's pressure (a residual doubt after s6 named the function.c source-DECL mechanism on AddTbpOfst alone)
- mechanism: if the phantom were an RA/reload emergent artifact it would scale with each cluster member's register pressure (different saved-reg counts); if it is a shared source-level dead-aggregate DECL it would be INVARIANT across the cluster regardless of pressure
- probe: decoded the frame layout (frame size, every sp-relative sw/lw) of all 4 cluster members directly from asm/funcs/; computed each one's args/vars/reg-save regions
- result: KILLED. All 4 (AddTbpOfst_80047EE8 72B/4regs, InitHiraRmd_80047FBC 80B/6regs, InitHiraRmd_800480C0 88B/8regs, func_800481E8 72B/4regs) reserve EXACTLY 32 phantom bytes at 0x18-0x37 with ZERO region accesses, despite 4/6/8/4 saved regs. The phantom is invariant under pressure -> a SHARED source-level dead ~32-byte local aggregate, corroborating (not a per-function RA accident). Directly supports the family/species escalation framing.
- verdict: KILLED (RA-accident theory falsified; source-DECL mechanism corroborated cluster-wide)

## [s7] The target's source aggregate can be pinned in size and type by mapping cc1's aggregate-size -> reserved-vars function, and a non-array (struct) construct might reproduce the zero-store 32-byte phantom legitimately
- mechanism: function.c assign_stack_local rounds the DECL size to 8-byte frame alignment; if a struct (a legitimate SDK type like MATRIX) reproduced the shape, a genuinely-used struct could be a non-cheat path
- probe: cc1 -O2 -G0 -mips1 on a size grid (int[6/7/8/9], addressed + dead) and a 32-byte MATRIX-shaped struct (addressed mtx_used, dead mtx_dead); read .frame vars= and sp-store presence (tmp/grind/AddTbpOfst_80047EE8/s7/probe/agg.{c,s})
- result: size map int[6]=24->vars24, int[7]=28->vars32, int[8]=32->vars32, int[9]=36->vars40; target vars=32 pins the source aggregate at 25-32 bytes (7-8 word int array); a8_dead reproduces target shape exactly (vars=32, ZERO sp stores). BUT the 32-byte STRUCT reserved vars=0 in BOTH addressed and dead forms (GCC scalarized/eliminated it) — only the ARRAY form reserves the zero-store slot. So the sole reproducing construct is a dead local ARRAY = the forbidden dead-vars-local-array; no legit struct substitute exists; WRITTEN carve-out inapplicable (zero region stores).
- verdict: KILLED (no legit non-array construct; forensic size/type pin re-confirms s6 endgame-lock, upstream of all RA/scheduling)

## [s7] The 32-byte phantom vars region is a per-function register-allocation accident, so it would vary with each cluster member's pressure.
- mechanism: RA/reload emergent artifacts scale with register pressure; a shared source-level dead-aggregate DECL would be invariant across the cluster.
- probe: Decoded frame layout (size + every sp-relative sw/lw) of all 4 cluster members directly from asm/funcs/; computed each one's args/vars/reg-save regions.
- result: All 4 (AddTbpOfst_80047EE8 72B/4regs, InitHiraRmd_80047FBC 80B/6regs, InitHiraRmd_800480C0 88B/8regs, func_800481E8 72B/4regs) reserve EXACTLY 32 phantom bytes at 0x18-0x37 with ZERO region accesses despite 4/6/8/4 saved regs. Phantom invariant under pressure => shared source-level dead ~32B local aggregate.
- verdict: KILLED

## [s7] A non-array (struct) local could reproduce the zero-store 32-byte phantom, opening a legit (SDK-type) non-cheat path.
- mechanism: function.c assign_stack_local rounds a DECL size to 8-byte frame alignment; if a struct reproduced the shape, a genuinely-used struct could be non-cheat.
- probe: cc1 -O2 -G0 -mips1 on int[6/7/8/9] (addressed+dead) and a 32-byte MATRIX-shaped struct (addressed+dead); read .frame vars= and sp-store presence. tmp/grind/AddTbpOfst_80047EE8/s7/probe/agg.{c,s}.
- result: Size map: int[6]=24->vars24, int[7]=28->vars32, int[8]=32->vars32, int[9]=36->vars40. Target vars=32 pins source aggregate at 25-32B (7-8 word int array); a8_dead reproduces target shape (vars=32, ZERO sp stores). The 32-byte STRUCT reserved vars=0 in BOTH forms (GCC scalarized/eliminated it) — only the ARRAY reserves the zero-store slot. Sole reproducing construct = dead local ARRAY = forbidden dead-vars-local-array; WRITTEN carve-out inapplicable.
- verdict: KILLED

## [s8] A structurally-different rederivation (fresh m2c / Kengo transplant / decomp.me corpus) yields a pure-C shape that reaches the target frame or a lower floor
- mechanism: rederive modality — an independent reconstruction of the target might surface a different C structure (different variable/pointer decomposition, a genuinely-used local aggregate, a community-matched idiom) that the s1-s7 in-basin search never tried
- probe: (1) fresh m2c decompile of asm/funcs/AddTbpOfst_80047EE8.s, transcribed to project idioms + sandboxed; (2) Kengo transplant via kengo_matches.csv; (3) decomp.me shingle search for closest community scratches
- result: KILLED. (1) m2c reconstructs arg0-live base + void*-walker with NO local aggregate (vars=0); transcribed form scores 35/52-insn — WORSE than floor 10, one insn short (arg0-live binds $a0 not $s2). An independent decompiler confirms the 32-byte phantom has no semantic correlate. (2) Kengo AddTbpOfst maps name-unique to am_rmd.c:49 but only the disc BIN exists (no source, PS2 toolchain) — not transplantable. (3) decomp.me closest scratch = 0.138 similarity (shingle noise), no structural sibling, no phantom-frame idiom.
- verdict: KILLED. All three rederive lanes exhausted; no structurally-different pure-C shape escapes the endgame-lock. Independently (via m2c) re-confirms the s6/s7 source-DECL mechanism: the 32-byte phantom is reproducible only by the forbidden dead-vars-local-array. Rederive modality dead; all modalities (recon/structural/permuter/forensics/rederive) now exhausted.

## [s8] A structurally-different rederivation (fresh m2c, Kengo transplant, or decomp.me corpus) yields a pure-C shape that reaches the target frame or a lower floor than 10.
- mechanism: rederive modality: an independent reconstruction of the target might surface a different C decomposition (different pointer/variable split, a genuinely-used local aggregate, or a community-matched idiom) that the s1-s7 in-basin search never tried, upstream of the RA/scheduling axes already killed.
- probe: (1) fresh m2c decompile of asm/funcs/AddTbpOfst_80047EE8.s, transcribed to project idioms and sandboxed --disable all; (2) Kengo transplant via kengo_matches.csv; (3) decomp.me shingle search for the closest community scratches to the target asm.
- result: (1) m2c reconstructs arg0-live base + single void*-walker with NO local aggregate (vars=0); the transcribed form measured sandbox=35, build_insns=52 (target 53) - WORSE than floor 10 and one insn short (arg0-live copy-props base==arg0, binding $a0 not target's $s2). (2) Kengo AddTbpOfst maps name-unique to src/amami/am_rmd.c:49 (49 insns) but only the disc BIN exists - no Kengo source tree, PS2 toolchain - not transplantable. (3) decomp.me closest scratch = 0.138 similarity (shingle noise from generic gcc2.7.2 table-walkers), no structural sibling, no phantom-frame idiom.
- verdict: KILLED

## [s9] A rederive lane s8 did NOT run — a direct transplant of the sibling InitHiraRmd_80047FBC committed chassis (u32 *base pointer-carry) — yields a structurally-different pure-C shape that reaches the target frame or a lower floor than 10
- mechanism: rederive modality; s8 ran m2c (arg0-live, scored 35) / Kengo (no source) / decomp.me (noise). The in-repo sibling's OWN committed chassis is a distinct, proven-reaching C shape (u32 *base pointer carry, arg0=0 FAKE, loop-body offset compute between a2v/a3v) never transcribed onto THIS body — it might reach the frame or a different residual.
- probe: transplanted the sibling's exact chassis (minus buf[8], minus arg2/arg3 sign-extension which AddTbpOfst lacks) onto AddTbpOfst_80047EE8; `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all`; full objdump diff build.norm vs target.norm
- result: score 10, 53/53 insns — CO-OPTIMAL with the floor-10 candidate, not lower. objdump diff: body byte-identical to target EXCEPT the 10 frame-offset insns (addiu sp -0x28 vs -0x48 + 8 shifted save/restore offsets) = the identical 32-byte phantom-frame residual. A THIRD structurally-distinct legit chassis converges on the SAME frame-only residual, confirming it is chassis-independent = the s6/s7 function.c source-DECL phantom (dead >=32-byte local array), upstream of all C-structure levers.
- verdict: KILLED. Rederive re-confirmed dead via a fresh lane s8 did not run; no structurally-different pure-C shape reaches the frame or beats floor 10. All modalities (recon/structural/permuter-two-basin/forensics/rederive-four-lane) exhausted with measured negatives; the 32-byte residual is invariant across every reaching C chassis. Owner-gated unavailable (no escalation entry names this function yet; sibling ruling still pending).

## [s9] A rederive lane s8 did NOT run — direct transplant of the sibling InitHiraRmd_80047FBC committed chassis (u32 *base pointer-carry) — yields a structurally-different pure-C shape that reaches the target frame or a floor below 10.
- mechanism: rederive modality; s8 covered m2c (arg0-live=35) / Kengo (no source) / decomp.me (noise) but never transcribed the in-repo sibling's OWN proven-reaching committed chassis (u32 *base carry, arg0=0 FAKE break, loop-body offset compute placed between the a2v/a3v reads) onto this body.
- probe: Transplanted the sibling chassis (minus buf[8] and minus the arg2/arg3 sign-extension AddTbpOfst lacks) onto AddTbpOfst_80047EE8; sandbox --disable all; full objdump diff of build.norm vs target.norm.
- result: score 10, 53/53 insns — CO-OPTIMAL with the floor-10 candidate, not lower. objdump diff shows the body byte-identical to target EXCEPT the 10 frame-offset insns (addiu sp -0x28 vs -0x48 + 8 save/restore offsets shifted by the 32-byte phantom vars region). A third structurally-distinct legit chassis converges on the SAME frame-only residual, confirming the residual is chassis-independent = the s6/s7 function.c source-DECL phantom (dead >=32-byte local array), upstream of every C-structure lever.
- verdict: KILLED

## [s10] SYNTHESIS — with the full modality ladder exhausted, both AND-gates of endgame-lock-disposition fail on AddTbpOfst_80047EE8 by DIRECT measurement, confirming the terminal species and the owner-escalation disposition
- mechanism: endgame-lock-disposition (standing 2026-07-20): a cheat-matched function a few insns short of a pure-C match reaches a sanctioned COMPLETED only via (a) canonical-asm authorization IF scan_hand_coded shows STRONG hand-coded signals, OR (b) a coercion family IF SOTN precedent exists; else INCOMPLETE-owner-accepted via owner-escalation. s1-s9 only transfer-argued gate (a) from the sibling; synthesis measures it directly here and merges every axis.
- probe: (a) tools/scan_hand_coded.py --single AddTbpOfst_80047EE8 + wteng canonical AddTbpOfst_80047EE8; (b) SOTN/community precedent survey for an unwritten >=32-byte local-array phantom-frame carrier (inherited s6 positive-control + s7 size/type pin + cluster-invariance, identical to the sibling escalation survey).
- result: (a) scan_hand_coded tier=LOW score=1/8 "no strong hand-coded indicators" — only weak S4 front-loads fires (ordinary table-walker load batch); STRONG S1/S2/S6 all absent; canonical verdict=C distance=8. Gate (a) FAILS (not hand-coded asm). (b) No SOTN precedent; the sole reproducing construct is the forbidden dead-vars-local-array (dead s32 buf[7-8]), WRITTEN carve-out inapplicable (zero region stores), no struct substitute (GCC scalarizes to vars=0). Gate (b) FAILS. Both AND-gates fail.
- verdict: CONFIRMED (endgame-lock species confirmed; disposition = mirroring OWNER-ESCALATION, drafted at escalation_draft.md; no un-run sanctioned pure-C axis and no un-run modality remain)

## [s10] With the full modality ladder exhausted, both AND-gates of endgame-lock-disposition fail on AddTbpOfst_80047EE8 by DIRECT measurement (s1-s9 only transfer-argued gate (a) from the sibling).
- mechanism: endgame-lock-disposition standing 2026-07-20: sanctioned COMPLETED requires (a) canonical-asm authorization IF scan_hand_coded shows STRONG hand-coded signals, OR (b) a coercion family IF SOTN precedent exists; else INCOMPLETE-owner-accepted via owner-escalation.
- probe: (a) tools/scan_hand_coded.py --single AddTbpOfst_80047EE8 + wteng canonical AddTbpOfst_80047EE8; (b) SOTN/community precedent survey for an unwritten >=32-byte local-array phantom-frame carrier (inherited s6/s7 forensic pins + cluster-invariance).
- result: (a) scan_hand_coded tier=LOW score=1/8 'no strong hand-coded indicators' — only weak S4 front-loads fires (ordinary table-walker load batch); STRONG S1/S2/S6 absent; canonical verdict=C distance=8 -> gate (a) FAILS. (b) No SOTN precedent; sole reproducing construct = forbidden dead-vars-local-array (dead s32 buf[7-8]), WRITTEN carve-out inapplicable (zero region stores), no struct substitute (GCC scalarizes to vars=0) -> gate (b) FAILS. Both fail.
- verdict: CONFIRMED

## [s11] The assigned structural axis contains an un-run lever that reaches the target frame or a floor below 10
- mechanism: structural levers (block-local var splits, declaration order, type narrowing, statement re-association) act on C-source structure / register allocation / scheduling; the question is whether any structural spelling not yet measured reserves the target's 32-byte phantom vars region without diverging stores
- probe: reviewed the exhausted structural search (s2 6-spelling init-chain sweep = all 11 except FAKE=10; s3 9-variant phantom-frame grid + v08 decisive tslLineG5Init control = all vars=0) against the s6/s7 forensic finding that the divergence is function.c assign_stack_local at RTL-expand from a source-level dead local ARRAY DECL — a decision UPSTREAM of global.c RA and reload; re-verified the clean floor by applying candidate.c and measuring sandbox --disable all
- result: NO un-run structural lever exists — the residual is upstream of every C-structure/RA/scheduling axis structural levers can touch, so any structural variant either leaves vars=0 (floor 10) or requires the forbidden dead local array. Clean candidate.c re-measured 10 (53/53) this session; the committed-cheat-stripped form measured 8 (51 insns, 2 short — a stripped artifact, not a reachable pure-C floor). Re-running the dead structural axes is forbidden by the prime directive.
- verdict: KILLED (structural axis confirmed exhausted; no new lever). Disposition: filed the mirroring OWNER-ESCALATION into docs/grind/decisions.md (2026-07-21, grind s11) per the s10 frontier + endgame-lock-disposition standing policy; result = owner-gated.

## [s11] The assigned structural axis contains an un-run lever (block-local var split / decl order / type narrowing / statement re-association) that reserves the target's 32-byte phantom vars region or reaches a floor below 10.
- mechanism: Structural levers act on C-source structure / register allocation / scheduling. The s6/s7 forensics NAMED the divergence as function.c assign_stack_local at RTL-expand from a source-level dead local ARRAY DECL, a decision UPSTREAM of global.c RA and reload — so no structural/RA/scheduling lever can reach it. s2 (6-spelling init-chain sweep) and s3 (9-variant phantom-frame grid + v08 decisive tslLineG5Init control) already exhausted the constructible structural space.
- probe: Reviewed the exhausted structural search against the s6/s7 upstream-mechanism proof; re-verified the clean floor by applying candidate.c to src/text1b.c and measuring sandbox --disable all, plus measured the HEAD committed-cheat form for contrast; reverted src via git checkout.
- result: No un-run structural lever exists; the residual is upstream of every axis structural levers touch. Clean candidate.c = score 10, 53/53 insns (floor re-confirmed). HEAD committed-cheat-stripped form = score 8, 51 insns (2 short of target 53 — a stripped-cheat artifact, not a reachable pure-C floor). Any structural variant leaves vars=0 (floor 10) or requires the forbidden dead local array.
- verdict: KILLED

## s12 (2026-08-20, escalation)

- **H-s12.1 CONFIRMED.** *The banked floor-10 chassis plus a `volatile u32 pre_pad[8]` first
  declaration - the 2026-08-18 sanctioned phantom-frame-slot spelling, never compiled on this
  function - closes the entire 10-instruction frame-offset residual and byte-matches target.*
  Mechanism: GCC 2.7.2 `function.c assign_stack_local` reserves the array's 32 bytes at RTL-expand
  from the source DECL and never reclaims `frame_offset` after DCE, reproducing target's
  allocated-but-untouched vars region 0x18-0x37 (`.frame $sp,72`). Probe: apply the composite to
  src/text1b.c:19, run the full driver build. Result: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa
  == oracle, MATCH; built prologue `addiu sp,sp,-72`. This also KILLS the sub-hypothesis that
  `volatile` might perturb the allocation away from exactly 32 bytes (unpark-brief Judge-risk flag
  2): it does not.

- **H-s12.2 CONFIRMED.** *The honest floor cannot read 0 from `sandbox` for this function until
  `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` carries a `func_80047EE8` row.* Probe:
  read engine/volatile_cheats.py:746-772 (`_is_sanctioned_pad` gates on exact function + name +
  element count + volatile) and run the sandbox on the composite. Result: 10, cheat_asm_stripped
  279. The disposition is therefore an INTEGRATION HANDOFF, not an endgame lock.

- **H-s12.3 KILLED (the prior disposition).** *func_80047EE8 is an endgame lock whose gate (b)
  fails for want of a SOTN-master precedent (2026-07-22 ruling, decisions.md:1298).* Killed by the
  owner's 2026-08-18 general-family ruling plus the s12 byte proof: the precedent exists
  (sotn-construct-index.md:101/:103) and the construct closes the function. The 2026-07-22 REFUSED
  / OWNER-ACCEPTED INCOMPLETE disposition on this function is SUPERSEDED and should not be quoted
  by future sessions as live.

- **H-s12r.1 CONFIRMED (re-verification, not inheritance).** *The s12 byte proof reproduces on a
  clean tree; the discarded filing was a paperwork defect, not a measurement defect.* Probe: fresh
  apply of memory/grind/func_80047EE8/candidate.c over src/text1b.c:19 + full driver build + fresh
  sandbox on the same tree. Result: build SHA1 == oracle (MATCH), sandbox 10 with 53/53 insn
  counts, 0 regfix/asmfix rows. Every self-vet citation re-resolved in the live tree. Verdict: the
  function IS byte-matched by candidate.c; the only thing standing between it and COMPLETED-C is
  the owner-class row `"func_80047EE8": frozenset({("pre_pad", 8)})` in
  engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS.

- **Process note for the next session:** do NOT re-open the pure-C search. Every sanctioned pure-C
  axis is measured dead (s3-s11) and the closing construct is already compiled and proven. If the
  owner declines the allowlist row, the disposition reverts to the 2026-07-22 standing-ruling
  refusal; if the row lands, the remaining work is the layer-2 cheat-reviewer + `queue done`. An
  escalation entry for this function MUST carry the literal token OWNER-ESCALATION (or
  CANONICAL-ASM GRANT PATH) in its `## ` heading or the driver discards the session.

## [s12] The s12 byte proof reproduces on a clean tree — the driver's discard of the first s12 filing was a paperwork defect (heading token), not a measurement defect.
- mechanism: GCC 2.7.2 function.c assign_stack_local reserves the declared-but-untouched volatile u32 pre_pad[8] slot at RTL-expand and never reclaims frame_offset, reproducing target's .frame $sp,72 vars region 0x18-0x37; the arg0=0 dead store to a PARAM defeats cse2 canonical-register substitution over {arg0,p,saved} for target insn #18.
- probe: Fresh apply of memory/grind/func_80047EE8/candidate.c over src/text1b.c:19 (tmp/grind/func_80047EE8/apply.py), then `& tools/wteng.ps1 main build`, then `sandbox func_80047EE8 --disable all` on the same tree, then a regfix/asmfix row count and a re-read of every self-vet citation in the live tree.
- result: build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH (build_oracle_s12b.log); sandbox score 10 with target_insns 53 / build_insns 53 / cheat_asm_stripped 279 (sandbox_composite_s12b.log); 0 regfix/asmfix rows naming func_80047EE8; .claude/rules/no-new-park-categories.md:390-402, docs/reference/sotn-construct-index.md:101 and :103, engine/volatile_cheats.py:746-772 all resolve as cited.
- verdict: CONFIRMED

## [s12] The sandbox cannot read 0 for this function until engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS carries a func_80047EE8 row, so the residual 10 is a stripping artifact rather than a real byte gap.
- mechanism: engine/volatile_cheats.py::_is_sanctioned_pad gates the unwritten-pad exemption on exact function name + exact local name + exact element count + volatile; func_80047EE8 has no row (rows today: func_8001E404, func_8001E6E4, func_8003CF84), so the pad is stripped before scoring while the real build honours it.
- probe: Read engine/volatile_cheats.py:746-772 and compared the sandbox score (pad stripped) against the full driver build (pad honoured) on the identical tree.
- result: sandbox 10 vs full-build SHA1 MATCH on the same source — the two disagree exactly by the pad. The 2026-08-18 family ruling itself states verbatim that the allowlist 'requires it and a per-function row', so adding the row is the prescribed integration step, and engine/ is outside a grind session's allowed surface.
- verdict: CONFIRMED

## [s12] func_80047EE8 is an endgame lock whose AND-gate (b) fails for want of a SOTN-master precedent (the 2026-07-22 REFUSED / OWNER-ACCEPTED INCOMPLETE disposition, decisions.md:1298).
- mechanism: Gate (b) requires an in-hand cited precedent for the closing construct; the 2026-07-22 ruling recorded none for the phantom-frame-slot pad.
- probe: Re-checked the owner's 2026-08-18 general-family ruling (.claude/rules/no-new-park-categories.md:390) and its SOTN-master exhibits (docs/reference/sotn-construct-index.md:101 src/st/sel/2C048.c:564 `volatile u32 pad; // !FAKE:`; :103 src/st/sel/stream.c:80 `volatile u32 pad[4]; // FAKE`) plus three landed in-repo applications, then compiled the construct and proved the bytes.
- result: Precedent exists and is citable; the construct compiles and closes the function. Gate (b) now PASSES, so the 2026-07-27 both-gates-fail auto-ruling does not apply.
- verdict: KILLED
