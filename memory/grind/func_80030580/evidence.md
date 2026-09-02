# Evidence bank — func_80030580

## Session 1 (2026-09-02, recon) — floor 45 → 2 (frame only)

### Function semantics (from asm/funcs/func_80030580.s, 148 insns, no calls)
Allocates one of 12 objects (stride 0x64) in the table at `D_80106A78` and initialises it
from a "source" struct (`arg0`, the player/character block) and a 14-byte-stride entry
table `D_8008E194[arg1]` (s16 fields: +0 kind, +4 speed, +6 vertical speed, +8 spin).
Free-slot test: `*(s16*)(obj+2) == -1 && *(u8*)(obj+0xA) == 0xFF`. Velocity uses the
sin/cos table `Judge` (declared `extern s16 Judge;` in this TU; index `(&Judge)[...]`)
with angle at `src+0x1CA` (read THREE separate times in the target: lhu for `&0xFFF`,
lh for `+0x400 & 0xFFF`, lhu for the copy into obj+0x56 — so it is not a single s16 local).
Callers (func_800307D0 / func_80030900 / cpu_set_move_command_and_dir) treat the result
as `s32 *` and poke raw offsets, so raw `u8 *obj` offsets are the TU-consistent spelling.

### Chassis
- `canonical func_80030580` → C (pure-C distance ≤ 50). HEAD honest floor at dispatch: 38
  (migration pin, pre-migration cheat-laden body in retired-chassis-2026-08/body.c —
  register pins, volatile coercion, barriers; do NOT restore it).
- s1 clean rewrite: draft1 = 45, draft2 = 15, draft3 = **2** (`memory/grind/func_80030580/candidate.c`).

### Body findings (all measured with `sandbox --disable all` + tools/pairdiff.py)
1. Loop shape that matches: explicit pointer biv, `for (i = 0; i < 12; i++, obj += 0x64)`
   with the break test in the body. loop.c makes `v1 = obj + 0xA` a giv serving both loads
   (`lh -8(v1)`, `lbu 0(v1)`); `a3` (obj) is bumped in the branch delay slot. Increment order
   in the target is `i++` THEN `obj += 0x64` (draft1 with `obj += 0x64` at the body end gave
   `addiu v1,100` before `addiu a0,1` → 2 insns off). Writing `obj = base + i*0x64` inside the
   body (draft2) makes obj a derived giv → extra `move`/`addiu` (score 15).
2. `if (k == 1) A else if (k == 2) B else if (k == 3) A else C` with A written twice matches
   (jump.c cross-jump merges the two A bodies; compare order 1,2,3 linear). A `switch` emits a
   compare tree (`li 2; beq; slti 3; beqz`) → 6 extra insns.
3. The `li v0,1` before the loop-exit label in both paths is reorg.c filling the `beq` delay
   slot from the target thread (the `obj+4 = 1` constant) — ordinary, no source construct.
4. Statement order in the post-loop block is body-sensitive even though sched reorders it:
   vel stores x,y,z then `pos += vel` (x,y,z) then `pos += vel/2` (x,y,z) is the only order
   measured to match. Permutations zxy/yxz/xzy, interleaved (x,x/2,y,y/2,...), vel folded into
   the += statements, `tbl` computed before the pos stores or at the top, `idx` locals,
   `tbl[2] * Judge[]` operand order, s16 `ang`/`sn,cs` locals — all change the body (18–185
   fdiff lines). Banked as instance kills in hypotheses.md.
5. `*(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C)` and three scalar copies both match the
   body (3 lw then 3 sw); `Vec3i` is the file-local typedef at src/code6cac_b.c:855.
6. `t1`/`t0` allocation (src copy in t1, `-1` in t0) fell into place once the loop shape was
   right — no lever needed.

### Frame finding (the whole remaining residual: target `addiu sp,-0x18`, ours `-8`)
- Target reserves 24 bytes of locals and touches NONE of them (no sp-relative access in the
  body; no saved regs). Ours reserves 8. Frame equation: vars only (args=0, regs=0).
- Instrument: `tmp/grind/func_80030580/s1/frame.py` (+ `frame.sh` / `frame_instr.sh`) applies a
  variant into src, runs the project cpp|cc1 (instrumented `tools/gcc-2.7.2/cc1` with
  `BB2_FRAME_DEBUG=1` under `frame_instr.sh`), prints the `.frame` line, the FRAMEDBG slot
  census, and the fdiff line count vs `var_base.s`. Run: `bash tools/wsl.sh 'bash
  tmp/grind/func_80030580/s1/frame_instr.sh <variant...>'`. Variants live in frame.py + extra.py.
- Our 8 bytes = `FRAMEDBG ctx=spill_new_p110 mode=SI size=8` — reload gives a stack slot to
  pseudo 110, whose ONLY reference is `(insn 393 (use (reg:SI 110)))` sitting right after
  `code_label 99` (the block after the `bgez` of the `/32`). Pseudo 110 was the first Judge
  lookup's address (`110 = idx*2 + &Judge`, .rtl insn 136); combine folded it into
  `lh %lo(Judge)(at)` and its REG_DEAD note found no home — combine.c:10836
  (distribute_notes: walk back from i3 hits a CODE_LABEL/JUMP → `emit_insn_after (USE reg)`).
  lreg: "Register 110 used 2 times ... ST_REGS or none; pointer" → no hard reg → spill slot,
  but a USE emits nothing → phantom bytes. The second (cos) lookup has identical RTL shape
  (.rtl insns 166–175) yet produced NO orphan — the combine attempt order differs (unresolved).
- So target 24 = ours 8 + 16: either TWO more orphan-USE pseudos (8 each after align-8
  rounding) or ONE untouched BLKmode stack temp of 16 (a 12-byte Vec3i temp rounds to 16:
  measured `ctx=stack_temp mode=BLK size=16` for a real `Vec3i tmp` local — but a real temp
  emits 3 sw/3 lw to sp, so it must be a temp that expand allocates and never touches).
- Body-neutral variants measured at vars=8 (no gain, all fdiff=0): `sfirst` (s16 read for the
  first index), `tblu8` (`(u8*)&D_8008E194 + arg1*14`), `sn32` (s32 sin/cos locals), `ptrjudge`
  (`*(&Judge + i)`), `bytejudge` (`*(s16*)((u8*)&Judge + i*2)`), `kind` (s16 local for the
  chain), `dy` (s16 local for /32), `pv`/`pv_late`/`pos_only` (`Vec3i *pos/*vel` pointers into
  obj), `srcp` (`Vec3i *sp` into src), `nocopy` (scalar copies), `vec3tmp`→24 but touches sp.
  `jp` (`s16 *jp = &Judge` pointer alias) → vars=0 AND body changes (19 lines) — kills the
  pointer-alias direction outright.

### Sibling
`func_80032064` (src/code6cac_b.c:2225, matched C in-tree) is the same idiom (same src
offsets 0xF4/0x1A/0x1CA, same Judge lookups, Vec3_copy struct copy) — its matched source
spells the /32 by hand (`if (v1 < 0) v1 += 0x1F; ... >> 5`) and uses `s32 speed` constant
locals; its frame is 0x28 with a call, so not directly comparable, but worth a FRAMEDBG census
to see whether its phantom count also comes from Judge-address orphans.

- [s1] canonical: C, pure-C distance <= 50; migration pin floor 38 was a cheat-laden body (retired-chassis-2026-08/body.c) — superseded by the clean draft3 at 2

- [s1] Target reserves 24 locals bytes with zero sp-relative accesses (phantom slots); ours 8; frame is the entire residual (pairdiff: 2 insns)

- [s1] Angle at src+0x1CA is loaded three separate times in the target (lhu/lh/lhu) — not one s16 local

- [s1] Vec3i struct copy and three scalar copies both match the 3-lw/3-sw copy block

- [s1] Sibling func_80032064 (src/code6cac_b.c:2225, matched) uses the identical idiom (same src offsets, Judge lookups, Vec3_copy) — template for spelling; its frame 0x28 includes a call

- [s1] Frame instrument: tmp/grind/func_80030580/s1/frame.py + frame_instr.sh (BB2_FRAME_DEBUG census, .frame line, fdiff count) — reusable next session
