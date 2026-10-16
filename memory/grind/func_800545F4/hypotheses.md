# Hypothesis ledger — func_800545F4

## s1 (recon) — frontier for next session

**H1 (primary): this is pure data, not a function — try a plain C global-data form instead of
keeping COMPLETED-INLINE-ASM-CANONICAL.**
Mechanism: func_800545F4 and its 3 alabels are never a jump/call target anywhere in the tree
(only `lw`-loaded, see evidence.md s1) and have no `jr $ra`. If the original source declared
these 4 words as ordinary data (e.g. `u32` globals initialized to the exact instruction-shaped
hex values 0x3C084C80/0x35088080/0x3C084880/0x35088080, or named per the GTE-opcode constants
0x4C808080/0x48808080 depending which half the compiler would have emitted), a plain C
declaration would be an honest, non-asm decompile — strictly better than the current
whole-body-asm grant, and would move this entry from ASM-WHOLE to COMPLETED-C. NOT yet measured:
untested whether `bb2.ld`/the TU's object layout can place an ordinary C global at this exact
byte offset inside `.text`, between the neighboring functions in `src/main/3AB48.c` (splat put
it there because the ROM's `.text` section literally contains these bytes at this address — the
question is whether the linker script's section-per-TU model tolerates a `.data`-class symbol
landing inside what's otherwise a contiguous text run, or needs a precedent mechanism this repo
doesn't have yet).
Next probe: grep the tree for any existing instance of a plain C global (not `__asm__`) that
the linker places inside `.text` — if a precedent exists, replicate its mechanism here and
measure with `sandbox func_800545F4 --disable all`; if none exists, that is itself evidence H1
needs a new linker mechanism (likely out of scope for `recon`/ordinary C sessions) and H2 is the
fallback.

**H2 (fallback): reaffirm the existing grant.**
If H1's linker precedent search comes back empty, the correct re-audit outcome is to leave the
ASM-WHOLE grant standing and file the owner-mandated re-audit disposition per
`docs/COMMIT_CONVENTIONS.md`'s `auth:`/`inline_asm_canonical:` DECISIVE-evidence-block format,
adding the s1 cross-reference finding (zero call/branch references; func_80041398 already
treats these as patch templates) as a second DECISIVE signal alongside the original
`no_return_fragment` one. This does not require new codegen work, only recording the
confirmation — but per the session's mandated-modality rule, disposing of the function
(owner-gated) is only valid in `escalation` modality, so this session banks the finding rather
than filing it.

## s2 (structural) — H1 KILLED (class scope)

**H1 result: KILLED.** Searched the whole tree for any `__attribute__((section("...")))` (or
equivalent) construct that would let a plain C global land inside a `.text` run anywhere in
`src/`, `include/`, or `engine/` — zero hits, confirming the s1 next_probe's "if none exists"
branch. Went further than the probe asked and read `bb2.ld`'s SECTIONS block directly
(`bb2.ld:16-834`): it groups output in strict per-category blocks across ALL TUs — every
`*.o(.rodata)` first, then every `*.o(.text)` (`bb2.ld:116-117`), then every `*.o(.data)`
(`bb2.ld:343-344`), then `.sdata`/`.sbss`/`.bss`. An ordinary C global declared in
`src/main/3AB48.c` (no section attribute — none exists in this repo's toolchain usage) is
emitted by cc1/as into `.data` (or `.rodata` if const), which bb2.ld places at
`3AB48.o(.data)` (`bb2.ld:343`), a byte range the linker never interleaves with
`3AB48.o(.text)` (`bb2.ld:116`). There is no mechanism, with or without a GCC attribute, by
which a same-TU plain C declaration reaches the mid-`.text` address `0x800545F4` the oracle's
layout requires — the linker script's category-grouped structure forecloses it architecturally,
not just empirically. This is a CLASS kill: it applies to any ordinary C data declaration in
this TU, not one specific spelling, because the predicate is the linker script's section
grouping, not anything about the declaration's syntax.
kill_scope: class. predicate_cite: `bb2.ld:116` (text block, 3AB48.o) together with
`bb2.ld:343` (data block, 3AB48.o) — the two non-adjacent, non-interleaved placements that
make mid-.text C-data placement impossible for this TU. measured_on: no FAKE construct
involved; this is a linker-script read, not a code measurement — chassis unchanged at floor 4
(`no_c_body: true`), confirmed via the s1 sandbox baseline (unchanged, no src edits this
session).

H2 (reaffirm the grant) is now the *only* live disposition path, but this session's mandated
modality is `structural`, not `escalation` — per the standing auto-ruling this session does not
file the owner-gated disposition. The next session dispatched in `escalation` modality should
treat H1 as closed (cite this s2 kill, do not re-search for a linker precedent) and move
straight to filing the H2 DECISIVE-evidence block with both this session's and s1's findings.

## s3 (permuter) — H1 re-confirmed by direct measurement

**H1 result: KILLED (reaffirmed, instance measurement added on top of s2's class-scope
read).** Mandated modality was `permuter`, but this function has `no_c_body: true` (whole-body
INCLUDE_ASM, 4 instructions) -- there is no existing C chassis for a permuter campaign to
mutate, so the applicable probe was to actually spell out H1's candidate form and measure it,
rather than only reason about the linker script as s2 did.
Edited `src/main/3AB48.c` line 657, replacing `INCLUDE_ASM("asm/funcs", func_800545F4);` with:
```c
u32 func_800545F4 = 0x3C084C80;
u32 D_800545F8 = 0x35088080;
u32 D_800545FC = 0x3C084880;
u32 D_80054600 = 0x35088080;
```
Measured `sandbox func_800545F4 --disable all`: `{"score": null, "scorable": false, "error":
"score unavailable: 'func_800545F4 not found in tmp/sandbox/func_800545F4/main/3AB48.o'. ..."}`
(saved: `tmp/grind/func_800545F4/s3/sandbox_H1_plainC.log`). Ran `nm` on the produced
`tmp/sandbox/func_800545F4/main/3AB48.o`: all four symbols land as type `D` (.data) at offsets
0x0/0x4/0x8/0xc relative to the TU's .data block -- never inside .text. This is a direct
build-artifact confirmation of s2's architectural read: a plain C global in this TU cannot
reach the mid-.text address 0x800545F4, so the sandbox's function lookup (which requires a
.text symbol at this name) cannot even find one, producing `scorable: false` rather than a
non-zero numeric score. Reverted the edit; baseline reconfirmed at score 4
(`tmp/grind/func_800545F4/s3/sandbox_baseline_restored.log`). Rejected form banked at
`memory/grind/func_800545F4/rejected/plain_c_data_global.c`.
kill_scope: instance (this exact spelling, on this chassis, no FAKE construct present,
measured via sandbox this session -- the underlying cause is the class-scope predicate
already recorded at s2, `bb2.ld:116`/`bb2.ld:343`, but this entry's own measured_on is this
session's single build). measured_on: no FAKE construct; plain `u32` globals, no asm, no
coercion -- chassis `src/main/3AB48.c` as of this session, score result `null`/`scorable:
false` (symbol type `D` at .data offset 0x0, confirmed via `nm`).

H2 (reaffirm the ASM-WHOLE grant) remains the only live disposition path and still requires
`escalation` modality to file, per the standing auto-ruling -- this session's modality is
`permuter`, not `escalation`, so no owner-gated disposition is filed here.

**H3 (process note, not function-specific):** `tools/scan_hand_coded.py` refuses to score any
`asm/funcs/*.s` under 5 instructions. If the owner's "many game entries were auto-added
2026-05-17" re-audit sweep covers other small ASM-WHOLE grants, they will hit the same tool
floor — worth flagging to whichever session/owner runs the broader sweep, not specific to
func_800545F4's disposition.

## [s3] Replacing INCLUDE_ASM("asm/funcs", func_800545F4); in src/main/3AB48.c with four plain u32 globals (func_800545F4=0x3C084C80, D_800545F8=0x35088080, D_800545FC=0x3C084880, D_80054600=0x35088080) produces a scorable C body for this entry.
- mechanism: Plain C data declarations compile into the TU's .data section; bb2.ld places all .text blocks before all .data blocks per TU (bb2.ld:116 vs bb2.ld:343), so no same-TU plain C global can land at the mid-.text address 0x800545F4 this entry requires.
- probe: Edited src/main/3AB48.c:657 to the plain-C-global form, ran `sandbox func_800545F4 --disable all`, then `nm` on the produced tmp/sandbox/func_800545F4/main/3AB48.o, then reverted and re-measured baseline.
- result: sandbox returned {"score": null, "scorable": false, "error": "...func_800545F4 not found in tmp/sandbox/func_800545F4/main/3AB48.o..."}; nm confirmed all four symbols as type D (.data) at offsets 0x0/0x4/0x8/0xc, never in .text. Reverted; baseline re-measured at score 4 (no_c_body: true), confirming the floor is unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: this exact u32-globals spelling, on the src/main/3AB48.c chassis as of this session, no FAKE construct present; sandbox output saved to tmp/grind/func_800545F4/s3/sandbox_H1_plainC.log
- predicate_cite: bb2.ld:116
