# saTan4FireDisp — WIP (opened 2026-08-04)

`src/text1a.c:1176`. Draws a fire effect: validates a stage/player id, converts
three colour channels to Q12, then runs a two-pass outer loop over a coordinate
table, uploading 16x1 image strips via `gpu_LoadImage` and calling
`func_80048A7C` per strip.

**33 regfix rules + 2 asmfix rules** (the asmfix pair moves a `{lbl#6}` label
before an `lh`/`lhu` — a jump-target fix for the `inner_check` goto, not a
codegen cheat in the usual sense). No prologue_config / frame_fix entries.
No prior WIP.

## Where it stands

| | score | frame | vars | insns |
|---|---|---|---|---|
| committed HEAD | 41 | 72 | 8 | 135 |
| **`candidate.c`** | **40** | **88** | **24** | 141 |
| target | 0 | 88 | 24 | 135 |

**Instruction counts already match (135/135).** The whole 41 is the
sp-offset cascade plus register naming.

## The frame: 16 bytes = TWO phantom slots

Target frame 88 = args 24 + regs 40 (10 saves at `0x30`-`0x54`) + vars 24.
Only `0x18`-`0x1F` of the vars area is ever touched — that is the real
`s16 rect[4]`, which we also have. `0x20`-`0x2F` is **never referenced**:
two untouched phantom slots.

Because the saves sit above vars, being at vars=8 instead of 24 shifts every
one of the 10 save/restore offsets and both `sp` adjusts. That is why the score
is 41 despite a matching instruction count — and why a *partial* frame fix
reads as a regression rather than progress (see below).

## Round 1-2 findings

Naming the thrice-read `*(((s16 *) fp_ptr) + 4)` field in an `s16 sid` local
moves vars 8 → 16 **at zero instruction cost**. Several variants give the same
free +8 (`sid` alone; `sid` + an s16 for the sentinel read; `sid` + s16
`xoff`/`yoff`; `sid` + a named comparand) — all score 47.

**41 → 47 on those forms is NOT a regression signal.** At vars=16 the offsets
are shifted 8 from HEAD but still 8 short of target, so they are wrong in a
different way. Only vars=24 can align them.

## Rounds 3-4: the mechanism is now measured, and the frame is reachable

`tmp/stf_orphan.py` (RTL `-da` dump + greg unallocated-pseudo report per
variant) established the rule directly:

> **vars = 8 × (number of pseudos in greg's allocate list that receive no hard
> register).**

HEAD has one such pseudo (80) → vars 8. Target needs **three** → vars 24.

| form | unallocated | vars | insns | score |
|---|---|---|---|---|
| HEAD | `[80]` | 8 | 135 | 41 |
| `sid` | `[80, 88]` | 16 | 135 | 47 |
| `rect_s16` | `[75, 80, 81]` | 24 | 141 | 51 |
| **`sid` + `s16 r`** | 3 | **24** | 141 | **40** |
| `sid` + `rect_s16` | 4 | 32 | 142 | 78 |

**`candidate.c` = `sid` + `s16 r`: score 40, and the frame is EXACTLY target's
(88 / vars 24 / regs 10 / args 24).** The `s16 r` conversion costs +6
instructions (sign-extends), but the score still improves because the
sp-offset cascade collapses once the frame is right. That confirms the frame
was the dominant term.

## The third slot is the whole remaining problem

`sid` supplies slot 2 for free. Every producer found for slot 3 costs
instructions:

- `s16 r` (the candidate): +6, score 40.
- naming one rect coordinate (`rx` **or** `ry`, keeping the other re-read):
  vars 24, +3 insns, score 48 — cheaper in instructions than `s16 r` but worse
  overall.
- `rect_s16` (both coordinates): vars 24, +6, score 51.
- `s16 r` with an explicit `(s32)` cast at the call, or with the sentinel read
  left alone: identical at 40 (the cast is inert).
- `u16 r` with a `(s16)` cast at the test: only 2 slots, 136 insns, 46.

Slot-3 probes that do **not** add a slot at all: `s16` on `g`, `b`, `g`+`b`,
`idx`, `outer`, `xoff`, `yoff`, a named `sent` for the sentinel read, an `s16`
carrier for the `0x10`/`1` rect constants, named `tbl` element reads. Naming
the twice-read `D_80094E08[sid]` palette index actually *removes* a slot
(back to vars 8) — its two separate reads are load-bearing.

## Diff shape (why this is frame-dominated)

`tmp/sbs.sh` on HEAD: the instruction streams are **already 1:1** for the whole
prologue and body. The only differences are the sp offsets (uniformly 16 apart)
and one register rotation — target holds `fp_ptr` in `$s8` where we use `$s7`,
though both builds save the same ten registers. So the 41 decomposes as ~22
offset diffs (10 saves + 10 restores + 2 `sp` adjusts) plus the rotation
cascade.

## Resume here

Find a **free** third slot — one that adds an unallocated pseudo without adding
instructions, the way `sid` does. `tmp/stf_orphan.py` is the detector: it prints
the unallocated set per variant, so candidates can be screened without reading
scores. Once at 3 slots / 135 insns the remaining gap should be only the
`$s7`/`$s8` rotation.

Start from `candidate.c` (40), not HEAD.

## Instruments

`tmp/stf.py` … `tmp/stf4.py` (rounds 1-4 sweeps), **`tmp/stf_orphan.py`** (the
unallocated-pseudo detector — screen slot candidates with this, not with
scores), `tmp/stf_bank.py`, `tmp/rtldump.sh`, `tmp/orphan_probe.py`,
`tmp/frame_probe.sh`, `tmp/fdiff.sh`, `tmp/sbs.sh`.
