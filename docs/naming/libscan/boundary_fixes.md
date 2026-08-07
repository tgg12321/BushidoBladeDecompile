# Mid-function XDEFs — a splat boundary follow-up, not a naming one

Nine PsyQ `XDEF` symbols recovered by `tools/libscan` land **inside** an existing
`asm/funcs` function rather than at its start. They were **excluded from the 2026-08-07
libscan naming wave** and are recorded here as separate work.

## What they are

They are not scan or parse errors. Each is a **second exported entry point** in a library
module whose *earlier* entry point splat also emitted. splat's function detection merged the
two into a single `asm/funcs/*.s` file, so the second entry point has no function of its own
to name. Two were verified by hand against the disassembly:

- `func_80078FF0` is `FlushCache` — four words, ending `jr $t2` / `nop` at `0x80078FFC`. The
  `_SendPAD` XDEF at `0x80079000` sits immediately *after* that return, at its module's base.
- `func_80083220` is `setjmp`, ending `jr $ra` at `0x80083254`. The `longjmp` XDEF at
  `0x8008325C` likewise follows a real return.

So in every case the XDEF marks a genuine function boundary that splat missed. The correct
fix is a **function-boundary correction** so each becomes its own `asm/funcs` function, after
which the name follows for free.

## The nine

| XDEF | address | lib/module | currently merged into | host span |
|---|---|---|---|---|
| `_SendPAD` | `0x80079000` | LIBAPI/SENDPAD | `func_80078FF0` | `0x80078FF0`–`0x80079028` |
| `longjmp` | `0x8008325C` | LIBC2/SETJMP | `func_80083220` | `0x80083220`–`0x800832A0` |
| `SsStart2` | `0x80083EBC` | LIBSND/SSSTART | `func_80083E9C` | `0x80083E9C`–`0x800841E0` |
| `SsSeqCalledTbyT` | `0x80083F6C` | LIBSND/SSCALL | `func_80083E9C` | `0x80083E9C`–`0x800841E0` |
| `note2pitch` | `0x80086B38` | LIBSND/VM_N2P | `func_80086818` | `0x80086818`–`0x80086BFC` |
| `_spu_FiDMA` | `0x80088BA0` | LIBSPU/SPU | `func_800889D4` | `0x800889D4`–`0x80088D0C` |
| `_spu_Fr_` | `0x80088C60` | LIBSPU/SPU | `func_800889D4` | `0x800889D4`–`0x80088D0C` |
| `SpuRGetAllKeysStatus` | `0x8008B330` | LIBSPU/SR_GAKS | `func_8008AF9C` | `0x8008AF9C`–`0x8008B400` |
| `_spu_2pitch` | `0x8008BA94` | LIBSPU/S_N2P | `func_8008B488` | `0x8008B488`–`0x8008BB24` |

Machine-readable source: `anomalies.txt` in this directory.

## Why this is not wave material

A rename moves symbols. Splitting a function moves **build inputs**: a new `asm/funcs/*.s`
file, a new `INCLUDE_ASM` line (or a `bb2.ld` object entry), and a re-derived function
universe that the census, the queue and every per-function ledger key on. `tools/naming_wave.py`
has no concept of creating a function, and `make setup` is forbidden here — `bb2.ld` is
hand-maintained (CLAUDE.md standing warning), so the re-split has to be done surgically
rather than by re-running splat.

## Two names the wave DOES apply, whose span is larger than the name

Distinct from the nine above: at these two addresses the **entry point** is correctly named
and the rename lands, but the `asm/funcs` function splat produced is *longer* than the thing
being named. Recorded here so the future re-split knows the name covers only the first few
words. Confirmed against the disassembly (BIOS index decode concurs, `tmp/bios_decode`).

**`0x80078FF0` -> `FlushCache`** (LIBAPI/C68, BIOS `A0:0x44`). The function is 14 words. The
first four are the trampoline — `addiu $t2,$zero,0xA0` / `jr $t2` / `addiu $t1,$zero,0x44` /
`nop` — and that is all of FlushCache. The remaining ten words from `0x80079000` are a
**separate, stack-framed body** (`addiu $sp,$sp,-0x18` … `jalr $t1` through `D_800A362C`):
that is `_SendPAD`, the mid-function XDEF listed in the table above. So after the wave the
name `FlushCache` sits on a file that also contains `_SendPAD`. The rename is still right —
the linker's symbol at `0x80078FF0` was `FlushCache` — but the file is two functions.

**`0x8008D060` -> `DelDrv`** (LIBAPI/A72, BIOS `B0:0x48`, indexed as `RemoveDevice`). Same
trampoline shape, three instructions plus a `nop`; everything from `0x8008D070` is the
all-zero padding blob. Lower risk than FlushCache — the tail is padding, not a second
function — but the split boundary is still wrong.

Neither was excluded from the wave. Excluding them would leave two functions carrying an
auto or invented name on the strength of a splat defect this document already tracks, and
`AddDrv` at `0x8008D050` is the same trampoline family with no such objection.

## Interactions to check before doing this

- **`func_80083220` is already `setjmp` after the naming wave**, and `setjmp` holds an
  `inline_asm_canonical.txt` authorization (`sp_manipulation`). Splitting `longjmp` out of it
  changes what that authorization covers, so the authorization must be re-derived for both
  halves rather than inherited.
- **`func_800889D4` and `func_8008AF9C` are live `engine/queue.json` items.** Splitting a
  queued function changes a worklist key and the honest distance of both halves; do it in a
  grinder-quiet window and regenerate the queue afterwards.
- Each split must be **oracle-verified** (`build` SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`),
  since it moves code between translation units even though no byte should change.
