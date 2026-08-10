# `docs/naming/bios_decode` — the BIOS jumptable decode (second census evidence tier)

Promoted 2026-08-07 from `tmp/bios_decode/` (task #23 — `tmp/` is unbacked). This is the
**second independent evidence chain** for the BIOS-trampoline region of the naming census,
alongside the libscan XDEF chain (`docs/naming/libscan/`). `docs/naming/build_census.py`
reads `bios_names.csv` and appends a `BIOS jumptable cross-check` line to the evidence of
every census row whose address appears here.

## Method

The PS1 BIOS exposes its services through three jump vectors: a call reaches `0xA0`,
`0xB0` or `0xC0` with the **function index in `$t1`**, and the BIOS dispatches through the
corresponding jumptable. That index is machine fact, decodable from the trampoline's own
bytes — no inference involved.

`decode.py` scans `asm/funcs/*.s` for the three trampoline shapes:

```
(1) j 0xA0            ; li $t1, <idx>     (index in the delay slot)
(2) li $t2, 0xA0 ; jr $t2 ; li $t1, <idx>
(3) li $t1, <idx>     ; j 0xA0
```

and joins `(table, index)` against the community-standard BIOS jumptable (nocash PSX-SPX
naming), embedded in `tables.py`. `final.py` cross-joins the result with the libscan
manifest (`docs/naming/libscan/rename_manifest.csv`) to produce `bios_names.csv`.

## Result — 41 trampolines, one row each

| `status` | count | meaning |
|---|---:|---|
| `CONFIRM` | 34 | decoded index agrees with the (pre-wave) name at the address |
| `RENAME` | 3 | the name disagreed; all three corrections went into the fe40a52b wave (`FlushCache` at 0x80078FF0, `WaitEvent` at 0x8008AAC4, `DelDrv` at 0x8008D060 — indexed `B0:0x48 RemoveDevice`, applied under its XDEF name per the CONFLICT rule below) |
| `FILL` | 2 | no semantic claim existed; filled by the wave (`_card_info`, `_card_load`) |
| `CONFLICT` | 2 | the libscan XDEF name and the jumptable spec name differ **without contradiction**: the library wrapper `Exec` trampolines to `A0:0x43 DoExecute`, `SetMem` to `A0:0x9F SetMemSize`. Owner ruling 2026-08-07: the XDEF (what the original linker placed) wins; the jumptable entry is recorded so the second chain is not lost. |

The `cross_check` column records agreement against the libscan chain (`CONCUR`,
`CONCUR-ALIAS` for PsyQ-symbol-vs-spec-name spelling skew, `LIBSCAN-UNCOVERED`).

**This CSV is a pre-wave snapshot** (like the libscan manifest): `current_name` and
`libscan_action` record the tree as it stood before fe40a52b, which is the evidence the
wave rested on. All 41 trampolines carry their decoded names in the tree today.

## The retired `bios_local` proximity heuristic

The old survey's 27-item `bios_local` list was derived by *proximity* — "near the BIOS
trampolines, so probably BIOS-related". The jumptable decode showed **26 of 27 were not
even trampolines**. The heuristic is dropped; nothing may cite it. Only the decoded
`$t1` index counts as BIOS-identity evidence.

## Files

- `bios_names.csv` — the 41-row decode + cross-check table (the census input)
- `decode.py` — trampoline scanner (regenerates the raw decode from `asm/funcs/`)
- `tables.py` — the A0/B0/C0 jumptable (nocash PSX-SPX names)
- `final.py` — cross-join against the libscan manifest (note: its manifest path predates
  the promotion; point it at `docs/naming/libscan/rename_manifest.csv` when re-running)
