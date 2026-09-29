# func_800198D0 — Ruling 11 (D) proof and landing record (manual, 2026-09-29)

Rulings spent: `.claude/rules/ordinary-c-judge-decidable.md` § Ruling 11 (owner 2026-09-26), with its
(D)(3) clauses "FAKE-construct spellings are set aside" (Q30) and "Mechanism + search" (Q31);
§ Ruling 4 (compound-assignment updates) for the reader state; § Ruling 13 (unattended run).
Nothing else. No FAKE-annotated construct, asm, pin, volatile or build change anywhere in the body.

## 0. Bodies, tools, commands

- **Landing** = `memory/grind/func_800198D0/candidate.c` = `r11/variants/landing.c`
  (byte-identical; it is also the exact text spliced into `src/code6cac.c` for the landing).
  Every per-value / ablation / structural / declaration / type variant is GENERATED from that
  exact text by `r11/tools/mkpv.py`, `mkst.py`, `mkdecl.py` and `mkty.py` (exact-text renames and
  declaration moves, each asserting its hit count; comments are left as they are), never
  hand-edited. So each variant differs from the landed body only in declarations and
  identifiers.
- **Full build**: the landing spliced into `src/code6cac.c` in place of its `INCLUDE_ASM` line
  builds to SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild --allow-dirty,
  2026-09-29).
- **Measurement**: the engine sandbox, `sandbox func_800198D0 --disable all --candidate <body>`
  (run through `tools/sandbox_sweep.ps1`); "score" = the engine's distance; results
  `r11/sandbox_sweep.txt`. Earlier probes on the pre-landing chassis: `r11/early/scores.txt`.
- **Dumps (D)(1)**: `r11/tools/alloc.sh <body> <dir>` = cpp of `src/code6cac.c` with the body
  substituted, then the instrumented `tools/gcc-2.7.2/cc1` with the build flags
  (`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
  -msoft-float`) plus `-dr -dl -dg` and `BB2_ALLOC_DEBUG=1`. `r11/tools/instcheck.sh` shows the
  instrumented cc1's assembly equals the build cc1's for every dumped body (only the echoed
  option comment differs): `r11/dumps/instcheck.txt`.
  `r11/tools/r11table.py` names every user-variable pseudo: GCC 2.7.2 expands a declaration when
  it parses it, so the `reg/v` pseudos of the `.rtl` dump, sorted, are the declarations in
  textual order (parameters first, macro block-locals included); the script checks the two
  counts agree. It prints each pseudo's hard register (`.greg` "Register dispositions") and its
  global.c line (ALLOCDBG: allocation order `ord`, `nrefs`, `livelen`, priority `pri`).
  `r11/tools/conf.py` adds the hard registers in the pseudo's `.greg` conflict list.
  Outputs: `r11/dumps/<body>.table.txt`, `.alloc.txt` (full ALLOCDBG), `.conflicts.txt` (every
  `.greg` conflict line), and `r11/dumps/conflicts.txt` (the R11 variables, all bodies).
- **Mechanism sources** (tools/gcc-2.7.2): global.c:635-656 `allocno_compare` (allocnos are
  seated in decreasing priority = floor_log2(n_refs) * n_refs / live_length, ALLOCDBG prints it
  x10000) and global.c:952 `find_reg` (each allocno takes the lowest-numbered hard register that
  is free of its conflicts; pass 0 only considers registers already in use, global.c:344-372).
  Register numbers: v0 2, v1 3, a0 4, a1 5, a2 6, a3 7, t0-t7 8-15, s0-s2 16-18, t8 24, t9 25.

## 1. The reused locals and their values

"Value" as Ruling 11 defines it (writes that can reach a common read). Six locals hold more than
one; every other local holds one (section 7).

| local | declared (innermost scope enclosing all writes) | values | name (E) |
|---|---|---|---|
| `idx` | function body (writes in the keyframe block and the post-pass) | V1 keyframe channel-loop index (`idx = 0` / `idx++`), V2 post-pass column index (`idx = 0` / `idx++`) | loop index, (E)(ii) |
| `idx2` | function body | V1 sub-frame loop index (`idx2 = 0` keyframe path, `idx2 = sub - 1` continue path, `idx2++`), V2 post-pass row index | loop index, (E)(ii) |
| `field` | function body | V1-V3 the three keyframe header words, V4 the per-channel keyframe flag, V5-V7 the three sub-frame header words, V8 case 3's 4-bit low code. A header's flag write and its 16-bit write both reach `work[k] = field`, so each header is one value | a bit field read by GETBITS: every write is `GETBITS(field, n)`, (E)(ii) |
| `temp` | channel-loop body | V1 the channel's decoded delta (case 1's zigzag result; case 2's `0` / 12-bit read; case 3's flag bit when it is 0, `((temp << 3) | ...) + 1`, `-temp`), V2 case 1's magnitude, V3 case 2's zero flag, V4 case 3's magnitude | generic, (E)(i) |
| `nbits` | case 1's block | V1 the zero-run length (`nbits = 0` / `nbits++`), V2 the suffix length `nbits = nbits - 1` | bit count, (E)(ii) |
| `nbits2` | case 3's `if (temp)` block | the same for case 3 | bit count, (E)(ii) |

Target registers (asm/funcs/func_800198D0.s): `idx` $t4 in both loops (0x80019C90 `addu $t4,$zero,$zero`,
0x80019D34 `addiu $t4,$t4,1`; 0x8001A39C, 0x8001A3BC); `idx2` $t8 (0x80019D48, 0x80019D88/0x80019DB4
`addiu $t8,$a1,-1`, 0x8001A380; 0x8001A394, 0x8001A3CC); `field` $t2 (e.g. 0x80019AC0 `srl $t2,$t0,31`,
0x80019B3C `or $t2,$a0,$v0`, 0x80019CA8, 0x8001A20C `srl $t2,$t0,28`, 0x8001A314 `andi $v1,$t2,7`);
`temp` $t5 (0x8001A030 `ori $t5,$a0,0x800`, 0x8001A0EC `addu $t5,$v0,$zero`, 0x8001A0F4
`srl $t5,$t0,31`, 0x8001A32C `negu $t5,$v1`); `nbits` $a0 (0x80019F9C, 0x80019FE0, 0x8001A058
`addiu $a0,$a0,-1`); `nbits2` $a0 (0x8001A218, 0x8001A240, 0x8001A2B8).

## 2. Measured one-variable-per-value spellings (sandbox, r11/sandbox_sweep.txt)

| body | score |
|---|---|
| landing (variants/landing.c = candidate.c) | **0** (749/749) |
| pv_idx (kidx, col) | 146 (745 insns) |
| pv_idx2 (step, row) | 21 |
| pv_field (h0-h5, kflag, lo) | 209 |
| pv_temp (delta, mag1, flag, mag3) | 283 (750 insns) |
| pv_nbits (zeros, len) / pv_nbits2 (zeros2, len2) | 7 / 7 |
| pv_all (all six split) | 311 (748 insns); pv_all_colfn (the same with the column index at function scope, the permuter base) 311 |
| ablations, field (one value split out, the rest shared): h0 / h1 / h2 / h3 / h4 / h5 / kflag / lo | 6 / 6 / 6 / 6 / 6 / 6 / 151 / 215 (755 insns) |
| ablations, temp: final delta / mag1 / flag / mag3 | 419 (752) / 223 (750) / 3 / 6 |
| structural: fs_idx / fs_field / fs_temp / fs_nbits / fs_nbits2 (per-value locals at function scope) | 146 / 209 / 283 / 7 / 7 |
| structural: st_idx_while (both loops as `while`) / st_idx_colfn (column index at function scope) | 146 / 146 |
| structural: st_idx2_rowblk (row index in a block around the post-pass) / st_idx2_dowhile (row loop as do-while) | 21 / 21 |
| declaration position of the closest splits (mkdecl.py): case-2 flag in the channel-loop body before / after `temp`, at function scope last / first | 3 / 3 / 3 / 3 |
| case-3 magnitude in the channel-loop body / at function scope / before `nbits2` | 6 / 6 / 6 |
| keyframe header word 0 at function scope last / first; sub-frame header word 0 at function scope | 6 / 6 / 6 |
| suffix length `len` in case 1's block after / before `zeros` | 7 / 7 |
| types: case-2 flag s32 / u32 / u16 / s8 / u8 | 4 (748) / 4 (748) / 3 / 3 / 3 |
| types: case-3 magnitude s32 / u16; `len` u32 / s16; header word 0 s32 / u16 | 6 / 6; 8 / 26 (750); 6 / 24 |

`idx`, `idx2`, `nbits`, `nbits2` hold two values, so they have no ablation beyond the full split.

## 3. Mechanism per Ruling 11 local (Q31 (a)), with the measured record (Q31 (b)-(c) in 3A)

Every figure below is a line of `r11/dumps/<body>.table.txt` / `conflicts.txt` (ALLOCDBG priority
is x10000). In every body all six locals and their split replacements are global allocnos (none is
local-alloc'd), so their seats are global.c's `allocno_compare` order plus `find_reg`.

### idx — global.c priority and conflict set
- Landing: pseudo 86 (both loop indices) nrefs 18, livelen 58, pri 12413, ord 35; hard-register
  conflicts v0 v1 a0 -> **$t4** (target).
- pv_idx: `col` (pseudo 585, declared in the row-loop body) nrefs 11, livelen 13, pri 25384, ord 10,
  conflicts only v0 -> $t2; `kidx` (151, declared in the keyframe block) nrefs 7, livelen 45,
  pri 3111, ord 50 -> $t7. The seat the landing gives the shared
  index is taken by `temp` ($t5 -> $t4), and the cascade seats `slot`/`sub`/`out` in $t9/$s0/$a2:
  two callee-saved registers instead of three (frame 8, 745 insns). Every structural respelling
  of the per-value form (while loops, the column index at function scope, both at function
  scope) measures the same 146.
- Record: every banked counting spelling for idx misses (3A).

### idx2 — global.c priority against the hoisted loop constant
- Landing: pseudo 88 (both indices) nrefs 19, livelen 386, pri 1968, ord 53 -> **$t8**, seated one
  step ahead of loop.c's hoisted constant 1 (pseudo 292, pri 1215, ord 55 -> $t9) (landing.alloc.txt);
  the post-pass constant -4096 (pseudo 591, pri 2222, ord 50) takes $t2. Target: `idx2` $t8,
  the constant 1 $t9 (0x80019DC4 `addiu $t9,$zero,1`), -4096 $t2 (0x8001A398).
- pv_idx2: `step` (88) nrefs 12, livelen 367, pri 980, ord 56 falls behind the constant 1 (293,
  ord 55 -> $t8) and takes $t9; `row` (89) nrefs 7, livelen 19, pri 7368, ord 42, conflicts only v0,
  takes $t2 first and pushes -4096 (592) to $t5 (21). The row-in-a-block and do-while respellings
  measure the same 21.
- Record: every banked counting spelling for idx2 misses (3A).

### field — global.c priority and conflict set
- Landing: pseudo 93 (all eight values) nrefs 72, livelen 164, pri 26341, ord 9, conflicts v0 v1 a0
  -> **$t2** (target).
- pv_field: each piece is short: h3-h5 pri 25714 (ord 9-11) and kflag 17142 (ord 20), conflicting
  only with a0 -> $v1; h0-h2 pri 8571 (ord 44-46), conflicting only with v1 -> $a0; lo pri 4931
  (ord 51) -> $t7. $t2 is then free when `ptr` (ord 12) is seated, and `ptr` $t3 -> $t2, `idx`
  $t4 -> $t3, `temp` $t5 -> $t4 (209). Each ablation that splits one value off moves that value
  to a lower free register (a header 6, the keyframe flag 151, the low code 215).
- Record: every banked counting spelling for field misses (3A).

### temp — global.c priority and conflict set
- Landing: pseudo 349 (all four values) nrefs 90, livelen 482, pri 11203, ord 37, conflicts
  v0 v1 a0 a1 a2 a3 -> **$t5** (target).
- pv_temp: `mag1` (356) pri 50526 is seated at ord 6, ahead of `bits` (ord 7), and takes $t1
  (bits -> $t2, field -> $t1 ...); `mag3` (471) pri 45000, conflicts v1 a0 -> $v0; `flag` (425)
  pri 38571, conflicts only a0 -> $v0 (the target reads the case-2 flag from $t5:
  0x8001A130 `sll $v0,$t5,16`); `delta` (349) pri 4751, ord 47 -> $t7 (283). Ablations: the
  flag alone 3 (it moves to $v0, nothing else), mag3 alone 6, mag1 alone 223, the final delta
  alone 419.
- Record: every banked counting spelling for temp misses (3A).

### nbits / nbits2 — global.c conflict set
- Landing: pseudo 355 (nbits) / 469 (nbits2), nrefs 48, livelen 31, pri 77419, ord 1 / 2,
  conflicts v0 v1 (the suffix read's `hi`/`need` sit in $v1 while the suffix length is live)
  -> **$a0**; target `addiu $a0,$a0,-1` (0x8001A058 / 0x8001A2B8) then `sllv $t2,$t9,$a0`.
- pv_nbits: `zeros` (355) nrefs 27, livelen 18, pri 60000, ord 4, conflicts only v0 -> $v1;
  `len` (381) pri 60000, ord 5, conflicts v0 v1 -> $a0; the count is read in $v1 and copied
  (`addiu $a0,$v1,-1`, `move $t5,$v1`) (7). The same for nbits2 (zeros2 469 -> $v1, len2 505 ->
  $a0; 7). Function-scope per-value locals: 7 / 7. Writing `n - 1` in the macro arguments with no
  second local measures 320 / 380 on the v18 chassis (r11/early/scores.txt).
- Record: every banked counting spelling for nbits / nbits2 misses (3A).

## 3A. (D)(3) under Q31 — the banked search

(a) Mechanisms: section 3, from the banked dumps of the landing and of each variable's
one-variable-per-value spelling.
(b) Every spelling proposed by the author is banked and measured: the 53 bodies in
`r11/variants/` (the landing and 52 generated spellings; scores in
r11/sandbox_sweep.txt and section 2) and the 28 earlier probes in `r11/early/` (on the v17 / v18
chassis, also banked there; scores in r11/early/scores.txt). Reviewer proposals are banked the same way before a
landing.
(c) No counting spelling reaches the target. The only bodies that score 0 are the landing
(`landing.c`) and, among the early probes, the chassis steps that split a
value which the landing therefore also splits (`sp_S_ich`: channel-loop index, `sp_S_m`: case-3
count, `sp_B_ch_m`, `misc_M_off`: `shift`); none of them splits any of the six reused locals
(each keeps the landing's sharing). No spelling carries a FAKE- or !FAKE-annotated construct, so
none is set aside under Q30. The permuter campaign (section 6) found no spelling reaching 0.

## 4. Prong walk (A)-(H)

- **(A)** Each is a local of func_800198D0 (not a parameter, global, static or `register`),
  declared once at the innermost block enclosing all of its writes (section 1). One reading to
  flag: `temp`'s writes all sit inside the `switch` body, but its V1 is read by the accumulation
  after the `switch`, so the innermost block that encloses all its writes and can also hold the
  declaration in scope of its reads is the channel-loop body (the same for the per-value
  `delta`); no other
  declaration was moved or re-scoped for it; no `&idx`, `&idx2`, `&field`, `&temp`, `&nbits`,
  `&nbits2` anywhere.
- **(B)(1)** Every write is read before the next write: loop indices by their loop tests (idx V1
  also by `work[idx + 3]`); each header write by `if (field)` or `work[k] = field`; the keyframe
  flag by `if (field)`; the low code by `field & 7` / `field & 8`; temp's magnitudes by the zigzag
  or `((temp << 3) | ...)`, its flags by `if (temp)`, its final values by the accumulation;
  `nbits`'s count by the loop test / `== 12` / `>= 2` / `temp = nbits` / `nbits - 1`, its suffix
  length by GETBITS_PRE.
  **(B)(2)** Writes that could re-store a held value, each with a feasible incoming path on which
  the variable holds something else: `idx = 0` (post-pass) — keyframe path: idx holds 63;
  `idx2 = 0` (post-pass) — any path where the sub-frame loop ran with `sub >= 1` (e.g. keyframe
  path, `frame & 7 == 3`): idx2 holds 3; every `GETBITS(field, n)` — the stream bits are data (no
  branch condition constrains them): e.g. header 0 decodes 0x1234 and header 1's flag bit is 0
  (field 0x1234 -> 0), a header's 16-bit read follows flag 1 and reads 2, the first keyframe flag
  0 follows header 2's value 0x10, the low code 3 follows the sub-frame's third header word 0x10;
  `temp = 0` (case 2) — temp holds the flag 1 on every path reaching it; the 12-bit read — temp
  holds 0 (flag) and reads e.g. 7; the zigzag — magnitude 1 gives -1; case 3's magnitude writes
  follow the flag 1 (e.g. `temp = nbits2` with 0); `((temp << 3) | ...) + 1` >= 1 differs from a
  held 0; `temp = -temp` negates a value >= 1; `nbits = 0` — the block-local holds the previous
  channel's count (e.g. 5); `nbits = nbits - 1` always changes the value. The loop-index
  increments always change the value.
- **(C)(1)** The one-variable-per-value spellings are `r11/variants/pv_<var>.c` (each value its own
  local at the innermost block of its writes) and `pv_all.c`; **(C)(2)** they differ from
  `landing.c` only in declarations and identifiers (mkpv.py renames and declaration moves; the
  landing writes the suffix length as `nbits = nbits - 1;` so that the per-value
  `len = zeros - 1;` is an identifier change); **(C)(3)** every value has a write whose
  instructions are in the target: the index increments (`addiu $t4,$t4,1`, `addiu $t8,$t8,1`),
  the field reads (`srl`/`srlv`/`or` into $t2), the channel values (`or`/`ori`/`srl`/`negu` into
  $t5), the counts (`addiu $a0,$a0,1` / `-1`).
- **(D)(1)** dumps: `r11/dumps/` (commands in section 0); **(D)(2)** mechanism per variable,
  section 3; **(D)(3)** under Q31: section 3 plus the banked search (3A); **(D)(4)** full
  per-value spellings, ablations and structural respellings (section 2), permuter (section 6).
- **(E)** `idx`, `idx2` (loop indices), `field` (every write a GETBITS bit-field read), `nbits`,
  `nbits2` (bit counts) under (E)(ii); `temp` under (E)(i).
- **(F)** Each declaration carries a comment naming the values and citing Ruling 11 and this file.
- **(G)** Fresh layer-2 cheat-reviewer: pending.
- **(H)** Everything else: section 7.

## 5. FAKE-construct spellings (Q30)

None was proposed or measured; the landing carries no FAKE- or !FAKE-annotated construct.

## 6. Permuter campaign (tools/permuter_campaign.py, `--stack-diffs`, standalone workspace)

Workspace `r11/tools/mkperm.sh <body> <dir>`: head.h (`common.h`, `extern u8 D_800F1B18[]`) +
the body, cpp-expanded; compile.sh = the Makefile's code6cac recipe (cc1 | prologue_fix | maspsx
with the code6cac gates | align fix | multu_pad | as); target.o = asm/funcs/func_800198D0.s after
the permuter prelude. Control: the landing's workspace builds 749/749 instructions.
- Campaign (tmp/func_800198D0/perm_pvall, from `pv_all_colfn.c` = all six locals split, the column
  index at function scope; the campaign's base.c, built by mkperm.sh before the landing's comments
  were added, is token-identical after cpp to the one mkperm.sh builds from today's
  variants/pv_all_colfn.c; sandbox 311 like pv_all; `-j 2`, fresh seed): base 2565 (permuter score),
  32,322 iterations in 3,151 s, 194 finds, best 1315 found at ~1,200 s; stopped after 23 minutes with
  no new best (harvest record r11/perm/campaign_meta.json, log tail r11/perm/campaign_log_tail.txt,
  base r11/perm/base.c). The landing's own workspace builds 749/749 against the same target.
- What the five best finds do (r11/perm/best_*_diff.txt): 1315 stores `mag1 & 1` into the sub-frame
  header local `h4` (a second value for a header variable: the `field` reuse property); 1336 moves
  the zigzag into the `else` arm (changes behaviour); 1410 writes `zeros - 1` in place of `len` at
  one refill (fewer `len` references: toward the shared-count property of `nbits`); 1500 casts
  `need` to unsigned char (changes behaviour); 1505 routes `len` through `mag1` (another reuse).
  None keeps one value per variable while approaching 0; the best is 1315 from 2565.

## 7. Everything else (H)

- **GETBITS / GETBITS_PRE / COPY33** are statement macros whose temporaries (`need`, `hi`, `left`,
  `top`, `from`, `to`, `k`) are block-locals written once per expansion. The target allocates
  `hi`/`need` to $a0/$v1 in alternation from site to site (a function-scope variable would be one
  pseudo and one register), so they are per-site locals. GETBITS_PRE evaluates its prefix once
  (`top`); a plain GETBITS is its own macro (GETBITS_PRE with prefix 0 at the plain sites measures
  2 on the v17 chassis).
- **`left`**: the refill computes the new bit count first, uses it as the low-part shift and
  stores `bits = left` last. That order is what leaves the target's `subu $v0,...; addu $t1,$v0,$zero`
  copy at every refill (a direct `bits = 32 - need` loses it: 17 micro spellings,
  memory/grind/func_800198D0/evidence.md [s2]).
- **Reader state `ptr` / `bits` / `cur`**: updated in place by the macros (`cur = *ptr++`,
  `cur <<= need`, `bits = left`, `bits -= nb`, `ptr++`), the bit-reader idiom of the completed
  sibling func_8001979C in this file. Every `bits` write and every `ptr` write reaches the next
  read that other writes also reach (one value each). `cur`'s refill load is read by
  `cur >> left` and shifted in place by `cur <<= need` before the next site: a load followed by a
  compound assignment on the same variable (Ruling 4); func_8001979C writes the same
  `cur = *arg1; ... cur >> bits_left; cur <<= needed;` sequence.
- **Data access**: the record is the existing byte array `D_800F1B18` (code6cac.c:114) read at
  constant offsets with casts, the style func_8001979C already uses for the same record; no
  declaration changes.
- **`slot`**: two writes (`slot = rec + ...`, `slot = prev`), one value (both reach the decode
  and the write-back). `off` and `shift` are one value each (splitting `shift` off `off` measures 0).
- **`goto decode`**: the keyframe path joins the sub-frame loop with `idx2 = 0`.
- **Zigzag** `temp = (temp & 1) ? -(temp / 2) - 1 : temp / 2;` and the 12-bit sign extension
  `*p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);` are plain expressions.
- **Header comment** describes what the code does (cache of four decoded frames with the reader
  state, keyframe every 8 frames, `work` layout from the accesses).
