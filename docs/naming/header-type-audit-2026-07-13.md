# Header type-correction candidate audit — 2026-07-13

Sweep of `include/*.h` `extern u8` / `extern u16` / `extern u32` declarations
against the four-prong test in
`.claude/rules/header-type-correction-from-use-sites.md` (owner ruling
2026-07-13, confirmed case `g_file_vram_timer` u16→s16 in commit `4598a98e`).

**This is a candidate report for OWNER REVIEW. No type flips have been applied.**
Every flip in this rule's scope needs the owner's sign-off plus a layer-2
`cheat-reviewer` that re-runs the grep + cast-necessity table against the tree
(not against this summary). Scope is **signedness only** — width flips
(`u8→u32` etc.) are explicitly out of the rule's scope.

## Method

1. Grepped every `extern (u8|u16|u32)` in `include/*.h`.
2. Grepped the whole `src/*.c` tree for compensating `(s8)`/`(s16)`/`(s32)`
   casts applied directly to global symbols (prong (b) signal).
3. Cross-referenced each cast target against its declaration (header vs.
   file-local extern) and current type.
4. For each header-declared unsigned symbol with compensating casts: full
   use-site census, value-range inspection, and `git blame` on the casts
   (prong (b) "predates the residual-chasing session" clause).

## Result summary

- **Header-declared candidates found: 1 MEDIUM, 3 LOW (one is a REJECT).**
- The u16 header symbols carry **zero** compensating signed casts → none are
  candidates (prong (b) fails outright).
- The strongest *signed-semantic* evidence in the tree lives on **file-local
  externs that are NOT in any `include/*.h`** (inconsistently declared
  `u16` in one TU and `s16` in another). These are out of the strict sweep
  scope but are the most likely genuine mistypes; listed in the "Related /
  out-of-scope" section for the owner because the natural fix is to hoist
  them into a header with the correct signed type.

---

## CANDIDATE 1 — `D_8010277C` (u8 → s8) — **MEDIUM**

- **Current decl:** `include/code6cac.h:401` — `extern u8 D_8010277C;`
  (shared header, included by every `code6cac*` TU). Decl added `f75aea097`
  2026-04-10.
- **Proposed decl:** `extern s8 D_8010277C;`

### Compensating `(s8)` scalar-read casts (would be eliminated)

| Site | Expression |
|---|---|
| `src/code6cac.c:1493` | `*(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC` |
| `src/code6cac.c:1509,1512` | `*(&D_8008E5A8 + (s8)*p)` (via the FAKE address cache) |
| `src/code6cac_b.c:3515` | `(&D_8008D538)[(s8)D_8010277C]` |
| `src/code6cac_b.c:3593` | `(&D_8008D538)[(s8)D_8010277C]` |
| `src/code6cac_b.c:3714` | `v0 = (s8)D_8010277C;` |
| `src/code6cac_b.c:3732` | `v0 = (s8)D_8010277C;` |
| `src/code6cac_b.c:3759` | `v0 = (s8)D_8010277C;` |
| `src/code6cac_b.c:3985` | `func_8003B20C((&D_8008D538)[(s8)D_8010277C])` |
| `src/code6cac_b.c:4037` | `(&D_8008D538)[(s8)D_8010277C]` |
| `src/code6cac_c_ab.c:349` | `(&D_8008D578)[(s8)(&D_8010277C)[arg0]]` |
| `src/code6cac_c_ab.c:426` | `(&D_8008D538)[(s8)D_8010277C]` |
| `src/code6cac_c_ab.c:461` | `(&D_8008D538)[(s8)D_8010277C]` |

**~12 compensating scalar casts across 3 TUs** — the single largest cast
cluster in the tree tied to one header symbol.

### Signed-specific idiom (prong (a))

Used as a **signed array index / signed pointer offset**:
`(&D_8008D538)[(s8)D_8010277C]` and `*(&D_8008E5A8 + (s8)D_8010277C)`. Under
`u8` the index is forced non-negative (0–255, always forward); the `(s8)`
cast is what lets the offset reach *before* the array base. This is the
"read into a wider signed lvalue with the implicit expectation of sign
extension" idiom the rule lists. In `code6cac_b.c:3714-3716` the sign-extended
value is chained through two more table lookups (`v0 = *(&D_8008D538 + v0);
v0 = *(&D_8008D9EC + v0);`).

### Cast necessity (prong (b)) — the load-bearing caveat

`git blame`: casts predate the 2026-07-13 session (e.g. `code6cac_b.c:3714`
= `ad013c8ee`, 2026-04-17). Timing clause **passes**.

**However**, prong (b) is defined as "removing the cast under the OLD type
changes runtime behavior on the target ABI." The scalar writes observed are
all small positive constants (`= 1, 8, 0x16, 0x1E, 0x20`) plus
`D_8010277C = D_8010277D;` and a byte-array store `*((u8*)&D_8010277C + v1) = v0`.
If the live value never sets bit 7, `(s8)x` and `x` produce **identical
runtime values**, and the casts would be behavior-neutral (prong (b) not met
on the strict runtime definition). The value range is **not provably < 0x80**
(the byte-array store can write anything), so this is inconclusive from source
alone.

The decisive evidence is the **byte-level `lb` vs `lbu`** the target emits at
these reads — the whole reason a matching decomp would carry `(s8)` on a `u8`
lvalue is to force sign-extending loads. That check requires a build/sandbox
run, which was out of scope for this read-only audit. **Owner/next agent must
confirm the target uses `lb` (not `lbu`) at these sites before flipping.**

### Prong (d) — residual sites

The scalar casts all eliminate cleanly under `s8`. But three sites view
`&D_8010277C` as the base of a **contiguous byte array** and cast the
*address*, not the scalar:

- `src/code6cac_b.c:3928` — `*((u8 *)&D_8010277C + v1) = v0;`
- `src/code6cac_c_ab.c:346` — `((s8 *)&D_8010277C)[arg0]`
- `src/code6cac_c_ab.c:431,444` / `src/code6cac.c:1502` — `u8 *p = &D_8010277C;`

These are type-agnostic array-view casts (D_8010277C is the first byte of a
per-index byte block spanning D_8010277D/E/80). They do **not** require `u8`
and do **not** break under `s8`, but they also do **not** get "eliminated,"
so the flip is a *partial* simplification rather than the whole-tree cleanup
the confirmed case exhibited. A reviewer must decide whether the array-view
casts count as "residual coercion" (they are not signedness coercion, so
likely fine, but it is murkier than `g_file_vram_timer`).

Also note a pre-existing sanctioned `/* FAKE */` block-local address cache at
`src/code6cac.c:1495-1502` — an unrelated scheduling workaround that stays
regardless of the type. It confirms this global is already a residual-chasing
hotspot.

### Prong (a) consistency — no unsigned-requiring site

No use site treats D_8010277C's *value* as unsigned: no `& 0xFF` on the
value, no `>>` needing zero-fill, no `>= 0x80` unsigned compare. Assignments
are type-agnostic. **Clean.**

### Confidence: MEDIUM

Strong cast cluster + single shared-header decl + clean prong (a)/(c), but
prong (b) hinges on the target `lb`/`lbu` opcode (unverified here) and prong
(d) is partial (address-view casts remain). Files are Grinder-owned
(`code6cac.c`, `code6cac_b.c`) — coordinate timing.

---

## CANDIDATE 2 — `D_80102784`, `D_80102785`, `D_80102786` (u8 → s8) — **LOW**

- **Current decl:** `include/code6cac.h:406-408` — `extern u8 D_80102784;` etc.
- **Proposed decl:** `extern s8 D_80102784;` etc.
- **Casts:** `src/code6cac_b.c:3947-3949` — `s8 val_85 = (s8)D_80102785;`
  `s8 val_86 = (s8)D_80102786;` `s8 val_84 = (s8)D_80102784;` (sign-extend
  into `s8`/`s32` locals). Blame `38101f313` 2026-03-28 → predate session.

### Why LOW (prong (b) fails on runtime behavior)

These are written via masked extractions:
`D_80102784 = ((u32)p[5] >> 4) & 0x3F;` (0x00–0x3F),
`D_80102786 = ((u32)p[8] >> 3) & 1;` (0/1), `D_80102786 = 1;`. The stored
values **never set bit 7**, so `(s8)x` is behaviorally identical to `x`; the
casts are behavior-neutral under the OLD type → prong (b) not satisfied on
the runtime-behavior definition. They are also accessed as a contiguous byte
array (`base = &D_80102785;` at `code6cac_b.c:4111`). Only a target `lb`/`lbu`
opcode check could justify a flip, and even then the ABI-neutrality makes this
weak. **Recommend: leave as-is unless a byte-diff proves `lb`.**

---

## CANDIDATE 3 — `D_80102787` (u8 → s8) — **LOW / REJECT (prong (b) timing)**

- **Current decl:** `include/code6cac.h:409` — `extern u8 D_80102787;`
- **Cast:** `src/code6cac_b.c:3950` — `s32 val_87 = (s8)D_80102787;`
- **`git blame`: `e83bce24e`, 2026-07-13 — the cast was added TODAY**, in the
  residual-chasing window. Prong (b)'s third abuse signal ("compensating casts
  introduced in the same session that discovered the residual") **fails by
  definition** — the cast and any proposed flip would be the same coercion,
  not evidence of one. Additionally the symbol is only ever written `= 0`
  (`code6cac.c:1349`), so the value is constant 0 and the sign cast is fully
  inert. **Do NOT flip. Recommend reviewing whether the 2026-07-13 cast itself
  is a coercion that should be reverted.**

---

## Related / out-of-strict-scope — file-local externs (strongest signed semantics, but NOT in `include/*.h`)

These symbols show the **cleanest** signed-specific idioms in the whole tree,
but they are declared as **file-local `extern`s** — and several are declared
**inconsistently** (`u16` in one TU, `s16` in another), which is itself the
"header was mistyped" smell, except there is no header to correct. They fail
the rule's prong (c) as-written (no single canonical shared-header decl). The
natural correction is to **hoist each into a header with the signed type** and
drop the per-TU externs; that is an owner decision beyond this sweep.

| Symbol | Conflicting decls | Signed idiom | Blame |
|---|---|---|---|
| `D_800A3528` | `extern s16` (text1b_b.c:713, text1b.c:15100) **vs** `extern u16` (text1b_b.c:748, text1b.c:15984) | `if ((s16)D_800A3528 < 0)`, `(s16)D_800A3528 % 3` (signed compare-against-0 + signed modulo) — `src/text1b.c:16002-16011` | `cbaa9fb39` 2026-05-15 (predates) |
| `D_800F0BC4` | `extern u16` (text1b_b.c:667, text1b.c:14546) **vs** `extern s16` (text1b_b.c:689) | `if ((s16)D_800F0BC4 < 11)` — `src/text1b.c:14772` | (predates session) |
| `D_80099478` | `extern u16` (text1a_b.c:7) **vs** `extern s16` (text1a_c2.c:8) | `(s16)(D_80099478 - 4)` — `src/text1a_b.c:17` | (predates) |
| `D_800A3310` | `extern u16` (code6cac.c:57, file-local) | `((s16)D_800A3310) >= 0xB` — `src/code6cac.c:1217` (positive compare; weaker) | (predates) |
| `D_80102808` | `extern u16` (main.c:1072, file-local) | `v = (s16)D_80102808;` — `src/main.c:1106,1112` | (predates) |

`D_800A3528` is the single most rule-shaped case in the tree (signed
compare-against-negative-adjacent `< 0` + signed `% 3`); if the owner wants a
"cleaner than D_8010277C" pilot, consolidating `D_800A3528` into a header as
`s16` is the strongest candidate — pending the same layer-2 verification and a
target-bytes confirmation.

## Symbols checked and ruled OUT (already correctly typed or not signedness)

- `D_80101ED2` — already `extern s16` (code6cac.h:302); the `(s16)` casts on
  it are **redundant** (F2 ordinary-cleanup, not a header flip).
- `D_800A36AC` — already `s32` everywhere; `(s16)(D_800A36AC << 2)` is a
  **narrowing truncation** into a u16 store target, not a signedness comp.
- `D_800A3570` — already `extern s16`; casts are truncation/re-extension of
  arithmetic results, not compensations for a wrong decl.
- `D_800A3558` — `s32`; `(s16)D_800A3558` is a **width** narrow (out of scope).
- `g_game_player_count` — `s32`; `(s16)` is a narrowing store into a u16/s16
  target (width, out of scope).
- All u16 `include/code6cac.h` symbols (`D_8008D59E`, `D_8008E3C0`,
  `D_8008EBA0`, `D_80094C68`, `D_800A36CA`, `D_800A389C`, `D_800A38C6`,
  `D_80101E9E`, `D_80102322`, `D_8010237E`) — **no compensating signed casts
  anywhere** → prong (b) fails outright, not candidates.
- Named globals (`g_file_disc_type` u8, `g_gpu_*` u8, `SpecialCam` u8,
  `g_disp_*` u8) — no compensating signed casts at any use site.

## Recommended next steps (owner)

1. For **D_8010277C**: run a sandbox/byte check on the affected reads to
   confirm the target emits `lb` (sign-extending). If yes → promote to HIGH
   and proceed to layer-2 review + owner sign-off. If `lbu` → the `(s8)` casts
   are noise; close by ordinary cast cleanup instead of a header flip.
2. For **D_80102787**: review whether today's `e83bce24e` `(s8)` cast is a
   coercion to revert (value is constant 0).
3. Consider whether the file-local inconsistent-decl cluster (esp.
   **D_800A3528**) should be hoisted into a header — the cleanest signed
   idioms live there, but they need a home before the four-prong rule applies.

---

## CANDIDATE 1 resolution — attempted 2026-07-13, layer-2 FAIL, REVERTED

The prong-(b) byte check this report requested was run: **every direct
symbol-addressed load of D_8010277C in the target disassembly is `lb`
(34/34; the 12 remaining refs are width-agnostic `sb` stores)**. On that
basis the flip was implemented (header s8 + ~12 scalar-cast eliminations;
full build SHA1 == oracle, byte-neutral) and sent to layer-2 review per the
rule's protocol.

**Layer-2 verdict: FAIL — reverted.** Two findings, both independently
verified by the reviewer against HEAD and the tree:

1. **Prong (d) fails as implemented.** The target emits `lbu` at the two
   pointer-mediated byte-copy reads in already-COMPLETED-C `func_8003B2C8` /
   `func_8003B328` (src/code6cac_c_ab.c:431,444). Under the old `u8` decl
   those sites need no cast; under `s8` they need a NEW `(u8 *)` view cast
   to keep their bytes. Net effect: ~12 scalar casts removed, 2 new pointer
   casts created — "a redistribution of the coercion, not an elimination,"
   which is the rule's named disqualifying shape.
2. **Prong (a) is not affirmatively met.** Every traceable write is < 0x80
   (sign-neutral); no use site exhibits sign-dependent program logic. The
   `lb` opcodes prove the original *expression type* was signed at scalar
   reads, but not that signedness ever mattered behaviorally — and the one
   unbounded write path (the D_801027A0-sourced table store at
   code6cac_b.c:~3928) was not bounded.

**Standing disposition: D_8010277C stays `extern u8`.** The 34-lb evidence
is preserved here for any future attempt, which per the reviewer must (a)
bound the runtime value range of the table-store write path, and (b) achieve
a whole-tree cast elimination that does not touch the two lbu byte-copy
wrappers. The existing `(s8)` scalar casts remain byte-load-bearing and
correct as-is.

Also confirmed during this pass: `D_80102787`'s same-day `(s8)` cast
(`e83bce24`, Grinder Match func_800343F0) is byte-load-bearing (target
emits `lb` at its read) — it is a match-required spelling, not gratuitous
residual-chasing; no revert warranted.
