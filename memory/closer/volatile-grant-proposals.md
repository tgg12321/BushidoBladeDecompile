# Volatile-grant proposals (owner-gated; audited between sessions)

Per the Closer hard rails: agents NEVER edit volatile_extern_allowlist.txt.
Each entry below carries the full two-prong evidence for an `extern volatile`
grant. The operator audits and appends ratified entries to the allowlist.

## Resolved proposals (2026-07-10 / 2026-07-11)

All 4 outstanding proposals filed in July 2026 have been ratified and applied.
Kept here as a historical audit trail; delete the section once it's stale.

| # | Symbols | Blocked functions | Resolved via | Applied commit |
|---|---|---|---|---|
| **§1** | `D_800F1B00`, `D_800F1B04`, `D_800F1AF0`, `D_800F1AF4`, `D_800F1AF8` (LIBCOMB SIO async-transfer state) | func_8008BEA4 (SioAnsyncRead), func_8008C184 (SioAnsyncWrite) | Owner-audited grant appended to volatile_extern_allowlist.txt | ab515c8e (Ruling 3 + SIO 5-symbol grant); ec557091 (adoption) |
| **§2** | `D_800A2884`, `D_800A2888`, `D_800A2D14` + `_spu_RQ` family | SpuSetKey (func_8008AAD4), spu_InitEx | Ruling 4 (ground-truth-codegen volatile class) | de188634 (grant), c80d976e (Ruling 4 correction), 2612ac5c (SpuSetKey adoption) |
| **§3** | `D_800A14D0..D_800A1500` per-member view (12 symbols, LIBCD cdread.c module state) | tslTm2LoadImage_2, saEft00Add, saEft00Add_sub, func_800826CC (CdRead), CdReadSync | Owner-audited grant | d568759c (grant), 1c00b41d (adoption) |
| **§4** | Canonical-asm content edit sanction (C114 LIBAPI header words at 0x800831D0/D4 written into bios_CdRemove_A0's authorized asm block) | func_80082D34 (trapIntr module splice) | Owner ruling — canonical-body content edits are owner-gated but sanctioned once justified | d7ee13b8 (LIBETC INTR module close) |

## No open proposals

The proposal queue is currently empty. New volatile-grant needs go here with
full two-prong evidence (IRQ writer citation + use-site shape).
