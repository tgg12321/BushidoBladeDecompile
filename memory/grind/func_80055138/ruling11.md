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
FINDREGDBG  pass0_used: 0 1 2 3 4 5 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG func=func_80055138 pseudo=89 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 4 5 6 7 8 9 10 11 16 29
FINDREGDBG  someone_prefers:
FINDREGDBG  pass0_used: 0 1 2 3 4 5 6 7 8 9 10 11 16 17 18 19 20 21 22 23 26 27 28 29 30 31
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
FINDREGDBG  pass0_used: 0 1 2 3 16 17 18 19 20 21 22 23 26 27 28 29 30 31
FINDREGDBG func=func_80055138 pseudo=106 alt=0 acc=0 retry=0
FINDREGDBG  conflicts: 2 3 4 16 29
FINDREGDBG  someone_prefers:
FINDREGDBG  pass0_used: 0 1 2 3 4 16 17 18 19 20 21 22 23 26 27 28 29 30 31
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
FINDREGDBG  pass0_used: 0 1 2 16 17 18 19 20 21 22 23 26 27 28 29 30 31
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
   allocno's conflicts nor in regs_someone_prefers (FINDREGDBG pass0_used), pass 1 the
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

## (D)(3) Necessity (every one-variable-per-value spelling)
- **temp.** Property of the reuse spelling: the values 1 and 4 sit in a pseudo referenced
  in more than one basic block, so local-alloc neither allocates nor ties it (a). In ANY
  one-variable-per-value spelling, value 1 has its own variable whose only references are
  its write and the two reads in `... = lvl5 * 0x180 + 0x280;`, and value 4 its own
  variable read only by `if (!(move_mask & bit))`; the target's instruction stream fixes
  both ranges as straight-line code with no label or branch between write and last read
  (0x800552E4..0x800552EC; 0x80055738..0x8005573C), and a C local is referenced exactly by
  its own write and reads. So each is referenced in one block and dies once whatever its
  declaration order, scope, type (s_u8), initializer form (s_decl_init) or the
  surrounding statement order, and combine_regs ties it to the dying input: the output
  register equals an input register (`andi X,X,0xff`, `or X,X,Y`). The target's
  `andi a2,v0,0xff` and `or a2,v0,v1` write a register that is neither input, so no
  one-variable-per-value spelling reproduces them. Values 2, 3, 5 and 6 are in the same
  variable because the target seats them in the same register, and each, split alone,
  leaves it (ablations below; mechanisms b and c).
- **idx.** Property: one pseudo live across the scan loop, whose conflicts fill 2..11 (d).
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

## (D)(4) Measured alternatives
Scores are `sandbox --disable all` measured in the tree under the landing lock with the
landing's header/consumer edits applied (each variant spliced by `sandbox --candidate`),
and the same bodies through r11/model.py `score` (identical build pipeline, scratch TU):
| spelling | model.py | sandbox |
|---|---|---|
| reuse (candidate.c) | 0/516 | SBX_cand |
| full one-variable-per-value twin, innermost scopes (v/pv.c) | 102 (512 insns) | SBX_pv |
| structural: twin, all at function scope (v/pv_fs.c) | 102 (512) | SBX_pv_fs |
| structural: twin, values typed u8 (v/s_u8.c) | 105 (515) | SBX_s_u8 |
| structural: twin, declared with initializers (v/s_decl_init.c) | 102 (512) | - |
| structural: twin, e[1] tests read stat1 (v/s_tests.c) | 102 (512) | - |
| ablation: value 1 (lvl5) alone split (v/abl_lvl5.c) | 4 | SBX_abl_lvl5 |
| ablation: value 2 (lvl3) alone (v/abl_lvl3.c) | 33 | SBX_abl_lvl3 |
| ablation: value 3 (row_idx) alone (v/abl_row_idx.c) | 33 | SBX_abl_row_idx |
| ablation: value 4 (move_mask) alone (v/abl_move_mask.c) | 2 | SBX_abl_move_mask |
| ablation: value 5 (stat1) alone (v/abl_stat1.c) | 6 (515) | SBX_abl_stat1 |
| ablation: value 6 (stat2) alone (v/abl_stat2.c) | 11 (515) | SBX_abl_stat2 |
| two variables: case values / loop values (v/part_case_loop.c) | 98 (514) | - |
| two variables: mask / the rest (v/part_mask_rest.c) | 2 | - |
| two variables: case values + mask / stats (v/part_casemask_stats.c) | 11 | - |
| idx split: clear loop's own counter (v/ctr_split.c) | 25 | SBX_ctr_split |
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
