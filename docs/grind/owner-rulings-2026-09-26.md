<!-- Provenance: recorded verbatim by the orchestrator from the owner's (Trenton's) AskUserQuestion answers on 2026-09-26; batch 1 (originally tmp/orch/owner_rulings_2026-09-26.md) then batch 2 (originally tmp/orch/owner_rulings_2026-09-26b.md), concatenated unedited (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (conversation with orchestrator, via AskUserQuestion) — VERBATIM RECORD

Context given to the owner (orchestrator's plain-language framing, which the owner approved):

## Q1 — Reused variables
Question: "Allow a local reused for several unrelated values when compiler dumps prove no one-variable-per-value spelling can match?"
Owner chose: **"Allow with proof (Recommended)"** — option text: "Admit only when allocator-dump proof shows necessity, honest generic name, and layer-2 still reviews. Unblocks 80055138, 8003993C, 8002DE20 (C part)."
Framing: "Allow it only with proof: the worker must show from compiler dumps that no single-purpose spelling can match, give the variable an honest generic name, and still pass layer-2."

## Q2 — Table merge (aggregate merge prong (a))
Question: "Accept 'only one object compiles to these bytes' (confirmed by the original PsyQ compiler) as evidence for merging adjacent data labels into one table?"
Owner chose: **"Accept, minimal span (Recommended)"** — option text: "Compiler-necessity proof + cc1psx confirmation + merged object limited to the labels actually used. Unblocks 800620B8, likely 8005D814."
Framing: "accept compiler necessity as evidence when it's confirmed by the original compiler and the merged object is the smallest span that covers the labels actually used."

## Q3 — Inner-loop break (Ruling 9 prong (c))
Question: "Ruling 9 prong (c): may a `break` that only exits an inner loop sit between the write and its use?"
Owner chose: **"Yes, inner-loop break OK (Recommended)"** — option text: "The break never skips the use. Unblocks func_8006F97C (full match, everything else already accepted)."

## Q4 — inline_o.h GTE blocks
Question: "Grant GTE blocks copied verbatim from PsyQ's inline_o.h (with its `move $12` setup step) as a class?"
Owner chose: **"Grant as a class (Recommended)"** — option text: "Character-identical to a pinned inline_o.h copy; no per-function owner row needed."
Framing: "grant this class, provided the block matches a pinned header copy character for character."

Not decided (left for separate investigation): func_80036140's build-model changes (-G8 per file, maspsx COMMON no-gp model).

# Owner rulings 2026-09-26 (second batch, via AskUserQuestion) — VERBATIM RECORD

Context given to the owner: func_80034708 (and func_80036140) need a new per-file -G8 TU. Evidence for func_80034708: the target reads D_800A3174 gp-relative 16 times, the neighbouring functions none; the original PsyQ cc1psx emits those gp reads at -G8 and none at -G0. The 2026-09-26 first batch explicitly left per-file -G8 undecided.

## Q5 — Per-file -G8
Question: "Allow giving a function its own source file compiled at -G8 (small-data setting) when the shipped code proves it — gp-relative reads in the original bytes that neighbours lack, confirmed by the original PsyQ compiler producing them only at -G8?"
Owner chose: **"Allow with that proof (Recommended)"** — option text: "Requires gp-relative accesses in the original bytes + cc1psx confirmation + neighbours moved unchanged; layer-2 still reviews. Unblocks func_80034708 (and part of func_80036140)."

## Q6 — `0($12)` in the inline_o.h class
Question: "Should the inline_o.h class grant also accept the `0($12)` spelling our assembler tool forces in place of the header's `($12)`?"
Owner chose: **"Keep strict wording"** — option text: "Only exact character copies; func_8002DE20's blocks need a per-function grant."
(No rule change; this confirms 262db111c's prong (C) as written.)

Not decided: func_80036140's maspsx COMMON-no-gp model (a maspsx behaviour change) — Q5 covers only the per-file -G8 part.

## Q7 — Mixed-field struct under the aggregate-merge compiler-necessity alternative
Question: "Your table-merge ruling covered one repeated record type. func_80034708 needs 0x78–0x87 declared as ONE struct with mixed fields (a u16 pair, four byte pairs, four bytes). Both compilers, including the original PsyQ one, match the shipped code only with that struct, and it spans exactly the bytes the function uses. Extend the ruling to mixed-field structs under the same proof?"
Owner chose: **"Allow with same proof (Recommended)"** — option text: "Same dump proof + cc1psx confirmation + minimal span of bytes actually used; layer-2 still reviews. Unblocks func_80034708 (with -G8)."

<!-- Batch 3: appended verbatim from tmp/orch/owner_rulings_2026-09-26c.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (third batch, via AskUserQuestion) — VERBATIM RECORD

## Q8 — -G8 screening scope
Question: "The -G8 screening rule says every small variable a -G8 file mentions must be on the small-data list. func_80034708's file mentions one 4-byte counter (D_800A37B8) that can't go on that list — other functions access it the normal way — and its code is identical at -G0 and -G8. The already-approved text1a -G8 files have 29 such variables. Should screening only require listing the variables whose compiled code actually changes under -G8?"
Owner chose: **"Only if code changes (Recommended)"** — option text: "A small variable must be listed only when -G8 changes its compiled instructions; proven by building it both ways (bytes identical). Unblocks func_80034708; matches existing text1a practice."

<!-- Batch 4: appended verbatim from tmp/orch/owner_rulings_2026-09-26d.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (fourth batch, via AskUserQuestion) — VERBATIM RECORD
Context: research report memory/grind/func_80036140/research-common-gp.md (commit 2974e2e6b): Sony ASPSX 2.34 run under dosemu2 shows tentative (.comm) definitions get gp only for the first byte, never sym+k; census of 2,271 gp accesses in the shipped binary, zero exceptions; proposed gated list maspsx_comm_syms.txt (--comm-syms); all 34 src objects byte-identical with it.

## Q9 — maspsx COMMON (tentative-definition) gated list
Question: "Add a small per-function list telling our assembler shim which variables were plain 'declared, no initial value' in the original source (so it stops using the gp register for their byte offsets) — proven by Sony's own ASPSX assembler reproducing the shipped bytes, zero exceptions across the binary, and all current files byte-identical with it?"
Owner chose: **"Allow, gated list (Recommended)"** — option text: "maspsx_comm_syms.txt naming only functions whose listed variables are proven by Sony's assembler; full oracle + layer-2 before use."

## Q10 — func_80036140's -G8 neighbour func_80036940
Question: "func_80036140 shares a gp-read variable with its not-yet-decompiled neighbour func_80036940 (currently rotated), so the original compiled both at -G8. How to proceed?"
Owner chose: **"Do 36940 first, then both (Recommended)"** — option text: "Bring func_80036940 back from rotation now and decompile it, then move both into one -G8 file together. No rule change."

## Q11 — func_8002DE20 GTE blocks: maspsx `($12)` parser fix + per-function DMPSX-word row
Context: slotE's banked ledger memory/grind/func_8002DE20/ (commit 72c3b4d41): islands written as separate verbatim inline_o.h macro statements; two residual deviations: D1 `0($12)` for `($12)` (6 statements; forced by maspsx's load/store parser tools/maspsx/maspsx/__init__.py:182-194 requiring a non-empty offset; a 2-line parser fix makes the verbatim form build to identical .text), D2 `.word 0x4A486012` for `.word 0x0000013f` (3 statements; the DMPSX post-pass substitution, mapping backed by the 2026-09-24 Extension).
Question: "For func_8002DE20's GTE blocks: fix our assembler shim so it accepts the header's exact `($12)` spelling (2-line parser fix, all builds byte-identical), and grant a per-function row for the one remaining difference — the 3 command words Sony's DMPSX tool would have patched in?"
Owner chose: **"Fix parser + grant row (Recommended)"** — option text: "Blocks become header-exact except the DMPSX-patched word; per-function owner_cluster_grants row for that only. Plus the engine recognizer update. Layer-2 reviews everything."

<!-- Batch 5: appended verbatim from tmp/orch/owner_rulings_2026-09-26e.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (fifth batch, via AskUserQuestion) — VERBATIM RECORD
Context: layer-2 review of func_8002DE20 (src/code6cac_b.c:2353 cross_a re-stores A x c on the test-1-fail path; :2368 cross_b re-stores A x p2 on the test-4-fail path; on other incoming paths the variable holds a different value, so each write is necessary). Ruling 11 (B)(2) / Ruling 5 2(c) text: "no write stores a value the variable already holds, judged by C semantics" — no path quantifier.

## Q12 — path-wise re-store
Question: "The reused-variable rules forbid 'a write that stores a value the variable already holds'. In func_8002DE20 two writes re-store the same value on ONE of the paths reaching them, but on the other paths the variable holds something else, so the write is needed there and removing it breaks the program. Does that count as the banned redundant write?"
Owner chose: **"No — only if redundant on all paths (Recommended)"** — option text: "A write is banned only when it's removable: the variable already holds that value on EVERY path reaching it. A write needed on some path is allowed."

<!-- Batch 6: appended verbatim from tmp/orch/owner_rulings_2026-09-26f.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (sixth batch, via AskUserQuestion) — VERBATIM RECORD
Context: func_80036140 (slotI). At -G8, no separate-object spelling reaches the target's `la; lX 0(reg)` at the E9C/EA4 read-modify-write sites (plain 18, pointer-RMW locals 18, function-scope alias 42); a <=8-byte variable is small data at -G8 so cse folds the pointer into a direct access. So E9C and EA4 must sit in one object >8 bytes, which necessarily covers E9E and EA0 — bytes func_80036140 never touches, but which other functions access with their own widths (cdrom_StartRead, game_FrameLoop: E9E as u16; cdrom_ReadyCallback, func_80036940: EA0 as s32). (a4')(2)/(3) require such bytes to be an offset-named filler, which would force pointer puns in those consumers, failing (a4')(5).

## Q13 — forced-in bytes in a mixed-field struct
Question: "func_80036140 needs a struct that must cover a few bytes it never touches itself. Other functions DO use those bytes: one as a 16-bit value, another as a 32-bit value. The mixed-struct rule says untouched bytes must be an anonymous filler, but then those other functions would need pointer tricks to reach them, which is banned. May such forced-in bytes be proper named fields, typed by how the other functions actually access them?"
Owner chose: **"Yes, typed by real users (Recommended)"** — option text: "Only bytes the necessity proof forces inside the span; each field's type must match another function's actual access width/signedness in the original bytes; layer-2 reviews."

<!-- Batch 7: appended verbatim from tmp/orch/owner_rulings_2026-09-26g.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (seventh batch, via AskUserQuestion) — VERBATIM RECORD
Context: func_80036140 (slotI). Under (a4') as amended by Q13 (61f37ea5b), forced-in byte 0x80101EA0 (g_cdread_expected_pos, s32 on main) is accessed in the original bytes only by cdrom_ReadyCallback 800360A4 lw + bne (equality), 800360E4 lw / addiu +1 / 800360FC sw, and func_80036940 80036AAC sw of CdPosToInt's return — no ordered slt/sltu, no plain sra/srl, no div/divu anywhere, so the committed no-default clause leaves it a filler and (a4')(5) then fails. s32 and u32 compile identically for these accesses.

## Q14 — signedness when the binary cannot distinguish
Question: "For func_80036140's struct, one forced-in 32-bit field (the expected disc position at 0x80101EA0) is only ever loaded, compared for equality, incremented and stored — so the shipped code can't reveal whether it was signed; signed and unsigned compile to identical bytes. May it keep the type it already has on main (s32, which also matches libcd's CdPosToInt returning int), when no instruction in the binary can tell them apart?"
Owner chose: **"Keep existing type (Recommended)"** — option text: "When no signedness-revealing instruction exists anywhere and both choices are byte-identical, the member keeps its current declared type on main (or the SDK type it's assigned from); layer-2 checks byte-identity."

<!-- Batch 8: appended verbatim from tmp/orch/owner_rulings_2026-09-26h.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (eighth batch, via AskUserQuestion) — VERBATIM RECORD
Context: layer-2 review of func_80036140's joint landing (slotI). (1) maspsx_comm_syms.txt prong (a)(2) requires ASPSX to match "the shipped words of that function, relocation immediates masked"; for func_80036140 cc1psx+ASPSX 2.34 differs in 81/512 words (64 = ASPSX li->ori expansion where shipped has addiu; rest = cc1psx scheduling) though every gp and sym+N decision matches; cdrom_SetMix (0/18) and func_80035F78 (0/12) match fully. (2) -G8 prong (iv) requires respellings to land first or be byte-neutral on the unsplit tree; the CdlATV merge is only byte-neutral after cdrom_SetMix moves to its -G8 file.

## Q15 — COMMON-list row proof when the archived toolchain is not byte-faithful
Question: "For the new 'declared, no value' assembler list: func_80036140's row needs Sony's archived assembler to reproduce the shipped bytes. Every gp/offset decision matches, but 81 of 512 words differ, all explained: the archived compiler schedules a few things differently, and the archived assembler expands `li` as `ori` where the shipped game has `addiu` (so the archive isn't byte-identical to what Square shipped). May the row count as proven when every gp/offset decision matches and the leftover differences are fully explained this way?"
Owner chose: **"Yes, if all differences explained (Recommended)"** — option text: "Every gp and sym+N decision must match; every other differing word must be classified (scheduling or li-expansion) in the ledger; layer-2 checks. The two neighbour rows already match 100%."

## Q16 — -G8 split / respelling order
Question: "The -G8 rule says any struct/type respelling must land BEFORE the file split (or be proven harmless before it). func_80036140's CdlATV merge only builds correctly AFTER cdrom_SetMix moves to its -G8 file. May the order be reversed — split commit first (proven identical to the original), then the merge commit (also proven identical)?"
Owner chose: **"Allow reverse order (Recommended)"** — option text: "Separate commits, split first then merge, each independently proven to build the exact original; layer-2 reviews both."

<!-- Batch 9: appended verbatim from tmp/orch/owner_rulings_2026-09-26i.md (line endings normalized to LF; blank separator line added). -->
# Owner ruling 2026-09-26 (ninth batch, conversation) — VERBATIM RECORD
Context: the orchestrator proposed a scratch-only study of a narrow cc1 fidelity patch for func_8001A820 (our GCC 2.7.2 merges `lhu; sll 16; sra 16` into `lh` for signed halfword reads through a lui-only scratchpad pointer; Sony's cc1psx keeps the shipped three-instruction form), citing the adopted PLUS->IOR patch (9bc64b751) as precedent. The owner asked "SOTN doesn't do compiler patches do they?"; the orchestrator answered that, to its knowledge, SOTN-style PS1 decomps use a rebuild of the original compiler version patched only to build on modern hosts, and switch compiler versions rather than patch output; recommended withdrawing the study.

## Q17 — compiler patches
Owner (verbatim): "Withdraw that option, consider a compiler patch a cheat. Update documentation, mark relevant funcs as cheated and add them back to the queue"

## Scoping note (orchestrator to owner, same conversation, after Q17; owner has not responded to it)
Orchestrator (verbatim excerpt): "One scoping point for you. Today's assembler-shim changes (the `($12)` parser fix and the "declared, no value" list) are changes to maspsx, not the compiler. SOTN uses maspsx too. I'm treating those as outside this ruling. The census will list them separately so you can decide if you disagree."
Owner: no reply to this point as of recording. Status: orchestrator's scoping, not an owner ruling; open for the owner to overrule.

<!-- Batch 10: appended verbatim from tmp/orch/owner_rulings_2026-09-26j.md (line endings normalized to LF; blank separator line added). -->
# Owner ruling 2026-09-26 (tenth batch, via AskUserQuestion) — VERBATIM RECORD
Context: func_80036140's maspsx_comm_syms row under Q15 (0134264b7): cc1psx -G8 + ASPSX 2.34 gives 516 words vs shipped 512; 68 li expansions and 8 one-to-one scheduling moves classify; all 25 gp/sym+N decisions match; unclassifiable under the strict text: 4 nop words (blocks 1, 6, 11, 16), each at a spot cc1psx's own output marks `#nop` after a reordered instruction (ASPSX materialises them; the shipped order needs none), and 1 branch (8003614C blez $t0, offset 0x79 vs 0x75) differing only because those 4 nops lie between it and its target. (slotI ledger 44b97560b, landing/q15/func_80036140.cc1psx-G8.strict.txt.)

## Q18 — nop pads and shifted branch offsets as scheduling fallout
Question: "func_80036140's assembler-list row: Sony's archived compiler orders a few instructions differently. At 4 of those spots the archived assembler inserts a no-op timing pad (the shipped order doesn't need one), so one branch's jump distance grows by those 4 pads. Every gp/offset decision matches and nothing else is unexplained. May those 4 pads and that one branch distance count as part of the already-allowed 'scheduling' differences?"
Owner chose: **"Yes, as scheduling fallout (Recommended)"** — option text: "Only nops the archived compiler itself marked (#nop) at a reordered spot, and branches whose only difference is offset shifted by exactly those nops; each listed by address; layer-2 checks."

<!-- Batch 11: appended verbatim from tmp/orch/owner_rulings_2026-09-26k.md (line endings normalized to LF; blank separator line added). -->
# Owner rulings 2026-09-26 (eleventh batch, via AskUserQuestion) — VERBATIM RECORD
Context: (Q19) func_80027AD8 (slotK): the target reads a stack-passed argument `rec` through two local copies (`tbl` for field reads, `tbl_arg` passed to func_800278C0); GCC 2.7.2 halves allocation priority for a never-reassigned stack-passed argument (local-alloc doubles its live length), so `rec` itself never gets a saved register; without tbl 35, without tbl_arg 52, both 2 (jump-table relocation only). Ruling 11 admits only multi-value locals and excludes bare copies of a parameter. (Q20) func_8001CE60 (slotM): matches (0/588) only when ONE local holds both the announcement length (80 or 100, set in if/else arms) and later the clock's frames-left; shared, it is not block-local so local-alloc combine_regs doesn't tie it (cc1psx agrees); Ruling 11 (C)(3) refuses an all-constant value.

## Q19 — local copies of a parameter
Question: "func_80027AD8: the original reads a stack-passed argument through TWO local copies (one for field reads, one passed to another function). Dumps show GCC gives an unmodified stack argument half priority, so the shipped register use only happens if the code copied it into locals; with both copies it matches. Plain copies of a parameter are banned today as a classic cheat device. Allow them under the same mechanism-proof standard as the reused-variable ruling?"
Owner chose: **"Allow with R11-style proof (Recommended)"** — option text: "Dumps must prove necessity by mechanism for each copy (no spelling without it matches, incl. all FAKE families), honest role names, each copy genuinely used; layer-2 reviews."

## Q20 — per-branch constants as a Ruling 11 value
Question: "func_8001CE60: it matches only when ONE local holds both an announcement length (set to 80 or 100 in if/else branches) and, later, the clock's frames-left. The reused-variable ruling currently refuses a value that is just a constant. Allow a per-branch constant (different constants on different paths) to count as a real value there, under the same proof?"
Owner chose: **"Allow per-branch constants (Recommended)"** — option text: "Only when the constant differs by path (a real choice made at runtime); a single unconditional constant still doesn't count; same R11 necessity proof and layer-2."

<!-- Batch 12: appended verbatim from tmp/orch/owner_rulings_2026-09-26l.md (line endings normalized to LF; blank separator line added). -->
# Owner ruling 2026-09-26/27 (twelfth batch, via AskUserQuestion) — VERBATIM RECORD
Context: func_8001CE60 (slotM) matches (sandbox 0/588; full build == oracle) only with D_800A3898[2] and D_800A38AA[2] declared as 2-element arrays in code6cac.c; with those arrays in scope func_800340A0 (COMPLETED-C, code6cac_b.c) drops to 5/88 because GCC (and Sony's cc1psx) keeps an array element's address in a register when the element is used twice; every array respelling of func_800340A0 stays at 5. Borderline entry filed 2026-09-26; edits in memory/grind/func_8001CE60/probes/per-tu-landing.diff.

## Q21 — per-file declarations of the same bytes
Question: "func_8001CE60 needs two player byte-pairs declared as 2-element arrays, but a completed neighbour in a different file (func_800340A0) only matches with the same bytes declared as separate single bytes — the original compiler (confirmed with Sony's cc1psx) can't produce both from one declaration. SOTN keeps annotated declaration mismatches when the original bytes require them. May each file declare those bytes the way its own code needs (arrays in code6cac.c, single bytes in code6cac_b.c), annotated, with the compiler evidence? A full build of this form matches the original."
Owner chose: **"Allow, per file, annotated (Recommended)"** — option text: "Only when proven that no single declaration compiles both files (dumps + cc1psx), declarations kept file-local (not in a shared header), each annotated; layer-2 reviews."

<!-- Batch 13: transcribed by the session author from the owner's AskUserQuestion answer in the 2026-09-27 manual session (func_8001CE60 landing); no separate tmp record exists. -->
# Owner ruling 2026-09-27 (thirteenth batch, via AskUserQuestion) — VERBATIM RECORD
Context: the layer-2 review of the func_8001CE60 landing (per-file score-byte declarations under Q21) FAILed with a counterexample: func_800340A0 compiles to its target under ONE shared `u8 [2]` array declaration when indexed through `s32 zero = 0; s32 one = 1; /* FAKE */` (tmp/review_ce60/arr_zero_one.c, banked as memory/grind/func_8001CE60/probes/calib_800340A0/A340_zero_one-Q22-refused.c; produces the same output as the per-file scalar form (A340_0) under both our cc1 and cc1psx; under cc1 that is the target), which defeats Q21 condition (1) if that construct is admissible.

## Q22 — dummy constant locals used as array indices
Question: "func_8001CE60 is matched, but the reviewer found a second way to make the old neighbour func_800340A0 match with the score bytes declared once, as an array: index the array with two dummy locals (`s32 zero = 0; s32 one = 1; /* FAKE */`, then `score[zero]`, `score[one]`). The compiler then treats the index as "unknown" and emits the direct accesses the original has. If that trick counts as allowed, the per-file declarations you approved (arrays in one file, single bytes in the other) are no longer proven necessary, and we'd have to add those FAKE locals to func_800340A0, which is clean C today. Our FAKE-local rules cover constants held across calls and `one = 1` against bit-test tricks, but not this purpose. Should dummy constant locals used as array indices count as an allowed construct?"
Owner chose: **"Doesn't count (Recommended)"** — option text: "Dummy constant locals used only as array indices are outside the sanctioned FAKE-local scope. The per-file declarations stay (no FAKE constructs anywhere). I fix the ledger proof and the file comments to cover this trick, then re-run layer-2."
Other options offered: "Counts: one shared declaration" — "Treat it as a sanctioned FAKE local. Declare the score bytes once as arrays in the shared header, and add the annotated `zero`/`one` FAKE locals to func_800340A0 (it goes from clean C to carrying two FAKE constructs). The per-file exception isn't used."; "Look for a clean form first" — "Don't decide yet: leave func_8001CE60 unlanded (INCLUDE_ASM), log the question to borderline.md, and keep searching for a single-declaration spelling of func_800340A0 with no FAKE constructs."

## Q23 — per-file declarations vs FAKE-construct spellings
Context (author's framing): the round-5 layer-2 review found func_800340A0 matching under one shared `u8 [2]` declaration through a FAKE pointer-alias spelling (tmp/rv5/ptr_end2.c, banked as memory/grind/func_8001CE60/probes/calib_800340A0/A340_ptr_end-Q23-set-aside.c).
Question: "The reviewer found a second trick that lets the finished function func_800340A0 match with the score bytes declared once as an array: a dummy pointer to the end of the array (`u8 *se = &score[2]; /* FAKE */`, then `se[-2]`, `se[-1]`). This one falls under an existing allowed FAKE family (pointer aliases), not the dummy-index trick you refused. Each such trick, if allowed, means the per-file declarations aren't "proven necessary", so we'd have to put FAKE constructs into func_800340A0, which is clean C today. Instead of ruling trick by trick: when the only way to use ONE shared declaration needs FAKE constructs, should the per-file declarations win?"
Owner chose: **"Per-file wins over FAKE (Recommended)"** — option text: "For the per-file-declaration rule, a one-declaration spelling that needs any FAKE-annotated construct (any family) doesn't count against it. The per-file form (zero FAKE constructs) lands, still with the compiler proof, annotations and layer-2. This matches the existing 'fewest no-purpose constructs wins' principle."
Other options offered: "Only this pointer trick" — "Refuse just this pointer-to-array-end-used-for-element-access trick (like the dummy-index one); other FAKE families still count against the per-file rule, case by case."; "Allow it: one declaration" — "The pointer alias is sanctioned, so the per-file rule isn't met: declare the score bytes once as arrays and add the FAKE pointer locals to func_800340A0."

## Q24 — how cc1psx counts where it cannot reproduce the matching form
Context (author's framing): after Q22/Q23 (rules commit 77297f798), the func_8001CE60 per-file proof splits into the score-byte addressing (cc1psx reproduces it) and func_800340A0's round-result store structure, which rules out ordinary winner-index single-declaration spellings (probes/calib_800340A0/A340_rv9_*) but which cc1psx does not reproduce even for the committed scalar form (A340_0.cc1psx.diff, 16-line jump-structure hunk).
Question: "For func_8001CE60's per-file declarations, your rule says the proof needs "dumps + cc1psx" (Sony's original compiler) to agree that one shared declaration can't work. cc1psx agrees on half of it: arrays break the score-byte addressing, single bytes don't. The other half is how func_800340A0 stores the round result, and that's what rules out the ordinary 'winner index' rewrites that fix the addressing under one declaration. cc1psx can't judge that half, because it doesn't reproduce the original's store/jump structure even for the version of func_800340A0 that already matches (it's a slightly different compiler build). Our compiler shows every such rewrite misses by 30-64 lines. How should cc1psx count for the part it can't reproduce?"
Owner chose: **"cc1psx only where it can (Recommended)"** — option text: "cc1psx must agree on every part of the proof where it reproduces the already-matching form; for parts it can't reproduce even there (like this store structure), our compiler's dumps and measurements decide. Record that as a narrow clarification, then land func_8001CE60 per-file after a fresh layer-2."
Other options offered: "Keep the full requirement" — "No landing without cc1psx backing every part. func_8001CE60 stays unlanded (INCLUDE_ASM) and the search continues for a one-declaration spelling of func_800340A0 in plain C."; "Try one declaration first" — "Before deciding, spend more effort looking for a plain-C one-declaration spelling of func_800340A0 (winner-index style). If found, land that instead with no per-file exception; if not, come back to this question."
Author's note (2026-09-27, after the layer-2 rule-text review; the transcript above is unchanged): the Q24 question's statement that cc1psx "can't judge that half" overstated it. cc1psx reproduces the round-result store structure of the committed scalar func_800340A0 except one arm, the P2 tie-break arm (memory/grind/func_8001CE60/probes/calib_800340A0/A340_0.cc1psx.diff hunk 72,81c72,77). The owner's chosen option is worded by location ("where it reproduces"); the rule text applies it instruction by instruction.
