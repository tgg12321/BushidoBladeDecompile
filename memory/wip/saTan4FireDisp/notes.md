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

HEAD's instruction count already matches target. Target frame 88 = args 24 +
regs 40 (10 saves at `0x30`-`0x54`) + vars 24, and only `0x18`-`0x1F` of vars is
ever touched (the real `s16 rect[4]`, which we also have). `0x20`-`0x2F` is
never referenced: **two untouched phantom slots**. Since the saves sit above
vars, being at vars=8 shifts all 10 save/restore offsets and both `sp` adjusts.

`s16 sid` (naming the thrice-read `*(((s16 *) fp_ptr) + 4)`) moves vars 8 → 16
at **zero instruction cost**. **41 → 47 on such forms is NOT a regression:** at
vars=16 the offsets are shifted 8 from HEAD but still 8 short of target, so
intermediate scores are uninformative.

## Rounds 3-4: the mechanism, measured

`tmp/stf_orphan.py` (RTL `-da` dump + greg unallocated-pseudo report per
variant) gives the rule directly: **vars = 8 × (pseudos in greg's allocate list
that receive no hard register)**. HEAD has one (80) → vars 8; target needs three.

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

`sid` supplies slot 2 free. Every slot-3 producer found costs instructions:
`s16 r` +6 (score 40, the candidate); naming one rect coordinate while keeping
the other re-read +3 (48); `rect_s16` both coordinates +6 (51). An explicit
`(s32)` cast at the call or leaving the sentinel read alone is inert (still 40);
`u16 r` with an `(s16)` test cast gives only 2 slots (46).

Adding **no** slot: `s16` on `g`, `b`, `g`+`b`, `idx`, `outer`, `xoff`, `yoff`;
a named `sent`; an `s16` carrier for the `0x10`/`1` rect constants; named `tbl`
element reads. Naming the twice-read `D_80094E08[sid]` palette index *removes* a
slot — its two separate reads are load-bearing.

## Diff shape

`tmp/sbs.sh` on HEAD: the instruction streams are **already 1:1** across the
whole prologue and body. The only differences are sp offsets (uniformly 16
apart) and one register rotation — target holds `fp_ptr` in `$s8`, we use `$s7`,
though both save the same ten registers. So the 41 is ~22 offset diffs (10
saves + 10 restores + 2 `sp` adjusts) plus that rotation.

## Rounds 5-6: the free third slot resists (17 more forms)

Target's load types were checked per site and **ours already match all of
them**: `lh 0x8($fp)` ×2 (the `sid` field), `lh D_800A9A20`, `lhu 0x0/0x2($s0)`
(the `tbl` pair), `lh 0x18/0x1A($sp)` (the `rect` read-back for
`func_80048A7C`), `lh 0x0($s0)` (the sentinel). So there is no type-correction
lever here — the `u16`/`s16` choices in the source are already right.

Screened by **unallocated-pseudo count** rather than score. Every form below
stays at exactly **two** slots (vars 16) at 135 insns:

- *pure namings of existing sub-expressions* (no type change): rect base
  pointer, `(s32)rect` call argument, the image source address, the palette
  table pointer, a second `sid` copy, the loop shift amount, an `s16` outer
  selector, the `func_8004881C` result chain — 8 forms, all `['80', '88/89']`.
- *`u16` carriers on the two `lhu` sites*: both together (53), `t0` only (47),
  `t1` only (47), a `u16 *` walker (47).
- *other memory carriers*: an `s16 sent` for the sentinel read (47), an
  `s16 want` for the `D_800A9A20` comparand (47), `u16` pair + sentinel (53).

**Why `s16 r` costs 6 and nothing else buys a slot free:** `sid` is free because
it names an s16 value that already arrives sign-extended from an `lh` and is
used where that form is wanted. `r` holds a *computed* s32 (`(a0 << 12) / 255`),
so narrowing it forces a truncate plus a re-extension — the diff shows target
passing it as `move a0,s7` where we emit `sll a0,s0,0x10 / sra a0,a0,0x10` plus
an extra `move`. Naming alone never adds a slot; only a narrowing does, and
every narrowing in this body sits on a computed value.

## Resume here

`candidate.c` (40) is the base; the frame is already exactly target's. The open
question is unchanged and now well-bounded: **a third unallocated pseudo at 135
instructions.** ~35 forms across six rounds say it is not reachable by naming or
by any type choice that matches target's load widths.

Untried and worth a fresh eye: whether the third slot in target comes from
something structural rather than a local — e.g. the `goto`/label shape around
`inner_check`, which the two asmfix rules already manipulate. Screen any idea
with `tmp/stf_orphan.py` (unallocated set), not with the score.

## Instruments

`tmp/stf.py` … `tmp/stf4.py` (rounds 1-4 sweeps), **`tmp/stf_orphan.py`** (the
unallocated-pseudo detector — screen slot candidates with this, not with
scores), `tmp/stf_bank.py`, `tmp/rtldump.sh`, `tmp/orphan_probe.py`,
`tmp/frame_probe.sh`, `tmp/fdiff.sh`, `tmp/sbs.sh`.
