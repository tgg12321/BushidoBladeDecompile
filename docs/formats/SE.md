# SE — Sound-effect / instrument bundle

Files: `disc/LOADSE/JADGE.SE`, `NINJA_C.SE`, `OTHER_C.SE`, `SYSTEM.SE` (4
files).

The `.SE` files are top-level instances of the **same container format used
by individual records inside `STAGE\*.BIN`** — see
`docs/formats/STAGE_BIN.md` section "Record layout" for the byte spec, and
`docs/formats/BNK.md` for the embedded VAB header layout.

## Layout summary

```
+0x00  u32   sub_toc_count  number of u32s in the per-record sub-TOC
+0x04  u32   vh_offset      absolute offset to the embedded VH (pBAV)
+0x08  u32   aux_off1       offset to a second resource block
+0x0C  u32   aux_off2       offset to a third resource block
+0x10  u32[] vram_dest_addrs absolute VRAM destinations for texture pages
                            (count = sub_toc_count - 4)
+vh_offset   VAB VH header (pBAV magic), followed by VAB body content
```

## Verified contents

| File        | size    | sub_toc_count | vh_offset | VH programs / tones / VAGs |
|-------------|---------|---------------|-----------|----------------------------|
| `SYSTEM.SE` |  27,260 | 0x10          | 0x24      | 2 / 5 / 6                  |
| `NINJA_C.SE`|  60,312 | 0x10          | 0x50      | 2 / 13 / 8                 |
| `JADGE.SE`  | 163,204 | 0x10          | 0x5C      | 3 / 19 / 20                |
| `OTHER_C.SE`| 167,672 | 0x10          | 0x80      | 3 / 32 / 18                |

(Note: SYSTEM.SE's `sub_toc_count` is 0x10 but the actual sub-TOC overlaps
into VH territory — the file uses fewer auxiliary VRAM destination addresses
than the others; the count is the requested allocation, not the used.)

## Naming convention

* `SYSTEM.SE` — system / UI sound effects (menus, options).
* `NINJA_C.SE` — character-specific SFX for the ninja class.
* `OTHER_C.SE` — character-specific SFX for other classes.
* `JADGE.SE` — "Judge" sound effects (likely round-end / referee voice).

## Inspector

Use `tools/inspect_bnk.py` (auto-detects SE container vs raw BNK body) or
`tools/inspect_stage.py` (same container format).

## Verified by

* All 4 `.SE` files in `disc/LOADSE/` have a sub-TOC at the very start whose
  word[1] points exactly at the bytes `pBAV` (verified by exhaustive search
  of the magic in each file — exactly one hit per file at the predicted
  offset).
* VH at the predicted offset parses successfully under the standard VAB-VH
  layout (`docs/formats/BNK.md`), yielding sensible program / tone / VAG
  counts.
* The container layout matches record 0 of every `STAGE\*.BIN` — same first
  16-u32 sub-TOC convention.
