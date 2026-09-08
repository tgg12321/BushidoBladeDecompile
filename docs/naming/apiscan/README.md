# apiscan: the API-restatement naming wave (2026-09-07)

## The evidence class

`api-restatement`: the proposed name restates **what the body does at the BIOS / PsyQ
library level**, using only

1. the VERIFIED (libscan-verbatim) library entry points the body calls, and
2. literal device / format strings the body passes to them (read from `disc/SLUS_006.63`).

No game-semantic inference is allowed. `memcard_Format` is admitted because the body is
`sprintf(buf, "bu%1d%1d:", port, slot); format(buf)` and nothing else; a name like
`memcard_FormatForSaveSlot` would NOT be, because "save slot" is not in the body.

This sits below `libscan-verbatim` (byte identity with a Sony module) and above every
analyzer heuristic: the claim is checkable by reading a few lines of asm, and it cannot be
right for the wrong reason the way a Kengo size match can.

Owner bar (2026-09-07): very high confidence per row, because false-positive names have
historically cost more than auto names. Every row was re-derived by a fresh adversarial
agent (default-refute) before the wave was applied; its verdict is in the manifest.

## Files

- `rename_manifest.csv`: one row per address: current name, proposed name, the exact
  call/string chain, the verifier's verdict. `docs/naming/build_census.py` derives
  RENAME rows from it (origin `apiscan-restatement`, tier CORROBORATED: body behaviour
  affirmatively agrees with the name's claim, with a citation).
- `evidence.json`: regenerable by `tools/apiscan/mine.py`: every function's
  VERIFIED-library callee set and the strings it references.

## Regenerating the evidence

```
python3 tools/apiscan/mine.py            # writes docs/naming/apiscan/evidence.json
```

## What was deliberately left out

- `_comb_control` sub-command wrappers (`func_8003A2DC`, `func_8003A308`, `func_8003A39C`):
  the sub-command numbers' meanings are not pinned by any verified source in-tree.
- `SsVabClose` one-liners with hard-coded VAB ids (`func_8005B644/B6AC/B6FC`): a name
  would have to assert which VAB is which.
- `func_8005C2A8` / `func_8005BF78`: multi-step VAB/SPU transfer routines whose names
  would need to summarise control flow, not restate calls.
- The 8 near-verbatim LIBSND/LIBSPU identifications in
  `memory/closer/libsnd-hunt-report.md` (1-5 word diffs at exact placement): held by the
  owner's 2026-08-18 STRUCTURAL policy; an owner ruling, not wave material.
