# func_80055138 — Ruling 11 (D) proof for `temp` and `idx` (2026-09-26, manual slotF)

Ruling: .claude/rules/ordinary-c-judge-decidable.md § Ruling 11 (owner 2026-09-26; its
Application paragraph names this function's `v`, renamed `temp` here per (E)).

Neither variable is admitted by Rulings 5/6/8/9/10: `temp`'s six values feed different
consumers (a halfword store, a byte store, a table index, a mask test, min/max updates:
Ruling 5 1(a)) through different templates (1(b)); it is not a record pointer (Ruling 6);
its writes are not one meaning at constant offsets (Ruling 9); Ruling 8 is vmNoiseOn's;
there is no verified original source (Ruling 10). `idx` has two values (two loops) whose
writes are the constant 0 and `idx++` (Ruling 5 1(e) refuses a constant write; not a
record pointer, not BASE + K). So Ruling 11 governs both, each on its own.

## Bodies (all generated from ONE base; banked under memory/grind/func_80055138/r11/)
(The r11/ scripts are banked copies of tmp/func_80055138/r11/; they read cand.c and write
v/, rtl/ and the wf3 scratch TU under tmp/func_80055138/.)
- **reuse spelling** = memory/grind/func_80055138/candidate.c (= r11/cand.c, byte-for-byte),
  the body spliced into src/text1b.c at landing.
- **one-variable-per-value twin for `temp`** = r11/v/pv.c, made by r11/mk.py from cand.c
  with ONLY the carrier split: `temp`'s declaration (and its comment) removed; six fresh
  `s32` locals `lvl5`, `lvl3`, `row_idx`, `move_mask`, `stat1`, `stat2`, each declared at
  the innermost block enclosing its writes (`if (D_800A389A) {`, the `else {`, the switch
  body, `if (e[4] == 0x40) {`, `if (e[1] != 0 && e[1] != 0xFF) {`,
  `if (e[2] != 0 && e[2] != 0xFF) {`); every write and read of each value renamed.
- **one-variable-per-value twin for `idx`** = r11/v/ctr_split.c: one added declaration
  `s32 zero_i;` and the zero loop's three `idx` renamed `zero_i`.
- (C)(2) receipt, r11/stmtcheck.py (strips comments, deletes the carrier/value
  declarations, renames each value back, compares): `cand.c` vs `pv.c` -> "IDENTICAL
  statement lists"; `cand.c` vs `ctr_split.c` -> "IDENTICAL statement lists".

## `temp` — the six values (no read can reach two of them)
| value | C writes | target (asm/funcs/func_80055138.s) |
|---|---|---|
| 1 case 2, D_800A389A set: level `D_800A37D2 / 5` | `temp = D_800A37D2 / 5;` | `andi a2,v0,0xff` 0x800552E4; read 0x800552E8/EC |
| 2 case 2 else: practice level `D_800A37D2 / 3`, 0 once it reaches 3 | `temp = D_800A37D2 / 3;` `temp = 0;` (both reach `(temp + 2) << 10`) | `andi a2,v0,0xff` 0x80055374; `move a2,zero` 0x8005538C; reads 0x80055378..0x800553A0 |
| 3 case 3: row in D_8009A9B4 | `temp = (u8)(D_800A38E2 / 10) * 2;` `temp--;` | `sll a2,v0,1` 0x800554E4; `addiu a2,a2,-1` 0x80055504 |
| 4 scan loop: the move entry's byte-assembled character mask | `temp = (e[8] << 24) \| ... \| e[5];` | `or a2,v0,v1` 0x80055738; read `and v0,a2,t9` 0x8005573C |
| 5 scan loop: stat byte e[1] | `temp = e[1];` | `move a2,v0` 0x8005575C; reads 0x80055760, 0x8005576C |
| 6 scan loop: stat byte e[2] | `temp = e[2];` | `move a2,v0` 0x80055784; reads 0x80055788..0x800557DC |

(A) fresh `s32` local of this function, declared once at function scope (its writes span
the switch and the scan loop), no `&temp`. (B)(1) every write is read before the next
write (table above). (B)(2) no write stores the value temp already holds: 2's `temp = 0`
runs only when temp >= 3; 5 and 6 load different lvalues (`e[1]`, `e[2]`; `e` is
recomputed each iteration). (C)(3) each value has a load / arithmetic write whose
instructions are in the target (multu/mfhi/srl/andi for 1-3, lbu/sll/or for 4, lbu for 5-6).

## `idx` — the two values
1 the byte index of the clear loop `for (idx = 0; idx < 8U; idx++) (p + idx)[0x444] = 0;`
(t4: `addu t4,zero,zero` 0x80055634/0x80055664, `addu v0,s0,t4` 0x80055668, `addiu t4,t4,1`
0x80055670, `sltiu v0,t4,8` 0x80055674); 2 the player index of `for (idx = 0; idx < 2; idx++)`
(t4: `addu t4,zero,zero` 0x80055684, `beqz t4` 0x80055690, `bnez t4` 0x800556F4, `addiu
t4,t4,1` 0x80055874, `slti v0,t4,2` 0x80055878). (A) fresh `s32`, function scope, no `&idx`.
(B)(1) each write is read by its loop test. (B)(2) the second `idx = 0` runs when idx == 8.
(C)(3) `idx++` is arithmetic in the target (addiu above).

## (D)(1) Dumps and command lines
Build model: r11/model.py `score` builds a scratch copy of src/text1b.c with the body
spliced and the landing's header/consumer edits applied (the same edits `model.py apply`
makes to the tree), with the build's pipeline (engine.pipeline.c_pipeline_cmd), and scores
vs build/src/text1b.o. Dumps: `bash tmp/func_80055138/r11/dump.sh <tag> <body.c>` =
cpp of that scratch TU (engine.buildconfig CPP_DEFS) -> tools/rtl_track/dump.py (build cc1
tools/gcc-2.7.2/build/cc1 `-da`: .rtl/.cse/.cse2/.flow/.lreg/.greg) -> the instrumented
tools/gcc-2.7.2/cc1 with `BB2_ALLOC_DEBUG=1` (FINDREG: r11/findreg.sh, `BB2_FINDREG_DEBUG=<pseudo>`).
Flags (engine.buildconfig CC_FLAGS): `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -mel -msoft-float`. Identity check (dump.sh): the
instrumented cc1's func_80055138 asm == the build cc1's for every dumped body ("IDENTITY
OK"). Whole run: r11/all.sh -> r11/all.log. Tags: `cand` = v/cand.c (== candidate.c),
`pv` = v/pv.c, `ctr` = v/ctr_split.c, `nostage` = v/nostage_fs.c.
Tree check, second landing (2026-09-26, landing edits applied under the lock). Other
workers' header edits landed between the first and second landing, so the preprocessed
landed text1b.c is no longer byte-identical to rtl/cand.i (the earlier "tree == dumped TU"
check, and the same claim in the ledger commit before this one, held only at the first
landing). Re-dumped instead: r11/dump_landed.sh dumps the landed src/text1b.c itself (tag
`landed`) and r11/redump_tree.ps1 the landed tree with each body spliced by the sandbox
(tags tcand, tpv, tctr); r11/cmp_dumps.py: Register lines, dispositions, ALLOCDBG and the
function's asm IDENTICAL to cand / pv / ctr, and the FINDREG traces for pseudos 99, 89
(landed, tcand), 188, 106 (tpv) and 90 (tctr) IDENTICAL. The excerpts below therefore are
the landed TU's.

Pseudo map, from the RTL sets (verbatim below): cand — 99 = temp, 89 = idx, 103 = hi2_val,
158 = case 2's srl result, 417/424 = the e[1]/e[2] byte loads. pv — 158 = lvl5 (159 = its
srl input), 188 = lvl3, 106 = row_idx, 404 = move_mask (415/417 = its `or` inputs),
424 = stat1, 432 = stat2, 89 = idx. ctr — 90 = zero_i, 89 = idx.

Reuse spelling (`cand`), .lreg verbatim:
```
Register 89 used 21 times across 118 insns; dies in 0 places; GR_REGS or none.
Register 99 used 53 times across 49 insns; dies in 6 places; GR_REGS or none.
Register 103 used 21 times across 20 insns; GR_REGS or none.
Register 158 used 2 times across 2 insns in block 14; GR_REGS or none.
Register 417 used 16 times across 4 insns; GR_REGS or none.
Register 424 used 16 times across 4 insns; GR_REGS or none.
```
.greg dispositions (the lines holding these pseudos), verbatim:
```
85 in 2  86 in 2  87 in 2  89 in 12  90 in 15  91 in 13  
98 in 10  99 in 6  100 in 3  101 in 5  102 in 4  103 in 3  
410 in 4  411 in 4  412 in 2  414 in 3  415 in 2  417 in 2  
420 in 17  422 in 2  424 in 2  429 in 2  430 in 2  432 in 2  
```
ALLOCDBG (global.c allocation order), verbatim:
```
ALLOCDBG func=func_80055138 ord=0 pseudo=417 hardreg=2 nrefs=16 livelen=4 pri=160000
ALLOCDBG func=func_80055138 ord=1 pseudo=424 hardreg=2 nrefs=16 livelen=4 pri=160000
ALLOCDBG func=func_80055138 ord=4 pseudo=99 hardreg=6 nrefs=53 livelen=49 pri=54081
ALLOCDBG func=func_80055138 ord=5 pseudo=103 hardreg=3 nrefs=21 livelen=20 pri=42000
ALLOCDBG func=func_80055138 ord=24 pseudo=89 hardreg=12 nrefs=21 livelen=118 pri=7118
```
FINDREG, verbatim:
```
FINDREGDBG func=func_80055138 pseudo=99 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 4 5 29
FINDREGDBG  someone_prefers: 4
FINDREGDBG  used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 24 25 26 27 28 29 31
FINDREGDBG  pass0_used: 0 1 2 3 4 5 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG  own_copy_prefs:
FINDREGDBG  own_full_prefs:
FINDREGDBG  pass1_used: 0 1 2 3 4 5 26 27 28 29 31
FINDREGDBG  used2_noconflict: 0 1 26 27 28 29 31
FINDREGDBG  class=1 mode=4 size=1
FINDREGDBG func=func_80055138 pseudo=89 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 4 5 6 7 8 9 10 11 16 29
FINDREGDBG  someone_prefers:
FINDREGDBG  used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 24 25 26 27 28 29 31
FINDREGDBG  pass0_used: 0 1 2 3 4 5 6 7 8 9 10 11 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG  own_copy_prefs:
FINDREGDBG  own_full_prefs:
FINDREGDBG  pass1_used: 0 1 2 3 4 5 6 7 8 9 10 11 16 26 27 28 29 31
FINDREGDBG  used2_noconflict: 0 1 26 27 28 29 31
FINDREGDBG  class=1 mode=4 size=1
```
temp's writes in cand.lreg (verbatim, abridged to the value writes):
```
(insn 183 182 187 (set (reg/v:SI 99) (zero_extend:SI (subreg:QI (reg:SI 158) 0))) 118 {zero_extendqisi2} (insn_list 182 (nil)) (expr_list:REG_DEAD (reg:SI 158) (nil)))
(insn 872 868 874 (set (reg/v:SI 99) (ior:SI (reg:SI 412) (reg:SI 414))) 96 {iorsi3} (insn_list 868 (insn_list 871 (nil))) (expr_list:REG_DEAD (reg:SI 412) (expr_list:REG_DEAD (reg:SI 414) (nil))))
(insn 901 898 904 (set (reg/v:SI 99) (reg:SI 417)) 159 {movsi_internal2} (nil) (expr_list:REG_DEAD (reg:SI 417) (nil)))
(insn 930 927 933 (set (reg/v:SI 99) (reg:SI 424)) 159 {movsi_internal2} (nil) (expr_list:REG_DEAD (reg:SI 424) (nil)))
```
cand.inst.fn.s at the six sites: `andi $6,$2,0x00ff` (1), `andi $6,$2,0x00ff` (2),
`or $6,$2,$3` (4), `move $6,$2` twice (5, 6) — $6 = $a2, as in the target.

One-variable-per-value twin (`pv`, sandbox-equivalent score 102/516, 512 insns), .lreg verbatim:
```
Register 106 used 4 times across 9 insns; GR_REGS or none.
Register 158 used 3 times across 3 insns in block 14; GR_REGS or none.
Register 159 used 2 times across 2 insns in block 14; GR_REGS or none.
Register 188 used 6 times across 13 insns; GR_REGS or none.
Register 404 used 8 times across 2 insns in block 48; GR_REGS or none.
Register 415 used 8 times across 2 insns in block 48; GR_REGS or none.
Register 417 used 8 times across 3 insns in block 48; GR_REGS or none.
```
(no Register line for 424 or 432: both were deleted before local-alloc, see (D)(2) c.)
.greg dispositions, verbatim:
```
85 in 2  86 in 2  87 in 2  89 in 11  90 in 14  91 in 12  
104 in 2  105 in 3  106 in 5  108 in 2  109 in 2  110 in 3  
153 in 3  154 in 3  155 in 2  157 in 2  158 in 3  159 in 3  
188 in 4  189 in 2  190 in 3  192 in 64  193 in 2  194 in 2  
398 in 5  402 in 3  403 in 2  404 in 2  406 in 2  407 in 2  
409 in 3  410 in 3  411 in 2  413 in 4  414 in 4  415 in 2  
417 in 3  418 in 2  420 in 3  423 in 25  426 in 2  428 in 3  
```
ALLOCDBG, verbatim:
```
ALLOCDBG func=func_80055138 ord=22 pseudo=188 hardreg=4 nrefs=6 livelen=13 pri=9230
ALLOCDBG func=func_80055138 ord=23 pseudo=106 hardreg=5 nrefs=4 livelen=9 pri=8888
ALLOCDBG func=func_80055138 ord=25 pseudo=89 hardreg=11 nrefs=21 livelen=116 pri=7241
```
FINDREG, verbatim:
```
FINDREGDBG func=func_80055138 pseudo=188 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 16 29
FINDREGDBG  someone_prefers:
FINDREGDBG  used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 24 25 26 27 28 29 31
FINDREGDBG  pass0_used: 0 1 2 3 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG  own_copy_prefs:
FINDREGDBG  own_full_prefs:
FINDREGDBG  pass1_used: 0 1 2 3 16 26 27 28 29 31
FINDREGDBG  used2_noconflict: 0 1 26 27 28 29 31
FINDREGDBG  class=1 mode=4 size=1
FINDREGDBG func=func_80055138 pseudo=106 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 4 16 29
FINDREGDBG  someone_prefers:
FINDREGDBG  used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 24 25 26 27 28 29 31
FINDREGDBG  pass0_used: 0 1 2 3 4 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG  own_copy_prefs:
FINDREGDBG  own_full_prefs:
FINDREGDBG  pass1_used: 0 1 2 3 4 16 26 27 28 29 31
FINDREGDBG  used2_noconflict: 0 1 26 27 28 29 31
FINDREGDBG  class=1 mode=4 size=1
```
pv's writes (in.i.rtl / pv.lreg, verbatim):
```
(insn 184 183 188 (set (reg/v:SI 158) (zero_extend:SI (subreg:QI (reg:SI 159) 0))) 118 {zero_extendqisi2} (insn_list 183 (nil)) (expr_list:REG_DEAD (reg:SI 159) (nil)))
(insn 259 258 262 (set (reg/v:SI 188) (zero_extend:SI (subreg:QI (reg:SI 189) 0))) 118 {zero_extendqisi2} (insn_list 258 (nil)) (expr_list:REG_DEAD (reg:SI 189) (nil)))
(insn 465 462 478 (set (reg/v:SI 106) (ashift:SI (reg:SI 275) (const_int 1))) 181 {ashlsi3} ...
(insn 878 874 880 (set (reg/v:SI 404) (ior:SI (reg:SI 415) (reg:SI 417))) 96 {iorsi3} (insn_list 874 (insn_list 877 (nil))) (expr_list:REG_DEAD (reg:SI 415) (expr_list:REG_DEAD (reg:SI 417) (nil))))
```
pv.inst.fn.s at the sites: `andi $3,$3,0x00ff` (1: output = input), `andi $4,$2,0x00ff`
(2: $a0), `or $2,$2,$3` (4: output = first input), and no copy for 5/6 (the tests and the
min/max updates read the load register: `slt $2,$3,$7` / `move $7,$3`).

stat1/stat2 through the passes (r11/passtrack.py; rtl/pv.stat_passes.txt), verbatim:
```
== pv.rtl: 8 insns
(insn 909 908 912 (set (reg/v:SI 424) (zero_extend:SI (reg:QI 425))) -1 (nil) (nil))
(insn 912 909 913 (set (reg:SI 426) (lt:SI (reg/v:SI 424) (reg/v:SI 96))) -1 (nil) (nil))
(insn 917 915 919 (set (reg/v:SI 96) (reg/v:SI 424)) -1 (nil) (nil))
(insn 940 939 943 (set (reg/v:SI 432) (zero_extend:SI (reg:QI 433))) -1 (nil) (nil))
(insn 943 940 944 (set (reg:SI 434) (lt:SI (reg/v:SI 97) (reg/v:SI 432))) -1 (nil) (nil))
(insn 948 946 950 (set (reg/v:SI 97) (reg/v:SI 432)) -1 (nil) (nil))
(insn 958 955 959 (set (reg:SI 437) (lt:SI (reg/v:SI 98) (reg/v:SI 432))) -1 (nil) (nil))
(insn 982 980 984 (set (reg/v:SI 98) (reg/v:SI 432)) -1 (nil) (nil))
== pv.cse: 4 insns
(insn 940 936 943 (set (reg/v:SI 432) (reg:SI 428)) 159 {movsi_internal2} (nil) (nil))
(insn 943 940 944 (set (reg:SI 434) (lt:SI (reg/v:SI 97) (reg/v:SI 432))) 258 {slt_si} (nil) (nil))
(insn 948 946 950 (set (reg/v:SI 97) (reg/v:SI 432)) 159 {movsi_internal2} (nil) (nil))
(insn 958 955 959 (set (reg:SI 437) (lt:SI (reg/v:SI 98) (reg/v:SI 432))) 258 {slt_si} (nil) (nil))
== pv.cse2: 1 insns
(insn 940 936 943 (set (reg/v:SI 432) (reg:SI 428)) 159 {movsi_internal2} (nil) (nil))
== pv.flow: 0 insns
```
The same insns in cand keep temp through cse, cse2 and flow (rtl/cand.stat_passes.txt), e.g.
`(insn 901 898 904 (set (reg/v:SI 99) (reg:SI 417)) 159 {movsi_internal2} (nil) (nil))`
and `(insn 904 901 905 (set (reg:SI 422) (lt:SI (reg/v:SI 99) (reg/v:SI 96))) 258 {slt_si} (nil) (nil))`
in cand.cse2.

Counter twin (`ctr`, 25/516), verbatim:
```
Register 89 used 12 times across 112 insns; dies in 0 places; GR_REGS or none.
Register 90 used 9 times across 6 insns; dies in 0 places; GR_REGS or none.
85 in 2  86 in 2  87 in 2  89 in 15  90 in 3  91 in 14  
ALLOCDBG func=func_80055138 ord=5 pseudo=90 hardreg=3 nrefs=9 livelen=6 pri=45000
ALLOCDBG func=func_80055138 ord=28 pseudo=89 hardreg=15 nrefs=12 livelen=112 pri=3214
FINDREGDBG func=func_80055138 pseudo=90 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 29
FINDREGDBG  someone_prefers:
FINDREGDBG  used_so_far: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 24 25 26 27 28 29 31
FINDREGDBG  pass0_used: 0 1 2 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG  own_copy_prefs:
FINDREGDBG  own_full_prefs:
FINDREGDBG  pass1_used: 0 1 2 26 27 28 29 31
FINDREGDBG  used2_noconflict: 0 1 26 27 28 29 31
FINDREGDBG  class=1 mode=4 size=1
```

## (D)(2) Mechanism
a. **local-alloc (values 1 and 4).** local-alloc.c:470-478 gives a pseudo `reg_qty = -2`
   (allocate locally) only when `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`, i.e. it
   is referenced in ONE basic block and dies once; otherwise `-1` (left to global.c).
   block_alloc (:1123) then tries to tie each insn's output to an input that dies there
   (:1215-1300 -> combine_regs :1784); for two pseudos combine_regs refuses when the output
   already has a quantity or is not locally allocatable (:1904-1905 `reg_qty[sreg] >= -1`),
   and otherwise adds the output to the dying input's quantity (:1916-1928), so both get
   the same hard register. In the reuse spelling temp (99) is referenced in several blocks
   ("used 53 times across 49 insns; dies in 6 places", no "in block") -> -1 -> never tied;
   the case-2 srl result (158, "in block 14") dies at insn 183 into $v0 while temp is
   global. In pv, lvl5 (158) and move_mask (404) are single-block ("in block 14", "in
   block 48") and are tied to their dying inputs (158/159 both in 3; 404/415 both in 2).
   The target writes both values into $a2 while the input dies elsewhere ($v0), i.e. from
   an untied, global pseudo.
b. **global.c find_reg (values 2 and 3; the seat $a2 for all six).** find_reg (:952)
   takes, in pass 0, the lowest hard register in regs_used_so_far that is not in the
   allocno's conflicts nor in regs_someone_prefers (
   lowest non-conflicting one. temp's allocno spans all six values, so its conflicts are
   the union over their ranges: 2 3 4 5 29, plus someone_prefers 4 -> pass 0 -> 6 ($a2).
   ($a0 conflicts through case 2, where the 0xCCCCCCCD constant stays in $a0 across value 1
   for the second multu at 0x80055320; $a1 through the scan loop, where the entry pointer
   `e` is in $a1 across values 4-6.) Split, lvl3's allocno conflicts only with 2 3 16 29 ->
   $a0; row_idx's with 2 3 4 16 29 -> $a1.
c. **cse.c make_regs_eqv (values 5 and 6).** cse.c:826-857 makes a new register the
   canonical head of its equivalence class only if it is a pseudo living beyond the current
   cse block (`regno_last_uid[new] > cse_basic_block_end` or `regno_first_uid[new] <
   cse_basic_block_start`) and longer than the current head; otherwise the older register
   stays canonical and the copy's reads are replaced by it. In cand, temp's first
   reference is insn 183 (case 2), before every scan-loop cse block, and it outlives the
   byte loads, so temp stays canonical: the reads use temp, the copies `move a2,v0`
   survive. In pv, stat1 (424) is replaced in cse and deleted, stat2's (432) reads are
   replaced in cse2 and its dead set deleted by flow (excerpts above).
d. **the counter.** idx (89) is live across the whole scan loop, so its conflicts are
   2..11, 16, 29 -> pass 0 -> 12 ($t4), the target's register for both loops. Split, the
   clear loop's counter (90) conflicts only with 2 29 -> $v1.

## (D)(3) Necessity — every value of `temp`, every one-variable-per-value spelling
(Rewritten after the 2026-09-26 layer-2 FAILs; third revision adds own preferences (G2),
local seats (L) and every post-allocation rewrite. The argument is made per value, and it covers
per-value spellings with ANY added sanctioned construct — FAKE dead store, self-assignment,
combine-foldable chain-extender / live-use detour, pointer alias, `do { } while (0);` —
because it constrains only what the allocators can choose given the instructions a matching
spelling must emit. The measurements in (D)(3b) confirm it construct by construct.)

**Frame (checked line by line against tools/gcc-2.7.2, third revision).**

*What the allocators see vs. what is emitted.* After global allocation, rest_of_compilation
(toplev.c) runs, in order: reload (:3082), which replaces each pseudo by the hard register it
was given and deletes moves whose two sides got the same register (a pseudo left without one
would be spilled to the stack; the target keeps every value of temp in a register, and its
frame saves only $s0-$s5/$ra, so no spill is admissible); sched2 (:3117), which reorders
instructions inside blocks; the second jump pass (:3142, `jump_optimize (insns, 1, 1, 0)`:
cross_jump = 1, noop_moves = 1), which deletes no-op moves and cross-jumps identical tails
into their join; and delay-slot filling (dbr_schedule, :3167), which moves or copies instructions into
delay slots (MACHINE_DEPENDENT_REORG, :3158, is not defined for MIPS). After cc1 the build's
prologue_fix / maspsx / multu_pad adjust the prologue order, expand macros and insert
load-delay nops; they do not reassign these operands.
None of these changes which hard register an operand uses. So (i) the output's
register usage is the allocation-time assignment, and every emitted instruction existed at
allocation time; (ii) the allocation-time stream may additionally hold moves later deleted as
no-ops, and duplicate tails later merged. Both extras are covered below: a no-op move is a
pairing like any other (own preferences, below), and a duplicated tail is followed by the
same code as the merged copy, so its liveness — and hence its conflicts — equals the merged
copy's (measured: dup_* rows).

*Local-alloc* (local-alloc.c, before global). A pseudo referenced in one basic block that dies
once gets a quantity (:470-478). block_alloc ties an insn's output to an input that dies there
(combine_regs :1784, tie :1916-1928): the output then shares the input's register. An untied
quantity is placed by find_free_reg (:2135): `used` = fixed_reg_set (call_used_reg_set if it
crosses a call) plus every register marked live in regs_live_at over its life — hard
registers live there and locals already placed (post_mark_life); global pseudos are not yet
assigned and occupy nothing. It is first restricted to the quantity's suggested registers
(qty_phys_copy_sugg / qty_phys_sugg), which combine_regs records only from an insn of the
block that pairs it with a HARD register (:1858-1898); otherwise the lowest register not in
`used` wins (no REG_ALLOC_ORDER on MIPS).

*Global* (global.c). global_conflicts (:777) calls mark_reg_store, which calls set_preference
(:1484 -> :1671) for every SET: when one side (the destination, or the source or its first
operand) is a global allocno and the other a hard register or a locally allocated pseudo
(reg_renumber), the allocno gets that register in its OWN hard_reg_preferences (and
hard_reg_copy_preferences for a plain copy, and hard_reg_full_preferences). expand_preferences
(:824-871, called at :552) then merges the preferences of two non-conflicting allocnos when
one dies in an insn that sets the other. prune_preferences (:882-930, called at :577) removes
from each allocno's own preferences its conflicts and, if it crosses a call, every call-used
register (:902), and builds regs_someone_prefers from lower-priority conflicting allocnos.
find_reg (:952) scans for best_reg: pass 0 = the lowest register not in
`used` = fixed/call-used ∪ conflicts ∪ ¬regs_used_so_far ∪ regs_someone_prefers; if none,
pass 1 = the lowest register not in used1 = fixed/call-used ∪ conflicts. It then REPLACES
best_reg by the lowest register of the allocno's own copy preferences not in `used`
(:1097-1131), else of its own preferences not in `used` (:1133-1166). (`used`/used1 also carry no_global_alloc_regs, the `losers` of a reload retry and
the complement of the allocno's class, :970-983. The caller-saves retry after :1166 runs only
when no register was found at all, which never happens here: $s6, $s7 and $fp stay free in
the target.)

*Therefore* a separate pseudo V that holds one value of temp ends in $a2 (6) only by one of:
- (L) local-alloc: tied to an input that is in $a2, or suggested $a2, or with $v0..$a1 (2-5)
  all in `used` over its life and $a2 not;
- (G1) find_reg's scan: 2-5 all excluded (conflict, preference of another, or not yet used)
  and 6 not;
- (G2) own preference: 6 in V's own copy preferences or preferences after prune, and 6 not
  in `used`.
Each value is checked against all three below.

**Target facts** (asm/funcs/func_80055138.s; r11/pairs.py prints the pairings):
- $a1 holds a live value only: the incoming arg1 up to its copy (0x80055140), while $a2 still
  holds the incoming arg2 up to 0x800551A4; `lbu a1,0x443(s0)` 0x800555F4..0x80055610; `e`
  across the scan loop (set at 0x800557F0); lo_val in the section tail 0x80055804..0x80055854.
  `jal file_GetFlag1` (0x80055530) lies on every path from the switch arms to 0x800555F4.
- Instructions pairing $a1 with another register (`python3 tmp/func_80055138/r11/pairs.py a1`,
  r11/pairs_a1.txt): the arg1 copy `addu s2,a1,zero` (0x80055140), `sll v0,a1,1` (0x8005560C,
  after the call), `addu a1,v0,t8` (0x800557F0, `e`) and `addu a1,v1,v0` (0x8005582C, tail).
- In the scan loop $a0 is live only 0x8005571C..0x80055734 (`lbu a0,6(a1)` -> `or v0,v0,a0`);
  $v1 only 0x80055704..0x8005570C, 0x80055718..0x80055728, 0x80055730..0x80055738 (the mask's
  inputs) and 0x800557A0..0x800557D4 (`cat`). The loop instructions pairing $v1 or $a0 with
  another register are only `andi v1,v0,7` (0x800557A0) and `sltiu v0,v1,2` (0x800557C8):
  `cat` with $v0 values. None pairs a loop-live allocno with $a0.

**(G2) own preferences and (L) local seats — every value.** In the output, $a2 is written
only by the instructions of temp's six values; its only other content is the incoming arg2,
copied out at 0x800551A4 (`python3 tmp/func_80055138/r11/pairs.py a2`, r11/pairs_a2.txt:
every $a2 pairing is a value of temp with $v0/$v1/$t0-$t2, plus `addu s4,a2,zero`).
Hard $a2 appears in the allocation-time RTL only as that entry copy: no call in the
function takes a third argument (func_8005509C takes one, file_GetFlag1 and rand none) and
results return in $v0.
- (L), every value. Value 1's and value 4's writes tie to their dying inputs, whose register
  holds no value of temp and so is not $a2 (target: $v0). For any value placed by
  find_free_reg: a suggestion of $a2 needs an insn of its block pairing it with hard $a2 —
  only block 0 has one, and no value of temp is written there; and lowest-free reaches $a2
  only if $v0..$a1 are all in `used` over its life, but $a1 is never in `used` in a block
  holding a value of temp: case 2/3 blocks hold no $a1 value at all, and in the scan loop
  $a1's only value is the global `e`, which occupies nothing at local-alloc time. So no value
  of temp, nor any local copy of one (a copy lies in the value's range, hence in such a
  block), is locally placed in $a2.
- (G2), every value. An allocno gets $a2 into its own preferences only (a) from a SET pairing
  it with hard $a2 or a local placed in $a2 (set_preference :1671-1745), or (b) by
  expand_preferences (:824-871) from an allocno W that has it. For (a): the hard case is the
  entry copy, whose allocno is arg2's pseudo, not a value of temp (a FAKE dead store of arg2
  into a value's variable is deleted by flow before allocation — Ruling 2's store-level
  deadness; measured ce_l_dead_* / ce_m_dead_goto); the local case needs a local in $a2,
  which (L) excludes wherever a value of temp lives, and elsewhere such a local would write
  $a2 in a block where the target has no $a2 instruction, unless its writes are no-op moves
  from another register in $a2, which bottoms out in the same two sources. A move deleted
  after allocation as a no-op (reload / jump2 noop_moves) is such a pairing and adds nothing
  beyond this. For (b): the only allocno with $a2 from (a) is arg2's pseudo; expand links it
  only through an insn that sets one allocno while the other dies there; arg2's pseudo is set
  only by the entry copy and read only at 0x80055694 (`cursor = arg2`), inside the player
  loop, where it stays live around the back edge (no REG_DEAD) — so it links to nothing, and
  a fresh local copied from arg2 is cse-merged into it (ce_l_arg1alias_glob shows the same
  for arg1). So no allocno holding a value of temp has $a2 in its own preferences; :902 also
  strips it from any call-crossing one. Traces: own_copy_prefs and own_full_prefs are EMPTY
  for every per-value allocno dumped — value 1 (ce_l_chain_callarg 158, ce_l_arg1alias_glob
  159, ce_l_entry_uninit 89), value 2 (tpv 188), value 3 (tpv 106), value 4
  (ce_m_chain_goto2 404), value 6 (abl_stat2_dw 428: `conflicts: 2 29`, `someone_prefers:`
  empty, `own_copy_prefs:` / `own_full_prefs:` empty -> $v1), the counter (tctr 90) — and for
  temp and idx in the reuse (tcand 99, 89). Value 5 has no surviving allocno in any measured
  per-value spelling (its copy is deleted by cse; abl_stat1_keep / _keep2 with a do-while and
  a chain-extender still lose it), and the argument above covers it when it does survive.
With (L) and (G2) closed, only (G1) remains, argued per value next.

**Case values — 1 (D_800A37D2 / 5), 2 (D_800A37D2 / 3), 3 (the D_8009A9B4 row).** Each needs
$a1 excluded. No conflict can do it: the only $a1 values are arg1 at entry — a range reaching
it starts before any write, i.e. reads an indeterminate value, and also overlaps arg2 in
$a2, so $a2 is excluded with it (measured ce_l_entry_uninit: `conflicts: 2 3 4 5 6 16 29`
-> $a3) — and values after `jal file_GetFlag1`, which a case-2/3 range reaches only by
crossing the call, and then $a2 is excluded as a call-used register. No preference can do it:
of the $a1 pairings, the arg1 copy gives only arg1's pseudo an $a1 preference, and that
pseudo crosses calls (it is read at 0x80055680 after `jal rand`), so :902 prunes it (a fresh
local initialised from arg1 is cse-merged into it: ce_l_arg1alias_glob -> still $a1); the
other three lie after `jal file_GetFlag1`, and an allocno they involve can conflict with a
case value only by being live across that call, which again prunes its call-used
preferences. A local
seat is no better: value 1 as a single-block local is tied to its dying input
(`andi X,X,0xff`), and a local's lowest free register in case 2/3 is below $a2 because $a1 is
free there. Measured per-value seats: value 1 tied, or $a1 when a surviving chain-extender
makes it global; value 2 $a0; value 3 $a1 (FINDREG traces in (D)(1) and (D)(3b)). So no
one-variable-per-value spelling, with or without added constructs, seats 1, 2 or 3 in $a2.
The reuse gets $a1 excluded from its loop segments: temp's one pseudo also holds values
4-6, whose ranges overlap `e` in $a1, and it crosses no call ("Register 99 used 53 times
across 49 insns; dies in 6 places", no "crosses").

**Loop values — 4 (the mask word), 5 (e[1]), 6 (e[2]).** Each needs $a0 excluded, and 4 and 5
also $v1. A conflict needs the range to include a point where $a0 (or $v1) holds a live value.
The mask's range 0x80055738..0x8005573C, stat1's 0x8005575C..0x8005576C and stat2's
0x80055784..0x800557DC (which does contain `cat` in $v1, so $v1 is excluded for value 6)
contain none, and extending a range to one crosses another loop value's range: every path
from the range to a later $v1/$a0 value either enters stat1's or stat2's range (the next $v1
value after the mask on the fall-through path is `cat`, inside stat2's range) or goes around
the loop back-edge; a variable live at the loop head is live into block 0x800557E0, i.e. out
of the stat blocks, and so across the stat values' ranges (for stat2 reaching the
0x8005571C..34 $a0 value: live along the skip path 0x80055770 -> 0x800557E0 and so across
stat1's range). In a matching spelling those values sit in $a2 too, and two pseudos
whose ranges overlap cannot share $a2 — so such an extension breaks the match itself. No
preference can do it: no emitted loop instruction pairs a loop-live allocno with $a0, and the
$v1 pairings involve only `cat` and the e[0] load, which live inside stat2's range. Pass 1
therefore yields $a0 or lower (the mask and stat1: $v1 or lower). A local seat is no better:
the mask as a single-block local is tied (`or X,X,Y`), and $a0 is free in all three ranges.
Values 5 and 6 need, besides, their copy `move a2,v0` to survive cse: with its own variable,
cse.c make_regs_eqv (:826-857) keeps the byte load canonical unless the variable outlives
the cse block, so the plain per-value spellings lose the copy (dumps in (D)(1)); the
permuter's `do { } while (0);` keeps stat2's copy by ending the cse block, and then stat2
lands in $v1 (x/abl_stat2_dw.c: 11/516, 516 insns). So no one-variable-per-value spelling
seats 4, 5 or 6 in $a2. The reuse gets $v1 and $a0 excluded from its case segments (value
2's range holds the n*15 temporary in $v1, 0x8005539C..0x800553A0; value 1's holds the
0xCCCCCCCD constant in $a0) and from regs_someone_prefers (trace: `someone_prefers: 4`).

So every value needs the shared pseudo: the case values take $a1's exclusion from the loop
values' ranges, the loop values take $v1's and $a0's from the case values' ranges, and no
one-variable-per-value spelling — whatever its declarations, scope, types, statement order
or added sanctioned constructs — gets either.

## (D)(3c) `idx`
Property: one pseudo live across the scan loop, whose conflicts fill 2..11 (d).
  In ANY one-variable-per-value spelling the clear loop has its own counter, referenced
  only by that loop (init, `(p + zero_i)[0x444]`, `zero_i++`, the test). In any spelling
  that reproduces the rest of the target, the only registers live with it are $v0 (the
  byte address, `addu v0,s0,t4`) and the callee-saved/fixed ones ($s0 = p, $s2/$s4 = the
  two list parameters). $v1..$t3 are dead across the clear loop in the target ($v1 is last
  read at 0x8005565C and next written at 0x800556C8; $a0/$a1 are last read at
  0x80055650/0x80055610; $a2 is last read in the switch at 0x80055508; $a3 and $t0-$t3
  are first written after the loop), so none
  of 3..11 is a conflict. All of 3..11 are in regs_used_so_far (local-alloc uses them
  throughout the function; trace: `pass0_used: 0 1 2 16 ...`, i.e. 3..15 usable). Pass 0
  returns the lowest of them not in regs_someone_prefers, and pass 1 the lowest
  non-conflicting one, $v1; $t4 (12) could come back only if every one of 3..11 were
  preferred by an allocno conflicting with the counter, and the only allocnos live with it
  are p and the two list parameters, which have no copy preference for those registers
  (someone_prefers is empty in the trace). So the split counter is never seated in $t4
  (measured: $v1).
  (L) and (G2), as in the Frame above: in the output $t4 holds only the two counters
  (`addu t4,zero,zero`, `addu v0,s0,t4`, `addiu t4,t4,1`, `sltiu v0,t4,8`, `beqz`/`bnez t4`,
  `slti v0,t4,2`); hard $t4 never appears in the RTL (it is no argument or return register),
  so an own preference for $t4 would need a pairing with a local placed in $t4, and a local
  reaches $t4 by lowest-free only with $v0..$t3 all in `used`, which the clear loop and its
  preheader do not have ($v1..$t3 dead there, listed above). The trace's `own_copy_prefs:` and
  `own_full_prefs:` lines are empty (tctr 90, verbatim in (D)(1)). The split counter spans the
  loop's blocks, so it is never local.

## (D)(3b) Sanctioned-construct escapes, measured
Each variant is v/pv.c (or v/ctr_split.c) plus ONE construct (r11/mk3.py; the entry, alias,
goto2, after, bit, rec, e, tail and do-while variants were one-off edits of the same bases;
the banked r11/v/ce_*.c and x/*.c files are exactly the measured bodies). Every row was
sandboxed in the tree under the landing lock (ce_* rows: r11/sbx_ce.ps1, dumped from the
sandbox TU with r11/dump_sbx.sh / r11/ce_report.py; the do-while, x/*_dw and dup_* rows:
r11/sbx_new.ps1 at the second landing, 2026-09-26, with the same scores as r11/model.py, and
dumped with r11/dump.sh / r11/site_report.py).
How each class behaves (dumps): self-assignments and dead stores are gone before flow; a
detour that fold-const or cse folds (`x - x` in one expression, same-block detours) leaves
no reference; `do { } while (0);` adds neither a reference nor a block boundary for flow
(lvl5 stays "used 3 times across 3 insns in block 14", the mask "used 8 times across 2 insns
in block 48") — all of these stay tied. A detour that survives to flow in another block
(ce_l_chain_callarg: `(insn 241 239 243 (set (reg:SI 189) (minus:SI (reg:SI 188) (reg/v:SI
158)))` is still in .flow, and .combine leaves only `(insn 1298 231 233 (use (reg/v:SI 158))`;
"Register 158 used 5 times across 22 insns") makes the value global, and find_reg then
gives value 1 `conflicts: 2 3 4 16 29`, `someone_prefers: 4` -> $a1, and the mask
`conflicts: 2 29` -> $v1, as (D)(3) predicts.

The clear loop's counter: $t4 for both loops means the two counters share a register, which
two distinct pseudos can do only if their ranges do not overlap. A split counter kept out
of the scan loop conflicts only with what is live in the clear loop (`conflicts: 2 29` ->
$v1); one extended into the scan loop by a chain-extender overlaps the player counter, which
holds $t4 there (ce_z_chain_e: `conflicts: 2 3 4 5 6 7 8 9 10 11 12 16 29` -> $t5), and those
extensions do not even fold to zero bytes (518 insns).

| variant (v/ce_*.c) | construct | sandbox | outcome at the site |
|---|---|---|---|
| ce_l_self_same / _self_clamp / _self_after | `lvl5 = lvl5;` in its block / clamp arm / after the call-if | 102 (512) each | tied `andi $3,$3,0xff` |
| ce_l_dead_clamp / ce_l_dead_init | dead `lvl5 = 0;` in the clamp arm / at function entry | 102 (512) each | tied |
| ce_l_chain_same | `(lvl5 + 1) * 0x180 + 0x280 - 0x180` | 104 (513) | tied |
| ce_l_chain_clamp | `0x1000 + lvl5 - lvl5` in the clamp arm | 102 (512) | tied (folded before flow) |
| ce_l_chain_callarg | `func_8005509C(... + lvl5 - lvl5)` | 102 (512) | global, $a1 (`andi $5,$2`) |
| ce_l_chain_callif | `(u8)(D_800A37D2 % 5) + lvl5 - lvl5 == 0` | 104 (513) | global, $a1 |
| ce_l_entry_uninit | `+ lvl5 - lvl5` at the first statement (read before write) | 103 (512) | global, $a3 |
| ce_lm_entry_uninit | same for lvl5 and the mask | 41 (514) | $t0 / $a3 |
| ce_l_arg1alias / _glob | `u16 *list_alias = arg1;` read by a folding detour (+ callarg chain) | 102 (512) each | tied / $a1 |
| ce_l_alias | `lvl5_p = &lvl5;` | 111 (513) | tied `andi $3,$3` |
| ce_m_self_after / ce_m_dead_goto / ce_m_chain_goto | self-assign / dead store / `cursor += m - m` | 102 (512) each | tied `or $2,$2,$3` |
| ce_m_chain_goto2 / ce_m_chain_after | `lo = lo + move_mask - move_mask;` in the goto arm / after the test | 98 (512) each | global, $v1 |
| ce_m_chain_e1 | `e[1] + move_mask - move_mask` (read before write) | 91 (515) | global, crosses 5 calls, $s1 |
| ce_z_self_after / ce_z_dead_end / ce_z_chain_ploop / ce_z_chain_scan | self-assign / dead store / folding detours | 25 (516) each | counter $v1 |
| ce_z_chain_bit / _rec / _e / _tail | `+ zero_i - zero_i` inside the section loop | 59/59/53/53 (518) | $t7/$t7/$t5/$t5, not zero-byte |
| ce_l_dw_between / ce_l_dw_wrapuse | `do { } while (0);` after the write / around the store | 102 (512) each | tied, "in block 14" |
| ce_l_dw_wrap | `do { lvl5 = ...; } while (0);` | 109 (512) | tied `andi $2,$2` |
| ce_m_dw_between / ce_m_dw_wrap | the same for the mask | 102 (512) each | tied, "in block 48" |
| x/abl_stat2_dw.c / x/abl_stat1_dw.c | stat2 / stat1 alone split, plus `do { } while (0);` after its write | 11 (516) / 6 (515) | stat2's copy kept, stat2 in $v1 |
| x/pv_dw_stats.c | the twin plus both stat wraps (the permuter's find, extended) | 33 (515) | |
| dup_lvl5_tail | the call-if duplicated into both arms of the clamp (no statement of value 1 itself can be put into arms: its write opens the D_800A389A arm and its only read is the next statement) | 115 (507) | level `andi $3,$4` ($v1, not $a2) |
| dup_lvl3 | the five statements after `if (lvl3 >= 3)` duplicated into both arms | 108 (518) | practice level `andi $4,$2` ($a0) |
| dup_row_idx | `pair = ...; p[0x424] = ...; p[0x3F6] = ...;` into both arms of `if (... % 10 == 0)` | 118 (519) | row in $a1 (`sll $5,$6,1`, `addu $5,$5,-1`) |
| dup_stat1 | the e[1] block duplicated into both arms of `if (e[4] == 0x40)` (an else added) | 98 (512) | mask tied `or $2,$2,$3`; no stat1 copy |
| dup_stat2 | `cat = ...; if (hi2 < stat2 ...)` into both arms of `if (hi1 < stat2)` | 97 (533, not merged) | no stat copies |
| dup_zero_i | the clear loop duplicated into both arms of the rand() test | 33 (524) | counter `sltu $2,$5,8` ($a1) |
None reaches 0; none seats a value of temp in $a2 or the clear loop's counter in $t4.

## (D)(4) Measured alternatives
`sandbox --disable all` scores were measured 2026-09-26 in the tree under the landing lock,
with the landing's edits applied (r11/model.py `apply`; full-build SHA1 == oracle) and
each variant given as `--candidate memory/grind/func_80055138/r11/v/<name>.c`
(r11/sbx_all.ps1). The model.py column is the scratch-TU build of the same bodies:
| spelling | model.py | sandbox |
|---|---|---|
| reuse (candidate.c) | 0/516 | 0/516 (516 insns) |
| full one-variable-per-value twin, innermost scopes (v/pv.c) | 102 (512 insns) | 102 (512) |
| structural: twin, all at function scope (v/pv_fs.c) | 102 (512) | 102 (512) |
| structural: twin, values typed u8 (v/s_u8.c) | 105 (515) | 105 (515) |
| structural: twin, declared with initializers (v/s_decl_init.c) | 102 (512) | 102 (512) |
| structural: twin, e[1] tests read stat1 (v/s_tests.c) | 102 (512) | 102 (512) |
| ablation: value 1 (lvl5) alone split (v/abl_lvl5.c) | 4 | 4 (516) |
| ablation: value 2 (lvl3) alone (v/abl_lvl3.c) | 33 | 33 (516) |
| ablation: value 3 (row_idx) alone (v/abl_row_idx.c) | 33 | 33 (516) |
| ablation: value 4 (move_mask) alone (v/abl_move_mask.c) | 2 | 2 (516) |
| ablation: value 5 (stat1) alone (v/abl_stat1.c) | 6 (515) | 6 (515) |
| ablation: value 6 (stat2) alone (v/abl_stat2.c) | 11 (515) | 11 (515) |
| two variables: case values / loop values (v/part_case_loop.c) | 98 (514) | 98 (514) |
| two variables: mask / the rest (v/part_mask_rest.c) | 2 | 2 (516) |
| two variables: case values + mask / stats (v/part_casemask_stats.c) | 11 | 11 (516) |
| idx split: clear loop's own counter (v/ctr_split.c) | 25 | 25 (516) |
| hi2_val staging receipts: base_val at function scope (v/nostage_fs.c) | 6 (515) | 6 (515) |
| base_val at block scope (v/nostage_blk.c) | 6 (515) | 6 (515) |
| base inline three times (v/nostage_inline.c) | 8 (517) | 8 (517) |
| permuter from v/pv.c (tmp/func_80055138/r11/perm_pv, -j2, --stack-diffs) | base 1069 -> best 285 permuter units | |

Permuter campaign (tools/permuter_campaign.py, label r11-pv-twin, launched 2026-09-26
21:17 UTC, ~9,100 iterations over ~20 minutes, harvested and stopped): best 285 = v/pv.c
plus `do { } while (0);` after `stat2 = e[2];` (x/perm285.c: 33/516 in model.py terms; it
adds a statement); 350 routes value 1 through `cat` (`cat = lvl5;`, a variable reuse);
the rest are constant carriers (`new_var = 0x400;`, `hi2_val = 7;`), `new_var` staging of a
test, `(double)` casts, or operand respellings. No find reaches 0.
Earlier per-job chassis (s2, before this session): honest per-job floor 35/516 after two
permuter campaigns (evidence.md s2), every find a constant carrier or a variable reuse.

## (E)/(F)
`temp`: (E)(i) generic scratch word. `idx`: (E)(ii), both values are loop indices (the
byte index 0..7 and the player index 0..1), and the name says only that. Declaration
comments name every value and cite Ruling 11 and this file (candidate.c lines 9-12 and 21-26).

## Other constructs in the body (H) — judged on their own
- `hi2_val` staging (FAKE-annotated, staged-value-reused-variable): hi2_val's own value is
  the 0x404 store; the shared base `rec->0x40A + 100` is staged through it and completed
  by `hi2_val += hi2 * 40`. Bounds: the staged value is read three times (lo_val, hi1_val,
  the +=); hi2_val's previous value (last section's) was stored before and is dead; the
  staged value is consumed by hi2_val's own completing assignment. Mechanism (dumps
  `nostage` vs `cand`): a separate `base_val` is "used 12 times across 13 insns in block
  65" and tied to the dying lh result 447 (`104 in 3`, `447 in 3`: lh v1; addiu v1,v1,100);
  hi2_val (103, "used 21 times across 20 insns", no block) is global, the lh result 446 is
  in $v0 (`446 in 2`) -> `addiu v1,v0,100`, the target. Receipts: base_val at function
  scope 6 (515 insns), at block scope 6 (515), inline three times 8 (517).
- Other multi-write locals hold ONE value each (every read can reach every write):
  `rec`, `cursor`, `list`, `chr` are bound once per player in the if/else arms and read
  after the join (`cursor` then walks the same list: one role); `lo`/`hi1`/`hi2` are
  min/max accumulators; `lo_val`/`hi1_val` are set once per arm. `sec`, `cat`, `bit`,
  `base`, `src`, `row`, `pair`, `other`, `e` are written once per scope/iteration.
- Data model (landing edits, r11/model.py `apply`): see evidence.md s3.
