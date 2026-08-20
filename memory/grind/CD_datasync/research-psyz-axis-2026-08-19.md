# CD_datasync (= splat `saEft01Init`, 0x80081BB0) — psyz transplant axis: assessment

Read-only research session, 2026-08-19. No tracked file touched.

**Bottom line: the psyz axis is a verified dead end, and it was already tried in a
stronger form.** psyz leaves `CD_datasync` as `INCLUDE_ASM` — there is no PsyQ-4.0
matched body to transplant. The SOTN-family C (which psyz's `bios.c` would have been
another copy of) was already transcribed into BB2 in grind session 7 and measured at
27–35 honest distance versus the banked floor of 7. The reference does not lower the
score; it never did.

However, the audit's UNCLEAR flag surfaced something else that IS actionable and has
nothing to do with psyz: **main is carrying the 18-distance chassis, not the banked
7-distance one.** See §5.

---

## 1. Residual analysis — what the park actually rests on

**Park record:** `docs/grind/decisions.md:2789`, dated 2026-08-01, filed by grind
session 17 in escalation modality. Disposition **REFUSED / OWNER-ACCEPTED INCOMPLETE**
under the owner's standing 2026-07-27 both-gates-fail auto-ruling
(`.claude/rules/endgame-lock-disposition.md`). Nothing is pending on the owner; the
driver parked it terminally and advanced.

**The two gates, and why each failed:**

- **Gate 1 (canonical-asm hand-coded signals) — FAIL.** `scan_hand_coded.py --single`
  returned tier LOW, score 1/8, and the single hit is S4 (front loads), not one of the
  STRONG S1/S2/S6 signals `hand-coded-asm-recognition` requires. The S4 window at insn
  46 is the `printf` argument block, i.e. ordinary GCC 2.7.2 `load_register_parameters`
  shape. The engine's canonical gate independently routes the function **C**. This gate
  had already been ruled on for this exact function in the 2026-07-09 01:25 DENIAL of
  the twin family `{cpu_side_move_dir_4, marionation_Exec, saEft01Init}`
  (`docs/grind/decisions.md:8`).
- **Gate 2 (in-hand SOTN-master precedent for the closing construct) — FAIL, vacuously.**
  Nothing closes the function in pure C at any distance reached, so there is no closing
  construct to cite a precedent for.

**The residual itself** (forensically established with the instrumented cc1, sessions
15–16, not inferred):

- Honest floor **7 differing instructions out of 91**, at target's exact instruction
  count — so the residual is purely positional, entirely inside build idx 46–61, the
  `printf` argument block.
- Target's block keeps the idx[0] address chain alive in `$a0` and issues
  `lw $a3,0($a0)` as the block's **last** memory reference. Every BB2 form runs that
  chain through `$v0` and issues `lw $a3,0($v0)` **early** (build idx 54).
- `rank_for_schedule`'s load/store dependence-class tie-break is **totally inert** here
  (every in-block `RANKDBG` decision returns `val=0`; 54/54 and 49/49 across chassis) —
  no C spelling can act through it.
- At sched1 the block's `INSN_PRIORITY`s are **flat** (16 of 19 picks carry
  `LAUNCH_PRIORITY`), so order is decided by latency-queue release plus `INSN_LUID` and
  nothing else. The expand-time split the residual would need is already achieved by the
  named-ADDRESS-pointer form, which still scores 9.

**Exhaustion record:** 17 sessions, floor flat at 7/91 across nine consecutive sessions
(s9–s17), six distinct modalities all measured dead, ~114,000 permuter iterations over
four fresh-seed campaigns under two objectives (including a re-aligned objective
verified monotone with the sandbox), 67 argument-block spellings over four rigid
attractors, 50+ banked rejects in `memory/grind/CD_datasync/rejected/`.

---

## 2. Identity — confirmed, and confirmed twice over

`saEft01Init` is a splat/Kengo-affinity misnomer (`docs/naming/suspect-list.md:195`
marks it RESET). The function is **Sony PsyQ LIBCD/BIOS `CD_datasync(int mode)`**,
established by the 2026-07-09 provenance census (100% of non-reloc-masked bits across
the module .text) and independently corroborated by the DMA3 CHCR access
(`*D_800A14C0 & 0x01000000` = CD-ROM channel-busy bit). The alias table entry is
`docs/grind/borderline.md:41`.

The asm at `asm/funcs/CD_datasync.s` is already renamed and now calls `VSync`, `puts`,
`printf`, `CD_flush` under their real Sony names — the old misnomer callee names in the
ledger (`sys_VSync`, `tslTm2LoadImage_2`, `cdrom_ClearIrq`, `debug_printf`) are stale
and any resumed session must translate them. Symbol map (each confirmed against the
target disassembly per the session-7 reference header):

| BB2 | Sony |
|---|---|
| `D_800F19B8/BC/C0` | `Alarm.unk0` deadline / `unk4` spin counter / `unk8` caller-name pointer |
| `D_800A11DC[]` | `CdlCom` name table |
| `D_800A11D5` | `CD_com`, current command byte |
| `D_800A125C[]` | interrupt-state name table |
| `D_800A1494` | `Intr` — `struct { u8 sync; u8 ready; u8 c; }`, so `idx_1494[0..1]` = `Intr.sync/.ready` |
| `*D_800A14C0` | DMA3 (CD-ROM) CHCR |

A separate public wrapper `CdDataSync` exists at `src/system.c:316` and
`asm/funcs/CdDataSync.s`; do not confuse the two.

---

## 3. psyz — local checkout exists; the body does not

A local psyz checkout **is** on disk (no network fetch needed or performed):
`tmp/psyq_prov/psyz/decomp/src/libcd/`. Its `CD_datasync` is a stub:

```
tmp/psyq_prov/psyz/decomp/src/libcd/bios.c:200:  INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_datasync);
tmp/psyq_prov/psyz/decomp/src/libcd/sys.c:150:   int CdDataSync(int mode) { return CD_datasync(mode); }
```

This is exactly what the 2026-08-18 sweep already concluded.
`docs/grind/psyz-sweep-2026-08-18.md` lists `CD_datasync (15 rules)` under **"Confirmed
dead ends (in-library queue items with NO psyz body)"** — all 17 remaining queue items
inside verbatim PsyQ 4.0 modules resolve to Sony symbols that psyz leaves as
`INCLUDE_ASM`. The world's only PsyQ-4.0-targeted decomp has not cracked this function
either.

**And the version-skew warning is moot here anyway**, because the transplant was already
done from the same C. Session 7 (`memory/grind/CD_datasync/ref/sotn_libcd_bios_CD_datasync.c`)
transcribed SOTN's `libcd/bios.c` body — and that header explicitly lists psyz's
`decomp/src/libcd/bios.c` as one of five sibling projects carrying the *same* source.
Measured results, all `sandbox --disable all`:

| form | score |
|---|---|
| reference hand-inlined verbatim, no helper locals | 35 / 91 |
| + three explicit table-base locals | 37 / 93 |
| `set_alarm`/`get_alarm` as real `static __inline__` | 31 / 94 |
| Sony's `volatile` on `Alarm`/`Intr`, candidate chassis | 16 / 91 |
| candidate + only the `||` short-circuit timeout test | 27 / 92 |
| proven chassis + reference statements, zero levers | 32 / 96 |
| clean + only the `k` LICM defeat | 21 / 90 |

Session 8 then swept the whole five-project corpus: **all three genuinely distinct real
`CD_datasync` spellings cost 27–35.** The reason is mechanical and known: the reference
writes a **real loop**, which hands `loop.c` the two compare constants as movables that
are hoisted unconditionally (`loop.c:1631`, threshold = 1 + n_non_fixed_regs = 61 over a
50-insn loop), costing two extra callee-saves and +5 instructions. Every reference
spelling pays it.

The reference's real value was never a lower score — it was settling **which constructs
are original**, and it indicted the then-inherited form: the original has no named
`arg4` intermediate and no `k`.

---

## 4. Idiom conventions from the solved siblings

`CdControl` (`src/system.c:134`) and `CdControlF` (`src/system.c:188`) are both
COMPLETED-C in the same TU, and they share one house style that a `CD_datasync` body
should follow:

- **Named base/element locals hoisted before the loop** (`base = g_cd_sector_buf;
  elem = base + idx;`) rather than inline global indexing — the same shape as
  `CD_datasync`'s `tbl_11dc` / `idx_1494` / `tbl_125c`.
- **A bare-`goto` back edge with explicit labels** (`loop:` / `next:` / `done:`), not a
  `while`/`for`.
- **A `do { … } while (0)` wrapper as the match device**, carrying a mandatory inline
  `/* FAKE: … */` annotation that states the mechanism (`flow.c` life analysis,
  `reg_n_refs += loop_depth`, feeding `global.c allocno_compare`) **and** cites the
  lever-exhaustion ledger path. Both siblings do this verbatim. This is the owner's
  2026-07-06 do-while(0) ruling in practice.
- All locals declared as a plain block at the top, `s32`/`u8 *` typed, no `const`.

So the banked `CD_datasync` candidate is already idiom-conformant with its solved
siblings — its `do{}while(0)` + FAKE annotation is exactly the `CdControl`/`CdControlF`
pattern. Note that per ledger frontier F23, that wrapper has **never** been through a
fresh adversarial `cheat-reviewer`; it must clear one before any completion claim, even
though the siblings' identical construct is accepted.

---

## 5. The one genuinely actionable finding — main carries the 18-chassis, not the 7

`engine/queue.json` reports `CD_datasync` at **distance 18** with 15 rules, and the body
at `src/system.c:785` matches the "inherited 18/91" chassis described in the ledger:

- no `do { } while (0)` wrapper — a plain braced block instead;
- **both** `arg4` and `arg5` named (the banked floor-7 form names only `arg4` and passes
  `tbl_125c[idx_1494[1]]` inline).

The floor-7 form is banked at `memory/grind/CD_datasync/candidate.c:357` and, per the
park entry, was measured at 7/91 as recently as session 17. It was never committed.

That is not a defect — the 15 `regfix.txt` rules are calibrated to the chassis on main,
and swapping the body would break the byte-match and require re-deriving them, which
`no-new-regfix-guard` forbids. But it means **the "distance 18" in the queue is not this
function's floor**, and any future audit reading 18 off the queue is reading the wrong
number by 11 instructions. Worth recording wherever the audit's numbers live.

## 5b. The 15-rule stack (`regfix.txt:71–92`)

Zero `asmfix.txt` entries. Zero cheat-asm in the body.

| lines | n | what |
|---|---|---|
| 71–72 | 2 | callee-save 3-way rotation: `$16 <-> $18`, `$16 <-> $17` |
| 74–76 | 3 | prologue stack-offset substs (24/32/28) |
| 78–82 | 4 | `$2 <-> $3` and `$2 <-> $4` renames over the argument block, idx 35–42 |
| 84 | 1 | `subst "0($3)" "0($2)"` @ 44 |
| 86 | 1 | `reorder 36,35,37,38,39,40,44,43,41,45,46,47,48,42 @ 35-48` |
| 88–90 | 3 | epilogue stack-offset substs |
| 92 | 1 | `reorder 72,70,71 @ 70-72` |

The prologue/epilogue substs and the two callee-save swaps are downstream paperwork for
the register rotation. **The load-bearing residual is the 14-instruction argument-block
reorder at line 86 plus the four renames feeding it** — exactly the 7 the sandbox
charges. (Note the decisions entry cites these as lines 97–119; they are at 71–92 today,
the file having shifted since 2026-08-01. Contents are identical.)

---

## 6. Recommendation

**Do not open a psyz-transplant session on this function.** The axis is closed on two
independent grounds, either sufficient: psyz has no matched body (verified on disk), and
the identical SOTN-family C was already transplanted and measured 20–28 instructions
worse than the banked floor. Re-running it would burn a session to reproduce a
2026-08-01 result. The stale-park audit's UNCLEAR should resolve to **park STANDS, psyz
ground eliminated**.

**If the function is ever un-parked anyway**, the ledger frontier — not a reference
transplant — is where a session starts, and the ledger is explicit about what NOT to
re-run (do not re-sweep argument spellings; 67 forms over four rigid attractors are
banked; do not re-probe `const`/`RTX_UNCHANGING_P`; do not launch another permuter
campaign, measured dead twice under two objectives):

1. **F29 — the sched1 replay simulator (the only live technical avenue).** Because
   sched1's in-block priorities are flat and its class tie-break is inert, sched1's
   output is a deterministic function of (pre-sched RTL order, dependence edges, latency
   queue). So the set of pre-sched LUID orders that replay to target's 14-insn sequence
   is **computable**, collapsing the search from "which C spelling" to "which LUID order,
   and can C emit it". Write `schedsim.py`; parse the block and its dependence edges out
   of `system.i.combine` / `.sched`; replay the backward pass with flat LAUNCH priorities
   and LUID tie-breaks; **validate by reproducing the observed `.sched` order for all
   four banked chassis before trusting it** (dumps on disk at
   `tmp/grind/saEft01Init/s16/`); then enumerate the pre-sched orders C can actually emit.
   A universal negative is proof-grade for the whole argument-block axis; a hit names the
   exact C to write.
2. **F30 — second chassis.** The same 14-instruction block with the same target order and
   the same early-`lw a3` failure occurs in `cpu_side_move_dir_4` (= `CD_sync`) and
   `marionation_Exec`. Run the s10 `idump.sh` / `blockdump.py` forensics on
   `cpu_side_move_dir_4` (same TU, dump already covers it) and compare its sched1 pick
   trace. Anything that behaves differently across the two is chassis pressure, and that
   difference is itself a measurement. Use as **evidence only** — the 2026-07-09 ruling
   explicitly disqualifies cluster precedent as authorization evidence.
3. **F23 — the cheat-reviewer debt.** Before any completion claim, the `do{}while(0)`
   wrapper goes to a fresh adversarial `cheat-reviewer` with the full lever-exhaustion
   record.

Sequencing: F29 first (it is the only axis that can produce a proof-grade result in
either direction), F30 as its cross-check, F23 only if a form reaches 0.

---

## 7. Claims verified vs inferred

**Verified by direct read this session:**

- psyz `CD_datasync` is `INCLUDE_ASM` — read at `tmp/psyq_prov/psyz/decomp/src/libcd/bios.c:200`.
- A local psyz checkout exists at `tmp/psyq_prov/psyz/`; no network access was used.
- The sweep classifies `CD_datasync` as a confirmed dead end with no psyz body —
  `docs/grind/psyz-sweep-2026-08-18.md`.
- Rule count is exactly 15, all in `regfix.txt:71–92`, zero in `asmfix.txt` — counted
  with `grep -c "^CD_datasync:"`.
- The park entry, both gate results, the scan output, and the exhaustion record — read at
  `docs/grind/decisions.md:2789–2999`.
- The reference-corpus measurement table (35/37/31/16/27/32/21) — read verbatim from
  `memory/grind/CD_datasync/ref/sotn_libcd_bios_CD_datasync.c`.
- The queue entry reads `distance: 18, rules: 15, status: parked` — read from
  `engine/queue.json`.
- Main's body at `src/system.c:785` has no `do{}while(0)` and names both `arg4` and
  `arg5`; the banked candidate at `memory/grind/CD_datasync/candidate.c:357` has the
  wrapper and names only `arg4`. Both read directly, diffed by eye.
- `CdControl`/`CdControlF` idioms — read at `src/system.c:134` and `:188`.
- Target asm structure (91 instructions, the `lw a3,0(a0)`-last argument block, the
  `0x1000000` CHCR test) — read at `asm/funcs/CD_datasync.s`.
- 17 sessions, floor history 18→7 with 7 flat from s9 — read from
  `memory/grind/CD_datasync/state.json`.
- 50+ banked rejects — counted in `memory/grind/CD_datasync/rejected/`.

**Reported by the ledger/decisions and NOT independently re-measured this session:**

- The honest floor of 7/91 for the candidate and 18 for main. No `sandbox` run was made —
  the engine appends to `metrics/events.jsonl`, a tracked file, and the brief was
  read-only. Both figures are consistent across `state.json`, the queue, and the park
  entry, but they are quoted, not re-derived.
- The ~114,000 permuter iterations and the 67 argument spellings.
- The cc1-internals mechanism claims (flat sched1 priorities, inert rank tie-break,
  `allocno_compare` arithmetic, `loop.c:1631` hoist threshold).
- That `docs/grind/decisions.md` cites the rules at lines 97–119 while they now sit at
  71–92: I verified the current lines and the contents match the table, but did not
  reconstruct the file history to prove nothing was added or removed in between.

**Inferred (my judgement, flagged as such):**

- That the psyz axis is closed. This follows from two verified facts (no psyz body; the
  same SOTN-family C already measured at 27–35), but "no version-correct seed exists
  anywhere" is a statement about the world that rests on the sweep's corpus scan, which I
  did not re-run.
- That committing the banked floor-7 body would require re-deriving the 15 rules and is
  therefore blocked by `no-new-regfix-guard`. Mechanically near-certain, not measured.
- The idiom-conformance reading in §4 is a style comparison, not a measurement.
