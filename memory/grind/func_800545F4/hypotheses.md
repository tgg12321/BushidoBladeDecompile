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

**H3 (process note, not function-specific):** `tools/scan_hand_coded.py` refuses to score any
`asm/funcs/*.s` under 5 instructions. If the owner's "many game entries were auto-added
2026-05-17" re-audit sweep covers other small ASM-WHOLE grants, they will hit the same tool
floor — worth flagging to whichever session/owner runs the broader sweep, not specific to
func_800545F4's disposition.
