# Evidence — func_8006A564 (src/text1b.c)

## Session 1 (recon)

- **OBJECT MODEL: MATCHES.** The brief's DATA MODEL flagged `D_800A374C`
  (census: `g_dma_buf_base`, decl `extern s32 D_800A374C;` in
  `include/m2c_context.h`). Grepped every existing use in `src/text1b.c`
  (60+ sites across the file, e.g. lines 1661/3373/3453/3607/3661/...):
  ALL treat it as a plain `s32` OT/DMA-buffer base added to a `<<2`-scaled
  index and passed as the first arg to `AddPrim(s32, void*)`. The target
  asm for func_8006A564 (`asm/funcs/func_8006A564.s:49-53`,
  `:87-91`, `:127-131`, `:191-195`) does the identical
  `lui/lw %hi/%lo(D_800A374C); sll a0,a0,2; addu a0,v0,a0` sequence four
  times. No declaration mismatch, no split-scalar signal, no
  base-register-store-invisible-to-grep pattern found. Object model needs
  no fix; this is an ordinary global read, not the residual's source. No
  measurement needed beyond the grep (it's a decl-vs-use consistency check,
  not a codegen question) — MATCHES is the finding, not a score.

- Function is not yet decompiled: HEAD was `INCLUDE_ASM("asm/funcs",
  func_8006A564);` (src/text1b.c:6418 before this session's edit).
  `no_c_body: true` on the pre-session sandbox run.
- `canonical func_8006A564` = verdict C, hand_coded_tier LOW (no S1/S2/S6
  signal) — ordinary pure-C target, not a canonical-asm candidate.
- Target asm (`asm/funcs/func_8006A564.s`, 209 lines / 199 real insns,
  `.frame` implied by `addiu sp,sp,-0x30`) is a leaf-ish rendering routine:
  reads `arg0` (a struct with a growing tile-buffer pointer at +0x18, a
  second-object pointer at +4, and a draw-mode field at +0x1C), `arg1` (a
  ~0x2C-byte "packet" struct — same shape as `func_80075830`'s local
  `u8 packet[0x2C]`, src/text1b.c:8303 — with byte color fields at
  0x29/0x2A/0x2B, matching `src/text1b.c:8311-8313`), and `arg2` (an int
  compared against `D_800A34F8 & 0xF` three times).
- Wrote an initial full-body C translation (see candidate.c) mirroring the
  asm literally: 3x `SetTile`/color-select-if/`SetSemiTrans`/`AddPrim`
  tile-draw blocks (color bytes chosen by `D_800A34F8&0xF == arg2`), then a
  second-object dereference chain (`*(arg0+4)` -> `+0x1C` -> `+0x28`) whose
  result feeds `arg1`'s packet, a similar color-select block (halving
  instead of literal 0x50/0x20/0x10), two calls to `func_8007352C(arg1)`
  (each preceded by staging a different source field into `arg1+8`), and a
  closing `SetDrawMode`/`AddPrim`/`+=0xC` on `*(arg0+0x1C)`.
- **First sandbox measurement of a real C body: score 137** (was 199 /
  no-c-body). `build_insns 204` vs `target_insns 199` — the body compiles
  and is structurally very close (+5 insns) but not byte-identical.
- **srl-vs-sra byte diff, KILLED.** The two `color >> 1` halving ops
  (block 3 and the final color-select block) initially emitted `sra`
  (arithmetic shift) because `v0` is declared `s32` and GCC's default
  shift-right on a signed type is arithmetic. Target emits `srl` (logical)
  at both sites (`asm/funcs/func_8006A564.s:101/105/148/149`). Fix: cast
  through `(u32)` before the shift (`v0 = (u32)v0 >> 1;`). Confirmed via
  objdump of `tmp/sandbox/func_8006A564/text1b.o` (turn ~14 of this
  session): both sites now emit `srl s0,s0,0x1` matching target exactly.
  Sandbox score UNCHANGED at 137 (the srl/sra opcode swap was already
  counted at the same weight either way in the masked score, or was
  already dwarfed by the larger register-allocation diff below — not
  independently isolated this session).
- **Register-allocation mismatch, register mapping only — not yet
  resolved.** Disassembled `tmp/sandbox/func_8006A564/text1b.o` (objdump,
  turn ~11): our build allocates `s3=arg0, s2=arg1, s4=arg2,
  s5=(the local I call \`s4\` in C — the SetSemiTrans-arg /
  func_8006E480-arg2 flag), s0/s1 scratch`, needs 6 callee-saved regs
  (s0-s5+ra) and a `-56` (0x38) byte frame (`addiu sp,sp,-56`, extra
  `sw s5,44(sp)` vs target). Target (`asm/funcs/func_8006A564.s:2-10`)
  allocates `s2=arg0, s1=arg1, s3=arg2, s4=(the same flag local)`, only
  5 callee-saved regs (s0-s4+ra), frame `-0x30`. This is the entire
  +5-instruction gap (204 vs 199): our build's extra callee-save
  push/pop (`sw`/`lw s5`) plus the bigger `addiu` immediates. No lever
  applied yet this session (recon budget) — this is the frontier for the
  next session.
- No permuter run this session (recon modality — establishing the
  baseline candidate and object-model check was the mandate).

- [s1] OBJECT MODEL: MATCHES for D_800A374C -- grepped 60+ existing src/text1b.c use sites, all plain s32 OT-base + <<2-scaled-index feeding AddPrim(s32,void*); target asm does the identical lui/lw hi/lo + sll 2 + addu sequence 4x (asm/funcs/func_8006A564.s:49-53,87-91,127-131,191-195). No declaration mismatch, no split-scalar/base-register-invisible signal. Needs no fix.

- [s1] Function had no C body at session start (INCLUDE_ASM, no_c_body:true, distance 199); canonical verdict C, hand_coded_tier LOW -- ordinary pure-C target, ruled out canonical-asm routing.

- [s1] Wrote a full literal C translation of the 199-insn target (3x SetTile/color-select/SetSemiTrans/AddPrim tile-draw blocks + second-object dereference chain + two func_8007352C(arg1) calls + closing SetDrawMode/AddPrim) -- first-cut sandbox score 137, build_insns 204 vs target 199 (+5 insns).

- [s1] Disassembled tmp/sandbox/func_8006A564/text1b.o: our build allocates {s0,s1,s2,s3,s4,s5} (6 callee-saved, arg0=s3,arg1=s2,arg2=s4, flag-local=s5) with an 0x38-byte frame; target allocates only {s0,s1,s2,s3,s4} (arg0=s2,arg1=s1,arg2=s3,flag-local=s4) with an 0x30-byte frame (asm/funcs/func_8006A564.s:2-10). This register-mapping/frame-size difference is the entire +5-insn residual.
