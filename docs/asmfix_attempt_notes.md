# asmfix attempt notes

Per-function notes from prior decomp attempts. When picking up an asmfix
function from the active queue (`dc.sh next-asmfix`), grep this file for
the function name first to see what's been tried and where the
last attempt plateaued.

The goal in every case is **100% pure C, byte-identical match**. These
notes are NOT permission to skip a function — they're a starting point
so each subsequent attempt builds on prior measurement instead of starting
from zero.

If you make progress on any function below, update its entry with the new
best-known diff count + what you tried that helped (or didn't). If you
match it, delete the entry (and commit the match).

---

## 2026-05-18 session attempts

### cpu_check_run_attack (317 insns, code6cac.c) — PROMISING

**Best state:** mine=314 vs target=317 (3 short), 63 STRUCTURAL + 50 REG-RENAME + 10 BRANCH-OFFSET. Already 86% matched.

**Existing body:** substantial — 4 register pins (t4, s0, s1, s2) + scratchpad writes + GTE inline asm + bisection algorithm.

**Tried:** `register s32 *data asm("t8")` + `register s32 *base asm("t3")` → REG-RENAME dropped 50→41 (9 fewer).

**Next steps:** Continue register pinning for the temp variables. The 63 STRUCTURAL diffs concentrate in the bisect block — investigate whether the if-else order or mid-x/y/z computation can be restructured. The GTE+scratchpad section is fine; focus on the pre-GTE bisection logic.

---

### func_8006B578 (200 insns, text1b.c) — STRUCTURAL BLOCKER

**Best state:** Link error on retire — undefined references to `.L8006B6B8` etc. in `asm/data/101C.rodata_text1a_b_pre.s`.

**Blocker:** `jtbl_80015988` lives in another CU's rodata file. C-emitted switch in text1b.c rodata would land at a different address than 0x80015988.

**Next steps:** Either rewrite the rodata file to use C-emitted label names (drift-prone) or restructure the splat layout so the jtbl is sourced from the right CU. See `cu-split-asmfix-drift.md` for the trap details.

---

### saTan2KabutoWareMove (215 insns, code6cac_b.c) — 4 PRIOR ATTEMPTS PLATEAU

**Best state:** Per `memory/satan2-kabuto-pin.md`: 4 prior sessions plateaued at 20 regions / 4 insns short.

**Main gap:** 17-insn miss at target_idx 126-144 — cc1 emits both if-else branches inline vs target's join-label sharing.

**Next steps:** See `memory/satan2-kabuto-pin.md` for the full progression. Restructure the if-else at line ~590 to use an explicit join label so the call sequence is shared.

---

### DispPracticeMenuTex_A (231 insns, code6cac.c)

**Best state:** 67 diffs after retire. C body is plausible-looking interpolation code.

**Diffs:** Largely $v0/$v1 register-rename cascade in `use_high` branch (idx 112-134, 23 insns) + scheduling drift of `sh D_800A3310` (target idx 54 vs mine idx 60).

**Tried:** Source reorders (move D=0 before/after new_var, split new_var into two halves, move dst[0x18]) — all neutral or worse.

**Next steps:** Probably needs register pin for use_high path or specific subst rules. Try pinning the use_high temp to a callee-save that target uses.

---

### func_80048FFC (232 insns, text1b.c)

**Best state:** 185 diffs after retire.

**Diffs:** Frame size mismatch (mine 0x90 vs target 0xA0, +16 bytes / 4 words). Adding `s32 buf[4]` dead var bumped to 0x98 but cascade diffs went UP.

**Notes:** Function has 16+ stack locals, complex loop with `initLoadImage` calls + bit-packing.

**Next steps:** Investigate which specific local target reserves that mine doesn't. The frame size suggests a missing 16-byte buffer somewhere.

---

### md_game_check_mode (234 insns, code6cac_c2.c)

**Best state:** 231 diffs after m2c body paste.

**Diffs:** m2c output for this function has incorrect pointer arithmetic (var_s1 + 2 etc.) for the function's actual layout — produces 2 extra insns and completely different codegen.

**Next steps:** Needs hand-written body from asm analysis, not m2c-paste. The loop walks a data structure via byte offsets but m2c misidentifies the element size.

---

### PutRobShadow (252 insns, code6cac_b.c)

**Best state:** Existing C body produces 248 diffs.

**Diffs:** GCC hoists scratchpad LUI loads into separate registers (mine) vs target's `lui+sw` per write idiom.

**Tried:** Converting to inline-asm `__asm__ volatile("sw $0, 0x1F800XXX")` triggered sibling label-drift cascade (func_8002EA24's regfix `.L348` reference broke).

**Next steps:** Either (a) update sibling regfix to drift-immune form OR (b) ensure new C body emits same label count.

---

### func_8006BB68 (112 insns, text1b.c)

**Best state:** 65-90 diffs after retire.

**Diffs:** Frame size mismatch (mine 0x60, target 0x68 — 8 bytes off) + complex 3-call packet+motion+texpage pattern with delay-slot stores.

**Notes:** Body needs careful packet layout (sp+0x18..0x40 is packet1 with byte at 0x40, sp+0x48..0x4F is packet2).

**Next steps:** Use explicit packet1[0x29] + packet2[4] declarations to control frame layout. Workable but tedious.

---

### coli_HitPauseKatana_2 (178 insns, main.c) — SOLVED 2026-05-22 (b233af8)

**Status:** 100% pure C, 0 diffs, 0 pins, 0 regfix, 0 asm. Tier 4. The `replace_with_asmfile` bridge is retired.

**Winning technique — "split-initial-read":** the single post-branch read through a shared `base` pointer was forcing the wrong register allocation. Duplicating the read into BOTH arms of the `D_800A2CD4 & 1` flag branch — direct `D_800F7298[...]` indexing in the set arm, `base[...]` only in the else arm — stops GCC hoisting the arg2/arg3 halfword offsets across the switch. `mode`/`mask`/`base`/accum then land in `t1`/`t0`/`a0`/`t2` naturally with NO pins. `D_800A2CD4` and `D_800A28A0` marked `volatile` to match the target's repeated loads. Reusable form: `.claude/rules/split-read-defeats-hoist.md`.

**Why the prior plateau was false:** the 32→12-diff grind (60+ variants, directed search, 13k+ permuter iters) plateaued on pure register-rename diffs with a documented "4-cluster coupling" and concluded a CU split might be needed. That floor was a STRUCTURAL artifact, not an allocation limit — a control-flow restructure dissolved it. **A register-rename plateau is a signal to restructure the C, not to add more pins/levers.**

---

### func_8005C6D0 (134 insns, text1b.c)

**Best state:** Stub returning 0 (only 2 insns vs target's 134).

**Notes:** Loops over `D_800EFB78[]` entries, calls `coli_calc_motion2` + others.

**Next steps:** Needs full body decomp from asm. Substantial work.

---

### SetPacketData (159 insns, main.c)

**Best state:** Same length (159=159), 10 STRUCTURAL + 15 REG-RENAME + 1 BRANCH-OFFSET = 26 total diffs.

**Existing body:** Substantial, with register pins.

**Tried:** Volatile cast on `D_800F1AF4` reads helped (closed 3-insn shortage from 4→0).

**Plateau cause:** Register rotation $s0/$s2/$s3 (mine uses $s2,$s3,$s0 where target uses $s0,$s2,$s3). Removing `goto main_work;` to make main_work fall-through regressed STRUCTURAL 10→41.

**Next steps:** Try pinning s1/s5/spu to specific callee-saves. The goto-restructure is a dead end.

---

### saTan1GaugeMain (166 insns, text1b.c) — STUB

**Best state:** mine=2 vs target=166 (full body missing).

**Next steps:** Needs full body decomp.

---

### func_8006DD94 (117 insns, text1b.c) — GOOD CANDIDATE

**Best state:** mine=116 vs target=117 (1 short), 18 STRUCTURAL + 2 REG-RENAME + 1 BRANCH-OFFSET = 21 total diffs.

**Existing body:** Full pure-C body written from asm analysis (`u8 packet[0x30]` + 3-iter loop + 3 chained func calls + final `func_8006D808` / `func_80069898`).

**Tried:** Volatile pinning helped (`zero_s5` added, `const80`/`const40` to s4/s3).

**Plateau cause:** Callee-save ordering ($s1 saved at idx 5 in target vs idx 12 in mine) and inline saMotionSet/initTexPage scheduling.

**Next steps:** Restructure declaration order to influence GCC's callee-save schedule. The body code is in the deferred-revert branch; can be re-applied to retry.

---

### func_8006D3DC (126 insns, text1b.c)

**Best state:** mine=130 vs target=126 (4 over), 31 STRUCTURAL + 13 REG-RENAME + 1 BRANCH-OFFSET = 45 total diffs.

**Body shape:** sibling to 8006DD94 with 5-way conditional dispatch (case_zero/case_sin/case_40/case_80).

**Tried:** Pins applied: zero_s5, const40_s2. case_80 condition is `i >= 2 && i != 3 && i == D_800A3528 + 4` — uncommon dispatch path.

**Plateau cause:** Same callee-save ordering issue as 8006DD94 ($22/$23 extra saved regs).

---

### func_80057ACC (127 insns, text1b.c)

**Best state:** mine=131 vs target=127 (4 over), 43 STRUCTURAL + 11 REG-RENAME + 4 BRANCH-OFFSET = 58 total diffs.

**Body shape:** Closest-segment-to-point search function (4 args: arg0=GameObj, arg1=entry list, arg2/arg3 passed to func_8005763C). Has outer/inner loops.

**Key insight:** Target reads pt[i] via `lh + sw 0x10(sp)` to s32 stack slots, not `lhu + sh`. Forcing `*(s32*)(sp10+off) = (s32)pt[0]` produced sign-extended s32 stores.

**Plateau cause:** Stack layout differs (mine puts sp10 at sp+0x18, target at sp+0x10) — likely needs explicit packet struct + frame-coercion.

**Useful pitfall:** `(s16)((u32)x << 16) >> 16` returns 0 for u8 x with GCC; use `(s16)entry[3]` directly. This caused GCC to eliminate the entire inner loop as dead code before fix.

---

### tslGlobalMemFree_8005C2A8 (134 insns, text1a.c) — STUB

**Best state:** mine=2 vs target=134 (stub returning 0).

**Body shape:** State-machine cleanup with 7 extern function calls (saTan2GaugeInit_80077D00, title_mv_exec2, func_80087F64, func_8005C074, func_8005C5A8, func_80088584, debug_printf, func_800859F0) + inner array loop + global state updates (D_800A3404/D_800A3408/D_800A340C/D_800EFC38/D_800EFB38).

**Next steps:** Needs full body decomp. Writing the body would take 30+ min.

---

### func_80069F80 (136 insns, text1b.c)

**Best state:** mine=130 vs target=136 (6 short), 76 STRUCTURAL + 7 REG-RENAME + 1 BRANCH-OFFSET = 84 total diffs.

**Existing body:** Wrote pure-C packet+motion+texpage body with 4 nested conditionals (if arg1&2, then if D_800A3524&8, then if arg1&1, with else branches).

**Plateau cause:** Target stores sp30 (=0x9C/0x4E) and sp40 (=1/0) to memory inside the if-else, mine optimized them to register temps.

**Next steps:** Use explicit packet writes per branch instead of local temps. Pattern matches text1b.c motion family plateau.

---

### DispPracticeMenuTex_C (231 insns, code6cac_b.c) — STUB

**Best state:** Stub returns 0 — full body decomp needed.

**Notes:** Sibling DispPracticeMenuTex_A (in code6cac.c) is named identically but Kengo comment notes "size coincidence, different stack frames" indicating different actual function body. Can't copy A's body.

---

### func_8006A1A0 (139 insns, text1b.c) — STUB (sibling pattern)

**Best state:** mine=2 vs target=139.

**Body shape:** Structurally identical to func_80069F80 (packet+motion+texpage with arg1&1 / arg1&2 / D_800A3524&8 nested conditionals, sp[0x18..0x40] packet, calls func_80073728+func_8007352C+saMotionSet+initTexPage+ot_Link). Same plateau pattern predicted (76+ STRUCTURAL).

---

### saTan0KiWareMoveB (212 insns, code6cac_b.c) — STUB (GTE)

**Best state:** mine=2 vs target=212.

**Body shape:** GTE function with 25 GTE ops (ctc2/mvmva/lwc2/swc2/mtc2). Pre-GTE has pointer subtraction (s1 vs s0 for x,y,z deltas), LUT lookup (D_8008D118), and 3 extern calls (single_game_getEnemyCharId, func_8007FA1C, func_8007F87C). Then 2x mvmva matrix-vector with $a8+0x88 setup.

**Recipe applies:** Per `gte-3x3.md` but the function has substantial non-GTE complexity.

---

### func_8005C074 (141 insns, text1b.c) — STUB

**Best state:** mine=2 vs target=141.

**Body shape:** Bit-table iterator: scans D_800EFB38 entries with srav-based bit-testing, finds best entry via threshold, then second loop iterates D_800EFC38 entries with multi-jal chain. Complex bit logic.

---

### Confirmed-stub functions (retire shows mine=2 vs target=N, no body written yet)

Each of these was retired in this session and confirmed to have a placeholder
C body (return 0 or empty). Writing the full body from asm is the next step.

| Function | Size | Source file |
|---|---:|---|
| func_80063BD0 | 144 | text1b.c |
| func_8007CBB0 | 151 | display.c |
| func_80054604 | 160 | text1b.c |
| func_80076D74 | 161 | text1b.c |
| func_8007D048 | 161 | display.c |
| func_8005BA8C | 169 | code6cac.c |
| func_8006BEC4 | 169 | code6cac_c.c |
| func_8007D6D8 | 187 | display.c |
| func_8006CCC8 | 189 | text1b.c |
| saTan3GaugeMain_80073200 | 203 | text1b.c |
| func_80017A44 | 208 | ings.c (uses gte_SetRotMatrix etc.) |
| func_80074488 | 212 | text1b.c |
| func_8006CFBC | 218 | text1b.c |
| func_8006E534 | 222 | text1b.c (needs_rodata_split) |
| DispPracticeMenuTex_B | 231 | code6cac.c |
| func_80084500 | 235 | main.c |
| func_80077374 | 236 | text1b.c (needs_rodata_split) |
| func_80048BA4 | 237 | code6cac.c |
| replay_camera_get_attack_number | 242 | code6cac_c2.c |
| saTan2Main | 247 | main.c |
| func_80075F80 | 251 | text1b.c |
| func_8003FA24 | 263 | config.c |
| tslTm2LoadImage | 263 | system.c |
| func_8006F100 | 266 | text1b.c |
| func_8005FC9C | 267 | code6cac_c2.c |
| func_80071C4C | 270 | text1b.c |
| action_CheckHitZangeki | 271 | main.c |
| special_camera_Exec | 274 | code6cac_b2.c (needs_rodata_split) |
| func_8006F528 | 277 | text1b.c |
| func_8006B120 | 278 | text1b.c |
| func_80074E08 | 281 | text1b.c |
| func_80021DB0 | 285 | code6cac.c |
| func_800571C0 | 287 | text1b.c |
| func_8005E098 | 289 | text1b.c |
| single_game_CheckStatusUpDataTotalOver | 289 | code6cac.c |
| func_8001C8DC | 291 | code6cac.c (needs_rodata_split) |
| func_8005763C | 292 | text1b.c |
| func_800768DC | 294 | text1b.c |
| func_8002F2D0 | 296 | code6cac_b.c (gte_function) |
| func_8007BC08 | 298 | display.c |
| efc_rob_set_type_particle | 302 | text1a_c.c |
| func_80067200 | 306 | text1b.c (gte_function) |
| func_800872A4 | 307 | main.c |
| func_800693CC | 307 | text1b.c |
| func_80086CF8 | 311 | main.c |
| func_80089F3C | 318 | main.c |
| func_8002F770 | 324 | code6cac.c (gte_function) |
| func_80031B24 | 327 | code6cac_b.c |
| func_80087770 | 335 | main.c |
| func_80073728 | 340 | text1b.c |
| func_80053E9C | 349 | text1b.c |
| md_game_rob_data_init | 351 | code6cac.c (gte_function) |
| func_80080828 | 354 | system.c (needs_rodata_split) |
| func_8006D808 | 355 | text1b.c |
| func_80035828 | 360 | code6cac_b2.c (needs_rodata_split) |
| special_camera_Init | 370 | code6cac_b.c (gte_function) |
| func_8001BE20 | 393 | code6cac.c (needs_delay_slot_ra) |
| func_80053754 | 466 | text1b.c |
| mario_test_Exec | 469 | code6cac.c |
| func_80043454 | 479 | text1a_c.c |
| single_game_SetStatusUpData | 516 | code6cac.c |
| func_8005D814 | 545 | code6cac.c |
| camera_set_target_zoom | 588 | code6cac.c |
| func_800841E0 | 200 | main.c |
| func_8006E534 | 222 | text1b.c (lwl_swl + jlabel_switch) |

(These are also still listed in the active asmfix queue; check
`dc.sh next-asmfix` for current ordering.)
