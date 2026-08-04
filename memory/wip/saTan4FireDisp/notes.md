# saTan4FireDisp — WIP (current state 2026-08-04, round 8)

`src/text1a.c:1176`. Fire effect: validates a stage/player id, converts three
colour channels to Q12, then a two-pass outer loop over a coordinate table
uploading 16x1 strips via `gpu_LoadImage` + `func_80048A7C`. 33 regfix + 2
asmfix rules (the asmfix pair moves a `{lbl#6}` label before an `lh`/`lhu`).

## Where it stands

| | score | frame | vars | insns |
|---|---|---|---|---|
| committed HEAD | 41 | 72 | 8 | 135 |
| **`candidate.c`** | **29** | **88** | **24** | **135** |
| target | 0 | 88 | 24 | 135 |

**Frame and instruction count are both exactly target's.** The residual 29 is a
3-cycle callee-save rotation (ours `s2`/`s3`/`s4` where target has
`s3`/`s4`/`s2`) plus the scheduling that rides on it.

## The structural fix (round 7) — the whole win

Target keeps the **red channel** and the **loop sentinel** in *different*
registers (colour args from `$s7`/`$s6`/`$s3`, sentinel in `$v0`), and at the
loop label reads `tbl[0]` **twice**: `lh $v0,0($s0)` for the test, `lhu
$v1,0($s0)` for the next `rect[0]`. The m2c-shaped body conflated both into one
`r`, which put the label in the wrong place — exactly what the two asmfix rules
compensate for. Giving the sentinel its own variable and writing
`while ((sent = tbl[0]) >= 0)` puts the read at the label as target does,
supplies the missing third phantom slot, and holds 135 insns. **41 → 29.**

Also a **semantic correction**: the old body passed `r` to `func_80048A7C`, so
from the second iteration onward it passed the previous `tbl[0]` instead of the
red channel. Target always passes `$s7`.

**Expected side effect:** the two asmfix rules should become unnecessary —
verify at the completion gate.

## The frame mechanism (rounds 3-6)

`tmp/stf_orphan.py` gives the rule: **vars = 8 × (pseudos in greg's allocate
list that get no hard register)**. HEAD has one → vars 8; target needs three.
Screen slot ideas with it, **not** the score — intermediate frames read worse
than both endpoints (a vars=16 form scores 47 vs HEAD's 41 purely because the
offsets are shifted 8 but still 8 short). Before the structural fix, three slots
only came from a type narrowing that cost instructions (`s16 r` +6 → 40; one
rect coordinate +3 → 48; both +6 → 51).

## Measured negative / inert (do not re-run)

- **Load types** audited per site against target (`lh 0x8($fp)` ×2,
  `lh D_800A9A20`, `lhu 0x0/0x2($s0)`, `lh 0x18/0x1A($sp)`, `lh 0x0($s0)`) —
  ours already match all of them; no type-correction lever exists.
- **Pure namings never add a slot** (8 forms: rect base pointer, `(s32)rect`
  argument, image source address, palette table pointer, second `sid` copy, loop
  shift amount, `s16` outer selector, `func_8004881C` result chain).
- **No third slot** from: `u16` carriers on the two `lhu` sites (pair / `t0` /
  `t1` / `u16 *` walker); `s16 sent` alone; `s16 want` for the comparand; `s16`
  on `g`, `b`, `g`+`b`, `idx`, `outer`, `xoff`, `yoff`; an `s16` carrier for the
  `0x10`/`1` rect constants; named `tbl` element reads.
- Naming the twice-read `D_80094E08[sid]` palette index **removes** a slot — its
  two separate reads are load-bearing.
- **Call-arg LUID** ([[hoist-call-arg-local-flips-jal-delay]], round 8): naming
  both `gpu_LoadImage` args first in the loop block, or either one alone — all
  inert at 29; moving the rect stores after the arg locals regresses to 52.
- `for (;;) { sent = tbl[0]; if (sent < 0) break; … }`: 131 insns, 69 — the
  `while`-with-assignment form is specifically the matching one.
- `sid` combined with the corrected loop overshoots to four slots (vars 32).

## Round 9 — RA Step-0 diagnosis + Levers A-C, no movement

**Step-0 (`.greg`, candidate applied).** The rotating values are:

| pseudo | value | ours | target |
|---|---|---|---|
| 76 | `r` (red) | `$s7` (23) | `$s7` ✓ |
| 77 | `g` | `$s6` (22) | `$s6` ✓ |
| 78 | `b` | `$s2` (18) | **`$s3`** |
| 82 | `yoff` | `$s3` (19) | **`$s4`** |
| 79 | `outer` | `$s4` (20) | **`$s2`** |

Pseudos 75-84 **mutually conflict** — one clique — so each simply takes the
first free register in allocno-priority order. Ours allocates in the order
`b, yoff, outer`; target's order must be `outer, b, yoff`. So the whole residual
is one permutation of three allocno priorities, and any lever must raise
`outer` above `b` and push `yoff` last.

**Levers A-C measured (10 forms), none flips it:** colour birth order `b,r,g`
(32) / `g,b,r` (39) / `r,b,g` (29); outer if/else arm swap (31); `yoff` before
`xoff` in both arms (31); `outer = outer + 1` long form (29, inert); an extra
`outer` reference in each arm (29, inert and contrived — not a form to keep);
`s16 outer` (34, +4 insns); `s16 yoff` (29, inert). Frame and instruction count
stay exact (24 / 135) throughout — none regress the structure, they just do not
move the priority order.

## Resume here — banked at 29

Start from `candidate.c` (29). One question remains: the 3-cycle allocno-priority
permutation above. What has **not** been run is the rest of the
[[register-alloc-pure-c]] ladder:

1. **ALLOCDBG numbers.** The instrumented cc1 (`tools/gcc-2.7.2/cc1`, per
   [[instrumented-cc1-location]]) prints the allocno-sort table with the actual
   `floor_log2(nrefs)*nrefs/livelen*10000` priorities. Get the three numbers
   before trying another lever — this round's 10 forms were aimed at the right
   pseudos but without knowing how far apart their priorities are.
2. **Sanctioned ref-lift** ([[duplicated-statement-into-arms]]) on `outer`, with
   byte-neutrality verified and a FAKE annotation, once (1) shows how much lift
   is needed.
3. **Permuter**, clean single-function target at offset 0, directed `PERM_*`,
   fresh-seed discipline — not yet attempted for this function.

Screen anything touching the frame with `tmp/stf_orphan.py`.

## Instruments

`tmp/stf*.py` (rounds 1-9), **`tmp/stf_orphan.py`** (unallocated-pseudo
detector), `tmp/stf_apply.py`, `tmp/rtldump.sh`, `tmp/orphan_probe.py`,
`tmp/frame_probe.sh`, `tmp/sbs.sh`.
