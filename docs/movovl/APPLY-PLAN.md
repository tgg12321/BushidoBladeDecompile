# MOVOVL.EXE — apply-time build-integration plan

What a future MAIN-TREE session must add to turn the `movovl/` scaffolding
(authored 2026-08-07 on a worktree branch) into a verifying matching build.
Scaffolding status and layout: [movovl/README.md](../../movovl/README.md).

## The oracle

```
SHA1(disc/STR/MOVOVL.EXE) = a1307dbebefca0b057e02509207d00f6225e13e4   (124,928 bytes)
```

Recorded in `movovl/movovl.sha1` and in `movovl/splat.movovl.yaml` (`sha1:`).
The overlay build is DONE when header+link output byte-matches this file,
exactly as the main EXE matches `62efab4f…`.

## Steps for the apply session

1. **Source the binary**: `cp disc/STR/MOVOVL.EXE movovl/MOVOVL.EXE`
   (gitignored via `movovl/.gitignore`; verify SHA1 against `movovl/movovl.sha1`).

2. **Venv deps**: the WSL venv's splat (0.41.0) currently fails to import
   without `n64img` and `pygfxd` (spimdisasm's n64 side imports them
   unconditionally). Either `pip install n64img pygfxd` into `.venv`, or
   keep the scaffolding-session workaround (`pip install --target` to a
   scratch dir + `PYTHONPATH`). Only needed to RE-run splat; the split
   output is already committed.

3. **Makefile target** (new `movovl` target, or `movovl/Makefile` included
   from the main one — do NOT entangle with the main `all` target until the
   overlay matches). Pipeline per object, identical to the main EXE where C
   is involved:
   - `movovl/asm/*.s` (game + 59 lib modules + rodata_800 + data_C268)
     assembled with the same `mipsel-linux-gnu-as` flags as main asm.
   - Future matched C files in `movovl/src/` go through the full
     `cpp | cc1 (-mel) | maspsx --aspsx-version=2.34 | as` pipeline —
     same SDK snapshot (CVS tags identical to main), so the same toolchain
     settings are the right default. Note: main-EXE per-function pipeline
     gates (`maspsx_label_nop_funcs.txt`, `expand_lb_funcs.txt`
     etc.) are main-EXE state; the overlay starts with NO per-function gates,
     and per the completion standard — pure C or authorized asm only.
   - Link with `mipsel-linux-gnu-ld -T movovl/movovl.ld` (splat-generated;
     regenerate or hand-adjust as sections move from asm to C — the main
     project's bb2.ld hand-maintenance lesson applies once C sub-TUs exist).
   - `objcopy -O binary` → raw image (must be exactly 0x1E000 bytes; the
     header declares text size 0x1E000 and no data/bss segments).
   - `python3 tools/make_psexe.py disc/STR/MOVOVL.EXE build/movovl.bin
     build/MOVOVL.EXE` — reuses the original 0x800 header verbatim (the tool
     copies the first 0x800 bytes from the original, so entry/gp/size fields
     are correct by construction).
   - SHA1 compare against the oracle; wire a `movovl-verify` convenience
     target mirroring the main SHA1 check.

4. **Linker script details to watch**:
   - `movovl.ld` as generated references `build/` object paths under
     `movovl/` (`base_path: .` relative to the yaml) — keep the overlay's
     build dir separate from the main `build/`.
   - vram `0x801D8800`, `$gp = 0x801F65D0`: gp-relative (`%gp_rel`)
     references in `game.s` (e.g. `func_801DA070`) need the same GP handling
     as the main build (`-G` settings consistent with how splat emitted the
     asm; the scaffold used splat defaults — revisit if `as` complains about
     gp_rel relocations).
   - BSS: the header declares none (`addr=0, size=0`); everything past
     `0x801F6800` is runtime-implicit. No `.bss` output section should land
     in the binary image.

5. **Engine integration (optional, later)**: the engine CLI pins to the main
   EXE (`build/`, `bb2.ld`, oracle SHA1). Overlay support would need a
   second build profile (target path, oracle, ld script) — out of scope for
   the first matching build; a standalone `make movovl` + SHA1 check is
   enough to start grinding the 16 game functions by hand.

6. **Queue/policy**: the 59 library modules are NOT decomp targets (see the
   policy section of `movovl/README.md`). Only the 16 game units in
   `asm/game.s` would ever enter a worklist, and only after the overlay
   build verifies end-to-end with all-asm objects (step 3 first, byte-match
   with zero C, THEN convert functions one at a time — the same
   "oracle-first" discipline as the main project).

## Known loose ends (deliberate)

- The two splat auto-splits (`func_801D9714` dead fragment,
  `func_801E2DD4` SENDPAD padding word) are cosmetic; they live inside
  committed asm and need no action.
- splat suggested rodata file splits at `0x88C` / `0xFF0` (jumptable
  alignment inside `rodata_800`) — only relevant once game rodata is
  attributed to C sub-TUs.
- The m2c drafts in `movovl/src/draft/` are NOT wired into any build and
  must not be until the all-asm build byte-matches.

## STATUS 2026-08-10 — all-asm overlay build is GREEN (byte-matching)

`make -C movovl check` passes:

```
SHA1(movovl/build/MOVOVL.EXE) = a1307dbebefca0b057e02509207d00f6225e13e4   (124,928 bytes)
```

identical to `disc/STR/MOVOVL.EXE`. The overlay now has its own oracle, so the
16 game functions in `asm/game.s` can be converted to C one at a time against a
verifying build (steps 5/6 below).

**Root cause of the earlier +16 bytes:** three splat-generated `. = ALIGN(., 16)`
statements inside the `.movovl` output section of `movovl/movovl.ld` — at the end
of the rodata, text and data sub-runs. The original overlay's sub-sections are only
4-byte aligned, so each ALIGN(16) injected padding the original does not have:

| Sub-run | ends at | ALIGN(16) pushed to | pad |
|---|---|---|---|
| rodata (`rodata_800`) | `0x801D919C` | `0x801D91A0` | +4 |
| text (through `libds_dscb`) | `0x801E3C6C` | `0x801E3C70` | +4 |
| data (`data_C268`) | `0x801F6808` | `0x801F6810` | +8 |

The first pad shifted the whole `.text` run by +4, which is why every jumptable
pointer in `rodata_800` read +4 and the first byte divergence landed at ROM `0x88D`
(inside `jtbl_801D888C`). Fix: those three statements are now `. = ALIGN(., 4)`
(a no-op at the real boundaries) — commit `movovl: fix +16-byte overlay …`.
The `__romPos`/BSS `ALIGN(…, 16)` statements outside the loaded runs were left
alone; they do not affect the image.

**`movovl/movovl.ld` is now HAND-MAINTAINED** — same lesson as `bb2.ld`. Re-running
splat regenerates it with `ALIGN(., 16)` and re-breaks the match; after any re-split,
re-apply the three ALIGN(4) edits.

Diagnostic used: `tmp/movovl_map_diff.py` (compares `movovl/build/movovl.map`
symbol addresses against `movovl/symbol_addrs.movovl.txt` and reports delta
transitions plus inter-section gaps). `tmp/` is gitignored; re-create as needed.

## First all-asm build attempt — 2026-08-10 (RED, diagnosed — superseded by the above)

`movovl/Makefile` landed (standalone `make -C movovl`; asm→ld→objcopy;
`sha1sum -c movovl.sha1` gate; the .header section is linked so objcopy
yields the full file — `tools/make_psexe.py` is NOT needed).

Result: **124,944 bytes vs 124,928 (16 extra), first divergence at ROM
0x88D** — the jumptable-alignment loose end splat flagged at 0x88C. Every
jumptable pointer reads +4 vs original (0x801DA130→...34), i.e. FOUR 4-byte
paddings are inserted in early .text/.rodata, shifting all downstream vrams.
Next debug step: diff `build/movovl.map` against the scan's placements.json
to locate the four insertion points (likely per-object section alignment —
ld SUBALIGN(2) vs 8-byte-aligned object sections, or `.align` directives at
module boundaries). The venv splat import fix (n64img, pygfxd, crunch64) is
done — splat 0.41.0 imports clean for future re-splits.

The overlay remains a scaffold: no completion claims, drafts stay unwired.
