# Closer Phase 3 — PsyQ psxsdk adoption progress ledger

## FINAL (2026-07-13) — mission CLOSED; 7 candidates triaged; remaining work handed to the Grinder

Closer Phase 3 is **retired**. The adoption mission delivered its closes
(14+ functions across sessions 4–14: LIBGTE canonical batch, saTan0/2Main,
_spu_t/_spu_init, cdread family, DispStuff, DMA/Ansync pairs, …). The 7
still-banked candidates were triaged for a clean wrap-up. **Key framing:**
every one of the 7 maps to a function that is still `active` in
`engine/queue.json` — so the autonomous Grinder inherits all remaining work
by construction. Parking was NOT used (contradicts [[no-deferral-work-to-completion]]
2026-06-12 + [[no-park-permanently]] 2026-06-24, and queue.json was off-limits
during this pass — the Grinder was live on func_8001C624). "Keep as reference"
therefore means: the banked candidate stays as the authoritative record; the
function stays ACTIVE for the Grinder to reach in queue order.

| # | Candidate | Function(s) | Class | Action |
|---|---|---|---|---|
| 1 | cdcontrol_trio_prologue_order.c | func_80080258 / func_80080390 / tslPolyF4Init | B | keep — prologue def-order wall (twin of func_8007C2A0/C4B8); all verdict C, Grinder-reachable |
| 2 | cdcw_tslTm2LoadImage.c | tslTm2LoadImage (CD_cw) | B | keep — coupled to the marionation nrefs family; authoritative 57/263 record |
| 3 | exec_game_sotn_hybrid.c | exec_game (_spu_gcSPU) | B | keep — banked 30; cc1 ICE on structured pointer-walk (dodgeable via goto, as HEAD does) |
| 4 | marionation_p6_volatile1496.c | marionation_Exec | A | **DELETED** — redundant with live Grinder ledger (57 sessions) |
| 5 | marionation_vAT1_notailwrap.c | marionation_Exec | A | **DELETED** — redundant with live Grinder ledger (57 sessions) |
| 6 | spu_writebyio_splice.c | DispUpdateStatusMessage (_spu_FiDMA) | C | **ESCALATED** — cc1 fork-segfault, Ruling-2 needed → docs/escalations/closer-cc1-fork-divergence.md |
| 7 | spusetreverbmodeparam_struct.c | func_80089F3C | B | keep — 3-word reload spill divergence; needs cc1 BB2_RELOAD_DEBUG instrumentation (noted in the escalation as related-but-unproven) |

**Class A (deleted) rationale:** both marionation twins are `marionation_Exec`,
which has a live driver-managed Grinder ledger (`memory/grind/marionation_Exec/`,
57 sessions, floor 4). The twins' lower *masked-2* score is achieved through a
heavy FAKE-device stack (nested do-while(0), opaque `new_var` mask holders,
pointer-alias staging) — precisely the construct family the default-FAIL Judge
is currently rejecting (see docs/grind/decisions.md func_8001C624 / func_8001B138
rulings 2026-07-12/13). The Grinder's floor-4 is the honest floor; its ledger's
basin catalog already covers 20 distinct masked-4 spellings across these axes.
The masked-2 mechanism itself is independently preserved in `phase2-closeout.md`
(§W1) and this ledger (session 12/earlier) — nothing is lost by deleting the .c
files. No Class-D (novel lever) items exist.

**Mission status:** CLOSED. No further Closer sessions. The 5 kept candidates
are reference records the Grinder consults when it reaches those active queue
items; the 1 escalation awaits an owner ruling.

## SESSION 15 (2026-07-12) — Docket sweep across the 7 remaining banked candidates

Ran a triage pass across every remaining banked candidate. Result:
**no additional closes this session**; each candidate documents a real
ongoing investigation that either couples to Grinder autonomous work or
needs infrastructure investment beyond a single session. This entry
records the assessment so future agents don't re-derive it.

### cdcontrol_trio (func_80080258, func_80080390, tslPolyF4Init) — plateau confirmed

Applied the banked candidate C body to `func_80080258` in a test edit;
oracle went red immediately. The 3 regfix rules (`$18 <-> $20 @ 11-59`,
`$19 <-> $20 @ 11-59`, whole-prologue `reorder 3,18,4,17,5,6,7,19,8,9,10,
11,12,13,14,15,16 @ 3-19`) are anchored to the CURRENT cheat-source body
(register pin + DImode chain + int-new_var bias). Applying the banked
honest C shifts indices, rules break, oracle fails. Reverted.

The wall is genuine: sandbox `--disable all` = 8 on `tslPolyF4Init`'s
current honest body, meaning even with the rules and prologue_config
stripped, 8 words differ. Same family as the func_8007C2A0/C4B8 prologue
twin wall (session 5b). No novel lever attempted this session.

Effective state: the current committed forms carry documented cheats
(register pins, DImode chain, prologue_config overrides) that keep oracle
green while the pure-C plateau remains at 8/6/8. Banked candidate
`cdcontrol_trio_prologue_order.c` stays as the reference honest-C form.

### cdcw_tslTm2LoadImage — coupled to marionation Grinder progress

Session 12 banked at 57/263 with 4 named residual clusters. Clusters 3+4
were explicitly documented as "same family as cpu_side_move_dir_4
s96-s97 nrefs partition — solve once, apply twice." The Grinder has run
98 sessions on cpu_side_move_dir_4 (current floor 2) and 57 sessions on
marionation_Exec (current floor 4); when the nrefs family cracks, CD_cw
mirrors. Not a productive single-session target.

### spusetreverbmodeparam_struct — cc1 reload-pass instrumentation needed

Session 14's negative-lever catalog covers the quick source-side probes.
The named next-avenue (patch cc1 with `BB2_RELOAD_DEBUG` dumps to
identify the pass-1-stacked pseudo set) is multi-session infrastructure
work.

### spu_writebyio_splice — cc1 fork segfault, needs Ruling-2 or big search

`_spu_FwriteByIO` was landed bit-exact by session 11. `_spu_FiDMA` hits a
cc1 fork segfault on the target's `while (volatile MMIO) { ... break; }`
pattern (15 variants measured negative). Session 11's next-avenues: (1)
loop-note-preserving spelling whose exit test doesn't target loop-end
(major search), (2) input-side fix (forbidden per no-compiler-divergence),
(3) Ruling-2 (fork-divergence class) — user policy decision. Not a
single-session close without owner input.

### exec_game_sotn_hybrid (_spu_gcSPU) — big function, banked at 30

Session 2 got from HEAD's 121 to score 30 via SOTN chassis hybrid; 4
named residual clusters (cur giv-split, key_b invariant caching,
p-derivation operand order, prologue li placement). cc1 ICE class
documented for structured pointer-walk. Substantial dedicated work
required.

### marionation_p6_volatile1496 / marionation_vAT1_notailwrap — Grinder territory

Phase 2 twins. Grinder ledgers: `memory/grind/marionation_Exec/` (57
sessions, floor 4) and `memory/grind/cpu_side_move_dir_4/` (98 sessions,
floor 2). Explicitly Grinder-owned per closer mission scope.

### Conclusion

The 7 legitimately-banked candidates all represent real investigations
that couple to Grinder autonomy, need infrastructure investment, or need
owner policy decisions. Documenting this here so future sessions don't
re-derive the assessment; the candidates themselves stay as the
authoritative record of each function's state.

## SESSION 14 (2026-07-12) — saTan2Main closed (new sanctioned lever); SpuSetReverbModeParam banked

**saTan2Main (LIBSND vs_vh SsVabOpenHeadWithMode, dist 245 → 0) — CLOSED
COMPLETED-C** (commit d2e55756). Session 8 (2026-07-10) landed the SOTN
psxsdk transcription to floor 5 with the per-arm sum form (245/247 insns);
residual was 4 real words at the malloc-fail/overflow join (sum in `$a0`
not target's `$v1`) via a named mechanism — `sum` inherits `var_s0`'s
`$a0` copy-preference through `global.c:expand_preferences` from the
duplicated `sum = spuAllocMem + var_s0` expression in each branch.

Closing lever: HOIST the shared `sum = spuAllocMem + var_s0;` OUT of both
branches into a single post-if/else statement. With one sum pseudo, RA
determines its register from its ONE use (the overflow compare) — GCC
picks `$v1` naturally. GCC's `jump2` pass duplicates the simple assignment
back into each arm at codegen time, producing target's per-arm
`addu $v1, $a3, $s0` bytes. Sandbox 5 → 0 first try; `retire` dropped the
whole-body asmfix splice; SHA1 == oracle.

Signature promoted to Sony types: `extern s16 saTan2Main(u8 *, s16, s16,
u32);` (was `(s32, s16, s32, s32)`). Three callers gain `(u8 *)`/`(u32)`
casts. Oracle stays green.

Review trail: layer-1 cheat-reviewer FAILed on "novel unsanctioned shape"
grounds — the hoist-shared-arm-computation lever hadn't been catalogued.
User adjudicated 2026-07-12 (the C is textbook DRY refactoring; `jump2`'s
duplicate-into-arms is standard codegen pipeline; SOTN's psxsdk uses the
equivalent inline-compare shape). SANCTIONED as a new pure-C lever:
`hoist-shared-arm-computation-defeats-copy-pref` (rule doc in
`.claude/rules/`, indexed in codegen-technique-index.md).

**SpuSetReverbModeParam (func_80089F3C, dist 23) — banked at 23** (no
change). 4 quick source-level levers tried (`u32 sp58`,
`sp58`-declared-first, split-init `var_s4`, precompute shifted delay into
`d12`/`d13` locals) — all inert or worse. Score stays at 23 (20 self-heal
reloc + 3 REAL sp58-offset words). Named next-avenue (session 6): patch
cc1's reload pass with BB2_RELOAD_DEBUG dumps to identify the pass-1
stacked pseudo set. That's multi-session infrastructure work; deferred.
Candidate `spusetreverbmodeparam_struct.c` updated with the negative-lever
catalog.

**Queue net (session 13 + 14 combined)**: 380 → 369 active (11 items
closed since session 12: 10 LIBGTE canonical + saEft00Add + saTan2Main).

## SESSION 13 (2026-07-11) — LIBGTE canonical-asm authorization pass + saEft00Add close-out

**10 LIBGTE census-proven functions retired as COMPLETED-INLINE-ASM-CANONICAL**
(commit 9eba9a3e). All are verbatim-linked Sony PsyQ 4.0 LIBGTE .LIB
objects; splat tags every cop2 op "handwritten instruction"; no pure-C form
exists under the 2026-05-31 cheat catalog. Whole-body
`__asm__("glabel ... endlabel")` blocks replacing C bodies with register-
asm pins, single-instruction `negu`/`lui` general-purpose-opcode asm, and
inline-move-aliasing constructs.

| Function | LIBGTE module | Sony name | Rules retired |
|---|---|---|---|
| func_8007E1AC | MSC06 | LoadAverage12 | 0 |
| func_8007E1FC | MSC06 | LoadAverage0 | 0 |
| func_8007ED6C | MTX_05 | ApplyMatrix | 0 |
| func_8007F24C | SMP_03 | RotTransPers3 | 0 |
| func_8007F2DC | CMB_00 | RotTransPers4 | 0 |
| func_8007E4DC | MTX_000 | MulMatrix0 | 0 |
| func_8007EB4C | MTX_03 | MulMatrix | 0 |
| func_8007E8DC | MTX_00A | ScaleMatrixL | 1 (fill_delay) |
| func_8007E74C | MTX_004 | ApplyMatrixLV | 13 (dead-branch-scheduling insert_after `sra` + j→b + fill_delay clusters) |
| func_8007EA0C | MTX_01 | ApplyRotMatrixLV | 13 (same archived pattern) |

**Cluster corroboration**: siblings calc_fc_frame_8007EC5C (ASM-WHOLE
2026-05-31), func_8007E8AC (2026-06-11), func_8007F0BC/F0E4 (2026-06-11)
were already canonical-body under the archived gte-3x3.md — which
explicitly named E4DC and EB4C as siblings to follow the same path.
Cheat-reviewer PASS with 9 independent verifications (census cross-check,
target.s byte-diff, scan_hand_coded S8 STRONG on E8DC, oracle rebuild).

**saEft00Add closed** (commit c7114628, followup). Session 9 landed its
honest bit-exact CDREAD-family close via cdread_family_close.patch (0/133
words vs the EXE); the 2 residual sandbox-score words are reloc-spelling
addend artifacts (per-member symbol addends vs D_800A14D0+K base). `queue
done saEft00Add` accepted: 0 rules, 0 cheat-asm, SHA1 == oracle →
COMPLETED-C.

**Infrastructure finding — canonical-asm-authorization-recipe corner case.**
Emitting `.set reorder\n.set at\n` at the end of a file-scope `__asm__`
glabel block when the NEXT block opens with `.section .text` causes maspsx
to insert a stray load-delay nop in the following function (confirmed for
ED6C→EDBC and F2DC→motutil_GetWalkDir). Fix: omit the trailing
`.set reorder/at` when the next block starts with `.section .text`.

**Queue net**: 380 → 369 active (11 items closed this session; 1 previously-
parked flipped back to active during regen).

**Housekeeping** (this session):
- 9 obsolete banked patches deleted from `memory/closer/candidates/`
  (cdread_family_close.patch, cdread_triple.patch + README,
  intr_module_close.patch + README, saeft00add_sony_shape.c,
  sio_ansync_pair_volatile.c, spusetkey_v40_proven.c,
  vwait_pad1_volatile_array.c). All target functions COMPLETED; patches
  served their purpose.
- 8 stale WIP entries deleted (D_80083418, func_80064E90, func_8007E35C,
  func_8007F1A8, func_80085270, func_80089EB0, saEft03Start2, spu_InitEx)
  — all functions are COMPLETED-C or COMPLETED-INLINE-ASM-CANONICAL.
- `volatile-grant-proposals.md` collapsed to a resolved-proposals audit
  table (§1-§4 all ratified). Proposal queue is now empty.

## SESSION 12 (2026-07-10) - LIBETC INTR module proven 0/297 (blocked on Proposal 4); CD_cw banked at 57/263

**func_80082D34 (= trapIntr + setIntr + stopIntr + restartIntr + memclr,
LIBETC intr.c v1.76, dist 295, asmfix:58 splice) - PROVEN 0/297 BIT-EXACT
over the full 0x4A4 splice extent (prover tmp/closer/intr_prove.sh, link_sim
vs EXE); NOT claimed - blocked on ONE owner sign-off (Proposal 4).**
- Patch banked: memory/closer/candidates/intr_module_close.patch (+ _README).
  src/ings2.c reverted to HEAD (splice double-emits otherwise).
- SOTN intr.c transcription onto the session-3 intrEnv_t struct. ONE lever:
  Sony's trapIntr mask loop is ASSIGNMENT-IN-CONDITION -
  `while ((mask = (enabled & *i_stat) & *i_mask) != 0)` - gives the
  compute-into-v0 + `beqz v0; addu s0,v0` delay copy at entry AND rotated
  bottom (plain assign-then-test allocates into s0 directly; 21 words).
- TU head-parity verified (label renumbering only; parser must normalize
  BOTH `.L\d` and `.Lfe\d`).
- THE BLOCKER: last 8 bytes of the extent (0x800831D0/D4 = 0x15007350
  0x0040809C) are LIBAPI C114 OBJECT HEADER DATA - proven vs
  tmp/libscan/psyq40/LIBAPI.LIB module C114 (psyq_lib.py): .text=32 bytes,
  XDEF _96_remove at +0x8, bytes +0..+7 = exactly these words, zero relocs;
  +0x8..+0x1F = exactly the authorized bios_CdRemove_A0 trampoline. Honest
  home = 2 leading `.word` lines in that canonical block (in the patch).
  Canonical-asm content edits are owner-gated => Proposal 4 in
  memory/closer/volatile-grant-proposals.md. Driver mechanics on sign-off:
  apply patch + strip asmfix:58 in the SAME commit.

**tslTm2LoadImage (= CD_cw, LIBCD BIOS v1.86, dist 262, asmfix:62 splice) -
BANKED at 57/263 differing (from 262); candidate
memory/closer/candidates/cdcw_tslTm2LoadImage.c; src reverted to HEAD.**
Prover: clone triple_prove.sh -> link_sim out.o tslTm2LoadImage 0x800812FC
0x41C. Levers that WORKED (all measured, in the candidate):
- SOTN bios.c CD_cw chassis + v1.86 deltas (inlined set_alarm/get_alarm +
  callback windows a la the twins; EARLY CdlSetmode store D_800A11D4 before
  the Intr stores; 11-slot -> same).
- `(a0 & 0xFF)` spelled PER USE on the s32 param (per-use andi; the offset
  sll CSEs across the D_800A13FC/D_800A11DC pair).
- `if (a3 == 0) { alarm+loop+tail } return 0;` restructure (NOT
  `if (a3) return 0;`) - puts the ret-0 block at the end so reorg fills the
  bnez delay from the target thread (v0=0) instead of stealing the reg1
  store (145->…).
- `extern volatile u8 *D_800A1480/D_800A1484` (MMIO pointees, block-scope,
  type-level sanction): volatile makes the reg1 sb INELIGIBLE for the bnez
  delay steal AND fixes the reg2-loop shape. 145 -> 60.
- Entry test = DIRECT volatile-struct access (`g_cd_status_a.sync != 0` via
  block-scope `extern volatile CdIntr g_cd_status_a`) -> la a0 + lbu;
  then `tbl = D_800A125C; intr = (volatile u8 *)&g_cd_status_a; intr1 =
  &g_cd_status_a.ready;` AFTER the test -> cse yields `addu s2,a0` +
  `addiu s4,s2,1` (the twins' preamble shape, honestly derived - GCC itself
  rewrites intr1 to intr+1, lreg insn 280).
- Tail 8-byte copy: counter must be the REUSED `cnt` (alarm counter) - a
  fresh/i-reused counter allocates a0/v1 swapped.
- do-while respell of the outer loop: 252 (rotation chaos) - KILLED; goto
  chassis is right.
- struct-member access EVERYWHERE (no pointer locals): 234 - KILLED (each
  volatile zero-offset access re-materializes la; loses s2/s4 entirely).
Remaining 57 = FOUR named clusters:
1. PROLOGUE home rotation (8 words): target homes a1,a2,a3,a0 (s1,s5,s2,s0
   pair order after prologue_fix); ours a0-first. Same family as the
   CD_datasync banked param-seat rotation.
2. TAIL-COPY beqz delay (1-insn length delta -> ~24 words of branch-offset
   fallout): ours fills the `beqz a2` delay with `li v1,7`; target leaves
   nop + li after. reorg fill_eager analysis: target predicted TAKEN
   (own_thread fails on the multi-pred cw_done label -> nop); ours predicts
   not-taken and steals from fall-through. Likely loop-note/prediction
   difference (marionation region-3 dbr-fill family).
3. intr/saved s1<->s2 SWAP (~10 words): greg-measured priorities intr
   (p85: 6 refs/132 len = 0.0909) vs saved (p80: 2 refs/23 len = 0.0870) -
   ours allocates intr first (s1); target allocated saved first (s1).
   MARGIN IS TINY. Killed: intr1-before-intr order (derives from the a0
   temp, wrong preamble bytes); direct-struct bottom test (la form, wrong
   bytes). Untried: anything that adds ~1 ref to saved or drops intr to 5
   refs while keeping all five required use sites byte-identical - the
   cpu_side_move_dir_4 s96-s97 nrefs partition is THE SAME FAMILY; solve
   once, apply twice.
4. Debug-window arg scheduling (~10 words): target order = both lbu's, lw
   a1(19C0), tbl[ready] -> sw sp0x10 EARLY, lbu CD_com, lw a2, lw a3 last.
   Measured: both-staged arg4/arg5 (57, best); arg4-only staged (60);
   arg5-only (62); byte-staged sy/rd (60); sy+arg5 hybrid (94).
Clusters 3+4 interact with the twins' contested windows - if Phase 2 cracks
the nrefs family, mirror here.

**Method note:** greg/lreg dumps for system.c via tmp/closer/cdcw_greg.sh;
allocation ORDER is the `;; N regs to allocate:` line; pseudo stats are in
.lreg (`Register N used X times across Y insns`), priorities =
floor_log2(refs)*refs*size/length.


## SESSION 11 (2026-07-10) - LIBSPU/SPU module batch: _spu_t + _spu_init closed at 0; _spu_FwriteByIO proven bit-exact (banked)

**saTan0GaugeDraw (= _spu_t, dist 57, 49 rules) - COMPLETED at sandbox 0,
unmasked-identical, claimed.** SOTN spu.c switch transcription. Levers (all
measured):
- Real stdarg: `va_start(ap,parmN) = (&parmN + 1)` (PsyQ style) is
  LOAD-BEARING - taking &mode homes the named arg (target's sw a0,24(sp));
  __builtin_next_arg does NOT emit it. va_arg = manual ptr bump.
- Timeout loops: u32 counter (sltiu), SOTN `while(read != tsa){if(++i>0xF00)
  return -2;}` for case 3; cases 1/0 need if+do-while with the ENTRY READ
  through a fresh u16 temp: `t = volatile-read; i = 0; flag = 0; if ((t &
  0xFFFF) != tsa) do{...}while(volatile-read != tsa);`. The volatile load into
  a dying temp keeps the target's explicit andi 0xFFFF (combine refuses
  substitution through volatile defs) AND reads before the flag store.
- THE const-1 seat (li a2 vs v1, 4-site exchange - the last 4): `i = 0;`
  placed BEFORE the flag store in cases 1/0. Mechanism (greg-verified): the
  switch's CSE-shared const-1 pseudo (dispatch beq + case-0 store + case-3
  ck2 compare) dies at case-0's flag store; i=0 before the store overlaps it
  -> conflict -> const1 pushed off v1 (v0/a0/a1/s0 taken) -> a2 = target.
  Emitted position identical (reorg pulls i=0 into the delay slot either way).

**func_80088740 (= _spu_init, dist 56, 17 rules + memory-barrier cheat-asm) -
COMPLETED at sandbox 0, unmasked-identical, claimed.** SOTN _spu_init with
BB2-4.0 deltas (WASTE_TIME = out-of-line spu_WriteReg16 = _spu_Fw1ts; local
0xF01 timeout counters; PLAIN key_on/key_off assignments not |=). Levers:
- Status loop: indexed `for (ch=0; ch<10; ch++) D_800F7420[ch] = 0;` - i used
  in addressing blocks check_dbra reversal; giv reduces to the walking ptr.
  D_800F7420 decl widened [4]->[10] (Sony object clears 10 halfwords here;
  grant is symbol-keyed; SpuSetKey re-measured 0 after).
- Voice loop: `vp = (volatile u16 *)D_800A2CDC;` cached BEFORE the loop +
  indexed vp[ch*8+k] stores. loop_has_volatile blocks MEM invariant hoisting
  in 2.7.2 loop.c, so the base load must be a source-level local.
- kon/koff s32 locals = 0xFFFF/0xFF (ori spelling; live across the Fw1ts
  call groups -> s1/s0).
- maspsx_label_nop_funcs.txt += func_80088740 (voice-loop label nop; same
  gate as spu_DmaTransfer).
- Return type s32/return 0 (Sony); killed the two lost-codegen insert rules.

**DispUpdateStatusMessage (= _spu_FwriteByIO + _spu_FiDMA + _spu_Fr_, whole
splice asmfix:123, 206 words) - NOT claimed; candidate banked at
memory/closer/candidates/spu_writebyio_splice.c; src reverted to HEAD stub
(claiming would double-emit against the active splice).** Status:
- _spu_FwriteByIO: BIT-EXACT 132/132 (prover tmp/closer/spu_splice_prove.sh,
  full Makefile maspsx flags). KEY: Sony's vestigial `volatile s32 sp0, sp4;`
  WASTE_TIME macro locals (SOTN ships the decls) explain the 0x30 frame -
  8 bytes of untouched locals. FAKE-annotated Ruling-3-class.
- _spu_FiDMA: 2 words short - THE NAMED RESIDUAL: (a) our fork SEGFAULTS on
  Sony's literal `while (volatile-read & 0x30) { if (++i > 0xF00) break; }`
  (any while/for + break/goto-out spelling; cc1psx compiles it fine ->
  tmp/closer/spu3/t_psx.s emits the EXACT target bytes incl. the dead
  `addu $3,$3,-1`); (b) compiling equivalents: jump.c duplicates the
  [i++; limit-test] entry (empty break arm -> exit test targets loop-end
  directly -> duplicate_loop_exit_test fires) -> rotation + li-1 fold; goto
  spellings avoid rotation but lose loop notes -> mostly_true_jump returns
  likely=0 (DBRDBG-traced, tmp/gccdbg/cc1 BB2_DBR_DEBUG=1) -> reorg's
  increment-compensation clause (reorg.c:3615, the ONLY producer of the
  addiu -1) never fires. A dead source-level i-- is flow-deleted (measured).
  15 variants measured in tmp/closer/spu3/. NEXT LEVERS: (1) find a
  loop-note-preserving spelling whose exit test does NOT target loop-end
  (blocks the dup) while keeping body order [i++; limit; cond]; (2) fix the
  crash class input-side (the segfault IS the divergence - cc1psx exhibit);
  (3) Ruling-2 material if enumeration completes: "our fork cannot emit;
  the original compiler can and did" - fork-divergence class like the
  marionation fp-allocatability finding.
- _spu_Fr_: 2 masked diffs (ori s0,0x10 fills the jal-ReadReg delay in ours;
  target leaves nop and puts ori in the lw-CE4p load-delay). Un-ground.
- _spu_FiDMA is address-referenced only via g_snd_irq_data (=0x80088BA0
  absolute); _spu_Fr_ is UNREFERENCED Sony dead code (S_SCA precedent).

**Fork-crash finding (REUSABLE):** GCC 2.7.2 decompals fork segfaults on
`while (<volatile MMIO read expr>) { ... break; }` loop class (volatile read
in the while condition + any early exit in the body). Workaround: if+do-while
or goto spellings. cc1psx does NOT crash. Candidate for a technique-index
entry after sign-off.

**Blast radius checks:** SpuSetKey (func_8008AAD4) sandbox 0 after the [10]
decl; TU cc1 diagnostics at HEAD parity (same 3 benign conflict pairs);
main.c label-anchored rules unaffected (exec_game regex, SetPacketData
splice per session-4 finding).

## SESSION 10 (2026-07-10) — cdread family REPLAYED+re-proven; DispStuff (SsStart+SSCALL) closed 0/209

**cdread family close (session-9 banked patch) — REPLAYED, re-proven, claimed.**
git apply memory/closer/candidates/cdread_family_close.patch (clean); all five
regions re-proven bit-exact vs the EXE this session (triple_prove.sh /
saeft_prove.sh / link_sim): tslTm2LoadImage_2 region 0/263, saEft00Add 0/133,
CdReadSync 0/51, CdRead 0/65, saEft00Add_sub 0/39. Sandbox: saEft00Add 2
(reloc-spelling artifacts), tslTm2LoadImage_2 243 (extent-split artifact).
Fresh layer-1 cheat-reviewer verdict: C content CLEAN on every cheat test
(volatiles all granted §3, de-volatile casts gone, byte proofs independently
re-run); formal FAIL solely on the still-wired rules (asmfix:62 splice +
saEft00Add's 9 regfix + 2 asmfix) — which is the DRIVER's strip-then-rebuild
job under the outcome contract (SpuSetKey-42-rules / DMA-pair precedent; the
Closer is rail-forbidden from regfix/asmfix). Claimed: tslTm2LoadImage_2 +
saEft00Add. DRIVER MECHANICS: strip the splice + rules in the SAME commit
(rules-on build unbuildable until then, session-9 note stands).

**DispStuff (= SsStart + SSCALL module, dist 207, 1 asmfix splice) —
CANDIDATE-COMPLETE, claimed. 0/209 bit-exact** (tmp/closer/dispstuff_prove.sh,
region 0x80083E9C+0x344), and saTan5TakeAnim2_2's region re-proven 0/154 (it
references the now-static trampolines). Sandbox 201 = extent-split artifact
(my DispStuff C fn is 8 insns; 4 statics carry the rest of the extent).
Structure: DispStuff{saTan5TakeAnim2_2(1)} + static SsStart2{(0)} + static
D_80083EDC (_SsTrapIntrVSync) + static D_80083F1C (_SsSeqCalledTbyT_1per2) +
static SsSeqCalledTbyT (sotn sscall.c literal, BB2 flag offset 0x98/stride
0xB0, _ss_score as pointer table). Levers measured:
- unk20 via struct member (added to SndSeqTickEnv): 34 differing — 3 refs
  through the struct base la-materialize the address. Fix WITHOUT a second
  handle: unk20 is NOT in the struct (struct ends +0x13 as committed);
  D_800A26E0 is its own standalone splat symbol (dlabel in 7D920.data.s,
  named_syms: g_alarm_pending_priority_flag) declared as the block-scope
  extern in D_80083F1C — the ONLY handle in the TU. 0 differing.
- `if ((1 << i) & D_801027E4)`: 2 differing (and-operand order). Swapped
  order `flag & (1 << i)`: WORSE (6) — tree-level bit-test fold fires
  (srav+andi). `s32 bit = 1 << i; if (D_801027E4 & bit)`: 0 differing —
  the named local blocks the tree fold and gives flag-first RTL order.
- Collateral: func_800841E0/func_80084500 placeholder sigs -> (s16,s16)
  (Sony prototypes; bodies are separate splices, emission-inert).
Review: fresh cheat-reviewer PASSed everything except the D_800A26E0 extern
(flagged as second-handle before the struct-member removal); re-invoked with
the fix + the splat-dlabel evidence it had missed -> PASS (all 4 new facts
independently verified incl. the target asm's own four %hi/%lo(D_800A26E0)
relocs and a fresh 0/209 prover re-run; 'no second handle' confirmed).
DRIVER MECHANICS: strip asmfix.txt:125 (DispStuff replace_with_asmfile) in
the same commit.

**Identity note:** func_800841E0 (_SsSndCrescendo, dist 198) + func_80084500
(_SsSndDecrescendo, dist 233) are call-position-identified but sit in the
census GAP (0x800841E0..0x800848AC not proven) — excluded per the LIBSND-gap
rule; natural next targets IF ground truth lands (their protos + all shared
symbols are now in place).

**Not attempted (scoped, for the record):** func_8007D3F8 (_addque2) needs
the volatile QueueItem array + SYS block state (LIBGPU whole-module design,
same wall as gpu_SetDispMask — dedicated session). CRES/DECRE excluded (gap).
LIBCOMB Syncro pair = self-decomp vs LIBCOMB.LIB (no SOTN ref; heavy legacy
cheat-asm bodies) — dedicated session.


## SESSION 9 (2026-07-10) — CDREAD family closed bit-exact (triple + saEft00Add)

**tslTm2LoadImage_2 (puts + cb_read + cb_data, dist 261, 1 asmfix splice) —
CANDIDATE-COMPLETE, claimed.** Grant §3 ratified (d568759c) unblocked the
banked patch; applied + re-proven 0/263 bit-exact (triple_prove.sh). Sandbox
reads 243 = extent-split artifact only (my puts is 20 insns vs the 263-insn
splice extent; statics get own F symbols; self-heals on driver rebuild —
S_SCA/SpuSetKey precedent). DRIVER MECHANICS: the asmfix splice must be
stripped in the SAME commit (in-tree rules-on build is unbuildable until then).

**saEft00Add (cd_read_retry, dist 18, 11 rules) — CANDIDATE-COMPLETE, claimed
(BONUS: the long-parked coupled-fixpoint function).** 0/133 bit-exact via
saeft_prove.sh; sandbox 3 = pure reloc-spelling artifacts (struct addend +
string symbol addends). The honest close, all levers measured this session:
- Volatile per-member decls (granted §3) + HEAD goto chassis: 18 -> 14.
- Mixed exit forms (goto end1 / inline return / goto end3 / goto end4, the
  cross-jump-store-tail-merge recipe): 14 -> 9; one tail pair still merged.
- INTERIM FINDING (superseded, kept for the record): reloc-spelling variance
  (`D_800A14D0.cnt = -1` vs scalar) defeats find_cross_jump (non-rtx_equal
  mems, identical bytes). 9 -> 6. SUPERSEDED because Sony's CDREAD.OBJ
  ground truth shows ALL tail stores carry the IDENTICAL reloc
  (.data+0x18) — Sony's mems were identical; the real mechanism is the
  SOTN-source MIXED INLINE RETURNS: tail2 is `return D_800A14E4 = -1;`
  (assignment-expression return, no volatile re-read -> different RTL
  suffix shape; SOTN cdread.c line `return D_80032DBC.unk14 = -1;`), tails
  1/3 are store+re-read returns. The Sony-literal shapes measure 133/133
  bit-exact in our fork with zero synthetic devices. Reviewer round-2
  correctly forced this replacement.
- KEY NEW FINDING — zero-offset volatile struct access is UNFOLDABLE la-form
  in this fork (cse.c find_best_addr skips bare-REG addresses; combine
  refuses substitution into volatile mems; measured in tmp/closer/rtl/
  dumps). Macro-form volatile access of D_800A14D0+0 therefore REQUIRES the
  scalar decl; cross-member negative-displacement addressing (CdReadSync's
  s1-cached base) REQUIRES the struct decl. Since C forbids both at file
  scope, the views are FUNCTION-SCOPED block externs (no file-scope decl;
  GCC 2.7.2 accepts sibling-scope type-mismatched externs, warning-only).
  Final per-site design (reviewer round-2 discipline: every decl cites its
  target bytes, struct dropped where not required): CdReadSync keeps the
  ONE in-body CdlREAD object view (cached-base cross-member displacements
  at 0x800827E8+); saEft00Add + CdRead use in-body CD_sectors scalar views
  (macro read 0x800825EC / pointer-local la store 0x80082728); sub +
  cb_read respelled to granted scalars + pointer-locals. All six regions
  re-proven 0-differing after the respell.
- saEft00Add_sub 0/39, CdRead 0/65, CdReadSync 0/51 re-proven bit-exact with
  the scoped decls; head_cmp.sh: emission differs ONLY in the six intended
  functions; the CdGetSector wrapper signature fixes are emission-identical.

**Review trail:** layer-1 reviewer FAILed the banked patch's de-volatile
compensation casts (volatile-coercion family, any direction) — the honest
close above is the fix; second fresh reviewer invoked on the final state.
Full patch banked: memory/closer/candidates/cdread_family_close.patch.

**Artifacts:** tmp/closer/saeft_prove.sh, tmp/closer/one_prove.sh,
tmp/closer/rtl_dump.sh + tmp/closer/rtl/ (per-pass dumps), foldscan.sh.

## SESSION 8 (2026-07-10) — SpuSetKey claimed; CDREAD triple proven 0/263 (grant-gated)

**func_8008AAD4 (SpuSetKey, dist 7, 42 rules) — CANDIDATE-COMPLETE, claimed.**
Banked spusetkey_v40_proven.c applied verbatim (Ruling 4 unblocked it: the
five allowlist grants at lines 40-44 stand). Sandbox 8 = 127/127 insns, all
8 diffs are %lo(D_800F7420+4/+6) vs the split D_800F7424 reloc spellings
(self-heal on driver rebuild, S_SCA precedent). link_sim: 0/127 words differ
bit-exact vs the EXE. Layer-1 cheat-reviewer PASS with full independent
re-verification (re-ran sandbox + link_sim, re-derived S_SK relocs from
LIBSPU.LIB, re-measured volatile-removal collapse 8->56 w/ 127->109 insns,
verified detector allowlist handling). Driver mechanics: strip the 42 rules,
D_800F7424's undefined_syms entry may stay (unreferenced after the [4] merge).

**tslTm2LoadImage_2 (= puts + cb_read + cb_data, dist 261, 1 asmfix splice) —
PROVEN 0/263 BIT-EXACT, banked; blocked on volatile grant (proposal §3).**
Full candidate: memory/closer/candidates/cdread_triple.patch + _README.md.
Everything is in the README: the GCC 2.7.2 volatile addressing-form law
(volatile-cast/struct-first-access -> la-form; volatile-decl zero-offset ->
macro form; cc1psx-confirmed), the byte-neutral saEft00Add compensation
(cast/plain site swaps, verified via tmp/closer/head_cmp.sh per-function
HEAD-vs-current cc1 diff = label renumbering only), the CdGetSector wrapper
arg-forwarding fix, and cb_data's 26-insn sunk prologue reproducing
NATURALLY from plain C. src/system.c REVERTED to HEAD this session (the
un-strippable splice + ungranted volatiles make the in-tree form unbuildable
rules-on); replay = git apply the patch after the operator grants §3.

**func_80080828 (getintr, dist 352) — BLOCKED, jtbl placement wall (new class
for system.c).** Its switch table jtbl_8001622C lives at 0x8001622C inside
text1a_b_post_rodata.o's rodata region; bb2.ld places system.o(.rodata)
elsewhere, so a C switch's GCC-emitted table cannot land at the target
address (main.c functions do not have this problem - S_SCA's tables emitted
correctly because main.o's rodata region covers them). No committed C in the
tree uses computed-goto-through-extern-jtbl; resolving this needs either an
owner ruling on that spelling or TU re-attribution evidence. ALSO needs
volatile D_800A1494/95/96 (g_cd_status_a/b/c) grants for the chained
Intr.ready = Intr.c re-read stores (same Ruling-4 class; the twins' HEAD
bodies already carry these decls as ungranted debt). Full asm analysis done
(SOTN bios.c getintr maps 1:1 with v1.86 printf-guard deltas) - transcription
is ~1h once both blockers clear.

**saTan2Main (SsVabOpenHeadWithMode, dist 245, 1 splice) — banked at floor 5
(subagent adoption run; candidate memory/closer/candidates/satan2main_vsvh.c;
src/main.c restored to placeholder).** 245 -> 5 in one session. Everything
matches except one 5-line cluster at the SpuMalloc-fail/overflow join; named
mechanism: the overflow-sum temp must land in $v1 but a named `u32 sum`
inherits var_s0's $a0 preference via global.c expand_preferences; the SOTN
expression form gets v1 but jump2 over-merges the malloc errbody (floor 4,
245/247). Load-bearing deltas vs SOTN documented in the candidate header
(u8 vs/vspad split for VabHdr.vs — our fork's u16->u8 subreg narrowing takes
the HIGH byte; SOTN's var_a2 staging line is load-bearing; no
_svm_brr_start_addr store in 4.0). Killed: split-init accumulation, sum decl
position, do-while(0) wraps (both). Full _svm_* symbol map appended to
sony-naming-map.md.

**func_80089F3C (SpuSetReverbModeParam) — no change (banked 3-word spill
residual stands; reload two-pass mechanism needs a dedicated instrumentation
session; deliberately not ground per batch discipline).**

**Method artifacts (tmp/closer/):** link_sim.py upgraded (splat-name address
fallback, build/bb2.map symbol fallback, region-scoped .text resolution —
cross-TU regions now provable); triple_prove.sh (real-src standalone prover:
cpp|cc1|prologue_fix|maspsx|as + link_sim; tolerates the pre-existing benign
marionation new_var cc1 error — NB the Makefile pipe swallows that error;
never add set -e on cc1 for system.c); head_cmp.sh (per-function HEAD-vs-
current cc1 diff — THE byte-neutrality auditor for shared-TU decl changes).


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
