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
