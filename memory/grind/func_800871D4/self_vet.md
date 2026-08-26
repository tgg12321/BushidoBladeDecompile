# SELF-VET — func_800871D4  (session 8 / this session; supersedes the s7 vet)

Measured THIS session with the body below in place in src/main.c:
`sandbox func_800871D4 --disable all` -> score 0, target_insns == build_insns == 52,
rules_dropped 0. `verify-oracle` -> ok=true, build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true.
Zero regfix/asmfix rules, zero inline asm, zero mask, zero register pin.

CONSTRUCTS: (1) two fresh `u16` staging locals `okof1`/`okof2`, each written once
and read once, holding the OR'd key-off words across the two voice-slot halfword
clears; (2) `u16 voice` local (the type of the global it loads, D_8010280A) and
`idx = voice * 54` byte-offset addressing of the voice-slot fields — ordinary C,
matching the Sony/psyz reference shape.

## T1 semantic purpose
(1) okof1/okof2: YES — they carry real values. Each holds `D_801078D8 | bitsLower`
/ `D_801078DA | bitsUpper`, the pending key-off masks, and those exact values are
in the target's bytes (the `or` + `sh` pairs at D_801078D8 and D_801078DA).
Removing them is not a no-op simplification: it forces the load and the store of
each key-off word into one statement, which changes the function's *memory order*
relative to the voice-slot release. The routine computes the new key-off state
from the pre-release voice index and commits it after the slot is released; the
locals are what makes that phase distinction expressible. Behaviour is
observationally the same, but the construct is not a decoration on a simpler form
— it is the only C spelling of the target's memory order (see T3).
(2) `voice`/`idx`: ordinary; `voice` is the declared type of D_8010280A and `idx`
is the array-stride computation the function genuinely performs.

## T2 human-programmer
(1) A programmer writing "latch the new key-off words, release the voice slot,
then publish them" writes exactly these two locals. Sony's own source groups the
two key-off commits together and then the two key-on updates; this form keeps that
grouping verbatim. A reader does not ask "why is this here?" — the locals are
named for the values they hold and each is consumed two lines later. The one thing
a reader *could* question is why the two clears sit between the staging and the
commit; that is why the /* FAKE */ annotation is present rather than absent.
(2) `voice`/`idx`: yes, unremarkable.

## T3 GCC-internals justification
Partly yes, and it is disclosed rather than hidden — which is why this is filed as
a claimed sanctioned family with a FAKE annotation, not as ordinary C.
The mechanism is sched.c's alias-driven dependence construction, and it is
dump-proven in THIS form's own dump (tmp/grind/func_800871D4/s4/sched_win.txt,
produced this session from the instrumented tools/gcc-2.7.2/cc1 via
tools/grinder/dump.ps1):

    insn 60  (set (mem:QI (plus (reg 92) (symbol_ref "D_800F4E35"))) 0)   byte clear
    insn 63  (set (reg:HI 94) (mem:HI (symbol_ref "D_801078D8")))         key-off load 1
    insn 71  (set (reg:HI 97) (mem:HI (symbol_ref "D_801078DA")))         key-off load 2
    insn 81  (set (mem:HI (plus (reg 92) (symbol_ref "D_800F4E1C"))) 0)   halfword clear
             -> (insn_list 53 (insn_list:REG_DEP_ANTI 63 (insn_list:REG_DEP_ANTI 71 (nil))))
    insn 86  (set (mem:HI (plus (reg 92) (symbol_ref "D_800F4E18"))) 0)   halfword clear
             -> same two REG_DEP_ANTI edges on 63 and 71
    insn 89  (set (mem:HI (symbol_ref "D_801078D8")) ...)                 key-off store
             -> (insn_list:REG_DEP_OUTPUT 60 (insn_list:REG_DEP_OUTPUT 81
                 (insn_list:REG_DEP_OUTPUT 86 ...)))

GCC 2.7.2's memrefs_conflict_p cannot disambiguate a `(plus (reg) (symbol_ref))`
MEM from a bare `(symbol_ref)` MEM, so the loads are pinned ABOVE the clears by
anti-dependences and the stores pinned BELOW them by output dependences. The
target requires the loads above and the stores below. Therefore the loaded values
must be held in registers across the clears, and the only C construct that holds a
value across an intervening statement is a local.

The direct / psyz-verbatim form (`D_801078D8 |= bitsLower;` — load and store in one
statement) was re-measured this campaign at score 8 / 52 insns and is FORECLOSED,
not merely worse (memory/grind/func_800871D4/rejected/s5-psyz-verbatim-order-score8.c;
dump tmp/grind/func_800871D4/s4/sched_direct.txt). Control datum from the same
dump: sched1 DID reorder two bare-symbol MEMs with different symbols, so the
barrier is specifically the register-plus-symbol address.

NOTE the deliberate difference from the s6 form the 2026-08-26 04:54 layer-1 review
FAILed: that form was justified by global.c allocno_compare priorities and read the
local a second time (`D_800F1B10 &= ~okof1;`). This form's justification is memory
order, and Sony's key-on lines are kept verbatim (`D_800F1B10 &= ~D_801078D8;`), so
each local is once-written / once-read. The target's `nor` (register reuse rather
than a reload of the global) is produced by cse.c store-to-load forwarding, not by
the source (tmp/grind/func_800871D4/s4/cse_win.txt:
`(insn 96 (set (reg:SI 103) (not:SI (reg:SI 96))))`, no reload of D_801078D8
survives).

## T4 permuter/search provenance
No. The form was derived from psyz's PsyQ-4.0 `LIBSND/VM_NOWOF` `_SsVmKeyOffNow`
source plus the dump-named sched.c foreclosure above. The ~2550-iteration permuter
campaign on this function (tmp/grind/func_800871D4/s4/permuter_campaign.log)
returned NO novel find; it did not produce this body.

## T5 family check
Claimed under the frozen **Named-intermediate declaration order** entry as clarified
by the owner 2026-08-17 (see SANCTIONED-FAMILY-CLAIMS below), with the /* FAKE */
annotation present. It is NOT: a register pin, hardcoded-$N asm, a scheduling
barrier, a volatile coercion, a dead store or dead local (both locals are read), a
redundant width cast (the banned dual-use `andi` construct is absent — there is no
mask anywhere in this body), a phase-split RMW carrier (the banned keyoff_lo/hi
4-local split and its 2-local respelling both cached the global's *pre-OR* value in
a local and rewrote Sony's key-on lines in terms of it; here the OR is a single
expression and the key-on lines are verbatim Sony), or an aggregate merge. Per the
2026-08-26 05:09 ruling the s6 ban is narrowed, not extended, to this spelling: its
decisive defect was the local's second read. The twice-read tail must not be revived.

## T6 naming-announces-intent
No. `okof1`/`okof2` are Sony's own symbol names for the values held (`_svm_okof1` /
`_svm_okof2`); `voice`, `idx`, `bitsLower`, `bitsUpper` are semantic. No
`pad`/`dummy`/`unused`/`spill`/`tmp` naming, and every local is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order (frozen SOTN entry + owner
          clarification 2026-08-17, func_8001979C escalation)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written, once-read — multi-write carriers are NOT this entry (the `y1` FAIL, decisions.md:1833, stands); (2) real value — the intermediate holds a computation that appears in the target's own bytes and only relocates where the value is named; pure no-op copies stay with the dead-store family and its prerequisites; (3) byte-neutral — `build_insns == target_insns`, the compiler folds the copy; (4) fresh local, not a borrow — [[staged-value-reused-variable]] keeps its own bounds; (5) destination not live-pre-initialized (the `x/tx` FAIL, decisions.md:4251, stands); (6) standard prerequisites: dump-proven named mechanism, documented lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2 review."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189
  PRECEDENT: docs/reference/sotn-construct-index.md:113
  PREREQUISITE CHECKLIST (all six, verified against this diff):
   (1) once-written, once-read — okof1: written at `okof1 = D_801078D8 | bitsLower;`,
       read at `D_801078D8 = okof1;`. okof2: symmetric. No other occurrence of
       either identifier in the function. Sony's key-on lines re-read the GLOBAL,
       not the local.
   (2) real value — the OR'd key-off words are in the target's bytes (`or` +
       `sh D_801078D8` / `sh D_801078DA`); the locals only name where the value
       already exists. Not a no-op copy.
   (3) byte-neutral — build_insns == target_insns == 52, score 0, measured this
       session; full-build SHA1 == oracle.
   (4) fresh local, not a borrow — both declared in this function, no prior value,
       not reused for any second purpose.
   (5) destination not live-pre-initialized — okof1/okof2 have no initializer and
       no store before their single write.
   (6) dump-proven named mechanism (sched.c dependence construction /
       memrefs_conflict_p — sched_win.txt this session, sched_direct.txt s7);
       documented lever exhaustion across ledger sessions s3-s7 including the
       direct Sony form re-measured at score 8, the ~2550-iteration permuter
       negative, and 20 banked rejected forms in
       memory/grind/func_800871D4/rejected/; /* FAKE: ... */ annotation present
       (below); layer-1 + layer-2 review pending on this submission.

ANNOTATION-CONFORMANCE:
  /* FAKE: named-intermediate (no-new-park-categories.md 'Named-intermediate
     declaration order' + owner clarification 2026-08-17) - okof1/okof2 stage
     the key-off words across the voice-slot halfword clears; mechanism:
     sched.c cannot move a bare-symbol MEM across a (plus (reg) (symbol_ref))
     store; lever-exhaustion: memory/grind/func_800871D4/hypotheses.md [s3]-[s7]
     + evidence.md (direct Sony form re-measured at 8). */
  Present in src/main.c immediately above the two staging lines. Carries all three
  required parts: what (named-intermediate staging of the key-off words),
  mechanism (sched.c / bare-symbol MEM vs (plus (reg) (symbol_ref)) store), and
  lever-exhaustion (ledger pointer). Wording follows the 2026-08-26 05:09 Judge
  ruling's prescribed text, with the lever-exhaustion clause appended as the FAKE
  template requires.
