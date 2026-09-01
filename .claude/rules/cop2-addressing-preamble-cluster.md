---
name: cop2-addressing-preamble-cluster
description: "The 2026-08-17 owner CLUSTER ruling for func_8002FDB0 covers a 28-function family in the 0x8001-0x8003 band sharing the `addu $t4,$aN,$zero` + cop2 hand-asm idiom. Membership list built and verified here; each member inherits the canonical-asm disposition subject to the same mechanical per-function check, applied by the Judge without re-escalation."
paths: ["inline_asm_canonical.txt", "src/code6cac.c", "src/code6cac_b.c"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
metadata:
  type: project
---

# The `addu $t4,$aN,$zero` + cop2 cluster — membership list for the 2026-08-17 ruling

## Why this file exists

The owner's 2026-08-17 ruling on `func_8002FDB0` (docs/grind/decisions.md:5610,
authorization line at `inline_asm_canonical.txt:268`) ends with a **CLUSTER
RULING**:

> the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total
> in the 0x8001-0x8003 band) inherit this disposition subject to the same
> mechanical per-function check ... which the Judge may apply without
> re-escalation.

The ruling never enumerated the 28. Until it is enumerated, no session can tell
whether the function in front of it is a cluster member, so the grant sits
unusable. This file is the enumeration.

## The idiom, precisely

A **materialize-then-copy addressing preamble** with no consumer other than a
cop2 transfer:

```
lui   $a1, %hi(ADDR)          # or the address already in an arg register
ori   $a1, $a1, %lo(ADDR)
addu  $t4, $a1, $zero         # <-- redundant copy; GCC never copies a fresh
                              #     constant into a second register with no use
mtc2  $t4, $30                # ... or lw $t5/$t6/$t7 -> ctc2 / lwc2 / swc2
nop                           # unfilled cop2 load-delay slots
nop
```

The hand-asm signature is the redundant copy plus the unfilled delay slots plus
splat's `/* handwritten instruction */` tags — the same evidence set the owner
accepted for `func_8001A67C` and `func_800274BC` on 2026-06-10 and for
`func_8002FDB0` on 2026-08-17.

## Membership (28) — mechanically derived, verifiable

Scan: every `asm/funcs/*.s` line matching `addu $t4, $aN, $zero` whose next
6 lines contain a cop2 transfer referencing `$t4`. Exactly 28 functions in the
0x8001-0x8003 band, matching the ruling's own count. (A looser scan allowing any
destination register finds 36 across the whole binary; the 8 extra sit outside
the band the ruling scoped and are NOT covered.)

Queue state is as of 2026-08-18: **22 active, 2 parked, 4 already off the queue.**
The ruling said 26 queued; two have since left the queue.

| function | queue state | idiom sites | first site (evidence) |
|---|---|---|---|
| `func_80018094` | active, dist 103, 75 rules | 3 | L10 `addu $t4, $a2, $zero` -> `lw $t5, 0x0($t4)` ; `lw $t6, 0x4($t4)` ; `ctc2 $t5, $0` |
| `func_80018300` | active, dist 307, 0 rules | 2 | L172 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `sra $v0, $t1, 16` ; `sll $v0, $v0, 6` |
| `func_800187F4` | active, dist 644, 0 rules | 4 | L419 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8001A67C` | not queued | 1 | L49 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8001A820` | active, dist 576, 0 rules | 1 | L171 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8001F2E4` | active, dist 347, 0 rules | 2 | L79 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_800207C8` | active, dist 317, 0 rules | 10 | L39 `addu $t4, $a2, $zero` -> `lw $t5, 0x0($t4)` ; `lw $t6, 0x4($t4)` ; `ctc2 $t5, $0` |
| `func_800274BC` | not queued | 1 | L26 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_800288C8` | active, dist 465, 0 rules | 1 | L141 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002A458` | active, dist 416, 0 rules | 2 | L89 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002BC68` | not queued | 1 | L32 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002BEA0` | parked, dist 9, 7 rules | 1 | L31 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002CD58` | active, dist 352, 0 rules | 3 | L112 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002D320` | active, dist 32, 13 rules | 1 | L75 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002D518` | active, dist 33, 33 rules | 1 | L106 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002D780` | active, dist 56, 65 rules | 1 | L141 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002DAD0` | active, dist 204, 0 rules | 1 | L125 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002DE20` | active, dist 506, 0 rules | 3 | L23 `addu $t4, $a0, $zero` -> `lwc2 $0, 0x0($t4)` ; `lwc2 $1, 0x4($t4)` ; `nop` |
| `func_8002E838` | active, dist 36, 0 rules | 1 | L51 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002EA24` | parked, dist 18, 10 rules | 1 | L71 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002EBDC` | active, dist 97, 45 rules | 1 | L44 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002F2D0` | active, dist 270, 0 rules | 1 | L219 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002F770` | active, dist 298, 0 rules | 1 | L247 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_8002FC80` | active, dist 28, 4 rules | 2 | L41 `addu $t4, $a3, $zero` -> `lw $t5, 0x0($t4)` ; `lw $t6, 0x4($t4)` ; `ctc2 $t5, $0` |
| `func_8002FDB0` | not queued | 3 | L68 `addu $t4, $a1, $zero` -> `lw $t5, 0x0($t4)` ; `lw $t6, 0x4($t4)` ; `ctc2 $t5, $0` |
| `func_800300B4` | active, dist 37, 0 rules | 1 | L18 `addu $t4, $a0, $zero` -> `lw $t5, 0x0($t4)` ; `lw $t6, 0x4($t4)` ; `ctc2 $t5, $0` |
| `func_80032314` | active, dist 27, 12 rules | 1 | L76 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |
| `func_800325E0` | active, dist 37, 16 rules | 1 | L50 `addu $t4, $a0, $zero` -> `mtc2 $t4, $30` ; `nop` ; `nop` |

Sub-families, by what the preamble feeds:

- **LZCS/LZCR leading-zero-count** (21 members) — `addu $t4,$a0,$zero` ->
  `mtc2 $t4,$30` -> 2 unfilled nops -> `swc2 $31,...`. This is the shape the
  owner authorized directly for `func_8001A67C` / `func_800274BC` /
  `func_8002BC68`.
- **SetRotMatrix / long-vector transfer** (6 members: `func_80018094`,
  `func_800207C8`, `func_8002DE20`, `func_8002FC80`, `func_8002FDB0`,
  `func_800300B4`) — `addu $t4,$aN,$zero` -> `lw $t5/$t6/$t7` -> `ctc2 $0/$2/$4`,
  or `lwc2` / `swc2` triples. This is `func_8002FDB0`'s own shape.
- `func_80018300` is the one hybrid (mtc2 preamble whose successor is ordinary
  compiled arithmetic) — check it by hand rather than by family.

## The per-function mechanical check (from the ruling; nothing added)

A member inherits the disposition **only** when all of these hold. This is a
check, not a lever — it does not lower anyone's distance:

1. `sandbox <func> --disable all` == **0**;
2. **zero** register pins, zero `move %0,%1` aliasing blocks, zero scheduling
   barriers anywhere in the body;
3. in-island GPR instructions limited to the **cop2 addressing preamble** —
   nothing else may be swallowed into the template;
4. before `queue done`: fresh layer-2 `cheat-reviewer` on the applied diff **and**
   `verify-oracle --rebuild` (object-level proof is not sufficient).

**The load-bearing negative.** Most members are nowhere near condition 1 — look
at the distances: `func_800187F4` is at 644, `func_8001A820` at 576,
`func_8002DE20` at 506. The grant closes the *tail island*; it does nothing for a
600-instruction pure-C body. Membership means "when you get to 0, the island is
not what blocks you" — it is **not** a shortcut to completion, and reaching for
it early is exactly what got `func_8002EA24` refused on 2026-07-30 (nine of its
residual points had nothing to do with the GTE regions).

The near-miss members worth checking first, by distance: `func_80032314` (27),
`func_8002FC80` (28), `func_8002D320` (32), `func_8002D518` (33), `func_8002E838`
(36), `func_800300B4` (37), `func_800325E0` (37).

## Related
- [[canonical-asm-authorization-recipe]] — how the island itself is written.
- [[reload-spill-reg-reveals-asm-clobbers]] — the clobber-list question that
  arises in the same islands (and the unspent grant for `func_8002BEA0`, a
  cluster member).
- [[inline-asm-injection]] — the boundary condition 3 enforces.
- [[gte-wrapper-misroute-park]] / [[gte-3x3]] — neighbouring GTE families.
- [[judge-sole-gate]] — why no member needs a fresh escalation.

## Widened anchor (owner GRANT 2026-09-01 — supersedes the $aN scope limit)

Owner ruling 2026-09-01 (decisions.md "cop2 materialize-then-copy WIDENED
ANCHOR — OWNER GRANT"; informed ruling — the LOW scan tier, zero-hit SOTN
census and the Judge's 2026-09-01 09:01 routing FAIL were all presented before
the grant): the anchor widens from the literal `addu $t4,$aN,$zero` to the
**general materialize-then-copy cop2 addressing preamble** — the copy source
may be ANY register (observed in-band: `$v0/$v1/$s0-$s6/$t0/$t1/$sp`), same
0x8001-0x8003 band, same evidence set (redundant copy GCC 2.7.2 never emits +
unfilled cop2 load-delay slots + splat `/* handwritten instruction */` tags),
same 4-point mechanical per-function check below, unchanged.

Mechanical enumeration (2026-09-01, `tmp/scan_cop2_widened.py`: `addu $tN,$rX,
$zero` with a cop2 transfer referencing `$tN` within the next 6 lines, band
filter on the function address): **68 in-band carriers, 66 with at least one
non-`$aN`-source site** newly covered by the widening — a superset of the 28
rows above (the table's queue states are 2026-08-18-stale; the scan is the
live membership source, re-run it rather than trusting the table). Confirmed
handwritten-tagged carriers among the Judge-enumerated set: `func_80019310`,
`func_800203B4` (integrated under this grant), `func_800204C0`,
`func_8002FF20`, `func_80031890`, `func_8003E6D8`. (`func_80017FA0` reached
COMPLETED-C in pure C on 2026-08-20 and needs nothing.)

The load-bearing negative is unchanged and applies with full force to the
widened set: membership closes the TAIL ISLAND only, when the function's
pure-C body independently reaches sandbox 0 under the normal grind gates.
