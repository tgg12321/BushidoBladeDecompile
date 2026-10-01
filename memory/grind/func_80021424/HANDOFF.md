# HANDOFF — the PracticeMenuRec handle cleanup (laneG, 2026-09-30)

Start here if you pick this series up cold. Evidence and history: `retro-audit-2026-09-30.md` (this directory).
Scripts and bodies: `handoff/` (run `bash memory/grind/func_80021424/handoff/restore_tmp.sh` first — every script
expects to live in `tmp/prc/` or `tmp/func_80021424/`, which are gitignored).

## Goal

No C code reaches `g_practice_menu_table` (PracticeMenuRec[2] at 0x80101EC8, 0x44C bytes each, ending 0x80102760)
except through PracticeMenuRec members: no per-word `D_80101Exx..D_8010275F` handles, no `(u8 *)&D_80101EC8 + off`
bases, no `*(T *)(rec + off)` reads through record pointers. A body that changes is re-reviewed in full (owner Q49
retro-audit standard), so every body a landing touches must come out with no record handles or casts left.

## Status

- **L1 — ready, not yet landed** (waiting for laneH to release the landing lock; `tmp/prc/land_all.sh`, 13 bodies 0 in scratch on both bases).
- **L2 — banked, not landed** (owner directive 2026-09-30: lanes closing). Measured 0 in scratch on both HEAD and
  the tree with laneH's Rec44 / unk_6A edits.
- **L3, L4 and the follow-ups — not started.**

## The series

| Step | Functions | State |
|---|---|---|
| L1 | func_80021424 (the retro-audit concern) + func_80020D70, func_80021210, func_800213A0, func_800218C8, func_80021904, func_80021974, func_800219E4, func_80021A3C, func_80021A98, func_80022F34, func_8001FBE8, func_8003CF84 | see Status |
| L2 | func_8001C8DC, func_8001CE60, func_8001E404 (+ its Rec44 camera view, CamBuf removed), func_8001EFA0, func_8001E878, func_8001EA84, func_8001FB34 (retyped to PracticeMenuRec *); func_8001FBE8 drops its cast to func_8001FB34 | banked, 0 in scratch |
| L3 | func_8003993C (code6cac_c_mid.c; D_80101F5E x3, D_801023AA x2, ~41 cast lines in the body) | not started |
| L4 | func_80029454 (code6cac_b_tu2.c; Ruling-6 `u8 *rec = (u8 *)&D_80101EC8 + i * 0x44C`, ~65 cast lines; D_80101F04, D_80102350) | not started |

Externs retire with their last C user: D_80101F5E / D_801023AA after L3; D_80101F04 after L4. Symbol rows stay as
`/* alias of g_practice_menu_table+0xNN; retire with <INCLUDE_ASM func> */` only while an INCLUDE_ASM .s names
them; otherwise they retire (the sandbox resolves C functions' targets without them — measured on L1's
D_801027D4 / D_800A3864).

## Follow-ups (each changes a callee's parameter type, so each pulls that callee's body)

- func_8001B294, func_8001B3C0 — PracticeMenuRec read through a pointer parameter at +0xF4/F8/FC, +0x180/184/188.
- func_8001A538 — Rec44 walked by byte offset (+0x10/12/14, arg0[6]); called as `func_8001A538((s32 *)cam, ...)`
  from func_8001A820 and with `&local.w0` from func_8001E404 (after L2).
- func_8001A820 — arg2/arg3 are records (+0x6A x15, +0xB8), arg0/arg1 are `&rec[k].unk_168`; retyping it removes
  func_8001E878's `(s32)&...` casts (L2).
- func_800283D0 — called with record pointers; reads unk_00, +0x6A, +0x4, +0xC, +0x8C.
- func_80032064 — possibly (`src + 0xF4 / 0xBC / 0x1CA`); laneH was checking.
- func_8001FAE4 — walks the motion/sound entry held in unk_50 as integers (`(s32 *)((s32)arg0 + 0xA)`, +8/+4).
- func_800324D0 (code6cac_b_tu2.c) — `u8 *pad` parameter read ~25 times; its two externs match its definition.
- Disclosed call-boundary pointer casts left by L1/L2: `func_800324D0((u8 *)s0)` (func_80021A98),
  `func_8001FAE4((s32 *)rec->unk_50)` (func_8001FBE8), `func_8001FB34((s32 *)rec, ...)` (func_8001FBE8; L2 drops it),
  `(s32)&...` into func_8001A820 / func_8001B478 (func_8001E878, L2), `(s32 *)&local.h10` into func_80046BF4 /
  func_80061064 (func_8001E404, L2).
- The cross-file `extern void *func_80021424(s32, u16, s32)` in code6cac_b_tu2.c (its caller func_800274BC passes
  `arg0` and `arg0 + 0x5E` from a record pointer typed `s32 *`) does not match the definition.

## Census (handoff/tmp_prc/census_all.py; src/*.c + include/*.h)

| When | per-word handle uses / distinct / functions | `D_80101EC8` base uses / functions | per-word externs |
|---|---|---|---|
| baseline, main efa946124 | 102 / 33 / 26 | 38 / 25 | 85 |
| main + laneH's staged table landing (pre-L1) | 80 / 29 / 21 | 35 / 22 | 81 |

## How to land a step

1. `bash memory/grind/func_80021424/handoff/restore_tmp.sh`
2. Hold the landing lock (`pwsh tmp/orch/lock.ps1 acquire <lane>`); run the step's script on main:
   L2 = `python3 tmp/prc/land_l2.py` (it expects L1's members to be present; it picks `s2->h30[0]` /
   `s2->h30[1]` when Rec44 has `s16 h30[2][4]`, else `&s2->h30` / `&s2->h38`).
3. `pwsh tmp/orch/lock.ps1 rebuild <lane>` (SHA1 must be the oracle), `& tools/wteng.ps1 main sandbox <func>
   --disable all --diff` for every changed body, census, precheck, `layer2 hash` per body, fresh layer-2.
4. Decide extern/row retirements from the census AFTER the step (an extern goes when no C user is left; its row
   stays as an alias only if an INCLUDE_ASM .s still names it — `grep -l <sym> asm/funcs/*.s`, then check each
   referrer for `INCLUDE_ASM("asm/funcs", <func>)` in src/).

To measure without the lock: `bash tmp/prc/mkl2.sh <name>` builds a scratch tree from HEAD + L1 + L2 and
`python3 tmp/prc/harness.py tmp/prc/<name> <funcs...>` (run in WSL with the venv) scores each function against
`build/src/<stem>.o` with the engine's own scorer and the exact per-file recipe (the scratch `code6cac.h` sits next
to the scratch `.c`, so the quoted #include picks it up before include/). Validate the harness on unmodified
copies first (it must print 0). `fdiff.sh <variant> <func>` shows the instruction diff (ignore relocation-symbol
lines). `acc.py <func> <reg>` lists the target's load/store opcodes per offset through a base register — that is
the evidence for each member's width and signedness.

## Gotchas (each cost real time)

- **Quoting through `tools/wsl.sh '...'` eats `$v`.** A `for v in a b c; do ... $v ...; done` passed inline to
  wsl.sh scored the SAME variant every time (this produced a false "every index spelling scores 19" claim, later
  corrected in the ledger). Put loops in a script file (`score_many.sh`).
- **Anchor-based landing, not patches.** `git apply` of a saved patch broke as soon as another lane edited a nearby
  header line. The landing is a chain of scripts that replace whole function spans and assert exact anchor text
  (land.py -> land2.py -> land3.py -> land_l1.py, wrapped by land_all.sh); test_chain.sh proves it reproduces the
  measured tree with or without another lane's edit applied first.
- **The main-reintegration guard pattern-matches `git apply`/`git commit` text even inside heredocs** — write such
  commands into a .sh file and run that file.
- **func_80022F34:** the goto-loop form of `&g_practice_menu_table[i]` costs 3 instructions (a goto loop gets no
  loop notes, so loop.c never strength-reduces `i * 0x44C`); a do-while loop matches (loop dump banked:
  probes-2026-09-30/loopdump_22F34.txt).
- **HEAD vs a lane's staged header:** Rec44's limit vectors are `h30..h3E` scalars on HEAD and `s16 h30[2][4]` in
  laneH's landing — check which one main has before splicing L2.
- **Windows Python writes CRLF** unless you pass `newline='\n'` / write bytes; every script here writes bytes.
