# evidence — func_80032314 (src/code6cac_b.c:2760)

## Floor history
| session | modality | floor (sandbox --disable all) | note |
|---|---|---|---|
| — (HEAD) | — | 27 | 106 build insns vs 109 target |
| 1 | recon | **15** | 109/109 insns; residual is a pure register rotation |

## What the function is
Leaf, no calls, 8-byte frame. Walks a 4-element array of 0x2C-byte records at
`D_80104E88`. For each active record (`*t0 != 0`) it selects one of two
0x44C-byte entities in `D_80101EC8` (index = `(rec[3] == 0)`), skips the entity
if its state halfword at `+0x6A` is 4 / 0x14 / 0xF / 0x11 / in [0x1C,0x1D] /
[0x1E,0x1F] / [0x20,0x21], then computes the squared distance between the
entity position (`ent+0xF4/0xF8/0xFC`) and the record position
(`rec+4/8/0xC`). It converts that squared distance to a log2-ish magnitude via
the 256-byte table `D_8008D118` (direct index when `dist_sq < 0x400`, otherwise
a GTE-LZC-normalised index), and if the magnitude is below
`(rec[2]*30)*... = ((v1<<4)-v1)<<1 + 0x1F4`, sets `*(s16*)(ent+0x286) = 5` and
clears `*t0`.

`canonical func_80032314` = **ASM-PARTIAL**, 2/109 insns canonical
(`mtc2 $t4,$30`, `swc2 $31,0($t4)` — GTE LZCS/LZCR, no C form). The rest is
ordinary C. So the end state for this function is COMPLETED-INLINE-ASM-CANONICAL
with a single authorized GTE island, exactly like its sibling (below), not a
whole-body asm.

## FINDING 1 (the session-1 win, 27 → 15): the matched sibling in the SAME FILE
`func_800274BC` (src/code6cac_b.c:279-318) is an already-matched function that
computes the SAME table lookup with the SAME GTE LZC island, and its inline-asm
block was **user-authorized 2026-06-10** (comment at src/code6cac_b.c:287-291:
"Hand-written GTE leading-zero-count block (LZCS in, LZCR out) — canonical
inline asm, user-authorized 2026-06-10. Sibling of func_8001A67C (code6cac.c)").
There is a third instance at src/code6cac_b.c:751 ("Canonical GTE LZCS island
(mtc2/swc2 — no C form)").

Copying that sibling's block VERBATIM plus its tail-arithmetic spelling into
func_80032314 dropped the floor 27 → 15 and brought build_insns from 106 to
exactly 109 = target_insns.

The sibling's authorized block (reproduce it, do not re-derive it):

    __asm__ volatile(
        "addu   $t4, %1, $zero\n"
        "mtc2   $t4, $30\n"        /* LZCS <- dist_sq */
        "nop\n"
        "nop\n"
        "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
        "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
        : "=m"(sp_tmp)
        : "r"(dist_sq)
        : "$12");

and its tail spelling:

    u32 v0_m = (u32)-2; u32 v1_m; u32 idx; u32 hi;
    v0_m &= clz;  v1_m = 0x16 - v0_m;  idx = dist_sq >> v1_m;
    v1_m = v1_m >> 1;
    hi = (u32)((u8)(*((&D_8008D118) + idx)));
    log2_val = (hi << 16) >> (0x13 - v1_m);

## FINDING 2 (why HEAD's spelling was 12 points worse — an ENGINE CLASSIFIER GAP)
HEAD spelled the same island as two `.word` blocks plus a `$t4` register pin and
two bare `__asm__("nop")` statements. The cheat-invisible sandbox keeps only
blocks that `tools/classify_inline_asm.py` calls canonical, and its `.word`
test is

    CANONICAL_DOTWORD_RE = re.compile(r'\.word\s*0x4[89A-Fa-f]', re.IGNORECASE)
    # tools/classify_inline_asm.py:62

`.word 0x488CF000` (mtc2) passes; **`.word 0xE99F0000` (swc2 $31,0($t4)) does
NOT** — swc2's opcode byte is 0xE9, outside the 0x48-0x4F window — so the LZCR
STORE was stripped from every scored build, taking the result dataflow with it.
The two `__asm__("nop")` blocks were stripped too (nop is in CHEAT_ASM_OPS), and
the `$t4` pin was stripped, deleting both feeder `addu` moves. Net: 3 missing
instructions plus garbage `clz`.

The MNEMONIC spelling avoids the gap entirely without touching engine/: `mtc2`
and `swc2` are both in `CANONICAL_ASM_OPS` (tools/classify_inline_asm.py:56),
and a block containing any canonical instruction is kept WHOLE — which is why
the `addu $t4,...` feeder moves and the two `nop`s survive inside the same
block. This is the documented "canonical GTE sequences include their feeder
loads" behaviour (engine/inlineasm.py docstring, line 8).

**Rule of thumb for any future BB2 GTE island: write real mnemonics, never
`.word`, unless the opcode byte is in 0x48-0x4F.**

## FINDING 3: the entire remaining residual (15) is ONE 3-cycle register rotation
With the session-1 form, build and target are instruction-for-instruction
identical in opcode, operand shape, order, delay-slot fill, and branch
structure. `objdump` side-by-side (tmp/grind/func_80032314/s1/target.txt vs
build15.txt) differs ONLY in which of $a1/$a2/$a3 three pseudos land in:

| value                              | target | our build | #insns |
|---|---|---|---|
| record walker `a3 = &D_80104E88+2` | `$a3`  | `$a1`     | 7 |
| entity pointer `ent`               | `$a2`  | `$a3`     | 6 |
| first `mflo` (dx*dx)               | `$a1`  | `$a2`     | 2 |

7 + 6 + 2 = 15 = the score exactly. It is a 3-cycle: walker a1→a3, ent a3→a2,
mflo1 a2→a1. Everything else — including `$t0`/`$t1` for the base pointer and
the loop counter, and all of `$v0`/`$v1`/`$a0` — already matches target.

RTL evidence (`.greg` dump, tmp/grind/func_80032314/s1/base.i.greg, dispositions
block for func_80032314):

    72 in 8   (t0 base pointer)     -> matches target
    73 in 9   (t1 loop counter)     -> matches target
    74 in 5   (walker)   want 7 (a3)
    75 in 7   (ent)      want 6 (a2)
    115 in 6  (mflo1)    want 5 (a1)
    ;; Hard regs used: 2 3 4 5 6 7 8 9 10 12

`tools/gcc-2.7.2/config/mips/mips.h` defines **no** `REG_ALLOC_ORDER`, so
regclass/global use the DEFAULT ascending hard-register order
(`$zero,$at,$v0,$v1,$a0,$a1,$a2,$a3,$t0,...`). Under that order the two
loop-carried pseudos (72, 73) nevertheless landed in `$t0`/`$t1`, which means
the low registers were consumed by locally-allocated (single-block) pseudos
first — consistent with `local_alloc` running before `global_alloc`. The
causal chain for the rotation is therefore believed to be: **the walker takes
$a1 first, which pushes mflo1 to $a2 and ent to $a3.** Making the walker take
$a3 (i.e. allocating it later / at lower priority) should cascade the other two
into place in one step. NOT YET PROVEN — that is the session-2 probe.

## Rules dropped by the sandbox
12 regfix `subst` rules (regfix.txt:473-484). All 12 are pure register renames
in exactly the two regions above: lines 473-475 rewrite the mult/mflo cluster
($4/$2/$3), lines 476-484 rewrite the LZC tail ($2/$3/$4). They are paperwork
for the same rotation the honest floor now exposes; no rule addresses anything
structural.

- [s1] Honest floor at session start: 27 (build_insns 106 vs target_insns 109, 12 regfix rules dropped). Honest floor at session end: 15 with build_insns == target_insns == 109.

- [s1] `canonical func_80032314` = ASM-PARTIAL, 2/109 insns canonical (mtc2 $t4,$30 at target idx 73 and swc2 $31,0($t4) at idx 75 — GTE LZCS/LZCR, no C form). The end state for this function is COMPLETED-INLINE-ASM-CANONICAL with a single authorized GTE island, exactly like its siblings — not a whole-body asm and not pure C.

- [s1] func_800274BC (src/code6cac_b.c:279-318) is an ALREADY-MATCHED function in the SAME FILE that performs the identical squared-distance -> D_8008D118 log2 table lookup, and its GTE-LZC inline-asm block is annotated 'canonical inline asm, user-authorized 2026-06-10. Sibling of func_8001A67C (code6cac.c)'. A third instance sits at src/code6cac_b.c:751. Copying that block plus its tail-arithmetic spelling verbatim is what produced the 27 -> 15 drop.

- [s1] ENGINE CLASSIFIER GAP (generalises beyond this function): the cheat-invisible sandbox keeps a `.word`-spelled cop2 instruction only when the opcode byte is 0x48-0x4F (tools/classify_inline_asm.py:62). swc2 (0xE8-0xEB) and lwc2 (0xC8-0xCB) words are therefore classified CHEAT and stripped, silently deleting real canonical GTE stores/loads from every scored build. The MNEMONIC spelling is immune because `mtc2`/`swc2`/`lwc2` are all in CANONICAL_ASM_OPS (tools/classify_inline_asm.py:56) and a block containing any canonical instruction is kept WHOLE (engine/inlineasm.py docstring line 8: 'canonical GTE sequences include their feeder loads'). Rule of thumb for all future BB2 GTE islands: write real mnemonics, never `.word`, unless the opcode byte is in 0x48-0x4F. No engine/ file was touched.

- [s1] The entire remaining residual of 15 is ONE 3-cycle register rotation, verified instruction-by-instruction from a side-by-side objdump (tmp/grind/func_80032314/s1/target.txt vs build15.txt): record walker `&D_80104E88+2` is $a3 in target and $a1 in our build (7 insns); entity pointer `ent` is $a2 in target and $a3 in our build (6 insns); the first mflo (dx*dx) is $a1 in target and $a2 in our build (2 insns). 7+6+2 = 15 = the score exactly. Opcodes, operand shapes, instruction order, delay-slot fills and branch structure are otherwise IDENTICAL, including $t0/$t1 for the base pointer and loop counter and all of $v0/$v1/$a0.

- [s1] RTL confirmation from a cc1 -da dump of the cheat-stripped sandbox source (tmp/grind/func_80032314/s1/base.i.greg, dispositions block for func_80032314): pseudo 72 in 8 (t0, matches target), 73 in 9 (t1, matches target), 74 in 5 (walker, wants 7), 75 in 7 (ent, wants 6), 115 in 6 (mflo1, wants 5); 'Hard regs used: 2 3 4 5 6 7 8 9 10 12'.

- [s1] tools/gcc-2.7.2/config/mips/mips.h defines NO REG_ALLOC_ORDER, so allocation uses the default ascending hard-register order ($zero,$at,$v0,$v1,$a0,$a1,$a2,$a3,$t0,...). Yet the two loop-carried pseudos landed in $t0/$t1, which means the low registers were consumed by locally-allocated single-block pseudos first (local_alloc runs before global_alloc). The working causal model for the rotation is: the walker claims $a1 first, which pushes mflo1 to $a2 and ent to $a3.

- [s1] All 12 dropped regfix rules (regfix.txt:473-484) are pure register-rename `subst` rules covering exactly the two affected regions (473-475 the mult/mflo cluster, 476-484 the LZC tail). They are paperwork for the same rotation the honest floor now exposes; none addresses anything structural.

- [s1] src/code6cac_b.c was restored to HEAD before the session ended (git checkout); the working tree carries no source edits from this session.

- [s1] The register-asm pins in the HEAD body ($t0/$t1/$a3/$a0/$t4) are cheats and are stripped by the sandbox, so they contribute nothing to the floor of 15 — measured identical with and without them. They must be deleted before any candidate-ready submission.

## SESSION 2 (structural) — floor 15 -> 1

- [s2] Floor at session start re-measured 15 (pin-free candidate.c applied; 109/109 insns) — chassis unchanged from s1. Floor at session end: **1**, 109/109 insns, zero rules dropped, no pins.

- [s2] THE ALLOCNO PRIORITY FORMULA, read from the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1; hook at global.c:605 prints the sorted allocno_order): `pri = floor_log2(n_refs) * n_refs / live_length * 10000 * size`. Measured ranking for the s1 form (tmp/grind/func_80032314/s2/allocdbg.txt): walker p74 (8 refs, len 82) pri 2926 -> $a1; mflo1 p115 (2 refs, len 10) pri 2000 -> $a2; ent p75 (6 refs, len 63) pri 1904 -> $a3; t0 p72 (6 refs, len 84) pri 1428 -> $t0; t1 p73 pri 963 -> $t1. Allocation = descending pri, find_reg takes the lowest free non-conflicting hard reg (no REG_ALLOC_ORDER in mips.h).

- [s2] KEY ARITHMETIC INSIGHT: mflo1 (2000) ALREADY outranks ent (1904) — the target order (mflo1, ent, walker) requires ONLY dropping the walker below 1904 while staying above t0. n_refs 8 -> 7 crosses the floor_log2 boundary (3 -> 2): pri 2*7/82*10000 = 1707, inside the band (t0 rises to 7 refs = 1666 — margin only 41; any spelling that adds a t0 ref can flip it back).

- [s2] THE 15 -> 1 EDIT (measured, candidate.c): the radius byte read in the final compare block is spelled `*(u8 *)(t0 + 2)` instead of `*a3`. Same value (a3 == t0+2 is loop-invariant but invisible to cse across the back-edge, so no re-canonicalization). This removes exactly one walker ref (8 -> 7). Verified in the re-run ALLOCDBG trace: p115 -> hardreg 5 ($a1), p75 -> 6 ($a2), p74 (7 refs, pri 1707) -> 7 ($a3), p72 (7 refs, 1666) -> 8. Every register in all 109 instructions now matches target (tmp/grind/func_80032314/s2/build1.txt vs s1/target.txt); the SINGLE remaining diff is the edited insn itself: build `lbu v1,2(t0)` vs target `lbu $v1,0x0($a3)`. Score 1.

- [s2] PURE-C IMPOSSIBILITY ARGUMENT for the last instruction (why score 0 is arithmetically closed to ordinary spelling): a byte-matching build requires 8 walker appearances ($a3: init set + 5 loads + latch read/write). flow.c counts refs on pre-combine RTL; combine only merges (never splits), and its reg_n_refs bookkeeping can leave counts stale-HIGH, never below the final appearance count. So any byte-matching compile has walker n_refs >= 8 -> pri >= 2926. ent's ceiling is byte-pinned (6 appearances, live length 63 fixed by the pinned set/store positions -> 1904 < 2926); mflo1 likewise (2 refs, len 10 -> 2000 < 2926). Therefore in every byte-matching pure-C compile the walker is allocated first and the 3-cycle rotation is FORCED. Splitting the walker into two $a3 pseudos needs a connecting copy which GCC 2.7.2 does not delete post-allocation (delete_noop_moves runs only in jump1, pre-alloc) -> emitted `move $a3,$a3` -> byte FAIL.

- [s2] THE SANCTIONED ENDGAME (F1 family, .claude/rules/dead-store-fake-exception.md:32 "combine-foldable chain-extender", owner ruling 2026-07-01): stale reg_n_refs bumps on BOTH ent and mflo1 invert the ranking without touching bytes. Exact arithmetic (verified against find_reg conflict structure): ent 6 -> 8 stale refs = pri 3809; mflo1 2 -> 4 stale refs = pri 8000; order becomes mflo1(8000) > ent(3809) > ord5/6 locals (3333) > walker(2926) -> assignment 5, 6, ..., 7 = target. +1 is NOT enough for ent (7 refs = 2222 < 2926 — floor_log2 stays 2). mflo1 chain must use link-time-opaque constants (e.g. +(s32)&SYM ... -(s32)&SYM) because tree-level fold-const eliminates +C-C detours before RTL. Byte-neutrality (insn count 109, no new address materialization) must be verified per the rule's extra prerequisite, and the construct requires /* FAKE */ + documented ladder exhaustion — NOT available at session 2; this is the banked endgame for the driver's exhaustion phase.

- [s2] flow.c weights reg_n_refs by loop_depth, but loop_depth comes only from NOTE_INSN_LOOP notes — the goto-form loop has none, so all refs count 1x. Spelling the loop as do-while (to double inside-loop refs into the target order) was measured FATAL: score 59, 122 insns — loop.c LICM hoisted ~6 per-iteration constants into s-regs and built a giv. Banked as rejected/do-while-loop-notes-licm-explosion.c, including why the second-entry (`goto` into the body) loop-invalidation trick cannot survive jump1.

- [s2] Correction to the s1 frontier: F2's mechanism claim ("shorter live range lowers priority") is INVERTED — priority DIVIDES by live_length, so shortening a range RAISES priority. F2 was arithmetically dead as specced. F3-as-specced (single duplicated ent statement) is also dead: +1 ref = 2222 < 2926.

- [s2] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) before session end; the working tree carries no source edits. The score-1 form lives in candidate.c.

- [s2] Allocno priority formula and full ranking measured via BB2_ALLOC_DEBUG instrumented cc1 (tools/gcc-2.7.2/cc1): s1-form walker(8,82)=2926 > mflo1(2,10)=2000 > ent(6,63)=1904 > t0=1428; score-1 form walker(7,82)=1707 between ent 1904 and t0 1666 (margin 41 — spellings adding a t0 ref can flip it).

- [s2] Score-1 form: sandbox --disable all = 1, 109/109 insns, no pins, no rules; only diff lbu v1,2(t0) vs target lbu $v1,0x0($a3). Saved as candidate.c; artifacts tmp/grind/func_80032314/s2/{allocdbg.txt,trace_func.txt,build1.txt}.

- [s2] Pure-C impossibility for the last insn: byte-match forces 8 walker appearances; flow-time n_refs never undercounts final appearances (combine only merges, bookkeeping stale-high only) -> walker pri >= 2926 in every byte-matching compile, while ent/mflo1 ceilings are byte-pinned at 1904/2000 -> rotation forced. Walker split across two $a3 pseudos needs a copy insn GCC 2.7.2 never deletes post-alloc (delete_noop_moves is jump1-only) -> byte FAIL.

- [s2] Sanctioned endgame specified exactly (F4): chain-extender (dead-store-fake-exception.md:32, owner ruling 2026-07-01) stale-ref bumps on BOTH mflo1 (2->4, pri 8000) and ent (6->8, pri 3809) invert the order around the byte-forced walker (2926); +1 on ent alone is insufficient (2222). Detours must use link-time-opaque &SYM constants (tree fold-const kills +C-C). Requires /* FAKE */ + full-ladder exhaustion — gate not open at session 2, so banked, not attempted as a candidate.

- [s2] src/code6cac_b.c restored to HEAD; score-1 form lives in memory/grind/func_80032314/candidate.c.

## SESSION 3 (structural) — F4 endgame MEASURED AT SCORE 0; pure-C closure completed

- [s3] Chassis re-confirmed at session start: candidate.c floor-1 form applied to src measured sandbox --disable all = 1, 109/109 insns, 0 rules — identical to the s2 exit state.

- [s3] F6 PREMISE-2 HOLE FOUND AND CLOSED BY MEASUREMENT. final.c:1796-1806 skips emitting any SET whose source and destination are the same hard register (unless the insn is in a delay slot / final_sequence) — so a post-allocation identity move IS deletable, contradicting the s2 argument's "emitted move $a3,$a3 -> byte FAIL" premise as stated. The escape is nevertheless UNREACHABLE from C, measured via the split-walker form (carrier a3w init+latch / cursor a3 loop-top copy + body loads): score 16, 109 insns. Three independent mechanisms kill it: (1) cse copy-propagates the loop-top `a3 = a3w` into every same-extended-BB use, collapsing the cursor to 2 refs (radius read + latch, allocated $t2) while the body loads emit via a3w($a1); (2) the surviving `move t2,a1` was captured by reorg into the beqz delay slot, where final.c's elision explicitly does not apply; (3) global.c records copy preferences only toward HARD regs (set_preference, global.c:1671-1750; allocno-to-allocno merges at 857-866 only IOR existing hard-reg bits), so nothing steers the second pseudo of a pseudo-pseudo copy into $a3 — it takes lowest-free. Banked as rejected/split-walker-copy-cse-propagated.c. With this, BOTH premise-holes of the s2 impossibility argument are closed by measurement or source-read: pure-C (no-FAKE) score 0 does not exist for this function.

- [s3] F6 PREMISE-1 CONFIRMED AT SOURCE: combine.c:52-57 states verbatim "reg_n_refs is not adjusted in the rare case when a register is no longer required in a computation" — stale-HIGH only; the only downward adjustment is zeroing a fully-dead i2dest/i1dest (combine.c:2306-2337), which cannot apply to a pseudo that still appears in the final code. Flow-time refs of a surviving pseudo never undercount its final appearances.

- [s3] F4 CHAIN-EXTENDER MEASURED: SCORE 0, 109/109 insns, zero rules, no pins (sandbox --disable all = 0 with the form in src/). The winning spelling (candidate.c): name the first square `s32 dxs = dx * dx;`, keep dist_sq = dxs + dy*dy + dz*dz, and place the fold-away pair `ent = (u8*)((s32)ent + dxs); ent = (u8*)((s32)ent - dxs);` at the END of the `dist_sq < 0x400` then-arm. combine folds (ent+dxs)-dxs -> ent to a self-move elided by final.c:1800; the pair leaves stale refs ent 6->10 (pri 4615) and dxs 2->4 (len 14, pri 5714). ALLOCDBG (tmp/grind/func_80032314/s3/allocdbg.txt): order p115-tail(8571)->$a0, p76(6666)->$v1, p119-dys-temp(6666)->LO, p126(6000)->$v1, p114-dxs(5714)->$a1, p77(5555)->$a0, p75-ent(4615)->$a2, p116/p117(3333)->$a0, p74-walker(8 refs, 2857)->$a3, p72-t0(1395)->$t0, p73-t1(941)->$t1 — every register in all 109 insns matches target including lbu $v1,0x0($a3) (radius via *a3, walker at its byte-forced 8 refs). Improvement over the ledger's original F4 spec: routing the detour through live pseudos (ent as multi-set dest, dxs as operand) instead of +(s32)&SYM-(s32)&SYM avoids the dead lui/addiu materialization pair that a fresh &SYM constant would leave behind after combine folds the detour (nothing deletes dead insns post-combine).

- [s3] F4 PLACEMENT IS LOAD-BEARING TWICE (both mis-placements measured, score 23 each, banked as rejected/detour-multiset-dxs-lo-capture.c): (1) making dxs itself the multi-set detour dest (6 refs, pri 9230) or (2) placing the pair directly after the sum (len 11, pri 7272) both push dxs's priority ABOVE the dys mult-temp (6666) — dxs then allocates while LO is still free and find_reg's copy-preference pass (global.c:1097-1104; the mflo set gives the pseudo an LO copy-preference) hands it hardreg 65 (LO), cascading ent->$a1/walker->$a2/t0->$a3. The required window for the bumped mflo1 pseudo is pri in (4687, 6666): below the dys temp (which then claims LO first and blocks it for dxs via their range conflict) and above bumped-ent. Arm-end placement (len 14 -> 5714) hits it.

- [s3] SUBMISSION GATE (why this session returns progress, not candidate-ready): the pair is the sanctioned F1 chain-extender family (.claude/rules/dead-store-fake-exception.md:32, owner ruling 2026-07-01) whose prerequisites are (a) full modality ladder demonstrably spent per the DRIVER's ledger, (b) named GCC-pass mechanism, (c) FAKE annotation. (b) and (c) are ready (mechanism documented above; annotation template below). (a) is NOT satisfied at s3 (recon, structural x2) and exhaustion is the driver's call, not the session's. The submitting session (once the driver's gate opens) applies candidate.c, adds on each of the three F4 lines an annotation of the form
      /* FAKE: fold-away ent+dxs detour, mechanism: combine.c stale reg_n_refs (52-57) + global.c allocno priority, lever-exhaustion: memory/grind/func_80032314/hypotheses.md (s2 arithmetic closure + s3 premise-hole measurements) */
  writes self_vet.md citing dead-store-fake-exception.md:32 as PRECEDENT, and re-measures sandbox = 0 before the outcome.

- [s3] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) before session end; the score-0 form lives in candidate.c. Artifacts: tmp/grind/func_80032314/s3/{allocdbg.txt,build0.txt,greg.sh,diff0.sh,base.i.combine,base.i.greg,base.i.lreg,base.i.sched2}.

- [s3] Chassis re-confirmed at session start: floor-1 candidate applied to src measured sandbox --disable all = 1 (109/109, 0 rules) — unchanged from s2.

- [s3] F4 endgame measured sandbox --disable all = 0 THIS session with the form in src/ (subsequently reverted to INCLUDE_ASM per asm-until-matched); exact form saved to memory/grind/func_80032314/candidate.c with full mechanism header and the FAKE annotation template.

- [s3] Submission deliberately withheld: the construct is the F1 chain-extender FAKE family whose prong (a) requires the driver-verified full modality ladder, not satisfied at s3 (recon + structural x2). Reported floor stays 1 (the un-gated honest floor); the gated form measures 0.

- [s3] combine.c:52-57 and final.c:1796-1806 are the two load-bearing GCC-source facts banked this session; global.c:1671-1750 (hard-reg-only copy preferences) and the LO-capture window (4687,6666) for mflo-sourced pseudos generalize to other mult-cluster allocation walls.

- [s3] src/code6cac_b.c restored to HEAD; working tree carries no source edits.

## SESSION 4 (permuter) — second score-0 endgame found and SUBMITTED (candidate-ready)

- [s4] Chassis re-confirmed at session start: floor-1 no-FAKE form applied to src measured
  sandbox --disable all = 1 (109/109, 0 rules) — identical to the s2/s3 exit state. All
  banked spelling conclusions remained chassis-valid.

- [s4] BASIN A NULL (the mandated-modality measurement against the closed pure-C case):
  permuter campaign s4_floor1_random on the floor-1 natural-geometry chassis (radius read
  *(u8*)(t0+2)), pure random, -j 8 --stop-on-zero, base_score 10. Result: **32,797
  iterations / 27 minutes / ZERO finds of any score** (not even a base-score re-find).
  Harvested + stopped at the fresh-seed window. This is the permuter-modality null the s3
  frontier predicted: the pure-C (no-FAKE) closure remains closed — the s2 arithmetic
  argument and s3 premise-hole measurements now additionally carry a 32.8k-iteration
  random-search null on the natural chassis.

- [s4] BASIN B FIND: campaign s4_a3rotation_random on the *a3 rotation chassis (score-15
  form, base_score 80 permuter-metric), pure random, --stop-on-zero. At iteration ~3,284
  (160 s after launch) the permuter found a **SCORE-0 form**: the rotation chassis with
  everything after the `a0 == 4` check wrapped in a single-level `do { ... } while (0);`
  (tmp/grind/func_80032314/s4/permB/output-0-1/source.c). Verified honestly:
  **sandbox --disable all = 0, 109/109 insns, 0 rules, no pins**, with the form (plus FAKE
  annotation) applied to src/code6cac_b.c — this is the state at session end (submission).

- [s4] MECHANISM (ALLOCDBG, tmp/grind/func_80032314/s4/allocdbg.txt): the wrap emits
  NOTE_INSN_LOOP notes; flow.c weights reg_n_refs by loop_depth for refs inside them.
  Weighted counts: walker p74 nrefs 12 / len 82 -> pri 4390; ent p75 nrefs 10 / len 63 ->
  pri 4761; mult-temp p116 nrefs 4 / len 10 -> pri 8000. Allocation order mult-temp > ent
  > walker seats them hardreg 5/6/7 = $a1/$a2/$a3 — every register in all 109 insns
  matches target (the same end allocation the s3 F4 chain-extender produced by
  combine-stale refs). t0 p72 (weighted 7, pri 1666) -> $t0, t1 p73 -> $t1 unchanged.

- [s4] WHY THIS DOES NOT CONTRADICT THE s2 do-while KILL: the s2 form spelled the OUTER
  4-iteration loop as do-while, arming loop.c LICM/giv on a real back-edge (score 59, 122
  insns). The s4 wrap encloses a straight-line once-through region with no back-edge work:
  loop.c finds nothing to hoist, emission is byte-identical, and the ONLY surviving effect
  is the flow.c ref weighting. The s2 kill stands for real-loop respelling; it never
  covered an inner once-through wrap.

- [s4] POLICY BASIS FOR SUBMITTING (vs. the withheld F4): the construct is the do-while(0)
  family, .claude/rules/do-while-zero-exception.md — owner ruling 2026-07-06 FINAL, whose
  sanction sentence reads: "do { <any body> } while (0); — including empty bodies — is a
  sanctioned pure-C match device for ANY codegen effect, including register allocation."
  The former reorg.c-only scoping is explicitly abolished by that rule (its lines 26-29),
  and memory/feedback/do-while0-ra-weighting-not-sanctioned.md records the supersession.
  Prerequisites for SINGLE-LEVEL wraps: inline FAKE annotation naming the observed effect
  (present, at the construct site in src) — exhaustion is NOT a hard gate for single-level
  wraps (rule prereq 2), though this ledger documents it anyway (s2 arithmetic closure, s3
  premise-hole measurements, s4 basin-A null). Confirmed RA-purposed application:
  marionation_Exec (rule file "Confirmed applications", line 99). Judge precedent verified:
  the 2026-07-13 func_8001C624 FAIL (docs/grind/decisions.md:54-70) failed a NESTED pair
  on an unverifiable exhaustion record while explicitly listing do-while(0) among the
  frozen-list families; this submission is single-level with a fully verifiable ledger.

- [s4] The F4 chain-extender endgame (s3) remains valid as the ALTERNATIVE score-0 form
  (spec in [s3]); it stays gated on dead-store-fake-exception prong (a) (driver's
  exhaustion call). The do-while wrap supersedes it as the submission because its family
  carries no such gate for single-level use.

- [s4] Permuter telemetry banked in metrics/events.jsonl (permuter-launch/harvest x2).
  Artifacts: tmp/grind/func_80032314/s4/{allocdbg.txt,base.i.greg,base.i.flow,greg.sh,
  perm/ (basin A workspace, 0 finds),permB/ (basin B workspace, output-0-1 = the find)}.

- [s4] Session end state: src/code6cac_b.c carries the annotated score-0 form (NOT
  restored to HEAD — candidate-ready submission); self-vet at
  memory/grind/func_80032314/self_vet.md.

- [s4-resubmit 2026-08-20] The previous session's candidate-ready was DISCARDED by the
  driver validator on self_vet.md FORMAT only: the two SCOPE quotes were wrapped across
  multiple lines, and grindlib.py's _SCOPE_LINE regex (tools/grinder/grindlib.py:57)
  requires the full quoted sentence on the single SCOPE: line, so it counted 0 verbatim
  quotes. No technical objection was raised. This session re-applied candidate.c verbatim
  to src/code6cac_b.c, RE-MEASURED sandbox --disable all = 0 (109/109, 0 rules,
  cheat_asm_stripped inert), and rewrote self_vet.md with each SCOPE sentence as a
  single-line verbatim quote plus file:line precedents
  (.claude/rules/do-while-zero-exception.md:23,
  .claude/rules/cop2-addressing-preamble-cluster.md:86) — verified against the validator
  source directly. Content of the vet is unchanged from the discarded session.
