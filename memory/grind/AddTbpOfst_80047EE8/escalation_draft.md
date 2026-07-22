# DRAFT — mirroring OWNER-ESCALATION for AddTbpOfst_80047EE8

Prepared by grind s10 (synthesis). Ready to file VERBATIM into docs/grind/decisions.md
by an escalation-modality session. Species CONFIRMED: both AND-gates of
endgame-lock-disposition measured dead directly on this function (s10). Per the
parallel-filing precedent (func_80049A2C, gnd_init_80041688, InitHiraRmd_80047FBC all
filed while awaiting ruling), this need NOT wait for the sibling ruling to land.

---

## 2026-07-2X — AddTbpOfst_80047EE8 (src/text1b.c) — **OWNER-ESCALATION** (filed per the standing 2026-07-20 endgame-lock-disposition rule; awaiting owner ruling — do not self-resolve)

**The question:** every sanctioned pure-C axis across s1-s10 is measured dead on the
+32-byte locals-frame slot the target prologue demands. The composite match on main
requires TWO constructs together, one Judge-blessed under a narrow carve-out and one
with no covering sanctioned family:

- (i) `arg0 = 0; /* FAKE */` — dead-store-fake-exception (2026-07-01); lever-exhaustion
  DISCHARGED s2 (6 pure spellings of the saved/p init chain measured dead on this body:
  const decl-init / decl-order / split-init / u32 retype / two-statement rebind all = 11,
  mask offset = 14 byte-diverging, FAKE removed = 11; FAKE uniquely = 10). Same cse2
  canonical-register mechanism the sibling's FAKE half was Judge-PASSed under (line 981).
- (ii) An unused, unwritten ~32-byte local array (`s32 buf[7-8]; (void)buf;`) whose sole
  effect is to hold `vars=32` in `.frame $sp,72` so the epilogue byte-matches target.
  NO sanctioned covering family: dead-vars-local-array 2026-07-01 WRITTEN carve-out
  requires target sw stores in the locals region — asm/funcs/AddTbpOfst_80047EE8.s has
  ZERO sw/lw in sp+0x18..sp+0x37 (target's only sp stores are the 4 reg-saves
  s0/s1/s2/ra @0x38-0x44 + the 5th outgoing efc arg $v0@0x10). No SOTN precedent for an
  unwritten local array acting as a phantom frame carrier.

**Two mutually exclusive owner options:**

- **(a) Sanction the composite as a new coercion family** contingent on SOTN evidence for
  the buf[] half. Honest cost: no such SOTN precedent exists (s6/s7 surveys). I state this
  plainly rather than argue around it.
- **(b) Refuse the family and accept INCOMPLETE-owner-accepted** per endgame-lock-disposition.md.
  Current cheat form on main retained solely to hold the oracle byte match; parked out of
  active grind, not COMPLETED-C, eligible for re-attempt if a genuine pure-C lever emerges.
  No new rule text, no engine detector weakening, no cheat carve-out precedent.

**Frame layout (decoded from asm/funcs/AddTbpOfst_80047EE8.s):** `.frame $sp,72`;
args 0x00-0x17 (24, 5th efc arg $v0@0x10), vars 0x18-0x37 (32 phantom, ZERO sw/lw),
regs 0x38-0x47 (s0@0x38 s1@0x3C s2@0x40 ra@0x44). cc1 size-pin (s7): vars=32 pins the
dead source aggregate at 25-32 bytes = a 7- or 8-word int array; a same-size struct
scalarizes to vars=0, so ARRAY-only.

**Ledger of exhaustion (s1-s10; do not re-derive):**
- s1 recon — floor 15->10 (single-walker + live first-arg precompute + FAKE arg0=0). Residual = exactly the 10 frame-offset insns.
- s2 structural — F1 FAKE-arg0 lever-exhaustion discharged (6 pure spellings dead).
- s3 structural — F2 phantom-frame 9-variant grid dead; v08 (exact tslLineG5Init trigger) vars=0; only a WRITTEN s32 rec[6] reserves vars, at +6 diverging stores target lacks.
- s4 permuter — floor-10 chassis, 9451 iters, --stack-diffs; only lever = forbidden volatile-pad (~8B), best 202, never 0.
- s5 permuter — floor-11 no-FAKE chassis, 17057 iters; plateau 207, only forbidden volatile-uint-pad; two-basin ~26500 iters both dead.
- s6 forensics — NAMED function.c assign_stack_local at RTL-expand from a source-level dead local-aggregate DECL; reload-spill theory FALSIFIED (get_frame_size=0 both passes; zero region sw/lw); positive control int buf[8] reproduces target shape exactly.
- s7 forensics — cluster-invariant 32B phantom across all 4 members (4/6/8/4 saved regs); cc1 size/type pin; array-only (struct scalarizes to vars=0).
- s8 rederive — m2c arg0-live=35 (one insn short, loses $s2 binding) / Kengo no source / decomp.me shingle noise.
- s9 rederive — sibling u32*base chassis transplant = co-optimal floor 10; chassis-invariant frame-only residual (third distinct reaching chassis).
- s10 synthesis — both AND-gates measured dead directly: (a) scan_hand_coded tier=LOW 1/8 no STRONG signals, canonical verdict C dist 8; (b) no SOTN precedent, dead-array-only. Species CONFIRMED.

**Governing rule:** .claude/rules/endgame-lock-disposition.md (standing 2026-07-20).
Textbook species fit: bytes provable on main via composite, FAKE half mechanism-blessed
on the sibling, hand-coded certification NEGATIVE (ordinary table walker), no SOTN
precedent for the buf[] half.

**Precedent (same species, all ruled option (b) since 2026-07-20):** motion_SetMotion
(FAMILY REFUSED), saTan0Init, cpu_side_move_dir_4, func_80057CC8 (all REFUSED /
OWNER-ACCEPTED INCOMPLETE); func_80049A2C, InitHiraRmd_80047FBC (sibling cluster member),
gnd_init_80041688 filed 2026-07-20 awaiting ruling. The agent does not self-resolve.
