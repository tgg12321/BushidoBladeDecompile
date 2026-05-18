# Project Status

Live status numbers and a brief snapshot of the state of the matching decompilation. The numbers in this file are pulled by hand from the working tree at a point in time — for the always-current canonical work list, see [`WORK_QUEUE.md`](../WORK_QUEUE.md).

To regenerate the headline counts at any time, run the snippet at the bottom of this document.

## Snapshot — 2026-05-16

### Build

| | |
|---|---|
| Branch | `main` |
| Last commit author date | 2026-05-16 |
| Build SHA1 | matches original `disc/SLUS_006.63` |
| Active marker (`.bb2_active_func`) | function in progress on main: `func_80077B30` |

### Function inventory

| | Count |
|---|------:|
| Total identified functions | 1,410 |
| `asm/funcs/<name>.s` files (one per function, some now orphaned) | 1,433 |
| C source files (`src/*.c`) | 21 |
| C source line count (total) | ~46,892 |
| Function definitions in C source | ~1,516 (includes static helpers that may not be in the asm count) |
| Functions with semantic names (across src/ + asm/funcs/) | ~526 |
| Functions still named `func_<addr>` | ~880 |
| Bridged functions (`replace_with_asmfile` in asmfix.txt) | ~148 |
| Authorized canonical inline-asm functions (`inline_asm_canonical.txt`) | ~80 |

Source-file size distribution (in lines):

| File | Lines | Approximate VRAM range |
|---|---:|---|
| `text1b.c` | 18,566 | `0x80047ED0`–`0x8007A28C` |
| `code6cac_b.c` | 4,247 | `0x80026DA4`–`0x80035438` |
| `display.c` | 3,494 | `0x8007B244`–`0x8008008C` |
| `code6cac.c` | 3,319 | `0x80017FA0`–`0x80026DA4` |
| `main.c` | 3,095 | `0x80083BE4`–`0x8008D120` |
| `text1a_c.c` | 1,949 | `0x80042504`–`0x800460E4` |
| `code6cac_c_mid.c` | 1,855 | `0x80037F08`–`0x8003AB44` |
| `code6cac_c2.c` | 1,686 | `0x8003B9D0`–`0x8003F168` |
| `text1a.c` | 1,446 | `0x800401CC`–`0x80042504` |
| `system.c` | 1,123 | `0x8008008C`–`0x8008289C` |
| `sound.c` | 1,015 | `0x80046780`–`0x80047ED0` |
| `ings.c` | 962 | `0x800164AC`–`0x80017FA0` |
| `ings2.c` | 746 | `0x8008289C`–`0x80083BE4` |
| `code6cac_b2.c` | 690 | `0x80035438`–`0x800375EC` |
| `gpu.c` | 678 | `0x8007A28C`–`0x8007B244` |
| `code6cac_c_ab.c` | 648 | `0x8003AB44`–`0x8003B9D0` |
| `config.c` | 575 | `0x8003F168`–`0x800401CC` |
| `code6cac_c.c` | 413 | `0x800375EC`–`0x80037D14` |
| `text1a_c2.c` | 245 | `0x800460E4`–`0x800466C0` |
| `code6cac_c0.c` | 108 | `0x80037D14`–`0x80037F08` |
| `text1a_b.c` | 32 | `0x800466C0`–`0x80046780` |

For finer detail on what lives inside each file, see [`SUBSYSTEM_MAP_2026-05-12.md`](handoffs/2026-05-12-subsystem-map.md).

### Work queue (from `WORK_QUEUE.md`)

| Queue | Count |
|---|---:|
| Active decomp queue | 62 |
| Structural split queue | 1 |
| Asmfix retirement queue | 147 |
| Permanent out-of-scope | 7 |
| **Total active / deferred** | **63** |
| **Total pure-C / asmfix-retirement backlog** | **210** |

The active decomp queue is currently dominated by GTE-using functions in `text1b.c` that need their inline-asm bodies converted to mostly-C with narrowed inline blocks for the cop2 ops.

### Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 484 |
| Last 1 month | 774 |
| Total since 2026-03-23 | 1,381 |

Most recent commits (`git log --oneline -5`):

```
d75a5d6 auto_drift_repair: rollback on no-improvement + detect maspsx-set-stripping
8f903a0 text1b: authorize func_800526A0 canonical inline-asm (GTE LZCS/LZCR primitive)
9a1c378 auto-repair: detect SHA1-silent pair-shift drift via MISMATCH line
cee3dda housekeeping: refresh queue after func_8004C388 match
6fd1679 text1b: match func_8004C388 (xyz+packed-rgb midpoint averager, ...)
```

## Health indicators

These are the project's standing technical debt items that the 1.0 release criteria will close out:

| Item | State | Detail |
|---|---|---|
| Bridged functions (`replace_with_asmfile`) | ~148 active | Each one is a function whose C body is dead code; the build substitutes raw hand-disassembled MIPS. Retirement is in flight via `dc.sh next-asmfix`. |
| Inline-asm debt (`inline_asm_debt`) | scanned live | Live-scanned by `tools/gen_work_queue.py` from `src/*.c`. Suspect non-canonical inline asm patterns are tagged and re-injected into the active decomp queue. |
| `# RETIRE:` lines in asmfix.txt | 3 (as of last check) | These are commented-out bridges from functions whose retirement is half-complete — their C body matches but the bridge line is kept until `dc.sh purge-retirements` verifies and removes it. |
| Drift-fragile literal `.L<N>` rules | tracked | A `subst` rule with a hardcoded `.L347` silently breaks when a sibling function shifts GCC's per-CU label counter. The `dc.sh start` briefing reports the count; `dc.sh regfix-drift-immune --apply` rewrites the safe ones. |
| Asm-cheat patterns | tracked | Force-rewrite patterns (wildcard `subst .*`), file-scope `__asm__("glabel ...")` function bodies, and large splice rules are tracked by `tools/audit_asm_cheats.py` and surfaced in the session briefing. |

## Regenerating this snapshot

To regenerate the headline counts:

```bash
# From the project root, in WSL with the venv active:

# Function count
grep -c "^\\.glabel " asm/6CAC.s

# C source line counts
wc -l src/*.c

# Function definitions in src/
grep -hcE "^(s32|u32|s16|u16|s8|u8|void|int|char|float|short|long)[ *]+[a-zA-Z_]" src/*.c | awk '{s+=$1} END {print s}'

# Bridged function count
grep -c "^[a-zA-Z_].*: replace_with_asmfile" asmfix.txt

# Authorized canonical inline-asm function count
grep -cE "^[a-zA-Z_]" inline_asm_canonical.txt

# Work queue counts (from WORK_QUEUE.md "Backlog Summary" table)
sed -n '/Backlog Summary/,/^$/p' WORK_QUEUE.md

# Recent commit velocity
git log --oneline --since="2 weeks ago" | wc -l
git log --oneline | wc -l
```

A future enhancement would be a script under `tools/` that emits this whole document. For now, manual refresh is the convention — the numbers in this file are not load-bearing for any tooling.
