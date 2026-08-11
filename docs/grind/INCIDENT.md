# GRINDER CIRCUIT-BREAK — 2026-08-11 09:51

**Reason:** oracle not green after session on main

git HEAD: c0b4f121
git status:
```
 M metrics/events.jsonl

```
Last 20 log lines:
```
  ** REGRESSION func_8008C1E8 (exe)
     vram 0x8008C1F8 size 636: 155 differing word(s) vs disc/SLUS_006.63

  ** REGRESSION _comb_control (exe)
     vram 0x8008C474 size 3052: 742 differing word(s) vs disc/SLUS_006.63

  ** REGRESSION AddDrv (exe)
     vram 0x8008D060 size 16: 1 differing word(s) vs disc/SLUS_006.63

  ** REGRESSION DelDrv (exe)
     vram 0x8008D070 size 192: 43 differing word(s) vs disc/SLUS_006.63

  ** REGRESSION g_data_start (exe)
     vram 0x8008D070 size 32: 3 differing word(s) vs disc/SLUS_006.63

  ** REGRESSION g_module_func_tbl (exe)
     vram 0x8008D090 size 152: 38 differing word(s) vs disc/SLUS_006.63

A COMPLETED-C function carries zero rules, so a non-zero honest distance means its BYTES moved. Before re-grinding the function, check `--mode config`: a dropped detector grant produces the same symptom with the build untouched.

```

## RESOLVED — 2026-08-11 10:05 (coordinator)

False trip. A fresh `verify-oracle --rebuild` at the same HEAD is GREEN
(62efab4f73f992798c43e8c730aa43baa10bb4fa). Cause: the byte-fail path reverts
src but did not restore build/ with a clean rebuild, and the post-session
oracle check does not rebuild — so it read the failed retire's stale
artifacts. Two driver fixes landed: the byte-fail path now restores green
build/ (mirroring the ESCALATE path), and the post-session check retries once
with --rebuild before circuit-breaking (a --rebuild red still breaks).
The banked byte-fail constraint for `main` is REAL and stands (masked-0
register diff class); the grind continues on main.
