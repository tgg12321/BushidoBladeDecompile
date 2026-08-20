# Evidence bank — func_80038170

- WIP rejected_form: {'form': 'dummy0/dummy1 + __asm__ "m" (HEAD)', 'score': 'matches-via-cheat', 'why': 'FORBIDDEN frame-coercion; sandbox strips it -> honest 12'}

- WIP rejected_form: {'form': 'strip all cheats, s1,s2,s3 order (clean pure C)', 'score': 12, 'why': 'pure +8-byte frame gap; body byte-identical'}

- WIP rejected_form: {'form': 's32 s3=0,s2=0,s1=0 (reversed decl order) — rejected/decl-order-prologue-flip.c', 'score': 'would be 12 same', 'why': 'FAIL: cheat-reviewer — prologue-save-order manipulation, param-local-alias-prologue-pair-flip family'}

- WIP rejected_form: {'form': '{ s32 tmp[2]; } (dead array nested block, no stores)', 'score': 'gives vars=16/frame=56 with correct save offsets, no extra body stores', 'why': 'FORBIDDEN: dead-vars-local-array; engine detector refuses completion'}

- WIP rejected_form: {'form': '{ s32 tmp[2]; tmp[0]=0; tmp[1]=0; } (dead array with stores)', 'score': 'gives vars=16 but adds 2 extra sw insns + shifts register alloc', 'why': 'FORBIDDEN: dead-vars-local-array + produces wrong body asm'}

- WIP rejected_form: {'form': 's32 tmp1,tmp2; (dead scalars)', 'score': 'vars=8', 'why': 'GCC eliminates dead scalar stack slots entirely'}

- == imported from memory/wip notes.md ==
# func_80038170 (code6cac_c_mid.c) — BLOCKED: phantom 8-byte frame local

## TL;DR
Same class as func_8001924C. Target frame -56, cc1 naturally gives -48. The ONLY pure-C
mechanism that gives vars=16/frame=56 without extra body asm is `{ s32 tmp[2]; }` (dead array,
no stores) — **FORBIDDEN** per `dead-vars-local-array` (engine detector refuses completion).
Declaration-order reversal (s3,s2,s1) also **FAILS** cheat-reviewer (prologue-save-order
manipulation). Honest distance = **12, all from +8-byte frame delta**. 141-insn body is
otherwise byte-identical. No recoverable semantic local. Card BLOCKED.

## The gap (precise)
- natural cc1 (clean pure C): `addiu sp,sp,-48`, vars=8, saves at 24/28/32/36/40
- target:                       `addiu sp,sp,-56`, vars=16, saves at 32/36/40/44/48
- Body: NO sp-relative accesses except prologue saves + epilogue restores.
- The 8 extra bytes at sp+16..sp+31 (target) are NEVER accessed. Phantom local.

## What was tried (full lever record)

1. **HEAD cheats stripped** → score 12 (all frame delta). Body byte-identical.
2. **s3,s2,s1 declaration order** → gives correct save ORDER naturally (s0,s3,s2,s1,ra).
   FAILS cheat-reviewer: prologue-save-order manipulation (param-local-alias-prologue-pair-flip).
   Saved under `rejected/decl-order-prologue-flip.c`.
3. **Dead array `{ s32 tmp[2]; }` (no stores, nested end block)** → vars=16/frame=56, correct
   save offsets, NO extra body stores, no register-alloc shift. FORBIDDEN: dead-vars-local-array.
   This IS the compiler mechanism but the technique is forbidden by policy.
4. **Dead array with stores** → vars=16 but adds 2 extra `sw $0,16($sp)` instructions + shifts
   register allocation. Does not match target body even if frame is right.
5. **Dead scalars `s32 tmp1,tmp2`** → vars=8. GCC eliminates scalar dead-stack slots entirely.
6. **Arg-area expansion** → func_80079194 called with only 2 args; expanding prototype doesn't
   change arg-build area (only actual passed args count).
7. **Structural variants** (all s1/s2/s3 orderings, outer j/k/v loops, mask-before-i) → all
   give vars=8/frame=48. No structural variant produces vars=16.

## HEAD cheats (all forbidden)
- `register s32 i asm("a3")`, `register s32 mask asm("a1")` — register pins
- `s32 dummy0, dummy1; __asm__ volatile ("" :: "m"(dummy0), "m"(dummy1));` — dead-vars frame-coercion
- regfix `reorder 10,11,13,12,9 @ 9-13` — save-order reorder
- `prologue_config.json` entry — replaces prologue with hardcoded -56 frame

## Resume guidance
No remaining pure-C levers. The correct disposition is park (unrecoverable phantom frame local,
no semantic local recoverable from body). Surface for user authorization (canonical-asm or accept
as indefinitely parked INCOMPLETE). Do NOT attempt any dead-array or declaration-order approach.


- == session s1 (2026-07-28, recon) ==

- [s1] Clean-strip baseline re-measured: floor 14 (not the ledger's 12) — 12 frame-delta insns (2x addiu sp + 10 save/restore offsets) + prologue pair-order residue. Metric: engine masked Levenshtein vs build/src reference .o.

- [s1] FRAME GAP SOLVED IN PURE C (kills the WIP-era "unrecoverable phantom local / no remaining pure-C levers" conclusion). Mechanism = phantom-frame-slots-gcc272 (memory/project/): rewriting the if(s3) pair as ONE-table indexing `(&D_8008F19C)[s3*2+0]` / `[s3*2+1]` (D_8008F19D is the splat per-byte auto-symbol for D_8008F19C+1) makes GCC 2.7.2 allocate an 8-byte compiler stack temp: vars 8->16, frame 48->56 == target, zero dead decls, zero extra insns, zero stores to the slot. Floor 14 -> 5. Layer-1 cheat-reviewer: PASS on this construct ("the MORE correct spelling").

- [s1] Phantom-slot trigger matrix (cc1 .frame probes, tmp/grind/func_80038170/s1/): pair off one symbol with shared var index in TWO statements = +8 (vI); single read = 0 (vJ/vL); two-symbol spelling = 0 (vK); named-pointer staging u8*t=&SYM+i*2 = 0 (vM2/vM4); u16 HImode named temp in copy loop = 0. Outer-block pairs (D_8008F1A8 x2) share ONE slot (= the pre-existing natural 8); a pair inside the if() CONDITIONAL allocates a SECOND slot (no reuse across cond scope) -> 16. Copy loops contribute nothing (vD only-copies = vars 0).

- [s1] Residual floor-5 = prologue init/save pair order ONLY: ours s0,s1,s2,s3,ra; target s0,s3,s2,s1,ra; frame + all offsets correct. Order tracks the source order of the three live zero-inits.

- [s1] `s1 = s2 = s3 = 0;` (chained, decl order unchanged) emits s3,s2,s1 (RTL right-to-left) -> floor 1. Layer-1 cheat-reviewer: FAIL (same family as banked decl-order-prologue-flip; "different spelling of the same intent"). Banked: rejected/chained-zeroing-order.c. Do not re-propose order-steering spellings; s1 files a ruling-request instead.

- [s1] The floor-1 residual (with an order lever) is a PROVEN text-only artifact: source emits reloc D_8008F19C+1 where the reference .o (old cheat-form build) spells symbol D_8008F19D. Linked words identical (lui 0x0980 / lbu imm 0xF19D both ways). Word-level diff vs asm/funcs raw words: ONLY the jal func_80079194 reloc word differs pre-link (141/141 insns, count + stream otherwise byte-equal). Once build/ regenerates from this src the sandbox reads 0. Sandbox-0-this-session is unreachable BY CONSTRUCTION for the correct source spelling.

- [s1] Cheat carriers still in tree (driver/retire surface, NOT src): 1 regfix rule (reorder @9-13 — would now MANGLE the already-correct natural prologue if applied) + tools/prologue_config.json func_80038170 entry (hardcoded -0x38 prologue — now redundant: natural cc1 output is identical text). Sandbox filters both (engine/pipeline.py treats prologue_fix as tracked cheat), so floor 5/1 is honest pure-C.

- [s1] Oversized-locals carve-out (2026-07-13 owner ruling, dead-vars-local-array.md) would have covered this function's frame gap (frame-math: 56 - 24 saves - 16 args = 16-byte locals region, 0 bytes written) — NOT NEEDED: the natural live-form closes it. Recorded for family reference only.

- [s1] canonical: verdict C, distance 1..5 range, asm_insns 0 — pure-C target confirmed.

- == session s2 (2026-07-28, recon — post-Judge-ruling) ==

- [s2] Judge-sanctioned spelling applied verbatim (decl `s32 s1, s2, s3;` + separate `s3 = 0; s2 = 0; s1 = 0;`): save/init pair order now correct NATURALLY (s0,s3,s2,s1,ra offsets 0x20/0x2C/0x28/0x24/0x30, frame -0x38). Engine sandbox floor 1.

- [s2] NEW MEASUREMENT — the Judge spelling ALONE does NOT byte-match: with `for (i = 0; ...)`, sched1 emits `move a3,zero` (i-init) AFTER the li a0,1 / lui+lw D_80106A50 cluster and `sw ra` lands at slot 12 instead of 14 — 5 words displaced vs target (target order: move a3 @9, li @10, lui @11, lw @12, li a2 @13, sw ra @14). THE ENGINE MASKED METRIC HIDES THIS (still prints 1) — raw .o word diff is the only honest gauge for this window. Prior floor-1 word-proof was for the (rejected) chained form only; nobody had word-diffed the separate-statement form.

- [s2] FIX (layer-1 reviewer PASS): standalone `i = 0;` BEFORE `mask = D_80106A50;`, empty for-init (`for (; i < 0x1B; i++)`). Restores target schedule exactly: 141/141 insn words match the oracle stream (diff vs build/ reference .o = oracle bytes). Ordinary live-statement order; reviewer classed it with store-before-jal / hoist-call-arg-local accepted scheduling-lever families.

- [s2] Sole remaining .o-text diff re-proven linker-identical: candidate `lui %hi(D_8008F19C)` + `lbu %lo(D_8008F19C)+1` = 0x8009 / 0xF19D == target `lui %hi(D_8008F19D)` + `lbu %lo(D_8008F19D)` (asm words 3C018009 / 9022F19D). Vanishes at the .o level once build/ regenerates from this src — sandbox 0 unreachable BY CONSTRUCTION until then (carriers block a rebuild, below).

- [s2] Carriers measured HARMFUL now: sandbox with rules applied scores 4 (vs clean floor 1) — the regfix.txt:1250 reorder @9-13 scrambles the now-correct natural prologue. A full `build` with the carriers active would break the oracle; integration must retire regfix.txt:1250 + tools/prologue_config.json func_80038170 entry FIRST, then rebuild → sandbox 0 → SHA1==oracle → FINAL CALL. Both surfaces are driver-only (forbidden to grind sessions).

- [s2] canonical re-confirmed: verdict C, distance 1, asm_insns 0.

- [s1] Judge-sanctioned spelling applied verbatim in src/code6cac_c_mid.c: decl `s32 s1, s2, s3;`, separate `s3 = 0; s2 = 0; s1 = 0;` — save order + frame -0x38 + all offsets natural-correct

- [s1] NEW: prior floor-1 word-level proof covered only the rejected chained form; the Judge form alone leaves 5 displaced words (move a3,zero late, sw ra at 12 not 14) that the engine masked metric hides — raw .o word diff is the honest gauge for this window

- [s1] Fix: `i = 0;` standalone before the mask load, `for (; i < 0x1B; i++)` — 141/141 word match vs build/ reference (= oracle bytes); layer-1 cheat-reviewer PASS on the full body

- [s1] Reloc artifact re-proven linker-identical: %hi/%lo(D_8008F19C)+1 == %hi/%lo(D_8008F19D) == 0x8009/0xF19D (target words 3C018009 / 9022F19D); vanishes once build/ regenerates from this src — sandbox 0 unreachable by construction until then

- [s1] Carriers measured harmful: rules-applied sandbox 4 vs clean 1; regfix.txt:1250 + tools/prologue_config.json func_80038170 entry must be retired at integration BEFORE any rebuild (both driver-only surfaces)

- [s1] canonical: verdict C, distance 1, asm_insns 0

- [s2] src/code6cac_c_mid.c at s2 session start did NOT carry the Judge form — HEAD still had the old cheat body (pins + dummy-asm + two-symbol spelling); grind-session src edits are reverted between sessions, so integration MUST re-verify src carries the candidate (it now does, applied this session)

- [s2] sandbox func_80038170 --disable all with the Judge form in src: score 1, target_insns 141, build_insns 141, rules_dropped 1, cheat_asm_stripped 29 (file-wide count, not this function)

- [s2] word_diff vs asm/funcs oracle words: 141/141, sole diff the jal reloc placeholder resolved at link — parity with the oracle stream re-proven this session (tmp/grind/func_80038170/s2/diffcheck_out.txt)

- [s2] normalized 141-insn diff vs build/src/code6cac_c_mid.o reference: exactly 6 differing insns — [93]/[95] the linker-identical reloc spelling (D_8008F19C+1 imm 90220001 vs D_8008F19D 90220000), [29]/[33]/[35]/[37] intra-function j words off by one word (080000c2 vs 080000c3) purely from the file-wide cheat-asm-strip offset shift in the sandbox .o (tmp/grind/func_80038170/s2/norm_diff_out.txt)

- [s2] D_8008F19D no longer referenced anywhere in src/code6cac_c_mid.c after the edit — no dangling extern

- [s2] Form unchanged from the banked layer-1-reviewer-PASS candidate; Judge-binding spelling followed verbatim (decl s32 s1, s2, s3; separate s3=0; s2=0; s1=0;)

- [s3] [s3] src at session start had again reverted to the cheat form (pins + dummy-asm + two-symbol spelling); Judge form re-applied verbatim, sandbox --disable all = 1 (141/141, rules_dropped 1) — third independent reproduction of the floor

- [s3] [s3] Exhaustion completed: the D_8008F19D-rebased one-symbol pair (only untested spelling class) also scores 1 — the reloc addend artifact is inherent to EVERY frame-preserving spelling; sandbox 0 is unreachable from source by construction until build/ regenerates from the Judge form

- [s3] [s3] Deadlock made explicit: grind.ps1 Invoke-CandidatePath requires sandbox==0 BEFORE it runs retire, but sandbox 0 requires build/ regenerated from the Judge form, which requires the two harmful carriers (regfix.txt:1250, tools/prologue_config.json entry — rules-applied sandbox 4 vs clean 1, s2) retired FIRST; both surfaces are forbidden to grind sessions, so no session can ever pass the gate

- [s3] [s3] OWNER-ESCALATION filed in docs/grind/decisions.md (line '## 2026-07-28 — func_80038170 — OWNER-ESCALATION — integration-gate deadlock...') with the 4-step owner integration recipe: apply candidate.c, drop both carriers, full build (SHA1==oracle expected), queue done

- == session s4 (2026-08-19, permuter modality — pre-empted by a chassis change) ==

- [s4] CHASSIS MOVED SINCE s3, AND IT BROKE THE DEADLOCK. The asm-until-matched migration (commit 4faaa384, "migrate: asm-until-matched batch 2") RETIRED the harmful `regfix.txt:1250 reorder 10,11,13,12,9 @ 9-13` carrier and replaced the function body in src with `INCLUDE_ASM("asm/funcs", func_80038170);`. Only a stale COMMENT block remains at regfix.txt:655-657. `sandbox --disable all` now reports `rules_dropped 0`. The s3 OWNER-ESCALATION's circular gate (sandbox 0 requires a rebuild; the rebuild requires retiring a carrier no grind session may touch) therefore no longer exists: with no rule to mangle the already-correct natural prologue, the banked body can simply be spliced over the INCLUDE_ASM line and built.

- [s4] The banked s3 candidate.c DID NOT LINK on this chassis: `undefined reference to func_80079194`. That address is `strcpy` (asm/funcs/strcpy.s starts at 0x80079194; known_psyq_stdlib.txt:15 lists func_80079194 in the psyq stdlib set), and the prototype `extern u8 *strcpy(u8 *, u8 *);` was already present at src/code6cac_c_mid.c:279. Spelling the call `strcpy(out + 4, D_8008F1C0)` links and is codegen-neutral (same jal target, same two pointer args). Also corrected: `(&D_8008F204)[i]` / `(&D_8008F1A8)[...]` -> plain `D_8008F204[i]` / `D_8008F1A8[...]`, since both are declared `extern u8 X[];` in include/code6cac.h (identical codegen; the `&D_8008F19C` spelling is RETAINED — that symbol is a scalar `extern u8` and the shared &-base is what produces the -0x38 frame).

- [s4] SOLVED. `engine build` -> build/bb2.exe sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH. Then `sandbox func_80038170 --disable all` -> score 0, target_insns 141, build_insns 141, rules_dropped 0, cheat_asm_stripped 17 (file-wide, other functions). Both measured this session with the body in place in src/code6cac_c_mid.c.

- [s4] The surviving tools/prologue_config.json func_80038170 entry is a MEASURED NO-OP for this function, not a load-bearing carrier: sandbox --disable all STRIPS prologue_fix (engine/pipeline.py tracks it as a cheat surface) and still scores 0 against a build/ reference that was produced WITH prologue_fix active. Therefore the natural cc1 prologue is textually identical to the hardcoded replacement list. `retire func_80038170` should delete it and the oracle will stay MATCH.

- [s4] No permuter campaign was launched (mandated modality pre-empted at the first measurement). The banked form closed without any search; launching a campaign after a proven oracle match would have been pure waste. Artifacts: tmp/grind/func_80038170/s4/measurements.txt, tmp/grind/func_80038170/s4/body.c.

- [s4] Ledger correction for future readers: the s1/s2/s3 conclusion "sandbox 0 is unreachable BY CONSTRUCTION" was correct ONLY relative to the then-current chassis (a stale build/ reference plus an un-retirable regfix carrier). It was a CHASSIS-RELATIVE statement, not a property of the source. This is the concrete case the dispatch brief's chassis-check warning describes — re-measure banked "impossible" conclusions before spending a session on them.
