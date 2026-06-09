---
name: jtbl-rodata-split-infrastructure
paths: ["asmfix.txt", "src/*.c"]
description: "jtbl-rename + delete_between asmfix rules are canonical jump-table infra, not cheats: a switch whose table splat put in asm/data rodata can't reach zero-asmfix without a global rodata reorder — park it."
metadata:
  type: reference
---

# Jump-table rename/delete asmfix rules are canonical infrastructure — park, don't grind

> **UPDATE 2026-06-09 — rodata-cleanup project COMPLETED.** All 12
> `asm/data/*.rodata*.o(.rodata)` blocks (including the one containing
> `jtbl_800108CC`) have been retired from `bb2.ld` and their bytes
> re-attributed to C source files. The `jtbl_800108CC` symbol is now
> defined in `src/code6cac_b_rodata.c` instead of the asm/data block.
>
> The 24 asmfix rules on `replay_camera_rob_back_loose2` ARE STILL
> IN PLACE because they bridge GCC's per-function emitted jtbl (in
> `code6cac_b2.o`) to the external `jtbl_800108CC` symbol — both
> halves of that bridge still exist; only the rodata DEFINITION moved.
> The per-function decomp to remove those rules requires preventing
> GCC from emitting its own jtbl (a structural restructure of the C
> source, or canonical-asm authorization). That's still pending and
> still "park, don't grind" per the original guidance below.
>
> What's CHANGED: the "needs a global rodata reorder" caveat below
> is now historical. The reorder is DONE for the cluster level; only
> the per-function bridge teardown remains.

## The pattern

A queue item routes `C`, shows `pure-C distance 0`, and carries a cluster of
**asmfix.txt** rules (no regfix, no cheat-asm pins) shaped like:

```
func: rename ".L28" "jtbl_800108CC"          # GCC auto-label -> external jtbl symbol
func: rename ".L14" ".L80035644"             # ... 14 more case-target renames
func: replace_first "^\.L80035644:$" "jlabel .L80035644"   # mark jtbl targets
func: delete_between "^\.section\s+\.rodata" "^\.text$"     # strip GCC's emitted table
```

These are **not** register/codegen cheats. They are the mechanism for a `switch`
whose jump table splat carved into the **`asm/data` rodata split** rather than
letting the C file own it.

**Scope (measured 2026-05-26):** exactly **ONE** function currently uses this —
`replay_camera_rob_back_loose2`. Across all 165 asmfix-keyed functions, only it
carries `delete_between`-rodata / `jlabel` / `jtbl_` rules. (An earlier draft of
this doc estimated "~26 project-wide"; that was wrong — verify with
`tmp/jtbl_diag2.py` or `grep -c jtbl_ asmfix.txt`.) The mechanism is general and
could recur, so the classifier below handles the *pattern*, not a population of 26.

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

## What pure-C would actually require — EVIDENCE-BASED ATTRIBUTION ONLY

> **Important framing (user policy 2026-06-01):** the resolution path here
> is **evidence-based source-file re-attribution**, NOT a speculative
> system-wide rodata reorder. The distinction matters: speculative
> reorders to force a SHA1 match are structural cheats (same category as
> regfix offset paperwork), which the project does NOT sanction. See
> [[no-new-park-categories]]. SOTN does evidence-based re-attribution
> routinely; they do NOT do speculative reorders.

The legitimate move: investigate whether the original source's TU
boundaries differ from splat's current grouping. Evidence to gather:

- **Function ↔ jtbl adjacency.** Is the jtbl at an address adjacent to
  rodata that the SAME function (or functions in a single logical TU)
  references? If yes, those functions likely shared a .c file in the
  original source.
- **Cross-references.** Does the jtbl have references ONLY from one
  function (single owner) or from multiple? Single owner is strong
  evidence the original .c file held both.
- **Byte signatures / structure.** Does the jtbl's structure match a
  well-known pattern (e.g. a 16-entry switch dispatch generated by GCC
  from a specific switch shape)? Is its size consistent with the
  referencing function's case count?
- **Comparable matched siblings.** Have other functions in the queue
  been resolved by re-splitting their .c file? If so, what evidence
  did they have, and does this jtbl have the same shape?

If the evidence supports re-attribution: split the splat-output .c file
so the function lives in its own (or a smaller) .c that the linker can
place earlier in rodata, update `bb2.ld` to reflect the discovered TU
boundary. **The rodata layout follows the source attribution; we do not
reverse-engineer the layout to force a match.** This is the SOTN
workflow — they re-split splat groupings + re-order `bb2.ld` all the
time when evidence justifies.

If the evidence is absent or weak: keep parked. The function stays
INCOMPLETE in queue.json (its current state) under the asmfix rules
that anchor it in place; no speculative reorder.

Either way: **out of scope for a single-function completion pass.** A
re-attribution pass needs user sign-off + a multi-function impact
analysis (changing one .c file's rodata placement shifts addresses for
every later file's rodata, which can cascade through other functions'
relocations — verify on a multi-file basis, not in isolation).

## Action — now automatic

**IMPLEMENTED (2026-05-26):** `cheats.is_jtbl_infra(func)` recognizes this pattern
(asmfix-only `rename`/`replace_first`/`delete_between`, references a `jtbl_`
symbol, zero regfix), and `queue generate` routes such functions to the
`authorize` bucket (verdict `JTBL-INFRA`) instead of `active` — so they no longer
get handed to a worker. `headless_review.py` auto-confirms a park in this category.
You don't need to manually `queue park` a new one; a `queue regen` routes it.

The remaining pure-C path (relocate the table out of `asm/data/*.rodata.s` into the
C file **and** reorder the global rodata layout so the C file's `.rodata` lands at
the table's address) is a project-wide architecture change needing user sign-off.
Since it currently affects **one** function, it is **low urgency** — not a systemic
blocker.

## Bonus gotcha: the sandbox can't even score these files

`sandbox <func> --disable all` does a **whole-file** build with cheat-asm
stripped. Stripping shifts maspsx instruction indices, which breaks any
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
`func_8003D39C` reorder rule crashed regfix after cheat-asm strip). Parked.

## Related
- [[inline-asm-policy]] — canonical inline / canonical-body asm; this is the rodata analogue
- [[canonical-asm-retirement]] — when asm/infra is the legitimate end state
- [[lost-codegen-insert-cheat]] — the asmfix rules that ARE cheats (contrast)
