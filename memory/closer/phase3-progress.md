# Closer Phase 3 — PsyQ psxsdk adoption progress ledger

## SESSION 6 (2026-07-10) — S_SCA close + S_SRMP near-close

**Review trail:** layer-1 cheat-reviewer initially FAILed func_8008AF9C on the
threshold gate (read the 229-vs-281 extent split as a non-match and distrusted
the masked comparator). Re-invoked with NEW evidence per review-discipline:
tmp/closer/link_sim.py — a relocation-RESOLVING link simulation compared
BIT-EXACT (zero masking) against disc/SLUS_006.63 itself: 0/281 text words and
0/16 jtbl words differ. Fresh reviewer independently reproduced every
measurement (score.py mechanism, objdump symbol layouts, link_sim re-run,
census cross-check) and PASSed. func_80089F3C's WIP body also PASSed its own
scoped review (struct adoption + MMIO volatile + degenerate switch all
cleared). link_sim.py is now THE local proof tool when sandbox-0 is
structurally unreachable before a reference rebuild.

**Unblock implication for the multi-static splice regions (func_80082D34
trapIntr + DispStuff):** statics DO get local F symbols (extent splits), BUT
the claim path works anyway — prove bytes with link_sim vs the EXE, claim, let
the driver strip + rebuild (reference symbol layout then matches and sandbox
reads 0). The session-5b "blocker" is procedural, not physical.


**func_8008AF9C (SpuSetCommonAttr, was dist-279 whole-body asmfix splice) —
CANDIDATE-COMPLETE, claimed.** Full SOTN s_sca.c transcription (4.0 block
order: mvolL/R, cdvolL/R, extvolL/R, cdrev/mix, extrev/mix) PLUS a static
`SpuRGetAllKeysStatus` after it (Sony SR_GAKS DEAD CODE at 0x8008B330-B3FC —
unreferenced, no glabel, shares the splat extent; loop body mirrors the
matched sibling func_8008B400's off/bit idiom, which killed the only real
register-rotation residual).
- **Sandbox reads 54 — ENTIRELY the extent-split artifact**: the static gets
  its own local F symbol (+.size), so my func_8008AF9C extent is 229 insns vs
  the spliced reference's 281 (proven this session: statics DO get F-typed
  local symbols through the pipeline — tmp/closer/statictest.sh — answering
  the session-5b open question NEGATIVELY for the multi-static splice
  regions; the extent SPLITS, but the score self-heals once the driver's
  rebuild gives the reference the same symbol layout).
- **Byte proof** (tmp/closer/region_cmp.py, masked word compare over the full
  0x464 region): 251/281 bit-exact + 26 reloc-field-identical + 4 assembler
  artifacts (local-label branch vs PC16 reloc in the spliced ref — displace-
  ments verified equal, e.g. 0x1A at +0x28). Rodata verified word-identical:
  the 4 SPU debug strings MOVED up before func_80088740 and the hand
  jtbl_80016420/40 consts DELETED — the real switches now emit the tables at
  .rodata+0x60/+0x80 with R_MIPS_32 .text relocs resolving to the target
  values.
- **DRIVER MECHANICS (load-bearing):** the asmfix splice (asmfix.txt:56) MUST
  be stripped in the same commit — the spliced asm references the deleted
  jtbl symbols, so any rules-ON build of this tree is un-linkable until the
  rule is gone. Sandbox-0 will only read 0 after the reference rebuild
  (saTan5TakeAnim2_2 precedent).

**func_80089F3C (SpuSetReverbModeParam, was dist-316 whole-body splice) —
banked at 3 REAL words** (memory/closer/candidates/
spusetreverbmodeparam_struct.c; body left in src/ — SAFE, its splice rule
stays active). 94 → 33 → 23 levers, all measured:
- **SpuRevAttr struct adoption** (94→33): Sony declares _spu_rev_attr as ONE
  struct (sotn libspu_internal.h:87); anchored `extern SpuRevAttr D_800A2888`
  (added to undefined_syms_auto.txt). Struct-member MEM semantics produce the
  target's store-then-reload of .mode (scalar spelling CSE-folds it — that
  was most of the 94).
- **Degenerate switch for the mode gates** (33→23): `switch (mode) case 7:
  case 8:` — expand_case emits the two-slti bound checks; the `< 9 && >= 7`
  if-spelling gets tree-range-folded to addiu/sltiu. New reusable finding.
- **Residual 3 words**: sp58's spill slot sp+0x5C vs target sp+0x58. Named
  mechanism: our reload runs TWO spill passes ("Spilling reg 8/64" ×2 in the
  greg dump) leaking two transient slots before sp58's; Sony's compile leaked
  one. All 315 other words + every register exact. Killed: flag-init reorder
  (rotates s-regs, 31), shared-quotient local (inert). The other 20 masked
  diffs are struct-reloc addends (self-heal class).

**Method artifacts:** tmp/closer/region_cmp.py (masked word-level region
comparator vs reference .o — THE tool for extent-artifact/struct-adoption
proofs), tmp/closer/statictest.sh (static-symbol pipeline probe),
tmp/closer/srmp_dump.sh (per-function greg extraction from the sandbox src).


## SESSION 5b (2026-07-10) — post-grant batch

**Completed (sandbox --disable all this session, edits in src/main.c;
layer-1 cheat-reviewer PASS on all three):**

| queue item | Sony name | sandbox | notes |
|---|---|---|---|
| func_80085270 | _SsSndStop (LIBSND SSSTOP) | 0 | was PARKED "scheduler-only floor" — closed with two honest respellings (see below); unmasked-IDENTICAL incl. registers; 2 rules to strip |
| func_800856B0 | _SsSndTempo (LIBSND TEMPO) | 0 | sotn tempo.c chassis + 4.0-interim early-exit; 65 rules to strip; unmasked-IDENTICAL |
| saTan5TakeAnim2_2 | _SsStart (LIBSND SSSTART) | 0 | NO new edits — session-2 committed body now measures 0 (reference .o caught up after the driver's commits); claim-only, unmasked-IDENTICAL |

**func_8008AAD4 (SpuSetKey) — applied, measured 8 (all addend-class,
byte-identical after link, 127/127), then REVERTED to HEAD on layer-1
cheat-reviewer FAIL.** Reviewer grounds (full verdict in its output; both
independent): (1) `extern volatile u16 D_800F7420[4]` is an ARRAY-typed
volatile extern — [[legitimate-volatile-interrupt-touched]] item 5 is
scalar-only ("FAIL any non-scalar-extern spelling"); (2) the §2 grant's
claimed async writer coli_HitPauseKatana_2 is an ordinary C function
called synchronously via jal from func_80089A24/func_8008A904 — the
reviewer found NO SysSetCallback/InterruptCallback/MMIO installation
evidence, i.e. the "VSync-tick/IRQ path" in the proposal was asserted, not
demonstrated. This CONFLICTS with the operator-ratified grant commit
de188634 (allowlist :40-44 explicitly ships the array decl). Per
review-discipline, FAIL is never bypassed → reverted; the conflict is
surfaced as this session's ruling question. Blast-radius measurements
while applied (for the record): spu_InitEx 0 with D_800A289C volatile;
func_80088740 unchanged at 56 with the [4] merge. The proven candidate
remains banked at memory/closer/candidates/spusetkey_v40_proven.c.

**RULING NEEDED (owner):** does the operator-audited §2 grant stand
(reviewer objections overruled — then next session re-applies the banked
candidate verbatim, ~5 minutes to sandbox-8/oracle), or does the reviewer's
reading control (then the grant should be pulled from the allowlist and
SpuSetKey needs either a verified interrupt-installation citation for the
_spu_RQ family or a volatile-free pure-C derivation)? Key factual question
to settle it: is coli_HitPauseKatana_2 (Sony _spu_gcSPU-adjacent flush)
reachable from an interrupt/DMA-callback context, or only from synchronous
gameplay calls? (exec_game = _spu_gcSPU @0x800896A0 is its own queue item —
its callers may answer this.)

**_SsSndStop levers (killed the 2026-06-07 park reason):**
- Diff A (sll,sra,lui,addiu → sll,lui,addiu,sra): the COMPLETED sibling
  spu_SetMotionActive's exact two-local prologue — `s32 shifted = a0<<16;
  s32 *addr = (s32*)&D_80106F28; base = addr + (shifted>>14);` — the park
  session tried each half separately, never the sibling's combination. 5→3.
- Diff B (li t2/li t1 before the copy-loads, move t0,s0 after): the t0
  walking pointer is a strength-reduction GIV, not a source local — respell
  `*(s16*)((u8*)hp+0x60)=0x7F; hp++;` as indexed `*(s16*)(p + i*2 + 0x60)
  = 0x7F;` (Sony stop.c: `score->vol[i] = 127`). Loop body unchanged
  (recomputed `ip = p+i` kept); giv init lands after the hoisted constants
  in RTL → target order. 3→0.

**_SsSndTempo levers (65 → 0 in 4 steps):**
- SOTN tempo.c chassis + BB2 4.0-interim delta (counter<0 early clear+return). 65→20.
- Late flag-clears: fresh SHARED `tbl2` local (fresh full recompute incl. la,
  but base CSE'd across the two &=~0x40/&=~0x80 statements); reusing the
  prologue's `shifted`/`addr` locals over-CSEs (target recomputes from a
  t1-copied a0); a fully-inline expression under-CSEs (at-macro form). 20→12→5.
- ++/-- diamond: HEAD's m2c labeled-store form (`new_val = cur-1; goto
  tempo_store;` into the second if's body) — produces the bnez-into-shared-
  tail + delay-slot addiu; the plain else-if spelling leaves an unmerged
  j/sw pair. 5→2.
- Clear-block prologue: same two-local shifted2/addr2 spelling as diff A. 2→0.

**Multi-static splice regions identified (NOT quick wins — dedicated sessions):**
- func_80082D34 (dist 295, 1 asmfix splice, 0x4A4 bytes): the WHOLE rest of
  LIBETC INTR — trapIntr + setIntr@0x80082F1C + stopIntr@0x80083070 +
  restartIntr@0x8008311C + memclr(func_800831A4) + 2 junk tail words
  (0x15007350/0x0040809C @0x800831D0 — original Sony object tail/next-module
  data, NOT compiler-emittable). setIntr/stopIntr/restartIntr are referenced
  ONLY by raw .word in 7D920.data.s (0x800A25E8/F0/F8) — no symbolic callers.
  BLOCKER to sandbox-0: engine score extracts my .o's func_80082D34 extent
  = symbol offset → next F-typed symbol (engine/score.py:_o_func_table);
  separate C functions break the 0x4A4 extent. Open question: do static
  functions get F-typed .o symbols through this pipeline? (If NOT, statics
  keep the extent intact = the path to close.) Sony C is fully known
  (sotn intr.c v1.73; v1.76 has the startIntr arg delta).
- DispStuff (dist 207, target 209 insns = 0x80083E9C..0x800841E0): SsStart +
  the whole SSCALL module. Same splice category.

**LIBGTE queue items are NOT pure-C adoptable:** func_8007E1AC (LoadAverage12)
is hand-written GTE asm (mtc2/lwc2/gpf/gpl, splat "handwritten instruction"
markers) — the whole LIBGTE cluster (E1AC/E1FC/ED6C/F24C/F2DC/E4DC/EB4C/
E8DC/EA0C/E74C) is canonical-asm territory needing owner authorize routing,
not C grinding.

## SESSION 4 (2026-07-10)

**Completed (sandbox --disable all == 0 this session, edits in src/):**

| queue item | Sony name | proof | notes |
|---|---|---|---|
| func_80082A14 | v_wait (LIBETC VSYNC static) | 0 + oracle-green in place | Ruling 3 candidate applied verbatim (asm barrier removed) |
| func_80078E58 | _Pad1 (LIBAPI PAD static) | 0 + oracle-green in place | Ruling 3 candidate (unwritten pad[2] removed) |
| func_8008BEA4 | SioAnsyncRead (LIBCOMB static) | 0 + oracle-green in place | granted volatile B00/B04; flag&&flag wrapper removed |
| func_8008C184 | SioAnsyncWrite (LIBCOMB static) | 0 + oracle-green in place | granted volatile AF0/AF4/AF8 |
| D_80083418 | trapIntrDMA (LIBETC INTR_DMA static) | 0 (12 rules to strip) | SOTN intr_dma.c literal while/for; goto-form + one/mask_const opaques removed |
| D_8008359C | setIntrDMA (LIBETC INTR_DMA static) | 0 (20 rules to strip) | SOTN literal; `extern volatile u32 *D_800A263C` (Sony's volatile-MMIO-pointee decl); conv_matrix_rotation respelled `*D_800A263C = 0` — re-measured 0 |

**KEY FINDING — session-3 label-drift blocker DISPROVEN:** SetPacketData is
a whole-body `replace_with_asmfile` splice (asmfix:130) with address-named
labels (.L8008Cxxx) — label-count-invariant. asmfix:139-140 (.L761/.L764
replace_first) are INERT (no .L761 in the spliced text; sandbox with rules
ON scores 0 under 96-item cheat-strip label renumbering). Full build SHA1
== oracle measured THIS session with the 4 rule-free edits in place. The
Ansync pair does NOT need the Syncro pair to land. exec_game's .L rules are
`\.L\d+` regex (drift-robust). Only literal-.L rules in main.c were
SetPacketData's two inert ones.

**Full-oracle caveat:** after the DMA-pair edits (rule-carrying), local
verify-oracle is NOT runnable (old rules mis-apply to new bodies) — the
driver's strip-then-build is the proof, per session-2 precedent.

**Layer-1 cheat-reviewer: PASS on the 4-function rule-free batch** (v_wait,
_Pad1, SioAnsyncRead/Write) — IRQ-writer citations independently verified
against asm/funcs/func_8008C464.s. DMA pair uses only type-level MMIO
volatile (Sony's own `volatile u_long *` pointee decl) — no new grants.

**saEft01Init (CD_datasync) — measured, NOT closable this session (banked):**
- HEAD goto-accumulator chassis is structurally CORRECT (the v0=-1/0 join,
  in-loop 0x3C0000/0x1000000 constants, three hoisted bases) — residual 18
  = (a) param seat s0-vs-s2 allocation rotation + (b) the do_timeout window
  seats — the SAME contested family as the twins (evidence.md s36/s39/s53).
- Faithful SOTN while(1)+ret form: 35 (constants hoist to callee-saves,
  window loses base caches). NESTED-inline get_alarm: 52 (nested fn forces
  captured locals to STACK — kill that approach). volatile intr ptr
  (&g_cd_status_a) + staged sync-first args: 18 (lbu order fixed, deref
  order wrong). Direct printf args + volatile intr ptr: 24 (com lbu
  hoists early where target schedules it after the ready deref; window
  temps a3/v0/v1 rotated). src/ reverted to HEAD.
- KEY positive: `volatile u8 *intr = &g_cd_status_a` reproduces target's
  cached-base `lbu a0,0(s1); lbu v0,1(s1)` SHAPE (HEAD's non-volatile
  D_800A1494 alias also does); the volatile fixes the lbu ORDER (sync
  first). What remains is deref scheduling + the seat rotation — same knot
  Phase 2 owns for the twins; solve there once, mirror here.

**func_8007C4B8 (SetDrawEnv2, dist 4):** residual = the two prologue
save/move pairs swapped (target homes a1->s0 before a0->s1); its 1 rule is
`reorder 3,4,1,2 @ 1-4`. This is the documented C2A0/C4B8 prologue-order
wall; the alias lever is the ARCHIVED-FORBIDDEN
param-local-alias-prologue-pair-flip. Not attempted further.

**func_8008BB24 (_spu_note2pitch, 39):** BB2 links the 4.0 curve-scan
algorithm (0x103B semitone multiplier loop) — SOTN s_n2p.c is the
table-lookup revision, NOT transplantable (same finding as func_8008BC60
in session 2). Needs self-decomp from ground-truth object.

## SESSION 3 (2026-07-10)

**Completed (measured this session, edits in src/, full build SHA1 == oracle
with everything in place; layer-1 cheat-reviewer PASS):**

| queue item | Sony name | sandbox | notes |
|---|---|---|---|
| func_8007A28C | memmove (LIBC2/MEMMOVE) | 0 | SOTN memmove.c verbatim (`while (n-- > 0)` both arms); 17 regfix rules now no-op |
| func_80082C58 | startIntr (LIBETC/INTR static) | 1* | faithful intrEnv_t struct at D_800A1578; *the single masked diff is `%lo(D_800A1578+60)` vs `%lo(D_800A15B4)` addend spelling — byte-identical, proven by the oracle SHA1 |

**startIntr levers (all measured):**
- `setjmp` TRUE-NAME RENAME (func_80083220 → setjmp; glabel + decl +
  inline_asm_canonical.txt): GCC 2.7.2 keys returns-twice on the literal
  callee name — with the real name the compiler stops caching &intrEnv
  across the call and re-anchors at &buf[1] (29 → 8). The -60/-4/+0xFDC
  spellings then fall out of related-address CSE (no source arithmetic).
- `*D_800A2604 = (*(volatile u16 *)g_sys_irq_counter = 0);` single-statement
  chained MMIO store+re-read (8 → 4; staging the mask ptr in a local first
  loses the load order).
- `_96_remove` arg evidence: SOTN v1.73's plain `_96_remove();` compiles to
  $v1 for the second pCallbacks load (measured, tmp/closer/intr_test.c);
  Sony's v1.76 object has $a0 with the value live into the call → v1.76
  source passed the pointer. Spelled `r = conv_matrix_rotation(); cb =
  g_sys_irq_vtable; cb[1] = r; bios_CdRemove_A0(cb);` (call-first so cb
  doesn't cross the call; the arg-copy gives the a0 qty suggestion). 4 → 1.

**KEY METHOD FINDING — sandbox strips un-granted volatile (measure physics
standalone!):** the s-2 banked negative "volatile did NOT hold store order"
for the SIO pair was an artifact — `extern volatile` on non-allowlisted
symbols is cheat-invisible (stripped before scoring; cheat_asm_stripped
count rises when you add them). Real volatile compiles to EXACTLY the
target's strictly-ordered stores. Standalone proof pipeline:
tmp/closer/diffsa.sh (cpp|cc1|prologue_fix|maspsx|as + masked diff vs
build/src/<file>.o).

**Blocked — owner volatile grant needed (proposal filed at
memory/closer/volatile-grant-proposals.md §1):** func_8008BEA4
(SioAnsyncRead) + func_8008C184 (SioAnsyncWrite) — candidates PROVEN
standalone-identical (25/25, registers included) at
memory/closer/candidates/sio_ansync_pair_volatile.c. ALSO label-drift
coupled: asmfix.txt:139-140 anchor SetPacketData on .L761/.L764; removing
the fake wrappers shifts all later main.c labels (-5/-6) → the pair can
only land together with the COMB Syncro pair shedding those label-anchored
rules (module-coherent close).

**Blocked — owner ruling needed (research memo:
memory/closer/sotn-volatile-delay-array-research-2026-07-10.md):**
func_80082A14 (v_wait) + func_80078E58 (_Pad1) — both sandbox-0 AND
oracle-green in place this session with the SOTN partial-use
`volatile s32 arr[N]` delay-counter idiom (vsync.c ships `timeout[2]`
using only [0]); layer-1 reviewer FAILed the unwritten-element sub-case as
not-yet-ratified and directed the ruling. Candidates:
memory/closer/candidates/vwait_pad1_volatile_array.c. NOTE: v_wait's park
reason is otherwise STALE (predates the g_sys_dma_region volatile grant).

**Label-drift discipline (load-bearing for any main.c/display.c edit):**
functions with .L-anchored regfix/asmfix rules: SetPacketData + exec_game
(main.c), func_8007B6C8/func_8007CA00/func_8007D3F8 (display.c),
saEft00Add/marionation_Exec (system.c), others — full list via grepping
'\.L[0-9]' in regfix/asmfix. Any upstream label-count change in those files
silently no-ops the rules and breaks the oracle.

**gpu_SetDispMask**: confirmed the memset base is same-symbol CSE off the
debug byte (`addiu a0,s1,0x6A`, no reloc) → one static SYS-block struct in
v1.129. Blast radius: dozens of per-symbol uses across display.c (DrawSync,
checkRECT family, etc.) — whole-module conversion, CDREAD-scale, needs
LIBGPU.LIB .data ground-truth design. Deferred (multi-session).

Session 1 (2026-07-09). Reference tree: sparse clone at `tmp/sotn-decomp`
(src/main/psxsdk only). Work list: memory/closer/psyq-queue-hits.json.

## SESSION 2 (2026-07-09/10) — batch 3

**10 functions measured at sandbox --disable all == 0, edits in src/:**

| queue item | Sony name | rules | fix |
|---|---|---|---|
| cdrom_FramesToBcd | CdIntToPos | 1 | pins removed; Sony nested `inline int ENCODE_BCD(n)` form (sys.c), returns p |
| func_80089E30 | _SpuIsInAllocateArea | 3 | pins/asm removed; SOTN s_m_util.c + 4.0 NULL-list guard; SpuMemRec struct |
| func_80089EB0 | _SpuIsInAllocateArea_ | 0 | same |
| spu_SetMotionActive | _SsSndReplay | 0 | if(1)/new_var devices removed; sibling spu_ResetMotionEntry idiom |
| func_80083A48 | _SsInit | 0 | 15+ pins removed; SOTN ssinit.c literal (2D `D_80106FA8[32][16]`) |
| func_80086BFC | note2pitch2 | 0 | pin + sll/sra asm removed; SOTN vmanager.c literal; VagAtr struct, `VagAtr *D_80101BC8` (_svm_tn) |
| saEft03Start2 | SpuSetReverb | 4 | SOTN s_sr.c switch; spucnt via `*(volatile u16 *)(ptr+0x1AA)` (MMIO type-level); ARM ORDER: read spucnt FIRST, then flag store (v4.0 order differs from SOTN rev) |
| md_game_check_change_main_mode_katinuki | SpuClearReverbWorkArea | 0 | do-while(0)/dead v0_cmp removed; SOTN s_crwa.c literal; needed D_800A2D14 volatile GRANT (see below) |
| spu_DmaTransfer | SpuFree | 0 | volatile pad removed; SOTN s_m_f.c; list referenced DIRECT-GLOBAL in loop (a staged `list` local hoists the load above the count test — 5 vs 0) |
| coli_HitPauseKatana | SpuMalloc | 65 | full SOTN s_m_m.c adoption; three levers below |

**SpuMalloc levers (s_m_m.c):** (1) Sony's `*(volatile u32*)&list[i].addr`
re-read is REAL in the target (extra lw) — kept verbatim w/ comment;
(2) swap block: keep OLD kb pointer for the [D+1] stores (kb[1].addr/.size
AFTER `g_spu_voice_key_b++`), _addr AND _size as pre-computed locals;
(3) kb spelled `(SpuMemRec *)((g_spu_voice_key_b << 3) + (s32)_spu_memList)`
— the s32-add shift-first spelling fixes the final addu operand order
(`&list[key_b]` and u8*-adds all give base-first = 1 off).

**Volatile grant added:** D_800A2D14 (= g_spu_voice_key_... NO — same
address as g_spu_init_flag, = Sony _spu_transferCallback) appended to
volatile_extern_allowlist.txt with ground-truth citation (Sony
libspu_internal.h declares it volatile; double-read-across-sequence-point
in SpuClearReverbWorkArea).

**Struct-adopted, byte-identical but sandbox≠0 (reloc addends only) — left
in src/, NOT in completed list:** saTan5TakeAnim2_2 (_SsStart, SSSTART) —
sandbox 12, ALL 12 diffs are `%lo(D_800A26CC+K)` vs `%lo(D_800A26DC..+0)`
addend spellings; K∈{12,16,17,18} maps exactly to the per-member symbols.
SndSeqTickEnv struct declared at D_800A26CC; SetBloodSpot (SsSetTickMode,
matched) converted to .unk0/.unk4 members (sandbox 2, same addend class);
spu_InitEx's `D_800A2884 = D_800A2D44` respelled `D_800A2D44[0]` (array
decl for _spu_rev_startaddr) — spu_InitEx re-measured 0.

**Banked (candidates/ + measured floors):**
- cdcontrol_trio_prologue_order.c — CdControl 8 / CdControlF 6 /
  CdControlB 8 (HEAD: 25/23/8 with pins). v1.86 accumulator form proven
  (ret=0..ret=-1..return ret+1, ret→s7 needs real-loop weights);
  LICM of in-loop 1/-1 killed via multi-set `t` reuse
  (defeat-licm-hoist-var-reuse). Residual = the prologue def-order wall
  (a1/a2 homes before a0, li s0,3 before andi) — same family as the
  whole-prologue overrides in tools/prologue_config.json that HEAD uses
  for all three. src/ reverted to HEAD for these.
- exec_game_sotn_hybrid.c — _spu_gcSPU at 30 vs HEAD's honest 121.
  cc1 ICE catalogued: structured pointer-walk scans SIGSEGV cc1 in this
  function; goto-spelled scan compiles. src/ reverted to HEAD.
- func_8008BC60 (_spu_pitch2note): SOTN s_n2p.c is a DIFFERENT algorithm
  (closed form) than BB2 4.0's curve-scan loop — needs self-decomp from
  the ground-truth object, do not transplant SOTN.

**Metric/process notes:**
- NEVER run `wteng main build` with dirty src — it clobbers the pristine
  build/src reference. Recovery used: stash → verify-oracle --rebuild
  (clean, SHA1 green) → stash pop. Full-oracle proof of this batch is NOT
  possible locally while claimed functions still carry rules (coli's
  label-anchored rule breaks the link against the new body) — the driver's
  strip-then-build path is the proof.
- cp1252 trap: PowerShell python -c writes non-ASCII as cp1252 (0x97)
  which breaks the engine's utf-8 read. ASCII-only in code edits.

## KEY METHOD FINDING — struct adoption vs the sandbox metric

Adopting Sony's module-state STRUCT (the honest form) makes every member
access emit relocs as `D_<base>+addend` while the pristine `build/src/*.o`
reference spells them as per-member symbols with addend 0. **Identical
linked bytes, nonzero masked sandbox score.** The sandbox is therefore NOT
the right gate for struct adoptions — the full build + per-region EXE diff
is (`tmp/closer/exe_diff_regions.py` localizes differing words per function
via build/bb2.map). Proven: func_800826CC scored sandbox 39 with ZERO
differing EXE words.

**PROOF OF THIS SESSION'S BATCH: full build SHA1 == oracle
(62efab4f73f992798c43e8c730aa43baa10bb4fa) with all edits in place**
(measured 2026-07-09, `wteng main build` → MATCH, exe_diff_regions → 0
differing words). HEAD-carried rules for these functions are now inert
no-ops (their text anchors no longer match, e.g. saEft00Add_sub 151-152)
or unchanged (saEft00Add's, whose body kept HEAD parity).

## Completed honestly this session (byte-proven at the oracle, edits in src/)

| queue item | Sony name | what was fixed |
|---|---|---|
| cdrom_GetCmdName | CdComstr | goto-end accumulator → Sony early-return, u8 param (sandbox 0) |
| cdrom_GetResultName | CdIntstr | same (sandbox 0) |
| cdrom_ConfigSPU | CD_initvol | outer-volatile ptr + staged locals → Sony faithful body, chained vol[] init (sandbox 0) |
| func_800826CC | CdRead | 6 asm("sym") alias renames + volatile saved_ptr device → volatile CdlREAD struct, direct switch stores, direct saEft00Add call |
| func_800827D0 | CdReadSync | fn-ptr cast workaround → Sony while(1) body + honest CdReady wrapper call |
| saEft00Add_sub | CdReadBreak | (was PARKED dist 4!) pointer device → volatile struct members; rules now inert |
| sys_SetTimer | CdReadMode | volatile cast on g_sys_timer → volatile struct member (2 accesses CSE the address = target's lui+addiu form; plain `*p` was measured 2 insns SHORT — the queue note's suggested fix is WRONG) |
| spu_ResetMotionEntry | _SsSndNextSep | dead `s32 offset` removed; body already Sony's mixed score-ptr/full-index shape (next.c) |

Non-queue upgrades in the same proof: Vu0SetLightColMatrix_800801E8/80080208
(CdSync/CdReady) now forward (mode, result) honestly (were void(void) with
register-passthrough accident); tslTmlGetHeda (CdDataCallback) returns s32;
the `saEft00Add_ret`/`func_80080660_ret` asm() function aliases deleted.

## The CdlREAD struct (cdread.c module state)

Declared in src/system.c + src/ings2.c (same layout): base symbol
`D_800A14D0`, members sectors/buf/p/mode/size/cnt/t2/t1/pos/cbsync/cbready/
cbdata/tslmode at +0..+0x30 (= D_800A14D0..D_800A1500; g_sys_timer is an
absolute-symbol alias of D_800A1500 used by other TUs). CD_ReadCallbackFunc
is the preceding word D_800A14CC. SOTN ref: `static volatile cdreadStruct
D_80032DBC` in src/main/psxsdk/libcd/cdread.c. IRQ-mutated by cb_read
(local at 0x80082050) → volatile is original semantics.
**Anchor MUST be D_800A14D0** (not 14CC): CdRead's codegen materializes
&sectors as the struct base (`lui v0; addiu v0,%lo(D_800A14D0)`); a
CC-anchored struct loses that insn (measured, 4-byte EXE shrink).

## saEft00Add (cd_read_retry) — still INCOMPLETE, banked findings

- HEAD body restored verbatim (keeps its ~10 regfix + 2 asmfix rules,
  volatile casts, `__asm__("")`); one interim de-volatile cast
  `*(s32 *)&D_800A14D0.sectors` keeps HEAD parity (volatile read there
  costs +1 insn — measured).
- Faithful Sony early-return shape (cdread.c cd_read_retry): measured 81
  differing EXE words — the goto-end chassis is much closer to target.
  All-volatile members also cross-jump-merge the three `cnt=-1` exit tails
  (-28 bytes) — HEAD's mixed volatile/plain spellings keep them apart.
  Honest closure needs the mixed-exit treatment on the struct chassis.
  Candidate banked: memory/closer/candidates/saeft00add_sony_shape.c.

## Metric notes for the next session

- `wteng main build` writes build/bb2.exe but does NOT regenerate
  build/src/*.o — sandbox stays anchored to pristine HEAD objects.
- Sandbox scores on struct-adopted functions = (real diffs) + (reloc addend
  artifacts). Use exe_diff_regions.py for truth.
- LF endings: Write/Edit tools produced clean builds throughout.

## Batch 2 (same session): main.c/gpu.c/display.c — all sandbox-0 AND in the
## final oracle-green build

| queue item | Sony name | fix |
|---|---|---|
| func_8008AE7C | SpuSetTransferMode | dead v0++/v0-- if-chain → Sony's switch + return transMode |
| func_8008B400 | SpuGetAllKeysStatus | opaque one/three/two → literals (they hoist naturally); `limit=24` kept (= Sony's inlined `max` var, real+used); indexed `status[voice]` stores (walking ptr is loop.c strength-reduction) |
| func_800892F8 | SpuQuit | g_spu_init_flag/g_spu_timer are `void (* volatile)()` = Sony's _spu_transferCallback/_spu_IRQCallback (libspu_internal.h:39,42 types them volatile) — original semantics, not coercion |
| func_8008ADC4 | SpuWrite | same declaration fix (func_8008AD64 SpuRead also 0) |
| spu_IrqHandler | SpuInitMalloc | always-true wrapper → `if (num > 0) {...} return 0;` with size staged first (4.0 statement order; SOTN's older order mismatches — branch shape needs positive-path-first) |
| func_80085448 | SsSetSerialVol | s32 buf[10] → real SpuVolume/SpuCommonAttr structs (sizeof 0x28 == old frame; the "32 bytes" claim in the reopen note is wrong) |
| single_game_getEnemyCharId | ratan2 | dead var_v0 decl removed |
| gpu_SetMode | ResetGraph | new_var=-1 stage + decl removed — plain literals already match |

**FINAL PROOF: full build SHA1 == oracle with ALL batch-1+2 edits in place;
exe_diff_regions = 0 differing words (2026-07-09, end of session 1).**

## Banked (findings measured, HEAD body restored / untouched)

- **func_8008BEA4 / func_8008C184 (SioAnsyncRead/Write, LIBCOMB)**: the
  always-true `if((flag&&flag)&&flag){do{}while(0);}` is load-bearing: it
  keeps target's strictly-ordered B04→B00 stores + un-hoisted base load.
  MEASURED: plain removal 4; all-volatile decls 9 (flag loses its la
  form); volatile only B04/B00 4 (volatile did NOT hold store order!);
  bare do-while(0) fence 4; swapped source order 4 (scheduler swaps
  back deterministically). Next avenue: volatile SioAsync struct at
  D_800F1AFC (+0 flag, +4 buf, +8 len — one IRQ-shared Sony block, same
  pattern as CdlREAD) — requires respelling cpu_side_move_dir_3's uses
  (rule-carrying! needs byte-parity care, incl. one non-volatile
  cast like saEft00Add's .sectors). NO asm() renames (tried, self-reverted
  — forbidden family).
- **func_80082C58 (startIntr, LIBETC intr.c)**: full Sony struct mapped:
  intrEnv_t at D_800A1578 {u16 interruptsInitialized; u16 inInterrupt;
  Callback handlers[11] (+0x04); u16 enabledInterruptsMask (+0x30); u16
  savedMask; s32 savedPcr; jmp_buf buf (+0x38, D_800A15B4 = buf[1] =
  JB_SP); s32 stack[1024] (+0x68)}; sotn intr.c:10-19 + startIntr():79.
  flag_ptr = s0b-60 == &intrEnv.interruptsInitialized. Whole-module
  conversion needed (siblings func_80082D34=trapIntr, D_80083418=
  trapIntrDMA, D_8008359C=setIntrDMA share the symbols; some carry rules
  → byte-parity discipline like CDREAD).
- **gpu_SetDispMask (SetDispMask, LIBGPU sys.c v1.129)**: needs the SYS
  .data-block struct (base 0x8009BE24; debug byte +0x52; DISPENV cache
  +0xBC; memset arg = debug_base + 0x6A). Same Ruling-1-clean single-
  object treatment; design against tmp/libscan/psyq40/LIBGPU.LIB.
- **saEft00Add (cd_read_retry)**: see candidates/saeft00add_sony_shape.c.
