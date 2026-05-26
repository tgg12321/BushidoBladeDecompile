---
name: jtbl-rodata-split-infrastructure
paths: ["asmfix.txt", "src/*.c"]
description: "jtbl-rename + delete_between asmfix rules are canonical jump-table infra, not cheats: a switch whose table splat put in asm/data rodata can't reach zero-asmfix without a global rodata reorder — park it."
metadata:
  type: reference
---

# Jump-table rename/delete asmfix rules are canonical infrastructure — park, don't grind

## The pattern

A queue item routes `C`, shows `pure-C distance 0`, and carries a cluster of
**asmfix.txt** rules (no regfix, no tier-3 pins) shaped like:

```
func: rename ".L28" "jtbl_800108CC"          # GCC auto-label -> external jtbl symbol
func: rename ".L14" ".L80035644"             # ... 14 more case-target renames
func: replace_first "^\.L80035644:$" "jlabel .L80035644"   # mark jtbl targets
func: delete_between "^\.section\s+\.rodata" "^\.text$"     # strip GCC's emitted table
```

These are **not** register/codegen cheats. They are the project's standard
mechanism for a `switch` whose jump table splat carved into the **`asm/data`
rodata split** rather than letting the C file own it. ~26 such jtbl-rename
rule-sets exist project-wide.

## Why they can't be retired to zero-asmfix (the structural proof)

The jump table (`jtbl_800108CC`, `dlabel` in `asm/data/101C.rodata.s`) lives at
rodata address `0x800108CC` in an **asm/data** rodata block — `101C.rodata_pre*.o(.rodata)`
at `bb2.ld:29-31` (and `800.rodata_pre.o` at `bb2.ld:25` is earlier still).
The function's own file (e.g. `code6cac_b2.c`) links its `.rodata` **far later**
in the same output section (`bb2.ld:50`, after code6cac, the text1a* family,
main, …). Rodata link order is listed explicitly in `bb2.ld` and is independent
of text order.

Consequence: if you drop the rules, GCC emits its own jump table into
`code6cac_b2.o(.rodata)` — which can only land at the file's **late** rodata
address, never at `0x108CC`. The external `jtbl_800108CC` (still defined in
`800.rodata_pre`) is then orphaned, and the `lw %lo(.L28)` reference points at
the wrong address. `retire` / `queue done` (full SHA1) **roll back**. The
rename-to-external-symbol + delete-GCC's-table rules are mandatory given the
split.

Verify in ~5s: `grep -n "jtbl_<addr>\|<file>.o(.rodata)" bb2.ld` — if the jtbl's
`asm/data` rodata segment is ordered *before* the function's own `.o(.rodata)`,
a pure-C in-object table is impossible.

## What pure-C would actually require (out of scope for one function)

Relocate the jtbl out of `asm/data/*.rodata.s` into the C file **and** reorder
the whole global rodata layout so the C file's `.rodata` lands at the jtbl's
address — displacing every other file's rodata. Project-wide architectural
change; needs user sign-off. Do **not** attempt it inside a single-function
Tier-4 pass.

## Action

`queue park <func> --reason "jtbl in asm/data rodata split; rodata link order
(bb2.ld) makes a GCC in-object table land at the wrong address; pure-C needs a
global rodata reorder — see [[jtbl-rodata-split-infrastructure]]"`. Then stop.

The engine's `disable all` counts these `rename`/`delete_between` rules as
cheats (no infrastructure exemption in `engine/cheats.py`), so they keep
surfacing in the queue. A systematic fix would be to teach the queue/cheat
classifier to treat jtbl-rename + delete_between asmfix rules as canonical
(like canonical-asm), so the ~26 affected functions stop blocking pure-C work.

## Bonus gotcha: the sandbox can't even score these files

`sandbox <func> --disable all` does a **whole-file** build with tier-3 inline
asm stripped. Stripping shifts maspsx instruction indices, which breaks any
**sibling** function in the same file whose regfix uses an **index-based
`reorder`** rule — `tools/regfix.py` raises `KeyError` on the now-missing index.
With no pipefail in the pipeline, the crash truncates the stream, `as` "succeeds"
on the stub, and you get a tiny object with zero symbols → `KeyError: <func> not
found in tmp/sandbox/<func>/<file>.o`. The score is unavailable through no fault
of the target function; rely on the structural analysis above, not the sandbox.

## Confirmed case — replay_camera_rob_back_loose2 (code6cac_b2.c, 2026-05-26)

Queue top, verdict C, distance 0, 24 asmfix rules (15 renames -> jtbl_800108CC +
case labels, 8 jlabel conversions, 1 delete_between). jtbl_800108CC at 0x800108CC
is a dlabel in `asm/data/101C.rodata.s` -> `101C.rodata_pre*.o` (bb2.ld:29-31);
code6cac_b2.o(.rodata) = bb2.ld:50. Sandbox
errored `KeyError: replay_camera_rob_back_loose2 not found` (sibling
`func_8003D39C` reorder rule crashed regfix after tier-3 strip). Parked.

## Related
- [[inline-asm-tiers]] — tier-1/2 canonical asm; this is the rodata analogue
- [[canonical-asm-retirement]] — when asm/infra is the legitimate end state
- [[lost-codegen-insert-cheat]] — the asmfix rules that ARE cheats (contrast)
