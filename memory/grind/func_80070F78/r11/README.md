# Ruling 11 package — func_80070F78's `vram`, `sheets`, `cells` (2026-09-29, manual s2, laneA)

Body: the landed func_80070F78 in src/text1b_tu1d.c (the D_800A3560 record union, -G8 TU;
Q44/Q54 change 3, 2026-09-30). This package was first built on
memory/grind/func_80070F78/candidate.c, the older per-byte body (identical to
tmp/func_80070F78/final.c at banking); (D)(1)-(4) are measured on that body and (D)(5) on
the landed one. Three locals hold more than one value in the
sense of Ruling 11 (writes that no common read can see):

| var | value | write(s) | reads | target write site |
|---|---|---|---|---|
| `vram` (loop-2 body block) | V1 | `vram = *(u8 **)(D_800A35A8 + 0x7C); vram += i << 6;` at the loop top | `vram + id * 8` at the ==3 and confirm LoadImage calls | asm lines 108/112 `lw $s4,0x7C($v0)` / `addu $s4,$s4,$v0` |
| | V2 | the same two statements in the locked-slot arm | `vram + id * 8` at the locked LoadImage | lines 612/616 `lw $s4,0x7C($a1)` / `addu $s4,$s4,$a0` |
| `sheets` (function scope) | S1 | `sheets = *(s32 **)(D_800A35A8 + 0x74);` at entry, and the same statement in the selected-slot arm (both reach the tail's `sheets[i + 1]` / `sheets[i]`, so they are one value) | `s->header = sheets[0]`, `sheets[3]`, `sheets[i + 1]` / `sheets[i]` | lines 15 and 442 `lw $fp,0x74($v0)` |
| | S2 | `sheets = *(s32 **)(D_800A35A8 + 0x60);` after loop 2 | `s->header = sheets[0];` | line 692 `lw $fp,0x60($v0)` |
| `cells` (function scope) | C1 | `cells = s->header + 0xC;` (selected-slot arm) | `s->table = cells;` | line 466 `addiu $s0,$v0,0xC` |
| | C2 | `cells = s->header + 0xC;` (draw tail) | `s->table = cells;` | line 530 `addiu $s0,$v0,0xC` |
| | C3 | `cells = s->header + 0x24;` before loop 3 | `s->table = cells;` in loop 3 | line 704 `addiu $s0,$v1,0x24` |

Every other local is either written once or written on exclusive paths that all reach one
read (`flag`, `max`/`min`, each block's `sel`), so it holds one value.

## (A) Fresh locals, not borrows
Plain locals, never parameters/globals/statics/register, address never taken. Each is
declared at the innermost scope enclosing all of its writes: `vram` in the loop-2 body
block (V1 in that block, V2 in the locked arm inside it); `sheets` and `cells` at
function scope (writes at function level and inside loop 2).

## (B) Every write live; no re-store
- (1) Every write is read before the next write or scope end: V1 by the ==3/confirm
  LoadImage (on the locked path V1 is overwritten unread — store-level deadness on one
  path is not a dead store: V1 is read on the unlocked paths); V2 by the locked
  LoadImage; S1 by the next `s->header = sheets[...]`; S2 by `s->header = sheets[0]`;
  C1/C2 by the following `s->table = cells;`; C3 by loop 3.
  The compound `vram = ...; vram += i << 6;` pairs are one computation split per
  Ruling 4; the first statement is read by the second.
- (2) Re-stores, judged path-wise (Ruling 5 2(c) with its 2026-09-26 clarification):
  - **V2 vs V1 (re-load of `*(u8 **)(D_800A35A8 + 0x7C)`).** On every path into the
    locked arm, V1 was loaded from the same lvalue earlier in the iteration, and the
    call `func_8005C650(1, 0x7F, 0x7F)` intervenes. By C semantics that call may write
    the lvalue: it stores through `(u8 *)&D_800EFB78 + off` (src/text1b.c
    func_8005C650), and nothing in C shows that the object D_800A35A8 points to is not
    that pool; D_800A35A8 itself is a global the callee may also reach. **This is the
    weakest point of the package; the reviewer should judge it.** Measured: the
    per-value spelling (V2 in its own local) scores 13 (below), and the target bytes
    contain the second load (line 612), so the source re-loaded.
  - **S1's second write (selected-slot arm) vs the earlier S1 load.** Same lvalue
    `*(s32 **)(D_800A35A8 + 0x74)`; on every path to it, calls intervene
    (SetDrawMode, AddPrim, func_8006E480 at entry; func_8005C650 / func_8007352C /
    func_80073728 / LoadImage in earlier iterations), each of which may write global
    memory by C semantics. Same caveat as V2. Target line 442 holds the reload.
  - **S2 vs S1:** different lvalue (+0x60 vs +0x74); on the path where the two table
    pointers differ (distinct fields of the resource block) the variable holds S1.
  - **C1/C2/C3:** each follows a new `s->header` store/reload (C1 after
    `s->header = sheets[3]`, C2 after `sheets[i + 1]`/`sheets[i]`, C3 after
    `sheets[0]` of the +0x60 table) or a different offset (C3 +0x24 vs +0xC); on the
    path where the headers differ the variable holds a different value.

## (C) Same statements; every value real
- (1) The one-variable-per-value spelling is `ablf/r11_all.c` (banked here as
  r11/r11_all.c): `vram2` in the locked block, `sheets2` at function scope, `cells_a`
  in the selected-slot block, `cells_b` in the draw-tail block, `cells_c` at function
  scope (each at the innermost scope enclosing its writes).
- (2) It differs from the landing only in declarations and identifiers
  (tmp/func_80070F78/abl_final.py generates it from final.c by renaming).
- (3) Every write is a load or an addition present in the target (table above).

## (D) Allocator proof
**(1) Dumps.** Mini-TU builds (tmp/func_80070F78/mini/pre.c + body; memory/grind/
func_80070F78/tools/dump.py) with the instrumented cc1 and `BB2_ALLOC_DEBUG=1`:

    tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da

Extracted facts (r11/r11info.txt = tools/r11info.py over the .lreg/.greg/ALLOCDBG of
each body; r11/allocdbg.txt):
- reuse (final): `vram` pseudo 144 "used 18 times across 193 insns; crosses 4 calls",
  global pri 3730 (nrefs 18, livelen 193), ord 38 -> hard 20 ($s4), before `port`
  (pseudo 78, pri 2830, ord 39 -> $s5). `sheets` pseudo 74 crosses 20 calls -> 30 ($fp),
  including the +0x60 load (insn 1818). `cells` pseudo 76 crosses 1 call (loop 3's
  func_80073728) -> 16 ($s0) at insns 1241/1408/1846.
- r11_vram: V1 pseudo 144 drops to nrefs 10 / livelen 176 / pri 1704, ordered after
  `port` (ord 38, pri 2830 -> $s4), so V1 gets $s5; V2 pseudo 678 "used 8 times across
  17 insns in block 67" -> local-alloc, hard 6 ($a2). Score 13.
- r11_sheets: S2 pseudo 75 "used 2 times across 3 insns in block 74" -> local, $v0.
  Score 3.
- r11_cells_a / _b: C1 (pseudo 440) / C2 (pseudo 572) "across 2 insns in block N" ->
  local, $v0; scores 4 / 7. r11_cells_c: C3 alone in pseudo 75 ($s0); C1+C2 pseudo 77 no
  longer crosses a call -> $v0; score 10.
- r11_all: every split value as above; score 24.

**(2) Mechanism.** local-alloc.c allocates only pseudos whose life lies in one basic
block (reg_qty; non-local pseudos are left to global.c), first-fit in REG_ALLOC_ORDER
with the call-clobbered $2/$3/$4.. first. global.c gives a call-crossing allocno only
call-saved registers and allocates allocnos in allocno_compare priority order
(floor_log2(refs) * refs / live_length). In the reuse spelling each variable's pseudo
spans blocks and crosses calls (vram: the loop-2 calls; sheets: the whole function;
cells: loop 3's func_80073728), so global.c seats it in one call-saved register for its
whole life, and every write site lands there: $s4 at lines 108 and 612, $fp at 15, 442,
692, $s0 at 466, 530, 704. For vram the merged refs also lift its priority above
`port`'s, which is why vram takes $s4 and port $s5.

**(3) Necessity: mechanism plus search (Q31).**
- The target writes V2, S2, C1, C2 into call-saved registers although none of those
  values crosses a call. With one variable per value each of them is written and read
  inside one basic block with no call between, so local-alloc takes it and its
  first-fit choice is call-clobbered (dumps above). No per-value spelling can move those
  values into $s4/$fp/$s0 without extending their lives across a call, which would add
  statements ((C)(2) forbids). V1 alone keeps a call-crossing life but loses the refs
  that order it before `port`.
- The search: every banked counting spelling misses (table), and the permuter campaign
  below found none.

**(4) Measured alternatives** (mini TU, engine score; the landing body scores 1 = the
GPREL16 name artifact `%gp_rel(D_800A35C8+2)` vs `%gp_rel(D_800A35CA)`, same bytes;
full-build SHA1 == oracle 2026-09-29):

| spelling | score / insns |
|---|---|
| r11_all (one variable per value, innermost scopes) | 24 / 810 |
| r11_vram (V2 split only) | 13 / 810 |
| r11_sheets (S2 split only) | 3 / 810 |
| r11_cells_a (C1 split only) | 4 / 810 |
| r11_cells_b (C2 split only) | 7 / 810 |
| r11_cells_c (C3 split only) | 10 / 810 |
| r11_cells_all | 10 / 810 |
| split_all with function-scope declarations (earlier body, r11/split_all.c) | 24 / 810 |
| S2 with no local, `s->header = **(s32 **)(D_800A35A8 + 0x60);` | 3 / 810 |
| C1 and C2 with no local (`s->table = s->header + 0xC;`) | 10 / 810 |
| V2 in a function-scope `vram2` (either declaration order) | 13 / 810 |
| vram one statement per value (`= load + (i << 6)`) | 29 / 809 |
| permuter campaign from r11_all | see "Permuter" |

## (E) Names
Form (ii): `vram` — every value is player i's VRAM rect row (the RECT array at
*(D_800A35A8 + 0x7C) advanced by i << 6), the LoadImage destination base.
`sheets` — every value is a table of sprite-sheet header pointers (`s->header =
sheets[k]`, the sheet func_8007352C / func_80073728 draw). `cells` — as in
func_800720FC: the address of the 8-byte cell table following a sheet header.

## (F) Annotation
Each declaration comment in candidate.c says the variable holds several values of one
kind, names them, and cites Ruling 11 and this file.

## (G) Review
Layer-2 on the manual path.

## (H) Everything else
Judged on its own: the per-site record offsets and `tim` pointers under the
named-intermediate entry (evidence.md [s2] ablations), the Ruling 4 vram split, the
prototype / func_80070C70 descriptor type.

## Permuter
Workspace tmp/func_80070F78/permB (tools/mkws.sh on r11/r11_all.c), -j 2, --stack-diffs,
through tools/permuter_campaign.py (telemetry in metrics/events.jsonl):
- r11-all-final: 2026-09-29 17:58-18:08 UTC, 580 s, 5495 iterations, best permuter score 655
  (base 710; the landing body itself scores 145 in the permuter metric, symbol-name artifacts).
- r11-all-final-2: launched 18:08 UTC, harvested 2026-09-30 ~04:20 UTC after a network outage
  (36653 s, 45702 iterations), best permuter score 425.
- Engine re-score of every output's function (tmp/func_80070F78/rescore_perm.py, sc.py --mini;
  r11/permuter_engine_scores.txt): best 21 (output-640-1), then 24 x5 (425-1 included), 26, 27,
  29, 35; three outputs did not rebuild outside the permuter TU. None reaches the target (1).

## (D)(5) Landed form (2026-09-30, Q44/Q54 change 3)
(D)(1) and (D)(4) above were measured at -G0 on the per-byte body. The split-local variants below are
generated by the reviewer's r11/landed-2026-09-30/r11landed.py (run from the repo root; its
outputs, the variant bodies *.c and scores.txt, are banked beside it: same 0, cells_a 3, cells_b 6,
cells_c 9, cells_all 9, sheets2 2, vram2 12, r11_all 23, re-run 2026-09-30). On the landed form (the
D_800A3560 record union, text1b_tu1d at -G8, sandbox --disable all; the landing body scores
0 / 810), the change-3 layer-2 reviewer measured:

| spelling | score / insns |
|---|---|
| r11_all (one variable per value) | 23 / 810 |
| sheets2 (S2 split) | 2 / 810 |
| sheets, no local (`**(s32 **)(D_800A35A8 + 0x60)`) | 2 / 810 |
| cells_c (C3 split) | 9 / 810 |
| cells_a, no local | 3 / 810 |
| cells_b, no local | 6 / 810 |
| cells_a and cells_b, no local | 9 / 810 |
| cells_a (C1 split local) | 3 / 810 |
| cells_b (C2 split local) | 6 / 810 |
| cells_all (C1, C2, C3 split) | 9 / 810 |
| vram2 (V2 split) | 12 / 810 |
| tim index lever | 30 / 810 |

Every alternative still misses on the landed form. The (D)(2) mechanism, confirmed on the
landed body by the re-review (g8-review2) from the sandbox asm diffs: vram2 puts V2 in a
call-clobbered register, $a2 (`lw a2,124(a1)`), where the target has $s4, and V1 and `port`
swap between $s4 and $s5 (V1 loses the refs that ordered it before `port`); sheets2 puts S2 in
$v0 (target $fp); cells_a puts C1 in $v0 (target $s0). Same effects as the (D)(1) dumps on the
per-byte body: a value whose life stays in one block goes to local-alloc's call-clobbered
first-fit, and only the reused variable spans the calls that seat it in the call-saved register.
