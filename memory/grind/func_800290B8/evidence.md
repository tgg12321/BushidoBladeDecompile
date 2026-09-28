# func_800290B8 evidence

## s1 [manual, cloud Linux container 2026-09-28] — 231 -> 0 (first C body)

Picked from the rotated set (rotation 2026-09-21 was a recon-only skip: "28-branch nested
collision/audio search ... selecting the next C candidate with only five branches"; no prior
attempt, no ledger). `canonical`: verdict C, hand-coded tier LOW.

### What the function does
`s32 func_800290B8(s32 side, s32 swap, LeafPos *quads)`, called twice from func_80029454
(`side` 0 or 1, `quads` a table of 12-byte vertices, four per side laid out as a 2x2 grid).
1. Bounding box of the side's four corners into the scratchpad record at 0x1F8002B8: corner 0
   is struct-copied into the max slot (+0x84) and then into the min slot (+0x78); corners 1-3
   (`i = 1..3`, row `i / 2`, column `i & 1`, vertex `side * 4 + row * 2 + col`) widen min/max x
   and z and max y (min y keeps corner 0's value).
2. Walks the marker list from func_8004678C (16-byte records `{s16 type; s16 done; s32 x, y,
   z;}`, type 0 terminates; func_8002906C clears every `done`). A marker not yet done and inside
   the box (y <= max y, x and z within [min, max]) has its x/z copied to scratchpad +0x100/+0x108
   and is tested against the quad's two triangles (corners 0,1,2 then 1,2,3) with func_8002E6B0.
   On a hit: a marker whose type is not 2 -> midpoint y into +0x104, return 1. Type 2 with
   `swap` set on side 0 -> switch to side 1 and restart the triangle loop (`tmp_b = -1;
   continue;`). Otherwise func_80044B30(marker index, func_8002FC80(a, b, c)),
   func_80033550(corner a), mark the marker done, return 0. After both triangles miss:
   `swap && side != 0` -> the same midpoint-y, return 1.
3. Return 0 when no marker qualifies.

### Ladder (sandbox --disable all --candidate)
| step | score |
|---|---|
| first full body: list[j] indexing, flat corner index `side*4 + (i/2)*2 + (i&1)` | 62 |
| walking marker pointer `e` + `e->y > max` operand order (loads e->y first, as the target) | 46 |
| corner row/column through the function's two index variables (reuse) | 0 |
| cleanup: `LeafPos` vertices (the existing 12-byte {x,y,z} type func_80033550 takes), found-label at the end of the loop body (no jump into a block), no `list` copy | 0 |

The last 46 points were two allocation facts, both read off the target: the marker index is in
t0 with caller-save spills around the two calls (`sw/lw t0,0x18(sp)`), and loop 1 computes
`i / 2` into t0 and `i & 1` into s0 (the triangle counter's register). The .lreg/.greg dumps
showed the separate-variable marker index with 8 refs over 2 calls -> no register (global.c
caller-save retry refused, regs.h:165), and the column as a block-local pseudo in v1. Sharing the
two index variables between the loops reproduces both. Ruling 11 applies to both variables:
full (D) proof in ruling11.md, dumps in dumps/ruling11-dumps.txt, comparison bodies and
scripts in r11/.

### Constructs a reviewer should scrutinize (everything else is plain C)
- `tmp_a`, `tmp_b`: Ruling 11 (ruling11.md walks (A)-(H)).
- `vtx`: the corner's vertex index, one write statement per iteration, read seven times
  (x/z/y compares and stores). A real value (target `addu a0,v0,s0` 0x80029168; the target
  recomputes `vtx * 12 + quads` after each join from a0). Written inline instead
  (`quads[side * 4 + tmp_a * 2 + tmp_b]` seven times, r11/no_vtx.c) it measures 89.
- `u8 *scr = (u8 *)0x1F8002B8;` with `*(s32 *)(scr + off)` / `*(LeafPos *)(scr + off)`: the
  scratchpad record convention of this file (func_8002C22C, func_8002EBDC, ...). The two
  `LeafPos` struct copies are the target's `lw/lw/lw; sw/sw/sw` pairs 0x80029110-44.
- `(VECTOR *)a` etc. for func_8002FC80, whose definition in this file takes `VECTOR *` and
  reads only vx/vy/vz; the table is 12-byte {x,y,z} (func_80033550 takes the same corner as
  `LeafPos *`). Call-argument conversion only, no codegen effect.
- File-local prototypes for func_8002E6B0 / func_8002FC80 / func_80033550 (defined later in
  this file with the same signatures) and func_80044B30 (text1a_c.c, `void (s32, s32)`).
- `side = 1; tmp_b = -1; continue;` rewrites the parameter and restarts the triangle loop
  (target: `addiu s7,zero,1` / `j 0x800293B8` / `addiu s0,zero,-1`, then the step makes it 0).

### Landing checks
- `sandbox func_800290B8 --disable all` on the spliced src: 0 (231/231).
- `build` (full clean build): SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH.
- `tools/check_completion_integrity.py`: OK.
(Re-run 2026-09-28 s2 on HEAD a6aa639c: all three still hold.)

### Environment note
The cloud container has no `disc/`; disc/SLUS_006.63 was rebuilt from build/bb2.bin plus the
standard PS-X EXE header (pc0 0x800836EC, t_addr 0x80010000, t_size 0x93800, s_addr
0x801FFFF0, NA license string) and proven by its SHA1 == the oracle (tmp/rebuild_header.py).
The permuter was cloned into tools/decomp-permuter (gitignored) with pycparser; the mips objdump
it looks for is a symlink to mipsel-linux-gnu-objdump.

## s2 [manual, cloud Linux container 2026-09-28] — landing package completed (0, unchanged body)

Picked from the rotated set again (the queue top is being worked by other agents). Body
unchanged; this session completed the Ruling 11 (D)(4) receipts and corrected (D)(3).

### permuter
Both campaigns from the per-value body ran to completion (details, iteration counts and the
finds in ruling11.md (D)(4)): best 20 from 1070 in each, no zero; each best find stages a loop-1
bounding-box value through `mark`, i.e. puts a second value in the marker index's pseudo (the
tmp_a lever, spelled as a staged-value borrow, not admissible). Faithfulness: the reuse
candidate scores 0 in the same minimal TU (tmp/perm290_chk2).

### sanctioned families on the per-value body (fam/, mkfam.py)
18 probes (do-while(0) wraps at seven sites incl. nested, chain extenders, step duplicated
into every continue arm, pointer alias; plus the init wrap combined with the ablations and with
loop-1 wraps). Result: `do { mark = 0; } while (0);` (fam/dw_init.c) scores **4/231**: the wrap
lifts mark's weighted refs to 9, so the caller-save retry passes and mark gets t0 with the
target's spills (dumps/fam-dw_init-dumps.txt). The residual is exactly the loop-1 row (v0 vs
t0) and column (v1 vs s0) operands. No family moves those: every family probe keeps them in
v0/v1 (all 4 or worse).

### Consequence for the Ruling 11 proof
s1's necessity argument for tmp_a (the marker index can never exceed 8 refs) was wrong: a
sanctioned wrap exceeds it. ruling11.md (D)(2)/(D)(3) are rewritten: tmp_a's necessity is the
corner row's t0 seat (the row is block-local in every per-value spelling, local-alloc's
lowest-first scan with no hard-register suggestions gives v0), exactly parallel to tmp_b's
column/s0 argument. The withdrawn claim is recorded in (D)(3).

### Landing checks (s2, spliced over INCLUDE_ASM in src/code6cac_b.c)
- `sandbox func_800290B8 --disable all`: 0 (231/231).
- `build` (full clean build): SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa MATCH.
- `tools/check_completion_integrity.py`: OK. `dossier`: CONSISTENCY OK.
- cc1psx calibration: not available in this container (no tools/cc1psx.exe); Ruling 11 does not
  require it.
