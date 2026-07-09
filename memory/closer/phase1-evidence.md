# Closer Phase 1 — external evidence sweep: findings & measurements (2026-07-09)

Session outcome in one line: **the twins are PsyQ libcd bios.c v1.86 CD_sync /
CD_ready (identity proven), the W4 one-struct question is settled (SEPARATE —
h5 is a fakematch; but 1494/95/96 are one volatile struct), and the recovered
original volatile semantics dropped marionation_Exec's floor 4 → 2 with
build/target insn parity 179/179 for the first time (region-3 nop CLOSED).**

## W4 — structure provenance (kill criterion MET)

Verdict memo: memory/closer/structure-provenance.md. Summary:
- 0x800A125C..0x800A1497 is NOT one object: it is libcd's `CD_intstr[8]` +
  four `static int` command tables + MMIO register-pointer statics + the
  3-byte `volatile CD_intr Intr` struct (1494/95/96).
- Ruling 1 SEPARATE branch triggers: **h5 cross-symbol form = fakematch**;
  csmd4 cannot complete with it.
- The honest member-access respelling was measured (P2/P3/P4 below): does NOT
  preserve masked 2 on csmd4. csmd4 endgame remains open.

## W2 — era-corpus idiom mining (kill criterion MET, jackpot variant)

- Identity: strings at 0x800161xx + `$Id: bios.c,v 1.86 1997/03/28 makoto` →
  cpu_side_move_dir_4 = CD_sync, marionation_Exec = CD_ready, saEft01Init =
  CD_cw, func_80080828 = getintr, cdrom_ClearIrq = CD_flush,
  sys_GetVblankCount = CheckCallback, tslTm2LoadImage_2 = puts,
  debug_printf = printf. Full table: memory/closer/libcd-identity.md.
- Reference source fetched: sotn-decomp psxsdk/libcd/bios.c (v1.77, MIT) →
  tmp/closer/sotn_bios.c. Its CD_sync/CD_ready C shape corresponds
  statement-for-statement to the BB2 candidates' reconstructed chassis
  (goto-loop = inlined get_alarm; poll block = inlined callback(); the
  `temp==2||temp==5 → *idx=2 → 8-byte copy` tail is literal SOTN
  `sync==CdlComplete||sync==CdlDiskError`).
- Key source fact: `static volatile CD_intr Intr` — the status bytes are
  VOLATILE in the original (IRQ-mutated via InterruptCallback(2, callback)).
  BB2's own already-completed siblings in system.c (lines ~650-960) already
  use `volatile u8 g_cd_status_*` — corroborating.
- Target prologue fact: CD_sync stages BOTH &Intr (s2) and &Intr+1 (s4)
  (`addiu s4,s2,1`) and reads `ready` two different ways (1(s2) in the window,
  0(s4) at the callback) — v1.86's source manifestly materializes pointer
  values; the two-pointer chassis of the current candidates is faithful.

### Measurements (sandbox, --disable all, masked)

| probe | function | form | score | insns | verdict |
|---|---|---|---|---|---|
| base | csmd4 | HEAD src | 7 | 160/160 | ref |
| P2 | csmd4 | direct volatile member access | 38 | 159/160 | KILL |
| P3 | csmd4 | volatile ptr chassis + honest idx_1495=idx_1494+1 | 28 | 160/160 | KILL |
| P4 | csmd4 | P3 + volatile Alarm globals | 28 (byte-identical to P3) | 160/160 | KILL (alarm volatility inert) |
| base | marionation | vT40 candidate | 4 | 178/179 | ref (57-session floor) |
| P5 | marionation | vT40 + all three idx ptrs volatile | 8 | **179/179** | nop FIXED, window regressed |
| P6 | marionation | vT40 + ONLY idx_1496 volatile | **2** | **179/179** | **NEW FLOOR** |

P6 candidate: memory/closer/candidates/marionation_p6_volatile1496.c.
P6 residual (from normalized objdump diff):
1. `{sll a0,a0,0x2 <-> addu v0,v0,s5}` single transposition in the do_timeout
   window — SAME shape as csmd4's h5 residual; the twins now share one
   micro-residual.
2. Two `beqz (a1==0)` guards branch to the FIRST copy of the duplicated
   `j;move v0,a2` tail (0x23c); target branches to the SECOND (0x28c) —
   unmasked byte diffs; known lever family: cross-jump-store-tail-merge
   label-direction steering.

## W3 — region-3 nop ownership (kill criterion MET, axis RESOLVED not just closed)

- cc1 .s inspection (tmp/closer/system_cc1.s): cc1 emits self-filled branches
  inside explicit `.set noreorder/nomacro` blocks and leaves unfilled branches
  in reorder mode (assembler inserts the nop). maspsx adds only load-delay
  nops (is_label logic), never branch-slot nops → NO maspsx lever exists.
- Ownership: our build's fill is cc1 reorg.c dbr (s51's insn 445 dst2=a1);
  the target's nop = original cc1 found NO eligible fill → `as` nop.
- The honest C condition that makes the fill ineligible is now IDENTIFIED AND
  MEASURED: volatile `*idx_1496` access at the fill site (original semantics —
  Intr is volatile). cc1 marks volatile insns (`#.set volatile`) and reorg
  refuses them as fill candidates. P5/P6 both produce the target nop (179/179).
- Axis CLOSED with the lever banked.

## W1 — multi-build prep (kill criterion MET)

- Locator built + validated: tmp/closer/locate_window.py — finds all anchors
  in "Bushido Blade 2 (USA).bin" (raw MODE2/2352 + descrambled), in
  disc/SLUS_006.63 (correct vaddrs 0x800161xx), reports bios.c version
  (v1.86), and found a SECOND libcd copy inside the disc (MOVOVL.EXE region).
- Shopping list reframed by identity: docs/closer/acquisitions.md — top item
  is now PsyQ SDK LIBCD.LIB (BIOS.OBJ = the original compiled object of
  bios.c with symbols; PsyQ 3.6/4.0-era should bracket v1.86), then BB2 JP
  (SLPS-01210), demos, BB1 for version-delta triangulation.

## Frontier (next Closer session)

1. **marionation close-out**: on the P6 chassis, fix the branch-destination
   pair via cross-jump-store-tail-merge label steering (mix exit forms /
   move `done:`), and hunt the single sll/addu transposition (the s33
   qty-tie machinery now applies to a ONE-pair residual on a 179/179 chassis).
   Also re-derive which FAKE wraps the volatile chassis makes REMOVABLE
   (volatile provides ordering/weighting some wraps coerced) — the Judge will
   demand the minimal-FAKE form, and partial-volatile (idx_1496 only) needs a
   ruling or replacement by a full-volatile form driven to ≤2.
2. **csmd4 respell**: replace h5 (now ruled fakematch) using the v1.86 idiom.
   The volatile family alone doesn't do it (P2/P3/P4); candidate direction:
   the CD_ready P6 lesson says volatility belongs on SOME accesses —
   enumerate which csmd4 accesses the original made volatile-effective vs
   pointer-cached (the callback arm vs window split), and re-run the s97 p79
   nrefs analysis on the faithful chassis.
3. **PsyQ LIBCD.LIB fetch** (sanctioned SDK corpus): BIOS.OBJ of the v1.8x era
   = original object code with symbol names; direct byte comparison against
   the BB2 windows; also brackets the v1.77→v1.86 source diff.
