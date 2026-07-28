/* s4 (2026-07-28): SANDBOX 0 (23/23 insns, 0 rules, 0 cheat-asm) — Judge ruling
   2026-07-28 06:28 (docs/grind/decisions.md): PASS, both constructs sanctioned.
   Re-applied to src and re-verified sandbox 0 in the follow-up session
   (tmp/grind/func_80037AA4/s4/sandbox_zero.json). Two load-bearing constructs:

   1. Guard `if (var_a1 < var_a2)` (a1 just zeroed; trip-0-equivalent to a2>0).
      Orphan-slt mechanism (s2, gdb-proven): expand emits a reg-reg slt pseudo;
      cse folds the operand; combine folds slt+branch -> blez, orphaning the
      pseudo; reload1.c alter_reg gives it a dead 4-byte slot -> the target's
      8-byte no-store frame (addiu sp,-8/+8 + final nop) arises naturally.
      Reviewer position: unenumerated frame-fabrication family, needs ruling.
      Worker position: ordinary two-variable compare, plausibly the original
      spelling (the original source DID produce vars=8 somehow), same class as
      sanctioned spelling-choice techniques.

   2. `s32 sh = 0xD` shift-amount constant-holder, /* FAKE */-annotated —
      named-local-fake-exception family (owner ruling 2026-07-01; SOTN
      src/dra/cd.c ships `new_var2 = 6` used once as a shift amount).
      Mechanism (s4, dump-proven in tmp/grind/func_80037AA4/s4/dump_mE/):
      the holder's li survives cse (guard JOIN blocks const-prop — same-bb
      forms were cse-folded in s3), so global-alloc sees sum at 11 refs/17
      live-length = pri 19411 < pointer 20000 -> target allocation (sum=$4,
      p=$3); then local-alloc update_equiv_regs marks the single-def pseudo
      == const 13 and reload substitutes it into the ashrsi3 and deletes the
      li — zero extra bytes (.combine shows reg-form sra, .greg shows
      const_int 13). This is s3's corridor A (post-greg-deleted sum-live
      insns) reached via reload constant-equivalence, a family s3 never
      enumerated.

   Found by permuter campaign (g1 seed, 56 iters), minimized from 3 mutations
   to 1 (pointer-temp + dup-return measured inert). */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;
    s32 sh; /* FAKE: shift-amount constant-holder (SOTN cd.c new_var2 shape) — survives
               cse past the guard join, lifts var_a0 to 11 refs/17 len (pri 19411 < 20000)
               so global-alloc assigns a0/v1 in target order; reload's constant-equivalence
               (update_equiv_regs) then substitutes 13 and deletes the li: zero extra bytes */

    sh = 0xD;
    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a1 < var_a2) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> sh;
    return 0xF - var_a0;
}
