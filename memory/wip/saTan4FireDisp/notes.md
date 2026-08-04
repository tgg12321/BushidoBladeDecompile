# saTan4FireDisp — WIP (current state 2026-08-04, round 8)

`src/text1a.c:1176`. Draws a fire effect: validates a stage/player id, converts
three colour channels to Q12, then runs a two-pass outer loop over a coordinate
table, uploading 16x1 image strips via `gpu_LoadImage` and calling
`func_80048A7C` per strip.

33 regfix rules + 2 asmfix rules (the asmfix pair moves a `{lbl#6}` label before
an `lh`/`lhu`). No prologue_config / frame_fix entries.

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
registers: the body's colour args come from `$s7`/`$s6`/`$s3` while the sentinel
lives in `$v0`, and at the loop label target reads `tbl[0]` **twice** —
`lh $v0,0($s0)` for the test and `lhu $v1,0($s0)` for the next `rect[0]`.

The m2c-shaped body conflated the two into a single `r`, which is what put the
label in the wrong place — and that is precisely what the two asmfix rules were
compensating for. Giving the sentinel its own variable and writing the loop as a
`while` with the assignment in the condition:

```c
s32 sent;
...
idx = 0;
while ((sent = tbl[0]) >= 0) {
    ... body, colours still r/g/b ...
}
```

puts the read at the label exactly as target does, supplies the missing third
phantom slot, and keeps the instruction count at 135. **41 → 29.**

This is also a **semantic correction**: the old body passed `r` to
`func_80048A7C`, so from the second iteration onward it passed the previous
`tbl[0]` instead of the red channel. Target always passes `$s7` (red).

**Expected side effect:** if this lands, the two asmfix rules for this function
should become unnecessary. Verify at the completion gate.

## The frame mechanism (rounds 3-6)

`tmp/stf_orphan.py` (RTL `-da` dump + greg unallocated-pseudo report per
variant) gives the rule directly: **vars = 8 × (pseudos in greg's allocate list
that receive no hard register)**. HEAD has one (80) → vars 8; target needs
three. Screen slot candidates with this, **not** with the score — intermediate
frames score worse than both endpoints (a vars=16 form reads as 47 vs HEAD's 41
purely because the offsets are shifted 8 but still 8 short).

Before the structural fix, the only route to three slots was a type narrowing
that cost instructions: `s16 r` (+6, score 40 — the previous candidate), naming
one rect coordinate (+3, 48), both coordinates (+6, 51). `sid` (naming the
thrice-read `*(((s16 *) fp_ptr) + 4)`) is free because it names an s16 value
that already arrives sign-extended from an `lh`, whereas `r` holds a *computed*
s32, so narrowing it forces a truncate plus re-extension.

## Measured negative / inert (do not re-run)

- **Load types**: audited per site against target — `lh 0x8($fp)` ×2,
  `lh D_800A9A20`, `lhu 0x0/0x2($s0)`, `lh 0x18/0x1A($sp)`, `lh 0x0($s0)` — ours
  already match all of them. No type-correction lever exists.
- **Pure namings** (8 forms: rect base pointer, `(s32)rect` argument, image
  source address, palette table pointer, second `sid` copy, loop shift amount,
  `s16` outer selector, `func_8004881C` result chain) — never add a slot.
- **`u16` carriers on the two `lhu` sites** (pair / `t0` / `t1` / `u16 *`
  walker), `s16 sent` alone, `s16 want` for the comparand — no third slot.
- **`s16` on** `g`, `b`, `g`+`b`, `idx`, `outer`, `xoff`, `yoff`; an `s16`
  carrier for the `0x10`/`1` rect constants; named `tbl` element reads — no slot.
- Naming the twice-read `D_80094E08[sid]` palette index **removes** a slot; its
  two separate reads are load-bearing.
- **Round 8, call-arg LUID** ([[hoist-call-arg-local-flips-jal-delay]]): naming
  both `gpu_LoadImage` args first in the loop block, or only the source address,
  or only the rect pointer — **all inert at 29**. Moving the rect stores after
  the arg locals regresses to 52 (loses the frame). Target computes those args
  early and we compute them late, but no C-visible lever moves it.
- `for (;;) { sent = tbl[0]; if (sent < 0) break; … }` forms: 131 insns, 69 —
  the `while`-with-assignment form is the one that matches.
- `sid` combined with the corrected loop: overshoots to four slots (vars 32).

## Resume here

Start from `candidate.c` (29). The only open question is the 3-cycle callee-save
rotation `s2`/`s3`/`s4` → `s3`/`s4`/`s2` — a register-allocation tie of the kind
[[register-alloc-pure-c]] covers. The frame and instruction count are settled, so
nothing structural remains. Screen with `tmp/stf_orphan.py` if any idea touches
the frame.

## Instruments

`tmp/stf.py` … `tmp/stf8.py` (rounds 1-8), **`tmp/stf_orphan.py`** (the
unallocated-pseudo detector), `tmp/stf_bank2.py`, `tmp/stf_apply.py`,
`tmp/rtldump.sh`, `tmp/orphan_probe.py`, `tmp/frame_probe.sh`, `tmp/sbs.sh`.
