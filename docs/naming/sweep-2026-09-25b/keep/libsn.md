# libsn vein — examined, no change (2026-09-25)

- 0x800A3668 D_800A3668 — SNMAIN .sbss+0x0 (the $ra save slot _start writes before `jal InitHeap`); the OBJ has no XDEF/LOCAL for it, so there's no Sony name. Stays auto.
- 0x80016514 pcdrv_LoadFile / 0x800165F8 pcdrv_LoadSectors — game code in src/ings.c. No LIBSN module places there, so LIBSN has no competing name. Their callees PCopen/PClseek/PCread/PCclose are now libscan-verbatim, which makes the restatement stronger. No conflict.
- 0x8008D070 g_data_start — SNMAIN's secstart:.ctors/.dtors and the `__data` value. That is a linker section boundary, not a symbol, so there's nothing to rename. "start of .data" is accurate.
- 0x800A3308 g_ang_hosei_state_pairs_3 — SNMAIN's secstart:.sbss, which is just where BSS clearing starts. A section boundary is not an identity, so this carries no evidence either way (outside this vein).
- 0x801078E0 D_801078E0 — SNMAIN's secend:.bss (BSS end / heap-base source). Not a symbol. Stays auto.
- 0x80083910 — `.L80083910` inside PCread. READ's local REL26 there is a `j` to its epilogue, not a hidden static. Nothing to name.
- 0x800A2664/0x800A2660 D_ — these belong to the LIBETC VMODE neighbour, not LIBSN (outside this vein).
