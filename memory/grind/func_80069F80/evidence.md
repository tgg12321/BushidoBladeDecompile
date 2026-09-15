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
