# func_800620B8 — evidence (manual lane, slotC, 2026-09-26)

## Status (2026-09-26) — LANDED COMPLETED-INLINE-ASM-CANONICAL; ledger closed
auth 1fa01e822 (gte_stsz island row), Match 96db91246, queue 99c3a6c12. Layer-2 PASS on the
session-3c body (candidate.c = cand_split1: 4 FAKE pointer aliases + ONE FAKE chain-extender on the
sel_a address + integer frame addresses `index * sizeof(*table) + (s32)table`). Oracle SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa; sandbox --disable all 0/501; check_completion_integrity OK.
Reviewer non-blocking notes (recorded as asked): (1) the h2-noX16-1 permuter base (m_no_x16) was not
FAKE-clean -- it carried the strip32 pointer-form extender; (2) no permuter campaign ran on the final
integer-address chassis without the extender (q2, sandbox 35).

## Session 3c (2026-09-26, slotA5) — layer-2 FAIL on the 3b split landing; permuter + extender scope
Layer-2 FAILED the 3b split landing (cand_split0: 4 aliases + 2 extenders) on: (1) no permuter from a
clean base (dead-store-fake-exception prereq 1); (2) the strip16 extender is outside the
chain-extender family — its effect is operand order, not reg_n_refs. Strip32 + aliases "likely
reviewable".
- Dumps (tmp/func_800620B8/s3/d/x0,x1,x2,xq1,xq2; -df -dc added): CONFIRMED the reviewer on (2):
  strip16's pseudo 94 is unallocated with or without its extender (nrefs 3 -> 7 changes nothing);
  the only effect is combine rebuilding the add as (plus idx p). Worse, the cand_split0 strip32
  extender ALSO reordered: direct `(s32)strip32[n]` is (plus p idx) in combine.fn, extended is
  (plus idx p). So both pointer-form extenders steered order.
- Fix: spell both frame selections as `index * sizeof(*table) + (s32)table` (ordinary C integer
  address arithmetic; D_800A3488/348C are s32 words). Direct form combine.fn: (plus idx p) already
  (xq2), so the sel_a extender `... + (s32)strip32 - (s32)strip32 + (s32)strip32` now folds back to
  exactly the direct RTL (xq1 combine.fn insn 249 == xq2 insn 245 shape) and its only effect is
  flow's count: strip32 nrefs 3 -> 7, pri 45 -> 212, -> $fp. Scores: q2 direct 35, q1 extended 0,
  parenthesised detour q3 35 (tree-folded, no refs), q4 reordered detour 0. sel_b needs no extender.
- cand_split1 (candidate.c) = 4 FAKE aliases + ONE FAKE chain-extender + integer frame addresses:
  0/501. Minimisation on this chassis: BA50 direct 3, BA58 direct 7, BA30 direct 3, all three 12.
- Permuter (tools/permuter_campaign.py, -j1 each, --stack-diffs, fresh seeds, workspaces
  tmp/func_800620B8/perm/, builder prong_a/extender/mkws.py):
  * split-direct-47 (FAKE-free split base s3/split.c): 6948 iters / 28.8 min, 35 finds all within
    ~3 min, best perm 260 = sandbox 23 (a rediscovered loop-top BA00 alias `new_var`); others 23-30.
  * h2-noX16-1 (m_no_x16, the 1-insn operand residual): 6955 iters / 28.8 min, 0 finds.
  * f1-aliases-39 (four function-scope aliases, no extenders): 7221 iters / 28.9 min, best perm 355
    = sandbox 14.
  * h2-aliases-looptop (h2_bare: aliases, strip32 at loop top, no extenders): 7221 iters / 28.9 min,
    best perm 70 = sandbox 8 (a do { sel_a tail } while (0) wrap); next 230/280 = 10.
  No campaign reached 0 from any FAKE-extender-free base; ~28.3k iterations total.

## Session 3b (2026-09-26, slotA5) — layer-2 FAIL on (a1); SPLIT SPELLING REACHES 0 -> merge defeated
Layer-2 (orchestrator relay) FAILED the TexRec[12] merge landing on ONE (a1) gap, accepting
everything else ((a2), (a3), (a4), (b)-(e), TexRec move, comment fixes, D_8009BD44[] array, region
hash, island/grant/goto tails/SetTransMatrix/rot/tag link): the target only needs the BA00 pseudo to
reach nrefs >= 4 with one in-loop use, and the sanctioned combine-foldable chain-extender
(dead-store-fake-exception.md:51-65) adds reg_n_refs flow.c counts and combine folds to zero bytes —
untested. Tested it: **it closes the split spelling.** The session-3 "no separate-object spelling"
argument was wrong: it assumed every extra reference would survive as a `$fp` use in the bytes.
Measurements (sandbox --disable all, tmp/func_800620B8/s4, generators in prong_a/extender/):
- f1 chassis (four function-scope table pointers, 39) + extender on the BA00 use:
  `x - p + p` 7, `p + (x - p)` 11, u8 round trip 11, `x + p - p` 11, index `+1 -1` 51,
  self-assign 39 (no insn, no refs), pre-loop `+1 / -1` 11.
- a4 chassis (alias at loop top, 50) + the same: 12 / 16 / 50 / 16 / 62 / 16.
- f1 + E1 residual = BA00 set placed before the entry test -> move ONLY strip32's set to the loop
  top (loop.c hoists it into the pre-header after the entry test): 1 (h2), residual = operand order
  `addu $v0,$t0,$v0` vs target `addu $v0,$v0,$t0` in sel_b; spelling-order probes (`strip16 + n`,
  `n + strip16`) 1; the same round trip on the strip16 use: **0/501** (h2_ba00_top_r1).
- Minimisation (every construct load-bearing): BA50 direct 3, BA58 direct 7, both direct 10,
  BA30 direct 3, no strip16 extender 1, no strip32 extender 15.
- Dump (tmp/func_800620B8/s3/d/x0, prong_a/extender/h2_r1_alloc.txt): strip32 pseudo 92 nrefs 7,
  live 660, pri 212 -> $fp; sv/flag pri 88 -> spilled; the three other aliases unallocated ->
  reload rebuilds their constants in $t0 — the target's allocation, with separate symbols.
Consequence: (a1) is NOT met (its defeat clause: an admissible split spelling reaching the target's
codegen). The merge body is banked as rejected/table-merge-TexRec12-0-a1-defeated.c. candidate.c =
the split body with four FAKE-annotated pointer aliases (pointer-alias-fake-exception) and two
FAKE-annotated chain-extenders (dead-store-fake-exception combine-foldable chain-extender), 0/501.
Session-3 (a2)/(a3)/(a4) material below stays as a record; it no longer carries a landing.

## Session 3 (2026-09-26, slotA5) — prong (a) via the 2026-09-26 amendment (a1)-(a4)
Rule text: .claude/rules/no-new-park-categories.md "Amendment (owner ruling 2026-09-26) —
compiler-necessity evidence for prong (a)" (commit 262db111c; owner record
docs/grind/owner-rulings-2026-09-26.md Q2, "Accept, minimal span (Recommended)").
Artifacts (all regenerable): `prong_a/tools/` (gen.py makes the variants from the old split candidate;
dump.sh = instrumented project cc1 dumps; psx.sh / psx_obj.sh = cc1psx; cmp.py = normalised diff vs
target; the scripts run from tmp/func_800620B8/s3/), `prong_a/split/` and `prong_a/merged/`
(each: variant.c, cmd.txt = exact command lines, cse.fn / loop.fn / cse2.fn / lreg.fn / greg.fn =
the func_800620B8 section of each RTL dump, alloc.txt = BB2_ALLOC_DEBUG global-alloc order, f.s =
our cc1's asm, psx_f.s / psx_obj.txt = cc1psx asm and its assembled objdump, focus.txt = the
table-address instructions vs the target, psx_vs_target.diff = whole-function normalised diff).
The two variants differ ONLY in the table declaration and the four address expressions
(split: `TexRec D_8009BA00[6], D_8009BA30[4], D_8009BA50, D_8009BA58`; merged: `TexRec D_8009BA00[12]`
with `&D_8009BA00[10]`, `[(n & 3) + 6]`, `[11]`). Sandbox --disable all: split 47, merged 0.

**(a1) Necessity, from the dumps.** Target: `lui/addiu $fp, D_8009BA00` in the loop pre-header
(target insns 72-73), ONE in-loop use `addu $a0,$a0,$fp` (0x800622AC); BA50/BA30/BA58 each rebuilt
`lui/addiu $t0` at the use (0x800622C4, 0x80062328, 0x8006235C); sv (base+0x34) and flag (base+0x3C)
spilled. The chain that produces this, with source locations in tools/gcc-2.7.2:
1. cse (merged/cse.fn insn 252, insn 344): the alternate-record address `(const (plus
   (symbol_ref D_8009BA00) 80))` is rewritten as `(plus (reg 142) 80)` where reg 142 holds
   D_8009BA00 — cse.c:6531-6535 calls use_related_value (cse.c:1781), which can only find a
   register through get_related_value (rtlanal.c:211-224): it returns the base of a
   `(const (plus SYM N))` and **0 for a bare symbol_ref** (rtlanal.c:214). split/cse.fn insn 252 /
   344: `(symbol_ref D_8009BA50)` / `(symbol_ref D_8009BA58)` — no relation is possible.
2. loop (merged/loop.fn lines 15-22): `Insn 232: regno 142 (life 10), move-insn savings 2 moved`,
   insn 252 (BA00+80) `forces 232 ... moved`, same for 324/344 (BA00+48 / +88). split/loop.fn
   lines 15-22: every table load `(life 1), move-insn savings 1 not desirable`. The decision is
   loop.c:1631 `threshold * savings * m->lifetime >= insn_count` with m->lifetime / m->savings =
   n_times_used from loop.c:791-793: the related use in insn 252 is what gives reg 142 a second use
   and a 10-insn life. With separate symbols reg 142 has one use and dies in its block.
3. cse2 (merged/cse2.fn insns 1064-1070): in the pre-header the hoisted sets become
   `142 = D_8009BA00; 148 = 142+80; 169 = 142+48; 175 = 142+88`, each with a REG_EQUAL constant.
4. global alloc (merged/alloc.txt; priority = floor_log2(nrefs)*nrefs/live_length, global.c:642-648 allocno_compare; order printed by the BB2_ALLOC_DEBUG hook global.c:605-616):
   pseudo 142 nrefs 6 live 666 pri 180 -> hard reg 30 ($fp); 148/169/175 nrefs 3 pri 45 -> -1
   (unallocated; reload rematerialises their REG_EQUIV constants at each use = the target's
   `lui/addiu $t0` rebuilds); sv (79) / flag (80) pri 88 -> -1 (spilled, as in the target).
   split/alloc.txt: sv (79) pri 88 -> $fp, flag spilled; no table pseudo crosses the loop (47).
Why no separate-object spelling can produce it: the target's `$fp` = D_8009BA00 has exactly one
in-loop use, so its set (1) + that use (weight 2) give nrefs 3, pri ~45 < sv/flag's 88 — it loses
`$fp`. It wins only with more references that do NOT appear as `$fp` uses in the bytes, i.e. the
pre-header `(plus reg N)` sets whose pseudos are then rematerialised as constants (step 4). Those
sets exist only if cse relates the other addresses to D_8009BA00's register, which rtlanal.c:214
refuses for any address that is not `D_8009BA00 + N` — i.e. unless the addresses are offsets of ONE
object. Any extra in-loop use would be a second `$fp` use the target does not have (model check,
s2 m1_modelcheck_pun: nrefs 5 -> $fp, but 17 and a pun). This covers every split spelling, not only
the measured ones. Split spellings resting on refused/banned constructs, set aside per (a1):
`D_8009BA00 + 6`/`[10]` through a [6]-declared symbol (F4 cross-symbol arithmetic,
no-new-park-categories.md:587), and per-use pointer puns.
Admissible split spellings measured (re-measured 2026-09-26 on current main, sandbox --disable all;
none reaches 0, so (a1)'s defeat clause does not fire): plain split 47 (split_tex / split.c);
FAKE-style pointer aliases under [[pointer-alias-fake-exception]]: alias set in the sel_a tail 47
(s2/a1.c), function-scope alias before the loop 50 (s2/a3.c), alias at the loop top 50 (s2/a4.c,
hoisted life 47 but pri 45 < 88), per-tail aliases for both frame tables 47 (s2/a5.c); structural
respellings: four function-scope table pointers 39 (s/f1.c..f6.c, all four unallocated, sv takes
`$fp`), per-tail table-pointer locals 55 / 47 / 55 (s/l1_locals4.c, l2_frames_only.c, l3_alt_first.c);
duplicated switch tails 49; 11 tail-order permutations 52-117 (sw/).

**(a2) Original compiler agrees** (calibration only: tools/cc1psx_wrapper.sh, PsyQ GCC 2.7.2.SN.1, on
the SAME t.i each variant built from, with the build's cc1 flags `-O2 -G0 -funsigned-char -quiet
-mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float`; output run through the build's
prologue_fix | maspsx | align fix | multu_pad | as and objdumped; normalised diff by cmp.py).
merged/focus.txt: `lui $fp,%hi / addiu $fp,$fp,%lo(0x8009BA00)`, `addu $a0,$a0,$fp`, and
`lui/addiu $t0` for 0x8009BA50 / 0x8009BA30 / 0x8009BA58 — opcode, register and offset identical to
the target's nine table-address instructions; frame -80 as the target (cc1psx asm:
`la $fp,D_8009BA00` ... `la $8,D_8009BA00+80`, `+48`, `+88`).
split/focus.txt: `$fp` = s0+60 (flag), the four tables formed in $v0/$v1 at their uses, frame -72 —
not the target's shape. Whole-function normalised diffs are banked (merged 430/501 lines equal,
split 390/501; cc1psx is not our build compiler — the oracle decides the match).

**(a3) Minimal span.** func_800620B8 references exactly four labels (asm/funcs/func_800620B8.s
lines 74-75, 137-138, 166-167, 179-180): D_8009BA00 (6 records, indexed `% 6` * 8), D_8009BA30
(4 records, `& 3` * 8), D_8009BA50 (1 record), D_8009BA58 (1 record). Lowest label start 0x8009BA00;
highest label end 0x8009BA5F (D_8009BA58 is 8 bytes, enddlabel before D_8009BA60). Span =
0x8009BA00..0x8009BA5F = 0x60 bytes. Splat labels inside the span: exactly those four
(asm/data/7D920.data.s:23262-23304) — no unreferenced label inside, none outside merged. The
same-shape records before 0x8009BA00 (D_8009B9F0 etc.) and the u8 table D_8009BA60 (used by
func_80060A68 / func_80060B70 / text1b.c:3029) are NOT merged.

**(a4) One element shape.** Every 8-byte element of the span is {u16 clut_x, u16 clut_y, u16 u,
u16 v} (asm/data/7D920.data.s:23262-23304):
  [0..5]  (BA00..BA28) clut 0x3C0,0xFD  u 0x00,0x20,0x40,0x60,0x80,0xA0  v 0x94
  [6..9]  (BA30..BA48) clut 0x3F0,0xFC  u 0x00,0x10,0x20,0x30            v 0x80
  [10]    (BA50)       clut 0x3C0,0xFE  u 0x00 v 0x94 (record [0]'s u/v)
  [11]    (BA58)       clut 0x3F0,0xFD  u 0x30 v 0x80 (record [9]'s u/v)
The function's own accesses read the element as four halfwords at +0/+2/+4/+6 (`lhu 0x0/0x2($v1)`
through D_800A348C at 0x800624A0/A4 = clut x/y; `lhu 0x4/0x6` through D_800A3488 at 0x800622D0,
0x800622E8, 0x80062368, 0x80062380, 0x800624CC, 0x800624E0 = u/v) and index both strips with an
8-byte stride (`sll $a0,$a0,3` 0x800622A0, `sll $v0,$v0,3` 0x80062340). Records [10]/[11] are only
reached through D_800A348C (clut fields); their u/v words hold the same kind of values as the
others. Declaration: `TexRec D_8009BA00[12]` (0x60 / 8 = 12); TexRec is the existing text1b.c record
type (comment: PsyQ getClut(x, y) fields + u/v origin), moved to include/game.h with the table.

Prong (b): array of the record type, index = record number (`[n % 6]`, `[(n & 3) + 6]`, `[10]`,
`[11]`), no stride-as-magic-number. (c): D_8009BA30/50/58 have no C handle and no symbol-config row
(undefined_syms_auto.txt / named_syms.txt / symbol_addrs.txt: none); only the data .s dlabels remain,
referenced by no code once func_800620B8.s is no longer included. (d): canonical declaration in
include/game.h. (e): no other consumer of any byte in the span (grep asm/funcs, src, include).

Corrections carried from the session-1 layer-2 FAIL: record [11] has record [9]'s u/v (not frame
0's); [10] has record [0]'s u/v; the [6]/[10]/[11] addresses are rebuilt as absolute constants by
reload (REG_EQUIV rematerialisation), not formed from `$fp`.

## Layer-2 FAIL (manual lane, 2026-09-26) — the aggregate merge only
A fresh cheat-reviewer FAILED the `D_8009BA00[12][4]` merge on prong (a)
(no-new-park-categories.md:250-274): only this function references BA00/BA30/BA50/BA58 (no
cross-TU evidence); the original loads BA30/50/58 as absolute lui/addiu, not off the BA00 base
register; no census row; the 12-record boundary is arbitrary (same-shape 8-byte sprite records
continue backwards through D_8009B9B8..B9F0 and on to at least D_8009B8E8, used by func_80065800
and others); the loop.c-hoisting argument is evidence from this session's own scores, which the
prong excludes (decisions.md:27102 precedent: accepted (a) evidence = one base register reaching a
span of offsets in the original binary, or sibling functions forming the base and reading at stride).
Factual corrections to the rejected comment/message: record [11] (BA58) has u=0x30,v=0x80 = frame 9's
uv, not frame 0's ([10] BA50 does share frame 0's uv); the [6]/[10]/[11] addresses are rebuilt as
absolute constants by reload, not "rematerialised from $fp".
ACCEPTED by the same reviewer (do not re-litigate): the gte_stsz island + canonical row + region hash,
goto into the sibling case tails, SetTransMatrix((u8 *)tv - 0x14), the `rot` local, the scratch-word
tag link; no pins/barriers/FAKEs.

## Session 2 (2026-09-26, slotC2) — split-symbol floor PROVEN, prong-(a) search exhausted
Harness: tmp/func_800620B8/s2/dump.sh (splice variant, instrumented cc1 dumps), alloc.sh
(BB2_ALLOC_DEBUG priorities), psx.sh (original cc1psx, calibration only); copies of the scanner
and the key numbers are in this ledger (scan_base.py).

**Proof that no ordinary split-symbol body reaches the target (not a score argument).**
1. Target facts (asm/funcs/func_800620B8.s): `$fp` = 0x8009BA00 is set in the loop PRE-HEADER (after
   the entry `beqz`, next to the loop.c-hoisted `addiu $s0,$s1,0x25`) and used exactly ONCE in the
   loop (`addu $a0,$a0,$fp`, 0x800622AC). sv (base+0x34) and flag (base+0x3C) are spilled to
   0x10/0x18(sp). BA30/50/58 are rebuilt in `$t0` (the reload register) at each use.
2. A pre-header placement can only come from loop.c (a user statement before the `for` lands before
   the entry test; a3 variant, 50). loop.c hoists a constant load only if 27*savings*life >= 348
   (loop has calls: threshold 1+n_non_fixed_regs; life 10 not desirable, life 47 moved).
3. global.c priority = floor_log2(nrefs)*nrefs/live_length, refs weighted by loop depth (1 outside,
   2 inside), live_length DOUBLED for REG_EQUIV constants (local-alloc.c:1063). sv and flag: nrefs 3,
   live 339 -> 88. A hoisted BA00 pseudo with its one in-loop use: nrefs 1+2 = 3, live ~666 -> 45:
   it loses the ninth callee-saved register to sv/flag (a3, a4 dumps). It wins ($fp) only with
   nrefs >= 4. Model check (tmp .../m1_modelcheck_pun.c, a pun, NOT landable): a second in-loop use
   gives nrefs 5 -> pri 150 -> `$fp`, sv/flag spilled, exactly as predicted.
4. In the merged build the extra refs are the pre-header sets `(plus reg_BA00 48/80/88)` that combine
   forms from cse's related-value constants (BA00 nrefs 6, pri 180); those three pseudos lose
   allocation and reload rematerialises them as absolute constants in `$t0` — the target's bytes.
   The target has ONE in-loop `$fp` use, so its extra refs cannot be in-loop uses: they must be
   pre-header references, i.e. other hoisted pseudos computed from the BA00 register. With four
   distinct symbols cse cannot relate the addresses, so no such reference exists (a duplicated
   in-loop use would show a second `$fp` use the target does not have). No ordinary split-symbol
   body exists; the floor for split symbols is structural, not a search gap.
5. **Original compiler agrees** (tools/cc1psx_wrapper.sh, PsyQ GCC 2.7.2.SN.1, calibration only):
   split symbols -> `$fp` = base+60 (flag), each table `la $2/$3,D_8009BAxx` separately (the 47 shape);
   one 12-record table -> `la $fp,D_8009BA00` ... `addu $4,$4,$fp`, `la $8,D_8009BA00+80`,
   `la $8,D_8009BA00+48`, `la $8,D_8009BA00+88` — the target's registers exactly.

Split-symbol variants this session: a1 alias set in sel_a before `%6` 47 (life 10, not hoisted);
a3 function-scope alias before the `for` 50 (not allocated, remat); a4 alias at loop top 50 (hoisted
life 47, pri 45, loses to sv); a5 alias per tail for both frame tables 47.

**Prong-(a) evidence search (all negative, 2026-09-26).**
- scan_base.py over the ORIGINAL main EXE (0x80010000..0x8008D080) and MOVOVL.EXE: tracks every
  lui/addiu pointer into 0x8009B7F0..0x8009BA5F and every addiu/load/store offset off it. 40 address
  formations, every one to a distinct splat label; ONE derived offset (0x800606D0, D_8009B840+8,
  inside its own label); zero cross-label offsets; zero data words pointing into the region; nothing
  in the overlay.
- Every indexed table in the region is indexed over EXACTLY its own label extent: B8E8 by frame<7
  (func_800646E8), B920 by `&3` (func_80063E10 via func_80063B34 `(>>17)&3`), B998/B9B8 by a value
  clamped to <=3 (func_800678A8), BA00 by `%6`, BA30 by `&3`. The alternates (B940..B9F0) are only
  ever loaded as single absolute records in branch arms. No sibling reaches across a label.
- No census row / symbol-config row for BA00/BA30/BA50/BA58 (docs/, include/, *.txt). No symbol or
  map file on the disc. Only this function references the four labels.
- Data bounds: 0x8009BA60 begins a u8 sequence (different type) — the object ends at 0x8009BA5F.
  The start is not fixed by any byte: 0x8009B9F8 is a same-shape record referenced by nothing.
Conclusion: the only evidence that 0x8009BA00..0x8009BA5F is one object is this function's own
register assignment (items 1-5 above). That is the owner question (borderline.md 2026-09-26 + the
session-2 addendum): it is not a score, but it is derived from this function.

Context-bundle note: the dossier flags D_8009BD44 as a naming-alias "plus_1" piece of
g_menu_screen_live_byte_c (0x8009BD43). The dlabel is a 5-word object and func_800646E8 landed with
`extern s32 D_8009BD44[]` (layer-2 PASS, 119ff2237); candidate.c uses the same form.

## Why split symbols cannot reach the target (mechanism, 2026-09-26 loop dumps)
The target keeps 0x8009BA00 in callee-saved `$fp` (a loop.c movable placed after the entry test) and
rebuilds BA30/50/58 in `$t0` (reload rematerialising hoisted-but-unallocated constant pseudos). In the
merged build cse's use_related_value writes `D_8009BA00+80` etc. as `(plus reg_BA00 N)`, so the BA00
movable gets life 9 / savings 2 and the other three are `forces`-moved with it; with four distinct
symbols every constant load has life 1 and nothing is hoisted (47). Measured with split symbols:
per-tail table-pointer locals 47/55 (cse propagates the constant), four function-scope table pointers
39 (all four spill; sv wins $s8 since their live length is ~2x). No split-symbol spelling can create
the relation: it needs one C object (array or struct) spanning the four records. Merging the whole
same-shape region instead (`D_8009B920[40][4]`, indices 28..39) also measures 0 but has the same
prong-(a) gap (only func_80063E10 indexes from B920, records 0..3). Policy question filed in
docs/grind/borderline.md 2026-09-26.

## Proven constructs and their receipts (all measured on the final chassis, 2026-09-26)
Each row = candidate with ONLY that construct respelled.

| construct in candidate | alternative measured | score |
|---|---|---|
| switch: case 3 / case 2 set sizes then `goto` into case 0 / case 1's shared tail | each case carries its own copy of the tail (order 3,0,2,1) | 49 |
| one 12-record table `D_8009BA00[12][4]` ([0..5] frames A, [6..9] frames B, [10]/[11] alt-CLUT records) | four split splat symbols D_8009BA00/BA30/BA50/BA58 | 47 |
| `SetTransMatrix((u8 *)tv - 0x14)` | `SetTransMatrix(base)` | 65 |
| rotation-matrix pointer read into `rot` before the dst32 copies | read `D_800A3474` at the func_80061FAC call | 3 |
| `rot` also used for SetRotMatrix | (i.e. `SetRotMatrix(rot)`) | 71 |
| tag link through the D_800A34E8 / D_800A34E4 scratch words | first half through `prim->tag` | 22 |
| `width`/`height` s16 locals for the clamped projected size | inline ternaries | 5 |
| (height via if/else stores) | | 1 |
| `prim` read after dst16[0] | prim read first | 3 |

Why each is what the original did (mechanism, from dumps in tmp/func_800620B8/dump):
- **Table merge.** Target holds 0x8009BA00 in callee-saved `$fp` across the loop (hoisted by
  loop.c) while 0x8009BA30/50/58 are rematerialised in `$t0` at each use. loop.c only hoists the
  BA00 load (`life 9, savings 2, moved`) when cse relates the other three addresses to it as
  `(const (plus D_8009BA00 N))` (use_related_value) — i.e. one symbol. With four separate symbols
  the BA00 load has life 1-2 and is "not desirable" -> not hoisted -> 47. Data-layout evidence
  (independent of codegen): 0x8009BA00..0x8009BA5F is 12 contiguous 8-byte records of one shape
  {clut x, clut y, u, v} (asm/data/7D920.data.s:23262-23304): [0..5] clut(0x3C0,0xFD) u=0..0xA0
  step 0x20 v=0x94 (32px strip, +0x1F), [6..9] clut(0x3F0,0xFC) u=0..0x30 step 0x10 v=0x80 (16px,
  +0xF/+0x13), [10] clut(0x3C0,0xFE) same uv as [0] = alternate CLUT for A, [11] clut(0x3F0,0xFD)
  = alternate CLUT for B. The original indexes both groups with stride 8 (`(n%6)*8 + $fp`,
  `(n&3)*8 + base`). Only func_800620B8 references any of the four labels.
- **SetTransMatrix(tv - 0x14).** Same idiom in the ORIGINAL binary in two siblings:
  func_80063084 (0x800632CC `addiu $a0,$t1,-0x14`) and func_800646E8 (0x800648AC
  `addiu $a0,$t0,-0x14`). base+0x10/0x12 are the w/h halfwords (would be m[2][2]/pad), so there is
  no MATRIX at base: SetTransMatrix reads only m->t, and the code points it 0x14 below the
  translation vector. With `SetTransMatrix(base)` base stays live through the loop and the whole
  allocation shifts (65).
- **goto-shared case tails.** Target case 3 = `lw v1,A8; li 0x151; sh; lw v1,AC; j <case 0's AC sh>;
  li 0xA8` and case 2 likewise into case 1: jump2 cross-jumps the AC store into the goto target.
  Duplicated tails are NOT cross-jumped by our cc1 (identical tails stay, +33 insns).
- **Tag link.** Target reloads D_800A34E4/D_800A34E8 after the `sw` into the prim tag: the store was
  not MEM_IN_STRUCT, so it went through the scratch word, not a struct member.
- **rot.** Target loads D_800A3474 into `$a2` between the dst16 and dst32 stores and re-reads the
  global for SetRotMatrix after the call. Same shape as matched sibling func_80060B70 (same TU,
  `last_arg`).

## Open
- Landing needs: header declaration of the 12-record table (aggregate-merge prong (d)), POLY_FT4
  typedef moved above the function (it is defined after it in text1b.c), auth: row for the gte_stsz
  island (inline_asm_canonical.txt + tools/canonical_asm_regions.json), oracle rebuild.

## D_8009BD44 spelling (2026-09-26)
Adopted `extern s32 D_8009BD44[];` / `D_8009BD44[0] & 1` to agree with slotB's func_800646E8 (which needs the array form; the scalar form scores 5 there). Byte-neutral here: split body 47/501 either way, merged body 0/501 either way. The dlabel is a 5-word object (0x8009BD44..0x8009BD57), so the array declaration is accurate.

