# ASM-SUSPECT / ASM-PARTIAL adjudication census — 2026-08-06

Campaign 2 of the strategic plan. Every **active** queue item carrying verdict
`ASM-SUSPECT` (108) or `ASM-PARTIAL` (46) — 154 functions — adjudicated into
canonical-eligible / compiler-emitted / uncertain with per-function evidence.

Machine-readable companion: `tmp/census_2026-08-06.json`.
Reproduce: `tmp/census_scan.py` → `tmp/census_reasons.py` → `tmp/census_merge.py` → `tmp/census_report.py`.

## Headline

**89% of this population is ordinary compiler-emitted C, not hand-written assembly.**
Only 7 of 154 functions (4.5%) carry evidence supporting canonical-asm authorization,
and 10 more (6.5%) are genuinely uncertain.

| Bucket | Count | Share |
|---|---:|---:|
| canonical-eligible | 7 | 5% |
| uncertain | 10 | 6% |
| compiler-emitted (real decomp targets) | 137 | 89% |
| **total** | **154** | |

| Queue verdict | canonical-eligible | uncertain | compiler-emitted |
|---|---:|---:|---:|
| ASM-SUSPECT | 0 | 3 | 105 |
| ASM-PARTIAL | 7 | 7 | 32 |

### Why the population was in limbo (and why it mostly shouldn't be)

The two verdicts do not mean the same thing, and neither means "hand-written":

- **`ASM-SUSPECT` is a distance-only verdict.** `engine/canonical.py:_verdict` assigns it
  when the gate finds *zero* definitive opcode signals but the honest pure-C distance
  exceeds `SUSPECT_DISTANCE = 50`. The gate's own 2026-06-09 calibration comment says
  distance alone is NOT hand-asm evidence — the 2026-06-09 audit found 0/26 auto-escalated
  functions had any hand-coded signal. This census confirms that at 5× the sample size:
  105 of 108 ASM-SUSPECT functions have no hand-coded signal of any kind.
- **`ASM-PARTIAL` means the gate found real cop2/GTE (or trapping) opcodes**, but below the
  0.8 density that would make it `ASM-WHOLE`. For most of these the cop2 span is a handful of
  instructions inside a large ordinary C body — the gate is already region-granular, so the
  correct treatment is pure C plus inline asm for the cop2 span, not whole-function authorization.

## Method and the base-rate correction

Signals came from three sources, run over all 154 functions:

1. `tools/scan_hand_coded.py` S1–S8 (run in-process over all 1,323 parseable `asm/funcs/*.s`).
2. The canonical gate's own definitive-opcode detector, `engine/canonical.classify`, driven
   off `objdump` of the existing `build/bb2.elf` (read-only — no build was run).
3. New detectors written for this census (`tmp/census_scan.py`): trapping arithmetic (T1),
   ghost callee-save (G1), leaf-saves-`$ra` (R1), no-frame-uses-`$sN` (N1), non-standard
   `break` code (B1), duplicated adjacent cop2 read (X1), and opcode-signature similarity
   to an already-authorized canonical function (K1).

**Two signals were measured to be noise and are excluded from scoring.** Over the 656
corpus functions of ≥40 instructions, S4 (front-loaded loads) fires on 253 — 39% — and an
unfilled `jr $ra` delay slot fires on nearly everything. Scoring them produced a 37-function
"uncertain" bucket consisting entirely of large functions with a standard GCC epilogue. They
are recorded per-function in the JSON as `noise_signals` but never contribute to a tier.

**G1 (ghost callee-save) was demoted from decisive to corroborating by direct measurement.**
A ghost callee-save is `sw $sN,K($sp)` in the prologue and `lw $sN,K($sp)` in the epilogue
with the register untouched in between. Base-rating it across all 1,437 `asm/funcs/*.s`:

| Population | G1 hits |
|---|---:|
| authorized canonical-asm (`inline_asm_canonical.txt`) | 11 |
| still-queued | 5 |
| **already COMPLETED-C (0 rules, byte-matched)** | **2** |

The two COMPLETED-C witnesses — `func_80037540` and `replay_camera_rob_back_loose3` —
prove GCC 2.7.2 *does* emit a save/restore pair for a register whose uses were later deleted,
exactly analogous to the phantom frame slots documented in `phantom-frame-slots-gcc272`.
So G1 is strongly *enriched* in hand-written code (11 of ~130 authorized vs 2 of ~1,150
completed) but is not GCC-impossible, and cannot carry an authorization on its own.

A signal was treated as **decisive** only if it is GCC-2.7.2-impossible with zero
counter-examples in the COMPLETED-C population: trapping arithmetic, non-standard `break`,
S1/S2/S6/S7/S8, duplicated cop2 read, or near-clone of an authorized function.

## Bucket A — canonical-eligible (7)

Split into two kinds. The first three carry a construct GCC 2.7.2 cannot emit at all and are
candidates for **whole-function** canonical authorization. The last four are GTE-dense leaves
where the cop2 spans are the function's substance and **region-granular** authorization applies.

### A1 — GCC-impossible construct (whole-function candidates)

| Function | Verdict | Dist | Insns | Rules | Tier | Key evidence |
|---|---|---:|---:|---:|---|---|
| `ang_hosei` | ASM-PARTIAL | 50 | 51 | 1 | STRONG | T1:trapping-arith(addi); B1:nonstd-break(break 0,263,break 0,1) |
| `func_8004C388` | ASM-PARTIAL | 29 | 30 | 0 | STRONG | T1:trapping-arith(add) |
| `func_80052788` | ASM-PARTIAL | 23 | 29 | 0 | STRONG | T1:trapping-arith(sub); X1:duplicated-cop2-read(mfc2 $t0, $9); GTE-region:15/27 (56%) in 3 region(s) |

**`ang_hosei`** (ings2.c, distance 50, 51 insns) — GCC-2.7.2-impossible construct present: T1:trapping-arith(addi); B1:nonstd-break(break 0,263,break 0,1). No counter-example exists in the COMPLETED-C population.

**`func_8004C388`** (text1b.c, distance 29, 30 insns) — GCC-2.7.2-impossible construct present: T1:trapping-arith(add). No counter-example exists in the COMPLETED-C population.

**`func_80052788`** (text1b.c, distance 23, 29 insns) — GCC-2.7.2-impossible construct present: T1:trapping-arith(sub); X1:duplicated-cop2-read(mfc2 $t0, $9). No counter-example exists in the COMPLETED-C population.

Detail on each:

- **`ang_hosei`** is the strongest case in the whole census and is also mislabelled. Its first
  block is a 5-instruction `break 0, 263` trampoline — the same Marionation engine-call family
  as the already-authorized `md_gview_init` (`break` code 0x104) and `bios_FileReadRaw` (0x105).
  Everything after the `jr $ra` is **unreachable from it** and is the program's C runtime
  startup: it begins at `0x800836EC`, which `AGENTS.md` records as the executable's entry
  point. It zeroes a BSS range, computes `$sp` from `D_800A2690` with a trapping `addi`,
  sets `$gp` from `_gp` and `$fp` from `$sp`, calls `InitHeap`, and terminates in `break 0, 1`.
  A crt0 is hand-written asm by definition; there is no C form for establishing `$sp`/`$gp`.
- **`func_8004C388`** is a 30-instruction leaf with no frame and no spills: six front-loaded
  `lh` loads of two 3-vectors, three trapping `add`s, three `sra 1` (a midpoint average),
  then a packed-byte lerp of the trailing halfword, with the final `sh` in the `jr $ra` delay
  slot. Five trapping `add`s where GCC would emit `addu`. Zero regfix rules today.
- **`func_80052788`** is the GTE `gpf`/`gpl` interpolation sibling of the same family: 15 of 27
  instructions are cop2, it uses a trapping `sub` to form `0x1000 - t`, and it emits
  `mfc2 $t0, $9` **twice in a row** — an identical redundant read that GCC's CSE would collapse,
  present as a hand-coder's GTE result-latency pad.

### A2 — GTE-dense leaves (region-granular authorization)

| Function | Verdict | Dist | Insns | Rules | Tier | Key evidence |
|---|---|---:|---:|---:|---|---|
| `func_80052930` | ASM-PARTIAL | 51 | 60 | 0 | WEAK | GTE-region:26/60 (43%) in 8 region(s); S3:no-spills |
| `func_80052A88` | ASM-PARTIAL | 25 | 30 | 1 | WEAK | GTE-region:14/28 (50%) in 4 region(s) |
| `func_80052B7C` | ASM-PARTIAL | 20 | 26 | 1 | WEAK | GTE-region:14/24 (58%) in 3 region(s); K1:canon-similar func_8007ED6C j=0.704 |
| `game_2d_CheckLifeGaugeNoDisp` | ASM-PARTIAL | 20 | 26 | 1 | WEAK | GTE-region:14/24 (58%) in 3 region(s); K1:canon-similar func_8007ED6C j=0.704 |

These four sit in `src/text1a.c`-adjacent GTE wrapper territory and form a cluster:
`func_80052A88`, `func_80052B7C`, and `game_2d_CheckLifeGaugeNoDisp` are 24–30-instruction
leaves that are 43–58% cop2, and the latter two are opcode-signature-similar (Jaccard 0.70)
to the already-authorized `func_8007ED6C`. `func_80052930` is the larger sibling (26/60 cop2
across 8 regions, no spills). All four already carry 0–1 regfix rules, so authorization
closes them cheaply. Treatment is the standard GTE-wrapper form, not new policy.

## Bucket B — uncertain (10)

No GCC-impossible construct, but enough corroborating signal that a manual `.s` read is
warranted before committing to a pure-C grind. Ordered by review priority.

| Function | Verdict | Dist | Insns | Rules | Tier | Key evidence |
|---|---|---:|---:|---:|---|---|
| `DispHira` | ASM-PARTIAL | 295 | 299 | 1 | WEAK | GTE-region:11/297 (4%) in 4 region(s); G1:ghost-callee-save($s2) |
| `func_800678A8` | ASM-PARTIAL | 282 | 283 | 1 | WEAK | GTE-region:6/283 (2%) in 3 region(s); S3:no-spills |
| `func_8005763C` | ASM-SUSPECT | 274 | 292 | 1 | WEAK | G1:ghost-callee-save($s0) |
| `hirahira_w_ctrl_2` | ASM-SUSPECT | 58 | 132 | 63 | WEAK | G1:ghost-callee-save($s1) |
| `pad_main_control` | ASM-SUSPECT | 57 | 98 | 2 | WEAK | G1:ghost-callee-save($s4) |
| `func_80019310` | ASM-PARTIAL | 44 | 83 | 2 | WEAK | GTE-region:14/81 (17%) in 6 region(s); S3:no-spills |
| `saTan5TakeAnim2` | ASM-PARTIAL | 33 | 154 | 33 | WEAK | GTE-region:2/144 (1%) in 2 region(s); S3:no-spills |
| `func_8002D320` | ASM-PARTIAL | 32 | 126 | 13 | WEAK | GTE-region:8/120 (7%) in 4 region(s); S3:no-spills; S5:cluster-sibling |
| `Pad_Prs` | ASM-PARTIAL | 27 | 111 | 12 | WEAK | GTE-region:2/109 (2%) in 2 region(s); S3:no-spills |
| `coli_check_circle_hit_line` | ASM-PARTIAL | 21 | 92 | 0 | WEAK | GTE-region:10/90 (11%) in 4 region(s); S3:no-spills |

Reading notes:

- **`func_8005763C` (d=274) and `DispHira` (d=295)** are the two worth reading first: both are
  ~290-instruction bodies whose only distinguishing signal is a ghost callee-save, and both sit
  at a distance high enough that a hand-coded origin would not be surprising. `DispHira` also
  has four small cop2 regions.
- **`pad_main_control` (d=57) and `hirahira_w_ctrl_2` (d=58)** carry the same G1 signal but at
  distances a genuinely hand-written 98–132-instruction function would not plausibly sit at.
  The prior for these two is compiler-emitted-with-a-ghost-save; expect them to resolve into
  bucket C. `hirahira_w_ctrl_2` carries 63 regfix rules, the most of any function in bucket B.
- The remaining six are ASM-PARTIAL functions with a *small* cop2 region (1–17%) plus
  `S3:no-spills`. `S3` fires on 68 of 656 large corpus functions, so it is suggestive rather
  than probative; the likely disposition is "pure C body, inline asm for the cop2 span."

## Bucket C — compiler-emitted (137)

Real decomp targets. Every one has zero definitive gate signals and zero S1/S2/S6/S7/S8;
105 are ASM-SUSPECT (distance-only) and
32 are ASM-PARTIAL with a small cop2 region.
They carry 807 regfix rules between them; 9 are already rule-free.

Distribution by source file and distance:

| File | Count |
|---|---:|
| `src/text1b.c` | 62 |
| `src/code6cac_b.c` | 22 |
| `src/code6cac.c` | 15 |
| `src/main.c` | 12 |
| `src/display.c` | 4 |
| `src/code6cac_c2.c` | 4 |
| `src/text1a_c.c` | 3 |
| `src/system.c` | 3 |
| `src/config.c` | 3 |
| `src/text1b_b.c` | 2 |
| `src/code6cac_b2_post.c` | 2 |
| `src/ings.c` | 2 |
| `src/sound.c` | 1 |
| `src/code6cac_c_mid.c` | 1 |
| `src/text1a.c` | 1 |

| Pure-C distance | Count |
|---|---:|
| ≤100 | 25 |
| 101–250 | 49 |
| >250 | 63 |

A sanity check on the far end of the distribution: `func_80043454` is the highest-distance
member (477) and its prologue is textbook GCC — full callee-save block, `$a1`/`$a2`/`$a3` homed
to the stack frame, and `sll $a0,16 / sra $s7,$a0,16` sign-extending an `s16` parameter. It is
a 479-instruction C function, not assembly. Size is size.

Full listing, easiest first:

| Function | Verdict | Dist | Insns | Rules | Tier | Key evidence |
|---|---|---:|---:|---:|---|---|
| `save_vc_ctrl` | ASM-PARTIAL | 2 | 16 | 1 | NONE | — |
| `cpu_check_tubazeri` | ASM-PARTIAL | 28 | 76 | 4 | NONE | GTE-region:10/74 (14%) in 4 region(s) |
| `calc_fc_frame_800203B4` | ASM-PARTIAL | 33 | 67 | 0 | NONE | GTE-region:11/65 (17%) in 4 region(s) |
| `func_8002FF20` | ASM-PARTIAL | 34 | 101 | 0 | NONE | GTE-region:11/99 (11%) in 4 region(s) |
| `func_80031890` | ASM-PARTIAL | 36 | 165 | 0 | NONE | GTE-region:11/163 (7%) in 4 region(s) |
| `saSeInit` | ASM-PARTIAL | 36 | 123 | 0 | NONE | GTE-region:13/119 (11%) in 6 region(s) |
| `func_800300B4` | ASM-PARTIAL | 37 | 85 | 0 | NONE | GTE-region:11/83 (13%) in 4 region(s) |
| `func_800325E0` | ASM-PARTIAL | 37 | 157 | 16 | NONE | GTE-region:2/149 (1%) in 2 region(s) |
| `single_game_SetAbilityData` | ASM-PARTIAL | 52 | 124 | 0 | NONE | GTE-region:11/122 (9%) in 4 region(s) |
| `func_800430E4` | ASM-SUSPECT | 52 | 88 | 25 | NONE | — |
| `func_8007352C` | ASM-SUSPECT | 54 | 127 | 11 | NONE | — |
| `ang_near_dif` | ASM-SUSPECT | 55 | 71 | 0 | NONE | — |
| `saTan0KiWareMoveA` | ASM-PARTIAL | 56 | 212 | 65 | NONE | GTE-region:8/202 (4%) in 4 region(s) |
| `marionation_Exec` | ASM-SUSPECT | 56 | 179 | 42 | NONE | S5:cluster-sibling |
| `saSeInit_2` | ASM-SUSPECT | 57 | 123 | 2 | NONE | S3:no-spills |
| `tslPrintScreen` | ASM-SUSPECT | 61 | 128 | 26 | NONE | — |
| `func_80047BE0` | ASM-SUSPECT | 63 | 109 | 0 | NONE | — |
| `func_8008BC60` | ASM-SUSPECT | 64 | 74 | 0 | NONE | S3:no-spills |
| `mk_leaf_newpos` | ASM-SUSPECT | 65 | 176 | 43 | NONE | — |
| `gnd_land_hit_char_tsuba` | ASM-SUSPECT | 65 | 176 | 88 | NONE | — |
| `func_80074B18` | ASM-SUSPECT | 79 | 133 | 29 | NONE | — |
| `func_80017848` | ASM-SUSPECT | 84 | 127 | 2 | NONE | — |
| `saTan4FireDisp_80048864` | ASM-SUSPECT | 87 | 134 | 36 | NONE | — |
| `DispSchoolBG` | ASM-PARTIAL | 97 | 188 | 45 | NONE | GTE-region:24/182 (13%) in 10 region(s) |
| `CalcHiraNormal` | ASM-SUSPECT | 98 | 143 | 2 | NONE | S3:no-spills |
| `marionation_camera_Exec` | ASM-PARTIAL | 103 | 155 | 75 | NONE | GTE-region:10/153 (7%) in 6 region(s) |
| `func_8006BB68` | ASM-SUSPECT | 110 | 112 | 1 | NONE | — |
| `func_8006DD94` | ASM-SUSPECT | 115 | 117 | 1 | NONE | — |
| `func_8005C6D0` | ASM-SUSPECT | 117 | 118 | 1 | NONE | — |
| `func_80070C70` | ASM-SUSPECT | 118 | 194 | 32 | NONE | — |
| `func_8003DE14` | ASM-SUSPECT | 119 | 179 | 36 | NONE | — |
| `exec_game` | ASM-SUSPECT | 121 | 194 | 104 | NONE | S3:no-spills |
| `func_8006D3DC` | ASM-SUSPECT | 124 | 126 | 1 | NONE | — |
| `func_80057ACC` | ASM-SUSPECT | 125 | 127 | 1 | NONE | — |
| `tslGlobalMemFree_8005C2A8` | ASM-SUSPECT | 133 | 134 | 1 | NONE | — |
| `func_80069F80` | ASM-SUSPECT | 134 | 136 | 1 | NONE | — |
| `func_8006A1A0` | ASM-SUSPECT | 137 | 139 | 1 | NONE | — |
| `func_8005C074` | ASM-SUSPECT | 139 | 141 | 1 | NONE | — |
| `func_80063BD0` | ASM-SUSPECT | 143 | 144 | 1 | NONE | S3:no-spills |
| `func_8007CBB0` | ASM-SUSPECT | 149 | 151 | 1 | NONE | — |
| `func_80054604` | ASM-SUSPECT | 158 | 160 | 1 | NONE | — |
| `func_8007D048` | ASM-SUSPECT | 159 | 161 | 1 | NONE | — |
| `func_80076D74` | ASM-SUSPECT | 159 | 161 | 1 | NONE | — |
| `func_80048FFC` | ASM-SUSPECT | 163 | 232 | 1 | NONE | — |
| `saTan1GaugeMain` | ASM-SUSPECT | 164 | 166 | 1 | NONE | — |
| `func_8006BEC4` | ASM-SUSPECT | 167 | 169 | 1 | NONE | — |
| `func_8005BA8C` | ASM-SUSPECT | 168 | 169 | 1 | NONE | — |
| `cpu_check_run_attack` | ASM-PARTIAL | 170 | 317 | 1 | NONE | GTE-region:34/307 (11%) in 14 region(s) |
| `func_8002CA8C` | ASM-SUSPECT | 177 | 179 | 2 | NONE | — |
| `func_8007D6D8` | ASM-SUSPECT | 185 | 187 | 1 | NONE | — |
| `func_8006CCC8` | ASM-SUSPECT | 187 | 189 | 1 | NONE | — |
| `func_80056CB8` | ASM-SUSPECT | 188 | 204 | 15 | NONE | — |
| `saTan3GaugeMain_8006A564` | ASM-SUSPECT | 197 | 199 | 1 | NONE | — |
| `func_800841E0` | ASM-SUSPECT | 198 | 200 | 1 | NONE | — |
| `func_8006B578` | ASM-SUSPECT | 198 | 206 | 1 | NONE | — |
| `saTan3GaugeMain_80073200` | ASM-SUSPECT | 201 | 203 | 1 | NONE | — |
| `saTan0KiWareMoveB` | ASM-PARTIAL | 203 | 212 | 1 | NONE | GTE-region:29/204 (14%) in 12 region(s) |
| `DispUpdateStatusMessage` | ASM-SUSPECT | 204 | 207 | 1 | NONE | — |
| `func_80017A44` | ASM-SUSPECT | 206 | 208 | 1 | NONE | — |
| `func_800747D8` | ASM-SUSPECT | 206 | 213 | 1 | NONE | — |
| `func_8006ECF4` | ASM-SUSPECT | 207 | 215 | 1 | NONE | — |
| `func_80074488` | ASM-SUSPECT | 210 | 212 | 1 | NONE | — |
| `func_8006CFBC` | ASM-SUSPECT | 217 | 218 | 1 | NONE | — |
| `func_8006E534` | ASM-SUSPECT | 220 | 227 | 1 | NONE | — |
| `DispPracticeMenuTex_B` | ASM-SUSPECT | 229 | 231 | 1 | NONE | — |
| `DispPracticeMenuTex_C` | ASM-SUSPECT | 230 | 231 | 1 | NONE | — |
| `md_game_check_mode` | ASM-SUSPECT | 232 | 234 | 1 | NONE | — |
| `func_80084500` | ASM-SUSPECT | 233 | 235 | 1 | NONE | — |
| `func_80077374` | ASM-SUSPECT | 234 | 242 | 1 | NONE | — |
| `func_80048BA4` | ASM-SUSPECT | 235 | 237 | 1 | NONE | — |
| `PutRobShadow` | ASM-SUSPECT | 236 | 252 | 1 | NONE | S3:no-spills |
| `replay_camera_get_attack_number` | ASM-SUSPECT | 238 | 242 | 1 | NONE | — |
| `md_game_end` | ASM-SUSPECT | 247 | 249 | 1 | NONE | — |
| `func_80075F80` | ASM-SUSPECT | 249 | 251 | 1 | NONE | — |
| `AllocBukiRmd` | ASM-SUSPECT | 257 | 259 | 1 | NONE | — |
| `func_8003FA24` | ASM-SUSPECT | 262 | 263 | 1 | NONE | — |
| `tslTm2LoadImage` | ASM-SUSPECT | 262 | 263 | 1 | NONE | — |
| `func_8006F100` | ASM-SUSPECT | 264 | 266 | 1 | NONE | — |
| `func_8005FC9C` | ASM-SUSPECT | 266 | 267 | 1 | NONE | — |
| `func_8002F2D0` | ASM-PARTIAL | 268 | 296 | 1 | NONE | GTE-region:13/270 (5%) in 6 region(s) |
| `func_80071C4C` | ASM-SUSPECT | 268 | 270 | 1 | NONE | — |
| `action_CheckHitZangeki` | ASM-SUSPECT | 269 | 271 | 1 | NONE | — |
| `special_camera_Exec` | ASM-SUSPECT | 272 | 286 | 1 | NONE | — |
| `func_8006F528` | ASM-SUSPECT | 275 | 277 | 1 | NONE | — |
| `func_8006B120` | ASM-SUSPECT | 276 | 278 | 1 | NONE | — |
| `func_80074E08` | ASM-SUSPECT | 279 | 281 | 1 | NONE | — |
| `func_80021DB0` | ASM-SUSPECT | 283 | 285 | 1 | NONE | — |
| `func_800571C0` | ASM-SUSPECT | 285 | 287 | 1 | NONE | — |
| `single_game_CheckStatusUpDataTotalOver` | ASM-SUSPECT | 287 | 289 | 1 | NONE | — |
| `func_8005E098` | ASM-SUSPECT | 288 | 289 | 1 | NONE | — |
| `func_8001C8DC` | ASM-SUSPECT | 289 | 297 | 1 | NONE | — |
| `func_800768DC` | ASM-SUSPECT | 292 | 294 | 1 | NONE | — |
| `func_80021424` | ASM-SUSPECT | 295 | 301 | 1 | NONE | — |
| `func_8007BC08` | ASM-SUSPECT | 296 | 298 | 1 | NONE | — |
| `func_8002F770` | ASM-PARTIAL | 297 | 324 | 1 | NONE | GTE-region:13/298 (4%) in 6 region(s) |
| `func_80067200` | ASM-PARTIAL | 297 | 306 | 1 | NONE | GTE-region:16/298 (5%) in 6 region(s) |
| `func_800872A4` | ASM-SUSPECT | 299 | 307 | 1 | NONE | S3:no-spills |
| `efc_rob_set_type_particle` | ASM-SUSPECT | 300 | 302 | 1 | NONE | — |
| `func_80086CF8` | ASM-SUSPECT | 303 | 311 | 1 | NONE | — |
| `func_800693CC` | ASM-SUSPECT | 306 | 307 | 1 | NONE | — |
| `func_800207C8` | ASM-PARTIAL | 315 | 325 | 1 | NONE | GTE-region:44/317 (14%) in 16 region(s) |
| `func_80087770` | ASM-SUSPECT | 321 | 335 | 1 | NONE | — |
| `func_80031B24` | ASM-SUSPECT | 325 | 327 | 1 | NONE | — |
| `func_80073728` | ASM-SUSPECT | 339 | 340 | 1 | NONE | — |
| `func_80026DA4` | ASM-SUSPECT | 340 | 350 | 1 | NONE | — |
| `md_game_rob_data_init` | ASM-PARTIAL | 345 | 351 | 1 | NONE | GTE-region:4/347 (1%) in 4 region(s) |
| `func_80053E9C` | ASM-SUSPECT | 345 | 349 | 1 | NONE | — |
| `func_8007636C` | ASM-SUSPECT | 346 | 348 | 1 | NONE | — |
| `special_camera_Init` | ASM-PARTIAL | 351 | 370 | 1 | NONE | GTE-region:54/352 (15%) in 24 region(s) |
| `func_80080828` | ASM-SUSPECT | 352 | 359 | 1 | NONE | — |
| `func_8006D808` | ASM-SUSPECT | 353 | 355 | 1 | NONE | — |
| `func_80035828` | ASM-SUSPECT | 358 | 378 | 1 | NONE | — |
| `func_80073C78` | ASM-SUSPECT | 360 | 362 | 1 | NONE | — |
| `func_800759D0` | ASM-SUSPECT | 362 | 364 | 1 | NONE | — |
| `func_80052D00` | ASM-SUSPECT | 384 | 385 | 1 | NONE | — |
| `func_8001BE20` | ASM-SUSPECT | 391 | 393 | 1 | NONE | — |
| `func_8005490C` | ASM-SUSPECT | 398 | 407 | 1 | NONE | — |
| `func_8002A458` | ASM-PARTIAL | 415 | 428 | 1 | NONE | GTE-region:4/416 (1%) in 4 region(s) |
| `func_80079244` | ASM-SUSPECT | 416 | 443 | 1 | NONE | — |
| `func_8006295C` | ASM-PARTIAL | 418 | 420 | 1 | NONE | GTE-region:1/420 (0%) in 1 region(s) |
| `saTan1MainJump` | ASM-SUSPECT | 421 | 437 | 1 | NONE | — |
| `func_80063E10` | ASM-PARTIAL | 430 | 443 | 1 | NONE | GTE-region:58/431 (13%) in 20 region(s) |
| `func_80057E84` | ASM-SUSPECT | 445 | 447 | 1 | NONE | — |
| `saTan3MainJump` | ASM-PARTIAL | 463 | 489 | 1 | NONE | GTE-region:2/465 (0%) in 2 region(s) |
| `func_80053754` | ASM-SUSPECT | 464 | 466 | 1 | NONE | — |
| `func_80032C50` | ASM-SUSPECT | 465 | 539 | 1 | NONE | — |
| `func_80077D94` | ASM-SUSPECT | 466 | 473 | 1 | NONE | — |
| `mario_test_Exec` | ASM-SUSPECT | 467 | 469 | 1 | NONE | — |
| `func_80043454` | ASM-SUSPECT | 477 | 479 | 1 | NONE | — |
| `func_800646E8` | ASM-PARTIAL | 488 | 490 | 1 | NONE | GTE-region:6/490 (1%) in 3 region(s) |
| `func_800620B8` | ASM-PARTIAL | 499 | 501 | 1 | NONE | GTE-region:1/501 (0%) in 1 region(s) |
| `func_8002DE20` | ASM-PARTIAL | 505 | 548 | 1 | NONE | GTE-region:18/506 (4%) in 6 region(s) |
| `func_8001A820` | ASM-PARTIAL | 574 | 582 | 1 | NONE | GTE-region:2/576 (0%) in 2 region(s) |
| `single_game_setModeRequest` | ASM-PARTIAL | 642 | 662 | 1 | NONE | GTE-region:48/644 (7%) in 19 region(s) |
| `func_80063084` | ASM-PARTIAL | 659 | 667 | 1 | NONE | GTE-region:6/661 (1%) in 3 region(s) |
| `func_80067D14` | ASM-PARTIAL | 1046 | 1053 | 1 | NONE | GTE-region:27/1047 (3%) in 8 region(s) |
| `motion_SetExMotion` | ASM-PARTIAL | 1453 | 1473 | 1 | NONE | GTE-region:15/1454 (1%) in 8 region(s) |

## Recommended batch order

1. **Batch 1 — owner authorization request (3 functions, A1).** `ang_hosei`, `func_8004C388`,
   `func_80052788`. All small (29–51 insns), all carrying a construct with no C form, all with
   ≤1 regfix rule. `ang_hosei` should additionally be flagged for renaming: the body after its
   `jr $ra` is the executable entry point / crt0, not the `ang_hosei` routine.
2. **Batch 2 — GTE-wrapper cluster authorization (4 functions, A2).** `func_80052930`,
   `func_80052A88`, `func_80052B7C`, `game_2d_CheckLifeGaugeNoDisp`. Same treatment as the
   existing authorized GTE wrappers; cheap to close and they clear an ASM-PARTIAL sub-cluster.
3. **Batch 3 — manual `.s` review (10 functions, B).** Read `func_8005763C` and `DispHira`
   first (highest distance + G1); then `pad_main_control` / `hirahira_w_ctrl_2` to confirm the
   ghost-save-in-C prior; then the six small-cop2-region ASM-PARTIALs.
4. **Batch 4 — return to the normal grind (137 functions, C).** These need no adjudication and
   should be re-sorted into the ordinary easiest-first queue. The 25 at distance ≤100 are the
   natural next targets, and 9 of the 137 already carry zero regfix rules.

## Follow-ups worth considering

- **The `ASM-SUSPECT` verdict is not earning its keep.** 105 of 108 resolve to compiler-emitted,
  and the three that don't were all detected by the gate's own *opcode* signals (they are
  ASM-PARTIAL, not ASM-SUSPECT). Not a single ASM-SUSPECT function was adjudicated
  canonical-eligible. Consider whether `SUSPECT_DISTANCE` should produce a verdict at all,
  versus routing straight to `C` — the label currently reads as a hand-asm suspicion the
  evidence never supports.
- **Ghost callee-save is worth adding to `scan_hand_coded.py` as a corroborating signal (not
  a STRONG-tier one).** With an 11-vs-2 enrichment it is more informative than S3, S4, or S5,
  all of which are already in the scanner.
- **Trapping arithmetic is already detected by the gate but only as a tiebreaker.** It fires on
  exactly 3 of 154 functions here and is genuinely GCC-impossible; it may deserve promotion to
  an `ASM-WHOLE`-class auto-route the way cop2 density is.

---

*Census produced 2026-08-06. Read-only pass — no tracked build files, `src/`, `regfix.txt`,
`asmfix.txt`, or `engine/queue.json` were modified, and no build was run.*
