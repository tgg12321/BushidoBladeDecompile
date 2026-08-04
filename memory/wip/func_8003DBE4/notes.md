# func_8003DBE4 — WIP (current state 2026-08-04): MATCHED, honest 0, dead array retired

## TL;DR

Honest distance **21 → 0** in one session. The forbidden `s32 buf[2];` dead
local array is **deleted**. The function now has zero cheats of any kind:
no regfix/asmfix rules, no pipeline-config entries, no pins, no asm, no dead
code. Full build SHA1 == oracle.

The 2026-07-06 session's conclusion — that this function is **POLICY-GRADE and
not closable in pure C** — is **refuted**. See "What the prior session got
wrong" below; it matters because a sibling cluster was escalated to the owner
on the same reasoning.

## The fix — one line

```c
i = 0;
colors = (s32 *)((u8 *)colors + (D_800A36AC & 1) * 24);

if (i < limit) {            /* was: if (limit > 0) */
    u32 rgb_mask = 0xFFFFFF;
    do { ... } while (i < limit);
}
```

`i` is already 0 at that point, so this is the ordinary rotated-`while`
idiom: guard with the same comparison the loop-back test uses. Nothing else
in the body changed.

## Why it works — the phantom slot

Target: `.frame $sp,64 # vars= 8, regs= 9/0, args= 16`. Clean C without the
lever: frame 56, `vars= 0`. Those 8 bytes are a **phantom slot** — an
unallocated pseudo handed frame space by reload's `alter_reg` at zero code
cost. Nothing in the emitted code touches it, by construction.

Two producers are documented in this codebase (mechanism write-up and the
57-instance census live in `memory/wip/func_8003D9A0/notes.md` and
`tmp/orphan_census.py`):

1. combine's orphan USE — `tools/gcc-2.7.2/combine.c:10836-10841`
   (`distribute_notes`, REG_DEAD case).
2. **a folded loop-guard compare** whose pseudo survives `reg_n_refs`
   accounting — what this function now uses.

Writing the guard as `if (i < limit)` makes it a duplicate of the do-while
exit test; the guard comparison then folds into a bare `blez $s5` and its
pseudo is left unallocated. Same producer as `func_8003D9A0`'s
`if (s2 != -1)` guard, and the same shape as the in-tree Judge-ruled
precedent `func_800400F8` (`src/config.c:543`, `s0 = 0; if (s2[0] > s0)`,
2026-07-14).

Once the frame is right the whole 21-instruction gap disappears: it was
entirely the sp adjust plus the 9 saves and 9 restores shifted by 8 bytes.

## What the prior session got wrong

The 2026-07-06 entry argued: *"objdump shows zero sw/lw touching stack offsets
0x10-0x17, and m2c reconstructs zero stack-resident locals; therefore no real
local can explain the 8 bytes, therefore only a declared-but-unreferenced local
can produce them."*

Both observations are correct and the inference is invalid. **A phantom slot is
untouched by construction** — the pseudo is never allocated, so no instruction
can reference it, and m2c (which reads only the emitted code) cannot see it
either. "No accesses" is exactly what a phantom slot looks like; it is not
evidence for a dead declaration. `memory/project/phantom-frame-slots-gcc272.md`
(2026-07-13) had already flagged this specific inference as unsound.

**Action item for the main session:** `AddTbpOfst_80047EE8` /
`InitHiraRmd_80047FBC` (`memory/wip/AddTbpOfst_80047EE8`, commit `3d7e222d`,
2026-06-14) were escalated to the owner as a **project-wide policy request** on
exactly this reasoning — a 32-byte unused-frame gap. 32 bytes is 4 phantom
slots, and the orphan census already shows multi-orphan functions in the tree
(`tslPrintScreen` vars=16, `func_8007C2A0` vars=32). That escalation should be
re-opened and re-measured before any policy change is considered.

## Measured negatives (do not re-run)

| form | vars | score |
|---|---|---|
| `if (limit > 0)` (committed form minus the dead array) | 0 | 21 |
| `if (limit >= 1)` | 0 | 21 |
| `if (i != limit)` | 0 | 22 |
| `for (; i < limit; )` | 8 | 5 |
| `while (i < limit)` | 8 | 5 |
| `if (i < limit) do {…} while (++i < limit)` | 8 | 24 |
| **`if (i < limit)` + `do {…} while (i < limit)`** | **8** | **0** |

The plain `for`/`while` forms get the phantom but lose the guard+do-while
control flow the target has — so the do-while spelling is required, not
cosmetic.

## The two pre-existing constructs are load-bearing (measured)

Both predate this session and belong to sanctioned families. I measured each
with the frame already correct, which supplies the exhaustion prong those rules
require:

- duplicated `step = base_val - arg0;` in the else-arm
  ([[duplicated-statement-into-arms]], sanctioned 2026-07-01) — removing it
  gives score **3**.
- `tmp = limit - 1;` variable reuse ([[defeat-licm-hoist-var-reuse]],
  SOTN-accepted 2026-06-02) — removing it gives score **15**. Removing both: 18.

Reviewer note: the duplicated-statement rule asks for a `/* FAKE */`
annotation; the in-tree comment reads `/* redundant: steers base_val into v0
… */`. I left the wording alone on purpose — annotation policy is an
owner/reviewer call, not the worker's.

## Rule carriers (grep-verified)

Zero. No `regfix.txt` / `asmfix.txt` rules; no entries in
`tools/prologue_config.json`, `tools/frame_fix_funcs.txt`,
`tools/delay_slot_ra_funcs.txt`, `maspsx_label_nop_funcs.txt`,
`expand_lb_funcs.txt`, `multu_funcs.txt`. There is nothing to retire — the
only cheat was the source-level array, now deleted.

## Handoff

Completion gate (main session): layer-2 `cheat-reviewer` + `queue done`.
Not run here, per instruction.

`candidate.c` in this directory is the **superseded** 2026-07-06 clean body
(score 21, `if (limit > 0)`); it is kept only as history and no longer
reflects src.

## Instruments

`tmp/frame_probe.sh` (fast per-function `.frame` / `vars=` gradient — the
cheap detector for this whole class), `tmp/fdiff.sh`, `tmp/orphan_census.py`,
`tmp/dbe4.py` (loop-guard sweep), `tmp/dbe4b.py` (load-bearing test for the
two pre-existing constructs).
