# CD_cw — evidence (CLOSED: COMPLETED-C bba90442b, manual session s2, 2026-09-22)

## SOLVED — the mechanism (read this first)
The whole residual was the DATA MODEL, not the C. Sony's bios.c DEFINES its
module state in the TU (`static volatile CD_intr Intr = {0};`), and GCC 2.7.2
addresses a TU-defined object differently from an `extern` one: the target's
register-materialized `&Intr` / `&Intr+1` and loop.c hoisting come from the
definition. Same body, Intr as extern = 52/263; defined (static or global,
initialized) = 0 (after the relocation name difference, which links identically).
Landing: asm/data/7D920.data.s split at vram 0x800A1494 (the 4 zero bytes
0x800A1494..97 leave the asm; tail = asm/data/91C98.data.s) and bb2.ld links
system.o(.data) between the halves. Alarm = plain `extern Alarm_t Alarm;`
(named_syms.txt) — SOTN reaches it through a non-volatile view; a volatile or
aliased Alarm costs 23-35. volatile on Intr is load-bearing (non-volatile = 52).
Dead ends on the way to that: tentative definition (`volatile CD_intr Intr;`)
emits `.comm Intr,3,1`, which maspsx cannot parse (it expects `.comm sym,size`);
static .bss (`.lcomm`) crashes the same way. Layer-2 PASS.

**Transfer lead (CD_sync / CD_ready / CD_datasync / cdrom_IrqHandler):** they
reach the same bytes through FAKE-annotated `idx_1494 = &g_cd_status_a` handles.
With Intr now defined in system.c, `Intr.sync` / `Intr.ready` may reproduce their
targets honestly — a cheat-cleanup candidate for each.

---
(Session s1 notes follow, kept for the record.)


Identity: PsyQ libcd `bios.c` v1.86 `CD_cw(u_char com, u_char *param, u_char *result, int async)`
(string xref `"CD_cw"` @ 0x8001626C; memory/closer/libcd-identity.md). Reference C: SOTN
`src/main/psxsdk/libcd/bios.c` (v1.77, MIT) — fetched to /tmp/sotn_bios.c for this session.

## Honest floor: 38/263 (`candidate.c`, re-measured 2026-09-22 after the scorer dlabel fix; first recorded as 42)
Real struct-typed externs (`extern volatile CD_intr D_800A1494;`, `extern volatile Alarm_t
g_vsync_timeout_deadline;` — both already linker names, undeclared in system.c) + a
`volatile CD_intr *intr = &D_800A1494;` second handle in CD_cw and each inline helper
(pointer-alias-fake-exception shape; NOT yet FAKE-annotated — annotate before landing).
The `Intr`/`Alarm` names in the ground-truth doc are not yet linker names; landing should
add them to named_syms.txt (address-proven) and use them instead of the D_/g_ stand-ins.

## Structural facts that are SETTLED (each measured, keep them)
- SOTN structure is right: `static inline` `set_alarm` / `get_alarm` / `callback` / `_memcpy`.
  The `v0=-1 / v0=0 / bnez v0` tail is `get_alarm()` inlined; the 8-byte result copy is
  `_memcpy(result, &D_800F19A0, 8)` (`while (_size--)` gives the count-to-minus-1 loop).
  Draft with no helpers: 71 -> helpers: 43.
- BB2 v1.86 differs from SOTN v1.77: the `com == 0xE` (Setmode) `CD_mode = param[0]` is BEFORE
  the command write (SOTN does it after the wait), and the result copy is unconditional.
- nparam table: `D_800A13FC[com]` in the early check, `D_800A12FC[com + 0x40]` in the
  param-write loop (SOTN's exact spelling). The target's `addiu v0,v1,0x100` is the +0x40
  index, not a 2-D array. Using the dlabel names verbatim also avoids the
  score-symtab-blind-to-asm-data-dlabels false diff.
- Signature: the file prototypes are `CD_cw(s32, void *, void *, s32)`. A local `u8 *prm =
  param` copy costs 4 (param->s1 copy order in the prologue); index the param directly.
  The prologue matched once `prm` was dropped (E5, 23).
- Alarm as ONE volatile struct `{time, count, name}` at 0x800F19B8 fixes the timeout printf
  argument schedule (CD_com load after the stack arg): -7 on its own (E5 23 -> F2 11 with a
  cast view).

## The open residual — how `Intr` is addressed (all 38 remaining points)
The target materializes `&Intr` into a register even for the single `Intr.sync = 0` store
(`lui v0/addiu v0; sb zero,0(v0)`), re-materializes it for the pre-loop test (`la a0;
lbu 0(a0)`), and loop.c hoists `&Intr` (s2) and `&Intr + 1` (s4 = `addiu s4,s2,1`) plus
CD_intstr (s3) into the preheader (cse2 turns the hoisted `la` into `move s2,a0`). Only
an address that GCC does NOT fold into `(const (plus sym k))` produces that.

Measured (sandbox --disable all, 2026-09-22):
| spelling of Intr access | score |
|---|---|
| `*(volatile CD_intr *)&g_cd_status_a` cast view + Alarm cast view (F3) | **0** after the scorer fix (was 4, all of it the dlabel gap) — FORBIDDEN form |
| same, ready via `Intr.ready` in get_alarm (G1) / callback (G2) | 6 / 19 |
| real `extern volatile CD_intr` symbol, `Intr.sync` direct (J1/J2) | 62 |
| real symbol + SOTN's `((Alarm_t *)&Alarm)->` (K1/K2) | 56 |
| `extern volatile CD_intr X[]; #define Intr (*X)` (L1/L2) | 56 |
| `extern volatile u8 D_800A1494[]` array (I1/I2) | 34 / 56 |
| u8 member aliases `volatile u8 *ready = &Intr.ready` on cast view (H1/H3) | 25 / 31 |
| struct-pointer alias `volatile CD_intr *intr = &X` everywhere (M1/M2) | **42** -> **38** re-measured (honest floor) |

The volatile-cast view is FORBIDDEN (pointer-alias-fake-exception § "Volatile-cast
aliases"), so its 0 is mechanism evidence, not a candidate: it says the original accessed
Intr through an address expression GCC could not fold, i.e. a cast from a differently
typed object or a pointer the compiler can't see through. Decay-from-array does not do it.

## Scorer / tooling notes
- At F3 the sandbox printed score 4 but the --diff classified every hunk not-scored. Root
  cause: engine/score.py could not resolve dlabel-only data symbols (D_800A13FC etc.).
  Fixed 2026-09-22; F3 now scores 0 and the honest candidate 38. Scores in the table
  above measured before that fix (all except F3/M1) may be a few points high.
- cc1psx-check on E5 (23): cc1psx scores 31 -> SOURCE-SIDE, not a compiler gap.

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The bba90442b landing FAILed the 2026-09-29 retro-audit: Alarm_t aggregate merge incomplete and TU-local (old per-word D_800F19B8..C0 / Alarm_plus_0x4/0x8 externs still used in the same TU, prongs (c)/(d)); Intr is a second C object over the bytes the TU still reaches as g_cd_status_a/b/c. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", CD_cw);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Removed with the body: CD_cw's private static inline helpers set_alarm/get_alarm/callback. LEFT IN PLACE (shared or merge declarations, per the reopen brief): typedef CD_intr + `static volatile CD_intr Intr = {0};` (still used by getintr), typedef Alarm_t + `extern Alarm_t Alarm;` (now UNUSED), the D_800A12FC/D_800A13FC/D_80016254/D_8001625C/D_8001626C externs (now unused), the SOTN `CD_cw(u8, u8 *, u8 *, s32)` prototypes, the asm/data split + bb2.ld system.o(.data) placement, and named_syms.txt `Alarm = 0x800F19B8`.

## 2026-09-30 — laneB manual session: the Alarm merge completed (frontier = the retro-audit objection)

Starting point: the retro-audit body (rejected/retro-audit-2026-09-29.c). Its second
objection (Intr a second C object over g_cd_status_a/b/c) was cleared by the peer's
5dec24b3b (one `static volatile CD_intr Intr`). The remaining objection: Alarm_t was a
TU-local struct while CD_sync / CD_datasync still reached the same bytes as per-word
externs D_800F19B8 / Alarm_plus_0x4 / Alarm_plus_0x8 (prongs (c)/(d)).

Measurements (sandbox --disable all; multi-function runs use
probes-0930/score_full.py on a full modified copy of src/system.c generated by
probes-0930/gen.py, `Alarm = 0x800F19B8` pre-seeded in the scorer symtab as the landing
adds it to named_syms.txt; probe bodies were in tmp/CD_cw/):
| variant | CD_cw | CD_sync | CD_datasync | CdLastPos |
|---|---|---|---|---|
| A: per-word Alarm externs in the helpers (the TU's existing model, no struct) — rejected/perword-alarm-20.c | 20/263 (2 source-level hunks: CD_com load hoisted above the stack-arg store in the timeout printf) | – | – | – |
| B0: `Alarm_t Alarm` (time/count/name) for ALL consumers; CD_sync/CD_datasync respelled member-for-word, pp alias kept as `&Alarm.name` | 8 | 0/160 | 0/91 | – |
| B1: B0 with the `pp` FAKE alias dropped in CD_sync and CD_datasync (direct `Alarm.name` read) | 8 | 0/160 | 0/91 | – |
| B2: B1 + `extern u8 CD_pos[4]` (was a scalar indexed `(&CD_pos)[i]`), CdLastPos `return CD_pos;` | 8 | 0 | 0 | 0/4 |
| B3: B2 with the loop bound spelled `D_800A13FC[com]` | 31 | 0 | 0 | 0 |
| FINAL (B2 + annotations = candidate.c) | 8 | 0 | 0 | 0; every other scorable system.c function 0 |

- The pp alias in CD_sync / CD_datasync existed only for the per-word model (its
  ablations, 18/160 and 4/91, were measured there); with the struct the direct read is
  byte-exact, so the landing drops both pp aliases (one fewer FAKE each).
- CD_cw's residual 8 is ONE hunk, operand-only: `sb $zero,%lo(D_800A1495)($at)` (target)
  vs `%lo(Intr+1)` (ours) — the section-relative addend of the TU-defined static Intr,
  which the scorer cannot resolve against the splat alias (same class as
  project/sandbox-lo16-text-addend-false-distance). It links to the same address; the
  full build decides (landing step).
- Loop bound: SOTN's matched CD_cw reads the parameter count through the ready-flag
  table's base, `D_80032BE8[com + 0x40]` (src/main/psxsdk/libcd/bios.c:314 @aa53500;
  D_80032BE8 is a 32-entry static at :69, the count table D_80032CE8 at :73 lies 0x100
  bytes above; file is `[0xA398, c, psxsdk/libcd/bios]` in config/splat.us.main.yaml; no
  INCLUDE_ASM/NON_MATCHING/hack marker in the file). The target shows the same shape
  (`addiu $v0,$v1,0x100` at 0x80081460 on the kept `&D_800A12FC`). The simpler direct
  spelling `D_800A13FC[com]` measures 31 (B3). Admitted on the Q50/Q55 citation with Q53's
  FAKE annotation + this exhaustion record.
