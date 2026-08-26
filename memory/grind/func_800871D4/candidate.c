/* CANDIDATE - func_800871D4 (Sony LIBSND `_SsVmKeyOffNow`) - session 5 (synthesis, 2026-08-26)
   sandbox --disable all: score = 0   (target_insns = build_insns = 52, rules_dropped = 0)
   FULL BUILD verified this session: verify-oracle ok=true,
   build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true.
   ZERO regfix/asmfix rules, ZERO inline asm, ZERO /* FAKE */ construct,
   ZERO redundant-width mask.

   WHY THIS SUPERSEDES THE LAYER-1-FAILED s4 FORM
   ----------------------------------------------
   s4 also reached score 0, but it manufactured the target's two
   `andi $v1,$a0,0xFFFF` instructions with `vc = raw & 0xFFFF` written twice
   over a `u16`-typed load - a mask that is a semantic no-op, kept alive only
   because two uses of the raw load make combine.c's can_combine_p refuse to
   fold it. The layer-1 cheat-reviewer FAILed that as family F2
   (redundant width casts), correctly.

   This session removed the masks entirely. The two `andi`s are produced by
   GCC 2.7.2's PROMOTE_MODE handling of an ordinary `u16` local: `voice` lives
   in an SImode pseudo whose upper bits are not known clean, so every use of
   it in `int` context is a `zero_extend` - insn 13 (before the `sltiu`) and
   the re-materialisation at the end of the else arm, after `voice - 16` has
   consumed the extended copy. That is exactly the declaration Sony's own
   source uses: psyz's PsyQ-4.0 decomp of LIBSND/VM_NOWOF
   (tmp/psyq_prov/psyz/decomp/src/libsnd/vm_nowof.c) declares
   `u16 voice; voice = _svm_cur.voice;` inside `_SsVmKeyOffNow(int mode)`.
   No mask appears anywhere in Sony's source, and none appears here.

   PROVENANCE (this is a near-verbatim transplant, not a reconstruction)
   --------------------------------------------------------------------
   psyz `_SsVmKeyOffNow`:
       void _SsVmKeyOffNow(int mode) {
           int bitsUpper; int bitsLower; u16 voice;
           voice = _svm_cur.voice;
           if (voice < 16) { bitsLower = 1 << voice;        bitsUpper = 0; }
           else            { bitsLower = 0; bitsUpper = 1 << (voice - 16); }
           _svm_voice[voice].unk1b = 0;
           _svm_voice[voice].unk04 = 0;
           _svm_voice[voice].unk0  = 0;
           _svm_okof1 |= bitsLower;  _svm_okof2 |= bitsUpper;
           _svm_okon1 &= ~_svm_okof1; _svm_okon2 &= ~_svm_okof2;
       }
   Symbol map (BB2 <- Sony): D_8010280A <- _svm_cur.voice;
   D_800F4E18 <- _svm_voice[] base (BB2 stride is 54, stock PsyQ 4.0 is 52 -
   the documented 2-byte SpuVoice growth, memory/grind/note2pitch/
   psyz-sweep-2026-08-18.md); the cleared fields are at +0x1d (byte), +4 and
   +0; D_801078D8/DA <- _svm_okof1/2 (pending key-off masks, voices 0-15 /
   16-23); D_800F1B10/12 <- _svm_okon1/2 (matching key-on masks), which is
   why each key-off bit is also cleared from the key-on word.
   Applied verbatim, psyz's statement order measures score 8 / 52 insns
   (banked rejected/s5-psyz-verbatim-order-score8.c) - correct registers,
   correct instructions, wrong emission order for the two key-off loads.

   THE ONE DELIBERATE DEPARTURE FROM psyz, AND WHY IT IS ORDINARY C
   ---------------------------------------------------------------
   BB2's build reads both key-off words, then clears the voice slot, then
   applies both masks and both key-on updates - i.e. the two 24-voice halves
   are handled in parallel rather than one whole read-modify-write group at a
   time. Two independent measurements force this, and neither is a codegen
   trick:
     (1) GCC 2.7.2's schedulers will not move ANY memory reference across a
         store whose address is (plus reg symbol_ref) - the alias check cannot
         disambiguate `_svm_voice[voice].x` from a fixed global. Measured
         directly: rejected/s5-okof-rmw-before-clears-score12.c moves the
         key-off updates above the voice clears and the emitted memory order
         follows the source exactly (score 12). So the target's order - both
         key-off LOADS above the two `sh $zero`, both key-off STORES below
         them - can only come from source in which the loads are separated
         from the stores.
     (2) With the loads separated but the write-backs still done one group at
         a time (rejected/s5-grouped-writeback-score6-a1a2-swap.c) the stream
         is instruction-exact but the two mask locals take each other's seats
         ($a1 <-> $a2), because global.c's allocno_compare
         (floor_log2(n_refs)*n_refs*size / live_length) ranks bitsLower at
         3/19 = .158 above bitsUpper at 3/21 = .143. Interleaving the
         write-backs shortens bitsUpper's live range to <= bitsLower's, which
         is the `live_shrink pseudo 73: 21->19` atom that
         tools/ra_solver/inverse.py ranks as solution #2 for the swap.
   Four source permutations that keep the grouped write-back were measured and
   all stay at 6 (rejected/s5-or-swap-plus-keyon-hoist-score6.c and the
   notes in hypotheses.md): sched1 sinks each `or` next to its own store
   regardless of source order, so the live-range gap cannot be closed from
   inside the grouped shape.

   Every local here is written and read; nothing is dead, nothing is masked
   for effect, nothing is annotated. See self_vet.md. */
void func_800871D4(s32 mode)
{
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;
    s32 idx;
    u16 keyoff_lo;
    u16 keyoff_hi;
    u16 keyon_lo;
    u16 keyon_hi;

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
    keyoff_lo = D_801078D8;
    keyoff_hi = D_801078DA;
    *(s16 *)((u8 *)&D_800F4E1C + idx) = 0;
    *(s16 *)((u8 *)&D_800F4E18 + idx) = 0;
    keyon_lo = D_800F1B10;
    keyon_hi = D_800F1B12;
    keyoff_lo |= bitsLower;
    keyoff_hi |= bitsUpper;
    D_801078D8 = keyoff_lo;
    D_801078DA = keyoff_hi;
    D_800F1B10 = keyon_lo & ~keyoff_lo;
    D_800F1B12 = keyon_hi & ~keyoff_hi;
}
