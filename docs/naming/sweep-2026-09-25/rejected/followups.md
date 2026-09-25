# followups — examined and dropped (2026-09-25)

| addr | name | idea | why dropped |
|---|---|---|---|
| 0x8005C6D0 | func_8005C6D0 | RENAME `snd_FlushKeyOnTable` (api-restatement) | The key verb rests on SsUtKeyOnV, which is CORROBORATED libscan-near and can't be caller-pinned (its only caller is this function). "Flush…Table" also summarises control flow. Full gap in keep.md. |
| 0x8005C6D0 | func_8005C6D0 | literal alternatives (`snd_KeyOnSlots`, …) | Same blocker: every candidate verb is SsUtKeyOnV. |
| 0x800526A0 | math_sqrt | RENAME `math_SquareRoot0` | The results are bit-identical to libgte SquareRoot0, but this is a separate copy with its own 256-entry table. The name would read as Sony's function. Kept `math_sqrt` (UPGRADE). |
| 0x800526A0 | math_sqrt | claim "exact isqrt" (09-24 README) | False. Exhaustive bucket analysis gives result − isqrt(x) ∈ [−260, 0], and the relative error reaches 5.9% (x=289 → 16). Yesterday's sqrtcheck.py counted a result as good within max(1, 1%). The row states the true bound. |
| 0x8005B72C / B868 / B9C4 | obj_InitAll / InitPair / InitTask | RENAME via api-restatement (`snd_CloseVab9`, `snd_CloseVabs8And4`, `snd_ResetVabsAndReverb`) | Rejected. apiscan deliberately left the SsVabClose-with-hard-coded-id one-liners (func_8005B644/B6AC/B6FC) auto-named (apiscan/README.md:42-43). obj_InitAll also has an INFERRED tail callee (obj_InitChars) and an AUTO head callee (func_800858D0). RESET only. |
| 0x8005B5AC | obj_InitChars | RESET | Out of my item list (it's in the sys_snd_io vein's targets). The finding for that miner: the body is func_800858D0(0) plus a 24-slot loop that zeroes D_800EFB78[i].ptr, sets bytes +4/+5 = 0x7F, and calls func_80086130(i,0,0) (PROBABLE SsUtSetVVol). That's all sound state, so `obj_` is contradicted the same way. |
| 0x80052754 | decbs0_helper | RESET / `math_SumSquares3D` | Out of my items (gte vein). Note for that miner: the body is identical to func_80052720 up to the sum, then `jr ra`, so it returns x²+y²+z² of the (s16) args. Nothing MDEC/"decbs" in reach. |
| 0x80088BA0 | g_snd_irq_data (alias) | separate RESET row | Folded into the _spu_FiDMA UPGRADE row notes (retire the alias when the census row is fixed; the live C use at src/main.c:2115/2122 has to switch to `_spu_FiDMA`). |
| other split XDEFs (SsStart2, SsSeqCalledTbyT, SpuRGetAllKeysStatus, _SendPAD, longjmp) | — | census UPGRADE like item 3 | None of them has a census row under the Sony name. They're still merged into their host functions (boundary_fixes.md), so there's nothing to upgrade. |

## Item 5 — libcd cdread.c statics: why no rows (exact missing input)

- **What's needed:** the PsyQ 4.0 `LIBCD.LIB`, module `CDREAD`. It's the same SDK set `tools/libscan/scan.py` read to produce `matches.json`, where CDREAD is verbatim, 543 words, single placement 0x80082050.
- **Where the tools look:** `PSYQ_LIB_DIR`, default `tmp/libscan/psyq40/` (`tools/libscan/manifest.py:30`). That directory doesn't exist, and `tmp/libsnd_hunt/` is gone too (yesterday's lib_followups vein noted the same).
- **Where I searched:**
  - `find` over `C:\Users\Trenton` (depth 7), `C:\GameDev`, `C:\tmp`, `C:\temp`, `C:\c`, `C:\mnt` and `C:\New Folder` (depth 8), for `LIBCD.LIB` / `CDREAD.OBJ` / `*psy*` `*.LIB`.
  - `wsl find / -xdev`, same patterns.
  - Nothing found.
- **Near-misses that don't help:**
  - `tmp/psyz-ref/decomp/sdk/` holds only `psyq400.tar.gz.sha256` (532320403f0da881e15d537c74e2adb3a3ad9c0153bbd8d27f435b4ae9731b4b) and `Psy-Q_47.zip.sha256`. Their Makefile fetches `https://github.com/psyqz/psyz/releases/download/requirements/psyq400.tar.gz`, which I did not download. That's a third-party SDK and an external fetch, so it's the owner's call.
  - psyz's own symbol lists (`symbols.400.txt`, `symbols.400.bss.txt`) name the CDREAD **text** statics (cb_read, cb_data, cd_read_retry) but **no** CDREAD data statics. Its splat400.yaml has a CDREAD `data` segment at ROM 0xB56C0..0xB5700 (0x40 bytes) with no names.
  - `tmp/psyq-decomp-ref/psy-q/` is empty.
- **Ready once the LIB is present:** `tmp/naming_sweep2/followups/cdread/cdread_locals.py`. It's a new script. It imports `tools/libscan/psyq_lib.py` read-only and doesn't modify `manifest.py`.
  - Usage: `PSYQ_LIB_DIR=<dir with LIBCD.LIB> python3 cdread_locals.py`.
  - It prints every section, XDEF and LOCAL_SYMBOL.
  - It recovers each non-text section's load address from CDREAD's own `.text` HI16/LO16 (and gp-rel) relocations. It reads the EXE words at placement+offset and checks that all pairs agree.
  - It then maps every non-text LOCAL to `base + offset`.
  - It compiles, and exits with a MISSING INPUT message today. It hasn't been run against a real OBJ, so check the reloc-type numbers (82/84 HI/LO as in scan.py; 100/102 assumed for gp-rel) on the first real run.
- **Caveat for whoever runs it:** Sony's `.data` statics carry LOCAL_SYMBOL records only if the OBJ kept them. The cb_read/cb_data precedent proves this for `.text`, not `.data`. If CDREAD has no data LOCALs, the block stays `D_` for good. g_CdReadCallback_func / g_CdReadMode_value (CONFIRMED 09-24) would then remain the conservative names.

## .bss statics follow-up (2026-09-25)

| addr | idea | why dropped / downgraded |
|---|---|---|
| 0x800F1848 | `n` as a bare global | Held (MEDIUM). A one-letter global collides with 83 local `n` identifiers and can't be grepped. The identity itself is proven (see candidates_bss.csv). |
| 0x800F1858 / 0x800F187C | bare `p0` / `p1` (drop GCC's `.87`/`.88`) | Rejected. `p0`/`p1` are local/param names 28x/108x in src (plus gte.h macro params), and dropping the suffix is our own invention. `p0_dot_87`/`p1_dot_88` (psyz spelling) are MEDIUM, held for an owner ruling. |
| 0x800F1858 | keep `g_stdout_column_counter_plus_8_800F1858` as a member alias | Rejected. It's a different module's object (LIBGPU/SYS p0.87, not LIBC2/PUTCHAR column), so it goes to RESET HIGH. |
| 0x800F185C..6C | re-point `g_gpu_color_table_field_*` to ctlbuf members | Rejected. They sit below ctlbuf (0x800F189C) inside p0.87, so they go to RESET HIGH. |
| 0x800F19B8 | keep `g_vsync_timeout_deadline` beside `Alarm` | Retire as superseded (noted in the Alarm row). Its description of Alarm.time is accurate, so it isn't contradicted, but it's a second name at one address. |
| old g_ names at patch0 / column / n / Result | RESET as contradicted | Not contradicted: each describes the object roughly correctly (INT_RP record, column counter, LCG state, CD result buffers). They're superseded by the Sony names, not reset. |
| LIBMCRD ev0 and the other MCRD locals | map to addresses | LIBMCRD isn't linked (no anchor placement; no XDEF names in the tree). |
| other 4.0 .bss locals in no-match modules (LIBDS Result, ISO9660 file/dire/load_buf, CDPLAY loc/ntoc, FONT/KPRINTF, TMD, GS …) | map | Out of scope: their modules aren't verbatim-placed, so there's no text placement to recover a .bss base from. |
