# func_80058580 — Ruling 11 package for work1..work5 (laneB, 2026-10-01)

Ruling: `.claude/rules/reused-local-necessity.md` § Ruling 11 (owner 2026-09-26; Q20/Q28/Q34 value
clauses, Q30/Q31/Q58 proof standard). Each of `work1`..`work5` is judged on its own.

**Why Ruling 11 and not 5/6/9/10/Q51.** The values of each variable feed different consumers
(compares, table indices, call arguments, stores, other locals) through different templates (Ruling 5
1(a)/(b)); none is one record pointer per region (6); the writes are not `VAR = BASE + K` of one
record kind (9); there is no public original source (10); no SOTN function reuses a variable in these
roles (Q51 needs the same roles at corresponding statements).

## Bodies (all generated from ONE base; scripts in this directory, run from the repo root)
- **reuse spelling** = `memory/grind/func_80058580/candidate.c` (the body spliced at landing).
- **one-variable-per-value twins**: `roles.py <cand> var <out> workN` -> `pv_workN.c`: every value of
  workN renamed to its own fresh `s32 <value>_` (declared after `pick`), nothing else changed;
  `roles.py <cand> all <out>` -> `pv_all.c` (all 39 values split). `... bs` declares each value
  local at the innermost block enclosing all its occurrences (`pvbs_*.c`): byte-identical results.
- **single-value ablations**: `roles.py <cand> one <dir>` -> `r_<value>.c` (just that value split).
- **structural respellings**: `typed.py` -> `pvt_*.c` (each value typed by what it holds: flags u8,
  angles s16, the script address `u8 *` without the carrier cast, masks u32); `mk_struct.py` ->
  `pvs_work4a.c` (walk as a `for` loop), `pvs_work4b.c` (walk index re-read, no copy),
  `pvs_work5a.c` (no work5: inline `work1 >> 27`, the offset written twice, et tested directly),
  `pvs_work5b.c` (only the et copy removed).
- **(C)(2) receipt**: `stmtcheck.py <cand> <twin>` (comments stripped, value declarations
  deleted, every `<value>_` renamed back): `IDENTICAL statement lists` for pv_work1..5, pv_all,
  pvbs_all and every r_*.c (stmtcheck.txt).
- Value table (occurrence indices, comments excluded): `roles.py <cand> list` (roles_list.txt).

## Measurement
`measure.sh <bodies>`: `memory/grind/func_80058580/probes/s5/fullobj.sh` = the Makefile pipeline
(cpp | build cc1 | prologue_fix | maspsx | multu_pad | as | objcopy) on a scratch text1b.c with the
body spliced and the landing's declaration edits (probes/s5/splice.py: transcribed jtbl arrays
removed, D_8009A838[] and its func_80056FE8 read, the D_8009A8C8 table, func_80055138's read), then
`probes/s5/linkcheck.py` (links that text1b.o alone at 0x80047ED0/.rodata 0x8001585C, undefined
symbols from build/bb2.elf, compares with build/bb2.bin; negative control build/src/text1b.o: 0
words) and the masked score (`engine.score.score_func` vs build/src/text1b.o). The reuse body: **0
differing words** (text 102804 bytes, rodata 88); masked score 8 = relocation-representation residue
(section-relative jtbl addends, D_8009A8C8+2 for the old alias), not instruction differences.

## (D)(4) results (scores_*.txt)
| body | differing .text words | masked score | insns |
|---|---|---|---|
| reuse (candidate.c) | **0** | 8 | 2991 |
| pv_work1 / pvbs_work1 | 7706 | 333 | 2988 |
| pv_work2 / pvbs_work2 | 7676 | 274 | 2989 |
| pv_work3 / pvbs_work3 | 7652 | 123 | 2992 |
| pv_work4 / pvbs_work4 | 7636 | 119 | 2989 |
| pv_work5 / pvbs_work5 | 6789 | 33 | 2992 |
| pv_all / pvbs_all | 7703 | 343 | 2985 |
| pvt_work1 / 2 / 3 / all (typed) | 2380 / 8587 / 8633 / 8592 | 339 / 302 / 161 / 408 | |
| pvt_work4 / 5 | as pv (no type change for ints) / 6789 | 119 / 33 | |
| pvs_work4a / 4b | 7636 / 7819 | 119 / 120 | |
| pvs_work5a / 5b | 6718 / 21 | 35 / 29 | |

Single-value ablations (words / masked score), every value fails alone:
work1 n449 8/16, base 7/15, kind 12/20, kind2 10/17, x1 7599/67, m445 45/53, bsel 7703/328, w 7/15,
lo 6483/68; work2 n445 5/13, sum 5/13, pace 5/13, y1 7/15, m449 45/53, best 23/31, cnt2 137/144,
lvl 10/18; work3 side 4/12, n447 10/18, ang 6/14, prod 4/12, force 7/15, idx 4/12, dist 7802/191,
farflag 7683/10, dang 5/13, coin 4/12, sel3 7614/210, slot 8/16, mask 6/13, cmask 3/11, ok 6476/61,
script4 16/24; work4 i 6/14, n 7636/119, k 204/211; work5 top 6789/29, adj 4/12, kindet 21/29.
(The pick-loop counter, formerly a work4 value, splits free and is its own `pick`; the D_8009A850
entry distance was an assignment inside the condition and is now its own statement — both in the
reuse body.)

Permuter campaign from the full split (tmp workspace = pv_all preprocessed, other function bodies
stripped, target.o assembled from asm/funcs/func_80058580.s; 3 x 9.5 min, -j 2, ~3500 iterations):
base 4205, best 3585 (a junk `far` retype plus inserted dead statements); the reuse body scores 180
on the same scorer (relocation-name residue only). No find approaches the target.

## (D)(1) dumps and command lines
`dump.sh <tag> <body>`: splice (probes/s5/splice.py) -> `mipsel-linux-gnu-cpp` with the build's
CPP_FLAGS/CPP_DEFS (patched include first) -> `tools/decomp-permuter/strip_other_fns.py` -> build cc1
`tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
-w -mel -msoft-float -dr -dl -dg` -> instrumented `tools/gcc-2.7.2/cc1` with `BB2_ALLOC_DEBUG=1`; the
two compilers' func_80058580 asm is compared ("IDENTITY OK" for cand, pv_work1..5, pv_all).
`findreg.sh <tag> <pseudo>...`: the instrumented cc1 with `BB2_FINDREG_DEBUG=<pseudo>`.
Banked excerpts: `alloc_<tag>.txt` (ALLOCDBG), `regs_<tag>.txt` (the .lreg `Register` lines of the
work/value pseudos), `findreg_<tag>.txt`. Full dumps regenerate in tmp/ (`dump_all.sh`).

Pseudo map: candidate.c — 84 work1, 85 work2, 86 work3, 87 work4, 88 work5, 89 pick, 91 et, 105 ep.
pv_workN — 84..88 are the (now unused) work declarations; the values of workN are 90, 91, ... in the
order of `roles.py list`.

Reuse spelling (`regs_cand.txt`, `alloc_cand.txt`), verbatim:
```
Register 84 used 58 times across 284 insns; dies in 11 places; crosses 1 call; GR_REGS or none.
Register 85 used 47 times across 331 insns; dies in 8 places; crosses 4 calls; GR_REGS or none.
Register 86 used 122 times across 1292 insns; dies in 9 places; crosses 15 calls; GR_REGS or none.
Register 87 used 28 times across 375 insns; crosses 3 calls; GR_REGS or none.
Register 88 used 16 times across 29 insns; dies in 3 places; GR_REGS or none.
ALLOCDBG func=func_80058580 ord=14 pseudo=88 hardreg=5 nrefs=16 livelen=29 pri=22068
ALLOCDBG func=func_80058580 ord=24 pseudo=84 hardreg=17 nrefs=58 livelen=284 pri=10211
ALLOCDBG func=func_80058580 ord=66 pseudo=85 hardreg=18 nrefs=47 livelen=331 pri=7099
ALLOCDBG func=func_80058580 ord=80 pseudo=86 hardreg=19 nrefs=122 livelen=1292 pri=5665
ALLOCDBG func=func_80058580 ord=108 pseudo=87 hardreg=20 nrefs=28 livelen=375 pri=2986
```
FINDREG (findreg_cand.txt): 84 conflicts `2 3 4 5 16 29`, pass1_used excludes every call-used
register (it crosses a call) -> first free = 17 ($s1); 85 conflicts `... 16 17 ...` -> 18 ($s2); 86
-> 19 ($s3); 87 conflicts `... 16 17 18 19 ...` -> 20 ($s4); 88 conflicts `2 3 4 29`, crosses no
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
  registers — e.g. pv_work1 x1_ (94) conflicts `2 3 16 17 29`, crosses no call -> $a0 (4); w_ (97)
  conflicts `2 3 29` -> $a0; pv_work2 cnt2_ (96) conflicts `2 3 29` -> $a0; pv_work3 ang_ (92)
  conflicts `2 3 16 29` -> $a0, slot_ (101) conflicts `2 3 4 29` -> $a1; pv_work4 i_ (90) -> $a2 (6);
  the ones that cross a call get call-saved registers in a different priority order
  (pv_work4 k_ (92) priority 879 -> $s6; pv_work3 sel3_ (100) "used 5 times across 1108 insns;
  crosses 19 calls", hardreg -1, i.e. spilled), and the rest of the function's call-saved assignment
  shifts with them (regs_pv_*.txt, alloc_pv_*.txt). Values that live in one block (pv_work1 kind2_
  93 "in block 217", pv_work2 sum_ 91 "in block 137", pv_work3 cmask_ 103 "in block 444") are local-alloc'd
  to call-clobbered registers. The target computes each of them in $s1..$s4.
- **work5** (no calls; $a1): in the reuse spelling its range includes the et-test region where $v0,
  $v1 and $a0 are busy (conflicts `2 3 4 29`), so find_reg's first free register is $a1 for all three
  values. Split (pv_work5): top_ (90) conflicts only `29` -> $v0 (target `sra $a1,$s1,27`
  0x8005A7B4); adj_ (91, "in block 459") is local-alloc'd and tied to its dying input (target `addiu
  $a1,$v1,-0x190` 0x8005AA84); only kindet_ (92) keeps $a1 (conflicts `2 3 4 16 29`).
- **work3's 0x394 slot** (sel3) is read uninitialized on one path in the original: when p[0x39C] == 1
  and the opponent state is neither 0x19 nor 0x1A, the switch reads whatever work3 held (target $s3,
  no write on that path, 0x80059D18-0x80059DB0). The reuse spelling reproduces that; the split
  sel3_ is live from function entry, crosses 19 calls and is spilled (alloc_pv_work3.txt).

## Values, writes and the target (registers from asm/funcs/func_80058580.s)
(A) every variable is a fresh `s32` local of this function, declared once at function scope (each
variable's writes span the whole body), no other declaration moved, address never taken.
(B)(1) every write is read on some path before the next write of the variable (the compares, table
indices and arguments listed). (B)(2) no write stores a value the variable already holds on every
feasible incoming path: the constant writes are per-path alternatives (0/1 flags, the script address
choices, the slot constants); `work1 = p[0x44A]` at the waypoint script re-loads the byte read for the
lim chain only on paths with wtype == 1 — on the feasible path wtype != 1 (p[0x39D] != 0 and
p[0x39E] != 1) work1 holds the stage base or the 0x449 flag there, and func_80057E84 (called on the
record path, gets p) may write p[0x44A] in between.
(C)(3) each value has load / arithmetic / call-result writes whose instructions are in the target;
the constants are per-branch alternatives selected by runtime conditions (Q20) inside value groups
(0/1 flags; 0x12..0x16 slots; the three script addresses), or a group's initial alternative beside
computed writes (100000 vs D_8009A838[stage] * 8; best's -1 vs score; lvl's 0 vs the byte loads; the
loop counters' 0 vs ++), as in func_80055138's admitted `temp` (`temp = 0` beside `D_800A37D2 / 3`).

| var | value | C write(s) | target write ($reg, address) |
|---|---|---|---|
| work1 $s1 | n449 | `work1 = p[0x449] == 0;` | sltiu 0x800587C0 |
| | base | `work1 = 100000;` / `= D_8009A838[CPU_S16(0xE)] * 8;` | lui/ori 0x80058EF4/EFC, sll 0x80058F10 |
| | kind | `work1 = p[0x44A];` | lbu 0x80059254 |
| | kind2 | `work1 = p[0x44A];` | lbu 0x800595B0 |
| | x1 | `work1 = CPU_S16(0x36A);` | lh 0x80059804 |
| | m445 | `work1 = p[0x445] == 0;` | sltiu 0x80059C04 |
| | bsel | `work1 = (s8)besti` | sra 0x8005A368 |
| | w | `work1 = D_8009A9F0[..][..];` `work1 >>= 4;` `work1 >>= ...;` | lw 0x8005A7A4, sra 0x8005A7C8, srav 0x8005A840 |
| | lo | `work1 = e[1] * 40;` `= -(et < 5) & 100000;` `+= work5 + ...;` | sll 0x8005A978, and 0x8005AA48, addu 0x8005AA8C |
| work2 $s2 | n445 | `work2 = p[0x445] == 0;` | sltiu 0x800587B8 |
| | sum | `work2 = D_8009A850[work4][2] * 16 + work1 + CPU_S16(0x40A);` | addu 0x80058F80 |
| | pace | `work2 = p[0x444];` | lbu 0x800595B8 |
| | y1 | `work2 = CPU_S16(0x36C);` | lh 0x8005980C |
| | m449 | `work2 = p[0x449] == 0;` | sltiu 0x80059C08 |
| | best | `work2 = -1;` `work2 = score;` (Q34 copy) | li 0x8005A108/A118, `addu $s2,$s3,$zero` 0x8005A350 |
| | cnt2 | `work2 = work1 & 0xF;` `work2--;` | andi 0x8005A7AC, addiu 0x8005A7D8 |
| | lvl | `= p[0x34D]` / `= 0` / `= p[0x34A]` / `= CPU_S16(0x330)` | lbu 0x8005ADCC, move 0x8005AE3C, lbu 0x8005AE40, lh 0x8005AE4C |
| work3 $s3 | side | `= CPU_OPP[0xAF] & 1;` `= !work3;` | andi 0x800586A0, sltiu 0x80058760 |
| | n447 | `= p[0x447] == 0;` | sltiu 0x800587B0 |
| | ang | `= CPU_S16(0x43A);` `= (work3 + 0x800) & 0xFFF;` `-= 0x1000;` | lh 0x80058A74, andi 0x80058A80, addiu 0x80058A90 |
| | prod | `= 0x1000 - (...);` `= (CPU_S16(0x438) * work3) >> 12;` | subu 0x80058D90, sra 0x80058DA8 |
| | force | `= 0;` / `= 1;` (Q20) | 0x80058E84 / 0x80058EAC |
| | idx | `= p[0x362] - 1;` | addiu 0x800595BC |
| | dist | `= SquareRoot0(..)` x3, `+= SquareRoot0(..)` x3 | 0x80059748/80, 0x800597F0, 0x80059858 |
| | farflag | `= lim < work3;` | slt 0x8005985C |
| | dang | `= (ratan2(..) - CPU_S16(0x1CA)) & 0xFFF;` `-= 0x1000;` | andi 0x80059974, addiu 0x80059984 |
| | coin | `= pick_weight[4] < (rand() & 0xFF);` `work3++` | lbu/slt 0x80059C34/3C, addiu 0x80059CC4 |
| | sel3 | `= CPU_S32(0x394);` / `= buf[..];` / `= buf2[..];` | lw 0x80059D0C, lbu 0x80059D60/DA4 |
| | slot | `= (u8)(D_800A38E2 / 10) * 2;` `work3--;` / `= 0x12..0x16` | sll 0x8005A6CC, addiu 0x8005A6F0, li 0x8005A6F8..A724 |
| | mask | `= D_8009A928[..][work3];` / `= 0; |= ...;` `= 1 << ..;` / `= D_8009A8C8[..][..].mask;` | lhu 0x8005A754, 0x8005A7A8, or 0x8005A7E0, sllv 0x8005A850, lhu 0x8005A878 |
| | cmask | `= q[4] << 24 | ... | q[1];` | or 0x8005A92C |
| | ok | `= 0;` / `= 1;` x6 (Q20) | 0x8005A998, 0x8005A9F0, 0x8005AB10, 0x8005AB7C, 0x8005ABB4 |
| | script4 | `= 0;` / `= (s32)D_8009A8C0` / `8B4` / `8AC` (Q20) | 0x8005AD08, lui/addiu 0x8005ADA4/A8, AF60/64, AF70/74 |
| work4 $s4 | i | `for (work4 = 0; ..; work4++)` | 0x80058F14, addiu 0x800590C0 |
| | n | `work4 = work3;` (Q34 copy) `work4 = work4 - 1;` | `addu $s4,$s3,$zero` 0x800596DC, 0x800597E0 |
| | k | `work4 = 0;` `work4++;` | 0x8005A638, addiu 0x8005AC44 |
| work5 $a1 | top | `work5 = work1 >> 27;` | sra 0x8005A7B4 |
| | adj | `work5 = (((0x1000 - lv) * 625) >> 10) - 400;` | addiu 0x8005AA84 |
| | kindet | `work5 = et;` (Q34 copy) | `addu $a1,$s5,$zero` 0x8005AA94 |

**Q34 copy values.** best (`work2 = score;`: score is a named local read again by the next compare,
no cast, target `addu $s2,$s3,$zero` 0x8005A350), n (`work4 = work3;`: work3 is read again by `if
(work3 == 0)`, target `addu $s4,$s3,$zero` 0x800596DC), kindet (`work5 = et;`: et is read again by
`row2[et]`; the s16 -> s32 conversion adds no instruction, target `addu $a1,$s5,$zero` 0x8005AA94).
One copy value per variable. The fresh local (r_best 23 words, r_n 7636, r_kindet 21) and the no-copy
bodies (pvs_work4b re-reads p[0x362] - 1: 7819; pvs_work5b tests et directly: 21) are measured above;
best has no no-copy form (the running maximum must be kept).

## (E)/(F)
Names are the generic `work1`..`work5` (Ruling 11 (E)); each declaration carries the inline comment
naming its values, citing the ruling and this file.
