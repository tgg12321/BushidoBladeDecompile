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
