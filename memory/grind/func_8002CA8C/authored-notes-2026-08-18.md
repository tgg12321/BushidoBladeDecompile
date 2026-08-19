# func_8002CA8C — structure analysis + first-measurement plan

`src/code6cac_b.c:1020` · target 179 insns · 2 asmfix rules (whole-body blob, **ROBUST** `.frame`/`.end`
anchors) · 0 regfix · scanner tier=LOW 0/8 · **0 cop2 instructions** → pure C, no canonical-asm path.

Draft: `tmp/authoring/func_8002CA8C.c`. Raw m2c: `tmp/authoring/func_8002CA8C.m2c.c` (clean run, no
warnings on stderr).

Banked state: **none** (`memory/wip/func_8002CA8C/` and `memory/grind/func_8002CA8C/` do not exist).
Nothing has been tried, nothing killed.

## 1. Phases

| phase | asm range | what happens |
|---|---|---|
| prologue | 8002CA8C–8002CB10 | frame 0x60; save `$ra $fp $s0-$s7`; home `a0/a1/a2` to `0x18/0x20/0x28($sp)`; zero `0x30($sp)`; compute `id = *(s16*)(a0+4)`, `$s5 = &D_800F5F68 + id*0x1B8`, `$s2 = $s5 + 0x12`, `$fp = $s4 = id*0x108` |
| loop head | 8002CB14–8002CB34 | skip guard: proceed if `*(s16*)(a0+0x26C) != 0`, else skip when `(u32)(i-6) < 4` |
| AABB reject | 8002CB38–8002CBDC | six short-circuit tests against the scratchpad bounds; all failures cross-jumped onto one `s0 = 1` |
| dispatch | 8002CBE4–8002CCC0 | two fully duplicated arms (`a1 != 0` → `func_8002D780`, else `func_8002D320`), each doing call #1, then optionally call #2 |
| accumulate | 8002CCC4–8002CCF4 | `hitMask \|= 1<<i` from call #2, `seenMask \|= 1<<i` from call #1 |
| increment/exit | 8002CCF8–8002CD54 | five `addiu` induction updates, `slti $s3,0x16`, then store both masks and restore |

**Loop:** `do/while` with the test at the bottom and *no* entry guard — i.e. source is a plain
`for (i = 0; i < 0x16; i++)` (22 iterations) and GCC deleted the provably-true guard. Contrast with
func_80056CB8, where the guard survives; do not carry that function's `__asm__` guard trick here.

## 2. Data objects and evident layouts

### `D_800F5F68` — per-character collision-sphere table
Stride **0x1B8 per id**, 22 records of **0x14** bytes (`22 * 0x14 == 0x1B8` exactly).

| offset | type | evidence | role |
|---|---|---|---|
| +0x00 | s16 | `lh 0($s5)` | enable / has-second-test flag |
| +0x0C | u16 | `lhu -6($s2)`, `$s2 = $s5+0x12` | radius, used both as AABB slop and as call #1 arg4 |
| +0x0E | u16 | `lhu -4($s2)` | call #1 arg5 |
| +0x10 | u16 | `lhu -2($s2)` | call #2 arg4 |
| +0x12 | u16 | `lhu 0($s2)` | call #2 arg5 |

Offsets 0x02–0x0B are untouched by this function. GCC biased the base to `$s5+0x12` so all four
`lhu`s fit in negative displacements — that is a CSE artifact, **not** a source-level pointer.

### `0x1F8000A8` — scratchpad position array
Stride **0x108 per id**, 22 entries of **0xC** bytes (`22 * 0xC == 0x108`). Entry = `{s32 x, y, z}`
read at `+0x00 / +0x04 / +0x08` via the three literal bases `0x1F8000A8 / AC / B0`.

### `0x1F8002B8` — the scratchpad "query" block (`$s1`, live across the whole function)

| offset | role |
|---|---|
| +0x78 / +0x7C / +0x80 | min x / y / z |
| +0x84 / +0x88 / +0x8C | max x / y / z |
| +0xB4 | out: `seenMask` (call #1 passed) |
| +0xC4 | out: `hitMask` (call #2 passed) |

Accepted TU spelling for this base is `u8 *scr = (u8 *)0x1F8002B8;` (`src/code6cac_b.c:1576`);
accepted spelling for the register-indexed scratchpad loads is
`*(s32 *)((u8 *)0x1F8000A8 + off)` (`src/code6cac_b.c:1881-1893`), which is precisely the
`lui $at,hi / addu $at,$reg,$at / lw $x,lo($at)` shape in target.

### Frame — **do not model it**
`0x18 / 0x20 / 0x28 / 0x30` at an **8-byte stride** are spilled pseudos (MIPS `BIGGEST_ALIGNMENT`
is 64 bits), not declared locals: the three homed parameters plus the `seenMask` accumulator. The
19 spills the scanner counted are the same phenomenon. `0x10($sp)` is the 5th outgoing argument
slot. Modelling these as a struct is exactly the mistake frozen into func_80056CB8's harness.

## 3. First-measurement plan

1. **Drop the draft in verbatim, strip nothing else.** The stub at `src/code6cac_b.c:1020` is the
   only thing being replaced; the two forward declarations for `func_8002D320` / `func_8002D780`
   go above it (both are already *defined* later in the same TU at :1027 / :1229 with the exact
   signature used here — do not write new types).
2. **`sandbox func_8002CA8C --disable all`.** The current 177 is the distance of an empty function;
   any mechanically correct body should land far below it. Record the number — *that* is the real
   starting point and the session's primary deliverable.
3. **The blob is anchored on `.frame` / `.end`, so partial progress is oracle-safe and committable.**
   Verify with a full build + SHA1 anyway; commit the improved-but-incomplete C the way commit
   `2f20c593` did for E6B0/EB84/17848.
4. **Then read the residual in this order:**
   - *induction variables* first ([S3] in the draft header). Target has two (`$s7 = i*0xC`,
     `$s4 = base + i*0xC`) because the loads and the call argument spell the scratchpad address
     differently. If the count is wrong, adjust the two spellings — do **not** add walking pointers
     (the func_800393C8 lesson: hand-written pointers *add* IVs).
   - *the redundant `beqz $s0` at .L8002CCD8* ([S4]). If that single branch is the whole residual,
     switch to the duplicated-arm spelling.
   - *compare operand order* in the six AABB tests — target puts the bound on the left
     (`slt max, c-r`), which the draft already spells as `max < c - r`.
5. **Do not** open with `scan_hand_coded.py`; `_SHARED.md` §0 records tier=LOW 0/8 and the grant
   path needs STRONG. Do not open with the register-pin ladder either — this TU already carries 98
   pins file-wide and none of them are an end state.

## 4. Biggest unknowns

- Whether the duplicated dispatch arms are duplicated **in source** (draft's assumption) or emerge
  from GCC un-inlining a shared tail. Two direct `jal`s to different callees make a function
  pointer implausible.
- Whether `*(s16 *)(a0 + 0x26C)` is reloaded every iteration because it is genuinely re-read in
  source, or because `a0` was spilled and GCC could not keep the value. The draft reads it inside
  the loop, matching the `lw $t0,0x18($sp) / lh $v0,0x26C($t0)` pair.
- The exact `r` widening. `r` is `u16` and participates in `c - r` / `c + r`; target does
  `subu`/`addu` on the zero-extended `lhu` with no further masking, consistent with `u16` promoted
  to `s32`.
