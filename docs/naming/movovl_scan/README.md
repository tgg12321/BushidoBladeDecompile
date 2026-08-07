# MOVOVL.EXE PsyQ library scan — evidence archive (2026-08-07)

First-ever scan of `disc/STR/MOVOVL.EXE` (the FMV/MDEC playback overlay,
122,880 bytes, loads at `0x801D8800`) for verbatim-linked PsyQ library
modules — the overlay counterpart of the main-EXE libscan whose evidence
lives in [`../libscan/`](../libscan/). Archived from `tmp/movovl_scan/`
per the 2026-08-07 handoff (task #22): `tmp/` is gitignored and unbacked,
and these results justify future naming claims, so they need history.

## Headline results

- **91.2% of the overlay text is verbatim Sony library code** — 59 modules
  across 7 libraries; libpress confirmed (DecDCT* XDEFs), plus LIBC2,
  LIBAPI, LIBCD, LIBETC, LIBGPU, LIBSN.
- **Game code is only 3,832 bytes = 11 functions + 3 stubs.** A full decomp
  of the overlay is a SMALL job if ever wanted.
- **264-symbol overlay map** with virtual addresses in `symbols.txt`.

## Files

| File | What |
|---|---|
| `symbols.txt` | the 264-symbol map: vaddr → lib/module → XDEF symbol(s) |
| `matches.json` | per-module verbatim-match records (the auditable evidence, same schema idea as `../libscan/matches.json`) |
| `placements.json` | module placement layout over the overlay text |
| `gaps.txt` | unmatched text regions (the game-code residue) |
| `scan_movovl.py` | the scanner — adapted from the main-EXE libscan (`tools/libscan/`); masked-word anchor search, 100%-of-unmasked-bits verification, XDEF extraction at overlay vaddrs |
| `analyze_movovl.py`, `boundary.py`, `characterize.py`, `psyq_lib.py` | analysis/support scripts used in the session |

The PsyQ `.LIB` archives themselves are NOT committed (same policy as the
main libscan). The scripts take the lib directory as an argument and are
archived for reproducibility, not wired into any pipeline.

## Method

Same as the main-EXE libscan (see `../libscan/` and
`docs/naming/README.md`): parse each `.LIB` module into a masked `.text`
word sequence (relocation-affected fields masked out), anchor-search the
overlay text, and accept only on verification of 100% of the unmasked
bits — so every name here is a bit-verbatim claim, not a heuristic.
