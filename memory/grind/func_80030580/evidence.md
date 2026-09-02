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

## Session 2 (2026-09-02, structural) — floor 2 (unchanged); the frame residual is now MECHANISM-TYPED

### Chassis re-measured this session
`sandbox func_80030580 --disable all` = **2** with draft3 (= candidate.c) applied to
src/code6cac_b.c. Body still byte-identical; the whole residual is `addiu sp,-0x18`
(target, vars=24) vs `addiu sp,-8` (ours, vars=8).

### THE DECISIVE NEW DATUM — corpus census of leaf-function frames
`tmp/grind/func_80030580/s2/census2.py` compiles a whole `src/*.c` with the instrumented
cc1 (`BB2_FRAME_DEBUG=1`), groups every FRAMEDBG allocation by function, and cross-reads
`vars=`/`regs=`/`args=` from the `.frame` line plus the count of `($sp)` accesses in the
emitted body. Run over 20 source files (code6cac, code6cac_b, code6cac_c, code6cac_c0,
code6cac_c2, code6cac_c_ab, code6cac_c_mid, config, display, gpu, ings, ings2, main,
sound, system, text1a_b, text1a_c, text1a_c2, text1b, text1b_b), filtered to functions
with the SAME frame signature as our target (`args=0, regs=0, sp_acc=0` — a leaf that
touches none of its frame), every single function with `vars >= 16` is composed
**exclusively of `ctx=spill_new_pNNN` slots, 8 bytes each**:

    config:func_8003FECC      vars=16  spill_new_p84  + spill_new_p115
    display:get_cs            vars=16  spill_new_p85  + spill_new_p99
    display:get_ce            vars=16  spill_new_p85  + spill_new_p99
    main:SpuSetCommonAttr     vars=16  spill_new_p98  + spill_new_p120

NO leaf anywhere in the scanned corpus reaches a non-zero `vars` through a BLKmode
`ctx=stack_temp`. In this TU `ctx=stack_temp` appears only in functions that HAVE calls
(func_800300B4 16/8/32 with args=16; func_8002FF20 32; func_80032064 8 = its real
`s16 sp_area[2]`), i.e. aggregate-argument / declared-aggregate temps, all of which emit
sp traffic. **Therefore the target's 24 bytes are three 8-byte orphan-USE spill slots**
(frontier F1), and F2 (one untouched 16-byte BLKmode temp) is not the shape of this frame.

### The generative recipe for an orphan spill slot, read off a matched witness
`get_cs`/`get_ce` (src/display.c:556/574, both COMPLETED-C leaves) are the template: each
clamps against a **distinct halfword global** (`D_8009BE78`, `D_8009BE7A`) inside a
ternary, so each global's `symbol_ref` pseudo is folded by combine into a `lh %lo(SYM)(at)`
mem while a CODE_LABEL from the ternary sits between the pseudo's (now deleted) setter and
the folding point. combine.c:10836 then emits `(use (reg N))` after the label, lreg gives
the pseudo no class (its only reference is a USE), reload's `alter_reg` hands it a stack
slot, and the USE emits nothing. Two such globals ⇒ 2 slots ⇒ vars=16.
Our function reads exactly three globals: `Judge` (folded twice — confirmed in the target
bytes: two `lui $at,%hi(Judge)` + `lh %lo(Judge)($at)` pairs at 0x80030654/0x80030698),
`D_8008E194` and `D_80106A78` (both materialised as real base registers via
`lui`+`addiu`, so their symbol pseudos stay live and cannot orphan).

### The open asymmetry (this is the crack to widen)
Pre-combine RTL (`tmp/grind/func_80030580/s2/base.rtl`) shows the two Judge lookups are
STRUCTURALLY IDENTICAL: insn 131 `(set (reg 107) (symbol_ref "Judge"))` → insn 136
`(set (reg 110) (plus (reg 109) (reg 107)))` → insn 138 `(set (reg:HI 111) (mem (reg 110)))`
for the sin lookup, and insn 166/171/173 with regs 127/130/131 for the cos lookup.
combine folds BOTH into `(mem (plus (reg idx) (symbol_ref "Judge")))` (combine insns 140
and 175) — but only reg 110 produces an orphan USE (`insn 393 (use (reg:SI 110))` right
after `code_label 99`, the `/32` `bgez` target). reg 130's REG_DEAD note is dropped
(most likely the `elim_i2` early-`break` at combine.c:10761, i.e. reg 130 was exactly the
register the i2→i3 fold eliminated, so the note never reaches the label-walk). Why the
first lookup escapes that path and the second does not is UNRESOLVED and is the single
highest-value forensics question left on this function.

### 44 structural variants measured this session — every one at vars=8, one spill slot
Instrument: `tmp/grind/func_80030580/s2/frame2.py` (+ `extra2.py`), same harness as s1 —
applies a variant to src, runs the project cpp|instrumented-cc1, prints `vars=`, the
FRAMEDBG census, the `($sp)` count, and the fdiff line count vs `s1/var_base.s`.

Body-NEUTRAL (fdiff = 0) and still vars=8 — these are free to compose with any future
lever, which is their value: `hd32` (sibling func_80032064's hand-spelled `/32`:
`{ s32 v1 = *(s16*)(src+0x1A); if (v1 < 0) v1 += 0x1F; ... - (v1 >> 5); }`),
`castjudge` (`(s32)*(&Judge + idx)` sibling spelling), `hd32cast`, `vely` (`s32 vel_y`
local), `dord1`/`dord2`/`dord3` (all declaration-order permutations of obj/src/tbl/i),
`lp_nest` (nested ifs instead of `&&`), `lp_while` (while form with the bumps in the
body), `c_255` (`255` for `0xFF`), `acopy` (u16 local for the angle copy), `slot8`
(`(u8)i` cast), `a0idx` (src assigned in the body instead of the declarator), `s32idx`
(`s32 *src` with word indices `src[61..63]`), `ang32` (`s32 ang` local feeding the FIRST
Judge index only), `arridx` (`((Vec3i*)(obj+0x2C))[1] = [0]`), `arridx2`, `compref`
(function-scope `struct PB { Vec3i pos, old, vel; }` with `pb->old = pb->pos`),
`comprefp`, `unioncpy` (function-scope union member copy), `jmix2`.

Body-CHANGING (recorded so they are never re-proposed): `spd`/`sibfull` (65),
`veccopy` (14, `Vec3_copy` typedef instead of `Vec3i`), `blk1` (2), `i16` (11), `iu` (2),
`lp_hi` (4), `z16` (13), `spin` (161), `tbl_late` (18), `hd32top` (16), `copylast` (20),
`bothu16` (2), `u16cast` (2), `secondzx` (3), `firstsx` (2), `tblidx` (64),
`tblidx_arm` (152), `posy_mid` (32), `posy_last` (196).

One variant moved the frame DOWN: `jmix` (second Judge lookup spelled
`*(s16*)((u8*)&Judge + (idx << 1))`) → **vars=0**, fdiff 4. It removes the only orphan,
confirming from the other side that the single slot we do have is produced by the SECOND
lookup's combination, not the first.

- [s2] Corpus census (20 files): in leaf functions (args=0, regs=0, 0 sp accesses) every non-zero `vars` is built only from 8-byte `ctx=spill_new` orphan slots — never a BLKmode `ctx=stack_temp`; target vars=24 ⇒ THREE orphan slots
- [s2] get_cs / get_ce (src/display.c:556/574, COMPLETED-C leaves) are the 2-orphan witnesses: two DISTINCT globals folded into `lh %lo(SYM)(at)` across a ternary's CODE_LABEL ⇒ vars=16
- [s2] func_80030580 reads only three globals; D_8008E194 and D_80106A78 are materialised as live base registers in the target bytes (`lui`+`addiu`), so only `Judge` can orphan
- [s2] Both Judge lookups have identical pre-combine RTL and both are folded, yet only one orphans; reg 130's REG_DEAD note is dropped (probable `elim_i2` break, combine.c:10761)
- [s2] 15 body-neutral respellings banked (hd32, castjudge, dord1-3, lp_nest, lp_while, a0idx, s32idx, ang32, arridx, compref, unioncpy, vely, acopy) — all vars=8, all composable
- [s2] Instrument: tmp/grind/func_80030580/s2/frame2.py + extra2.py (variant sweep) and census2.py (corpus frame census) — reusable

- [s2] Chassis re-measured this session: sandbox func_80030580 --disable all = 2 with candidate.c (draft3) applied to src/code6cac_b.c; 148/148 insns, body byte-identical, residual is addiu sp,-0x18 (vars=24) vs ours -8 (vars=8).

- [s2] Corpus census over 20 src/*.c: in leaf functions (args=0, regs=0, zero ($sp) accesses) every non-zero vars is built only from 8-byte ctx=spill_new orphan slots; ctx=stack_temp never appears in such a leaf. Target vars=24 therefore means THREE orphan slots.

- [s2] The 2-orphan witnesses are matched, in-tree C: display.c:556 get_cs and display.c:574 get_ce (vars=16 each, two ctx=spill_new), plus config:func_8003FECC and main:SpuSetCommonAttr. Their recipe is one slot per DISTINCT global folded into lh %lo(SYM)(at) across a CODE_LABEL emitted by a ternary/if.

- [s2] func_80030580 reads exactly three globals. The target bytes materialise D_80106A78 (lui+addiu at 0x80030588) and D_8008E194 (lui+addiu at 0x80030620) as live base registers, so their symbol pseudos cannot orphan in any matching form; only Judge is folded (two lui %hi(Judge) + lh %lo(Judge)($at) pairs at 0x80030654 and 0x80030698).

- [s2] Pre-combine RTL shows the two Judge lookups are structurally identical (insn 131 set reg107 = symbol_ref Judge / insn 136 reg110 = plus / insn 138 mem, versus insns 166/171/173 with regs 127/130/131), combine folds both (combine insns 140 and 175), yet only reg 110 produces insn 393 (use (reg:SI 110)) after code_label 99 - the /32 bgez target.

- [s2] 44 structural variants measured at vars=8 with one slot; 15 of them are body-neutral and banked as composable, including the sibling func_80032064 idioms (hand-spelled /32 and (s32)*(&Judge + i)) that s1 had flagged as the next thing to port - they transfer cleanly but are frame-inert.

- [s2] Reusable instruments written this session: tmp/grind/func_80030580/s2/frame2.py + extra2.py (variant sweep printing vars=, the FRAMEDBG census, the ($sp) count and fdiff) and census2.py (corpus-wide leaf frame census).

## Session 3 (2026-09-02, structural) — floor 2 (flat); the frame residual now has a GENERATIVE LAW

### Chassis re-measured this session
`sandbox func_80030580 --disable all` = **2** with candidate.c (= s1/draft3.c) applied to
src/code6cac_b.c. Unchanged: body byte-identical, residual is `addiu sp,-0x18` (vars=24)
vs ours `-8` (vars=8).

### THE DECISIVE NEW DATUM — orphan slots scale with the number of INDEXED Judge sites
Measured with the s2/s3 harness (`tmp/grind/func_80030580/s3/frame3.py`, instrumented cc1,
`BB2_FRAME_DEBUG=1`), by adding/removing whole `(&Judge)[...]` lookup sites:

    j0read  (0 lookup sites)  vars= 0   0 spill_new
    j1only  (1 lookup site)   vars= 0   0 spill_new
    base    (2 lookup sites)  vars= 8   spill_new_p110
    j3read  (3 lookup sites)  vars=16   spill_new_p110 + p130
    j4read  (4 lookup sites)  vars=24   spill_new_p110 + p130 + p143

**The law is `slots = max(0, sites - 1)`, one 8-byte `ctx=spill_new` slot each.** The
target's vars=24 therefore requires **FOUR source-level indexed-Judge lookup sites**, of
which the emitted loads for two must disappear again — the target's bytes contain exactly
two `lui %hi(Judge)` + `lh %lo(Judge)($at)` pairs (asm/funcs/func_80030580.s:59/61 and
76/78) and no labels anywhere near them (straight-line block 0x80030644-0x800306D0).

### The generator is the INDEXED fold, not a global read as such
`otherglob` / `otherglob2` add one and two reads of unrelated constant-address globals
(`D_8008EBA0`, `D_80101E02`) — both stay at vars=8 (bodydiff 6 / 10). A global read whose
address needs no index register is folded into a plain `lh %lo(SYM)($at)` and orphans
nothing. The orphan comes specifically from the `(mem (plus (reg idx) (symbol_ref)))`
fold, i.e. an ARRAY-INDEXED global read. This kills the s2 reading of the recipe as
"one slot per DISTINCT global".

### s2's "CODE_LABEL is required" reading is also dead
New 3-orphan witness found by a whole-corpus census (`tmp/grind/func_80030580/s3/census3.py`
over every `src/*.c`, filtering for >=3 `ctx=spill_new` or a leaf with vars>=24):
**`text1a_post:func_80041E10` (src/text1a_post.c:465)** — `vars=24, regs=0, args=0,
sp_acc=0`, three `ctx=spill_new` slots — exactly the target's frame signature. Its body
contains **no branch and no label at all** (three `mult`/`mfhi` divide-by-255 sequences
and three `sh $x,g_anim_select+k` stores). So the CODE_LABEL that s2 saw next to our
`insn 393 (use (reg 110))` is incidental, not a precondition.
The direct N-orphan witness for our own generator is **`text1a_c:func_80042874`
(src/text1a_c.c:193)** — ~6 indexed `Judge[...]` lookups, 6 `ctx=spill_new` slots, vars=48.
Other >=3-spill functions in the corpus: code6cac:func_8001B478 (3), display:SetDrawEnv /
SetDrawEnv2 (3 each), text1a_c:func_80042A88 (6), text1a_c2:func_800460E4 (3),
text1a_post:func_80041AC8 (3), text1b:func_80060E38 (9).

### Why a 3rd/4th site cannot (yet) be added for free
- A duplicate whose value CSE can prove redundant is deleted **pseudo and all**:
  `dupread` (`sn = Judge[A]; sn = Judge[A];`) is byte-neutral (fdiff 0) and stays at
  vars=8. So the extra site must survive to combine.
- The only byte-REMOVING pass after combine is jump2 cross-jumping (2.7.2 order: jump,
  cse, loop, cse2, flow, **combine**, sched, lreg, greg, reload, jump2, sched2, reorg).
  Every extra site measured so far costs emitted insns: `dupX` +15 fdiff (vars=16),
  `dupZ` +16 (vars=16), `dupXZ` +199 (**vars=24** — the right frame, wrong body),
  `j3read` +18 (vars=16), `arms2` (vel.x duplicated into the two already-cross-jumped
  kind-chain arms 1 and 3) +17 (vars=16), `armsvx_keep` +73 (vars=16), `armsvx`
  (moved into all four arms) +84 (vars=8).

### Structural variants measured this session — all body-neutral ones still vars=8
Harness: `tmp/grind/func_80030580/s3/frame3.py` (+ `extra3.py`), same shape as s2's but
it also applies FILE-level substitutions (declaration levers) and prints `TUdiff` = how
many lines of the REST of the TU changed, so a declaration lever is only usable at
TUdiff=0. Reference full-TU asm: `tmp/grind/func_80030580/s3/tu_ref.s`.

Body-neutral (fdiff 0) and vars=8 — free to compose, banked:
`jlocalext`, `alllocalext`, `jlocalext_late` (block-scope `extern` redeclaration of
Judge / D_8008E194 / D_80106A78), `s32addr`, `s32addr1`, `s32addr2` (Judge address via
`*(s16 *)((s32)&Judge + (idx * 2))`), `modmask` (`% 0x1000` for `& 0xFFF`),
**`judgearr`** (TU-wide `extern s16 Judge[];` + `Judge[i]` at every use site in
code6cac_b.c — TUdiff = 0, so it is safe for the other matched functions in the file),
`hi1`, `qi6`, `hitest`, `hiarg`, `notmask` (`& ~0xF000`), `notmask2` (`& ~(-0x1000)`),
`notmaskboth`, `hloc`, `dupread`, `tblglob0`, `tbl2decl` (a redundant second
`&D_8008E194 + arg1*7` local that CSE merges).

Body-CHANGING this session (recorded so they are never re-proposed): `d32tern` (41 —
the `/32` as a ternary `(v<0 ? v+0x1F : v) >> 5`; the get_cs/get_ce ternary shape does
NOT transfer), `div2tern` (199), `bothtern` (216), `d32tern_x` (206), `hi2` (2),
`hi12` (2), `hloc2` (2), `tblglob2` (46), `tblglob4` (152), `objglob` (33), `j3read` (18),
`j4read` (25), `j1only` (157), `j0read` (177), `dupX` (15), `dupZ` (16), `dupXZ` (199),
`arms2` (17), `armsvx` (84), `armsvx_keep` (73), `otherglob` (6), `otherglob2` (10).

### Note for the next session: the F2.1 forensics probe is NOT available to a grind session
s2's frontier asked for a `BB2_COMBINE_DEBUG` print inside `distribute_notes`. That means
editing `tools/gcc-2.7.2/combine.c`, which the grind-session contract forbids
(`NEVER edit .claude/rules/engine/tools/Makefile/*.ld`). The `slots = sites - 1` law above
was obtained without it and supersedes the question it was asked to answer.

- [s3] Orphan spill slots scale as `slots = max(0, indexed-Judge sites - 1)`: 0/1 sites -> vars=0, 2 -> 8, 3 -> 16, 4 -> 24 (variants j0read/j1only/base/j3read/j4read). The target's vars=24 needs FOUR source-level lookup sites while emitting only two `lh %lo(Judge)` loads.
- [s3] The generator is the ARRAY-INDEXED global fold `(mem (plus (reg idx) (symbol_ref)))`, not a global read in general: adding one or two reads of constant-address globals (otherglob/otherglob2) leaves vars=8.
- [s3] s2's "one slot per DISTINCT global, needs a CODE_LABEL" recipe is refuted: text1a_post:func_80041E10 (src/text1a_post.c:465) has vars=24/regs=0/args=0/sp_acc=0 with three spill_new and NO branch or label in its body; text1a_c:func_80042874 (src/text1a_c.c:193) has 6 spill_new from ~6 indexed Judge lookups.
- [s3] A CSE-redundant duplicate lookup is deleted pseudo and all (dupread: fdiff 0, vars=8); an extra site must survive to combine, and the only byte-removing pass after combine is jump2 cross-jumping, so a byte-neutral 3rd/4th site must be a duplicate whose merged code coincides with insns the target already emits.
- [s3] dupXZ (both velocity statements duplicated) reaches the target's exact vars=24 with three spill_new slots but costs 199 fdiff lines - the frame is reachable, the byte-neutral spelling is not yet found.
- [s3] `judgearr` (TU-wide `extern s16 Judge[];` with `Judge[i]` indexing in code6cac_b.c) is body-neutral AND TU-neutral (TUdiff=0) - a free composable declaration change for any future lever.
- [s3] Instruments: tmp/grind/func_80030580/s3/frame3.py + extra3.py (variant sweep with FILE-level declaration levers and a TUdiff column) and census3.py + run_census3.sh (whole-corpus census for functions with >=3 orphan slots).

- [s3] Chassis re-measured this session: sandbox func_80030580 --disable all = 2 with candidate.c (= s1/draft3.c) applied to src/code6cac_b.c; body byte-identical, residual is addiu sp,-0x18 (vars=24) vs ours -8 (vars=8).

- [s3] GENERATIVE LAW: orphan spill slots = max(0, indexed-Judge lookup sites - 1). Measured 0/1/2/3/4 sites -> vars 0/0/8/16/24 with 0/0/1/2/3 ctx=spill_new slots (variants j0read, j1only, base, j3read, j4read). The target's vars=24 therefore corresponds to FOUR source-level lookup sites.

- [s3] The generator is the ARRAY-INDEXED global fold (mem (plus (reg idx) (symbol_ref))). Constant-address global reads produce no orphan: otherglob (+D_8008EBA0) and otherglob2 (+D_8008EBA0 +D_80101E02) both stay at vars=8.

- [s3] New 3-orphan witness with our target's exact frame signature: text1a_post:func_80041E10 (src/text1a_post.c:465) - vars=24, regs=0, args=0, sp_acc=0, three ctx=spill_new - and its body contains no branch and no label at all. This refutes s2's CODE_LABEL precondition. text1a_c:func_80042874 (src/text1a_c.c:193) is the direct N-orphan witness: six ctx=spill_new from roughly six indexed Judge lookups of ONE global.

- [s3] Other >=3-orphan functions found by the corpus census: code6cac:func_8001B478 (3), display:SetDrawEnv and SetDrawEnv2 (3 each), text1a_c:func_80042A88 (6), text1a_c2:func_800460E4 (3), text1a_post:func_80041AC8 (3), text1b:func_80060E38 (9).

- [s3] The target's bytes contain exactly two Judge accesses (lui %hi(Judge) at asm/funcs/func_80030580.s:59 and :76, lh %lo(Judge)($at) at :61 and :78) inside a straight-line block, so two of the four source-level sites must be removed after combine - and the only byte-removing pass after combine in GCC 2.7.2's order (jump, cse, loop, cse2, flow, combine, sched, lreg, greg, reload, jump2, sched2, reorg) is jump2 cross-jumping.

- [s3] A CSE-redundant duplicate is deleted pseudo and all: dupread (sn = Judge[A]; sn = Judge[A];) is byte-neutral (fdiff 0) and stays at vars=8. An extra site must survive to combine to buy a slot.

- [s3] dupXZ (both velocity statements duplicated, four sites) measures the target's exact vars=24 with three ctx=spill_new slots at a cost of 199 fdiff body lines - proof the frame is reachable on this chassis, banked as rejected/dup-judge-reads-frame-24-body-199.c.

- [s3] 17 further body-neutral respellings banked as composable, including judgearr (TU-wide extern s16 Judge[]; with Judge[i] indexing - TUdiff = 0, so the other matched functions in code6cac_b.c are unaffected), block-scope extern redeclarations of Judge / D_8008E194 / D_80106A78, *(s16 *)((s32)&Judge + (idx * 2)), % 0x1000 and & ~0xF000 / & ~(-0x1000) mask spellings, a (u16) cast on the first index, and a redundant second &D_8008E194 + arg1*7 local.

- [s3] s2's frontier probe (a BB2_COMBINE_DEBUG print inside distribute_notes) is NOT available to a grind session: it requires editing tools/gcc-2.7.2/combine.c, which the session contract forbids. The sites-1 law was obtained without it and supersedes the question that probe was asked to answer.

- [s3] src/code6cac_b.c was restored to HEAD at the end of the session; the working tree carries only memory/grind/func_80030580/ ledger changes.

## Session 4 (2026-09-02, permuter) — measured facts

- **Chassis re-measured.** With `tmp/grind/func_80030580/s1/draft3.c` (== the body in
  `memory/grind/func_80030580/candidate.c`) applied to `src/code6cac_b.c`,
  `engine sandbox func_80030580 --disable all` prints `"score": 2`. Unchanged from s1–s3.
  `tools/fake_ablate.py` reports **no FAKE-annotated constructs** in the candidate, so
  every s1–s4 instance kill was measured with zero FAKE carriers present.

- **Why only Judge orphans (new, refines the s3 law).** The target materialises the other
  two globals into hard registers and folds only Judge into the mem:
  `asm/funcs/func_80030580.s:4-5` `lui $a3,%hi(D_80106A78); addiu $a3,$a3,%lo(...)`,
  `:46-47` `lui $a0,%hi(D_8008E194); addiu $a0,$a0,%lo(...)`, versus `:59-61` and `:76-78`
  `lui $at,%hi(Judge); addu $at,$at,$v0; lh $v1,%lo(Judge)($at)`. Our candidate emits the
  identical pattern. A materialised symbol pseudo stays live and never dies into an
  orphaned REG_DEAD note, so **only a folded-into-mem symbol_ref with a register index
  generates a `ctx=spill_new` slot** — `tblall` (four indexed `(&D_8008E194)[...]` sites,
  `tbl` local removed) stays at vars=8 / one slot at bodydiff=64.

- **jump2 cross-jump re-merge is real and ~free.** `armjoin1` — `*(s16 *)obj = 0;`
  duplicated into all four arms of the kind chain and deleted from the join — compiles to
  **139 insns vs the base's 140** (`bodydiff=1`, the diff line is a *removed* insn). Costs
  for the other placements: `armjoin1_keep` 3, `armjoin_keep` 13, `armjoin` (all three join
  statements) 17, `armp2z` 157. All stay vars=8. So F3.1's mechanism works — but the
  target's only cross-jump merge point is the four-arm join at `.L800307B0`, whose three
  statements (`obj+0x50 = 1`, `obj+5 = 0`, `obj+0 = 0`) contain no Judge read, and the two
  statements that do (`obj+0x44`, `obj+0x4C`) must run before the `+=` passes.

- **Thirteen more body-neutral spellings banked** (all bodydiff=0, all vars=8, free to
  compose with any future frame lever):
  `judge2d` (`extern s16 Judge[][0x1000];` + `Judge[0][i]`, TU-safe),
  `judgestr` (`extern struct { s16 t[0x1000]; } Judge;` + `Judge.t[i]`, TU-safe),
  `jcastb` (`((s16 *)(u8 *)&Judge)[i]`), `jbyteoff` / `jbyteoff1`
  (`*(s16 *)((u8 *)&Judge + i*2)`), `jptr2` (two `s16 *` locals aliasing `&Judge`),
  `jidx2` (named `s32 ax, az` index locals), `i16` (`(u16)` cast on the first index),
  `uidx` (`(u32)` cast on the second), `objalias` (a second `u8 *` local aliasing `obj`),
  `srcs16` (`src` retyped `s16 *`, all offsets halved), `regall` / `regi` (`register`
  storage class on the locals). Sources: `tmp/grind/func_80030580/s4/var_<name>.c`.

- **Permuter campaigns (the session's mandated modality) — 84,064 iterations, 0 finds.**
  Workspaces built by `tmp/grind/func_80030580/s4/mk_ws.sh` (single-function preprocessed
  TU + `prelude.inc` + `asm/funcs/func_80030580.s` at offset 0; `compile.sh` copied from
  `tmp/perm_stf` with the cheat stages omitted, so the search space is the honest pure-C
  one). Both launched through `tools/permuter_campaign.py` with `--stack-diffs` (default —
  required here, since without it the scorer normalises the `addiu sp` offsets away and
  the entire remaining gap becomes invisible) and `--stop-on-zero`.
  - `tmp/perm_30580_s4a`, label `draft3-chassis`: base score 10, **38,802 iterations /
    973 s**, one output — `output-10-1` at 2.0 s, a score-**10 tie** with the base.
  - `tmp/perm_30580_s4b`, label `jidx2-named-index-chassis` (seeded from the body-neutral
    named-index-local variant, a structurally different AST): base score 10, **45,262
    iterations / ~1090 s**, one output — `output-10-1` at 22.8 s, again a score-10 tie.
  - Both harvested with `--stop`; `permuter_campaign.py status` = **0 live campaigns**.
  - Reusable fact: the permuter's randomizer set (`tools/decomp-permuter/src/randomizer.py`)
    contains exactly one frame-moving randomizer, `perm_pad_var_decl`, and it moves the
    frame by adding an emitted store — immediately penalised by the byte scorer. The
    permuter therefore cannot reach this function's residual from a body-exact chassis.

- [s4] Chassis re-measured this session: with s1/draft3.c (== candidate.c's body) applied to src/code6cac_b.c, engine sandbox func_80030580 --disable all prints "score": 2 - the whole residual is still the prologue/epilogue pair (addiu sp,-8 vs the target's -24).

- [s4] tools/fake_ablate.py finds NO FAKE-annotated constructs in memory/grind/func_80030580/candidate.c, so every s1-s4 instance kill was measured with zero FAKE carriers present.

- [s4] The orphan generator is narrower than s3 recorded: it is a symbol_ref that combine folds INTO the mem with a register index. asm/funcs/func_80030580.s materialises D_80106A78 (lines 4-5) and D_8008E194 (lines 46-47) into hard registers with lui %hi / addiu %lo, and folds only Judge (lines 59/61 and 76/78, lui $at,%hi(Judge); addu $at,$at,$v0; lh %lo(Judge)($at)). Four indexed sites on D_8008E194 (tblall) leave vars=8.

- [s4] jump2 cross-jumping re-merges a statement duplicated into all FOUR arms of the kind chain at negative cost: armjoin1 (*(s16 *)obj = 0; into every arm, deleted from the join) compiles to 139 insns vs the base's 140. armjoin1_keep 3, armjoin_keep 13, armjoin 17, armp2z 157 - all vars=8.

- [s4] The target's only cross-jump merge point is the four-arm join at .L800307B0, whose statements are obj+0x50 = 1, obj+5 = 0 and obj+0 = 0 - none reads Judge - so the confirmed merge mechanism has no Judge-carrying payload available on this chassis.

- [s4] Thirteen further body-neutral spellings banked (bodydiff 0, vars 8, free to compose): judge2d, judgestr (both also TU-neutral), jcastb, jbyteoff, jbyteoff1, jptr2, jidx2, i16, uidx, objalias, srcs16, regall, regi - sources at tmp/grind/func_80030580/s4/var_<name>.c.

- [s4] Permuter campaign s4a (draft3 chassis): base score 10, 38,802 iterations / 973 s, one output - a score-10 tie at 2.0 s. Campaign s4b (jidx2 named-index chassis): base score 10, 45,262 iterations / ~1090 s, one output - a score-10 tie at 22.8 s. Both harvested with --stop; permuter_campaign.py status = 0 live campaigns, 0 stale registry entries.

- [s4] --stack-diffs is load-bearing for this function: without it the permuter scorer normalises the sp offsets away and the entire remaining gap becomes invisible (the workspace would false-match at score 0). permuter_campaign.py passes it by default.

- [s4] tools/decomp-permuter/src/randomizer.py contains exactly one frame-moving randomizer, perm_pad_var_decl, and it moves the frame by adding an emitted store that the byte scorer penalises - so the permuter cannot reach this residual from a body-exact chassis. Recording this so a future session does not re-seed a third body-exact campaign.

- [s4] A reusable permuter workspace builder for this function now exists: tmp/grind/func_80030580/s4/mk_ws.sh <dir> [body.c] (single-function preprocessed TU with the Vec3i typedef and the three extern decls, prelude.inc + asm/funcs/func_80030580.s at offset 0, compile.sh with the cheat stages omitted).

## Session 5 (2026-09-02, synthesis) — floor 2 (flat); the ORPHAN-GENERATOR CATALOG

### Chassis re-measured this session
`sandbox func_80030580 --disable all` = **2** with `candidate.c` (= s1/draft3.c) applied to
src/code6cac_b.c. Unchanged: body byte-identical, the whole residual is `addiu sp,-0x18`
(target, vars=24) vs `addiu sp,-8` (ours, vars=8, one `ctx=spill_new_p110` slot).

### KILL RE-AUDIT (mandated) — the s3/s4 site-law kills all stand on the current chassis
Re-ran `tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupXZ dupX` with the
instrumented cc1 on the current tree. `tools/fake_ablate.py` reported no FAKE constructs
in candidate.c at s4 and the candidate is unchanged, so these are zero-FAKE measurements:

    base    vars= 8  bodydiff=  0   spill_new_p110
    j3read  vars=16  bodydiff= 18   p110 + p130
    j4read  vars=24  bodydiff= 25   p110 + p130 + p143     <- exact target frame
    dupX    vars=16  bodydiff= 15   p110 + p125
    dupXZ   vars=24  bodydiff=199   p110 + p125 + p145

Identical to the s3/s4 numbers. The `slots = max(0, indexed-Judge sites - 1)` law is
re-confirmed, and **j4read (+25 body lines) is the cheapest known form that reaches the
target's exact frame** — it is the form the next session should attack, not dupXZ (+199).

### NEW INSTRUMENT — a standalone orphan-generator catalog probe
`tmp/grind/func_80030580/s5/genprobe.py <file.c>` compiles an arbitrary self-contained C
file with the project's cpp/cc1 flags under `BB2_FRAME_DEBUG=1` and prints, per function,
`vars=`, the `($sp)` access count and the list of non-`round_frame` FRAMEDBG contexts.
It makes "does construct X generate a phantom slot?" a five-second question with no
src/ edit and no sandbox run. Probe corpora written this session:
`s5/gen_probe.c` (14 shapes), `s5/gen2.c` (17), `s5/gen3.c` (16).

### GENERATOR 2 (from the project memory) is INERT here — HImode bitwise
`phantom-frame-slots-gcc272` documents "two HImode (s16) locals feeding an HImode bitwise
expression `(a & ~b) & 1`" as a phantom-slot trigger (measured on func_80037540). Probed
five in-function forms (`himin` s16, `himin32` s32, `hi1` single local, `hiu` unsigned,
`qimin` QImode) with `s5/frame5.py`: **all stay vars=8** (bodydiff 7-10). The standalone
repro `gen_probe.c:p_hi` also measures vars=0. This generator does not fire in
func_80030580's context. Banked `rejected/himode-bitwise-phantom-generator-inert.c`.

### GENERATOR 3 — NEW, characterised this session, and NOT hostable by this function
`text1a_post:func_80041E10`'s three orphan slots (the s3 "no branch, no label" witness)
are now explained. Reduced to a minimal repro and bisected across 33 standalone shapes:

    p_div255_1/2/3   g_sel[k] = (s16)((((a1 >> 8*k) & 0xFF) << 12) / 255);
                     -> vars = 8 / 16 / 24, ONE ctx=spill_new PER SITE (no -1)

Both halves of the shape are load-bearing:
  * the dividend must be masked/shifted — `q1` `(s16)(x / 255)` = vars 0, `r3`
    `(s16)(((x & 0xFF) << 12) / 255)` = vars 8, `r11` `(x & 0xFFFF)` also fires;
  * the quotient must be truncated to HImode at the store — `r2` (same expression, s32
    destination) = vars 0, `r14` (via an s32 local then `(s16)`) = vars 8;
  * the division must be a magic-multiply one — `r7` (`>> 3` instead of `/`) = 0,
    `r17` (`%`) = 0, `p_div2` (`/2`, `/32`) = 0, `q9` (`/256`) = 0;
  * any further arithmetic after the truncation kills it — `r13` (`* y`) = 0, `r15`
    (`+ 1`) = 0. `r6` (`/3`) and `r8` (`* 0x1000 / 255`) do fire, so the constant is free.

**This generator counts SITES, not sites-1** — it is the only known way to buy 3 slots
from 3 source constructs. func_80030580 divides only by 32 and by 2 (both expand to shift
sequences), so it cannot host it: `t32a`/`t32b`/`m32a`/`m32b`/`m32c` (truncation and
mask/shift spellings of the `/32`) are all byte-neutral AND frame-inert, `t32c`/`t32d`
cost 15 body lines, `h2x`/`h2all` (truncating the `/2` chain) cost 29/51.
Banked `rejected/truncated-division-generator-not-hostable.c`.

### The signature axis is dead — an unused trailing parameter buys nothing
`arg3u` / `arg4u` / `arg5u` (3, 4 and 5 parameters with the extras unused), `arg3p`
(pointer) and `arg3s` (s16) are all bodydiff=0, vars=8. A dead incoming-argument pseudo
is deleted before reload and never becomes an orphan, so "the original had more
parameters" cannot explain the 16 missing frame bytes.
Banked `rejected/unused-trailing-params-frame-inert.c`.

### Pass attribution (dumps regenerated this session, `tmp/grind/func_80030580/dumps/`)
* `.lreg` names exactly ONE pseudo for this function:
  `Register 110 used 2 times across 67 insns in block 6; ST_REGS or none; pointer.`
  The other three address pseudos of the two Judge lookups (107, 127, 130) do not appear
  at all — combine deleted them outright, note and pseudo.
* `.combine` contains exactly one orphan USE, `(insn 393 99 100 (use (reg:SI 110)))`,
  placed immediately after `code_label 99` (the `/32` `bgez` join). The only other USE is
  the epilogue's `(use (reg/i:SI 2 v0))`.
* Both emitted Judge loads sit in the SAME basic block (target 0x80030644-0x800306D0, no
  label between 0x80030618 and 0x80030790), so the "one orphan per fold" reading cannot
  be rescued by block geometry: two folds in one block yield one orphan.

### Body-neutral composables added this session (bodydiff 0, vars 8, free to compose)
`hmask1` `(u16)(ang & 0xFFF)` on the first index, `hnot1` `(u16)(ang & (u16)~0xF000)`,
`hloc1` (a `u16 ang` local feeding the first index only), `hcopy` (`& (u16)0xFFFF` on the
0x56 copy), `hcopy2` (`c | *(u16 *)(obj + 0x54)` — the just-stored zero is forwarded),
`hkind` (`s16 k = tbl[0]` driving the chain), `t32a`/`t32b` ((s16)/(s32)(s16) truncation
of the `/32`), `m32a`/`m32b` (`<< 0` on the `/32` dividend), `m32c` (`*(s32 *)(src+0x18)
>> 16` for the same halfword), `svx` (`(s16)tbl[3]`), `mulpar`, `muls16`, `mulboth16`
((s16) casts on the multiply operands), `arg3u`/`arg4u`/`arg5u`/`arg3p`/`arg3s`.
Sources: `tmp/grind/func_80030580/s5/var_<name>.c`.

- [s5] Chassis re-measured: sandbox func_80030580 --disable all = 2 with candidate.c applied; residual still addiu sp,-0x18 (vars=24) vs -8 (vars=8, one ctx=spill_new_p110 slot).
- [s5] Kill re-audit on the current chassis reproduces s3/s4 exactly: j3read vars=16/bodydiff 18, j4read vars=24/bodydiff 25, dupX vars=16/bodydiff 15, dupXZ vars=24/bodydiff 199. j4read is the CHEAPEST known form that reaches the target's exact frame and is the correct attack surface, not dupXZ.
- [s5] The phantom-frame HImode-bitwise generator ((a & ~b) & 1 on two s16 locals, project memory phantom-frame-slots-gcc272) does NOT fire in func_80030580: five in-function forms all vars=8, and a standalone repro measures vars=0.
- [s5] A THIRD generator is characterised: (s16)(<masked-or-shifted dividend> / <constant>) yields one ctx=spill_new slot PER SITE with no -1 (p_div255_1/2/3 -> vars 8/16/24). It needs the mask/shift on the dividend, the HImode truncation at the store, a magic-multiply division, and no arithmetic after the cast. It explains text1a_post:func_80041E10's three orphans.
- [s5] Generator 3 is not hostable by func_80030580: its only divisions are /32 and /2, which expand to shift sequences and never fire it (t32a/t32b/m32a/m32b/m32c byte-neutral and frame-inert; t32c/t32d cost 15; h2x/h2all cost 29/51).
- [s5] Unused trailing parameters are body-neutral AND frame-inert (arg3u/arg4u/arg5u/arg3p/arg3s all bodydiff=0, vars=8) - the "original signature was wider" hypothesis buys no frame bytes.
- [s5] Dumps: .lreg names only Register 110 (ST_REGS or none; pointer); pseudos 107/127/130 are deleted by combine outright. .combine holds exactly one orphan USE, insn 393 after code_label 99. Both emitted Judge loads are in ONE basic block, so two folds in one block yield one orphan.
- [s5] New reusable instrument: tmp/grind/func_80030580/s5/genprobe.py + gen_probe.c / gen2.c / gen3.c - a standalone catalog probe that answers "does construct X generate a phantom slot?" in one cc1 run with no src edit and no sandbox run.
- [s5] src/code6cac_b.c restored to HEAD at the end of the session; the working tree carries only the ledger changes.

- [s5] Chassis re-measured this session: sandbox func_80030580 --disable all = 2 with candidate.c (= s1/draft3.c) applied to src/code6cac_b.c; body byte-identical, the whole residual is addiu sp,-0x18 (vars=24) vs ours -8 (vars=8, one ctx=spill_new_p110 slot).

- [s5] Kill re-audit on the current chassis reproduces s3/s4 exactly (base 8/0, j3read 16/18, j4read 24/25, dupX 16/15, dupXZ 24/199), so the slots = max(0, indexed-Judge sites - 1) law stands with zero FAKE carriers present.

- [s5] j4read is the cheapest known form that reaches the target's exact vars=24, at +25 fdiff lines against dupXZ's +199; fdiff itemises its surplus as exactly the two extra Judge lookups (lh angle / addu const / andi 0xFFF / sll 1 / lh Judge / addu, twice), so a byte-neutral 4-site form must make those ~11 net insns vanish after combine.

- [s5] The HImode-bitwise phantom generator documented in the project memory phantom-frame-slots-gcc272 does NOT fire in func_80030580: himin/himin32/hi1/hiu/qimin all stay vars=8 (bodydiff 7-10) and a standalone repro measures vars=0.

- [s5] A THIRD generator is now characterised and explains the s3 witness text1a_post:func_80041E10: (s16)(<masked-or-shifted dividend> / <constant>) yields one ctx=spill_new slot PER SITE with no -1 (p_div255_1/2/3 -> vars 8/16/24). It needs the mask/shift on the dividend, the HImode truncation at the store, and a magic-multiply division, and any arithmetic after the cast kills it.

- [s5] Generator 3 is not hostable by func_80030580: its only divisions are /32 and /2, which expand to shift sequences and never enter the magic-multiply path (t32a/t32b/m32a/m32b/m32c byte-neutral and frame-inert, t32c/t32d cost 15, h2x/h2all cost 29/51).

- [s5] Unused trailing parameters are body-neutral AND frame-inert (arg3u/arg4u/arg5u/arg3p/arg3s all bodydiff=0, vars=8) - a dead incoming-argument pseudo is deleted before reload and never orphans.

- [s5] Pass attribution from freshly regenerated dumps: .lreg names only Register 110 (ST_REGS or none; pointer) while pseudos 107/127/130 are deleted outright by combine, and .combine holds exactly one orphan USE (insn 393) after code_label 99; both emitted Judge loads live in one basic block in the target as well.

- [s5] 20 further body-neutral composables banked (bodydiff 0, vars 8): hmask1, hnot1, hloc1, hcopy, hcopy2, hkind, t32a, t32b, m32a, m32b, m32c, svx, mulpar, muls16, mulboth16, arg3u, arg4u, arg5u, arg3p, arg3s - sources at tmp/grind/func_80030580/s5/var_<name>.c.

- [s5] New reusable instrument: tmp/grind/func_80030580/s5/genprobe.py plus gen_probe.c / gen2.c / gen3.c - a standalone orphan-generator catalog probe that answers 'does construct X generate a phantom slot?' in one cc1 run with no src edit and no sandbox run.

- [s5] src/code6cac_b.c was restored to HEAD at the end of the session; the working tree carries only memory/grind/func_80030580/ ledger changes.

## Session 6 (2026-09-02, synthesis) — floor 2 → **0 (bytes proven, construct UNRULED)**

### Chassis re-measured this session
`sandbox func_80030580 --disable all` = **2** with candidate.c's previous body
(= s1/draft3.c, now preserved as `memory/grind/func_80030580/pure-c-floor2-body.c`)
applied to src/code6cac_b.c. `tools/fake_ablate.py` again reports no FAKE-annotated
constructs, so every s1–s6 instance kill is a zero-FAKE measurement.

### KILL RE-AUDIT (mandated) — the site-law kills all reproduce exactly
`INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s3/frame3.py base j3read j4read
dupX armsvx armsvx_keep arms2` on the current chassis:

    base          vars= 8  bodydiff=  0   p110
    j3read        vars=16  bodydiff= 18   p110 p130
    j4read        vars=24  bodydiff= 25   p110 p130 p143   <- exact target frame
    dupX          vars=16  bodydiff= 15   p110 p125
    arms2         vars=16  bodydiff= 17   p110 p130
    armsvx        vars= 8  bodydiff= 84   p115
    armsvx_keep   vars=16  bodydiff= 73   p110 p130

Identical to s3/s4/s5. Note that `armsvx` (one site MOVED into all four arms = four
sites) yields ONE slot and `armsvx_keep` (five sites) yields two — so the
`slots = sites - 1` law is a straight-line-block law, not a global one; duplicated
sites in sibling arms collapse.

### AXIS A KILLED — symbol-address bias is folded before the pseudos separate
New idea, measured with `tmp/grind/func_80030580/s6/frame6.py`: respell an EXISTING
Judge lookup as `(&Judge + K)[i - K]` so that combine has two intermediate address
pseudos to fold instead of one, buying an orphan with no emitted instruction. Every
spelling — `bias1`, `bias2`, `biasboth` (both sites), `biasneg` (negative bias),
`bias1s` (K=1), `biases` (a different K per site), `biasp` (a biased `s16 *` pointer
local shared by both sites) — is bodydiff=0 and **vars=8**, i.e. body-neutral and
frame-inert; the byte-offset spelling `biasbyte` collapses the existing orphan to
vars=0 at bodydiff 4. cse folds the constant into the symbol before the address chain
ever splits. Banked `rejected/symbol-address-bias-frame-inert.c`. The upside: seven
more body-neutral composables.

### FRONTIER 3 ANSWERED — no untouched non-spill_new slot found in a leaf
`tmp/grind/func_80030580/s6/gen4.c` (run through `s5/genprobe.py`) probed eleven shapes
that could plausibly allocate a stack object a leaf never touches: a union word/half pun
(`e1`), three struct-temp shapes (`e2`/`e8`/`e11`), a struct copy through a local
(`e3`), a pointer-cast struct copy (`e4`), a two-halfword struct read as a word (`e5`),
`long long` arithmetic (`e6`), a 64-bit `(x*y)>>12` (`e7`), and a struct-typed
conditional expression (`e9`). Every shape that allocated anything got a
`ctx=stack_temp` **with 3–9 `($sp)` accesses** (e2/e3/e5/e8/e11); every shape with sp=0
had vars=0 — except `e10`, which is generator 4 below and is `ctx=spill_new`. The same
file also confirms that the constructs func_80030580 already contains generate nothing
on their own: the pointer-biv slot-search loop (`f1`), `(x*y)>>12` (`f2`), the `/32`
(`f3`), the four-arm equality chain (`f4`), a `Vec3i` global-to-global copy (`f5`), and
an `(s16)` truncation of a Judge read (`f8`) are all vars=0.

### GENERATOR 4 — a register-allocated union of one SImode and two HImode members
`tmp/grind/func_80030580/s6/gen5.c` and `gen6.c`, bisected across 29 standalone shapes:

    a1  U4 u; u.h[0]=x; u.h[1]=x; g_w[0]=u.w;                    vars= 8 sp=0  1 spill_new
    a7  g_w[0] = (x & 0xFFFF) | (x << 16);                       vars= 0 sp=0  SAME emitted insns as a1
    a10 U4 u; u.h[0]=x; u.h[1]=x; return u.w;                    vars=16 sp=0  2 spill_new
    a11 U4 u; u.h[0]=x; u.h[1]=x; g_h[0]=u.h[1];                 vars=16 sp=0  2 spill_new, ONE emitted sh
    a13 two independent unions                                   vars=16 sp=0  2 spill_new
    b1  U4 u; u.h[0]=0; u.h[1]=x; g_h[0]=u.h[0]; g_u[1]=u.h[1];  vars=16 sp=0  2 spill_new
    b3  U4 u; u.w=x; g_u[0]=u.h[0]; g_u[1]=u.h[1];               vars= 0
    c1  u16 a=0, b=x; g_h[0]=a; g_u[1]=b;                        vars= 0   identical data flow, no union
    d1  struct { u16 a, b; } s; ...                              vars= 8 sp=2  ctx=stack_temp
    d2  union { s32 w; struct { u16 a, b; } h; } ...             vars=16 sp=0  2 spill_new
    a3 / a4  only ONE half written                               vars= 0
    a6  union with four u8 members                               vars= 0

Load-bearing conditions, all measured: (i) the object must be a UNION containing a
word-sized member — the same data flow in two plain `u16` locals (c1/c2/c3) or a bare
two-halfword struct (d1) produces nothing or a real sp-touching `stack_temp`; (ii) BOTH
halfword members must be written — writing one (a3/a4) gives vars=0; (iii) the members
must be HImode — a four-`u8` union (a6) gives vars=0; (iv) reading a half back rather
than the word doubles the yield (a11/a10 = 2 slots, a1 = 1). Decisively, **a1 and a7
emit the identical instruction sequence** while a1 reserves 8 phantom bytes and a7
reserves none — this generator is byte-free.

### The generator closes the function: sandbox --disable all = 0
In-function sweeps with `tmp/grind/func_80030580/s6/frame6.py` (+ `extra6.py`):

    z05   union{s32 w;u16 h[2]} z; z.h[0]=0; z.h[1]=0;
          *(u8 *)(obj+5)=z.h[0]; *(s16 *)obj=z.h[1];        vars=24 bodydiff=4 sp=0 -> SANDBOX 0
    z05n  same with a named struct { u16 a, b; } in the union   vars=24 bodydiff=4
    z78   same pair hosted on the obj+7 / obj+8 byte stores      vars=24 bodydiff=4
    z05b  union declared in a block around the two stores        vars=24 bodydiff=4
    z05i / z05u   s16 members / u32 word member                  vars=24 bodydiff=4
    u0 / u2 / u0b   dead-member spellings                        vars=24 bodydiff=4 -> SANDBOX 0
    u56h1 / u54 / u56blk / u58   dead-member, mid-function        vars=24 bodydiff=6
    u56h0 / u56w / u5456   overshoot                             vars=32
    z05s bare struct / z05a u16 array                            vars=16 bodydiff=11 sp=3
    q54 / q54n / q56 / q0a   live pair, DIFFERENT values          vars=16 bodydiff=26-31
    q5c   live pair inside kind-chain arm 1                      vars=24 bodydiff=74
    u5c   dead-member spelling inside arm 1                      vars=24 bodydiff=27

bodydiff=4 is exactly the two wanted prologue/epilogue lines (`subu $sp,$sp,8` ->
`subu $sp,$sp,24` and the matching `addu`), i.e. the body is byte-identical AND the
frame is exact. `engine sandbox func_80030580 --disable all` printed **"score": 0** with
`var_z05.c` applied to src/code6cac_b.c, and again with `var_u0.c`.

The byte-neutral placements all need the two union members to hold the SAME value
(zero), because reading two different values back out of the union forces shift/mask
composition (q54 +28 lines). That is why the closing placements are the function's zero
stores.

### Why this is a ruling-request and not a candidate-ready
The construct is an AGGREGATE-typed local whose only effect on the program is the frame.
Honest six-test result: T1 FAIL (behaviour is identical without it), T2 FAIL (a reader
asks why two zeros are routed through a union), T3 FAIL (the mechanism is a named GCC
pass — combine distributing an orphaned REG_DEAD note as `(use (reg N))`, which reload's
`alter_reg` then backs with a stack slot that emits nothing), T4 pass (found by a
standalone generator census, not by search output), T6 pass (`z` announces nothing). T5
is the open question: the frozen constant-holder grant
(`.claude/rules/named-local-fake-exception.md`) is scalars-only with frame coercion
forbidden, while each union MEMBER satisfies the named-intermediate prongs (fresh,
once-written, once-read, a real value landing in the target's bytes, byte-neutral) — the
union OBJECT is the thing doing the work, and no frozen family names an aggregate in
that role. First reach of an unsanctioned family is a cheat regardless of spelling, so
the session asks rather than submits.
`docs/reference/sotn-construct-index.md` contains ZERO occurrences of "union" — it does
not index union locals, so it is silent on this construct and provides no precedent
either way.

- [s6] Chassis re-measured: sandbox func_80030580 --disable all = 2 with the s1-s5 body (now preserved as memory/grind/func_80030580/pure-c-floor2-body.c); fake_ablate reports no FAKE constructs in it.
- [s6] Kill re-audit reproduces s3/s4/s5 exactly (base 8/0, j3read 16/18, j4read 24/25, dupX 16/15, arms2 16/17, armsvx 8/84, armsvx_keep 16/73). armsvx (four sites in four arms) yielding ONE slot shows the sites-1 law is per-straight-line-block, not global.
- [s6] AXIS A KILLED: the symbol-address bias identity (&Judge)[i] == (&Judge + K)[i - K] is body-neutral but frame-inert in seven spellings (bias1/bias2/biasboth/biasneg/bias1s/biases/biasp all vars=8); cse folds the constant into the symbol before the address pseudos separate. biasbyte collapses the existing orphan to vars=0 at bodydiff 4.
- [s6] FRONTIER 3 ANSWERED for eleven shapes: no construct probed (union word/half pun, struct temps, struct copies through a local, long long arithmetic, 64-bit (x*y)>>12, struct-typed conditional) produces an UNTOUCHED non-spill_new slot in a leaf - every ctx=stack_temp came with 3-9 ($sp) accesses. The constructs func_80030580 already contains (pointer-biv loop, (x*y)>>12, /32, four-arm chain, Vec3i copy, (s16) truncation of a Judge read) generate nothing standalone.
- [s6] GENERATOR 4 FOUND: a 4-byte UNION local with an SImode member and two HImode members, both halfword members written and at least one read back, is register-allocated (sp=0) and yields TWO ctx=spill_new orphan slots (gen6.c b1/d2, gen5.c a10/a11). It is BYTE-FREE: gen5.c a1 and a7 emit the identical instruction sequence, a1 with 8 phantom bytes and a7 with none.
- [s6] Generator 4 preconditions are all measured: the union wrapper with a word member is required (two plain u16 locals c1/c2 = vars 0; a bare two-halfword struct d1 and a u16[2] array become a real ctx=stack_temp with ($sp) traffic, in-function z05s/z05a = vars 16 with 3 sp accesses and 11 body lines); both halves must be written (a3/a4 = 0); the members must be HImode (a four-u8 union a6 = 0); a union written as a word and split gives nothing (b3 = 0).
- [s6] THE FUNCTION CLOSES AT ZERO: variant z05 - union { s32 w; u16 h[2]; } z; z.h[0]=0; z.h[1]=0; *(u8 *)(obj+5)=z.h[0]; *(s16 *)obj=z.h[1]; - measures vars=24, sp=0, three ctx=spill_new slots and bodydiff=4 (only the two wanted sp lines), and engine sandbox func_80030580 --disable all printed "score": 0. Equivalent byte-neutral placements: z05n, z78, z05b, z05i, z05u, and the dead-member spellings u0/u2/u0b.
- [s6] Byte-neutrality requires the two union members to carry the SAME value, so the closing placements are the function zero stores; a live pair holding two DIFFERENT real values (q54/q54n/q56/q0a/q5c) costs 26-74 body-diff lines because reading the members back forces shift/mask composition.
- [s6] The construct is UNRULED, not accepted: it fails the six-test checklist on T1/T2/T3 and no frozen family covers an aggregate-typed local in this role (named-local-fake-exception.md is scalars-only with frame coercion forbidden; the named-intermediate prongs describe each union MEMBER but not the union OBJECT). docs/reference/sotn-construct-index.md has zero occurrences of "union", so it does not index this construct and is silent rather than negative.
- [s6] Instruments: tmp/grind/func_80030580/s6/frame6.py + extra6.py (in-function sweep, same harness as s3/s5) and gen4.c / gen5.c / gen6.c (standalone catalog probes run through s5/genprobe.py).
- [s6] src/code6cac_b.c restored to HEAD at the end of the session; the working tree carries only memory/grind/func_80030580/ ledger changes.

## Session 6 (driver session 6, 2026-09-02, synthesis) — floor 2 (honest, stripped) / **BYTES PROVEN on main**

### Starting point
The previous ledger session's union-local candidate was ruled **FAIL** by the Judge
(docs/grind/decisions.md 2026-09-02 17:07): the union type carries no semantic content
(gen6.c c1/c2 emit the same instructions through two plain u16 locals at vars=0), so it
needs frozen-family membership and has none. The ruling names the one frozen route to
this residual: the **Phantom-frame-slot volatile pad local** family in its exact
form-constrained shape (`volatile u32 pad[N];`, first-decl, `// !FAKE`, engine allowlist
row, frame forensics per .claude/rules/phantom-slot-frame-lever.md). This session
restored `memory/grind/func_80030580/pure-c-floor2-body.c` (byte-identical to
s1/draft3.c) as the chassis and measured that route.

### The measurement — `tmp/grind/func_80030580/s6/padsweep.py` (INSTR=1, instrumented cc1)
N swept 1..6, pad inserted as the FIRST declaration of the body:

    padbase   vars= 8   bodydiff=  0  sp=0   ctx=spill_new_p110
    pad1      vars=16   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110
    pad2      vars=16   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110
    pad3      vars=24   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110
    pad4      vars=24   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110
    pad5      vars=32   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110
    pad6      vars=32   bodydiff=  4  sp=0   ctx=stack_temp | ctx=spill_new_p110

`pre_pad[4]` (16 bytes = exactly the gap between our 8-byte combine orphan and the
target's 24) is the exact-fit choice; `pad3` reaches the same rounded 24 with 12 bytes.
`tools/fdiff.py` on pad4 vs the base prints ours=140 target=140 with the entire diff
being the two wanted lines: `subu $sp,$sp,8` -> `subu $sp,$sp,24` and the matching
`addu`. Zero `($sp)` references touch the pad — the family's untouched-slot prerequisite
is satisfied by direct measurement, not by inference.

### THE FUNCTION IS BYTE-MATCHED ON MAIN
`& tools/wteng.ps1 main build` with this body in src/code6cac_b.c:

    sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
    want 62efab4f73f992798c43e8c730aa43baa10bb4fa
    MATCH

Re-run a second time after the annotation was reworded — MATCH both times. The body is
the s1-s5 pure-C body verbatim plus the one annotated pad declaration.

### Why the sandbox still prints 2 — the integration handoff
`sandbox func_80030580 --disable all` = **2**. The cheat-stripper removes the pad
declaration because `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS` has no row
for this function; the family's FORM CONSTRAINT explicitly requires that row
(no-new-park-categories.md:420-424). `engine/` is outside a grind session's writable
surface. This is exactly the shape of the 2026-08-22 grant rows (func_80049A2C,
func_800481E8, func_80041688), whose own comment records the principle: "the allowlist
affects only the sandbox score, never the real build, so the proof needs no row"
(engine/volatile_cheats.py:759-765). Operator step: add
`"func_80030580": frozenset({("pre_pad", 4)}),` to `_SANCTIONED_UNWRITTEN_PADS`, after
a fresh layer-2 cheat-reviewer on the C.

### DETECTOR FINDING (disclosed, not exploited)
`find_unused_local_arrays` decides "unused" with a plain `\bname\b` search over the
function body text — COMMENTS INCLUDED. The first version of this session's annotation
spelled the identifier `pre_pad` in the comment above the declaration; that alone made
the detector treat the pad as referenced, the stripper left it in place, and
`sandbox --disable all` printed `"score": 0`. That 0 is a detector artifact, not an
honest floor. The comment was rewritten to avoid the identifier and the sandbox
correctly returned to 2. Any function whose ledger prose or annotation names its own
unused array is currently invisible to this detector — engine-side finding for the
operator (engine/volatile_cheats.py:273-278); this session did not and must not spend it.

- [s6b] Judge-mandated route MEASURED and it closes the function: `volatile u32 pre_pad[4];` as the first declaration of the s1-s5 pure-C body gives vars=24, sp=0, bodydiff=4 (only the two subu/addu $sp lines, 140/140 insns) and a full clean build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, verified twice.
- [s6b] Pad-size gradient measured with the instrumented cc1 (s6/padsweep.py): pad1/pad2 vars=16, pad3/pad4 vars=24, pad5/pad6 vars=32; pre_pad[4] is the exact-fit 16 bytes between our 8-byte combine orphan slot and the target's 24.
- [s6b] The honest sandbox score stays 2 because the cheat-stripper deletes the pad: the Phantom-frame-slot volatile pad family requires a per-function row in engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS ("func_80030580": ("pre_pad", 4)), a surface a grind session may not touch. Same integration shape as the byte-proven 2026-08-22 rows func_80049A2C / func_800481E8 / func_80041688.
- [s6b] The union candidate is dead by Judge ruling (2026-09-02 17:07 FAIL, aggregate carrier with no semantic content and no frozen family); it stays banked at rejected/union-dead-member-store-frame-24-unruled.c and must not be respelled.
- [s6b] DETECTOR FINDING, disclosed and not exploited: engine.volatile_cheats.find_unused_local_arrays checks references with a plain \bname\b search over the body INCLUDING COMMENTS, so an annotation that spells the pad's own identifier makes the stripper skip it and the sandbox print a false 0. Observed once this session, immediately reverted by rewording the comment.
- [s6b] src/code6cac_b.c restored to HEAD at the end of the session; the proven body lives in memory/grind/func_80030580/candidate.c.

## Session 7 (2026-09-02, synthesis) — floor 2; s6's byte-match INDEPENDENTLY RE-VERIFIED

s6 was discarded by the driver on a filing technicality (its handoff entry heading
lacked the literal `OWNER-ESCALATION` token, so `grind.ps1:1171` could not match it).
Its *measurements* were never disputed, and none of them were taken on trust here —
every one below was re-run from scratch this session.

### Re-verification (all this session, current chassis)
| what | command | result |
|---|---|---|
| HEAD chassis | `sandbox func_80030580 --disable all` | **148** (HEAD is `INCLUDE_ASM`; explains the dispatch "measurement unavailable") |
| pure-C floor body | apply `pure-c-floor2-body.c`, sandbox | **2** — the ledger floor reproduces exactly |
| candidate (floor body + `volatile u32 pre_pad[4]; // !FAKE`) | sandbox | **2** — the stripper deletes the pad (`engine/volatile_cheats.py:249`) |
| candidate | full clean driver `build` | `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want` → **MATCH** |
| frame forensics | `INSTR=1 padsweep.py 2 3 4 5` | `padbase vars=8 bodydiff=0` · `pad2 vars=16` · `pad3 vars=24` · `pad4 vars=24 bodydiff=4 sp=0` · `pad5 vars=32`; every variant `sp=0` |

The `bodydiff=4` on pad4 is exactly the `subu $sp,$sp,8 -> subu $sp,$sp,24` line and
its matching `addu` — 140/140 body instructions identical, zero `($sp)` references
touching the pad.

### The residual is now TYPED (this is the session's new finding)
Our matching body compiles to `vars=8`, and the frame instrument attributes that 8
to a single `ctx=spill_new_p110` slot. The target is `vars=24`. Because **all 140
body instructions already match**, the missing 16 bytes cannot be a spill slot that
carries traffic — any such slot would show as body divergence, and there is none.
So the target's extra frame bytes are a **declared frame object of 13..16 bytes
that no instruction touches** (MIPS_STACK_ALIGN rounds 17..24 to 24, so 13..16 all
land on 24; measured: `pad3` = 12 bytes → 24 as well, i.e. 9..16 bytes qualify).

This RETIRES the s2/s3 model — "the target's vars=24 is three 8-byte combine-orphan
spill slots" — that shaped sessions 2 through 5 and sent them hunting for extra
indexed-`Judge` lookup sites under the sites-1 orphan law. It confirms live-frontier
item 3 (which was ranked third) and demotes frontier items 1 and 2 (both of which
assume an orphan-spill producer).

### Why the sandbox cannot print 0 here
The Judge's 2026-09-02 17:07 constraint names the Phantom-frame-slot volatile pad
local family as the only frozen route to these bytes, and that family's FORM
CONSTRAINT includes a per-function row in `_SANCTIONED_UNWRITTEN_PADS`
(`engine/volatile_cheats.py:746`). `engine/` is outside a grind session's writable
surface, so the score stays at 2 no matter how correct the C is. This is an
INTEGRATION HANDOFF, filed at `docs/grind/decisions.md` (2026-09-02,
`func_80030580 — OWNER-ESCALATION: INTEGRATION HANDOFF`), not an exhaustion claim.

### Adjacent forms checked and NOT pursued (reasoning, no measurement spent)
- Non-volatile dead scalar locals would survive the stripper (`find_volatile_unused_locals`
  only matches `volatile`; `find_orphaned_local_decls` skips zero-reference decls) and
  would make the sandbox print 0 without any engine row. **Not pursued**: the
  2026-09-02 17:07 ruling states `named-local-fake-exception.md:5,72` "excludes arrays
  and frame coercion explicitly", so a dead scalar used for frame coercion is outside
  that family, and reaching 0 by a spelling the detector happens not to catch is
  cheat-checklist T4/T5 by construction. Recorded so no later session re-derives it as
  a "discovery".

- [s6] HEAD chassis measured this session: sandbox func_80030580 --disable all = 148 (HEAD carries INCLUDE_ASM), which explains the dispatch brief's 'measurement unavailable'.

- [s6] memory/grind/func_80030580/pure-c-floor2-body.c re-measured on the current chassis: sandbox --disable all = 2, reproducing the ledger floor exactly; the residual 2 is subu $sp,$sp,8 vs the target's subu $sp,$sp,24 plus the matching addu, with all 140 body instructions identical.

- [s6] memory/grind/func_80030580/candidate.c applied to src/code6cac_b.c and built with the full clean driver: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want -> MATCH. The function is solved; only the sandbox scorer disagrees.

- [s6] Frame forensics re-run on the current chassis (instrumented cc1, BB2_FRAME_DEBUG=1): padbase vars=8 bodydiff=0, pad2 vars=16, pad3 vars=24, pad4 vars=24 bodydiff=4 sp=0, pad5 vars=32 - every variant sp=0, so no ($sp) reference touches the pad and bodydiff=4 is exactly the two stack-adjust lines.

- [s6] The blocker is engine/volatile_cheats.py:746 _SANCTIONED_UNWRITTEN_PADS, which needs the row "func_80030580": frozenset({("pre_pad", 4)}). engine/ is outside a grind session's writable surface, so no C spelling can make the sandbox print 0 for the Judge-mandated form.

- [s6] Precedent for the handoff shape: the 2026-08-22 rows for func_80049A2C, func_800481E8 and func_80041688 were byte-proven first and granted the row afterwards; engine/volatile_cheats.py:759-765 records the principle that the allowlist affects only the sandbox score, never the real build.

- [s6] Engine finding disclosed and not exploited (re-confirmed this session): find_unused_local_arrays decides 'unused' with a plain \bname\b search over the body text including comments, so an annotation that spells its own pad identifier makes the pad invisible to the stripper and yields a false score of 0. The candidate's annotation is worded to avoid the identifier and the sandbox correctly prints 2.

- [s6] s6 was discarded on a filing technicality only - its handoff heading lacked the literal OWNER-ESCALATION token that tools/grinder/grind.ps1:1171 matches on; its measurements were never disputed and have all been independently re-run here.

- [s6] src/code6cac_b.c was restored to HEAD at session end; the proven body lives at memory/grind/func_80030580/candidate.c and the self-vet at memory/grind/func_80030580/self_vet.md.

## Session 7 (2026-09-02, solver) — floor 2 (flat); H-s7-2 CORRECTED, three new kills

### Chassis (re-measured, zero FAKE constructs)
`pure-c-floor2-body.c` applied to `src/code6cac_b.c`: `sandbox func_80030580 --disable all`
= **2** (`target_insns` 148, `build_insns` 148). `tools/fake_ablate.py --func func_80030580
--file code6cac_b --candidate memory/grind/func_80030580/pure-c-floor2-body.c` reports
"no FAKE-annotated constructs found; nothing to ablate". Every number below is therefore a
zero-FAKE measurement on the current chassis.

### The solver verdict — both solver models are OFF the residual (typed, not guessed)
`tools/ra_solver/inverse_compose.py classify code6cac_b func_80030580` refuses the text
path for this zero-rule function (it would report a fictitious PRE-RA verdict) and routes
to the object path. `tools/ra_solver/goal_from_tgt.py classify code6cac_b func_80030580`:

    func_80030580 (code6cac_b): ours 148 insns, target 148 insns  [object-level: replace_with_asmfile-safe]
    FIRST DIVERGENCE: PRE-RA
      next tool: none — the residual is upstream of every model
      instruction shapes present in ONE stream only:
        ours only  : addiu #,#,-8   x1 / addiu #,#,8   x1
        target only: addiu #,#,-24  x1 / addiu #,#,24  x1

Neither `ra_solver` (global.c / local-alloc / reload seat models) nor `sched_solver` (both
scheduler passes) has anything to say here: the register disposition and the emission order
are already identical, and the only divergence is the frame SIZE, fixed by
`assign_stack_local` calls that happen before any seat or clock the models represent. The
solver modality is mechanically closed for this function.

### H-s7-2 IS CORRECTED — the residual IS spill slots, and the 8-byte granularity is proved
The previous session inferred "a declared untouched frame object of 9..16 bytes, not
additional combine-orphan spill slots" from the fact that every body instruction matches.
That inference is wrong: an UNALLOCATED pseudo's `alter_reg` slot costs zero instructions
by construction — invisible body is exactly what an orphan slot looks like. Direct FRAMEDBG
evidence on the base chassis (`tmp/grind/func_80030580/s7/fdbg.py`):

    FRAMEDBG func=func_80030580 ctx=spill_new_p110 mode=4 size=8 align=-1 alignment=8 frame_offset=8
    FRAMEDBG func=func_80030580 ctx=round_frame  mode=26 size=0 align=0 alignment=8 frame_offset=8

`ctx=spill_new_p*` is the instrumentation at `tools/gcc-2.7.2/reload1.c:2403`, inside
`alter_reg`'s `from_reg == -1` arm — a pseudo with refs and no hard register getting a
brand-new stack slot. `mode=4` is SImode; the slot is 8 bytes because `assign_stack_local`
is called with `align = -1`, which rounds to `alignment = 8`. The re-audit run confirms the
same 8-byte granularity for every additional slot: j3read `p110 + p130` at frame_offset
8/16, j4read `p110 + p130 + p143` at 8/16/24. **The target's vars=24 is therefore THREE
8-byte `alter_reg` slots and our vars=8 is one — the s2/s3 orphan-spill model is
REINSTATED, and the "declared 13..16-byte untouched object" framing of the s6 frontier (and
the Judge constraint written on top of it) rests on a retracted premise.**

### KILL RE-AUDIT (mandated) — the site-law numbers reproduce exactly, zero FAKE present
`INSTR=1 PYTHONPATH=. python3 tmp/grind/func_80030580/s3/frame3.py base j3read j4read dupX`:

    base    vars= 8  bodydiff=  0  sp=0  p110
    j3read  vars=16  bodydiff= 18  sp=0  p110 + p130
    j4read  vars=24  bodydiff= 25  sp=0  p110 + p130 + p143     <- exact target frame
    dupX    vars=16  bodydiff= 15  sp=0  p110 + p125

Identical to s3/s4/s5. `slots = max(0, indexed-Judge sites - 1)` stands on this chassis.

### THE STRUCTURAL CONSEQUENCE — the target cannot reach 3 slots from Judge alone
Our body is byte-identical to the target's 148 instructions, and it contains exactly TWO
indexed `Judge` loads. Under the re-confirmed sites-1 law, two sites yield ONE orphan. The
target has THREE slots. Its other two slots therefore come from a generator that emits ZERO
instructions and is NOT the Judge fold. Across the s5/s6 corpus census the only byte-free
multi-slot generator found anywhere in this tree is **generator 4** (the small
register-allocated union with a word member and two written HImode members: gen5.c `a1` and
`a7` emit the identical instruction stream, `a1` reserving 8 phantom bytes and `a7` none;
`a10`/`a11`/`b1`/`d2` reserve two slots). 1 Judge orphan + 2 generator-4 slots = 24. This is
the sharpest statement of the residual the ledger has: the original source very likely
declared a small aggregate local, and that construct was FAILed by the Judge on 2026-09-02
17:07 as frame coercion. Recorded here as the SHAPE of the answer, not as a proposal to
resubmit.

### THREE NEW KILLS (all measured this session, instrumented cc1, current chassis)
1. **Pointer alias to `Judge` is frame-NEGATIVE.** `s16 *jt = &Judge;` for both lookups:
   **vars=0** (the one existing orphan is destroyed) at bodydiff=19. Materialising the
   symbol in a live pointer register stops combine folding `(symbol_ref)` into the loads,
   so no address pseudo survives to orphan. Banked
   `rejected/judge-pointer-alias-destroys-orphan.c`. Same direction as s2's `jmix`.
2. **There is no SCALAR spelling of generator 4.** `s32 t; *(u16 *)&t = 0;
   *((u16 *)&t + 1) = 0;` read back into the function's own obj+5 / obj+0 zero stores:
   vars=16 **bodydiff=11 sp=3**, contexts `put_reg_into_stack` + `spill_new_p111`. Taking
   `&t` forces the pseudo into a real frame object with three `($sp)` accesses; generator
   4's byte-freeness depends on the union staying register-allocated. The obj+7 / obj+8
   placement (`var_punpair7.c`) is the same: vars=16 bodydiff=12 sp=3. The frozen
   sub-word-access family therefore cannot buy generator 4's slots without an aggregate.
   Banked `rejected/subword-pun-scalar-forces-frame-address.c`.
3. **A CODE_LABEL between the two Judge folds does not strand the second note.** Moving the
   `/32` term's bgez join so it sits between the two lookups (`var_divmid.c`): **vars=8**
   (one slot, now p102) at bodydiff=30. Re-confirms the s5/s6 block-geometry kill on a new
   spelling and on the current chassis. Banked
   `rejected/div32-between-judge-sites-no-second-orphan.c`.

### Frame-inert / byte-neutral shapes measured this session (negative, but composable)
    posptr    Vec3i *pos/*vel into obj+0x2C / obj+0x44   vars= 8  bodydiff=  0  sp=0
    srcptr    Vec3i *sp into src+0xF4                    vars= 8  bodydiff= 23  sp=0
    whilerot  rotated while-loop guard                   vars= 8  bodydiff=  0  sp=0
    kindlocal s32 kind = tbl[0] driving the arm chain     vars= 8  bodydiff=  0  sp=0
    anglocal  s32 ang = *(u16 *)(src+0x1CA), first index  vars= 8  bodydiff=  0  sp=0
    neguard   `i != 12` loop guard                        vars= 8  bodydiff= 20  sp=0
    limitvar  `s32 n = 12` loop bound                     vars= 8  bodydiff= 11  sp=0
    velnamed  vx/vy/vz named velocity intermediates       vars= 8  bodydiff=100  sp=0
    vecnamed  named `Vec3i v` for the 0x2C->0x38 copy     vars=24  bodydiff= 10  sp=6  (stack_temp, real traffic)
    vechalf   named `Vec3i h` for the three /2 halves     vars=24  bodydiff=183  sp=5
    srcvec    named `Vec3i p` for the three src reads     vars=24  bodydiff=173  sp=5
    tblstruct `s16 e[4]` cache of tbl[0]/tbl[4]           vars=16  bodydiff=154  sp=3

`posptr` (byte-neutral, a natural typed-pointer spelling of the position/velocity triples)
is saved as `memory/grind/func_80030580/composable-posptr-byte-neutral.c`. Producer 1 of
`.claude/rules/phantom-slot-frame-lever.md` (folded loop-guard compare) is now measured dead
here in three spellings (whilerot / neguard / limitvar) and producer 3 (live named locals on
multi-read fields) in five (kindlocal / anglocal / velnamed / vecnamed / srcvec): every
named-local shape that reaches vars=24 does so through a `ctx=stack_temp` with real `($sp)`
traffic, never through a byte-free slot.

### Instruments added
- `tmp/grind/func_80030580/s7/fdbg.py` — prints the raw FRAMEDBG lines (mode/size/align/
  frame_offset) for one variant body; this is what proved the 8-byte `align=-1` rounding.
- `tmp/grind/func_80030580/s7/runfiles.py` — measures vars / bodydiff / `($sp)` count for an
  arbitrary list of variant `.c` bodies (file-driven; no in-script string surgery).
- `tmp/grind/func_80030580/s7/sweep.py` — the 11-shape named-local / loop-guard sweep.

### Housekeeping
`candidate.c` is RESET to the FAKE-free `pure-c-floor2-body.c`: the s6 candidate's
`volatile u32 pre_pad[4]` route is closed by the standing Judge constraint, so the ledger's
best *submittable* form is the honest floor-2 body. `src/code6cac_b.c` restored to HEAD.

- [s7] Chassis re-measured this session: pure-c-floor2-body.c applied to src/code6cac_b.c gives sandbox func_80030580 --disable all score 2 with target_insns 148 and build_insns 148; tools/fake_ablate.py reports no FAKE-annotated constructs in it, so every measurement below is a zero-FAKE measurement.

- [s7] SOLVER VERDICT: inverse_compose.py classify refuses the text path for this zero-rule function (it would report a fictitious PRE-RA verdict) and routes to the object path; goal_from_tgt.py classify returns FIRST DIVERGENCE: PRE-RA with 'next tool: none - the residual is upstream of every model', the only shape difference being ours addiu $sp,-8/+8 vs target addiu $sp,-24/+24.

- [s7] FRAMEDBG on the base chassis prints ctx=spill_new_p110 mode=4 size=8 align=-1 alignment=8 frame_offset=8 - the slot is SImode, is 8 bytes only because alter_reg passes align=-1 to assign_stack_local which rounds to alignment=8, and is emitted from the from_reg == -1 arm at tools/gcc-2.7.2/reload1.c:2403.

- [s7] H-s7-2 IS CORRECTED: the residual is NOT 'a declared untouched frame object of 9..16 bytes'. An unallocated pseudo's alter_reg slot costs zero instructions by construction, so a byte-identical body is exactly what an orphan spill slot looks like. j3read and j4read confirm the 8-byte granularity directly (p110@8 + p130@16 + p143@24 = vars 24). The s2/s3 orphan-spill model is reinstated, and the s7 frontier item 1 plus the Judge constraint phrased on top of it rest on a retracted premise.

- [s7] Mandated kill re-audit reproduces s3/s4/s5 exactly on the current chassis with zero FAKE carriers: base vars=8 bodydiff=0, j3read vars=16 bodydiff=18, j4read vars=24 bodydiff=25 (exact target frame), dupX vars=16 bodydiff=15. The law slots = max(0, indexed-Judge sites - 1) stands.

- [s7] STRUCTURAL CONSEQUENCE: our body is byte-identical to the target's 148 instructions and contains exactly TWO indexed Judge loads, so under the site law the target's Judge folds yield ONE of its three slots. Its other two slots must come from a generator emitting zero instructions. The only byte-free multi-slot generator in the whole s5/s6 census is generator 4 (register-allocated union, word member plus two written HImode members; gen5.c a1 and a7 emit identical instruction streams with a1 reserving 8 phantom bytes and a7 none). 1 + 2 = 3 = vars 24. That construct was Judge-FAILed on 2026-09-02 17:07, so this is banked as the shape of the answer, not a proposal.

- [s7] NEW KILL 1 - pointer alias to Judge is frame-NEGATIVE: s16 *jt = &Judge; used for both lookups gives vars=0 (the one existing orphan is destroyed) at bodydiff=19; materialising the symbol in a live register stops the (symbol_ref) fold so no address pseudo survives to orphan.

- [s7] NEW KILL 2 - there is no scalar spelling of generator 4 on this chassis: s32 t with *(u16 *)&t writes read back into the function's own zero stores measures vars=16 bodydiff=11 sp=3 with ctx=put_reg_into_stack; taking &t forces a real addressable frame object. The obj+7/obj+8 placement is the same (vars=16 bodydiff=12 sp=3).

- [s7] NEW KILL 3 - a CODE_LABEL between the two Judge folds does not strand the second REG_DEAD note: moving the /32 bgez join between the lookups leaves vars=8 (one slot, p102) at bodydiff=30, re-confirming the s5/s6 block-geometry kill on a new spelling.

- [s7] phantom-slot-frame-lever producers 1 and 3 are measured dead here in eight spellings; four of them (whilerot, kindlocal, anglocal, posptr) are byte-neutral (bodydiff=0, vars=8) and are banked as free composables for a future session to stack on top of a real generator.

- [s7] candidate.c is RESET to the FAKE-free pure-c-floor2-body.c: the s6 candidate's volatile u32 pre_pad[4] route is closed by the standing Judge constraint, so the ledger's best submittable form is the honest floor-2 body. src/code6cac_b.c was restored to HEAD; the working tree carries only memory/grind/func_80030580/ ledger changes.


## [s8 -- forensics, 2026-09-02] The residual 16 frame bytes are SOLVED mechanically: a bytes-proven vars=24 form exists at sandbox distance 0

**Chassis re-measured this session.** HEAD (`INCLUDE_ASM`) = 148. `pure-c-floor2-body.c`
applied = **2** (this is the number to quote; the ledger's "floor=2" is confirmed on the
current chassis, zero FAKE constructs present -- `candidate.c` is byte-identical to
`pure-c-floor2-body.c`).

**The lever (new, previously unknown to this ledger).** Adding a self-cancelling term
`+ K - K` to an existing store, where `K` is a MEMORY read that is *not already loaded at
that program point*, buys exactly **+8 frame bytes and zero instructions**. Two such terms
at the right pair of sites reach the target frame exactly:

    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4) + *(u16 *)(src + 4) - *(u16 *)(src + 4);
    ...
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4) + *(u16 *)(src + 4) - *(u16 *)(src + 4);

measures `vars=24 bodydiff=4 sp=0` (the 4 diff lines are ONLY the two `subu $sp,$sp,N` /
`addu $sp,$sp,N` prologue/epilogue lines -- all 140 body instructions are byte-identical),
and **`sandbox func_80030580 --disable all` printed 0 with those two edits in src/ this
session**. Saved as `memory/grind/func_80030580/ruling-form-cse-cancelled-load-orphan.c`.
It is NOT `candidate.c` and was NOT left in src/: on this agent's own 6-test reading the
construct fails T1 (no observable effect) and T2 (no human writes `x + K - K`), and no
frozen SOTN family covers "an expression whose value cse proves redundant", so the session
returns a `ruling-request` rather than a submission.

**Pass attribution (dumps banked, not guessed).** `pwsh tools/grinder/dump.ps1
func_80030580` on the vars=24 form; sections extracted with
`tmp/grind/func_80030580/s8/dsec.py`:

  - `code6cac_b.cse2`: the func_80030580 section contains exactly ONE `(use (reg ...))`
    insn -- `(insn 389 ... (use (reg/i:SI 2 v0)))`, the ordinary return-value use.
  - `code6cac_b.combine`: FIVE -- insn 413 `(use (reg:SI 89))`, insn 414 `(use (reg:SI 116))`,
    insn 416 `(use (reg:HI 188))`, insn 415 `(use (reg/v:SI 75))`, plus the v0 use.
  - `code6cac_b.lreg`: the same five survive local-alloc unallocated.
  - `code6cac_b.greg`: only two remain as register references -- reg 75 got a hard register
    (`$t1`); regs 89, 116 and 188 got NO hard register.

So the producing pass is **`combine.c` / `distribute_notes`**: when combine deletes the insn
that set a pseudo and the pseudo's REG_DEAD note has no insn to be rehomed on, it emits a
bare `(use (reg N))`. That leaves a pseudo with refs but no live range; `global.c` gives it
no hard register; `reload1.c alter_reg` then calls `assign_stack_local (SImode, 8, -1)` for
each, and `align == -1` rounds each slot to 8 bytes. FRAMEDBG agrees exactly:
`ctx=spill_new_p89 | ctx=spill_new_p116 | ctx=spill_new_p188 | ctx=round_frame`, i.e.
8 (the base Judge-fold orphan) + 8 + 8 = 24. This CONFIRMS the s7 alter_reg model and
retires the s7 "byte-free generator must be an aggregate" framing: the byte-free generator
is a *deleted load*, not a declared object.

**The input-shape law (measured, 30 single-site + 10 paired + 13 operand variants).**
Writing `X + K - K` onto every assignment statement in the body, one at a time:
  - `K` register-resident or already loaded at that point (`arg1`, `i`, `tbl[3]` at the
    obj+0x48 store, `*(s32 *)(obj + 0x44)` at the obj+0x48 store) -> `vars=8`, no slot.
  - `K` a memory read not yet loaded there -> `vars=16`, `sp=0`, always exactly one extra slot
    (measured for tbl[0], tbl[2], `*(u16 *)(tbl + 4)`, `*(u16 *)(src + 4)`,
    `*(s16 *)(src + 0x1A)`, `*(s16 *)(src + 0x1CA)`, `*(s32 *)(src + 0xF4)`, `*(u8 *)(obj + 0xA)`).
  - Byte-neutral single sites (bodydiff=4, i.e. frame only): statement 10
    (`*(u8 *)(obj + 0xA) = i;`), 11 (`*(s16 *)(obj + 2) = arg1;`), 16
    (`*(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);`) and 38
    (`*(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);`, the tbl[0]==2 arm).
  - Pairing two byte-neutral sites: **10+38, 11+38 and 16+38 all give `vars=24 bodydiff=4
    sp=0`.** Pairs drawn from the early cluster (10+11, 10+16, 11+16) reach vars=24 but cost
    bodydiff=40 (register-pressure fallout), and pairs involving statement 15 stay at vars=16.
  - Scaling: `+ JX - JX` with `JX` an indexed `Judge` read costs SIX slots (`vars=48`), and a
    four-term Judge cancel costs twelve (`vars=96`) -- the Judge index expression carries many
    more pseudos, so it massively overshoots.

**Frontier-2 answered (the never-measured cse question).** "Does cse deleting a redundant
Judge LOAD also delete the address pseudo that combine would otherwise fold and orphan?"
**No -- the orphan survives, and that is exactly why the lever works.** `jeqx2`
(`tbl[3] + JX - JX`) measures `vars=48 bodydiff=6 sp=0`; the only body change is one
`lw $4,68($7)` moved a single slot earlier. The route is open, just badly over-powered:
Judge is the wrong operand, a plain field read is the right one. For the record the banked
4-site forms re-measured identically on the current chassis: `j3read vars=16 bodydiff=18`,
`j4read vars=24 bodydiff=25` -- the site law is intact.

**Nineteen ordinary-C / sanctioned-family respellings measured frame-INERT** (all `vars=8`,
chassis floor 2, no FAKEs): duplicated-statement-into-arms (obj+0x50 and obj+0x58 pushed into
all four arms), `do { } while (0)` wraps around the arm chain and around the Judge block,
mixed exit forms (`goto endK` + inline return), a same-value re-store of the obj+0x2C field,
a split-read hoist of `*(u16 *)(tbl + 4)`, variable-reuse borrowing `i` for the angle,
named-intermediate locals for the scaled product and for the angle, a pointer alias over the
source position vector, four loop-guard respellings (`&` instead of `&&`, operand swap,
`continue` form, `< 0` form), merging the identical tbl[0]==1 and tbl[0]==3 arms, a dead
scalar local fed by two Judge reads (`vars=8 bodydiff=0` -- its loads die BEFORE combine, so
no orphan is created; this is why the dead-scalar-local family cannot host the lever), and a
Judge read multiplied by zero (folded in the front end, no RTL at all). Banked as
`rejected/sanctioned-family-shapes-frame-inert-s8.c` and
`rejected/self-cancel-register-operand-frame-inert.c`.

**Kill re-audit (mandated, floor flat 3+ sessions).** The closest banked instance kill,
`vecnamed` (the named Vec3i local, s7's only vars=24 ordinary shape), re-measured on the
current chassis with zero FAKE constructs: `vars=24 bodydiff=10 sp=6` -- unchanged, still
carrying six real `($sp)` accesses. The kill stands.

**Why this narrows the function to a single question.** The frame lever is now known, cheap,
and exactly tunable -- but its defining property is that the added expression *has no
observable effect*, which is the same property the cheat checklist tests for. Every
semantically-real respelling measured this session left the frame at 8. So the remaining
search is not "find another generator" (that is answered) but "is there an ORDINARY-C
statement whose value GCC 2.7.2 proves redundant in cse2, at one of statements 10/11/16/38".


## [s9 -- forensics, 2026-09-02] Pass attribution CORRECTED to combine.c, and four more source-shape families measured frame-inert

**Chassis re-measured this session (the brief's dispatch measurement was unavailable).**
`pure-c-floor2-body.c` applied to `src/code6cac_b.c`: `sandbox func_80030580 --disable all`
= **2**, `target_insns` 148, `build_insns` 148, `rules_dropped` 0. `candidate.c` is
byte-identical to that body and `tools/fake_ablate.py` reports *"no FAKE-annotated constructs
found ... nothing to ablate"*, so every measurement below is a zero-FAKE measurement on the
same chassis as the s7/s8 kills. Mandated kill re-audit: `base` vars=8 bodydiff=0 (p110),
`pr16_38` vars=24 bodydiff=4 sp=0 (p89 | p116 | p188), `j4read` vars=24 bodydiff=25
(p110 | p130 | p143), `sa38` vars=16 bodydiff=4 (p110 | p182) -- all identical to s8.

**PASS ATTRIBUTION CORRECTED (dumps read, not guessed).** s8 recorded the frame lever as
"cse2 proves the sum equals the original value so the loads die". The `-da` dumps say
otherwise. New instrument `tmp/grind/func_80030580/s8/dumpvar.py` dumps every RTL pass for an
arbitrary variant body; sections counted with `s8/dsec.py`:

| variant | rtl | jump | cse | loop | cse2 | flow | combine | lreg | greg | section lines @cse2 |
|---|---|---|---|---|---|---|---|---|---|---|
| base   | 1 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 1 | 914 |
| sa38   | 1 | 1 | 1 | 1 | 1 | 1 | 4 | 4 | 2 | 929 |
| rb_o54 | 1 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 1 | 923 |
| rb_tbl | 1 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 1 | 930 |

(counts are `(use (reg` insns inside the func_80030580 section.) The cancel form's extra
insns are STILL PRESENT at `.cse2` (929 lines vs base 914); the orphan `(use)` insns first
appear at `.combine`. The producing site is **`tools/gcc-2.7.2/combine.c:10835-10840`**, whose
guard is `REG_NOTE_KIND (note) == REG_DEAD && place == 0 && tem != 0` -- combine deleted the
setter, the death note found **no home**, and the backward scan had **crossed a CODE_LABEL**.
The pseudos then survive `.lreg` unallocated, get no hard register at `.greg`, and
`reload1.c alter_reg` calls `assign_stack_local (SImode, 8, -1)` on each (align -1 rounds to 8).

**This turns the search into a two-clause predicate**, which is what the four probe families
below were designed against:
  - clause A (`tem != 0`): the dead register's death scan must cross a CODE_LABEL -- satisfied
    by anything computed before the `tbl[0]` arm chain and consumed inside an arm;
  - clause B (`place == 0`): combine's substitution must leave the register with **no remaining
    reference at all**.
Every shape measured to date splits cleanly on clause B: constructs where combine *folds* the
value into its consumer (packed-word narrowing, pre-branch address folding, read-back) keep a
reference and rehome the note; only constructs where combine *drops* the operand entirely
(`+ K - K`, multiply-by-zero, truncation) orphan it -- and dropping an operand entirely is the
same property the cheat checklist's T1 tests for.

**Family 1 -- READ-BACK (10 shapes, all vars=8): KILLED.** Consuming a field the function just
stored rather than the value stored (`tbl = &D_8008E194 + *(s16 *)(obj + 2) * 7`, `obj+0x50`
from `obj+4`, `obj+0x54` from `obj+7`, `obj+0x58` from `obj+8`, arm stores from `obj+0x54`,
the angle from `obj+0x56`, the flag from `obj+0xA`). GCC 2.7.2's cse does **not** forward these
stores to the later loads at all, so the read-back stays a REAL instruction: bodydiff 3 / 3 / 5
/ 6 / 7 / 9 / 22 / 37 with the frame untouched. Banked
`rejected/readback-of-stored-field-frame-inert-s8.c`.

**Family 2 -- PACKED-WORD EXTRACTION (10 shapes, all vars=8): KILLED, but 8 are byte-neutral.**
Spelling a narrow load as a field extraction from the containing aligned 32-bit word --
`(*(u32 *)(src + 0x1C8) >> 16) & 0xFFF` for `*(u16 *)(src + 0x1CA) & 0xFFF`,
`(*(s32 *)(src + 0x18) >> 16) / 32` for `*(s16 *)(src + 0x1A) / 32`, `*(u32 *)(src + 0x1C8) >> 16`
for the `obj+0x56` store -- compiles to **identical bytes** (bodydiff=0 for pk_j1, pk_j2, pk_j12,
pk_j1o56, pk_j1u, pk_o56, pk_s1a): combine narrows the `lw`+`srl` back to one `lhu` at the
+0x1CA offset and rehomes the note on the narrowed insn. Frame unmoved. Banked
`rejected/packed-word-extraction-frame-inert-s8.c`. These are free composables for a future
session (they change nothing in the bytes and can be layered under a real lever).

**Family 3 -- FRONTIER ITEM 3, the post-reload jump2 cross-jump route: KILLED.** The plan was
real insns before reload (which fixes the frame) that jump2 tail-merges away afterwards.
Measured: duplicating the *existing* `obj+0x50` / `obj+5` / `obj+0` tail into all four `tbl[0]`
arms costs **bodydiff=17** -- jump2 dedups duplicate tails but always keeps one copy, it never
deletes the last one -- and a same-value Judge re-store duplicated into 2 / 3 / 4 arms buys
exactly **one** extra slot regardless of copy count (`vars=16` in all of cj_jonly4 / cj_jt2 /
cj_jt3 / cj_jt4) at bodydiff 35..53, because cse merges the copies' folded addresses long
before reload. This also corrects the s8 reading of armdup50/armdup58 (a *single* duplicated
statement is byte-neutral; a three-statement tail at four arms is not). Banked
`rejected/crossjump-arm-duplication-not-byte-neutral-s8.c`.

**Family 4 -- PRE-BRANCH FOLD (8 shapes, all vars=8; 6 byte-neutral): KILLED.** Designed
directly from clause A: `s16 *e4 = tbl + 4` consumed inside the arms; `s16 *d = obj + 0x5C`
with all arm stores through `d[0..2]`; a named `kind = tbl[0]` driving the chain; a named
`amp = tbl[2]`; a source position pointer; `jb = &Judge` for the first lookup; `u8 *fl = obj + 0xA`
across the loop. bodydiff=0 for xb_e4, xb_e4b, xb_dst, xb_kind, xb_jbase, xb_srcp; 15 for
xb_flag; 65 for xb_amp -- clause A is satisfied and clause B is not, in every one. Worth noting
for the record: `xb_jbase` (ONE lookup through a `&Judge` pointer) is byte-neutral and KEEPS the
existing p110 orphan, whereas s7's kill (BOTH lookups through the pointer) measured vars=0 --
one folded site is enough to preserve the orphan. Banked
`rejected/prebranch-fold-shapes-frame-inert-s8.c`;
`xb_e4b` saved as `composable-tblentry-pointer-byte-neutral.c`.

**Net.** 36 measured shapes this session, 0 frame movement outside the already-banned
self-cancel family, the pass attribution corrected from cse2 to combine.c/distribute_notes, and
the residual reduced to a single sharply-stated question: *is there C with real semantics that
makes combine drop an operand entirely?* `candidate.c` is unchanged (the FAKE-free floor-2
body); `src/code6cac_b.c` restored to HEAD.

- [s8] Chassis re-measured this session (the dispatch measurement was unavailable): pure-c-floor2-body.c applied to src/code6cac_b.c gives sandbox func_80030580 --disable all score 2, target_insns 148, build_insns 148, rules_dropped 0, with zero FAKE constructs (fake_ablate).

- [s8] Target frame confirmed from asm/funcs/func_80030580.s: addiu $sp,$sp,-0x18 / addiu $sp,$sp,0x18, no $ra or callee-saved saves, and NOT ONE ($sp) reference in the 148-instruction body - so the 24 bytes are entirely untouched frame, consistent only with unallocated-pseudo alter_reg slots.

- [s8] PASS ATTRIBUTION (dumps banked, not guessed): the orphan (use (reg N)) insns first appear at .combine, not .cse2 - base has 1 through flow and 2 at combine; the vars=16 cancel form sa38 has 1 through flow and 4 at combine, and its .cse2 section is 929 lines against base 914. The emission site is tools/gcc-2.7.2/combine.c:10835-10840, guard REG_NOTE_KIND (note) == REG_DEAD && place == 0 && tem != 0.

- [s8] The residual is now a two-clause predicate: clause A - the dead register's death scan crosses a CODE_LABEL (tem != 0); clause B - combine's substitution leaves the register with no remaining reference (place == 0). Every measured family splits on clause B: operand-FOLDING shapes (packed-word narrowing, pre-branch address, read-back) rehome the note; only operand-DROPPING shapes (+ K - K, multiply-by-zero, truncation) orphan it.

- [s8] READ-BACK family measured frame-inert in 10 shapes (all vars=8, bodydiff 0..37): GCC 2.7.2 cse does no store-to-load forwarding for these u8*/u16* stores, so a read-back costs real instructions instead of a slot.

- [s8] PACKED-WORD EXTRACTION measured frame-inert in 10 shapes, 8 of them at bodydiff=0 - combine narrows lw+srl back to a single lhu and rehomes the note. Eight new byte-neutral composables banked.

- [s8] POST-RELOAD CROSS-JUMP (frontier item 3) measured: jump2 dedups duplicate arm tails but never deletes the last copy (cj_tail4 bodydiff=17 for a purely semantics-preserving duplication), and 2/3/4 duplicated Judge re-stores all buy exactly one extra slot (vars=16) at bodydiff 35..53. The route cannot be byte-neutral.

- [s8] PRE-BRANCH FOLD measured frame-inert in 8 shapes, 6 at bodydiff=0, including a named table-entry pointer (s16 *e4 = tbl + 4), a destination pointer for the obj+0x5C triple, a named kind local and a source position pointer - all ordinary C, all free.

- [s8] Side correction to the s7 judge-pointer-alias kill: routing ONE of the two Judge lookups through jb = &Judge is byte-neutral AND keeps the existing p110 orphan (vars=8); only routing BOTH lookups through the pointer destroys it (s7's vars=0).

- [s8] 36 measured shapes this session; the only bytes-proven vars=24 form remains the Judge-banned self-cancel pair (pr16_38, vars=24 bodydiff=4 sp=0), which was not applied to src/ and is not candidate.c.

- [s8] src/code6cac_b.c restored to HEAD; the working tree carries only memory/grind/func_80030580/ ledger changes plus metrics/events.jsonl.
