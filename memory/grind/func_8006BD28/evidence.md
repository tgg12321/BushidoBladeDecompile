# Evidence — func_8006BD28

## 2026-10-01 (laneA, manual) — byte-exact honest C: candidate.c, sandbox 0

Scored with `engine sandbox func_8006BD28 --disable all --diff --candidate` through a wrapper
(`tmp/func_8006BD28/sbx_typed.py`, gitignored) that also retypes the later forward declaration to
`S_6A880 *`, as the landing does. Re-baseline: pre-authorization body (`preauth_body.c`, main's
callee names) = 46.

### What closed it (48 -> 0)
1. **arg2 typed as the existing `S_6A880 *`** (`src/text1b_tu1c.c`, the 0x2C descriptor
   func_8007352C consumes; declared above the function). An alias local
   (`S_6A880 *arg2 = (S_6A880 *)arg2_p;`) costs the prologue order (v2 = 8: `move s1,a2` sinks
   below the `li` constants); the typed parameter gives the target prologue exactly.
2. **Frame 0x58 / vars=24 comes out naturally** (`.frame $sp,88 # vars= 24`): spill slots for
   `sheets` (sp+0x18), the hoisted `arg0 << 3` (sp+0x20) and the rotated inner-loop guard
   `(set (reg 117) (lt j n))`, which combine folds into `beqz s2` and leaves as
   `(use (reg 117))` -> `(use (mem sp+40))` in `.greg` (phantom-slot-frame-lever producer 1,
   sp+0x28). No pad. The old "8-byte frame skew" was the allocation: once the
   j*8 giv stays unreduced, the 9 callee-saved regs go to arg0..arg3, i, j, n, 0x12 and 8
   (target s4,s5,s1,s6,s3,s0,s2,s7,fp), and `sheets` + `arg0*8` spill.
3. **`n = (arg0 == 0x12) ? 3 : 1;`** — one write (the preauth `n = 1; if (..) n = 3;` was a
   multi-write local). If/else form also 0 (`rejected/alt-n-ifelse-0.c`); `(arg0 != 0x12) ? 1 : 3`
   = 3 (`rejected/n3-3.c`).
4. No `v` local: `arg2->header = ...; if (arg2->header == -1) return;` (a `v` local also gives 0,
   `rejected/alt-v-local-0.c`; simplest form lands).

### FAKE 1 — `cells` named intermediate (no-new-park-categories entry 6)
`cells = arg2->header + 0xC; arg2->table = cells + j * 8;` Prerequisites: once-written; real value
(target `addiu v0,v0,12` at 0x8006BE18); build_insns 103 == target 103; fresh; not live-pre-init.
Every inline spelling measured 31 (same diff): `rejected/c_inline-31.c`
(`header + 0xC + j * 8`), `c_paren-31.c` (`(header + 0xC) + j * 8`), `c_ptr-31.c` (u8 * cast form),
`c_arr-31.c` (`&((s32 (*)[2])(header + 0xC))[j]`).
Mechanism (loop dumps `cc1 -da`, build flags): in one expression, fold-const.c:3685-3703
(`associate:` / `split_tree`) moves the constant out: `(header + j*8) + 12`. loop.c then sees giv
`mult 8 add 12` (c_inline `.loop`: "Insn 149: giv reg 105 ... mult 8 add 12", "giv at 149 reduced
to (reg:SI 123)"), the reduced induction register takes a callee-saved reg (123 in 18), and the
hoisted constant 8 loses its register (rematerialized `li` in the loop). With `cells`, the giv is
only `mult 8 add 0` and loop.c:3828 rejects it ("giv of insn 152 not worth while, 0 vs 28"), as
in the target (`sll a1,s0,3` every iteration). Same-TU idiom: `cells = s.header + 0xC; s.table = cells;`
in func_8006A880 / func_8006C21C (src/text1b_tu1c.c).

### FAKE 2 — operand grouping `*(sheets + i + arg0 * 2)` (or-tree-shape-shift carve-out)
Target: `sll t0,s4,3` in the prologue, spilled to sp+0x20; per iteration
`sll v0,s3,2; addu v0,v0,<sheets>; addu v0,<arg0*8>,v0`. Natural orders measured dead:
`rejected/r_rowmajor-33.c` (`sheets[arg0 * 2 + i]`), `r_idxsum-33.c` (`sheets[i + arg0 * 2]`),
`r_rowptr-51.c` (`(sheets + arg0 * 2)[i]`), `r_ptrrow-51.c` (`*(sheets + arg0 * 2 + i)`),
`r_2d-51.c` (`((s32 (*)[2])sheets)[arg0][i]`). (`(sheets + i)[arg0 * 2]` also 0.)
Mechanism (outer loop, `.loop` dumps): candidate — "Insn 52: regno 88 (life 3), savings 1 moved to
253" = `(ashift (reg 72 arg0) 3)` hoisted alone (loop.c:1631 desirability test passes, life 3);
the index sum stays in the loop. Index-sum spellings scale `(arg0*2 + i)` as one value: insn 52 is
`arg0 << 1`, "life 1 ... not desirable", nothing hoisted, one spill fewer (vars 16). Row-pointer
spellings hoist `sheets + arg0*8` whole ("Insn 57 ... cond forces 55 ... moved") and loop.c
strength-reduces `i*4 + that` ("giv at 59 reduced to (reg:SI 124)") into a callee-saved reg (vars 8).

### Data model / other consumers
- Caller func_8006BEC4: `s32 sp10[12]` -> `S_6A880 sp10`, calls pass `&sp10`; the forward
  declaration `extern void func_8006BD28(s32, s32, s32 *, s32);` is removed (definition precedes);
  the `D_800A36E0/E4` externs move above func_8006BD28. Whole-TU compile old vs new: only
  func_8006BD28 differs (`tmp/func_8006BD28/tucmp.sh`); func_8006BEC4 byte-identical.
- Casts kept: `*(s32 **)(*(s32 *)(D_800A34FC + 0x24) + 0x20)` (same byte-offset model as the
  caller's `*(Vec2s16 **)(*(s32 *)(D_800A34FC + 0x24) + 0x48)`); `(s32)` / `(u8 *)` conversions
  between the `u8 *` globals D_800A36E0/E4 and the s32-typed prototypes / descriptor fields.
