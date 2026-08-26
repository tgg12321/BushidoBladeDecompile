/* CANDIDATE - func_800871D4 (Sony LIBSND `_SsVmKeyOffNow`) - session 7 (forensics, 2026-08-26)
   sandbox --disable all: score = 0   (target_insns = build_insns = 52, rules_dropped = 0)
   FULL BUILD verified this session: verify-oracle ok=true,
   build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true.
   ZERO regfix/asmfix rules, ZERO inline asm, ZERO /* FAKE */ construct, ZERO mask.

   STATUS: submitted as a RULING REQUEST, not as candidate-ready. The two
   staging locals (`okof1`/`okof2`) are close enough in shape to the s6 form
   the 2026-08-26 04:54 layer-1 review FAILed that a session may not
   self-approve them; the question this session asks the owner/Judge is stated
   at the bottom of this header and in the outcome JSON.

   WHAT THE LAYER-1 REVIEW ASKED FOR, AND WHAT MEASURING IT PROVED
   --------------------------------------------------------------
   The 04:54 layer-1 FAIL's explicit remedy was: "First measure the DIRECT
   form with zero intermediate locals - `D_801078D8 |= bitsLower;
   D_801078DA |= bitsUpper; D_800F1B10 &= ~D_801078D8;
   D_800F1B12 &= ~D_801078DA;`, grouped Sony's way". That form is byte-for-byte
   psyz's PsyQ-4.0 decomp of LIBSND/VM_NOWOF, and this session measured it on
   the current chassis: **score 8, build_insns 52**
   (rejected/s5-psyz-verbatim-order-score8.c, re-measured 2026-08-26).

   It is not merely "worse"; it is FORECLOSED, and the forensic dump names the
   mechanism. The target's instruction stream is:

       sb   $0,  D_800F4E35(voice slot .unk1b)     <- voice-slot byte clear
       lhu  $v1, D_801078D8                        <- key-off word 1 LOAD
       lhu  $a0, D_801078DA                        <- key-off word 2 LOAD
       sh   $0,  D_800F4E1C(voice slot .unk04)     <- voice-slot halfword clears
       sh   $0,  D_800F4E18(voice slot .unk0)
       ... or / sh D_801078D8 / nor / and / sh D_800F1B10 ...

   i.e. both key-off LOADS sit between the byte clear and the two halfword
   clears, while both key-off STORES sit below all three. In the direct form
   the load and the store are one statement, so no source ordering can put
   them on opposite sides of the halfword clears - and the scheduler cannot
   fix it. tmp/grind/func_800871D4/s4/sched_direct.txt (cc1 -dS slice for this
   function, direct form) shows sched.c's dependence lists:

       (insn 60 ... (set (mem:QI (plus:SI (reg:SI 90) (symbol_ref "D_800F4E35"))) 0)  ref_count = 8
       (insn 65 ... (set (mem:HI (plus:SI (reg:SI 90) (symbol_ref "D_800F4E1C"))) 0)  ref_count = 8
       (insn 70 ... (set (mem:HI (plus:SI (reg:SI 90) (symbol_ref "D_800F4E18"))) 0)  ref_count = 8
       (insn 73 (set (reg:HI 94) (mem:HI (symbol_ref "D_801078D8")))
                 ... (insn_list 60 (insn_list 65 (insn_list 70 ...

   Every later memory reference carries a true dependence on all three
   voice-slot stores: GCC 2.7.2's memrefs_conflict_p cannot disambiguate a MEM
   whose address is `(plus (reg) (symbol_ref))` from a MEM at a bare
   `(symbol_ref)`, so sched.c refuses to move any load across them. (It DOES
   disambiguate two bare symbol_refs with different symbols - the same dump
   shows sched1 hoisting insn 81, the D_801078DA load, above insn 76/78, the
   D_801078D8 or/store.) The pass and the decision are therefore named, not
   guessed: sched.c dependence construction, on a store the alias check must
   treat as may-alias.

   Conclusion: the target's memory order requires the loaded key-off words to
   be held in registers ACROSS the two voice-slot halfword clears. In C the
   only way to hold a value across an intervening statement is a local. That
   is why this body has `okof1`/`okof2`, and it is a different justification
   from the one the s6 form offered (which argued from global.c allocno
   priorities and was FAILed for exactly that reason).

   WHAT CHANGED vs THE FAILED s6 FORM
   ---------------------------------
   s6 wrote the tail as `D_801078D8 = okof1; D_800F1B10 &= ~okof1;` - the
   local read twice, and Sony's key-on line rewritten in terms of it. This
   form keeps Sony's key-on lines VERBATIM:

       D_800F1B10 &= ~D_801078D8;      == _svm_okon1 &= ~_svm_okof1;
       D_800F1B12 &= ~D_801078DA;      == _svm_okon2 &= ~_svm_okof2;

   so at the C level each staging local is written ONCE and read ONCE, and the
   only departure from psyz's source is that the two key-off words' new values
   are computed before the voice slot is released and committed after.

   The `nor $v1,$0,$v1` in the target - reusing the register rather than
   reloading the global - is produced by the COMPILER, not by the source:
   tmp/grind/func_800871D4/s4/cse_win.txt (the .cse slice for this form) shows
   cse.c store-to-load forwarding the re-read, so after cse the insn is
   `(insn 96 (set (reg:SI 103) (not:SI (reg:SI 96))))` where reg 96 is the OR
   result, and no reload of D_801078D8 is emitted. The source's re-read of the
   global is therefore honest C that costs nothing.

   Both `s32 okof1/okof2` and `u16 okof1/okof2` measure 0 / 52 insns; `u16` is
   adopted because it is the type of the globals being staged
   (tmp/grind/func_800871D4/s4/v_reread.c is the s32 variant).

   THE OPEN QUESTION (see outcome JSON `ruling_question`)
   -----------------------------------------------------
   Is a fresh, once-written / once-read local that stages a global's loaded
   value across an intervening may-alias store - where the RTL dumps show the
   staging is the ONLY C spelling of the target's memory order, and where the
   local's value is literally in the target's bytes - ordinary C needing no
   family, or must it be claimed under the named-intermediate family
   (.claude/rules/narrow-byte-args-packed-call.md + the 2026-08-17
   clarification) with a /* FAKE */ annotation? And does the existing ban on
   the s6 spelling (`... &= ~okof1`, RA-justified) extend to this one
   (`... &= ~D_801078D8`, memory-order-justified)?

   Symbol map (BB2 <- Sony): D_8010280A <- _svm_cur.voice; D_800F4E18 <-
   _svm_voice[] base (BB2 stride 54 vs stock PsyQ 4.0's 52 - the documented
   SpuVoice growth, memory/grind/note2pitch/psyz-sweep-2026-08-18.md), cleared
   fields at +0 (u16), +4 (u16) and +0x1d (u8); D_801078D8/DA <-
   _svm_okof1/_svm_okof2 (pending key-off masks, voices 0-15 / 16-23);
   D_800F1B10/12 <- _svm_okon1/_svm_okon2 (matching key-on masks). */
void func_800871D4(s32 mode) {
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;
    s32 idx;
    u16 okof1;
    u16 okof2;

    voice = D_8010280A;
    if (voice < 16) {
        bitsLower = 1 << voice;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (voice - 16);
    }
    idx = voice * 54;
    *(s8 *)((u8 *)&D_800F4E35 + idx) = 0;
    okof1 = D_801078D8 | bitsLower;
    okof2 = D_801078DA | bitsUpper;
    *(s16 *)((u8 *)&D_800F4E1C + idx) = 0;
    *(s16 *)((u8 *)&D_800F4E18 + idx) = 0;
    D_801078D8 = okof1;
    D_801078DA = okof2;
    D_800F1B10 &= ~D_801078D8;
    D_800F1B12 &= ~D_801078DA;
}
