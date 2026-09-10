# Evidence bank — func_80074B18

## s1 (2026-09-10, recon) — SOLVED TO THE ASSEMBLER SEAM: honest floor 1, bytes proven with the label-nop gate

OBJECT MODEL: D_800A374C (census g_dma_buf_base, `extern s32 D_800A374C;` in src/text1b.c:6742
and include/m2c_context.h:289) — MATCHES. The target uses it as an OT base:
`lui/lw D_800A374C; sll a0,idx,2; addu a0,base,idx` (asm/funcs/func_80074B18.s:97-101).
`AddPrim(D_800A374C + ot * 4, p)` with the existing s32 declaration reproduces those four
instructions byte-for-byte (sandbox 1 form, pairdiff_v6.txt: no diff at that site). Every
other matched AddPrim caller in this TU uses the same `s32` shape. No declaration change
needed, no signal on the symbol. D_800A36A0 (`extern u8 *D_800A36A0;` src/text1b.c:6741) —
MATCHES: `lw a0,%gp_rel(D_800A36A0)($gp)` re-read per outer iteration, `lbu 0x65(a0)`,
`lw 4(a0)` -> `lw 0x3C(..)` all byte-identical with raw u8* offsets.

Chassis: HEAD 938d5bdf3, CC_FLAGS `-mel -msoft-float`, cc1 tools/gcc-2.7.2/build/cc1.
canonical: verdict C, hand_coded_tier LOW (no S1/S2/S6), 133 insns. Pure-C target.

### Measurements (all `sandbox func_80074B18 --disable all`, rules_dropped 0, 133/133 insns)
| form | score | delta cause |
|---|---|---|
| v1 raw offsets, ternary OT index in the call arg, `s32 n` | 46 | baseline |
| v2 = v1 + `ot = 0xB; if (arg1) ot = 0x15;` before AddPrim | 32 | OT index selected as a register value, one sll/addu (target shape) |
| v3 structs, `t++` before AddPrim, `p++` in the arg | 47 | biv increment placement diverges |
| v4 structs, `p++` in the arg, `t++` after | 42 | same |
| v5 = v2 with `t += 0xC` moved before AddPrim | 38 | same |
| **v6 = v2 with `s16 n`** | **1** | only the tail load-delay nop remains |
| v7 = v2 with `u8 n` | 3 | extra `andi v1,v1,0xff` at the preheader copy + spill slot shift |

The v6 body is memory/grind/func_80074B18/candidate.c. Diffs: tmp/grind/func_80074B18/s1/pairdiff_v*.txt.

### The one residual: ASPSX load-delay nop across a `.L` merge label (maspsx blind spot)
Target 0x80074CEC-0x80074CF4: `lw $a3,0x10($sp)` / `.L80074CF0:` / `nop` / `sw $s3,0x14($a3)`
(asm/funcs/func_80074B18.s:130-133). Our cc1 emits the identical stream with the label
between the load and its base-register consumer (tmp/grind/func_80074B18/dumps/text1b.s:
`lw $7,16($sp)` / `.L945:` / `sw $19,20($7)`); reorg.c relax_delay_slots (tools/gcc-2.7.2/
reorg.c:4003-4013) retargeted the top `beqz` past the redundant reload because `$a3` already
holds arg0 on that path, which is exactly what the target shows. maspsx `is_label()` only
recognises `$L` locals, so the base-register store consumer across the `.L` label loses its
hazard nop. The per-function gate (maspsx `__init__.py:832-838`, `line_loads_from_reg`
matches a store's `(base)` operand at `__init__.py:92`) is the documented PURE-C retirement
path (.claude/rules/maspsx-label-nop-gate.md; adjudicated FIDELITY in
.claude/rules/maspsx-gate-lists.md). No C spelling can emit an assembler hazard nop.

### Byte proof WITHOUT touching maspsx_label_nop_funcs.txt (tmp/ only)
* tmp/grind/func_80074B18/s1/build_gated.sh: exact Makefile pipeline for text1b (cpp -> cc1
  `-mel -msoft-float` -> prologue_fix -> maspsx with `--label-nop-funcs=` pointed at a SCRATCH
  copy of the list plus func_80074B18 -> multu_pad -> as) -> gated.o.
* engine.score.normalized_insns(gated.o) vs build/src/text1b.o: **zero differing instructions**
  (tmp/grind/func_80074B18/s1/gated_diff.txt is empty); the ungated v6.o differs only by the nop.
* tmp/grind/func_80074B18/s1/link_gated.sh: copy of the oracle-matching build/ tree with gated
  text1b.o swapped in, relinked via a path-rewritten bb2.ld, objcopy + make_psexe:
  **SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle** (gated_sha1.txt).
* The scope check forbids a session staging the gate list (precedent: func_80027640 s1 was
  discarded for doing so, docs/grind/decisions.md:20004-20020). It was never edited here.

### Mechanisms (pass-attributed from tmp/grind/func_80074B18/dumps/, form v1/v2)
1. `s32 n` vs `s16 n` (32 -> 1). jump.c duplicate_loop_exit_test copies the inner loop's
   exit block before the loop with fresh regs (147/148); cse substitutes j=0 giving
   `(set 148 (gt n 0))` + `(if (eq 148 0))`. loop.c will not hoist it: "Insn 267: regno 148
   (life 1), savings 1 not desirable" on the 77-insn outer loop (move rule loop.c:1631
   `threshold*savings*lifetime >= insn_count`, threshold = 1*(1+n_non_fixed_regs) ~ 29 with
   calls, loop.c:532). Left in-block, combine merges it into the jump and, because
   set_nonzero_bits_and_sign_copies (combine.c:724-733) recorded n's two sets (5|8, sign bit
   clear), simplify_comparison turns GT 0 into NE 0 (combine.c:8989-8995) -> `beqz n`, and n's
   own pseudo is spilled at both `li` sites. With `s16 n` (PROMOTE_MODE SImode pseudo) the
   compiler produces the target's shape exactly: `li v1,5/8` in a caller-saved reg, a
   preheader copy spilled to 40(sp), a real `slt a3,zero,v1` spilled to 48(sp), `beqz` on the
   reload. Measured, not derived: the exact RTL route for the s16 case was not traced (it was
   not needed once the sandbox read 1); a reader who cares should dump v6 and read
   .loop/.combine around the inner-loop entry.
2. OT index (46 -> 32): a ternary inside the call argument lets jump/cse fold `+ 0x2C/+0x54`
   into both arms; `ot = 0xB; if (arg1 != 0) ot = 0x15;` (same idiom as `n = 5; if (arg2) n = 8;`
   at the top, li in the delay slot) yields `li a0,11; li a0,21; sll; addu` once.
3. Biv increments: writing `AddPrim(...); p += 0x10; t += 0xC;` is correct; sched2 hoists the
   four increments above the jal by itself in the s16 form. Reordering them in C or using
   post-increment in the argument makes it worse (v3/v4/v5).
4. Outer entry test `beqz (lbu+3)`: combine GT->NE via nonzero_bits of `(zero_extend QI)+3`
   — natural `for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++)`, matched from v1 on.

### Data model recovered (for naming, not needed for bytes)
arg0[5] = prim cursor (16-byte TILE prims: tag, r0 g0 b0 code @4..7, x0 y0 @8,0xA, w h @0xC,0xE).
D_800A36A0->ptr4->0x3C = 12-byte tile defs {u16 x,y,w,h; u8 r,g,b,pad}. Rows =
D_800A36A0->u8[0x65] + 3; columns = arg2 ? 8 : 5. x += arg1*240 (player slot), y += row*34+0x2B
(arg2) or row*17+0x7C. OT index 0x15 if arg1 else 0xB.

Artifacts: tmp/grind/func_80074B18/s1/{v1..v7 .c/.o, pairdiff_*.txt, gated.s, gated.o,
gated_diff.txt, gated_sha1.txt, build_gated.sh, link_gated.sh, prove.sh, splice.py, pdiff.py,
measure.ps1}, tmp/grind/func_80074B18/dumps/ (form v1 cc1 pass dumps).

- [s1] OBJECT MODEL: D_800A374C (census g_dma_buf_base, extern s32 in src/text1b.c:6742) MATCHES - AddPrim(D_800A374C + ot * 4, p) reproduces lui/lw/sll/addu byte-for-byte; D_800A36A0 (extern u8 *) MATCHES; no declaration change

- [s1] canonical: verdict C, hand_coded_tier LOW, 133 insns; sandbox --disable all on candidate.c = 1 (133/133 insns, rules_dropped 0); the lone diff is the nop at 0x80074CF0

- [s1] Gated proof in tmp/ only: build_gated.sh (exact Makefile pipeline, --label-nop-funcs= scratch list + func_80074B18) -> gated.o, engine-normalized diff vs build/src/text1b.o empty; link_gated.sh relink SHA1 == oracle (gated_sha1.txt). maspsx_label_nop_funcs.txt was never edited

- [s1] Pass attribution (dumps of v1): loop dump 'Insn 267: regno 148 (life 1), savings 1 not desirable' (outer loop 77 real insns; threshold = 1*(1+n_non_fixed_regs), loop.c:532/1631); combine merges the entry compare into the jump and folds GT 0 -> NE 0 from reg_nonzero_bits (5|8) of the s32 n pseudo (combine.c:724-733, :8989-8995)

- [s1] reorg.c relax_delay_slots (tools/gcc-2.7.2/reorg.c:4003-4013) retargets the top beqz past the redundant lw $a3,0x10($sp), producing the target's .L80074CF0 label placement; our cc1 already does this, only the maspsx nop is missing

- [s1] Measured spellings: v1 46, v2 32, v3 47, v4 42, v5 38, v6 1, v7 3 (evidence.md table; rejected forms banked with reasons)

- [s1] docs/grind/decisions.md: 2026-09-10 func_80074B18 OWNER-ESCALATION - INTEGRATION HANDOFF entry filed with exact operator steps (mirrors func_80027640, decisions.md:20004)

- [s1] src/text1b.c restored to HEAD (INCLUDE_ASM) at session end; candidate lives in memory/grind/func_80074B18/candidate.c
