# func_80058580 — Ruling 11 package for work1..work5 (laneB, 2026-10-01; final body [s11])

Ruling: `.claude/rules/reused-local-necessity.md` § Ruling 11 (owner 2026-09-26; Q20/Q28/Q34 value
clauses, Q30/Q31/Q58 proof standard), with owner rulings Q74 (work3's read-before-write) and Q75 (work2's
constant start + copy), rules commit 30a3e2d2d; Q82 (the s16 compare of work2's best value), rules
commit 215f2d11a. Each of `work1`..`work5` is judged on its own.

**Why Ruling 11 and not 5/6/9/10/Q51.** The values of each variable feed different consumers
(compares, table indices, call arguments, stores, other locals) through different templates (Ruling 5
1(a)/(b)); none is one record pointer per region (6); the writes are not `VAR = BASE + K` of one
record kind (9); there is no public original source (10); no SOTN function reuses a variable in these
roles (Q51 needs the same roles at corresponding statements).

## Bodies (all generated from ONE base; scripts in this directory, run from the repo root under WSL)
- **reuse spelling** = `memory/grind/func_80058580/candidate.c` (the body spliced at landing; `p` typed
  `PracticeMenuRec *`, no casts but Q82's; evidence.md [s8]-[s11]). Values that split free on the
  previous body (the chosen pick, the path length, the waypoint index, the state-0x15 counter) are now
  ordinary locals (`besti` read directly, `dist`, `wi`, `level`); every remaining value fails alone.
- `gen.sh` regenerates every body below into tmp/func_80058580/r11b/b/ (paths inside the scripts).
- **one-variable-per-value twins**: `roles.py <cand> var <out> workN` -> `pv_workN.c`: every value of
  workN renamed to its own fresh `s32 <value>_` (declared after `pick`), nothing else changed;
  `roles.py <cand> all <out>` -> `pv_all.c` (all 39 values split). `... bs` declares each value
  local at the innermost block enclosing all its occurrences (`pvbs_*.c`): byte-identical results.
- **single-value ablations**: `roles.py <cand> one <dir>` -> `r_<value>.c` (just that value split).
- **structural respellings**: `typed.py` -> `pvt_*.c` (each value typed by what it holds: flags u8,
  angles s16, the script address `u8 *` without the integer carrier casts, masks u32 without the
  `(u32)` casts); `mk_struct.py` -> `pvs_work4a.c` (walk as a `for` loop), `pvs_work4b.c` (walk
  index re-read `p->unk_362 - 1`, no copy), `pvs_work5a.c` (no work5: inline `work1 >> 27`, the
  offset written twice, et tested directly), `pvs_work5b.c` (only the et copy removed).
- **(C)(2) receipt**: `stmtcheck.py <cand> <twin>` (comments stripped, value declarations deleted,
  every `<value>_` renamed back): `IDENTICAL statement lists` for pv_work1..5, pv_all, pvbs_* and
  every r_*.c (stmtcheck.txt; the pvt_/pvs_ respellings differ by construction).
- Value table (occurrence indices, comments excluded): `roles.py <cand> list` (roles_list.txt).

## Measurement
`measure2.sh <bodies>` (driven by `runall.sh`): each body is spliced with ../typed/* (func_80055138,
func_80056FE8, func_80055B44 and the header edits of this landing) by ../typed/run.sh = the Makefile
pipeline (cpp | build cc1 | prologue_fix | maspsx | multu_pad | as | objcopy) on a scratch text1b.c,
then a standalone link of that text1b.o at 0x80047ED0 / .rodata 0x8001585C compared with build/bb2.bin
(negative control: build/src/text1b.o, 0 words), plus the masked engine score and ../typed/fd.py's
function diff. The reuse body: **0 differing words** (text 102804 bytes, rodata 88). Raw output:
scores_typed.txt.

## (D)(4) results (scores_typed.txt)
| body | differing .text words | masked score | insns |
|---|---|---|---|
| reuse (candidate.c) | **0** | 6 (jtbl relocation residue) | 2991 |
| pv_work1 / pvbs_work1 | 7706 | 332 | 2988 |
| pv_work2 / pvbs_work2 | 7676 | 254 | 2989 |
| pv_work3 / pvbs_work3 | 7648 | 106 | 2992 |
| pv_work4 / pvbs_work4 | 7824 | 589 | 2989 |
| pv_work5 / pvbs_work5 | 6789 | 31 | 2992 |
| pv_all / pvbs_all | 7881 | 768 | 2985 |
| pvt_all (typed) | 8614 | 821 | 3011 |
| pvt_work1 / 2 / 3 / 4 / 5 | 2380 / 8561 / 8608 / 7824 / 6789 | 338 / 277 / 141 / 589 / 31 | |
| pvs_work4a / 4b | 7824 / 8009 | 589 / 593 | |
| pvs_work5a / 5b | 6718 / 21 | 33 / 27 | |

Single-value ablations (differing words / masked score), every value fails alone:
work1 n449 8/14, base 7/13, kind 12/18, kind2 15/20, x1 13/19, m445 45/51, w 92/98, lo 6483/66, near 7703/327;
work2 n445 5/11, sum 124/130, pace 10/16, y1 12/18, m449 45/51, best 7675/256, cnt2 121/127;
work3 side 4/10, n447 10/16, ang 6/12, prod 4/10, force 7/13, farflag 7733/175, dang 5/11, coin 4/10,
sel3 7614/196, slot 8/14, mask 6/12, cmask 3/9, ok 6476/59; work4 i 6/12, n 7824/589, k 192/198;
work5 top 6789/27, adj 4/10, kindet 21/27.

Permuter campaign from the full split (tmp/func_80058580/permsplit3: base.c = dumps/pv_all/t.i of this
body, other function bodies stripped; compile.sh = build cc1 | prologue_fix | maspsx | as, function
extracted; target.o = asm/funcs/func_80058580.s; -j 2, 15 min): split base 6730; best finds 4205, 4585,
5675 (perm-output-4205-1/diff.txt: declaration moves and a deleted block, junk). The reuse body scores
6 on the engine scorer and links byte-identical. No find approaches the target.

## (D)(1) dumps and command lines
`dump2.sh <tag> <body>` (`dumpall2.sh` for reuse, pv_work1..5, pv_all): splice with ../typed/* ->
`mipsel-linux-gnu-cpp` with the build's CPP_FLAGS/CPP_DEFS (patched include first) ->
`tools/decomp-permuter/strip_other_fns.py` -> build cc1 `tools/gcc-2.7.2/build/cc1 -O2 -G0
-funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg`
-> instrumented `tools/gcc-2.7.2/cc1` with `BB2_ALLOC_DEBUG=1`; the two compilers' func_80058580 asm
is compared ("IDENTITY OK" for all seven). `findreg.sh <tag> <pseudo>...`: the instrumented cc1 with
`BB2_FINDREG_DEBUG=<pseudo>`. Banked: `alloc_<tag>.txt` (ALLOCDBG), `regs_<tag>.txt` (.lreg
`Register` lines), `findreg_<tag>.txt` (cand = reuse).

Pseudo map: candidate.c — 85 work1, 86 work2, 87 work3, 88 work4, 89 pick, 1900 work5 (declared inside
`while (off != 0)`, so numbered when that block is expanded); 1008 dist, 1009 wi (block-scoped; both
$s3). pv_workN — 85..88 are the (now unused) function-scope work declarations, 89 pick, the values of
workN are 90, 91, ... in the order of `roles.py list`.

Reuse spelling (`regs_cand.txt`, `alloc_cand.txt`), verbatim:
```
Register 85 used 57 times across 288 insns; dies in 11 places; crosses 1 call; GR_REGS or none.
Register 86 used 40 times across 278 insns; dies in 7 places; crosses 3 calls; GR_REGS or none.
Register 87 used 99 times across 1088 insns; dies in 7 places; crosses 10 calls; GR_REGS or none.
Register 88 used 28 times across 368 insns; crosses 3 calls; GR_REGS or none.
Register 1900 used 16 times across 29 insns; dies in 3 places; GR_REGS or none.
ALLOCDBG func=func_80058580 ord=14 pseudo=1900 hardreg=5 nrefs=16 livelen=29 pri=22068
ALLOCDBG func=func_80058580 ord=42 pseudo=85 hardreg=17 nrefs=57 livelen=288 pri=9895
ALLOCDBG func=func_80058580 ord=65 pseudo=86 hardreg=18 nrefs=40 livelen=278 pri=7194
ALLOCDBG func=func_80058580 ord=81 pseudo=87 hardreg=19 nrefs=99 livelen=1088 pri=5459
ALLOCDBG func=func_80058580 ord=108 pseudo=88 hardreg=20 nrefs=28 livelen=368 pri=3043
```
FINDREG (findreg_cand.txt): 85 conflicts `2 3 4 5 16 29`, pass1_used excludes every call-used
register (it crosses a call) -> first free = 17 ($s1); 86 conflicts `... 16 17 ...` -> 18 ($s2); 87
-> 19 ($s3); 88 conflicts `... 16 17 18 19 ...` -> 20 ($s4); 1900 conflicts `2 3 4 29`, crosses no
call -> first free = 5 ($a1).

## (D)(2) Mechanism (pass and source location; all in tools/gcc-2.7.2)
1. **flow.c** counts each pseudo's weighted references (`reg_n_refs[regno] += loop_depth`,
   flow.c:2081) and live length (`reg_live_length`, flow.c:1685/2087) and calls crossed.
2. **local-alloc.c:472-475** hands a pseudo to local allocation only when it lives in one basic block
   and dies once (`reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`); every other pseudo goes to
   global.c.
3. **global.c:575/635** sorts allocnos by `floor_log2(n_refs) * n_refs / live_length`.
4. **global.c:952-1080 find_reg**: an allocno that crosses a call may only take a call-saved register
   (`used1 = call_used_reg_set`, :974-975); one that crosses none may take any (`fixed_reg_set`,
   :972-973); the first register (no REG_ALLOC_ORDER on MIPS, :1061) not in `used` wins.

Applied:
- **work1..work4**: in the reuse spelling each is ONE allocno whose range spans all its values and
  crosses calls (1 / 4 / 15 / 3), so find_reg gives it a call-saved register, and its priority order
  gives $s1, $s2, $s3, $s4 — the registers every value is computed into in the target (table below).
  Split, each value is its own allocno: the short ones cross no call and take call-clobbered
  registers — e.g. pv_work1 x1_ (94) conflicts `2 3 16 17 29`, crosses no call -> $a0 (4); w_ (96)
  conflicts `2 3 29` -> $a0; pv_work2 cnt2_ (96) conflicts `2 3 29` -> $a0; pv_work3 ang_ (92)
  conflicts `2 3 16 29` -> $a0, slot_ (99) conflicts `2 3 4 29` -> $a1; pv_work4 i_ (90) -> $a2 (6);
  the ones that cross a call get call-saved registers in a different priority order (pv_work4 k_ (92)
  -> $s6; pv_work3 sel3_ crosses 19 calls and is spilled), and the rest of the function's call-saved
  assignment shifts with them (regs_pv_*.txt, alloc_pv_*.txt). Values that live in one block (pv_work1
  kind2_, pv_work2 sum_, pv_work3 cmask_) are local-alloc'd to call-clobbered registers. The target
  computes each of them in $s1..$s4.
- **work5** (no calls; $a1): in the reuse spelling its range includes the et-test region where $v0,
  $v1 and $a0 are busy (conflicts `2 3 4 29`), so find_reg's first free register is $a1 for all three
  values. Split (pv_work5): top_ (90) conflicts only `29` -> $v0 (target `sra $a1,$s1,27`
  0x8005A7B4); adj_ (91, "in block 459") is local-alloc'd and tied to its dying input (target `addiu
  $a1,$v1,-0x190` 0x8005AA84); only kindet_ (92) keeps $a1.
- **work3's 0x394 slot, Q74.** The target's slot switch reads $s3 (`sltiu` 0x80059D70, `sll`
  0x80059DB4) with no write on the path unk_39C == 1, opponent state neither 0x19 nor 0x1A (0x80059D18
  -> 0x80059D6C -> 0x80059DB0). The reuse spelling reproduces that read; the split sel3_ is live from
  function entry, crosses 19 calls and is spilled (alloc_pv_work3.txt). Per Q74 that read is excluded
  when grouping work3's values; the thirteen values below are the groups of writes reaching common
  reads without it, and (B)(1) holds for every write through its other reads.

## Values, writes and the target (registers from asm/funcs/func_80058580.s)
(A) every variable is a fresh `s32` local of this function, declared once at the innermost scope
enclosing its writes (work1..work4 at function scope — their writes span the body; work5 inside
`while (off != 0)`), no other declaration moved, address never taken.
(B)(1) every write is read on some path before the next write of the variable (the compares, table
indices and arguments listed). (B)(2) no write stores a value the variable already holds on every
feasible incoming path: the constant writes are per-path alternatives (0/1 flags, the script address
choices, the slot constants); `work1 = p->unk_444[6]` at the waypoint script re-loads the byte read for
the lim chain only on paths with wtype == 1 — on the feasible path wtype != 1 (unk_39D != 0 and
unk_39E != 1) work1 holds the stage base or the unk_444[5] flag there, and func_80057E84 (called on
the record path, gets p) may write unk_444[6] in between.
(C)(3) each value has load / arithmetic / call-result writes whose instructions are in the target;
the constants are per-branch alternatives selected by runtime conditions (Q20) inside value groups
(0/1 flags; 0x12..0x16 slots; the three script addresses), or a group's initial alternative beside
computed writes (100000 vs D_8009A838[stage] * 8; the loop counters' 0 vs
++), as in func_80055138's admitted `temp` (`temp = 0` beside `D_800A37D2 / 3`). work2's best is the
Q75 value: one constant start (-1) plus one plain copy (score).

| var | value | C write(s) | target write ($reg, address) |
|---|---|---|---|
| work1 $s1 | n449 | `work1 = p->unk_444[5] == 0;` | sltiu 0x800587C0 |
| | base | `work1 = 100000;` / `= D_8009A838[p->unk_0E] * 8;` | lui/ori 0x80058EF4/EFC, sll 0x80058F10 |
| | kind | `work1 = p->unk_444[6];` | lbu 0x80059254 |
| | kind2 | `work1 = p->unk_444[6];` | lbu 0x800595B0 |
| | x1 | `work1 = p->unk_364[1].x;` | lh 0x80059804 |
| | m445 | `work1 = p->unk_444[1] == 0;` | sltiu 0x80059C04 |
| | w | `work1 = D_8009A9F0[..][..];` `work1 >>= 4;` `work1 >>= ...;` | lw 0x8005A7A4, sra 0x8005A7C8, srav 0x8005A840 |
| | lo | `work1 = e[1] * 40;` `= et < 5 ? 100000 : 0;` `+= work5 + p->unk_40A;` | sll 0x8005A978, and 0x8005AA48, addu 0x8005AA8C |
| | near | `= p->unk_00->unk_3F8[..];` / `= p->unk_00->unk_404[..] + 300;` | lh 0x8005ADD8, addiu 0x8005AE24 |
| work2 $s2 | n445 | `work2 = p->unk_444[1] == 0;` | sltiu 0x800587B8 |
| | sum | `work2 = D_8009A850[work4][2] * 16 + work1 + p->unk_40A;` | addu 0x80058F80 |
| | pace | `work2 = p->unk_444[0];` | lbu 0x800595B8 |
| | y1 | `work2 = p->unk_364[1].z;` | lh 0x8005980C |
| | m449 | `work2 = p->unk_444[5] == 0;` | sltiu 0x80059C08 |
| | best | `work2 = -1;` `work2 = score;` (Q75) | li 0x8005A108/A118, `addu $s2,$s3,$zero` 0x8005A350 |
| | cnt2 | `work2 = work1 & 0xF;` `work2--;` | andi 0x8005A7AC, addiu 0x8005A7D8 |
| work3 $s3 | side | `= p->unk_00->unk_AF & 1;` `= !work3;` | andi 0x800586A0, sltiu 0x80058760 |
| | n447 | `= p->unk_444[3] == 0;` | sltiu 0x800587B0 |
| | ang | `= p->unk_43A;` `= (work3 + 0x800) & 0xFFF;` `-= 0x1000;` | lh 0x80058A74, andi 0x80058A80, addiu 0x80058A90 |
| | prod | `= 0x1000 - (...);` `= (p->unk_438 * work3) >> 12;` | subu 0x80058D90, sra 0x80058DA8 |
| | force | `= 0;` / `= 1;` (Q20) | 0x80058E84 / 0x80058EAC |
| | farflag | `= lim < dist;` | slt 0x8005985C |
| | dang | `= (ratan2(..) - p->unk_1C8.vy) & 0xFFF;` `-= 0x1000;` | andi 0x80059974, addiu 0x80059984 |
| | coin | `= pick_weight[4] < (rand() & 0xFF);` `work3++` | lbu/slt 0x80059C34/3C, addiu 0x80059CC4 |
| | sel3 | `= p->unk_394;` / `= slots0[..];` / `= slots1[..];` | lw 0x80059D0C, lbu 0x80059D60/DA4 |
| | slot | `= D_800A38E2 / 10 * 2;` `work3--;` / `= 0x12..0x16` | sll 0x8005A6CC, addiu 0x8005A6F0, li 0x8005A6F8..A724 |
| | mask | `= D_8009A928[..][work3];` / `= 0; |= ...;` `= 1 << ..;` / `= D_8009A8C8[..][..].mask;` | lhu 0x8005A754, 0x8005A7A8, or 0x8005A7E0, sllv 0x8005A850, lhu 0x8005A878 |
| | cmask | `= q[4] << 24 | ... | q[1];` | or 0x8005A92C |
| | ok | `= 0;` / `= 1;` x6 (Q20) | 0x8005A998, 0x8005A9F0, 0x8005AB10, 0x8005AB7C, 0x8005ABB4 |
| work4 $s4 | i | `for (work4 = 0; ..; work4++)` | 0x80058F14, addiu 0x800590C0 |
| | n | `work4 = wi;` (Q34 copy, in the walk branch) `work4--;` | `addu $s4,$s3,$zero` 0x800596DC (bnez delay slot), 0x800597E0 |
| | k | `work4 = 0;` `work4++;` | 0x8005A638, addiu 0x8005AC44 |
| work5 $a1 | top | `work5 = work1 >> 27;` | sra 0x8005A7B4 |
| | adj | `work5 = (((0x1000 - lv) * 625) >> 10) - 400;` | addiu 0x8005AA84 |
| | kindet | `work5 = et;` (Q34 copy) | `addu $a1,$s5,$zero` 0x8005AA94 |

**Copy-clause values (one per variable).** best (Q75: `work2 = -1;` + `work2 = score;`, score a named
local read again by the next compare, no cast, target `li $s2,-1` 0x8005A108/0x8005A118 and `addu
$s2,$s3,$zero` 0x8005A350), n (`work4 = wi;`: wi is read again by `if (wi == 0)`, target `addu $s4,$s3,$zero`
0x800596DC; wi is in $s3), kindet (`work5 = et;`: et is read again by `row2[et]`; the s16 ->
s32 conversion adds no instruction, target `addu $a1,$s5,$zero` 0x8005AA94). The fresh locals (r_best,
r_n, r_kindet) and the no-copy bodies (pvs_work4b re-reads unk_362 - 1; pvs_work5b tests et directly)
are measured above; best has no no-copy form (the running maximum must be kept).

## (E)/(F)
Names are the generic `work1`..`work5` (Ruling 11 (E)); each declaration carries the inline comment
naming its values, citing the ruling and this file; work3's comment also names the Q74 path and
addresses, work2's the Q75 constant loads and move and the Q82 compare (commented again at the site).

## Reviewer spellings banked
- rev-58580-r11-c, wi countdown (rev_wi_countdown.sh: the walk counts wi down directly, no work4 copy):
  misses by 1 instruction (cc1 asm of the function differs in 52 lines; work4's n value stays necessary).
