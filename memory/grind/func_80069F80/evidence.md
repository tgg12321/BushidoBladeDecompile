# Evidence bank - func_80069F80

## OBJECT MODEL: (recon session 1, 2026-09-15)
Flagged symbol in the brief's DATA MODEL block:

- **D_800A374C** (census `g_dma_buf_base`, decl `extern s32 D_800A374C;` in
  m2c_context.h and in seven `src/*.c` TU-local extern blocks) - **MATCHES**.
  Measured: the score-5 candidate declares it exactly as `extern s32 D_800A374C;`
  (the text1b.c house convention - src/text1b.c:1571, :3006, :3329, :5816 all
  use the same `s32` spelling) and the only use site in this function is
  `AddPrim(D_800A374C + 0xC, arg0[7])`, which reproduces the target's
  `lui %hi / lw %lo / addiu $a0,$a0,0xC` triple byte-for-byte (diff indices
  119-123 were clean at every score measured this session: 24/12/9/8/6/5).
  A pointer spelling `extern u8 *D_800A374C;` also exists in the repo
  (src/ings.c:40) but is not the text1b.c convention and is not needed here.
  No other DATA MODEL symbol was flagged.

Non-flagged globals this function touches, all already declared in text1b.c and
all reproducing their target instructions with the `s32` spelling:
`D_800A3524` (`((s32 *)D_800A3524)[8] & 8`), `D_800A3514`
(`(D_800A3514 & 0x1F) << 7`), `D_800A34FC` (`*(s16 *)(D_800A34FC + 0xC)`).
All three are gp-relative in the target and are emitted gp-relative by the
candidate.

## Floor history (this session, chassis HEAD 2026-09-15, -mel -msoft-float)
| form | score | what it fixed |
|---|---|---|
| HEAD (INCLUDE_ASM, no C body) | 136 | baseline, driver-confirmed |
| v1: first full reconstruction, struct 0x2C | 24 | full shape + control flow, 136/136 insns |
| v2: struct extended to 0x3C (sp44..sp50) | 12 | frame 0x60 -> 0x70, all save offsets |
| v3: distinct temps + `p2 += 0x14` mutation in the `arg1&1` block | 8 | idx 102-104 order |
| v5: `s.sp1C = p1 + 0xC;` before `s.sp18 = p1;` | 9 | idx 84-86 ORDER (reg still swapped) |
| A: `s.sp2C = 3;` moved BEFORE `s.sp28 = 0;` and `s.sp1C = ...` | 6 | idx 68-73 v0/v1 allocation |
| **G (BEST = candidate.c)**: A + `p1=ptr[1]; s.sp18=p1; s.sp1C=p1+0xC;` | **5** | idx 82 load register |

Variants measured and rejected: B (sp28,sp1C,sp2C)=9, C (sp2C,sp1C,sp28)=6,
D (sp1C,sp2C,sp28)=6, E (q1 temp, sp18 then sp1C)=5, F (q1 temp, sp1C then
sp18)=6, H (`s.sp18 = ptr[1]; s.sp1C = s.sp18 + 0xC;`)=5, J (sp20 hoisted
between)=8, K (`p1 += 0xC` mutation at site 82)=5, and the five sp28-first
cross-products ZE/ZG/ZF/ZH/ZK = 8/8/9/8/8.

## Structural facts banked
1. **The function's shape is fully solved.** The build emits 136/136 insns with
   identical control flow, identical call sequence and identical frame layout.
   Only 5 instructions differ, all inside one basic block (block 9 in the cc1
   dumps = the join after the `D_800A3524` test).
2. **The descriptor struct is 0x3C bytes, not 0x2C.** Locals occupy
   sp+0x18..sp+0x57 in the target (frame 0x70, saves at 0x58..0x68). The sibling
   reconstructions in this TU (`EnvA` near src/text1b.c:6070, `S_69AE4` at
   src/text1b.c:5611) stop at 0x2C / 0x28. Adding four trailing s32 words
   (sp44, sp48, sp4C, sp50) to reach 0x3C is worth **12 points** on its own
   (24 -> 12): it corrects the frame size and therefore every
   `sw/lw $sN, N($sp)` offset plus both `addiu $sp` insns. This is a
   declaration fact about the shared descriptor type, not a pad - the fields
   are simply not written by this call site.
3. **Field map** (offsets relative to the struct base = sp+0x18):
   +0x00 header ptr, +0x04 table ptr (header+0xC, or header+0x14 in the
   `arg1 & 1` block), +0x08 chain/out for func_8007352C, +0x0C chain/out for
   func_80073728, +0x10 semi, +0x14 ot index (3, then 2), +0x18 x, +0x1C y,
   +0x20 and +0x24 two scale words (0x100/0x100, or 0x80/0x80 on the
   no-`D_800A3524`-bit path), +0x28 has_color u8, +0x29..+0x2B col r/g/b u8.
4. **The rsin scale is 47, not 32.** The target's `sll,addu,sll,subu` chain is
   GCC 2.7.2's synthesis of `* 47` (`((v<<1)+v)<<4 - v`). The near-identical
   sibling func_8006D3DC (src/text1b.c:6112) uses `<< 5` (i.e. *32) with a
   `+ 0x1FF` phase; this function uses `* 47` with no phase offset. Copying the
   sibling's spelling verbatim would have been wrong.
5. **The two residual sites are COUPLED through local-alloc.** Both live in the
   same basic block, so their quantities are sorted together by `qty_compare`
   (tools/gcc-2.7.2/local-alloc.c:1641; priority =
   `floor_log2(n_refs) * n_refs * size / lifetime`, highest first). Putting
   `s.sp2C = 3;` ahead of `s.sp28 = 0;` fixes the v0/v1 assignment at idx 68-73
   (worth 3) but costs the store ORDER at idx 70/71 (worth 2). The reverse
   order restores the store order and re-breaks the allocation: measured 8 for
   ALL five site-82 spellings crossed with it. Net: sp2C-first is 3 better.
   Any future attack must move ONE of these without disturbing the other.
6. **Residual detail, idx 84-86 (worth 3).** Target:
   `addiu $v1,$v0,0xC ; sw $v0,0x18($sp) ; sw $v1,0x1C($sp)` - the loaded
   pointer and its +0xC derivative are simultaneously live, so local-alloc does
   NOT tie them. Every spelling tried lets sched1 hoist the `sw 0x18` above the
   `addiu`, after which `combine_regs` ties them into one register
   (`sw ; addiu ; sw`). PASS ATTRIBUTION CONFIRMED FROM THE DUMP
   (tmp/grind/func_80069F80/dumps/text1b.sched): the pre-sched RTL order was
   insn 212 (`reg78 = reg77 + 12`) then insn 215 (`mem[fp+24] = reg77`), and
   the scheduler emitted 215 before 212. So this is a **sched.c pass-1
   tie-break**, not a source-statement-order effect - reordering the two C
   statements cannot fix it, which is exactly what E/F/G/H/K measured.
7. **Residual detail, idx 70/71 (worth 2).** Target emits `sw zero,0x28($sp)`
   then `sw $v0,0x2C($sp)`; the candidate emits them swapped. Two stores to the
   same base are memory-dependent in GCC 2.7.2's scheduler, so their relative
   order follows source order exactly - and source order is what pins the
   allocation in fact 5. This is the cheaper of the two residuals to attack.

- [s1] OBJECT MODEL: D_800A374C (census g_dma_buf_base, flagged in the brief's DATA MODEL block) MATCHES as `extern s32 D_800A374C;` - the text1b.c house convention at src/text1b.c:1571, :3006, :3329, :5816. Its single use site here, `AddPrim(D_800A374C + 0xC, arg0[7])`, reproduced the target's lui %hi / lw %lo / addiu $a0,$a0,0xC triple byte-for-byte at every score measured this session (24/12/9/8/6/5). The pointer spelling `extern u8 *D_800A374C;` that exists at src/ings.c:40 is not the text1b.c convention and is not needed here. No other DATA MODEL symbol was flagged. The three unflagged globals the function touches - D_800A3524, D_800A3514, D_800A34FC - are all already declared `extern s32` in text1b.c and all emit their target instructions gp-relative with that spelling.

- [s1] Honest floor dropped 136 -> 24 -> 12 -> 8 -> 6 -> 5 this session; the best form (memory/grind/func_80069F80/candidate.c) contains NO FAKE constructs and claims no sanctioned family - it is plain C.

- [s1] The descriptor struct is 0x3C bytes. The sibling reconstructions in the same TU (EnvA near src/text1b.c:6070, S_69AE4 at src/text1b.c:5611) declare only 0x2C/0x28; the extra four s32 words are worth 12 points because they set frame 0x70 vs 0x60 and therefore every callee-save offset.

- [s1] Field map at struct base sp+0x18: +0x00 header, +0x04 table (header+0xC, or header+0x14 in the arg1&1 block), +0x08 chain for func_8007352C, +0x0C chain for func_80073728, +0x10 semi, +0x14 ot index (3 then 2), +0x18 x, +0x1C y, +0x20/+0x24 two scale words (0x100/0x100 or 0x80/0x80), +0x28 has_color u8, +0x29..+0x2B colour r/g/b u8.

- [s1] The rsin scale here is 47, not the sibling's 32: the target's sll/addu/sll/subu chain is GCC 2.7.2's synthesis of `* 47`. func_8006D3DC (src/text1b.c:6112) uses `<< 5` with a `+ 0x1FF` phase; func_80069F80 uses `* 47` with no phase offset. Copying the sibling verbatim would have been wrong.

- [s1] The two residual sites share one basic block (block 9 in the cc1 dumps), so their local-alloc quantities are sorted together and any fix for one perturbs the other - measured, not inferred: every sp28-first spelling scored 8 or 9 while every sp2C-first spelling scored 5 or 6.

- [s1] Signature: func_80069F80(s32 *arg0, s32 arg1) - only $a0 and $a1 are read. arg0[1] -> +0x1C gives the object pointer; arg0[2], arg0[5] and arg0[7] are the three chain slots the function updates.

## Session 2 (structural, 2026-09-15) - floor 5 -> 0
| form | score | what it fixed |
|---|---|---|
| G (session-1 candidate, re-measured at dispatch) | 5 | baseline |
| V1: `s32 tbl` assigned at both join-block fills | 2 | idx 84-86 (addiu ; sw ; sw) |
| V2: `tbl` at fills 2+3 only | 7 | rejected |
| V3: `tbl` at all three fills | 9 | rejected |
| **V4 = V1 + `s.sp28 = 0;` before `s.sp2C = 3;`** | **0** | idx 70/71 store order; MATCH |

- [s2] PASS ATTRIBUTION, read from source: the idx 84-86 order is set by tools/gcc-2.7.2/sched.c adjust_priority() (called from schedule_insn for each predecessor whose ref_count reaches zero). Pre-reload n_deaths is always 0, so a newly-ready insn is raised to LAUNCH_PRIORITY iff birthing_insn_p(PATTERN) holds, and birthing_insn_p returns `reg_n_sets[regno] == 1`. That is why every once-set temp for p1 + 0xC (session-1 spellings E/F/G/H/K) scheduled directly before its own store, and why a local set at two sites does not. The .sched dump of the session-1 body (tmp/grind/func_80069F80/s2/f.sched, block 9) shows it: the ready list at T-9 is `212 (4) 215 (7f000001)`, 215 being the addiu carrying the launch bonus.
- [s2] The matched body contains NO FAKE construct and claims no sanctioned family: a `tbl` local written and read at two fills (the same spelling the matched func_80069E18 uses at src/text1b.c:5793) plus a plain statement order. Diff vs the session-1 body: tmp/grind/func_80069F80/s2/final.diff.
- [s2] Session 1's H3 "coupling" was an artefact of the once-set pseudo: combine_regs tied it into the loaded pointer's quantity, and that tie is what made the sp28-first order lose the v0/v1 seat. With the two-set local the seat is stable under both orders (V1=2 with sp2C-first, V4=0 with sp28-first).
- [s2] `tbl` must be confined to the join block: extending it into the `arg1 & 1` fill (V2=7, V3=9) moves the pseudo out of local-alloc and the third fill still needs the in-place `p2 += 0x14` mutation.

## Session 2b (2026-09-15, permuter modality)
- The 2026-09-15 00:46 layer-1 FAIL was solely the unreferenced `s32 q1;` leftover; removing it re-measures 0/136 (sandbox, --disable all, HEAD chassis -mel -msoft-float). No other change to the V4 body.
- Applied body: tmp/grind/func_80069F80/s2/V5_noq1_full.c (whole text1b.c); diff vs main: tmp/grind/func_80069F80/s2/final_noq1.diff (101 lines, confined to the INCLUDE_ASM line).
- No permuter campaign launched (distance already 0); no FAKE constructs; no sanctioned-family claim.

## Session 3 (enumerate, 2026-09-15) - systematic join-block sweep
Chassis: HEAD 2026-09-15 (-mel -msoft-float); every variant keeps the 0x3C
descriptor so the frame is isolated; no FAKE constructs anywhere.
Generator: tmp/grind/func_80069F80/s3/gen.py; sweep loop
tmp/grind/func_80069F80/s3/sweep.ps1 (splice + `sandbox --disable all`).

Axes: 15 variable-carrying schemes for the table pointer x 6 orders of
(sp28, sp2C, sp1C) in fill 1 x 2 orders of the (sp18, sp1C) stores in fill 2
= 180 variants (memory/grind/func_80069F80/s3_enum_results_180.txt). A
further 25 variants over the fill-2 (sp34, sp30, sp40) order axis were
measured before that axis was dropped (all 7..11, worse than the target order;
s3_enum_results_partial_o2axis.txt).

ENUMERATION: 180 spellings, best 0, 3 at 0, 28 at the old floor 5, histogram
0:3 2:6 4:4 5:28 6:43 7:10 8:45 9:26 10:13 11:2.

Per-scheme minimum (12 variants each):
| scheme | fill-1 table | fill-2 table | min |
|---|---|---|---|
| a_plain | s.sp18 + 0xC | s.sp1C = p1 + 0xC | 5 |
| b_split | s.sp18 + 0xC | tbl = p1; tbl += 0xC | 5 |
| c_memsplit | s.sp18 + 0xC | s.sp1C = p1; s.sp1C += 0xC | 5 |
| e_hdrreuse | s.sp18 + 0xC | p1 + 0xC, p1 reused as fill-3 header | 6 |
| i_split2 | tbl = s.sp18; tbl += 0xC | tbl = p1; tbl += 0xC | **0** |
| j_splitafter | s.sp18 + 0xC | (after header store) tbl = p1; tbl += 0xC | 5 |
| r_p1fill1 | p1 = s.sp18; p1 + 0xC | p1 + 0xC | 6 |
| s_p1all | p1 = s.sp18 | p1 in all three fills | 6 |
| t_tblfill1 | tbl = s.sp18 + 0xC (fill 1 only) | p1 + 0xC | 5 |
| u_tblf1_p1all | p1 = s.sp18; tbl = p1 + 0xC | p1 + 0xC, p1 all fills | 6 |
| v_kmut | s.sp18 + 0xC | p1 += 0xC; s.sp1C = p1 | 5 |
| w_memre | s.sp18 + 0xC | s.sp1C = s.sp18 + 0xC | 5 |
| x_ptrre | s.sp18 + 0xC | s.sp1C = ptr[1] + 0xC | 7 |
| y_ptr0 | ptr[0] + 0xC | p1 + 0xC | **4** |
| z_hdrmut3 | s.sp18 + 0xC | p1 + 0xC, fill 3 `p2 + 0x14` no mutation | 5 |

- [s3] The ONLY scheme reaching 0 is i_split2: the table local assigned at
  BOTH join-block fills, respelled as split-init. That is the banned two-site
  `tbl` carrier by another spelling (same intent: reg_n_sets == 2 on the
  addiu pseudo so sched.c:2505 birthing_insn_p is false). NOT submitted;
  banked at rejected/split-init-tbl-both-fills-score0-banned-respelling.c.
- [s3] Single-site split-init (b_split, j_splitafter) and struct-member
  split-init (c_memsplit) do NOT reproduce the effect: combine merges the
  copy into the add and decrements reg_n_sets (combine.c:2309/2332), so the
  addiu is back to one set and is deferred to just before its store. Measured
  8 (sp28-first) / 5 (sp2C-first), identical to the plain form.
- [s3] y_ptr0 (fill 1 reads the header through `ptr[0] + 0xC` instead of
  re-reading s.sp18, order sp1C/sp28/sp2C) is a NEW honest floor of 4 with no
  tbl carrier: idx 68-73 all match (seats and store order), leaving idx 84-86
  (3) plus one operand diff the diff helper does not flag, target
  `lw $v1,0x18($sp)` vs ours `lw $v1,0($s2)`. It is lateral: the target
  provably re-reads s.sp18 and every s.sp18-re-read spelling floors at 5.
  Saved as memory/grind/func_80069F80/candidate_floor4_ptr0.c.
- [s3] Header-reuse schemes (e, r, s, u: one pointer local carried across
  fills) are all WORSE (6): the extra sets change the local-alloc quantity
  order in block 9 and lose the fill-1 seats.
- [s3] Frame equation for the ruling request, from the target bytes alone
  (tools/gcc-2.7.2/config/mips/mips.c compute_frame_size: var_size =
  ALIGN8(vars), args_size = ALIGN8(outgoing), gp_reg_size = ALIGN8(saves)):
  frame 0x70; saves $s0-$s3,$ra = 20 -> 0x18; outgoing args for the 5-arg
  SetDrawMode call = 20 -> 0x18; locals region = 0x70 - 0x18 - 0x18 = 0x40 =
  64 bytes. Bytes actually touched in sp+0x18..0x57: 0x18..0x43 (the 0x2C
  descriptor, address taken by `addiu $a0,$sp,0x18`); 0x44..0x57 never read,
  written or addressed (grep of asm/funcs/func_80069F80.s). Fully-written
  form: ALIGN8(44) = 48 -> frame 0x60 != 0x70 (measured: 24 -> 12 when the
  tail was added, session 1). So no fully-written locals set reaches the
  target frame; the declared descriptor size is recoverable only as the range
  0x39..0x40 (s32 members: 0x3C and 0x40 byte-identical). Same shape as
  func_8006DD94 (Judge PASS docs/grind/decisions.md:26632 and :26636,
  live-object prong 2, same callee func_8007352C, same TU).
- [s3] In-tree precedent for the per-fill-reassigned pointer locals:
  func_80069E18 (src/text1b.c:5786-5809), Judge final-call PASS
  docs/grind/decisions.md:1231, commit 220b9c97b: `p0 = <hdr>; p1 = p0 + 0xC;
  s.p0 = p0; s.p1 = p1;` repeated at three func_8007352C fills, each local
  assigned three times - the exact idiom the 01:13 layer-1 FAIL named a
  multi-write carrier here.
