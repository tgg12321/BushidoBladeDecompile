---
name: sotn-family-research-2026-07-01
description: "Full-tree SOTN census + 4-project corroboration for the 5 stuck construct families. Verdicts: F1 dead stores ACCEPTED, F3 named/constant locals ACCEPTED (strongest), F4 pointer aliases ACCEPTED, F5 volatile-MMIO type-level ACCEPTED, F2 casts NOT ESTABLISHED, F6 already sanctioned (annotation gaps). Even zeldaret/oot ships annotated coercion C; only universal hard line = no non-compiled bytes. ~35-45 of 91 regressions + 4 proven blocked unlocks close under F1/F3/F4/F5."
metadata:
  type: project
---

# SOTN-evidence research: the five stuck construct families (2026-07-01)

## Why this exists

By 2026-07-01 forward decomp progress had stalled completely: the last `Match:`
commit landed 2026-06-15; the two weeks since produced only re-audit activity
(1,145 clean re-audits, 118 `reaudit-regressed` findings over 97 unique
functions, ~91 still open after the 2026-06-24 sweep). Several blocked-lane
adjudications reached the same shape of conclusion: *the target bytes ARE
C-reachable, but only via a form the current policy forbids.* The owner
commissioned a second SOTN-evidence pass (the 2026-06-02 / 06-04 / 06-07
methodology) over the five families where the auditor lens and the blocked
lane are concentrated, **plus** two measurements the June research never made:
**prevalence** (routine practice vs one-off oddity) and the **BB2 unlock map**
(exactly which regressions / walls each ruling closes).

Method: local shallow clone of `Xeeynamo/sotn-decomp` master @ `db41b28`
(1,675 `.c` files) — full-tree ripgrep census + context-reads, not rate-limited
`gh search` sampling; plus `gh search` corroboration across mkst/esa,
FoxdieTeam/mgs_reversing, pmret/papermario, zeldaret/oot, ser-pounce/
rood-reverse; plus a full parse of `docs/fleet/log.jsonl`, `regressions.md`,
`pending_owner.md`/`adjudications.md`, and all 73 `memory/wip/*/meta.json`.

## Baseline census (new numbers)

- Raw case-insensitive `fake` sweep of SOTN `src/`: 280 hits / 87 files.
  **The `FAKE` marker is overloaded 3 ways** — (a) deliberate codegen
  coercion, (b) "not sure this is right" doubt, (c) guessed literal/symbol.
  Only (a) is the announced-cheat analog.
- **Genuine codegen-coercion FAKE annotations: ~34 sites in ~30 distinct
  `.c` files**, the bulk in PSX-era GCC 2.7.2 dirs (src/dra, src/ric,
  src/st/*, src/weapon, src/main/psxsdk). (~46 annotation sites counting the
  uncertainty buckets; ~2.6% of all `.c` files carry ≥1 FAKE annotation.)
- Announced-fake **identifiers** are broader still: `FakePrim*`
  reinterpret-cast alias over `g_PrimBuf` in 10+ files, `color_fake`,
  `fakeAngle`, `field_7_fake_program`, plus 65+ `pad`/`dummy`/`unused`
  dead-local declaration sites across 44+ files.
- **Every FAKE construct found is legal C that the compiler compiles.**
  Zero hardcoded-`$N` `__asm__`, zero build-time rewrite mechanisms, zero
  register-asm pins (re-confirms [[community-standard]]'s 2026-05-30 zero-pins
  finding). SOTN's FAKE culture is *announced pure-C coercion*, never
  bytes-from-rule-text.
- **SOTN's other disposition for "no acceptable C found" is not matching at
  all:** 1,956 `INCLUDE_ASM` invocations remain in master (224 in the mature
  PSX core, 456 PSP, 1,276 Saturn). SOTN tolerates unmatched functions
  indefinitely; BB2's [[no-park-permanently]] deliberately rejects that
  disposition — this is BB2's largest divergence from SOTN, and it is a
  chosen one.

## Per-family verdicts

### F1 — dead stores / self-assignments — **ACCEPTED in SOTN**

Literal self-assign is rare but real: `src/dra/alu_anim.c:175`
`idxSub = idxSub;` (PSX; "Probably fake. Makes PSP registers match." comment
above), `src/main_psp/1BCFC.c:499` `ptr = ptr; // FAKE!` (sits AFTER a
`return 0;`), `src/main_psp/mpeg/display.c:175` `ge_addr = ge_addr; // FAKE`.
Dead stores: `src/dra/66590.c:390` `dest = val1; // fake` (never read),
`src/dra/menu.c:1993,2009,2017` `j = menu->unk1D; // FAKE?`,
`src/weapon/w_049.c:293` `i = 0; // FAKE? why here?`,
`src/st/no0/clock_room.c:309` `primIndex = g_Player.status; // FAKE`.
Dead round-trips (F1-in-spirit) in `src/dra/84B88.c:817-884` /
`src/dra/7879C.c:3038-3044` (`spriteX = four + ptr[0]; ... width = spriteX -
four;`).

**Corroboration:** zeldaret/oot ships `rtile = rtile; // Fake match?`
(`src/code/PreRender.c:493,500`); MGS ships "Fake match/hack no reason to
assign these again but no match if we don't" (`source/weapon/socom.c:89-91`)
and a dead pointer round-trip (`source/libdg/divide.c:545-548`).

**Erratum this evidence exposes:** [[register-alloc-pure-c]] Lever-D's
rationale line "SOTN's bar rejects them" is factually wrong — SOTN master
ships dead stores with FAKE annotations. The Lever-D *prohibition* is a
legitimate BB2-stricter-than-SOTN choice, but its stated justification
misattributes it to SOTN.

### F2 — redundant width casts — **NOT ESTABLISHED in SOTN**

The honest negative result. Double-same-cast: 0 hits. The greppable
cast-in-comparison instances (12) are mostly semantically defensible
(`== (s16)ANIMSET_OVL(n)` width-forcing). One `// FAKE` cast exists
(`src/main/psxsdk/libcd/event.c:26-27` `return (float)count; // FAKE`) but
prevalence cannot be demonstrated. **Recommendation: no ruling needed** —
see impact: the BB2 F2 findings close by ordinary cleanup anyway.

### F3 — suspicious locals / named temps / constant-holders — **ACCEPTED (strongest evidence)**

- m2c-artifact names committed as-is: `new_var` — 80 occurrence-lines across
  9 files (8 PSX-era): `src/dra/42398.c:280` `sprite->u0 = (D_801362B4 & 1)
  << (new_var = 7);`, `src/dra/cd.c:520-552` (`int new_var2; s32 new_var;
  CdThing* new_var3;`, `new_var2 = 6` used once as a shift amount).
- Hand-named constant-carrier locals: `src/dra/84B88.c:817-860` `s16 three
  = 3; s16 one = 1; ... unk7E.val += three; unk7C += one;`;
  `src/dra/7879C.c:2067` `s32 zero = 0; // needed for PSP`;
  `src/st/cat/e_hellfire_beast.c:615,819-823` `s32 fake; ... // FAKE! PSX
  requires some trickery here, was not able to resolve` / `fake = 8;
  prim->drawMode = fake;` — a **constant-holder local named `fake`**, the
  exact shape of the `DispSamnailWindow` regression finding.
- Pass-through intermediates: `src/weapon/w_037.c:300-302` `// FAKE but
  makes register allocation work / randy = basePoint.x; baseX = randy;`.

**Corroboration:** papermario keeps a match-temp with "Difficult to not set
the temp in the for conditional"; oot keeps `unk2C7 = this->unk_2C7; // temp
probably fake match`.

### F4 — pointer-alias intermediates (second C handle to a global) — **ACCEPTED / idiomatic**

- `src/boss/mar/CA94.c:23-24` — `// n.b.! unused, required for PSP` /
  `tilemap = &g_Tilemap;` — an **unused alias kept solely to match**, with
  in-source admission.
- `src/st/no0/e_stone_rose.c:611` — `Entity* fakeEntity = self; // !FAKE`
  then `EntityExplosion(fakeEntity);`.
- `src/dra/cd.c:539` — `new_var3 = &g_Cd;` used once where `g_Cd` would do.
- Repeated idiom: `Tilemap* t = &g_Tilemap;` (`src/dra/4CE2C.c:63`,
  `src/st/cen/e_chamber.c:56,72`), `pLoadFile = &g_LoadFile;`
  (`src/dra/cd.c:646`), `PlayerStatus* status = &g_Status;`
  (`src/boss/mar/clock_room.c:22`).
- The `FakePrim*` reinterpret-cast family (10+ files, incl. `#define
  FAKEPRIM ((FakePrim*)prim)`) is a tree-wide announced type-alias lever.

Raw `= &g_...` count is 160, but only ~40-55% of the sample is the
"redundant second handle" shape (rest: conditional pointer selection and
file-scope initializers — legitimate). Still: 10+ verified-genuine instances
incl. one self-admitted-unused. **Scope note:** this covers C-level pointer
aliases ONLY; the `extern T G_v asm("G")` alias-RENAME stays forbidden (no
SOTN analog exists — that form is not C-level aliasing, it is symbol-table
manipulation).

### F5 — volatile for MMIO / IRQ probes — **ACCEPTED, via TYPE-LEVEL declaration (not use-site shapes)**

The decisive structural finding: **SOTN does not gate volatile per use-site
at all.** Hardware registers are declared volatile ONCE, at the pointer
declaration, and every read inherits it:

- `src/main/psxsdk/libetc/intr.c:42-44` — `static volatile u16* i_stat =
  (u16*)0x1F801070; static volatile u16* g_InterruptMask = (u16*)0x1F801074;`
- `src/main/psxsdk/libcd/registers.h:4-7`, `libapi/counter.c:15-16`,
  `libetc/intr_dma.c:12-13`, `libgpu/sys.c:88-95` — same idiom throughout.

Consequently **single-read MMIO probes are volatile in SOTN with zero
per-site justification**: `intr.c:69` `u16 GetIntrMask(void) { return
*g_InterruptMask; }`; `libgpu/sys.c:549` `u_long _status(void) { return
*GPU_STATUS; }`; and `libgpu/sys.c:889` `return ((u_long)*GPU_STATUS >> 0xC)
& 1;` — read-once/test-bit/return-bool, the exact structural analog of
`func_80078EC0`. SOTN does NOT use EnterCriticalSection as a probe-read
substitute (it brackets multi-step mutations only).

**Implication:** the [[sotn-volatile-interrupt-research-2026-06-07]]
three-shape use-site gate was derived from `extern volatile` on *game-state
globals*; for *hardware-MMIO-range* addresses (0x1F80xxxx) SOTN's precedent
is simpler and broader: **volatile by type, all shapes, including single
reads.** A type-level MMIO carve-out (hardware address range ⇒ volatile
pointer/decl is legitimate, no shape test) would be strictly SOTN-parity.
The game-state-global two-prong criterion stays as-is.

**Explicit caveat:** this does NOT close `func_80078EC0` — its wall is GCC's
jump-opt boolean fold, not load CSE (adjudicated 2026-06-24). It may unlock
sibling IRQ probes and closes the `spu_InitEx` cluster (see impact).

### F6 — do-while(0) / empty blocks — **already sanctioned; findings are prerequisite gaps**

12 open F6 regressions are dominated by *missing `/* FAKE */` annotation /
missing lever-exhaustion ledger*, not construct disputes —
[[do-while-zero-exception]] already covers the mechanism. `func_80044670`
is PROVEN load-bearing (strip ⇒ 0→4) via the exact reorg.c peephole the
exception sanctions. Empty-if evidence also present in SOTN
(`src/dra/5D5BC.c:768-772` `// !FAKE, permuter found it`; `src/dra/cd.c:536`
`if (!g_Cd.D_80137F74 && !g_Cd.D_80137F74) { }` — empty body AND duplicated
condition; `src/ric/pl_steps.c:124-126` nested empty if). Recommended
disposition: where the mechanism is evident, add the annotation + ledger
retroactively instead of demanding re-derivation; the two always-true-wrapper
hybrids (`func_8008BEA4`, `func_8008C184`) still need their F0 wrapper fixed.

## Community corroboration (is SOTN an outlier?)

**No — SOTN is the norm.** All five projects surveyed ship annotated
no-semantic-purpose coercion C in committed matched code: mkst/esa
(`if (D_800E5274->unk386) {}; // fakematch`), MGS (`socom.c` redundant
re-assign "no match if we don't"), papermario (written policy: label
NON_MATCHING/NON_EQUIVALENT functions and "please leave a comment"; ships
`// TODO fake match` tricks in matched code), **zeldaret/oot — the strictest
reference project — ships `rtile = rtile; // Fake match?`, no-op `& 0xFFFFFFFF`
masks, and match-temps**, Vagrant Story (`// Nasty match hack`). The
near-universal gate is a COMMENT, not a prohibition. What projects formally
restrict is the orthogonal axis: shipping non-matching/non-equivalent code.
**No surveyed project ships hardcoded-`$N` asm injection or any
regfix-analog** — the "no non-compiled bytes" line is the community-universal
hard rule, and BB2 already draws it.

## BB2 impact map (what each ruling unlocks)

Of 97 unique flagged functions (2026-06-17→06-29 re-audit), 6 cleaned
2026-06-24, ~91 open. Primary-family bucketing (full lists in
`docs/fleet/log.jsonl` parse; multi-family cases counted once):

| Family | Open regressions | Proven blocked/WIP unlocks | Realistic closes under ruling |
|---|---|---|---|
| F1 dead stores | 11 | `func_8006E49C` (0→35) | ~8-10 (rest: constructs GCC already DCEs — close by removal, no ruling needed) |
| F2 casts | 10 | — | 0 needed — `func_80065xxx` cluster closes by retyping ~12 s16→u16 globals; shift-pairs are F0 |
| F3 locals/temps | 18 | — | ~12-15 (`DispSamnailWindow`, RA/scheduling coercion set, param-alias set) |
| F4 ptr aliases | 10 | `func_800455AC` (0→20), `func_80082C58` | ~8-10 |
| F5 volatile | 10 | `spu_InitEx` cluster (3 fns; volatile removal breaks full build −40B) | ~6-8 (several are auditor-verified inert — close by removal) |
| F6 do-while/empty | 12 | `func_80044670` (0→4) | most, via retroactive annotation + ledger |
| F0 other | 26 | — | 0 by ruling; ~7 trivial retypes/redecls, ~5 structural redos, rest hard walls |

**Aggregate: F1+F3+F4+F5 rulings ⇒ ~35-45 of ~91 open regressions close
(mostly mechanically: annotate + document instead of revert), plus 4 proven
blocked/WIP unlocks.** A further ~15-20 close by ordinary cleanup regardless
of any ruling (inert constructs, retype cluster).

**What NO family ruling closes** (stays hard, honest walls):
- `func_8003B10C` — same-global split-read plateau (floor 10 even with F5).
- `func_80078EC0` — boolean-fold wall; closing Variant F needs a register
  PIN + dead store; pins remain forbidden (zero community precedent).
- `motion_SetMotion` (+ saEft00Add / marionation_Exec / cpu_side_move_dir_4 /
  func_8007C97C cluster) — RA-priority + cross-jump coupling; closing lever
  is pin+regfix.
- The F0 pin/regfix/canonical-asm WIP set (12 fns listed in the impact-map
  session log).

## What this research does NOT support relaxing

1. **Register-asm pins** — zero instances in SOTN or any surveyed project.
2. **Hardcoded-`$N` `__asm__` injection** — no community analog.
3. **regfix/asmfix-style build-time rewriting** — no community analog;
   "bytes must come from compilation" is the universal line.
4. **`asm("sym")` alias renames** — no community analog.
5. **F2 redundant casts as a sanctioned technique** — evidence insufficient.

## Owner decision matrix (rulings requested)

Per family, the options mirror the do-while-zero operationalization template
([[sotn-do-while-zero-research-2026-06-04]] → [[do-while-zero-exception]]):
each ACCEPT = a narrow rule file with (a) SOTN citations, (b) mandatory
`/* FAKE */`-style annotation at the construct, (c) demonstrated
lever-exhaustion prerequisite (construct is last-resort, not first reach),
(d) explicit non-extension language, (e) detector + cheat-reviewer
family-check updates.

1. **F1 dead stores/self-assigns** — ACCEPT (SOTN + oot + MGS evidence) or
   KEEP FORBIDDEN (BB2-stricter-than-SOTN; then fix the erroneous "SOTN's bar
   rejects them" rationale in [[register-alloc-pure-c]]).
2. **F3 named/constant-holder locals** — ACCEPT (strongest evidence) or KEEP.
3. **F4 C-level pointer aliases** — ACCEPT (asm-rename stays banned) or KEEP.
4. **F5 type-level MMIO volatile** (hardware range ⇒ volatile decl legit, no
   shape test; game-state globals keep the two-prong gate) — ACCEPT or KEEP.
5. **F6** — no new sanction needed; ruling requested on *retroactive
   annotation* as the cure for the 12 findings (vs re-derivation).
6. **F2** — recommend NO sanction (insufficient evidence); findings close by
   cleanup.

Also surfaced (no action taken): SOTN's INCLUDE_ASM-indefinitely disposition
(1,956 functions) vs BB2's [[no-park-permanently]] — BB2 is deliberately
stricter here; the hard-wall set above is where that difference costs the
most.

## Related

- [[sotn-borderline-research-2026-06-02]] — the June evidence this extends
  (individual examples → prevalence + impact).
- [[sotn-do-while-zero-research-2026-06-04]] / [[do-while-zero-exception]] —
  the operationalization template.
- [[sotn-volatile-interrupt-research-2026-06-07]] — the two-prong game-state
  carve-out F5 would sit beside, not replace.
- [[no-new-park-categories]] — the policy framework; each ACCEPT here follows
  its "clear its own SOTN-master-branch evidence bar" clause.
- [[community-standard]] — zero-pins finding re-confirmed.
- Session agents' full logs: FAKE census, construct sweep, volatile probe,
  impact map, cross-project corroboration (2026-07-01 session transcripts).
