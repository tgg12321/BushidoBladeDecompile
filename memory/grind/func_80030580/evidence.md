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
