# Evidence bank — func_800460E4

## [s4] 2026-08-25 — recon session after the second layer-1 FAIL (case-3 volatile REMOVED; new honest close at sandbox 0)

Context: the [s3] close was layer-1 FAILED (decisions.md 2026-08-25 03:53) solely
on the pre-existing case-3 volatile cast (`*(volatile s32 *)&ptr[-1]`), now in
the BANNED list for this function. This session removed it, diagnosed the 3-insn
fold it had been masking, and closed the residual with ordinary/named-intermediate
C. **Result: sandbox --disable all = 0 (248/248, rules_dropped=10), measured at
the closing chassis 2026-08-25 (twice: before and after the FAKE annotation on
the new intermediates).** The banned volatile is GONE; the banned arg1/s1 merge
remains absent.

### The fold, fully attributed (dump + disassembly proven)

With the volatile removed and everything else [s3]-identical: sandbox 9,
build 245/248. The 3 lost insns are case 3's `s4` ALIGN4 tail
(`srl $2,$2,2; sll $2,$2,2; addu $20,$16,$2`). Attribution chain:
1. sched1 places `li $2,1; sh $2,D_8009947A` BETWEEN the two `ptr[-N]` loads
   (fills the lw load-delay). The sh has NO dependence on the loads because the
   loads are `mem/s` (MEM_IN_STRUCT_P — the `ptr[-1]` indexing spelling) and
   sched.c exempts struct refs from conflicting with the fixed-symbol store.
2. With li/sh inside the address pseudo's live range, local-alloc seats the
   address in $a0 and the m1 value in $v0 (target: address $v0, m1 $a0).
3. Case 3's tail `srl/sll/addu $20,$16,$2` is then byte-identical to case 34's
   tail, and jump2 find_cross_jump merges the 3-insn suffix (case 3 ends
   `j .L31` into case 34; dumps/text1a_c2.s .L19/.L31 at the novol chassis,
   artifact s4/ours_novol.dis, s4/p4.dis).

### Kills (banked; do not re-probe)

- Statement-order sweep P1-P4 (store last / store mid / reads swapped /
  case-13-style ALIGN4-at-read): ALL flat at 9. Banked as
  rejected/case3-statement-order-sweep.c.
- sched_solver (model re-extracted at this chassis, parity=True): target order
  for the case-3 block (pass 1, block 19) is UNREACHABLE by perturbation —
  depth 2 over ALL 584 atoms, depth 3 over the 273 spellable luid/luid_move
  atoms. Conclusion: no statement reordering of the mem/s RTL reaches target;
  the original had a REAL dependence edge.

### The closing edit (case 3 only; rest of body [s3]-identical)

    s32 *pm2 = ptr - 2;            /* fresh, once-written once-read */
    s32 *pm1 = ptr - 1;
    s32 raw_m2 = *pm2;
    s32 raw_m1 = *pm1;
    D_8009947A = 1;
    s6 = (s32 *)((u8 *)s0 + ALIGN4(raw_m2));
    s4 = (s32 *)((u8 *)s0 + ALIGN4(raw_m1));

Mechanism (dump-proven at the closing chassis, dumps/text1a_c2.sched):
- The plain-var derefs emit `(mem:SI ...)` WITHOUT the /s flag (insns 312/315;
  combine folds the pointer decrements into `-8`/`-4` load offsets, zero extra
  bytes).
- The D_8009947A store (insn 320) now carries `REG_DEP_ANTI 312` and
  `REG_DEP_ANTI 315` — real anti-dependence edges force li/sh AFTER both
  loads = target order; the seats follow (address $v0, m1 $a0) and the
  case-34 tail merge is impossible (different registers).
- FAKE-annotated as named-intermediate family (fresh once-written/once-read
  locals, real consumed values, byte-neutral 248/248).

The [s3] chain-extender on s1 is still load-bearing and unchanged: removing it
at this chassis measures 32 (248/248 — the pure 3-seat rotation returns), so
the case-3 fix and the rotation fix are independent levers.

Integration note: unchanged from [s1]/[s3] — the 10 regfix rules at
regfix.txt:868-882 retire via the normal retire path; jtbls stay C-emitted;
wave-2 INCLUDE_RODATA remains unnecessary.

Artifacts: tmp/grind/func_800460E4/s4/ (apply.py, extract.py, solver.sh,
blocks.py, findblock.py, ours_novol.dis, p4.dis), tmp/grind/func_800460E4/dumps/
(full -da set at the closing chassis), tmp/sched_solver_work/text1a_c2.sched.json.

## [s1] 2026-08-25 — recon session (first session; closed to sandbox 0)

Chassis at session start: committed rule-era C body in src/text1a_c2.c + 10 regfix
rules (regfix.txt:868-882). Honest floor at start (sandbox --disable all,
rules dropped, this session's measurement): **35 / 248**, build_insns == target_insns.

### The two divergence families (mapped from the rule stack + target asm)

1. **3-way callee-saved rotation** ($17→$18, $18→$19, $19→$17 over insns 15-226,
   6 rules incl. prologue sw substs). Ground truth from the .greg dump
   (tmp/grind/func_800460E4/dumps/text1a_c2.greg, "Register dispositions" block):
   our pseudos 73 (arg1 param copy) → $19, 78 (s2 pointer) → $17, 79 (s3 count) →
   $18, 80 (s1 local) → $19. Target wants 73→$17, 78→$18, 79→$19, 80→$17
   (arg1 and the s1 local occupy $s1 with disjoint lifetimes in target;
   asm/funcs/func_800460E4.s lines 4-6, 119, 133-237).
2. **Four commutative `addu` operand-order diffs** (rules @76/@102/@129/@165):
   all four are exactly the `(s3 << 2)` scaled-index address computations
   (target emits offset-first: `addu a0,a1,s0` at 0x36A1C, `addu v0,a1,s1` at
   0x36A84, `addu v0,v0,s0` at 0x362F4-region, `addu v1,v1,s0` at 0x36B88).
   Every ALIGN4-derived address in target is base-first; ONLY the scaled-index
   sites are offset-first.

### Measured spelling facts (chassis: this session's src edits, -mel toolchain)

- `(s32 *)((u8 *)s0 + (s3 << 2))` (rule-era committed spelling) → base-first addu. ✗
- `&s0[s3]` (address-of-index) → STILL base-first addu (measured flat, 35). ✗
  GCC 2.7.2 expands &a[i] with the pointer as operand 0.
- RVALUE indexing `p[s3]` (a load) already emits offset-first (`sll; addu v0,v0,p;
  lw`) — matched target at those sites all along.
- `(s32 *)((s3 << 2) + (s32)s0)` (integer add, shift term first) → offset-first
  addu. ✓ Closed all four operand-order diffs (35 → 32 together with dropping the
  p/p2 alias locals).
- Dropping the `p` / `p2` param-alias locals (`s32 *p = (s32*)arg1;` etc.) in
  favor of direct `((s32 *)arg1)[s3]` reads: byte-neutral at those sites
  (loads identical), raises arg1 pseudo refs; on its own did NOT flip the
  rotation (still 32 = pure rotation residual).

### RA mechanism (solver-proven, not guessed)

- `tools/ra_solver/extract.py func_800460E4 text1a_c2` →
  tmp/ra_solver_work/func_800460E4.model.json; `simulate.py --trace` reproduces
  our build's dispositions **17/17** (sort order MATCH). Priorities at the
  32-floor chassis: 78 (s2) pri=4444, 79 (s3) pri=4210, 80 (s1 local) pri=3975,
  73 (arg1) pri=3703 — global.c `allocno_compare` allocates 78→$17, 79→$18,
  80→$19, 73→$19.
- `inverse.py global --goal {73:17, 80:17, 78:18, 79:19} --depth 2`: REACHABLE;
  **all 22 vectors are refs_up on pseudo 73 or 80** (minimal: +2 refs). No
  livelen or refs_down vector reaches the goal within bounds. Preference atoms
  FORECLOSED (callee-saved regs can't appear in pre-RA RTL).
- Honest ref-count audit: our arg1 refs (10) and s1-local refs (11) already
  equal target's visible $s1 uses in each lifetime segment — the original
  cannot have had MORE refs on two split pseudos. Therefore the original's
  priority lift must come from the two lifetimes being ONE pseudo = ONE C
  variable: merged refs ≈ 21 → floor_log2-weighted priority ≈ 5100 > 4444, so
  the merged variable is allocated first and takes $17; s2 then takes $18,
  s3 takes $19 — exactly target's seating, with no other assignment disturbed.

### The closing edit

Deleted the `s32 *s1;` local and used `arg1` (the param, dead after the fp_ptr
block) as the carrier for the value passed to func_80045600 (`arg1 = (s32)s4;`,
per-case `arg1 = (s32)s2;`, `arg1 = func_80044670(..., arg1);`,
`snd_SetVolume(arg1); arg1 = arg1 + snd_GetMaxFade();`). Zero added statements,
zero dead stores — every store's value is consumed on all paths. FAKE-annotated
at the first reuse site per the variable-reuse family gate
(.claude/rules/staged-value-reused-variable.md bounds; SOTN precedent
docs/reference/sotn-construct-index.md:51 "fake reuse of i", PSX provenance).

**Result: sandbox --disable all = 0 (248/248), measured twice this session
(before and after adding the FAKE annotation). rules_dropped=10,
cheat_asm_stripped=0.**

### Integration note (for the driver/operator)

The 10 regfix rules at regfix.txt:868-882 are still present (session may not
touch regfix.txt). With the new C body they no longer correspond to the build
output; the normal retire path must drop them when integrating. The function is
one of the jtbl-coupled deferred set (asm references the C-generated
jtbl_800152B4/jtbl_80015334) — it was already committed as a C body, so no
INCLUDE_ASM unwind is involved; wave-2 INCLUDE_RODATA remains dead per
docs/grind/borderline.md:88 and is NOT needed: this close keeps the body C and
the jtbls C-emitted.

Artifacts: tmp/grind/func_800460E4/s1/ (ours.dis, ours2.dis,
extract_candidate.py), tmp/grind/func_800460E4/dumps/ (full -da dump set),
tmp/ra_solver_work/func_800460E4.model.json.

## [s3] 2026-08-25 — recon session after the layer-1 FAIL (new close, banned merge reverted; NOTE: chronologically AFTER [s2] below)

Context: the [s1]/[s2] close was layer-1 FAILED (decisions.md 2026-08-25 03:37) —
construct (3), the whole-function arg1/s1 variable merge, is BANNED for this
function in any spelling. This session reverted it and closed the rotation by a
different, sanctioned attack.

### Honest floor re-established without the banned construct

Reverted state = [s1] candidate MINUS the merge (s32 *s1; restored as the
carrier; arg1 dead after the fp_ptr block; all [s1] spelling wins kept: the
four `(s32 *)((s3 << 2) + (s32)s0)` integer-add scaled-index sites + the p/p2
alias drops). Measured THIS session: **sandbox --disable all = 32** (248/248,
rules_dropped=10) — the pure 3-seat rotation, exactly the [s1] banked residual.
canonical: verdict C (distance 32). No duplicate leads (tmp/duplicates_leads.txt
has no entry for this function).

### Structural kill: duplicated-statement-into-arms has NO site here

Target asm mapping (asm/funcs/func_800460E4.s): every $s1 ref is byte-pinned —
prologue `addu $s1,$a1,$zero` (line 6) + 9 more arg1-lifetime uses = 10 = our
pseudo-73 refs; `addu $s1,$s4,$zero` in the range-guard beqz delay slot (line
133) + per-case heads `addu $s1,$s2,$zero` (142/158/182/205) + case-11 pair
(176/180) + case-4/7/18 pair (167/173) + final call read (237) = 11 = our
pseudo-80 refs. The `s1 = s2;` stores are all arm HEADS; cross-jump merges only
TAILS, so no real-statement duplication can add flow-visible-but-byte-dead refs
on 73/80 in this control structure. The [[duplicated-statement-into-arms]]
family (the usual byte-free ref-lift) is therefore structurally unavailable —
this completes the exhaustion chain: solver inverse says ONLY refs_up(73|80)+2
reaches the target seating; honest visible refs already equal target's; dead
stores are INERT for global RA (flow deletes before counting, motion_SetMotion
measurement); duplication has no site.

### The closing edit — sanctioned combine-foldable chain-extender

`.claude/rules/dead-store-fake-exception.md:32` (owner ruling 2026-07-01
same-day scope extension): a LIVE store routed through an algebraically-
equivalent detour that combine folds back to the direct form with zero emitted
bytes — surviving effect = the extra reg_n_refs flow.c records. Applied to
s1's default init:

    s1 = (s32 *)((s32)s4 - (s32)s0);   /* FAKE-annotated in src */
    s1 = (s32 *)((s32)s1 + (s32)s0);

Dump-proven mechanism (tmp/grind/func_800460E4/dumps/, regenerated this
session at the closing chassis):
- text1a_c2.flow:22 — "Register 80 used 13 times" (11 honest + 2 detour).
- text1a_c2.greg:29-31 — dispositions **73 in 17, 78 in 18, 79 in 19,
  80 in 17** = target's exact seating (arg1 and s1 share $s1 with disjoint
  lifetimes; s2→$s2, s3→$s3).
- Zero-byte fold verified: build_insns == target_insns == 248 AND score 0
  (no subtract materializes; the sandbox compares full opcodes, only branch
  targets masked).

**Result: sandbox --disable all = 0 (248/248), measured twice this session
(before and after adding the FAKE annotation). rules_dropped=10,
cheat_asm_stripped=0.** The banned construct is ABSENT: s1 is a real local
carrier, arg1 is never written, no assignment/use site is repointed.

### Pre-existing case-3 volatile read — measured load-bearing, flagged

`s32 raw_m1 = *(volatile s32 *)&ptr[-1];` (case 3) predates every grind session
(rule-era committed body; unchanged by [s1]/[s3] diffs; present in the body the
2026-08-25 03:37 layer-1 review examined and did not cite). Probed this session:
removing the qualifier → sandbox 9, build 245/248 — the compiler folds 3 insns
target keeps. Load-bearing; fold not diagnosed; restored as inherited. Banked as
rejected/case3-plain-read-no-volatile.c and disclosed in self_vet.md so the
reviewer rules on it with the measurement in hand.

Integration note: unchanged from [s1] — the 10 regfix rules at
regfix.txt:868-882 retire via the normal retire path; jtbls stay C-emitted;
wave-2 INCLUDE_RODATA remains unnecessary and dead.

## [s2] 2026-08-25 — resubmission session (vet-format fix only)

The prior session's candidate-ready outcome was DISCARDED by the driver
validator on self_vet.md FORMAT alone: the SANCTIONED-FAMILY-CLAIMS SCOPE
quote was line-wrapped, and grindlib's `_SCOPE_LINE` regex
(tools/grinder/grindlib.py:64) requires the entire `SCOPE: "..."` quote on a
single line ending with the closing quote. No technical defect was cited.

This session: re-applied memory/grind/func_800460E4/candidate.c verbatim to
src/text1a_c2.c (splice script tmp/grind/func_800460E4/s1/apply.py, binary
I/O, LF preserved) and re-measured: **sandbox --disable all = 0 (248/248,
rules_dropped=10, cheat_asm_stripped=0) THIS session.** Zero new constructs;
the diff vs the [s1] candidate is empty. self_vet.md rewritten with the scope
sentence of staged-value-reused-variable.md bound 2 quoted verbatim on one
line (validated with grindlib.validate_self_vet → (True, '')). Owner directive
(RULES-TO-ZERO CAMPAIGN 2026-08-24) acknowledged and executed: this close is
the COMPLETED-C path that retires the function's 10 regfix rules at
integration (regfix.txt:868-882, operator/retire-path surface — see the
integration note in [s1]).
