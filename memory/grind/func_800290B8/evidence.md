# func_800290B8 — evidence

> Merge provenance (2026-09-30): func_800290B8 was landed COMPLETED-C independently by two
> lanes on 2026-09-28: local main `2b8872877` (manual; ledger: r11/proof.md) and origin/main
> `02e9cfe6b` (manual, cloud container; ledger: ruling11.md, dumps/, fam/, r11/). Both bodies
> byte-match with the same Ruling 11 two-variable reuse. The merged tree keeps the LOCAL body
> in src/code6cac_b_tu2.c (func_80029454 was landed after it in that TU context); origin's
> body is banked as candidate.origin-02e9cfe6b.c. Both lanes' records follow in full.

# [local lane — 2b8872877, landed 2026-09-28]

# func_800290B8 — evidence

## s1 (manual, 2026-09-28, parallel lane beside the func_8006C21C session)

Picked from the rotated tail: rotated 2026-09-21 with no ledger ("28-branch nested collision/
audio search ... selecting the next C candidate with only five branches") — i.e. never worked.
`canonical`: verdict C, hand_coded_tier LOW, 231 insns.

What the function is (read from asm/funcs/func_800290B8.s): `tbl` is 2 × 4 LeafPos points
(caller func_80029454 passes idx 0 or 1). It copies point idx*4 into the scratchpad record at
0x1F8002B8 as both max (+0x84) and min (+0x78), folds points 1..3 in (x/z min+max, y max
only), then walks the 16-byte entry list from func_8004678C (s16 type, s16 used, s32 x/y/z;
func_8002906C clears `used`) for unused in-bounds entries, puts x/z at +0x100 and tests the
two triangles (0,1,2) / (1,2,3) with func_8002E6B0. See the function comment in candidate.c.

Score trail (`sandbox --disable all`, candidate):
- 71/231 — first transcription (inline `(i / 2) * 2 + (i & 1)`, hit block after the loop).
- 52 — `i / 2` and `i & 1` written into the list-index / triangle-number variables (the target
  holds them in t0 and s0, the registers of those two variables), `rec->y > max.y` order.
- 5 — hit block placed at the outer loop's tail (`if (flag == 0 || idx == 0) continue; hit:`)
  — the target lays it out inline before the `j++` continue; `return 0` vs `goto end` and
  `k = 0` placement measured neutral.
- **0** — head loop `tbl[n].y > max.y` operand order. Every remaining hunk is `not-scored`
  (masked branch displacement).

The 0 needs the two reused locals (`temp`: row then list index; `temp2`: column then triangle
number). No earlier ruling fits (not one role/meaning — Rulings 5/6/9; not SOTN/original source
— 8/10), so they are submitted under Ruling 11: r11/proof.md (all prongs, dumps, mechanism,
measured alternatives, permuter).

Mechanisms (r11/dumps.txt):
- list index: crosses 2 calls, so global.c's caller-save retry needs refs > 8
  (`CALLER_SAVE_PROFITABLE`, regs.h:164). Reuse 12 refs → t0 + caller-save (target). Split 8
  refs → no register → stack slot (`sw $0,24($sp)`).
- column: split `col` is a single-block pseudo → local-alloc → v1; target's `andi s0` needs the
  multi-block global pseudo (temp2) seated in s0.

## s1 layer-2 round 1 (2026-09-28): FAIL(EVIDENCE) — ledger only
Fresh cheat-reviewer confirmed the staged body == r11/final.c, the oracle, the behaviour trace
against the asm, every (D)(2) citation in tools/gcc-2.7.2, and both necessity arguments in
substance, and passed (H). It failed three things, all fixed without touching the body:
1. r11/proof.md + dumps.txt named pseudo 98 as the split `col` (a number from an earlier split
   variant's dump); the banked excerpt contradicted it. Now the mapping is computed from each
   dump by r11/pmap.py (split: row 93 → v0, col 94 → v1 in block 1, triangle number 176 → s0,
   list index 82 → no register).
2. (C)(1): the one-var form left `temp2`'s second value at function scope; it is now declared
   in the record-loop body (byte-identical asm, still 21). Permuter campaign 3 re-run from it.
3. The commit message pre-claimed a layer-2 PASS and said "below" for "not above" the
   caller-save threshold.
Also taken from the review: global.c's kick-out path cannot hand out t0 (used2 ⊇
call_used_reg_set on the first pass), now stated in (D)(2); function comment's "mid y" reworded.

## s1 layer-2 round 2 (2026-09-28): PASS
Fresh cheat-reviewer re-derived the package from scratch (raw dumps, pmap mapping read from the
insns, gcc citations, permuter logs) and passed both variables on every Ruling 11 prong plus (H).
Non-blocking notes applied before commit: campaign-1 tally 28 finds (2 unscored, both invalid),
iteration counts per the logs, commit message cites regs.h for CALLER_SAVE_PROFITABLE.

# [origin lane — 02e9cfe6b, landed 2026-09-28]

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

### Layer-2 #1 (s2, 2026-09-28): FAIL — (D)(3) premise, body not at issue
Reviewer passed (A), (B), (C), (E), (F), (H) and the (D)(2) citations. It failed (D)(3): "row
and column are block-local in every per-value spelling" is false. Duplicating the `vtx`
statement into the arms of a test (duplicated-statement-into-arms family) makes both pseudos
multi-block, so global.c seats them, and that path was not argued. Response:
- Probes banked: fam/dup_vtx_col.c 25, dup_vtx_row.c 24, dwi_dup_col.c 8, dwi_dup_row.c 7
  (mkfam.py); dumps dumps/fam-dup-dumps.txt (fam/dump_dup.sh + fam/gexcerpt.py). In each, row is
  global allocno 92, preference {v0} only, seated in v0; column allocno 96, no preference, v1.
- ruling11.md (D)(3) rewritten as a two-allocator contradiction argument keyed to the target:
  a3 is referenced once (0x80029338, marker loop), so it is free over the row/column lives. That
  bounds local-alloc's lowest-first scan and global.c `find_reg` pass 0 (call-used regs are in
  regs_used_so_far; a3 cannot be someone-preferred because call-crossing allocnos lose call-used
  preferences in pruning, and the only a3 preference goes to `scr`, which conflicts with every
  loop-1 allocno) at <= 7, and the preference step can only name v0/a0/a1. Never t0/s0.

### Layer-2 #2 (s2, 2026-09-28): FAIL — (D)(3) completeness, body not at issue
Reviewer passed (A), (B), (C), (E), (F), (G precondition), (H) and verified all other citations. Gaps:
(1) point 6 claimed reload re-seats evicted pseudos through find_reg. Local-alloc pseudos have no
allocno and are served from spill registers (t0 in pv.c), and point 6 was closed only by
measurement. (2) global.c's kick-out path (1096-1159) was not argued. (3) The preference-step
bullet ignored expand_preferences merges into the allocno's own sets. Response: ruling11.md (D)(3)
points 5 and 6 rewritten. Seats <= 7 are never spill registers: v0/a0-a3 are explicitly used, so
they go into bad_spill_regs; v1 is behind 8 zero-use call-used registers (t2-t9, absent from the
target) in potential_reload_regs, and an insn needs at most 3. Kick-out cannot target call-used
registers for call-crossing allocnos, and never runs for the others while t2-t9 are free. No
preference chain can carry t0/s0: no pre-allocation RTL names them, and a local-alloc seat there
would be the row/column itself. The mips.h CALL_USED list is corrected (also 24-29, 31).
Housekeeping: the reviewer restored metrics/events.jsonl to HEAD after its sandbox runs. The
only uncommitted change at that time was its own sandbox lines, since this session had
committed metrics before the review.

### Layer-2 #3 (s2, 2026-09-28): PASS
Fresh cheat-reviewer, given the corrected ledger. Walked (A)-(H) for tmp_a and tmp_b separately,
re-ran the gates (splice == candidate.c, sandbox 0 231/231, full build SHA1 MATCH, integrity OK,
find_all_cheats []), verified every GCC source citation and the banked dumps, and compiled its own
counter-spellings on the dw_init base: register row/col, reordered or function-scope declarations,
u32 col, column statement first, vtx operand order, and F6 empty-condition reads after
func_8004678C and inside the marker loop. None scored below 4 or moved the row off v0 or the
column off v1. It noted three wording slips, which it judged not disqualifying: the
index-split-temp case in point 5, disjoint-live-range pseudos for the a3 preference, and the
(C)(2) citation in point 1. All three are corrected in ruling11.md in the landing commit.
