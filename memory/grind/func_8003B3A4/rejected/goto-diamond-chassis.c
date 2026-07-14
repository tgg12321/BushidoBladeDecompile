/* REJECTED s4 (2026-07-14, permuter modality): goto-structured store diamond —
   alias-free clean chassis. Compiles to 55 insns (target 56) with the D_8010277D
   stores still in the assembler $at sb-macro form AFTER the value diamond; no
   pre-branch $v1 materialization. Permuter base score 730 (worse than the
   direct-conditional-write chassis at 520). A 26-min / 37,903-iteration random
   campaign seeded from this chassis (goto-diamond-randbasin) reached best 515 and
   surfaced ONLY known attractor classes (staged-flag local, constant-holder,
   identity inline_fn wrapper, param-alias copy, store reorders) — none alias-free
   AND none producing the register-indirect sb. Together with basin 1
   (clean-direct-write-randbasin: 8,770 iters, base 520, best 315, same attractor
   classes), this kills the hypothesis that whole-function random exploration can
   reach the target byte shape from a clean chassis. */
    if (a1 != 0) {
        goto set_e;
    }
    D_8010277D = 0x1D;
    goto after;
set_e:
    D_8010277D = 0xE;
after:
    D_8010277F = 0;
