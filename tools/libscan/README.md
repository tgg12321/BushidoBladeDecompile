# `tools/libscan` — recovering Sony's own symbol names from verbatim-linked PsyQ modules

BB2 links stock PsyQ 4.0 library modules. Where a module was linked **unchanged**, the
shipped executable contains that module's `.text` *bit-for-bit*, and the module's OBJ
record still carries the names Sony's own linker exported. This directory recovers those
names and turns them into a decision-ready rename manifest.

Unlike every other naming source in this project (Kengo cross-matching, the naming
analyzer, family-prefix heuristics) this evidence is **dispositive rather than
inferential**: the bytes either match a known module or they don't.

## Method

**1. Bit-verbatim module placement (`scan.py`).** For every module in every PsyQ `.LIB`:
parse the SN Systems `LIB\x01` container and its `LNK\x02` OBJ modules, take the module's
`.text` word sequence, and **mask the instruction fields the linker rewrites** — HI16/LO16
`imm16` (reloc type 82/84), the low 26 bits of a REL26 jump (74), and the whole word for a
FULL32 (16). An unknown reloc type masks the entire word rather than guessing. The masked
sequence is anchor-searched across `disc/SLUS_006.63`'s `.text`, then the *full* module is
verified word-for-word under the same mask. A module accepted this way is `status=verbatim`.

**2. Per-XDEF offsets (`manifest.py`).** A placement names a *span*, not a function. The OBJ's
`XDEF` records carry each exported symbol's section-relative offset, so

```
symbol vaddr = module_placement_vaddr + xdef_offset
```

gives the BB2 virtual address of each exported name. `manifest.py` cross-references those
addresses against the BB2 function universe (`asm/funcs/*.s` glabels) and every live naming
claim (`glabel`, `named_syms.txt`, `symbol_addrs.txt`, `undefined_syms_auto.txt`, the link
map, and the `docs/naming/function-names.csv` census), then classifies each in-span function.

**3. Classification.** A proposal is emitted only when the address is *both* a known function
start *and* an XDEF address:

| classification | proposal? | meaning |
|---|---|---|
| `CONTRADICTED` | yes | the **glabel itself** asserts a different name |
| `CONTRADICTED_ALIAS` | yes | glabel is auto, but a live alias asserts a different name |
| `FILL` | yes | no naming claim exists at the address |
| `CONFIRM` | no-op | already carries the Sony name |
| `MODULE_LOCAL_STATIC` | **no** | the OBJ entry is a *local*, not an XDEF. A module-local static name is not an exported claim, so per `names-require-evidence` nothing is proposed; the local name is recorded as a note |
| `AMBIGUOUS` | **no** | two code-identical modules match here, or one module matched two addresses — the bytes do not pick a name |
| `IN_SPAN_NO_SYMBOL` | **no** | inside a matched module with no OBJ symbol at its entry |

**Known limitation — mid-function XDEFs.** Nine XDEFs land *inside* an existing function
rather than at its start. These are not scan errors: they are second exported entry points in
a module whose earlier entry point splat also emitted, so splat merged both into one
`asm/funcs` file. They need a splat function-boundary correction, not a rename, and are
excluded from the manifest. Listed in `docs/naming/libscan/anomalies.txt`; follow-up plan in
`docs/naming/libscan/boundary_fixes.md`.

## Inputs

The **PsyQ 4.0 `.LIB` set is a third-party SDK input and is deliberately NOT committed.**
Point `PSYQ_LIB_DIR` at a local copy (default `tmp/libscan/psyq40/`, gitignored) to re-run
the scan. The scan's *output* (`matches.json`) **is** committed, so the manifest and every
claim resting on it can be re-derived and audited without the SDK.

## Outputs — all under `docs/naming/libscan/`

| File | What it is |
|---|---|
| `matches.json` | every module placement attempt with status, address(es), word count |
| `libsyms.json` | every XDEF / module-local placement with lib, module, offset, kind |
| `rename_manifest.csv` | the decision-ready manifest — one row per in-span function |
| `manifest_report.md` | the human-readable narrative: counts, hard-misname table, collisions |
| `anomalies.txt` | the mid-function XDEF list |
| `wave_addresses.txt` | the address restriction handed to `tools/naming_wave.py` |

These artifacts are a **pre-wave snapshot**: they record what the names were *before* the
rename wave, which is the evidence the wave rests on. Re-running `manifest.py` after the wave
re-classifies the same rows as `CONFIRM` — useful as verification, but it overwrites the
record of what was wrong. Regenerate deliberately, not by reflex.

## Regenerating

```bash
# 1. rescan the .LIB set -> docs/naming/libscan/matches.json  (needs PSYQ_LIB_DIR)
PSYQ_LIB_DIR=/path/to/psyq40 python3 tools/libscan/scan.py

# 2. re-derive the manifest from matches.json + the current tree
python3 tools/libscan/manifest.py
```

Step 2 alone refreshes the manifest after the tree changes (it re-reads the live names and the
queue) and needs the `.LIB` set only for the XDEF tables. Both scripts are read-only with
respect to the build: they touch nothing outside `docs/naming/libscan/`.

## Applying

**Never by hand.** Function names are pipeline keys. `docs/naming/build_census.py` reads
`rename_manifest.csv` as the `libscan-verbatim` origin tier (ranked VERIFIED, above
`kengo-derived` and `naming-analyzer`), and `tools/naming_wave.py` is the only sanctioned
applier — it rewrites every surface where a name is a key and proves byte-neutrality against
the oracle. It must run in a window where **no other agent is writing the repo**: a concurrent
`git reset` on main destroys a half-applied wave and leaves the tree unbuildable. See
`docs/naming/README.md`.
