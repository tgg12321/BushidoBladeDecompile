# Canonical-asm disposition sweep — owner decision packet

**Scope:** the 86 active queue items carrying an `ASM-PARTIAL` or `ASM-SUSPECT` canonical-gate
verdict. For each, `tmp/campaign1/results.json` holds the raw `engine canonical <func>` output and
the raw `tools/scan_hand_coded.py --single <func>` output. Merged working data:
`tmp/campaign1/merged.json`. Read-only analysis; no engine commands, no builds, no source edits.

**Standard applied:** [[hand-coded-asm-recognition]] (STRONG tier requires S1 uniform multu/mflo
pacing, S2 empty-body branch, or S6 BIOS jumptable), [[canonical-asm-retirement]] (STRONG signal +
own reading of the target asm + cluster corroboration), [[gte-wrapper-misroute-park]] (the
zero-ambiguity GTE-leaf carve-out: cop2 ops + *mechanical I/O packaging only*, no general-purpose
computation), and [[canonical-gate-distance-not-evidence]] (distance is never routing evidence).
Distance is reported below as context only.

---

## 1. Executive summary

| Class | Count | What it means |
|---|---|---|
| **A — AUTHORIZE-RECOMMENDED** | **1** | `func_80052930`. Canonical-asm authorization recommended. |
| **B — AMBIGUOUS** | **4** | One specific, cheap piece of evidence decides each. |
| **C — ROUTE-TO-C** | **81** | Stay pure-C grind targets. No canonical case. |

### The headline finding, stated honestly

**Zero of the 86 functions reach STRONG scanner tier.** 85 score `LOW`, one scores `TIGHT_C`
(`func_8002D320`, 3/8). Across all 86, the signals that actually fired are S4 (front-loaded loads,
64×), S3 (no spills, 11×) and S5 (cluster, 2×) — every one of which the recognition rule classifies
as a *tightness* signal that "also fires on tight pure-C clusters" and is "insufficient alone."
**S1 fired 0 times. S2 fired 0 times. S6 fired 0 times.** Under the strict autonomous-authorization
gate, nothing in this cohort qualifies on hand-coded-authorship evidence.

The gate's own definitive-signal detector agrees. Across all 86, every definitive hit is a GTE/cop2
opcode. In the target disassembly there is not one `syscall`, not one undisassembled `.word`, and
not one trapping-overflow `add`/`addi`/`sub` in the entire cohort — so no BIOS-trampoline case and
no handwritten-overflow-op case exists here. (Distinct from the `.word` *source spellings* discussed
in B3/B4 below, which are how cop2 ops are currently written in some C bodies.)

Consequently the Class A recommendation below does **not** rest on the scanner. It rests on a
different, stronger basis: a documented cluster precedent where the owner has already ruled the
family canonical, plus address adjacency to an unbroken run of already-authorized siblings.

### The two structural facts that explain most of the cohort

1. **All 39 `ASM-PARTIAL` verdicts are GTE islands, not asm functions.** The cop2 fraction is
   0.43 at the very top and 0.00–0.15 for the other 38. Thirty-seven of the 39 have calls and/or
   branches; the median is a several-hundred-instruction C function containing one to four small
   cop2 blocks. That is precisely the case [[inline-asm-allowed]] describes as *already handled*:
   "the surrounding control flow and integer arithmetic must still be C; only the cop2 ops are asm."
   An `ASM-PARTIAL` verdict is not an authorization event — it tells the worker which spans to write
   as canonical inline asm, and the rest of the function remains ordinary pure-C work.

2. **46 of the 86 have a distance equal to their full instruction count**, because no C body has been
   drafted yet (34 have no extractable body in `src/` at all; the rest are empty stubs). For these,
   distance is an artifact of unwritten code and carries no information whatsoever about authorship.
   They are the largest, least-started items in the queue — not evidence of hand-written asm.

### Expected queue impact

All 86 are currently `status=active` (none parked), so this sweep moves nothing into or out of the
`authorize` bucket by itself.

- Accepting Class A retires **1** item to `COMPLETED-INLINE-ASM-CANONICAL` — contingent on the
  prerequisite in §2 (the current body's 11 register pins and 2 cheat-asm blocks must come out
  first; the finished form is a whole-body `__asm__("glabel …")` block, not the present pin scaffold).
- Class B, whichever way each resolves, touches at most 4 more.
- **81 items (94%) stay exactly where they are: active pure-C work.** They carry 825 regfix/asmfix
  rules and 29 functions' worth of register pins between them — that is the real outstanding debt in
  this cohort, and none of it is retired by a canonical-asm ruling.

---

## 2. Class A — AUTHORIZE-RECOMMENDED (1)

| Rank | func | file | insns | cop2 | signals | distance (context only) |
|---|---|---|---|---|---|---|
| A1 | `func_80052930` | `src/text1b.c` | 60 | 26 (43%) | cluster precedent + address adjacency + no-C-form pipeline; scanner **LOW 2/8** (S3, S4) | 51, 0 rules |

### A1 — `func_80052930` @ 0x80052930 (text1b.c), 60 insns, 23 splat "handwritten instruction" tags

**Gate:** `26/60 insns canonical-asm (GTE/cop2 op (c2); GTE/cop2 op (ctc2); GTE/cop2 op (mfc2);
GTE/cop2 op (mtc2))`

**Scanner — stated honestly:** `tier=LOW score=2/8`, `Reason: no strong hand-coded indicators`.
Only S3 (`60 insns, 0 spills, 25 distinct regs`) and S4 (`5 loads in 8-insn window @ insn 0`) fired.
**This function does not clear the STRONG-tier bar and I am not claiming it does.** The scanner has
no signal for the GTE-3x3 pipeline shape; that gap is exactly what the archived `gte-3x3` rule
documents.

**The actual evidence — four independent strands:**

1. **The owner has already ruled this family canonical.**
   `.claude/rules/gte-3x3.md` is an *archived-forbidden* rule stating that the pin+lui-mask recipe
   "only worked because the recipe encoded **exactly what hand-coded asm looks like** — the recipe's
   bytes happened to equal target bytes because the original was hand-written asm," and that for this
   cluster "the honest pure-C distance … is the distance to a non-existent pure-C form." It names
   `calc_fc_frame_8007EC5C` (ASM-WHOLE, 2026-05-31) plus `func_8007E4DC` / `func_8007EB4C` and directs
   that cluster siblings "should follow the same path when they reach the queue top."

2. **`func_80052930` carries every signature that rule lists.** From `asm/funcs/func_80052930.s`:
   - Per-cycle mask re-materialization held in a hand-allocated register:
     `lui $t9, (0xFFFF0000 >> 16)` at insn 5, then `and $t5, $t1, $t9` / `and $v0, $t2, $t9` reusing it.
   - The mfc2-during-mvmva-latency interleave, three times:
     `mvmva 1, 0, 0, 0, 0` → `srl $v1, $t0, 16` / `or $v0, $v0, $v1` (next cycle's packing computed
     inside this cycle's GTE latency window) → `mfc2 $t5, $9` / `mfc2 $t6, $10` / `mfc2 $t7, $11`.
   - A store in the `jr $ra` delay slot: `jr $ra` / `sh $t7, 0x10($a2)` @ 0x80052A1C. GCC 2.7.2's
     `reorg.c` `stop_search_p` halts delay-slot search at `ASM_INPUT`, so with the cop2 ops written
     as asm this byte is unreachable from any C — the same argument the owner accepted for
     `func_80052A20` / `func_80052A88` / `func_80052B7C` on 2026-08-06.
   - Every cop2 op tagged `/* handwritten instruction */` by splat (23 tags).
   - Zero calls, zero branches, zero spills, single `jr $ra` — a pure leaf.

3. **Address adjacency to an unbroken run of already-authorized siblings.** `func_80052930` ends at
   0x80052A1C. The next function is `func_80052A20`. In `inline_asm_canonical.txt`, text1b.c already
   carries `func_80052720`, `func_80052754`, `func_80052788`, `func_80052A20`, `func_80052A88`,
   `func_80052B00`, `func_80052B44`, `func_80052B7C` — a contiguous LIBGTE matrix/vector leaf block,
   the last four owner-authorized 2026-08-06 (`docs/grind/decisions.md`, commit `f010b4b6`).
   `func_80052930` sits *inside* that run and is the only member still queued.

4. **The body is mechanical I/O packaging only** — the [[gte-wrapper-misroute-park]] boundary test.
   Every non-cop2 instruction exists solely to feed or drain the GTE registers: 10 `lw` of the matrix
   and vector, `and`/`andi`/`or`/`sll`/`srl` doing s16 hi/lo pack-unpack into the GTE input encodings,
   9 `sh` draining mfc2 outputs. There is **no general-purpose computation on the mfc2 results** — no
   sums, no comparisons, no arithmetic beyond the packing. This is the same shape the owner accepted
   for `LoadAverageShort12` / `LoadAverageShort0` ("mechanical s16 unpack/repack I/O packaging … no
   general-purpose computation on mfc2 outputs").

**Prerequisite before any authorization commit.** The current body (`src/text1b.c`) is the archived
**forbidden** gte-3x3 recipe verbatim: 11 `register … asm("$N")` pins, `__asm__ volatile ("lui %0,
0xFFFF" : "=r"(mask))`, and `__asm__ volatile ("nop")`. Those are cheat-asm and must be *removed*,
not authorized. The finished form is a whole-body `__asm__("glabel func_80052930" …)` block per
[[canonical-asm-authorization-recipe]] — including the label-shift cascade check (removing 60 insns
of C body from text1b.c will renumber `.L<N>` labels for every later function in that TU, and
text1b.c siblings are known to anchor on hardcoded `.L<N>`).

**Proposed `inline_asm_canonical.txt` line:**

```
func_80052930  # LIBGTE 3x3-mvmva matrix x s16-packed-vector transform leaf (text1b.c, 60 insns, 23 splat 'handwritten instruction' tags, 0 calls/0 branches/0 spills): 5x lw <- *a0 -> 5x ctc2 $0-$4 + 3x ctc2 $zero to $5-$7 (packed R matrix, zero translation) -> 5x lw <- *a1 -> s16 hi/lo pack via a hand-held `lui $t9,0xFFFF` mask -> 3 mvmva 1,0,0,0,0 cycles, each with the NEXT cycle's packing computed inside the current cycle's GTE latency window and mfc2 $9/$10/$11 drained between -> 9x sh to *a2, the last IN the jr-ra delay slot (0x80052A1C). Zero general-purpose computation on mfc2 outputs: cop2 ops + mechanical s16 I/O packaging only. No pure-C form — GCC 2.7.2 reorg.c stop_search_p cannot fill a delay slot across ASM_INPUT, and the per-cycle mask re-materialization + latency interleave are hand-scheduling. Cluster: archived .claude/rules/gte-3x3.md rules this family canonical (calc_fc_frame_8007EC5C ASM-WHOLE 2026-05-31); immediately address-adjacent to the authorized text1b.c LIBGTE leaf run func_80052A20/A88/B00/B44/B7C (owner-authorized 2026-08-06, f010b4b6). scan_hand_coded tier=LOW 2/8 (S3+S4 only) — authorization rests on cluster precedent + no-C-form pipeline, NOT on scanner tier. Owner-authorized <DATE>.
```

---

## 3. Class B — AMBIGUOUS (4)

Each row states the one thing that decides it. None should be authorized on present evidence.

| # | func | file | insns | cop2 | scanner | The deciding question |
|---|---|---|---|---|---|---|
| B1 | `func_8002FDB0` | `code6cac_b.c` | 92 | 10 (`c2,ctc2,lwc2,swc2`) | LOW 1/8 (S3 only) | Leaf-shaped (0 calls, 0 branches, 0 spills) with 5 "handwritten" tags — but unlike A1 its body is **real computation with a direct C form** (scratchpad vector subtraction `*(s32*)0x1F800360 = a.x - c.x` etc., already written that way in source at distance 21). **Decides it:** strip the 4 register pins, the `__asm__("" ::: "$5")` scheduling barrier and the `move %0,%1` alias (all cheat-asm today) and re-measure. If the residual goes to 0 it is COMPLETED-C. Only a residual that is provably GCC-unreachable — not merely stubborn — reopens the canonical question. |
| B2 | `func_800678A8` | `text1b.c` | 283 | 6 (`cfc2,ctc2` only) | LOW 2/8 (S3, S4) | The cop2 content is *only* GTE control-register save/restore (`cfc2`/`ctc2`), the OuterProduct/SquareRoot wrapper signature — but wrapped in 283 instructions with 8 branches and no C body drafted (distance 515 == full size). **Decides it:** draft the body. If the residual after a first honest draft concentrates *inside* the cfc2/ctc2 island, that island is a canonical GTE block inside a C function (normal, no authorization). If the whole 283 insns resist, that is a separate finding needing its own evidence. Right now there is no evidence either way. |
| B3 | `func_80018300` | `code6cac.c` | 317 | 34 across **14** regions | LOW 1/8 (S4) | The densest multi-island case: 34 cop2 insns in 14 separate spans, 0 calls, 23 branches. Its grind ledger records that the pre-existing body carried "4 register-asm pin(s), 6 hardcoded-$N inline-asm block(s), 62 `__asm__` occurrence(s) incl. **34 raw `.word` encoding(s)**." **Decides it:** audit those 34 `.word`s one by one. A `.word` that decodes to a cop2 opcode is canonical inline asm; a `.word` that decodes to a general-purpose opcode is [[inline-asm-injection]] and must be deleted. The count matching the 34 cop2 insns exactly is suggestive but not proof — someone has to decode them. |
| B4 | `func_80018094` | `code6cac.c` | 155 | 10 (`ctc2,mtc2,swc2`) | LOW 1/8 (S4) | 21 GTE-shaped `__asm__` blocks in source against only 10 cop2 insns in the target, plus **75 regfix rules** — the largest rule-to-cop2 ratio in the cohort. **Decides it:** same `.word` audit as B3, then a rule inventory. If the 75 rules are ordinary RA/scheduling paperwork around a small legitimate GTE island, this is straightforward Class C with heavy debt; if they are reconstructing cop2 bytes, that is a cheat-cleanup finding, not a canonical one. |

**Cross-cutting policy question the owner may want to settle once** (affects the 18 ASM-PARTIAL
functions whose current source contains GTE-shaped `__asm__` blocks): when a cop2 op is written as a
raw `.word 0x…` with a hardcoded `$N` operand rather than as a `%0`-placeholder `gte_*()`-style
block, is that canonical inline asm or hardcoded-`$N` injection? The precedent in
`inline_asm_canonical.txt` accepts `.word` cop2 encodings inside *authorized* bodies (e.g.
`func_80052754`, `.word 0x4AA00428` for `sqr`), but B3/B4 are unauthorized bodies. A single ruling
here would unblock the `.word` audits in B3 and B4 and set the bar for the other 16.

---

## 4. Class C — ROUTE-TO-C (81)

These stay pure-C grind targets. No canonical-asm case exists for any of them on present evidence.
Grouped reasons:

- **47 `ASM-SUSPECT`** — the gate's own reason string is the distance threshold and nothing else
  (`pure-C distance N > 50 → structural-asm suspect`, or `> 500 but scan_hand_coded tier=LOW →
  distance is not evidence`). Per [[canonical-gate-distance-not-evidence]], that is the gate's
  best guess on incomplete signal, not proof. Twenty-plus of these carry an explicit prior ruling in
  their own grind ledgers — e.g. `func_8005D814`: *"Park status is a REJECTED canonical routing
  (2026-06-09 audit: scan_hand_coded LOW, asm reads as standard GCC output, distance>500 heuristic
  misroute). This is pure-C work, not canonical-asm."* Authorizing any of them would reverse a
  documented audit.
- **37 of the 39 `ASM-PARTIAL`** — a small GTE island (0.00–0.17 cop2 fraction) inside an ordinary C
  function with calls and branches. The cop2 spans are already the accepted canonical form; the
  surrounding body is ordinary pure-C work.
- **5 census-matched PsyQ library bodies** — see §5. Compiled C by identity.

| # | func | file | insns | cop2 | calls/br | rules | scanner | reason |
|---|---|---|---|---|---|---|---|---|
| 1 | `func_800187F4` | code6cac.c | 662 | 48 | 1/42 | 0 | LOW 1/8 | GTE island only: 48/644 cop2 (c2,lwc2,mtc2,swc2) inside an ordinary C function (1 calls, 42 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 2 | `func_80019310` | code6cac.c | 83 | 14 | 0/2 | 2 | LOW 2/8 | GTE island only: 14/81 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (0 calls, 2 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 3 | `func_8001A820` | code6cac.c | 582 | 2 | 13/54 | 0 | LOW 1/8 | GTE island only: 2/576 cop2 (mtc2,swc2) inside an ordinary C function (13 calls, 54 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 4 | `func_8001F2E4` | code6cac.c | 351 | 4 | 9/33 | 0 | LOW 1/8 | GTE island only: 4/347 cop2 (mtc2,swc2) inside an ordinary C function (9 calls, 33 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 5 | `func_800203B4` | code6cac.c | 67 | 11 | 2/0 | 0 | LOW 1/8 | GTE island only: 11/65 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (2 calls, 0 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 6 | `func_800204C0` | code6cac.c | 124 | 11 | 2/7 | 0 | LOW 1/8 | GTE island only: 11/122 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (2 calls, 7 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 7 | `func_800207C8` | code6cac.c | 325 | 44 | 4/8 | 0 | LOW 1/8 | GTE island only: 44/317 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (4 calls, 8 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 8 | `func_800288C8` | code6cac_b.c | 489 | 2 | 9/55 | 0 | LOW 1/8 | GTE island only: 2/465 cop2 (mtc2,swc2) inside an ordinary C function (9 calls, 55 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 9 | `func_8002A458` | code6cac_b.c | 428 | 4 | 11/29 | 0 | LOW 1/8 | GTE island only: 4/416 cop2 (mtc2,swc2) inside an ordinary C function (11 calls, 29 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 10 | `func_8002CD58` | code6cac_b.c | 370 | 54 | 8/10 | 0 | LOW 1/8 | GTE island only: 54/352 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (8 calls, 10 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 11 | `func_8002D320` | code6cac_b.c | 126 | 8 | 0/14 | 13 | LOW 3/8 | GTE island only: 8/120 cop2 (c2,lwc2,mtc2,swc2) inside an ordinary C function (0 calls, 14 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 12 | `func_8002D518` | code6cac_b.c | 154 | 2 | 0/20 | 33 | LOW 2/8 | GTE island only: 2/144 cop2 (mtc2,swc2) inside an ordinary C function (0 calls, 20 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 13 | `func_8002D780` | code6cac_b.c | 212 | 8 | 3/10 | 65 | LOW 1/8 | GTE island only: 8/202 cop2 (c2,lwc2,mtc2,swc2) inside an ordinary C function (3 calls, 10 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 14 | `func_8002DAD0` | code6cac_b.c | 212 | 29 | 4/5 | 0 | LOW 1/8 | GTE island only: 29/204 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (4 calls, 5 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 15 | `func_8002DE20` | code6cac_b.c | 548 | 18 | 0/37 | 0 | LOW 1/8 | GTE island only: 18/506 cop2 (c2,lwc2,swc2) inside an ordinary C function (0 calls, 37 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 16 | `func_8002E838` | code6cac_b.c | 123 | 13 | 4/2 | 0 | LOW 1/8 | GTE island only: 13/119 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (4 calls, 2 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 17 | `func_8002EBDC` | code6cac_b.c | 188 | 24 | 6/5 | 45 | LOW 1/8 | GTE island only: 24/182 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (6 calls, 5 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 18 | `func_8002F2D0` | code6cac_b.c | 296 | 13 | 5/20 | 0 | LOW 1/8 | GTE island only: 13/270 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (5 calls, 20 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 19 | `func_8002F770` | code6cac_b.c | 324 | 13 | 11/20 | 0 | LOW 1/8 | GTE island only: 13/298 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (11 calls, 20 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 20 | `func_8002FC80` | code6cac_b.c | 76 | 10 | 1/1 | 4 | LOW 1/8 | GTE island only: 10/74 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (1 calls, 1 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 21 | `func_8002FF20` | code6cac_b.c | 101 | 11 | 6/0 | 0 | LOW 1/8 | GTE island only: 11/99 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (6 calls, 0 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 22 | `func_800300B4` | code6cac_b.c | 85 | 11 | 5/0 | 0 | LOW 1/8 | GTE island only: 11/83 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (5 calls, 0 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 23 | `func_80031890` | code6cac_b.c | 165 | 11 | 3/11 | 0 | LOW 1/8 | GTE island only: 11/163 cop2 (c2,ctc2,lwc2,mtc2,swc2) inside an ordinary C function (3 calls, 11 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 24 | `func_80032314` | code6cac_b.c | 111 | 2 | 0/12 | 12 | LOW 2/8 | GTE island only: 2/109 cop2 (mtc2,swc2) inside an ordinary C function (0 calls, 12 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 25 | `func_800325E0` | code6cac_b.c | 157 | 2 | 1/10 | 16 | LOW 1/8 | GTE island only: 2/149 cop2 (mtc2,swc2) inside an ordinary C function (1 calls, 10 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 26 | `func_8003E6D8` | code6cac_c2.c | 299 | 11 | 8/23 | 0 | LOW 1/8 | GTE island only: 11/297 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (8 calls, 23 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 27 | `func_800620B8` | text1b.c | 501 | 1 | 12/20 | 0 | LOW 1/8 | GTE island only: 1/501 cop2 (swc2) inside an ordinary C function (12 calls, 20 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 28 | `func_8006295C` | text1b.c | 420 | 1 | 12/16 | 0 | LOW 1/8 | GTE island only: 1/420 cop2 (swc2) inside an ordinary C function (12 calls, 16 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 29 | `func_80063084` | text1b.c | 667 | 6 | 11/29 | 0 | LOW 1/8 | GTE island only: 6/661 cop2 (ctc2,swc2) inside an ordinary C function (11 calls, 29 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 30 | `func_80063E10` | text1b.c | 443 | 58 | 3/10 | 0 | LOW 1/8 | GTE island only: 58/431 cop2 (c2,ctc2,lwc2,mfc2,mtc2,swc2) inside an ordinary C function (3 calls, 10 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 31 | `func_800646E8` | text1b.c | 490 | 6 | 8/20 | 0 | LOW 1/8 | GTE island only: 6/490 cop2 (ctc2,swc2) inside an ordinary C function (8 calls, 20 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 32 | `func_80065800` | text1b.c | 1456 | 15 | 24/49 | 0 | LOW 1/8 | GTE island only: 15/1454 cop2 (c2,cfc2,ctc2,lwc2,mfc2,swc2) inside an ordinary C function (24 calls, 49 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 33 | `func_80067200` | text1b.c | 306 | 16 | 8/18 | 0 | LOW 1/8 | GTE island only: 16/298 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (8 calls, 18 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 34 | `func_80067D14` | text1b.c | 1053 | 27 | 6/61 | 0 | LOW 1/8 | GTE island only: 27/1047 cop2 (c2,ctc2,lwc2,swc2) inside an ordinary C function (6 calls, 61 branches) — cop2 stays canonical inline asm, rest is pure-C work |
| 35 | `CD_ready` | system.c | 179 | 0 | 9/16 | 42 | LOW 2/8 | census-matched compiled PsyQ library body (LIBCD) — compiled C by identity |
| 36 | `_comb_control` | main.c | 763 | 0 | 15/51 | 0 | LOW 1/8 | census-matched compiled PsyQ library body (LIBCOMB) — compiled C by identity |
| 37 | `_spu_gcSPU` | main.c | 194 | 0 | 0/23 | 104 | LOW 1/8 | census-matched compiled PsyQ library body (LIBSPU) — compiled C by identity |
| 38 | `_spu_pitch2note` | main.c | 74 | 0 | 0/7 | 0 | LOW 1/8 | census-matched compiled PsyQ library body (LIBSPU) — compiled C by identity |
| 39 | `func_80017848` | ings.c | 127 | 0 | 1/9 | 2 | LOW 0/8 | gate reason is distance-only (`pure-C distance 84 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 40 | `func_800198D0` | code6cac.c | 749 | 0 | 0/72 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 749 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 41 | `func_8001CE60` | code6cac.c | 588 | 0 | 20/61 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 588 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 42 | `func_80022580` | code6cac.c | 621 | 0 | 10/52 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 621 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 43 | `func_80023F08` | code6cac.c | 2983 | 0 | 65/345 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 2983 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 44 | `func_80027AD8` | code6cac_b.c | 574 | 0 | 38/59 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 574 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 45 | `func_80029454` | code6cac_b.c | 1025 | 0 | 12/104 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 1025 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 46 | `func_8002AB08` | code6cac_b.c | 1112 | 0 | 14/111 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 1112 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 47 | `func_8002CA8C` | code6cac_b.c | 179 | 0 | 4/19 | 2 | LOW 0/8 | gate reason is distance-only (`pure-C distance 177 > 50`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 48 | `func_8002E6B0` | code6cac_b.c | 98 | 0 | 0/2 | 2 | LOW 1/8 | gate reason is distance-only (`pure-C distance 57 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 49 | `func_80030D7C` | code6cac_b.c | 709 | 0 | 17/63 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 709 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 50 | `func_800335D8` | code6cac_b.c | 176 | 0 | 6/18 | 43 | LOW 0/8 | gate reason is distance-only (`pure-C distance 65 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 51 | `func_80034708` | code6cac_b.c | 544 | 0 | 23/31 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 544 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 52 | `func_80036140` | code6cac_b2_post.c | 512 | 0 | 29/52 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 512 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 53 | `func_800393C8` | code6cac_c_mid.c | 123 | 0 | 0/15 | 2 | LOW 2/8 | gate reason is distance-only (`pure-C distance 57 > 50`); scanner LOW 2/8, zero S1/S2/S6 |
| 54 | `func_8003993C` | code6cac_c_mid.c | 526 | 0 | 30/40 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 526 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 55 | `func_8003DE14` | code6cac_c2.c | 179 | 0 | 7/11 | 36 | LOW 1/8 | gate reason is distance-only (`pure-C distance 119 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 56 | `func_8003EB84` | code6cac_c2.c | 143 | 0 | 0/19 | 2 | LOW 1/8 | gate reason is distance-only (`pure-C distance 98 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 57 | `func_8003F6D8` | config.c | 71 | 0 | 2/4 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 55 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 58 | `func_8003F824` | config.c | 128 | 0 | 4/12 | 26 | LOW 1/8 | gate reason is distance-only (`pure-C distance 61 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 59 | `func_80042C80` | text1a_c.c | 132 | 0 | 0/0 | 63 | LOW 0/8 | gate reason is distance-only (`pure-C distance 58 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 60 | `func_800430E4` | text1a_c.c | 88 | 0 | 1/1 | 25 | LOW 1/8 | gate reason is distance-only (`pure-C distance 52 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 61 | `func_80047BE0` | sound.c | 109 | 0 | 3/5 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 63 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 62 | `func_80048864` | text1b.c | 134 | 0 | 5/5 | 36 | LOW 0/8 | gate reason is distance-only (`pure-C distance 87 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 63 | `func_80055138` | text1b.c | 516 | 0 | 5/42 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 516 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 64 | `func_80055B60` | text1b.c | 1110 | 0 | 13/156 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 1110 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 65 | `func_80056CB8` | text1b.c | 204 | 0 | 3/13 | 15 | LOW 1/8 | gate reason is distance-only (`pure-C distance 188 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 66 | `func_80058580` | text1b.c | 2991 | 0 | 31/428 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 2991 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 67 | `func_8005C8A8` | text1b.c | 753 | 0 | 29/25 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 753 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 68 | `func_8005D554` | text1b.c | 176 | 0 | 9/4 | 88 | LOW 0/8 | gate reason is distance-only (`pure-C distance 65 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 69 | `func_8005D814` | text1b.c | 545 | 0 | 12/22 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 545 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 70 | `func_8005E54C` | text1b.c | 799 | 0 | 28/47 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 799 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 71 | `func_8005F1C8` | text1b.c | 564 | 0 | 15/28 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 564 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 72 | `func_8006A880` | text1b.c | 552 | 0 | 47/9 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 552 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 73 | `func_8006C21C` | text1b.c | 622 | 0 | 27/13 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 622 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 74 | `func_8006F97C` | text1b.c | 515 | 0 | 25/24 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 515 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 75 | `func_80070188` | text1b.c | 698 | 0 | 14/49 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 698 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 76 | `func_80070C70` | text1b.c | 194 | 0 | 17/7 | 32 | LOW 1/8 | gate reason is distance-only (`pure-C distance 118 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 77 | `func_80070F78` | text1b.c | 810 | 0 | 21/60 | 0 | LOW 0/8 | gate reason is distance-only (`pure-C distance 810 > 500 but scan_hand_coded tier=LOW`); scanner LOW 0/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 78 | `func_800720FC` | text1b.c | 690 | 0 | 33/45 | 0 | LOW 1/8 | gate reason is distance-only (`pure-C distance 690 > 500 but scan_hand_coded tier=LOW`); scanner LOW 1/8, zero S1/S2/S6; no C body drafted yet, so distance == instruction count |
| 79 | `func_8007352C` | text1b.c | 127 | 0 | 7/8 | 11 | LOW 1/8 | gate reason is distance-only (`pure-C distance 54 > 50`); scanner LOW 1/8, zero S1/S2/S6 |
| 80 | `func_80074B18` | text1b.c | 133 | 0 | 3/7 | 29 | LOW 0/8 | gate reason is distance-only (`pure-C distance 79 > 50`); scanner LOW 0/8, zero S1/S2/S6 |
| 81 | `sprintf` | text1b_b.c | 535 | 0 | 3/63 | 0 | LOW 0/8 | census-matched compiled PsyQ library body (LIBC) — compiled C by identity |

---

## 5. Conflicts

### 5.1 Census-matched compiled library bodies inside the sweep (5) — hard exclusions

Five of the 86 are `EXPORTED_XDEF` census identities for PsyQ/libc functions that ship as *compiled
C* in the reference tree. A canonical-asm routing for any of these would assert the original was
hand-written assembly, which the census contradicts directly. **None may be authorized.**

| func | addr | verdict | dist | rules | Census identity |
|---|---|---|---|---|---|
| `CD_ready` | 0x80081030 | ASM-SUSPECT | 56 | 42 | LIBCD/BIOS, SOTN C body `src/main/psxsdk/libcd/bios.c:260-289` (30 lines, `has_inline_asm: false`) |
| `_spu_pitch2note` | 0x8008BC60 | ASM-SUSPECT | 64 | 0 | LIBSPU/S_N2P, SOTN C body `src/main/psxsdk/libspu/s_n2p.c:56-96` |
| `_spu_gcSPU` | 0x800896A0 | ASM-SUSPECT | 121 | 104 | LIBSPU/S_M_INT, SOTN C body `src/main/psxsdk/libspu/s_m_m.c:88-153` |
| `sprintf` | 0x80079A30 | ASM-SUSPECT | 535 | 0 | LIBC/SPRINTF, SOTN C body `src/main/psxsdk/libc/sprintf.c:21-307` (287 lines) |
| `_comb_control` | 0x8008C464 | ASM-SUSPECT | 763 | 0 | LIBCOMB/COMB (no SOTN body captured) |

All five are LOW tier with zero S1/S2/S6, so none was a canonical candidate anyway — but the
collision is worth recording because four of the five come with a reference C body that is a strong
*pure-C* lead, and because `CD_ready` is the case the brief flagged as appearing in both sets.

Two carry census caveats that matter before anyone acts on the identity:
- `sprintf` — `classification: CONTRADICTED_ALIAS`, contradicting live alias
  `dispsleepmenutex_helper_80079A30`. Its own ledger still refers to the function as
  `func_80079A30`.
- `_comb_control` — `classification: CONTRADICTED_ALIAS`, contradicting live alias
  `bios_helper_8008C464`. Its ledger documents a rodata/jump-table placement issue in that TU
  (jump tables landing at the start of `.text`), which is an infrastructure question, not an
  authorship one.

### 5.2 No canonical/census collision in Class A or B

`func_80052930` and the four Class B functions are all auto-named `func_*` with no census identity,
no `sony_name`, and no reference body. Nothing in the census contradicts the Class A recommendation.

### 5.3 Prior-audit collisions

At least 20 Class C items carry a 2026-06-09 canonical-audit REJECTION recorded in their ledger or
prior park reason (`func_8005D814`, `func_8006A880`, `func_8005F1C8`, `func_80027AD8`,
`func_8001CE60`, `func_80022580`, `func_8006C21C`, `func_800720FC`, `func_80070188`,
`func_80030D7C`, `func_800198D0`, `func_8005C8A8`, `_comb_control`, `func_8005E54C`,
`func_80070F78`, `func_80029454`, and others). They are all currently `status=active`, i.e. the
rejection stuck and they were correctly returned to pure-C work. Nothing in this sweep's data
disturbs that.

---

## 6. Ledger / WIP evidence already on file

40 of the 86 have a grind ledger (`memory/grind/<func>/`), 5 have a WIP entry
(`memory/wip/<func>/`). Notable disposition-relevant content:

- **Canonical routing already rejected, in the function's own words:** `func_8005D814`,
  `func_8006A880`, `func_8005F1C8`, `func_80027AD8`, `func_8001CE60`, `func_80022580`,
  `func_8006C21C`, `func_800720FC`, `func_80070188`, `func_80030D7C`, `func_800198D0`,
  `func_8005C8A8`, `func_8005E54C`, `func_80070F78`, `func_80029454`, `_comb_control`.
- **GTE island explicitly identified as the sole ASM-PARTIAL trigger** (i.e. the verdict is
  understood and is not an authorship claim): `func_8001A820` — *"ASM-PARTIAL cause identified: ONE
  GTE cop2 island at L172-177 … marked 'handwritten instruction' by splat"*; `func_80063084` —
  *"The ASM-PARTIAL verdict = 6 splat-marked 'handwritten' cop2 insns inside otherwise standard GCC
  output"*; `func_80067D14` — *"4 blocks … M2C_ERROR markers exactly at the 4 GTE blocks"*;
  `func_800187F4` — *"EVERY GTE idiom needed has a COMPLETED same-file precedent."* These four are
  unambiguous Class C: the ledger author already worked out that the cop2 island is routine.
- **Cheat-asm warnings on file:** `func_80018300` (see B3); `func_8006F97C` ledger warns its template
  sibling `func_80070C70` itself carries cheat-asm (`register s32 c60 asm("$20")` +
  `__asm__("addiu %0,$0,1")`) — `func_80070C70` is also in this sweep (Class C, 32 rules, 7 cheat-asm
  blocks in source).
- **`func_8002DAD0`** keeps a `pre-include-asm-body.c` noting *"Inline-asm scaffolding retired;
  pure-C decomp pending"* — an explicit prior decision to route to C.

---

## 7. What I am *not* claiming

- I did not run any engine command, build, or `objdump` for this packet; every number above comes
  from the pre-captured `results.json` / `xref.json` plus read-only inspection of `asm/funcs/*.s`,
  `src/*.c`, `inline_asm_canonical.txt`, and the ledgers.
- The Class A recommendation is not scanner-corroborated. Its tier is LOW 2/8 and the strict
  [[canonical-asm-retirement]] gate (STRONG + S1/S2/S6) is **not** met. It is offered as an
  owner-judgment case resting on the archived `gte-3x3` family ruling, address adjacency to the
  authorized text1b.c LIBGTE run, and the mechanical-packaging test — the same basis the owner used
  on 2026-08-06 for `func_80052A20` / `func_80052A88` / `func_80052B7C`. If that basis is not
  accepted, `func_80052930` moves to Class B, not Class C.
- Distance figures appear throughout as context. None of them is offered as routing evidence.
